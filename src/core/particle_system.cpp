#include "core/particle_system.h"

#include <stdexcept>
#include <CL/cl2.hpp>
#include <memory>
#include <algorithm>
#include <cctype>
#include <cmath>

#include "resources/opencl_kernels.h"
#include "core/data_variable.h"

#include "utilities/load_file.h"

ParticleSystem::ParticleSystem(cl::Platform & platform,
                               cl::Device & device,
                               cl::Context & context,
                               cl::CommandQueue & command_queue) : 
	data_system(platform, device, context, command_queue)
{
	// Load kernels:
	kernel_random_deletion = get_kernel_from_file("cl_kernels/delete_particles_random.cl", "membrane_simulate_particles");
	kernel_concatenate = get_kernel_from_file("cl_kernels/sim_cell_division2.cl", "concatenate");
	kernel_delete_sort = get_kernel_from_file("cl_kernels/delete_particles.cl", "sort_deleted_list");

	// Test count:
	particle_count = 32*32;	// TODO: remove
	particle_count = 1;
	//particle_count = 0;
	maximal_cell_count = 32*32*32*32;

	// Get standard functions for growing and shrinking arrays:
	std::string kernel_code_1 = load_file("cl_kernels/append_buffer.cl");
	std::string kernel_code_2 = load_file("cl_kernels/delete_particles.cl");
	cl::Program::Sources sources;
	sources.push_back({kernel_code_1.c_str(), kernel_code_1.length()});
	sources.push_back({kernel_code_2.c_str(), kernel_code_2.length()});

	m_standard_functions = cl::Program(m_context, sources);
	if(m_standard_functions.build({m_device}) != CL_SUCCESS)
	{
		message_error("Error building: " << m_standard_functions.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device));
		exit(1);
	}

	setupBuffers();
}
ParticleSystem::~ParticleSystem() { }

void ParticleSystem::setupBuffers()
{
	new_cell_indices = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);
	new_cell_group_size = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count/workgroup_size);
	deleted_cell_indices = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);
	deleted_cell_group_size = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count/workgroup_size);
	empty_cells = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);				
	copied_cells = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);
}

std::vector<event_info> ParticleSystem::customDeletionFunction(cl::Buffer& empty_cells,
                                            unsigned int& empty_count)
{
	//std::vector<event_info> info;
	// TODO: return info vector
	for(auto const& value: m_managed_arrays)
	{
		value->performDeletion(m_command_queue, empty_cells, empty_count);
	}
	
	return {};
}

std::vector<event_info> ParticleSystem::customDuplicationFunction(cl::Buffer& empty_particles,
                                               cl::Buffer& copied_particles,
                                               unsigned int& copied_count)
{
	for(auto const& value: m_managed_arrays)
	{
		value->performDuplication(m_command_queue, empty_particles, copied_particles, copied_count);
	}
	
	return {};
}

std::vector<event_info> ParticleSystem::finalizeDuplicationDeletion()
{
	// ************************************************** //
	//  Gather deletion information and delete particles  //
	// ************************************************** //
	
	// Get the list of particle indices to be deleted:
	// This list is expected to be concatenated already by a previous kernel.
	unsigned int array_cell_deleted_group_size[maximal_cell_count/workgroup_size];

	m_command_queue.enqueueReadBuffer(deleted_cell_group_size, CL_TRUE, 0, maximal_cell_count/workgroup_size*sizeof(cl_uint), array_cell_deleted_group_size, nullptr);
	m_command_queue.finish();

	// Count the number of particles that have to be removed to determine the ndrange of the removal kernels:
	unsigned int empty_count = 0;
	for(int i=0; i<std::ceil(double(particle_count)/256.0); i++)
	{
		empty_count += array_cell_deleted_group_size[i];
	}

	//message_debug("ARRAY_DELETED: " << array_cell_deleted_group_size[0] << array_cell_deleted_group_size[1] << array_cell_deleted_group_size[2] << array_cell_deleted_group_size[3]);

	if(empty_count != 0)
	{
		message_debug("empty_count = " << empty_count << " | particle count: " << particle_count);
		
		// Send signal to rearrange the variable list in the derived class:
		if(empty_count <= particle_count)
		{
			customDeletionFunction(empty_cells, empty_count);
			particle_count -= empty_count;
		} else {
			message_error("Tried to delete more particles than currently present! This is a bug.");
		}
	}

	

	m_command_queue.finish();
	//TODO: REMOVE ANY AND ALL BLOCKING CALLS!!!!

	

	// **************************************************** //
	//  Gather duplication information and duplicate cells  //
	// **************************************************** //

	cl::Event event_read_group_size;
	cl::Event event_fill_new_cell_group_size;
	cl::Event event_fill_copied_cells;

	unsigned int array_cell_group_size[maximal_cell_count/workgroup_size];
	//queue.enqueueReadBuffer(new_cell_group_size, CL_TRUE, 0, std::ceil(double(cell_count)/256.0)*sizeof(cl_uint), array_cell_group_size, nullptr, &event_read_group_size);
	m_command_queue.enqueueReadBuffer(new_cell_group_size, CL_TRUE, 0, maximal_cell_count/workgroup_size*sizeof(cl_uint), array_cell_group_size, nullptr, &event_read_group_size);
	m_command_queue.finish();

	copied_count = 0;
	for(int i=0; i<std::ceil(double(particle_count)/256.0); i++)
	{
		copied_count += array_cell_group_size[i];
		
	}

	if(copied_count != 0)
	{
		message_debug("copied_count = " << copied_count);
		if(copied_count + particle_count < maximal_cell_count)
		{
			customDuplicationFunction(empty_cells, copied_cells, copied_count);
			particle_count += copied_count;
		} else {
			message_error("Particle count has exceeded limits.");
		}
	}

	message_debug("CURRENT PARTICLE COUNT: " << particle_count);

	// ********************************************** //
	//      Clear division and deletion buffers       //
	// ********************************************** //

	cl_int fill_pattern = 0;
	
	m_command_queue.enqueueFillBuffer(new_cell_group_size, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count/workgroup_size, nullptr, &event_fill_new_cell_group_size);
	m_command_queue.enqueueFillBuffer(copied_cells, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count, nullptr, &event_fill_copied_cells);
	m_command_queue.enqueueFillBuffer(deleted_cell_group_size, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count/workgroup_size, nullptr);
	m_command_queue.enqueueFillBuffer(empty_cells, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count, nullptr);
	m_command_queue.enqueueFillBuffer(deleted_cell_indices, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count, nullptr);
	
	return {{event_read_group_size, "CELL_SYSTEM: read group size"},
		{event_fill_new_cell_group_size, "CELL_SYSTEM: fill new cell group size"},
		{event_fill_copied_cells, "CELL_SYSTEM: fill copied cells"}};

	//return {};
}

unsigned int ParticleSystem::getMaximalParticleCount()
{
	return maximal_cell_count;
}

void ParticleSystem::manageArray(data_buffer::AbstractArray* array)
{
	m_managed_arrays.push_back(array);
}

//void particle_system::build()
//{
	
//}



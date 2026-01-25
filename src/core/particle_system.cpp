#include "main.h"

#include "core/particle_system.h"

#include <numeric>
#include <algorithm>
#include <filesystem>
#include <CL/opencl.hpp>

#include "core/data_variable.h"

#include "utilities/load_file.h"

namespace fs = std::filesystem;

ParticleSystem::ParticleSystem(cl::CommandQueue & command_queue, bool use_opengl_context) : DataSystem(command_queue, use_opengl_context)
{
	// Paths:
	fs::path delete_particles_random = "share/cl_kernels/delete_particles_random.cl";
	fs::path concatenate             = "share/cl_kernels/sim_cell_division2.cl";
	fs::path delete_sort             = "share/cl_kernels/delete_particles.cl";

	// Load kernels:
	kernel_random_deletion = get_kernel_from_file("share/cl_kernels/delete_particles_random.cl", "membrane_simulate_particles");
	kernel_concatenate     = get_kernel_from_file("share/cl_kernels/sim_cell_division2.cl", "concatenate");
	kernel_delete_sort     = get_kernel_from_file("share/cl_kernels/delete_particles.cl", "sort_deleted_list");

	// Test count:
	particle_count = 32*32;	// TODO: remove
	particle_count = 1;
	//particle_count = 0;
	//maximal_cell_count = 32*32*32*32;
	//maximal_cell_count = 32*32;

	// Get standard functions for growing and shrinking arrays:
	std::string kernel_code_1 = load_file("share/cl_kernels/append_buffer.cl");
	std::string kernel_code_2 = load_file("share/cl_kernels/delete_particles.cl");
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

const cl::Program& ParticleSystem::getStandardParticleFunctions() {
	return m_standard_functions;
}

void ParticleSystem::setupBuffers() {
	list_of_particles_to_duplicate        = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);
	new_cell_group_size     = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count/workgroup_size);
	list_of_particles_to_delete    = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);
	deleted_cell_group_size = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count/workgroup_size);
	concatenated_list_of_particles_to_delete             = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);				
	concatenated_list_of_particles_to_duplicate            = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);
}

std::vector<event_info> ParticleSystem::customDeletionFunction(cl::Buffer& empty_cells, unsigned int& empty_count)
{
	EventLog log;

	for(auto const& value: m_managed_arrays)
	{
		log.add(value->performDeletion(m_command_queue, empty_cells, empty_count));
	}
	
	return std::move(log.get());
}

std::vector<event_info> ParticleSystem::customDuplicationFunction(cl::Buffer& empty_particles,
                                               	cl::Buffer& copied_particles,
                                               	unsigned int& copied_count,
												std::vector<cl::Event>& wait_for_events)
{
	EventLog log;
	for(auto const& value: m_managed_arrays)
	{
		log.add(value->performDuplication(m_command_queue, empty_particles, copied_particles, copied_count, wait_for_events));
	}
	
	return std::move(log.get());
}

std::vector<event_info> ParticleSystem::deleteMarkedParticles() {
	EventLog log;
	// ************************************************** //
	//  Gather deletion information and delete particles  //
	// ************************************************** //
	
	// Get the list of particle indices to be deleted:
	// This list is expected to be concatenated already by a previous kernel.
	unsigned int array_cell_deleted_group_size[maximal_cell_count/workgroup_size];

	cl::Event event_read_deleted_group_size;

	m_command_queue.finish();

	m_command_queue.enqueueReadBuffer(
		deleted_cell_group_size,
		CL_TRUE,
		0,
		maximal_cell_count/workgroup_size*sizeof(cl_uint),
		array_cell_deleted_group_size,
		nullptr,
		&event_read_deleted_group_size
	);
	m_command_queue.finish();
	log.add(event_info("READ: deleted cell group size", event_info::read_buffer, event_read_deleted_group_size));

	std::size_t group_count = (particle_count + workgroup_size - 1) / workgroup_size;  // ceiling division
	unsigned int empty_count = std::accumulate(
		array_cell_deleted_group_size,
		array_cell_deleted_group_size + group_count,
		0u
	);

	//message_debug("ARRAY_DELETED: " << array_cell_deleted_group_size[0] << array_cell_deleted_group_size[1] << array_cell_deleted_group_size[2] << array_cell_deleted_group_size[3]);

	if(empty_count != 0)
	{
		message_debug("empty_count = ", empty_count, " | particle count: ", particle_count);
		
		// Send signal to rearrange the variable list in the derived class:
		if(empty_count <= particle_count)
		{
			log.add(customDeletionFunction(concatenated_list_of_particles_to_delete, empty_count));
			particle_count -= empty_count;
		} else {
			message_error("Tried to delete more particles than currently present! This is a bug.");
		}
	}

	m_command_queue.finish();
	//TODO: REMOVE ANY AND ALL BLOCKING CALLS!!!!

	return std::move(log.get());
}

std::vector<event_info> ParticleSystem::duplicateMarkedParticles() {
	EventLog log;
	// **************************************************** //
	//  Gather duplication information and duplicate cells  //
	// **************************************************** //
	cl::Event event_read_group_size;

	const size_t array_length = maximal_cell_count/workgroup_size;

	cl_uint array_cell_group_size[array_length];

	m_command_queue.enqueueReadBuffer(
		new_cell_group_size,
		CL_TRUE,
		0,
		array_length*sizeof(cl_uint),
		array_cell_group_size,
		nullptr,
		&event_read_group_size
	);

	log.add(event_info("CELL_SYSTEM: read new_cell_group_size", event_info::read_buffer, event_read_group_size));

	m_command_queue.finish();

	size_t num_groups = (particle_count + workgroup_size - 1) / workgroup_size;
	num_groups = std::min(num_groups, static_cast<size_t>(maximal_cell_count / workgroup_size));

	copied_count = std::accumulate(
		array_cell_group_size,
		array_cell_group_size + num_groups,
		0u
	);

	if(copied_count != 0)
	{
		kernel_concatenate.setArg(0, concatenated_list_of_particles_to_duplicate);
		kernel_concatenate.setArg(1, list_of_particles_to_duplicate);
		kernel_concatenate.setArg(2, new_cell_group_size);

		cl::Event concatenation_finished;

		// Concatenate:
		m_command_queue.enqueueNDRangeKernel(
			kernel_concatenate,
			cl::NullRange,
			cl::NDRange(num_groups*workgroup_size),
			cl::NDRange(workgroup_size),
			nullptr,
			&concatenation_finished
		);

		log.add(event_info("CELL_SYSTEM: kernel_concatenate", event_info::kernel, concatenation_finished));

		std::vector<cl::Event> wait_for_events = {concatenation_finished};
		
		if(copied_count + particle_count < maximal_cell_count)
		{
			customDuplicationFunction(concatenated_list_of_particles_to_delete, concatenated_list_of_particles_to_duplicate, copied_count, wait_for_events);
			particle_count += copied_count;
		} else {
			message_error("Particle count has exceeded limits.");
		}
	}
	

	//message_debug("CURRENT PARTICLE COUNT: ", particle_count);

	
	return std::move(log.get());
}

std::vector<event_info> ParticleSystem::clearDuplicationDeletionBuffers() {
	EventLog log;
	// ********************************************** //
	//      Clear division and deletion buffers       //
	// ********************************************** //

	cl::Event event_fill_new_cell_group_size;
	cl::Event event_fill_concatenated_list_of_particles_to_duplicate;
	cl::Event event_fill_deleted_cell_group_size;
	cl::Event event_fill_concatenated_list_of_particles_to_delete;
	cl::Event event_fill_list_of_particles_to_delete;

	cl_int fill_pattern = 0;
	
	m_command_queue.enqueueFillBuffer(new_cell_group_size, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count/workgroup_size, nullptr, &event_fill_new_cell_group_size);
	m_command_queue.enqueueFillBuffer(concatenated_list_of_particles_to_duplicate, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count, nullptr, &event_fill_concatenated_list_of_particles_to_duplicate);
	m_command_queue.enqueueFillBuffer(deleted_cell_group_size, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count/workgroup_size, nullptr, &event_fill_deleted_cell_group_size);
	m_command_queue.enqueueFillBuffer(concatenated_list_of_particles_to_delete, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count, nullptr, &event_fill_concatenated_list_of_particles_to_delete);
	m_command_queue.enqueueFillBuffer(list_of_particles_to_delete, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count, nullptr, &event_fill_list_of_particles_to_delete);
	
	log.add(event_info("CELL_SYSTEM: fill new_cell_group_size", event_info::fill_buffer, event_fill_new_cell_group_size));
	log.add(event_info("CELL_SYSTEM: fill concatenated_list_of_particles_to_duplicate", event_info::fill_buffer, event_fill_concatenated_list_of_particles_to_duplicate));
	log.add(event_info("CELL_SYSTEM: fill event_fill_deleted_cell_group_size", event_info::fill_buffer, event_fill_deleted_cell_group_size));
	log.add(event_info("CELL_SYSTEM: fill event_fill_concatenated_list_of_particles_to_delete", event_info::fill_buffer, event_fill_concatenated_list_of_particles_to_delete));
	log.add(event_info("CELL_SYSTEM: fill event_fill_list_of_particles_to_delete", event_info::fill_buffer, event_fill_list_of_particles_to_delete));
	return std::move(log.get());
}

std::vector<event_info> ParticleSystem::finalizeDuplicationDeletion()
{
	EventLog log;
	//log.add(deleteMarkedParticles());
	log.add(duplicateMarkedParticles());
	log.add(clearDuplicationDeletionBuffers());
	return std::move(log.get());
}

unsigned int ParticleSystem::getMaximalParticleCount()
{
	return maximal_cell_count;
}

void ParticleSystem::manageArray(data_buffer::AbstractParticleData* array)
{
	m_managed_arrays.push_back(array);
}

//void particle_system::build()
//{
	
//}



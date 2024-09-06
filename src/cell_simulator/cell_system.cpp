#include "cell_simulator/cell_system.h"

#include <stdexcept>
#include <CL/cl2.hpp>
#include <memory>
#include <algorithm>
#include <cctype>


CellSystem::CellSystem(cl::Platform & platform,
                       cl::Device & device,
                       cl::Context & context,
                       cl::CommandQueue & command_queue) : 
	ParticleSystem(platform, device, context, command_queue),
	m_position_1(*this, m_standard_functions, [this](unsigned int) -> cl::Kernel { position_duplicator.setArg(3, (unsigned long)std::rand()); return position_duplicator; }),
	m_velocity_1(*this, m_standard_functions),
	m_radius_1(*this, m_standard_functions),
	m_neighbors(*this, m_standard_functions, [this](unsigned int) -> cl::Kernel { return neighbors_duplicator; }),
	kernel_physics(platform, device, context, command_queue),
	kernel_particle_marker(platform, device, context, command_queue,
	                       "if (generate_random_int(seed, (uint2)(0, 0)).x < 4294967) MARK_DUPLICATE;")
{

	position_duplicator = get_kernel_from_file("cl_kernels/append_buffer_cell_division_random_displacement.cl", "append_buffer");
	position_duplicator = get_kernel_from_file("cl_kernels/append_buffer_cell_division_random_displacement.cl", "append_buffer");
	

	cl_float4 particle_1 = {0.0f, 0.0f, 0.0f, 1.0f};
	
	m_position_1.append(m_command_queue, {particle_1});
	m_velocity_1.append(m_command_queue, {particle_1});
	m_radius_1.append(m_command_queue, {1.0f});
	m_command_queue.finish();
		
}

CellSystem::~CellSystem() { }

void CellSystem::build()
{
	// Create buffers:
	
	
	//kernel_physics.set_buffers();
	//m_position_1.
}

std::vector<event_info> CellSystem::run()
{
	std::vector<event_info> info1 = kernel_physics.run(m_position_1,
	                                                   m_velocity_1,
	                                                   m_velocity_1,
	                                                   m_radius_1,
	                                                   m_position_1,
	                                                   m_radius_1,
	                                                   particle_count);

	// Mark cells for duplication:
	std::vector<event_info> info2 = kernel_particle_marker.run(new_cell_indices,
	                                                           new_cell_group_size,
	                                                           copied_cells,
	                                                           particle_count);

	//std::vector<event_info> info3 = kernel_particle_marker.run(deleted_cell_indices,
	//                                                           deleted_cell_group_size,
	//                                                           empty_cells,
	//                                                           particle_count);

	
	
	std::vector<event_info> info = finalizeDuplicationDeletion();

	std::vector<event_info> all_events;

	for(unsigned int i=0; i<info1.size(); i++)
	{
		all_events.push_back(info1.at(i));
	}

	for(unsigned int i=0; i<info2.size(); i++)
	{
		all_events.push_back(info2.at(i));
	}

	for(unsigned int i=0; i<info.size(); i++)
	{
		all_events.push_back(info.at(i));
	}

	return all_events;
}

void CellSystem::initialize_render()
{
	test_renderer.initialize(m_position_1, m_command_queue);
}

void CellSystem::render(camera gl_camera)
{
	test_renderer.render(gl_camera, m_position_1, m_command_queue);
}



//void particle_system::build()
//{
	
//}



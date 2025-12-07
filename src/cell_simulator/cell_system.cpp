#include "cell_system.h"

#include <CL/opencl.hpp>
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

std::vector<event_info> CellSystem::calculatePhysicsStep() {
    return kernel_physics.run(
		m_position_1,
		m_velocity_1,
		m_velocity_1,
		m_radius_1,
		m_position_1,
		m_radius_1,
		particle_count
	);
}

std::vector<event_info> CellSystem::markParticlesForDivisionOrDeletion() {
	return kernel_particle_marker.run(
		new_cell_indices,
		new_cell_group_size,
		copied_cells,
		particle_count
	);
}

std::vector<event_info> CellSystem::run()
{
	EventLog log;
	log.add(calculatePhysicsStep());
	log.add(markParticlesForDivisionOrDeletion());
	log.add(finalizeDuplicationDeletion());
	return std::move(log.get());
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



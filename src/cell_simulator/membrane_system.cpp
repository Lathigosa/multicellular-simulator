#include "membrane_system.h"

#include <CL/opencl.hpp>

MembraneSystem::MembraneSystem(cl::CommandQueue & command_queue)
	: ParticleSystem(command_queue, false)
	, kernel_physics(command_queue)
	, kernel_particle_marker(command_queue,
	                       "uint2 random_variable = generate_random_int(seed, (uint2)(0, 0));"
						   "if (random_variable.x < (0xFFFFFFFF / 4000)) MARK_DUPLICATE;"
						   "if (random_variable.y < (0xFFFFFFFF / 4000)) MARK_DELETE;"
						)
	, m_position_1(*this, [this](unsigned int) -> cl::Kernel { position_duplicator.setArg(4, (unsigned long)std::rand()); return position_duplicator; })
	, m_velocity_1(*this)
	, m_radius_1(*this)
	, m_neighbors(*this, [this](unsigned int) -> cl::Kernel { return neighbors_duplicator; })
	, m_membrane_vertex_positions(*this)
	, m_membrane_vertex_normal(*this)
	, m_membrane_edges(*this)
	, m_membrane_faces(*this)
{
	position_duplicator = get_kernel_from_file("share/cl_kernels/append_buffer_cell_division_random_displacement.cl", "split_particle");

	cl_float4 particle_1 = {{0.0f, 0.0f, 0.1f, 1.0f}};
	
	m_position_1.append(m_command_queue, {particle_1});
	m_velocity_1.append(m_command_queue, {particle_1});
	m_radius_1.append(m_command_queue, {1.0f});
	m_command_queue.finish();
}

MembraneSystem::~MembraneSystem() { }

void MembraneSystem::build()
{
	// Create buffers:
	
	//kernel_physics.set_buffers();
	//m_position_1.
}

std::vector<event_info> MembraneSystem::calculatePhysicsStep() {
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

std::vector<event_info> MembraneSystem::markParticlesForDivisionOrDeletion() {
	return kernel_particle_marker.run(
		list_of_particles_to_duplicate,
		new_cell_group_size,
		concatenated_list_of_particles_to_duplicate,
		particle_count
	);
}

std::vector<event_info> MembraneSystem::run()
{
	EventLog log;
	// Pseudocode for now:
	log.add(calculatePhysicsStep()); // Move each vertex according to a physics simulation.
	// log.add(flipTriangles()); // Flip edges in quads so that the triangles are never too long.
	log.add(markParticlesForDivisionOrDeletion());
	log.add(finalizeDuplicationDeletion());
	return std::move(log.get());
}

void MembraneSystem::initialize_render()
{
	test_renderer.initialize(m_position_1, m_command_queue);
}

void MembraneSystem::render(camera gl_camera)
{
	test_renderer.render(gl_camera, m_position_1, m_command_queue);
}



//void particle_system::build()
//{
	
//}



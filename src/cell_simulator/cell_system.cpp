#include "cell_system.h"

#include <CL/opencl.hpp>

CellSystem::CellSystem(cl::CommandQueue & command_queue, bool use_opengl_context)
	: ParticleSystem(command_queue, use_opengl_context)
	, kernel_physics(command_queue)
	, kernel_particle_marker(command_queue,
	                       "uint2 random_variable = generate_random_int(seed, (uint2)(0, 0));"
						   "if (random_variable.x < (0xFFFFFFFF / 4000)) MARK_DUPLICATE;"
						   "if (random_variable.y < (0xFFFFFFFF / 4000)) MARK_DELETE;"
						)
	, m_position_1(*this, [this](unsigned int) -> cl::Kernel {
		position_duplicator.setArg(4, (unsigned long)std::rand());
		//message_debug("Running splitter.");
		return position_duplicator;
	})
	, m_velocity_1(*this)
	, m_radius_1(*this)
	, m_neighbors(*this)
	, m_membrane(*this)
	, m_centrosome_pair(*this)
	//m_membrane_vertex_positions(*this, m_standard_functions),
	
{
	position_duplicator = get_kernel_from_file("share/cl_kernels/append_buffer_cell_division_random_displacement.cl", "split_particle");

	cl_float4 particle_1 = {{0.0f, 0.0f, 0.1f, 1.0f}};

	cl_float8 centrosomes_1 = {{
		-0.001f, 0.2f, 0.001f, 0.1f,
		0.003f, -0.2f, -0.004f, 0.1f
	}};

	m_position_1.append(m_command_queue, {particle_1});
	m_velocity_1.append(m_command_queue, {particle_1});
	m_radius_1.append(m_command_queue, {1.0f});
	m_membrane.addIcosphereParticle(m_command_queue);
	m_centrosome_pair.centrosome_pair_position.append(m_command_queue, {centrosomes_1});
	m_command_queue.finish();

	message_debug("Constructed particle system. Each particle takes up ", calculate_memory_footprint_per_particle(), " bytes of memory.");
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
		list_of_particles_to_duplicate,
		new_cell_group_size,
		concatenated_list_of_particles_to_duplicate,
		particle_count
	);
}

std::vector<event_info> CellSystem::run()
{
	EventLog log;
	log.add(calculatePhysicsStep());
	log.add(markParticlesForDivisionOrDeletion());
	log.add(finalizeDuplicationDeletion());
	log.add(m_membrane.run_physics_step(m_command_queue));
	log.add(m_centrosome_pair.run_physics_step(m_command_queue, m_membrane));
	return std::move(log.get());
}

void CellSystem::initialize_render()
{
	//test_renderer.initialize(m_position_1, m_command_queue);
	centrosome_renderer.initialize(m_position_1, m_centrosome_pair, m_command_queue);
	membrane_renderer.initialize(m_position_1, m_membrane, m_command_queue);
}

void CellSystem::render(camera gl_camera)
{
	//test_renderer.render(gl_camera, m_position_1, m_command_queue);
	centrosome_renderer.render(gl_camera, m_position_1, m_centrosome_pair, m_command_queue);
	membrane_renderer.render(gl_camera, m_position_1, m_membrane, m_command_queue);
}



//void particle_system::build()
//{
	
//}



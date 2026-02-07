#include "cell_simulator/centrosome_pair.hpp"

CentrosomePair::CentrosomePair(ParticleSystem& parent_system)
    : centrosome_pair_position(parent_system)
{
    calculate_centrosome_force = parent_system.get_kernel_from_file("share/cl_kernels/centrosome_physics.cl", "calculate_physics_step");
}

std::vector<event_info> CentrosomePair::run_physics_step(
    cl::CommandQueue& queue,
    const ParticleMembraneData& membrane
) {
    cl::Event event_calculate_forces;

	//TODO: remove:
	float step_size = 0.1f;

    calculate_centrosome_force.setArg(0, membrane.edges_index_buffer.getFrontBuffer());
    calculate_centrosome_force.setArg(1, membrane.vertex_opposite_to_triangle_edge_buffer.getFrontBuffer());
    calculate_centrosome_force.setArg(2, membrane.vertex_edges_buffer.getFrontBuffer());
    calculate_centrosome_force.setArg(3, membrane.vertex_opposite_edges_buffer.getFrontBuffer());
    calculate_centrosome_force.setArg(4, membrane.vertex_buffer.getFrontBuffer());
    calculate_centrosome_force.setArg(5, centrosome_pair_position.getFrontBuffer());
    calculate_centrosome_force.setArg(6, centrosome_pair_position.getBackBuffer());
    calculate_centrosome_force.setArg(7, (unsigned int)membrane.max_vertices_per_cell);

    queue.enqueueNDRangeKernel(
        calculate_centrosome_force, 
        cl::NullRange,
		cl::NDRange(centrosome_pair_position.used_count(), 1),
        cl::NullRange,
        nullptr,
        &event_calculate_forces
    );

    centrosome_pair_position.swapBuffers();

	EventLog log;
	log.add(event_info("CENTROSOME PAIR: calculate force", event_info::kernel, event_calculate_forces));
	return std::move(log.get());
}
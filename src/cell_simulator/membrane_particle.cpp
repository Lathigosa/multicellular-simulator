#include "cell_simulator/membrane_particle.hpp"

#include "cl.h"
#include "model/icosphere.hpp"

ParticleMembraneData::ParticleMembraneData(ParticleSystem& parent_system)
    : vertex_buffer(parent_system, [this](unsigned int) -> cl::Kernel {
		position_duplicator.setArg(4, constant_buffer_icosphere);
        position_duplicator.setArg(5, (unsigned long)std::rand());
		//message_debug("Running splitter.");
		return position_duplicator;
	})
    , vertex_force_vector_buffer(parent_system)
    , edges_index_buffer(parent_system)
    , triangles_index_buffer(parent_system)
    , vertex_opposite_to_triangle_edge_buffer(parent_system)
    , vertex_edges_buffer(parent_system)
    , vertex_opposite_edges_buffer(parent_system)
{
    position_duplicator = parent_system.get_kernel_from_file("share/cl_kernels/split_particle_membrane.cl", "split_particle_membrane");
    calculate_curvature_force = parent_system.get_kernel_from_file("share/cl_kernels/membrane_physics.cl", "membrane_curvature_force");
    integrate_forces = parent_system.get_kernel_from_file("share/cl_kernels/membrane_physics.cl", "membrane_integrate_forces");

    setupConstantBuffers(parent_system.getCommandQueue(), parent_system.getContext());
}

void ParticleMembraneData::setupConstantBuffers(cl::CommandQueue& queue, cl::Context& context)
{
    constant_buffer_icosphere = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(cl_float4)*max_vertices_per_cell);
    
    
    Icosphere icosphere = Icosphere(2);
    
    const auto& vertices = icosphere.vertices();
    message_debug("Amount of vertices in a sphere:", vertices.size());
    cl::array<cl_float4, max_vertices_per_cell> particle_vertices{};
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const auto& v = vertices[i];
        constexpr float radius = 0.2f;
        particle_vertices[i] = cl_float4{{v.x, v.y, v.z, radius}};
    }

    queue.enqueueWriteBuffer(constant_buffer_icosphere, CL_TRUE, 0, sizeof(cl_float4)*max_vertices_per_cell, &particle_vertices);

}

void ParticleMembraneData::addIcosphereParticle(cl::CommandQueue& queue)
{
    // TODO: redesign architecture such that all of this boilerplate is not necessary in so many lines.
    // Create icosphere:
    Icosphere icosphere = Icosphere(2);

    // Upload vertices to GPU:
    const auto& vertices = icosphere.vertices();
    message_debug("Amount of vertices in a sphere:", vertices.size());
    cl::array<cl_float4, max_vertices_per_cell> particle_vertices{};
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const auto& v = vertices[i];
        constexpr float radius = 0.2f;
        particle_vertices[i] = cl_float4{{v.x, v.y, v.z, radius}};
    }
    std::vector<cl::array<cl_float4, max_vertices_per_cell>> particles_vertices;
    particles_vertices.push_back(particle_vertices);
    vertex_buffer.append(queue, particles_vertices);
    
    // Upload edges to GPU:
    const auto& edges = icosphere.edges();
    cl::array<cl_uint2, max_edges_per_cell> particle_edges{};
    for (std::size_t i = 0; i < edges.size(); ++i) {
        const auto& e = edges[i];
        particle_edges[i] = cl_uint2{{e.x, e.y}};
    }
    std::vector<cl::array<cl_uint2, max_edges_per_cell>> particles_edges;
    particles_edges.push_back(particle_edges);
    edges_index_buffer.append(queue, particles_edges);

    // Upload edge neighbors to GPU:
    const auto& edge_neighbors = icosphere.vertex_neighbors_of_edge();
    cl::array<cl_uint2, max_edges_per_cell> particle_edge_neighbors{};
    for (std::size_t i = 0; i < edge_neighbors.size(); ++i) {
        const auto& e = edge_neighbors[i];
        particle_edge_neighbors[i] = cl_uint2{{e.x, e.y}};
    }
    std::vector<cl::array<cl_uint2, max_edges_per_cell>> particles_edge_neighbors;
    particles_edge_neighbors.push_back(particle_edge_neighbors);
    vertex_opposite_to_triangle_edge_buffer.append(queue, particles_edge_neighbors);

    // Upload triangles to GPU:
    const auto& triangles = icosphere.faces();
    cl::array<Triangle, max_triangles_per_cell> particle_triangles{};
    for (std::size_t i = 0; i < triangles.size(); ++i) {
        const auto& t = triangles[i];
        particle_triangles[i] = {t.x, t.y, t.z};
    }
    std::vector<cl::array<Triangle, max_triangles_per_cell>> particles_triangles;
    particles_triangles.push_back(particle_triangles);
    triangles_index_buffer.append(queue, particles_triangles);

    // Upload vertex edges to GPU:
    const auto& vertex_edges = icosphere.vertex_edges();
    cl::array<cl_uint8, max_vertices_per_cell> particle_vertex_edges{};
    for (std::size_t i = 0; i < vertex_edges.size(); ++i) {
        const auto& t = vertex_edges[i];
        particle_vertex_edges[i] = {{t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7]}};
    }
    std::vector<cl::array<cl_uint8, max_vertices_per_cell>> particles_vertex_edges;
    particles_vertex_edges.push_back(particle_vertex_edges);
    vertex_edges_buffer.append(queue, particles_vertex_edges);

    // Upload vertex opposite edges to GPU:
    const auto& vertex_opposite_edges = icosphere.vertex_opposite_edges();
    cl::array<cl_uint8, max_vertices_per_cell> particle_vertex_opposite_edges{};
    for (std::size_t i = 0; i < vertex_opposite_edges.size(); ++i) {
        const auto& t = vertex_opposite_edges[i];
        particle_vertex_opposite_edges[i] = {{t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7]}};
    }
    std::vector<cl::array<cl_uint8, max_vertices_per_cell>> particles_vertex_opposite_edges;
    particles_vertex_opposite_edges.push_back(particle_vertex_opposite_edges);
    vertex_opposite_edges_buffer.append(queue, particles_vertex_opposite_edges);

    // Initialize forces to zero:
    cl::array<cl_float4, max_vertices_per_cell> zeroes({});
    for (auto& entry : zeroes) {
        entry = cl_float4{{0.0f, 0.0f, 0.0f, 0.0f}};
    }
    std::vector<cl::array<cl_float4, max_vertices_per_cell>> zeroes_vertex_force;
    zeroes_vertex_force.push_back(zeroes);
    vertex_force_vector_buffer.append(queue, zeroes_vertex_force);
}



std::vector<event_info> ParticleMembraneData::run_physics_step(cl::CommandQueue& queue)
{
	cl::Event event_calculate_curvature_force;
	cl::Event event_integrate_forces;

	//TODO: remove:
	float step_size = 0.1f;

    calculate_curvature_force.setArg(0, edges_index_buffer.getFrontBuffer());
    calculate_curvature_force.setArg(1, vertex_opposite_to_triangle_edge_buffer.getFrontBuffer());
    calculate_curvature_force.setArg(2, vertex_edges_buffer.getFrontBuffer());
    calculate_curvature_force.setArg(3, vertex_opposite_edges_buffer.getFrontBuffer());
    calculate_curvature_force.setArg(4, vertex_buffer.getFrontBuffer());
    calculate_curvature_force.setArg(5, vertex_force_vector_buffer.getBackBuffer());
    calculate_curvature_force.setArg(6, (unsigned int)max_vertices_per_cell);

    queue.enqueueNDRangeKernel(
        calculate_curvature_force, 
        cl::NullRange,
		cl::NDRange(max_edges_per_cell, edges_index_buffer.used_count()),
        cl::NDRange(max_edges_per_cell, 1),
        nullptr,
        &event_calculate_curvature_force
    );

    std::vector<cl::Event> dependencies_for_integrate_forces = { 
		event_calculate_curvature_force
	};

    vertex_force_vector_buffer.swapBuffers();

    integrate_forces.setArg(0, vertex_buffer.getFrontBuffer());
    integrate_forces.setArg(1, vertex_force_vector_buffer.getFrontBuffer());
    integrate_forces.setArg(2, vertex_buffer.getBackBuffer());
    integrate_forces.setArg(3, vertex_buffer.used_count());
    integrate_forces.setArg(4, step_size);

    queue.enqueueNDRangeKernel(
        integrate_forces,
        cl::NullRange,
		cl::NDRange(vertex_buffer.used_count() * max_vertices_per_cell),
        cl::NullRange,
        &dependencies_for_integrate_forces,
        &event_integrate_forces
    );

	// Switch the buffers:
	vertex_buffer.swapBuffers();

    std::vector<cl::Event> dependencies_for_wait_list = { 
		event_integrate_forces
	};

    queue.enqueueBarrierWithWaitList(&dependencies_for_wait_list);

	EventLog log;
	log.add(event_info("MEMBRANE SYSTEM: calculate curvature force", event_info::kernel, event_calculate_curvature_force));
	log.add(event_info("MEMBRANE SYSTEM: integrate forces", event_info::kernel, event_integrate_forces));
	return std::move(log.get());
}

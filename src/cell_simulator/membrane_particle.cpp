#include "cell_simulator/membrane_particle.hpp"

#include "model/icosphere.hpp"

ParticleMembraneData::ParticleMembraneData(ParticleSystem& parent_system)
    : vertex_buffer(parent_system, [this](unsigned int) -> cl::Kernel {
		position_duplicator.setArg(4, (unsigned long)std::rand());
		//message_debug("Running splitter.");
		return position_duplicator;
	})
    , edges_index_buffer(parent_system)
    , triangles_index_buffer(parent_system)
{
    position_duplicator = parent_system.get_kernel_from_file("share/cl_kernels/split_particle_membrane.cl", "split_particle_membrane");
}

void ParticleMembraneData::addIcosphereParticle(cl::CommandQueue& queue)
{
    // TODO: redesign architecture such that all of this boilerplate is not necessary in so many lines.
    // Create icosphere:
    Icosphere icosphere = Icosphere(2);

    // Upload vertices to GPU:
    const auto& vertices = icosphere.vertices();
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
}

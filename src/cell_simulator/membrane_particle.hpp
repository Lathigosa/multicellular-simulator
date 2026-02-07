#ifndef MEMBRANE_PARTICLE_HPP_INCLUDED
#define MEMBRANE_PARTICLE_HPP_INCLUDED

#include "core/data_variable.h"

#include <CL/opencl.hpp>

#include <glm/glm.hpp>

struct Triangle {
    unsigned int i0;
    unsigned int i1;
    unsigned int i2;
};

class ParticleMembraneData
{
public:
    static constexpr size_t max_vertices_per_cell = 256;
    static constexpr size_t max_edges_per_cell = 3*max_vertices_per_cell;
    static constexpr size_t max_triangles_per_cell = 2*max_vertices_per_cell;

    ParticleMembraneData(ParticleSystem& parent_system);

    void addIcosphereParticle(cl::CommandQueue& queue);

    data_buffer::ParticleData<cl::array<cl_float4, max_vertices_per_cell>> vertex_buffer;
    data_buffer::ParticleData<cl::array<cl_float4, max_vertices_per_cell>> vertex_force_vector_buffer;
    data_buffer::ParticleData<cl::array<cl_uint2, max_edges_per_cell>> edges_index_buffer;
    data_buffer::ParticleData<cl::array<Triangle, max_triangles_per_cell>> triangles_index_buffer;
    data_buffer::ParticleData<cl::array<cl_uint2, max_edges_per_cell>> vertex_opposite_to_triangle_edge_buffer;
    data_buffer::ParticleData<cl::array<cl_uint8, max_vertices_per_cell>> vertex_edges_buffer;
    data_buffer::ParticleData<cl::array<cl_uint8, max_vertices_per_cell>> vertex_opposite_edges_buffer;

    std::vector<event_info> run_physics_step(cl::CommandQueue& queue);

private:
    void setupConstantBuffers(cl::CommandQueue& queue, cl::Context& context);

    cl::Buffer constant_buffer_icosphere;

    cl::Kernel position_duplicator;

    cl::Kernel calculate_curvature_force;
    cl::Kernel integrate_forces;
};

#endif // MEMBRANE_PARTICLE_HPP_INCLUDED
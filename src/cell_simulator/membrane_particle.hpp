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
    data_buffer::ParticleData<cl::array<cl_uint2, max_edges_per_cell>> edges_index_buffer;
    data_buffer::ParticleData<cl::array<Triangle, max_triangles_per_cell>> triangles_index_buffer;

private:
    cl::Kernel position_duplicator;
};

#endif // MEMBRANE_PARTICLE_HPP_INCLUDED
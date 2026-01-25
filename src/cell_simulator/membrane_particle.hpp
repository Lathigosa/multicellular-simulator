#ifndef MEMBRANE_PARTICLE_HPP_INCLUDED
#define MEMBRANE_PARTICLE_HPP_INCLUDED

#include "core/data_variable.h"

#include <CL/opencl.hpp>

class ParticleMembraneData : public data_buffer::ParticleData<cl::array<cl_float4, 256>>
{
public:
    static constexpr size_t max_vertices_per_cell = 256;

    ParticleMembraneData(ParticleSystem& parent_system);

    void addIcosphereParticle(cl::CommandQueue& queue);

private:
    cl::Kernel position_duplicator;
};

#endif // MEMBRANE_PARTICLE_HPP_INCLUDED
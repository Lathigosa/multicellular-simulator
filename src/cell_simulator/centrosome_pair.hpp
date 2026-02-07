#pragma once

#include "core/data_variable.h"

#include "cell_simulator/membrane_particle.hpp"

#include <CL/opencl.hpp>

class CentrosomePair
{
public:
    CentrosomePair(ParticleSystem& parent_system);

    data_buffer::ParticleData<cl_float8> centrosome_pair_position;

    std::vector<event_info> run_physics_step(cl::CommandQueue& queue, const ParticleMembraneData& membrane);

private:
    void setupConstantBuffers(cl::CommandQueue& queue);

    cl::Kernel position_duplicator;

    cl::Kernel calculate_centrosome_force;
};
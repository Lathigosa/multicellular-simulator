#include "cell_simulator/membrane_particle.hpp"

#include "model/icosphere.hpp"

ParticleMembraneData::ParticleMembraneData(ParticleSystem& parent_system)
    : data_buffer::ParticleData<cl::array<cl_float4, 256>>(parent_system)
{
    
}

void ParticleMembraneData::addIcosphereParticle(cl::CommandQueue& queue)
{
    Icosphere icosphere = Icosphere(2);

    const auto& vertices = icosphere.vertices();

    cl::array<cl_float4, 256> particle_vertices{};

    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const auto& v = vertices[i];
        constexpr float radius = 0.2f;
        particle_vertices[i] = cl_float4{{v.x, v.y, v.z, radius}};
    }

    std::vector<cl::array<cl_float4, 256>> particles;
    particles.push_back(particle_vertices);

    append(queue, particles);
}

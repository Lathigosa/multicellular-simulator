#ifndef PARTICLE_DELETION_RANDOM_H
#define PARTICLE_DELETION_RANDOM_H

#include "main.h"

#include "core/data_kernel.h"
//#include "particle_template.h"

#include <CL/cl2.hpp>

#include <vector>
#include <string>

#include "core/data_variable.h"

class ParticleMarker : public data_kernel
{
public:
	ParticleMarker(cl::Platform & platform,
	               cl::Device & device,
	               cl::Context & context,
	               cl::CommandQueue & command_queue,
	               const std::string marker_code);
	
	virtual ~ParticleMarker();


	void build(const std::string marker_code);

	std::vector<event_info> run(const cl::Buffer& marked_particle_indices,
                                const cl::Buffer& marked_particle_group_size,
                                const cl::Buffer& copied_particles,
                                unsigned int particle_count);
private:

	cl::Kernel kernel_random_marker;
	cl::Kernel kernel_concatenate;

	size_t wg_size = 32;
};

#endif // PARTICLE_DELETION_RANDOM_H

#ifndef CELL_PARTICLE_PHYSICS_H
#define CELL_PARTICLE_PHYSICS_H

#include "main.h"

#include "core/data_kernel.h"
//#include "particle_template.h"

#include <CL/cl2.hpp>

#include <vector>
#include <string>

#include "core/data_variable.h"

class cell_particle_physics : public data_kernel
{
public:
	cell_particle_physics(cl::Platform & platform,
	                      cl::Device & device,
	                      cl::Context & context,
	                      cl::CommandQueue & command_queue);
	
	virtual ~cell_particle_physics();


	void build();

	std::vector<event_info> run(data_buffer::Array<cl_float4>& position_1,
	                            data_buffer::Array<cl_float4>& velocity_1,
	                            data_buffer::Array<cl_float4>& acceleration_1,
	                            data_buffer::Array<cl_float>& radius_1,
	                            const data_buffer::Array<cl_float4>& position_2,
	                            const data_buffer::Array<cl_float>& radius_2,
	                            unsigned int particle_count);
private:

	cl::Kernel kernel_membrane_physics;
	cl::Kernel kernel_append_buffer;
	cl::Kernel kernel_sort_particles;

	size_t wg_size = 32;

	cl::Buffer grid_with_particles;				// Buffer containing particle indices per voxel.
	cl::Buffer grid_with_particles_count;		// Buffer containing amount of particles per voxel.

	unsigned int grid_size_x = 64;
	unsigned int grid_size_y = 64;
	unsigned int grid_size_z = 64;
};

#endif // CELL_PARTICLE_PHYSICS_H

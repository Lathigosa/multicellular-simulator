#ifndef CELL_PARTICLE_PHYSICS_H
#define CELL_PARTICLE_PHYSICS_H

#include "main.h"

#include "core/data_kernel.h"
//#include "particle_template.h"

#include <CL/opencl.hpp>

#include <glm/glm.hpp>

#include <vector>
#include <string>

#include "core/data_variable.h"

/*
class ForceField {
public:
	virtual ~ForceField();
	virtual std::vector<cl_float4> getRenderableGeometry();

	virtual const std::string getOpenCLCode();
};

class PlanarForceField : public ForceField {
public:
	std::vector<cl_float4> getRenderableGeometry() override;

	const std::string getOpenCLCode() override;

private:
	static constexpr std::string_view kernel_code;
	std::vector<cl_float4> geometry;
};

class SphericalForceField : public ForceField {
	
};*/

class cell_particle_physics : public data_kernel {
public:
	cell_particle_physics(cl::CommandQueue & command_queue);
	
	virtual ~cell_particle_physics();


	void build();

	std::vector<event_info> run(data_buffer::ParticleData<cl_float4>& position_1,
	                            data_buffer::ParticleData<cl_float4>& velocity_1,
	                            data_buffer::ParticleData<cl_float4>& acceleration_1,
	                            data_buffer::ParticleData<cl_float>& radius_1,
	                            const data_buffer::ParticleData<cl_float4>& position_2,
	                            const data_buffer::ParticleData<cl_float>& radius_2,
	                            unsigned int particle_count);

	void render_grid();
private:

	cl::Kernel kernel_membrane_physics;
	cl::Kernel kernel_append_buffer;
	cl::Kernel kernel_sort_particles;

	size_t wg_size = 32;

	cl::Buffer grid_with_particles;				// Buffer containing particle indices per voxel.
	cl::Buffer grid_with_particles_count;		// Buffer containing amount of particles per voxel.

	glm::vec<3, unsigned int> grid_size = {64, 64, 64};
	glm::vec<3, float> voxel_size = {10.0f, 10.0f, 10.0f};

	unsigned int max_cells_per_voxel = 256;

	//std::vector<std::unique_ptr<ForceField>> force_fields;
};

#endif // CELL_PARTICLE_PHYSICS_H

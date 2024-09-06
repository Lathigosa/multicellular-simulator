#include "membrane.h"

using namespace simulation;

#include <iostream>
#include "resources/opencl_kernels.h"
#include <cmath>

#include <CL/cl_gl.h>

#include "utilities/load_file.h"

#include "lua/membrane_lib.h"

#include <cstdlib>

membrane::membrane(	cl::Platform & platform_cl,
					cl::Device & device_cl,
					cl::Context & context,
					cl::CommandQueue & command_queue) : particle_template(platform_cl, device_cl, context, command_queue)
{
	std::string kernel_code = load_file("cl_kernels/cell_hookian_repel.cl");
	std::string kernel_code3 = load_file("cl_kernels/sort_particles_in_3d_grid.cl");

	sources_membrane_physics.push_back({kernel_code3.c_str(), kernel_code3.length()});
	sources_membrane_physics.push_back({kernel_code.c_str(), kernel_code.length()});
						
	program_membrane_physics = cl::Program(opencl_context, sources_membrane_physics);
	cl_int error = program_membrane_physics.build({device});
	if(error != CL_SUCCESS)
	{
		message_error("Error building: " << program_membrane_physics.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << " error code" << error);
		exit(1);
	}

	kernel_membrane_physics = cl::Kernel(program_membrane_physics, "cell_hookian_repel");
	
	kernel_append_buffer = get_kernel_from_file("cl_kernels/append_buffer_cell_division.cl", "append_buffer");
	kernel_sort_particles = get_kernel_from_file("cl_kernels/sort_particles_in_3d_grid.cl", "spatial_particle_sort_3d");
						
	kernel_sort_particles.getWorkGroupInfo(device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, &wg_size);
}

membrane::~membrane()
{
	//dtor
}

error membrane::initialize_buffers()
{
	max_buffer_size = dependency_pointers[0]->get_custom_uint("max_particle_count");

	buffer_position = create_particle_double_buffer("position", sizeof(cl_float4));
	buffer_velocity = create_particle_double_buffer("velocity", sizeof(cl_float4));

	grid_with_particles = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*16*grid_size_x*grid_size_y*grid_size_z);
	grid_with_particles_count = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*grid_size_x*grid_size_y*grid_size_z);

	// TEST: load initial values:
	float array_in[4*particle_count];

	array_in[0] = 0.0f;
	array_in[1] = 0.0f;
	array_in[2] = 0.0f;
	array_in[3] = 1.0f;

	cl_float4 fill_pattern;
	fill_pattern.s[0] = 0.0f;
	fill_pattern.s[1] = 0.0f;
	fill_pattern.s[2] = 0.0f;
	fill_pattern.s[3] = 0.0f;

	cl_uint fill_pattern_uint = 0;

	queue.enqueueWriteBuffer(get_buffer(buffer_position, 1), CL_TRUE, 0, sizeof(cl_float4)*particle_count, array_in);
	queue.enqueueWriteBuffer(get_buffer(buffer_position, 0), CL_TRUE, 0, sizeof(cl_float4)*particle_count, array_in);
	queue.enqueueFillBuffer(get_buffer(buffer_position, 1), fill_pattern, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*(max_buffer_size - particle_count), nullptr);
	queue.enqueueFillBuffer(get_buffer(buffer_position, 0), fill_pattern, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*(max_buffer_size - particle_count), nullptr);
	queue.enqueueFillBuffer(get_buffer(buffer_velocity, 1), fill_pattern, 0, sizeof(cl_float4)*max_buffer_size, nullptr);
	queue.enqueueFillBuffer(get_buffer(buffer_velocity, 0), fill_pattern, 0, sizeof(cl_float4)*max_buffer_size, nullptr);

	queue.enqueueFillBuffer(grid_with_particles, fill_pattern_uint, 0, sizeof(cl_uint)*16*grid_size_x*grid_size_y*grid_size_z, nullptr);
	queue.enqueueFillBuffer(grid_with_particles_count, fill_pattern_uint, 0, sizeof(cl_uint)*grid_size_x*grid_size_y*grid_size_z, nullptr);

	return error::success;
}

#ifndef NO_UI
error membrane::initialize_buffers(std::vector<GLuint> from_gl_buffer)
{
	// TODO: include cl-gl interoperability.
	message_debug("UNIMPLEMENTED FUNCTION.");
	assert(false);

	return error::success;
}
#endif // NO_UI

unsigned int membrane::get_buffer_size(unsigned int index, bool double_buffer) const
{
	if(index <= 1)
		return particle_count * sizeof(cl_float4);

	return 0;
}

error membrane::organize_unit()
{
	return error::undefined_function;
}

std::vector<event_info> membrane::simulate_unit(float step_size)
{
	cl::Event event_sort_particles;
	cl::Event event_physics;

	// Clear voxel data before adding to it to prevent filling it with previous data:
	cl_uint fill_pattern_uint_full = 0; //0xFFFFFFFF;
	queue.enqueueFillBuffer(grid_with_particles, fill_pattern_uint_full, 0, sizeof(cl_uint)*16*grid_size_x*grid_size_y*grid_size_z, nullptr, nullptr);
	cl_uint fill_pattern_uint_zero = 0;
	queue.enqueueFillBuffer(grid_with_particles_count, fill_pattern_uint_zero, 0, sizeof(cl_uint)*grid_size_x*grid_size_y*grid_size_z, nullptr, nullptr);

	// Sort particles into voxels:
	kernel_sort_particles.setArg(0, grid_with_particles);
	kernel_sort_particles.setArg(1, grid_with_particles_count);
	kernel_sort_particles.setArg(2, get_buffer(buffer_position, !buffer_index));
	kernel_sort_particles.setArg(3, particle_count);
	queue.enqueueNDRangeKernel(kernel_sort_particles, cl::NullRange, cl::NDRange((unsigned int)(wg_size*ceil(double(particle_count)/double(wg_size)))), cl::NullRange, nullptr, &event_sort_particles);
	
	// Simulate physics:
	kernel_membrane_physics.setArg(0, get_buffer(buffer_position, buffer_index));
	kernel_membrane_physics.setArg(1, get_buffer(buffer_velocity, buffer_index));
	kernel_membrane_physics.setArg(2, get_buffer(buffer_position, !buffer_index));
	kernel_membrane_physics.setArg(3, get_buffer(buffer_velocity, !buffer_index));
	kernel_membrane_physics.setArg(4, grid_with_particles);
	kernel_membrane_physics.setArg(5, grid_with_particles_count);
	kernel_membrane_physics.setArg(6, (unsigned int)particle_count);
	kernel_membrane_physics.setArg(7, step_size);
	queue.enqueueNDRangeKernel(kernel_membrane_physics, cl::NullRange, cl::NDRange((unsigned int)(wg_size*ceil(double(particle_count)/double(wg_size)))), cl::NullRange, nullptr, &event_physics);

	// TODO: enqueuebarrier!!!

	
	// Switch the buffers:
	buffer_index = !buffer_index;

	return {{event_sort_particles, "sort particles"}, {event_physics, "physics"}};
}



error membrane::signal_cell_reindex(const std::string sim_unit_name,
											cl::Buffer empty_cells,
		                              		cl::Buffer copied_cells,
		                              		unsigned int copied_count)
{
	//message_debug("Got signal" << copied_count << " " << particle_count);
	//return error::success;
	
	if(particle_count + copied_count > max_buffer_size)
		return error::buffer_overflow;
	
	kernel_append_buffer.setArg(0, get_buffer(buffer_position, !buffer_index));
	kernel_append_buffer.setArg(1, copied_cells);
	kernel_append_buffer.setArg(2, particle_count);
	
	// TODO: make something prettier for getting a buffer from a dependency:
	// Getting the division plane buffer:
	kernel_append_buffer.setArg(3, dependency_pointers[1]->get_buffer(0, 0));   // Division plane
	queue.enqueueNDRangeKernel(kernel_append_buffer, cl::NullRange, cl::NDRange(copied_count), cl::NullRange);

	kernel_append_buffer.setArg(0, get_buffer(buffer_velocity, !buffer_index));
	kernel_append_buffer.setArg(1, copied_cells);
	kernel_append_buffer.setArg(2, particle_count);
	kernel_append_buffer.setArg(3, 0.0f);
	queue.enqueueNDRangeKernel(kernel_append_buffer, cl::NullRange, cl::NDRange(copied_count), cl::NullRange);

	particle_count += copied_count;

	//message_debug("Copied" << copied_count);
	
	return error::success;
}

error membrane::spawn_cells(unsigned int count, cl_float4* positions, cl_float4* velocities)
{
	//message_debug("Got signal" << copied_count << " " << particle_count);
	
	if(particle_count + count > max_buffer_size)
		return error::buffer_overflow;

	if (positions == nullptr)
		return error::invalid_argument;

	cl_float4 fill_pattern;
	fill_pattern.s[0] = 0.0f;
	fill_pattern.s[1] = 0.0f;
	fill_pattern.s[2] = 0.0f;
	fill_pattern.s[3] = 5.0f;

	queue.enqueueWriteBuffer(get_buffer(buffer_position, !buffer_index), CL_TRUE, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*count, positions);
	if (velocities == nullptr)
	{
		queue.enqueueFillBuffer(get_buffer(buffer_velocity, !buffer_index), fill_pattern, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*count, nullptr);
	} else
	{
		queue.enqueueWriteBuffer(get_buffer(buffer_velocity, !buffer_index), CL_TRUE, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*count, velocities);
	}

	particle_count += count;
	
	return error::success;
}

const void membrane::expose_lua_library(lua_State* L) const
{
	// Include the library to the table at the top of the stack:
	luaL_register(L, nullptr, lua::sim_membrane::functions);
}

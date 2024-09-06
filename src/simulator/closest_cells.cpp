#include "closest_cells.h"

using namespace simulation;

#include <iostream>
#include "resources/opencl_kernels.h"
#include <cmath>

#include <CL/cl_gl.h>

#include "utilities/load_file.h"

//#include "lua/membrane_lib.h"

#include <cstdlib>

find_closest_cells::find_closest_cells(	cl::Platform & platform_cl,
					cl::Device & device_cl,
					cl::Context & context,
					cl::CommandQueue & command_queue) : sim_unit_template(platform_cl, device_cl, context, command_queue)
{
	std::string kernel_code = load_file("cl_kernels/find_16_closest_points.cl");
						
	sources_membrane_physics.push_back({kernel_code.c_str(), kernel_code.length()});

	program_membrane_physics = cl::Program(opencl_context, sources_membrane_physics);
	cl_int error = program_membrane_physics.build({device});
	if(error != CL_SUCCESS)
	{
		message_error("Error building: " << program_membrane_physics.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << " error code" << error);
		exit(1);
	}

	kernel_membrane_physics = cl::Kernel(program_membrane_physics, "cell_organize_closest");

	kernel_membrane_physics.getWorkGroupInfo(device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, &wg_size);

	// Test count:
	particle_count = 1 * 1; 	// TODO: remove
	max_buffer_size = particle_count * 32*32*16;
}

find_closest_cells::~find_closest_cells()
{
	//dtor
}

error find_closest_cells::initialize_buffers()
{
	position[0] = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_float4)*max_buffer_size);
	position[1] = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_float4)*max_buffer_size);
	velocity[0] = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_float4)*max_buffer_size);
	velocity[1] = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_float4)*max_buffer_size);

	// TEST: load initial values:
	float array_in[4*particle_count];
	//for(int a=0; a<32; a++)
	//{
	//	for(int b=0; b<32; b++)
	//	{
	//		array_in[b * 4 * 32 + a* 4 + 0] = a * 9.0f;
	//		array_in[b * 4 * 32 + a* 4 + 1] = b * 9.0f;
	//		array_in[b * 4 * 32 + a* 4 + 2] = 0.0f;
	//		array_in[b * 4 * 32 + a* 4 + 3] = 0.0f;
	//	}
	//}
	//array_in[2] = 1.0f;

	array_in[0] = 0.0f;
	array_in[1] = 0.0f;
	array_in[2] = 0.0f;
	array_in[3] = 0.0f;

	cl_float4 fill_pattern;
	fill_pattern.s[0] = 0.0f;
	fill_pattern.s[1] = 0.0f;
	fill_pattern.s[2] = 0.0f;
	fill_pattern.s[3] = 0.0f;

	queue.enqueueWriteBuffer(position[1], CL_TRUE, 0, sizeof(cl_float4)*particle_count, array_in);
	queue.enqueueWriteBuffer(position[0], CL_TRUE, 0, sizeof(cl_float4)*particle_count, array_in);
	queue.enqueueFillBuffer(position[1], fill_pattern, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*(max_buffer_size - particle_count), nullptr);
	queue.enqueueFillBuffer(position[0], fill_pattern, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*(max_buffer_size - particle_count), nullptr);
	queue.enqueueFillBuffer(velocity[1], fill_pattern, 0, sizeof(cl_float4)*max_buffer_size, nullptr);
	queue.enqueueFillBuffer(velocity[0], fill_pattern, 0, sizeof(cl_float4)*max_buffer_size, nullptr);

	return error::success;
}

#ifndef NO_UI
error find_closest_cells::initialize_buffers(std::vector<GLuint> from_gl_buffer)
{
	message_debug("Creating shared buffers of <membrane>.");

	// TODO: if cl_khr_gl_sharing isn't supported, disable this function!

	if(from_gl_buffer.size() == 1)
	{
		// TEST:
		message_debug("Linked buffer <position> of <membrane> to OpenGL display.");

		position[0] = cl::BufferGL(opencl_context, CL_MEM_READ_WRITE, from_gl_buffer[0]);
		position[1] = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_float4)*max_buffer_size);
		velocity[0] = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_float4)*max_buffer_size);
		velocity[1] = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_float4)*max_buffer_size);

	}

	/*
	if(from_gl_buffer.size() >= 2)
	{
		message_debug("Linked buffer <position> of <membrane> to OpenGL display.");
		position[0] = cl::BufferGL(context, CL_MEM_READ_WRITE, from_gl_buffer[0]);
		position[1] = cl::BufferGL(context, CL_MEM_READ_WRITE, from_gl_buffer[1]);
	} // else
	//	return error::general_error;

	if(from_gl_buffer.size() >= 4)
	{
		message_debug("Linked buffer <velocity> of <membrane> to OpenGL display.");
		velocity[0] = cl::BufferGL(context, CL_MEM_READ_WRITE, from_gl_buffer[2]);
		velocity[1] = cl::BufferGL(context, CL_MEM_READ_WRITE, from_gl_buffer[3]);
	}*/

	return error::success;
}
#endif // NO_UI

cl::Buffer find_closest_cells::get_buffer(unsigned int index, bool double_buffer)
{
	if(index == 0)
		return position[0 + double_buffer];
	
	if(index == 1)
		return velocity[0 + double_buffer];

	// TODO: Else, error!
}

unsigned int find_closest_cells::get_buffer_size(unsigned int index, bool double_buffer) const
{
	if(index <= 1)
		return particle_count * sizeof(cl_float4);

	return 0;
}

unsigned int find_closest_cells::get_buffer_index(const std::string name) const
{
	if(name == "position")
		return 0;
	
	if(name == "velocity")
		return 1;

	// TODO: Else, error!
	
	return 0;
}

error find_closest_cells::organize_unit()
{
	return error::undefined_function;
}

std::vector<event_info> find_closest_cells::simulate_unit(float step_size)
{
	// Get work group info to determine the proper kernel load size:
	//size_t wg_size;
	
	//wg_size = 1024;
	cl::Event event;
	
	kernel_membrane_physics.setArg(0, position[buffer_index]);
	kernel_membrane_physics.setArg(1, velocity[buffer_index]);
	kernel_membrane_physics.setArg(2, position[!buffer_index]);
	kernel_membrane_physics.setArg(3, velocity[!buffer_index]);
	kernel_membrane_physics.setArg(4, (unsigned int)particle_count);
	//message_debug("particle count ceil = " << wg_size*ceil(double(particle_count)/double(wg_size)) << ", wg_size = " << wg_size);
	kernel_membrane_physics.setArg(5, 0.1f);
	queue.enqueueNDRangeKernel(kernel_membrane_physics, cl::NullRange, cl::NDRange((unsigned int)(wg_size*ceil(double(particle_count)/double(wg_size)))), cl::NullRange, nullptr, &event);
	//queue.finish();

	// TODO: enqueuebarrier!!!

	//message_debug("Simulate Unit");
	
	// Switch the buffers:
	buffer_index = !buffer_index;

	return {{event, "closest_cell_sorting"}};
}

error find_closest_cells::signal_cell_reindex(const std::string sim_unit_name,
											cl::Buffer empty_cells,
		                              		cl::Buffer copied_cells,
		                              		unsigned int copied_count)
{
	//message_debug("Got signal" << copied_count << " " << particle_count);
	//return error::success;
	
	if(particle_count + copied_count > max_buffer_size)
		return error::buffer_overflow;
	
	kernel_append_buffer.setArg(0, position[!buffer_index]);
	kernel_append_buffer.setArg(1, copied_cells);
	kernel_append_buffer.setArg(2, particle_count);
	kernel_append_buffer.setArg(3, (float) (rand()) / (float) (RAND_MAX));
	queue.enqueueNDRangeKernel(kernel_append_buffer, cl::NullRange, cl::NDRange(copied_count), cl::NullRange);

	kernel_append_buffer.setArg(0, velocity[!buffer_index]);
	kernel_append_buffer.setArg(1, copied_cells);
	kernel_append_buffer.setArg(2, particle_count);
	kernel_append_buffer.setArg(3, 0.0f);
	queue.enqueueNDRangeKernel(kernel_append_buffer, cl::NullRange, cl::NDRange(copied_count), cl::NullRange);

	particle_count += copied_count;

	//message_debug("Copied" << copied_count);
	
	return error::success;
}

error find_closest_cells::spawn_cells(unsigned int count, cl_float4* positions, cl_float4* velocities)
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
	fill_pattern.s[3] = 0.0f;

	queue.enqueueWriteBuffer(position[!buffer_index], CL_TRUE, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*count, positions);
	if (velocities == nullptr)
	{
		queue.enqueueFillBuffer(velocity[!buffer_index], fill_pattern, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*count, nullptr);
	} else
	{
		queue.enqueueWriteBuffer(velocity[!buffer_index], CL_TRUE, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*count, velocities);
	}

	particle_count += count;
	
	return error::success;
}

const void find_closest_cells::expose_lua_library(lua_State* L) const
{
	// Include the library to the lua state:
	//luaL_register(L, "sim_membrane", lua::sim_membrane::functions);
}

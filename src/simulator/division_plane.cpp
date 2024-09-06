#include "division_plane.h"

using namespace simulation;

#include <iostream>
#include "resources/opencl_kernels.h"
#include <cmath>

#include <CL/cl_gl.h>

#include "utilities/load_file.h"

//#include "simulator/intracellular_reactions_lib/intracellular_reactions_lib.h"

#include <cstdlib>
#include <cctype>
#include <map>

#include <clocale>

division_plane::division_plane(	cl::Platform & platform_cl,
					cl::Device & device_cl,
					cl::Context & context,
					cl::CommandQueue & command_queue) : particle_template(platform_cl, device_cl, context, command_queue)
{
	// TODO: remove:
	kernel_append_buffer = get_kernel_from_file("cl_kernels/append_buffer.cl", "append_buffer");
}

division_plane::~division_plane()
{
	//dtor
}

error division_plane::initialize_buffers()
{
	max_buffer_size = dependency_pointers[0]->get_custom_uint("max_particle_count");
	buffer_division_plane = create_particle_double_buffer("division_plane", sizeof(cl_float4));

	// TEST: load initial values:
	unsigned int particle_count = 1;
	float array_in[4*particle_count];

	array_in[0] = 0.5f;
	array_in[1] = 0.01f;
	array_in[2] = 0.5f;
	array_in[3] = 0.0f;

	cl_float4 fill_pattern;
	fill_pattern.s[0] = 0.0f;
	fill_pattern.s[1] = 0.0f;
	fill_pattern.s[2] = 0.0f;
	fill_pattern.s[3] = 0.0f;

	queue.enqueueWriteBuffer(get_buffer(buffer_division_plane, 1), CL_TRUE, 0, sizeof(cl_float4)*particle_count, array_in);
	queue.enqueueWriteBuffer(get_buffer(buffer_division_plane, 0), CL_TRUE, 0, sizeof(cl_float4)*particle_count, array_in);
	queue.enqueueFillBuffer(get_buffer(buffer_division_plane, 1), fill_pattern, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*(max_buffer_size - particle_count), nullptr);
	queue.enqueueFillBuffer(get_buffer(buffer_division_plane, 0), fill_pattern, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*(max_buffer_size - particle_count), nullptr);

	return error::success;
}

#ifndef NO_UI
error division_plane::initialize_buffers(std::vector<GLuint> from_gl_buffer)
{
	message_debug("Error: UNIMPLEMENTED FUNCTION!!!");
	assert(false);
	// TODO: if cl_khr_gl_sharing isn't supported, disable this function!

	return error::success;
}
#endif // NO_UI


std::vector<event_info> division_plane::simulate_unit(float step_size)
{
	std::vector<event_info> event_list = {};
	//buffer_index = 1;

	return event_list;
}

//TODO: remove:
error division_plane::signal_cell_reindex(const std::string sim_unit_name,
											cl::Buffer empty_cells,
		                              		cl::Buffer copied_cells,
		                              		unsigned int copied_count)
{
	// Copy all particle data:
	if(particle_count + copied_count > max_buffer_size)
		return error::buffer_overflow;

	

	kernel_append_buffer.setArg(0, get_buffer(buffer_division_plane, !buffer_index));
	kernel_append_buffer.setArg(1, copied_cells);
	kernel_append_buffer.setArg(2, particle_count);
	//kernel_append_buffer.setArg(3, (float) (rand()) / (float) (RAND_MAX));
	queue.enqueueNDRangeKernel(kernel_append_buffer, cl::NullRange, cl::NDRange(copied_count), cl::NullRange);

	kernel_append_buffer.setArg(0, get_buffer(buffer_division_plane, buffer_index));
	kernel_append_buffer.setArg(1, copied_cells);
	kernel_append_buffer.setArg(2, particle_count);
	//kernel_append_buffer.setArg(3, (float) (rand()) / (float) (RAND_MAX));
	queue.enqueueNDRangeKernel(kernel_append_buffer, cl::NullRange, cl::NDRange(copied_count), cl::NullRange);
	

	particle_count += copied_count;
	
	return error::success;
}

const void division_plane::expose_lua_library(lua_State* L) const
{
	// Include the library to the lua state:
	//luaL_register(L, nullptr, lua::sim_cell_polarity::functions);
}

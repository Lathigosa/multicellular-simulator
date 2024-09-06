#include "cell_polarity.h"

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

cell_polarity::cell_polarity(	cl::Platform & platform_cl,
					cl::Device & device_cl,
					cl::Context & context,
					cl::CommandQueue & command_queue) : particle_template(platform_cl, device_cl, context, command_queue)
{
	// TODO: remove next line, the kernel gets constructed from something else instead:
	kernel_membrane_physics = get_kernel_from_file("cl_kernels/intracellular_reactions.cl", "simulate_reactions");
	kernel_membrane_physics.getWorkGroupInfo(device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, &wg_size);
	kernel_append_buffer = get_kernel_from_file("cl_kernels/append_buffer_cell_polarity.cl", "append_buffer");

	// Test count:
	//particle_count = 1*1; 	// TODO: remove
	//max_buffer_size = particle_count * 32*32*16;
	molecular_species_count = 4;
}

cell_polarity::~cell_polarity()
{
	//dtor
}

error cell_polarity::initialize_buffers()
{
	max_buffer_size = dependency_pointers[0]->get_custom_uint("max_particle_count");
	//TODO: resize buffer based on molecular species count!!!
	buffer_concentrations = create_particle_double_buffer("concentrations", sizeof(cl_float4));

	// TEST: load initial values:
	unsigned int particle_count = 1;
	float array_in[4*particle_count];

	array_in[0] = 0.0f;
	array_in[1] = 0.0f;
	array_in[2] = 1.0f;
	array_in[3] = 0.0f;

	cl_float4 fill_pattern;
	fill_pattern.s[0] = 0.0f;
	fill_pattern.s[1] = 0.0f;
	fill_pattern.s[2] = 0.0f;
	fill_pattern.s[3] = 0.0f;

	queue.enqueueWriteBuffer(get_buffer(buffer_concentrations, 1), CL_TRUE, 0, sizeof(cl_float4)*particle_count, array_in);
	queue.enqueueWriteBuffer(get_buffer(buffer_concentrations, 0), CL_TRUE, 0, sizeof(cl_float4)*particle_count, array_in);
	queue.enqueueFillBuffer(get_buffer(buffer_concentrations, 1), fill_pattern, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*(max_buffer_size - particle_count), nullptr);
	queue.enqueueFillBuffer(get_buffer(buffer_concentrations, 0), fill_pattern, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*(max_buffer_size - particle_count), nullptr);

	return error::success;
}

#ifndef NO_UI
error cell_polarity::initialize_buffers(std::vector<GLuint> from_gl_buffer)
{
	message_debug("Error: UNIMPLEMENTED FUNCTION!!!");
	assert(false);
	// TODO: if cl_khr_gl_sharing isn't supported, disable this function!

	return error::success;
}
#endif // NO_UI

error cell_polarity::organize_unit()
{
	return error::undefined_function;
}

std::vector<event_info> cell_polarity::simulate_unit(float step_size)
{
	std::vector<event_info> event_list = {};

	/*// Perform simulation 8 times:
	for(int i=0; i<8; i++)
	{
		cl::Event event;
		
		kernel_membrane_physics.setArg(0, get_buffer(buffer_concentrations, buffer_index));
		kernel_membrane_physics.setArg(1, get_buffer(buffer_concentrations, !buffer_index));
		kernel_membrane_physics.setArg(2, (unsigned int)particle_count);
		kernel_membrane_physics.setArg(3, step_size*0.0001f / 8.0f);
		queue.enqueueNDRangeKernel(kernel_membrane_physics, cl::NullRange, cl::NDRange((unsigned int)(wg_size*ceil(double(particle_count)/double(wg_size)))), cl::NullRange, nullptr, &event);

		event_list.push_back({event, "iteration"});

		buffer_index = !buffer_index;
	}*/

	// TODO: enqueuebarrier!!!

	return event_list;
}

error cell_polarity::signal_cell_reindex(const std::string sim_unit_name,
											cl::Buffer empty_cells,
		                              		cl::Buffer copied_cells,
		                              		unsigned int copied_count)
{
	if(particle_count + copied_count > max_buffer_size)
		return error::buffer_overflow;
	
	kernel_append_buffer.setArg(0, get_buffer(buffer_concentrations, !buffer_index));
	kernel_append_buffer.setArg(1, get_buffer(buffer_concentrations, !buffer_index));
	kernel_append_buffer.setArg(2, copied_cells);
	kernel_append_buffer.setArg(3, particle_count);
	
	// TODO: make something prettier for getting a buffer from a dependency:
	// Getting the division plane buffer:
	kernel_append_buffer.setArg(4, dependency_pointers[1]->get_buffer(0, 0));   // Division plane
	queue.enqueueNDRangeKernel(kernel_append_buffer, cl::NullRange, cl::NDRange(copied_count), cl::NullRange);

	particle_count += copied_count;
	
	return error::success;
}

const void cell_polarity::expose_lua_library(lua_State* L) const
{
	// Include the library to the lua state:
	//luaL_register(L, nullptr, lua::sim_cell_polarity::functions);
}

#include "particle_template.h"

using namespace simulation;

#include <iostream>
#include "resources/opencl_kernels.h"
#include <cmath>

#include <CL/cl_gl.h>

#include "utilities/load_file.h"

#include "lua/membrane_lib.h"

#include <cstdlib>

particle_template::particle_template(	cl::Platform & platform_cl,
					cl::Device & device_cl,
					cl::Context & context,
					cl::CommandQueue & command_queue) : sim_unit_template(platform_cl, device_cl, context, command_queue)
{
	kernel_append_buffer = get_kernel_from_file("cl_kernels/append_buffer.cl", "append_buffer");
	delete_particles_float4 = get_kernel_from_file("cl_kernels/delete_particles.cl", "delete_particles_float4");
						
						
	// Test count:
	particle_count = 1 * 1; 	// TODO: remove
	max_buffer_size = particle_count * 32*32*16;
}

particle_template::~particle_template()
{
	//dtor
}

void particle_template::init()
{
	max_buffer_size = dependency_pointers[0]->get_custom_uint("max_particle_count");
	initialize_buffers();
}

error particle_template::initialize_buffers()
{
	max_buffer_size = dependency_pointers[0]->get_custom_uint("max_particle_count");
	return error::success;
}

#ifndef NO_UI
error particle_template::initialize_buffers(std::vector<GLuint> from_gl_buffer)
{
	
}
#endif // NO_UI

unsigned int particle_template::get_buffer_size(unsigned int index, bool double_buffer) const
{
	if(index < particle_buffer_indices.size())
		return particle_count * get_buffer_entry_size(index);

	return 0;
}


error particle_template::organize_unit()
{
	return error::undefined_function;
}

std::vector<event_info> particle_template::simulate_unit(float step_size)
{
	
}

error particle_template::signal_cell_reindex(const std::string sim_unit_name,
											cl::Buffer empty_cells,
		                              		cl::Buffer copied_cells,
		                              		unsigned int copied_count)
{
	// Copy all particle data:
	if(particle_count + copied_count > max_buffer_size)
		return error::buffer_overflow;

	

	for(unsigned int i=0; i<particle_buffer_indices.size(); i++)
	{
		// TODO: support not only float4!
		assert(get_buffer_entry_size(i) == sizeof(cl_float4));
		
		kernel_append_buffer.setArg(0, get_buffer(particle_buffer_indices.at(i), !buffer_index));
		kernel_append_buffer.setArg(1, copied_cells);
		kernel_append_buffer.setArg(2, particle_count);
		//kernel_append_buffer.setArg(3, (float) (rand()) / (float) (RAND_MAX));
		queue.enqueueNDRangeKernel(kernel_append_buffer, cl::NullRange, cl::NDRange(copied_count), cl::NullRange);
	}

	particle_count += copied_count;
	
	return error::success;
}

error particle_template::signal_particle_remove(const std::string sim_unit_name,
		                                 		cl::Buffer empty_cells,
		                                 		unsigned int empty_count)
{
	// Copy all particle data:
	if(particle_count < empty_count)
		return error::buffer_overflow;

	for(unsigned int i=0; i<particle_buffer_indices.size(); i++)
	{
		// TODO: support not only float4!
		assert(get_buffer_entry_size(i) == sizeof(cl_float4));

		// TODO: why isn't this double buffered?
		delete_particles_float4.setArg(0, get_buffer(particle_buffer_indices.at(i), !buffer_index));
		delete_particles_float4.setArg(1, get_buffer(particle_buffer_indices.at(i), !buffer_index));
		delete_particles_float4.setArg(2, empty_cells);
		delete_particles_float4.setArg(3, particle_count);
		delete_particles_float4.setArg(4, empty_count);
		queue.enqueueNDRangeKernel(delete_particles_float4, cl::NullRange, cl::NDRange(empty_count), cl::NullRange);
	}

	particle_count -= empty_count;
	
	return error::success;
}

unsigned int particle_template::create_particle_double_buffer(std::string name, unsigned int entry_size)
{
	unsigned int new_buffer_index = create_double_buffer(name, entry_size, particle_count, max_buffer_size);
	particle_buffer_indices.push_back(new_buffer_index);
	return new_buffer_index;
}

const void particle_template::expose_lua_library(lua_State* L) const
{
	// Include the library to the table at the top of the stack:
	luaL_register(L, nullptr, lua::sim_membrane::functions);
}

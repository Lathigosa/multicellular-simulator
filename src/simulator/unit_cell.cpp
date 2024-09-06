#include "unit_cell.h"

using namespace simulation;

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "resources/opencl_kernels.h"
#include <cmath>

#include "utilities/load_file.h"

#include "simulator/cell_lib/cell_lib.h"

#include <CL/cl_gl.h>

#include <cstdlib>

cell_global::cell_global(	cl::Platform & platform_cl,
							cl::Device & device_cl,
							cl::Context & context,
							cl::CommandQueue & command_queue) : sim_unit_template(platform_cl, device_cl, context, command_queue)
{
	// Load kernels:
	kernel_random_deletion = get_kernel_from_file("cl_kernels/delete_particles_random.cl", "membrane_simulate_particles");
	kernel_concatenate = get_kernel_from_file("cl_kernels/sim_cell_division2.cl", "concatenate");
	kernel_delete_sort = get_kernel_from_file("cl_kernels/delete_particles.cl", "sort_deleted_list");

	// Test count:
	cell_count = 32*32;	// TODO: remove
	cell_count = 1;
	maximal_cell_count = 32*32*32*32;
	set_custom_data("max_particle_count", (unsigned int)maximal_cell_count);
}

cell_global::~cell_global()
{
	//dtor
}

error cell_global::initialize_buffers()
{
	// Particle duplication:
	new_cell_indices = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);
	new_cell_group_size = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count/workgroup_size);
	deleted_cell_indices = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);
	deleted_cell_group_size = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count/workgroup_size);
	empty_cells = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);				
	copied_cells = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);	

	//TODO: determine proper deletion list size:
	// Particle deletion:
	deleted_cells = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*256);
	deleted_cells_sorted = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*256);

	
	cl_int fill_pattern = 0;
	
	// ===== TEST: ===== //
	// Load initial values:
	
	cl_uint array_in[256];

	array_in[0] = 16-7;
	array_in[1] = 16-11;
	array_in[2] = 16-13;
	array_in[3] = 16-2;
	array_in[4] = 16-6;
	array_in[5] = 16-3;

	queue.enqueueFillBuffer(deleted_cells_sorted, fill_pattern, 0, sizeof(cl_uint)*256, nullptr);
	queue.enqueueWriteBuffer(deleted_cells, CL_TRUE, 0, sizeof(cl_uint)*256, array_in);
	//queue.enqueueWriteBuffer(deleted_cells_sorted, CL_TRUE, 0, sizeof(cl_uint)*256, array_in);

	// ================= //

	queue.enqueueFillBuffer(deleted_cell_group_size, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count/workgroup_size, nullptr);
	queue.enqueueFillBuffer(empty_cells, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count, nullptr);
	
	queue.enqueueFillBuffer(new_cell_group_size, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count/workgroup_size, nullptr);
	queue.enqueueFillBuffer(copied_cells, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count, nullptr);
	return error::success;
}

#ifndef NO_UI
error cell_global::initialize_buffers(std::vector<GLuint> from_gl_buffer)
{
	

	return error::success;
}
#endif // NO_UI

error cell_global::organize_unit()
{

}

std::vector<event_info> cell_global::simulate_unit(float step_size)
{
	// ********************************************** //
	//  Gather deletion information and delete cells  //
	// ********************************************** //
	unsigned int array_cell_deleted_group_size[maximal_cell_count/workgroup_size*sizeof(cl_uint)];

	queue.enqueueReadBuffer(deleted_cell_group_size, CL_TRUE, 0, maximal_cell_count/workgroup_size*sizeof(cl_uint), array_cell_deleted_group_size, nullptr);
	queue.finish();

	unsigned int empty_count = 0;
	for(int i=0; i<ceil(double(cell_count)/256.0); i++)
	{
		empty_count += array_cell_deleted_group_size[i];
	}

	// ********************************************** //
	//        	     Send deletion signal             //
	// ********************************************** //

	signal_particle_remove(get_name(), empty_cells, empty_count);
	for (int i=0; i<dependency_child_pointers.size(); i++)
	{
		dependency_child_pointers[i]->signal_particle_remove(get_name(), empty_cells, empty_count);
	}

	cl::Event event_read_group_size;
	cl::Event event_fill_new_cell_group_size;
	cl::Event event_fill_copied_cells;
	
	queue.finish();
	//TODO: REMOVE ANY AND ALL BLOCKING CALLS!!!!

	// **************************************************** //
	//  Gather duplication information and duplicate cells  //
	// **************************************************** //

	unsigned int array_cell_group_size[maximal_cell_count/workgroup_size*sizeof(cl_uint)];
	//queue.enqueueReadBuffer(new_cell_group_size, CL_TRUE, 0, ceil(double(cell_count)/256.0)*sizeof(cl_uint), array_cell_group_size, nullptr, &event_read_group_size);
	queue.enqueueReadBuffer(new_cell_group_size, CL_TRUE, 0, maximal_cell_count/workgroup_size*sizeof(cl_uint), array_cell_group_size, nullptr, &event_read_group_size);
	queue.finish();

	copied_count = 0;
	for(int i=0; i<ceil(double(cell_count)/256.0); i++)
	{
		copied_count += array_cell_group_size[i];
		
	}
	
	// ********************************************** //
	//              Send division signal              //
	// ********************************************** //
	signal_cell_reindex(get_name(), empty_cells, copied_cells, copied_count);
	for (int i=0; i<dependency_child_pointers.size(); i++)
    {
		dependency_child_pointers[i]->signal_cell_reindex(get_name(), empty_cells, copied_cells, copied_count);
    }

	// ********************************************** //
	//      Clear division and deletion buffers       //
	// ********************************************** //

	cl_int fill_pattern = 0;
	
	queue.enqueueFillBuffer(new_cell_group_size, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count/workgroup_size, nullptr, &event_fill_new_cell_group_size);
	queue.enqueueFillBuffer(copied_cells, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count, nullptr, &event_fill_copied_cells);
	queue.enqueueFillBuffer(deleted_cell_group_size, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count/workgroup_size, nullptr);
	queue.enqueueFillBuffer(empty_cells, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count, nullptr);
	queue.enqueueFillBuffer(deleted_cell_indices, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count, nullptr);
	

	// Switch the buffers:
	buffer_index = !buffer_index;
	return {{event_read_group_size, "read group size"},
		{event_fill_new_cell_group_size, "fill new cell group size"},
		{event_fill_copied_cells, "fill copied cells"}};
}

error cell_global::signal_cell_reindex(const std::string sim_unit_name,
											cl::Buffer empty_cells,
		                              		cl::Buffer copied_cells,
		                              		unsigned int copied_count)
{
	if(cell_count + copied_count > maximal_cell_count)
		return error::buffer_overflow;

	cell_count += copied_count;
	
	return error::success;
}

error cell_global::signal_particle_remove(const std::string sim_unit_name,
		                                 		cl::Buffer empty_cells,
		                                 		unsigned int empty_count)
{
	// Copy all particle data:
	if(cell_count < empty_count)
		return error::buffer_overflow;

	cell_count -= empty_count;
	
	return error::success;
}

// Custom Functions:


error cell_global::add_particles(unsigned int count)
{
	// Safety check against buffer overflow:
	if( cell_count + count > maximal_cell_count )
        return error::buffer_overflow;

	// Append cells to buffer:
	//queue.enqueueWriteBuffer(position[1], CL_TRUE, 0, sizeof(cl_float4)*positions.size(), positions.data());

	// Indicate the addition of new cells to all child modules:
	buffer_rearrange_info info;
	info.type = info.insertion;
	info.range_start = cell_count;
	info.range_end = cell_count + count - 1;

	cell_count += count;

    for (int i=0; i<dependency_child_pointers.size(); i++)
    {
		dependency_child_pointers[i]->signal_buffer_rearrange(get_name(), 0, info);
    }

	return error::success;
}

const void cell_global::expose_lua_library(lua_State* L) const
{
	// Include the library to the lua state:
	luaL_register(L, nullptr, lua::sim_cell::functions);
}


#include "division_random.h"

using namespace simulation;

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "resources/opencl_kernels.h"
#include <cmath>

#include "utilities/load_file.h"

#include <CL/cl_gl.h>

#include <cstdlib>

division_random::division_random(	cl::Platform & platform_cl,
							cl::Device & device_cl,
							cl::Context & context,
							cl::CommandQueue & command_queue) : sim_unit_template(platform_cl, device_cl, context, command_queue)
{
	kernel_random_division = get_kernel_from_file("cl_kernels/sim_cell_division.cl", "membrane_simulate_particles");

	// "Concatenate" kernel:
	kernel_concatenate = get_kernel_from_file("cl_kernels/sim_cell_division2.cl", "concatenate");

	// Test count:
	cell_count = 32*32;	// TODO: remove
	cell_count = 32*32;
}

division_random::~division_random()
{
	//dtor
}

error division_random::initialize_buffers()
{
	maximal_cell_count = dependency_pointers[0]->get_custom_uint("max_particle_count");
	new_cell_indices = dependency_pointers[0]->get_buffer(0, 0);
	new_cell_group_size = dependency_pointers[0]->get_buffer(1, 0);
	empty_cells = dependency_pointers[0]->get_buffer(2, 0);
	copied_cells = dependency_pointers[0]->get_buffer(3, 0);
	//new_cell_indices = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);
	//new_cell_group_size = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*32);
	//empty_cells = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);				
	//copied_cells = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*maximal_cell_count);			

	
	cl_int fill_pattern = 0;
	
	queue.enqueueFillBuffer(new_cell_group_size, fill_pattern, 0, sizeof(cl_int)*32, nullptr);
	queue.enqueueFillBuffer(copied_cells, fill_pattern, 0, sizeof(cl_int)*maximal_cell_count, nullptr);
	return error::success;
}

#ifndef NO_UI
error division_random::initialize_buffers(std::vector<GLuint> from_gl_buffer)
{
	/*message_debug("Creating shared buffers of <membrane>.");

	// TODO: if cl_khr_gl_sharing isn't supported, disable this function!

	if(from_gl_buffer.size() == 1)
	{
		// TEST:
		message_debug("Linked buffer <position> of <membrane> to OpenGL display.");

		position[0] = cl::BufferGL(opencl_context, CL_MEM_READ_WRITE, from_gl_buffer[0]);
		position[1] = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_float4)*particle_count);
		velocity[0] = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_float4)*particle_count);
		velocity[1] = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, sizeof(cl_float4)*particle_count);

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

error division_random::organize_unit()
{

}

std::vector<event_info> division_random::simulate_unit(float step_size)
{
	cl::Event event_random_division;
	cl::Event event_concatenate;
	// ********************************************** //
	//      TEST: Simulate random cell divisions      //
	// ********************************************** //
	kernel_random_division.setArg(0, new_cell_indices);
	kernel_random_division.setArg(1, new_cell_group_size);
	kernel_random_division.setArg(2, cell_count);
	kernel_random_division.setArg(3, std::rand());
	
	queue.enqueueNDRangeKernel(kernel_random_division, cl::NullRange, cl::NDRange(cell_count), cl::NDRange(256), nullptr, &event_random_division);   // TODO: determine proper local size!
	//queue.finish();

	//unsigned int array_test_out_p[32*sizeof(cl_uint)];
	//queue.enqueueReadBuffer(new_cell_group_size, CL_TRUE, 0, 32*sizeof(cl_uint), array_test_out_p);

	//message_debug("START:");
	//for(int i=0; i<32; i++)
	//{
		//message_debug(array_test_out_p[i]);
	//}

	// ********************************************** //
	// 				Concatenate the data		      //
	// ********************************************** //
	kernel_concatenate.setArg(0, copied_cells);
	kernel_concatenate.setArg(1, new_cell_indices);
	kernel_concatenate.setArg(2, new_cell_group_size);
	
	queue.enqueueNDRangeKernel(kernel_concatenate, cl::NullRange, cl::NDRange(ceil(double(cell_count)/256.0)), cl::NullRange, nullptr, &event_concatenate);

	//queue.finish();
	//TODO: REMOVE ANY AND ALL BLOCKING CALLS!!!!

	/*unsigned int array_cell_group_size[32*sizeof(cl_uint)];
	queue.enqueueReadBuffer(new_cell_group_size, CL_TRUE, 0, ceil(double(cell_count)/256.0)*sizeof(cl_uint), array_cell_group_size);

	queue.finish();

	copied_count = 0;
	for(int i=0; i<ceil(double(cell_count)/256.0); i++)
	{
		copied_count += array_cell_group_size[i];
	}
	
	// ********************************************** //
	//     Perform the divisions by copying data      //
	// ********************************************** //
	for (int i=0; i<dependency_child_pointers.size(); i++)
    {
		dependency_child_pointers[i]->signal_cell_reindex(get_name(), empty_cells, copied_cells, copied_count);
    }*/
	

	// Switch the buffers:
	buffer_index = !buffer_index;
	return {{event_random_division, "random division"}, {event_concatenate, "concatenate"}};
}

error division_random::signal_cell_reindex(const std::string sim_unit_name,
											cl::Buffer empty_cells,
		                              		cl::Buffer copied_cells,
		                              		unsigned int copied_count)
{
	if(cell_count + copied_count > maximal_cell_count)
		return error::buffer_overflow;

	//cell_count += copied_count;
	
	return error::success;
}

// Custom Functions:


error division_random::add_cells(std::vector<cl_float4> positions, std::string type)
{
	// Safety check against buffer overflow:
	if( cell_count + positions.size() > maximal_cell_count )
        return error::buffer_overflow;

	// Append cells to buffer:
	//queue.enqueueWriteBuffer(position[1], CL_TRUE, 0, sizeof(cl_float4)*positions.size(), positions.data());

	// Indicate the addition of new cells to all child modules:
	buffer_rearrange_info info;
	info.type = info.insertion;
	info.range_start = cell_count;
	info.range_end = cell_count + positions.size() - 1;

	cell_count += positions.size();

    for (int i=0; i<dependency_child_pointers.size(); i++)
    {
		dependency_child_pointers[i]->signal_buffer_rearrange(get_name(), 0, info);
    }

	return error::success;
}


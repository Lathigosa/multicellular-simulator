#include "division_after_concentration_treshold.h"

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

division_after_concentration_treshold::division_after_concentration_treshold(	cl::Platform & platform_cl,
							cl::Device & device_cl,
							cl::Context & context,
							cl::CommandQueue & command_queue) : particle_template(platform_cl, device_cl, context, command_queue)
{
	kernel_random_division = get_kernel_from_file("cl_kernels/division_due_to_concentration_treshold.cl", "simulate_division");
	kernel_concatenate = get_kernel_from_file("cl_kernels/sim_cell_division2.cl", "concatenate");
}

division_after_concentration_treshold::~division_after_concentration_treshold()
{
	//dtor
}

error division_after_concentration_treshold::initialize_buffers()
{
	max_buffer_size = dependency_pointers[0]->get_custom_uint("max_particle_count");
	new_cell_indices = dependency_pointers[0]->get_buffer(0, 0);
	new_cell_group_size = dependency_pointers[0]->get_buffer(1, 0);
	empty_cells = dependency_pointers[0]->get_buffer(2, 0);
	copied_cells = dependency_pointers[0]->get_buffer(3, 0);
	concentrations[0] = dependency_pointers[1]->get_buffer(0, 0);
	concentrations[1] = dependency_pointers[1]->get_buffer(0, 1);
	
	return error::success;
}

#ifndef NO_UI
error division_after_concentration_treshold::initialize_buffers(std::vector<GLuint> from_gl_buffer)
{
	// TODO: use gl-cl interoperability.

	return error::success;
}
#endif // NO_UI

error division_after_concentration_treshold::organize_unit()
{

}

std::vector<event_info> division_after_concentration_treshold::simulate_unit(float step_size)
{
	countdown--;
	if(countdown > 0)
		return {};

	countdown = 64;

	message_debug("Size = " << particle_count);

	cl::Event event_random_division;
	cl::Event event_concatenate;
	
	// ********************************************** //
	//         Simulate random cell divisions         //
	// ********************************************** //
	kernel_random_division.setArg(0, new_cell_indices);
	kernel_random_division.setArg(1, new_cell_group_size);
	kernel_random_division.setArg(2, particle_count);
	kernel_random_division.setArg(3, concentrations[buffer_index]);
	
	queue.enqueueNDRangeKernel(kernel_random_division, cl::NullRange, cl::NDRange(ceil(double(particle_count)/256.0)*256), cl::NDRange(256), nullptr, &event_random_division);   // TODO: determine proper local size!

	// ********************************************** //
	// 				Concatenate the data		      //
	// ********************************************** //
	kernel_concatenate.setArg(0, copied_cells);
	kernel_concatenate.setArg(1, new_cell_indices);
	kernel_concatenate.setArg(2, new_cell_group_size);
	
	queue.enqueueNDRangeKernel(kernel_concatenate, cl::NullRange, cl::NDRange(ceil(double(particle_count)/256.0)), cl::NullRange, nullptr, &event_concatenate);
	//TODO: REMOVE ANY AND ALL BLOCKING CALLS!!!!

	// Switch the buffers:
	buffer_index = !buffer_index;
	return {{event_random_division, "random division"}, {event_concatenate, "concatenate"}};
}

void division_after_concentration_treshold::signal_buffer_insert(const std::string sim_unit_name,
		                                  unsigned int buffer_index,
		                                  unsigned int range_start,
		                                  unsigned int count)
{
	//TODO: if(sim_unit_name == "sim_cell")
	if(particle_count + count > max_buffer_size)
		return;

	particle_count += count;
}

error division_after_concentration_treshold::signal_cell_reindex(const std::string sim_unit_name,
											cl::Buffer empty_cells,
		                              		cl::Buffer copied_cells,
		                              		unsigned int copied_count)
{
	if(particle_count + copied_count > max_buffer_size)
		return error::buffer_overflow;

	particle_count += copied_count;
	
	return error::success;
}


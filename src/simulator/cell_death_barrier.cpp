#include "cell_death_barrier.h"

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

cell_death_barrier::cell_death_barrier(	cl::Platform & platform_cl,
							cl::Device & device_cl,
							cl::Context & context,
							cl::CommandQueue & command_queue) : particle_template(platform_cl, device_cl, context, command_queue)
{
	kernel_random_death = get_kernel_from_file("cl_kernels/delete_particles_random.cl", "membrane_simulate_particles");
	kernel_concatenate = get_kernel_from_file("cl_kernels/sim_cell_division2.cl", "concatenate");
}

cell_death_barrier::~cell_death_barrier()
{
	//dtor
}

error cell_death_barrier::initialize_buffers()
{
	max_buffer_size = dependency_pointers[0]->get_custom_uint("max_particle_count");
	deleted_cell_indices = dependency_pointers[0]->get_buffer(4, 0);
	deleted_cell_group_size = dependency_pointers[0]->get_buffer(5, 0);
	empty_cells = dependency_pointers[0]->get_buffer(2, 0);
	//copied_cells = dependency_pointers[0]->get_buffer(3, 0);
	//concentrations[0] = dependency_pointers[1]->get_buffer(0, 0);
	//concentrations[1] = dependency_pointers[1]->get_buffer(0, 1);			

	return error::success;
}

#ifndef NO_UI
error cell_death_barrier::initialize_buffers(std::vector<GLuint> from_gl_buffer)
{
	

	return error::success;
}
#endif // NO_UI

error cell_death_barrier::organize_unit()
{

}

std::vector<event_info> cell_death_barrier::simulate_unit(float step_size)
{
	cl::Event event_random_death;
	cl::Event event_concatenate;
	
	// ********************************************** //
	//      TEST: Simulate random cell death          //
	// ********************************************** //
	kernel_random_death.setArg(0, deleted_cell_indices);
	kernel_random_death.setArg(1, deleted_cell_group_size);
	kernel_random_death.setArg(2, particle_count);
	kernel_random_death.setArg(3, std::rand() / static_cast<float>(RAND_MAX));
	
	queue.enqueueNDRangeKernel(kernel_random_death, cl::NullRange, cl::NDRange(ceil(double(particle_count)/256.0)*256), cl::NDRange(256), nullptr, &event_random_death);   // TODO: determine proper local size!

	// ********************************************** //
	// 				Concatenate the data		      //
	// ********************************************** //
	kernel_concatenate.setArg(0, empty_cells);
	kernel_concatenate.setArg(1, deleted_cell_indices);
	kernel_concatenate.setArg(2, deleted_cell_group_size);
	
	queue.enqueueNDRangeKernel(kernel_concatenate, cl::NullRange, cl::NDRange(ceil(double(particle_count)/256.0)), cl::NullRange, nullptr, &event_concatenate);
	
	

	// Switch the buffers:
	buffer_index = !buffer_index;
	return {{event_random_death, "random death"}, {event_concatenate, "concatenate"}};
}

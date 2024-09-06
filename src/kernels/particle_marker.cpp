#include "kernels/particle_marker.h"

#include "utilities/load_file.h"

#include <cmath>


ParticleMarker::ParticleMarker(cl::Platform & platform,
                               cl::Device & device,
                               cl::Context & context,
                               cl::CommandQueue & command_queue,
                               const std::string marker_code) : 
	data_kernel(platform, device, context, command_queue)
{
	build(marker_code);
}

ParticleMarker::~ParticleMarker() { }

void ParticleMarker::build(const std::string marker_code)
{
	std::string kernel_code = load_file("cl_kernels/snippet_particle_duplication/particle_duplication.cl");

	cl::Program::Sources sources_membrane_physics;

	
	// Insert the custom duplication kernel:
	size_t index = kernel_code.find("<user_code>", index);
	
	if (index == std::string::npos) return;
	
	// Make the replacement.
	//kernel_code.replace(index, 11, "if (random_float((float)get_global_id(0)/(float)count, 0.0f, (float)seed) > 0.9992f) MARK_DUPLICATE;");
	kernel_code.replace(index, 11, marker_code.c_str());
	
	sources_membrane_physics.push_back({kernel_code.c_str(), kernel_code.length()});

	cl::Program program_membrane_physics;
	program_membrane_physics = cl::Program(m_context, sources_membrane_physics);
	cl_int error = program_membrane_physics.build({m_device}, "-I standard_libraries");
	if(error != CL_SUCCESS)
	{
		message_error("Error building: " << program_membrane_physics.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device) << " error code" << error);
		exit(1);
	}

	kernel_random_marker = cl::Kernel(program_membrane_physics, "mark_particles");
	
	kernel_concatenate = get_kernel_from_file("cl_kernels/sim_cell_division2.cl", "concatenate");
	
	kernel_random_marker.getWorkGroupInfo(m_device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, &wg_size);
}

std::vector<event_info> ParticleMarker::run(const cl::Buffer& marked_particle_indices,
                                            const cl::Buffer& marked_particle_group_size,
                                            const cl::Buffer& copied_particles,
                                            unsigned int particle_count)
{
	cl::Event event_random_marker;
	cl::Event event_concatenate;

	// Simulate random cell divisions:
	kernel_random_marker.setArg(0, marked_particle_indices);
	kernel_random_marker.setArg(1, marked_particle_group_size);
	kernel_random_marker.setArg(2, particle_count);
	kernel_random_marker.setArg(3, (unsigned long)std::rand());
	
	m_command_queue.enqueueNDRangeKernel(kernel_random_marker, cl::NullRange, cl::NDRange(ceil(double(particle_count)/256.0)*256), cl::NDRange(256), nullptr, &event_random_marker);   // TODO: determine proper
	
	// Concatenate data:
	kernel_concatenate.setArg(0, copied_particles);
	kernel_concatenate.setArg(1, marked_particle_indices);
	kernel_concatenate.setArg(2, marked_particle_group_size);
	
	m_command_queue.enqueueNDRangeKernel(kernel_concatenate, cl::NullRange, cl::NDRange(ceil(double(particle_count)/256.0)), cl::NullRange, nullptr, &event_concatenate);

	return {{event_random_marker, "CELL_SYSTEM::DELETION random death"}, {event_concatenate, "CELL_SYSTEM::DELETION concatenate"}};
}

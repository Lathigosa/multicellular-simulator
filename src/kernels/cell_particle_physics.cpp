#include "kernels/cell_particle_physics.h"

#include "utilities/load_file.h"

#include <iomanip>
#include <sstream>
#include <cmath>


cell_particle_physics::cell_particle_physics(cl::CommandQueue & command_queue)
	: data_kernel(command_queue)
{
	build();
}

cell_particle_physics::~cell_particle_physics() { }

void cell_particle_physics::build()
{
	// Derive options to pass to the OpenCL program:
	std::ostringstream opts;

	opts << std::fixed << std::setprecision(1)
		<< "-DGRID_X_SIZE=" << grid_size_x << " "
		<< "-DGRID_Y_SIZE=" << grid_size_y << " "
		<< "-DGRID_Z_SIZE=" << grid_size_z << " "
		<< "-DVOXEL_X_SIZE=" << voxel_x_size << "f "
		<< "-DVOXEL_Y_SIZE=" << voxel_y_size << "f "
		<< "-DVOXEL_Z_SIZE=" << voxel_z_size << "f "
		<< "-DMAX_CELLS_PER_VOXEL=" << max_cells_per_voxel;

	std::string options = opts.str();

	// Load kernel files and compile them:
	std::string kernel_code1 = load_file("share/cl_kernels/cell_hookian_repel.cl");
	std::string kernel_code2 = load_file("share/cl_kernels/sort_particles_in_3d_grid.cl");

	cl::Program::Sources sources_membrane_physics;

	sources_membrane_physics.push_back({kernel_code2.c_str(), kernel_code2.length()});
	sources_membrane_physics.push_back({kernel_code1.c_str(), kernel_code1.length()});

	cl::Program program_membrane_physics;
	program_membrane_physics = cl::Program(m_context, sources_membrane_physics);
	cl_int error = program_membrane_physics.build({m_device}, options.c_str());
	if(error != CL_SUCCESS)
	{
		message_error(
			"Error building: "
			 << program_membrane_physics.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device)
			 << " error code"
		     << error);
		exit(1);
	}

	kernel_membrane_physics = cl::Kernel(program_membrane_physics, "cell_hookian_repel");
	kernel_sort_particles = cl::Kernel(program_membrane_physics, "spatial_particle_sort_3d");
	
	kernel_append_buffer = get_kernel_from_file("share/cl_kernels/append_buffer_cell_division.cl", "split_particle");
						
	kernel_sort_particles.getWorkGroupInfo(m_device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, &wg_size);

	// Initialize Buffers:
	grid_with_particles = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*max_cells_per_voxel*grid_size_x*grid_size_y*grid_size_z);
	grid_with_particles_count = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_uint)*grid_size_x*grid_size_y*grid_size_z);
}

std::vector<event_info> cell_particle_physics::run(data_buffer::ParticleData<cl_float4>& position_1,
                                                   data_buffer::ParticleData<cl_float4>& velocity_1,
                                                   data_buffer::ParticleData<cl_float4>& acceleration_1,
                                                   data_buffer::ParticleData<cl_float>& radius_1,
                                                   const data_buffer::ParticleData<cl_float4>& position_2,
                                                   const data_buffer::ParticleData<cl_float>& radius_2,
                                                   unsigned int particle_count)
{
	cl::Event event_clean_grid_with_particles;
	cl::Event event_clean_grid_with_particles_count;
	cl::Event event_sort_particles;
	cl::Event event_physics;

	//TODO: remove:
	float step_size = 0.1;

	// Clear voxel data before adding to it to prevent filling it with previous data:
	cl_uint fill_pattern_uint_full = 0; //0xFFFFFFFF;
	m_command_queue.enqueueFillBuffer(
		grid_with_particles,
		fill_pattern_uint_full,
		0,
		sizeof(cl_uint)*16*grid_size_x*grid_size_y*grid_size_z,
		nullptr,
		&event_clean_grid_with_particles
	);
	cl_uint fill_pattern_uint_zero = 0;
	m_command_queue.enqueueFillBuffer(
		grid_with_particles_count,
		fill_pattern_uint_zero,
		0,
		sizeof(cl_uint)*grid_size_x*grid_size_y*grid_size_z,
		nullptr,
		&event_clean_grid_with_particles_count
	);

	auto range = (unsigned int)(wg_size*ceil(double(particle_count)/double(wg_size)));

	std::vector<cl::Event> dependencies_for_kernel_sort_particles = { 
		event_clean_grid_with_particles_count,
		event_clean_grid_with_particles
	};

	// Sort particles into voxels:
	kernel_sort_particles.setArg(0, grid_with_particles);
	kernel_sort_particles.setArg(1, grid_with_particles_count);
	kernel_sort_particles.setArg(2, position_1.getFrontBuffer());
	kernel_sort_particles.setArg(3, particle_count);
	m_command_queue.enqueueNDRangeKernel(
		kernel_sort_particles,
		cl::NullRange,
		cl::NDRange(range),
		cl::NullRange,
		&dependencies_for_kernel_sort_particles,
		&event_sort_particles
	);

	std::vector<cl::Event> dependencies_for_kernel_membrane_physics = { event_sort_particles };
	
	// Simulate physics:
	kernel_membrane_physics.setArg(0, position_1.getFrontBuffer());
	kernel_membrane_physics.setArg(1, velocity_1.getFrontBuffer());
	kernel_membrane_physics.setArg(2, position_1.getBackBuffer());
	kernel_membrane_physics.setArg(3, velocity_1.getBackBuffer());
	kernel_membrane_physics.setArg(4, grid_with_particles);
	kernel_membrane_physics.setArg(5, grid_with_particles_count);
	kernel_membrane_physics.setArg(6, (unsigned int)particle_count);
	kernel_membrane_physics.setArg(7, step_size);
	m_command_queue.enqueueNDRangeKernel(kernel_membrane_physics,
		cl::NullRange,
		cl::NDRange(range),
		cl::NullRange,
		&dependencies_for_kernel_membrane_physics,
		&event_physics
	);

	std::vector<cl::Event> dependencies_for_memory_barrier = { event_physics };

	m_command_queue.enqueueBarrierWithWaitList(
		&dependencies_for_memory_barrier,
		nullptr
	);

	// TODO: enqueuebarrier!!!

	
	// Switch the buffers:
	position_1.swapBuffers();
	velocity_1.swapBuffers();

	EventLog log;
	log.add(event_info("CELL SYSTEM: sort particles", event_info::kernel, event_sort_particles));
	log.add(event_info("CELL SYSTEM: physics", event_info::kernel, event_physics));
	return std::move(log.get());
}

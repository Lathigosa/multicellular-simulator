/**
 * @file particle_system.h
 * @brief A GPU-based particle system whose particles can duplicate or delete themselves.
 *
 * This file defines the ParticleSystem class, which extends the `data_system` base class
 * and manages a collection of particles on the GPU. Particles can dynamically duplicate
 * or delete themselves during simulation, with operations executed efficiently using
 * OpenCL kernels.
 *
 * @author Nathan Boogerd
 * @date 2025-12-05
 */

#ifndef PARTICLE_SYSTEM_H_INCLUDED
#define PARTICLE_SYSTEM_H_INCLUDED

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include <CL/opencl.hpp>
#include <vector>

#include "core/event_info.h"
#include "core/data_system.h"

namespace data_buffer
{
	class AbstractParticleData;
}

class ParticleSystem : public DataSystem
{
public:
	ParticleSystem(cl::CommandQueue & command_queue, bool use_opengl_context);
	ParticleSystem(const ParticleSystem& from) = delete;			// TODO: add copy and assignment?
	ParticleSystem& operator=(const ParticleSystem&) = delete;

	//size_t getDimensions() const override {return 1;}

	virtual std::vector<event_info> customDeletionFunction(cl::Buffer& empty_cells,
	                                                       unsigned int& empty_count);
	
	virtual std::vector<event_info> customDuplicationFunction(cl::Buffer& empty_particles,
	                                                          cl::Buffer& copied_particles,
	                                                          unsigned int& copied_count,
															  std::vector<cl::Event>& wait_for_events);

	std::vector<event_info> finalizeDuplicationDeletion();

	void setupBuffers();

	virtual ~ParticleSystem();

	unsigned int getMaximalParticleCount();

	void manageArray(data_buffer::AbstractParticleData* array);

	const cl::Program& getStandardParticleFunctions();

protected:
	cl::Kernel kernel_random_deletion;
	cl::Kernel kernel_concatenate;
	cl::Kernel kernel_delete_sort;

	cl::Buffer deleted_cells;
	cl::Buffer deleted_cells_sorted;
	
	// The buffers are double buffered. buffer_index is 0 or 1 depending on which buffer is active:
	bool buffer_index = false;
	
	cl::Buffer list_of_particles_to_duplicate;		// One list per workgroup keeping track of which indices must be copied.
	cl::Buffer new_cell_group_size;					// The size of each list in "list_of_particles_to_duplicate".

	cl::Buffer list_of_particles_to_delete;			// One list per workgroup keeping track of which indices must be copied.
	cl::Buffer deleted_cell_group_size;				// The size of each list in "list_of_particles_to_delete".

	cl::Buffer concatenated_list_of_particles_to_delete;			// Concatenated list of empty cell indices.
	cl::Buffer concatenated_list_of_particles_to_duplicate;			// Concatenated list of cell indices to be copied.
	unsigned int copied_count = 0;					// Size of data inside "copied_cells".

protected:
	unsigned int particle_count = 0;
	unsigned int workgroup_size = 256;
	unsigned int maximal_cell_count = 32*32*32;	// TODO: change value

	// Test:
	int countdown = 64*4;

	std::vector<data_buffer::AbstractParticleData*> m_managed_arrays;

	
	cl::Program m_standard_functions;

private:
	std::vector<event_info> deleteMarkedParticles();
	std::vector<event_info> duplicateMarkedParticles();
	std::vector<event_info> clearDuplicationDeletionBuffers();

};

//#include "core/data_variable.h"

#endif // DATA_SYSTEM_H_INCLUDED

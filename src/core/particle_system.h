#ifndef PARTICLE_SYSTEM_H_INCLUDED
#define PARTICLE_SYSTEM_H_INCLUDED

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include "main.h"

#include <CL/cl2.hpp>
#include <vector>
#include <string>
#include <memory>
#include <map>

//class ParticleSystem;

#include "core/event_info.h"
#include "core/data_system.h"

namespace data_buffer
{
	class AbstractArray;
}

class ParticleSystem : public data_system
{
public:
	ParticleSystem(cl::Platform & platform,
	               cl::Device & device,
	               cl::Context & context,
	               cl::CommandQueue & command_queue);
	ParticleSystem(const ParticleSystem& from) = delete;			// TODO: add copy and assignment?
	ParticleSystem& operator=(const ParticleSystem&) = delete;

	//size_t getDimensions() const override {return 1;}

	virtual std::vector<event_info> customDeletionFunction(cl::Buffer& empty_cells,
	                                                       unsigned int& empty_count);
	
	virtual std::vector<event_info> customDuplicationFunction(cl::Buffer& empty_particles,
	                                                          cl::Buffer& copied_particles,
	                                                          unsigned int& copied_count);

	std::vector<event_info> finalizeDuplicationDeletion();

	void setupBuffers();

	virtual ~ParticleSystem();

	unsigned int getMaximalParticleCount();

	void manageArray(data_buffer::AbstractArray* array);

protected:
	cl::Kernel kernel_random_deletion;
	cl::Kernel kernel_concatenate;
	cl::Kernel kernel_delete_sort;

	cl::Buffer deleted_cells;
	cl::Buffer deleted_cells_sorted;
	
	// The buffers are double buffered. buffer_index is 0 or 1 depending on which buffer is active:
	bool buffer_index = false;
	
	cl::Buffer new_cell_indices;		// One list per workgroup keeping track of which indices must be copied.
	cl::Buffer new_cell_group_size;		// The size of each list in "new_cell_indices".

	cl::Buffer deleted_cell_indices;	// One list per workgroup keeping track of which indices must be copied.
	cl::Buffer deleted_cell_group_size;	// The size of each list in "new_cell_indices".

	cl::Buffer empty_cells;				// Concatenated list of empty cell indices.
	cl::Buffer copied_cells;			// Concatenated list of cell indices to be copied.
	unsigned int copied_count = 0;		// Size of data inside "copied_cells".

protected:
	unsigned int particle_count = 0;
	unsigned int workgroup_size = 256;
	unsigned int maximal_cell_count = 32*32*8;	// TODO: change value

	// Test:
	int countdown = 64*4;

	std::vector<data_buffer::AbstractArray*> m_managed_arrays;

	
	cl::Program m_standard_functions;
};

//#include "core/data_variable.h"

#endif // DATA_SYSTEM_H_INCLUDED

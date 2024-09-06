#ifndef cell_death_barrier_H
#define cell_death_barrier_H

#include "main.h"

#include "core/sim_unit_template.h"
#include "simulator/particle_template.h"

#include <glm/glm.hpp>
#include <vector>
#include <assert.h>

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

namespace simulation {

	/// Class in charge of simulating each cell:
    class cell_death_barrier : public particle_template
	{
		public:
			SIMULATOR_DEFINITION("sim_cell_death_barrier", P99_PROTECT({"sim_cell", "sim_simple_cell_physics"}))

			cell_death_barrier(cl::Platform & platform_cl, cl::Device & device_cl, cl::Context & context, cl::CommandQueue & command_queue);
			virtual ~cell_death_barrier();

			error initialize_buffers() override;
			#ifndef NO_UI
			error initialize_buffers(std::vector<GLuint> from_gl_buffer) override;
			#endif

			error organize_unit() override;		//
			std::vector<event_info> simulate_unit(float step_size) override;
			
		protected:
		private:
			cl::Kernel kernel_random_death;
			cl::Kernel kernel_concatenate;
			
			cl::Buffer deleted_cell_indices;		// One list per workgroup keeping track of which indices must be copied.
			cl::Buffer deleted_cell_group_size;		// The size of each list in "new_cell_indices".

			cl::Buffer empty_cells;				// Concatenated list of empty cell indices.
			unsigned int copied_count = 0;		// Size of data inside "copied_cells".

			cl::Buffer concentrations[2];			// Concentrations from sim_intracellular_reactions

			unsigned int workgroup_size = 256;

	};
}

#endif // cell_death_barrier_H

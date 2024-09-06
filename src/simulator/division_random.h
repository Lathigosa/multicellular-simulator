#ifndef DIVISION_RANDOM_H
#define DIVISION_RANDOM_H

#include "main.h"

#include "core/sim_unit_template.h"

#include <glm/glm.hpp>
#include <vector>
#include <assert.h>

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

namespace simulation {

	/// Class in charge of simulating each cell:
    class division_random : public sim_unit_template
	{
		public:
			SIMULATOR_DEFINITION("sim_division_random", {"sim_cell"})

			division_random(cl::Platform & platform_cl, cl::Device & device_cl, cl::Context & context, cl::CommandQueue & command_queue);
			virtual ~division_random();

			error initialize_buffers() override;
			#ifndef NO_UI
			error initialize_buffers(std::vector<GLuint> from_gl_buffer) override;
			#endif

			error organize_unit() override;		//
			std::vector<event_info> simulate_unit(float step_size) override;
			cl::Buffer get_buffer(unsigned int index, bool double_buffer) override
			{
				switch(index) {
					case 0: return new_cell_indices;
					case 1: return new_cell_group_size;
				}
			}
			unsigned int get_buffer_index(const std::string name) const override
			{
				if (name == "new_cell_indices")			return 0;
				if (name == "new_cell_group_size")		return 1;

				// Crash when the name string does not match anything:
				message_error("ERROR: name does not match any internal values!");
				assert(false);
			}

			error signal_cell_reindex(const std::string sim_unit_name, 
		                              			cl::Buffer empty_cells,
		                              			cl::Buffer copied_cells,
		                              			unsigned int copied_count) override;

			// Custom functions:
			error add_cells(std::vector<cl_float4> positions, std::string type);
		protected:
		private:
			cl::Program::Sources sources_membrane_physics;
			cl::Program program_membrane_physics;
			cl::Kernel kernel_random_division;
			cl::Program::Sources sources_concatenate;
			cl::Program program_concatenate;
			cl::Kernel kernel_concatenate;
			
			// Double buffered:
			bool buffer_index = false;
			
			cl::Buffer new_cell_indices;		// One list per workgroup keeping track of which indices must be copied.
			cl::Buffer new_cell_group_size;		// The size of each list in "new_cell_indices".

			cl::Buffer empty_cells;				// Concatenated list of empty cell indices.
			cl::Buffer copied_cells;			// Concatenated list of cell indices to be copied.
			unsigned int copied_count = 0;		// Size of data inside "copied_cells".

			unsigned int cell_count = 0;
			unsigned int maximal_cell_count = 32*32*16;	// TODO: change value

	};
}

#endif // DIVISION_RANDOM_H

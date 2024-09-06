#ifndef SIM_INTRACELLULAR_REACTIONS_H
#define SIM_INTRACELLULAR_REACTIONS_H

#include "main.h"

#include "core/sim_unit_template.h"
#include "simulator/particle_template.h"

#include <glm/glm.hpp>
#include <vector>

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

namespace simulation
{

	//struct membrane_particle
	//{
	//	glm::vec3 position;
	//	glm::vec3 normal;

	//	float surface_area;								// The surface area that this piece of membrane represents. Is used to determine spring sizes.

	//	std::vector<unsigned short> connected_to;		// The particles that this membrane particle is connected to.
	//};

	class intracellular_reactions : public particle_template
	{
		public:
			SIMULATOR_DEFINITION("sim_intracellular_reactions", {"sim_cell"})

			intracellular_reactions(cl::Platform & platform_cl, cl::Device & device_cl, cl::Context & context, cl::CommandQueue & command_queue);
			virtual ~intracellular_reactions();

			error initialize_buffers() override;
			#ifndef NO_UI
			error initialize_buffers(std::vector<GLuint> from_gl_buffer) override;
			#endif

			error organize_unit() override;
			std::vector<event_info> simulate_unit(float step_size) override;

			error signal_cell_reindex(const std::string sim_unit_name, 
		                             			cl::Buffer empty_cells,
		                             			cl::Buffer copied_cells,
		                              			unsigned int copied_count) override;

			const void expose_lua_library(lua_State* L) const override;

			// Own library functions:
			error set_reactions(std::string reaction_definitions);
			
		protected:
		private:
			cl::Program::Sources sources_membrane_physics;
			cl::Program::Sources sources_append_buffer;
			cl::Program program_membrane_physics;
			cl::Program program_append_buffer;
			cl::Kernel kernel_membrane_physics;
			cl::Kernel kernel_append_buffer;

			size_t wg_size = 32;

		public:		// debug only (TODO: remove 'public')
			unsigned int buffer_concentrations;

			unsigned int molecular_species_count;
	};

}

#endif // SIM_INTRACELLULAR_REACTIONS_H

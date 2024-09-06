#ifndef SIM_division_plane_H
#define SIM_division_plane_H

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

	class division_plane : public particle_template
	{
		public:
			SIMULATOR_DEFINITION("sim_division_plane", {"sim_cell"})

			division_plane(cl::Platform & platform_cl, cl::Device & device_cl, cl::Context & context, cl::CommandQueue & command_queue);
			virtual ~division_plane();

			error initialize_buffers() override;
			#ifndef NO_UI
			error initialize_buffers(std::vector<GLuint> from_gl_buffer) override;
			#endif

			error signal_cell_reindex(const std::string sim_unit_name, 
		                              			cl::Buffer empty_cells,
		                              			cl::Buffer copied_cells,
		                              			unsigned int copied_count) override;

			std::vector<event_info> simulate_unit(float step_size) override;

			const void expose_lua_library(lua_State* L) const override;

			
		protected:
		private:
			cl::Kernel kernel_append_buffer;
			unsigned int buffer_division_plane;
	};

}

#endif // SIM_division_plane_H

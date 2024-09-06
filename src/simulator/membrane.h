#ifndef MEMBRANE_H
#define MEMBRANE_H

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

	struct membrane_particle
	{
		glm::vec3 position;
		glm::vec3 normal;

		float surface_area;								// The surface area that this piece of membrane represents. Is used to determine spring sizes.

		std::vector<unsigned short> connected_to;		// The particles that this membrane particle is connected to.
	};

	class membrane : public particle_template
	{
		public:
			SIMULATOR_DEFINITION("sim_simple_cell_physics", P99_PROTECT({"sim_cell", "sim_division_plane"}))

			membrane(cl::Platform & platform_cl, cl::Device & device_cl, cl::Context & context, cl::CommandQueue & command_queue);
			virtual ~membrane();

			error initialize_buffers() override;
			#ifndef NO_UI
			error initialize_buffers(std::vector<GLuint> from_gl_buffer) override;
			#endif

			error organize_unit() override;		//
			std::vector<event_info> simulate_unit(float step_size) override;
			unsigned int get_buffer_size(unsigned int index, bool double_buffer) const override;

			error signal_cell_reindex(const std::string sim_unit_name, 
		                              			cl::Buffer empty_cells,
		                              			cl::Buffer copied_cells,
		                             			unsigned int copied_count) override;

			

			const void expose_lua_library(lua_State* L) const override;

			// Own library functions:
			error spawn_cells(unsigned int count, cl_float4* positions, cl_float4* velocities = nullptr);
			
		protected:
		private:
			cl::Program::Sources sources_membrane_physics;
			cl::Program::Sources sources_append_buffer;
			cl::Program program_membrane_physics;
			cl::Program program_append_buffer;
			cl::Kernel kernel_membrane_physics;
			cl::Kernel kernel_append_buffer;

			cl::Program::Sources sources_sort_particles;
			cl::Program program_sort_particles;
			cl::Kernel kernel_sort_particles;

			cl::Kernel kernel_delete_particles;

			size_t wg_size = 32;

		public:		// debug only (TODO: remove 'public')
			// Double buffered:
			//bool buffer_index = false;
			cl::Buffer position[2];
			cl::Buffer velocity[2];
			unsigned int buffer_position;
			unsigned int buffer_velocity;

			cl::Buffer grid_with_particles;				// Buffer containing particle indices per voxel.
			cl::Buffer grid_with_particles_count;		// Buffer containing amount of particles per voxel.

			//unsigned int particle_count;
			//unsigned int max_buffer_size;

			unsigned int grid_size_x = 64;
			unsigned int grid_size_y = 64;
			unsigned int grid_size_z = 64;
	};

}

#endif // MEMBRANE_H

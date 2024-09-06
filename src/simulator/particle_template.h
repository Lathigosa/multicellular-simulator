#ifndef PARTICLE_TEMPLATE_H
#define PARTICLE_TEMPLATE_H

#include "main.h"

#include "core/sim_unit_template.h"

#include <glm/glm.hpp>
#include <vector>
#include <map>

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

namespace simulation
{

	class particle_template : public sim_unit_template
	{
		public:
			SIMULATOR_DEFINITION("template_particle_data", {"sim_cell"})

			particle_template(cl::Platform & platform_cl, cl::Device & device_cl, cl::Context & context, cl::CommandQueue & command_queue);
			virtual ~particle_template();

			void init() override;
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

			error signal_particle_remove(const std::string sim_unit_name,
		                                 		cl::Buffer empty_cells,
		                                 		unsigned int empty_count) override;

			const void expose_lua_library(lua_State* L) const override;

			// Own library functions:
			error spawn_cells(unsigned int count, cl_float4* positions, cl_float4* velocities = nullptr);
			
		protected:
		private:

			cl::Kernel kernel_append_buffer;
			cl::Kernel delete_particles_float4;

			size_t wg_size = 32;

			std::vector<unsigned int> particle_buffer_indices;

		protected:
			unsigned int create_particle_double_buffer(std::string name, unsigned int entry_size);

			bool buffer_index = false;
			
			unsigned int particle_count;
			unsigned int max_buffer_size;
	};

}

#endif // PARTICLE_TEMPLATE_H

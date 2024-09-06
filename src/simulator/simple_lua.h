#ifndef SIMPLE_LUA_H
#define SIMPLE_LUA_H

#include "main.h"

#include "core/sim_unit_template.h"

#include "core/lua_functions.h"

#include <glm/glm.hpp>
#include <vector>
#include <string>

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI



namespace simulation
{

	class simple_lua : public sim_unit_template
	{
		public:
			SIMULATOR_DEFINITION("sim_simple_lua", {})

			const std::string lua_name;
			lua_State* L = nullptr;

			simple_lua(cl::Platform & platform_cl,
			           cl::Device & device_cl,
			           cl::Context & context,
			           cl::CommandQueue & command_queue,
			           const std::string name,
			           lua_State* state);
			
			virtual ~simple_lua();

			error initialize_buffers() override;
			#ifndef NO_UI
			error initialize_buffers(std::vector<GLuint> from_gl_buffer) override;
			#endif

			error organize_unit() override;		//
			std::vector<event_info> simulate_unit(float step_size) override;
			cl::Buffer get_buffer(unsigned int index, bool double_buffer) override;
			unsigned int get_buffer_index(const std::string name) const override;
			unsigned int get_buffer_size(unsigned int index, bool double_buffer) const override;

			error signal_cell_reindex(const std::string sim_unit_name, 
		                              			cl::Buffer empty_cells,
		                              			cl::Buffer copied_cells,
		                              			unsigned int copied_count) override;
		protected:
		private:
			cl::Program::Sources sources_membrane_physics;
			cl::Program::Sources sources_append_buffer;
			cl::Program program_membrane_physics;
			cl::Program program_append_buffer;
			cl::Kernel kernel_membrane_physics;
			cl::Kernel kernel_append_buffer;

			

		public:		// debug only (TODO: remove 'public')
			// Double buffered:
			bool buffer_index = false;
			cl::Buffer position[2];
			cl::Buffer velocity[2];

			unsigned int particle_count;
			unsigned int max_buffer_size;
	};

	
}

extern "C" cl_platform_id lua_cl_get_platform_id(void* pointer);


#endif // SIMPLE_LUA_H

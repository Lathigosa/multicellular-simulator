#include "simple_lua.h"

using namespace simulation;

#include <iostream>
#include "resources/opencl_kernels.h"
#include <cmath>

#include <CL/cl_gl.h>

#include "utilities/load_file.h"

#include <cstdlib>

simple_lua::simple_lua(	cl::Platform & platform_cl,
					cl::Device & device_cl,
					cl::Context & context,
					cl::CommandQueue & command_queue,
			        const std::string name,
                    lua_State* state
                       ) : sim_unit_template(platform_cl, device_cl, context, command_queue),
							lua_name(name),
							L(state)
{
	// Expose the opencl objects to the Lua implementation:
	// First test to see if there is a Lua implementation:
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_simulations");
	lua_getfield(L, -1, lua_name.c_str());
	
	if(lua_type(L, -1) == LUA_TTABLE)
	{
		// If core.registered_simulations[name] exists and is a table:
		// Expose "this" pointer:
		message_debug("Pushing this = " << this << " " << lua_name.c_str());
		lua_pushlightuserdata(L, this);
		lua_setfield(L, -2, "__sim_pointer");

		// Check if "on_create" function exists and execute it:
		lua_getfield(L, -1, "on_create");
		lua_insert(L, -2);
		if(lua_type(L, -2) == LUA_TFUNCTION)
		{
			lua_call(L, 1, 0);
		}
	}
}

simple_lua::~simple_lua()
{
	//dtor
}

error simple_lua::initialize_buffers()
{
	message_debug("INITIALIZING SIMPLE LUA!!!");

	// First test to see if there is a lua implementation:
	lua_getglobal(L, "core");
	lua_pushstring(L, "registered_simulations");
	lua_gettable(L, -2);

	lua_pushstring(L, lua_name.c_str());
	lua_gettable(L, -2);
	if(lua_type(L, -1) == LUA_TTABLE)
	{
		// If core.registered_simulations[name] exists and is a table:
		// Check if function exists and execute it:
		lua_getfield(L, -1, "on_init");
		lua_insert(L, -2);
		if(lua_type(L, -2) == LUA_TFUNCTION)
		{
			lua_call(L, 1, 0);
		}
	}
	
	return error::success;
}

#ifndef NO_UI
error simple_lua::initialize_buffers(std::vector<GLuint> from_gl_buffer)
{
	message_debug("Creating shared buffers of <membrane>.");

	// TODO: if cl_khr_gl_sharing isn't supported, disable this function!

	

	return error::success;
}
#endif // NO_UI

cl::Buffer simple_lua::get_buffer(unsigned int index, bool double_buffer)
{
	if(index == 0)
		return position[0 + double_buffer];
	
	if(index == 1)
		return velocity[0 + double_buffer];

	// TODO: Else, error!
}

unsigned int simple_lua::get_buffer_size(unsigned int index, bool double_buffer) const
{
	if(index <= 1)
		return particle_count * sizeof(cl_float4);

	return 0;
}

unsigned int simple_lua::get_buffer_index(const std::string name) const
{
	if(name == "position")
		return 0;
	
	if(name == "velocity")
		return 1;

	// TODO: Else, error!
	
	return 0;
}

error simple_lua::organize_unit()
{
	return error::undefined_function;
}

std::vector<event_info> simple_lua::simulate_unit(float step_size)
{
	// TODO: remove overhead!
	
	// First test to see if there is a lua implementation:
													// Stack: 0
	lua_getglobal(L, "core");						// Stack: 1
	lua_pushstring(L, "registered_simulations");	// Stack: 2
	lua_gettable(L, -2);							// Stack: 2

	lua_pushstring(L, lua_name.c_str());			// Stack: 3
	lua_gettable(L, -2);							// Stack: 3
	if(lua_type(L, -1) == LUA_TTABLE)
	{
		// If core.registered_simulations[name] exists and is a table:
		// Check if function exists and execute it:
		lua_getfield(L, -1, "on_simulate");			// Stack: 4
		lua_insert(L, -2);							// Stack: 4
		if(lua_type(L, -2) == LUA_TFUNCTION)
		{
			if(lua_pcall(L, 1, 0, 0))				// Stack: 2 (one arg + 1) (or 3, on error)
			{
				//std::string error_message = lua_tostring(L, -1);
				message_lua(lua_tostring(L, -1));
				lua_pop(L, 1);
			}
		}

		message_lua("Returned from Lua simulation function!");
													// Stack: 2
	} else {
		lua_pop(L, 1);								// Stack: 2
	}

	lua_pop(L, 2);									// Stack: 0
	
	// Switch the buffers:
	buffer_index = !buffer_index;

	message_lua("Returning with 'success'!");


	//TODO: return event list!!!
	return {};
}

error simple_lua::signal_cell_reindex(const std::string sim_unit_name,
											cl::Buffer empty_cells,
		                              		cl::Buffer copied_cells,
		                              		unsigned int copied_count)
{
	

	//message_debug("Copied" << copied_count);
	
	return error::success;
}

// Lua wrapper functions (dangerous):

extern "C" cl_platform_id lua_cl_get_platform_id(void* pointer)
{
	simulation::simple_lua* sim =
			reinterpret_cast<simulation::simple_lua*>(pointer);
	return sim->get_platform()();
}

extern "C" cl_device_id lua_cl_get_device_id(void* pointer)
{
	simulation::simple_lua* sim =
			reinterpret_cast<simulation::simple_lua*>(pointer);
	return sim->get_device()();
}

extern "C" cl_context lua_cl_get_context(void* pointer)
{
	simulation::simple_lua* sim =
			reinterpret_cast<simulation::simple_lua*>(pointer);
	return sim->get_context()();
}

extern "C" cl_command_queue lua_cl_get_command_queue(void* pointer)
{
	simulation::simple_lua* sim =
			reinterpret_cast<simulation::simple_lua*>(pointer);
	return sim->get_command_queue()();
}

#include "lua_functions.h"

#include <iostream>

#include "resources/strings.h"

#include "simulation_file_object.h"
#include "simulator/membrane.h"
#include "utilities/safe_pointer.h"
#include "simulator/simulator_map.h"
#include "renderer/renderer_map.h"

#include "simulator/simple_lua.h"

//#if defined(DEBUG) | defined(_DEBUG)
//#define message_lua(message) std::cout << "LUA: " << message << std::endl;
//#else
//#define message_debug(message)
//#endif

#if !defined LUA_VERSION_NUM
/* Lua 5.0 */
#define luaL_Reg luaL_reg
#endif

namespace lua
{

namespace core
{
	/// @param 1 name of the registered cell
	/// @param 2 definition table of the cell
	int register_cell (lua_State* L)
	{
		const int xoffset = lua_tointeger(L, 1);
		const int yoffset = lua_tointeger(L, 2);

		message_lua("Registered cell!");

		// Get pointer to core class:
		lua_getfield(L, LUA_REGISTRYINDEX, "__sfo_pointer");
		simulation_file_object* sfo =
			reinterpret_cast<simulation_file_object*>(lua_touserdata(L, -1));
		lua_pop(L, 1);

		// Get pointer to core class:
		lua_getfield(L, LUA_REGISTRYINDEX, "__sim_membrane_pointer");
		void* sim_membrane = lua_touserdata(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, LUA_REGISTRYINDEX, "__undefined_pointer");
		void* undefined = lua_touserdata(L, -1);
		lua_pop(L, 1);

		message_lua("SFO: " << sfo << ", sim_membrane: " << sim_membrane << ", undefined: " << undefined);
		//sfo->world.simulation_units.at(0)->

		lua_pushinteger(L, 7);
		lua_pushinteger(L, 5);
		return 2;
	}



	
	// Register simulator.
	/// @param 1 name of the registered simulator
	/// @param 2 definition table of the simulator
	int register_simulation (lua_State* L)
	{
		int argc = lua_gettop(L);
		//lua_pop(L, 1);

		if(argc != 2)
			return luaL_error(L, res::str::lua::error_missing_arguments);

		// Get core.registered_simulations:
		lua_getglobal(L, "core");
		lua_insert(L, 1);   //[1] = core

		lua_pushstring(L, "registered_simulations");
		lua_gettable(L, 1);
		lua_insert(L, 2);   //[2] = core.registered_simulations

		// Set metatable "core.mt_simulation" (found in standard_libraries.core):
		lua_getfield(L, 1, "mt_simulation");
		lua_setmetatable(L, -2);

		// Finally, place the definition into the core.registered_simulations table:
		lua_settable(L, 2);



		//TODO: 2 still in stack.

		return 0;
	}


	std::vector<simulation::sim_unit_template*> get_dependency_pointers (lua_State* L, const std::string name, std::vector<std::string> deps)
	{
		std::vector<simulation::sim_unit_template*> deps_pointers;
		
		for (int i=0; i<deps.size(); i++)
		{
			// Get pointer to sfo:
			std::string lua_ref_name = "__" + deps.at(i) + "_pointer";
			lua_getfield(L, LUA_REGISTRYINDEX, lua_ref_name.c_str());
			simulation::sim_unit_template* ref =
				reinterpret_cast<simulation::sim_unit_template*>(lua_touserdata(L, -1));
			lua_pop(L, 1);

			if (ref == nullptr)
			{
				luaL_error(L, res::str::lua::error_include_simulator_dependency, name.c_str(), deps.at(i).c_str());
			}

			deps_pointers.push_back(ref);
		}

		return deps_pointers;
	}

	

	int use_simulation (lua_State* L)
	{
		int argc = lua_gettop(L);
		
		const std::string name = lua_tostring(L, 1);

		// Get pointer to sfo:
		lua_getfield(L, LUA_REGISTRYINDEX, "__sfo_pointer");
		simulation_file_object* sfo =
			reinterpret_cast<simulation_file_object*>(lua_touserdata(L, -1));
		lua_pop(L, 1);

		if (sfo == nullptr)
			return 0;

		// First test to see if there is a lua implementation:
		lua_getglobal(L, "core");
		lua_pushstring(L, "registered_simulations");
		lua_gettable(L, -2);

		lua_pushstring(L, name.c_str());
		lua_gettable(L, -2);
		if(lua_type(L, -1) == LUA_TTABLE)
		{
			// If core.registered_simulations[name] exists and is a table:
			message_lua("A table exists!");

			std::vector<std::string> deps;

			// Check dependencies (only if they are defined):
			lua_getfield(L, -1, "dependencies");
			if(lua_type(L, -1) == LUA_TTABLE)
			{
				//TODO: allow tables as well!
				
			} else if(lua_type(L, -1) == LUA_TSTRING)
			{
				const std::string dep_name = lua_tostring(L, -1);
				message_lua("Dependency found! " << dep_name);
				
				deps.push_back(dep_name);
				
			}
			std::vector<simulation::sim_unit_template*> deps_pointers = get_dependency_pointers(L, name, deps);

			// Add the new simulator:
			sfo->world->simulation_units.push_back(
					std::unique_ptr<simulation::sim_unit_template>(
								new simulation::simple_lua(
											sfo->world->default_platform,
											sfo->world->default_device,
											sfo->world->opencl_context,
											sfo->world->queue,
								            name,
								            L)
								)
					);

			// Pass the pointer list to the sim unit:
			sfo->world->simulation_units.back()->set_dependency_pointers(deps_pointers);

			// Initialize buffers. TODO: include the ability to set options in
			// this command!
			sfo->world->simulation_units.back()->init();

			
		} else

		// Else, see if the requested simulator exists and add it:
		if (simulation::standard_sim_units.count(name) > 0)
		{
			std::vector<simulation::sim_unit_template*> deps_pointers;

			// Check dependencies (only if they are defined):
			if (simulation::standard_sim_unit_dependencies.count(name) > 0)
			{
                std::vector<std::string> deps =
						simulation::standard_sim_unit_dependencies.at(name)();

				if (argc > 1 && lua_istable(L, 2))
				{
					for (unsigned int i=0; i<deps.size(); i++)
					{
						// See if the dependencies are overridden:
						lua_getfield(L, 2, deps.at(i).c_str());		// Get index [1]:
						message_lua("deps.at(i).c_str() = " << deps.at(i).c_str());
						if (lua_istable(L, -1))
						{
							message_lua("WE FOUND A TABLE");
							// Override:

							// Get the first entry, which should be a simulator_ref:
							lua_getfield(L, -1, "pointer");
								
							simulation::sim_unit_template* ref =
								reinterpret_cast<simulation::sim_unit_template*>(lua_touserdata(L, -1));
							lua_pop(L, 1);
							
							if (ref == nullptr)
							{
								return luaL_error(L, res::str::lua::error_include_renderer_simulator_dependency, name.c_str(), deps.at(i).c_str());
							}
							
							deps_pointers.push_back(ref);
							
							
						}
						else
							return luaL_error(L, "Expected simulator_ref table!", name.c_str(), deps.at(i).c_str());

						lua_pop(L, 1);
					}
				} else {
					for (int i=0; i<deps.size(); i++)
					{
						// Get pointer to sfo:
						std::string lua_ref_name = "__" + deps.at(i) + "_pointer";
						lua_getfield(L, LUA_REGISTRYINDEX, lua_ref_name.c_str());
						simulation::sim_unit_template* ref =
							reinterpret_cast<simulation::sim_unit_template*>(lua_touserdata(L, -1));
						lua_pop(L, 1);

						if (ref == nullptr)
						{
							return luaL_error(L, res::str::lua::error_include_simulator_dependency, name.c_str(), deps.at(i).c_str());
						}

						deps_pointers.push_back(ref);
					}
				}
			}

			// Add the new simulator:
			sfo->world->simulation_units.push_back(
					std::unique_ptr<simulation::sim_unit_template>(
								simulation::standard_sim_units.at(name)(
											sfo->world->default_platform,
											sfo->world->default_device,
											sfo->world->opencl_context,
											sfo->world->queue)
								)
					);

			// Pass the pointer list to the sim unit:
			sfo->world->simulation_units.back()->set_dependency_pointers(deps_pointers);

			// Add child pointers as well:
			message_lua("child pointer " << deps_pointers.size());
			for (int i=0; i<deps_pointers.size(); i++)
			{
				message_lua("added child pointer");
				deps_pointers.at(i)->add_child_pointer(sfo->world->simulation_units.back().get());
			}

			// Initialize buffers. TODO: include the ability to set options in
			// this command!
			sfo->world->simulation_units.back()->init();

			

			message_lua("registered " << name);
		} else {
			return luaL_error(L, res::str::lua::error_include_undefined_simulator);
		}

		// Register a pointer to the newly added component in the LUA environment:
		std::string registry_name = "__"
			+ sfo->world->simulation_units.back()->get_name()
			+ "_pointer";
		if ( registry_name != "__undefined_pointer" )
		{
			lua_pushlightuserdata(L, sfo->world->simulation_units.back().get());
			lua_setfield(L, LUA_REGISTRYINDEX, registry_name.c_str());

			std::cout << "LUA INIT: set " << registry_name << std::endl;
		}

		// Create the reference table containing the data of the added simulator:
		lua_createtable(L, 0, 4);

		lua_pushstring(L, sfo->world->simulation_units.back()->get_name().c_str());
		lua_setfield(L, -2, "name");

		lua_pushlightuserdata(L, sfo->world->simulation_units.back().get());
		lua_setfield(L, -2, "pointer");

		lua_pushnumber(L, sfo->world->simulation_units.size() - 1);
		lua_setfield(L, -2, "index");

		lua_pushnumber(L, 1);
		lua_setfield(L, -2, "queue");

		sfo->world->simulation_units.back()->expose_lua_library(L);

		return 1;
	}

	int use_renderer (lua_State* L)
	{
		int argc = lua_gettop(L);
		
		const std::string name = lua_tostring(L, 1);

		// Get pointer to sfo:
		lua_getfield(L, LUA_REGISTRYINDEX, "__sfo_pointer");
		simulation_file_object* sfo =
			reinterpret_cast<simulation_file_object*>(lua_touserdata(L, -1));
		lua_pop(L, 1);

		if (sfo == nullptr)
			return 0;


		// See if the requested simulator exists and add it:
		if (renderer::standard_render_units.count(name) > 0)
		{
			std::vector<simulation::sim_unit_template*> deps_pointers;

			std::vector<std::string> dependency_buffer_indices = {};
			

			// Check dependencies (only if they are defined):
			if (renderer::standard_render_unit_dependencies.count(name) > 0)
			{
                std::vector<std::string> deps =
						renderer::standard_render_unit_dependencies.at(name)();

				dependency_buffer_indices.resize(deps.size());

				if (argc > 1 && lua_istable(L, 2))
				{
					for (unsigned int i=0; i<deps.size(); i++)
					{
						// See if the dependencies are overridden:
						lua_getfield(L, 2, deps.at(i).c_str());		// Get index [1]:
						message_lua("deps.at(i).c_str() = " << deps.at(i).c_str());
						if (lua_istable(L, -1))
						{
							message_lua("WE FOUND A TABLE");
							// Override:

							// Get the first entry, which should be a simulator_ref:
							
							lua_pushnumber(L, 1);
							lua_gettable(L, -2);

							if (lua_istable(L, -1))
							{
								lua_getfield(L, -1, "pointer");
								
								simulation::sim_unit_template* ref =
									reinterpret_cast<simulation::sim_unit_template*>(lua_touserdata(L, -1));
								lua_pop(L, 1);
								
								if (ref == nullptr)
								{
									return luaL_error(L, res::str::lua::error_include_renderer_simulator_dependency, name.c_str(), deps.at(i).c_str());
								}
								
								deps_pointers.push_back(ref);
							}
							else
								return luaL_error(L, "Expected simulator_ref table!", name.c_str(), deps.at(i).c_str());
							//deps.at(i) = lua_tostring(L, -1);
							lua_pop(L, 1);

							// Get the second entry, which should be a string with the name of the buffer:

							lua_pushnumber(L, 2);
							lua_gettable(L, -2);

							if (!lua_isstring(L, -1))
								return luaL_error(L, "Expected string!", name.c_str(), deps.at(i).c_str());

							dependency_buffer_indices.at(i) = lua_tostring(L, -1);
							lua_pop(L, 1);
						}
					}
				} else {	// No override:
				for (unsigned int i=0; i<deps.size(); i++)
					{
					
						// Get pointer to simulator dependencies:
						std::string lua_ref_name = "__" + deps.at(i) + "_pointer";
						lua_getfield(L, LUA_REGISTRYINDEX, lua_ref_name.c_str());
						simulation::sim_unit_template* ref =
							reinterpret_cast<simulation::sim_unit_template*>(lua_touserdata(L, -1));
						lua_pop(L, 1);

						if (ref == nullptr)
						{
							return luaL_error(L, res::str::lua::error_include_renderer_simulator_dependency, name.c_str(), deps.at(i).c_str());
						}

						deps_pointers.push_back(ref);
					}
				}

			}

			// Add the new renderer:
			
			sfo->renderer->render_units.push_back(
					std::unique_ptr<render_unit_template>(
								renderer::standard_render_units.at(name)()
								)
					);

			// TODO: testestestest
			

			// Pass the pointer list to the sim unit:
			sfo->renderer->render_units.back()->set_dependency_pointers(deps_pointers);
			sfo->renderer->render_units.back()->set_dependency_buffer_indices(dependency_buffer_indices);

			message_lua("registered renderer " << name);
		} else {
			return luaL_error(L, res::str::lua::error_include_undefined_renderer);
		}

		// Register a pointer to the newly added component in the LUA environment:
		std::string registry_name = "__renderer_"
			+ sfo->renderer->render_units.back()->get_name()
			+ "_pointer";
		if ( registry_name != "__renderer_undefined_pointer" )
		{
			lua_pushlightuserdata(L, sfo->renderer->render_units.back().get());
			lua_setfield(L, LUA_REGISTRYINDEX, registry_name.c_str());

			std::cout << "LUA INIT: set " << registry_name << std::endl;
		}

		return 0;
	}

	int set_grid_color(lua_State* L)
	{

	}

	//int set_grid_color(lua_State* L)
	//{
//
	//}

	const luaL_Reg functions[] =
	{
		{"register_cell", register_cell},
		{"use_simulation", use_simulation},
		{"use_renderer", use_renderer},
		{"register_simulation", register_simulation},
		{nullptr, nullptr}
	};
}
}

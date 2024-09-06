#include "main.h"
#include "simulation_file_object.h"

#include "lua_functions.h"

#include <iostream>
#include <string>

simulation_file_object::simulation_file_object()
{
	world.reset(new simulation::simulation_world());
	renderer.reset(new render_collection());
	
	// Initialize simulator:
	world->init(false);
	renderer->init();
}

void simulation_file_object::open_file(const std::string& filename)
{
	if(L) lua_close(L);
	
	world.reset(new simulation::simulation_world());
	renderer.reset(new render_collection());
	
	// Initialize simulator:
	world->init(false);
	renderer->init();

	// Initialize Lua:
	L = luaL_newstate();

	if (L == nullptr)
		return;

	// Load standard LUA libraries:
	luaL_openlibs(L);

	// Extend the package searchers:
	{
		lua_getglobal(L, "package");
		lua_getfield(L, -1, "path");
		const std::string search_path = lua_tostring(L, -1);
		lua_pop(L, 1);
		const std::string extended_path = search_path + std::string(";./standard_libraries/?.lua;./?/init.lua;./standard_libraries/?/init.lua");
		lua_pushstring(L, extended_path.c_str());
		lua_setfield(L, -2, "path");
		lua_pop(L, 2);
	}
	
	// Load libraries defined by this program:
	luaL_register(L, "core", lua::core::functions);
	if ( luaL_dofile(L, "./standard_libraries/core/init.lua") )
	{
		std::string error_message = lua_tostring(L, -1);
		lua_close(L);
		L = nullptr;
		throw sfo_parsing_exception(error_message);
		return;
	}

	// Store a pointer to the "simulation_file_object" class and all simulation
	// unit classes in LUA, so that their member functions can be called:
	lua_pushlightuserdata(L, this);
	lua_setfield(L, LUA_REGISTRYINDEX, "__sfo_pointer");

	// Create empty table at core.registered_simulations:
	lua_getglobal(L, "core");
	lua_pushstring(L, "registered_simulations");
	lua_newtable(L);
	lua_settable(L, -3);

	for (unsigned int i=0; i<world->simulation_units.size(); i++)
	{
		std::string registry_name = "__"
			+ world->simulation_units.at(i)->get_name()
			+ "_pointer";
		if ( registry_name != "__undefined_pointer" )
		{
			lua_pushlightuserdata(L, this);
			lua_setfield(L, LUA_REGISTRYINDEX, registry_name.c_str());

			std::cout << "LUA INIT: set " << registry_name << std::endl;
		}
	}

	// Run the loaded file:
	if ( luaL_dofile(L, filename.c_str()) )
	{
		std::string error_message = lua_tostring(L, -1);
		//lua_close(L);
		//L = nullptr;
		//TODO: close off when encountering an error!
		throw sfo_parsing_exception(error_message);
		return;
	}

	// Get global setting variables to set up the scene:
	{
		lua_getglobal(L, "WORLD_DIMENSION");
		if (lua_isstring(L, -1))
		{
			std::string setting_var = lua_tostring(L, -1);
			if (setting_var == "2" || setting_var == "2D")
			{
				message_debug("LUA: Set the world to 2D.");
			} else if (setting_var == "3" || setting_var == "3D")
			{
				message_debug("LUA: Set the world to 3D.");
			} else // Invalid value:
			{
				message_debug("LUA: Erroneous world value.");
			}
		}
		lua_pop(L, 1);
	}
}

simulation_file_object::~simulation_file_object()
{
	if(L) lua_close(L);
//dtor
}

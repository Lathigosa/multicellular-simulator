#include "cell_lib.h"

#include <iostream>

#include "resources/strings.h"

#include "core/simulation_file_object.h"
#include "simulator/unit_cell.h"
#include "utilities/safe_pointer.h"
#include "simulator/simulator_map.h"
#include "renderer/renderer_map.h"

namespace lua
{

namespace sim_cell
{
	/// @param 1 this (table)
	/// @param 2 reaction definitions (string)
	int add_particles (lua_State* L)
	{
		// See if the number of arguments is correct:
		int argc = lua_gettop(L);

		if(argc < 1)
			return luaL_error(L, res::str::lua::error_missing_arguments);
		
		// Get the reference to the sim_intracellular_reactions object:
		std::string lua_ref_name = "pointer";
		lua_getfield(L, 1, lua_ref_name.c_str());
		simulation::cell_global* ref =
			reinterpret_cast<simulation::cell_global*>(lua_touserdata(L, -1));
		lua_pop(L, 1);

		if (ref == nullptr)
		{
			return luaL_error(L, res::str::lua::error_function_undefined_simulator, "sim_cell");
		}

		// Get the definition string from the second argument:
		unsigned int count = lua_tointeger(L, 2);
		
		ref->add_particles(count);
		
		return 0;
	}

	const luaL_Reg functions[] =
	{
		{"add_particles", add_particles},
		{nullptr, nullptr}
	};
}
}

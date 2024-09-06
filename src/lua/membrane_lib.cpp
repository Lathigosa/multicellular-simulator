#include "membrane_lib.h"

#include <iostream>

#include "resources/strings.h"

#include "core/simulation_file_object.h"
#include "simulator/membrane.h"
#include "utilities/safe_pointer.h"
#include "simulator/simulator_map.h"
#include "renderer/renderer_map.h"

#if !defined LUA_VERSION_NUM
/* Lua 5.0 */
#define luaL_Reg luaL_reg
#endif

namespace lua
{

namespace sim_membrane
{
	/// @param 1 name of the registered cell
	/// @param 2 definition table of the cell
	int spawn_cell (lua_State* L)
	{
		// See if the number of arguments is correct:
		int argc = lua_gettop(L);

		if(argc < 1)
			return luaL_error(L, res::str::lua::error_missing_arguments);
		
		// Get the reference to the sim_membrane object:
		std::string lua_ref_name = "pointer";
		lua_getfield(L, 1, lua_ref_name.c_str());
		simulation::membrane* ref =
			reinterpret_cast<simulation::membrane*>(lua_touserdata(L, -1));
		lua_pop(L, 1);

		if (ref == nullptr)
		{
			return luaL_error(L, res::str::lua::error_function_undefined_simulator, "sim_simple_cell_physics");
		}

		cl_float4 positions[argc - 1];

		// Cycle through all the arguments:
		for(int i=2; i<=argc; i++)
		{
			if (!lua_istable(L, i))
				return luaL_error(L, res::str::lua::error_arg_not_3d_vector);

			lua_getfield(L, i, "x");
			lua_getfield(L, i, "y");
			lua_getfield(L, i, "z");

			if (!lua_isnumber(L, -1) || !lua_isnumber(L, -2) || !lua_isnumber(L, -3))
				return luaL_error(L, res::str::lua::error_arg_not_3d_vector);

			positions[i-1].s[0] = lua_tonumber(L, -3);
			positions[i-1].s[1] = lua_tonumber(L, -2);
			positions[i-1].s[2] = lua_tonumber(L, -1);
			positions[i-1].s[3] = 1.0f;
		}
		
		ref->spawn_cells(argc, positions);
		
		return 0;
	}

	const luaL_Reg functions[] =
	{
		{"spawn_cell", spawn_cell},
		{nullptr, nullptr}
	};
}
}

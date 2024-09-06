#include "core/lua_functions.h"

#include <iostream>
#include "resources/strings.h"

#include "core/simulation_file_object.h"
#include "simulator/membrane.h"
#include "simulator/unit_cell.h"
#include "utilities/safe_pointer.h"
#include "simulator/simulator_map.h"

namespace lua
{
namespace sim_cell
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

	/// @param 1 name of the registered cell
	/// @param 2 {x, y, z} of the cell position
	int place_cell(lua_State* L)
	{
		//if (lua_isstring(L, 1))
		//{
		//	std::string cell_def_name = lua_tostring(L, -1);
		//}

		if (lua_istable(L, 2))
		{
			lua_getfield(L, 2, "x");
			lua_getfield(L, 2, "y");
			lua_getfield(L, 2, "z");
			const double x = luaL_checknumber(L, -3);
			const double y = luaL_checknumber(L, -2);
			const double z = luaL_checknumber(L, -1);

			message_debug("sim_cell.place_cell(" << x << ", " << y << ", " << z << ").");

			lua_pop(L, 3);

			// Get pointer to core class:
			lua_getfield(L, LUA_REGISTRYINDEX, "__sim_cell_pointer");
			simulation::cell_global* sim_cell_ptr =
				reinterpret_cast<simulation::cell_global*>(lua_touserdata(L, -1));
			lua_pop(L, 1);

			// Place cell:



		} else {
			return luaL_argerror(L, 2, res::str::lua::error_arg_not_3d_vector);
		}

		return 0;
	}

	/// @param 1 name of the registered cell
	/// @param 2 {x, y, z} of the bounding box start point
	/// @param 3 {x, y, z} of the bounding box end point
	/// @param 4 {x, y, z} of the cell count distributed over the respective dimensions
	int place_cell_cube(lua_State* L)
	{

	}

	//const luaL_reg functions[] =
	//{
	//	{"register_cell", register_cell},
	//	{"place_cell", place_cell},
	//	{nullptr, nullptr}
	//};
}
}

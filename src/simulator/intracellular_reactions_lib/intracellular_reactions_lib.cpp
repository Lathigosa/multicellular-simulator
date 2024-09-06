#include "intracellular_reactions_lib.h"

#include <iostream>

#include "resources/strings.h"

#include "core/simulation_file_object.h"
#include "simulator/intracellular_reactions.h"
#include "utilities/safe_pointer.h"
#include "simulator/simulator_map.h"
#include "renderer/renderer_map.h"

#if !defined LUA_VERSION_NUM
/* Lua 5.0 */
#define luaL_Reg luaL_reg
#endif

namespace lua
{

namespace sim_intracellular_reactions
{
	/// @param 1 this (table)
	/// @param 2 reaction definitions (string)
	int set_reactions (lua_State* L)
	{
		// See if the number of arguments is correct:
		int argc = lua_gettop(L);

		if(argc < 1)
			return luaL_error(L, res::str::lua::error_missing_arguments);
		
		// Get the reference to the sim_intracellular_reactions object:
		std::string lua_ref_name = "pointer";
		lua_getfield(L, 1, lua_ref_name.c_str());
		simulation::intracellular_reactions* ref =
			reinterpret_cast<simulation::intracellular_reactions*>(lua_touserdata(L, -1));
		lua_pop(L, 1);

		if (ref == nullptr)
		{
			return luaL_error(L, res::str::lua::error_function_undefined_simulator, "sim_intracellular_reactions");
		}

		// Get the definition string from the second argument:
		std::string reaction_definitions = lua_tostring(L, 2);
		
		ref->set_reactions(reaction_definitions);
		
		return 0;
	}

	const luaL_Reg functions[] =
	{
		{"set_reactions", set_reactions},
		{nullptr, nullptr}
	};
}
}

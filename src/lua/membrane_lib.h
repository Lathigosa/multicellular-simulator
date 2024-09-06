#ifndef MEMBRANE_LIB_H_INCLUDED
#define MEMBRANE_LIB_H_INCLUDED

#include "simulator/membrane.h"

#if !defined LUA_VERSION_NUM
/* Lua 5.0 */
#define luaL_Reg luaL_reg
#endif

namespace lua
{
namespace sim_membrane
{
	extern const luaL_Reg functions[];
}
}

#endif // MEMBRANE_LIB_H_INCLUDED

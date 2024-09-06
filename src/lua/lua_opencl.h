#ifndef LUA_OPENCL_H_INCLUDED
#define LUA_OPENCL_H_INCLUDED

#include "core/lua_functions.h"

#if !defined LUA_VERSION_NUM
/* Lua 5.0 */
#define luaL_Reg luaL_reg
#endif

namespace lua
{
namespace opencl
{
	extern const luaL_Reg functions[];
}
}

#endif // LUA_OPENCL_H_INCLUDED

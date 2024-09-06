#include "lua/lua_opencl.h"

#include <iostream>
#include <CL/cl2.hpp>
#include <resources/strings.h>

#if !defined LUA_VERSION_NUM
/* Lua 5.0 */
#define luaL_Reg luaL_reg
#endif

namespace lua
{

namespace opencl
{
	// kernels:
	int kernel_getInfo(lua_State* L)
	{
		
		return 0;
	}

	const luaL_Reg functions[] =
	{
		{"kernel_getInfo", kernel_getInfo},
		{nullptr, nullptr}
	};
}
}

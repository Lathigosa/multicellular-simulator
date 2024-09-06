#ifndef LUA_FUNCTIONS_H_INCLUDED
#define LUA_FUNCTIONS_H_INCLUDED

#include <luajit-2.1/lua.hpp>

#define message_lua(message) std::cout << "LUA: " << message << std::endl;

#define expose_namespace_functions(name)	namespace name \
											{	\
												extern const luaL_reg functions[];	\
											}

#if !defined LUA_VERSION_NUM
/* Lua 5.0 */
#define luaL_Reg luaL_reg
#endif

namespace lua
{
namespace core
{
	extern const luaL_Reg functions[];
}

namespace sim_cell
{
	extern const luaL_Reg functions[];
}
}

#endif // LUA_FUNCTIONS_H_INCLUDED

local C = require("opencl.C")
local ffi = require("ffi")

ffi.cdef[[
	cl_platform_id lua_cl_get_platform_id(void* pointer);
	cl_device_id lua_cl_get_device_id(void* pointer);
	cl_context lua_cl_get_context(void* pointer);
	cl_command_queue lua_cl_get_command_queue(void* pointer);
]]

core.mt_simulation = {
	on_create = function(self)
		self.platform = self:get_platform()
		self.device = self:get_device()
		self.context = self:get_context()
		self.command_queue = self:get_command_queue()
	end,
	test = "Test succeeded!",
	
	get_platform = function(self)
		return ffi.C.lua_cl_get_platform_id(self.__sim_pointer);
	end,
	
	get_device = function(self)
		return ffi.C.lua_cl_get_device_id(self.__sim_pointer);
	end,
	
	get_context = function(self)
		return ffi.C.lua_cl_get_context(self.__sim_pointer);
	end,
	
	get_command_queue = function(self)
		return ffi.C.lua_cl_get_command_queue(self.__sim_pointer);
	end
}

core.mt_simulation.__index = core.mt_simulation

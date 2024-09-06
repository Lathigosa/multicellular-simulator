--core.use_simulation("sim_membrane")

local ffi = require 'ffi'

print(package.path)

local cl = require("opencl")

local platforms = cl.get_platforms();

print(platforms[1]:get_info("profile"))

core.register_simulation("cell_hookian_repel", {
	dependencies = {},
	--dependencies = "sim_cell",
	
	particle_count = 32*32,
	max_buffer_size = 32*32*16,
	
	double_buffer = false,
	
	on_init = function (self)
		file = io.open("./cl_kernels/test_empty.cl")
		--file = io.open("./cl_kernels/cell_hookian_repel.cl")
		local content = file:read("*a")
		file:close()
		print(content)
		local program = self.context:create_program_with_source(content)
		program:build()
		self.kernel = program:create_kernel("cell_hookian_repel")
		--self.kernel = program:create_kernel("return_constant")
		
		self.position1 = self.context:create_buffer("read_write", ffi.sizeof("cl_float4")*self.max_buffer_size)
		self.velocity1 = self.context:create_buffer("read_write", ffi.sizeof("cl_float4")*self.max_buffer_size)
		self.position2 = self.context:create_buffer("read_write", ffi.sizeof("cl_float4")*self.max_buffer_size)
		self.velocity2 = self.context:create_buffer("read_write", ffi.sizeof("cl_float4")*self.max_buffer_size)
		
		--self.cl_particle_count = self.context:create_buffer("read_write", ffi.sizeof("cl_uint"))
		--self.cl_timestep = self.context:create_buffer("read_write", ffi.sizeof("cl_float"))
		
		self.cl_particle_count = ffi.new("cl_float[1]")
		self.cl_particle_count[0] = ffi.cast("cl_float", self.particle_count)
	end,
	on_simulate = function (self)
		--self.kernel:set_arg(0, self.position1)
		if self.double_buffer == false then
			self.kernel:set_arg(0, self.position1)
			self.kernel:set_arg(1, self.velocity1)
			self.kernel:set_arg(2, self.position2)
			self.kernel:set_arg(3, self.velocity2)
			self.double_buffer = true
		else
			self.kernel:set_arg(0, self.position2)
			self.kernel:set_arg(1, self.velocity2)
			self.kernel:set_arg(2, self.position1)
			self.kernel:set_arg(3, self.velocity1)
			self.double_buffer = false
		end
		-- TODO: crash the application using the next line and HANDLE THE EXCEPTION PROPERLY:
		--self.kernel:set_arg(4, self.cl_particle_count)
		--local step_size = ffi.cast("cl_float", 0.1)
		
		local step_size = ffi.new("cl_float[1]")
		step_size[0] = ffi.cast("cl_float", 0.1)
		--self.kernel:set_arg(4, ffi.sizeof("cl_int"), self.cl_particle_count)
		--self.kernel:set_arg(5, ffi.sizeof("cl_float"), step_size)
		--self.kernel:set_arg(4, self.cl_particle_count)
		--self.kernel:set_arg(5, self.cl_timestep)
		
		local particle_count = self.particle_count
		local wg_size = 32
		--]]
		--wg_size*math.ceil(particle_count/wg_size)
		
		local dataset_size = ffi.new("size_t[1]")
		dataset_size[0] = ffi.cast("size_t", 32)
		
		local return_value = self.command_queue:enqueue_ndrange_kernel(self.kernel, nil, {32*32}, {32})
		--print("return_value: "..return_value)
		--self.command_queue:finish()
		print("something")
	end
})

--core.use_simulation("cell_hookian_repel")

local particle_system = core.create_particle_system()

local simulator_chemistry = particle_system:add_simulation("sim_intracellular_reactions")
local simulator_division = particle_system:add_simulation("sim_division_after_concentration_treshold", {sim_intracellular_reactions = simulator_chemistry})

local simulator_division_axis = particle_system:add_simulation("sim_division_plane")
local simulator_physics = particle_system:add_simulation("sim_simple_cell_physics", {sim_division_plane = simulator_division_axis})
local simulator_death = particle_system:add_simulation("sim_cell_death_barrier", {sim_simple_cell_physics = simulator_physics})
local simulator_polarity = particle_system:add_simulation("sim_cell_polarity", {sim_division_plane = simulator_division_axis})



--local initial_cell = particle_system:add_particle()
--initial_cell.position = {x=2, y=2, z=2}

--print(initial_cell.test)

--core.mt_particle = {
--	test = "modified"
--}

--print(initial_cell.test)

--particle_system:add_particle()
--particle_system:add_particle()
--particle_system:add_particle()
--particle_system:add_particle()




--simulator_physics:set_max_dimension_sizes({particle_count = 32*32*32, grid_x_size = 32})

--local particle_system2 = core.create_particle_system()
--local simulator_physics2 = particle_system2:add_simulation("sim_simple_cell_physics")

--[[local simulator_cell = core.use_simulation("sim_cell")
--core.use_simulation("cell_hookian_repel")
local simulator_physics = core.use_simulation("sim_simple_cell_physics", {sim_cell = simulator_cell})
local simulator_chemistry = core.use_simulation("sim_intracellular_reactions", {sim_cell = simulator_cell})
--]]

--TODO: fix bug where there is a SEGMENTATION FAULT when running this code:
--TODO: suspicion: simulator_chemistry does not have "velocity", so it is accessing something wrong.
--core.use_renderer("cells_velocity", {position = {simulator_physics, "position"},
--									--velocity = {simulator_physics, "velocity"},
--									velocity = {simulator_chemistry, "velocity"}
--									})


--core.use_renderer("cells_velocity", {position = {simulator_physics, "position"},
--									--velocity = {simulator_physics, "velocity"},
--									velocity = {simulator_physics, "velocity"}
--									})
core.use_renderer("cells_as_dots", {position = {simulator_physics, "position"} --{simulator_physics, "position"}
									})
									
core.use_renderer("polar_spheres", {position = {simulator_physics, "position"}, --{simulator_physics, "position"}
									velocity = {simulator_polarity, "concentrations"},
									division_plane = {simulator_division_axis, "division_plane"}
									})
									
--core.use_renderer("cells_as_dots", {position = {simulator_chemistry, "concentrations"}
--									})

local reaction_scheme = "A -> B, 100, 100\n"..
	"A + B -> C + D, 100, 100\n"..
	"C -> , 100, 100\n"..
	"D -> , 100, 100\n"..
	"-> A, 100, 100"
	
local reaction_scheme_2 = "-> A\n"..
	"-> B\n-> B\n"..
	"A + B -> \n"
	
local reaction_scheme_3 = "-> A\n-> A\n-> A\n"..
	"-> B\n-> B\n-> B\n-> B\n-> B\n-> B\n"..
	"A + B -> \nA + B -> \nA + B -> \n"..
	"A + B + C ->  \nA + B + C ->  \nA + B + C ->  \n"..
	"-> C\n"
	
local reaction_scheme_4 = "A + X -> 2X, 4\n"..
	"X + Y -> 2Y, 40\n"..
	"Y -> B, 1000\n"..
	"A -> 2A, 100\n"..
	"B -> , 1000\n"
	--"-> A\n"..
	--"B -> \n"..
	--"B -> \n"
	
local reaction_scheme_4_slow = "A + X -> 2X, 0.4\n"..
	"X + Y -> 2Y, 4\n"..
	"Y -> B, 100\n"..
	"A -> 2A, 10\n"..
	"B -> , 100\n"

--for i=0, 20 do
--	reaction_scheme_2 = reaction_scheme_2 .. " -> A\n"
--end


simulator_chemistry:set_reactions(reaction_scheme_4)

--cells.add()
--[[
cells:spawn({
	x = 0,
	y = 1,
	z = 2,
	chemical1 = 1.0,
	orientation_x = 1.0,
	orientation_y = 1.0,
	orientation_z = 1.0,
})

--]]
									
--print("sim_cell: "..simulator_cell.name)
--print("sim_physics: "..simulator_physics.name)

local i
for i=0, 100, 8 do
	--simulator_physics2:spawn_cell({x=-200.0, y=100.0, z=i})
end

--simulator_physics:spawn_cell({x=-20.0, y=20.0, z=0.0})

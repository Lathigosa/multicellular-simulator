core.mt_particle = {
	test = "UNMODIFIED"
}

core.mt_particle.__index = core.mt_particle

core.mt_particle_system = {
	on_create = function(self)
		self.platform = self:get_platform()
		self.device = self:get_device()
		self.context = self:get_context()
		self.command_queue = self:get_command_queue()
	end,
	
	add_simulation = function(self, simulation_name, simulation_options)
		simulation_options = simulation_options or {}
		simulation_options.sim_cell = simulation_options.sim_cell or self.simulator_division
		local new_simulation = core.use_simulation(simulation_name, simulation_options)
		self.simulation_reference_list[#self.simulation_reference_list + 1] = new_simulation
		return new_simulation
	end,
	
	add_particle = function(self)
		local new_particle = {}
		
		self.particle_count = self.particle_count + 1
		
		new_particle.particle_system = self
		new_particle.index = self.particle_count
		
		setmetatable(new_particle, core.mt_particle)
		
		self.simulator_division:add_particles(1)
		
		return new_particle
	end
}

core.mt_particle_system.__index = core.mt_particle_system

--===================================================================--

core.create_particle_system = function()
	
	local new_particle_system = {}
	
	new_particle_system.simulator_division = core.use_simulation("sim_cell")
	new_particle_system.buffer_reference_list = {}
	new_particle_system.simulation_reference_list = {}
	new_particle_system.particle_count = 0
	
	setmetatable(new_particle_system, core.mt_particle_system)
	
	return new_particle_system
end

--core.link_simulation_to_particle_systems = function(simulation_name, )

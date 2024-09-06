// In contrast to sim_cell_division1.cl, this kernel consolidates the data generated by sim_cell_division1.cl.

// !! Makes use of "sort_particles_in_3d_grid.cl" !! //

kernel void cell_hookian_repel(global float4* out_position,
										global float4* out_velocity,
										global const float4* in_position,
										global const float4* in_velocity,
										global const uint* in_grid,				// Used for optimization.
										global const uint* in_grid_counter,		// Used for optimization.
										private int count,
										private float timestep)	// TODO: possibly replace "float4* normal" with "half* normal" and use vload_half4(index, normal) to access it.
{
	float3 my_position = in_position[get_global_id(0)].xyz;
	//"float3 my_velocity = in_velocity[get_global_id(0)].xyz;"
	float3 my_acceleration = (float3)(0.0f, 0.0f, 0.0f);

	float3 my_normal = (float3)(0.0f, 0.0f, 1.0f);		// TODO: get normal
	float my_size = in_position[get_global_id(0)].w;
	
	// Find surrounding cells:
	int3 grid_coords = get_grid_coords(my_position);		// Defined in "sort_particles_in_3d_grid.cl".
	uint grid_id = get_grid_id_from_coords(grid_coords); 	// Defined in "sort_particles_in_3d_grid.cl".
	
	//int3 side = get_grid_side(my_position);
	//side = (int3)(0,0,0);
	
	// Iterate through surrounding voxels (including own) to find cells:
	for(int x=-1; x<=1; x++)
	for(int y=-1; y<=1; y++)
	for(int z=-1; z<=1; z++)
	{
		uint grid_id = get_grid_id_from_coords((int3)(grid_coords.x + x, grid_coords.y + y, grid_coords.z + z)); 	// Defined in "sort_particles_in_3d_grid.cl".
		
		// TODO: base next loop on in_grid_counter:
		for(int i=0; i<in_grid_counter[grid_id]; i++)		// MAX_CELLS_PER_VOXEL defined in "sort_particles_in_3d_grid.cl".
		{
			int a = in_grid[grid_id * MAX_CELLS_PER_VOXEL + i];
			
			float3 your_position = in_position[a].xyz;
			float dist = distance(my_position, your_position);

			// TODO: remove this if-statement:
			//if(dist <= 1.0f) dist = 1.0f;

			// Normalize:
			float3 force_direction = (my_position - your_position) / dist;

			// Lennard-Jones Potential:
			//float force_magnitude = 24.0f * (0.5f) / dist * (2.0f*pown((in_position[a].w + my_size) / dist, 12) - pown((in_position[a].w + my_size) / dist, 6));
			
			// Hookian Repel:
			float force_magnitude = max(0.0f, (in_position[a].w + my_size - dist) * 1.0f);	//FOR PERFORMANCE TESTING
			//float force_magnitude = max(0.0f, pown((in_position[a].w + my_size) / dist, 12) * 0.0001f);	//FOR PERFORMANCE TESTING PURPOSES, SET MULTIPLIER TO 100.0f!

			float3 force = force_magnitude * force_direction;

			// Discard values that are NAN (for test purposes):
			if (!isnan(fast_length(force)) && !isinf(fast_length(force)))

			my_acceleration = my_acceleration + force;	// TODO: add mass to particles ( / (3.1415926f * 4.0f / 3.0f * my_size * my_size * my_size)).
		}
	}

	{
		float plane_length = my_position.z;
		float plane_force = min(0.0f, (plane_length - 10.0f)*10.0f);
		float3 plane_direction = (float3)(0.0f, 0.0f, -1.0f);
		
		if (!isnan(plane_length) && plane_length != 0.0f)
		
		my_acceleration = my_acceleration + plane_force * plane_direction;
	}
	
	/*{
		float sphere_length = length(my_position);
		float sphere_force = min(0.0f, (sphere_length - 100.0f)*10.0f);
		float3 sphere_direction = -my_position / sphere_length;
		
		if (!isnan(sphere_length) && sphere_length != 0.0f)
		
		my_acceleration = my_acceleration + sphere_force * sphere_direction;
	}*/

	// Calculate velocity (based on timestep):
	//my_acceleration = my_acceleration - in_velocity[get_global_id(0)].xyz * 0.1f;
	//float3 my_velocity = in_velocity[get_global_id(0)].xyz + (timestep * my_acceleration);
	float3 my_velocity = (timestep * my_acceleration);
	my_position = my_position + (timestep * my_velocity);

	// Upload the values:
	out_velocity[get_global_id(0)] = (float4) (my_velocity, 1.0f); // Velocity Damping
	out_position[get_global_id(0)] = (float4) (my_position, min(my_size + 0.006f * timestep, 5.0f));
	//"out_position[get_global_id(0)] = (float4) (0.0f, 1.0f, 2.0f, 1.0f);"
}

// Bla

kernel void delete_particles_float4(	global float4* out_buffer,
							const global float4* in_buffer,
							const global uint* in_deleted_particles,
							uint total_count,
							uint deleted_particle_count)
{
	uint id = total_count - get_global_id(0) - 1;
	out_buffer[in_deleted_particles[get_global_id(0)]] = (float4)(in_buffer[id]);
};

// This kernel performs step 2: sorting the "deleted_particle" index list to its right format:
kernel void sort_deleted_list(global uint* in_deleted_particles,	// Unsorted list
								global uint* out_deleted_particles,	// Sorted list
								uint total_count,
								uint deleted_particle_count)
{
	uint id = get_global_id(0);
	while(true)
	{
		// Only if the index is outside of the new list size and would be cut off:
		uint current_index = in_deleted_particles[id];
		uint reversed_index = total_count - current_index;
		if(reversed_index <= deleted_particle_count)
		{
			// Stage 1: copy index from 
		
			out_deleted_particles[get_global_id(0)] = in_deleted_particles[reversed_index - 1];
			
			id = reversed_index - 1;
		} else
			break;
	}
	
	barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
	
	// Only if the index is outside of the new list size and would be cut off:
	uint current_index = in_deleted_particles[get_global_id(0)];
	uint reversed_index = total_count - current_index;
	if(reversed_index <= deleted_particle_count)
	{
		// Stage 1: copy index from 
		
		//out_deleted_particles[get_global_id(0)] = in_deleted_particles[reversed_index - 1];
		
		// Stage 2:
		out_deleted_particles[reversed_index - 1] = current_index;
	} else
	{
		out_deleted_particles[get_global_id(0)] = in_deleted_particles[get_global_id(0)];
	}
}

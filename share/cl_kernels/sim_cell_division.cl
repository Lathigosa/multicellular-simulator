// Bla


uint random(ulong randoms, uint globalID)
{
	//uint seed = randoms.x + globalID;
	//uint t = seed ^ (seed << 11);
	//return randoms.y ^ (randoms.y >> 19) ^ (t ^ (t >> 8));

	ulong seed = randoms + globalID;
	seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
	return seed >> 16;
};

bool random_bool(uint seed)
{
	return (random(random(random(seed + 5456378764458675, random(seed + 43266543, get_global_id(0))), random(seed + 677568659764, get_global_id(0))), random(random(seed + 979875332664, get_global_id(0)), get_global_id(0))) < 1000000);
}

void queue_division(local uint* local_division_queue, local uint* queue_counter)
{
	// Allocate some memory to indicate division:
	//barrier(CLK_LOCAL_MEM_FENCE);
	uint counter = atomic_inc(&queue_counter[0]);
	local_division_queue[counter] = get_global_id(0);
};

#define GROUP_SIZE 256

kernel void membrane_simulate_particles(//global uint* in_should_divide,
										global uint* out_new_cells,
										global uint* out_new_cell_count,
										//global const float4* in_position,
										//global const float4* in_velocity,
										//global const float4* in_normal,		// Indicates the rotation of the particle as well.
										//global uint* out_division_queue,
										int count,
										uint seed)
										//float timestep)	// TODO: possibly replace "float4* normal" with "half* normal" and use vload_half4(index, normal) to access it.
{
	local uint local_division_queue[256];		// TODO: Determine proper size.
	local uint queue_counter;					// The counter keeping track of where to put the next index.

	// Initialize local_division_queue counter:
	if(get_local_id(0)==0)
		queue_counter = 0;
	
	barrier(CLK_LOCAL_MEM_FENCE);

	if(random_bool(seed))
		queue_division(local_division_queue, &queue_counter);
	
	barrier(CLK_LOCAL_MEM_FENCE);
	
	
	
	out_new_cells[get_global_id(0)] = local_division_queue[get_local_id(0)];
	
	if(get_local_id(0)<=get_group_id(0))
		out_new_cell_count[get_group_id(0) + get_num_groups(0)*get_local_id(0)] = queue_counter;
};

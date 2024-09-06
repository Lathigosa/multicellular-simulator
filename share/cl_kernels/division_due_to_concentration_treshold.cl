// Queues division for every cell that has a chemical above a certain treshold level.

void queue_division(local uint* local_division_queue, local uint* queue_counter)
{
	// Allocate some memory to indicate division:
	//barrier(CLK_LOCAL_MEM_FENCE);
	uint counter = atomic_inc(&queue_counter[0]);
	local_division_queue[counter] = get_global_id(0);
};

#define GROUP_SIZE 256

#define MOLECULAR_SPECIES_COUNT 4
#define C get_global_id(0) * MOLECULAR_SPECIES_COUNT + 2

kernel void simulate_division(
										global uint* out_new_cells,
										global uint* out_new_cell_count,
										unsigned int count,
										global const float* in_concentrations)
{
	if(get_global_id(0) >= count)
		return;
	
	local uint local_division_queue[256];		// TODO: Determine proper size.
	local uint queue_counter;					// The counter keeping track of where to put the next index.

	// Initialize local_division_queue counter:
	if(get_local_id(0)==0)
		queue_counter = 0;
	
	barrier(CLK_LOCAL_MEM_FENCE);

	if(in_concentrations[C] > 70.0f)
		queue_division(local_division_queue, &queue_counter);
	
	barrier(CLK_LOCAL_MEM_FENCE);
	
	
	
	out_new_cells[get_global_id(0)] = local_division_queue[get_local_id(0)];
	
	if(get_local_id(0) == 0)
		out_new_cell_count[get_group_id(0)] = queue_counter;
	//if(get_local_id(0)<=get_group_id(0))
	//	out_new_cell_count[get_group_id(0) + get_num_groups(0)*get_local_id(0)] = queue_counter;
};

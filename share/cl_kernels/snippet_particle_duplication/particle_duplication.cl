// 
#include <random>

void queue_division(local uint* local_division_queue, local uint* queue_counter)
{
	// Allocate some memory to indicate division:
	uint counter = atomic_inc(&queue_counter[0]);
	local_division_queue[counter] = get_global_id(0);
}

#define GROUP_SIZE 256

#define MOLECULAR_SPECIES_COUNT 4
#define C get_global_id(0) * MOLECULAR_SPECIES_COUNT + 2

#define MARK_DUPLICATE queue_division(local_division_queue, &queue_counter)

kernel void mark_particles(
								global uint* out_new_cells,
								global uint* out_new_cell_count,
								unsigned int count,
								uint2 seed)
								//global const float* in_concentrations)
{
	if(get_global_id(0) >= count)
		return;
	
	local uint local_division_queue[256];		// TODO: Determine proper size.
	local uint queue_counter;					// The counter keeping track of where to put the next index.

	// Initialize local_division_queue counter:
	if(get_local_id(0) == 0)
		queue_counter = 0;
	
	// Perform the user-defined division code, which calls queue_division iff the current index must duplicate:
	barrier(CLK_LOCAL_MEM_FENCE);
	
	{
		<user_code>
	} //0.1%
	
	barrier(CLK_LOCAL_MEM_FENCE);
	
	// Upload the local memory buffer to global memory:
	out_new_cells[get_global_id(0)] = local_division_queue[get_local_id(0)];
	
	if(get_local_id(0) == 0)
		out_new_cell_count[get_group_id(0)] = queue_counter;
}

// 
#include <random>

void queue_division(local uint* local_division_queue, local uint* queue_counter)
{
	// Allocate some memory to indicate division:
	uint counter = atomic_inc(&queue_counter[0]);
	local_division_queue[counter] = get_global_id(0);
}

void queue_deletion(local uint* local_deletion_queue, local uint* queue_counter)
{
	// Allocate some memory to indicate division:
	//barrier(CLK_LOCAL_MEM_FENCE);
	uint counter = atomic_inc(&queue_counter[0]);
	local_deletion_queue[counter] = get_global_id(0);
};

#define GROUP_SIZE 256

#define MOLECULAR_SPECIES_COUNT 4
#define C get_global_id(0) * MOLECULAR_SPECIES_COUNT + 2

#define MARK_DUPLICATE queue_division(local_division_queue, &division_queue_counter)
#define MARK_DELETE queue_deletion(local_deletion_queue, &deletion_queue_counter)

kernel void mark_particles(
	global uint* out_new_cells,
	global uint* out_new_cell_count,
	unsigned int count,
	uint2 seed
)
{
	if(get_global_id(0) >= count)
		return;
	
	local uint local_division_queue[256];		// TODO: Determine proper size.
	local uint division_queue_counter;			// The counter keeping track of where to put the next index.

	local uint local_deletion_queue[256];		// TODO: Determine proper size.
	local uint deletion_queue_counter;			// The counter keeping track of where to put the next index.

	// Initialize local_division_queue counter:
	if(get_local_id(0) == 0)
	{
		division_queue_counter = 0;
		deletion_queue_counter = 0;
	}
	
	// Perform the user-defined division code, which calls queue_division iff the current index must duplicate:
	barrier(CLK_LOCAL_MEM_FENCE);
	
	{
		<user_code>
	} 
	
	barrier(CLK_LOCAL_MEM_FENCE);
	
	// Upload the local memory buffer to global memory:
	out_new_cells[get_global_id(0)] = local_division_queue[get_local_id(0)];
	
	if(get_local_id(0) == 0)
		out_new_cell_count[get_group_id(0)] = division_queue_counter;
}

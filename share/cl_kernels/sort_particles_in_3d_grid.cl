// This kernel sorts particles in a 3d grid data structure for faster collision detection.

#define GRID_X_SIZE 64
#define GRID_Y_SIZE 64
#define GRID_Z_SIZE 64

#define VOXEL_X_SIZE 10.0f
#define VOXEL_Y_SIZE 10.0f
#define VOXEL_Z_SIZE 10.0f

#define MAX_CELLS_PER_VOXEL 16

int3 get_grid_coords(float3 position)
{
	int3 i = (int3)(position.x / VOXEL_X_SIZE + GRID_X_SIZE / 2,
					position.y / VOXEL_Y_SIZE + GRID_Y_SIZE / 2,
					position.z / VOXEL_Z_SIZE + GRID_Z_SIZE / 2);
					
	i.x = min(max(i.x, 0), GRID_X_SIZE - 1);
	i.y = min(max(i.y, 0), GRID_Y_SIZE - 1);
	i.z = min(max(i.z, 0), GRID_Z_SIZE - 1);
	
	return i;
}

// Gets whether the point is in the positive (1) or negative (0) side of a voxel for each axis:
int3 get_grid_side(float3 position)
{
	int3 i = (int3)(position.x / (VOXEL_X_SIZE / 2) + GRID_X_SIZE,
			 		position.y / (VOXEL_Y_SIZE / 2) + GRID_Y_SIZE,
			 		position.z / (VOXEL_Z_SIZE / 2) + GRID_Z_SIZE) & (int3)(0x01, 0x01, 0x01);
	
	return i;
}

uint get_grid_id(float3 position)
{
	int3 i = (int3)(position.x / VOXEL_X_SIZE + GRID_X_SIZE / 2,
					position.y / VOXEL_Y_SIZE + GRID_Y_SIZE / 2,
					position.z / VOXEL_Z_SIZE + GRID_Z_SIZE / 2);
					
	i.x = min(max(i.x, 0), GRID_X_SIZE - 1);
	i.y = min(max(i.y, 0), GRID_Y_SIZE - 1);
	i.z = min(max(i.z, 0), GRID_Z_SIZE - 1);
	
	return i.x + i.y * GRID_X_SIZE + i.z * GRID_Y_SIZE * GRID_X_SIZE;
}

uint get_grid_id_from_coords(int3 grid_coords)
{
	grid_coords.x = min(max(grid_coords.x, 0), GRID_X_SIZE - 1);
	grid_coords.y = min(max(grid_coords.y, 0), GRID_Y_SIZE - 1);
	grid_coords.z = min(max(grid_coords.z, 0), GRID_Z_SIZE - 1);
	
	return grid_coords.x + grid_coords.y * GRID_X_SIZE + grid_coords.z * GRID_Y_SIZE * GRID_X_SIZE;
}

// out_grid should have MAX_CELLS_PER_VOXEL uints per voxel. Each uint stores a cell index that resides in the voxel.
// temp_grid_counter should have 1 uint per voxel. It is used to count the number of cells in one voxel, so that the proper index of the voxel uints can be accessed.

kernel void spatial_particle_sort_3d(	global uint* out_grid,
										global uint* temp_grid_counter,
										global const float4* in_position,
										private uint count)
{
	if(get_global_id(0) >= count)
		return;
	
	//TODO: do not include unused particles (due to NDRANGE size being in increments)!
	float3 p = in_position[get_global_id(0)].xyz;
	
	uint grid_id = get_grid_id(p);
	
	// TODO: investigate the removal of the next if-statement (queue.flush fails with error -9999 without it for n>131072):
	if(temp_grid_counter[grid_id] < 16)
	{
	
		uint index = atomic_inc(&temp_grid_counter[grid_id]);	// Get first empty index and increase counter of selected voxel.
		//uint index = 0;
		if(index < 16)
			out_grid[grid_id * MAX_CELLS_PER_VOXEL + index] = get_global_id(0);		// Store index of cell in the empty spot of the voxel.
	}
	//TODO: handle cases of overflow!
}

//TODO: test implementation in which only changes to the positions are accounted for to decrease memory access!
// Possible implementation 1: coding it in the physics kernel (not extendible, unless kernel is dynamically coded);
// Possible implementation 2: using the "velocity" to predict changes (possibly more slow);
// Possible implementation 3: separate kernels, but with one integrator kernel afterwards which also accounts for voxels.

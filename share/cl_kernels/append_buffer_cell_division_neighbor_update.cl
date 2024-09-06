// OpenCL

#include <random>

// Assume for now that there are no simultaneous divisions. TODO: remove this assumption.

kernel void append_buffer(	global uint16* out_buffer_neighbors,
							global uint* in_new_cells,
							int append_index,
							global float4* in_position,
							global uint16* vertex_indices,
							
							global float4* vertices,
							global uint2* edges,
							global uint16* faces)
{
	//TODO: perhaps double buffer to prevent race conditions!!!
	float new_size = out_buffer[in_new_cells[get_global_id(0)]].w * 0.793700526f;	// Multiply by 1/2^(1/3) to preserve volume.
	
	out_buffer[get_global_id(0) + append_index] = 
	
	out_buffer[get_global_id(0) + append_index] = (float4)(out_buffer[in_new_cells[get_global_id(0)]].xyz, new_size) + (float4)(sin_a * cos_b, sin_a * sin_b, cos_a, 0.0f);
	
	out_buffer[in_new_cells[get_global_id(0)]].w = new_size;
};

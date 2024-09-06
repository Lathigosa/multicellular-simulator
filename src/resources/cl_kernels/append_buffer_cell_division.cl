// Bla

kernel void append_buffer(	global float4* out_buffer,
							global uint* in_new_cells,
							int append_index,
							const global float4* in_division_axis)
{
	float4 division_axis = in_division_axis[get_global_id(0)];
	float3 current_position = out_buffer[in_new_cells[get_global_id(0)]].xyz;
	
	//TODO: perhaps double buffer to prevent race conditions!!!
	float new_size = out_buffer[in_new_cells[get_global_id(0)]].w * 0.793700526f;	// Multiply by 1/2^(1/3) to preserve volume.
	
	// Displace the cells in opposite direction:
	out_buffer[get_global_id(0) + append_index] = (float4)(current_position - division_axis.xyz, new_size);
	
	out_buffer[in_new_cells[get_global_id(0)]] = (float4)(current_position + division_axis.xyz, new_size);
};

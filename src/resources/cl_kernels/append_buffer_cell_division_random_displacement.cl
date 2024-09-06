// Bla

kernel void append_buffer(	global float4* out_buffer,
							global uint* in_new_cells,
							int append_index,
							float random)
{
	//TODO: perhaps double buffer to prevent race conditions!!!
	float new_size = out_buffer[in_new_cells[get_global_id(0)]].w * 0.793700526f;	// Multiply by 1/2^(1/3) to preserve volume.
	
	out_buffer[get_global_id(0) + append_index] = (float4)(out_buffer[in_new_cells[get_global_id(0)]].xyz, new_size) + (float4)(1.0f * random * random * random, random * random * 1.0f, random * 1.0f, 0.0f);
	
	out_buffer[in_new_cells[get_global_id(0)]].w = new_size;
};

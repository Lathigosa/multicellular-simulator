// Bla

kernel void append_buffer(	global float4* out_buffer,
							global uint* in_new_cells,
							int append_index)
{
// TODO: perhaps double buffer to prevent race conditions! Is it necessary?
	out_buffer[get_global_id(0) + append_index] = out_buffer[in_new_cells[get_global_id(0)]];
};

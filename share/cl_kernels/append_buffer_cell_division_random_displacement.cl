// Bla

#include <random>

kernel void append_buffer(	global float4* out_buffer,
							global uint* in_new_cells,
							int append_index,
							uint2 seed)
{
	//TODO: perhaps double buffer to prevent race conditions!!!
	float new_size = out_buffer[in_new_cells[get_global_id(0)]].w * 0.793700526f;	// Multiply by 1/2^(1/3) to preserve volume.
	
	float2 angles = generate_random_float(seed, (uint2)(0, 0));
	angles.x = angles.x * M_PI_F;
	angles.y = acos(1.0f - angles.y * 2.0f);
	const float sin_a = sin(angles.x);
	const float cos_a = cos(angles.x);
	const float sin_b = sin(angles.y);
	const float cos_b = cos(angles.y);
	
	out_buffer[get_global_id(0) + append_index] = (float4)(out_buffer[in_new_cells[get_global_id(0)]].xyz, new_size) + (float4)(sin_a * cos_b, sin_a * sin_b, cos_a, 0.0f);
	
	out_buffer[in_new_cells[get_global_id(0)]].w = new_size;
};

// This kernel keeps track of any new membrane vertices, edges or faces as well:
kernel void append_buffer2(	global float4* out_buffer,
							global uint* in_new_cells,
							int append_index,
							uint2 seed,
							global uint16* vertex_indices,
							global uint16* edge_indices,
							
							global float4* vertices,
							global uint2* edges,
							global uint16* faces)
{
	float4 my_position = out_buffer[in_new_cells[get_global_id(0)]];
	
	//TODO: perhaps double buffer to prevent race conditions!!!
	float new_size = my_position.w * 0.793700526f;	// Multiply by 1/2^(1/3) to preserve volume.
	
	float2 angles = generate_random_float(seed, (uint2)(0, 0));
	angles.x = angles.x * M_PI_F;
	angles.y = acos(1.0f - angles.y * 2.0f);
	const float sin_a = sin(angles.x);
	const float cos_a = cos(angles.x);
	const float sin_b = sin(angles.y);
	const float cos_b = cos(angles.y);
	
	float3 division_plane_normal = (float3)(sin_a * cos_b, sin_a * sin_b, cos_a);
	
	out_buffer[get_global_id(0) + append_index] = (float4)(my_position.xyz + division_plane_normal, new_size);
	
	out_buffer[in_new_cells[get_global_id(0)]].w = new_size;
	
	// Depending on the angle, divide up the edges in this cell:
	union
	{
		uint elarray[16];
		uint16 elvector;
		int16 bits;
	} my_edge_indices;
	my_edge_indices.elvector = edge_indices[in_new_cells[get_global_id(0)]];
	
	for(uint x=0; x<16; x++)
	{
		uint2 current_edge = edges[my_edge_indices.elarray[x]];
		
		// Get vertex positions relative to the cell's position:
		float3 vertex_1 = vertices[current_edge.x].xyz - my_position.xyz;
		float3 vertex_2 = vertices[current_edge.y].xyz - my_position.xyz;
		
		// The sign of the result of the next equation indicates on which side of the plane the vertices lie. Therefore,
		// if the signs differ for vertex_1 and vertex_2, the edge crosses the division plane:
		if (sign(dot(division_plane_normal, vertex_1)) != sign(dot(division_plane_normal, vertex_2)))
		{
			// This edge crosses the division plane. Subdivide the edge.
			
		}
	}
};

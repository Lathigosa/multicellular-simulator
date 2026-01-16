// Bla

#include <random>

// Scale factor for splitting a particle into two smaller particles
// such that the combined volume of the children equals the parent.
// (1/2)^(1/3) \approx 0.793700526
#define HALF_VOLUME_RADIUS_SCALE 0.793700526f

kernel void split_particle( global const float4* in_buffer,
							global float4* out_buffer,
							global uint* in_marked_particles,
							int first_empty_element_in_array,
							uint2 seed)
{
	size_t new_particle_index = get_global_id(0) + first_empty_element_in_array;
	size_t old_particle_index = in_marked_particles[get_global_id(0)];

	//TODO: perhaps double buffer to prevent race conditions!!!
	float child_particle_radius = in_buffer[old_particle_index].w * HALF_VOLUME_RADIUS_SCALE;	// Multiply by 1/2^(1/3) to preserve volume.
	float3 current_position = in_buffer[old_particle_index].xyz;
	
	uint gid = get_global_id(0);

	float2 angles = generate_random_float(seed, (uint2)(gid, gid * 1664525u));
	angles.x = angles.x * M_PI_F;
	angles.y = acos(1.0f - angles.y * 2.0f);
	const float sin_a = sin(angles.x);
	const float cos_a = cos(angles.x);
	const float sin_b = sin(angles.y);
	const float cos_b = cos(angles.y);

	float3 division_axis = (float3)(sin_a * cos_b, sin_a * sin_b, cos_a);
	
	out_buffer[new_particle_index] = (float4)(current_position - division_axis, child_particle_radius);
	out_buffer[old_particle_index] = (float4)(current_position + division_axis, child_particle_radius);

	//if(get_global_id(0) == 0)
	//{
	//	printf("GPU Kernel: Cell division random displacement\n", old_particle_index, " ", new_particle_index);
	//}
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

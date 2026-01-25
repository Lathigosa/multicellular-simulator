
// Call with global_id(0) \in [0, array_size], global_id(1) \in [0, amount_of_particles_to_copy]
/*
#define TEMPLATE_APPEND_ARRAY_KERNEL(TYPE, BYTE_SIZE) \
kernel void append_particles_array_##BYTE_SIZE (global const TYPE * in_buffer, \
										global TYPE * out_buffer, \
										global uint* in_new_particles, \
										int append_index) \
{ \
	uint current_particle = in_new_particles[get_global_id(1)]; \
	uint new_index = (get_global_id(1) + append_index)*get_global_size(0) + get_global_id(0); \
	uint old_index = (current_particle)*get_global_size(0) + get_global_id(0); \
	out_buffer[new_index] = in_buffer[old_index]; \
}
*/

#include <random>

kernel void split_particle_membrane( global const float4* in_buffer,
							global float4* out_buffer,
							global uint* in_marked_particles,
							int first_empty_element_in_array,
							uint2 seed)
{
	const uint current_particle = in_marked_particles[get_global_id(1)];
	const uint new_index = (get_global_id(1) + first_empty_element_in_array)*get_global_size(0) + get_global_id(0);
	const uint old_index = (current_particle)*get_global_size(0) + get_global_id(0);

	//TODO: perhaps double buffer to prevent race conditions!!!
	float child_particle_radius = in_buffer[old_index].w; // * HALF_VOLUME_RADIUS_SCALE;	// Multiply by 1/2^(1/3) to preserve volume.
	float3 current_position = in_buffer[old_index].xyz;
	
	uint gid = current_particle;

	float2 angles = generate_random_float(seed, (uint2)(gid, gid * 1664525u));
	angles.x = angles.x * M_PI_F;
	angles.y = acos(1.0f - angles.y * 2.0f);
	const float sin_a = sin(angles.x);
	const float cos_a = cos(angles.x);
	const float sin_b = sin(angles.y);
	const float cos_b = cos(angles.y);

	float3 division_axis = (float3)(sin_a * cos_b, sin_a * sin_b, cos_a);
    division_axis = (float3)(1.0, 0.0, 0.0);
	
	out_buffer[new_index] = (float4)(current_position - division_axis, child_particle_radius);
	out_buffer[old_index] = (float4)(current_position + division_axis, child_particle_radius);

	//if(get_global_id(0) == 0)
	//{
	//	printf("GPU Kernel: Cell division random displacement\n", old_particle_index, " ", new_particle_index);
	//}
};


// Compute a bounding sphere. Not necessarily the smallest bounding sphere, but quick to calculate.
kernel void compute_bounding_sphere_reduced(
    global const float4* in_buffer,
    global float4* sphere_position_output
)
{
    const uint local_index     = get_local_id(0);   // 0..255
    const uint particle_index  = get_global_id(1);  // particle id
    const uint base_index      = particle_index * 256;

    // ------------------------------------------------------------------
    // Local memory used for staggered summation of positions
    // ------------------------------------------------------------------
    local float3 local_sum_of_positions[256];

    // Each work-item loads one position into local memory
    local_sum_of_positions[local_index] =
        in_buffer[base_index + local_index].xyz;

    // Ensure all work-items have written their values
    barrier(CLK_LOCAL_MEM_FENCE);

    // ------------------------------------------------------------------
    // Tree-style reduction to sum all 256 positions
    // ------------------------------------------------------------------
    for (uint stride = 128; stride > 0; stride >>= 1)
    {
        if (local_index < stride)
        {
            local_sum_of_positions[local_index] += local_sum_of_positions[local_index + stride];
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // ------------------------------------------------------------------
    // One work-item computes center and radius
    // ------------------------------------------------------------------
    if (local_index == 0)
    {
        float3 center_position = local_sum_of_positions[0] * (1.0f / 256.0f);

        float bounding_sphere_radius = 0.0f;

        for (uint i = 0; i < 256; ++i)
        {
            float3 offset = in_buffer[base_index + i].xyz - center_position;

            float distance_from_center = length(offset);
            bounding_sphere_radius = fmax(bounding_sphere_radius, distance_from_center);
        }

        // xyz = center position, w = radius
        sphere_position_output[particle_index] =
            (float4)(center_position, bounding_sphere_radius);
    }
}
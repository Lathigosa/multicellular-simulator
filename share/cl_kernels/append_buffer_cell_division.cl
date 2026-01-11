/**
 * @file append_buffer.cl
 * @brief Generates new particles by splitting existing particles along a division axis.
 *
 * @copyright Nathan Boogerd, 2025
 *
 * This kernel performs a particle "split" operation:
 * - Each parent particle specified in `in_marked_particles` is divided into two child particles.
 * - The children are displaced in opposite directions along the provided `in_particle_division_axis`.
 * - The sum of the particle volumes equals the parent particle volume. This is achieved by scaling the child particle radii by 1/2^(1/3).
 *
 * The kernel assumes a single buffer (`out_buffer`) is used for both reading and writing. 
 * Care may be needed to avoid race conditions; double-buffering may be required.
 *
 * @param out_buffer           Global float4 buffer containing particle positions and sizes (x,y,z,w).
 *                             x,y,z = position, w = particle radius.
 *                             The buffer is updated in-place: old particle positions are replaced
 *                             with one child, new positions are written to indices starting at `first_empty_element_in_array`.
 * @param in_marked_particles  Global uint array of parent particle indices to split.
 * @param first_empty_element_in_array Integer offset specifying where new child particles should be written in `out_buffer`.
 * @param in_particle_division_axis Global float4 array specifying the normalized division axis for each parent particle.
 *
 */

#define NEW_PARTICLE_INDEX get_global_id(0) + first_empty_element_in_array
#define OLD_PARTICLE_INDEX in_marked_particles[get_global_id(0)]

// Scale factor for splitting a particle into two smaller particles
// such that the combined volume of the children equals the parent.
// (1/2)^(1/3) \approx 0.793700526
#define RADIUS_SCALE_FACTOR 0.793700526f

kernel void split_particle(	global float4* out_buffer,
							global const uint* in_marked_particles,
							int first_empty_element_in_array,
							global const float4* in_particle_division_axis)
{
	size_t new_particle_index = get_global_id(0) + first_empty_element_in_array;
	size_t old_particle_index = in_marked_particles[get_global_id(0)];

	float4 division_axis = in_particle_division_axis[get_global_id(0)];
	float3 current_position = out_buffer[old_particle_index].xyz;
	
	//TODO: perhaps double buffer to prevent race conditions!!!
	float new_size = out_buffer[old_particle_index].w * RADIUS_SCALE_FACTOR;	// Multiply by 1/2^(1/3) to preserve volume.

	// Displace the cells in opposite direction:
	out_buffer[new_particle_index] = (float4)(current_position - division_axis.xyz, new_size);
	out_buffer[old_particle_index] = (float4)(current_position + division_axis.xyz, new_size);
};

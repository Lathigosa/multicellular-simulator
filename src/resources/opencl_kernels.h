#ifndef OPENCL_KERNELS_H_INCLUDED
#define OPENCL_KERNELS_H_INCLUDED

#include <string>

/**
*   This file includes the openCL kernels.
*/

namespace res {

    namespace ocl {

        /********************************************//**
         * The openCL kernel names (as provided in the openCL source) must be named according to the following standard:
         * \code "kernel void <unit>_<program_type>_<description>()"
         * , where
         * \param <unit> is the simulation unit (e.g. membrane, cell, protein...);
         * \param <program_type> is either "simulate" or "organize";
         * \param <description> is the general description of the program, which can be anything.
         * \example "kernel void membrane_simulate_particles(...)"
         ***********************************************/


    	/// The openCL kernel which simulates the membrane particles:
        const std::string program_membrane_simulate_particles =
			"kernel void membrane_simulate_particles(global float4* out_position,"
													"global float4* out_velocity,"
													"global const float4* in_position,"
													"global const float4* in_velocity,"
													//"global const float4* in_normal,"		// Indicates the rotation of the particle as well.
													"int count,"
													"float timestep)"	// TODO: possibly replace "float4* normal" with "half* normal" and use vload_half4(index, normal) to access it.
			"{"
				"float3 my_position = in_position[get_global_id(0)].xyz;"
				//"float3 my_velocity = in_velocity[get_global_id(0)].xyz;"
				"float3 my_acceleration = (float3)(0.0f, 0.0f, 0.0f);"

				"float3 my_normal = (float3)(0.0f, 0.0f, 1.0f);"		// TODO: get normal

				//"position[get_global_id(0)];"
				//"float4 current_normal = vload_half4(get_global_id(0), in_normal);"			// Load the normal and convert the halfs to floats.
                // dot product = 0 means that the other particle is in the plane of the membrane.
				"for(int a=0; a<count; a++)"
				"{"
					"float3 your_position = in_position[a].xyz;"
					"float3 your_normal = (float3)(0.0f, 0.0f, 1.0f);"	// TODO: get normal
					"float dist = distance(my_position, your_position);"

					// TODO: remove this if-statement:
					"if(dist <= 0.3f) dist = 1.0f;"

					// Normalize:
					"float3 force_direction = (my_position - your_position) / dist;"

					// Lennard-Jones Potential:
					"float force_magnitude = 24.0f * (0.5f) / dist * (2.0f*pown(8.0f / dist, 12) - pown(8.0f / dist, 6));"
					//"float force_size = 1.0f;"

					//"force_magnitude = force_magnitude * (1.0f - fabs(dot(my_normal, force_direction)));"

					"float3 force = force_magnitude * force_direction;"
					//"float3 acceleration = 1.0f * force;"

					// Discard values that are NAN (for test purposes):
					"if (!isnan(fast_length(force)) && !isinf(fast_length(force)))"

					"my_acceleration = my_acceleration + force * 1.0f;"	// TODO: add mass to particles (currently: 1.0f).
				"}"

				// Calculate velocity (based on timestep):
				"float3 my_velocity = in_velocity[get_global_id(0)].xyz + (timestep * my_acceleration);"
				"my_position = my_position + (timestep * my_velocity);"

				// Upload the values:
				"out_velocity[get_global_id(0)] = (float4) (my_velocity, 1.0f);"
				"out_position[get_global_id(0)] = (float4) (my_position, 1.0f);"
				//"out_position[get_global_id(0)] = (float4) (0.0f, 1.0f, 2.0f, 1.0f);"
			"}";

			/*
			const std::string program_membrane_simulate_particles =
			"kernel void membrane_simulate_particles(global float4* out_position,"
													"global float4* out_velocity,"
													"global const float4* in_position,"
													"global const float4* in_velocity,"
													//"global const float4* in_normal,"		// Indicates the rotation of the particle as well.
													"int count,"
													"float timestep)"	// TODO: possibly replace "float4* normal" with "half* normal" and use vload_half4(index, normal) to access it.
			"{"
				"float3 my_position = in_position[get_global_id(0)].xyz;"
				//"float3 my_velocity = in_velocity[get_global_id(0)].xyz;"
				"float3 my_acceleration = (float3)(0.0f, 0.0f, 0.0f);"
				//"position[get_global_id(0)];"
				//"float4 current_normal = vload_half4(get_global_id(0), in_normal);"			// Load the normal and convert the halfs to floats.
                // dot product = 0 means that the other particle is in the plane of the membrane.
				"for(int a=0; a<count; a++)"
				"{"
					"float3 your_position = in_position[a].xyz;"
					"float dist = distance(my_position, your_position);"
					"float3 force_direction = my_position - your_position;"

					// TODO: remove this if-statement:
					"if(dist == 0.0f) dist = 1.0f;"

					// Lennard-Jones Potential:
					"float force_size = 24.0f * (0.1f) / dist * (2.0f*pown(8.0f / dist, 12) - pown(8.0f / dist, 6));"
					//"float force_size = 1.0f;"

					"float3 force = force_size * force_direction;"
					//"float3 acceleration = 1.0f * force;"
					"my_acceleration = my_acceleration + force * 1.0f;"	// TODO: add mass to particles (currently: 1.0f).
				"}"

				// Calculate velocity (based on timestep):
				"float3 my_velocity = in_velocity[get_global_id(0)].xyz + (timestep * my_acceleration);"
				"my_position = my_position + (timestep * my_velocity);"

				// Upload the values:
				"out_velocity[get_global_id(0)] = (float4) (my_velocity, 1.0f);"
				"out_position[get_global_id(0)] = (float4) (my_position, 1.0f);"
				//"out_position[get_global_id(0)] = (float4) (0.0f, 1.0f, 2.0f, 1.0f);"
			"}";
			*/

		/********************************************//**
		 * \brief The openCL kernel which simulates the membrane particle's
		 * contents diffusion in the membrane, stochastically.
		 *
		 * \param in_position Array containing all particle positions.
		 * \param attached_indices Array containing the indices of all particles connected to each particle.
		 * \param moles 2D Array containing the number of moles of a substance for each membrane particle.
		 * \param delta_time The timestep of the simulation.
		 ***********************************************/
        const std::string program_membrane_simulate_plane_diffusion =
			"kernel void membrane_simulate_plane_diffusion(global const float4* in_position, global const uint8* attached_indices, global float* moles, float delta_time)"
			"{"
				"int column = get_global_id(0);"
				"int row = get_global_id(1);"
				""
			"}";

		/*/// The openCL kernel which simulates the membrane particles:
		// Argument "in_normal" is of the format [x, y, z, angle], where x-y-z gives the normalized normal vector, and angle gives the rotation (radians) of the particle. This can be used to determine anisotropic characteristics.
        // TODO: convert equilibrium_angle to an interpolated array, so that there is an equilibrium angle for each direction the particle is facing (and anisotropy is possible).
        const std::string program_membrane_organize_links =
			"kernel void membrane_organize_links(float equilibrium_angle, global const float4* in_position, global const float4* in_normal, int total_vertex_count, local int local_vertex_count)"	// TODO: possibly replace "float4* normal" with "half* normal" and use vload_half4(index, normal) to access it.
			"{"
				//"position[get_global_id(0)];"
				//"float4 current_normal_data = vload_half4(get_global_id(0), normal);"			// Load the normal and convert the halfs to floats.
                // dot product = 0 means that the other particle is in the plane of the membrane.
				"float4 current_normal_data = in_normal[get_global_id(0)];"

				"float3 temp_normal = current_normal_data.xyz;"
				"float angle = current_normal_data.w;"

				"for(int a=0; a<local_vertex_count; a++)"
				"{"
					""
				"}"



			"}";*/

		/// The openCL kernel which simulates the membrane particles:
		// Argument "in_normal" is of the format [x, y, z, angle], where x-y-z gives the normalized normal vector, and angle gives the rotation (radians) of the particle. This can be used to determine anisotropic characteristics.
        // TODO: convert equilibrium_angle to an interpolated array, so that there is an equilibrium angle for each direction the particle is facing (and anisotropy is possible).
        const std::string program_membrane_organize_links =
			"kernel void membrane_organize_links(global uint8* out_connected_vertices"		// The 8 connected vertices.
												"float equilibrium_angle,"
												"global const uint16* in_closest_cells,"
												"global const float4* in_position,"
												"global const float4* in_normal,"
												"local float4* local_position,"
												"int total_vertex_count,"
												"local int local_vertex_count)"	// TODO: possibly replace "float4* normal" with "half* normal" and use vload_half4(index, normal) to access it.
			"{"
				/// get_global_id(0) is the cell index.
				//"position[get_global_id(0)];"
				//"float4 current_normal_data = vload_half4(get_global_id(0), normal);"			// Load the normal and convert the halfs to floats.
                // dot product = 0 means that the other particle is in the plane of the membrane.
				"float4 current_normal_data = in_normal[get_global_id(0)];"

				"union"
				"{"
					"uint elarray[16];"
					"uint16 elvector;"
					"int16 bits;"
				"} cells;"

				"cells.elvector = in_closest_cells[get_global_id(0)];"

				"for(int b=0; b<16; b++)"
				"{"
					"for(int a=0; a<local_vertex_count; a++)"
					"{"
						// Add vertex to detection queue (because they are close to this cell):
						"in_position[a]"
					"}"
				"}"

				// TODO: optimize memory access in this kernel (it can be done).

				// First, load the vertices to the local buffer:
				""

				""

				"float3 temp_normal = current_normal_data.xyz;"
				"float angle = current_normal_data.w;"





			"}";

		/// The openCL kernel which determines the 16 closest cells with respect to each cell:
		// Note: this kernel only works for cell_count >= 16. (POSSIBLY NOT)
		// Note: this kernel is optimized for vector operations of a large size.
		// If the platform uses a scalar architecture, this might not have the best performance.
		const std::string program_cell_organize_closest =
			"float get_largest_value(float16 vector)"
			"{"
				"float8 vector8 = max(vector.lo, vector.hi);"
				"float4 vector4 = max(vector8.lo, vector8.hi);"
				"float2 vector2 = max(vector4.lo, vector4.hi);"
				"return max(vector2.lo, vector2.hi);"
			"}"

			"kernel void cell_organize_closest(global uint16* out_closest_cells, global const float4* in_position, uint cell_count)"	// TODO: possibly replace "float4* normal" with "half* normal" and use vload_half4(index, normal) to access it.
			"{"
				// Define the (rather hackish) 16-element sized uint:
				"union"
				"{"
					"uint elarray[16];"
					"uint16 elvector;"
					"int16 bits;"
				"} temp_closest;"

				// Define the distances for each stored temporary smallest cell:
				"union"
				"{"
					"float elarray[16];"
					"float16 elvector;"
					"int16 bits;"
				"} temp_distances;"


				//"uint16 temp_closest;"
				"float3 my_position = in_position[get_global_id(0)].xyz;"

				// Iterate through the first few cells and determine their distance:
				"for(uint a=0; a<16; a++)"
				"{"
					"temp_closest.elarray[a] = a;"
					"float3 testing_position = in_position[a].xyz;"
					"float current_distance = distance(testing_position, my_position);"
					"temp_distances.elarray[a] = current_distance;"
				"}"


				// Iterate through all cells and determine their distance to 'me':
				// ===TODO: change int a to uint a without compiler errors at line "(temp_true & a)!!!==="
				"for(int a=16; a<cell_count; a++)"
				"{"
					"float3 testing_position = in_position[a].xyz;"
					"union {float f; int bits;} current_distance;"
					"current_distance.f = distance(testing_position, my_position);"
					"union {float f; int bits;} largest_distance;"
					"largest_distance.f = get_largest_value(temp_distances.elvector);"

					// If the distance is smaller than the largest in temp_distances, replace the largest:
					"if(current_distance.f < largest_distance.f)"
					"{"
						//"replace_largest_value(temp_distances.elvector, largest_distance, temp_closest.elvector, a);"
						"int16 temp_false = (temp_distances.elvector != largest_distance.f);"
						"int16 temp_true = (temp_distances.elvector == largest_distance.f);"

						"temp_closest.bits = (temp_false & temp_closest.bits) | (temp_true & a);"
						"temp_distances.bits = (temp_false & temp_distances.bits) | (temp_true & current_distance.bits);"
					"}"
				"}"

				// After identifying all 16 closest cells, push the data to the global array:
				"out_closest_cells[get_global_id(0)] = temp_closest.elvector;"
			"}";

		/// The openCL kernel which determines the 16 closest cells with respect to each cell:
		// Note: this kernel only works for cell_count >= 16. (POSSIBLY NOT)
		// Note: this kernel is optimized for vector operations of a large size.
		// If the platform uses a scalar architecture, this might not have the best performance.
		const std::string program_cell_organize_touch_surface =
			"float get_largest_value(float16 vector)"
			"{"
				"float8 vector8 = max(vector.lo, vector.hi);"
				"float4 vector4 = max(vector8.lo, vector8.hi);"
				"float2 vector2 = max(vector4.lo, vector4.hi);"
				"return max(vector2.lo, vector2.hi);"
			"}"

			// This function returns the intersection point of a plane. Note that it only works when n1 dot (n2 cross n3) isn't 0.
			"float3 get_intersect_point_of_planes(float4 normal1, float4 normal2, float4 normal3)"
			"{"
				"float3 n1 = normal1.xyz;"
				"float3 n2 = normal2.xyz;"
				"float3 n3 = normal3.xyz;"
				"float3 P = (-normal1.w * cross(n2, n3) - normal2.w * cross(n3, n1) - normal3.w * cross(n1, n2)) / dot(n1, cross(n2, n3));"
				"return P;"
			"}"

			"kernel void cell_organize_touch_surface(global const uint16* in_closest_cells, global const float4* in_position, uint cell_count)"	// TODO: possibly replace "float4* normal" with "half* normal" and use vload_half4(index, normal) to access it.
			"{"
				// Define the (rather hackish) 16-element sized uint:
				"union"
				"{"
					"uint elarray[16];"
					"uint16 elvector;"
					"int16 bits;"
				"} temp_closest;"

				"temp_closest.elvector = in_closest_cells[get_global_id(0)];"

				"float3 my_position = in_position[get_global_id(0)].xyz;"

				// Define the coefficients for the half-way planes:
				"float4 planes[16];"
				"for(int a=0; a<16; a++)"
				"{"
					"float3 current_position = in_position[temp_closest.elarray[a]].xyz;"

					// Get normal vector (pointing outwards):
					"float4 current_normal;"
					"current_normal.xyz = current_position - my_position;"

					// Get half-way point and calculate normal.w:
					"float3 current_center_point = (current_position + my_position) / 2;"
					"current_normal.w = dot(current_center_point, current_normal.xyz);"
				"}"

				// Get intersection points of the planes:
				"for(int a=0; a<16; a++)"
				"{"
					"for(int b=0; b<16; b++)"
					"{"
						"if (b == a) break;"
						"for(int c=0; c<16; c++)"
						"{"
							//"if (c == b) break;"
							//"if (c == a) break;"

						"}"
					"}"
				"}"

			"}";
    }

}

#endif // OPENCL_KERNELS_H_INCLUDED

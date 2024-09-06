/// The openCL kernel which determines the 16 closest cells with respect to each cell:
// Note: this kernel only works for cell_count >= 16. (POSSIBLY NOT)
// Note: this kernel is optimized for vector operations of a large size.
// If the platform uses a scalar architecture, this might not have the best performance.
float get_largest_value(float16 vector)
{
	float8 vector8 = max(vector.lo, vector.hi);
	float4 vector4 = max(vector8.lo, vector8.hi);
	float2 vector2 = max(vector4.lo, vector4.hi);
	return max(vector2.lo, vector2.hi);
}

kernel void cell_organize_closest(global uint16* out_closest_cells, global const float4* in_position, uint cell_count)	// TODO: possibly replace "float4* normal" with "half* normal" and use vload_half4(index, normal) to access it.
{
	// Define the (rather hackish) 16-element sized uint:
	union
	{
		uint elarray[16];
		uint16 elvector;
		int16 bits;
	} temp_closest;

	// Define the distances for each stored temporary smallest cell:
	union
	{
		float elarray[16];
		float16 elvector;
		int16 bits;
	} temp_distances;


	//"uint16 temp_closest;"
	float3 my_position = in_position[get_global_id(0)].xyz;

	// Iterate through the first few cells and determine their distance:
	for(uint a=0; a<16; a++)
	{
		temp_closest.elarray[a] = a;
		float3 testing_position = in_position[a].xyz;
		float current_distance = distance(testing_position, my_position);
		temp_distances.elarray[a] = current_distance;
	}


	// Iterate through all cells and determine their distance to 'me':
	// ===TODO: change int a to uint a without compiler errors at line "(temp_true & a)!!!==="
	for(int a=16; a<cell_count; a++)
	{
		float3 testing_position = in_position[a].xyz;
		union {float f; int bits;} current_distance;
		current_distance.f = distance(testing_position, my_position);
		union {float f; int bits;} largest_distance;
		largest_distance.f = get_largest_value(temp_distances.elvector);

		// If the distance is smaller than the largest in temp_distances, replace the largest:
		if(current_distance.f < largest_distance.f)
		{
			//"replace_largest_value(temp_distances.elvector, largest_distance, temp_closest.elvector, a);"
			int16 temp_false = (temp_distances.elvector != largest_distance.f);
			int16 temp_true = (temp_distances.elvector == largest_distance.f);

			temp_closest.bits = (temp_false & temp_closest.bits) | (temp_true & a);
			temp_distances.bits = (temp_false & temp_distances.bits) | (temp_true & current_distance.bits);
		}
	}

	// After identifying all 16 closest cells, push the data to the global array:
	out_closest_cells[get_global_id(0)] = temp_closest.elvector;
};

// Bla

// This function returns the surface area of the polar spot, after division has occurred. Hence, it returns the
// surface area that goes to one cell. To get the surface area of the other cell, use total_area - get_split_area().
float get_split_area(float cluster_size_angle, float cell_radius, float p3_over_p2)
{
	float rho = cell_radius;
	float alpha = cluster_size_angle;
	float cot_alpha = 1 / tan(cluster_size_angle);
	
	return 2*rho*rho*atan((sqrt(1-(p3_over_p2*cot_alpha)*(p3_over_p2*cot_alpha)))/sqrt((p3_over_p2)*(p3_over_p2)+(p3_over_p2*cot_alpha)*(p3_over_p2*cot_alpha))) + rho*rho * cos(alpha) * (-M_PI + 2 * asin(p3_over_p2 * cot_alpha));
}

// Computes the angle of the polar spot when given a surface area of the polar spot.
float get_angle_from_area(float area, float cell_radius)
{
	float cell_radius_squared = cell_radius*cell_radius;
	return acos(1-area/2/M_PI/cell_radius_squared);
}

kernel void append_buffer(	global float4* out_buffer,
							const global float4* in_buffer,
							global uint* in_new_cells,
							int append_index,
							const global float4* in_division_axis)
{
	// Assuming that the division axis is in the x-direction:
	// TODO: change these hard-coded values:
	float cell_radius = 1.0f;
	float4 division_axis = in_division_axis[get_global_id(0)];
	float4 v1 = in_buffer[in_new_cells[get_global_id(0)]];	// The polarity vector 1
	
	// Calculate the fractions of the polar spot that go to either cell:
	
	// p3_over_p2 contains information about the orientation of the division axis with respect to the polarity vector (see model description):
	//float p3_over_p2 = division_axis.z / length(division_axis.xy);
	float cosine_axis = dot(division_axis.xyz, v1.xyz) / length(division_axis) / length(v1);
	float p3_over_p2 = cosine_axis / sqrt(1 - cosine_axis*cosine_axis);
	
	// Get the polar spot angle before any transforms, so as to calculate the total area that's transferred:
	float initial_size_angle = length(v1.xyz);
	
	float full_area = cell_radius*cell_radius*2*M_PI*(1 - cos(initial_size_angle));
	float split_area = get_split_area(initial_size_angle, cell_radius, p3_over_p2);
	
	// If the split_area is NaN, there are no intersecting points; the surface area won't be split:
	if(!isnan(split_area))
	{
		
		// Calculate the new angle:
		
		
		float new_cluster_size_angle1 = get_angle_from_area(split_area, cell_radius);
		float new_cluster_size_angle2 = get_angle_from_area(full_area - split_area, cell_radius);
	
		out_buffer[get_global_id(0) + append_index] = (float4)(0.0f, 0.0f, new_cluster_size_angle1, 0.0f);
	
		out_buffer[in_new_cells[get_global_id(0)]] = (float4)(0.0f, 0.0f, new_cluster_size_angle2, 0.0f);
	}
};

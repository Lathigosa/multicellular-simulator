

kernel void calculate_physics_step(
    global const uint2* in_membrane_edges,
    global const uint2* in_membrane_edge_neighbors,
    global const uint8* in_membrane_vertex_edges,
    global const uint8* in_membrane_vertex_opposite_edges,
    global const float4* in_membrane_vertices,
	global const float8* in_centrosome_positions,
    global float8* out_centrosome_positions,
    uint membrane_vertex_buffer_stride
    )
{
    size_t cell_index = get_global_id(0);
    float8 current_centrosomes = in_centrosome_positions[cell_index];
    float3 current_centrosome_1 = current_centrosomes.s012;
    float3 current_centrosome_2 = current_centrosomes.s456;
    float3 total_force_1 = (float3)(0.0f, 0.0f, 0.0f);
    float3 total_force_2 = (float3)(0.0f, 0.0f, 0.0f);
    for (size_t i = 0; i < 162; i++)
    {
        float3 current_membrane_vertex = in_membrane_vertices[membrane_vertex_buffer_stride*cell_index + i].xyz;
        float distance_to_membrane_1 = distance(current_centrosome_1, current_membrane_vertex);
        float distance_to_membrane_2 = distance(current_centrosome_2, current_membrane_vertex);
        float3 direction_1 = normalize(current_membrane_vertex - current_centrosome_1);
        float3 direction_2 = normalize(current_membrane_vertex - current_centrosome_2);
        float force_multiplier = 10.0f;
        float3 force_push_1 = force_multiplier/(distance_to_membrane_1*distance_to_membrane_1)*direction_1;
        if(!(isnan(force_push_1.x) || isnan(force_push_1.y) || isnan(force_push_1.z)))
            total_force_1 -= force_push_1;
        float3 force_push_2 = force_multiplier/(distance_to_membrane_2*distance_to_membrane_2)*direction_2;
        if(!(isnan(force_push_2.x) || isnan(force_push_2.y) || isnan(force_push_2.z)))
            total_force_2 -= force_push_2;

        float pull_force_multiplier = 1.8f;
        float3 force_pull_1 = pull_force_multiplier*direction_1;
        if(!(isnan(force_pull_1.x) || isnan(force_pull_1.y) || isnan(force_pull_1.z)))
            total_force_1 += force_pull_1;
        float3 force_pull_2 = pull_force_multiplier*direction_2;
        if(!(isnan(force_pull_2.x) || isnan(force_pull_2.y) || isnan(force_pull_2.z)))
            total_force_2 += force_pull_2;
    }

    current_centrosomes.s012 = current_centrosome_1 + total_force_1*0.01f*0.001f;
    current_centrosomes.s456 = current_centrosome_2 + total_force_2*0.01f*0.001f;

    out_centrosome_positions[cell_index] = current_centrosomes;
}

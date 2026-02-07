// 

// Call with global_id(0) \in [0, array_size], global_id(1) \in [0, amount_of_particles_to_copy]

#define MAX_VERTICES_PER_PARTICLE 256
#define MAX_EDGES_PER_PARTICLE 768

#define INVALID_INDEX_UINT 0xFFFFFFFF

kernel void membrane_curvature_force(
	global const uint2* in_edges,
    global const uint2* in_edge_neighbors,
    global const uint8* in_vertex_edges,
    global const uint8* in_vertex_opposite_edges,
    global const float4* in_vertices,
    //global float* cytokinetic_ring,
	global float4* out_forces,
    uint vertex_buffer_stride
	)
{
    local float4 local_vertices[MAX_VERTICES_PER_PARTICLE];

    // Obtain edge index:
    const size_t current_index_offset = get_global_id(1) * vertex_buffer_stride;
    const size_t current_edge_index = get_global_id(0) + get_global_id(1) * get_global_size(0);

    // Import vertices of the local cell membrane:
    if (get_local_id(0) < vertex_buffer_stride)
        local_vertices[get_local_id(0)] = in_vertices[current_index_offset + get_local_id(0)];

    barrier(CLK_LOCAL_MEM_FENCE);

    // Obtain vertex indices:
    const uint2 current_edge = in_edges[current_edge_index];
    const uint2 current_edge_neighbors = in_edge_neighbors[current_edge_index];

    // Obtain vertex positions:
    const float4 vertex_1 = local_vertices[current_edge.x];
    const float4 vertex_2 = local_vertices[current_edge.y];
    const float4 opposite_vertex_1 = local_vertices[current_edge_neighbors.x];
    const float4 opposite_vertex_2 = local_vertices[current_edge_neighbors.y];

    float3 edge_vector = vertex_2.xyz - vertex_1.xyz;

    // Compute triangle normals (TODO: check handedness):
    float3 normal_1 = normalize(cross(edge_vector, opposite_vertex_1.xyz - vertex_1.xyz));
    float3 normal_2 = normalize(cross(edge_vector, opposite_vertex_2.xyz - vertex_1.xyz));

    // Dihedral angle cosine:
    float cosine_of_angle = clamp(dot(normal_1, normal_2), -1.0f, 1.0f);
    float angle = acos(cosine_of_angle);

    // Calculate angle force (linear spring force):
    const float kappa = 0.1f;
    float3 force_magnitude = kappa * angle;

    float3 edge_direction = normalize(edge_vector);
    float3 normal_sum = normalize(normal_1 + normal_2);
    float3 force_vector = force_magnitude * cross(edge_direction, normal_sum);

    local float4 local_angular_forces[MAX_EDGES_PER_PARTICLE];

    local_angular_forces[get_local_id(0)] = (float4)(force_vector, 0.0f);

    // Calculate planar force (linear spring force):
    const float stiffness = 1.0f;
    const float rest_length = 0.238f;
    float length_difference = (length(edge_vector) - rest_length);
    float3 planar_force_vector = stiffness*length_difference*length_difference*edge_direction;

    local float4 local_planar_forces[MAX_EDGES_PER_PARTICLE];

    local_planar_forces[get_local_id(0)] = (float4)(planar_force_vector, 0.0f);

    barrier(CLK_LOCAL_MEM_FENCE);

    /* Loop over vertices */

    if (get_local_id(0) >= vertex_buffer_stride) return;

    union {
		uint elarray[8];
		uint8 elvector;
	} vertex_edges;

    union {
		uint elarray[8];
		uint8 elvector;
	} vertex_opposite_edges;

    uint vertex_id = get_global_id(0) + get_global_id(1) * vertex_buffer_stride;
    uint gid = get_global_id(0);
    
    vertex_edges.elvector = in_vertex_edges[vertex_id];
    vertex_opposite_edges.elvector = in_vertex_opposite_edges[vertex_id];
    float3 total_curvature_force = (float3)(0.0f, 0.0f, 0.0f);
    float3 total_planar_force = (float3)(0.0f, 0.0f, 0.0f);
    uint valence = 0;
    for (size_t i=0; i<8; i++) {
        // Add contribution of neighboring edges:
        uint edge_index = vertex_edges.elarray[i];
        if (edge_index != INVALID_INDEX_UINT) {
            uint2 edge = in_edges[edge_index + get_global_id(1) * get_global_size(0)]; // SLOW!! Global access, requires ~300 cycles in worst case.
            if (edge.x == get_global_id(0)) {
                total_planar_force += local_planar_forces[edge_index].xyz;
            } else if (edge.y == get_global_id(0)) {
                total_planar_force -= local_planar_forces[edge_index].xyz;
            } else {
                total_planar_force = NAN;
                if (get_global_id(0) == 0) printf("Indices don't match: get_global_id(0) = %u, edge.x = %u, edge.y=%u\n", gid, edge.x, edge.y);
            }
            total_curvature_force -= 0.5f*local_angular_forces[edge_index].xyz;
        }
        
        // Add contribution from opposite edges:
        uint index2 = vertex_opposite_edges.elarray[i];
        if (index2 != INVALID_INDEX_UINT) total_curvature_force += local_angular_forces[index2].xyz;
        if (edge_index != INVALID_INDEX_UINT) valence++;
    }
    if (valence > 0) total_curvature_force /= (float)valence;
    // TODO: this is a fake normal. Replace it with a real normal.
    //float3 n = normalize(local_vertices[vertex_id].xyz);
    //total_force = dot(total_force, n) * n;
    //total_force = -total_force;

    float3 vertex = local_vertices[get_global_id(0)].xyz;
    float ring_force_magnitude = min(fabs(vertex.x) - 0.1f, 0.0f);
    float3 ring_force_direction = vertex.xyz;
    float3 ring_force = 5.0f*ring_force_magnitude*ring_force_direction;

    float3 total_force = total_planar_force + total_curvature_force + ring_force;

    //Apply point force:
    //if (get_global_id(0) == 0) total_force += (float3)(0.0f, 0.0f, 1.0f);

    out_forces[vertex_id] = (float4)(total_force, 0.0f);
}

kernel void membrane_integrate_forces(
    global const float4* in_vertices,
	global const float4* in_forces,
    global float4* out_vertices,
	private int count,
	private float timestep
	)
{
    out_vertices[get_global_id(0)] = in_vertices[get_global_id(0)] + in_forces[get_global_id(0)] * timestep * timestep;
}
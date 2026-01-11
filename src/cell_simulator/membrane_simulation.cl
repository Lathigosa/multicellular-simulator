// Simulate a membrane physics step, without incorporating any other forces.

/* 

Simulate a membrane physics step, incorporating the internal energy forces,
without incorporating any other forces just yet.

The overall pipeline is as follows:

1. First, all forces on the membrane are calculated.
2. Second, the forces are integrated to derive the new position in the membrane.


A membrane is represented as a list of vertices, each of which is connected to
other vertices.

The properties of vertex i in the array is given by:
array<float4> vertex_position[i];
array<float4> vertex_velocity[i];
array<float4> vertex_force[i];

The neighbors of each vertex are implicitly stored in the following index array
buffer:
array<uint16> vertex_indices;
    where the indices represent triangles in an OpenGL triangle-strip layout.

Mathematically, we define a membrane as a closed 2D manifold in three-
dimensional space. Now, we can assume something important: that the membrane is
*incompressible* in the tangential axes at any specific point on the membrane.
However, it *is* able to flow in these two axes. This means that we can state
that at any point in time, the total *surface area* of the manifold must remain
identical.

The triangulation of this manifold is in some sense arbitrary. This means that
although we can choose to express the manifold as a set of triangles whose
surface area doesn't change, this may be too constraining for a simulation.

Instead, we allow the vertices to slide along the manifold freely, and the only
constraint imposed on the membrane manifold is that any surface element needs to
maintain its surface area. The triangle area, however, may become larger or
smaller over time, because again, they can slide around. This flexibility
allows parts of the manifold with higher curvature to be represented with more
vertices, which would improve the simulation accuracy.

The force on any particular point $\vec{p}$ on the manifold can be grouped to a
particular nearest neighbor vertex. These forces can next be decomposed into
those that lie inside the tangent plane, and those that are perpendicular to the
tangent plane. If they are inside the tangent plane, they will have an effect
on the position of any protein or other molecule that is anchored to the
membrane at those points. However, these do not contribute to the shape of the
membrane. Only forces that are perpendicular to the membrane's tangent plane
will contribute to the shape, and as such, each *vertex* that represents a
membrane point ignores any forces that are tangential. These forces are of
course still relevant for flow simulations, so they do not get ignored for those
parts, but for the shape we can safely ignore them.

Instead, on the tangential axis, we generate a number of *fake* forces that act
on each vertex, which forces the vertices to maintain a certain amount of
distance from one another, while moving closer together when there are sharp
bends. These fake forces do not correspond to real forces, and instead, the
real flow underneath has to be simulated separately and importantly must
compensate for the fake forces to cancel their effects.

The shape calculation depends only on minimizing the *bending energy*. Yet, we
may have applications of point forces at a specific time $t$, which force the
system out of equilibrium.



*/

/*

Each vertex has at most 6 vertex neighbors if it's a perfect icosphere.

Let's assume for now that arbitrary shapes we have at most 16 edges, because
we limit the angles to be at least 22.5 degrees.

*/

kernel void simulate_membrane_physics_step(
    global float4 * vertex_positions,
    global ushort16 * vertex_edges,
    global ushort * vertex_indices
) {
    // Calculate vertex normals.
    ushort neighbor_index_1 = 0; // TODO
    float4 neighbor_1_position = vertex_positions[neighbor_index_1];
    float4 vertex_normal = vertex_positions;

    // Calculate real forces.
    // Simulate fake forces to push vertices around along the tangent plane.
}

void flip_vertices(
    global float4 * vertex_positions,
    global ushort16 * edges_around_vertex,
    global ushort16 * triangles_around_vertex,
    global ushort3 * triangle_index_buffer
) {
    float4 my_position = vertex_positions[get_global_id(0)];

    union {
		ushort elarray[16];
		ushort16 elvector;
		short16 bits;
	} my_neighbors;
    my_neighbors.elvector = vertex_edges[get_global_id(0)];

    union {
		float elarray[16];
		float16 elvector;
	} my_neighbor_positions;

    // Assume cyclic ordering.
    for (size_t i=0; i<16; i++) {
        ushort neighbor_index = my_neighbors.elvector[i];
        my_neighbor_positions.elvector[i] = vertex_positions[neighbor_index];
        edge_length = length(my_position, your_position);
        edge_angle = atan2();
    }
}

kernel void rearrange_vertices() {
    
}
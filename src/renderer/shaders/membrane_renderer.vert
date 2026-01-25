/*#version 330 core

attribute vec3 coordinate;
attribute float radius;
varying vec4 f_color;
uniform mat4 MVP_matrix;
uniform mat4 MV_matrix;
uniform mat4 P_matrix;
uniform float screen_width;

void main(void) {
	vec4 eye_position = MV_matrix * vec4(coordinate, 1.0);
	vec4 projection = P_matrix * vec4(radius, radius, eye_position.z, eye_position.w);
	gl_Position = P_matrix * eye_position;
	float screen_z = (-gl_Position.z) * 1.0;
	//"gl_PointSize = screen_z * 1.0;"
	gl_PointSize = screen_width * projection.x / projection.w;
	//"f_color = vec4(screen_z / 200.0 + 0.5, screen_z * 0.0005 + 1.0, 1.0, 1.0);
	f_color = vec4(min(gl_VertexID / 16.0 + 0.1, 1.0), min(gl_VertexID / 256.0 + 0.1, 1.0), min(gl_VertexID / 256.0 / 256.0 + 0.1, 1.0), 1.0);
	//"vec4 eye_pos = MV_matrix * vec4(coordinate, 1.0);"

}


*/

#version 460 core

layout(std430, binding = 0) readonly buffer VertexBuffer {
    vec4 vertex_positions[];
};

layout(std430, binding = 1) readonly buffer ParticleBuffer {
    vec4 particle_positions[];
};

uniform uint vertices_per_cell;
uniform mat4 MVP_matrix;
uniform mat4 MV_matrix;
uniform mat4 P_matrix;
uniform float screen_width;

out vec4 f_color;

void main()
{
    // Which block this instance corresponds to:
    uint block_offset = gl_InstanceID * vertices_per_cell;

    // Compute the absolute vertex index
    uint global_index = block_offset + gl_VertexID;

    // Fetch the vertex
    vec4 pos = vertex_positions[global_index] + particle_positions[gl_InstanceID];

    // Output clip-space position
    f_color = vec4(min(global_index / 16.0 + 0.1, 1.0), min(global_index / 256.0 + 0.1, 1.0), min(global_index / 256.0 / 256.0 + 0.1, 1.0), 1.0);
    gl_Position = MVP_matrix * vec4(pos.xyz, 1.0); // assume already in clip-space for simplicity
}
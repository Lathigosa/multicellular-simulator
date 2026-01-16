#version 330 core

layout(location = 0) in vec3 coordinate;
layout(location = 1) in float radius;
out vec4 f_color;

uniform mat4 MVP_matrix;
uniform mat4 MV_matrix;
uniform mat4 P_matrix;
uniform float screen_width;

void main(void) {
    if (position.w == 0.0) {
        gl_Position = vec4(0.0); // will be clipped
        return;
    }

    vec4 eye_position = MV_matrix * vec4(coordinate, 1.0);
    vec4 clip_position = P_matrix * eye_position;
    gl_Position = clip_position;

    // Compute point size in pixels
    gl_PointSize = screen_width * radius / clip_position.w;

    // Color based on vertex ID
    f_color = vec4(
        min(float(gl_VertexID) / 16.0 + 0.1, 1.0),
        min(float(gl_VertexID) / 256.0 + 0.1, 1.0),
        min(float(gl_VertexID) / 65536.0 + 0.1, 1.0),
        1.0
    );
}

#version 330 core

layout(location = 0) in vec3 coordinate;
layout(location = 1) in vec3 velocity;
layout(location = 2) in vec3 division_plane;
layout(location = 3) in float radius;

out vec4 f_color;
out vec4 division_axis;
out vec4 v1;

uniform mat4 MVP_matrix;
uniform mat4 MV_matrix;
uniform mat4 P_matrix;
uniform float screen_width;

void main() {
    vec4 eye_position = MV_matrix * vec4(coordinate, 1.0);
    gl_Position = P_matrix * eye_position;

    // example point size
    gl_PointSize = screen_width * radius / gl_Position.w;

    // compute color
    float screen_z = -gl_Position.z;
    f_color = vec4(screen_z / 200.0 + 0.5, screen_z * 0.0005 + 1.0, 1.0, 1.0);

    // transform vectors to eye space
    v1 = MV_matrix * vec4(velocity, 0.0);
    division_axis = MV_matrix * vec4(division_plane, 0.0);
}
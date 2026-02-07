#version 330 core

attribute vec3 global_coordinate;
attribute vec3 local_coordinate;
attribute float radius;
varying vec4 f_color;
uniform mat4 MVP_matrix;
uniform mat4 MV_matrix;
uniform mat4 P_matrix;
uniform float screen_width;

void main(void) {
	vec4 eye_position = MV_matrix * vec4(global_coordinate + local_coordinate, 1.0);
	vec4 projection = P_matrix * vec4(radius, radius, eye_position.z, eye_position.w);
	gl_Position = P_matrix * eye_position;
	float screen_z = (-gl_Position.z) * 1.0;
	//gl_PointSize = screen_z * 1.0;
    gl_PointSize = screen_width * projection.x / projection.w;
    //"f_color = vec4(screen_z / 200.0 + 0.5, screen_z * 0.0005 + 1.0, 1.0, 1.0);"
    f_color = vec4(0.3, 1.0, 0.3, 1.0);
	//"vec4 eye_pos = MV_matrix * vec4(coordinate, 1.0);"

}
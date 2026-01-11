#version 400 core

attribute vec3 coordinate;
varying vec4 f_color;
uniform mat4 MVP_matrix;
uniform ivec2 grid_repeat;
uniform vec2 grid_shift;

void main(void) {
	float pos_x = (gl_InstanceID % grid_repeat.y) - grid_shift.x;
	float pos_y = (gl_InstanceID / grid_repeat.y) - grid_shift.y;
	vec3 new_coordinate = coordinate + vec3(pos_x, pos_y, 0.0);

	vec4 screen_position = (MVP_matrix * vec4(new_coordinate, 1.0));
	gl_Position = screen_position;
	float screen_z = (-screen_position.z + 2.0) * 2.0;

	gl_PointSize = 2.0f;
	f_color = vec4(1.0, 0.0, 1, 1);
}
#version 400 core

attribute vec4 coordinate;
varying vec4 f_color;
uniform mat4 MVP_matrix;

void main(void) {
	gl_Position = MVP_matrix * vec4(coordinate.xyz, 1.0);

	gl_PointSize = 2.0f;
	f_color = vec4(coordinate.xyz / coordinate.w, 1);
}
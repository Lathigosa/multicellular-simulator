#ifndef RENDER_HEADER_H_INCLUDED
#define RENDER_HEADER_H_INCLUDED

/*
*	This header file defines the struct "render_header", which gives information
*	on how to render the information present in the buffers of the simulation
*	unit.
*/

#include <string>
#include <GL/gl.h>

struct render_header
{
	std::string position_buffer;
	std::string vector_buffer;
	std::string color_buffer;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint shader_program;
};

#endif // RENDER_HEADER_H_INCLUDED

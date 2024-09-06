#include "shader_tools.h"
#include "main.h"

GLuint create_shader(const char * const source_vertex, const char * const source_fragment)
{
	GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
	GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);

	// Compile the vertex shader:
	glShaderSource(shader_vertex, 1, &source_vertex, nullptr);
	glCompileShader(shader_vertex);

	// Compile the fragment shader:
	glShaderSource(shader_fragment, 1, &source_fragment, nullptr);
	glCompileShader(shader_fragment);

	// Link the program:
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, shader_vertex);
	glAttachShader(shader_program, shader_fragment);
	glLinkProgram(shader_program);

	// Delete remains:
	glDeleteShader(shader_vertex);
	glDeleteShader(shader_fragment);

	return shader_program;
}

GLuint create_shader_geo(const char * const source_vertex, const char * const source_geometry, const char * const source_fragment)
{
	GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
	GLuint shader_geometry = glCreateShader(GL_GEOMETRY_SHADER);
	GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);

	// Compile the vertex shader:
	glShaderSource(shader_vertex, 1, &source_vertex, nullptr);
	glCompileShader(shader_vertex);

	// Compile the geometry shader:
	glShaderSource(shader_geometry, 1, &source_geometry, nullptr);
	glCompileShader(shader_geometry);

	//GLchar * infolog = new GLchar[10000];
	//glGetShaderInfoLog(shader_geometry, 10000, nullptr, infolog);
	//message_debug(infolog);
	//delete[] infolog;

	// Compile the fragment shader:
	glShaderSource(shader_fragment, 1, &source_fragment, nullptr);
	glCompileShader(shader_fragment);

	// Link the program:
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, shader_vertex);
	glAttachShader(shader_program, shader_geometry);
	glAttachShader(shader_program, shader_fragment);

	//GLchar * infolog2 = new GLchar[10000];
	//glGetProgramInfoLog(shader_program, 10000, nullptr, infolog2);
	//message_debug(infolog2);
	//delete[] infolog2;
	
	glLinkProgram(shader_program);

	//GLchar * infolog3 = new GLchar[10000];
	//glGetProgramInfoLog(shader_program, 10000, nullptr, infolog3);
	//message_debug(infolog3);
	//delete[] infolog3;

	// Delete remains:
	glDeleteShader(shader_vertex);
	glDeleteShader(shader_geometry);
	glDeleteShader(shader_fragment);

	return shader_program;
}

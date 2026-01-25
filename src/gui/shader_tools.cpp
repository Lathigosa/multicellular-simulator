#include "shader_tools.h"
#include "main.h"

#include <vector>
#include <iostream>

static bool check_shader_compile(GLuint shader, const char* name, const char* source)
{
    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_TRUE)
        return true;

    GLint log_length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

    std::vector<char> log(log_length);
    glGetShaderInfoLog(shader, log_length, nullptr, log.data());

    std::cerr << "ERROR compiling " << name << " shader:\n";
    std::cerr << log.data() << "\n";

    // Print source with line numbers
    std::cerr << "Shader source (" << name << "):\n";
    int line = 1;
    std::cerr << line++ << " | ";

    for (const char* c = source; *c; ++c)
    {
        std::cerr << *c;
        if (*c == '\n')
            std::cerr << line++ << " | ";
    }
    std::cerr << "\n";

    return false;
}

GLuint create_shader(const char * const source_vertex,
                     const char * const source_fragment)
{
    GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(shader_vertex, 1, &source_vertex, nullptr);
    glCompileShader(shader_vertex);
    if (!check_shader_compile(shader_vertex, "VERTEX", source_vertex))
        return 0;

    glShaderSource(shader_fragment, 1, &source_fragment, nullptr);
    glCompileShader(shader_fragment);
    if (!check_shader_compile(shader_fragment, "FRAGMENT", source_fragment))
        return 0;

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, shader_vertex);
    glAttachShader(shader_program, shader_fragment);
    glLinkProgram(shader_program);

    GLint link_ok = GL_FALSE;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &link_ok);
    if (!link_ok)
    {
        GLint log_length = 0;
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &log_length);

        std::vector<char> log(log_length);
        glGetProgramInfoLog(shader_program, log_length, nullptr, log.data());

        std::cerr << "ERROR linking shader program:\n";
        std::cerr << log.data() << "\n";

        glDeleteProgram(shader_program);
        shader_program = 0;
    }

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

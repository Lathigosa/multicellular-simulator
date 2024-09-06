#include "kernels/cells_as_dots.h"

#include <iostream>

#include <CL/cl_gl.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "gui/shader_tools.h"

const char * const source_vertex_points = "#version 330 core\n"
"attribute vec3 coordinate;"
"attribute float radius;"
"varying vec4 f_color;"
"uniform mat4 MVP_matrix;"
"uniform mat4 MV_matrix;"
"uniform mat4 P_matrix;"
"uniform float screen_width;"

"void main(void) {"
	"vec4 eye_position = MV_matrix * vec4(coordinate, 1.0);"
	"vec4 projection = P_matrix * vec4(radius, radius, eye_position.z, eye_position.w);"
	"gl_Position = P_matrix * eye_position;"
	"float screen_z = (-gl_Position.z) * 1.0;"
	//"gl_PointSize = screen_z * 1.0;"
	"gl_PointSize = screen_width * projection.x / projection.w;"
	//"f_color = vec4(screen_z / 200.0 + 0.5, screen_z * 0.0005 + 1.0, 1.0, 1.0);"
	"f_color = vec4(min(gl_VertexID / 16.0 + 0.1, 1.0), min(gl_VertexID / 256.0 + 0.1, 1.0), min(gl_VertexID / 256.0 / 256.0 + 0.1, 1.0), 1.0);"
	//"vec4 eye_pos = MV_matrix * vec4(coordinate, 1.0);"

"}";

const char * const source_fragment_points = "#version 330 core\n"
"varying vec4 f_color;"

"void main(void) {"
	"if(length(gl_PointCoord - vec2(0.5, 0.5)) > 0.5)"
		"discard;"
	
    "gl_FragColor = (1.0f - 1.0f*length(gl_PointCoord - vec2(0.5, 0.5))) * f_color;"
"}";

position_spheres::position_spheres()
{
	

	// Test count:
	particle_count = 32 * 32 * 8; 	// TODO: remove
}

position_spheres::~position_spheres()
{
	// TODO: clear OpenGL objects!
	//dtor
}

void position_spheres::initialize(data_buffer::Array<cl_float4>& positions, cl::CommandQueue& queue)
{
	has_initialized = true;

	// Capture previous VAO state:
	GLint previous_VAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_VAO);

	// ===== GRID RENDERER ===== //
	// Set up shaders:
	gl_SHA_points = create_shader(source_vertex_points, source_fragment_points);

	gl_UNI_mvp_matrix = glGetUniformLocation(gl_SHA_points, "MVP_matrix");
	gl_UNI_mv_matrix = glGetUniformLocation(gl_SHA_points, "MV_matrix");
	gl_UNI_p_matrix = glGetUniformLocation(gl_SHA_points, "P_matrix");
	gl_UNI_screen_width = glGetUniformLocation(gl_SHA_points, "screen_width");
	gl_ATT_coordinate = glGetAttribLocation(gl_SHA_points, "coordinate");
	gl_ATT_radius = glGetAttribLocation(gl_SHA_points, "radius");


    /// Set up VAO:
    glGenVertexArrays(1, &gl_VAO);
    glBindVertexArray(gl_VAO);

	// Set up the openGL VBOs:
	//glGenBuffers(1, &gl_VBO_cell_location);
	//glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_location);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 32 * 32, nullptr, GL_DYNAMIC_DRAW);

	gl_VBO_cell_location = positions.getVBO(queue);
	glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_location);


	glEnableVertexAttribArray(gl_ATT_coordinate);

	glVertexAttribPointer(
		gl_ATT_coordinate,     // attribute
		3,                   // number of elements per vertex, here (x, y, z)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		sizeof(float) * 4,       // space between values
		nullptr                    // use the vertex buffer object
	);

	glEnableVertexAttribArray(gl_ATT_radius);

	glVertexAttribPointer(
		gl_ATT_radius,     // attribute
		1,                   // number of elements per vertex, here (x, y, z)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		sizeof(float) * 4,       // space between values
		(const GLvoid*)(3*sizeof(GLfloat))                    // use the vertex buffer object
	);


	// Unbind the VAO:
	glBindVertexArray(previous_VAO);
	
}

void position_spheres::render(camera gl_camera, data_buffer::Array<cl_float4>& positions, cl::CommandQueue& queue)
{
	if (has_initialized == false)
	{
		// TODO: remove this ugly and low-performance code!!! The renderer should be initialized before rendering!!!
		initialize(positions, queue);
		//return;
	}

	// RENDER POINTS:
	glUseProgram(gl_SHA_points);
	glBindVertexArray(gl_VAO);

	gl_VBO_cell_location = positions.getVBO(queue);

	glUniformMatrix4fv(gl_UNI_mvp_matrix, 1, GL_FALSE, &gl_camera.get_view_projection_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_mv_matrix, 1, GL_FALSE, &gl_camera.get_view_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_p_matrix, 1, GL_FALSE, &gl_camera.get_projection_matrix()[0][0]);
	glUniform1f(gl_UNI_screen_width, gl_camera.viewport_size.x);
	glDrawArrays(GL_POINTS, 0, positions.used_count());

	//glDrawArrays(GL_POINTS, 0, 1);
}

#include "cells_velocity.h"

using namespace renderer;

#include <iostream>

#include <CL/cl_gl.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "gui/shader_tools.h"

const char * const source_vertex_points = "#version 330 core\n"
"attribute vec3 coordinate;"
"attribute vec3 velocity;"
"varying vec4 g_color;"
"varying vec4 g_velocity;"
"uniform mat4 MVP_matrix;"

"void main(void) {"
	"vec4 screen_position = MVP_matrix * vec4(coordinate, 1.0);"
	"gl_Position = screen_position;"
	"float screen_z = (-screen_position.z + 2.0) * 2.0;"
	//"gl_PointSize = screen_z * 1.0;"
	"gl_PointSize = 2.0f;"
	"g_color = vec4(1.0, screen_z / 100.0 + 0.5, screen_z * 0.001 + 1.0, 1.0);"
	"g_velocity = MVP_matrix * vec4(velocity, 0.0);"
"}";

const char * const source_geometry_points = "#version 330 core\n"
"layout (points) in;"
"layout (line_strip, max_vertices = 2) out;"

"in vec4 g_color[1];"
"in vec4 g_velocity[1];"
"out vec4 f_color;"

"void main(void) {"
	"f_color = g_color[0];"
	
	"gl_Position = gl_in[0].gl_Position;"
	"EmitVertex();"

	"gl_Position = gl_in[0].gl_Position - 2 * g_velocity[0];"
	"EmitVertex();"

	"EndPrimitive();"
"}";

const char * const source_fragment_points = "#version 330 core\n"
"varying vec4 f_color;"

"void main(void) {"
    "gl_FragColor = f_color;"
"}";

cells_velocity::cells_velocity() : render_unit_template()
{
	

	// Test count:
	particle_count = 32 * 32; 	// TODO: remove
}

cells_velocity::~cells_velocity()
{
	// TODO: clear OpenGL objects!
	//dtor
}

error cells_velocity::initialize()
{
	has_initialized = true;

	// Capture previous VAO state:
	GLint previous_VAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_VAO);

	// ===== GRID RENDERER ===== //
	// Set up shaders:
	gl_SHA_points = create_shader_geo(source_vertex_points, source_geometry_points, source_fragment_points);
	
	gl_UNI_mvp_matrix = glGetUniformLocation(gl_SHA_points, "MVP_matrix");
	gl_ATT_coordinate = glGetAttribLocation(gl_SHA_points, "coordinate");
	gl_ATT_velocity = glGetAttribLocation(gl_SHA_points, "velocity");

    /// Set up VAO:
    glGenVertexArrays(1, &gl_VAO);
    glBindVertexArray(gl_VAO);

	// == LOCATION ATTRIBUTE: ==
	//glGenBuffers(1, &gl_VBO_cell_location);
	//glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_location);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 32 * 32, nullptr, GL_DYNAMIC_DRAW);

	gl_VBO_cell_location = get_VBO(0);
	glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_location.VBO);

	glEnableVertexAttribArray(gl_ATT_coordinate);

	glVertexAttribPointer(
		gl_ATT_coordinate,     // attribute
		3,                   // number of elements per vertex, here (x, y, z)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		sizeof(float) * 4,       // space between values
		nullptr                    // use the vertex buffer object
	);
	
	
	// == VELOCITY ATTRIBUTE: ==
	//glGenBuffers(1, &gl_VBO_cell_velocity);
	//glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_velocity);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 32 * 32, nullptr, GL_DYNAMIC_DRAW);

	gl_VBO_cell_velocity = get_VBO(1);
	glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_velocity.VBO);

	glEnableVertexAttribArray(gl_ATT_velocity);

	glVertexAttribPointer(
		gl_ATT_velocity,     // attribute
		3,                   // number of elements per vertex, here (x, y, z)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		sizeof(float) * 4,       // space between values
		nullptr                    // use the vertex buffer object
	);

	// Unbind the VAO:
	glBindVertexArray(previous_VAO);
	
	return error::success;
}

error cells_velocity::render(camera gl_camera)
{
	if (has_initialized == false)
		return error::uninitialized;
	
	// RENDER POINTS:
	glUseProgram(gl_SHA_points);
	
	glBindVertexArray(gl_VAO);

	//glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_location);

	// Get the VBO from "sim_membrane" (index 0).
	gl_VBO_cell_location = get_VBO(0);
	gl_VBO_cell_velocity = get_VBO(1);

	unsigned int size = dependency_pointers[0]->get_buffer_size(0, 0);

	glUniformMatrix4fv(gl_UNI_mvp_matrix, 1, GL_FALSE, &gl_camera.get_view_projection_matrix()[0][0]);
	
	glDrawArrays(GL_POINTS, 0, gl_VBO_cell_location.size / 4);
	
	return error::success;
}

#include "polar_spheres.h"

using namespace renderer;

#include <iostream>

#include <CL/cl_gl.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "gui/shader_tools.h"

const char source_vertex_points[] = {
	#embed "shaders/polar_spheres.vert"
	, '\0'
};

const char source_fragment_points[] = {
	#embed "shaders/polar_spheres.frag"
	, '\0'
};

polar_spheres::polar_spheres() : render_unit_template()
{
	

	// Test count:
	particle_count = 32 * 32 * 8; 	// TODO: remove
}

polar_spheres::~polar_spheres()
{
	// TODO: clear OpenGL objects!
	//dtor
}

error polar_spheres::initialize()
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
	gl_ATT_velocity = glGetAttribLocation(gl_SHA_points, "velocity");
	gl_ATT_division_plane = glGetAttribLocation(gl_SHA_points, "division_plane");

    /// Set up VAO:
    glGenVertexArrays(1, &gl_VAO);
    glBindVertexArray(gl_VAO);

	// Set up the openGL VBOs:
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

	glEnableVertexAttribArray(gl_ATT_radius);

	glVertexAttribPointer(
		gl_ATT_radius,     // attribute
		1,                   // number of elements per vertex, here (x, y, z)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		sizeof(float) * 4,       // space between values
		(const GLvoid*)(3*sizeof(GLfloat))                    // use the vertex buffer object
	);

	// == VELOCITY ATTRIBUTE: ==
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

	// == DIVISION PLANE ATTRIBUTE: ==
	gl_VBO_cell_division_plane = get_VBO(2);
	glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_division_plane.VBO);

	glEnableVertexAttribArray(gl_ATT_division_plane);

	glVertexAttribPointer(
		gl_ATT_division_plane,     // attribute
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

error polar_spheres::render(camera gl_camera)
{
	if (has_initialized == false)
		return error::uninitialized;
	
	// RENDER POINTS:
	glUseProgram(gl_SHA_points);
	glBindVertexArray(gl_VAO);

	//glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_location);

	// Get the VBO from "sim_membrane" (index 0).
	//gl_VBO_cell_location = dependency_pointers[0]->get_buffer_as_VBO_info(0, 0);
	gl_VBO_cell_location = get_VBO(0);
	gl_VBO_cell_velocity = get_VBO(1);
	gl_VBO_cell_division_plane = get_VBO(2);

	//unsigned int size = dependency_pointers[0]->get_buffer_size(0, 0);

	glUniformMatrix4fv(gl_UNI_mvp_matrix, 1, GL_FALSE, &gl_camera.get_view_projection_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_mv_matrix, 1, GL_FALSE, &gl_camera.get_view_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_p_matrix, 1, GL_FALSE, &gl_camera.get_projection_matrix()[0][0]);
	glUniform1f(gl_UNI_screen_width, gl_camera.viewport_size.x);
	glDrawArrays(GL_POINTS, 0, gl_VBO_cell_location.size / 4);
	
	return error::success;
}

#include "polar_spheres.h"

using namespace renderer;

#include <iostream>

#include <CL/cl_gl.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "gui/shader_tools.h"

const char * const source_vertex_points = "#version 330 core\n"
"attribute vec3 coordinate;"
"attribute vec3 velocity;"
"attribute vec3 division_plane;"
"attribute float radius;"
"varying vec4 f_color;"
"varying vec4 division_axis;"
"varying vec4 v1;"
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
	"f_color = vec4(screen_z / 200.0 + 0.5, screen_z * 0.0005 + 1.0, 1.0, 1.0);"
	//"f_color = vec4(min(gl_VertexID / 16.0 + 0.1, 1.0), min(gl_VertexID / 256.0 + 0.1, 1.0), min(gl_VertexID / 256.0 / 256.0 + 0.1, 1.0), 1.0);"
	//"vec4 eye_pos = MV_matrix * vec4(coordinate, 1.0);"
	"v1 = MV_matrix * vec4(velocity, 0.0f);"
	"division_axis = MV_matrix * vec4(division_plane, 0.0f);"
"}";

const char * const source_fragment_points = "#version 330 core\n"
"varying vec4 f_color;"
"varying vec4 v1;"
"varying vec4 division_axis;"

"void main(void) {"
	"if(length(gl_PointCoord - vec2(0.5, 0.5)) > 0.5)"
		"discard;"

	//"vec3 division_axis = vec3(0.0f, 0.0f, 1.0f);"

	"float polarity_angle_size = length(v1.xyz);"
	"if(polarity_angle_size == 0.0f)"
		"discard;"
	"vec3 normalized_v1 = normalize(v1.xyz);"

	"vec2 center_coord = 2.0f*vec2(gl_PointCoord.x - 0.5, - gl_PointCoord.y + 0.5);"

	"vec3 fragment_coord = vec3(center_coord, sqrt(1.0 - center_coord.x*center_coord.x - center_coord.y*center_coord.y));"
	"float polarity_intensity = min(dot(fragment_coord, normalized_v1), 1.0f);"
	"float polarity_intensity_back = min(dot(vec3(fragment_coord.xy, -fragment_coord.z), normalized_v1), 1.0f);"
	"if(polarity_intensity < cos(polarity_angle_size) && polarity_intensity_back < cos(polarity_angle_size) && abs(dot(division_axis.xyz, fragment_coord)) > 0.01f)"
		"discard;"

	"float modifier = 1.0f;"

	"if(polarity_intensity < cos(polarity_angle_size))"
		"modifier = 0.5f;"

	"if(abs(dot(division_axis.xyz, fragment_coord)) <= 0.01f)"
		"modifier = 20.0f;"
	
    "gl_FragColor = (1.0f - 1.0f*length(gl_PointCoord - vec2(0.5, 0.5))) * modifier * f_color;"
"}";

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

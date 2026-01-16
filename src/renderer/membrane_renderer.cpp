#include "main.h"

#include "renderer/membrane_renderer.hpp"

#define GLM_ENABLE_EXPERIMENTAL

#include <CL/cl_gl.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "gui/shader_tools.h"

/*
const char membrane_source_vertex_points[] = {
	#embed "shaders/membrane_renderer.vert"
	, '\0'
};

const char membrane_source_fragment_points[] = {
	#embed "shaders/membrane_renderer.frag"
	, '\0'
};
*/

void checkError() {
	GLenum error = glGetError();
	if ( error != 0) {
		message_debug("Current GL Error (if any):", error);
	} else {
		message_debug("No error");
	}
}

const char * const membrane_source_vertex_points = "#version 330 core\n"
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

const char * const membrane_source_fragment_points = "#version 330 core\n"
"varying vec4 f_color;"

"void main(void) {"
	"if(length(gl_PointCoord - vec2(0.5, 0.5)) > 0.5)"
		"discard;"
	
    "gl_FragColor = (1.0f - 1.0f*length(gl_PointCoord - vec2(0.5, 0.5))) * f_color;"
"}";

MembraneRenderer::MembraneRenderer()
{
	

	// Test count:
	particle_count = 32 * 32 * 8; 	// TODO: remove
}

MembraneRenderer::~MembraneRenderer()
{
	// TODO: clear OpenGL objects!
	//dtor
}

void MembraneRenderer::initialize(ParticleMembraneData& membranes, cl::CommandQueue& queue)
{
	has_initialized = true;

	checkError();

	// Capture previous VAO state:
	GLint previous_VAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_VAO);

	checkError();

	// ===== GRID RENDERER ===== //
	// Set up shaders:
	gl_SHA_points = create_shader(membrane_source_vertex_points, membrane_source_fragment_points);

	checkError();

	gl_UNI_mvp_matrix = glGetUniformLocation(gl_SHA_points, "MVP_matrix");
	gl_UNI_mv_matrix = glGetUniformLocation(gl_SHA_points, "MV_matrix");
	gl_UNI_p_matrix = glGetUniformLocation(gl_SHA_points, "P_matrix");
	gl_UNI_screen_width = glGetUniformLocation(gl_SHA_points, "screen_width");
	gl_ATT_coordinate = glGetAttribLocation(gl_SHA_points, "coordinate");
	gl_ATT_radius = glGetAttribLocation(gl_SHA_points, "radius");

	checkError();


    /// Set up VAO:
    glGenVertexArrays(1, &gl_VAO);
    glBindVertexArray(gl_VAO);

	checkError();

	// Set up the openGL VBOs:
	//glGenBuffers(1, &gl_VBO_cell_location);
	//glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_location);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 32 * 32, nullptr, GL_DYNAMIC_DRAW);

	gl_VBO_cell_location = membranes.getVBO(queue);
	glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_location);

	checkError();


	glEnableVertexAttribArray(gl_ATT_coordinate);

	checkError();

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

	checkError();
	
}

void MembraneRenderer::render(camera gl_camera, ParticleMembraneData& membranes, cl::CommandQueue& queue)
{
	if (has_initialized == false)
	{
		// TODO: remove this ugly and low-performance code!!! The renderer should be initialized before rendering!!!
		initialize(membranes, queue);
		//return;
	}
	//message_debug("Rendering Membrane");

	// RENDER POINTS:
	glUseProgram(gl_SHA_points);
	glBindVertexArray(gl_VAO);

	gl_VBO_cell_location = membranes.getVBO(queue);

	glUniformMatrix4fv(gl_UNI_mvp_matrix, 1, GL_FALSE, &gl_camera.get_view_projection_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_mv_matrix, 1, GL_FALSE, &gl_camera.get_view_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_p_matrix, 1, GL_FALSE, &gl_camera.get_projection_matrix()[0][0]);
	glUniform1f(gl_UNI_screen_width, gl_camera.viewport_size.x);
	glDrawArrays(GL_POINTS, 0, membranes.used_count()*256);
	/*glDrawArraysInstanced(
		GL_POINTS,
		0,          // first
		256,        // vertices per instance
		membranes.used_count()  // instance count
	);*/

	//glDrawArrays(GL_POINTS, 0, 1);
}

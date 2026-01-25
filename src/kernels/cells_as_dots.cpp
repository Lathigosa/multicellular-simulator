#include "kernels/cells_as_dots.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <CL/cl_gl.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "gui/shader_tools.h"

const char source_vertex_points[] = {
	#embed "shaders/vertices_as_spheres.vert"
	,'\0'
};

const char source_fragment_points[] = {
	#embed "shaders/vertices_as_spheres.frag"
	,'\0'
};

position_spheres::position_spheres()
{
	
}

position_spheres::~position_spheres()
{
	// TODO: clear OpenGL objects!
	//dtor
}

void position_spheres::initialize(data_buffer::ParticleData<cl_float4>& positions, cl::CommandQueue& queue)
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

	gl_VBO_cell_location = positions.getOpenGLBuffer(queue);
	glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_location);


	glEnableVertexAttribArray(gl_ATT_coordinate);

	glVertexAttribPointer(
		gl_ATT_coordinate,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 4,
		nullptr
	);

	glEnableVertexAttribArray(gl_ATT_radius);

	glVertexAttribPointer(
		gl_ATT_radius,
		1,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 4,
		(const GLvoid*)(3*sizeof(GLfloat))
	);


	// Unbind the VAO:
	glBindVertexArray(previous_VAO);
	
}

void position_spheres::render(camera gl_camera, data_buffer::ParticleData<cl_float4>& positions, cl::CommandQueue& queue)
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

	gl_VBO_cell_location = positions.getOpenGLBuffer(queue);

	glUniformMatrix4fv(gl_UNI_mvp_matrix, 1, GL_FALSE, &gl_camera.get_view_projection_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_mv_matrix, 1, GL_FALSE, &gl_camera.get_view_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_p_matrix, 1, GL_FALSE, &gl_camera.get_projection_matrix()[0][0]);
	glUniform1f(gl_UNI_screen_width, gl_camera.viewport_size.x);
	glDrawArrays(GL_POINTS, 0, positions.used_count());

	//glDrawArrays(GL_POINTS, 0, 1);
}

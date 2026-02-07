#include "centrosomes_as_dots.hpp"

#define GLM_ENABLE_EXPERIMENTAL

#include <CL/cl_gl.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "gui/shader_tools.h"

const char source_vertex_points[] = {
	#embed "shaders/local_vertices_as_spheres.vert"
	,'\0'
};

const char source_fragment_points[] = {
	#embed "shaders/local_vertices_as_spheres.frag"
	,'\0'
};

CentrosomesAsDotsRenderer::CentrosomesAsDotsRenderer()
{
	
}

CentrosomesAsDotsRenderer::~CentrosomesAsDotsRenderer()
{
	// TODO: clear OpenGL objects!
	//dtor
}

void CentrosomesAsDotsRenderer::initialize(data_buffer::ParticleData<cl_float4>& positions, CentrosomePair& centrosomes, cl::CommandQueue& queue)
{
	has_initialized = true;

	// Capture previous VAO state:
	GLint previous_VAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_VAO);

	// Set up shaders:
	gl_SHA_points = create_shader(source_vertex_points, source_fragment_points);

	gl_UNI_mvp_matrix = glGetUniformLocation(gl_SHA_points, "MVP_matrix");
	gl_UNI_mv_matrix = glGetUniformLocation(gl_SHA_points, "MV_matrix");
	gl_UNI_p_matrix = glGetUniformLocation(gl_SHA_points, "P_matrix");
	gl_UNI_screen_width = glGetUniformLocation(gl_SHA_points, "screen_width");
	gl_ATT_global_coordinate = glGetAttribLocation(gl_SHA_points, "global_coordinate");
    gl_ATT_local_coordinate = glGetAttribLocation(gl_SHA_points, "local_coordinate");
	gl_ATT_radius = glGetAttribLocation(gl_SHA_points, "radius");

    /// Set up VAO:
    glGenVertexArrays(1, &gl_VAO_1);
    glBindVertexArray(gl_VAO_1);

	gl_VBO_cell_location = positions.getOpenGLBuffer(queue);
	glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_location);

	glEnableVertexAttribArray(gl_ATT_global_coordinate);

	glVertexAttribPointer(
		gl_ATT_global_coordinate,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 4,
		nullptr
	);

    gl_VBO_centrosome_location = centrosomes.centrosome_pair_position.getOpenGLBuffer(queue);
    glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_centrosome_location);

    glEnableVertexAttribArray(gl_ATT_local_coordinate);

    glVertexAttribPointer(
		gl_ATT_local_coordinate,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 8,
		nullptr
	);

    glEnableVertexAttribArray(gl_ATT_radius);

	glVertexAttribPointer(
		gl_ATT_radius,
		1,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 8,
		(const GLvoid*)(3*sizeof(GLfloat))
	);

    /// Set up VAO:
    glGenVertexArrays(1, &gl_VAO_2);
    glBindVertexArray(gl_VAO_2);

	gl_VBO_cell_location = positions.getOpenGLBuffer(queue);
	glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_cell_location);

	glEnableVertexAttribArray(gl_ATT_global_coordinate);

	glVertexAttribPointer(
		gl_ATT_global_coordinate,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 4,
		nullptr
	);

    gl_VBO_centrosome_location = centrosomes.centrosome_pair_position.getOpenGLBuffer(queue);
    glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_centrosome_location);

    glEnableVertexAttribArray(gl_ATT_local_coordinate);

    glVertexAttribPointer(
		gl_ATT_local_coordinate,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 8,
		(const GLvoid*)(4*sizeof(GLfloat))
	);

    glEnableVertexAttribArray(gl_ATT_radius);

	glVertexAttribPointer(
		gl_ATT_radius,
		1,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 8,
		(const GLvoid*)(7*sizeof(GLfloat))
	);

	// Unbind the VAO:
	glBindVertexArray(previous_VAO);
	
}

void CentrosomesAsDotsRenderer::render(camera gl_camera, data_buffer::ParticleData<cl_float4>& positions, CentrosomePair& centrosomes, cl::CommandQueue& queue)
{

	if (has_initialized == false)
	{
		// TODO: remove this ugly and low-performance code!!! The renderer should be initialized before rendering!!!
		initialize(positions, centrosomes, queue);
		//return;
	}

	// RENDER POINTS:
	glUseProgram(gl_SHA_points);
	glBindVertexArray(gl_VAO_1);

	gl_VBO_cell_location = positions.getOpenGLBuffer(queue);
    gl_VBO_centrosome_location = centrosomes.centrosome_pair_position.getOpenGLBuffer(queue);

	glUniformMatrix4fv(gl_UNI_mvp_matrix, 1, GL_FALSE, &gl_camera.get_view_projection_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_mv_matrix, 1, GL_FALSE, &gl_camera.get_view_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_p_matrix, 1, GL_FALSE, &gl_camera.get_projection_matrix()[0][0]);
	glUniform1f(gl_UNI_screen_width, gl_camera.viewport_size.x);

	glDrawArrays(GL_POINTS, 0, positions.used_count());

    glBindVertexArray(gl_VAO_2);

    glUniformMatrix4fv(gl_UNI_mvp_matrix, 1, GL_FALSE, &gl_camera.get_view_projection_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_mv_matrix, 1, GL_FALSE, &gl_camera.get_view_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_p_matrix, 1, GL_FALSE, &gl_camera.get_projection_matrix()[0][0]);
	glUniform1f(gl_UNI_screen_width, gl_camera.viewport_size.x);

    glDrawArrays(GL_POINTS, 0, positions.used_count());

	//glDrawArrays(GL_POINTS, 0, 1);
}

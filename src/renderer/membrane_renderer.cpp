#include "main.h"

#include "renderer/membrane_renderer.hpp"

#define GLM_ENABLE_EXPERIMENTAL

#include <CL/cl_gl.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "gui/shader_tools.h"


const char membrane_source_vertex_points[] = {
	#embed "shaders/membrane_renderer.vert"
	, '\0'
};

const char membrane_source_fragment_points[] = {
	#embed "shaders/membrane_renderer.frag"
	, '\0'
};

void checkError() {
	GLenum error = glGetError();
	if ( error != 0) {
		message_debug("Current GL Error (if any):", error);
	} else {
		message_debug("No error");
	}
}

MembraneRenderer::MembraneRenderer() { }

MembraneRenderer::~MembraneRenderer() { }

void MembraneRenderer::initialize(data_buffer::ParticleData<cl_float4> positions, ParticleMembraneData& membranes, cl::CommandQueue& queue)
{
	has_initialized = true;

	checkError();

	// Capture previous VAO state:
	GLint previous_VAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_VAO);

	checkError();

	// Set up shaders:
	gl_SHA_points = create_shader(membrane_source_vertex_points, membrane_source_fragment_points);

	checkError();

	gl_UNI_mvp_matrix = glGetUniformLocation(gl_SHA_points, "MVP_matrix");
	gl_UNI_mv_matrix = glGetUniformLocation(gl_SHA_points, "MV_matrix");
	gl_UNI_p_matrix = glGetUniformLocation(gl_SHA_points, "P_matrix");
	gl_UNI_screen_width = glGetUniformLocation(gl_SHA_points, "screen_width");
	gl_UNI_vertices_per_cell = glGetUniformLocation(gl_SHA_points, "vertices_per_cell");

	checkError();

    /// Set up VAO:
    glGenVertexArrays(1, &gl_VAO);
    glBindVertexArray(gl_VAO);

	checkError();

	gl_SSBO_vertex_location = membranes.vertex_buffer.getOpenGLBuffer(queue);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_SSBO_vertex_location);

	checkError();

	gl_SSBO_cell_location = positions.getOpenGLBuffer(queue);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_SSBO_cell_location);

	checkError();

	gl_IBO_triangles = membranes.triangles_index_buffer.getOpenGLBuffer(queue);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_IBO_triangles);

	gl_IBO_edges = membranes.edges_index_buffer.getOpenGLBuffer(queue);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_IBO_edges);

	checkError();

	// Unbind the VAO:
	glBindVertexArray(previous_VAO);

	checkError();
	
}

void MembraneRenderer::render(camera gl_camera, data_buffer::ParticleData<cl_float4> positions, ParticleMembraneData& membranes, cl::CommandQueue& queue)
{
	//assert(has_initialized == true);
	// TODO: remove this ugly and low-performance code!!! The renderer should be initialized before rendering!!!
	if (has_initialized == false) initialize(positions, membranes, queue);
	//message_debug("Rendering Membrane");

	// RENDER POINTS:
	glUseProgram(gl_SHA_points);
	glBindVertexArray(gl_VAO);

	// Refresh the vertex buffer:
	gl_SSBO_vertex_location = membranes.vertex_buffer.getOpenGLBuffer(queue);
	gl_IBO_triangles = membranes.triangles_index_buffer.getOpenGLBuffer(queue);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gl_SSBO_vertex_location);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gl_SSBO_cell_location);

	// Pass viewport uniform matrices:
	glUniformMatrix4fv(gl_UNI_mvp_matrix, 1, GL_FALSE, &gl_camera.get_view_projection_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_mv_matrix, 1, GL_FALSE, &gl_camera.get_view_matrix()[0][0]);
	glUniformMatrix4fv(gl_UNI_p_matrix, 1, GL_FALSE, &gl_camera.get_projection_matrix()[0][0]);
	glUniform1f(gl_UNI_screen_width, gl_camera.viewport_size.x);
	glUniform1ui(gl_UNI_vertices_per_cell, membranes.max_vertices_per_cell);

	//glDrawArrays(GL_POINTS, 0, 256*membranes.vertex_buffer.used_count());

	// Issue the draw command:
	/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_IBO_triangles);
	glDrawElementsInstanced(
		GL_TRIANGLES,
		3*membranes.max_triangles_per_cell,
		GL_UNSIGNED_INT,
		nullptr,
		membranes.vertex_buffer.used_count()
	);*/

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_IBO_edges);
	glDrawElementsInstanced(
		GL_LINES,
		2*membranes.max_edges_per_cell,
		GL_UNSIGNED_INT,
		nullptr,
		membranes.vertex_buffer.used_count()
	);
}

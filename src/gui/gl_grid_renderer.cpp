#include "gl_grid_renderer.h"
#include "shader_tools.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Shader code:
const char source_vertex_grid_2d[] = {
	#embed "shaders/grid.vert"
	, '\0'
};

const char source_fragment_grid_2d[] = {
	#embed "shaders/grid.frag"
	, '\0'
};

const char source_vertex_axes[] = {
	#embed "shaders/axes.vert"
	, '\0'
};

const char source_fragment_axes[] = {
	#embed "shaders/axes.frag"
	, '\0'
};

const float grid_square[4*12] = {
	0.01, -1.0, 0.0, 0.0,
	-0.01, -1.0, 0.0, 0.0,
	-0.01, 0.0, 0.0, 0.0,

	-0.01, 0.0, 0.0, 0.0,
	0.01, -1.0, 0.0, 0.0,
	0.01, 0.0, 0.0, 0.0,

	0.0, -0.01, 0.0, 0.0,
	0.0, 0.01, 0.0, 0.0,
	1.0, 0.01, 0.0, 0.0,

	1.0, 0.01, 0.0, 0.0,
	0.0, -0.01, 0.0, 0.0,
	1.0, -0.01, 0.0, 0.0
};

const float grid_square_line[4*4] = {
	0.0, -1.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	1.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
};

const float axis_vertices[4*32] = {
	-1.0, 0.0, 0.0, -1.0,		// x-axis;
	1.0, 0.0, 0.0, 1.0,
	0.0, -1.0, 0.0, -1.0,	// y-axis;
	0.0, 1.0, 0.0, 1.0,
	0.0, 0.0, -1.0, -1.0,	// z-axis;
	0.0, 0.0, 1.0, 1.0
};		// z-axis;

gl_grid_renderer::gl_grid_renderer()
{

}

gl_grid_renderer::~gl_grid_renderer()
{
	//dtor
}

void gl_grid_renderer::init()
{
	has_initialized = true;

	// Capture previous VAO state:
	GLint previous_VAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_VAO);

	// ===== GRID RENDERER ===== //
	// Set up shaders:
	gl_SHA_grid_2d = create_shader(source_vertex_grid_2d, source_fragment_grid_2d);

	gl_UNI_mvp_matrix = glGetUniformLocation(gl_SHA_grid_2d, "MVP_matrix");
	gl_ATT_coordinate = glGetAttribLocation(gl_SHA_grid_2d, "coordinate");
	gl_UNI_grid_repeat = glGetUniformLocation(gl_SHA_grid_2d, "grid_repeat");
	gl_UNI_grid_shift = glGetUniformLocation(gl_SHA_grid_2d, "grid_shift");
	gl_UNI_grid_color = glGetUniformLocation(gl_SHA_grid_2d, "grid_color");

	// Set up VAO:
    glGenVertexArrays(1, &gl_VAO_grid);
    glBindVertexArray(gl_VAO_grid);

    // Set up the openGL VBOs:
    glGenBuffers(1, &gl_VBO_grid_line);
	glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_grid_line);

	// Set data size of the line (xyzw * 2 vertices), we don't care about the data:
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 12, grid_square, GL_STATIC_DRAW);

	// Vertex attribute array:
	glEnableVertexAttribArray(gl_ATT_coordinate);

	glVertexAttribPointer(
		gl_ATT_coordinate,     // attribute
		3,                   // number of elements per vertex, here (x, y, z)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		sizeof(float) * 4,       // space between values
		nullptr                    // use the vertex buffer object
	);

	// ===== AXES RENDERER ===== //
	// Set up shaders:
	gl_SHA_axes = create_shader(source_vertex_axes, source_fragment_axes);

	gl_UNI_axes_mvp_matrix = glGetUniformLocation(gl_SHA_axes, "MVP_matrix");
	gl_ATT_axes_coordinate = glGetAttribLocation(gl_SHA_axes, "coordinate");

	// Set up VAO:
    glGenVertexArrays(1, &gl_VAO_axes);
    glBindVertexArray(gl_VAO_axes);

    // Set up the openGL VBOs:
    glGenBuffers(1, &gl_VBO_axes_line);
	glBindBuffer(GL_ARRAY_BUFFER, gl_VBO_axes_line);

	// Set data size of the line (xyzw * 2 vertices), we don't care about the data:
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 6, axis_vertices, GL_STATIC_DRAW);

	// Vertex attribute array:
	glEnableVertexAttribArray(gl_ATT_axes_coordinate);

	glVertexAttribPointer(
		gl_ATT_axes_coordinate,     // attribute
		4,                   // number of elements per vertex, here (x, y, z, other)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		sizeof(float) * 4,       // space between values
		nullptr                    // use the vertex buffer object
	);

	// Unbind the VAO:
	glBindVertexArray(previous_VAO);
}

void gl_grid_renderer::render(camera gl_camera)
{
	if (has_initialized == false)
		return;

	glUseProgram(gl_SHA_grid_2d);
	glBindVertexArray(gl_VAO_grid);

	glm::mat4 gl_MAT_mvp = gl_camera.get_view_projection_matrix();
	gl_MAT_mvp = glm::scale(gl_MAT_mvp, glm::vec3(10.0, 10.0, 10.0));									// Set uniform matrix.

	//gl_MAT_mvp = glm::rotate(gl_MAT_mvp, 20.0f, glm::vec3(1,0,0));	// Rotate the matrix.

	// Update camera:
	glUniformMatrix4fv(gl_UNI_mvp_matrix, 1, GL_FALSE, &gl_MAT_mvp[0][0]);
	glUniform2i(gl_UNI_grid_repeat, 32, 32);
	glUniform2f(gl_UNI_grid_shift, 16, 16);
	glUniform4f(gl_UNI_grid_color, grid_color.x, grid_color.y, grid_color.z, grid_color.w);
	
	glDrawArraysInstanced(GL_TRIANGLES, 0, 12*4, 32*32);

	// Render lines:
	//glUniform2i(gl_UNI_grid_repeat, 32, 32);
	//glUniform2f(gl_UNI_grid_shift, 16, 16);
	//glUniform4f(gl_UNI_grid_color, 1.0, 0.0, 0.0, 0.0);
	
	//glDrawArraysInstanced(GL_LINES, 0, 4*4, 32*32);

	// Re-render small:
	gl_MAT_mvp = gl_camera.get_view_projection_matrix();
	gl_MAT_mvp = glm::scale(gl_MAT_mvp, glm::vec3(1.0, 1.0, 1.0));									// Set uniform matrix.
	glUniform2i(gl_UNI_grid_repeat, 80, 80);

	// Move grid based on camera target position:

	glUniform2f(gl_UNI_grid_shift, 40 - glm::floor(gl_camera.camera_target.x / 1.0f), 39 - glm::floor(gl_camera.camera_target.y / 1.0f));
	glUniformMatrix4fv(gl_UNI_mvp_matrix, 1, GL_FALSE, &gl_MAT_mvp[0][0]);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 12*4, 80*80);

	// Render axes:
	glUseProgram(gl_SHA_axes);
	glBindVertexArray(gl_VAO_axes);

	glUniformMatrix4fv(gl_UNI_axes_mvp_matrix, 1, GL_FALSE, &gl_MAT_mvp[0][0]);

	glDrawArrays(GL_LINES, 0, 6*4);

}

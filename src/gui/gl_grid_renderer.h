#ifndef GL_GRID_RENDERER_H
#define GL_GRID_RENDERER_H

#include <epoxy/gl.h>
#include "utilities/camera.h"
#include <glm/glm.hpp>

class gl_grid_renderer
{
	public:
		gl_grid_renderer();
		virtual ~gl_grid_renderer();

		// Grid renderer:
		GLuint		gl_VBO_grid_line = 0;					// The VBO used for rendering grid lines.
        GLuint		gl_SHA_grid_2d = 0;						// The shader used for rendering grid lines (should be used instanced).
        GLuint		gl_UNI_mvp_matrix = 0;					// The uniform pointer to the mvp matrix uniform.
        GLuint		gl_UNI_grid_repeat = 0;					// The uniform pointer to the ivec2 uniform indicating the amount of repeats of the grid.
        GLuint		gl_UNI_grid_shift = 0;
        GLuint		gl_UNI_grid_color = 0;
        GLuint		gl_ATT_coordinate = 0;					// Coordinate attribute.
        GLuint		gl_VAO_grid = 0;						// The grid VAO.

        // Axes renderer:
        GLuint		gl_SHA_axes = 0;
        GLuint		gl_UNI_axes_mvp_matrix = 0;
        GLuint		gl_ATT_axes_coordinate = 0;
        GLuint		gl_VAO_axes = 0;
        GLuint		gl_VBO_axes_line = 0;

        glm::vec4	grid_color = glm::vec4(0.3, 0.3, 0.3, 1.0);							// The color of the grid.

		void init();
		void render(camera gl_camera);


	protected:

	private:
		bool has_initialized = false;
};

#endif // GL_GRID_RENDERER_H

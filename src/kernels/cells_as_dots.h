#ifndef KERNEL_CELLS_AS_DOTS_H
#define KERNEL_CELLS_AS_DOTS_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include "core/data_variable.h"

#include "utilities/camera.h"

//namespace renderer
//{


class position_spheres // : public render_unit_template
{
	public:
		//RENDER_DEFINITION("cells_as_dots",  {"position"} )

		position_spheres();
		virtual ~position_spheres();

		void initialize(data_buffer::ParticleData<cl_float4>& positions, cl::CommandQueue& queue);

		void render(camera gl_camera, data_buffer::ParticleData<cl_float4>& positions, cl::CommandQueue& queue);		//
	protected:
	private:
		// Grid renderer:
		//GLuint		gl_VBO_cell_location;				// The VBO used for rendering grid lines.
		GLuint		gl_VBO_cell_location;
	    GLuint		gl_SHA_points;						// The shader used for rendering grid lines (should be used instanced).
	    GLuint		gl_UNI_mvp_matrix;					// The uniform pointer to the mvp matrix uniform.
		GLuint		gl_UNI_mv_matrix;					// The uniform pointer to the model-view matrix uniform.
		GLuint		gl_UNI_p_matrix;					// The uniform pointer to the projection matrix uniform.
		GLuint		gl_UNI_screen_width;				// The uniform pointer to the screen width float.
		GLuint		gl_ATT_coordinate;					// Coordinate attribute.
		GLuint		gl_ATT_radius;						// Radius attribute.
	    GLuint		gl_VAO;								// The grid VAO.

		bool has_initialized;
};

//}

#endif // KERNEL_CELLS_AS_DOTS_H

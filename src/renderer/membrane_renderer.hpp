#ifndef MEMBRANE_RENDERER_HPP
#define MEMBRANE_RENDERER_HPP

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include "cell_simulator/membrane_particle.hpp"

#include "utilities/camera.h"

//namespace renderer
//{


class MembraneRenderer // : public render_unit_template
{
	public:
		//RENDER_DEFINITION("cells_as_dots",  {"position"} )

		MembraneRenderer();
		virtual ~MembraneRenderer();

		void initialize(data_buffer::ParticleData<cl_float4> positions, ParticleMembraneData& membranes, cl::CommandQueue& queue);

		void render(camera gl_camera, data_buffer::ParticleData<cl_float4> positions, ParticleMembraneData& membranes, cl::CommandQueue& queue);		//
	protected:
	private:
		// Grid renderer:
		//GLuint		gl_VBO_cell_location;				// The VBO used for rendering grid lines.
		GLuint		gl_SSBO_vertex_location;
		GLuint		gl_SSBO_cell_location;
		GLuint		gl_IBO_triangles;
		GLuint      gl_IBO_edges;
		GLuint		gl_IBO_edge_neighbors;
		GLuint		gl_UNI_vertices_per_cell;
	    GLuint		gl_SHA_points;						// The shader used for rendering grid lines (should be used instanced).
	    GLuint		gl_UNI_mvp_matrix;					// The uniform pointer to the mvp matrix uniform.
		GLuint		gl_UNI_mv_matrix;					// The uniform pointer to the model-view matrix uniform.
		GLuint		gl_UNI_p_matrix;					// The uniform pointer to the projection matrix uniform.
		GLuint		gl_UNI_screen_width;				// The uniform pointer to the screen width float.
		GLuint		gl_ATT_coordinate;					// Coordinate attribute.
		GLuint		gl_ATT_radius;						// Radius attribute.
	    GLuint		gl_VAO;								// The grid VAO.

		unsigned int particle_count;

		bool has_initialized = false;
};

//}

#endif // MEMBRANE_RENDERER_HPP

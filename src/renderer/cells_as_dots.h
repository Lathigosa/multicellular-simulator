#ifndef CELLS_AS_DOTS_H
#define CELLS_AS_DOTS_H

#include "main.h"

#include "core/sim_unit_template.h"
#include "core/render_unit_template.h"

#include <glm/glm.hpp>
#include <vector>

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

namespace renderer
{

	//struct membrane_particle
	//{
	//	glm::vec3 position;
	//	glm::vec3 normal;

	//	float surface_area;								// The surface area that this piece of membrane represents. Is used to determine spring sizes.

	//	std::vector<unsigned short> connected_to;		// The particles that this membrane particle is connected to.
	//};

	class cells_as_dots : public render_unit_template
	{
		public:
			RENDER_DEFINITION("cells_as_dots",  {"position"} )

			cells_as_dots();
			virtual ~cells_as_dots();

			error initialize() override;

			error render(camera gl_camera) override;		//
		protected:
		private:
			// Grid renderer:
			//GLuint		gl_VBO_cell_location;				// The VBO used for rendering grid lines.
			simulation::VBO_info	gl_VBO_cell_location;
		    GLuint		gl_SHA_points;						// The shader used for rendering grid lines (should be used instanced).
		    GLuint		gl_UNI_mvp_matrix;					// The uniform pointer to the mvp matrix uniform.
			GLuint		gl_UNI_mv_matrix;					// The uniform pointer to the model-view matrix uniform.
			GLuint		gl_UNI_p_matrix;					// The uniform pointer to the projection matrix uniform.
			GLuint		gl_UNI_screen_width;				// The uniform pointer to the screen width float.
			GLuint		gl_ATT_coordinate;					// Coordinate attribute.
			GLuint		gl_ATT_radius;						// Radius attribute.
		    GLuint		gl_VAO;								// The grid VAO.

			unsigned int particle_count;

			bool has_initialized;
	};

}

#endif // CELLS_AS_DOTS_H

#ifndef GUI_WIDGET_CELL_VIEW_H
#define GUI_WIDGET_CELL_VIEW_H

#include <epoxy/gl.h>

#include "main.h"
#include <CL/cl2.hpp>
#include <memory>



#include <gtkmm/glarea.h>
#include <gtkmm/hvbox.h>

// Math toolkit:
#include <glm/glm.hpp>          // Can be installed using: "sudo apt-get install libglm-dev".


#include "utilities/camera.h"

#include "simulator/membrane.h"
#include "core/simulation_collection.h"
#include "core/simulation_file_object.h"

// Peripheral renderers:
#include "gui/gl_grid_renderer.h"

class gui_widget_cell_view : public Gtk::VBox
{
    public:
        gui_widget_cell_view(simulation_file_object& ref);
        gui_widget_cell_view(const gui_widget_cell_view& from) = delete;

        virtual ~gui_widget_cell_view();

		void initialize_renderer();					// Must be called after loading new renderers!
    protected:
        Gtk::GLArea gl_area;

        void realize();
        void unrealize();
        bool render(const Glib::RefPtr<Gdk::GLContext>& /* context */);
        void resize(int width, int height);

        bool scroll_event(GdkEventScroll* event);
		bool button_press_event(GdkEventButton *event);
		bool motion_notify_event(GdkEventMotion* event);
		bool button_release_event(GdkEventButton *event);

		bool timer_event();
		bool timer_event_framerate();


    private:
		// Boolean that makes sure the opencl code does not block rendering:
		bool has_rendered = false;
		unsigned int framerate_counter = 0;
		unsigned int simulation_step_counter = 0;

		// How fast the simulation goes:
		unsigned int steps_per_frame = 8;
		
    	// Control-related variables:
    	camera		gl_camera;							// The camera object containing the MVP matrix and data.
    	camera		gl_camera_previous;					// The camera object before moving it (used in the camera manipulations).
    	enum {
			no_movement,
			rotation,
			translation,
			zoom
    	} camera_transform_mode = no_movement;

    	double previous_cursor_x;						// Variable for keeping track of the mouse movement.
    	double previous_cursor_y;						// Variable for keeping track of the mouse movement.

        // OpenGL-related variables:
        GLuint		gl_FRA_framebuffer;					// The main frame buffer.
        GLuint		gl_TEX_framebuffer;					// The frame buffer texture.
        GLuint		gl_REN_renderbuffer;				// The main render buffer.

		bool bool_render_points = true;					// The bool indicating whether or not it should render points.
		bool bool_render_grid = true;					// The bool indicating whether or not it should render the grid.

        void render_grid();								// Renders the grid.

        // Peripheral renderers:
        gl_grid_renderer grid_renderer;

		// TEST:
		cl::Platform default_platform;
		cl::Device default_device;
		cl::Context opencl_context;
		cl::CommandQueue queue;
		//simulation::membrane membrane;
		simulation::simulation_world sandbox;
		//std::unique_ptr<simulation_file_object> sfo;

		// The currently opened project:
		simulation_file_object& sfo;
};

#endif // GUI_WIDGET_CELL_VIEW_H

#include "gui_widget_cell_view.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <iostream>
#include <exception>

#include "simulator/membrane.h"

#include "utilities/safe_pointer.h"
#include <vector>

#include <glibmm.h>

#include "gui/shader_tools.h"

gui_widget_cell_view::gui_widget_cell_view(simulation_file_object& ref) : sandbox(),
																		sfo(ref)
{
	// Enable mouse-events:
    gl_area.add_events(Gdk::BUTTON_PRESS_MASK);
    gl_area.add_events(Gdk::POINTER_MOTION_MASK);
    gl_area.add_events(Gdk::BUTTON_RELEASE_MASK);
    gl_area.add_events(Gdk::SCROLL_MASK);

    // Connect gl area signals
    gl_area.signal_realize().connect(sigc::mem_fun(*this, &gui_widget_cell_view::realize));
    // Important that the unrealize signal calls our handler to clean up
    // GL resources _before_ the default unrealize handler is called (the "false")
    gl_area.signal_unrealize().connect(sigc::mem_fun(*this, &gui_widget_cell_view::unrealize), false);
    gl_area.signal_render().connect(sigc::mem_fun(*this, &gui_widget_cell_view::render), false);
    gl_area.signal_resize().connect(sigc::mem_fun(*this, &gui_widget_cell_view::resize));

    gl_area.signal_scroll_event().connect(sigc::mem_fun(*this, &gui_widget_cell_view::scroll_event));
    gl_area.signal_button_press_event().connect(sigc::mem_fun(*this, &gui_widget_cell_view::button_press_event));
    gl_area.signal_motion_notify_event().connect(sigc::mem_fun(*this, &gui_widget_cell_view::motion_notify_event));
    gl_area.signal_button_release_event().connect(sigc::mem_fun(*this, &gui_widget_cell_view::button_release_event));

    // Setup timer:
    sigc::slot<bool> timer_slot = sigc::mem_fun(*this, &gui_widget_cell_view::timer_event);
    Glib::signal_timeout().connect(timer_slot, 16);

	sigc::slot<bool> timer_slot_framerate = sigc::mem_fun(*this, &gui_widget_cell_view::timer_event_framerate);
    Glib::signal_timeout().connect(timer_slot_framerate, 1000);


    gl_area.set_hexpand(true);
    gl_area.set_vexpand(true);
    //gl_area.set_auto_render(true);

    add(gl_area);
    gl_area.show();

    // This is necessary to prevent GPU-related crashes when the widget size becomes 0:
    set_size_request(16, 16);

	// Set the required version to OpenGL version 4.0:
	gl_area.set_required_version(4, 0);







}

gui_widget_cell_view::~gui_widget_cell_view()
{
    //dtor
}

bool gui_widget_cell_view::button_press_event(GdkEventButton* event)
{
	// TODO: make the controls adjustable in a menu.
	if(event->type == GDK_BUTTON_PRESS && event->button == 2)		// If it is the middle button;
	{
		gl_camera_previous = gl_camera;
		if(event->state & GDK_CONTROL_MASK)
		{
			camera_transform_mode = zoom;
		} else if(event->state & GDK_SHIFT_MASK)
		{
			camera_transform_mode = translation;
		} else
		{
			camera_transform_mode = rotation;
		}
		previous_cursor_x = event->x;
		previous_cursor_y = event->y;
	}

	return true;
}

bool gui_widget_cell_view::scroll_event(GdkEventScroll* event)
{
	// TODO: make the controls adjustable in a menu.
	float delta_y = 0.0f;

	if(event->direction == GDK_SCROLL_UP)
		delta_y = -1.0f;
	else if(event->direction == GDK_SCROLL_DOWN)
		delta_y = 1.0f;
	else if(event->direction == GDK_SCROLL_SMOOTH)
		delta_y = event->delta_y;

	gl_camera_previous = gl_camera;
	if(event->state & GDK_CONTROL_MASK)			// Change clip
	{
		gl_camera.far_clip += delta_y;
	} else if(event->state & GDK_SHIFT_MASK)	// Change clip
	{
		gl_camera.near_clip += delta_y;
	} else					// Zoom
	{
		float distance = gl_camera_previous.get_target_distance();
		gl_camera.set_target_distance(distance + delta_y * distance * 0.1f);
	}

	return true;
}

bool gui_widget_cell_view::motion_notify_event(GdkEventMotion* event)
{
	double delta_x = (previous_cursor_x - event->x) * 0.005;
	double delta_y = (previous_cursor_y - event->y) * 0.005;

	if(camera_transform_mode == zoom)		// If it is the middle button;
	{
		float distance = gl_camera_previous.get_target_distance();
		gl_camera.set_target_distance(distance - delta_y * 100);
		//gl_area.queue_draw();

	} else if(camera_transform_mode == translation)		// If it is the middle button;
	{
		gl_camera.translate_target(glm::vec3(delta_x * 0.2, -delta_y * 0.2, 0.0));

		previous_cursor_x = event->x;
		previous_cursor_y = event->y;

		//gl_area.queue_draw();
	} else if(camera_transform_mode == rotation)		// If it is the middle button;
	{
		gl_camera.rotate_around_target(glm::vec2(delta_x, delta_y));

		//gl_area.queue_draw();

		previous_cursor_x = event->x;
		previous_cursor_y = event->y;
	}



	return true;
}

bool gui_widget_cell_view::timer_event()
{
	// Make sure that we don't start too many calculations:
	
	
	if(has_rendered == false)
		return true;

	has_rendered = false;
	
	for(int a=0; a<steps_per_frame; a++)
	{
		//sfo.world->simulate_all();
		//sfo.world->running = true;
		simulation_step_counter++;
	}
	//sfo.world->finish();

	gl_area.queue_draw();
	return true;
}

bool gui_widget_cell_view::timer_event_framerate()
{
	// Set the steps_per_frame (speed):
	//if (framerate_counter > 50)
	//	steps_per_frame = steps_per_frame + 1;
	
	//if (framerate_counter < 30)
	//	steps_per_frame = steps_per_frame - 1;

	//if (steps_per_frame < 1)
	//	steps_per_frame = 1;
	
	// Calculate and display framerate:
	//message_notify("Current framerate: " << (framerate_counter) << " per second.");
	//message_notify("Current simulation step rate: " << (simulation_step_counter) << " per second.");
	framerate_counter = 0;
	simulation_step_counter = 0;
	
	return true;
}

bool gui_widget_cell_view::button_release_event(GdkEventButton* event)
{
	if(event->type == GDK_BUTTON_RELEASE && event->button == 2)		// If it is the middle button;
	{
		camera_transform_mode = no_movement;
	}

	return true;
}

void gui_widget_cell_view::realize()
{
    gl_area.make_current();
    try
    {
        gl_area.throw_if_error();

        grid_renderer.init();

        /// Set up MSAA framebuffer:
        glGenFramebuffers(1, &gl_FRA_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gl_FRA_framebuffer);

        // Create the multisampled color texture:
		glGenTextures(1, &gl_TEX_framebuffer);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gl_TEX_framebuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB, gl_area.get_width(), gl_area.get_height(), GL_TRUE);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, gl_TEX_framebuffer, 0);
		
        // Create the also multisampled renderbuffer object for depth and stencil attachments:
		glGenTextures(1, &gl_REN_renderbuffer);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gl_REN_renderbuffer);
		
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_DEPTH_COMPONENT, gl_area.get_width(), gl_area.get_height(), GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D_MULTISAMPLE, gl_REN_renderbuffer, 0);
		
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			message_error("FRAMEBUFFER:: Framebuffer is not complete!");
			exit(1);
		}
		

        /// Set up OpenGL properties:
		//glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_PROGRAM_POINT_SIZE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

        /// Set camera properties:
		gl_camera.camera_position = glm::vec3(0.0, 0.001, 100.0);
		gl_camera.far_clip = 1000.0f;
		gl_camera.near_clip = 10.0f;
    }
    catch(const Gdk::GLError& gle)
    {
        message_error("An error occurred during realize: gui_widget_cell_view.\nDetails: " << gle.domain() << "\nCode: " << gle.code() << "\nDescription: " << gle.what());
    }
    //set_realized();
}

void gui_widget_cell_view::initialize_renderer()
{
	gl_area.make_current();
    try
    {
        gl_area.throw_if_error();

        sfo.renderer->init();

		
		
		sfo.world->initialize_render();
		
    }
    catch(const Gdk::GLError& gle)
    {
        message_error("An error occurred during initialize_renderer: gui_widget_cell_view.\nDetails: " << gle.domain() << "\nCode: " << gle.code() << "\nDescription: " << gle.what());
    }
	
}

void gui_widget_cell_view::unrealize()
{
    gl_area.make_current();
    try
    {
        gl_area.throw_if_error();

        // Delete buffers and program
        //glDeleteBuffers(1, &m_Vao);
        //glDeleteProgram(m_Program);
    }
    catch(const Gdk::GLError& gle)
    {
        message_error("An error occurred during unrealize: gui_widget_cell_view.\nDetails: " << gle.domain() << "\nCode: " << gle.code() << "\nDescription: " << gle.what());
    }
}

bool gui_widget_cell_view::render(const Glib::RefPtr<Gdk::GLContext>& this_context)
{
    try
    {
		has_rendered = true;
		framerate_counter++;
		
        gl_area.throw_if_error();
		
        // Initialize multisampling:
        gl_area.attach_buffers();
		GLint framebuffer_standard;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer_standard);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl_FRA_framebuffer);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// TODO: call the next two lines only ONCE after framebuffer creation.
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		
		// Render everything:
		if (bool_render_grid == true)
			grid_renderer.render(gl_camera);

		sfo.renderer->render_all(gl_camera);
		sfo.world->render_all(gl_camera);

		// Perform multisampling:
		
		
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gl_FRA_framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_standard);
		glBlitFramebuffer(0, 0, gl_area.get_width(), gl_area.get_height(), 0, 0, gl_area.get_width(), gl_area.get_height(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glFlush();

        return true;
    }
    catch(const Gdk::GLError& gle)
    {
		message_error("An error occurred during render callback of GLArea: gui_widget_cell_view.\nDetails: " << gle.domain() << "\nCode: " << gle.code() << "\nDescription: " << gle.what());
        return false;
    }
}

void gui_widget_cell_view::resize(int width, int height)
{
    //gl_area.make_current();
    try
    {
        gl_area.throw_if_error();

        glViewport(0, 0, width, height);

        // Resize the framebuffer:
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl_FRA_framebuffer);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gl_TEX_framebuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, gl_TEX_framebuffer, 0);

        // Resize the renderbuffer:
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gl_REN_renderbuffer);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT, gl_area.get_width(), gl_area.get_height(), GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D_MULTISAMPLE, gl_REN_renderbuffer, 0);
		
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // Set camera properties:
        gl_camera.aspect_ratio = float(width) / float(height);
		gl_camera.viewport_size = glm::vec2(float(width), float(height));
    }
    catch(const Gdk::GLError& gle)
    {
		message_error("An error occurred during resize: gui_widget_cell_view.\nDetails: " << gle.domain() << "\nCode: " << gle.code() << "\nDescription: " << gle.what());
    }
}

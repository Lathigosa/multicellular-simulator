#include <epoxy/gl.h>

#include "main.h"

#include <ctype.h>

//#define GLEW_STATIC
//#include <GL/glew.h> // install libgl by running "sudo apt-get install libgl-dev" or similar.


#include <CL/cl2.hpp>

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <clocale>
//#include <gtkmm-3.0/gtkmm.h>    // If it does not compile, run "sudo apt-get install libgtkmm-3.0-dev" in the command line to install the necessary dependencies.
// Also remember to set settings>compiler>global compiler settings>toolchain executables>c compiler to "g++" instead of "gcc".
// To install g++, run "sudo apt-get install g++"
#include <gtkmm-3.0/gtkmm/application.h>
#include <gtkmm-3.0/gtkmm/window.h>

#include "gui/gui_window_main.h"

#include "resources/strings.h"
#include "resources/opencl_kernels.h"

//#include "gui/gui_plane_cell_view.h"

// TEST:
#include "core/simulation_file_object.h"

// PYBIND11 test:
#include <pybind11/pybind11.h>

int main (int argc, char *argv[])
{
	
    std::cout << res::str::program_title << std::endl;

    ///////////// Test: /////////////
    //simulation_file_object opened_file("test_lua_code.lua");
    /////////////////////////////////

    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, nullptr);         // Hide any unnecessary warnings during startup.

    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, __APPLICATION_ID);

    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, nullptr);        // Show warnings again after startup.



    gui_window main_window;

	// Reset locale to standard:
	setlocale(LC_NUMERIC, "C");
	
    return app->run(main_window);
}

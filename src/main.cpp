#include <epoxy/gl.h>

#include "main.h"

#include <CL/opencl.hpp>

#include <stdlib.h>
#include <iostream>
#include <clocale>
#include <gtkmm-3.0/gtkmm/application.h>
#include <gtkmm-3.0/gtkmm/window.h>

#include "gui/gui_window_main.h"

#include "resources/strings.h"

// PYBIND11 test:
#include <pybind11/pybind11.h>

#include <iostream>
#include <exception>
#include <stdexcept>
#include <csignal>
#include <cstdlib>
#include <stacktrace> // Assuming C++23 support
#include <execinfo.h>

/**
 * @brief Callback to ignore GTK warning messages.
 *
 * This function matches the signature required by `GLogFunc` and can be
 * used with `g_log_set_handler` to suppress GTK warnings of a
 * specified log level. It intentionally does nothing.
 *
 * @param log_domain The log domain of the message (ignored).
 * @param log_level The severity of the message (ignored).
 * @param message The warning message string (ignored).
 * @param user_data Pointer to user data passed to the handler (ignored).
 *
 * @note This is a safe alternative to casting an incompatible function
 * pointer like `gtk_false` to `GLogFunc`. Using a cast in that case
 * would invoke undefined behavior and could crash your program.
 */
void ignore_warning_log(const char *log_domain,
                        GLogLevelFlags log_level,
                        const char *message,
                        void *user_data)
{
    (void)log_domain;
    (void)log_level;
    (void)message;
    (void)user_data;
}

void signalHandler(int signal) {
    std::cerr << "Received signal: " << signal << std::endl;

    // Allocate array to hold stack trace addresses
    void *array[10];
    size_t size;

    // Get stack trace
    size = backtrace(array, 50);
    std::cerr << "Stack trace (most recent call first):" << std::endl;
    backtrace_symbols_fd(array, size, STDERR_FILENO);

    std::exit(EXIT_FAILURE); // Exit after handling the signal
}

/**
 * @brief Entry point of the application.
 *
 * This function initializes and runs the GTK application GUI.
 *
 * @param argc The number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return Exit code of the application (from `Gtk::Application::run`).
 */
int main (int argc, char *argv[])
{
    // Register signal handlers
    std::signal(SIGSEGV, signalHandler); // Segmentation Fault
    std::signal(SIGABRT, signalHandler); // Abort signal
    std::signal(SIGINT, signalHandler);  // Interrupt signal (Ctrl+C)
	
    std::cout << res::str::program_title << std::endl;

    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, ignore_warning_log, nullptr);         // Hide any unnecessary warnings during startup.

    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, __APPLICATION_ID);

    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, nullptr);        // Show warnings again after startup.

    gui_window main_window;

	// Reset locale to standard:
	setlocale(LC_NUMERIC, "C");
	
    return app->run(main_window);
}

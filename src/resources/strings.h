#ifndef RES_STRINGS_H_INCLUDED
#define RES_STRINGS_H_INCLUDED

/**
*   This file includes the standard strings.
*/

namespace res {

    namespace str {
        const char* const program_title = "Cell Simulator";
        const char* const menu_files = "File";
		const char* const menu_files_open = "Open...";
        const char* const menu_edit = "Edit";
        const char* const menu_view = "View";

        const char* const plane_cell_view_name = "Cell View";
        const char* const plane_neural_network_view_name = "Neural Network View";

		namespace lua {
			const char* const error_arg_not_3d_vector = "Passed argument is not a vector table in the format {x, y, z}.";
			const char* const error_include_undefined_simulator = "Attempt to use simulation which does not exist.";
			const char* const error_include_simulator_dependency = "Attempt to use simulation '%s', which has an unmet dependency on '%s'.";
			const char* const error_include_undefined_renderer = "Attempt to use renderer which does not exist.";
			const char* const error_include_renderer_simulator_dependency = "Attempt to use renderer '%s', which has an unmet dependency on '%s.'";
			const char* const error_function_undefined_simulator = "Attempt to use function of simulation '%s', which does not exist.";
			const char* const error_missing_arguments = "Missing arguments, not enough arguments passed.";
		}
    }

}

#endif // STRINGS_H_INCLUDED

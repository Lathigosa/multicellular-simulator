#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

/**
*   This file includes the standard strings.
*/

#define GLM_ENABLE_EXPERIMENTAL

#include <string>
#include <iostream>

enum struct error
{
	success,
	generic_error,
	undefined_function,
	buffer_overflow,
	uninitialized,
	invalid_argument
};

// TODO: change the target opencl version to 200, when the support is there:
//#define CL_HPP_MINIMUM_OPENCL_VERSION 120
//#define CL_HPP_TARGET_OPENCL_VERSION 120
//#define CL_HPP_ENABLE_EXCEPTIONS

#if defined(DEBUG) | defined(_DEBUG)
template<typename... Args>
inline void message_debug(Args&&... args) {
    (std::cout << ... << args) << std::endl; // fold expression (C++17+)
}
#else
template<typename... Args>
inline void message_debug([[maybe_unused]] Args&&... args) { }
#endif



// Comment out this line to remove warning messages:
#define NOTIFY_MSG

#if defined(NOTIFY_MSG)
#define message_notify(message) std::cout << message << std::endl;
#else
#define message_notify(message)
#endif

// Comment out this line to remove error messages:
#define ERROR_MSG

#if defined(ERROR_MSG)
#define message_error(message) std::cout << message << std::endl;
#else
#define message_error(message)
#endif

// Uncomment the following line to build without UI:
//#define NO_UI

//TODO: change application id!
#define __APPLICATION_ID                  "org.gtkmm.examples.base"

#endif // MAIN_H_INCLUDED

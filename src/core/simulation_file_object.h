#ifndef SIMULATION_FILE_OBJECT_H
#define SIMULATION_FILE_OBJECT_H

#include <string>
#include <luajit-2.1/lua.hpp>
#include <exception>
#include <memory>

#include <gtkmm/glarea.h>

#include "core/simulation_collection.h"
#include "core/render_collection.h"

/// The exception that the simulation_file_object can throw:
class sfo_parsing_exception : public std::runtime_error
{
public:
	sfo_parsing_exception(std::string message) : std::runtime_error(message) {}
};

/// simulation_file_object is a class representing the currently loaded
/// simulation file, including the Lua objects.
class simulation_file_object
{
	public:
		simulation_file_object();
		virtual ~simulation_file_object();
		simulation_file_object(const simulation_file_object&) = delete;
		simulation_file_object operator=(const simulation_file_object&) = delete;

		std::unique_ptr<simulation::simulation_world> world;
		std::unique_ptr<render_collection> renderer;
		//simulation::simulation_world world;

		void open_file(const std::string& filename);

	protected:


	private:
		

		lua_State* L = nullptr;
};

#endif // SIMULATION_FILE_OBJECT_H

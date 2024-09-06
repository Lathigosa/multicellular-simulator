#ifndef RENDERER_MAP_H_INCLUDED
#define RENDERER_MAP_H_INCLUDED

#include <string>
#include <vector>
#include <map>
#include "core/render_unit_template.h"
#include "utilities/safe_pointer.h"

#include "renderer/cells_as_dots.h"
#include "renderer/cells_velocity.h"
#include "renderer/polar_spheres.h"

namespace renderer
{
	template<typename T> render_unit_template* instantiate_renderer()
	{
		return new T();
	}

	template<typename T> std::vector<std::string> get_sim_dependencies()
	{
		return T::dependencies();
	}

	typedef std::map<std::string, render_unit_template*(*)()> init_map_type;

	typedef std::map<std::string, std::vector<std::string>(*)()> dep_map_type;

	#define render_unit(T)				{T::get_name_static(), \
									&instantiate_renderer<T>}

	#define render_dependencies(T) 	{T::get_name_static(), \
									&get_sim_dependencies<T>}

	/// This is a list of all standard simulation units:
	const init_map_type standard_render_units =
	{
		render_unit(cells_velocity),
		render_unit(cells_as_dots),
		render_unit(polar_spheres)
	};

	const dep_map_type standard_render_unit_dependencies =
	{
		render_dependencies(cells_velocity),
		render_dependencies(cells_as_dots),
		render_dependencies(polar_spheres)
	};

}



#endif // RENDERER_MAP_H_INCLUDED

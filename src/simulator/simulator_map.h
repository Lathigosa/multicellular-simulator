#ifndef SIMULATOR_MAP_H_INCLUDED
#define SIMULATOR_MAP_H_INCLUDED

#include <string>
#include <vector>
#include <map>
#include "core/sim_unit_template.h"
#include "utilities/safe_pointer.h"

#include "simulator/unit_cell.h"
#include "simulator/membrane.h"
#include "simulator/intracellular_reactions.h"
#include "simulator/division_after_concentration_treshold.h"
#include "simulator/cell_death_barrier.h"
#include "simulator/cell_polarity.h"
#include "simulator/division_plane.h"

namespace simulation
{
	template<typename T> sim_unit_template* instantiate_simulator(
			cl::Platform & platform_cl,
			cl::Device & device_cl,
			cl::Context & context,
			cl::CommandQueue & command_queue)
	{
		return new T(platform_cl,
						device_cl,
						context,
						command_queue);
	}

	template<typename T> std::vector<std::string> get_sim_dependencies()
	{
		return T::dependencies();
	}

	typedef std::map<std::string, sim_unit_template*(*)(cl::Platform & platform_cl, 
			cl::Device & device_cl,
			cl::Context & context,
			cl::CommandQueue & command_queue)> init_map_type;

	typedef std::map<std::string, std::vector<std::string>(*)()> dep_map_type;

	#define sim_unit(T)				{T::get_name_static(), \
									&instantiate_simulator<T>}

	#define sim_dependencies(T) 	{T::get_name_static(), \
									&get_sim_dependencies<T>}

	/// This is a list of all standard simulation units:
	const init_map_type standard_sim_units =
	{
		sim_unit(cell_global),
		sim_unit(membrane),
		sim_unit(intracellular_reactions),
		sim_unit(division_after_concentration_treshold),
		sim_unit(cell_death_barrier),
		sim_unit(cell_polarity),
		sim_unit(division_plane)
	};

	const dep_map_type standard_sim_unit_dependencies =
	{
		sim_dependencies(cell_global),
		sim_dependencies(membrane),
		sim_dependencies(intracellular_reactions),
		sim_dependencies(division_after_concentration_treshold),
		sim_dependencies(cell_death_barrier),
		sim_dependencies(cell_polarity),
		sim_dependencies(division_plane)
	};

}



#endif // SIMULATOR_MAP_H_INCLUDED

#ifndef RENDER_UNIT_TEMPLATE_H_INCLUDED
#define RENDER_UNIT_TEMPLATE_H_INCLUDED

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include "main.h"

#include <CL/cl2.hpp>
#include <vector>
#include <string>
#include <memory>

#include "sim_unit_template.h"

#include "utilities/camera.h"

//namespace simulation
//{
	

	/// This definition should be added in each derived class of sim_unit_template:
	#define RENDER_DEFINITION(name, dependency_list) const std::string get_name() const override \
			{ \
				return get_name_static(); \
			} \
			const std::vector<std::string> get_dependencies() const override \
			{ \
				return dependencies(); \
			} \
			static const char* get_name_static() \
			{ \
				return name; \
			} \
			static const std::vector<std::string> dependencies() \
			{ \
				return dependency_list; \
			}

	/// struct containing information about what happened to a buffer:
	/*struct buffer_rearrange_info
	{
		enum rearrange_type{
			deletion,			// Delete the entries in the range and close the gap.
			insertion,			// Insert new entries in the range and push the old entries.
		};

		rearrange_type type;

		unsigned int range_start;
		unsigned int range_end;
	};*/

	//struct buffer_dependency_definition
	//{
	//	const std::string simulation_name;
	//	const std::string buffer_name;
	//}

	class render_unit_template
	{
	public:
		render_unit_template();
		virtual ~render_unit_template();

		virtual error initialize();
		//#ifndef NO_UI
		//virtual error initialize_buffers(std::vector<GLuint> from_gl_buffer);
		//#endif

		/// Gets the name used to store the pointer of the instance in LUA:
		virtual const std::string get_name() const;

		/// Gets the names of simulation units it has a dependency on:
		virtual const std::vector<std::string> get_dependencies() const;

		/// Push a list of pointers to the dependencies to this sim unit:
		error set_dependency_pointers(std::vector<simulation::sim_unit_template*> &list);
		error set_dependency_buffer_indices(std::vector<std::string> &list);
		//error set_child_pointers(std::vector<sim_unit_template*> &list);

		virtual error render(camera gl_camera);		// Render the unit.

		// Signals:
		/// Signal that is called from parent to indicate that the entries in a buffer of the parent have been rearranged:
		//virtual error signal_buffer_rearrange(const std::string sim_unit_name,
		//										unsigned int buffer_index,
		//										buffer_rearrange_info info);
	protected:
		//cl::Platform & platform;
		//cl::Device & device;
		//cl::Context & opencl_context;
		//cl::CommandQueue & queue;

		simulation::VBO_info get_VBO(unsigned int index);

		std::vector<simulation::sim_unit_template*> dependency_pointers;		// The simulation units that this unit depends on.
		std::vector<unsigned int> dependency_buffer_indices;					// The corresponding buffer indices (each entry in this list corresponds with an entry in "dependency_pointers".
	};
//}



#endif // RENDER_UNIT_TEMPLATE_H_INCLUDED

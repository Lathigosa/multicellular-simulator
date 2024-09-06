#ifndef SIM_UNIT_TEMPLATE_H_INCLUDED
#define SIM_UNIT_TEMPLATE_H_INCLUDED

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include "main.h"

//TODO: perhaps substitute this for <luajit-2.0/lua.hpp>:
#include "core/lua_functions.h"

#include <CL/cl2.hpp>
#include <vector>
#include <string>
#include <memory>
#include <map>

#include "core/event_info.h"

namespace simulation
{
	//enum struct error
	//{
	//	success,
	//	generic_error,
	//	undefined_function,
	//	buffer_overflow,
	//};

	/// This definition should be added in each derived class of sim_unit_template:
	#define SIMULATOR_DEFINITION(name, dependency_list) const std::string get_name() const override \
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

	#define P99_PROTECT(...)	__VA_ARGS__

	struct VBO_info
	{
		GLuint VBO;
		bool needs_refresh;
		unsigned int size;
	};

	/// struct containing information about what happened to a buffer:
	struct buffer_rearrange_info
	{
		enum rearrange_type{
			deletion,			// Delete the entries in the range and close the gap.
			insertion,			// Insert new entries in the range and push the old entries.
		};

		rearrange_type type;

		unsigned int range_start;
		unsigned int range_end;
	};

	// struct containing a double buffer and extras:
	struct double_buffer
	{
		cl::Buffer buffer[2];
		unsigned int entry_size = 0;
		unsigned int entry_count = 0;
	};

	// class containing an arbitrary data union for use in each simulator's arbitrary data map:
	struct custom_datum
	{
		union
		{
			int value_int;
			unsigned int value_uint;
			float value_float;
		};
		enum {c_int, c_uint, c_float} data_type;
		
	};

	/*struct event_info
	{
		cl::Event event;
		const char* const name;
		enum {
			kernel,
			fill_buffer,
			write_buffer,
			copy_buffer,
			read_buffer
		} type;
	};*/

	class sim_unit_template
	{
	public:
		sim_unit_template(cl::Platform & platform_cl, cl::Device & device_cl, cl::Context & context, cl::CommandQueue & command_queue);
		virtual ~sim_unit_template();

		virtual error initialize_buffers();
		#ifndef NO_UI
		virtual error initialize_buffers(std::vector<GLuint> from_gl_buffer);
		#endif

		/// Gets the name used to store the pointer of the instance in LUA:
		virtual const std::string get_name() const;

		/// Gets the names of simulation units it has a dependency on:
		virtual const std::vector<std::string> get_dependencies() const;

		/// Push a list of pointers to the dependencies to this sim unit:
		error set_dependency_pointers(std::vector<sim_unit_template*> &list);
		error set_child_pointers(std::vector<sim_unit_template*> &list);
		error add_child_pointer(sim_unit_template* unit);

		virtual void init();											// The function that calls initialize_buffers();
		std::vector<event_info> simulate(float step_size);				// The function that calls simulate_unit();

		// TODO: make protected:
		virtual error organize_unit();		//
		virtual std::vector<event_info> simulate_unit(float step_size);		// Simulate using local buffers.
		virtual error function();			// Simulate using passed buffers.
			
		virtual cl::Buffer get_buffer(unsigned int index, bool double_buffer);
		virtual unsigned int get_buffer_size(unsigned int index, bool double_buffer) const;
		virtual unsigned int get_buffer_entry_size(unsigned int index) const;
		//virtual unsigned int get_max_buffer_size(unsigned int index, bool double_buffer) const;
		virtual unsigned int get_buffer_index(const std::string name) const;

		// OpenGL-rendering helpers:
		virtual GLuint get_buffer_as_VBO(unsigned int index, bool double_buffer);		// Gets the specified buffer as VBO, it only updates when necessary.
		virtual VBO_info get_buffer_as_VBO_info(unsigned int index, bool double_buffer);		// Gets the specified buffer as VBO_info, it only updates when necessary.

		// Signals:
		/// Signal that is called from parent to indicate that the entries in a buffer of the parent have been rearranged:
		virtual error signal_buffer_rearrange(const std::string sim_unit_name,
												unsigned int buffer_index,
												buffer_rearrange_info info);
		virtual error signal_particle_remove(const std::string sim_unit_name,
		                                 		cl::Buffer empty_cells,
		                                 		unsigned int empty_count);
		// Signal that is called from parent to indicate that a cellular buffer must be rearranged (for example, after cell divisions or cell death):
		virtual error signal_cell_reindex(const std::string sim_unit_name, 
		                              			cl::Buffer empty_cells,
		                              			cl::Buffer copied_cells,
		                              			unsigned int copied_count);
		virtual void signal_buffer_insert(const std::string sim_unit_name,
		                                  unsigned int buffer_index,
		                                  unsigned int range_start,
		                                  unsigned int count);

		virtual const void expose_lua_library(lua_State* L) const;

		int get_custom_int(const std::string key) const;
		float get_custom_float(const std::string key) const;
		unsigned int get_custom_uint(const std::string key) const;
		

		const cl::Platform get_platform() const;
		const cl::Device get_device() const;
		const cl::Context get_context() const;
		const cl::CommandQueue get_command_queue() const;
	protected:
		cl::Platform & platform;
		cl::Device & device;
		cl::Context & opencl_context;
		cl::CommandQueue & queue;

		//std::map<std::string, cl::Buffer> buffers;					// Buffers indexed by a string.
		//std::vector<double_buffer> buffers;
		//std::map<std::string, unsigned int> buffer_indices;

		void set_custom_data(const std::string key, int value);
		void set_custom_data(const std::string key, float value);
		void set_custom_data(const std::string key, unsigned int value);

		std::vector<sim_unit_template*> dependency_pointers;		// The simulation units that this unit depends on.
		std::vector<sim_unit_template*> dependency_child_pointers;	// The simulation units that are dependent on this one.

		unsigned int create_double_buffer(std::string name, unsigned int entry_size, unsigned int entry_count, unsigned int max_count);


		cl::Kernel get_kernel_from_file(const char* const file_name, const char* const kernel_name) const;
	private:
		bool VBO_refresh = true;					// Whether the VBOs need a refresh.

		std::map<std::string, custom_datum> custom_data;

		std::map<std::string, unsigned int> buffer_indices;
		std::vector<double_buffer> buffers;
			
		std::vector<VBO_info> VBO_list;
	};
}



#endif // SIM_UNIT_TEMPLATE_H_INCLUDED

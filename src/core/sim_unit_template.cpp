#include "sim_unit_template.h"

#include <algorithm>

#include "utilities/load_file.h"

using namespace simulation;

sim_unit_template::sim_unit_template(cl::Platform & platform_cl,
									cl::Device & device_cl,
									cl::Context & context,
									cl::CommandQueue & command_queue) : platform(platform_cl),
																		device(device_cl),
																		opencl_context(context),
																		queue(command_queue)
{

}

sim_unit_template::~sim_unit_template()
{
	//dtor
}

void sim_unit_template::init()
{
	initialize_buffers();
}

error sim_unit_template::initialize_buffers()
{
	return error::success;
}

const std::string sim_unit_template::get_name() const
{
	return "undefined";
}

unsigned int sim_unit_template::get_buffer_index(const std::string name) const
{
	return buffer_indices.at(name);
}

unsigned int sim_unit_template::get_buffer_size(unsigned int index, bool double_buffer) const
{
	return 0;
}

const std::vector<std::string> sim_unit_template::get_dependencies() const
{
	return {};
}

error sim_unit_template::set_dependency_pointers(
						std::vector<sim_unit_template*> &list
						)
{
	dependency_pointers = list;
	return error::success;
}

error sim_unit_template::set_child_pointers(
						std::vector<sim_unit_template*> &list
						)
{
	dependency_child_pointers = list;
	return error::success;
}

error sim_unit_template::add_child_pointer(
						sim_unit_template* unit
						)
{
	dependency_child_pointers.push_back(unit);
	return error::success;
}

#ifndef NO_UI
error sim_unit_template::initialize_buffers(std::vector<GLuint> from_gl_buffer)
{
	return error::success;
}
#endif

cl::Buffer sim_unit_template::get_buffer(unsigned int index, bool double_buffer)
{
	// TODO: ERROR if more than count!
	//return 0;
	return buffers.at(index).buffer[double_buffer];
}

unsigned int sim_unit_template::get_buffer_entry_size(unsigned int index) const
{
	return buffers.at(index).entry_size;
}

error sim_unit_template::organize_unit()
{
	return error::undefined_function;
}

std::vector<event_info> sim_unit_template::simulate(float step_size)
{
	VBO_refresh = true;
	for(int i=0; i<VBO_list.size(); i++)
	{
		VBO_list.at(i).needs_refresh = true;
	}
	
	//std::vector<event_info> info;
	for(int i=0; i<1; i++)
	{
		
		// Append new info to old:
		//info.insert(info.end(), new_info.begin(), new_info.end());
	}

	std::vector<event_info> info = simulate_unit(step_size);
	
	return info;
}

std::vector<event_info> sim_unit_template::simulate_unit(float step_size)
{
	//TODO: throw error: undefined function!
	return {};
}

error sim_unit_template::function()
{
	return error::undefined_function;
}

error sim_unit_template::signal_buffer_rearrange(const std::string sim_unit_name,
												unsigned int buffer_index,
												buffer_rearrange_info info)
{
	return error::undefined_function;
}

error sim_unit_template::signal_cell_reindex(const std::string sim_unit_name,
											cl::Buffer empty_cells,
		                              		cl::Buffer copied_cells,
		                              		unsigned int copied_count)
{
	return error::undefined_function;
}

error sim_unit_template::signal_particle_remove(const std::string sim_unit_name,
		                                 		cl::Buffer empty_cells,
		                                 		unsigned int empty_count)
{
	return error::undefined_function;
}

void sim_unit_template::signal_buffer_insert(const std::string sim_unit_name,
		                                  unsigned int buffer_index,
		                                  unsigned int range_start,
		                                  unsigned int count)
{
	//TODO: throw error, or ignore? Undefined function
}

//deprecated:
GLuint sim_unit_template::get_buffer_as_VBO(unsigned int index, bool double_buffer)
{
	// Resize array when necessary:
	if (index >= VBO_list.size())
		VBO_list.resize(index + 1, {0, true, 0});

	GLuint VBO = VBO_list.at(index).VBO;

	// Initialize new VBO when necessary:
	if (VBO == 0)
	{
		glGenBuffers(1, &VBO);
		unsigned int size = get_buffer_size(index, double_buffer);
		VBO_info new_VBO = {VBO, true, size};
		VBO_list.at(index) = new_VBO;
	}

	if (VBO_list.at(index).needs_refresh == true)
	{
		
		VBO_list.at(index).needs_refresh = false;

		cl::Buffer buffer = get_buffer(index, double_buffer);
		unsigned int size = get_buffer_size(index, double_buffer);

		// TODO: use glSubBufferData for performance improvement!

		float array_test_out_p[size];

		queue.enqueueReadBuffer(buffer, CL_TRUE, 0, size, array_test_out_p);

		queue.finish();
	
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, size, array_test_out_p, GL_DYNAMIC_DRAW);

	}
	
	return VBO_list.at(index).VBO;
}

VBO_info sim_unit_template::get_buffer_as_VBO_info(unsigned int index, bool double_buffer)
{
	// Resize array when necessary:
	if (index >= VBO_list.size())
		VBO_list.resize(index + 1, {0, true, 0});

	GLuint VBO = VBO_list.at(index).VBO;

	// Initialize new VBO when necessary:
	if (VBO == 0)
	{
		glGenBuffers(1, &VBO);
		unsigned int size = get_buffer_size(index, double_buffer);
		VBO_info new_VBO = {VBO, true, size};
		VBO_list.at(index) = new_VBO;
	}

	if (VBO_list.at(index).needs_refresh == true)
	{
		
		VBO_list.at(index).needs_refresh = false;

		cl::Buffer buffer = get_buffer(index, double_buffer);
		unsigned int size = get_buffer_size(index, double_buffer);

		// TODO: use glSubBufferData for performance improvement!
		//size = 32;
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		//glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		//for(unsigned int i=0; i<size; i+=4096)
		//{
		//	float array_test_out_p[4096];
		//	queue.enqueueReadBuffer(buffer, CL_TRUE, i, std::min((int)size - (int)i, 4096), array_test_out_p);
		//	glBufferSubData(GL_ARRAY_BUFFER, i, std::min((int)size - (int)i, 4096), array_test_out_p);
		//}

		//char array_test_out_p[size];

		char* array_test_out_p;

		array_test_out_p = new char[size];

		queue.enqueueReadBuffer(buffer, CL_TRUE, 0, size, array_test_out_p);
		

		
		queue.finish();
		//message_debug(array_test_out_p[0]);
		//message_debug("size = " + size + ", buffer = " + index);
	
		
		//message_debug("size = " << size / sizeof(cl_float4) << ", data = " << array_test_out_p[0]);
		glBufferData(GL_ARRAY_BUFFER, size, array_test_out_p, GL_DYNAMIC_DRAW);
		VBO_list.at(index).size = size;

		delete[] array_test_out_p;
	}
	
	return VBO_list.at(index);
}

const void sim_unit_template::expose_lua_library(lua_State* L) const
{
	// No library to include.
}

const cl::Platform sim_unit_template::get_platform() const
{
	return platform;
}

const cl::Device sim_unit_template::get_device() const
{
	return device;
}

const cl::Context sim_unit_template::get_context() const
{
	return opencl_context;
}

const cl::CommandQueue sim_unit_template::get_command_queue() const
{
	return queue;
}

int sim_unit_template::get_custom_int(const std::string key) const
{
	auto pos = custom_data.find(key);
	if (pos == custom_data.end()) {
		// TODO: handle the error with an exception.
	} else {
		custom_datum value = pos->second;
		if(value.data_type != custom_datum::c_int)
		{
			// TODO: handle the error with an exception.
		}
		return value.value_int;
	}
	return 0;
}

float sim_unit_template::get_custom_float(const std::string key) const
{
	auto pos = custom_data.find(key);
	if (pos == custom_data.end()) {
		// TODO: handle the error with an exception.
	} else {
		custom_datum value = pos->second;
		if(value.data_type != custom_datum::c_float)
		{
			// TODO: handle the error with an exception.
		}
		return value.value_float;
	}
	return 0.0f;
}

unsigned int sim_unit_template::get_custom_uint(const std::string key) const
{
	auto pos = custom_data.find(key);
	if (pos == custom_data.end()) {
		// TODO: handle the error with an exception.
	} else {
		custom_datum value = pos->second;
		if(value.data_type != custom_datum::c_uint)
		{
			// TODO: handle the error with an exception.
		}
		return value.value_uint;
	}
	return 0;
}

void sim_unit_template::set_custom_data(const std::string key, int value)
{
	custom_datum data;
	data.data_type = custom_datum::c_int;
	data.value_int = value;
	custom_data.insert(std::pair<std::string,custom_datum>(key, data));
}

void sim_unit_template::set_custom_data(const std::string key, float value)
{
	custom_datum data;
	data.data_type = custom_datum::c_float;
	data.value_float = value;
	custom_data.insert(std::pair<std::string,custom_datum>(key, data));
}

void sim_unit_template::set_custom_data(const std::string key, unsigned int value)
{
	custom_datum data;
	data.data_type = custom_datum::c_uint;
	data.value_uint = value;
	custom_data.insert(std::pair<std::string,custom_datum>(key, data));
}

unsigned int sim_unit_template::create_double_buffer(std::string name, unsigned int entry_size, unsigned int entry_count, unsigned int max_count)
{
	double_buffer new_buffer;
	new_buffer.buffer[0] = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, entry_size*max_count);
	new_buffer.buffer[1] = cl::Buffer(opencl_context, CL_MEM_READ_WRITE, entry_size*max_count);
	new_buffer.entry_size = entry_size;
	new_buffer.entry_count = entry_count;

	buffer_indices.insert(std::pair<std::string, unsigned int>(name, buffers.size()));
	buffers.push_back(new_buffer);
	return buffers.size() - 1;
}

cl::Kernel sim_unit_template::get_kernel_from_file(const char* const file_name, const char* const kernel_name) const
{
	std::string kernel_code = load_file(file_name);
	cl::Program::Sources sources_append_buffer;
	sources_append_buffer.push_back({kernel_code.c_str(), kernel_code.length()});

	cl::Program program_append_buffer = cl::Program(opencl_context, sources_append_buffer);
	if(program_append_buffer.build({device}) != CL_SUCCESS)
	{
		message_error("Error building: " << program_append_buffer.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
		exit(1);
	}

	return cl::Kernel(program_append_buffer, kernel_name);
}

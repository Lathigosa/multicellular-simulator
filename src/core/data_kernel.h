#ifndef DATA_KERNEL_H_INCLUDED
#define DATA_KERNEL_H_INCLUDED

#include "main.h"

#include <CL/cl2.hpp>
#include <vector>
#include <string>
#include <memory>
#include <map>

#include "core/data_variable.h"

class data_kernel
{
public:
	data_kernel(cl::Platform & platform,
				cl::Device & device,
				cl::Context & context,
				cl::CommandQueue & command_queue);
	
	data_kernel(const data_kernel& from) = default;
	data_kernel& operator=(const data_kernel&) = default;
	data_kernel(data_kernel&&) = default;
	data_kernel& operator =(data_kernel&&) = default;

	virtual ~data_kernel();

	//virtual std::vector<event_info> run(bool buffer_index);

protected:
	cl::Platform & m_platform;
	cl::Device & m_device;
	cl::Context & m_context;
	cl::CommandQueue & m_command_queue;
	
	cl::Program get_program_from_file(const char* const file_name) const;
	cl::Kernel get_kernel_from_file(const char* const file_name, const char* const kernel_name) const;
};




#endif // DATA_FUNCTION_H_INCLUDED

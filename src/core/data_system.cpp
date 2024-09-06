#include "core/data_system.h"

#include <stdexcept>
#include <CL/cl2.hpp>
#include <memory>
#include <algorithm>
#include <cctype>

#include "utilities/load_file.h"

data_system::data_system(cl::Platform & platform,
                         cl::Device & device,
                         cl::Context & context,
                         cl::CommandQueue & command_queue) : 
	m_platform(platform),
	m_device(device),
	m_context(context),
	m_command_queue(command_queue)
{
	
}

data_system::~data_system()
{

}

void data_system::build()
{
	
}

cl::Program data_system::get_program_from_file(const char* const file_name) const
{
	std::string kernel_code = load_file(file_name);
	cl::Program::Sources sources;
	sources.push_back({kernel_code.c_str(), kernel_code.length()});

	cl::Program program = cl::Program(m_context, sources);
	if(program.build({m_device}, "-I standard_libraries") != CL_SUCCESS)
	{
		message_error("Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device));
		exit(1);
	}

	return program;
}

cl::Kernel data_system::get_kernel_from_file(const char* const file_name, const char* const kernel_name) const
{
	cl::Program program = get_program_from_file(file_name);

	return cl::Kernel(program, kernel_name);
}

cl::Platform& data_system::getPlatform()
{
	return m_platform;
}

cl::Device& data_system::getDevice()
{
	return m_device;
}

cl::Context& data_system::getContext()
{
	return m_context;
}

cl::CommandQueue& data_system::getCommandQueue()
{
	return m_command_queue;
}



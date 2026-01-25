#include "main.h"

#include "core/data_system.h"

#include <CL/opencl.hpp>

#include "utilities/load_file.h"

DataSystem::DataSystem(cl::CommandQueue & command_queue) : 
	m_command_queue(command_queue),
	m_context(command_queue.getInfo<CL_QUEUE_CONTEXT>()),
    m_device(command_queue.getInfo<CL_QUEUE_DEVICE>()),
    m_platform(m_device.getInfo<CL_DEVICE_PLATFORM>())
{
	
}

DataSystem::~DataSystem()
{

}

void DataSystem::build()
{
	
}

cl::Program DataSystem::get_program_from_file(const char* const file_name) const
{
	std::string kernel_code = load_file(file_name);
	cl::Program::Sources sources;
	sources.push_back({kernel_code.c_str(), kernel_code.length()});

	cl::Program program = cl::Program(m_context, sources);
	try {
		if(program.build({m_device}, "-I share/standard_libraries") != CL_SUCCESS)
		{
			message_error("Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device));
			exit(1);
		}
	} catch (const cl::Error& error) {
		message_error("Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device));
	}

	return program;
}

cl::Kernel DataSystem::get_kernel_from_file(const char* const file_name, const char* const kernel_name) const
{
	cl::Program program = get_program_from_file(file_name);

	return cl::Kernel(program, kernel_name);
}

cl::Platform& DataSystem::getPlatform()
{
	return m_platform;
}

cl::Device& DataSystem::getDevice()
{
	return m_device;
}

cl::Context& DataSystem::getContext()
{
	return m_context;
}

cl::CommandQueue& DataSystem::getCommandQueue()
{
	return m_command_queue;
}



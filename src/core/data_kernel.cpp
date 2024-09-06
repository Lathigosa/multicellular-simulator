#include "core/data_kernel.h"

#include "utilities/load_file.h"

data_kernel::data_kernel(cl::Platform & platform,
						cl::Device & device,
						cl::Context & context,
						cl::CommandQueue & command_queue) :
	m_platform(platform),
	m_device(device),
	m_context(context),
	m_command_queue(command_queue)
{

}

data_kernel::~data_kernel() { }

cl::Program data_kernel::get_program_from_file(const char* const file_name) const
{
	std::string kernel_code = load_file(file_name);
	cl::Program::Sources sources;
	sources.push_back({kernel_code.c_str(), kernel_code.length()});

	cl::Program program = cl::Program(m_context, sources);
	if(program.build({m_device}) != CL_SUCCESS)
	{
		message_error("Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device));
		exit(1);
	}

	return program;
}

cl::Kernel data_kernel::get_kernel_from_file(const char* const file_name, const char* const kernel_name) const
{
	cl::Program program = get_program_from_file(file_name);

	return cl::Kernel(program, kernel_name);
}
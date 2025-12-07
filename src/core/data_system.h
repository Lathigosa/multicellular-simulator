#ifndef DATA_SYSTEM_H_INCLUDED
#define DATA_SYSTEM_H_INCLUDED

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include "main.h"

#include <CL/opencl.hpp>
#include <vector>
#include <string>
#include <memory>
#include <map>

//#include "core/data_variable.h"



class DataSystem
{
public:
	DataSystem(cl::Platform & platform,
				cl::Device & device,
				cl::Context & context,
				cl::CommandQueue & command_queue);
	DataSystem(const DataSystem& from) = delete;			// TODO: add copy and assignment?
	DataSystem& operator=(const DataSystem&) = delete;

	

	void build();

	virtual ~DataSystem();

	cl::Platform& getPlatform();
	cl::Device& getDevice();
	cl::Context& getContext();
	cl::CommandQueue& getCommandQueue();

protected:
	cl::Program get_program_from_file(const char* const file_name) const;
	cl::Kernel get_kernel_from_file(const char* const file_name, const char* const kernel_name) const;

	cl::Platform& m_platform;
	cl::Device& m_device;
	cl::Context& m_context;
	cl::CommandQueue& m_command_queue;

};



#endif // DATA_SYSTEM_H_INCLUDED

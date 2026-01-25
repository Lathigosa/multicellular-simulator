#ifndef DATA_SYSTEM_H_INCLUDED
#define DATA_SYSTEM_H_INCLUDED

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include <CL/opencl.hpp>

//#include "core/data_variable.h"



class DataSystem
{
public:
	DataSystem(cl::CommandQueue & command_queue, bool use_opengl_context);
	DataSystem(const DataSystem& from) = delete;			// TODO: add copy and assignment?
	DataSystem& operator=(const DataSystem&) = delete;

	void build();

	virtual ~DataSystem();

	cl::CommandQueue& getCommandQueue();
	cl::Context& getContext();
	cl::Device& getDevice();
	cl::Platform& getPlatform();

	cl::Program get_program_from_file(const char* const file_name) const;
	cl::Kernel get_kernel_from_file(const char* const file_name, const char* const kernel_name) const;

	bool isSharedWithOpenGL();

protected:
	cl::CommandQueue& m_command_queue;
	cl::Context m_context;
	cl::Device m_device;
	cl::Platform m_platform;

	bool is_shared_with_opengl;
};



#endif // DATA_SYSTEM_H_INCLUDED

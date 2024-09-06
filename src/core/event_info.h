#ifndef EVENT_INFO_H_INCLUDED
#define EVENT_INFO_H_INCLUDED

#include <CL/cl2.hpp>

struct event_info
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
};


#endif // EVENT_INFO_H_INCLUDED

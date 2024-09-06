#ifndef RENDER_COLLECTION_H_INCLUDED
#define RENDER_COLLECTION_H_INCLUDED

#include "main.h"

#include "core/render_unit_template.h"

#ifndef NO_UI
#include <epoxy/gl.h>
#include <epoxy/glx.h>		// TODO: see if I should switch to egl instead.
#endif // NO_UI

#include <iostream>

#include "utilities/camera.h"

#include <CL/cl2.hpp>
#include <vector>
#include <memory>

class render_collection
{
public:
	render_collection();

	render_collection(const render_collection&) = delete;
	render_collection& operator=(const render_collection&) = delete;

    virtual ~render_collection();

    std::vector<std::unique_ptr<render_unit_template>> render_units;

	void init(bool use_gl_context = false);

    void render_all(camera gl_camera);
    void finish();
protected:
private:
	bool has_initialized = false;

	// Platform and hardware information:
	std::vector<cl::Platform> all_platforms;
	std::vector<cl::Device> all_devices;

	

public:		// TODO: make private?
	cl::Platform default_platform;
	cl::Device default_device;
	cl::Context opencl_context;
	cl::CommandQueue queue;
};

#endif  // RENDER_COLLECTION_H_INCLUDED
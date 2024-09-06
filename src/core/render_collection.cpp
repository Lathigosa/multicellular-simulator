

#include "main.h"

#include "render_collection.h"

#include <iostream>

#ifndef NO_UI
#include <epoxy/gl.h>
#include <epoxy/glx.h>		// TODO: see if I should switch to egl instead.
#endif // NO_UI

using namespace simulation;

render_collection::render_collection()
{

}

void render_collection::init(bool use_gl_context)
{
	has_initialized = true;

	for(int a=0; a<render_units.size(); a++)
	{
		render_units[a]->initialize();
	}
}

void render_collection::render_all(camera gl_camera)
{
	if(has_initialized == false)
	{
		message_debug("ERROR! Not yet initialized render_collection.");
		return;
	}

	for(int a=0; a<render_units.size(); a++)
	{
		render_units[a]->render(gl_camera);
	}
}

void render_collection::finish()
{
	//queue.finish();
}

render_collection::~render_collection()
{
    //dtor
}

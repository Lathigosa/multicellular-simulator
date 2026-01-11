

#include "main.h"

#include "render_collection.h"

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
	for (const auto& unit : render_units) unit->initialize();

	has_initialized = true;
}

void render_collection::render_all(camera gl_camera)
{
	if(has_initialized == false)
	{
		message_debug("ERROR! Not yet initialized render_collection.");
		return;
	}

	for (const auto& unit : render_units) unit->render(gl_camera);
}

void render_collection::finish()
{
	//queue.finish();
}

render_collection::~render_collection()
{
    //dtor
}

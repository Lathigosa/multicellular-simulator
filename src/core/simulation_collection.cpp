

#include "main.h"

#include "simulation_collection.h"

#include <iostream>

#ifndef NO_UI
#include <epoxy/gl.h>
#include <epoxy/glx.h>		// TODO: see if I should switch to egl instead.
#endif // NO_UI

#include <thread>
#include <chrono>

#include "core/event_info.h"

using namespace simulation;

simulation_world::simulation_world()
{
	
}

void simulation_world::initialize_render()
{
	if(cell_system == nullptr)
		return;
	
	cell_system->initialize_render();
}

void simulation_world::render_all(camera gl_camera)
{
	if(cell_system == nullptr)
		return;
	
	cell_system->render(gl_camera);
}

void simulation_world::init(bool use_gl_context)
{
	message_debug("Created new simulation world.");

	// Setup the platform:
	cl::Platform::get(&all_platforms);
    if(all_platforms.size() == 0){
        message_error("No platforms found. Check OpenCL installation!");
        exit(1);
    }
    default_platform = all_platforms[0];
    message_notify("Using platform: "<<default_platform.getInfo<CL_PLATFORM_NAME>()<<"\nVersion: "<<default_platform.getInfo<CL_PLATFORM_VERSION>());

	// Get default device:
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if(all_devices.size() == 0)
	{
		message_error("No devices found. Check OpenCL installation!");
		exit(1);
	}
	default_device = all_devices[0];
	message_notify("Using device: " << default_device.getInfo<CL_DEVICE_NAME>());
	message_notify("Version: " << default_device.getInfo<CL_DEVICE_VERSION>());

	// If the device version does not match the OpenCL version used here:

	// TODO;

	#ifndef NO_UI
	if(use_gl_context == false)
	#endif // NO_UI

	{
		// Create a bare OpenCL context:
		message_debug("Created bare OpenCL context.");
		opencl_context = cl::Context({default_device});
	}

	#ifndef NO_UI
	else
	{
		message_debug("Connected OpenCL context to OpenGL.");
		// Create an OpenCL context linked to an OpenGL context:
		cl_context_properties properties[] = {
			CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext(),
			CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay(),
			CL_CONTEXT_PLATFORM, (cl_context_properties) default_platform(),
			0
		};

		opencl_context = cl::Context({default_device}, properties);
	}
	#endif // NO_UI

	// Create the command queue:
	queue = cl::CommandQueue(opencl_context, default_device, CL_QUEUE_PROFILING_ENABLE, nullptr);

	if(cell_system != nullptr)
		delete cell_system;
	
	cell_system = new CellSystem(default_platform, default_device, opencl_context, queue);

	has_initialized = true;
}

void CL_CALLBACK event_profiling_callback(cl_event event, cl_int, void* pUserData)
{
	simulation_world* self = reinterpret_cast<simulation_world*>(pUserData);
	self->event_callback();
}

void simulation_world::simulate_all()
{
	if(has_initialized == false)
	{
		message_debug("ERROR! Not yet initialized simulation_world.");
		return;
	}
	//current_frame++;
	//message_debug("Current frame: " << current_frame);
	

	for(int a=0; a<simulation_units.size(); a++)
	{
		std::vector<event_info> info = simulation_units[a]->simulate(0.1f);
		for(unsigned int i=0; i<info.size(); i++)
		{
			previous_events.push_back(info.at(i));
			//cl::Event event = info.at(i).event;
			//cl_int err;
			//message_debug(info.at(i).name << ": " << event.getProfilingInfo<CL_PROFILING_COMMAND_END>(&err) - event.getProfilingInfo<CL_PROFILING_COMMAND_START>());
			//message_debug(err);
			//TODO: we get error CL_PROFILING_INFO_NOT_AVAILABLE because the event has not yet finished! Call this function later.
		}
	}

	// TEST: Simulate cell_system:

	std::vector<event_info> info = cell_system->run();
	for(unsigned int i=0; i<info.size(); i++)
	{
		previous_events.push_back(info.at(i));
	}
}

void CL_CALLBACK EventCallback(cl_event event, cl_int, void* pUserData)
{
	simulation_world* self = reinterpret_cast<simulation_world*>(pUserData);
	self->event_callback();
}

void simulation_world::event_callback()
{
	// Profiling:
	std::map<const char*, cl_ulong> timings;
	for(unsigned int i=0; i<previous_events.size(); i++)
	{
		cl::Event event = previous_events.at(i).event;
		cl_ulong duration = event.getProfilingInfo<CL_PROFILING_COMMAND_END>() - event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
		
		cl_int err;
		timings[previous_events.at(i).name] += duration;
		//message_debug(previous_events.at(i).name << ": " << event.getProfilingInfo<CL_PROFILING_COMMAND_END>(&err) - event.getProfilingInfo<CL_PROFILING_COMMAND_START>());
		//message_debug(err);
		//TODO: we get error CL_PROFILING_INFO_NOT_AVAILABLE because the event has not yet finished! Call this function later.
		
	}

	// Get total time and calculate percents of load per key:
	cl_ulong total_time = 0;
	for (auto const& i : timings)
		total_time += i.second;
	
	for (auto const& i : timings)
		message_debug(std::to_string((float)i.second / (float)total_time * 100.0f) << "%, " << std::to_string(i.second) << ": " << i.first);
	
	
	
	{
		std::unique_lock<std::mutex> lk(m_callback);

		// Important to clear the list, else it grows too large:
		previous_events.clear();
		
		auto previous_time = measured_time;
		measured_time = std::chrono::high_resolution_clock::now();
		long nanoseconds =  std::chrono::duration_cast<std::chrono::nanoseconds>(measured_time-previous_time).count();
		message_debug("measured rate: " << 64.0/(double)nanoseconds*1000000000.0 << "Hz | time: " << nanoseconds << "ns");
		//framerate_counter++;
		//summed_frequency += 64.0/(double)nanoseconds*1000000000.0;
		//message_debug("average rate: " << summed_frequency / (double)framerate_counter << "Hz");
	}

	
	
	// Initiate next batch:
	if(running == true)
	{
		for(int i=0; i<64; i++)
		{
			simulate_all();
			
			
		}
		//std::chrono::milliseconds timespan(100);
		//std::this_thread::sleep_for(timespan);
		
		//TODO: find out if the next line is thread-safe?
		callbacks_running--;
		flush();
	} else {
		message_debug("Stopping simulation stepping.");

		// Signal that the thread has finished and the object can be destroyed:
		std::unique_lock<std::mutex> lk(m_callback);
		callback_has_finished = true;
		callbacks_running--;
		cv_callback.notify_one();
	}
	
}

void simulation_world::flush()
{
	int status = queue.enqueueMarkerWithWaitList(nullptr, &event_finish);
	if(status == CL_SUCCESS)
	{
		event_finish.setCallback(CL_COMPLETE, &EventCallback, this);
	} else {
		message_debug("Setting callback failed!");
	}
	//TODO: find out if the next line is thread-safe?
	callbacks_running++;
	int flush_status = queue.flush();
	if(flush_status != CL_SUCCESS)
	{
		message_error("Flush FAILED!");
		message_error("Flush failed with error code: " << flush_status);
		callbacks_running = 0;
		running = false;

		// TODO:Emit error!
	}
}

void simulation_world::finish()
{
	flush();
	if (queue.finish() != CL_SUCCESS);
		message_debug("Error during clFinish");
}

simulation_world::~simulation_world()
{
	message_debug("Destroying simulation world.");
	
    running = false;
	
	//queue.finish();
	event_finish.wait();
	queue.finish();

	// Wait for the callback thread to finish:
	std::unique_lock<std::mutex> lk(m_callback);
	//while (callbacks_running)
	//{
	//	message_debug(callbacks_running);
	//	cv_callback.wait(lk);
	//}

	if(cv_callback.wait_for(lk, std::chrono::milliseconds(2000), [this]{return callbacks_running == 0;})) 
	{
		message_debug("Thread finished waiting. i == " << callbacks_running);
	} else {
        message_debug("Thread timed out. i == " << callbacks_running);
	}

	// TODO: make sure it does not time-out!

	delete_counter++;
	message_debug(delete_counter);
	
	message_debug("!! The simulation has finished !!");

	if(cell_system != nullptr)
		delete cell_system;
}

#ifndef SIMULATION_WORLD_H
#define SIMULATION_WORLD_H

#include "main.h"

#include "core/sim_unit_template.h"

//#include "simulator/unit_cell.h"

#ifndef NO_UI
#include <gtkmm/glarea.h>
#endif // NO_UI


#include <CL/cl2.hpp>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "cell_simulator/cell_system.h"

namespace simulation
{
    //struct promotor
    //{
    //
    //};


	/********************************************//**
	 * This object represents one full simulation sandbox,
	 * including all simulation components, as well as the
	 * OpenCL contexts.
	 ***********************************************/
    class simulation_world
    {
    public:
		simulation_world();

		simulation_world(const simulation_world&) = delete;
		simulation_world& operator=(const simulation_world&) = delete;

        virtual ~simulation_world();

        std::vector<std::unique_ptr<sim_unit_template>> simulation_units;

		void init(bool use_gl_context = false);

        void simulate_all();
		void flush();
        void finish();

		bool running = false;

		void event_callback();		// The real callback function calls this class callback function.
		void after_destroy_event();	 // Called by the callback.

		void initialize_render();
		void render_all(camera gl_camera);
		

		//cl::Event on_finish_event;
    protected:
    private:
    	bool has_initialized = false;
		bool callback_has_finished = false;
		unsigned int callbacks_running = 0;

    	// Platform and hardware information:
    	std::vector<cl::Platform> all_platforms;
		std::vector<cl::Device> all_devices;

		std::mutex m_callback;
		std::condition_variable cv_callback;

		std::vector<event_info> previous_events;		// For profiling.

		cl::Event event_finish;

		int delete_counter = 0;

		unsigned int framerate_counter = 0;

		unsigned int current_frame = 0;
		
		// Frame counter:
		std::mutex m_frame_counter;
		std::chrono::time_point<std::chrono::high_resolution_clock> measured_time;
		double summed_frequency = 0.0;	   // Used to find average.

		// TODO: avoid use of pointers!
		CellSystem* cell_system = nullptr;

	public:		// TODO: make private?
		cl::Platform default_platform;
		cl::Device default_device;
		cl::Context opencl_context;
		cl::CommandQueue queue;
    };

}

#endif // SIMULATION_WORLD_H

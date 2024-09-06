#ifndef DATA_VARIABLE_H_INCLUDED
#define DATA_VARIABLE_H_INCLUDED

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include "main.h"

#include <CL/cl2.hpp>
#include <vector>
#include <string>
//#include <memory>
//#include <map>

#include "core/event_info.h"
#include "core/particle_system.h"

namespace data_buffer
{
	struct VBO_info
	{
		GLuint VBO;
		bool needs_refresh;
		unsigned int size;
	};

	// A base class for a one-dimensional double-buffered resizable array.
	class AbstractArray
	{
	public:
		virtual void swapBuffers() = 0;
		
		virtual cl::Buffer getFrontBuffer() const = 0;
		virtual cl::Buffer getBackBuffer() const = 0;

		virtual size_t entry_size() const = 0;
		virtual unsigned int used_count() const = 0;
		virtual unsigned int max_count() const = 0;
		virtual size_t memory_footprint() const = 0;

		virtual GLuint getVBO(cl::CommandQueue& queue) = 0;

		virtual std::vector<event_info> performDuplication(cl::CommandQueue& queue,
		                                           cl::Buffer empty_particles,
		                                           cl::Buffer copied_particles,
		                                           unsigned int copied_count) = 0;

		virtual std::vector<event_info> performDeletion(cl::CommandQueue& queue,
		                                                cl::Buffer empty_particles,
		                                                unsigned int empty_count) = 0;
	};
	
	// A one-dimensional double-buffered resizable array of type T. T can be any of the cl core datatypes.
	template <class T>
	class Array : AbstractArray
	{
	public:
		// Deprecated? Constructor. "size" indicates the number of elements in the array.
		Array(cl::Context& context, unsigned int max_count, cl::Program& standard_functions) : m_context(context)
		{
			// TODO: allow for custom read_write settings:
			m_buffers[0] = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(T)*max_count);
			m_buffers[1] = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(T)*max_count);
			m_max_count = max_count;

			// Some preprocessor magic to extract the right kernel:
			duplication_kernel = cl::Kernel(standard_functions, ("append_particles_" + std::to_string(sizeof(T))).c_str());
			deletion_kernel = cl::Kernel(standard_functions, ("delete_particles_" + std::to_string(sizeof(T))).c_str());
		}

		// Constructor. "size" indicates the number of elements in the array.
		Array(ParticleSystem& parent_system,
		      cl::Program& standard_functions,
		      std::function<cl::Kernel(unsigned int)> custom_duplication_function = nullptr,
		      std::function<cl::Kernel(unsigned int)> custom_deletion_function = nullptr)
				: m_context(parent_system.getContext())
		{
			m_max_count = parent_system.getMaximalParticleCount();
			parent_system.manageArray(this);
			
			// TODO: allow for custom read_write settings:
			m_buffers[0] = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(T)*m_max_count);
			m_buffers[1] = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(T)*m_max_count);

			m_duplication_function = custom_duplication_function;
			m_deletion_function = custom_deletion_function;
			
			// Some preprocessor magic to extract the right kernel:
			duplication_kernel = cl::Kernel(standard_functions, ("append_particles_" + std::to_string(sizeof(T))).c_str());
			deletion_kernel = cl::Kernel(standard_functions, ("delete_particles_" + std::to_string(sizeof(T))).c_str());
		}

		void swapBuffers() override { buffer_index = !buffer_index; }
		
		cl::Buffer getFrontBuffer() const override { return m_buffers[buffer_index]; }
		cl::Buffer getBackBuffer() const override { return m_buffers[!buffer_index]; }

		size_t entry_size() const override { return sizeof(T); }
		unsigned int used_count() const override { return m_used_count; }
		unsigned int max_count() const override { return m_max_count; }
		size_t memory_footprint() const override { return m_max_count*sizeof(T); }

		void append(cl::CommandQueue& queue, const std::vector<T>& values)
		{
			// TODO: throw error if trying to append more than allocated!
			if(values.size() + m_used_count > m_max_count)
				return;

			// TODO: write to front or back buffer?
			queue.enqueueWriteBuffer(getFrontBuffer(), CL_FALSE, 0, sizeof(T)*values.size(), &values[0]);
			queue.enqueueWriteBuffer(getBackBuffer(), CL_FALSE, 0, sizeof(T)*values.size(), &values[0]);

			m_used_count += values.size();
		}

		GLuint getVBO(cl::CommandQueue& queue) override
		{
			// Initialize new VBO when necessary:
			if (VBO == 0)
			{
				glGenBuffers(1, &VBO);
			}

			if (VBO_needs_refresh == true)
			{
		
				//VBO_needs_refresh = false;

				cl::Buffer buffer = m_buffers[buffer_index];
				size_t size = memory_footprint();

				// TODO: use glSubBufferData for performance improvement!
				glBindBuffer(GL_ARRAY_BUFFER, VBO);

				char* array_test_out_p = new char[size];

				queue.enqueueReadBuffer(buffer, CL_TRUE, 0, size, array_test_out_p);
		

				// TODO: remove any and all blocking calls!
				queue.finish();
	
				glBufferData(GL_ARRAY_BUFFER, size, array_test_out_p, GL_DYNAMIC_DRAW);

				delete[] array_test_out_p;
			}
	
			return VBO;
		}

		std::vector<event_info> performDuplication(cl::CommandQueue& queue,
		                                           cl::Buffer empty_particles,
		                                           cl::Buffer copied_particles,
		                                           unsigned int copied_count) override
		{
			// Make sure there is no buffer overflow:
			if(m_used_count + copied_count > m_max_count)
				return {};
				// TODO: Throw error: buffer overflow!

			// Perform the custom duplication function, if it exists:
			cl::Kernel active_kernel;
			if(m_duplication_function != nullptr)
			{
				active_kernel = m_duplication_function(copied_count);
			} else {
				active_kernel = duplication_kernel;
			}

			// Set the standard arguments and run the kernel:
			active_kernel.setArg(0, m_buffers[!buffer_index]);
			active_kernel.setArg(1, copied_particles);
			active_kernel.setArg(2, m_used_count);
			
			queue.enqueueNDRangeKernel(active_kernel, cl::NullRange, cl::NDRange(copied_count), cl::NullRange);
			
			m_used_count += copied_count;
			
			return {};
		}

		std::vector<event_info> performDeletion(cl::CommandQueue& queue,
		                                        cl::Buffer empty_particles,
		                                        unsigned int empty_count) override
		{
			// Copy all particle data:
			if(m_used_count < empty_count)
				return {};
				// TODO: Throw error: buffer underflow!

			// Perform the custom duplication function, if it exists:
			cl::Kernel active_kernel;
			if(m_deletion_function != nullptr)
			{
				active_kernel = m_deletion_function(empty_count);
			} else {
				active_kernel = deletion_kernel;
			}

			// Set the standard arguments and run the kernel:
			// TODO: why isn't this double buffered?
			deletion_kernel.setArg(0, m_buffers[!buffer_index]);
			deletion_kernel.setArg(1, m_buffers[!buffer_index]);
			deletion_kernel.setArg(2, empty_particles);
			deletion_kernel.setArg(3, m_used_count);
			deletion_kernel.setArg(4, empty_count);
			
			queue.enqueueNDRangeKernel(deletion_kernel, cl::NullRange, cl::NDRange(empty_count), cl::NullRange);

			m_used_count -= empty_count;
	
			return {};
		}
		
	private:
		cl::Buffer m_buffers[2];
		unsigned int m_used_count = 0;
		unsigned int m_max_count;

		cl::Context& m_context;

		cl::Kernel duplication_kernel;
		cl::Kernel deletion_kernel;

		bool buffer_index = false;

		GLuint VBO = 0;
		bool VBO_needs_refresh = true;

		std::function<cl::Kernel(unsigned int)> m_duplication_function = nullptr;
		std::function<cl::Kernel(unsigned int)> m_deletion_function = nullptr;
	};

	// A two-dimensional double-buffered array of type T, which can be any of the cl core datatypes.
	template <class T>
	class Array2D
	{
	public:
		// Constructor. "size" indicates the number of elements in the array.
		Array2D(cl::Context& context, unsigned int count_x, unsigned int count_y) : m_context(context)
		{
			// TODO: allow for custom read_write settings:
			m_buffers[0] = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(T)*count_x*count_y);
			m_buffers[1] = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(T)*count_x*count_y);
			m_count[0] = count_x;
			m_count[1] = count_y;
		}

		void swapBuffers() { buffer_index = !buffer_index; }
		
		cl::Buffer getFrontBuffer() const { return m_buffers[buffer_index]; }
		cl::Buffer getBackBuffer() const { return m_buffers[!buffer_index]; }
		
		size_t entry_size() const { return sizeof(T); }
		unsigned int count(unsigned int dimension) const { return m_count.at(dimension); }
		size_t memory_footprint() const { return m_count.at(0)*m_count.at(1)*sizeof(T); }
		
	private:
		cl::Buffer m_buffers[2];
		std::array<unsigned int, 2> m_count;

		cl::Context& m_context;

		bool buffer_index = false;
	};

	// A two-dimensional double-buffered array of type T, which can be any of the cl core datatypes.
	template <class T>
	class Array3D
	{
	public:
		// Constructor. "size" indicates the number of elements in the array.
		Array3D(cl::Context& context, unsigned int count_x, unsigned int count_y, unsigned int count_z) : m_context(context)
		{
			// TODO: allow for custom read_write settings:
			m_buffers[0] = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(T)*count_x*count_y*count_z);
			m_buffers[1] = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(T)*count_x*count_y*count_z);
			m_count[0] = count_x;
			m_count[1] = count_y;
			m_count[2] = count_z;
		}

		void swapBuffers() { buffer_index = !buffer_index; }
		
		cl::Buffer getFrontBuffer() const { return m_buffers[buffer_index]; }
		cl::Buffer getBackBuffer() const { return m_buffers[!buffer_index]; }

		size_t entry_size() const { return sizeof(T); }
		unsigned int count(unsigned int dimension) const { return m_count.at(dimension); }
		size_t memory_footprint() const { return m_count.at(0)*m_count.at(1)*m_count.at(2)*sizeof(T); }
		
	private:
		cl::Buffer m_buffers[2];
		std::array<unsigned int, 3> m_count;

		cl::Context& m_context;

		bool buffer_index = false;
	};

}

#endif // DATA_VARIABLE_H_INCLUDED

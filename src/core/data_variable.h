/**
 * @file data_variable.h
 * @brief Double-buffered GPU array classes for particle systems.
 *
 * This file defines abstract and templated classes for one-, two-,
 * and three-dimensional arrays that are double-buffered and suitable
 * for GPU operations using OpenCL. These arrays support duplication
 * and deletion of particles, GPU buffer management, and optional
 * OpenGL VBO integration.
 *
 * @author Nathan Boogerd
 * @date 2025-12-05
 */

#ifndef DATA_VARIABLE_H_INCLUDED
#define DATA_VARIABLE_H_INCLUDED

#include "main.h"

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include <CL/opencl.hpp>
#include <vector>
#include <string>
//#include <memory>
//#include <map>

#include "core/event_info.h"
#include "core/particle_system.h"

namespace data_buffer
{
	/**
     * @struct VBO_info
     * @brief Stores OpenGL VBO information for GPU buffers.
     */
	struct VBO_info
	{
		GLuint VBO;
		bool needs_refresh;
		unsigned int size;
	};

	class AbstractDoubleBuffer
	{
	public:
		/// Swap front and back buffers
		virtual void swapBuffers() = 0;

		/// Get the front buffer for read operations
		virtual cl::Buffer getFrontBuffer() const = 0;

		/// Get the back buffer for write operations
		virtual cl::Buffer getBackBuffer() const = 0;

		/// Get size of one element
		virtual size_t entry_size() const = 0;

		/// Compute the memory footprint of the array in bytes
		virtual size_t memory_footprint() const = 0;

	private:
		//cl::BufferGL render_buffer;
		//GLuint VBO = 0;
		//bool VBO_needs_refresh = true;
		//bool is_shared_with_opengl;
	};

    /**
     * @class AbstractParticleData
     * @brief Interface for a one-dimensional double-buffered resizable GPU array.
     *
     * Defines the common interface for arrays managed by the ParticleSystem.
     * Supports duplication, deletion, buffer swapping, and optional VBO
     * integration.
     */
	class AbstractParticleData : public AbstractDoubleBuffer
	{
	public:
		/// Get number of elements currently used
		virtual unsigned int used_count() const = 0;

		/// Get the maximum number of elements that can be stored
		virtual unsigned int max_count() const = 0;

		/**
         * @brief Duplicate elements in the array using a GPU kernel.
         * @param queue OpenCL command queue
         * @param empty_particles Buffer of empty particle indices
         * @param copied_particles Buffer of particle indices to copy
         * @param copied_count Number of particles to duplicate
         * @return Vector of event_info recording the operation
         */
		virtual std::vector<event_info> performDuplication(cl::CommandQueue& queue,
		                                           cl::Buffer empty_particles,
		                                           cl::Buffer copied_particles,
		                                           unsigned int copied_count,
												   std::vector<cl::Event>& wait_for_events) = 0;

		/**
         * @brief Delete elements from the array using a GPU kernel.
         * @param queue OpenCL command queue
         * @param empty_particles Buffer to receive emptied particle indices
         * @param empty_count Number of particles to delete
         * @return Vector of event_info recording the operation
         */
		virtual std::vector<event_info> performDeletion(cl::CommandQueue& queue,
		                                                cl::Buffer empty_particles,
		                                                unsigned int empty_count) = 0;
	};

	// Some tricks with which to detect whether a type is a cl::array or not:
	template <typename>
	struct is_cl_array : std::false_type {};

	template <typename U, size_t N>
	struct is_cl_array<cl::array<U, N>> : std::true_type {};

	/**
     * @class DoubleBuffer
     * @brief One-dimensional double-buffered GPU array of type T.
     *
     * @tparam T Type of elements stored in the array
     */
	template <typename T>
	class FixedSizeArray : public AbstractDoubleBuffer
	{
	public:
		size_t entry_size() const override {
			if constexpr (is_cl_array<T>::value) {
				return sizeof(typename T::value_type) * std::tuple_size<T>::value;
			} else {
				return sizeof(T);
			}
		}

		size_t memory_footprint() const override { return m_max_count*entry_size(); }
	private:
		size_t m_max_count = 0;
	};
	
	/**
     * @class ParticleData
     * @brief One-dimensional double-buffered GPU array of type T.
     *
     * Supports particle duplication and deletion, optional custom kernels,
     * and OpenGL VBO integration. T must be a valid OpenCL core datatype.
     *
     * @tparam T Type of elements stored in the array
     */
	template <typename T>
	class ParticleData : public AbstractParticleData
	{
	public:
		size_t entry_size() const override {
			if constexpr (is_cl_array<T>::value) {
				return sizeof(typename T::value_type) * std::tuple_size<T>::value;
			} else {
				return sizeof(T);
			}
		}
		unsigned int used_count() const override { return m_used_count; }
		unsigned int max_count() const override { return m_max_count; }
		size_t memory_footprint() const override { return m_max_count*entry_size(); }

		/**
         * @brief Construct array managed by a ParticleSystem.
         * @param parent_system ParticleSystem managing this array
         * @param standard_functions OpenCL program containing kernels
         * @param custom_duplication_function Optional custom duplication kernel generator
         * @param custom_deletion_function Optional custom deletion kernel generator
         */
		ParticleData(ParticleSystem& parent_system,
		      std::function<cl::Kernel(unsigned int)> custom_duplication_function = nullptr,
		      std::function<cl::Kernel(unsigned int)> custom_deletion_function = nullptr)
				: m_context(parent_system.getContext()), is_shared_with_opengl(parent_system.isSharedWithOpenGL())
		{
			m_max_count = parent_system.getMaximalParticleCount();
			parent_system.manageArray(this);

			if (is_shared_with_opengl) // TODO: properly release these objects on destruction.
			{
				glGenBuffers(1, &VBO);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, entry_size()*m_max_count, nullptr, GL_DYNAMIC_DRAW);
				render_buffer = cl::BufferGL(m_context, CL_MEM_READ_WRITE, VBO);
			}
			
			// TODO: allow for custom read_write settings:
			m_buffers[0] = cl::Buffer(m_context, CL_MEM_READ_WRITE, entry_size()*m_max_count);
			m_buffers[1] = cl::Buffer(m_context, CL_MEM_READ_WRITE, entry_size()*m_max_count);

			m_duplication_function = custom_duplication_function;
			m_deletion_function = custom_deletion_function;
			
			// Some preprocessor magic to extract the right kernel:
			initialize_standard_kernels(parent_system.getStandardParticleFunctions());
		}

		void initialize_standard_kernels(const cl::Program& standard_functions)
		{
			try {
				if constexpr (is_cl_array<T>::value) {
					duplication_kernel = cl::Kernel(standard_functions, ("append_particles_array_" + std::to_string(sizeof(typename T::value_type))).c_str());
					message_debug("Created kernel named ", "append_particles_array_" + std::to_string(sizeof(typename T::value_type)), " with index ", duplication_kernel.get());
					// TODO: fix deletion kernel!!
					deletion_kernel = cl::Kernel(standard_functions, ("delete_particles_" + std::to_string(sizeof(typename T::value_type))).c_str());
				} else {
					duplication_kernel = cl::Kernel(standard_functions, ("append_particles_" + std::to_string(entry_size())).c_str());
					message_debug("Created kernel named ", "append_particles_" + std::to_string(entry_size()), " with index ", duplication_kernel.get());
					deletion_kernel = cl::Kernel(standard_functions, ("delete_particles_" + std::to_string(entry_size())).c_str());
				}
			} catch (const cl::Error& error) {
				message_debug("error:", error.err());
				throw error;
			}
		}

		/// Swap front and back buffers
		void swapBuffers() override { buffer_index = !buffer_index; }

		// Front buffer is the read buffer, back buffer is the write buffer.
		// The read buffer must always be consistent.
		cl::Buffer getFrontBuffer() const override { return m_buffers[buffer_index]; }
		cl::Buffer getBackBuffer() const override { return m_buffers[!buffer_index]; }

		/**
         * @brief Append values to the array.
         * @param queue OpenCL command queue
         * @param values Vector of values to append
         */
		void append(cl::CommandQueue& queue, const std::vector<T>& values)
		{
			// TODO: throw error if trying to append more than allocated!
			if(values.size() + m_used_count > m_max_count)
				return;

			message_debug(
				"entry_size()*values.size() = ", entry_size()*values.size(),
				"entry_size() = ", entry_size(),
				"values.size() = ", values.size()
			);

			// TODO: write to front or back buffer?
			try {
				queue.enqueueWriteBuffer(getFrontBuffer(), CL_FALSE, 0, entry_size()*values.size(), &values[0]);
				queue.enqueueWriteBuffer(getBackBuffer(), CL_FALSE, 0, entry_size()*values.size(), &values[0]);
			} catch (const cl::Error& error) {
				message_debug("error:", error.err());
				throw error;
			}
			

			// swapBuffers();

			m_used_count += values.size();
		}

		std::vector<event_info> performDuplication(cl::CommandQueue& queue,
		                                           cl::Buffer list_of_particles_to_delete,
		                                           cl::Buffer list_of_particles_to_copy,
		                                           unsigned int amount_of_particles_to_copy,
												   std::vector<cl::Event>& wait_for_events) override
		{
			EventLog log;
			// Make sure there is no buffer overflow:
			if(m_used_count + amount_of_particles_to_copy > m_max_count)
				return {};
				// TODO: Throw error: buffer overflow!

			// Perform the custom duplication function, if it exists:
			cl::Kernel active_kernel = (m_duplication_function != nullptr)
									? m_duplication_function(amount_of_particles_to_copy)
									: duplication_kernel;
			
			cl::Event duplication_kernel_finished;
			cl::Event copy_event;

			queue.enqueueCopyBuffer(getFrontBuffer(), getBackBuffer(), 0, 0, m_used_count*entry_size(), nullptr, &copy_event);

			std::vector<cl::Event> dependencies_for_duplication_kernel = { copy_event };
			for (auto& event : wait_for_events)
			{
				dependencies_for_duplication_kernel.push_back(event);
			}

			// Set the standard arguments and run the kernel:
			active_kernel.setArg(0, getFrontBuffer());
			active_kernel.setArg(1, getBackBuffer());
			active_kernel.setArg(2, list_of_particles_to_copy);
			active_kernel.setArg(3, (int)m_used_count);

			cl::NDRange global_range;
			if constexpr (is_cl_array<T>::value) {
				global_range = cl::NDRange(std::tuple_size<T>::value, amount_of_particles_to_copy);
			} else {
				global_range = cl::NDRange(amount_of_particles_to_copy);
			}

			queue.enqueueNDRangeKernel(
				active_kernel,
				cl::NullRange,
				global_range,
				cl::NullRange,
				&dependencies_for_duplication_kernel,
				&duplication_kernel_finished
			);

			std::vector<cl::Event> dependencies_for_memory_barrier = { duplication_kernel_finished };

			queue.enqueueBarrierWithWaitList(
				&dependencies_for_memory_barrier,
				nullptr
			);

			//message_debug("A m_used_count = ", m_used_count, ", amount_of_particles_to_copy = ", amount_of_particles_to_copy);

			swapBuffers();
			
			m_used_count += amount_of_particles_to_copy;
			
			log.add(event_info("PARTICLE_DATA: front_to_back_buffer_copy_event_before_duplication", event_info::kernel, copy_event));
			log.add(event_info("PARTICLE_DATA: duplication_kernel", event_info::kernel, duplication_kernel_finished));
			return std::move(log.get());
		}

		std::vector<event_info> performDeletion(cl::CommandQueue& queue,
		                                        cl::Buffer indices_of_particles_to_delete,
		                                        unsigned int amount_of_particles_to_delete) override
		{
			// Copy all particle data:
			if(m_used_count < amount_of_particles_to_delete)
				return {};
				// TODO: Throw error: buffer underflow!

			// Perform the custom deletion function, if it exists:
			cl::Kernel active_kernel = (m_deletion_function != nullptr)
                                     ? m_deletion_function(amount_of_particles_to_delete)
                                     : deletion_kernel;

			// Set the standard arguments and run the kernel:
			active_kernel.setArg(0, getBackBuffer());
			active_kernel.setArg(1, getBackBuffer());
			active_kernel.setArg(2, indices_of_particles_to_delete);
			active_kernel.setArg(3, m_used_count);
			active_kernel.setArg(4, amount_of_particles_to_delete);
			
			queue.enqueueNDRangeKernel(active_kernel, cl::NullRange, cl::NDRange(amount_of_particles_to_delete), cl::NullRange);

			//swapBuffers();

			m_used_count -= amount_of_particles_to_delete;
	
			return {};
		}

		/**
         * @brief Get an OpenGL VBO representing the array.
         * @param queue OpenCL command queue to synchronize data transfer
         * @return GLuint OpenGL buffer handle
         */
		GLuint getOpenGLBuffer(cl::CommandQueue& queue)
		{
			// Initialize new VBO when necessary:
			if (VBO == 0)
			{
				glGenBuffers(1, &VBO);
				VBO_needs_refresh = true;
			}

			if (!VBO_needs_refresh) return VBO;

			const size_t bytes = memory_footprint();
			if (bytes == 0) return VBO;

			if (is_shared_with_opengl)
			{
				// If context-shared, we can copy directly on GPU:
				// (This may be sped up a bit more by not copying altogether,
				// but that introduces some extra thread management to prevent
				// concurrent access. This is TODO.)
				glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
				glFlush();

				std::vector<cl::Memory> shared_objects = {{ render_buffer }};

				// TODO: make GL object acquisition batched rather than individual like it is now.

				cl::Event acquire_event;

				// Acquire for OpenCL
				queue.enqueueAcquireGLObjects(&shared_objects, nullptr, &acquire_event);

				cl::Event copy_event;

				std::vector<cl::Event> dependencies_for_copy = { acquire_event };

				// OpenCL kernels or buffer operations go here
				// e.g. queue.enqueueWriteBuffer(...)
				queue.enqueueCopyBuffer(getFrontBuffer(), render_buffer, 0, 0, memory_footprint(), &dependencies_for_copy, &copy_event);

				std::vector<cl::Event> dependencies_for_release = { copy_event };

				// Release back to OpenGL
				queue.enqueueReleaseGLObjects(&shared_objects, &dependencies_for_release);
				queue.flush();

			} else {
				// If not context-shared, we have to copy through CPU (slow!):
				std::vector<std::byte> transfer_array(bytes);

				queue.enqueueReadBuffer(getFrontBuffer(), CL_TRUE, 0, bytes, transfer_array.data());

				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, bytes, transfer_array.data(), GL_DYNAMIC_DRAW);
			}
			
			VBO_needs_refresh = true;
			return VBO;
		}
		
	private:
		cl::Buffer m_buffers[2];
		cl::BufferGL render_buffer;
		size_t m_used_count = 0;
		size_t m_max_count;

		cl::Context& m_context;

		cl::Kernel duplication_kernel;
		cl::Kernel deletion_kernel;

		bool buffer_index = false;

		GLuint VBO = 0;
		bool VBO_needs_refresh = true;
		bool is_shared_with_opengl;

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

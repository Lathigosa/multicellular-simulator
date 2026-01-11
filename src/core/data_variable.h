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

    /**
     * @class AbstractParticleData
     * @brief Interface for a one-dimensional double-buffered resizable GPU array.
     *
     * Defines the common interface for arrays managed by the ParticleSystem.
     * Supports duplication, deletion, buffer swapping, and optional VBO
     * integration.
     */
	class AbstractParticleData
	{
	public:
		/// Swap front and back buffers
		virtual void swapBuffers() = 0;
		
		/// Get the front buffer for read/write operations
		virtual cl::Buffer getFrontBuffer() const = 0;

		/// Get the back buffer for read/write operations
		virtual cl::Buffer getBackBuffer() const = 0;

		/// Get size of one element
		virtual size_t entry_size() const = 0;

		/// Get number of elements currently used
		virtual unsigned int used_count() const = 0;

		/// Get the maximum number of elements that can be stored
		virtual unsigned int max_count() const = 0;

		/// Compute the memory footprint of the array in bytes
		virtual size_t memory_footprint() const = 0;

		/**
         * @brief Get an OpenGL VBO representing the array.
         * @param queue OpenCL command queue to synchronize data transfer
         * @return GLuint OpenGL buffer handle
         */
		virtual GLuint getVBO(cl::CommandQueue& queue) = 0;

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
		                                           unsigned int copied_count) = 0;

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
				return sizeof(typename T::value_type)*T::size();
			} else {
				return sizeof(T);
			}
		}
		unsigned int used_count() const override { return m_used_count; }
		unsigned int max_count() const override { return m_max_count; }
		size_t memory_footprint() const override { return m_max_count*entry_size(); }

		/**
         * @brief Construct array with maximum size.
         * @param context OpenCL context
         * @param max_count Maximum number of elements
         * @param standard_functions OpenCL program containing kernels
		 * @note Deprecated?
         */ 
		ParticleData(cl::Context& context, size_t max_count, cl::Program& standard_functions) : m_context(context), m_max_count(max_count)
		{
			// TODO: allow for custom read_write settings:
			m_buffers[0] = cl::Buffer(context, CL_MEM_READ_WRITE, entry_size()*m_max_count);
			m_buffers[1] = cl::Buffer(context, CL_MEM_READ_WRITE, entry_size()*m_max_count);

			// Some preprocessor magic to extract the right kernel:
			initialize_standard_kernels(standard_functions);
		}

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
				: m_context(parent_system.getContext())
		{
			m_max_count = parent_system.getMaximalParticleCount();
			parent_system.manageArray(this);
			
			// TODO: allow for custom read_write settings:
			m_buffers[0] = cl::Buffer(m_context, CL_MEM_READ_WRITE, entry_size()*m_max_count);
			m_buffers[1] = cl::Buffer(m_context, CL_MEM_READ_WRITE, entry_size()*m_max_count);

			m_duplication_function = custom_duplication_function;
			m_deletion_function = custom_deletion_function;
			
			// Some preprocessor magic to extract the right kernel:
			initialize_standard_kernels(parent_system.getStandardParticleFunctions());
		}

		void initialize_standard_kernels(cl::Program& standard_functions)
		{
			if constexpr (is_cl_array<T>::value) {
				duplication_kernel = cl::Kernel(standard_functions, ("append_particles_array_" + std::to_string(sizeof(typename T::value_type))).c_str());
				// TODO: fix deletion kernel!!
				deletion_kernel = cl::Kernel(standard_functions, ("delete_particles_" + std::to_string(sizeof(typename T::value_type))).c_str());
			} else {
				duplication_kernel = cl::Kernel(standard_functions, ("append_particles_" + std::to_string(entry_size())).c_str());
				deletion_kernel = cl::Kernel(standard_functions, ("delete_particles_" + std::to_string(entry_size())).c_str());
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

			// TODO: write to front or back buffer?
			// queue.enqueueWriteBuffer(getFrontBuffer(), CL_FALSE, 0, sizeof(T)*values.size(), &values[0]);
			queue.enqueueWriteBuffer(getBackBuffer(), CL_FALSE, 0, sizeof(T)*values.size(), &values[0]);

			m_used_count += values.size();
		}

		GLuint getVBO(cl::CommandQueue& queue) override
		{
			// Initialize new VBO when necessary:
			if (VBO == 0)
			{
				glGenBuffers(1, &VBO);
				VBO_needs_refresh = true;
			}

			if (!VBO_needs_refresh) return VBO;

			const size_t bytes = m_used_count * sizeof(T);
			if (bytes == 0) return VBO;

			std::vector<std::byte> transfer_array(bytes);

			queue.enqueueReadBuffer(getFrontBuffer(), CL_TRUE, 0, bytes, transfer_array.data());

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
    		glBufferData(GL_ARRAY_BUFFER, bytes, transfer_array.data(), GL_DYNAMIC_DRAW);

			VBO_needs_refresh = true;
			return VBO;
		}

		std::vector<event_info> performDuplication(cl::CommandQueue& queue,
		                                           cl::Buffer indices_of_particles_to_delete,
		                                           cl::Buffer indices_of_particles_to_copy,
		                                           unsigned int amount_of_particles_to_copy) override
		{
			// Make sure there is no buffer overflow:
			if(m_used_count + amount_of_particles_to_copy > m_max_count)
				return {};
				// TODO: Throw error: buffer overflow!

			// Perform the custom duplication function, if it exists:
			cl::Kernel active_kernel = (m_duplication_function != nullptr)
                                     ? m_duplication_function(amount_of_particles_to_copy)
                                     : duplication_kernel;

			// Set the standard arguments and run the kernel:
			active_kernel.setArg(0, getBackBuffer());
			active_kernel.setArg(1, indices_of_particles_to_copy);
			active_kernel.setArg(2, m_used_count);
			
			queue.enqueueNDRangeKernel(active_kernel, cl::NullRange, cl::NDRange(amount_of_particles_to_copy), cl::NullRange);
			
			m_used_count += amount_of_particles_to_copy;
			
			return {};
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
			// TODO: why isn't this double buffered?
			active_kernel.setArg(0, getBackBuffer());
			active_kernel.setArg(1, getBackBuffer());
			active_kernel.setArg(2, indices_of_particles_to_delete);
			active_kernel.setArg(3, m_used_count);
			active_kernel.setArg(4, amount_of_particles_to_delete);
			
			queue.enqueueNDRangeKernel(active_kernel, cl::NullRange, cl::NDRange(amount_of_particles_to_delete), cl::NullRange);

			m_used_count -= amount_of_particles_to_delete;
	
			return {};
		}
		
	private:
		cl::Buffer m_buffers[2];
		size_t m_used_count = 0;
		size_t m_max_count;

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

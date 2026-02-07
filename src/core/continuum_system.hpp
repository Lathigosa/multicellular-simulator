/**
 * @file continuum_system.hpp
 * @brief A GPU-based continuum system, such as a sheet.
 *
 * @author Nathan Boogerd
 * @date 2026-02-02
 */

#pragma once

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include <CL/opencl.hpp>
#include <vector>

#include "core/data_system.h"
#include "core/data_variable.h"

class ContinuumSystem : public DataSystem
{
public:
	ContinuumSystem(cl::CommandQueue & command_queue, bool use_opengl_context);
	ContinuumSystem(const ContinuumSystem& from) = delete;			// TODO: add copy and assignment?
	ContinuumSystem& operator=(const ContinuumSystem&) = delete;
	virtual ~ContinuumSystem();

	

	//void manageArray(data_buffer::AbstractParticleData* array);

	//const cl::Program& getStandardParticleFunctions();

protected:
	//data_buffer::FixedSizeArray<cl_float4> vertices;
	//data_buffer::FixedSizeArray<uint> indices;

	unsigned int dimension_x = 8;
	unsigned int dimension_y = 8;
	unsigned int dimension_z = 8;

	//std::vector<data_buffer::AbstractParticleData*> m_managed_arrays;
	
	cl::Program m_standard_functions;

	void setupBuffers();
};
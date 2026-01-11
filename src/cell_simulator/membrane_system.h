#ifndef CELL_SYSTEM_H_INCLUDED
#define CELL_SYSTEM_H_INCLUDED

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include <CL/opencl.hpp>
#include <vector>

#include "core/particle_system.h"
#include "kernels/cell_particle_physics.h"
#include "kernels/cells_as_dots.h"
#include "kernels/particle_marker.h"

class MembraneSystem : public ParticleSystem
{
public:
	MembraneSystem(cl::CommandQueue & command_queue);
	MembraneSystem(const MembraneSystem& from) = delete;				// TODO: add copy and assignment?
	MembraneSystem& operator=(const MembraneSystem&) = delete;

	void build();

	std::vector<event_info> run();

	void initialize_render();
	void render(camera gl_camera);

	virtual ~MembraneSystem();

private:
	cell_particle_physics kernel_physics;
	ParticleMarker kernel_particle_marker;

	cl::Kernel position_duplicator;
	cl::Kernel neighbors_duplicator;

	bool buffer_index = 0;

	// Variables per cell:
	data_buffer::ParticleData<cl_float4> m_position_1;
	data_buffer::ParticleData<cl_float4> m_velocity_1;
	//data_buffer::Array<cl_float4> m_acceleration_1;
	data_buffer::ParticleData<cl_float> m_radius_1;

	data_buffer::ParticleData<cl_uint16> m_neighbors;					// Indices of neighboring cells.

	data_buffer::ParticleData<cl::array<cl_float4, 256>> m_membrane_vertex_positions;
	data_buffer::ParticleData<cl::array<cl_float4, 256>> m_membrane_vertex_normal;
	data_buffer::ParticleData<cl::array<cl_ushort2, 512>> m_membrane_edges;
	data_buffer::ParticleData<cl::array<cl_ushort3, 512>> m_membrane_faces;

	std::vector<event_info> calculatePhysicsStep();
	std::vector<event_info> markParticlesForDivisionOrDeletion();

	// Render functions:
	position_spheres test_renderer;
};



#endif // DATA_SYSTEM_H_INCLUDED

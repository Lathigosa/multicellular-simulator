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
#include "renderer/membrane_renderer.hpp"
#include "cell_simulator/membrane_particle.hpp"

class CellSystem : public ParticleSystem
{
public:
	CellSystem(cl::CommandQueue & command_queue, bool use_opengl_context);
	CellSystem(const CellSystem& from) = delete;				// TODO: add copy and assignment?
	CellSystem& operator=(const CellSystem&) = delete;

	void build();

	std::vector<event_info> run();

	void initialize_render();
	void render(camera gl_camera);

	virtual ~CellSystem();

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

	ParticleMembraneData m_membrane;

	//data_buffer::ParticleData<cl::array<cl_float4, 256>> m_membrane_vertex_positions;
	//data_buffer::ParticleData<cl::array<cl_float4, 256>> m_membrane_vertex_normal;
	//data_buffer::ParticleData<cl::array<cl_ushort2, 512>> m_membrane_edges;
	//data_buffer::ParticleData<cl::array<cl_ushort3, 512>> m_membrane_faces;

	std::vector<event_info> calculatePhysicsStep();
	std::vector<event_info> markParticlesForDivisionOrDeletion();

	// Render functions:
	position_spheres test_renderer;
	MembraneRenderer membrane_renderer;
};



#endif // DATA_SYSTEM_H_INCLUDED

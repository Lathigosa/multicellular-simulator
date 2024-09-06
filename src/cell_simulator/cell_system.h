#ifndef CELL_SYSTEM_H_INCLUDED
#define CELL_SYSTEM_H_INCLUDED

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include "main.h"

#include <CL/cl2.hpp>
#include <vector>
#include <string>
#include <memory>
#include <map>


#include "core/particle_system.h"
#include "core/data_variable.h"
#include "kernels/cell_particle_physics.h"
#include "kernels/cells_as_dots.h"
#include "kernels/particle_marker.h"

class CellSystem : public ParticleSystem
{
public:
	CellSystem(cl::Platform & platform,
	           cl::Device & device,
	           cl::Context & context,
	           cl::CommandQueue & command_queue);
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
	data_buffer::Array<cl_float4> m_position_1;
	data_buffer::Array<cl_float4> m_velocity_1;
	//data_buffer::Array<cl_float4> m_acceleration_1;
	data_buffer::Array<cl_float> m_radius_1;

	data_buffer::Array<cl_uint16> m_neighbors;					// Indices of neighboring cells.
	//data_buffer::Array<cl_uint16> m_membrane_vertex_indices;	// Indices of membrane vertices of this cell.
	//data_buffer::Array<cl_uint16> m_membrane_edge_indices;		// Indices of membrane edges of this cell.

	// Not variables per cell (other datasystem, kinda. TODO: move to different data system):
	//data_buffer::Array<cl_float4> m_membrane_vertices;			// All membrane vertices.
	//data_buffer::Array<cl_uint2> m_membrane_edges;				// All membrane edges (defined by vertex indices).
	//data_buffer::Array<cl_uint16> m_membrane_faces;				// All membrane faces (defined by vertex indices).

	// Render functions:
	position_spheres test_renderer;
};



#endif // DATA_SYSTEM_H_INCLUDED

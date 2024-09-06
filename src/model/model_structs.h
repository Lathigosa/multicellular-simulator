#ifndef MODEL_STRUCTS_H
#define MODEL_STRUCTS_H

#include <glm/glm.hpp>
#include <vector>
#include <epoxy/gl.h>

struct vertex
{
	glm::vec3 position;
	std::vector<GLushort> connected_to;		// A list containing the vertex indices that this vertex is connected to.
};

struct model
{
	std::vector<vertex> vertices;
	std::vector<GLushort> faces;		// Contains the IBO-data to be sent to openGL.

	void add_face();
};


#endif // MODEL_STRUCTS_H

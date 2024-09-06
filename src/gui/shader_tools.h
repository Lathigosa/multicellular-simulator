#ifndef SHADER_TOOLS_H
#define SHADER_TOOLS_H

#include <epoxy/gl.h>

GLuint create_shader(const char * const source_vertex, const char * const source_fragment);

GLuint create_shader_geo(const char * const source_vertex, const char * const source_geometry, const char * const source_fragment);

#endif // SHADER_TOOLS_H

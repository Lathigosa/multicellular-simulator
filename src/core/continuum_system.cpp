#include <filesystem>

#include "continuum_system.hpp"

namespace fs = std::filesystem;

ContinuumSystem::ContinuumSystem(cl::CommandQueue & command_queue, bool use_opengl_context) : DataSystem(command_queue, use_opengl_context)
{
	
}

ContinuumSystem::~ContinuumSystem() = default;
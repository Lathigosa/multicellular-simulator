#include "load_file.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

std::string load_file(const char *filename)
{
	std::ifstream t(filename);
	std::stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();
}

#ifndef VOXEL_INDEX_LIST_H_INCLUDED
#define VOXEL_INDEX_LIST_H_INCLUDED

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

#include "main.h"

#include <CL/cl2.hpp>
#include <vector>
#include <string>
#include <memory>
#include <map>

namespace data_types
{
	// struct containing a double buffer and extras:
	class voxel_index_list
	{
	public:
		voxel_index_list();
        voxel_index_list(const voxel_index_list& from);

        virtual ~voxel_index_list();
			
	private:
		cl::Buffer buffer;
		unsigned int entry_size = 0;
		unsigned int entry_count = 0;
	};
}



#endif // VOXEL_INDEX_LIST_H_INCLUDED

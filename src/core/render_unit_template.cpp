#include "render_unit_template.h"

using namespace simulation;

render_unit_template::render_unit_template()
{

}

render_unit_template::~render_unit_template()
{
	//dtor
}

error render_unit_template::initialize()
{
	return error::success;
}

const std::string render_unit_template::get_name() const
{
	return "undefined";
}

const std::vector<std::string> render_unit_template::get_dependencies() const
{
	return {};
}

error render_unit_template::set_dependency_pointers(
						std::vector<sim_unit_template*> &list
						)
{
	dependency_pointers = list;
	
	return error::success;
}

error render_unit_template::set_dependency_buffer_indices(std::vector<std::string> &list)
{
	message_debug("Adding dependency buffer indices:");

	if(list.size() != dependency_pointers.size())
		exit(1);
	
	dependency_buffer_indices.clear();
	for(unsigned int i=0; i<list.size(); i++)
	{
		dependency_buffer_indices.push_back(dependency_pointers[i]->get_buffer_index(list.at(i)));
		message_debug("[" << i << "]: " << dependency_buffer_indices.back());
	}
	
	return error::success;
}

VBO_info render_unit_template::get_VBO(unsigned int index)
{
	return dependency_pointers[index]->get_buffer_as_VBO_info(dependency_buffer_indices[index], 1);
}


/*error sim_unit_template::set_child_pointers(
						std::vector<sim_unit_template*> &list
						)
{
	dependency_child_pointers = list;
	return error::success;
}*/

error render_unit_template::render(camera gl_camera)
{
	return error::undefined_function;
}

/*error sim_unit_template::signal_buffer_rearrange(const std::string sim_unit_name,
												unsigned int buffer_index,
												buffer_rearrange_info info)
{
	return error::undefined_function;
}*/

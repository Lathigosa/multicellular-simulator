#include "main.h"

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "core/simulation_file_object.h"
#include "core/simulation_collection.h"

#include "core/data_system.h"


namespace py = pybind11;

int main (int argc, char *argv[]);

int call_main()
{
	main(0, 0);
	return 0;
}

//PYBIND11_MAKE_OPAQUE(std::vector<data_type::component>);

PYBIND11_MODULE(gtk3_cell_simulator, m) {
	py::class_<simulation_file_object>(m, "simulation_file_object")
		.def(py::init())
		.def("open_file", &simulation_file_object::open_file, "(Deprecated) Load a LUA file.");
		//.def("__repr__",
		//	[](const simulation_file_object &a) {
		//		return "<Unnamed simulation_file_object>";
		//	};
		//);

	py::class_<simulation::simulation_world>(m, "simulation_collection")
		.def(py::init())
		.def("init", &simulation::simulation_world::init, "Initialize simulation collection.")
		.def("simulate_all", &simulation::simulation_world::simulate_all, "Start simulation. This function must be called after setting up all the simulation parameters.");
	

	m.def("main", &call_main, "(Deprecated) Test the standalone program using this function.");

	// === <data_system.h> === //
	/*
	py::class_<data_system>(m, "DataSystem")
		.def(py::init())
		
		.def("add_function", (std::vector<data_handle> (data_system::*)(std::string name,
		                                            std::shared_ptr<data_function> function,
		                                            std::vector<data_handle> inputs)
		                      )
		     &data_system::add_function,
		     "Append a function at the end of the function list for this DataSystem to execute on each run.")
		.def("add_function", (std::vector<data_handle> (data_system::*)(std::string name,
		                                            std::shared_ptr<data_function> function,
		                                            std::vector<data_handle> inputs,
		                                            std::vector<import_data_handle> imports)
		                      )
		     &data_system::add_function,
		     "Append a function at the end of the function list for this DataSystem to execute on each run.")
		.def("get_variable", &data_system::get_variable, "Get a DataHandle to a variable within this DataSystem.")
		.def("import_variable", &data_system::import_variable, "Import an ImportDataHandle for use in a different DataSystem.")
		.def("get_parameter", &data_system::get_parameter, "Get a DataHandle to a parameter within this DataSystem.")
		.def("import_parameter", &data_system::import_parameter, "Import an ImportDataHandle for use in a different DataSystem.")
		.def("build", &data_system::build, "Construct the code to run this DataSystem's functions. Must be called after initialization and before execution of the code.")
		
		;

	// === <data_types.h> === //

	py::class_<data_type> py_data_type(m, "DataType");
	py_data_type
		.def(py::init())
		.def_readwrite("name", &data_type::name)
		.def_readwrite("components", &data_type::components)
		.def("__repr__",
			[](const data_type &a) {
				return "<DataType named '" + a.get_name() + "'>";
			}
		);
	
	py::class_<data_type::component> py_data_type_component(py_data_type, "Component");
	py_data_type_component
		.def(py::init())
		.def_readwrite("data_type", &data_type::component::data_type)
		.def_readwrite("number_of_entries", &data_type::component::number_of_entries)
		.def_readwrite("own_buffer", &data_type::component::own_buffer);

	py::bind_vector<std::vector<data_type::component>>(py_data_type, "ComponentVector");

	py::enum_<data_type::component::type>(py_data_type_component, "Type")
		.value("Char", data_type::component::type::data_char)
		.value("UChar", data_type::component::type::data_uchar)
		.value("Short", data_type::component::type::data_short)
		.value("UShort", data_type::component::type::data_ushort)
		.value("Int", data_type::component::type::data_int)
		.value("UInt", data_type::component::type::data_uint)
		.value("Long", data_type::component::type::data_long)
		.value("ULong", data_type::component::type::data_ulong)
		.value("Float", data_type::component::type::data_float)
		.value("Double", data_type::component::type::data_double);

	py::enum_<data_type::component::size>(py_data_type_component, "Size")
		.value("Size1", data_type::component::size::size_1)
		.value("Size2", data_type::component::size::size_2)
		.value("Size3", data_type::component::size::size_3)
		.value("Size4", data_type::component::size::size_4)
		.value("Size8", data_type::component::size::size_8)
		.value("Size16", data_type::component::size::size_16);

	// === <data_handle.h> === //
	py::class_<data_handle>(m, "DataHandle")
		.def("__repr__",
			[](const data_handle &a) {
				return "<DataHandle of type '" + a.get_type().get_name() + "'>";
			}
		);
	py::class_<import_data_handle>(m, "ImportDataHandle")
		.def("__repr__",
			[](const import_data_handle &a) {
				return "<ImportDataHandle of type '" + a.get_type().get_name() + "'>";
			}
		);

	

	// === <data_function.h> === //
	py::class_<data_function, std::shared_ptr<data_function>> py_data_function(m, "DataFunction");
	py_data_function
		.def(py::init())
		.def("get_code", &data_function::get_code, "Get the generated code of this DataFunction.")  // TODO: remove this function.
		;

	py::class_<test_function, std::shared_ptr<test_function>>(m, "TestFunction", py_data_function)
		.def(py::init())
		.def("get_code", &test_function::get_code, "Get the generated code of this TestFunction.")  // TODO: remove this function.
		;

	// === <functions/hookian_repel.h> === //
	py::class_<hookian_repel, std::shared_ptr<hookian_repel>>(m, "HookianRepel", py_data_function)
		.def(py::init())
		.def("get_code", &hookian_repel::get_code, "Get the generated code of this HookianRepel.")  // TODO: remove this function.
		;

	// === <core/standard_data_types.h> === //
	py::module m2 = m.def_submodule("type", "Submodule containing standard DataTypes.");
	m2.attr("position") = type::position;
	m2.attr("velocity") = type::velocity;
	m2.attr("acceleration") = type::acceleration;
	m2.attr("radius") = type::radius;
	m2.attr("integer_counter") = type::integer_counter;
	*/
}

#include "intracellular_reactions.h"

using namespace simulation;

#include <iostream>
#include "resources/opencl_kernels.h"
#include <cmath>

#include <CL/cl_gl.h>

#include "utilities/load_file.h"

#include "simulator/intracellular_reactions_lib/intracellular_reactions_lib.h"

#include <cstdlib>
#include <cctype>
#include <map>

#include <clocale>

intracellular_reactions::intracellular_reactions(	cl::Platform & platform_cl,
					cl::Device & device_cl,
					cl::Context & context,
					cl::CommandQueue & command_queue) : particle_template(platform_cl, device_cl, context, command_queue)
{
	// TODO: remove next line, the kernel gets constructed from something else instead:
	kernel_membrane_physics = get_kernel_from_file("cl_kernels/intracellular_reactions.cl", "simulate_reactions");
	kernel_membrane_physics.getWorkGroupInfo(device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, &wg_size);
	kernel_append_buffer = get_kernel_from_file("cl_kernels/append_buffer.cl", "append_buffer");

	// Test count:
	//particle_count = 1*1; 	// TODO: remove
	//max_buffer_size = particle_count * 32*32*16;
	molecular_species_count = 4;
}

intracellular_reactions::~intracellular_reactions()
{
	//dtor
}

error intracellular_reactions::initialize_buffers()
{
	max_buffer_size = dependency_pointers[0]->get_custom_uint("max_particle_count");
	//TODO: resize buffer based on molecular species count!!!
	buffer_concentrations = create_particle_double_buffer("concentrations", sizeof(cl_float4));

	// TEST: load initial values:
	unsigned int particle_count = 1;
	float array_in[4*particle_count];

	array_in[0] = 100.0f;
	array_in[1] = 100.0f;
	array_in[2] = 1.0f;
	array_in[3] = 0.0f;

	cl_float4 fill_pattern;
	fill_pattern.s[0] = 0.0f;
	fill_pattern.s[1] = 0.0f;
	fill_pattern.s[2] = 0.0f;
	fill_pattern.s[3] = 0.0f;

	queue.enqueueWriteBuffer(get_buffer(buffer_concentrations, 1), CL_TRUE, 0, sizeof(cl_float4)*particle_count, array_in);
	queue.enqueueWriteBuffer(get_buffer(buffer_concentrations, 0), CL_TRUE, 0, sizeof(cl_float4)*particle_count, array_in);
	queue.enqueueFillBuffer(get_buffer(buffer_concentrations, 1), fill_pattern, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*(max_buffer_size - particle_count), nullptr);
	queue.enqueueFillBuffer(get_buffer(buffer_concentrations, 0), fill_pattern, sizeof(cl_float4)*particle_count, sizeof(cl_float4)*(max_buffer_size - particle_count), nullptr);

	return error::success;
}

#ifndef NO_UI
error intracellular_reactions::initialize_buffers(std::vector<GLuint> from_gl_buffer)
{
	message_debug("Error: UNIMPLEMENTED FUNCTION!!!");
	assert(false);
	// TODO: if cl_khr_gl_sharing isn't supported, disable this function!

	return error::success;
}
#endif // NO_UI

error intracellular_reactions::organize_unit()
{
	return error::undefined_function;
}

std::vector<event_info> intracellular_reactions::simulate_unit(float step_size)
{
	std::vector<event_info> event_list = {};

	// Perform simulation 8 times:
	for(int i=0; i<8; i++)
	{
		cl::Event event;
		
		kernel_membrane_physics.setArg(0, get_buffer(buffer_concentrations, buffer_index));
		kernel_membrane_physics.setArg(1, get_buffer(buffer_concentrations, !buffer_index));
		kernel_membrane_physics.setArg(2, (unsigned int)particle_count);
		kernel_membrane_physics.setArg(3, step_size*0.0001f / 8.0f);
		queue.enqueueNDRangeKernel(kernel_membrane_physics, cl::NullRange, cl::NDRange((unsigned int)(wg_size*ceil(double(particle_count)/double(wg_size)))), cl::NullRange, nullptr, &event);

		event_list.push_back({event, "iteration"});

		buffer_index = !buffer_index;
	}

	// TODO: enqueuebarrier!!!

	return event_list;
}

error intracellular_reactions::signal_cell_reindex(const std::string sim_unit_name,
											cl::Buffer empty_cells,
		                              		cl::Buffer copied_cells,
		                              		unsigned int copied_count)
{
	if(particle_count + copied_count > max_buffer_size)
		return error::buffer_overflow;
	
	kernel_append_buffer.setArg(0, get_buffer(buffer_concentrations, !buffer_index));
	kernel_append_buffer.setArg(1, copied_cells);
	kernel_append_buffer.setArg(2, particle_count);
	//kernel_append_buffer.setArg(3, 0.0f);
	queue.enqueueNDRangeKernel(kernel_append_buffer, cl::NullRange, cl::NDRange(copied_count), cl::NullRange);

	particle_count += copied_count;
	
	return error::success;
}

struct reaction_info
{
	int line = 0;
	double stoichiometry = 1.0;
	bool reactant = true;		// false = product, true = reactant. Determines sign in the rate equation.
};

error intracellular_reactions::set_reactions(std::string reaction_definitions)
{
	// TODO: improve this code!
	// Construct the kernel code from the definition file:

	std::vector<std::string> rate_equations;					// List containing a rate equations per reaction (each line).
	std::vector<double> rate_constants;							// Indices here match rate_equations.
	std::multimap<std::string, reaction_info> species_on_line;		// Tells the compiler what line this instance of the chemical species is located (for finding the corresponding rate equation).
	std::vector<std::string> species;						// List containing the names of all chemical species exactly once.

	// Pass 1: find all species names:
	{
		enum {
			STOICHIOMETRY_CONSTANT,
			SPECIES_NAME,
			RATE_CONSTANT,
			PLUS_ARROW_OR_COMMA,
			
		} expected_text = STOICHIOMETRY_CONSTANT;
		char expected_char = 0;

		
		std::string current_stoichiometry;	 // Stoichiometry constant
		std::string current_name;
		int current_line = 0;
		bool start_of_line = true;
		bool reactants = true;
		std::string current_rate_equation = "";
		std::string current_rate_constant;
		for(unsigned int i=0; i<reaction_definitions.size(); i++)
		{
			char current_char = reaction_definitions.at(i);

			//if(current_char != expected_char && expected_char != 0)
			//	return error::general_error;

			if(isblank(current_char))
				continue;

			

			if((isdigit(current_char) || current_char == '.'))
			{
				if(expected_text == STOICHIOMETRY_CONSTANT)
				{
					current_stoichiometry += current_char;
					start_of_line = false;
					continue;
				} else if(expected_text == RATE_CONSTANT)
				{
					current_rate_constant += current_char;
					continue;
				} else {
					message_debug("ERROR: did not expect number!");
				}
			} else if(isalpha(current_char) && expected_char == 0)
			{
				if(expected_text == STOICHIOMETRY_CONSTANT || expected_text == SPECIES_NAME)
				{
					
					expected_text = SPECIES_NAME;
					current_name += current_char;
					start_of_line = false;
					continue;
				} else {
					message_debug("ERROR: expected <something else>");
				}
				
			} else
			{
				if (current_name.size() != 0)
				{
					// Add to rate equation:
					if(reactants == true)
					{
						current_rate_equation += "*in_concentrations[";
						current_rate_equation += current_name;
						current_rate_equation += "]";
					}
				
					// End of name. Check if the name is already present in the list:
					if(species_on_line.count(current_name) == 0)
						species.push_back(current_name);

					reaction_info info;
					info.line = current_line;
				
					if(!current_stoichiometry.empty())
						info.stoichiometry = std::stod(current_stoichiometry);
				
					info.reactant = reactants;

					species_on_line.insert(std::pair<std::string, reaction_info>(current_name, info));
					current_name.clear();
					current_stoichiometry.clear();

					start_of_line = false;
				}

				if(current_char == '\r' or current_char == '\n')
				{
					if(start_of_line == false)
					{
						current_line++;
						rate_equations.push_back(current_rate_equation);
						if (current_rate_constant.empty())
							current_rate_constant = "1.0";
						rate_constants.push_back(std::stod(current_rate_constant));
					}

					expected_char = 0;
					start_of_line = true;
					reactants = true;
					expected_text = STOICHIOMETRY_CONSTANT;
					current_rate_equation.clear();
					current_rate_constant.clear();
					continue;
				} else
				{
					start_of_line = false;
				}
			}

			

			
		
			switch(current_char) {
			case '<':
					expected_char = '-';
					break;
			case '-':
					expected_char = '>';
					break;
			case '>':
					expected_char = 0;
					expected_text = STOICHIOMETRY_CONSTANT;
					reactants = false;
					break;
			case '+':
					expected_text = STOICHIOMETRY_CONSTANT;
					break;
			case ',':
					// After this symbol, the rest of the line must be a number indicating the rate constant.
					expected_text = RATE_CONSTANT;
					break;
			default:
					// Error: Unexpected character!
					//message_debug("1unexpected char!");
					break;
			}
		}

		if(start_of_line == false)
		{
			current_line++;
			rate_equations.push_back(current_rate_equation);
		}
		
	}

	std::string defines = "#define MOLECULAR_SPECIES_COUNT " + std::to_string(species.size()) + "\n";
	
	for(unsigned int i=0; i<species.size(); i++)
	{
		defines += "#define ";
		defines += species.at(i);
		defines += " get_global_id(0) * MOLECULAR_SPECIES_COUNT + ";
		defines += std::to_string(i);
		defines += "\n";
	}
	
	std::string final_rate_equation_code;

	// Pass 3: create rate equations for each species:
	for(unsigned int a=0; a<species.size(); a++)
	{
		auto range = species_on_line.equal_range(species.at(a));
		std::string current_rate_equation = "    float rate_" + species.at(a) + " = ";
		//TODO: simplify the resulting equation by combining terms!
		for (auto i = range.first; i != range.second; ++i)
		{
			current_rate_equation += i->second.reactant ? " - " : (i == range.first) ? "" : " + ";
			current_rate_equation += std::to_string(i->second.stoichiometry * rate_constants.at(i->second.line));
			current_rate_equation += rate_equations.at(i->second.line);
		}
		final_rate_equation_code += current_rate_equation + ";\n";
		final_rate_equation_code += "    out_concentrations[";
		final_rate_equation_code += species.at(a);
		final_rate_equation_code += "] = max(in_concentrations[";
		final_rate_equation_code += species.at(a);
		final_rate_equation_code += "] + timestep*rate_";
		final_rate_equation_code += species.at(a);
		final_rate_equation_code += ", 0.0f);\n\n";
	}
	message_debug("Final rate equations:");
	message_debug(final_rate_equation_code);

	// Combine everything into a kernel code:
	std::string kernel_code = defines;
	kernel_code += "\nkernel void simulate_reactions(global float* out_concentrations, global const float* in_concentrations, uint compartment_count, float timestep)\n";
	kernel_code += "{\n";
	kernel_code += final_rate_equation_code;
	kernel_code += "}";

	message_debug(kernel_code);



	// Compile new kernel:
	sources_membrane_physics.clear();
	sources_membrane_physics.push_back({kernel_code.c_str(), kernel_code.length()});

	program_membrane_physics = cl::Program(opencl_context, sources_membrane_physics);
	cl_int result = program_membrane_physics.build({device});
	if(result != CL_SUCCESS)
	{
		message_error("Error building: " << program_membrane_physics.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << " error code" << result);
		exit(1);
	}

	kernel_membrane_physics = cl::Kernel(program_membrane_physics, "simulate_reactions");

	kernel_membrane_physics.getWorkGroupInfo(device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, &wg_size);
	
	return error::success;
}



const void intracellular_reactions::expose_lua_library(lua_State* L) const
{
	// Include the library to the lua state:
	luaL_register(L, nullptr, lua::sim_intracellular_reactions::functions);
}

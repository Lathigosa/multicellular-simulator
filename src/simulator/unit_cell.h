#ifndef SIMULATION_UNIT_CELL_H
#define SIMULATION_UNIT_CELL_H

#include "main.h"

#include "core/sim_unit_template.h"

#include <glm/glm.hpp>
#include <vector>
#include <assert.h>

#ifndef NO_UI
#include <epoxy/gl.h>
#endif // NO_UI

namespace simulation {

    /*class unit_cell
    {
    public:
        unit_cell(unsigned int amount_of_genes, unsigned int amount_of_products);
        virtual ~unit_cell();

        error run_simulation();
    protected:
    private:

        glm::vec3 position;
        queue<unsigned int> adjacent_cells;
        queue<unit_polarity_complex> polarity_complexes;

        error append_simulation_genome();

        //queue<unit_gene> genes;
        /// @todo SUBQUEUE:
        buffer<double> gene_expression;             /// The gene expression in mol/s of each gene.
        buffer<double> product_concentration;       /// The gene products in mol/L. @todo include product location, since when two genes are close together and influence each other, it is more likely that the products are nearby.

        //vec3f visual_color;                         /// "GFP"-like color for visualization of outputs.
    };

    class unit_global
    {
    public:
        unit_global(unsigned int gene_amount, unsigned int product_amount);
        ~unit_global();

        unit_cell get_cell(unsigned int index) const;
        void set_cell(unsigned int index, const unit_cell &item);

        void add_cell(const unit_cell &item);
        void insert_cell(unsigned int at, const unit_cell & item);
        void remove_cell(unsigned int index);

        unsigned int get_amount_of_cells() const
        {
            return amount_of_cells;
        }

        // === simulation ==== //
        error simulate_all();

        //void set_gene_amount(unsigned int gene_amount);
        //void set_product_amount(unsigned int product_amount);
        void add_gene();
        void add_product();

    private:
        queue<double> gene_product_interaction_constants;       /// [genes][products] (products are continuous) -  Length would be amount_of_genes * amount_of_products.

        // === "unit_cell" === //
        queue<double> gene_expression;
        queue<double> product_concentration;
        //queue<vec3f> visual_color;
        queue<double> position_x;
        queue<double> position_y;
        queue<double> position_z;
        // =================== //

        // A few variables keeping track of the amount of items in each array:
        unsigned int amount_of_cells;
        unsigned int amount_of_genes;
        unsigned int amount_of_products;
    };*/

	/// Class in charge of simulating each cell:
    class cell_global : public sim_unit_template
	{
		public:
			SIMULATOR_DEFINITION("sim_cell", {})

			cell_global(cl::Platform & platform_cl, cl::Device & device_cl, cl::Context & context, cl::CommandQueue & command_queue);
			virtual ~cell_global();

			error initialize_buffers() override;
			#ifndef NO_UI
			error initialize_buffers(std::vector<GLuint> from_gl_buffer) override;
			#endif

			error organize_unit() override;		//
			std::vector<event_info> simulate_unit(float step_size) override;
			cl::Buffer get_buffer(unsigned int index, bool double_buffer) override
			{
				switch(index) {
					case 0: return new_cell_indices;
					case 1: return new_cell_group_size;
					case 2: return empty_cells;
					case 3: return copied_cells;
					case 4: return deleted_cell_indices;
					case 5: return deleted_cell_group_size;
				}

				message_error("ERROR: requesting buffer that does not exist (unit_cell)!");
				assert(false);
			}
			unsigned int get_buffer_index(const std::string name) const override
			{
				if (name == "new_cell_indices")			return 0;
				if (name == "new_cell_group_size")		return 1;
				if (name == "empty_cells")				return 2;
				if (name == "copied_cells")				return 3;
				if (name == "deleted_cell_indices")		return 4;
				if (name == "deleted_cell_group_size")	return 5;

				// Crash when the name string does not match anything:
				message_error("ERROR: name does not match any internal values!");
				assert(false);
			}

			error signal_cell_reindex(const std::string sim_unit_name, 
		                              			cl::Buffer empty_cells,
		                              			cl::Buffer copied_cells,
		                              			unsigned int copied_count) override;

			error signal_particle_remove(const std::string sim_unit_name,
		                                 		cl::Buffer empty_cells,
		                                 		unsigned int empty_count) override;

			const void expose_lua_library(lua_State* L) const override;

			// Custom functions:
			error add_particles(unsigned int count);
		protected:
		private:
			cl::Program::Sources sources_membrane_physics;
			cl::Program program_membrane_physics;
			cl::Kernel kernel_random_deletion;
			cl::Program::Sources sources_concatenate;
			cl::Program program_concatenate;
			cl::Kernel kernel_concatenate;

			cl::Kernel kernel_delete_sort;

			cl::Buffer deleted_cells;
			cl::Buffer deleted_cells_sorted;
			
			// Double buffered:
			bool buffer_index = false;
			
			cl::Buffer new_cell_indices;		// One list per workgroup keeping track of which indices must be copied.
			cl::Buffer new_cell_group_size;		// The size of each list in "new_cell_indices".

			cl::Buffer deleted_cell_indices;		// One list per workgroup keeping track of which indices must be copied.
			cl::Buffer deleted_cell_group_size;		// The size of each list in "new_cell_indices".

			cl::Buffer empty_cells;				// Concatenated list of empty cell indices.
			cl::Buffer copied_cells;			// Concatenated list of cell indices to be copied.
			unsigned int copied_count = 0;		// Size of data inside "copied_cells".

			unsigned int cell_count = 0;
			unsigned int workgroup_size = 256;
			unsigned int maximal_cell_count = 32*32*8;	// TODO: change value

			// Test:
			int countdown = 64*4;

	};
}

#endif // SIMULATION_UNIT_CELL_H

#ifndef UNIT_GENOME_H
#define UNIT_GENOME_H

#include "utilities/queue.h"
#include "utilities/buffer.h"

namespace simulation {

    struct gene
    {
        const char* name;       /// @todo Turn it into std::string or alternative!


    };

    class definition_genome
    {
    public:
        //buffer<buffer<double>> product_to_gene_feedback;    /// The amount in which the product influences the gene, a positive value is an activator, a negative value is a repressor. Format: array[gene][product].
        /// @todo Change buffer<buffer<double>> to buffer_2d<double>!

        queue<gene> genes;
        queue<double> product_to_gene_feedback;

        void add_gene();
        void add_product();
        void add_product_to_gene_feedback();
    };

    class unit_genome
    {
    public:
        unit_genome();
        virtual ~unit_genome();

        buffer<double> expression;               /// The gene expression in mol/s of each gene.
        buffer<double> product_concentration;    /// The gene products in mol/L. @todo include product location, since when two genes are close together and influence each other, it is more likely that the products are nearby.
    };

}

#endif // UNIT_GENOME_H

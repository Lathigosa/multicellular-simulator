#ifndef SIMULATION_UNIT_CELL_H
#define SIMULATION_UNIT_CELL_H

#include "utilities/queue.h"
#include "utilities/math.h"

using namespace math;

class simulation_unit_cell
{
    public:
        simulation_unit_cell();
        virtual ~simulation_unit_cell();
    protected:
    private:

        vec3f position;
        queue<unsigned int> adjacent_cells;


        //location
        //indices to adjacent cells
        //contains polarity complex protein array
        //
};

#endif // SIMULATION_UNIT_CELL_H

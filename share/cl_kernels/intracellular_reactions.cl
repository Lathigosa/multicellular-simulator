//
#define MOLECULAR_SPECIES_COUNT 8

#define A get_global_id(0) * MOLECULAR_SPECIES_COUNT + 0
#define B get_global_id(0) * MOLECULAR_SPECIES_COUNT + 1
#define C get_global_id(0) * MOLECULAR_SPECIES_COUNT + 4
#define D get_global_id(0) * MOLECULAR_SPECIES_COUNT + 5

kernel void simulate_reactions(global float* out_concentrations, global const float* in_concentrations, uint compartment_count, float timestep)
{
	timestep = timestep * 0.001;
	float rate_1 =  - 2.0f*in_concentrations[A] - 2.5f*in_concentrations[A]*in_concentrations[B] + 3.0;
	out_concentrations[A] = max(in_concentrations[A] + timestep*rate_1, -0.000000000000f);
	
	float rate_2 = 2.0f*in_concentrations[A]-2.5f*in_concentrations[A]*in_concentrations[B];
	out_concentrations[B] = max(in_concentrations[B] + timestep*rate_2, -0.000000000000f);
	
	float rate_3 = 2.5f*in_concentrations[A]*in_concentrations[B] - 3.0f*in_concentrations[C];
	out_concentrations[C] = max(in_concentrations[C] + timestep*rate_3, -0.000000000000f);
	
	float rate_4 = 2.5f*in_concentrations[A]*in_concentrations[B] - 4.0f*in_concentrations[D];
	out_concentrations[D] = max(in_concentrations[D] + timestep*rate_4, -0.000000000000f);
	
	//out_concentrations[C] = timestep*rate_2*1000000.0f;
	
	//if(rate_2 == 0.0f)
	//	out_concentrations[3] = 20.0f;
}

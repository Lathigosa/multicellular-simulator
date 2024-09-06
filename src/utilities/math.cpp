#include <math.h>
//#include "../utilities/math.h"

void rotate2DVector(float * outputVector2, float * inputVector2, float angle) {
	float cs = cos(angle);
	float sn = sin(angle);

	float temp_vector[2];

	temp_vector[0] = inputVector2[0] * cs - inputVector2[1] * sn;
	temp_vector[1] = inputVector2[0] * sn + inputVector2[1] * cs;

	outputVector2[0] = temp_vector[0];
	outputVector2[1] = temp_vector[1];
}

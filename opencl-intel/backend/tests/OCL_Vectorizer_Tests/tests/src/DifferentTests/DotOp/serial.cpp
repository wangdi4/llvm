// serial.cpp
// Implementation page of the serial Header File functions

#include "serial.h"
#include <math.h>

void runSerail_H(float* A, float* B, float* C, unsigned int count)
{
	for( int i =0; i< count; i++) {
		
		int wait = 50 * (i % 2);
		
		int d =0;
		for(int l = 0; l<wait; l++) d++;
		
		float tmp1 = A[i];
		float tmp2 = B[i];
		
		tmp1 = tmp1 * tmp1;
		tmp2 = tmp2 / tmp2;
		tmp1 = tmp1 * tmp2;
		tmp1 = tmp1 - tmp2;
		tmp2 = tmp1 * tmp2;
		tmp1 = tmp2 / tmp1;
		tmp2 = tmp1 * tmp2;
		tmp1 = tmp2 - tmp1;
		tmp1 = tmp2 * tmp1;
		tmp1 = tmp1 / tmp2;
		
		C[i] = (tmp1 + d - d);
		}
}
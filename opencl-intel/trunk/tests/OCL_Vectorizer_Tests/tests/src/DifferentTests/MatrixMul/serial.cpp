// serial.cpp
// Implementation page of the serial Header File functions

#include "serial.h"

void
runSerail_MUL(float* C, const float* A, const float* B, unsigned int hA, 
			  unsigned int wA, unsigned int wB)
{
	//double a;
	//double b;
	double sum;
	
    for (unsigned int i = 0; i < hA; ++i)
        for (unsigned int j = 0; j < wB; ++j) {
            sum = 0;
            for (unsigned int k = 0; k < wA; ++k) {
                //a = A[i * wA + k];
                //b = B[k * wB + j];
                sum += A[i * wA + k] * B[k * wB + j];
            }
            C[i * wB + j] = (float)sum;
        }
}
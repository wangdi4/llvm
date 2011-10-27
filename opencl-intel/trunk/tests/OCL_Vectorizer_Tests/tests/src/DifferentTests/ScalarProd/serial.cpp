// *******************************************************************************
// serial.cpp
// Implementation page of the serial Header File functions
// *******************************************************************************

// *******************************************************************************
// Include Files
// *******************************************************************************

#include "serial.h"

// *******************************************************************************
// Functions Implementaion
// *******************************************************************************

// *******************************************************************************
// scalarProdSerial -
// Calculate scalar products of VectorN vectors of ElementN elements.
// *******************************************************************************

extern "C"
void scalarProdSerial(
				   float *h_C,
				   float *h_A,
				   float *h_B,
				   int vectorN,
				   int elementN
				   )
{
    for(int vec = 0; vec < vectorN; vec++)
	{
        int vectorBase = elementN * vec;
        int vectorEnd  = vectorBase + elementN;
		
        double sum = 0;
        for(int pos = vectorBase; pos < vectorEnd; pos++)
            sum += h_A[pos] * h_B[pos];
		
        h_C[vec] = (float)sum;
    }
}
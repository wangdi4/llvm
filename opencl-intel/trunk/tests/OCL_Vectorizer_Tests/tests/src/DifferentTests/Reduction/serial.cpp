// serial.cpp
// Implementation page of the serial Header File functions

#include "serial.h"

float runSerial_Reduce(float *data, int size)
{
    float sum = 0;
    for(int i=0; i<size; i++) {
		sum += data[i];
    }
    return sum;
}

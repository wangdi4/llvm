/*
 * Part of the tests that check different LLVM instructions on different argument
 * types and different argument sources.
 *
 * Tests the OpenCL rotate function when it recieves uchar constants.
 * This function is vectorizable.
 *
 * @param in
 * 		The input array.
 * @param out
 * 		The output array.
 *
 */
#include "def.h"
__kernel void
func_rotate ( __global const float *in,
		__global float *out)
{
	int gid = get_global_id(0);
	uchar  rez = rotate(UCHAR1, UCHAR2);
	
	uchar4 vec = (UCHAR1,UCHAR1+UCHAR2,UCHAR2,UCHAR1*UCHAR2);
	vec = rotate(vec,vec);
	
	out[gid] =vec.y+rez;
}


/*
 * Part of the tests that check different LLVM instructions on different argument
 * types and different argument sources.
 *
 * Tests the OpenCL dot_product
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
func_abs_diff ( __global const double *in,
		__global float *out)
{
	int gid = get_global_id(0);
	double4 vec4 = (in[0],in[1],in[2],in[3]);
	float v = dot( vec4,(double4) (in[0]*in[1],in[3],in[2],12));	
	float leng = length(DOUBLE2_VEC1 ) * gid;
	out[gid] = v + (float) gid + leng;
}


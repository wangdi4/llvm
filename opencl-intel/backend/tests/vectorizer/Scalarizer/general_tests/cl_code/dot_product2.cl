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
func_abs_diff ( __global const float *in,
		__global float *out)
{
	int gid = get_global_id(0);
	float v = dot(FLOAT4_VEC1,FLOAT4_VEC1);	
	float leng = length(DOUBLE2_VEC1 ) * gid;
	out[gid] = v + (float) gid + leng;
}


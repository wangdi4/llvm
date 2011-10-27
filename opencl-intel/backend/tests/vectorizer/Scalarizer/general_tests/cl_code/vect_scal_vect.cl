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
		__global float4 *out)
{
	int gid = get_global_id(0);
	float4 v2_4=(float4) (in[1],in[2],in[3],in[4]);
	out[gid] =v2_4;
}


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
func_abs_diff ( __global double *in,
		__global double4 *out)
{
	int gid = get_global_id(0);
	double4 v2_4=(double4) (in[1],in[2],in[3],in[4]) + (double4) (in[gid],in[gid+2],in[gid]+3,112);
	out[gid] =v2_4;
}

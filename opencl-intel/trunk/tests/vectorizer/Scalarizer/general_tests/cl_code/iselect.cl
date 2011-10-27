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
		__global long4 *out)
{
	int gid = get_global_id(0);
	out[gid] = select(LONG4_VEC1,LONG4_VEC1,LONG4_VEC1);
}


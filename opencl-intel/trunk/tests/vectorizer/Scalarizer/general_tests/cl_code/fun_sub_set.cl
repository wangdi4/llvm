/*
 * Part of the tests that check different LLVM instructions on different argument
 * types and different argument sources.
 *
 * Tests the OpenCL sub_sat function when it recieves long constants.
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
func_sub_sat ( __global const float *in,
		__global float *out)
{
	int gid = get_global_id(0);
	out[gid] = (sub_sat(ULONG2_VEC1, ULONG2_VEC1)).x;
}


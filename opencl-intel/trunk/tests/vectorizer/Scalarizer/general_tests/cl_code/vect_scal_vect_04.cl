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
func_vect_scal ( __global float *in,
		__global float8 *out)
{
	int gid = get_global_id(0);
	float8 v1=FLOAT8_VEC1;
	float8 v2=v1 + (float8) (gid, in[0], -10.39f, 8.85f, 9.3523f, 2323.0f, 4317.433f, 0.00002f);
	float8 v3=v1*v2;
	out[gid] = v3;
}

/*
 * Part of the tests that check different LLVM instructions on different argument
 * types and different argument sources.
 *
 * Tests the LLVM mul instruction when it recieves char function arguments.
 * This instruction is vectorizable.
 *
 * @param in
 * 		The input array.
 * @param out
 * 		The output array.
 *
 */
 
 #include "def.h"
__kernel void
mul ( __global const float2 *in,
		__global float *out, __global const char8 charArg1, __global const char8 charArg2)
{
	int gid = get_global_id(0);
	// these assignment to tmp and add are done to ensure that mul on chars is used
	char8 tmp = charArg1 * charArg2;
	out[gid] = tmp.s0 + 1;
}


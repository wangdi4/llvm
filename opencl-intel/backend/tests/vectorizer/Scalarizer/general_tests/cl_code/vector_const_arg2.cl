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
		__global float *out, __global const char8 charArg1, __global const char8 charArg2,__global const int4 intArg1,__global const int4 intArg2)
{
	int gid = get_global_id(0);
	char8 tmp1 = charArg1 / charArg2;
	
	int4 tmp3 = intArg2 * intArg2;
	out[gid] = (float) tmp1.s0 + 1.+ (float) tmp3.s1 ;//+ (float) tmp.s2;
}
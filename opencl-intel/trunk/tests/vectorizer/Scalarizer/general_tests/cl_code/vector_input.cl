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
	float4 v1_4= FLOAT4_VEC1;
	float4 v2_4=(float4) (in[1],in[2],in[3],in[4]);
	float4 v3_4=v1_4+v2_4;
	float4 v4_4=v1_4+v2_4*v3_4;
//	float v = dot(v4,v4+ (float4) gid);	
//	float4 rez4=v4*v4+ (float4) (4);
	out[gid] =v4_4+v1_4;
}


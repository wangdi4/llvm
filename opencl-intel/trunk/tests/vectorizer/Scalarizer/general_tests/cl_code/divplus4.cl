
#include "def.h"
__kernel void
mul_vector (	__global const float4 *in,
		__global float4 *out)
{
	int gid = get_global_id(0);
	// using local vectorized variable (temp)
	float4 temp = in[gid] / FLOAT4_VEC2;
	out[gid] = temp + FLOAT4_VEC3;
}



 #include "def.h"
__kernel void
dist ( __global const float2 *in,__global float *out)
{
	int gid = get_global_id(0);
	int4 tmp = INT4_VEC1 + (int4) (gid, gid,gid,gid);
	int4  tmp2 = tmp*tmp;
	float dist= distance((float4) tmp,(float4) tmp2);

	out[gid] = dist;


}
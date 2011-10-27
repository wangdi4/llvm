

#include "def.h"
__kernel void
func_vect_distance ( __global const long *in,
		__global float *out)
{
	int gid = get_global_id(0);
	double4 vec4 = (double) (in[0],in[1],in[2],in[3]);
	float v = distance( vec4,(double4) (in[0]*gid,in[3]+gid,in[2],12));	
	out[gid] =(float) v + (float) gid ;
}


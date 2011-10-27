#include "def.h"
__kernel void
func_sub_sat ( __global const float *in,
		__global float *out)
{
	int gid = get_global_id(0);
	out[gid] = (max(ULONG2_VEC1, ULONG2_VEC1)).x;
}

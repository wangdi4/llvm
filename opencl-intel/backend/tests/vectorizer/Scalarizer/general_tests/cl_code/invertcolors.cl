#include "def.h"
__kernel void
and_short2 ( __global const float *in,
                                __global float *out)
{
                int gid = get_global_id(0);
                short2 vec = (ARG0, ARG1);
                short2 res = (short2) (vec & SHORT2_VEC1);
                out[gid] = res.x + res.y;
}

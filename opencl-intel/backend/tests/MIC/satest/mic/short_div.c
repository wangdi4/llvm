
__kernel void test(__global short *srcA, __global short *srcB, __global short *dst)
{
    int  tid = get_global_id(0);

    dst[tid] = srcA[tid] / srcB[tid];
}

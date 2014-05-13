extern void undefined_func(void);
__kernel void QueueKernel( const __global float *input, __global float *output)
{

    undefined_func();

    return;
}
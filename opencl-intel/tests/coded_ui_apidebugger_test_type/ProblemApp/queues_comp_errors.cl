__kernel void QueueKernel( const __global float *input, __global float *output)
{
    size_t index = get_global_id(0);
    const t=3;
    t=4;
    
    for(int i = 0; i < 10; i++)
    {
        output[index] = sin(fabs(input[index]));
    }
}

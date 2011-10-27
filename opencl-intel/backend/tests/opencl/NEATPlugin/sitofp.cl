
__kernel
void SIToFPTest(__global int * input,
                __global float4 * output,
                const uint buffer_size)
{
	uint tid = get_global_id(0);
	output[tid] = (float4)(10.f, 11.f, 11.f, 12.f) + (float4)input[tid];
	int z = -4;
	output[tid].w = (float)z;
}

__kernel
void ShuffleTest(__global float4 * input,
								__global float4 * output,
								const    uint  buffer_size)
{
	uint tid = get_global_id(0);
	output[tid] = input[tid].xxxx;
}
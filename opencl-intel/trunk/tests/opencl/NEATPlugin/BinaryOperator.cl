
__kernel
void BinaryKernel(__global float * input,
								__global float * output,
								const    uint  buffer_size)
{
	uint tid = get_global_id(0);
	output[tid] = input[tid] + 1.0f;
	output[tid] = output[tid] / 5.0f;
	output[tid] = 2.0f - output[tid];
	output[tid] = output[tid] * input[tid];
}

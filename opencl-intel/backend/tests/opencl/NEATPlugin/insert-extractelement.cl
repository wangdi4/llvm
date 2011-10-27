
__kernel
void InsertExtractElement(__global float4 * input, 
                          __global float4 * output, 
                          const    uint  buffer_size)
{
	uint tid = get_global_id(0);
	output[tid].x = input[tid].x+5.f;
	output[tid].y = input[tid].x+5.f;
	output[tid].z = input[tid].x+5.f;
	output[tid].w = input[tid].x+5.f;
}
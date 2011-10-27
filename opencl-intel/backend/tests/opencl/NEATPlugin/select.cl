// TODO: uncomment vector select!

__kernel
void SelectTest(__global float4 * input,
                __global float4 * output,
                const    uint  buffer_size)
{
	uint tid = get_global_id(0);
	output[tid].xy = (input[tid].x < 15.f) ? input[tid].xx : input[tid].yy;
//	output[tid].zw = (input[tid].zw >= (41.f, 41.f)) ? input[tid].xy : input[tid].zw;
}
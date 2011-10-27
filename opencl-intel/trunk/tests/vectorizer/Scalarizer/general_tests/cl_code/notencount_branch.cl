
__kernel void
if_goto (	__global const float *in, __global const float4 *in2)
		__global float *out)
{
	int gid = get_global_id(0);

	ulong tmp1 = in[1];
	float4 tmp2 = (in[1], in[2], in[3], 4);
	out[gid] = 0;

	if(tmp1 > ULONG2) {
		out[gid] = ULONG2;
		goto label1;
	}

	out[gid] = ULONG1;

	label1:
	out[gid] += ULONG2;

}


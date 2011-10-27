typedef struct _str1 {
  float4 f4;
  int i;
} str1;

__kernel
void InsertExtractValue(__global float4 * input, 
                          __global float4 * output, 
                          const    uint  buffer_size)
{
	uint tid = get_global_id(0);
	str1 s = {(float4)(1.f, 2.f, 3.f, 4.f), 0};
	output[tid] = s.f4;
}
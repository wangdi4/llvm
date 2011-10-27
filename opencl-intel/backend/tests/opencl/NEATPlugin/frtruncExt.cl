
__kernel
void ftruncExt(__global float4 * input, 
          __global float4 * output, 
          const    uint  buffer_size)
{
  uint tid = get_global_id(0);
  double ext = input[tid].y;
  output[tid].w = ext + 4.f;
}

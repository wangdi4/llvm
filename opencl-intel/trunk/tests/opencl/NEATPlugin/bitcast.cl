__kernel
void BitcastTest(__global float4 * input,
                 __global double2 * output,
                 const    uint  buffer_size)
{
  uint tid = get_global_id(0);
  double2 tmp = as_double2(input[tid]) * (double2)(0.0, 0.0) + (double2)(0.1, 0.2);
  input[tid] = as_float4(tmp);
  output[tid] = tmp;
}
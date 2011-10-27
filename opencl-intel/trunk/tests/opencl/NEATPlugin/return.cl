float retFunc(float a, float b)
{
  return a+b;
}

__kernel
void returnTest(__global float * input,
            __global float * output,
             const uint buffer_size)
{
  output[get_global_id(0)] = retFunc(input[get_global_id(0)], input[get_global_id(0)]);
}

#pragma OPENCL EXTENSION cl_khr_fp16 : enable

/**
 * cpu_error_kernel - kernels intended to fail upon build on CPU and succeed for
 * GPU
 */

__kernel void cpu_error_kernel(__global int *input, double input_size) {
  short4 x = 0;
  half4 y = as_half4(x);
  input[0] = (int)y.x;
}

#pragma OPENCL EXTENSION cl_khr_fp16 : enable

__kernel void fp16(__global half *input, __global half *output) {
  half half_value = *input;
  // Let the debugger set the output variable.
}

#pragma OPENCL EXTENSION cl_khr_depth_images : enable

__kernel void sample_kernel_2D_u16(__read_only image2d_depth_t input,
                                   __global float *output) {
  int tidX = get_global_id(0), tidY = get_global_id(1);
  float pixel = read_imagef(input, (int2)(tidX, tidY));
  output[tidX + tidY * get_global_size(0)] = pixel;
}

__kernel void sample_kernel_2D_f32(__read_only image2d_depth_t input,
                                   __global float *output) {
  int tidX = get_global_id(0), tidY = get_global_id(1);
  float pixel = read_imagef(input, (int2)(tidX, tidY));
  output[tidX + tidY * get_global_size(0)] = pixel;
}
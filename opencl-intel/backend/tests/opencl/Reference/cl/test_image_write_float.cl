__kernel void sample_kernel_1D(__global float4 *input,
                               write_only image1d_t output) {
  int tidX = get_global_id(0);
  int offset = tidX;
  write_imagef(output, tidX, input[offset]);
}

__kernel void sample_kernel_1Darr(__global float4 *input,
                                  write_only image1d_array_t output) {
  int tidX = get_global_id(0), tidY = get_global_id(1);
  int offset = tidY * get_image_width(output) + tidX;
  write_imagef(output, (int2)(tidX, tidY), input[offset]);
}

__kernel void sample_kernel_2D(__global float4 *input,
                               write_only image2d_t output) {
  int tidX = get_global_id(0), tidY = get_global_id(1);
  int offset = tidY * get_image_width(output) + tidX;
  write_imagef(output, (int2)(tidX, tidY), input[offset]);
}

__kernel void sample_kernel_2Darr(__global float4 *input,
                                  write_only image2d_array_t output) {
  int tidX = get_global_id(0), tidY = get_global_id(1), tidZ = get_global_id(2);
  int offset = tidZ * get_image_width(output) * get_image_height(output) +
               tidY * get_image_width(output) + tidX;
  write_imagef(output, (int4)(tidX, tidY, tidZ, 0), input[offset]);
}
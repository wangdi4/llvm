__kernel void sample_kernel_2D(__read_only image2d_t input,
                               __global float4 *output) {
  int tidX = get_global_id(0), tidY = get_global_id(1);
  float4 pixel = read_imagef(input, (int2)(tidX, tidY));
  output[tidX + tidY * get_global_size(0)] = pixel;
}
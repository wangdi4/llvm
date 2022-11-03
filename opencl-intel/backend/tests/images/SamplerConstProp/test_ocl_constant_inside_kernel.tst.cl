/*
 Test description:
 in this test we check that CLANG performs the proper constant propogation to
 the sampler in the scenario where a constant sampler is defined as a __constant
 inside the kernel
*/

__kernel void test_ocl_constant_inside_kernel(__global float4 *out,
                                              __read_only image2d_t image,
                                              int2 coord) {
  __constant sampler_t sampler =
      CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
  out[0] = read_imagef(image, sampler, coord);
}

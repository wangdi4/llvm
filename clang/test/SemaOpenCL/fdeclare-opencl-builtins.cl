// RUN: %clang_cc1 %s -triple spir -verify -pedantic -fsyntax-only -cl-std=CL2.0 -fdeclare-opencl-builtins
// expected-no-diagnostics

// Test the -fdeclare-opencl-builtins option.

<<<<<<< HEAD
typedef float float4 __attribute__((ext_vector_type(4)));
=======
#pragma OPENCL EXTENSION cl_khr_fp16 : enable

// Provide typedefs when invoking clang without -finclude-default-header.
#ifdef NO_HEADER
typedef char char2 __attribute__((ext_vector_type(2)));
typedef char char4 __attribute__((ext_vector_type(4)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef half half4 __attribute__((ext_vector_type(4)));
typedef int int2 __attribute__((ext_vector_type(2)));
>>>>>>> 988f1e3e32a95df46da4f98b5652b0dc8b444c7f
typedef int int4 __attribute__((ext_vector_type(4)));
typedef int int2 __attribute__((ext_vector_type(2)));
typedef unsigned int uint;
typedef __SIZE_TYPE__ size_t;

kernel void test_pointers(volatile global void *global_p, global const int4 *a) {
  int i;
  unsigned int ui;

  prefetch(a, 2);

  atom_add((volatile __global int *)global_p, i);
  atom_cmpxchg((volatile __global unsigned int *)global_p, ui, ui);
}

kernel void basic_conversion(global float4 *buf, global int4 *res) {
  res[0] = convert_int4(buf[0]);
}

kernel void basic_readonly_image_type(__read_only image2d_t img, int2 coord, global float4 *out) {
  out[0] = read_imagef(img, coord);
}

kernel void basic_image_readonly(read_only image2d_t image_read_only_image2d) {
  int2 i2;
  sampler_t sampler;
  half4 res;
  float4 resf;

  resf = read_imagef(image_read_only_image2d, i2);
  res = read_imageh(image_read_only_image2d, i2);
  res = read_imageh(image_read_only_image2d, sampler, i2);
}

kernel void basic_image_readwrite(read_write image3d_t image_read_write_image3d) {
  half4 h4;
  int4 i4;

  write_imageh(image_read_write_image3d, i4, h4);
}

kernel void basic_image_writeonly(write_only image1d_buffer_t image_write_only_image1d_buffer) {
  half4 h4;
  float4 f4;
  int i;

  write_imagef(image_write_only_image1d_buffer, i, f4);
  write_imageh(image_write_only_image1d_buffer, i, h4);
}

kernel void basic_subgroup(global uint *out) {
  out[0] = get_sub_group_size();
}

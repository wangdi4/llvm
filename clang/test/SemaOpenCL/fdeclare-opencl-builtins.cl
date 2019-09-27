// INTEL_CUSTOMIZATION
// This test is significantly modified, because
// -finclude-default-header and -fdeclare-opencl-builtins
// don't cowork in xmain due to
// https://git-amr-2.devtools.intel.com/gerrit/194550
// end INTEL_CUSTOMIZATION

// RUN: %clang_cc1 %s -triple spir -verify -pedantic -Wconversion -Werror -fsyntax-only -cl-std=CL -fdeclare-opencl-builtins
// RUN: %clang_cc1 %s -triple spir -verify -pedantic -Wconversion -Werror -fsyntax-only -cl-std=CL1.2 -fdeclare-opencl-builtins
// RUN: %clang_cc1 %s -triple spir -verify -pedantic -Wconversion -Werror -fsyntax-only -cl-std=CL2.0 -fdeclare-opencl-builtins

#if __OPENCL_C_VERSION__ >= CL_VERSION_2_0
// expected-no-diagnostics
#endif

// Test the -fdeclare-opencl-builtins option.

#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#if __OPENCL_C_VERSION__ < CL_VERSION_1_2
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

typedef float float4 __attribute__((ext_vector_type(4)));
typedef half half4 __attribute__((ext_vector_type(4)));
typedef int int4 __attribute__((ext_vector_type(4)));
typedef int int2 __attribute__((ext_vector_type(2)));
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
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

  int imgWidth = get_image_width(image_read_only_image2d);
}

#if __OPENCL_C_VERSION__ >= CL_VERSION_2_0
kernel void basic_image_readwrite(read_write image3d_t image_read_write_image3d) {
  half4 h4;
  int4 i4;

  write_imageh(image_read_write_image3d, i4, h4);

  int imgDepth = get_image_depth(image_read_write_image3d);
}
#endif // __OPENCL_C_VERSION__ >= CL_VERSION_2_0

kernel void basic_image_writeonly(write_only image1d_buffer_t image_write_only_image1d_buffer) {
  half4 h4;
  float4 f4;
  int i;

  write_imagef(image_write_only_image1d_buffer, i, f4);
  write_imageh(image_write_only_image1d_buffer, i, h4);
}

kernel void basic_subgroup(global uint *out) {
  out[0] = get_sub_group_size();
#if __OPENCL_C_VERSION__ < CL_VERSION_2_0
// expected-error@-2{{implicit declaration of function 'get_sub_group_size' is invalid in OpenCL}}
// expected-error@-3{{implicit conversion changes signedness: 'int' to 'uint' (aka 'unsigned int')}}
#endif
}

kernel void basic_vector_data() {
#if __OPENCL_C_VERSION__ >= CL_VERSION_2_0
  generic void *generic_p;
#endif
  constant void *constant_p;
  local void *local_p;
  global void *global_p;
  private void *private_p;
  size_t s;

  vload4(s, (const __constant ulong *) constant_p);
  vload16(s, (const __constant short *) constant_p);

#if __OPENCL_C_VERSION__ >= CL_VERSION_2_0
  vload3(s, (const __generic ushort *) generic_p);
  vload16(s, (const __generic uchar *) generic_p);
#endif

  vload8(s, (const __global long *) global_p);
  vload2(s, (const __local uint *) local_p);
  vload16(s, (const __private float *) private_p);
}

kernel void basic_work_item() {
  uint ui;

  get_enqueued_local_size(ui);
#if __OPENCL_C_VERSION__ < CL_VERSION_2_0
// expected-error@-2{{implicit declaration of function 'get_enqueued_local_size' is invalid in OpenCL}}
#endif
}

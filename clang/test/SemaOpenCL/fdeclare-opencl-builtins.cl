// RUN: %clang_cc1 %s -triple spir -verify -pedantic -fsyntax-only -cl-std=CL2.0 -fdeclare-opencl-builtins
// expected-no-diagnostics

// Test the -fdeclare-opencl-builtins option.

typedef float float4 __attribute__((ext_vector_type(4)));
typedef int int4 __attribute__((ext_vector_type(4)));
typedef int int2 __attribute__((ext_vector_type(2)));
typedef unsigned int uint;
typedef __SIZE_TYPE__ size_t;
<<<<<<< HEAD
=======
#endif

kernel void test_pointers(volatile global void *global_p, global const int4 *a) {
  int i;
  unsigned int ui;

  prefetch(a, 2);

  atom_add((volatile __global int *)global_p, i);
  atom_cmpxchg((volatile __global unsigned int *)global_p, ui, ui);
}

kernel void basic_conversion() {
  double d;
  float f;
  char2 c2;
  long2 l2;
  float4 f4;
  int4 i4;
>>>>>>> cc0ba28cf07fd696148d70eb454fbaeb9c0b30c2

kernel void basic_conversion(global float4 *buf, global int4 *res) {
  res[0] = convert_int4(buf[0]);
}

kernel void basic_readonly_image_type(__read_only image2d_t img, int2 coord, global float4 *out) {
  out[0] = read_imagef(img, coord);
}

kernel void basic_subgroup(global uint *out) {
  out[0] = get_sub_group_size();
}

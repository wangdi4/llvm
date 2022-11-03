// Test for ctz in SATest OpenCL Reference
__kernel void test_ctz(__global int *a, __global long *b) {
  a[get_global_id(0)] = ctz(a[get_global_id(0)]);
  b[get_global_id(0)] = ctz(b[get_global_id(0)]);
}

__kernel void test_ctz2(__global int *a, __global long *b) {
  size_t offset = get_global_id(0);
  int2 a2 = (int2)(a[2 * offset], a[2 * offset + 1]);
  long2 b2 = (long2)(b[2 * offset], b[2 * offset + 1]);
  int2 ctz2int = ctz(a2);
  long2 ctz2long = ctz(b2);
  a[2 * offset] = ctz2int.x;
  a[2 * offset + 1] = ctz2int.y;
  b[2 * offset] = ctz2long.x;
  b[2 * offset + 1] = ctz2long.y;
}

__kernel void test_ctz4(__global int *a, __global long *b) {
  size_t offset = get_global_id(0);
  int4 a4 = (int4)(a[4 * offset], a[4 * offset + 1], a[4 * offset + 2],
                   a[4 * offset + 3]);
  long4 b4 = (long4)(b[4 * offset], b[4 * offset + 1], b[4 * offset + 2],
                     b[4 * offset + 3]);
  int4 ctz4int = ctz(a4);
  long4 ctz4long = ctz(b4);

  a[4 * offset] = ctz4int.x;
  a[4 * offset + 1] = ctz4int.y;
  a[4 * offset + 2] = ctz4int.z;
  a[4 * offset + 3] = ctz4int.w;

  b[4 * offset] = ctz4long.x;
  b[4 * offset + 1] = ctz4long.y;
  b[4 * offset + 2] = ctz4long.z;
  b[4 * offset + 3] = ctz4long.w;
}

__kernel void test_ctz8(__global int *a, __global long *b) {
  size_t offset = get_global_id(0);
  int8 a8 = (int8)(a[8 * offset], a[8 * offset + 1], a[8 * offset + 2],
                   a[8 * offset + 3], a[8 * offset + 4], a[8 * offset + 5],
                   a[8 * offset + 6], a[8 * offset + 7]);
  long8 b8 = (long8)(b[8 * offset], b[8 * offset + 1], b[8 * offset + 2],
                     b[8 * offset + 3], b[8 * offset + 4], b[8 * offset + 5],
                     b[8 * offset + 6], b[8 * offset + 7]);
  int8 ctz8int = ctz(a8);
  long8 ctz8long = ctz(b8);

  a[8 * offset] = ctz8int.s0;
  a[8 * offset + 1] = ctz8int.s1;
  a[8 * offset + 2] = ctz8int.s2;
  a[8 * offset + 3] = ctz8int.s3;
  a[8 * offset + 4] = ctz8int.s4;
  a[8 * offset + 5] = ctz8int.s5;
  a[8 * offset + 6] = ctz8int.s6;
  a[8 * offset + 7] = ctz8int.s7;

  b[8 * offset] = ctz8long.s0;
  b[8 * offset + 1] = ctz8long.s1;
  b[8 * offset + 2] = ctz8long.s2;
  b[8 * offset + 3] = ctz8long.s3;
  b[8 * offset + 4] = ctz8long.s4;
  b[8 * offset + 5] = ctz8long.s5;
  b[8 * offset + 6] = ctz8long.s6;
  b[8 * offset + 7] = ctz8long.s7;
}
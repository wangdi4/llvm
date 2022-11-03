// RUN: SATest -VAL -config=%s.16i8.cfg | FileCheck %s
// RUN: SATest -VAL -config=%s.8i16.cfg | FileCheck %s
// RUN: SATest -VAL -config=%s.4i32.cfg | FileCheck %s
// RUN: SATest -VAL -config=%s.2i64.cfg | FileCheck %s

// CHECK: Test Passed.
__kernel void test2i64(__global long2 *p, __global long2 *dst) {
  *dst = popcount(*p);
}
__kernel void test4i32(__global int4 *p, __global int4 *dst) {
  *dst = popcount(*p);
}
__kernel void test8i16(__global short8 *p, __global short8 *dst) {
  *dst = popcount(*p);
}
__kernel void test16i8(__global char16 *p, __global char16 *dst) {
  *dst = popcount(*p);
}
__kernel void test2i8(__global char2 *src, __global char2 *dst) {
  int tid = get_global_id(0);
  char2 sA;
  sA = src[tid];
  char2 dstVal = popcount(sA);
  dst[tid] = dstVal;
}

// RUN: SATest -VAL -config=%s.cfg | FileCheck %s

// CHECK: Test Passed.
kernel void test(global int4 *a4, global int4 *b4, global int4 *c4,
                 global int4 *d4, global int8 *a8, global int8 *b8,
                 global int8 *c8, global int8 *d8, global int16 *a16,
                 global int16 *b16, global int16 *c16, global int16 *d16) {
  size_t i = get_global_id(0);
  d4[i] = mad_sat(a4[i], b4[i], c4[i]);
  d8[i] = mad_sat(a8[i], b8[i], c8[i]);
  d16[i] = mad_sat(a16[i], b16[i], c16[i]);
}

__kernel void test(global int4 *a, global int4 *b, global int4 *c) {
  int4 src1 = a[get_global_id(0)];
  int4 src2 = b[get_global_id(0)];
  uint4 mask = (uint4)(0, 7, 2, 3);
  c[get_global_id(0)] = shuffle2(src1, src2, mask);
}

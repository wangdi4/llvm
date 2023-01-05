__kernel void test(global float *a, global float *b, long16 c, char8 d) {
  a[get_global_id(0)] += b[get_global_id(0)];
  c *= (long16)(2);
  d += (char8)(123);
}


__kernel void test1(__global double16 *in, __global double16 *out) {
  double16 res = in[get_global_id(0)];
  ((uchar *)&res)[0] += 1;
  out[get_global_id(0)] = res;
}

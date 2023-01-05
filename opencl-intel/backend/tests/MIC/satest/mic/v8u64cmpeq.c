
__kernel void test_v8u64cmpeq(__global long8 *out, __global const ulong8 *a,
                              __global const ulong8 *b) {
  int index = get_global_id(0);
  out[index] = a[index] == b[index];
}

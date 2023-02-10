__kernel void test_masked_acos(__global double *a, __global double *c) {
  int i = get_global_id(0);
  if (a[i] <= 100 && a[i] >= -100) {
    c[i] = acosh(a[i]);
  }
}
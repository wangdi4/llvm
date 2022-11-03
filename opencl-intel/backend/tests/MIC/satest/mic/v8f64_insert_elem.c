
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void test_v8f64_insert_elem(__global double8 *f) {
  int index = get_global_id(0);
  f[index].s1 = 3.14;
  f[index].s5 = 3.14;
  f[index].s6 = 1.772;
}

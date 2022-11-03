// Test for work_group_all in SATest OpenCL Reference
__kernel void test_work_group_all(__global int *a) {
  printf("%d\n", work_group_all(a[get_global_id(0)]));
}

// Test for work_group_any in SATest OpenCL Reference
__kernel void test_work_group_any(__global int *a) {
  printf("%d\n", work_group_any(a[get_global_id(0)]));
}

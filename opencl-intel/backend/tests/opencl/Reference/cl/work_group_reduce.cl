// Test for work_group_reduce in SATest OpenCL Reference
__kernel void test_work_group_reduce(__global int *a) {
  printf("%d\n", work_group_reduce_add(a[get_global_id(0)]));
  printf("%d\n", work_group_reduce_min(a[get_global_id(0)]));
  printf("%d\n", work_group_reduce_max(a[get_global_id(0)]));
}

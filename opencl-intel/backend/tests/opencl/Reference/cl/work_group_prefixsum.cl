// Test for work_group_prefixsum in SATest OpenCL Reference
__kernel void test_work_group_prefixsum(__global int *a) {
  printf("%d\n", work_group_prefixsum_inclusive_add(a[get_global_id(0)]));
  printf("%d\n", work_group_prefixsum_inclusive_min(a[get_global_id(0)]));
  printf("%d\n", work_group_prefixsum_inclusive_max(a[get_global_id(0)]));

  printf("%d\n", work_group_prefixsum_exclusive_add(a[get_global_id(0)]));
  printf("%d\n", work_group_prefixsum_exclusive_min(a[get_global_id(0)]));
  printf("%d\n", work_group_prefixsum_exclusive_max(a[get_global_id(0)]));
}

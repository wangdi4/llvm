// Test for work_group_broadcast_1D in SATest OpenCL Reference
__kernel void test_work_group_broadcast(__global int *a) {
  size_t pid = 1; // item who broadcasts
  printf("%d\n", work_group_broadcast_1D(a[get_global_id(0)], pid));
}

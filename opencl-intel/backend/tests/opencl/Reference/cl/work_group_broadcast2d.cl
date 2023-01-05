// Test for work_group_broadcast_2D in SATest OpenCL Reference
__kernel void test_work_group_broadcast(__global int *a) {
  size_t pid[2] = {1, 1}; // item who broadcasts
  printf("%d\n",
         work_group_broadcast_2D(
             a[get_global_id(0) * get_global_size(1) + get_global_id(1)], pid));
}

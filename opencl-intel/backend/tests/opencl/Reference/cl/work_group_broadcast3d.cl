// Test for work_group_broadcast_3D in SATest OpenCL Reference
__kernel void test_work_group_broadcast(__global int *a) {
  size_t pid[3] = {1, 1, 0}; // item who broadcasts
  printf("%d\n",
         work_group_broadcast_3D(
             a[get_global_id(0) * get_global_size(1) * get_global_size(2) +
               get_global_id(1) * get_global_size(2) + get_global_id(2)],
             pid));
}

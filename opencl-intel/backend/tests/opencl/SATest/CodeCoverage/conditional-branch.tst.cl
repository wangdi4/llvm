__kernel void test_kernel(__global int *ptr) {
  if (get_local_id(0) == 0)
    *ptr = 10;
  if (get_local_id(0) == 1)
    *ptr = 20;
}

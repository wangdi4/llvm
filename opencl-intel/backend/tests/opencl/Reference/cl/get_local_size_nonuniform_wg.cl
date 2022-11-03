// Test for get_local_size in SATest OpenCL Reference
__kernel void test_get_local_size(__global int4 *a) {
  printf("%d\n", get_local_size(0));
  printf("%d\n", get_local_size(1));
  printf("%d\n", get_local_size(2));
}

// Test for get_local_linear_id in SATest OpenCL Reference
__kernel void test_get_local_linear_id(__global int *a) {
  printf("%d\n", get_local_linear_id());
}
__kernel void test_get_local_linear_id_2d(__global int *a) {
  printf("%d\n", get_local_linear_id());
}
__kernel void test_get_local_linear_id_3d(__global int *a) {
  printf("%d\n", get_local_linear_id());
}
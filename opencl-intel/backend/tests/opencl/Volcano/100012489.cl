kernel void printf_vector_test(__global int *rc) {
  printf("%#v2hlx\n", (uint2)(0x12345678, 0xFEDCBA));
  *rc = 0;
}

// This is a sample.
// This is a sample.

__kernel void test_fadd(__global char *buf1, __global char *buf2,
                        unsigned count) {
  size_t i = get_global_id(0);
  buf2[i + 1] = buf1[i];
}

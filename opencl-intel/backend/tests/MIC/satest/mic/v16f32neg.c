
__kernel void test_v16fxor32(__global float16 *out,
                             __global const float16 *in) {
  int index = get_global_id(0);
  out[index] = -0.0f - in[index];
}

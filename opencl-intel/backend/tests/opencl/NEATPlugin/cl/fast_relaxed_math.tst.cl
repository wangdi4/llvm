#define TEST_FRM_FUNCTION_KERNEL(_FUNC)                                        \
  __kernel void test_fast_relaxed_math_##_FUNC(                                \
      __global float *scalar_in, __global float *scalar_out,                   \
      __global float4 *vector_in, __global float4 *vector_out) {               \
    int tidX = get_global_id(0);                                               \
    scalar_out[tidX] = _FUNC(scalar_in[tidX]);                                 \
    vector_out[tidX] = _FUNC(vector_in[tidX]);                                 \
  }

TEST_FRM_FUNCTION_KERNEL(cos)
TEST_FRM_FUNCTION_KERNEL(sin)
TEST_FRM_FUNCTION_KERNEL(exp)
TEST_FRM_FUNCTION_KERNEL(exp2)
TEST_FRM_FUNCTION_KERNEL(exp10)
// TEST_FRM_FUNCTION_KERNEL(log) //disabled due to failures
// TEST_FRM_FUNCTION_KERNEL(log2) //disabled due to failures
TEST_FRM_FUNCTION_KERNEL(tan)

__kernel void test_fast_relaxed_math_div1(__global float *scalar_in,
                                          __global float *scalar_out,
                                          __global float4 *vector_in,
                                          __global float4 *vector_out) {
  int tidX = get_global_id(0);
  scalar_out[tidX] = 1.0f / scalar_in[tidX];
  vector_out[tidX] = 1.0f / vector_in[tidX];
}

__kernel void test_fast_relaxed_math_div(__global float *scalar_in1,
                                         __global float *scalar_in2,
                                         __global float *scalar_out,
                                         __global float4 *vector_in1,
                                         __global float4 *vector_in2,
                                         __global float4 *vector_out) {
  int tidX = get_global_id(0);
  scalar_out[tidX] = scalar_in1[tidX] / scalar_in2[tidX];
  vector_out[tidX] = vector_in1[tidX] / vector_in2[tidX];
}
__kernel void test_fast_relaxed_math_sincos(__global float *scalar_in,
                                            __global float *scalar_out1,
                                            __global float *scalar_out2,
                                            __global float4 *vector_in,
                                            __global float4 *vector_out1,
                                            __global float4 *vector_out2) {
  int tidX = get_global_id(0);
  scalar_out1[tidX] = sincos(scalar_in[tidX], &scalar_out2[tidX]);
  vector_out1[tidX] = sincos(vector_in[tidX], &vector_out2[tidX]);
}
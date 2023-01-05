// The kernel is used for testing builtin representation option.
__kernel void test_atomic_load(__global int *obj, __global int *val) {
  *val = atomic_load_explicit((volatile atomic_int *)obj, memory_order_relaxed);
}

__kernel void dot_product(__global float4 *a, __global float4 *b,
                          __global float *res) {
  size_t tid = get_global_id(0);

  res[tid] = dot(a[tid], b[tid]);
}

__kernel void math_func_test_f4(__global float4 *a, __global float4 *b) {
  size_t tid = get_global_id(0);

  b[tid] = sqrt(a[tid]);
}

__kernel void test_hostptr(__global float *srcA, __global float *srcB,
                           __global float *dst) {
  int tid = get_global_id(0);

  dst[tid] = srcA[tid] + srcB[tid];
}

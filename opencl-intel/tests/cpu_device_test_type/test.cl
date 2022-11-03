__kernel void dot_product(__global float4 *a, __global float4 *b,
                          __global float *res) {
  size_t tid = get_global_id(0);

  res[tid] = dot(a[tid], b[tid]);
}

__kernel void math_func_test_f4(__global float4 *a, __global float4 *b) {
  size_t tid = get_global_id(0);

  b[tid] = sqrt(a[tid]);
}

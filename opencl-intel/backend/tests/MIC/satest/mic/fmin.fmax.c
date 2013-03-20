__kernel void test_fmax_f64(__global double* in, __global double* out) {
  size_t idx = get_global_id(0);
  out[idx] = fmax(in[idx%2], in[(idx+1)%2]);
}
__kernel void test_fmax_f32(__global float* in, __global float* out) {
  size_t idx = get_global_id(0);
  out[idx] = fmax(in[idx%2], in[(idx+1)%2]);
}

__kernel void test_fmin_f64(__global double* in, __global double* out) {
  size_t idx = get_global_id(0);
  out[idx] = fmin(in[idx%2], in[(idx+1)%2]);
}
__kernel void test_fmin_f32(__global float* in, __global float* out) {
  size_t idx = get_global_id(0);
  out[idx] = fmin(in[idx%2], in[(idx+1)%2]);
}

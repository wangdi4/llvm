__kernel void acos8(__global float8 *out, __global float8 *in) {
  int i = get_global_id(0);
  out[i] = acos(in[i]);
}

__kernel void asin8(__global float8 *out, __global float8 *in) {
  int i = get_global_id(0);
  out[i] = asin(in[i]);
}

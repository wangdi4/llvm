
__kernel void vpInt4(__global int4 *in, __global int4 *out) {
  int i = get_global_id(0);
  prefetch(in + i + 8, 1);
  prefetch(out + i + 8, 1);
  out[i] = in[i];
}

__kernel void vpInt3(__global int3 *in, __global int3 *out) {
  int i = get_global_id(0);
  prefetch(in + i + 8, 1);
  prefetch(out + i + 8, 1);
  out[i] = in[i];
}

__kernel void vpDouble16(__global double16 *in, __global double16 *out) {
  int i = get_global_id(0);
  prefetch(in + i + 8, 1);
  prefetch(out + i + 8, 1);
  out[i] = in[i];
}

__kernel void f(__global int *src, __global int *dst) {
  int i = get_global_id(0);
  if (i < 10)
    dst[i] = src[i];
}

kernel void copy(global uchar *restrict dst, const global uchar *restrict src) {
  size_t gid = get_global_id(0);
  dst[gid] = src[gid];
}

kernel void set(global uchar *dst, uchar value) {
  dst[get_global_id(0)] = value;
}

kernel void set_zero(global uchar *dst) { dst[get_global_id(0)] = 0; }

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  size_t gid = get_global_id(0);
  uchar tmp = buf_in[gid];
  buf_out[gid] = tmp;
}

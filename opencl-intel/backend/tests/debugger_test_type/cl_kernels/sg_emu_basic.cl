__attribute__((intel_reqd_sub_group_size(16))) __kernel void
main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  int lid = get_local_id(0);
  int a = convert_int(buf_in[lid]);
  int x = sub_group_scan_inclusive_add(a);
  buf_out[lid] = convert_uchar(x);
}
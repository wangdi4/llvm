__attribute__((intel_reqd_sub_group_size(16))) __kernel void
main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  int lid = get_local_id(0);
  int x = work_group_reduce_add(lid);
  int y = sub_group_scan_inclusive_add(x);
  buf_out[lid] = convert_uchar(y);
}
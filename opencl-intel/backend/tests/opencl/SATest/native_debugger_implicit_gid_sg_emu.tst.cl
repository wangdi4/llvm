// Test kernel with subgroup builtins, check whether __ocl_dbg_gids work for
// subgroup emulation.
__kernel void main_kernel(__global int *buf_in, __global int *buf_out) {
  int lid = get_local_id(0);
  buf_out[lid] = sub_group_scan_inclusive_add(buf_in[lid]);
}

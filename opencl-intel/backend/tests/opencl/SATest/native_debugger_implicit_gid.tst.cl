// Test normal kernel, check whether __ocl_dbg_gids are inserted properly.
__kernel void main_kernel(__global int *buf_in, __global int *buf_out) {
  int lid = get_local_id(0);
  barrier(CLK_LOCAL_MEM_FENCE);
}

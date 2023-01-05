kernel void test(global float *a, global float *b) {
  size_t gid = get_global_id(0);
  b[gid] = sin(a[gid]);
}

kernel void testBarrier(global float *a, global float *b) {
  size_t gid = get_global_id(0);
  barrier(CLK_LOCAL_MEM_FENCE);
  b[gid] = sin(a[gid]) + get_sub_group_size();
}

kernel void test() {
  size_t gid = get_global_id(0);
  barrier(CLK_LOCAL_MEM_FENCE);
}

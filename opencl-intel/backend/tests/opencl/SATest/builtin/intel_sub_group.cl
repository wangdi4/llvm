kernel void test(global char *bufferC, global ulong *bufferL) {
  char c = (char)get_sub_group_local_id();
  bufferC[get_global_id(0)] = intel_sub_group_broadcast(c, 0);
  bufferL[get_global_id(0)] = intel_sub_group_block_read_ul(bufferL);
}

__kernel void test_mem_fence(const __global uint *p_in, __global uint *p_out) {
  size_t id = get_global_id(0);

  uint sum = 0;
  uint start = 7 * id;

  for (int i = 0; i < 10; i++) {
    sum += start;
    start++;
  }
  p_out[id] = sum;
  mem_fence(CLK_GLOBAL_MEM_FENCE);

  for (int i = 0; i < 10; i++) {
    sum += start + p_in[id];
    start++;
  }
  read_mem_fence(CLK_GLOBAL_MEM_FENCE);

  p_out[id] = sum;

  write_mem_fence(CLK_GLOBAL_MEM_FENCE);
}

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  // Initialize once (to 65800)
  if (get_global_id(0) == 20) {
    buf_out[0] = 8;
    buf_out[1] = 1;
    buf_out[2] = 1;
    buf_out[3] = 0;
  }
  barrier(CLK_LOCAL_MEM_FENCE); // make sure initialization happened

  __global unsigned int *bufuip = &buf_out[0];

  // always happens...
  if (buf_out[3] + buf_out[2] < 2) {
    atomic_add(bufuip, 1);
    barrier(CLK_LOCAL_MEM_FENCE);
  }

  // now the shared value was incremented #WI times

  for (int i = 0; i < 10; ++i) {
    atomic_sub(bufuip, 2);
    barrier(CLK_LOCAL_MEM_FENCE);
  }

  // the 'for' loop decremented the shared value 20 * #WI times

  uint val2 = *((__global uint *)&buf_out[0]);

  buf_out[6] = 0;
}

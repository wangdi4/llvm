__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  buf_out[0] = 0;
  barrier(CLK_LOCAL_MEM_FENCE);
  barrier(CLK_LOCAL_MEM_FENCE);
  barrier(CLK_LOCAL_MEM_FENCE);
  uchar b = buf_in[13];
  buf_out[6] = 0;
}

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  int ii = 170, jj = 400;
  barrier(CLK_LOCAL_MEM_FENCE);

  ii += 30;
  buf_out[0] = 0;
}

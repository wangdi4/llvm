__constant int globalInt = 1;
__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  bool bb = false;
  int ii = 1;
  __local float ll;
  __private double pp = 3;
  volatile int vv = 4;
  if (get_global_id(0) == 0 && get_global_id(1) == 0 && get_global_id(2) == 0) {
    ll = 2.0f;
    buf_out[0] = 99;
  }

  barrier(CLK_GLOBAL_MEM_FENCE);
  ii++;
  ll += 1.0f;
  pp += 1.0f;
  vv++;

  barrier(CLK_GLOBAL_MEM_FENCE);
  buf_out[0] = 0;
}

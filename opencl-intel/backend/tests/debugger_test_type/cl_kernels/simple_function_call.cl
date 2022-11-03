void foo(uchar c) {
  uchar d = c;
  d = d + 1;
}

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  foo(9);
  uchar p = 9;
  foo(p);
  p = p + 1;
  foo(p);
  p = p + 2;
  foo(p);
  buf_out[0] = 0;
}

uchar rrrr(uchar c) {
  uchar d = c + 1;
  return d;
}

void kwaaa(uchar c) {
  uchar d = c;
  return;
}

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  kwaaa(buf_in[0]);
  buf_out[0] = rrrr(buf_in[0]);
}

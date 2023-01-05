__kernel void fourth_kernel() {
  int i1 = 4;
  int i2 = 2;
  uchar c = i1 * i2;
  i1++;
  return;
}

__kernel void third_kernel() {
  int i1 = 3;
  int i2 = 2;
  uchar c = i1 * i2;
  fourth_kernel();
  return;
}

__kernel void second_kernel() {
  int i1 = 2;
  int i2 = 2;
  uchar c = i1 * i2;
  third_kernel();
  return;
}

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  int i1 = 1;
  int i2 = 2;
  uchar c = i1 * i2;
  second_kernel();
  return;
}

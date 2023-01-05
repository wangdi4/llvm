__constant uchar c = 6;
__constant int b = 5;
__constant int d = 4;
__kernel void fourth_kernel() { return; }

__kernel void third_kernel() {
  fourth_kernel();
  return;
}

__kernel void second_kernel() {
  third_kernel();
  return;
}

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  second_kernel();
  return;
}

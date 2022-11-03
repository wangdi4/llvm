__kernel void fourth_kernel(int a, int b) {
  a++;
  b++;
  b++;
  return;
}

__kernel void third_kernel(int a, int b) {
  a++;
  b++;
  fourth_kernel(a, b);
  return;
}

__kernel void second_kernel(int a, int b) {
  a++;
  b++;
  third_kernel(a, b);
  return;
}

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  int a = 1;
  int b = 2;
  second_kernel(a, b);
  return;
}

int test2();
int test3();
int test1() {
  int i1 = 1;
  return test2();
}

int test2() {
  int i2 = 2;
  return test3();
}

int test3() {
  int i3 = 3;
  return i3;
}

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  test1();
  return;
}

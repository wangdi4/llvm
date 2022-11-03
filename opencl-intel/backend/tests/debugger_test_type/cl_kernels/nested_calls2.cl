int test2();
int test3();
int test1() {
  int i1 = 1;
  i1++;
  test2();
  return i1;
}

int test2() {
  int i2 = 2;
  i2++;
  test3();
  return i2;
}

int test3() {
  int i3 = 3;
  i3++;
  return i3;
}

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  test3();
  int a = 1;
  a = test1();
  a = test2();
  a++;
  a = test2();
  a++;
  return;
}

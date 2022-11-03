__constant ulong4 globalLong = (ulong4)(1, 22, 333, 4444);

void foo() {
  int a = 1;
  a = a + 3;
  return;
}

int test3(int a, int b) {
  int i3 = a;
  i3++;
  volatile int c3 = 2;
  int *r3 = (int *)67;
  if (c3 > 1) {
    int ai3 = i3;
    volatile int ac3 = c3;
    ai3++;
  }
  a++;
  return i3;
}

int test2(int a) {
  int i2 = a;
  i2++;
  volatile int c2 = 2;
  int *r2 = (int *)67;
  test3(i2, a);
  if (c2 > 1) {
    int ai2 = i2;
    volatile int ac2 = c2;
    foo();
    ai2++;
  }
  a++;
  return i2;
}

int test1(int a) {
  int i1 = a;
  i1++;
  volatile int c1 = 2;
  int *r1 = (int *)67;
  test2(i1);
  foo();
  if (c1 > 1) {
    int ai1 = i1;
    volatile int ac1 = c1;
    ai1++;
  }
  a++;
  return i1;
}

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  int a = 1;
  int b = test1(a);
  a++;
  return;
}

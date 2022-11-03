void foo(int gf, int lf, __private int pff) {
  __private int pf = 3;
  volatile int vf = 4;
  int af = 5;
  int *rf = (int *)67;
  af++;
  return;
}
__constant int globalInt = 1;
__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  int g = 1;
  __local int l;
  __private int p = 3;
  volatile int v = 4;
  int a = 5;
  int *r = (int *)6;
  if (v > 3) {
    int gb = 1;
    __private int pb = 3;
    volatile int vb = 4;
    int ab = 5;
    int *rb = (int *)678;
    ab++;
    while (vb > 3) // one iteration only
    {
      int gb2 = 1;
      __private int pb2 = 3;
      volatile int vb2 = 4;
      int ab2 = 5;
      int *rb2 = (int *)6789;
      foo(g, l, p);
      vb--;
    }
    vb = 4;
    ab--;
  }
  foo(g, l, p);
  a++;
  return;
}

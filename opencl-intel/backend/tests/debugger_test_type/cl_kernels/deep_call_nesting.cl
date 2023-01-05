
uchar foo9(uchar c) // leaf!
{
  uchar d = c * 5 - c / 3;
  return d;
}

uchar foo8(uchar c) {
  uchar d = c - 555;
  d = foo9(d + c);
  return d;
}

uchar foo7(uchar c) {
  uchar d = c - 5;
  d = foo8(d + c);
  return d;
}

uchar foo6(uchar c) {
  uchar d = c * 4435;
  d = foo7(d + c);
  return d;
}

uchar foo5(uchar c) {
  uchar d = c * 125;
  d = foo6(d + c);
  return d;
}

uchar foo4(uchar c) {
  uchar d = c * 445;
  d = foo5(d + c);
  return d;
}

uchar foo3(uchar c) {
  uchar d = c * 115;
  d = foo4(d + c);
  return d;
}

uchar foo2(uchar c) {
  uchar d = c * 35;
  d = foo3(d + c);
  return d;
}

uchar foo1(uchar c) {
  uchar d = c * 25;
  d = foo2(d + c);
  return d;
}

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  uchar p = 9;
  buf_out[1] = foo1(p);
  buf_out[0] = 0;
}

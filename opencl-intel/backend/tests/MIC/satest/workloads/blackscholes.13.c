#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define ty float
#define MOD(A, B) (A % B)
__kernel void tile(__global ty *d_out, unsigned Xo, unsigned Yo, unsigned Zo,
                   const __global ty *d_in, unsigned Xi, unsigned Yi,
                   unsigned Zi) {
  int xo = get_global_id(0);
  int yo = get_global_id(1);
  if (xo >= Xo || yo >= Yo)
    return;
  int xi = MOD(xo, Xi);
  int yi = MOD(yo, Yi);
  int oid = yo * Xo + xo;
  int iid = yi * Xi + xi;
  int xyo = Xo * Yo;
  int xyi = Xi * Yi;
  for (int zo = 0; zo < Zo; zo++) {
    int zi = MOD(zo, Zi);
    d_out[zo * xyo + oid] = d_in[zi * xyi + iid];
  }
}

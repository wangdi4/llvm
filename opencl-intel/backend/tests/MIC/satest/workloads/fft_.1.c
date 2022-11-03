#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define ti float
#define to float2
#define CPLX 0
to convert(ti a) {
#if CPLX
  return a;
#else
  to b;
  b.x = a;
  b.y = 0;
  return b;
#endif
}
__kernel void fft_convert(__global to *d_out, unsigned Xo, unsigned Yo,
                          unsigned Zo, const __global ti *d_in, unsigned Xi,
                          unsigned Yi, unsigned Zi, unsigned n) {
  int x = get_global_id(0);
  int y = get_global_id(1);
  if (x >= Xo || y >= Yo)
    return;
  int oid = y * Xo + x + n * Xo * Yo * Zo;
  int iid = y * Xi + x + n * Xi * Yi * Zi;
  int no = Xo * Yo;
  int ni = Xi * Yi;
  for (int z = 0; z < Zo; z++) {
    if (x >= Xi || y >= Yi || z >= Zi)
      d_out[z * no + oid] = 0;
    else
      d_out[z * no + oid] = convert(d_in[z * ni + iid]);
  }
}

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define ti uchar
#define to float
#define CPLX 0
#define NOTNULL 1
#define NEG(A) -A
#if CPLX
to ABS(ti tmp) { return sqrt(tmp.x * tmp.x + tmp.y * tmp.y); }
ti conj(ti tmp) {
  tmp.y = -tmp.y;
  return tmp;
}
to real(ti tmp) { return tmp.x; }
to imag(ti tmp) { return tmp.y; }
to CONVERT(ti tmp) {
  to out;
  out.x = tmp;
  out.y = 0;
  return out;
}
#else
#define ABS(foo) fabs(foo)
ti conj(ti tmp) { return tmp; }
to real(ti tmp) { return tmp; }
to imag(ti tmp) { return 0; }
to CONVERT(ti tmp) { return (to)tmp; }
#endif
#if NOTNULL
#define GETK(in, k) in[k]
#else
#define GETK(in, k) 0
#endif
#define ones(foo) 1
#define zeros(foo) 0
__kernel void unary(const __global ti *in, __global to *out, int numel) {
  int tid = get_global_id(1) * get_global_size(0) + get_global_id(0);
  int off = get_global_size(0) * get_global_size(1);
  for (int k = tid; k < numel; k += off) {
    ti tmp = GETK(in, k);
    out[k] = (to)(CONVERT(tmp));
  }
}

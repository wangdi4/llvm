#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define ti float
#define to float
#define CPLX 0
#define ADD(L, R) (L) + (R)
#define SUB(L, R) (L) - (R)
#if CPLX
#define SQ(a) ((a) * (a))
#define SABS(in) SQ(in.x) + SQ(in.y)
to MUL(to left, to right) {
  to ret;
  ret.x = left.x * right.x - left.y * right.y;
  ret.y = left.x * right.y + left.y * right.x;
  return ret;
}
to DIV(to left, to right) {
  to ret;
  ret.x = (left.x * right.x + left.y * right.y);
  ret.y = (left.x * right.y - left.y * right.x);
  return ret / SABS(right);
}
#define CMIN(left, right) (SABS(left)) < (SABS(right)) ? (left) : (right)
to MIN(to left, to right) { return CMIN(left, right); }
#define CMAX(left, right) (SABS(left)) < (SABS(right)) ? (left) : (right)
to MAX(to left, to right) { return CMAX(left, right); }
#else
#define MUL(L, R) (L) * (R)
#define DIV(L, R) (L) / (R)
#define MIN(L, R) min((L), (R))
#define MAX(L, R) max((L), (R))
#define LE(L, R) (L) <= (R)
#define GE(L, R) (L) >= (R)
#define LT(L, R) (L) < (R)
#define GT(L, R) (L) > (R)
#define EQ(L, R) (L) == (R)
#define NE(L, R) (L) != (R)
#define AND(L, R) (L) && (R)
#define OR(L, R) (L) || (R)
#define BITOR(L, R) (L) | (R)
#define BITXOR(L, R) (L) ^ (R)
#define BITAND(L, R) (L) & (R)
#endif
__kernel void binary_rscalar(const __global ti *left, ti right,
                             __global to *out, unsigned numel) {
  int tid = get_global_id(1) * get_global_size(0) + get_global_id(0);
  int off = get_global_size(0) * get_global_size(1);
  for (int k = tid; k < numel; k += off) {
    ti l = left[k];
    out[k] = (to)(SUB(l, right));
  }
}

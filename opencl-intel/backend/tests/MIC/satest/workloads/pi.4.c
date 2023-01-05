#define ADD
#define ti uchar
#define TYPE_float
#if defined(TYPE_double) || defined(TYPE_cdouble)
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif
#if defined(TYPE_float2) || defined(TYPE_double2)
#ifdef TYPE_float2
#define to float2
#define re float
#else
#define to double2
#define re double
#endif
#define SQ(a) ((a) * (a))
#define SABS(in) SQ(in.x) + SQ(in.y)
#define ABS(in) sqrt(SABS(in))
to make_cplx(re x, re y) {
  to ret;
  ret.x = x;
  ret.y = y;
  return ret;
}
#ifdef ADD
to init() { return make_cplx(0, 0); }
to do_op(to left, to right) { return left + right; }
#endif
#ifdef SUB
to init() { return make_cplx(0, 0); }
to do_op(to left, to right) { return left - right; }
#endif
#ifdef MUL
to init() { return make_cplx(1, 0); }
to do_op(to left, to right) {
  to ret;
  ret.x = left.x * right.x - left.y * right.y;
  ret.y = left.x * right.y + left.y * right.x;
  return ret;
}
#endif
#ifdef DIV
to init() { return make_cplx(1, 0); }
to do_op(to left, to right) {
  to ret;
  re den = SABS(right);
  ret.x = (left.x * right.x + left.y * right.y) / den;
  ret.y = (left.x * right.y - left.y * right.x) / den;
  return ret;
}
#endif
#ifdef MIN
to init() { return make_cplx(INFINITY, 0); }
#define op(left, right) (SABS(left)) < (SABS(right)) ? (left) : (right)
to do_op(to left, to right) { return op(left, right); }
#undef op
#endif
#ifdef MAX
to init() { return make_cplx(0, 0); }
#define op(left, right) (SABS(left)) < (SABS(right)) ? (left) : (right)
to do_op(to left, to right) { return op(left, right); }
#undef op
#endif
#else
#define SABS(in) (in)
#ifdef TYPE_uchar
#define to uchar
#endif
#ifdef TYPE_float
#define to float
#endif
#ifdef TYPE_double
#define to double
#endif
#ifdef TYPE_int
#define to int
#endif
#ifdef TYPE_unsigned
#define to unsigned
#endif
#ifdef ADD
to init() { return 0; }
to do_op(to left, to right) { return (left) + (right); }
#endif
#ifdef SUB
to init() { return 0; }
to do_op(to left, to right) { return (left) - (right); }
#endif
#ifdef MUL
to init() { return 1; }
to do_op(to left, to right) { return (left) * (right); }
#endif
#ifdef DIV
to init() { return 1; }
to do_op(to left, to right) { return (left) / (right); }
#endif
#ifdef MIN
to init() {
#ifdef TYPE_float
  return INFINITY;
#endif
#ifdef TYPE_double
  return INFINITY;
#endif
#ifdef TYPE_int
  return 0x7fffffff;
#endif
#ifdef TYPE_unsigned
  return 0xffffffff;
#endif
#ifdef TYPE_uchar
  return 1;
#endif
}
to do_op(to left, to right) { return min(left, right); }
#endif
#ifdef MAX
to init() {
#ifdef TYPE_float
  return -INFINITY;
#endif
#ifdef TYPE_double
  return -INFINITY;
#endif
#ifdef TYPE_int
  return 0xffffffff;
#endif
#ifdef TYPE_unsigned
  return 0;
#endif
#ifdef TYPE_uchar
  return 0;
#endif
}
to do_op(to left, to right) { return max(left, right); }
#endif
#ifdef ANY
to init() { return 0; }
to do_op(to left, to right) { return left | (right != 0); }
#endif
#ifdef ALL
to init() { return 1; }
to do_op(to left, to right) { return left & (right != 0); }
#endif
#ifdef NNZ
to init() { return 0; }
to do_op(to left, to right) { return (left) + (right != 0); }
#endif
#endif
__kernel void reduce_column(const __global ti *d_X, __global to *d_Z,
                            int numel) {
  __local to dmem[256];
  int tid_ = get_local_id(0);
  int bid = get_group_id(0);
  int nid = get_group_id(1);
  int tid = get_global_id(0);
  int off = get_global_size(0);
  d_X = d_X + numel * nid;
  d_Z = d_Z + nid;
  __local to *smem = dmem + tid_;
  *smem = (to)init();
  for (int k = tid; k < numel; k += off) {
#if defined(ANY) || defined(ALL)
    *smem = do_op(*smem, (to)SABS(d_X[k]));
#else
    *smem = do_op(*smem, (to)d_X[k]);
#endif
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  for (int id = 128; id > 0; id >>= 1) {
    if (tid_ < id)
      *smem = do_op(*smem, smem[id]);
    barrier(CLK_LOCAL_MEM_FENCE);
  }
  if (tid_ == 0)
    d_Z[bid] = smem[0];
}
__kernel void reduce_stride(const __global ti *d_X, __global to *d_Z, int numel,
                            int stride, int batch) {
  __local to dmem[256];
  unsigned tid = get_global_id(1);
  unsigned gid = get_global_id(0);
  unsigned nid = gid / stride;
  unsigned bid = gid - nid * stride;
  unsigned xid = get_local_id(0);
  unsigned yid = get_local_id(1);
  if (gid >= stride * batch)
    return;
  __local to *smem = dmem + yid * get_local_size(0) + xid;
  *smem = (to)init();
  int off = get_global_size(1);
  d_X = d_X + bid * (numel * stride) + tid * stride + nid + get_local_id(0);
  d_Z = d_Z + bid * (off * stride) + tid * stride + nid + get_local_id(0);
  for (int k = tid; k < numel; k += off) {
#if defined(ANY) || defined(ALL)
    *smem = do_op(*smem, (to)SABS(d_X[k]));
#else
    *smem = do_op(*smem, (to)d_X[k]);
#endif
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  for (int id = 4; id > 0; id >>= 1) {
    if (yid < id)
      *smem = do_op(*smem, smem[id * 32]);
    barrier(CLK_LOCAL_MEM_FENCE);
  }
}

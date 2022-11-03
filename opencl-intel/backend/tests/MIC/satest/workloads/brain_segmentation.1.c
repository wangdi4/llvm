#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define ndims 3
#define ty float
enum stype_t {
  S_LIN,          ///< Linear (<tt>inc*i+first</tt>)
  S_NON,          ///< Nonlinear (directly specified in \p d_ind)
  S_GPU,          ///< Read from GPU device memory (zero-indexed)
  S_I,            ///< GFOR iterator
  S_IRITH_SCALAR, ///< GFOR iterator with scalar arithmetic
  S_IRITH_MATRIX, ///< GFOR iterator with non-scalar arithmetic
};
/// What type of data is this?
enum sty_t {
  STY_BOOL, ///< Boolean (only scalar subsasgn)
  STY_F32,  ///< Single-precision (32-bit)
  STY_F64,  ///< Double-precision (64-bit)
  STY_U32,  ///< Unsigned integer (32-bit)
};
/// Metadata describing subscript
typedef struct {
  enum stype_t type; ///< What type of subscript?
  union {
    struct {
      int first, inc;
    } lin; ///< Linear span (zero-index)
    struct {
      unsigned pos;
    } non; ///< Position in inds array (pos: zero-index)
    struct {
      // CHECKME
      // Changed from ty to tp for opencl
      // Added enum for opencl
      enum sty_t tp; ///< underlying data type
      int id;        ///< Which device pointer (0,1,2)
    } gpu;           ///< Which device pointer to use
  } d;
} sub_t;
#define MEM_ACCESS(type, index)                                                \
  (((type) == STY_F32   ? (unsigned)((__global const float *)g)[index]         \
    : (type) == STY_U32 ? (unsigned)((__global const unsigned *)g)[index]      \
                        : (unsigned)((__global const double *)g)[index]) -     \
   base)
#ifndef nsubs
#define nsubs ndims
#else
#define ndims nsubs
#endif
// convert between subscripts and linear indices (both zero-based)
unsigned sub2x(__global const unsigned *dims, unsigned *subs) {
  // compute linear index
  unsigned x = subs[ndims - 1];
  for (int i = ndims - 2; i >= 0; --i)
    x = x * dims[i] + subs[i];
  return x;
}
void x2sub(unsigned x, unsigned *sub, __global const unsigned *dims) {
  for (int i = 0; i < ndims; ++i) {
    unsigned y = x / dims[i];
    sub[i] = x - dims[i] * y;
    x = y;
  }
}
void sub2sub(unsigned *sub, __global const sub_t *subs, unsigned base,
             __global const unsigned *non, __global const void *d0,
             __global const void *d1, __global const void *d2) {
  for (int i = 0; i < nsubs; ++i) {
    if (subs[i].type == S_LIN) {
      int first = subs[i].d.lin.first, inc = subs[i].d.lin.inc;
      sub[i] = sub[i] * inc + first; // ramp
    } else if (subs[i].type == S_GPU) {
      __global const void *g = (subs[i].d.gpu.id == 0)   ? d0
                               : (subs[i].d.gpu.id == 1) ? d1
                                                         : d2;
      sub[i] = MEM_ACCESS(subs[i].d.gpu.tp, sub[i]); // look up in buffer
    } else if (subs[i].type == S_NON) {
      sub[i] = non[subs[i].d.non.pos + sub[i]] - base; // direct lookup
    }
  }
}
// gfor version
void sub2subn(unsigned *sub, __global const sub_t *subs, unsigned base,
              __global const unsigned *non, __global const void *d0,
              __global const void *d1, __global const void *d2, float iter,
              unsigned id, __global const unsigned *dims) {
  for (int i = 0; i < nsubs; ++i) {
    if (subs[i].type == S_LIN) {
      int first = subs[i].d.lin.first, inc = subs[i].d.lin.inc;
      sub[i] = sub[i] * inc + first; // ramp
    } else if (subs[i].type == S_GPU) {
      __global const void *g = (subs[i].d.gpu.id == 0)   ? d0
                               : (subs[i].d.gpu.id == 1) ? d1
                                                         : d2;
      sub[i] = MEM_ACCESS(subs[i].d.gpu.tp, sub[i]); // look up in buffer
    } else if (subs[i].type == S_IRITH_MATRIX) {
      __global const void *g = (subs[i].d.gpu.id == 0)   ? d0
                               : (subs[i].d.gpu.id == 1) ? d1
                                                         : d2;
      sub[i] = MEM_ACCESS(subs[i].d.gpu.tp,
                          dims[i] * id + sub[i]); // index into tile
    } else if (subs[i].type == S_IRITH_SCALAR) {
      __global const void *g = (subs[i].d.gpu.id == 0)   ? d0
                               : (subs[i].d.gpu.id == 1) ? d1
                                                         : d2;
      sub[i] = MEM_ACCESS(subs[i].d.gpu.tp, id); // look up tile
    } else if (subs[i].type == S_NON) {
      sub[i] = non[subs[i].d.non.pos + sub[i]] - base; // direct lookup
    } else if (subs[i].type == S_I) {
      int first = subs[i].d.lin.first, inc = subs[i].d.lin.inc;
      sub[i] = inc * iter + first; // ramp
    }
  }
}
unsigned x2x_(unsigned x, unsigned base, __global const void *d1,
              __global const void *d2, __global const void *d3,
              __global const unsigned *c_meta) {
  // unmarshall
  __global const unsigned *dims_top = c_meta;
  __global const unsigned *dims_bot = dims_top + nsubs;
  __global const sub_t *subs = (__global const sub_t *)(dims_bot + nsubs);
  __global const unsigned *non = (__global const unsigned *)(subs + nsubs);
  // map: x -> x'
  unsigned sub[nsubs];
  x2sub(x, sub, dims_top);
  sub2sub(sub, subs, base, non, d1, d2, d3);
  return sub2x(dims_bot, sub);
}
// gfor version
unsigned x2x_n(unsigned x, unsigned base, __global const void *d1,
               __global const void *d2, __global const void *d3, float i,
               unsigned id, __global const unsigned *c_meta) {
  // unmarshall
  __global const unsigned *dims_top = c_meta;
  __global const unsigned *dims_bot = dims_top + nsubs;
  __global const sub_t *subs = (__global const sub_t *)(dims_bot + nsubs);
  __global const unsigned *non = (__global const unsigned *)(subs + nsubs);
  // map: x -> x'
  unsigned sub[nsubs];
  x2sub(x, sub, dims_top);
  sub2subn(sub, subs, base, non, d1, d2, d3, i, id, dims_top);
  return sub2x(dims_bot, sub);
}
#ifdef __CUDACC__
// maybe since these are used frequently, we should make them softer on the
// eyes, e.g. __main instead of MAIN
#define MAIN __global__
#define DEVICE __device__
#define GLOBAL
#define SHARED __shared__
#define LOCAL
#define SYNC() __syncthreads()
#define TIDX threadIdx.x
#define TIDY threadIdx.y
#define BIDX blockIdx.x
#define BIDY blockIdx.y
#define BDMX blockDim.x
#define BDMY blockDIm.y
#define GDMX gridDim.x
#define GDMY gridDim.y
#define MOD(A, B) ((A) % (B))
#define STATIC static
#define BOOL bool
#else
#ifdef AFCL // is there not some OpenCL directive?
#define MAIN __kernel
#define DEVICE
#define GLOBAL __global
#define SHARED __local
#define LOCAL __local
#define SYNC() barrier(CLK_LOCAL_MEM_FENCE);
#define TIDX get_local_id(0)
#define TIDY get_local_id(1)
#define BIDX get_group_id(0)
#define BIDY get_group_id(1)
#define BDMX get_local_size(0)
#define BDMY get_local_size(1)
#define GDMX get_num_groups(0)
#define GDMY get_num_groups(1)
#define MOD(A, B) ((A) % (B))
#define STATIC // opencl does not like static kernels
#define BOOL uchar
#endif // AFCL
#endif // CUDACC
void _getkernel_0(__global ty *dst, const __global ty *src, unsigned ndst,
                  unsigned nsrc, unsigned ngfor, float start, float inc,
                  const __global void *d1, const __global void *d2,
                  const __global void *d3, unsigned base,
                  const __global unsigned *c_meta) {
  const unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  __global ty *d_dst = (__global ty *)dst + x;
  const __global ty *d_src = (const __global ty *)src;
  if (x >= ndst)
    return;
  const unsigned y = x2x_(x, base, d1, d2, d3, c_meta);
  *d_dst = (y < nsrc) ? d_src[y] : NAN; // ensure bounds
}
void _getkernel_1(__global ty *dst, const __global ty *src, unsigned ndst,
                  unsigned nsrc, unsigned ngfor, float start, float inc,
                  const __global void *d1, const __global void *d2,
                  const __global void *d3, unsigned base,
                  const __global unsigned *c_meta) {
  const unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  unsigned id = x / ndst, x_ = x - id * ndst; // block id and index
  float i = id * inc + start;                 // actual iterator value
  __global ty *d_dst = (__global ty *)dst + x;
  const __global ty *d_src = (const __global ty *)src;
  if (x >= ngfor * ndst)
    return;
  const unsigned y = x2x_n(x_, base, d1, d2, d3, i, id, c_meta);
  *d_dst = (y < nsrc) ? d_src[y] : NAN; // ensure bounds
}
void _getkernel_2(__global ty *dst, const __global ty *src, unsigned ndst,
                  unsigned nsrc, unsigned ngfor, float start, float inc,
                  const __global void *d1, const __global void *d2,
                  const __global void *d3, unsigned base,
                  const __global unsigned *c_meta) {
  const unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  unsigned id = x / ndst, x_ = x - id * ndst; // block id and index
  __global ty *d_dst = (__global ty *)dst + x;
  const __global ty *d_src =
      (const __global ty *)src + id * nsrc; // tile offset
  if (x >= ngfor * ndst)
    return;
  const unsigned y = x2x_(x_, base, d1, d2, d3, c_meta);
  *d_dst = (y < nsrc * ngfor) ? d_src[y] : NAN; // ensure bounds
}
void _getkernel_3(__global ty *dst, const __global ty *src, unsigned ndst,
                  unsigned nsrc, unsigned ngfor, float start, float inc,
                  const __global void *d1, const __global void *d2,
                  const __global void *d3, unsigned base,
                  const __global unsigned *c_meta) {
  const unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  unsigned id = x / ndst, x_ = x - id * ndst; // block id and index
  float i = id * inc + start;                 // actual iterator value
  __global ty *d_dst = (__global ty *)dst + x;
  const __global ty *d_src =
      (const __global ty *)src + id * nsrc; // tile offset
  if (x >= ngfor * ndst)
    return;
  const unsigned y = x2x_n(x_, base, d1, d2, d3, i, id, c_meta);
  *d_dst = (y < nsrc * ngfor) ? d_src[y] : NAN; // ensure bounds
}
__kernel void getkernel(__global ty *dst, const __global ty *src, unsigned ndst,
                        unsigned nsrc, unsigned ngfor, float start, float inc,
                        const __global void *d1, const __global void *d2,
                        const __global void *d3, unsigned base,
                        const __global unsigned *c_meta) {
  _getkernel_0(dst, src, ndst, nsrc, ngfor, start, inc, d1, d2, d3, base,
               c_meta);
}

#define nsubs 3
#define ts float
#define KER_3
#define ISCPLX 0
#define td ts
#define ty ts
#if ISCPLX
#define setcplx(a, b) (a) = (b)
#define setscalar(a, b)                                                        \
  do {                                                                         \
    a.x = b;                                                                   \
    a.y = 0;                                                                   \
  } while (0)
#else
#define setcplx(a, b) (a) = (b).x
#define setscalar(a, b) (a) = (b)
#endif
#define TS __global ts
#define TD __global td
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
typedef struct {
  TD *dst;
  TS *src;
  unsigned ndst, nsrc, ngfor;
  float start, inc;
  const __global void *d1;
  const __global void *d2;
  const __global void *d3;
  float2 scalar; // setscalar
  unsigned base;
  const __global unsigned *c_meta;
} param_t;
// Please do not put any space before #include
// The parser doesnt parse it well if thats the case
//---- core kernels ----//
// indexed subsasgn, read scalar from parameter stack
void _subs_s(unsigned x, param_t p, float i, unsigned id) {
#define SCALAR
  /* determine output index (and if bailing) */
#ifdef GUARDED
  unsigned x_ = x;
  if (!bool_x2x_(&x_, p.base, p.d1, p.d2, p.d3, i, id))
    return; /* bail */
#else
  const unsigned x_ = x2x_n(x, p.base, p.d1, p.d2, p.d3, i, id, p.c_meta);
#endif
  /* pull in B */
  td src;
#if defined(SCALAR)
  setcplx(src, p.scalar);
#elif defined(ISCALAR)
  setscalar(src, i);
#elif defined(GSCALAR)
  TS *d_src = (TS *)p.src;
  src = d_src[0];
#elif defined(NONSCALAR)
  TS *d_src = (TS *)p.src;
  src = d_src[x];
#endif
  /* perform subsasgn */
  TD *d_dst = (TD *)p.dst;
  if (x_ < p.ndst)
    d_dst[x_] = src;
#undef GUARDED
#undef SCALAR
#undef ISCALAR
#undef GSCALAR
#undef NONSCALAR
}
// indexed subsasgn, read scalar from global memory
void _subs_sg(unsigned x, param_t p, float i, unsigned id) {
#define GSCALAR
  /* determine output index (and if bailing) */
#ifdef GUARDED
  unsigned x_ = x;
  if (!bool_x2x_(&x_, p.base, p.d1, p.d2, p.d3, i, id))
    return; /* bail */
#else
  const unsigned x_ = x2x_n(x, p.base, p.d1, p.d2, p.d3, i, id, p.c_meta);
#endif
  /* pull in B */
  td src;
#if defined(SCALAR)
  setcplx(src, p.scalar);
#elif defined(ISCALAR)
  setscalar(src, i);
#elif defined(GSCALAR)
  TS *d_src = (TS *)p.src;
  src = d_src[0];
#elif defined(NONSCALAR)
  TS *d_src = (TS *)p.src;
  src = d_src[x];
#endif
  /* perform subsasgn */
  TD *d_dst = (TD *)p.dst;
  if (x_ < p.ndst)
    d_dst[x_] = src;
#undef GUARDED
#undef SCALAR
#undef ISCALAR
#undef GSCALAR
#undef NONSCALAR
}
// indexed subsasgn, compute scalar from iterator
void _subs_si(unsigned x, param_t p, float i, unsigned id) {
#define ISCALAR
  /* determine output index (and if bailing) */
#ifdef GUARDED
  unsigned x_ = x;
  if (!bool_x2x_(&x_, p.base, p.d1, p.d2, p.d3, i, id))
    return; /* bail */
#else
  const unsigned x_ = x2x_n(x, p.base, p.d1, p.d2, p.d3, i, id, p.c_meta);
#endif
  /* pull in B */
  td src;
#if defined(SCALAR)
  setcplx(src, p.scalar);
#elif defined(ISCALAR)
  setscalar(src, i);
#elif defined(GSCALAR)
  TS *d_src = (TS *)p.src;
  src = d_src[0];
#elif defined(NONSCALAR)
  TS *d_src = (TS *)p.src;
  src = d_src[x];
#endif
  /* perform subsasgn */
  TD *d_dst = (TD *)p.dst;
  if (x_ < p.ndst)
    d_dst[x_] = src;
#undef GUARDED
#undef SCALAR
#undef ISCALAR
#undef GSCALAR
#undef NONSCALAR
}
// indexed subsasgn, nonscalar B
void _subs_g(unsigned x, param_t p, float i, unsigned id) {
#define NONSCALAR
  /* determine output index (and if bailing) */
#ifdef GUARDED
  unsigned x_ = x;
  if (!bool_x2x_(&x_, p.base, p.d1, p.d2, p.d3, i, id))
    return; /* bail */
#else
  const unsigned x_ = x2x_n(x, p.base, p.d1, p.d2, p.d3, i, id, p.c_meta);
#endif
  /* pull in B */
  td src;
#if defined(SCALAR)
  setcplx(src, p.scalar);
#elif defined(ISCALAR)
  setscalar(src, i);
#elif defined(GSCALAR)
  TS *d_src = (TS *)p.src;
  src = d_src[0];
#elif defined(NONSCALAR)
  TS *d_src = (TS *)p.src;
  src = d_src[x];
#endif
  /* perform subsasgn */
  TD *d_dst = (TD *)p.dst;
  if (x_ < p.ndst)
    d_dst[x_] = src;
#undef GUARDED
#undef SCALAR
#undef ISCALAR
#undef GSCALAR
#undef NONSCALAR
}
//---- driver kernels (handle gfor/non) ----//
void subs_s(param_t p) {
  unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  if (x >= p.ngfor * p.nsrc)
    return;
  unsigned id = x / p.nsrc;
  float i = id * p.inc + p.start; /* actual iterator value */
  x -= id * p.nsrc;               /* adjust index */
/* adjust and replace device pointers if necessary */
#ifdef GFOR_A
  p.dst = (TD *)p.dst + id * p.ndst;
#endif
#ifdef GFOR_B
#if defined(GSCALAR)
  p.src = (TS *)p.src + id;
#elif defined(NONSCALAR)
  p.src = (TS *)p.src + id * p.nsrc;
#endif
#endif
#undef GSCALAR
#undef ISCALAR
#undef NONSCALAR
  _subs_s(x, p, i, id);
}
void subs_sg(param_t p) {
  unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  if (x >= p.ngfor * p.nsrc)
    return;
  unsigned id = x / p.nsrc;
  float i = id * p.inc + p.start; /* actual iterator value */
  x -= id * p.nsrc;               /* adjust index */
/* adjust and replace device pointers if necessary */
#ifdef GFOR_A
  p.dst = (TD *)p.dst + id * p.ndst;
#endif
#ifdef GFOR_B
#if defined(GSCALAR)
  p.src = (TS *)p.src + id;
#elif defined(NONSCALAR)
  p.src = (TS *)p.src + id * p.nsrc;
#endif
#endif
#undef GSCALAR
#undef ISCALAR
#undef NONSCALAR
  _subs_sg(x, p, i, id);
}
void subs_si(param_t p) {
  unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  if (x >= p.ngfor * p.nsrc)
    return;
  unsigned id = x / p.nsrc;
  float i = id * p.inc + p.start; /* actual iterator value */
  x -= id * p.nsrc;               /* adjust index */
/* adjust and replace device pointers if necessary */
#ifdef GFOR_A
  p.dst = (TD *)p.dst + id * p.ndst;
#endif
#ifdef GFOR_B
#if defined(GSCALAR)
  p.src = (TS *)p.src + id;
#elif defined(NONSCALAR)
  p.src = (TS *)p.src + id * p.nsrc;
#endif
#endif
#undef GSCALAR
#undef ISCALAR
#undef NONSCALAR
  _subs_si(x, p, i, id);
}
void subs_g(param_t p) {
  unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  if (x >= p.ngfor * p.nsrc)
    return;
  unsigned id = x / p.nsrc;
  float i = id * p.inc + p.start; /* actual iterator value */
  x -= id * p.nsrc;               /* adjust index */
/* adjust and replace device pointers if necessary */
#ifdef GFOR_A
  p.dst = (TD *)p.dst + id * p.ndst;
#endif
#ifdef GFOR_B
#if defined(GSCALAR)
  p.src = (TS *)p.src + id;
#elif defined(NONSCALAR)
  p.src = (TS *)p.src + id * p.nsrc;
#endif
#endif
#undef GSCALAR
#undef ISCALAR
#undef NONSCALAR
  _subs_g(x, p, i, id);
}
// gfor A
#define GFOR_A
void subs_s_A(param_t p) {
  unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  if (x >= p.ngfor * p.nsrc)
    return;
  unsigned id = x / p.nsrc;
  float i = id * p.inc + p.start; /* actual iterator value */
  x -= id * p.nsrc;               /* adjust index */
/* adjust and replace device pointers if necessary */
#ifdef GFOR_A
  p.dst = (TD *)p.dst + id * p.ndst;
#endif
#ifdef GFOR_B
#if defined(GSCALAR)
  p.src = (TS *)p.src + id;
#elif defined(NONSCALAR)
  p.src = (TS *)p.src + id * p.nsrc;
#endif
#endif
#undef GSCALAR
#undef ISCALAR
#undef NONSCALAR
  _subs_s(x, p, i, id);
}
void subs_sg_A(param_t p) {
  unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  if (x >= p.ngfor * p.nsrc)
    return;
  unsigned id = x / p.nsrc;
  float i = id * p.inc + p.start; /* actual iterator value */
  x -= id * p.nsrc;               /* adjust index */
/* adjust and replace device pointers if necessary */
#ifdef GFOR_A
  p.dst = (TD *)p.dst + id * p.ndst;
#endif
#ifdef GFOR_B
#if defined(GSCALAR)
  p.src = (TS *)p.src + id;
#elif defined(NONSCALAR)
  p.src = (TS *)p.src + id * p.nsrc;
#endif
#endif
#undef GSCALAR
#undef ISCALAR
#undef NONSCALAR
  _subs_sg(x, p, i, id);
}
void subs_si_A(param_t p) {
  unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  if (x >= p.ngfor * p.nsrc)
    return;
  unsigned id = x / p.nsrc;
  float i = id * p.inc + p.start; /* actual iterator value */
  x -= id * p.nsrc;               /* adjust index */
/* adjust and replace device pointers if necessary */
#ifdef GFOR_A
  p.dst = (TD *)p.dst + id * p.ndst;
#endif
#ifdef GFOR_B
#if defined(GSCALAR)
  p.src = (TS *)p.src + id;
#elif defined(NONSCALAR)
  p.src = (TS *)p.src + id * p.nsrc;
#endif
#endif
#undef GSCALAR
#undef ISCALAR
#undef NONSCALAR
  _subs_si(x, p, i, id);
}
void subs_g_A(param_t p) {
  unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  if (x >= p.ngfor * p.nsrc)
    return;
  unsigned id = x / p.nsrc;
  float i = id * p.inc + p.start; /* actual iterator value */
  x -= id * p.nsrc;               /* adjust index */
/* adjust and replace device pointers if necessary */
#ifdef GFOR_A
  p.dst = (TD *)p.dst + id * p.ndst;
#endif
#ifdef GFOR_B
#if defined(GSCALAR)
  p.src = (TS *)p.src + id;
#elif defined(NONSCALAR)
  p.src = (TS *)p.src + id * p.nsrc;
#endif
#endif
#undef GSCALAR
#undef ISCALAR
#undef NONSCALAR
  _subs_g(x, p, i, id);
}
#undef GFOR_A
// gfor B
#define GFOR_B
void subs_sg_B(param_t p) {
#define GSCALAR
  unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  if (x >= p.ngfor * p.nsrc)
    return;
  unsigned id = x / p.nsrc;
  float i = id * p.inc + p.start; /* actual iterator value */
  x -= id * p.nsrc;               /* adjust index */
/* adjust and replace device pointers if necessary */
#ifdef GFOR_A
  p.dst = (TD *)p.dst + id * p.ndst;
#endif
#ifdef GFOR_B
#if defined(GSCALAR)
  p.src = (TS *)p.src + id;
#elif defined(NONSCALAR)
  p.src = (TS *)p.src + id * p.nsrc;
#endif
#endif
#undef GSCALAR
#undef ISCALAR
#undef NONSCALAR
  _subs_sg(x, p, i, id);
}
void subs_g_B(param_t p) {
#define NONSCALAR
  unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  if (x >= p.ngfor * p.nsrc)
    return;
  unsigned id = x / p.nsrc;
  float i = id * p.inc + p.start; /* actual iterator value */
  x -= id * p.nsrc;               /* adjust index */
/* adjust and replace device pointers if necessary */
#ifdef GFOR_A
  p.dst = (TD *)p.dst + id * p.ndst;
#endif
#ifdef GFOR_B
#if defined(GSCALAR)
  p.src = (TS *)p.src + id;
#elif defined(NONSCALAR)
  p.src = (TS *)p.src + id * p.nsrc;
#endif
#endif
#undef GSCALAR
#undef ISCALAR
#undef NONSCALAR
  _subs_g(x, p, i, id);
}
#undef GFOR_B
// gfor A, gfor B
#define GFOR_A
#define GFOR_B
void subs_sg_AB(param_t p) {
#define GSCALAR
  unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  if (x >= p.ngfor * p.nsrc)
    return;
  unsigned id = x / p.nsrc;
  float i = id * p.inc + p.start; /* actual iterator value */
  x -= id * p.nsrc;               /* adjust index */
/* adjust and replace device pointers if necessary */
#ifdef GFOR_A
  p.dst = (TD *)p.dst + id * p.ndst;
#endif
#ifdef GFOR_B
#if defined(GSCALAR)
  p.src = (TS *)p.src + id;
#elif defined(NONSCALAR)
  p.src = (TS *)p.src + id * p.nsrc;
#endif
#endif
#undef GSCALAR
#undef ISCALAR
#undef NONSCALAR
  _subs_sg(x, p, i, id);
}
void subs_g_AB(param_t p) {
#define NONSCALAR
  unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  if (x >= p.ngfor * p.nsrc)
    return;
  unsigned id = x / p.nsrc;
  float i = id * p.inc + p.start; /* actual iterator value */
  x -= id * p.nsrc;               /* adjust index */
/* adjust and replace device pointers if necessary */
#ifdef GFOR_A
  p.dst = (TD *)p.dst + id * p.ndst;
#endif
#ifdef GFOR_B
#if defined(GSCALAR)
  p.src = (TS *)p.src + id;
#elif defined(NONSCALAR)
  p.src = (TS *)p.src + id * p.nsrc;
#endif
#endif
#undef GSCALAR
#undef ISCALAR
#undef NONSCALAR
  _subs_g(x, p, i, id);
}
#undef GFOR_A
#undef GFOR_B
__kernel void setscalar_(unsigned n, __global ty *d_dst, unsigned inc, ty src) {
  const unsigned x = (GDMY * BIDX + BIDY) * BDMX + TIDX;
  if (x < n)
    d_dst[inc * x] = src;
}
__kernel void subs_s_main(__global td *dst, __global ts *src, unsigned ndst,
                          unsigned nsrc, unsigned ngfor, float start, float inc,
                          const __global void *d1, const __global void *d2,
                          const __global void *d3, float2 scalar, unsigned base,
                          const __global unsigned *c_meta) {
  param_t p = {dst, src, ndst, nsrc,   ngfor, start, inc,
               d1,  d2,  d3,   scalar, base,  c_meta};
#if defined(KER_0)
  return subs_s(p);
#elif defined(KER_1)
  return subs_s_A(p);
#endif
}
__kernel void subs_g_main(__global td *dst, __global ts *src, unsigned ndst,
                          unsigned nsrc, unsigned ngfor, float start, float inc,
                          const __global void *d1, const __global void *d2,
                          const __global void *d3, float2 scalar, unsigned base,
                          const __global unsigned *c_meta) {
  param_t p = {dst, src, ndst, nsrc,   ngfor, start, inc,
               d1,  d2,  d3,   scalar, base,  c_meta};
#if defined(KER_0)
  return subs_g(p);
#elif defined(KER_1)
  return subs_g_A(p);
#elif defined(KER_2)
  return subs_g_B(p);
#elif defined(KER_3)
  return subs_g_AB(p);
#endif
}
__kernel void subs_sg_main(__global td *dst, __global ts *src, unsigned ndst,
                           unsigned nsrc, unsigned ngfor, float start,
                           float inc, const __global void *d1,
                           const __global void *d2, const __global void *d3,
                           float2 scalar, unsigned base,
                           const __global unsigned *c_meta) {
  param_t p = {dst, src, ndst, nsrc,   ngfor, start, inc,
               d1,  d2,  d3,   scalar, base,  c_meta};
#if defined(KER_0)
  return subs_sg(p);
#elif defined(KER_1)
  return subs_sg_A(p);
#elif defined(KER_2)
  return subs_sg_B(p);
#elif defined(KER_3)
  return subs_sg_AB(p);
#elif defined(KER_4)
  return subs_si(p);
#elif defined(KER_5)
  return subs_si_A(p);
#endif
}

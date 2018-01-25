#pragma OPENCL EXTENSION cl_khr_fp16 : enable

#define __truncXfYf2__ truncdfhf2

#define src_rep_t ulong
#define SRC_REP_C(x) (x##UL)
#define srcSigBits 52

#define dst_rep_t ushort
#define DST_REP_C(x) (x)
#define dstSigBits 10

#include "fp_trunc_impl.inc"

#undef __truncXfYf2__

#undef src_rep_t
#undef SRC_REP_C
#undef srcSigBits

#undef dst_rep_t
#undef DST_REP_C
#undef dstSigBits

half convert_halfd(double d) {
  return as_half(truncdfhf2(as_ulong(d)));
}




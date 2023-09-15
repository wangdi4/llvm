// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __NEAT_WRAP_H__
#define __NEAT_WRAP_H__

#include "ImagesALU.h"
#include "NEATValue.h"
#include "NEATVector.h"
#include <vector>

namespace Validation {
enum CmpType {
  CMP_FALSE,
  CMP_OEQ,
  CMP_OGT,
  CMP_OGE,
  CMP_OLT,
  CMP_OLE,
  CMP_ONE,
  CMP_ORD,
  CMP_UEQ,
  CMP_UGT,
  CMP_UGE,
  CMP_ULT,
  CMP_ULE,
  CMP_UNE,
  CMP_UNO,
  CMP_TRUE
};

class NEAT_WRAP {

  /// @brief default ctor
  NEAT_WRAP();

public:
  static NEATValue ToFloat_f(uint64_t intVal);
  static NEATValue ToFloat_f(int64_t intVal);
  static NEATValue ToFloat_d(uint64_t intVal);
  static NEATValue ToFloat_d(int64_t intVal);

  static NEATVector ToFloat_f(std::vector<uint64_t> intVals);
  static NEATVector ToFloat_f(std::vector<int64_t> intVals);
  static NEATVector ToFloat_d(std::vector<uint64_t> intVals);
  static NEATVector ToFloat_d(std::vector<int64_t> intVals);

  static NEATValue vload_half(size_t offset, const uint16_t *p);
  static NEATVector vload_half_2_true(size_t offset, const uint16_t *p);
  static NEATVector vload_half_3_true(size_t offset, const uint16_t *p);
  static NEATVector vload_half_4_true(size_t offset, const uint16_t *p);
  static NEATVector vload_half_8_true(size_t offset, const uint16_t *p);
  static NEATVector vload_half_16_true(size_t offset, const uint16_t *p);
  static NEATVector vload_half_2_false(size_t offset, const uint16_t *p);
  static NEATVector vload_half_3_false(size_t offset, const uint16_t *p);
  static NEATVector vload_half_4_false(size_t offset, const uint16_t *p);
  static NEATVector vload_half_8_false(size_t offset, const uint16_t *p);
  static NEATVector vload_half_16_false(size_t offset, const uint16_t *p);

  static NEATVector vload2_f(size_t offset, const NEATValue *p);
  static NEATVector vload3_f(size_t offset, const NEATValue *p);
  static NEATVector vload4_f(size_t offset, const NEATValue *p);
  static NEATVector vload8_f(size_t offset, const NEATValue *p);
  static NEATVector vload16_f(size_t offset, const NEATValue *p);
  static NEATVector vload2_d(size_t offset, const NEATValue *p);
  static NEATVector vload3_d(size_t offset, const NEATValue *p);
  static NEATVector vload4_d(size_t offset, const NEATValue *p);
  static NEATVector vload8_d(size_t offset, const NEATValue *p);
  static NEATVector vload16_d(size_t offset, const NEATValue *p);

  static void vstore2_f(NEATVector data, size_t offset, const NEATValue *p);
  static void vstore3_f(NEATVector data, size_t offset, const NEATValue *p);
  static void vstore4_f(NEATVector data, size_t offset, const NEATValue *p);
  static void vstore8_f(NEATVector data, size_t offset, const NEATValue *p);
  static void vstore16_f(NEATVector data, size_t offset, const NEATValue *p);
  static void vstore2_d(NEATVector data, size_t offset, const NEATValue *p);
  static void vstore3_d(NEATVector data, size_t offset, const NEATValue *p);
  static void vstore4_d(NEATVector data, size_t offset, const NEATValue *p);
  static void vstore8_d(NEATVector data, size_t offset, const NEATValue *p);
  static void vstore16_d(NEATVector data, size_t offset, const NEATValue *p);

#define VSTORE_HALF_RT_1(__str)                                                \
  static void vstore##__str##_f(const NEATValue &data, size_t offset,          \
                                uint16_t *p);                                  \
  static void vstore##__str##_d(const NEATValue &data, size_t offset,          \
                                uint16_t *p);

  VSTORE_HALF_RT_1(_half)
  VSTORE_HALF_RT_1(a_half)
  VSTORE_HALF_RT_1(_half_rte)
  VSTORE_HALF_RT_1(_half_rtz)
  VSTORE_HALF_RT_1(_half_rtp)
  VSTORE_HALF_RT_1(_half_rtn)
  VSTORE_HALF_RT_1(a_half_rte)
  VSTORE_HALF_RT_1(a_half_rtz)
  VSTORE_HALF_RT_1(a_half_rtp)
  VSTORE_HALF_RT_1(a_half_rtn)
#undef VSTORE_HALF_RT_1

#define VSTORE_HALF_RT_N(__str, n)                                             \
  static void vstore##__str##_##n##_f(const NEATVector &data, size_t offset,   \
                                      uint16_t *p);                            \
  static void vstore##__str##_##n##_d(const NEATVector &data, size_t offset,   \
                                      uint16_t *p);

  VSTORE_HALF_RT_N(_half, 2)
  VSTORE_HALF_RT_N(a_half, 2)
  VSTORE_HALF_RT_N(_half_rte, 2)
  VSTORE_HALF_RT_N(_half_rtz, 2)
  VSTORE_HALF_RT_N(_half_rtp, 2)
  VSTORE_HALF_RT_N(_half_rtn, 2)
  VSTORE_HALF_RT_N(a_half_rte, 2)
  VSTORE_HALF_RT_N(a_half_rtz, 2)
  VSTORE_HALF_RT_N(a_half_rtp, 2)
  VSTORE_HALF_RT_N(a_half_rtn, 2)
  VSTORE_HALF_RT_N(_half, 3)
  VSTORE_HALF_RT_N(a_half, 3)
  VSTORE_HALF_RT_N(_half_rte, 3)
  VSTORE_HALF_RT_N(_half_rtz, 3)
  VSTORE_HALF_RT_N(_half_rtp, 3)
  VSTORE_HALF_RT_N(_half_rtn, 3)
  VSTORE_HALF_RT_N(a_half_rte, 3)
  VSTORE_HALF_RT_N(a_half_rtz, 3)
  VSTORE_HALF_RT_N(a_half_rtp, 3)
  VSTORE_HALF_RT_N(a_half_rtn, 3)
  VSTORE_HALF_RT_N(_half, 4)
  VSTORE_HALF_RT_N(a_half, 4)
  VSTORE_HALF_RT_N(_half_rte, 4)
  VSTORE_HALF_RT_N(_half_rtz, 4)
  VSTORE_HALF_RT_N(_half_rtp, 4)
  VSTORE_HALF_RT_N(_half_rtn, 4)
  VSTORE_HALF_RT_N(a_half_rte, 4)
  VSTORE_HALF_RT_N(a_half_rtz, 4)
  VSTORE_HALF_RT_N(a_half_rtp, 4)
  VSTORE_HALF_RT_N(a_half_rtn, 4)
  VSTORE_HALF_RT_N(_half, 8)
  VSTORE_HALF_RT_N(a_half, 8)
  VSTORE_HALF_RT_N(_half_rte, 8)
  VSTORE_HALF_RT_N(_half_rtz, 8)
  VSTORE_HALF_RT_N(_half_rtp, 8)
  VSTORE_HALF_RT_N(_half_rtn, 8)
  VSTORE_HALF_RT_N(a_half_rte, 8)
  VSTORE_HALF_RT_N(a_half_rtz, 8)
  VSTORE_HALF_RT_N(a_half_rtp, 8)
  VSTORE_HALF_RT_N(a_half_rtn, 8)
  VSTORE_HALF_RT_N(_half, 16)
  VSTORE_HALF_RT_N(a_half, 16)
  VSTORE_HALF_RT_N(_half_rte, 16)
  VSTORE_HALF_RT_N(_half_rtz, 16)
  VSTORE_HALF_RT_N(_half_rtp, 16)
  VSTORE_HALF_RT_N(_half_rtn, 16)
  VSTORE_HALF_RT_N(a_half_rte, 16)
  VSTORE_HALF_RT_N(a_half_rtz, 16)
  VSTORE_HALF_RT_N(a_half_rtp, 16)
  VSTORE_HALF_RT_N(a_half_rtn, 16)
#undef VSTORE_HALF_RT_N

#define NEAT_ONEARG_H(_fname)                                                  \
  static NEATValue _fname##_f(const NEATValue &a);                             \
  static NEATValue _fname##_d(const NEATValue &a);                             \
  static NEATVector _fname##_f(const NEATVector &a);                           \
  static NEATVector _fname##_d(const NEATVector &a);

#define NEAT_ONEARG_FRM_H(_fname)                                              \
  static NEATValue _fname##_frm_f(const NEATValue &a);                         \
  static NEATVector _fname##_frm_f(const NEATVector &a);

#define NEAT_TWOARG_H(_fname)                                                  \
  static NEATValue _fname##_f(const NEATValue &a, const NEATValue &b);         \
  static NEATValue _fname##_d(const NEATValue &a, const NEATValue &b);         \
  static NEATVector _fname##_f(const NEATVector &a, const NEATVector &b);      \
  static NEATVector _fname##_d(const NEATVector &a, const NEATVector &b);

#define NEAT_TWOARG_FRM_H(_fname)                                              \
  static NEATValue _fname##_frm_f(const NEATValue &a, const NEATValue &b);     \
  static NEATVector _fname##_frm_f(const NEATVector &a, const NEATVector &b);

#define NEAT_THREEARG_H(_fname)                                                \
  static NEATValue _fname##_f(const NEATValue &a, const NEATValue &b,          \
                              const NEATValue &c);                             \
  static NEATValue _fname##_d(const NEATValue &a, const NEATValue &b,          \
                              const NEATValue &c);                             \
  static NEATVector _fname##_f(const NEATVector &a, const NEATVector &b,       \
                               const NEATVector &c);                           \
  static NEATVector _fname##_d(const NEATVector &a, const NEATVector &b,       \
                               const NEATVector &c);

  NEAT_TWOARG_H(add)
  NEAT_TWOARG_H(mul)
  NEAT_TWOARG_H(mul_fma)
  NEAT_TWOARG_H(div)
  NEAT_TWOARG_FRM_H(div)
  NEAT_TWOARG_H(native_divide)
  NEAT_TWOARG_H(half_divide)
  NEAT_ONEARG_H(native_recip)
  NEAT_ONEARG_H(half_recip)

  NEAT_TWOARG_H(fmod)
  NEAT_TWOARG_H(remainder)

  static NEATValue remquo_f(const NEATValue &a, const NEATValue &b,
                            int32_t *quo);
  static NEATValue remquo_d(const NEATValue &a, const NEATValue &b,
                            int32_t *quo);
  static NEATVector remquo_f(const NEATVector &a, const NEATVector &b,
                             std::vector<int32_t> &quo);
  static NEATVector remquo_d(const NEATVector &a, const NEATVector &b,
                             std::vector<int32_t> &quo);

  static NEATValue fcmp_f(const NEATValue &a, const NEATValue &b,
                          Validation::CmpType comparison);
  static NEATValue fcmp_d(const NEATValue &a, const NEATValue &b,
                          Validation::CmpType comparison);
  static NEATVector fcmp_f(const NEATVector &a, const NEATVector &b,
                           Validation::CmpType comparison);
  static NEATVector fcmp_d(const NEATVector &a, const NEATVector &b,
                           Validation::CmpType comparison);

  NEAT_TWOARG_H(ISequal)
  NEAT_TWOARG_H(ISnotequal)
  NEAT_TWOARG_H(ISgreater)
  NEAT_TWOARG_H(ISgreaterequal)
  NEAT_TWOARG_H(ISless)
  NEAT_TWOARG_H(ISlessequal)
  NEAT_TWOARG_H(ISlessgreater)
  NEAT_ONEARG_H(ISfinite)
  NEAT_ONEARG_H(ISinf)
  NEAT_ONEARG_H(ISnan)
  NEAT_ONEARG_H(ISnormal)
  NEAT_TWOARG_H(ISordered)
  NEAT_TWOARG_H(ISunordered)
  NEAT_ONEARG_H(signbit)

  NEAT_THREEARG_H(bitselect)

  NEAT_TWOARG_H(sub)
  NEAT_ONEARG_H(sin)
  NEAT_ONEARG_FRM_H(sin)
  NEAT_ONEARG_H(native_sin)
  NEAT_ONEARG_H(half_sin)
  NEAT_ONEARG_H(cos)
  NEAT_ONEARG_FRM_H(cos)
  NEAT_ONEARG_H(native_cos)
  NEAT_ONEARG_H(half_cos)

  static NEATValue sincos_f(const NEATValue &a, NEATValue *y);
  static NEATVector sincos_f(const NEATVector &vec1, NEATVector &vec2);
  static NEATValue sincos_d(const NEATValue &a, NEATValue *y);
  static NEATVector sincos_d(const NEATVector &vec1, NEATVector &vec2);

  static NEATValue sincos_frm_f(const NEATValue &a, NEATValue *y);
  static NEATVector sincos_frm_f(const NEATVector &vec1, NEATVector &vec2);

  NEAT_ONEARG_H(tan)
  NEAT_ONEARG_H(native_tan)
  NEAT_ONEARG_H(half_tan)
  NEAT_ONEARG_FRM_H(tan)
  NEAT_ONEARG_H(asin)
  NEAT_ONEARG_H(acos)
  NEAT_ONEARG_H(atan)
  NEAT_ONEARG_H(sinh)
  NEAT_ONEARG_H(cosh)
  NEAT_ONEARG_H(tanh)
  NEAT_ONEARG_H(asinpi)
  NEAT_ONEARG_H(acospi)
  NEAT_ONEARG_H(atanpi)
  NEAT_ONEARG_H(sinpi)
  NEAT_ONEARG_H(cospi)
  NEAT_ONEARG_H(tanpi)

  NEAT_TWOARG_H(atan2)
  NEAT_TWOARG_H(atan2pi)
  NEAT_ONEARG_H(asinh)
  NEAT_ONEARG_H(acosh)
  NEAT_ONEARG_H(atanh)

  NEAT_ONEARG_H(native_sqrt)
  NEAT_ONEARG_H(sqrt)
  NEAT_ONEARG_H(half_sqrt)
  NEAT_ONEARG_H(rsqrt)
  NEAT_ONEARG_H(native_rsqrt)
  NEAT_ONEARG_H(half_rsqrt)

  NEAT_ONEARG_H(lgamma)

  static NEATValue lgamma_r_f(const NEATValue &a, int32_t *signp);
  static NEATValue lgamma_r_d(const NEATValue &a, int32_t *signp);
  static NEATVector lgamma_r_f(const NEATVector &vec,
                               std::vector<int32_t> &signp);
  static NEATVector lgamma_r_d(const NEATVector &vec,
                               std::vector<int32_t> &signp);

  NEAT_THREEARG_H(mad)

  static NEATValue extractelement_fd(const NEATVector &vec,
                                     const uint32_t &idx);
  static NEATVector insertelement_fd(NEATVector vec, const NEATValue elt,
                                     const uint32_t &idx);
  static NEATVector shufflevector_fd(const NEATVector &vec1,
                                     const NEATVector &vec2,
                                     const std::vector<uint32_t> &mask);

  static NEATVector shuffle_fd(const NEATVector &vec1,
                               const std::vector<uint32_t> &mask);
  static NEATVector shuffle2_fd(const NEATVector &vec1, const NEATVector &vec2,
                                const std::vector<uint32_t> &mask);

  static NEATValue atomic_xchg_fd(NEATValue *p, const NEATValue &val);

  static NEATValue fpext_f2d(const NEATValue &a);
  static NEATVector bitcast_d2f(const NEATValue &a);
  static NEATVector bitcast_d2f(const NEATVector &a);
  static NEATVector bitcast_f2d_vec(const NEATVector &a);
  static NEATValue bitcast_f2d_val(const NEATVector &a);

  static NEATValue fptrunc_d2f(const NEATValue &a);

  NEAT_ONEARG_H(ceil)

  NEAT_THREEARG_H(clamp)
  static NEATVector clamp_f(const NEATVector &a, const NEATValue &in_min,
                            const NEATValue &in_max);
  static NEATVector clamp_d(const NEATVector &a, const NEATValue &in_min,
                            const NEATValue &in_max);

  static NEATValue frexp_f(const NEATValue &a, int *b);
  static NEATValue frexp_d(const NEATValue &a, int *b);
  static NEATVector frexp_f(const NEATVector &vec, std::vector<int> &b);
  static NEATVector frexp_d(const NEATVector &vec, std::vector<int> &b);

  static NEATValue ldexp_f(const NEATValue &a, const int &n);
  static NEATVector ldexp_f(const NEATVector &a, const int &n);
  static NEATVector ldexp_f(const NEATVector &vec, std::vector<int> &n);
  static NEATValue ldexp_d(const NEATValue &a, const int &n);
  static NEATVector ldexp_d(const NEATVector &a, const int &n);
  static NEATVector ldexp_d(const NEATVector &vec, std::vector<int> &n);

  static NEATValue modf_f(const NEATValue &a, NEATValue *n);
  static NEATValue modf_d(const NEATValue &a, NEATValue *n);
  static NEATVector modf_f(const NEATVector &a, NEATVector &n);
  static NEATVector modf_d(const NEATVector &a, NEATVector &n);

  static NEATValue rootn_f(const NEATValue &a, const int &n);
  static NEATVector rootn_f(const NEATVector &a, const std::vector<int> &n);
  static NEATValue rootn_d(const NEATValue &a, const int &n);
  static NEATVector rootn_d(const NEATVector &a, const std::vector<int> &n);

  NEAT_ONEARG_H(exp)
  NEAT_ONEARG_H(native_exp)
  NEAT_ONEARG_H(half_exp)
  NEAT_ONEARG_FRM_H(exp)
  NEAT_ONEARG_H(exp2)
  NEAT_ONEARG_H(native_exp2)
  NEAT_ONEARG_H(half_exp2)
  NEAT_ONEARG_FRM_H(exp2)
  NEAT_ONEARG_H(exp10)
  NEAT_ONEARG_H(native_exp10)
  NEAT_ONEARG_H(half_exp10)
  NEAT_ONEARG_FRM_H(exp10)
  NEAT_ONEARG_H(expm1)
  NEAT_ONEARG_H(log2)
  NEAT_ONEARG_H(native_log2)
  NEAT_ONEARG_H(half_log2)
  NEAT_ONEARG_FRM_H(log2)
  NEAT_ONEARG_H(log)
  NEAT_ONEARG_H(native_log)
  NEAT_ONEARG_H(half_log)
  NEAT_ONEARG_FRM_H(log)
  NEAT_ONEARG_H(log10)
  NEAT_ONEARG_H(native_log10)
  NEAT_ONEARG_H(half_log10)
  NEAT_ONEARG_H(log1p)
  NEAT_ONEARG_H(logb)
  NEAT_ONEARG_H(cbrt)

  NEAT_TWOARG_H(copysign)
  NEAT_TWOARG_H(fdim)
  NEAT_ONEARG_H(ilogb)

  NEAT_TWOARG_H(pow)
  NEAT_TWOARG_FRM_H(pow)
  NEAT_TWOARG_H(powr)
  NEAT_TWOARG_H(native_powr)
  NEAT_TWOARG_H(half_powr)

  static NEATValue pown_f(const NEATValue &a, const int &n);
  static NEATVector pown_f(const NEATVector &a, const std::vector<int> &n);
  static NEATValue pown_d(const NEATValue &a, const int &n);
  static NEATVector pown_d(const NEATVector &a, const std::vector<int> &n);

  // these functions are used to support LLVM 'select' instruction
  static NEATValue select_fd(const bool &cond, const NEATValue &a,
                             const NEATValue &b);
  static NEATVector select_fd(const bool &cond, const NEATVector &a,
                              const NEATVector &b);
  static NEATVector select_fd(const std::vector<bool> &cond,
                              const NEATVector &a, const NEATVector &b);

  // these functions are used to support OpenCL built-in 'select'
  static NEATValue select_f(const NEATValue &a, const NEATValue &b,
                            const int64_t &c);
  static NEATValue select_d(const NEATValue &a, const NEATValue &b,
                            const int64_t &c);
  static NEATVector select_f(const NEATVector &a, const NEATVector &b,
                             const std::vector<int64_t> &c);
  static NEATVector select_d(const NEATVector &a, const NEATVector &b,
                             const std::vector<int64_t> &c);

  static NEATValue dot_f(const NEATValue &val1, const NEATValue &val2);
  static NEATValue dot_f(const NEATVector &vec1, const NEATVector &vec2);
  static NEATValue dot_d(const NEATValue &val1, const NEATValue &val2);
  static NEATValue dot_d(const NEATVector &vec1, const NEATVector &vec2);

  static NEATVector cross_f(const NEATVector &vec1, const NEATVector &vec2);
  static NEATVector cross_d(const NEATVector &vec1, const NEATVector &vec2);

  static NEATValue length_f(const NEATValue &a);
  static NEATValue length_f(const NEATVector &a);
  static NEATValue length_d(const NEATValue &a);
  static NEATValue length_d(const NEATVector &a);
  static NEATValue fast_length_f(const NEATValue &a);
  static NEATValue fast_length_f(const NEATVector &a);
  static NEATValue fast_length_d(const NEATValue &a);
  static NEATValue fast_length_d(const NEATVector &a);

  static NEATValue distance_f(const NEATValue &a, const NEATValue &b);
  static NEATValue distance_f(const NEATVector &a, const NEATVector &b);
  static NEATValue distance_d(const NEATValue &a, const NEATValue &b);
  static NEATValue distance_d(const NEATVector &a, const NEATVector &b);
  static NEATValue fast_distance_f(const NEATValue &a, const NEATValue &b);
  static NEATValue fast_distance_f(const NEATVector &a, const NEATVector &b);
  static NEATValue fast_distance_d(const NEATValue &a, const NEATValue &b);
  static NEATValue fast_distance_d(const NEATVector &a, const NEATVector &b);

  NEAT_ONEARG_H(normalize)
  NEAT_ONEARG_H(fast_normalize)

  NEAT_THREEARG_H(mix)
  static NEATVector mix_f(const NEATVector &x, const NEATVector &y,
                          const NEATValue &a);
  static NEATVector mix_d(const NEATVector &x, const NEATVector &y,
                          const NEATValue &a);

  NEAT_ONEARG_H(radians)
  NEAT_ONEARG_H(degrees)
  NEAT_ONEARG_H(sign)

  NEAT_TWOARG_H(step)
  static NEATVector step_f(const NEATValue &edge, const NEATVector &x);
  static NEATVector step_d(const NEATValue &edge, const NEATVector &x);

  NEAT_THREEARG_H(smoothstep)
  static NEATVector smoothstep_f(const NEATValue &edge0, const NEATValue &edge1,
                                 const NEATVector &vec);
  static NEATVector smoothstep_d(const NEATValue &edge0, const NEATValue &edge1,
                                 const NEATVector &vec);

  NEAT_TWOARG_H(min)
  static NEATVector min_f(const NEATVector &x, const NEATValue &y);
  static NEATVector min_d(const NEATVector &x, const NEATValue &y);

  NEAT_TWOARG_H(max)
  static NEATVector max_f(const NEATVector &x, const NEATValue &y);
  static NEATVector max_d(const NEATVector &x, const NEATValue &y);

  NEAT_TWOARG_H(fmin)
  static NEATVector fmin_f(const NEATVector &x, const NEATValue &y);
  static NEATVector fmin_d(const NEATVector &x, const NEATValue &y);

  NEAT_TWOARG_H(fmax)
  static NEATVector fmax_f(const NEATVector &x, const NEATValue &y);
  static NEATVector fmax_d(const NEATVector &x, const NEATValue &y);

  NEAT_ONEARG_H(fabs)
  NEAT_ONEARG_H(floor)

  static NEATValue fract_f(const NEATValue &a, NEATValue *n);
  static NEATValue fract_d(const NEATValue &a, NEATValue *n);
  static NEATVector fract_f(const NEATVector &a, NEATVector &n);
  static NEATVector fract_d(const NEATVector &a, NEATVector &n);

  NEAT_TWOARG_H(hypot)
  NEAT_TWOARG_H(nextafter)
  NEAT_TWOARG_H(maxmag)
  NEAT_TWOARG_H(minmag)
  NEAT_THREEARG_H(fma)

  static NEATValue nan_f(const uint32_t &y);
  static NEATValue nan_d(const uint64_t &y);
  static NEATVector nan_f(const std::vector<uint32_t> &vec);
  static NEATVector nan_d(const std::vector<uint64_t> &vec);

  NEAT_ONEARG_H(rint)
  NEAT_ONEARG_H(round)
  NEAT_ONEARG_H(trunc)

  static NEATValue convert_float(int8_t *src);
  static NEATValue convert_float(uint8_t *src);
  static NEATValue convert_float(int16_t *src);
  static NEATValue convert_float(uint16_t *src);
  static NEATValue convert_float(int32_t *src);
  static NEATValue convert_float(uint32_t *src);
  static NEATValue convert_float(int64_t *src);
  static NEATValue convert_float(uint64_t *src);

  static NEATValue convert_double(int8_t *src);
  static NEATValue convert_double(uint8_t *src);
  static NEATValue convert_double(int16_t *src);
  static NEATValue convert_double(uint16_t *src);
  static NEATValue convert_double(int32_t *src);
  static NEATValue convert_double(uint32_t *src);
  static NEATValue convert_double(int64_t *src);
  static NEATValue convert_double(uint64_t *src);

#define CONVERT_FLOAT_N(type)                                                  \
  static NEATVector convert_float_2(type *src);                                \
  static NEATVector convert_float_3(type *src);                                \
  static NEATVector convert_float_4(type *src);                                \
  static NEATVector convert_float_8(type *src);                                \
  static NEATVector convert_float_16(type *src);
  CONVERT_FLOAT_N(int8_t)
  CONVERT_FLOAT_N(uint8_t)
  CONVERT_FLOAT_N(int16_t)
  CONVERT_FLOAT_N(uint16_t)
  CONVERT_FLOAT_N(int32_t)
  CONVERT_FLOAT_N(uint32_t)
  CONVERT_FLOAT_N(int64_t)
  CONVERT_FLOAT_N(uint64_t)

#undef CONVERT_FLOAT_N

#define CONVERT_DOUBLE_N(type)                                                 \
  static NEATVector convert_double_2(type *src);                               \
  static NEATVector convert_double_3(type *src);                               \
  static NEATVector convert_double_4(type *src);                               \
  static NEATVector convert_double_8(type *src);                               \
  static NEATVector convert_double_16(type *src);

  CONVERT_DOUBLE_N(int8_t)
  CONVERT_DOUBLE_N(uint8_t)
  CONVERT_DOUBLE_N(int16_t)
  CONVERT_DOUBLE_N(uint16_t)
  CONVERT_DOUBLE_N(int32_t)
  CONVERT_DOUBLE_N(uint32_t)
  CONVERT_DOUBLE_N(int64_t)
  CONVERT_DOUBLE_N(uint64_t)

#undef CONVERT_DOUBLE_N

  static NEATValue convert_float_d(NEATValue *src);
  static NEATVector convert_float_d_2(NEATValue *src);
  static NEATVector convert_float_d_3(NEATValue *src);
  static NEATVector convert_float_d_4(NEATValue *src);
  static NEATVector convert_float_d_8(NEATValue *src);
  static NEATVector convert_float_d_16(NEATValue *src);

  static NEATValue convert_double_f(NEATValue *src);
  static NEATVector convert_double_f_2(NEATValue *src);
  static NEATVector convert_double_f_3(NEATValue *src);
  static NEATVector convert_double_f_4(NEATValue *src);
  static NEATVector convert_double_f_8(NEATValue *src);
  static NEATVector convert_double_f_16(NEATValue *src);

  static NEATVector read_imagef_src_noneat_f(
      void *imageData, Conformance::image_descriptor *imageInfo, float x,
      float y, float z, Conformance::image_sampler_data *imageSampler);
};
} // namespace Validation

#endif // __NEAT_WRAP_H__

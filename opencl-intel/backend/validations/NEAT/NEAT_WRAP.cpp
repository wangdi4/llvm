// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "NEAT_WRAP.h"
#include "NEATALU.h"

namespace Validation {
NEAT_WRAP::NEAT_WRAP() {}

NEATValue NEAT_WRAP::ToFloat_f(uint64_t intVal) {
  return NEATALU::ToFloat<float, uint64_t>(intVal);
}
NEATValue NEAT_WRAP::ToFloat_f(int64_t intVal) {
  return NEATALU::ToFloat<float, int64_t>(intVal);
}
NEATValue NEAT_WRAP::ToFloat_d(uint64_t intVal) {
  return NEATALU::ToFloat<double, uint64_t>(intVal);
}
NEATValue NEAT_WRAP::ToFloat_d(int64_t intVal) {
  return NEATALU::ToFloat<double, int64_t>(intVal);
}

NEATVector NEAT_WRAP::ToFloat_f(std::vector<uint64_t> intVals) {
  return NEATALU::ToFloat<float, uint64_t>(intVals);
}
NEATVector NEAT_WRAP::ToFloat_f(std::vector<int64_t> intVals) {
  return NEATALU::ToFloat<float, int64_t>(intVals);
}
NEATVector NEAT_WRAP::ToFloat_d(std::vector<uint64_t> intVals) {
  return NEATALU::ToFloat<double, uint64_t>(intVals);
}
NEATVector NEAT_WRAP::ToFloat_d(std::vector<int64_t> intVals) {
  return NEATALU::ToFloat<double, int64_t>(intVals);
}

#define NEAT_ONEARG(_fname)                                                    \
  NEATValue NEAT_WRAP::_fname##_f(const NEATValue &a) {                        \
    return NEATALU::_fname<float>(a);                                          \
  }                                                                            \
  NEATValue NEAT_WRAP::_fname##_d(const NEATValue &a) {                        \
    return NEATALU::_fname<double>(a);                                         \
  }                                                                            \
  NEATVector NEAT_WRAP::_fname##_f(const NEATVector &a) {                      \
    return NEATALU::_fname<float>(a);                                          \
  }                                                                            \
  NEATVector NEAT_WRAP::_fname##_d(const NEATVector &a) {                      \
    return NEATALU::_fname<double>(a);                                         \
  }

#define NEAT_ONEARG_FRM(_fname)                                                \
  NEATValue NEAT_WRAP::_fname##_frm_f(const NEATValue &a) {                    \
    return NEATALU::_fname##_frm<float>(a);                                    \
  }                                                                            \
  NEATVector NEAT_WRAP::_fname##_frm_f(const NEATVector &a) {                  \
    return NEATALU::_fname##_frm<float>(a);                                    \
  }

#define NEAT_TWOARG(_fname)                                                    \
  NEATValue NEAT_WRAP::_fname##_f(const NEATValue &a, const NEATValue &b) {    \
    return NEATALU::_fname<float>(a, b);                                       \
  }                                                                            \
  NEATValue NEAT_WRAP::_fname##_d(const NEATValue &a, const NEATValue &b) {    \
    return NEATALU::_fname<double>(a, b);                                      \
  }                                                                            \
  NEATVector NEAT_WRAP::_fname##_f(const NEATVector &a, const NEATVector &b) { \
    return NEATALU::_fname<float>(a, b);                                       \
  }                                                                            \
  NEATVector NEAT_WRAP::_fname##_d(const NEATVector &a, const NEATVector &b) { \
    return NEATALU::_fname<double>(a, b);                                      \
  }

#define NEAT_TWOARG_FRM(_fname)                                                \
  NEATValue NEAT_WRAP::_fname##_frm_f(const NEATValue &a,                      \
                                      const NEATValue &b) {                    \
    return NEATALU::_fname##_frm<float>(a, b);                                 \
  }                                                                            \
  NEATVector NEAT_WRAP::_fname##_frm_f(const NEATVector &a,                    \
                                       const NEATVector &b) {                  \
    return NEATALU::_fname##_frm<float>(a, b);                                 \
  }

#define NEAT_THREEARG(_fname)                                                  \
  NEATValue NEAT_WRAP::_fname##_f(const NEATValue &a, const NEATValue &b,      \
                                  const NEATValue &c) {                        \
    return NEATALU::_fname<float>(a, b, c);                                    \
  }                                                                            \
  NEATValue NEAT_WRAP::_fname##_d(const NEATValue &a, const NEATValue &b,      \
                                  const NEATValue &c) {                        \
    return NEATALU::_fname<double>(a, b, c);                                   \
  }                                                                            \
  NEATVector NEAT_WRAP::_fname##_f(const NEATVector &a, const NEATVector &b,   \
                                   const NEATVector &c) {                      \
    return NEATALU::_fname<float>(a, b, c);                                    \
  }                                                                            \
  NEATVector NEAT_WRAP::_fname##_d(const NEATVector &a, const NEATVector &b,   \
                                   const NEATVector &c) {                      \
    return NEATALU::_fname<double>(a, b, c);                                   \
  }

NEAT_TWOARG(add)
NEAT_TWOARG(mul)
NEAT_TWOARG(mul_fma)
NEAT_TWOARG(div)
NEAT_TWOARG_FRM(div)
NEAT_TWOARG(native_divide)
NEAT_TWOARG(half_divide)
NEAT_ONEARG(native_recip)
NEAT_ONEARG(half_recip)

NEAT_TWOARG(fmod)
NEAT_TWOARG(remainder)

NEATValue NEAT_WRAP::remquo_f(const NEATValue &a, const NEATValue &b,
                              int32_t *quo) {
  return NEATALU::remquo<float>(a, b, quo);
}
NEATValue NEAT_WRAP::remquo_d(const NEATValue &a, const NEATValue &b,
                              int32_t *quo) {
  return NEATALU::remquo<double>(a, b, quo);
}

NEATVector NEAT_WRAP::remquo_f(const NEATVector &a, const NEATVector &b,
                               std::vector<int32_t> &quo) {
  return NEATALU::remquo<float>(a, b, quo);
}
NEATVector NEAT_WRAP::remquo_d(const NEATVector &a, const NEATVector &b,
                               std::vector<int32_t> &quo) {
  return NEATALU::remquo<double>(a, b, quo);
}

NEATValue NEAT_WRAP::fcmp_f(const NEATValue &a, const NEATValue &b,
                            CmpType comparison) {
  return NEATALU::fcmp<float>(a, b, comparison);
}
NEATValue NEAT_WRAP::fcmp_d(const NEATValue &a, const NEATValue &b,
                            CmpType comparison) {
  return NEATALU::fcmp<double>(a, b, comparison);
}
NEATVector NEAT_WRAP::fcmp_f(const NEATVector &a, const NEATVector &b,
                             CmpType comparison) {
  return NEATALU::fcmp<float>(a, b, comparison);
}
NEATVector NEAT_WRAP::fcmp_d(const NEATVector &a, const NEATVector &b,
                             CmpType comparison) {
  return NEATALU::fcmp<double>(a, b, comparison);
}

NEATValue NEAT_WRAP::vload_half(size_t offset, const uint16_t *p) {
  return NEATALU::vload_half(offset, p);
}
NEATVector NEAT_WRAP::vload_half_2_true(size_t offset, const uint16_t *p) {
  return NEATALU::vload_half<2, true>(offset, p);
}
NEATVector NEAT_WRAP::vload_half_3_true(size_t offset, const uint16_t *p) {
  return NEATALU::vload_half<3, true>(offset, p);
}
NEATVector NEAT_WRAP::vload_half_4_true(size_t offset, const uint16_t *p) {
  return NEATALU::vload_half<4, true>(offset, p);
}
NEATVector NEAT_WRAP::vload_half_8_true(size_t offset, const uint16_t *p) {
  return NEATALU::vload_half<8, true>(offset, p);
}
NEATVector NEAT_WRAP::vload_half_16_true(size_t offset, const uint16_t *p) {
  return NEATALU::vload_half<16, true>(offset, p);
}
NEATVector NEAT_WRAP::vload_half_2_false(size_t offset, const uint16_t *p) {
  return NEATALU::vload_half<2, false>(offset, p);
}
NEATVector NEAT_WRAP::vload_half_3_false(size_t offset, const uint16_t *p) {
  return NEATALU::vload_half<3, false>(offset, p);
}
NEATVector NEAT_WRAP::vload_half_4_false(size_t offset, const uint16_t *p) {
  return NEATALU::vload_half<4, false>(offset, p);
}
NEATVector NEAT_WRAP::vload_half_8_false(size_t offset, const uint16_t *p) {
  return NEATALU::vload_half<8, false>(offset, p);
}
NEATVector NEAT_WRAP::vload_half_16_false(size_t offset, const uint16_t *p) {
  return NEATALU::vload_half<16, false>(offset, p);
}

NEATVector NEAT_WRAP::vload2_f(size_t offset, const NEATValue *p) {
  return NEATALU::vload2<float>(offset, p);
}
NEATVector NEAT_WRAP::vload3_f(size_t offset, const NEATValue *p) {
  return NEATALU::vload3<float>(offset, p);
}
NEATVector NEAT_WRAP::vload4_f(size_t offset, const NEATValue *p) {
  return NEATALU::vload4<float>(offset, p);
}
NEATVector NEAT_WRAP::vload8_f(size_t offset, const NEATValue *p) {
  return NEATALU::vload8<float>(offset, p);
}
NEATVector NEAT_WRAP::vload16_f(size_t offset, const NEATValue *p) {
  return NEATALU::vload16<float>(offset, p);
}
NEATVector NEAT_WRAP::vload2_d(size_t offset, const NEATValue *p) {
  return NEATALU::vload2<double>(offset, p);
}
NEATVector NEAT_WRAP::vload3_d(size_t offset, const NEATValue *p) {
  return NEATALU::vload3<double>(offset, p);
}
NEATVector NEAT_WRAP::vload4_d(size_t offset, const NEATValue *p) {
  return NEATALU::vload4<double>(offset, p);
}
NEATVector NEAT_WRAP::vload8_d(size_t offset, const NEATValue *p) {
  return NEATALU::vload8<double>(offset, p);
}
NEATVector NEAT_WRAP::vload16_d(size_t offset, const NEATValue *p) {
  return NEATALU::vload16<double>(offset, p);
}

void NEAT_WRAP::vstore2_f(NEATVector data, size_t offset, const NEATValue *p) {
  NEATALU::vstore2<float>(data, offset, p);
}
void NEAT_WRAP::vstore3_f(NEATVector data, size_t offset, const NEATValue *p) {
  NEATALU::vstore3<float>(data, offset, p);
}
void NEAT_WRAP::vstore4_f(NEATVector data, size_t offset, const NEATValue *p) {
  NEATALU::vstore4<float>(data, offset, p);
}
void NEAT_WRAP::vstore8_f(NEATVector data, size_t offset, const NEATValue *p) {
  NEATALU::vstore8<float>(data, offset, p);
}
void NEAT_WRAP::vstore16_f(NEATVector data, size_t offset, const NEATValue *p) {
  NEATALU::vstore16<float>(data, offset, p);
}
void NEAT_WRAP::vstore2_d(NEATVector data, size_t offset, const NEATValue *p) {
  NEATALU::vstore2<double>(data, offset, p);
}
void NEAT_WRAP::vstore3_d(NEATVector data, size_t offset, const NEATValue *p) {
  NEATALU::vstore3<double>(data, offset, p);
}
void NEAT_WRAP::vstore4_d(NEATVector data, size_t offset, const NEATValue *p) {
  NEATALU::vstore4<double>(data, offset, p);
}
void NEAT_WRAP::vstore8_d(NEATVector data, size_t offset, const NEATValue *p) {
  NEATALU::vstore8<double>(data, offset, p);
}
void NEAT_WRAP::vstore16_d(NEATVector data, size_t offset, const NEATValue *p) {
  NEATALU::vstore16<double>(data, offset, p);
}

#define VSTORE_HALF_RT_1(__str)                                                \
  void NEAT_WRAP::vstore##__str##_f(const NEATValue &data, size_t offset,      \
                                    uint16_t *p) {                             \
    NEATALU::vstore##__str<float>(data, offset, p);                            \
  }                                                                            \
  void NEAT_WRAP::vstore##__str##_d(const NEATValue &data, size_t offset,      \
                                    uint16_t *p) {                             \
    NEATALU::vstore##__str<double>(data, offset, p);                           \
  }

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
  void NEAT_WRAP::vstore##__str##_##n##_f(const NEATVector &data,              \
                                          size_t offset, uint16_t *p) {        \
    NEATALU::vstore##__str<float, n>(data, offset, p);                         \
  }                                                                            \
  void NEAT_WRAP::vstore##__str##_##n##_d(const NEATVector &data,              \
                                          size_t offset, uint16_t *p) {        \
    NEATALU::vstore##__str<double, n>(data, offset, p);                        \
  }

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

NEAT_TWOARG(ISequal)
NEAT_TWOARG(ISnotequal)
NEAT_TWOARG(ISgreater)
NEAT_TWOARG(ISgreaterequal)
NEAT_TWOARG(ISless)
NEAT_TWOARG(ISlessequal)
NEAT_TWOARG(ISlessgreater)
NEAT_ONEARG(ISfinite)
NEAT_ONEARG(ISinf)
NEAT_ONEARG(ISnan)
NEAT_ONEARG(ISnormal)
NEAT_TWOARG(ISordered)
NEAT_TWOARG(ISunordered)
NEAT_ONEARG(signbit)

NEAT_THREEARG(bitselect)

NEAT_TWOARG(sub)
NEAT_ONEARG(sin)
NEAT_ONEARG(native_sin)
NEAT_ONEARG(half_sin)
NEAT_ONEARG_FRM(sin)
NEAT_ONEARG(cos)
NEAT_ONEARG(native_cos)
NEAT_ONEARG(half_cos)
NEAT_ONEARG_FRM(cos)

NEATValue NEAT_WRAP::sincos_f(const NEATValue &a, NEATValue *y) {
  return NEATALU::sincos<float>(a, y);
}
NEATVector NEAT_WRAP::sincos_f(const NEATVector &vec1, NEATVector &vec2) {
  return NEATALU::sincos<float>(vec1, vec2);
}

NEATValue NEAT_WRAP::sincos_d(const NEATValue &a, NEATValue *y) {
  return NEATALU::sincos<double>(a, y);
}
NEATVector NEAT_WRAP::sincos_d(const NEATVector &vec1, NEATVector &vec2) {
  return NEATALU::sincos<double>(vec1, vec2);
}

NEATValue NEAT_WRAP::sincos_frm_f(const NEATValue &a, NEATValue *y) {
  return NEATALU::sincos_frm<float>(a, y);
}
NEATVector NEAT_WRAP::sincos_frm_f(const NEATVector &vec1, NEATVector &vec2) {
  return NEATALU::sincos_frm<float>(vec1, vec2);
}

NEAT_ONEARG(tan)
NEAT_ONEARG_FRM(tan)
NEAT_ONEARG(native_tan)
NEAT_ONEARG(half_tan)
NEAT_ONEARG(asin)
NEAT_ONEARG(acos)
NEAT_ONEARG(atan)
NEAT_ONEARG(sinh)
NEAT_ONEARG(cosh)
NEAT_ONEARG(tanh)
NEAT_ONEARG(asinpi)
NEAT_ONEARG(acospi)
NEAT_ONEARG(atanpi)
NEAT_ONEARG(sinpi)
NEAT_ONEARG(cospi)
NEAT_ONEARG(tanpi)

NEAT_TWOARG(atan2)
NEAT_TWOARG(atan2pi)
NEAT_ONEARG(asinh)
NEAT_ONEARG(acosh)
NEAT_ONEARG(atanh)

NEAT_ONEARG(native_sqrt)
NEAT_ONEARG(sqrt)
NEAT_ONEARG(half_sqrt)
NEAT_ONEARG(rsqrt)
NEAT_ONEARG(native_rsqrt)
NEAT_ONEARG(half_rsqrt)

NEAT_ONEARG(lgamma)
NEATValue NEAT_WRAP::lgamma_r_f(const NEATValue &a, int32_t *signp) {
  return NEATALU::lgamma_r<float>(a, signp);
}
NEATValue NEAT_WRAP::lgamma_r_d(const NEATValue &a, int32_t *signp) {
  return NEATALU::lgamma_r<double>(a, signp);
}
NEATVector NEAT_WRAP::lgamma_r_f(const NEATVector &vec,
                                 std::vector<int32_t> &signp) {
  return NEATALU::lgamma_r<float>(vec, signp);
}
NEATVector NEAT_WRAP::lgamma_r_d(const NEATVector &vec,
                                 std::vector<int32_t> &signp) {
  return NEATALU::lgamma_r<double>(vec, signp);
}

NEAT_THREEARG(mad)

NEATValue NEAT_WRAP::extractelement_fd(const NEATVector &vec,
                                       const uint32_t &idx) {
  return NEATALU::extractelement(vec, idx);
}
NEATVector NEAT_WRAP::insertelement_fd(NEATVector vec, const NEATValue elt,
                                       const uint32_t &idx) {
  return NEATALU::insertelement(vec, elt, idx);
}
NEATVector NEAT_WRAP::shufflevector_fd(const NEATVector &vec1,
                                       const NEATVector &vec2,
                                       const std::vector<uint32_t> &mask) {
  return NEATALU::shufflevector(vec1, vec2, mask);
}

NEATVector NEAT_WRAP::shuffle_fd(const NEATVector &vec1,
                                 const std::vector<uint32_t> &mask) {
  return NEATALU::shuffle(vec1, mask);
}
NEATVector NEAT_WRAP::shuffle2_fd(const NEATVector &vec1,
                                  const NEATVector &vec2,
                                  const std::vector<uint32_t> &mask) {
  return NEATALU::shuffle2(vec1, vec2, mask);
}

NEATValue NEAT_WRAP::atomic_xchg_fd(NEATValue *p, const NEATValue &val) {
  return NEATALU::atomic_xchg(p, val);
}

NEATValue NEAT_WRAP::fpext_f2d(const NEATValue &a) {
  return NEATALU::fpext<float, double>(a);
}

NEATVector NEAT_WRAP::bitcast_d2f(const NEATValue &a) {
  return NEATALU::bitcast<double, float>(a);
}
NEATVector NEAT_WRAP::bitcast_d2f(const NEATVector &a) {
  return NEATALU::bitcast<double, float>(a);
}

NEATVector NEAT_WRAP::bitcast_f2d_vec(const NEATVector &a) {
  return NEATALU::bitcast<float, double>(a);
}

NEATValue NEAT_WRAP::bitcast_f2d_val(const NEATVector &a) {
  return NEATALU::bitcast<float, double, NEATValue>(a);
}

NEATValue NEAT_WRAP::fptrunc_d2f(const NEATValue &a) {
  return NEATALU::fptrunc<double, float>(a);
}

NEAT_ONEARG(ceil)

NEAT_THREEARG(clamp)
NEATVector NEAT_WRAP::clamp_f(const NEATVector &a, const NEATValue &in_min,
                              const NEATValue &in_max) {
  return NEATALU::clamp<float>(a, in_min, in_max);
}
NEATVector NEAT_WRAP::clamp_d(const NEATVector &a, const NEATValue &in_min,
                              const NEATValue &in_max) {
  return NEATALU::clamp<double>(a, in_min, in_max);
}

NEATValue NEAT_WRAP::frexp_f(const NEATValue &a, int *b) {
  return NEATALU::frexp<float>(a, b);
}
NEATValue NEAT_WRAP::frexp_d(const NEATValue &a, int *b) {
  return NEATALU::frexp<double>(a, b);
}
NEATVector NEAT_WRAP::frexp_f(const NEATVector &a, std::vector<int> &b) {
  return NEATALU::frexp<float>(a, b);
}
NEATVector NEAT_WRAP::frexp_d(const NEATVector &a, std::vector<int> &b) {
  return NEATALU::frexp<double>(a, b);
}

NEATValue NEAT_WRAP::ldexp_f(const NEATValue &a, const int &n) {
  return NEATALU::ldexp<float>(a, n);
}
NEATVector NEAT_WRAP::ldexp_f(const NEATVector &a, const int &n) {
  return NEATALU::ldexp<float>(a, n);
}
NEATVector NEAT_WRAP::ldexp_f(const NEATVector &a, std::vector<int> &n) {
  return NEATALU::ldexp<float>(a, n);
}
NEATValue NEAT_WRAP::ldexp_d(const NEATValue &a, const int &n) {
  return NEATALU::ldexp<double>(a, n);
}
NEATVector NEAT_WRAP::ldexp_d(const NEATVector &a, const int &n) {
  return NEATALU::ldexp<double>(a, n);
}
NEATVector NEAT_WRAP::ldexp_d(const NEATVector &a, std::vector<int> &n) {
  return NEATALU::ldexp<double>(a, n);
}

NEATValue NEAT_WRAP::modf_f(const NEATValue &a, NEATValue *n) {
  return NEATALU::modf<float>(a, n);
}
NEATValue NEAT_WRAP::modf_d(const NEATValue &a, NEATValue *n) {
  return NEATALU::modf<double>(a, n);
}
NEATVector NEAT_WRAP::modf_f(const NEATVector &a, NEATVector &n) {
  return NEATALU::modf<float>(a, n);
}
NEATVector NEAT_WRAP::modf_d(const NEATVector &a, NEATVector &n) {
  return NEATALU::modf<double>(a, n);
}

NEATValue NEAT_WRAP::rootn_f(const NEATValue &a, const int &n) {
  return NEATALU::rootn<float>(a, n);
}
NEATVector NEAT_WRAP::rootn_f(const NEATVector &a, const std::vector<int> &n) {
  return NEATALU::rootn<float>(a, n);
}
NEATValue NEAT_WRAP::rootn_d(const NEATValue &a, const int &n) {
  return NEATALU::rootn<double>(a, n);
}
NEATVector NEAT_WRAP::rootn_d(const NEATVector &a, const std::vector<int> &n) {
  return NEATALU::rootn<double>(a, n);
}

NEAT_ONEARG(exp)
NEAT_ONEARG(native_exp)
NEAT_ONEARG(half_exp)
NEAT_ONEARG_FRM(exp)
NEAT_ONEARG(exp2)
NEAT_ONEARG(native_exp2)
NEAT_ONEARG(half_exp2)
NEAT_ONEARG_FRM(exp2)
NEAT_ONEARG(exp10)
NEAT_ONEARG(native_exp10)
NEAT_ONEARG(half_exp10)
NEAT_ONEARG_FRM(exp10)
NEAT_ONEARG(expm1)
NEAT_ONEARG(log2)
NEAT_ONEARG(native_log2)
NEAT_ONEARG(half_log2)
NEAT_ONEARG_FRM(log2)
NEAT_ONEARG(log)
NEAT_ONEARG(native_log)
NEAT_ONEARG(half_log)
NEAT_ONEARG_FRM(log)
NEAT_ONEARG(log10)
NEAT_ONEARG(native_log10)
NEAT_ONEARG(half_log10)
NEAT_ONEARG(log1p)
NEAT_ONEARG(logb)
NEAT_ONEARG(cbrt)

NEAT_TWOARG(copysign)
NEAT_TWOARG(fdim)
NEAT_ONEARG(ilogb)

NEAT_TWOARG(pow)
NEAT_TWOARG_FRM(pow)
NEAT_TWOARG(powr)
NEAT_TWOARG(native_powr)
NEAT_TWOARG(half_powr)

NEATValue NEAT_WRAP::pown_f(const NEATValue &a, const int &n) {
  return NEATALU::pown<float>(a, n);
}
NEATVector NEAT_WRAP::pown_f(const NEATVector &a, const std::vector<int> &n) {
  return NEATALU::pown<float>(a, n);
}
NEATValue NEAT_WRAP::pown_d(const NEATValue &a, const int &n) {
  return NEATALU::pown<double>(a, n);
}
NEATVector NEAT_WRAP::pown_d(const NEATVector &a, const std::vector<int> &n) {
  return NEATALU::pown<double>(a, n);
}

// these functions are used to support LLVM 'select' instruction
NEATValue NEAT_WRAP::select_fd(const bool &cond, const NEATValue &a,
                               const NEATValue &b) {
  return NEATALU::select(cond, a, b);
}
NEATVector NEAT_WRAP::select_fd(const bool &cond, const NEATVector &a,
                                const NEATVector &b) {
  return NEATALU::select(cond, a, b);
}
NEATVector NEAT_WRAP::select_fd(const std::vector<bool> &cond,
                                const NEATVector &a, const NEATVector &b) {
  return NEATALU::select(cond, a, b);
}

// these functions are used to support OpenCL built-in 'select'
NEATValue NEAT_WRAP::select_f(const NEATValue &a, const NEATValue &b,
                              const int64_t &c) {
  return NEATALU::select<float>(a, b, c);
}
NEATValue NEAT_WRAP::select_d(const NEATValue &a, const NEATValue &b,
                              const int64_t &c) {
  return NEATALU::select<double>(a, b, c);
}
NEATVector NEAT_WRAP::select_f(const NEATVector &a, const NEATVector &b,
                               const std::vector<int64_t> &c) {
  return NEATALU::select<float>(a, b, c);
}
NEATVector NEAT_WRAP::select_d(const NEATVector &a, const NEATVector &b,
                               const std::vector<int64_t> &c) {
  return NEATALU::select<double>(a, b, c);
}

NEATValue NEAT_WRAP::dot_f(const NEATValue &val1, const NEATValue &val2) {
  return NEATALU::dot<float>(val1, val2);
}
NEATValue NEAT_WRAP::dot_f(const NEATVector &vec1, const NEATVector &vec2) {
  return NEATALU::dot<float>(vec1, vec2);
}
NEATValue NEAT_WRAP::dot_d(const NEATValue &val1, const NEATValue &val2) {
  return NEATALU::dot<double>(val1, val2);
}
NEATValue NEAT_WRAP::dot_d(const NEATVector &vec1, const NEATVector &vec2) {
  return NEATALU::dot<double>(vec1, vec2);
}

NEATVector NEAT_WRAP::cross_f(const NEATVector &vec1, const NEATVector &vec2) {
  return NEATALU::cross<float>(vec1, vec2);
}
NEATVector NEAT_WRAP::cross_d(const NEATVector &vec1, const NEATVector &vec2) {
  return NEATALU::cross<double>(vec1, vec2);
}

NEATValue NEAT_WRAP::length_f(const NEATValue &a) {
  return NEATALU::length<float>(a);
}
NEATValue NEAT_WRAP::length_f(const NEATVector &a) {
  return NEATALU::length<float>(a);
}
NEATValue NEAT_WRAP::length_d(const NEATValue &a) {
  return NEATALU::length<double>(a);
}
NEATValue NEAT_WRAP::length_d(const NEATVector &a) {
  return NEATALU::length<double>(a);
}
NEATValue NEAT_WRAP::fast_length_f(const NEATValue &a) {
  return NEATALU::fast_length<float>(a);
}
NEATValue NEAT_WRAP::fast_length_f(const NEATVector &a) {
  return NEATALU::fast_length<float>(a);
}
NEATValue NEAT_WRAP::fast_length_d(const NEATValue &a) {
  return NEATALU::fast_length<double>(a);
}
NEATValue NEAT_WRAP::fast_length_d(const NEATVector &a) {
  return NEATALU::fast_length<double>(a);
}

NEATValue NEAT_WRAP::distance_f(const NEATValue &a, const NEATValue &b) {
  return NEATALU::distance<float>(a, b);
}
NEATValue NEAT_WRAP::distance_f(const NEATVector &a, const NEATVector &b) {
  return NEATALU::distance<float>(a, b);
}
NEATValue NEAT_WRAP::distance_d(const NEATValue &a, const NEATValue &b) {
  return NEATALU::distance<double>(a, b);
}
NEATValue NEAT_WRAP::distance_d(const NEATVector &a, const NEATVector &b) {
  return NEATALU::distance<double>(a, b);
}
NEATValue NEAT_WRAP::fast_distance_f(const NEATValue &a, const NEATValue &b) {
  return NEATALU::fast_distance<float>(a, b);
}
NEATValue NEAT_WRAP::fast_distance_f(const NEATVector &a, const NEATVector &b) {
  return NEATALU::fast_distance<float>(a, b);
}
NEATValue NEAT_WRAP::fast_distance_d(const NEATValue &a, const NEATValue &b) {
  return NEATALU::fast_distance<double>(a, b);
}
NEATValue NEAT_WRAP::fast_distance_d(const NEATVector &a, const NEATVector &b) {
  return NEATALU::fast_distance<double>(a, b);
}

NEAT_ONEARG(normalize)
NEAT_ONEARG(fast_normalize)

NEAT_THREEARG(mix)
NEATVector NEAT_WRAP::mix_f(const NEATVector &x, const NEATVector &y,
                            const NEATValue &a) {
  return NEATALU::mix<float>(x, y, a);
}
NEATVector NEAT_WRAP::mix_d(const NEATVector &x, const NEATVector &y,
                            const NEATValue &a) {
  return NEATALU::mix<double>(x, y, a);
}

NEAT_ONEARG(radians)
NEAT_ONEARG(degrees)
NEAT_ONEARG(sign)

NEAT_TWOARG(step)
NEATVector NEAT_WRAP::step_f(const NEATValue &edge, const NEATVector &x) {
  return NEATALU::step<float>(edge, x);
}
NEATVector NEAT_WRAP::step_d(const NEATValue &edge, const NEATVector &x) {
  return NEATALU::step<double>(edge, x);
}

NEAT_THREEARG(smoothstep)
NEATVector NEAT_WRAP::smoothstep_f(const NEATValue &edge0,
                                   const NEATValue &edge1,
                                   const NEATVector &vec) {
  return NEATALU::smoothstep<float>(edge0, edge1, vec);
}
NEATVector NEAT_WRAP::smoothstep_d(const NEATValue &edge0,
                                   const NEATValue &edge1,
                                   const NEATVector &vec) {
  return NEATALU::smoothstep<double>(edge0, edge1, vec);
}

NEAT_TWOARG(min)
NEATVector NEAT_WRAP::min_f(const NEATVector &x, const NEATValue &y) {
  return NEATALU::min<float>(x, y);
}
NEATVector NEAT_WRAP::min_d(const NEATVector &x, const NEATValue &y) {
  return NEATALU::min<double>(x, y);
}

NEAT_TWOARG(max)
NEATVector NEAT_WRAP::max_f(const NEATVector &x, const NEATValue &y) {
  return NEATALU::max<float>(x, y);
}
NEATVector NEAT_WRAP::max_d(const NEATVector &x, const NEATValue &y) {
  return NEATALU::max<double>(x, y);
}

NEAT_TWOARG(fmin)
NEATVector NEAT_WRAP::fmin_f(const NEATVector &x, const NEATValue &y) {
  return NEATALU::fmin<float>(x, y);
}
NEATVector NEAT_WRAP::fmin_d(const NEATVector &x, const NEATValue &y) {
  return NEATALU::fmin<double>(x, y);
}

NEAT_TWOARG(fmax)
NEATVector NEAT_WRAP::fmax_f(const NEATVector &x, const NEATValue &y) {
  return NEATALU::fmax<float>(x, y);
}
NEATVector NEAT_WRAP::fmax_d(const NEATVector &x, const NEATValue &y) {
  return NEATALU::fmax<double>(x, y);
}

NEAT_ONEARG(fabs)
NEAT_ONEARG(floor)

NEATValue NEAT_WRAP::fract_f(const NEATValue &a, NEATValue *n) {
  return NEATALU::fract<float>(a, n);
}
NEATValue NEAT_WRAP::fract_d(const NEATValue &a, NEATValue *n) {
  return NEATALU::fract<double>(a, n);
}
NEATVector NEAT_WRAP::fract_f(const NEATVector &a, NEATVector &n) {
  return NEATALU::fract<float>(a, n);
}
NEATVector NEAT_WRAP::fract_d(const NEATVector &a, NEATVector &n) {
  return NEATALU::fract<double>(a, n);
}

NEAT_TWOARG(hypot)
NEAT_TWOARG(nextafter)
NEAT_TWOARG(maxmag)
NEAT_TWOARG(minmag)
NEAT_THREEARG(fma)

NEATValue NEAT_WRAP::nan_f(const uint32_t &y) {
  return NEATALU::nan<float, uint32_t>(y);
}
NEATValue NEAT_WRAP::nan_d(const uint64_t &y) {
  return NEATALU::nan<double, uint64_t>(y);
}
NEATVector NEAT_WRAP::nan_f(const std::vector<uint32_t> &vec) {
  return NEATALU::nan<float, uint32_t>(vec);
}
NEATVector NEAT_WRAP::nan_d(const std::vector<uint64_t> &vec) {
  return NEATALU::nan<double, uint64_t>(vec);
}

NEAT_ONEARG(rint)
NEAT_ONEARG(round)
NEAT_ONEARG(trunc)

#define CONVERT_FLOAT(type)                                                    \
  NEATValue NEAT_WRAP::convert_float(type *src) {                              \
    return NEATALU::convert_float<type>(src);                                  \
  }
CONVERT_FLOAT(int8_t)
CONVERT_FLOAT(uint8_t)
CONVERT_FLOAT(int16_t)
CONVERT_FLOAT(uint16_t)
CONVERT_FLOAT(int32_t)
CONVERT_FLOAT(uint32_t)
CONVERT_FLOAT(int64_t)
CONVERT_FLOAT(uint64_t)
#undef CONVERT_FLOAT

#define CONVERT_DOUBLE(type)                                                   \
  NEATValue NEAT_WRAP::convert_double(type *src) {                             \
    return NEATALU::convert_double<type>(src);                                 \
  }
CONVERT_DOUBLE(int8_t)
CONVERT_DOUBLE(uint8_t)
CONVERT_DOUBLE(int16_t)
CONVERT_DOUBLE(uint16_t)
CONVERT_DOUBLE(int32_t)
CONVERT_DOUBLE(uint32_t)
CONVERT_DOUBLE(int64_t)
CONVERT_DOUBLE(uint64_t)
#undef CONVERT_DOUBLE

#define CONVERT_FLOAT_N(type)                                                  \
  NEATVector NEAT_WRAP::convert_float_2(type *src) {                           \
    return NEATALU::convert_float<type, 2>(src);                               \
  }                                                                            \
  NEATVector NEAT_WRAP::convert_float_3(type *src) {                           \
    return NEATALU::convert_float<type, 3>(src);                               \
  }                                                                            \
  NEATVector NEAT_WRAP::convert_float_4(type *src) {                           \
    return NEATALU::convert_float<type, 4>(src);                               \
  }                                                                            \
  NEATVector NEAT_WRAP::convert_float_8(type *src) {                           \
    return NEATALU::convert_float<type, 8>(src);                               \
  }                                                                            \
  NEATVector NEAT_WRAP::convert_float_16(type *src) {                          \
    return NEATALU::convert_float<type, 16>(src);                              \
  }
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
  NEATVector NEAT_WRAP::convert_double_2(type *src) {                          \
    return NEATALU::convert_double<type, 2>(src);                              \
  }                                                                            \
  NEATVector NEAT_WRAP::convert_double_3(type *src) {                          \
    return NEATALU::convert_double<type, 3>(src);                              \
  }                                                                            \
  NEATVector NEAT_WRAP::convert_double_4(type *src) {                          \
    return NEATALU::convert_double<type, 4>(src);                              \
  }                                                                            \
  NEATVector NEAT_WRAP::convert_double_8(type *src) {                          \
    return NEATALU::convert_double<type, 8>(src);                              \
  }                                                                            \
  NEATVector NEAT_WRAP::convert_double_16(type *src) {                         \
    return NEATALU::convert_double<type, 16>(src);                             \
  }
CONVERT_DOUBLE_N(int8_t)
CONVERT_DOUBLE_N(uint8_t)
CONVERT_DOUBLE_N(int16_t)
CONVERT_DOUBLE_N(uint16_t)
CONVERT_DOUBLE_N(int32_t)
CONVERT_DOUBLE_N(uint32_t)
CONVERT_DOUBLE_N(int64_t)
CONVERT_DOUBLE_N(uint64_t)

#undef CONVERT_DOUBLE_N

NEATValue NEAT_WRAP::convert_float_d(NEATValue *src) {
  return NEATALU::convert_float<double>((NEATType<double>::type *)src);
}
NEATVector NEAT_WRAP::convert_float_d_2(NEATValue *src) {
  return NEATALU::convert_float<double, 2>((NEATType<double>::type *)src);
}
NEATVector NEAT_WRAP::convert_float_d_3(NEATValue *src) {
  return NEATALU::convert_float<double, 3>((NEATType<double>::type *)src);
}
NEATVector NEAT_WRAP::convert_float_d_4(NEATValue *src) {
  return NEATALU::convert_float<double, 4>((NEATType<double>::type *)src);
}
NEATVector NEAT_WRAP::convert_float_d_8(NEATValue *src) {
  return NEATALU::convert_float<double, 8>((NEATType<double>::type *)src);
}
NEATVector NEAT_WRAP::convert_float_d_16(NEATValue *src) {
  return NEATALU::convert_float<double, 16>((NEATType<double>::type *)src);
}

NEATValue NEAT_WRAP::convert_double_f(NEATValue *src) {
  return NEATALU::convert_double<float>((NEATType<float>::type *)src);
}
NEATVector NEAT_WRAP::convert_double_f_2(NEATValue *src) {
  return NEATALU::convert_double<float, 2>((NEATType<float>::type *)src);
}
NEATVector NEAT_WRAP::convert_double_f_3(NEATValue *src) {
  return NEATALU::convert_double<float, 3>((NEATType<float>::type *)src);
}
NEATVector NEAT_WRAP::convert_double_f_4(NEATValue *src) {
  return NEATALU::convert_double<float, 4>((NEATType<float>::type *)src);
}
NEATVector NEAT_WRAP::convert_double_f_8(NEATValue *src) {
  return NEATALU::convert_double<float, 8>((NEATType<float>::type *)src);
}
NEATVector NEAT_WRAP::convert_double_f_16(NEATValue *src) {
  return NEATALU::convert_double<float, 16>((NEATType<float>::type *)src);
}

NEATVector NEAT_WRAP::read_imagef_src_noneat_f(
    void *imageData, Conformance::image_descriptor *imageInfo, float x, float y,
    float z, Conformance::image_sampler_data *imageSampler) {
  return NEATALU::read_imagef_src_noneat(imageData, imageInfo, x, y, z,
                                         imageSampler);
}

} // namespace Validation

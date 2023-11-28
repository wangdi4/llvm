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

#ifndef __RANDOM_UNIFORM_PROVIDER_H__
#define __RANDOM_UNIFORM_PROVIDER_H__

#include "DataType.h"
#include "FloatOperations.h"
#include "dxfloat.h"
#include <float.h>

namespace Validation {
/// uniform random generator class is container of random generators
/// Random generators generate scalar values of supported data types
/// They share the same initial seed produces uniformly distributed values
/// within their interval
class RandomUniformProvider {
public:
  /// ctor with seed. should initialize
  explicit RandomUniformProvider(const uint64_t seed = 0);
  // obtain seed
  uint64_t seed() const { return m_seed; }
  /// get random next sample.
  /// @return uint32_t value from whole uint32_t set
  uint32_t sample_u32_unscaled() const;
  /// get random next sample.
  /// @return uint64_t value from whole uint64_t set
  uint64_t sample_u64(uint64_t min = 0, uint64_t max = UINT64_MAX) const;
  /// get random next sample.
  /// @return unscaled uint64_t value from whole uint64_t set
  uint64_t sample_u64_unscaled() const;
  // floating point
  // from -FLT_MAX and FLT_MAX
  float sample_f32(float min = -FLT_MAX, float max = FLT_MAX) const;
  // from -DBL_MAX and DBL_MAX
  double sample_f64(double min = -DBL_MAX, double max = DBL_MAX) const;

  // floating point
  // from -FLT_MAX and FLT_MAX
  float sample_f32_binary() const;
  // from -DBL_MAX and DBL_MAX
  double sample_f64_binary() const;

private:
  // hide copy ctor to disable passing as function argument by value
  RandomUniformProvider(const RandomUniformProvider &UniformQuasiGenerator);
  /// internal generator's variables
  mutable uint32_t m_v11, m_v12, m_v13, m_v2, m_flag;
  uint64_t m_seed;
  double Generator() const;
  void InitGenerator();
};

template <typename T> struct RandomGeneratorInterfaceProvider;

template <> struct RandomGeneratorInterfaceProvider<CFloat16> {
  static CFloat16 sample(const RandomUniformProvider &rnd) {
    return (CFloat16)(rnd.sample_f32());
  }
};
template <> struct RandomGeneratorInterfaceProvider<float> {
  static float sample(const RandomUniformProvider &rnd) {
    return rnd.sample_f32();
  }
};
template <> struct RandomGeneratorInterfaceProvider<double> {
  static double sample(const RandomUniformProvider &rnd) {
    return rnd.sample_f64();
  }
};
template <typename T> struct RandomGeneratorInterfaceProvider {
  static T sample(const RandomUniformProvider &rnd) {
    IsIntegerType<T> _notUsed;
    UNUSED_ARGUMENT(_notUsed);
    return rnd.sample_u64_unscaled();
  }
};

} // namespace Validation
#endif //__RANDOM_UNIFORM_PROVIDER_H__

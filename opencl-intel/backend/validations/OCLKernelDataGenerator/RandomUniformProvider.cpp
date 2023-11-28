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

#include "RandomUniformProvider.h"
#include "FloatOperations.h"
#include <iostream>

using namespace Validation;

RandomUniformProvider::RandomUniformProvider(const uint64_t seed) {
  m_seed = seed;
  InitGenerator();
}

void RandomUniformProvider::InitGenerator(void) {
  const uint32_t INIT_VALUE_1 = 6334;
  const uint32_t INIT_VALUE_2 = 18467;
  const uint32_t INIT_VALUE_3 = 41;

  // set init data for generator
  m_v11 = INIT_VALUE_1;
  m_v12 = INIT_VALUE_2;
  m_v13 = INIT_VALUE_3;
  m_v2 = (uint32_t)m_seed;
  m_flag = 1;
  // warm up the generator
  for (int32_t i = 0; i < 10; i++) {
    Generator();
  }
}
double RandomUniformProvider::Generator(void) const {
  const int32_t MPY_INT32 = 69069;
  const int32_t ADD_INT32 = 1013904243;
  const double MPY_DBL = 0.4656613e-9;

  int32_t local_1 = m_v12 - (m_v13 + m_flag);

  if (local_1 > 0) {
    m_flag = 0;
  } else {
    local_1 -= 18;
    m_flag = 1;
  }

  m_v13 = m_v12;
  m_v12 = m_v11;
  m_v11 = local_1;
  /// casting to int32_t is important
  int32_t local_2 = MPY_INT32 * m_v2 + ADD_INT32;

  m_v2 = local_2;
  return (MPY_DBL * (local_1 + local_2));
}

uint32_t RandomUniformProvider::sample_u32_unscaled() const {
  return (uint32_t)(UINT32_MAX * Generator());
}

uint64_t RandomUniformProvider::sample_u64_unscaled() const {
  return (((uint64_t)sample_u32_unscaled()) << 32) |
         (uint64_t)sample_u32_unscaled();
}

uint64_t RandomUniformProvider::sample_u64(uint64_t min, uint64_t max) const {
  uint64_t result;
  if ((max + min) < max || (max - min) > max) {
    uint64_t add = max / 2 + min / 2;
    double mpy = (double)(max / 2 - min / 2);
    result = (uint64_t)(mpy * Generator()) + add;
    if (result > max)
      result = max;
    if (result < min)
      result = min;
  } else {
    double add = (double)(max + min) / 2.0;
    double mpy = (double)(max - min) / 2.0;
    result = (uint64_t)(mpy * Generator() + add);
    if (result > max)
      result = max;
    if (result < min)
      result = min;
  }
  return result;
}

float RandomUniformProvider::sample_f32(float min, float max) const {
  float result;
  if ((max + min) < max || (max - min) > max) {
    uint64_t add = max / 2 + min / 2;
    double mpy = (double)(max / 2 - min / 2);
    result = (float)(mpy * Generator()) + add;
    if (result > max)
      result = max;
    if (result < min)
      result = min;
  } else {
    double add = (double)(max + min) / 2.0;
    double mpy = (double)(max - min) / 2.0;
    result = (float)(mpy * Generator() + add);
    if (result > max)
      result = max;
    if (result < min)
      result = min;
  }
  return result;
}

double RandomUniformProvider::sample_f64(double min, double max) const {
  double result;
  if ((max + min) < max || (max - min) > max) {
    uint64_t add = max / 2 + min / 2;
    double mpy = (double)(max / 2 - min / 2);
    result = (double)(mpy * Generator()) + add;
    if (result > max)
      result = max;
    if (result < min)
      result = min;
  } else {
    double add = (double)(max + min) / 2.0;
    double mpy = (double)(max - min) / 2.0;
    result = (double)(mpy * Generator() + add);
    if (result > max)
      result = max;
    if (result < min)
      result = min;
  }
  return result;
}

double RandomUniformProvider::sample_f64_binary() const {
  union {
    uint64_t I;
    double D;
  } tmpDouble;
  bool flagForNormalDouble = 0;
  while (flagForNormalDouble == 0) {
    tmpDouble.I = sample_u64();
    if ((!Utils::IsNaN(tmpDouble.D)) && (!Utils::IsInf(tmpDouble.D)) &&
        (!Utils::IsDenorm(tmpDouble.D)))
      flagForNormalDouble = 1;
  }
  return tmpDouble.D;
}

float RandomUniformProvider::sample_f32_binary() const {
  union {
    int I;
    float F;
  } tmpFloat;
  bool flagForNormalFloat = 0;
  while (flagForNormalFloat == 0) {
    tmpFloat.I = sample_u64();
    if ((!Utils::IsNaN(tmpFloat.F)) && (!Utils::IsInf(tmpFloat.F)) &&
        (!Utils::IsDenorm(tmpFloat.F)))
      flagForNormalFloat = 1;
  }
  return tmpFloat.F;
}

// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ESIMD_EMBARGO */
//==--------- hf8.hpp ------- SYCL hf8 conversion ------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of SIMD hf8 type. This type represents floating point with 
// 1 bit sign, 4 bit exponent, 3 bits mantissa.
//===----------------------------------------------------------------------===//

#pragma once

#include <CL/__spirv/spirv_ops.hpp>
#include <sycl/bit_cast.hpp>

namespace sycl {
__SYCL_INLINE_VER_NAMESPACE(_V1) {
  namespace ext {
  namespace intel {
  namespace experimental {
  namespace esimd {

  class hf8 {
    using storage_t = uint8_t;
    storage_t value;

  public:
    hf8() = default;
    hf8(const hf8 &) = default;
    ~hf8() = default;

    // Explicit conversion functions
    static storage_t from_float(const float &a) {
      uint32_t tmp_uint = sycl::bit_cast<uint32_t>(a);
      int32_t Exponent = (tmp_uint & 0x7f800000) >> 23;
      if (Exponent != 0) {
        Exponent -= 127;
        // Normalize exponent for hf8
        Exponent = Exponent < -7 || Exponent > 7 ? 15 : Exponent + 7; 
      }
      
      storage_t Result = ((tmp_uint & 0x80000000) >> (31 - 7)) |
                         ((Exponent & 0xf) << 3) |
                         ((tmp_uint & 0x700000) >> (23 - 3));
      return Result;
    }
    static float to_float(const storage_t &a) {
      // Normalize the hf8 exponent for float
      int32_t Exponent = ((a & 0x78) >> 3);
      Exponent = Exponent == 0 ? 0 : Exponent - 7 + 127;
      uint32_t Result = ((a & 0x7) << (23 - 3)) |
                        ((Exponent & 0xff) << 23) |
                        ((a & 0x80) << (31 - 7));
      return sycl::bit_cast<float>(Result);
    }

    // Implicit conversion from float to hf8
    hf8(const float &a) { value = from_float(a); }

    hf8 &operator=(const float &rhs) {
      value = from_float(rhs);
      return *this;
    }

    // Implicit conversion from hf8 to float
    operator float() const { return to_float(value); }

    // Get raw bits representation of hf8
    storage_t raw() const { return value; }

    // Logical operators (!,||,&&) are covered if we can cast to bool
    explicit operator bool() { return to_float(value) != 0.0f; }

    // Unary minus operator overloading
    friend hf8 operator-(hf8 &lhs) { return hf8(-to_float(lhs)); }
  };

  } // namespace esimd
  } // namespace experimental
  } // namespace intel
  } // namespace ext

} // __SYCL_INLINE_VER_NAMESPACE(_V1)
} // namespace sycl
/* end INTEL_FEATURE_ESIMD_EMBARGO */
/* end INTEL_CUSTOMIZATION */

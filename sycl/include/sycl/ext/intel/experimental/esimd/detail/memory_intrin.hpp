// INTEL_CUSTOMIZATION
//
// Modifications, Copyright (C) 2021 Intel Corporation
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
//==------------ memory_intrin.hpp - DPC++ Explicit SIMD API ---------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Declares experimental memory Explicit SIMD intrinsics.
//===----------------------------------------------------------------------===//

/// @cond ESIMD_DETAIL

#pragma once

#include <sycl/ext/intel/esimd/detail/atomic_intrin.hpp>
#include <sycl/ext/intel/esimd/detail/defines_elementary.hpp>
#include <sycl/ext/intel/esimd/detail/memory_intrin.hpp>

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ESIMD_EMBARGO */
#include <sycl/ext/intel/experimental/esimd/common.hpp>
/* end INTEL_FEATURE_ESIMD_EMBARGO */
/* end INTEL_CUSTOMIZATION */

// generic work-group split barrier
__ESIMD_INTRIN void __esimd_sbarrier(__ESIMD_ENS::split_barrier_action flag)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// Create an explicit data and GPU scoreboard dependency.
__ESIMD_INTRIN void __esimd_wait(uint16_t value)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ESIMD_EMBARGO */

// \brief Generalized raw send.
//
// @tparam eot is the flag that indicates whether this is an EOT message.
//
// @tparam sendc is the flag that indicates whether sendc should be used.
//
// @tparam exec_size the execution size.
//
// @tparam sfid the shared function ID.
//
// @tparam dst_size the number of bytes for the destination.
//
// @tparam src0_size the number of bytes for the first source operand.
//
// @tparam src1_size the number of bytes for the second source operand.
//
// @tparam desc the immediate descriptor.
//
// @param pred the predicate to specify enabled channels.
//
// @param msg_src0 the first source operand of send message.
//
// @param msg_src1 the second source operand of send message.
//
// @param ind0 the indirect descriptor 0.
//
// @param ind1 the indirect descriptor 1.
//
// @param msg_dst is the old value of the destination operand.
//
template <__ESIMD_ENS::raw_send_eot eot, __ESIMD_ENS::raw_send_sendc sendc,
          uint8_t exec_size, uint32_t sfid, uint32_t dst_size,
          uint32_t src0_size, uint32_t src1_size, uint64_t desc, typename T0,
          int N0, typename T1, int N1, typename T2, int N2>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<T2, N2>
__esimd_raw_sendg2(__ESIMD_DNS::simd_mask_storage_t<exec_size> pred,
                   __ESIMD_DNS::vector_type_t<T0, N0> msg_src0,
                   __ESIMD_DNS::vector_type_t<T1, N1> msg_src1, uint64_t ind0,
                   uint64_t ind1, __ESIMD_DNS::vector_type_t<T2, N2> msg_dst)
#ifdef __SYCL_DEVICE_ONLY__
{
  using namespace __ESIMD_ENS;
  if constexpr (sendc == raw_send_sendc::sendc) {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%11) raw_sendgc_eot.%1 (M1, %2) %0.0/%3 %4.0/%5 %6.0/%7 %8 %9 %10"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "^rw"(msg_src1), "n"(src1_size), "r"(ind0),
            "r"(ind1), "n"(desc), "^cr"(pred));
    } else {
      asm("(%11) raw_sendgc.%1 (M1, %2) %0.0/%3 %4.0/%5 %6.0/%7 %8 %9 %10"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "^rw"(msg_src1), "n"(src1_size), "r"(ind0),
            "r"(ind1), "n"(desc), "^cr"(pred));
    }
  } else {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%11) raw_sendg_eot.%1 (M1, %2) %0.0/%3 %4.0/%5 %6.0/%7 %8 %9 %10"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "^rw"(msg_src1), "n"(src1_size), "r"(ind0),
            "r"(ind1), "n"(desc), "^cr"(pred));
    } else {
      asm("(%11) raw_sendg.%1 (M1, %2) %0.0/%3 %4.0/%5 %6.0/%7 %8 %9 %10"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "^rw"(msg_src1), "n"(src1_size), "r"(ind0),
            "r"(ind1), "n"(desc), "^cr"(pred));
    }
  }
  return msg_dst;
}
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Generalized raw send.
//
// @tparam eot is the flag that indicates whether this is an EOT message.
//
// @tparam sendc is the flag that indicates whether sendc should be used.
//
// @tparam exec_size the execution size.
//
// @tparam sfid the shared function ID.
//
// @tparam dst_size the number of bytes for the destination.
//
// @tparam src0_size the number of bytes for the first source operand.
//
// @tparam src1_size the number of bytes for the second source operand.
//
// @tparam desc the immediate descriptor.
//
// @param pred the predicate to specify enabled channels.
//
// @param msg_src0 the first source operand of send message.
//
// @param msg_src1 the second source operand of send message.
//
// @param ind0 the indirect descriptor 0.
//
// @param msg_dst is the old value of the destination operand.
//
template <__ESIMD_ENS::raw_send_eot eot, __ESIMD_ENS::raw_send_sendc sendc,
          uint8_t exec_size, uint32_t sfid, uint32_t dst_size,
          uint32_t src0_size, uint32_t src1_size, uint64_t desc, typename T0,
          int N0, typename T1, int N1, typename T2, int N2>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<T2, N2>
__esimd_raw_sendg2(__ESIMD_DNS::simd_mask_storage_t<exec_size> pred,
                   __ESIMD_DNS::vector_type_t<T0, N0> msg_src0,
                   __ESIMD_DNS::vector_type_t<T1, N1> msg_src1, uint64_t ind0,
                   __ESIMD_DNS::vector_type_t<T2, N2> msg_dst)
#ifdef __SYCL_DEVICE_ONLY__
{
  using namespace __ESIMD_ENS;
  if constexpr (sendc == raw_send_sendc::sendc) {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%10) raw_sendgc_eot.%1 (M1, %2) %0.0/%3 %4.0/%5 %6.0/%7 %8 "
          "%%null.0/0 %9"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "^rw"(msg_src1), "n"(src1_size), "r"(ind0),
            "n"(desc), "^cr"(pred));
    } else {
      asm("(%10) raw_sendgc.%1 (M1, %2) %0.0/%3 %4.0/%5 %6.0/%7 %8 %%null.0/0 "
          "%9"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "^rw"(msg_src1), "n"(src1_size), "r"(ind0),
            "n"(desc), "^cr"(pred));
    }
  } else {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%10) raw_sendg_eot.%1 (M1, %2) %0.0/%3 %4.0/%5 %6.0/%7 %8 "
          "%%null.0/0 %9"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "^rw"(msg_src1), "n"(src1_size), "r"(ind0),
            "n"(desc), "^cr"(pred));
    } else {
      asm("(%10) raw_sendg.%1 (M1, %2) %0.0/%3 %4.0/%5 %6.0/%7 %8 %%null.0/0 %9"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "^rw"(msg_src1), "n"(src1_size), "r"(ind0),
            "n"(desc), "^cr"(pred));
    }
  }
  return msg_dst;
}
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Generalized raw send.
//
// @tparam eot is the flag that indicates whether this is an EOT message.
//
// @tparam sendc is the flag that indicates whether sendc should be used.
//
// @tparam exec_size the execution size.
//
// @tparam sfid the shared function ID.
//
// @tparam dst_size the number of bytes for the destination.
//
// @tparam src0_size the number of bytes for the first source operand.
//
// @tparam src1_size the number of bytes for the second source operand.
//
// @tparam desc the immediate descriptor.
//
// @param pred the predicate to specify enabled channels.
//
// @param msg_src0 the first source operand of send message.
//
// @param msg_src1 the second source operand of send message.
//
// @param msg_dst is the old value of the destination operand.
//
template <__ESIMD_ENS::raw_send_eot eot, __ESIMD_ENS::raw_send_sendc sendc,
          uint8_t exec_size, uint32_t sfid, uint32_t dst_size,
          uint32_t src0_size, uint32_t src1_size, uint64_t desc, typename T0,
          int N0, typename T1, int N1, typename T2, int N2>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<T2, N2>
__esimd_raw_sendg2(__ESIMD_DNS::simd_mask_storage_t<exec_size> pred,
                   __ESIMD_DNS::vector_type_t<T0, N0> msg_src0,
                   __ESIMD_DNS::vector_type_t<T1, N1> msg_src1,
                   __ESIMD_DNS::vector_type_t<T2, N2> msg_dst)
#ifdef __SYCL_DEVICE_ONLY__
{
  using namespace __ESIMD_ENS;
  if constexpr (sendc == raw_send_sendc::sendc) {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%9) raw_sendgc_eot.%1 (M1, %2) %0.0/%3 %4.0/%5 %6.0/%7 %%null.0/0 "
          "%%null.0/0 %8"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "^rw"(msg_src1), "n"(src1_size), "n"(desc),
            "^cr"(pred));
    } else {
      asm("(%9) raw_sendgc.%1 (M1, %2) %0.0/%3 %4.0/%5 %6.0/%7 %%null.0/0 "
          "%%null.0/0 %8"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "^rw"(msg_src1), "n"(src1_size), "n"(desc),
            "^cr"(pred));
    }
  } else {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%9) raw_sendg_eot.%1 (M1, %2) %0.0/%3 %4.0/%5 %6.0/%7 %%null.0/0 "
          "%%null.0/0 %8"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "^rw"(msg_src1), "n"(src1_size), "n"(desc),
            "^cr"(pred));
    } else {
      asm("(%9) raw_sendg.%1 (M1, %2) %0.0/%3 %4.0/%5 %6.0/%7 %%null.0/0 "
          "%%null.0/0 %8"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "^rw"(msg_src1), "n"(src1_size), "n"(desc),
            "^cr"(pred));
    }
  }
  return msg_dst;
}
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Generalized raw send.
//
// @tparam eot is the flag that indicates whether this is an EOT message.
//
// @tparam sendc is the flag that indicates whether sendc should be used.
//
// @tparam exec_size the execution size.
//
// @tparam sfid the shared function ID.
//
// @tparam src0_size the number of bytes for the first source operand.
//
// @tparam src1_size the number of bytes for the second source operand.
//
// @tparam desc the immediate descriptor.
//
// @param pred the predicate to specify enabled channels.
//
// @param msg_src0 the first source operand of send message.
//
// @param msg_src1 the second source operand of send message.
//
// @param ind0 the indirect descriptor 0.
//
// @param ind1 the indirect descriptor 1.
//
template <__ESIMD_ENS::raw_send_eot eot, __ESIMD_ENS::raw_send_sendc sendc,
          uint8_t exec_size, uint32_t sfid, uint32_t src0_size,
          uint32_t src1_size, uint64_t desc, typename T0, int N0, typename T1,
          int N1>
__ESIMD_INTRIN void
__esimd_raw_sendg2_noresult(__ESIMD_DNS::simd_mask_storage_t<exec_size> pred,
                            __ESIMD_DNS::vector_type_t<T0, N0> msg_src0,
                            __ESIMD_DNS::vector_type_t<T1, N1> msg_src1,
                            uint64_t ind0, uint64_t ind1)
#ifdef __SYCL_DEVICE_ONLY__
{
  using namespace __ESIMD_ENS;
  if constexpr (sendc == raw_send_sendc::sendc) {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%9) raw_sendgc_eot.%0 (M1, %1) %%null.0/0 %2.0/%3 %4.0/%5 %6 %7 %8"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "^rw"(msg_src1), "n"(src1_size), "r"(ind0), "r"(ind1), "n"(desc),
            "^cr"(pred));
    } else {
      asm("(%9) raw_sendgc.%0 (M1, %1) %%null.0/0 %2.0/%3 %4.0/%5 %6 %7 %8"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "^rw"(msg_src1), "n"(src1_size), "r"(ind0), "r"(ind1), "n"(desc),
            "^cr"(pred));
    }
  } else {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%9) raw_sendg_eot.%0 (M1, %1) %%null.0/0 %2.0/%3 %4.0/%5 %6 %7 %8"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "^rw"(msg_src1), "n"(src1_size), "r"(ind0), "r"(ind1), "n"(desc),
            "^cr"(pred));
    } else {
      asm("(%9) raw_sendg.%0 (M1, %1) %%null.0/0 %2.0/%3 %4.0/%5 %6 %7 %8"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "^rw"(msg_src1), "n"(src1_size), "r"(ind0), "r"(ind1), "n"(desc),
            "^cr"(pred));
    }
  }
}
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Generalized raw send.
//
// @tparam eot is the flag that indicates whether this is an EOT message.
//
// @tparam sendc is the flag that indicates whether sendc should be used.
//
// @tparam exec_size the execution size.
//
// @tparam sfid the shared function ID.
//
// @tparam src0_size the number of bytes for the first source operand.
//
// @tparam src1_size the number of bytes for the second source operand.
//
// @tparam desc the immediate descriptor.
//
// @param pred the predicate to specify enabled channels.
//
// @param msg_src0 the first source operand of send message.
//
// @param msg_src1 the second source operand of send message.
//
// @param ind0 the indirect descriptor 0.
//
template <__ESIMD_ENS::raw_send_eot eot, __ESIMD_ENS::raw_send_sendc sendc,
          uint8_t exec_size, uint32_t sfid, uint32_t src0_size,
          uint32_t src1_size, uint64_t desc, typename T0, int N0, typename T1,
          int N1>
__ESIMD_INTRIN void
__esimd_raw_sendg2_noresult(__ESIMD_DNS::simd_mask_storage_t<exec_size> pred,
                            __ESIMD_DNS::vector_type_t<T0, N0> msg_src0,
                            __ESIMD_DNS::vector_type_t<T1, N1> msg_src1,
                            uint64_t ind0)
#ifdef __SYCL_DEVICE_ONLY__
{
  using namespace __ESIMD_ENS;
  if constexpr (sendc == raw_send_sendc::sendc) {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%8) raw_sendgc_eot.%0 (M1, %1) %%null.0/0 %2.0/%3 %4.0/%5 %6 "
          "%%null.0/0 %7"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "^rw"(msg_src1), "n"(src1_size), "r"(ind0), "n"(desc), "^cr"(pred));
    } else {
      asm("(%8) raw_sendgc.%0 (M1, %1) %%null.0/0 %2.0/%3 %4.0/%5 %6 "
          "%%null.0/0 %7"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "^rw"(msg_src1), "n"(src1_size), "r"(ind0), "n"(desc), "^cr"(pred));
    }
  } else {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%8) raw_sendg_eot.%0 (M1, %1) %%null.0/0 %2.0/%3 %4.0/%5 %6 "
          "%%null.0/0 %7"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "^rw"(msg_src1), "n"(src1_size), "r"(ind0), "n"(desc), "^cr"(pred));
    } else {
      asm("(%8) raw_sendg.%0 (M1, %1) %%null.0/0 %2.0/%3 %4.0/%5 %6 %%null.0/0 "
          "%7"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "^rw"(msg_src1), "n"(src1_size), "r"(ind0), "n"(desc), "^cr"(pred));
    }
  }
}
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Generalized raw send.
//
// @tparam eot is the flag that indicates whether this is an EOT message.
//
// @tparam sendc is the flag that indicates whether sendc should be used.
//
// @tparam exec_size the execution size.
//
// @tparam sfid the shared function ID.
//
// @tparam src0_size the number of bytes for the first source operand.
//
// @tparam src1_size the number of bytes for the second source operand.
//
// @tparam desc the immediate descriptor.
//
// @param pred the predicate to specify enabled channels.
//
// @param msg_src0 the first source operand of send message.
//
// @param msg_src1 the second source operand of send message.
//
template <__ESIMD_ENS::raw_send_eot eot, __ESIMD_ENS::raw_send_sendc sendc,
          uint8_t exec_size, uint32_t sfid, uint32_t src0_size,
          uint32_t src1_size, uint64_t desc, typename T0, int N0, typename T1,
          int N1>
__ESIMD_INTRIN void
__esimd_raw_sendg2_noresult(__ESIMD_DNS::simd_mask_storage_t<exec_size> pred,
                            __ESIMD_DNS::vector_type_t<T0, N0> msg_src0,
                            __ESIMD_DNS::vector_type_t<T1, N1> msg_src1)
#ifdef __SYCL_DEVICE_ONLY__
{
  using namespace __ESIMD_ENS;
  if constexpr (sendc == raw_send_sendc::sendc) {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%7) raw_sendgc_eot.%0 (M1, %1) %%null.0/0 %2.0/%3 %4.0/%5 "
          "%%null.0/0 %%null.0/0 %6"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "^rw"(msg_src1), "n"(src1_size), "n"(desc), "^cr"(pred));
    } else {
      asm("(%7) raw_sendgc.%0 (M1, %1) %%null.0/0 %2.0/%3 %4.0/%5 %%null.0/0 "
          "%%null.0/0 %6"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "^rw"(msg_src1), "n"(src1_size), "n"(desc), "^cr"(pred));
    }
  } else {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%7) raw_sendg_eot.%0 (M1, %1) %%null.0/0 %2.0/%3 %4.0/%5 "
          "%%null.0/0 %%null.0/0 %6"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "^rw"(msg_src1), "n"(src1_size), "n"(desc), "^cr"(pred));
    } else {
      asm("(%7) raw_sendg.%0 (M1, %1) %%null.0/0 %2.0/%3 %4.0/%5 %%null.0/0 "
          "%%null.0/0 %6"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "^rw"(msg_src1), "n"(src1_size), "n"(desc), "^cr"(pred));
    }
  }
}
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Generalized raw send.
//
// @tparam eot is the flag that indicates whether this is an EOT message.
//
// @tparam sendc is the flag that indicates whether sendc should be used.
//
// @tparam exec_size the execution size.
//
// @tparam sfid the shared function ID.
//
// @tparam dst_size the number of bytes for the destination.
//
// @tparam src0_size the number of bytes for the first source operand.
//
// @tparam desc the immediate descriptor.
//
// @param pred the predicate to specify enabled channels.
//
// @param msg_src0 the first source operand of send message.
//
// @param ind0 the indirect descriptor 0.
//
// @param ind1 the indirect descriptor 1.
//
// @param msg_dst is the old value of the destination operand.
//
template <__ESIMD_ENS::raw_send_eot eot, __ESIMD_ENS::raw_send_sendc sendc,
          uint8_t exec_size, uint32_t sfid, uint32_t dst_size,
          uint32_t src0_size, uint64_t desc, typename T0, int N0, typename T1,
          int N1>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<T1, N1>
__esimd_raw_sendg(__ESIMD_DNS::simd_mask_storage_t<exec_size> pred,
                  __ESIMD_DNS::vector_type_t<T0, N0> msg_src0, uint64_t ind0,
                  uint64_t ind1, __ESIMD_DNS::vector_type_t<T1, N1> msg_dst)
#ifdef __SYCL_DEVICE_ONLY__
{
  using namespace __ESIMD_ENS;
  if constexpr (sendc == raw_send_sendc::sendc) {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%9) raw_sendgc_eot.%1 (M1, %2) %0.0/%3 %4.0/%5 %%null.0/0 %6 %7 %8"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "r"(ind0), "r"(ind1), "n"(desc), "^cr"(pred));
    } else {
      asm("(%9) raw_sendgc.%1 (M1, %2) %0.0/%3 %4.0/%5 %%null.0/0 %6 %7 %8"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "r"(ind0), "r"(ind1), "n"(desc), "^cr"(pred));
    }
  } else {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%9) raw_sendg_eot.%1 (M1, %2) %0.0/%3 %4.0/%5 %%null.0/0 %6 %7 %8"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "r"(ind0), "r"(ind1), "n"(desc), "^cr"(pred));
    } else {
      asm("(%9) raw_sendg.%1 (M1, %2) %0.0/%3 %4.0/%5 %%null.0/0 %6 %7 %8"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "r"(ind0), "r"(ind1), "n"(desc), "^cr"(pred));
    }
  }
  return msg_dst;
}
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Generalized raw send.
//
// @tparam eot is the flag that indicates whether this is an EOT message.
//
// @tparam sendc is the flag that indicates whether sendc should be used.
//
// @tparam exec_size the execution size.
//
// @tparam sfid the shared function ID.
//
// @tparam dst_size the number of bytes for the destination.
//
// @tparam src0_size the number of bytes for the first source operand.
//
// @tparam desc the immediate descriptor.
//
// @param pred the predicate to specify enabled channels.
//
// @param msg_src0 the first source operand of send message.
//
// @param ind0 the indirect descriptor 0.
//
// @param msg_dst is the old value of the destination operand.
//
template <__ESIMD_ENS::raw_send_eot eot, __ESIMD_ENS::raw_send_sendc sendc,
          uint8_t exec_size, uint32_t sfid, uint32_t dst_size,
          uint32_t src0_size, uint64_t desc, typename T0, int N0, typename T1,
          int N1>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<T1, N1>
__esimd_raw_sendg(__ESIMD_DNS::simd_mask_storage_t<exec_size> pred,
                  __ESIMD_DNS::vector_type_t<T0, N0> msg_src0, uint64_t ind0,
                  __ESIMD_DNS::vector_type_t<T1, N1> msg_dst)
#ifdef __SYCL_DEVICE_ONLY__
{
  using namespace __ESIMD_ENS;
  if constexpr (sendc == raw_send_sendc::sendc) {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%8) raw_sendgc_eot.%1 (M1, %2) %0.0/%3 %4.0/%5 %%null.0/0 %6 "
          "%%null.0/0 %7"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "r"(ind0), "n"(desc), "^cr"(pred));
    } else {
      asm("(%8) raw_sendgc.%1 (M1, %2) %0.0/%3 %4.0/%5 %%null.0/0 %6 "
          "%%null.0/0 %7"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "r"(ind0), "n"(desc), "^cr"(pred));
    }
  } else {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%8) raw_sendg_eot.%1 (M1, %2) %0.0/%3 %4.0/%5 %%null.0/0 %6 "
          "%%null.0/0 %7"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "r"(ind0), "n"(desc), "^cr"(pred));
    } else {
      asm("(%8) raw_sendg.%1 (M1, %2) %0.0/%3 %4.0/%5 %%null.0/0 %6 %%null.0/0 "
          "%7"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "r"(ind0), "n"(desc), "^cr"(pred));
    }
  }
  return msg_dst;
}
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Generalized raw send.
//
// @tparam eot is the flag that indicates whether this is an EOT message.
//
// @tparam sendc is the flag that indicates whether sendc should be used.
//
// @tparam exec_size the execution size.
//
// @tparam sfid the shared function ID.
//
// @tparam dst_size the number of bytes for the destination.
//
// @tparam src0_size the number of bytes for the first source operand.
//
// @tparam desc the immediate descriptor.
//
// @param pred the predicate to specify enabled channels.
//
// @param msg_src0 the first source operand of send message.
//
// @param msg_dst is the old value of the destination operand.
//
template <__ESIMD_ENS::raw_send_eot eot, __ESIMD_ENS::raw_send_sendc sendc,
          uint8_t exec_size, uint32_t sfid, uint32_t dst_size,
          uint32_t src0_size, uint64_t desc, typename T0, int N0, typename T1,
          int N1>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<T1, N1>
__esimd_raw_sendg(__ESIMD_DNS::simd_mask_storage_t<exec_size> pred,
                  __ESIMD_DNS::vector_type_t<T0, N0> msg_src0,
                  __ESIMD_DNS::vector_type_t<T1, N1> msg_dst)
#ifdef __SYCL_DEVICE_ONLY__
{
  using namespace __ESIMD_ENS;
  if constexpr (sendc == raw_send_sendc::sendc) {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%7) raw_sendgc_eot.%1 (M1, %2) %0.0/%3 %4.0/%5 %%null.0/0 "
          "%%null.0/0 %%null.0/0 %6"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "n"(desc), "^cr"(pred));
    } else {
      asm("(%7) raw_sendgc.%1 (M1, %2) %0.0/%3 %4.0/%5 %%null.0/0 %%null.0/0 "
          "%%null.0/0 %6"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "n"(desc), "^cr"(pred));
    }
  } else {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%7) raw_sendg_eot.%1 (M1, %2) %0.0/%3 %4.0/%5 %%null.0/0 "
          "%%null.0/0 %%null.0/0 %6"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "n"(desc), "^cr"(pred));
    } else {
      asm("(%7) raw_sendg.%1 (M1, %2) %0.0/%3 %4.0/%5 %%null.0/0 %%null.0/0 "
          "%%null.0/0 %6"
          : "=^rw"(msg_dst)
          : "n"(sfid), "n"(exec_size), "n"(dst_size), "^rw"(msg_src0),
            "n"(src0_size), "n"(desc), "^cr"(pred));
    }
  }
  return msg_dst;
}
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Generalized raw send.
//
// @tparam eot is the flag that indicates whether this is an EOT message.
//
// @tparam sendc is the flag that indicates whether sendc should be used.
//
// @tparam exec_size the execution size.
//
// @tparam sfid the shared function ID.
//
// @tparam src0_size the number of bytes for the first source operand.
//
// @tparam desc the immediate descriptor.
//
// @param pred the predicate to specify enabled channels.
//
// @param msg_src0 the first source operand of send message.
//
// @param ind0 the indirect descriptor 0.
//
// @param ind1 the indirect descriptor 1.
//
template <__ESIMD_ENS::raw_send_eot eot, __ESIMD_ENS::raw_send_sendc sendc,
          uint8_t exec_size, uint32_t sfid, uint32_t src0_size, uint64_t desc,
          typename T0, int N0>
__ESIMD_INTRIN void
__esimd_raw_sendg_noresult(__ESIMD_DNS::simd_mask_storage_t<exec_size> pred,
                           __ESIMD_DNS::vector_type_t<T0, N0> msg_src0,
                           uint64_t ind0, uint64_t ind1)
#ifdef __SYCL_DEVICE_ONLY__
{
  using namespace __ESIMD_ENS;
  if constexpr (sendc == raw_send_sendc::sendc) {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%7) raw_sendgc_eot.%0 (M1, %1) %%null.0/0 %2.0/%3 %%null.0/0 %4 %5 "
          "%6"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "r"(ind0), "r"(ind1), "n"(desc), "^cr"(pred));
    } else {
      asm("(%7) raw_sendgc.%0 (M1, %1) %%null.0/0 %2.0/%3 %%null.0/0 %4 %5 %6"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "r"(ind0), "r"(ind1), "n"(desc), "^cr"(pred));
    }
  } else {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%7) raw_sendg_eot.%0 (M1, %1) %%null.0/0 %2.0/%3 %%null.0/0 %4 %5 "
          "%6"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "r"(ind0), "r"(ind1), "n"(desc), "^cr"(pred));
    } else {
      asm("(%7) raw_sendg.%0 (M1, %1) %%null.0/0 %2.0/%3 %%null.0/0 %4 %5 %6"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "r"(ind0), "r"(ind1), "n"(desc), "^cr"(pred));
    }
  }
}
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Generalized raw send.
//
// @tparam eot is the flag that indicates whether this is an EOT message.
//
// @tparam sendc is the flag that indicates whether sendc should be used.
//
// @tparam exec_size the execution size.
//
// @tparam sfid the shared function ID.
//
// @tparam src0_size the number of bytes for the first source operand.
//
// @tparam desc the immediate descriptor.
//
// @param pred the predicate to specify enabled channels.
//
// @param msg_src0 the first source operand of send message.
//
// @param ind0 the indirect descriptor 0.
//
template <__ESIMD_ENS::raw_send_eot eot, __ESIMD_ENS::raw_send_sendc sendc,
          uint8_t exec_size, uint32_t sfid, uint32_t src0_size, uint64_t desc,
          typename T0, int N0>
__ESIMD_INTRIN void
__esimd_raw_sendg_noresult(__ESIMD_DNS::simd_mask_storage_t<exec_size> pred,
                           __ESIMD_DNS::vector_type_t<T0, N0> msg_src0,
                           uint64_t ind0)
#ifdef __SYCL_DEVICE_ONLY__
{
  using namespace __ESIMD_ENS;
  if constexpr (sendc == raw_send_sendc::sendc) {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%6) raw_sendgc_eot.%0 (M1, %1) %%null.0/0 %2.0/%3 %%null.0/0 %4 "
          "%%null.0/0 %5"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "r"(ind0), "n"(desc), "^cr"(pred));
    } else {
      asm("(%6) raw_sendgc.%0 (M1, %1) %%null.0/0 %2.0/%3 %%null.0/0 %4 "
          "%%null.0/0 %5"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "r"(ind0), "n"(desc), "^cr"(pred));
    }
  } else {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%6) raw_sendg_eot.%0 (M1, %1) %%null.0/0 %2.0/%3 %%null.0/0 %4 "
          "%%null.0/0 %5"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "r"(ind0), "n"(desc), "^cr"(pred));
    } else {
      asm("(%6) raw_sendg.%0 (M1, %1) %%null.0/0 %2.0/%3 %%null.0/0 %4 "
          "%%null.0/0 %5"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "r"(ind0), "n"(desc), "^cr"(pred));
    }
  }
}
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Generalized raw send.
//
// @tparam eot is the flag that indicates whether this is an EOT message.
//
// @tparam sendc is the flag that indicates whether sendc should be used.
//
// @tparam exec_size the execution size.
//
// @tparam sfid the shared function ID.
//
// @tparam src0_size the number of bytes for the first source operand.
//
// @tparam desc the immediate descriptor.
//
// @param pred the predicate to specify enabled channels.
//
// @param msg_src0 the first source operand of send message.
//
template <__ESIMD_ENS::raw_send_eot eot, __ESIMD_ENS::raw_send_sendc sendc,
          uint8_t exec_size, uint32_t sfid, uint32_t src0_size, uint64_t desc,
          typename T0, int N0>
__ESIMD_INTRIN void
__esimd_raw_sendg_noresult(__ESIMD_DNS::simd_mask_storage_t<exec_size> pred,
                           __ESIMD_DNS::vector_type_t<T0, N0> msg_src0)
#ifdef __SYCL_DEVICE_ONLY__
{
  using namespace __ESIMD_ENS;
  if constexpr (sendc == raw_send_sendc::sendc) {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%5) raw_sendgc_eot.%0 (M1, %1) %%null.0/0 %2.0/%3 %%null.0/0 "
          "%%null.0/0 %%null.0/0 %4"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "n"(desc), "^cr"(pred));
    } else {
      asm("(%5) raw_sendgc.%0 (M1, %1) %%null.0/0 %2.0/%3 %%null.0/0 "
          "%%null.0/0 %%null.0/0 %4"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "n"(desc), "^cr"(pred));
    }
  } else {
    if constexpr (eot == raw_send_eot::eot) {
      asm("(%5) raw_sendg_eot.%0 (M1, %1) %%null.0/0 %2.0/%3 %%null.0/0 "
          "%%null.0/0 %%null.0/0 %4"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "n"(desc), "^cr"(pred));
    } else {
      asm("(%5) raw_sendg.%0 (M1, %1) %%null.0/0 %2.0/%3 %%null.0/0 %%null.0/0 "
          "%%null.0/0 %4"
          :
          : "n"(sfid), "n"(exec_size), "^rw"(msg_src0), "n"(src0_size),
            "n"(desc), "^cr"(pred));
    }
  }
}
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/* end INTEL_CUSTOMIZATION */
/* end INTEL_FEATURE_ESIMD_EMBARGO */

/// Represents named barrier synchronization for a subgroup of threads.
/// Available only on PVC
///
/// @param mode  - is wait(0) or signal(1)
///
/// @param id  - barrier id
///
/// @param thread_count  - number of threads, ignored in 'wait' mode
__ESIMD_INTRIN void __esimd_nbarrier(uint8_t mode, uint8_t id,
                                     uint8_t thread_count)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// Initialize number of named barriers for a kernel
/// Available only on PVC
///
/// @param count  - number of named barriers
__ESIMD_INTRIN void __esimd_nbarrier_init(uint8_t count)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// Raw send signal to perform signal operation on named barriers
/// Available only on PVC
/// @tparam Ty  - message element type
///
/// @tparam N  - message length
///
/// @param is_sendc  - is sendc
///
/// @param extended_descriptor  - extended message descriptor
///
/// @param descriptor  - message descriptor
///
/// @param msg_var  - source operand of send message
///
/// @param pred  - predicate for enabled channels
template <typename Ty, int N>
__ESIMD_INTRIN void __esimd_raw_send_nbarrier_signal(
    uint32_t is_sendc, uint32_t extended_descriptor, uint32_t descriptor,
    __ESIMD_DNS::vector_type_t<Ty, N> msg_var, uint16_t pred = 1)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// SLM gather.
/// Supported platforms: DG2, PVC
///
/// Collects elements located at slm and returns them
/// as a single \ref simd object.
///
/// @tparam Ty is element type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements to load per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @param pred is predicates.
/// @param offsets is the zero-based offsets for SLM buffer in bytes.
/// @param OldValues contains the vector which elements are copied
/// to the returned result when the corresponding element of \p pred is 0.
/// @return is a vector of type T and size N * to_int<VS>()
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_load_merge_slm(
    __ESIMD_DNS::simd_mask_storage_t<N> pred,
    __ESIMD_DNS::vector_type_t<uint32_t, N> offsets,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> OldValues =
        0)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// Similar to __esimd_lsc_load_merge_slm(), but the argument OldValues is not
/// explicitly specified, which results into random values in those elements of
/// the returned result for which the corresponding element in \p pred is 0.
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_load_slm(__ESIMD_DNS::simd_mask_storage_t<N> pred,
                     __ESIMD_DNS::vector_type_t<uint32_t, N> offsets)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  return __esimd_lsc_load_merge_slm<Ty, L1H, L3H, AddressScale, ImmOffset, DS,
                                    VS, _Transposed, N>(pred, offsets);
}
#endif // __SYCL_DEVICE_ONLY__

/// Surface-based gather.
/// Supported platforms: DG2, PVC
///
/// Collects elements located at surface and returns them
/// as a single \ref simd object.
///
/// @tparam Ty is element type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements to load per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @tparam SurfIndAliasTy is the \ref sycl::accessor type.
/// @param pred is predicates.
/// @param offsets is the zero-based offsets in bytes.
/// @param surf_ind is the surface index.
/// @param OldValues contains the vector which elements are copied
/// to the returned result when the corresponding element of \p pred is 0.
/// @return is a vector of type T and N * to_int<VS>()
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N,
          typename SurfIndAliasTy>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_load_merge_bti(
    __ESIMD_DNS::simd_mask_storage_t<N> pred,
    __ESIMD_DNS::vector_type_t<uint32_t, N> offsets, SurfIndAliasTy surf_ind,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> OldValues =
        0)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// Similar to __esimd_lsc_load_merge_bti(), but the argument OldValues is not
/// explicitly specified, which results into random values in those elements of
/// the returned result for which the corresponding element in \p pred is 0.
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N,
          typename SurfIndAliasTy>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_load_bti(__ESIMD_DNS::simd_mask_storage_t<N> pred,
                     __ESIMD_DNS::vector_type_t<uint32_t, N> offsets,
                     SurfIndAliasTy surf_ind)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  return __esimd_lsc_load_merge_bti<Ty, L1H, L3H, AddressScale, ImmOffset, DS,
                                    VS, _Transposed, N, SurfIndAliasTy>(
      pred, offsets, surf_ind);
}
#endif // __SYCL_DEVICE_ONLY__

/// USM pointer gather.
/// Supported platforms: DG2, PVC
///
/// Collects elements located at specified address and returns them
/// as a single \ref simd object.
///
/// @tparam Ty is element type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements to load per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @param pred is predicates.
/// @param addrs is the load addresses.
/// @param old_values is the vector of values copied to the result when the
/// corresponding element in \p pred is unset.
/// @return is a vector of type T and N * to_int<VS>()
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_load_merge_stateless(
    __ESIMD_DNS::simd_mask_storage_t<N> pred,
    __ESIMD_DNS::vector_type_t<uintptr_t, N> addrs,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> old_values =
        0)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// USM pointer gather.
/// Supported platforms: DG2, PVC
///
/// Collects elements located at specified address and returns them
/// as a single \ref simd object.
///
/// @tparam Ty is element type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements to load per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @param pred is predicates.
/// @param addrs is the load addresses.
/// @return is a vector of type T and N * to_int<VS>()
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_load_stateless(__ESIMD_DNS::simd_mask_storage_t<N> pred,
                           __ESIMD_DNS::vector_type_t<uintptr_t, N> addrs)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  return __esimd_lsc_load_merge_stateless<Ty, L1H, L3H, AddressScale, ImmOffset,
                                          DS, VS, _Transposed, N>(pred, addrs);
}
#endif // __SYCL_DEVICE_ONLY__

/// Surface-based prefetch gather.
/// Supported platforms: DG2, PVC
///
/// Prefetches elements located at surface.
///
/// @tparam Ty is element type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements to load per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @tparam SurfIndAliasTy is the \ref sycl::accessor type.
/// @param pred is predicates.
/// @param offsets is the zero-based offsets in bytes.
/// @param surf_ind is the surface index.
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N,
          typename SurfIndAliasTy>
__ESIMD_INTRIN void
__esimd_lsc_prefetch_bti(__ESIMD_DNS::simd_mask_storage_t<N> pred,
                         __ESIMD_DNS::vector_type_t<uint32_t, N> offsets,
                         SurfIndAliasTy surf_ind)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// USM pointer prefetch gather.
/// Supported platforms: DG2, PVC
///
/// Prefetches elements located at specified address.
///
/// @tparam Ty is element type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements to load per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @param pred is predicates.
/// @param addrs is the prefetch addresses.
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N>
__ESIMD_INTRIN void
__esimd_lsc_prefetch_stateless(__ESIMD_DNS::simd_mask_storage_t<N> pred,
                               __ESIMD_DNS::vector_type_t<uintptr_t, N> addrs)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// SLM scatter.
/// Supported platforms: DG2, PVC
///
/// Scatters elements located to slm.
///
/// @tparam Ty is element type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements to load per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @param pred is predicates.
/// @param offsets is the zero-based offsets for SLM buffer in bytes.
/// @param vals is values to store.
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N>
__ESIMD_INTRIN void __esimd_lsc_store_slm(
    __ESIMD_DNS::simd_mask_storage_t<N> pred,
    __ESIMD_DNS::vector_type_t<uint32_t, N> offsets,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> vals)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// Surface-based scatter.
/// Supported platforms: DG2, PVC
///
/// Scatters elements to surface.
///
/// @tparam Ty is element type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements to load per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @tparam SurfIndAliasTy is the \ref sycl::accessor type.
/// @param pred is predicates.
/// @param offsets is the zero-based offsets in bytes.
/// @param vals is values to store.
/// @param surf_ind is the surface index.
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N,
          typename SurfIndAliasTy>
__ESIMD_INTRIN void __esimd_lsc_store_bti(
    __ESIMD_DNS::simd_mask_storage_t<N> pred,
    __ESIMD_DNS::vector_type_t<uint32_t, N> offsets,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> vals,
    SurfIndAliasTy surf_ind)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// USM pointer scatter.
/// Supported platforms: DG2, PVC
///
/// Scatters elements to specific address.
///
/// @tparam Ty is element type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements to load per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @param pred is predicates.
/// @param addrs is the prefetch addresses.
/// @param vals is values to store.
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N>
__ESIMD_INTRIN void __esimd_lsc_store_stateless(
    __ESIMD_DNS::simd_mask_storage_t<N> pred,
    __ESIMD_DNS::vector_type_t<uintptr_t, N> addrs,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> vals)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// 2D USM pointer block load.
/// Supported platforms: PVC
///
/// Collects elements located at specified address and returns them
/// as a single \ref simd object.
///
/// @tparam Ty is element type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam DS is the data size.
/// @tparam Transposed is the transposed version or not.
/// @tparam NBlocks is the number of blocks.
/// @tparam BlockWidth is the block width in number of elements.
/// @tparam BlockHeight is the block height in number of elements.
/// @tparam Transformed is apply VNNI transform or not.
/// @tparam N is the data size
/// @param Pred is predicates.
/// @param Ptr is the surface base address for this operation.
/// @param SurfaceWidth is the surface width minus 1 in bytes
/// @param SurfaceHeight is the surface height minus 1 in rows
/// @param SurfacePitch is the surface pitch minus 1 in bytes
/// @param X is zero based X-coordinate of the left upper rectangle corner in
/// number of elements.
/// @param Y is zero based Y-coordinate of the left upper rectangle corner in
/// rows.
/// @return is a vector of type T and size N, where N is
///  BlockWidth * BlockHeight * NBlocks, if transformed;
///  otherwise,
///  N = roundUpNextMultiple(BlockHeight, 4 / sizeof(T)) *
///   getNextPowerOf2(BlockWidth) * NBlocks
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_data_order _Transposed, uint8_t NBlocks,
          int BlockWidth, int BlockHeight, bool Transformed, int N>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N>
__esimd_lsc_load2d_stateless(__ESIMD_DNS::simd_mask_storage_t<N> Pred,
                             uintptr_t Ptr, int SurfaceWidth, int SurfaceHeight,
                             int SurfacePitch, int X, int Y)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// 2D USM pointer block prefetch.
/// Supported platforms: PVC
///
/// Prefetches elements located at specified address.
///
/// @tparam Ty is element type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam DS is the data size.
/// @tparam NBlocks is the number of blocks.
/// @tparam Transposed is the transposed version or not.
/// @tparam BlockWidth is the block width in number of elements.
/// @tparam BlockHeight is the block height in number of elements.
/// @tparam Transformed is apply VNNI transform or not.
/// @tparam N is the data size
/// @param Pred is predicates.
/// @param Ptr is the surface base address for this operation.
/// @param SurfaceWidth is the surface width minus 1 in bytes
/// @param SurfaceHeight is the surface height minus 1 in rows
/// @param SurfacePitch is the surface pitch minus 1 in bytes
/// @param X is zero based X-coordinate of the left upper rectangle corner in
/// number of elements.
/// @param Y is zero based Y-coordinate of the left upper rectangle corner in
/// rows.
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_data_order _Transposed, uint8_t NBlocks,
          int BlockWidth, int BlockHeight, bool Transformed, int N>
__ESIMD_INTRIN void __esimd_lsc_prefetch2d_stateless(
    __ESIMD_DNS::simd_mask_storage_t<N> Pred, uintptr_t Ptr, int SurfaceWidth,
    int SurfaceHeight, int SurfacePitch, int X, int Y)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// 2D USM pointer block store.
/// Supported platforms: PVC
///
/// Stores elements at specified address.
///
/// @tparam Ty is element type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam DS is the data size.
/// @tparam Transposed is the transposed version or not.
/// @tparam NBlocks is the number of blocks.
/// @tparam BlockWidth is the block width in number of elements.
/// @tparam BlockHeight is the block height in number of elements.
/// @tparam Transformed is apply VNNI transform or not.
/// @tparam N is the data size
/// @param Pred is predicates.
/// @param Ptr is the surface base address for this operation.
/// @param SurfaceWidth is the surface width minus 1 in bytes
/// @param SurfaceHeight is the surface height minus 1 in rows
/// @param SurfacePitch is the surface pitch minus 1 in bytes
/// @param X is zero based X-coordinate of the left upper rectangle corner in
/// number of elements.
/// @param Y is zero based Y-coordinate of the left upper rectangle corner in
/// rows.
/// @param Vals is a vector to store of type T and size N, where N is
///  BlockWidth * BlockHeight * NBlocks, if transformed;
///  otherwise,
///  N = roundUpNextMultiple(BlockHeight, 4 / sizeof(T)) *
///   getNextPowerOf2(BlockWidth) * NBlocks
template <typename Ty, __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_data_order _Transposed, uint8_t NBlocks,
          int BlockWidth, int BlockHeight, bool Transformed, int N>
__ESIMD_INTRIN void
__esimd_lsc_store2d_stateless(__ESIMD_DNS::simd_mask_storage_t<N> Pred,
                              uintptr_t Ptr, int SurfaceWidth,
                              int SurfaceHeight, int SurfacePitch, int X, int Y,
                              __ESIMD_DNS::vector_type_t<Ty, N> vals)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// SLM atomic.
/// Supported platforms: DG2, PVC
///
/// @tparam Ty is element type.
/// @tparam Op is operation type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @param pred is predicates.
/// @param offsets is the zero-based offsets.
template <typename Ty, __ESIMD_NS::native::lsc::atomic_op Op,
          __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_xatomic_slm_0(__ESIMD_DNS::simd_mask_storage_t<N> pred,
                          __ESIMD_DNS::vector_type_t<uint32_t, N> offsets)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// SLM atomic.
/// Supported platforms: DG2, PVC
///
/// @tparam Ty is element type.
/// @tparam Op is operation type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @param pred is predicates.
/// @param offsets is the zero-based offsets.
/// @param src0 is the first atomic operand.
template <typename Ty, __ESIMD_NS::native::lsc::atomic_op Op,
          __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_xatomic_slm_1(
    __ESIMD_DNS::simd_mask_storage_t<N> pred,
    __ESIMD_DNS::vector_type_t<uint32_t, N> offsets,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> src0)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// SLM atomic.
/// Supported platforms: DG2, PVC
///
/// @tparam Ty is element type.
/// @tparam Op is operation type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @param pred is predicates.
/// @param offsets is the zero-based offsets.
/// @param src0 is the first atomic operand.
/// @param src1 is the second atomic operand.
template <typename Ty, __ESIMD_NS::native::lsc::atomic_op Op,
          __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_xatomic_slm_2(
    __ESIMD_DNS::simd_mask_storage_t<N> pred,
    __ESIMD_DNS::vector_type_t<uint32_t, N> offsets,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> src0,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> src1)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// Accessor-based atomic.
/// Supported platforms: DG2, PVC
///
/// @tparam Ty is element type.
/// @tparam Op is operation type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @tparam SurfIndAliasTy is the \ref sycl::accessor type.
/// @param pred is predicates.
/// @param offsets is the zero-based offsets.
/// @param surf_ind is the surface index.
template <typename Ty, __ESIMD_NS::native::lsc::atomic_op Op,
          __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N,
          typename SurfIndAliasTy>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_xatomic_bti_0(__ESIMD_DNS::simd_mask_storage_t<N> pred,
                          __ESIMD_DNS::vector_type_t<uint32_t, N> offsets,
                          SurfIndAliasTy surf_ind)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// Accessor-based atomic.
/// Supported platforms: DG2, PVC
///
/// @tparam Ty is element type.
/// @tparam Op is operation type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @tparam SurfIndAliasTy is the \ref sycl::accessor type.
/// @param pred is predicates.
/// @param offsets is the zero-based offsets.
/// @param src0 is the first atomic operand.
/// @param surf_ind is the surface index.
template <typename Ty, __ESIMD_NS::native::lsc::atomic_op Op,
          __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N,
          typename SurfIndAliasTy>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_xatomic_bti_1(
    __ESIMD_DNS::simd_mask_storage_t<N> pred,
    __ESIMD_DNS::vector_type_t<uint32_t, N> offsets,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> src0,
    SurfIndAliasTy surf_ind)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// Accessor-based atomic.
/// Supported platforms: DG2, PVC
///
/// @tparam Ty is element type.
/// @tparam Op is operation type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @tparam SurfIndAliasTy is the \ref sycl::accessor type.
/// @param pred is predicates.
/// @param offsets is the zero-based offsets.
/// @param src0 is the first atomic operand.
/// @param src1 is the second atomic operand.
/// @param surf_ind is the surface index.
template <typename Ty, __ESIMD_NS::native::lsc::atomic_op Op,
          __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N,
          typename SurfIndAliasTy>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_xatomic_bti_2(
    __ESIMD_DNS::simd_mask_storage_t<N> pred,
    __ESIMD_DNS::vector_type_t<uint32_t, N> offsets,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> src0,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> src1,
    SurfIndAliasTy surf_ind)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// USM pointer atomic.
/// Supported platforms: DG2, PVC
///
/// @tparam Ty is element type.
/// @tparam Op is operation type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @param pred is predicates.
/// @param addrs is the prefetch addresses.
template <typename Ty, __ESIMD_NS::native::lsc::atomic_op Op,
          __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_xatomic_stateless_0(__ESIMD_DNS::simd_mask_storage_t<N> pred,
                                __ESIMD_DNS::vector_type_t<uintptr_t, N> addrs)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// USM pointer atomic.
/// Supported platforms: DG2, PVC
///
/// @tparam Ty is element type.
/// @tparam Op is operation type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)

/// @param pred is predicates.
/// @param addrs is the prefetch addresses.
/// @param src0 is the first atomic operand.
template <typename Ty, __ESIMD_NS::native::lsc::atomic_op Op,
          __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_xatomic_stateless_1(
    __ESIMD_DNS::simd_mask_storage_t<N> pred,
    __ESIMD_DNS::vector_type_t<uintptr_t, N> addrs,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> src0)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// USM pointer atomic.
/// Supported platforms: DG2, PVC
///
/// @tparam Ty is element type.
/// @tparam Op is operation type.
/// @tparam L1H is L1 cache hint.
/// @tparam L3H is L3 cache hint.
/// @tparam AddressScale is the address scale.
/// @tparam ImmOffset is the immediate offset added to each address.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements per address.
/// @tparam Transposed indicates if the data is transposed during the transfer.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @param pred is predicates.
/// @param addrs is the prefetch addresses.
/// @param src0 is the first atomic operand.
/// @param src1 is the second atomic operand.
template <typename Ty, __ESIMD_NS::native::lsc::atomic_op Op,
          __ESIMD_ENS::cache_hint L1H, __ESIMD_ENS::cache_hint L3H,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()>
__esimd_lsc_xatomic_stateless_2(
    __ESIMD_DNS::simd_mask_storage_t<N> Pred,
    __ESIMD_DNS::vector_type_t<uintptr_t, N> Addrs,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> src0,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> src1)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// Memory fence.
/// Supported platforms: DG2, PVC
///
/// @tparam Kind is the Sfid shaded function.
/// @tparam FenceOp is the fence operation.
/// @tparam Scope is the operation scope.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
/// @param pred is predicates.
template <__ESIMD_ENS::lsc_memory_kind Kind, __ESIMD_ENS::lsc_fence_op FenceOp,
          __ESIMD_ENS::lsc_scope Scope, int N>
__ESIMD_INTRIN void __esimd_lsc_fence(__ESIMD_DNS::simd_mask_storage_t<N> pred)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

__ESIMD_INTRIN uint32_t __esimd_slm_alloc(uint32_t size)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

__ESIMD_INTRIN void __esimd_slm_free(uint32_t id)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/// @endcond ESIMD_DETAIL

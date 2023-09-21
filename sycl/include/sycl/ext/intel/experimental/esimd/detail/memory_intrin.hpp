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

#ifdef __SYCL_DEVICE_ONLY__
// Create an explicit data and GPU scoreboard dependency.
__ESIMD_INTRIN void __esimd_wait(uint16_t value);
#endif // __SYCL_DEVICE_ONLY__

// \brief Raw sends.
//
// @param modifier	the send message flags (Bit-0: isSendc, Bit-1: isEOT).
//
// @param execSize the execution size, which must be a compile time constant.
//
// @param pred the predicate to specify enabled channels.
//
// @param numSrc0 the number of GRFs for source-0, which must be a compile time
// constant.
//
// @param numSrc1 the number of GRFs for source-1, which must be a compile time
// constant.
//
// @param numDst the number of GRFs for destination, which must be a compile
// time constant.
//
// @param sfid the shared function ID, which must be a compile time constant.
//
// @param exDesc the extended message descriptor.
//
// @param msgDesc the message descriptor.
//
// @param msgSrc0 the first source operand of send message.
//
// @param msgSrc1 the second source operand of send message.
//
// @param msgDst the destination operand of send message.
//
// Returns a simd vector of type Ty1 and size N1.
//
template <typename Ty1, int N1, typename Ty2, int N2, typename Ty3, int N3,
          int N = 16>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty1, N1>
__esimd_raw_sends2(uint8_t modifier, uint8_t execSize,
                   __ESIMD_DNS::simd_mask_storage_t<N> pred, uint8_t numSrc0,
                   uint8_t numSrc1, uint8_t numDst, uint8_t sfid,
                   uint32_t exDesc, uint32_t msgDesc,
                   __ESIMD_DNS::vector_type_t<Ty2, N2> msgSrc0,
                   __ESIMD_DNS::vector_type_t<Ty3, N3> msgSrc1,
                   __ESIMD_DNS::vector_type_t<Ty1, N1> msgDst)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Raw send.
//
// @param modifier	the send message flags (Bit-0: isSendc, Bit-1: isEOT).
//
// @param execSize the execution size, which must be a compile time constant.
//
// @param pred the predicate to specify enabled channels.
//
// @param numSrc0 the number of GRFs for source-0, which must be a compile time
// constant.
//
// @param numDst the number of GRFs for destination, which must be a compile
// time constant.
//
// @param sfid the shared function ID, which must be a compile time constant.
//
// @param exDesc the extended message descriptor.
//
// @param msgDesc the message descriptor.
//
// @param msgSrc0 the first source operand of send message.
//
// @param msgDst the destination operand of send message.
//
// Returns a simd vector of type Ty1 and size N1.
//
template <typename Ty1, int N1, typename Ty2, int N2, int N = 16>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<Ty1, N1>
__esimd_raw_send2(uint8_t modifier, uint8_t execSize,
                  __ESIMD_DNS::simd_mask_storage_t<N> pred, uint8_t numSrc0,
                  uint8_t numDst, uint8_t sfid, uint32_t exDesc,
                  uint32_t msgDesc, __ESIMD_DNS::vector_type_t<Ty2, N2> msgSrc0,
                  __ESIMD_DNS::vector_type_t<Ty1, N1> msgDst)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Raw sends.
//
// @param modifier	the send message flags (Bit-0: isSendc, Bit-1: isEOT).
//
// @param execSize the execution size, which must be a compile time constant.
//
// @param pred the predicate to specify enabled channels.
//
// @param numSrc0 the number of GRFs for source-0, which must be a compile time
// constant.
//
// @param numSrc1 the number of GRFs for source-1, which must be a compile time
// constant.
//
// @param sfid the shared function ID, which must be a compile time constant.
//
// @param exDesc the extended message descriptor.
//
// @param msgDesc the message descriptor.
//
// @param msgSrc0 the first source operand of send message.
//
// @param msgSrc1 the second source operand of send message.
//
template <typename Ty1, int N1, typename Ty2, int N2, int N = 16>
__ESIMD_INTRIN void
__esimd_raw_sends2_noresult(uint8_t modifier, uint8_t execSize,
                            __ESIMD_DNS::simd_mask_storage_t<N> pred,
                            uint8_t numSrc0, uint8_t numSrc1, uint8_t sfid,
                            uint32_t exDesc, uint32_t msgDesc,
                            __ESIMD_DNS::vector_type_t<Ty1, N1> msgSrc0,
                            __ESIMD_DNS::vector_type_t<Ty2, N2> msgSrc1)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

// \brief Raw send.
//
// @param modifier	the send message flags (Bit-0: isSendc, Bit-1: isEOT).
//
// @param execSize the execution size, which must be a compile time constant.
//
// @param pred the predicate to specify enabled channels.
//
// @param numSrc0 the number of GRFs for source-0, which must be a compile time
// constant.
//
// @param sfid the shared function ID, which must be a compile time constant.
//
// @param exDesc the extended message descriptor.
//
// @param msgDesc the message descriptor.
//
// @param msgSrc0 the first source operand of send message.
//
template <typename Ty1, int N1, int N = 16>
__ESIMD_INTRIN void
__esimd_raw_send2_noresult(uint8_t modifier, uint8_t execSize,
                           __ESIMD_DNS::simd_mask_storage_t<N> pred,
                           uint8_t numSrc0, uint8_t sfid, uint32_t exDesc,
                           uint32_t msgDesc,
                           __ESIMD_DNS::vector_type_t<Ty1, N1> msgSrc0)
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

<<<<<<< HEAD
/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ESIMD_EMBARGO */

// Wait for val to be ready
__ESIMD_INTRIN void __esimd_wait(uint16_t val)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else  // __SYCL_DEVICE_ONLY__
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

/* end INTEL_FEATURE_ESIMD_EMBARGO */
/* end INTEL_CUSTOMIZATION */

#ifndef __SYCL_DEVICE_ONLY__
// Shared utility/helper functions for LSC support under emulation
// (ESIMD_EMULATOR backend)

// Raw-address increment function for u8u32 and u16u32
template <typename Ty, __ESIMD_ENS::lsc_data_size DS>
constexpr uint32_t rawAddressIncrement() {
  if constexpr (DS == __ESIMD_ENS::lsc_data_size::u8u32) {
    return 1;
  } else if constexpr (DS == __ESIMD_ENS::lsc_data_size::u16u32) {
    return 2;
  } else {
    return (uint32_t)sizeof(Ty);
  }
}

// Vector index increment function for 'Transposed' 2D-surface access
template <int N, __ESIMD_EDNS::lsc_data_order _Transposed>
constexpr int vectorIndexIncrement() {
  if constexpr (_Transposed == __ESIMD_EDNS::lsc_data_order::transpose) {
    return 1;
  } else {
    return N;
  }
}

// Load/Store align bitmask generator for 1-D vector load/store
//
// Not only generates address-align bitmask, but also checks
// legitimacy of load/store operation with respect to vector size,
// data size
/// @tparam Ty is element type.
/// @tparam DS is the data size.
/// @tparam VS is the number of elements to load per address.
/// @tparam N is the SIMD size of operation (the number of addresses to access)
template <typename Ty, __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_ENS::lsc_data_size DS, int N>
constexpr unsigned loadstoreAlignMask() {
  constexpr __ESIMD_ENS::lsc_data_size _DS =
      __ESIMD_EDNS::finalize_data_size<Ty, DS>(); // Actual data_size

  if constexpr (VS == __ESIMD_EDNS::lsc_vector_size::n1) {
    static_assert(((_DS == __ESIMD_ENS::lsc_data_size::u32) ||
                   (_DS == __ESIMD_ENS::lsc_data_size::u64) ||
                   (_DS == __ESIMD_ENS::lsc_data_size::u8) ||
                   (_DS == __ESIMD_ENS::lsc_data_size::u16) ||
                   (_DS == __ESIMD_ENS::lsc_data_size::u8u32) ||
                   (_DS == __ESIMD_ENS::lsc_data_size::u16u32)) &&
                  "Wrong __ESIMD_EDNS::lsc_data_size for "
                  "__ESIMD_EDNS::lsc_vector_size == 1\n"
                  "(loadstoreAlignMask)");
    return 0x0;
  } else if constexpr ((VS == __ESIMD_EDNS::lsc_vector_size::n2) ||
                       (VS == __ESIMD_EDNS::lsc_vector_size::n3) ||
                       (VS == __ESIMD_EDNS::lsc_vector_size::n4) ||
                       (VS == __ESIMD_EDNS::lsc_vector_size::n8)) {
    static_assert(
        ((_DS == __ESIMD_ENS::lsc_data_size::u32) ||
         (_DS == __ESIMD_ENS::lsc_data_size::u64)) &&
        "Wrong Data Size for __ESIMD_EDNS::lsc_vector_size == 2/3/4/8\n"
        "(loadstoreAlignMask)");
    // 0x3 for u32 / 0x7 for u64
    if constexpr (_DS == __ESIMD_ENS::lsc_data_size::u32)
      return 0x3;
    else
      return 0x7;
  } else if constexpr ((VS == __ESIMD_EDNS::lsc_vector_size::n16) ||
                       (VS == __ESIMD_EDNS::lsc_vector_size::n32) ||
                       (VS == __ESIMD_EDNS::lsc_vector_size::n64)) {
    static_assert(
        (N == 1) &&
        "Unsupported Size for __ESIMD_EDNS::lsc_vector_size = 16/32/64\n"
        "(loadstoreAlignMask)");
    // 0x3 for u32 / 0x7 for u64
    if constexpr (_DS == __ESIMD_ENS::lsc_data_size::u32)
      return 0x3;
    else
      return 0x7;
  } else {
    static_assert((N != N) && "Wrong Vector Size!!");
  }
}

// Helper function for loading from indexed-surface and SLM
// INT_MAX is for SLM
template <typename Ty, uint16_t AddressScale, int ImmOffset,
          __ESIMD_ENS::lsc_data_size DS, __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N, uint32_t MASK>
auto __esimd_emu_lsc_offset_read_merge(
    __ESIMD_DNS::simd_mask_storage_t<N> Pred,
    __ESIMD_DNS::vector_type_t<uint32_t, N> Offsets, char *ReadBase,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> OldValues,
    int BufByteWidth = INT_MAX) {
  // TODO : Support AddressScale, ImmOffset
  static_assert(AddressScale == 1);
  static_assert(ImmOffset == 0);
  static_assert(DS != __ESIMD_ENS::lsc_data_size::u16u32h);

  auto Output = OldValues;

  constexpr int ChanlCount = __ESIMD_EDNS::to_int<VS>();

  for (int OffsetIdx = 0; OffsetIdx < N; OffsetIdx += 1) {
    if (Pred[OffsetIdx] == 0) {
      // Skip Output vector elements correpsonding to
      // predicates whose value is zero
      continue;
    }

    assert(((Offsets[OffsetIdx] & MASK)) == 0 && "Offset Alignment Error!!");

    // ByteDistance : byte-distance from buffer-read base
    int ByteDistance = Offsets[OffsetIdx];

    for (int ChanelIdx = 0, VecIdx = OffsetIdx; ChanelIdx < ChanlCount;
         ChanelIdx += 1, ByteDistance += rawAddressIncrement<Ty, DS>(),
             VecIdx += vectorIndexIncrement<N, _Transposed>()) {

      if ((ByteDistance >= 0) && (ByteDistance < BufByteWidth)) {
        Output[VecIdx] = *((Ty *)(ReadBase + ByteDistance));
        if constexpr (DS == __ESIMD_ENS::lsc_data_size::u8u32)
          Output[VecIdx] &= 0xff;
        else if constexpr (DS == __ESIMD_ENS::lsc_data_size::u16u32)
          Output[VecIdx] &= 0xffff;
      }
    }
  }
  return Output;
}

// Helper function for storing to indexed-surface and SLM. INT_MAX is
// for SLM
template <typename Ty, uint16_t AddressScale, int ImmOffset,
          __ESIMD_ENS::lsc_data_size DS, __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N, uint32_t MASK>
void __esimd_emu_lsc_offset_write(
    __ESIMD_DNS::simd_mask_storage_t<N> Pred,
    __ESIMD_DNS::vector_type_t<uint32_t, N> Offsets,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> vals,
    char *WriteBase, int BufByteWidth = INT_MAX) {
  // TODO : Support AddressScale, ImmOffset
  static_assert(AddressScale == 1);
  static_assert(ImmOffset == 0);
  static_assert(DS != __ESIMD_ENS::lsc_data_size::u16u32h);

  using StoreType = typename std::conditional_t<
      DS == __ESIMD_ENS::lsc_data_size::u8, uint8_t,
      std::conditional_t<
          DS == __ESIMD_ENS::lsc_data_size::u16, uint16_t,
          std::conditional_t<
              DS == __ESIMD_ENS::lsc_data_size::u32, uint32_t,
              std::conditional_t<
                  DS == __ESIMD_ENS::lsc_data_size::u64, uint64_t,
                  std::conditional_t<
                      DS == __ESIMD_ENS::lsc_data_size::u8u32, uint8_t,
                      std::conditional_t<DS ==
                                             __ESIMD_ENS::lsc_data_size::u16u32,
                                         uint16_t, void>>>>>>;
  for (int OffsetIdx = 0; OffsetIdx < N; OffsetIdx += 1) {
    if (Pred[OffsetIdx] == 0) {
      // Skip input vector elements correpsonding to
      // predicates whose value is zero
      continue;
    }

    assert(((Offsets[OffsetIdx] & MASK)) == 0 && "Offset Alignment Error!!");

    // ByteDistance : byte-distance from buffer-write base
    int ByteDistance = Offsets[OffsetIdx];
    constexpr int ChanlCount = __ESIMD_EDNS::to_int<VS>();

    for (int ChanelIdx = 0, VecIdx = OffsetIdx; ChanelIdx < ChanlCount;
         ChanelIdx += 1, ByteDistance += rawAddressIncrement<Ty, DS>(),
             VecIdx += vectorIndexIncrement<N, _Transposed>()) {

      if ((ByteDistance >= 0) && (ByteDistance < BufByteWidth)) {
        if constexpr (std::is_floating_point<Ty>::value) {
          *((StoreType *)(WriteBase + ByteDistance)) =
              sycl::bit_cast<StoreType>(vals[VecIdx]);
        } else {
          *((StoreType *)(WriteBase + ByteDistance)) = vals[VecIdx];
        }
      }
    }
  }
}

/// Generic helper function of 2D Block Read supporting both 2d-load
/// and raw_send
template <typename Ty, int N>
__ESIMD_DNS::vector_type_t<Ty, N>
__esimd_emu_read_2d(__ESIMD_DNS::simd_mask_storage_t<N> Pred, uintptr_t Ptr,
                    unsigned SurfaceWidth, unsigned SurfaceHeight,
                    unsigned SurfacePitch, int X, int Y, int Width, int Height,
                    int NBlks, __ESIMD_EDNS::lsc_data_order _Transposed,
                    bool Transformed) {
  assert(SurfaceHeight >= 0);
  assert(SurfaceWidth >= 0);
  assert(SurfaceWidth <= SurfacePitch);

  SurfaceHeight += 1;
  SurfaceWidth += 1;
  SurfacePitch += 1;

  constexpr unsigned sizeofTy = sizeof(Ty);

  __ESIMD_DNS::vector_type_t<Ty, N> Output = 0;

  char *buff = (char *)Ptr;
  assert(buff != NULL);

  int vecIdx = 0;
  int blkCount = 0;

  for (int xBase = X * sizeofTy; blkCount < NBlks; xBase += sizeofTy * Width) {
    if (Transformed == true) {
      constexpr int elems_per_DW = (sizeofTy == 1) ? 4 : 2; /// VNNI_pack
      int yRead = Y * SurfacePitch;
      for (int u = 0; u < Height;
           u += elems_per_DW, yRead += SurfacePitch * elems_per_DW) {
        vecIdx = u * sycl::detail::getNextPowerOfTwo(Width) +
                 blkCount * Height * sycl::detail::getNextPowerOfTwo(Width);
        if ((yRead < 0) || (yRead >= SurfacePitch * SurfaceHeight)) {
          /// Vertically out-of-bound, skip corresponding vector elements
          vecIdx += Width * elems_per_DW;
          continue;
        }

        int xRead = xBase;
        for (int v = 0; v < Width; v += 1, xRead += sizeofTy) {
          if ((xRead < 0) || (xRead >= SurfaceWidth)) {
            /// Horizontally out-of-bound, skip corresponding vector elements
            vecIdx += elems_per_DW;
            continue;
          }

          char *base = buff + xRead;
          int offset = yRead;
          for (int k = 0; k < elems_per_DW; k++, vecIdx += 1) {
            if (Pred[vecIdx] != 0) {
              if (offset >= 0 && offset < SurfacePitch * SurfaceHeight) {
                Output[vecIdx] = *((Ty *)(base + offset));
              }
            }
            // Increasing in Y-direction
            offset += SurfacePitch;
          } // k loop
        }   // v loop
      }     // u loop
    }       // (Transformed == true)
    else if (_Transposed == __ESIMD_EDNS::lsc_data_order::transpose) {
      int xRead = xBase;
      for (int v = 0; v < Width; v += 1, xRead += sizeofTy) {
        if ((xRead < 0) || (xRead >= SurfaceWidth)) {
          // Horizontally out-of-bound, skip corresponding vector elements
          vecIdx += Height;
          continue;
        }

        int yRead = Y * SurfacePitch;
        for (int u = 0; u < Height;
             u += 1, yRead += SurfacePitch, vecIdx += 1) {
          if (Pred[vecIdx] != 0) {
            if ((yRead >= 0) && (yRead < SurfacePitch * SurfaceHeight)) {
              Output[vecIdx] = *((Ty *)(buff + yRead + xRead));
            }
          }
        } // u loop
      }   // v loop
    }     // (_Transposed == __ESIMD_EDNS::lsc_data_order::transpose)
    else {
      int yRead = Y * SurfacePitch;
      for (int u = 0; u < Height; u += 1, yRead += SurfacePitch) {
        if ((yRead < 0) || (yRead >= SurfacePitch * SurfaceHeight)) {
          // Vertically Out-of-bound, skip corresponding vector elements
          vecIdx += Width;
          continue;
        }

        int xRead = xBase;
        for (int v = 0; v < Width; v += 1, xRead += sizeofTy, vecIdx += 1) {
          if (Pred[vecIdx] != 0) {
            if ((xRead >= 0) && (xRead < SurfaceWidth)) {
              Output[vecIdx] = *((Ty *)(buff + yRead + xRead));
            }
          }
        } // v loop
      }   // u loop
    }     // Linear loading
    blkCount += 1;
    vecIdx = blkCount * sycl::detail::getNextPowerOfTwo(Width) * Height;
  } // xBase loop

  return Output;
}

/// Generic helper function of 2D Block Write supporting both
/// 2d-write and raw_send
template <typename Ty, int N>
void __esimd_emu_write_2d(__ESIMD_DNS::simd_mask_storage_t<N> Pred,
                          uintptr_t Ptr, unsigned SurfaceWidth,
                          unsigned SurfaceHeight, unsigned SurfacePitch, int X,
                          int Y, __ESIMD_DNS::vector_type_t<Ty, N> vals,
                          int Width, int Height) {
  assert(SurfaceHeight >= 0);
  assert(SurfaceWidth >= 0);
  assert(SurfaceWidth <= SurfacePitch);

  SurfaceHeight += 1;
  SurfaceWidth += 1;
  SurfacePitch += 1;

  constexpr unsigned sizeofTy = sizeof(Ty);

  char *buff = (char *)Ptr;
  assert(buff != NULL);

  int vecIdx = 0;
  int rowCount = 0;
  for (int yWrite = Y * SurfacePitch; rowCount < Height;
       yWrite += SurfacePitch) {
    if (yWrite == SurfacePitch * SurfaceHeight) {
      // Vertically Out-of-bound
      break;
    }
    int writeCount = 0;
    for (int xWrite = X * sizeofTy; writeCount < Width;
         xWrite += sizeofTy, vecIdx += 1, writeCount += 1) {
      if (xWrite >= 0 && xWrite < SurfaceWidth && Pred[vecIdx] != 0) {
        *((Ty *)(buff + yWrite + xWrite)) = vals[vecIdx];
      }
    } // xWrite loop
    rowCount += 1;
  } // yWrite loop
}

/// Helper function for zero-source LSC-atomic operation accessing BTI
/// or SLM
template <typename Ty, __ESIMD_NS::native::lsc::atomic_op Op,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N, uint32_t MASK>
auto __esimd_emu_lsc_xatomic_offset_access_0(
    __ESIMD_DNS::simd_mask_storage_t<N> Pred,
    __ESIMD_DNS::vector_type_t<uint32_t, N> Offsets, const char *BaseAddr,
    const int BufByteWidth) {

  assert(BaseAddr != nullptr &&
         "Invalid BaseAddr for lsc_xatomic_operation under emulation!!");

  __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> Oldval = 0;

  for (int OffsetIdx = 0; OffsetIdx < N; OffsetIdx += 1) {
    if (Pred[OffsetIdx] == 0) {
      // Skip Oldval vector elements correpsonding to
      // predicates whose value is zero
      continue;
    }

    assert(((Offsets[OffsetIdx] & MASK)) == 0 && "Offset Alignment Error!!");

    // ByteDistance : byte-distance from buffer-access base
    int ByteDistance = Offsets[OffsetIdx];
    constexpr int ChanlCount = __ESIMD_EDNS::to_int<VS>();

    for (int ChanelIdx = 0, VecIdx = OffsetIdx; ChanelIdx < ChanlCount;
         ChanelIdx += 1, ByteDistance += rawAddressIncrement<Ty, DS>(),
             VecIdx += vectorIndexIncrement<N, _Transposed>()) {

      if ((ByteDistance >= 0) && (ByteDistance < BufByteWidth)) {
        if constexpr (Op == __ESIMD_NS::native::lsc::atomic_op::load) {
          Oldval[VecIdx] =
              __ESIMD_DNS::atomic_load<Ty>((Ty *)(BaseAddr + ByteDistance));
        } else if constexpr (Op == __ESIMD_NS::native::lsc::atomic_op::inc) {
          Oldval[VecIdx] = __ESIMD_DNS::atomic_add<Ty>(
              (Ty *)(BaseAddr + ByteDistance), static_cast<Ty>(1));
        } else if constexpr (Op == __ESIMD_NS::native::lsc::atomic_op::dec) {
          Oldval[VecIdx] = __ESIMD_DNS::atomic_sub<Ty>(
              (Ty *)(BaseAddr + ByteDistance), static_cast<Ty>(1));
        }
      }
    }
  }
  return Oldval;
}

/// Helper function for one-source LSC-atomic operation accessing BTI
/// or SLM
template <typename Ty, __ESIMD_NS::native::lsc::atomic_op Op,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N, uint32_t MASK>
auto __esimd_emu_lsc_xatomic_offset_access_1(
    __ESIMD_DNS::simd_mask_storage_t<N> Pred,
    __ESIMD_DNS::vector_type_t<uint32_t, N> Offsets, const char *BaseAddr,
    const int BufByteWidth,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> src0) {

  assert(BaseAddr != nullptr &&
         "Invalid BaseAddr for lsc_xatomic_operation under emulation!!");

  __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> Oldval = 0;

  static_assert(AddressScale == 1);
  static_assert(ImmOffset == 0);
  static_assert(DS != __ESIMD_ENS::lsc_data_size::u16u32h);

  for (int OffsetIdx = 0; OffsetIdx < N; OffsetIdx += 1) {
    if (Pred[OffsetIdx] == 0) {
      // Skip input vector elements correpsonding to
      // predicates whose value is zero
      continue;
    }

    assert(((Offsets[OffsetIdx] & MASK)) == 0 && "Offset Alignment Error!!");

    // ByteDistance : byte-distance from buffer-write base
    int ByteDistance = Offsets[OffsetIdx];
    constexpr int ChanlCount = __ESIMD_EDNS::to_int<VS>();

    for (int ChanelIdx = 0, VecIdx = OffsetIdx; ChanelIdx < ChanlCount;
         ChanelIdx += 1, ByteDistance += rawAddressIncrement<Ty, DS>(),
             VecIdx += vectorIndexIncrement<N, _Transposed>()) {

      if ((ByteDistance >= 0) && (ByteDistance < BufByteWidth)) {
        if constexpr (Op == __ESIMD_NS::native::lsc::atomic_op::store) {
          Oldval[VecIdx] = __ESIMD_DNS::atomic_store<Ty>(
              (Ty *)(BaseAddr + ByteDistance), src0[VecIdx]);
        } else if constexpr ((Op == __ESIMD_NS::native::lsc::atomic_op::add) ||
                             (Op == __ESIMD_NS::native::lsc::atomic_op::fadd)) {
          Oldval[VecIdx] = __ESIMD_DNS::atomic_add<Ty>(
              (Ty *)(BaseAddr + ByteDistance), src0[VecIdx]);
        } else if constexpr ((Op == __ESIMD_NS::native::lsc::atomic_op::sub) ||
                             (Op == __ESIMD_NS::native::lsc::atomic_op::fsub)) {
          Oldval[VecIdx] = __ESIMD_DNS::atomic_sub<Ty>(
              (Ty *)(BaseAddr + ByteDistance), src0[VecIdx]);
        } else if constexpr ((Op == __ESIMD_NS::native::lsc::atomic_op::smin) ||
                             (Op == __ESIMD_NS::native::lsc::atomic_op::umin) ||
                             (Op == __ESIMD_NS::native::lsc::atomic_op::fmin)) {
          Oldval[VecIdx] = __ESIMD_DNS::atomic_min<Ty>(
              (Ty *)(BaseAddr + ByteDistance), src0[VecIdx]);
        } else if constexpr ((Op == __ESIMD_NS::native::lsc::atomic_op::smax) ||
                             (Op == __ESIMD_NS::native::lsc::atomic_op::umax) ||
                             (Op == __ESIMD_NS::native::lsc::atomic_op::fmax)) {
          Oldval[VecIdx] = __ESIMD_DNS::atomic_max<Ty>(
              (Ty *)(BaseAddr + ByteDistance), src0[VecIdx]);
        } else if constexpr (Op ==
                             __ESIMD_NS::native::lsc::atomic_op::bit_and) {
          Oldval[VecIdx] = __ESIMD_DNS::atomic_and<Ty>(
              (Ty *)(BaseAddr + ByteDistance), src0[VecIdx]);
        } else if constexpr (Op == __ESIMD_NS::native::lsc::atomic_op::bit_or) {
          Oldval[VecIdx] = __ESIMD_DNS::atomic_or<Ty>(
              (Ty *)(BaseAddr + ByteDistance), src0[VecIdx]);
        } else if constexpr (Op ==
                             __ESIMD_NS::native::lsc::atomic_op::bit_xor) {
          Oldval[VecIdx] = __ESIMD_DNS::atomic_xor<Ty>(
              (Ty *)(BaseAddr + ByteDistance), src0[VecIdx]);
        }
      }
    }
  }
  return Oldval;
}

/// Helper function for two-source LSC-atomic operation accessing BTI
/// or SLM
template <typename Ty, __ESIMD_NS::native::lsc::atomic_op Op,
          uint16_t AddressScale, int ImmOffset, __ESIMD_ENS::lsc_data_size DS,
          __ESIMD_EDNS::lsc_vector_size VS,
          __ESIMD_EDNS::lsc_data_order _Transposed, int N, uint32_t MASK>
auto __esimd_emu_lsc_xatomic_offset_access_2(
    __ESIMD_DNS::simd_mask_storage_t<N> Pred,
    __ESIMD_DNS::vector_type_t<uint32_t, N> Offsets, const char *BaseAddr,
    const int BufByteWidth,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> src0,
    __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> src1) {

  assert(BaseAddr != nullptr &&
         "Invalid BaseAddr for lsc_xatomic_operation under emulation!!");

  __ESIMD_DNS::vector_type_t<Ty, N * __ESIMD_EDNS::to_int<VS>()> Oldval;

  static_assert(AddressScale == 1);
  static_assert(ImmOffset == 0);
  static_assert(DS != __ESIMD_ENS::lsc_data_size::u16u32h);

  for (int OffsetIdx = 0; OffsetIdx < N; OffsetIdx += 1) {
    if (Pred[OffsetIdx] == 0) {
      // Skip input vector elements correpsonding to
      // predicates whose value is zero
      continue;
    }

    assert(((Offsets[OffsetIdx] & MASK)) == 0 && "Offset Alignment Error!!");

    // ByteDistance : byte-distance from buffer-write base
    int ByteDistance = Offsets[OffsetIdx];
    constexpr int ChanlCount = __ESIMD_EDNS::to_int<VS>();

    for (int ChanelIdx = 0, VecIdx = OffsetIdx; ChanelIdx < ChanlCount;
         ChanelIdx += 1, ByteDistance += rawAddressIncrement<Ty, DS>(),
             VecIdx += vectorIndexIncrement<N, _Transposed>()) {

      if ((ByteDistance >= 0) && (ByteDistance < BufByteWidth)) {
        if constexpr (Op == __ESIMD_NS::native::lsc::atomic_op::cmpxchg ||
                      Op == __ESIMD_NS::native::lsc::atomic_op::fcmpxchg) {
          Oldval[VecIdx] = __ESIMD_DNS::atomic_cmpxchg(
              (Ty *)(BaseAddr + ByteDistance), src0[VecIdx], src1[VecIdx]);
        }
      }
    }
  }
  return Oldval;
}

// End : Shared utility/helper functions for LSC support under
// emulation
#endif // __SYCL_DEVICE_ONLY__

=======
>>>>>>> 3b0a1db918cb956a6cf32a6ab506627c2c6f5613
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

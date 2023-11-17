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
__esimd_raw_sendg(__ESIMD_DNS::simd_mask_storage_t<exec_size> pred,
                  __ESIMD_DNS::vector_type_t<T0, N0> msg_src0,
                  __ESIMD_DNS::vector_type_t<T1, N1> msg_src1, uint64_t ind0,
                  uint64_t ind1, __ESIMD_DNS::vector_type_t<T2, N2> msg_dst)
#ifdef __SYCL_DEVICE_ONLY__
    ;
#else
{
  __ESIMD_UNSUPPORTED_ON_HOST;
}
#endif // __SYCL_DEVICE_ONLY__

template <typename T>
__ESIMD_INTRIN __ESIMD_DNS::vector_type_t<uint32_t, 16>
__esimd_packed_4bit_upconvert_lut(
    __ESIMD_DNS::vector_type_t<uint32_t, 16> lookup_table,
    __ESIMD_DNS::vector_type_t<T, 16> src)
#ifdef __SYCL_DEVICE_ONLY__
    ;
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
/// @tparam InternalOp is operation type.
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
template <typename Ty, int InternalOpOp, __ESIMD_ENS::cache_hint L1H,
          __ESIMD_ENS::cache_hint L3H, uint16_t AddressScale, int ImmOffset,
          __ESIMD_ENS::lsc_data_size DS, __ESIMD_EDNS::lsc_vector_size VS,
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
/// @tparam InternalOp is operation type.
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
template <typename Ty, int InternalOp, __ESIMD_ENS::cache_hint L1H,
          __ESIMD_ENS::cache_hint L3H, uint16_t AddressScale, int ImmOffset,
          __ESIMD_ENS::lsc_data_size DS, __ESIMD_EDNS::lsc_vector_size VS,
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
/// @tparam InternalOp is operation type.
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
template <typename Ty, int InternalOp, __ESIMD_ENS::cache_hint L1H,
          __ESIMD_ENS::cache_hint L3H, uint16_t AddressScale, int ImmOffset,
          __ESIMD_ENS::lsc_data_size DS, __ESIMD_EDNS::lsc_vector_size VS,
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
/// @tparam InternalOp is operation type.
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
template <typename Ty, int InternalOp, __ESIMD_ENS::cache_hint L1H,
          __ESIMD_ENS::cache_hint L3H, uint16_t AddressScale, int ImmOffset,
          __ESIMD_ENS::lsc_data_size DS, __ESIMD_EDNS::lsc_vector_size VS,
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
/// @tparam InternalOp is operation type.
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
template <typename Ty, int InternalOp, __ESIMD_ENS::cache_hint L1H,
          __ESIMD_ENS::cache_hint L3H, uint16_t AddressScale, int ImmOffset,
          __ESIMD_ENS::lsc_data_size DS, __ESIMD_EDNS::lsc_vector_size VS,
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
/// @tparam InternalOp is operation type.
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
template <typename Ty, int InternalOp, __ESIMD_ENS::cache_hint L1H,
          __ESIMD_ENS::cache_hint L3H, uint16_t AddressScale, int ImmOffset,
          __ESIMD_ENS::lsc_data_size DS, __ESIMD_EDNS::lsc_vector_size VS,
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

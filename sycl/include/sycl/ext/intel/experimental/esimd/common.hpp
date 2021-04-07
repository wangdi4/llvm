//==---------------- common.hpp - DPC++ Explicit SIMD API   ----------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// definitions used in Explicit SIMD APIs.
//===----------------------------------------------------------------------===//

#pragma once

#include <CL/sycl/detail/defines.hpp>

#include <cstdint> // for uint* types

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {
namespace ext {
namespace intel {
namespace experimental {
namespace esimd {

using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;

#ifdef __SYCL_DEVICE_ONLY__
// Mark a function being nodebug.
#define ESIMD_NODEBUG __attribute__((nodebug))
// Mark a "ESIMD global": accessible from all functions in current translation
// unit, separate copy per subgroup (work-item), mapped to SPIR-V private
// storage class.
#define ESIMD_PRIVATE                                                          \
  __attribute__((opencl_private)) __attribute__((sycl_explicit_simd))
// Bind a ESIMD global variable to a specific register.
#define ESIMD_REGISTER(n) __attribute__((register_num(n)))
#else
// TODO ESIMD define what this means on Windows host
#define ESIMD_NODEBUG
// On host device ESIMD global is a thread local static var. This assumes that
// each work-item is mapped to a separate OS thread on host device.
#define ESIMD_PRIVATE thread_local
#define ESIMD_REGISTER(n)
#endif

// Mark a function being noinline
#define ESIMD_NOINLINE __attribute__((noinline))
// Force a function to be inlined. 'inline' is used to preserve ODR for
// functions defined in a header.
#define ESIMD_INLINE inline __attribute__((always_inline))

// Enums
// TODO FIXME convert the two enums below to nested enum or class enum to
// remove enum values from the global namespace
enum { GENX_NOSAT = 0, GENX_SAT };

enum ChannelMaskType {
  ESIMD_R_ENABLE = 1,
  ESIMD_G_ENABLE = 2,
  ESIMD_GR_ENABLE = 3,
  ESIMD_B_ENABLE = 4,
  ESIMD_BR_ENABLE = 5,
  ESIMD_BG_ENABLE = 6,
  ESIMD_BGR_ENABLE = 7,
  ESIMD_A_ENABLE = 8,
  ESIMD_AR_ENABLE = 9,
  ESIMD_AG_ENABLE = 10,
  ESIMD_AGR_ENABLE = 11,
  ESIMD_AB_ENABLE = 12,
  ESIMD_ABR_ENABLE = 13,
  ESIMD_ABG_ENABLE = 14,
  ESIMD_ABGR_ENABLE = 15
};

#define NumChannels(Mask)                                                      \
  ((Mask & 1) + ((Mask & 2) >> 1) + ((Mask & 4) >> 2) + ((Mask & 8) >> 3))

#define HasR(Mask) ((Mask & 1) == 1)
#define HasG(Mask) ((Mask & 2) >> 1 == 1)
#define HasB(Mask) ((Mask & 4) >> 2 == 1)
#define HasA(Mask) ((Mask & 8) >> 3 == 1)

enum class EsimdAtomicOpType : uint16_t {
  ATOMIC_ADD = 0x0,
  ATOMIC_SUB = 0x1,
  ATOMIC_INC = 0x2,
  ATOMIC_DEC = 0x3,
  ATOMIC_MIN = 0x4,
  ATOMIC_MAX = 0x5,
  ATOMIC_XCHG = 0x6,
  ATOMIC_CMPXCHG = 0x7,
  ATOMIC_AND = 0x8,
  ATOMIC_OR = 0x9,
  ATOMIC_XOR = 0xa,
  ATOMIC_MINSINT = 0xb,
  ATOMIC_MAXSINT = 0xc,
  ATOMIC_FMAX = 0x10,
  ATOMIC_FMIN = 0x11,
  ATOMIC_FCMPWR = 0x12,
  /* INTEL_CUSTOMIZATION */
  /* INTEL_FEATURE_ESIMD_EMBARGO */
  ATOMIC_FADD = 0x13,
  ATOMIC_FSUB = 0x14,
  ATOMIC_LOAD = 0x15,
  ATOMIC_STORE = 0x16,
  /* end INTEL_FEATURE_ESIMD_EMBARGO */
  /* end INTEL_CUSTOMIZATION */
  ATOMIC_PREDEC = 0xff
};

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ESIMD_EMBARGO */
enum class EsimdPrecisionType {
  U1 = 0,   // unsigned 1 bit
  S1 = 1,   // signed 1 bit
  U2 = 2,   // unsigned 2 bits
  S2 = 3,   // signed 2 bits
  U4 = 4,   // unsigned 4 bits
  S4 = 5,   // signed 4 bits
  U8 = 6,   // unsigned 8 bits
  S8 = 7,   // signed 8 bits
  BF16 = 8, // bfloat 16
  FP16 = 9, // half float
  BF8 = 10, // bfloat 8
  TF32 = 11 // tensorfloat 32
};
/* end INTEL_FEATURE_ESIMD_EMBARGO */
/* end INTEL_CUSTOMIZATION */

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ESIMD_EMBARGO */

// Data size or format to read or store
// DO NOT MODIFY THE FOLLOWING ENCODING
enum class lsc_data_size : uint8_t {
  default_size = 0,
  u8 = 1,
  u16 = 2,
  u32 = 3,
  u64 = 4,
  u8u32 = 5,   // load 8b, zero extend to 32b; store the opposite
  u16u32 = 6,  // load 16b, zero extend to 32b; store the opposite
  u16u32h = 7, // load 16b into high 16 of each 32b; store the high 16
};

namespace detail {
// LSC atomic operations op codes
// DO NOT MODIFY THE FOLLOWING ENCODING
enum class lsc_atomic_op : uint8_t {
  iinc = 0x08,    // atomic integer increment
  idec = 0x09,    // atomic integer decrement
  load = 0x0a,    // atomic load
  store = 0x0b,   // atomic store
  iadd = 0x0c,    // atomic integer add
  isub = 0x0d,    // atomic integer subtract
  smin = 0x0e,    // atomic signed int min
  smax = 0x0f,    // atomic signed int max
  umin = 0x10,    // atomic unsigned int min
  umax = 0x11,    // atomic unsigned int max
  icas = 0x12,    // atomic int compare and swap
  fadd = 0x13,    // floating-point add
  fsub = 0x14,    // floating-point subtract
  fmin = 0x15,    // floating-point min
  fmax = 0x16,    // floating-point max
  fcas = 0x17,    // floating-point CAS
  bit_and = 0x18, // logical (bitwise) AND
  bit_or = 0x19,  // logical (bitwise) OR
  bit_xor = 0x1a, // logical (bitwise) XOR
};

// DO NOT MODIFY THE FOLLOWING ENCODING
enum class lsc_vector_size : uint8_t {
  n1 = 1,
  n2 = 2,
  n3 = 3,
  n4 = 4,
  n8 = 5,
  n16 = 6,
  n32 = 7,
  n64 = 8,
};

// DO NOT MODIFY THE FOLLOWING ENCODING
enum class lsc_data_order : uint8_t {
  nontranspose = 1,
  transpose = 2,
};

template <lsc_vector_size VS> constexpr void check_lsc_vector_size() {
  static_assert(VS == lsc_vector_size::n1 || VS == lsc_vector_size::n2 ||
                    VS == lsc_vector_size::n3 || VS == lsc_vector_size::n4 ||
                    VS == lsc_vector_size::n8 || VS == lsc_vector_size::n16 ||
                    VS == lsc_vector_size::n64 || VS == lsc_vector_size::n32,
                "Unsupported vector size");
}

template <uint8_t VS> constexpr void check_lsc_vector_size() {
  static_assert(VS == 1 || VS == 2 || VS == 3 || VS == 4 || VS == 8 ||
                    VS == 16 || VS == 32 || VS == 64,
                "Unsupported vector size");
}

template <typename T, lsc_data_size DS> constexpr void check_lsc_data_size() {
  static_assert(DS != lsc_data_size::default_size || sizeof(T) == 1 ||
                    sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
                "Unsupported data type");
}

template <EsimdAtomicOpType Op> constexpr void check_lsc_atomic_op() {
  static_assert(Op == EsimdAtomicOpType::ATOMIC_ADD ||
                    Op == EsimdAtomicOpType::ATOMIC_SUB ||
                    Op == EsimdAtomicOpType::ATOMIC_INC ||
                    Op == EsimdAtomicOpType::ATOMIC_DEC ||
                    Op == EsimdAtomicOpType::ATOMIC_MIN ||
                    Op == EsimdAtomicOpType::ATOMIC_MAX ||
                    Op == EsimdAtomicOpType::ATOMIC_CMPXCHG ||
                    Op == EsimdAtomicOpType::ATOMIC_AND ||
                    Op == EsimdAtomicOpType::ATOMIC_OR ||
                    Op == EsimdAtomicOpType::ATOMIC_XOR ||
                    Op == EsimdAtomicOpType::ATOMIC_MINSINT ||
                    Op == EsimdAtomicOpType::ATOMIC_MAXSINT ||
                    Op == EsimdAtomicOpType::ATOMIC_FMAX ||
                    Op == EsimdAtomicOpType::ATOMIC_FMIN ||
                    Op == EsimdAtomicOpType::ATOMIC_FCMPWR ||
                    Op == EsimdAtomicOpType::ATOMIC_FADD ||
                    Op == EsimdAtomicOpType::ATOMIC_FSUB ||
                    Op == EsimdAtomicOpType::ATOMIC_LOAD ||
                    Op == EsimdAtomicOpType::ATOMIC_STORE,
                "Unsupported operation for LSC atomics");
}

/// Check the legality of lsc xatomic call in terms of size and type.
/// \ingroup sycl_esimd
template <EsimdAtomicOpType Op, unsigned NumSrc>
constexpr void check_lsc_atomic() {
  check_lsc_atomic_op<Op>();
  if constexpr (Op == EsimdAtomicOpType::ATOMIC_INC ||
                Op == EsimdAtomicOpType::ATOMIC_DEC ||
                Op == EsimdAtomicOpType::ATOMIC_LOAD) {
    static_assert(NumSrc == 0, "No source operands are expected");
  }
  if constexpr (Op == EsimdAtomicOpType::ATOMIC_STORE ||
                Op == EsimdAtomicOpType::ATOMIC_ADD ||
                Op == EsimdAtomicOpType::ATOMIC_SUB ||
                Op == EsimdAtomicOpType::ATOMIC_MINSINT ||
                Op == EsimdAtomicOpType::ATOMIC_MAXSINT ||
                Op == EsimdAtomicOpType::ATOMIC_MIN ||
                Op == EsimdAtomicOpType::ATOMIC_MAX ||
                Op == EsimdAtomicOpType::ATOMIC_FADD ||
                Op == EsimdAtomicOpType::ATOMIC_FSUB ||
                Op == EsimdAtomicOpType::ATOMIC_FMIN ||
                Op == EsimdAtomicOpType::ATOMIC_FMAX ||
                Op == EsimdAtomicOpType::ATOMIC_AND ||
                Op == EsimdAtomicOpType::ATOMIC_OR ||
                Op == EsimdAtomicOpType::ATOMIC_XOR) {
    static_assert(NumSrc == 1, "One source operand is expected");
  }
  if constexpr (Op == EsimdAtomicOpType::ATOMIC_CMPXCHG ||
                Op == EsimdAtomicOpType::ATOMIC_FCMPWR) {
    static_assert(NumSrc == 2, "Two source operands are expected");
  }
}

template <EsimdAtomicOpType Op> constexpr lsc_atomic_op to_lsc_atomic_op() {
  check_lsc_atomic_op<Op>();
  switch (Op) {
  case EsimdAtomicOpType::ATOMIC_ADD:
    return lsc_atomic_op::iadd;
  case EsimdAtomicOpType::ATOMIC_SUB:
    return lsc_atomic_op::isub;
  case EsimdAtomicOpType::ATOMIC_INC:
    return lsc_atomic_op::iinc;
  case EsimdAtomicOpType::ATOMIC_DEC:
    return lsc_atomic_op::idec;
  case EsimdAtomicOpType::ATOMIC_MIN:
    return lsc_atomic_op::umin;
  case EsimdAtomicOpType::ATOMIC_MAX:
    return lsc_atomic_op::umax;
  case EsimdAtomicOpType::ATOMIC_CMPXCHG:
    return lsc_atomic_op::icas;
  case EsimdAtomicOpType::ATOMIC_AND:
    return lsc_atomic_op::bit_and;
  case EsimdAtomicOpType::ATOMIC_OR:
    return lsc_atomic_op::bit_or;
  case EsimdAtomicOpType::ATOMIC_XOR:
    return lsc_atomic_op::bit_xor;
  case EsimdAtomicOpType::ATOMIC_MINSINT:
    return lsc_atomic_op::smin;
  case EsimdAtomicOpType::ATOMIC_MAXSINT:
    return lsc_atomic_op::smax;
  case EsimdAtomicOpType::ATOMIC_FMAX:
    return lsc_atomic_op::fmax;
  case EsimdAtomicOpType::ATOMIC_FMIN:
    return lsc_atomic_op::fmin;
  case EsimdAtomicOpType::ATOMIC_FCMPWR:
    return lsc_atomic_op::fcas;
  case EsimdAtomicOpType::ATOMIC_FADD:
    return lsc_atomic_op::fadd;
  case EsimdAtomicOpType::ATOMIC_FSUB:
    return lsc_atomic_op::fsub;
  case EsimdAtomicOpType::ATOMIC_LOAD:
    return lsc_atomic_op::load;
  case EsimdAtomicOpType::ATOMIC_STORE:
    return lsc_atomic_op::store;
  default:
    return lsc_atomic_op::iinc;
  }
}

template <lsc_vector_size VS> constexpr uint8_t to_int() {
  check_lsc_vector_size<VS>();
  switch (VS) {
  case lsc_vector_size::n1:
    return 1;
  case lsc_vector_size::n2:
    return 2;
  case lsc_vector_size::n3:
    return 3;
  case lsc_vector_size::n4:
    return 4;
  case lsc_vector_size::n8:
    return 8;
  case lsc_vector_size::n16:
    return 16;
  case lsc_vector_size::n32:
    return 32;
  case lsc_vector_size::n64:
    return 64;
  default:
    return 1;
  }
}

template <uint8_t VS> constexpr lsc_vector_size to_lsc_vector_size() {
  check_lsc_vector_size<VS>();
  switch (VS) {
  case 1:
    return lsc_vector_size::n1;
  case 2:
    return lsc_vector_size::n2;
  case 3:
    return lsc_vector_size::n3;
  case 4:
    return lsc_vector_size::n4;
  case 8:
    return lsc_vector_size::n8;
  case 16:
    return lsc_vector_size::n16;
  case 32:
    return lsc_vector_size::n32;
  case 64:
    return lsc_vector_size::n64;
  default:
    return lsc_vector_size::n1;
  }
}

template <typename T, lsc_data_size DS>
constexpr lsc_data_size finalize_data_size() {
  check_lsc_data_size<T, DS>();
  if (DS != lsc_data_size::default_size)
    return DS;
  else if (sizeof(T) == 1)
    return lsc_data_size::u8;
  else if (sizeof(T) == 2)
    return lsc_data_size::u16;
  else if (sizeof(T) == 4)
    return lsc_data_size::u32;
  else if (sizeof(T) == 8)
    return lsc_data_size::u64;
  else
    return DS;
}

} // namespace detail

/* end INTEL_FEATURE_ESIMD_EMBARGO */
/* end INTEL_CUSTOMIZATION */

// L1 or L3 cache hint kinds.
enum class CacheHint : uint8_t {
  None = 0,
  Uncached = 1,
  WriteBack = 2,
  WriteThrough = 3,
  Streaming = 4,
  ReadInvalidate = 5
};

enum class EsimdSbarrierType : uint8_t {
  WAIT = 0,  // split barrier wait
  SIGNAL = 1 // split barrier signal
};

#define ESIMD_SBARRIER_WAIT EsimdSbarrierType::WAIT
#define ESIMD_SBARRIER_SIGNAL EsimdSbarrierType::SIGNAL

} // namespace esimd
} // namespace experimental
} // namespace intel
} // namespace ext
} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)

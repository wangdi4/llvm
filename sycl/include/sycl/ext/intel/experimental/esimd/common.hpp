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

#ifdef __SYCL_DEVICE_ONLY__
#define SYCL_ESIMD_KERNEL __attribute__((sycl_explicit_simd))
#define SYCL_ESIMD_FUNCTION __attribute__((sycl_explicit_simd))

// Mark a function being nodebug.
#define ESIMD_NODEBUG __attribute__((nodebug))
// Mark a "ESIMD global": accessible from all functions in current translation
// unit, separate copy per subgroup (work-item), mapped to SPIR-V private
// storage class.
#define ESIMD_PRIVATE                                                          \
  __attribute__((opencl_private)) __attribute__((sycl_explicit_simd))
// Bind a ESIMD global variable to a specific register.
#define ESIMD_REGISTER(n) __attribute__((register_num(n)))

#define __ESIMD_API ESIMD_NODEBUG ESIMD_INLINE

#define __ESIMD_UNSUPPORTED_ON_HOST

#else // __SYCL_DEVICE_ONLY__
#define SYCL_ESIMD_KERNEL
#define SYCL_ESIMD_FUNCTION

// TODO ESIMD define what this means on Windows host
#define ESIMD_NODEBUG
// On host device ESIMD global is a thread local static var. This assumes that
// each work-item is mapped to a separate OS thread on host device.
#define ESIMD_PRIVATE thread_local
#define ESIMD_REGISTER(n)

#define __ESIMD_API ESIMD_INLINE

#define __ESIMD_UNSUPPORTED_ON_HOST throw cl::sycl::feature_not_supported()

#endif // __SYCL_DEVICE_ONLY__

// Mark a function being noinline
#define ESIMD_NOINLINE __attribute__((noinline))
// Force a function to be inlined. 'inline' is used to preserve ODR for
// functions defined in a header.
#define ESIMD_INLINE inline __attribute__((always_inline))

// Macros for internal use
#define __ESIMD_NS sycl::ext::intel::experimental::esimd
#define __ESIMD_QUOTE1(m) #m
#define __ESIMD_QUOTE(m) __ESIMD_QUOTE1(m)
#define __ESIMD_NS_QUOTED __ESIMD_QUOTE(__ESIMD_NS)
#define __ESIMD_DEPRECATED(new_api)                                            \
  __SYCL_DEPRECATED("use " __ESIMD_NS_QUOTED "::" __ESIMD_QUOTE(new_api))
// Defines a deprecated enum value. Use of this value will cause a deprecation
// message printed out by the compiler.
#define __ESIMD_DEPR_ENUM_V(old, new, t)                                       \
  old __ESIMD_DEPRECATED(new) = static_cast<t>(new)

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {
namespace ext {
namespace intel {
namespace experimental {
namespace esimd {

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ESIMD_EMBARGO */
#ifdef __SYCL_DEVICE_ONLY__
// TODO map bfloat16 to SYCL's half type storage for now, following CM practice.
// Will map to native bfloat16 (available since LLVM 11), once supported in BE.
using bfloat16 = _Float16;
#else
// TODO can't map to cl::sycl::detail::half_impl::StorageT, as it is a class on
// host and can't be a vector element. Implement generic solution for half and
// bfloat16.
using bfloat16 = uint16_t;
#endif // __SYCL_DEVICE_ONLY__
/* end INTEL_FEATURE_ESIMD_EMBARGO */
/* end INTEL_CUSTOMIZATION */

using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;

/// Gen hardware supports applying saturation to results of some operation.
/// This enum allows to control this behavior.
enum class saturation : uint8_t { off, on };

/// Integer type short-cut to saturation::off.
static inline constexpr uint8_t saturation_off =
    static_cast<uint8_t>(saturation::off);
/// Integer type short-cut to saturation::on.
static inline constexpr uint8_t saturation_on =
    static_cast<uint8_t>(saturation::on);

enum {
  __ESIMD_DEPR_ENUM_V(GENX_NOSAT, saturation::off, uint8_t),
  __ESIMD_DEPR_ENUM_V(GENX_SAT, saturation::on, uint8_t)
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

/// Represents a pixel's channel.
enum class rgba_channel : uint8_t { R, G, B, A };

namespace detail {
template <rgba_channel Ch>
static inline constexpr uint8_t ch = 1 << static_cast<int>(Ch);
static inline constexpr uint8_t chR = ch<rgba_channel::R>;
static inline constexpr uint8_t chG = ch<rgba_channel::G>;
static inline constexpr uint8_t chB = ch<rgba_channel::B>;
static inline constexpr uint8_t chA = ch<rgba_channel::A>;
} // namespace detail

/// Represents a pixel's channel mask - all possible combinations of enabled
/// channels.
enum class rgba_channel_mask : uint8_t {
  R = detail::chR,
  G = detail::chG,
  GR = detail::chG | detail::chR,
  B = detail::chB,
  BR = detail::chB | detail::chR,
  BG = detail::chB | detail::chG,
  BGR = detail::chB | detail::chG | detail::chR,
  A = detail::chA,
  AR = detail::chA | detail::chR,
  AG = detail::chA | detail::chG,
  AGR = detail::chA | detail::chG | detail::chR,
  AB = detail::chA | detail::chB,
  ABR = detail::chA | detail::chB | detail::chR,
  ABG = detail::chA | detail::chB | detail::chG,
  ABGR = detail::chA | detail::chB | detail::chG | detail::chR,
  // For backward compatibility ('ChannelMaskType::ESIMD_R_ENABLE' usage style):
  __ESIMD_DEPR_ENUM_V(ESIMD_R_ENABLE, rgba_channel_mask::R, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_G_ENABLE, rgba_channel_mask::G, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_GR_ENABLE, rgba_channel_mask::GR, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_B_ENABLE, rgba_channel_mask::B, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_BR_ENABLE, rgba_channel_mask::BR, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_BG_ENABLE, rgba_channel_mask::BG, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_BGR_ENABLE, rgba_channel_mask::BGR, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_A_ENABLE, rgba_channel_mask::A, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_AR_ENABLE, rgba_channel_mask::AR, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_AG_ENABLE, rgba_channel_mask::AG, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_AGR_ENABLE, rgba_channel_mask::AGR, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_AB_ENABLE, rgba_channel_mask::AB, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_ABR_ENABLE, rgba_channel_mask::ABR, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_ABG_ENABLE, rgba_channel_mask::ABG, uint8_t),
  __ESIMD_DEPR_ENUM_V(ESIMD_ABGR_ENABLE, rgba_channel_mask::ABGR, uint8_t)
};

#define __ESIMD_DEPR_CONST(old, new)                                           \
  static inline constexpr auto old __ESIMD_DEPRECATED(new) = new

// For backward compatibility ('ESIMD_R_ENABLE' usage style):
__ESIMD_DEPR_CONST(ESIMD_R_ENABLE, rgba_channel_mask::R);
__ESIMD_DEPR_CONST(ESIMD_G_ENABLE, rgba_channel_mask::G);
__ESIMD_DEPR_CONST(ESIMD_GR_ENABLE, rgba_channel_mask::GR);
__ESIMD_DEPR_CONST(ESIMD_B_ENABLE, rgba_channel_mask::B);
__ESIMD_DEPR_CONST(ESIMD_BR_ENABLE, rgba_channel_mask::BR);
__ESIMD_DEPR_CONST(ESIMD_BG_ENABLE, rgba_channel_mask::BG);
__ESIMD_DEPR_CONST(ESIMD_BGR_ENABLE, rgba_channel_mask::BGR);
__ESIMD_DEPR_CONST(ESIMD_A_ENABLE, rgba_channel_mask::A);
__ESIMD_DEPR_CONST(ESIMD_AR_ENABLE, rgba_channel_mask::AR);
__ESIMD_DEPR_CONST(ESIMD_AG_ENABLE, rgba_channel_mask::AG);
__ESIMD_DEPR_CONST(ESIMD_AGR_ENABLE, rgba_channel_mask::AGR);
__ESIMD_DEPR_CONST(ESIMD_AB_ENABLE, rgba_channel_mask::AB);
__ESIMD_DEPR_CONST(ESIMD_ABR_ENABLE, rgba_channel_mask::ABR);
__ESIMD_DEPR_CONST(ESIMD_ABG_ENABLE, rgba_channel_mask::ABG);
__ESIMD_DEPR_CONST(ESIMD_ABGR_ENABLE, rgba_channel_mask::ABGR);

#undef __ESIMD_DEPR_CONST

// For backward compatibility:
using ChannelMaskType = rgba_channel_mask;

constexpr int is_channel_enabled(rgba_channel_mask M, rgba_channel Ch) {
  int Pos = static_cast<int>(Ch);
  return (static_cast<int>(M) & (1 << Pos)) >> Pos;
}

constexpr int get_num_channels_enabled(rgba_channel_mask M) {
  return is_channel_enabled(M, rgba_channel::R) +
         is_channel_enabled(M, rgba_channel::G) +
         is_channel_enabled(M, rgba_channel::B) +
         is_channel_enabled(M, rgba_channel::A);
}

/// Represents an atomic operation.
enum class atomic_op : uint8_t {
  add = 0x0,
  sub = 0x1,
  inc = 0x2,
  dec = 0x3,
  min = 0x4,
  max = 0x5,
  xchg = 0x6,
  cmpxchg = 0x7,
  bit_and = 0x8,
  bit_or = 0x9,
  bit_xor = 0xa,
  minsint = 0xb,
  maxsint = 0xc,
  fmax = 0x10,
  fmin = 0x11,
  fcmpwr = 0x12,
  /* INTEL_CUSTOMIZATION */
  /* INTEL_FEATURE_ESIMD_EMBARGO */
  fadd = 0x13,
  fsub = 0x14,
  load = 0x15,
  store = 0x16,
  /* end INTEL_FEATURE_ESIMD_EMBARGO */
  /* end INTEL_CUSTOMIZATION */
  predec = 0xff,
  // For backward compatibility:
  __ESIMD_DEPR_ENUM_V(ATOMIC_ADD, atomic_op::add, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_SUB, atomic_op::sub, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_INC, atomic_op::inc, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_DEC, atomic_op::dec, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_MIN, atomic_op::min, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_MAX, atomic_op::max, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_XCHG, atomic_op::xchg, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_CMPXCHG, atomic_op::cmpxchg, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_AND, atomic_op::bit_and, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_OR, atomic_op::bit_or, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_XOR, atomic_op::bit_xor, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_MINSINT, atomic_op::minsint, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_MAXSINT, atomic_op::maxsint, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_FMAX, atomic_op::fmax, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_FMIN, atomic_op::fmin, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_FCMPWR, atomic_op::fcmpwr, uint8_t),
  /* INTEL_CUSTOMIZATION */
  /* INTEL_FEATURE_ESIMD_EMBARGO */
  __ESIMD_DEPR_ENUM_V(ATOMIC_FADD, atomic_op::fadd, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_FSUB, atomic_op::fsub, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_LOAD, atomic_op::load, uint8_t),
  __ESIMD_DEPR_ENUM_V(ATOMIC_STORE, atomic_op::store, uint8_t),
  /* end INTEL_FEATURE_ESIMD_EMBARGO */
  /* end INTEL_CUSTOMIZATION */
  __ESIMD_DEPR_ENUM_V(ATOMIC_PREDEC, atomic_op::predec, uint8_t)
};

// For backward compatibility:
using EsimdAtomicOpType = atomic_op;

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ESIMD_EMBARGO */

// DO NOT MODIFY THE FOLLOWING ENCODING
/// The scope that lsc_fence operation should apply to
/// Supported platforms: XEHP, DG2, PVC, PVC_XT, ELG+
enum class lsc_scope : uint8_t {
  group = 0,  /// flush out to the threadgroup's scope
  local = 1,  /// flush out to the local scope
  tile = 2,   /// tile, flush out to several DSSs
  gpu = 3,    /// entire GPU, flush out to the GPUs LLC
  gpus = 4,   /// all GPUs in the system, flush out to memory shared by all GPUs
  system = 5, /// the entire system memory space
  sysacq = 6, /// the entire system memory space with system-acquire semantics
};

// DO NOT MODIFY THE FOLLOWING ENCODING
/// The lsc_fence operation to apply to caches
/// Supported platforms: XEHP, DG2, PVC, PVC_XT, ELG+
enum class lsc_fence_op : uint8_t {
  none = 0,       /// no operation
  evict = 1,      /// dirty lines evicted and invalidated from L1
  invalidate = 2, /// invalidate all clean lines
  discard = 3,    /// direct and clean lines are discarded w/o eviction
  clean = 4,      /// dirty lines are written to memory, but retained in cache in clean state
  flushl3 = 5,    /// flush only L3
};

// DO NOT MODIFY THE FOLLOWING ENCODING
/// The specific LSC shared function to fence with lsc_fence
/// Supported platforms: XEHP, DG2, PVC, PVC_XT, ELG+
enum class lsc_sfid : uint8_t {
  ugm = 0,  /// unified global memory
  ugml = 1, /// low-bandwith untyped global memory
  tgm = 2,  /// typed global memory
  slm = 3,  /// shared local memory
};

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

template <atomic_op Op> constexpr void check_lsc_atomic_op() {
  static_assert(Op == atomic_op::add || Op == atomic_op::sub ||
                    Op == atomic_op::inc || Op == atomic_op::dec ||
                    Op == atomic_op::min || Op == atomic_op::max ||
                    Op == atomic_op::cmpxchg || Op == atomic_op::bit_and ||
                    Op == atomic_op::bit_or || Op == atomic_op::bit_xor ||
                    Op == atomic_op::minsint || Op == atomic_op::maxsint ||
                    Op == atomic_op::fmax || Op == atomic_op::fmin ||
                    Op == atomic_op::fcmpwr || Op == atomic_op::fadd ||
                    Op == atomic_op::fsub || Op == atomic_op::load ||
                    Op == atomic_op::store,
                "Unsupported operation for LSC atomics");
}

/// Check the legality of lsc xatomic call in terms of size and type.
/// \ingroup sycl_esimd
template <atomic_op Op, unsigned NumSrc>
constexpr void check_lsc_atomic() {
  check_lsc_atomic_op<Op>();
  if constexpr (Op == atomic_op::inc || Op == atomic_op::dec ||
                Op == atomic_op::load) {
    static_assert(NumSrc == 0, "No source operands are expected");
  }
  if constexpr (Op == atomic_op::store || Op == atomic_op::add ||
                Op == atomic_op::sub || Op == atomic_op::minsint ||
                Op == atomic_op::maxsint || Op == atomic_op::min ||
                Op == atomic_op::max || Op == atomic_op::fadd ||
                Op == atomic_op::fsub || Op == atomic_op::fmin ||
                Op == atomic_op::fmax || Op == atomic_op::bit_and ||
                Op == atomic_op::bit_or || Op == atomic_op::bit_xor) {
    static_assert(NumSrc == 1, "One source operand is expected");
  }
  if constexpr (Op == atomic_op::cmpxchg || Op == atomic_op::fcmpwr) {
    static_assert(NumSrc == 2, "Two source operands are expected");
  }
}

template <atomic_op Op> constexpr lsc_atomic_op to_lsc_atomic_op() {
  check_lsc_atomic_op<Op>();
  switch (Op) {
  case atomic_op::add:
    return lsc_atomic_op::iadd;
  case atomic_op::sub:
    return lsc_atomic_op::isub;
  case atomic_op::inc:
    return lsc_atomic_op::iinc;
  case atomic_op::dec:
    return lsc_atomic_op::idec;
  case atomic_op::min:
    return lsc_atomic_op::umin;
  case atomic_op::max:
    return lsc_atomic_op::umax;
  case atomic_op::cmpxchg:
    return lsc_atomic_op::icas;
  case atomic_op::bit_and:
    return lsc_atomic_op::bit_and;
  case atomic_op::bit_or:
    return lsc_atomic_op::bit_or;
  case atomic_op::bit_xor:
    return lsc_atomic_op::bit_xor;
  case atomic_op::minsint:
    return lsc_atomic_op::smin;
  case atomic_op::maxsint:
    return lsc_atomic_op::smax;
  case atomic_op::fmax:
    return lsc_atomic_op::fmax;
  case atomic_op::fmin:
    return lsc_atomic_op::fmin;
  case atomic_op::fcmpwr:
    return lsc_atomic_op::fcas;
  case atomic_op::fadd:
    return lsc_atomic_op::fadd;
  case atomic_op::fsub:
    return lsc_atomic_op::fsub;
  case atomic_op::load:
    return lsc_atomic_op::load;
  case atomic_op::store:
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

// TODO Cache hints APIs are being reworked.
// L1 or L3 cache hint kinds.
enum class CacheHint : uint8_t {
  None = 0,
  Uncached = 1,
  Cached = 2,
  WriteBack = 3,
  WriteThrough = 4,
  Streaming = 5,
  ReadInvalidate = 6
};

/// Represents a split barrier action.
enum class split_barrier_action : uint8_t {
  wait = 0,   // split barrier wait
  signal = 1, // split barrier signal
  // For backward compatibility:
  __ESIMD_DEPR_ENUM_V(WAIT, split_barrier_action::wait, uint8_t),
  __ESIMD_DEPR_ENUM_V(SIGNAL, split_barrier_action::signal, uint8_t)
};

// For backward compatibility:
using EsimdSbarrierType = split_barrier_action;

// Since EsimdSbarrierType values are deprecated, these macros will generate
// deprecation message.
#define ESIMD_SBARRIER_WAIT EsimdSbarrierType::WAIT
#define ESIMD_SBARRIER_SIGNAL EsimdSbarrierType::SIGNAL

/// Surface index type. Surface is an internal representation of a memory block
/// addressable by GPU in "stateful" memory model, and each surface is
/// identified by its "binding table index" - surface index.
using SurfaceIndex = unsigned int;

} // namespace esimd
} // namespace experimental
} // namespace intel
} // namespace ext
} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)

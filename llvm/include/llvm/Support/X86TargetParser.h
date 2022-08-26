//===-- X86TargetParser - Parser for X86 features ---------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a target parser to recognise X86 hardware features.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_X86TARGETPARSER_H
#define LLVM_SUPPORT_X86TARGETPARSER_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringMap.h"

namespace llvm {
template <typename T> class SmallVectorImpl;
class StringRef;

namespace X86 {

// This should be kept in sync with libcc/compiler-rt as its included by clang
// as a proxy for what's in libgcc/compiler-rt.
enum ProcessorVendors : unsigned {
  VENDOR_DUMMY,
#define X86_VENDOR(ENUM, STRING) \
  ENUM,
#include "llvm/Support/X86TargetParser.def"
  VENDOR_OTHER
};

// This should be kept in sync with libcc/compiler-rt as its included by clang
// as a proxy for what's in libgcc/compiler-rt.
enum ProcessorTypes : unsigned {
  CPU_TYPE_DUMMY,
#define X86_CPU_TYPE(ENUM, STRING) \
  ENUM,
#include "llvm/Support/X86TargetParser.def"
  CPU_TYPE_MAX
};

// This should be kept in sync with libcc/compiler-rt as its included by clang
// as a proxy for what's in libgcc/compiler-rt.
enum ProcessorSubtypes : unsigned {
  CPU_SUBTYPE_DUMMY,
#define X86_CPU_SUBTYPE(ENUM, STRING) \
  ENUM,
#include "llvm/Support/X86TargetParser.def"
  CPU_SUBTYPE_MAX
};

// This should be kept in sync with libcc/compiler-rt as it should be used
// by clang as a proxy for what's in libgcc/compiler-rt.
enum ProcessorFeatures {
#define X86_FEATURE(ENUM, STRING) FEATURE_##ENUM,
#include "llvm/Support/X86TargetParser.def"
  CPU_FEATURE_MAX
};

enum CPUKind {
  CK_None,
  CK_i386,
  CK_i486,
  CK_WinChipC6,
  CK_WinChip2,
  CK_C3,
  CK_i586,
  CK_Pentium,
  CK_PentiumMMX,
  CK_PentiumPro,
  CK_i686,
  CK_Pentium2,
  CK_Pentium3,
  CK_PentiumM,
  CK_C3_2,
  CK_Yonah,
  CK_Pentium4,
  CK_Prescott,
  CK_Nocona,
  CK_Core2,
  CK_Penryn,
  CK_Bonnell,
  CK_Silvermont,
  CK_Goldmont,
  CK_GoldmontPlus,
  CK_Tremont,
#if INTEL_CUSTOMIZATION
  CK_Gracemont,
#endif // INTEL_CUSTOMIZATION
  CK_Nehalem,
  CK_Westmere,
  CK_SandyBridge,
  CK_IvyBridge,
  CK_Haswell,
  CK_Broadwell,
#if INTEL_CUSTOMIZATION
  CK_CommonAVX512,
#if INTEL_FEATURE_ISA_AVX256
  CK_CommonAVX256,
#endif // INTEL_FEATURE_ISA_AVX256
#endif // INTEL_CUSTOMIZATION
  CK_SkylakeClient,
  CK_SkylakeServer,
  CK_Cascadelake,
  CK_Cooperlake,
  CK_Cannonlake,
  CK_IcelakeClient,
  CK_Rocketlake,
  CK_IcelakeServer,
  CK_Tigerlake,
  CK_SapphireRapids,
  CK_Alderlake,
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CPU_RPL
  CK_Raptorlake,
#endif // INTEL_FEATURE_CPU_RPL
#if INTEL_FEATURE_CPU_GNR
  CK_Graniterapids,
#endif // INTEL_FEATURE_CPU_GNR
#if INTEL_FEATURE_CPU_DMR
  CK_Diamondrapids,
#endif // INTEL_FEATURE_CPU_DMR
#if INTEL_FEATURE_CPU_MTL
  CK_Meteorlake,
#endif // INTEL_FEATURE_CPU_MTL
#if INTEL_FEATURE_CPU_EMR
  CK_Emeraldrapids,
#endif // INTEL_FEATURE_CPU_EMR
#endif // INTEL_CUSTOMIZATION
  CK_KNL,
  CK_KNM,
  CK_Lakemont,
  CK_K6,
  CK_K6_2,
  CK_K6_3,
  CK_Athlon,
  CK_AthlonXP,
  CK_K8,
  CK_K8SSE3,
  CK_AMDFAM10,
  CK_BTVER1,
  CK_BTVER2,
  CK_BDVER1,
  CK_BDVER2,
  CK_BDVER3,
  CK_BDVER4,
  CK_ZNVER1,
  CK_ZNVER2,
  CK_ZNVER3,
  CK_x86_64,
  CK_x86_64_v2,
  CK_x86_64_v3,
  CK_x86_64_v4,
  CK_Geode,
};

/// Parse \p CPU string into a CPUKind. Will only accept 64-bit capable CPUs if
/// \p Only64Bit is true.
CPUKind parseArchX86(StringRef CPU, bool Only64Bit = false);
CPUKind parseTuneCPU(StringRef CPU, bool Only64Bit = false);

/// Provide a list of valid CPU names. If \p Only64Bit is true, the list will
/// only contain 64-bit capable CPUs.
void fillValidCPUArchList(SmallVectorImpl<StringRef> &Values,
                          bool Only64Bit = false);
/// Provide a list of valid -mtune names.
void fillValidTuneCPUList(SmallVectorImpl<StringRef> &Values,
                          bool Only64Bit = false);

/// Get the key feature prioritizing target multiversioning.
ProcessorFeatures getKeyFeature(CPUKind Kind);

/// Fill in the features that \p CPU supports into \p Features.
void getFeaturesForCPU(StringRef CPU, SmallVectorImpl<StringRef> &Features);

/// Set or clear entries in \p Features that are implied to be enabled/disabled
/// by the provided \p Feature.
void updateImpliedFeatures(StringRef Feature, bool Enabled,
                           StringMap<bool> &Features);

uint64_t getCpuSupportsMask(ArrayRef<StringRef> FeatureStrs);
unsigned getFeaturePriority(ProcessorFeatures Feat);

} // namespace X86
} // namespace llvm

#endif

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

#include "llvm/TargetParser/X86TargetParser.h"
#include "llvm/ADT/Bitset.h"
#include "llvm/ADT/StringSwitch.h"
#include <numeric>

using namespace llvm;
using namespace llvm::X86;

namespace {

using FeatureBitset = Bitset<X86::CPU_FEATURE_MAX>;

struct ProcInfo {
  StringLiteral Name;
  X86::CPUKind Kind;
  unsigned KeyFeature;
  FeatureBitset Features;
  char Mangling;
  bool OnlyForCPUDispatchSpecific;
};

struct FeatureInfo {
  StringLiteral NameWithPlus;
  FeatureBitset ImpliedFeatures;

  StringRef getName(bool WithPlus = false) const {
    assert(NameWithPlus[0] == '+' && "Expected string to start with '+'");
    if (WithPlus)
      return NameWithPlus;
    return NameWithPlus.drop_front();
  }
};

} // end anonymous namespace

#define X86_FEATURE(ENUM, STRING)                                              \
  constexpr FeatureBitset Feature##ENUM = {X86::FEATURE_##ENUM};
#include "llvm/TargetParser/X86TargetParser.def"

// Pentium with MMX.
constexpr FeatureBitset FeaturesPentiumMMX =
    FeatureX87 | FeatureCMPXCHG8B | FeatureMMX;

// Pentium 2 and 3.
constexpr FeatureBitset FeaturesPentium2 =
    FeatureX87 | FeatureCMPXCHG8B | FeatureMMX | FeatureFXSR | FeatureCMOV;
constexpr FeatureBitset FeaturesPentium3 = FeaturesPentium2 | FeatureSSE;

// Pentium 4 CPUs
constexpr FeatureBitset FeaturesPentium4 = FeaturesPentium3 | FeatureSSE2;
constexpr FeatureBitset FeaturesPrescott = FeaturesPentium4 | FeatureSSE3;
constexpr FeatureBitset FeaturesNocona =
    FeaturesPrescott | Feature64BIT | FeatureCMPXCHG16B;

// Basic 64-bit capable CPU.
constexpr FeatureBitset FeaturesX86_64 = FeaturesPentium4 | Feature64BIT;
constexpr FeatureBitset FeaturesX86_64_V2 = FeaturesX86_64 | FeatureSAHF |
                                            FeaturePOPCNT | FeatureCRC32 |
                                            FeatureSSE4_2 | FeatureCMPXCHG16B;
constexpr FeatureBitset FeaturesX86_64_V3 =
    FeaturesX86_64_V2 | FeatureAVX2 | FeatureBMI | FeatureBMI2 | FeatureF16C |
    FeatureFMA | FeatureLZCNT | FeatureMOVBE | FeatureXSAVE;
constexpr FeatureBitset FeaturesX86_64_V4 = FeaturesX86_64_V3 | FeatureEVEX512 |
                                            FeatureAVX512BW | FeatureAVX512CD |
                                            FeatureAVX512DQ | FeatureAVX512VL;

// Intel Core CPUs
constexpr FeatureBitset FeaturesCore2 =
    FeaturesNocona | FeatureSAHF | FeatureSSSE3;
constexpr FeatureBitset FeaturesPenryn = FeaturesCore2 | FeatureSSE4_1;
constexpr FeatureBitset FeaturesNehalem =
    FeaturesPenryn | FeaturePOPCNT | FeatureCRC32 | FeatureSSE4_2;
constexpr FeatureBitset FeaturesWestmere = FeaturesNehalem | FeaturePCLMUL;
constexpr FeatureBitset FeaturesSandyBridge =
    FeaturesWestmere | FeatureAVX | FeatureXSAVE | FeatureXSAVEOPT;
constexpr FeatureBitset FeaturesIvyBridge =
    FeaturesSandyBridge | FeatureF16C | FeatureFSGSBASE | FeatureRDRND;
constexpr FeatureBitset FeaturesHaswell =
    FeaturesIvyBridge | FeatureAVX2 | FeatureBMI | FeatureBMI2 | FeatureFMA |
    FeatureINVPCID | FeatureLZCNT | FeatureMOVBE;
constexpr FeatureBitset FeaturesBroadwell =
    FeaturesHaswell | FeatureADX | FeaturePRFCHW | FeatureRDSEED;

// Intel Knights Landing and Knights Mill
// Knights Landing has feature parity with Broadwell.
constexpr FeatureBitset FeaturesKNL =
    FeaturesBroadwell | FeatureAES | FeatureAVX512F | FeatureEVEX512 |
    FeatureAVX512CD | FeatureAVX512ER | FeatureAVX512PF | FeaturePREFETCHWT1;
constexpr FeatureBitset FeaturesKNM = FeaturesKNL | FeatureAVX512VPOPCNTDQ;

#if INTEL_CUSTOMIZATION
constexpr FeatureBitset FeaturesCommonAVX512 =
    FeaturesBroadwell | FeatureAES | FeatureAVX512F | FeatureEVEX512 |
    FeatureAVX512CD;
#endif // INTEL_CUSTOMIZATION

// Intel Skylake processors.
constexpr FeatureBitset FeaturesSkylakeClient =
    FeaturesBroadwell | FeatureAES | FeatureCLFLUSHOPT | FeatureXSAVEC |
    FeatureXSAVES | FeatureSGX;
// SkylakeServer inherits all SkylakeClient features except SGX.
// FIXME: That doesn't match gcc.
constexpr FeatureBitset FeaturesSkylakeServer =
    (FeaturesSkylakeClient & ~FeatureSGX) | FeatureAVX512F | FeatureEVEX512 |
    FeatureAVX512CD | FeatureAVX512DQ | FeatureAVX512BW | FeatureAVX512VL |
    FeatureCLWB | FeaturePKU;
constexpr FeatureBitset FeaturesCascadeLake =
    FeaturesSkylakeServer | FeatureAVX512VNNI;
constexpr FeatureBitset FeaturesCooperLake =
    FeaturesCascadeLake | FeatureAVX512BF16;

// Intel 10nm processors.
constexpr FeatureBitset FeaturesCannonlake =
    FeaturesSkylakeClient | FeatureAVX512F | FeatureEVEX512 | FeatureAVX512CD |
    FeatureAVX512DQ | FeatureAVX512BW | FeatureAVX512VL | FeatureAVX512IFMA |
    FeatureAVX512VBMI | FeaturePKU | FeatureSHA;
constexpr FeatureBitset FeaturesICLClient =
    FeaturesCannonlake | FeatureAVX512BITALG | FeatureAVX512VBMI2 |
    FeatureAVX512VNNI | FeatureAVX512VPOPCNTDQ | FeatureGFNI | FeatureRDPID |
    FeatureVAES | FeatureVPCLMULQDQ;
constexpr FeatureBitset FeaturesRocketlake = FeaturesICLClient & ~FeatureSGX;
constexpr FeatureBitset FeaturesICLServer =
    FeaturesICLClient | FeatureCLWB | FeaturePCONFIG | FeatureWBNOINVD;
constexpr FeatureBitset FeaturesTigerlake =
    FeaturesICLClient | FeatureAVX512VP2INTERSECT | FeatureMOVDIR64B |
    FeatureCLWB | FeatureMOVDIRI | FeatureSHSTK | FeatureKL | FeatureWIDEKL;
constexpr FeatureBitset FeaturesSapphireRapids =
    FeaturesICLServer | FeatureAMX_BF16 | FeatureAMX_INT8 | FeatureAMX_TILE |
    FeatureAVX512BF16 | FeatureAVX512FP16 | FeatureAVXVNNI | FeatureCLDEMOTE |
    FeatureENQCMD | FeatureMOVDIR64B | FeatureMOVDIRI | FeaturePTWRITE |
    FeatureSERIALIZE | FeatureSHSTK | FeatureTSXLDTRK | FeatureUINTR |
    FeatureWAITPKG;
constexpr FeatureBitset FeaturesGraniteRapids =
    FeaturesSapphireRapids | FeatureAMX_FP16 | FeaturePREFETCHI ;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX256P
constexpr FeatureBitset FeaturesCommonAVX256 =
    FeaturesSkylakeClient | FeatureAVX256P;
#endif // INTEL_FEATURE_ISA_AVX256P
#if INTEL_FEATURE_CPU_DMR
#define ENABLE_CPU_DMR
constexpr FeatureBitset FeaturesLioncoveServer =
#endif // INTEL_FEATURE_CPU_DMR
#ifdef ENABLE_CPU_DMR
    FeaturesGraniteRapids |
    FeatureAMX_COMPLEX |
    FeatureSM3 |
    FeatureSM4 |
#if INTEL_FEATURE_ISA_AVX512_VNNI_FP16
    FeatureAVX512VNNIFP16 |
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_FP16
#if INTEL_FEATURE_ISA_AVX512_VNNI_INT16
    FeatureAVX512VNNIINT16 |
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT16
#if INTEL_FEATURE_ISA_AVX512_VNNI_INT8
    FeatureAVX512VNNIINT8 |
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT8
#if INTEL_FEATURE_ISA_AVX_CONVERT
// FIXME: Need to change to AVX-NE-CONVERT
    FeatureAVXCONVERT |
#endif // INTEL_FEATURE_ISA_AVX_CONVERT
    FeatureAVXVNNIINT16 |
    FeatureAVXVNNIINT8 |
#if INTEL_FEATURE_ISA_AVX512_MEDIAX
    FeatureAVX512MEDIAX |
#endif // INTEL_FEATURE_ISA_AVX512_MEDIAX
#if INTEL_FEATURE_ISA_AVX512_CONVERT
// FIXME: Need to change to AVX512-NE-CONVERT
    FeatureAVX512CONVERT |
#endif // INTEL_FEATURE_ISA_AVX512_CONVERT
#if INTEL_FEATURE_ISA_AVX512_BF16_NE
    FeatureAVX512BF16NE |
#endif // INTEL_FEATURE_ISA_AVX512_BF16_NE
    FeatureAVXIFMA |
    FeatureCMPCCXADD |
    FeatureSHA512 |
#if INTEL_FEATURE_ISA_PREFETCHST2
    FeaturePREFETCHST2 |
#endif // INTEL_FEATURE_ISA_PREFETCHST2
#if INTEL_FEATURE_ISA_AMX_TRANSPOSE
// FIXME: according to latest EAS (lnc-rev52 and pnc-rev11), part of AMX_TRANSPOSE
// is split into AMX_INTERLEAVE, which is introduced in pnc.
    FeatureAMX_TRANSPOSE |
    FeatureAMX_AVX512 |
#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE
#if INTEL_FEATURE_ISA_AMX_TF32
    FeatureAMX_TF32;
#endif // INTEL_FEATURE_ISA_AMX_TF32
#endif // ENABLE_CPU_DMR
#if INTEL_FEATURE_CPU_DMR
constexpr FeatureBitset FeaturesDiamondRapids = FeaturesLioncoveServer |
// FIXME: Add FeatureMTT
// FIXME: Add FeatureUMSR
// FIXME: Add FeatureAMXF8
// FIXME: Add FeatureAVX512NECONVERTFP8
// FIXME: Add FeatureAVX512VNNIFP8
#endif // INTEL_FEATURE_CPU_DMR
#ifdef ENABLE_CPU_DMR
#if INTEL_FEATURE_ISA_AMX_SPARSE
    FeatureAMX_SPARSE |
#endif // INTEL_FEATURE_ISA_AMX_SPARSE
#if INTEL_FEATURE_ISA_AMX_TILE2
    FeatureAMX_TILE2 |
#endif // INTEL_FEATURE_ISA_AMX_TILE2
#if INTEL_FEATURE_ISA_AMX_MOVRS
    FeatureAMX_MOVRS |
#endif // INTEL_FEATURE_ISA_AMX_MOVRS
#if INTEL_FEATURE_ISA_AVX512_MOVRS
    FeatureAVX512MOVRS |
#endif // INTEL_FEATURE_ISA_AVX512_MOVRS
#if INTEL_FEATURE_ISA_AVX512_MINMAX
    FeatureAVX512MINMAX |
#endif // INTEL_FEATURE_ISA_AVX512_MINMAX
#if INTEL_FEATURE_ISA_AVX512_COMPLEX
    FeatureAVX512COMPLEX |
#endif // INTEL_FEATURE_ISA_AVX512_COMPLEX
#if INTEL_FEATURE_ISA_AVX512_SAT_CVT
    FeatureAVX512SATCVT |
#endif // INTEL_FEATURE_ISA_AVX512_SAT_CVT
#if INTEL_FEATURE_ISA_AVX256P
    FeatureAVX256P |
#endif // INTEL_FEATURE_ISA_AVX256P
    FeatureRAOINT |
#if INTEL_FEATURE_ISA_AVX_RAO_FP
    FeatureAVXRAOFP |
#endif // INTEL_FEATURE_ISA_AVX_RAO_FP
#if INTEL_FEATURE_ISA_AVX512_VPMM
    FeatureAVX512VPMM |
#endif // INTEL_FEATURE_ISA_AVX512_VPMM
#if INTEL_FEATURE_ISA_AVX512_RAO_FP
    FeatureAVX512RAOFP;
#endif // INTEL_FEATURE_ISA_AVX512_RAO_FP
#endif // ENABLE_CPU_DMR
#endif // INTEL_CUSTOMIZATION

// Intel Atom processors.
// Bonnell has feature parity with Core2 and adds MOVBE.
constexpr FeatureBitset FeaturesBonnell = FeaturesCore2 | FeatureMOVBE;
// Silvermont has parity with Westmere and Bonnell plus PRFCHW and RDRND.
constexpr FeatureBitset FeaturesSilvermont =
    FeaturesBonnell | FeaturesWestmere | FeaturePRFCHW | FeatureRDRND;
constexpr FeatureBitset FeaturesGoldmont =
    FeaturesSilvermont | FeatureAES | FeatureCLFLUSHOPT | FeatureFSGSBASE |
    FeatureRDSEED | FeatureSHA | FeatureXSAVE | FeatureXSAVEC |
    FeatureXSAVEOPT | FeatureXSAVES;
constexpr FeatureBitset FeaturesGoldmontPlus =
    FeaturesGoldmont | FeaturePTWRITE | FeatureRDPID | FeatureSGX;
constexpr FeatureBitset FeaturesTremont =
    FeaturesGoldmontPlus | FeatureCLWB | FeatureGFNI;
constexpr FeatureBitset FeaturesAlderlake =
    FeaturesTremont | FeatureADX | FeatureBMI | FeatureBMI2 | FeatureF16C |
    FeatureFMA | FeatureINVPCID | FeatureLZCNT | FeaturePCONFIG | FeaturePKU |
    FeatureSERIALIZE | FeatureSHSTK | FeatureVAES | FeatureVPCLMULQDQ |
    FeatureCLDEMOTE | FeatureMOVDIR64B | FeatureMOVDIRI | FeatureWAITPKG |
    FeatureAVXVNNI | FeatureHRESET | FeatureWIDEKL;
constexpr FeatureBitset FeaturesSierraforest =
    FeaturesAlderlake | FeatureCMPCCXADD | FeatureAVXIFMA | FeatureUINTR |
    FeatureENQCMD | FeatureAVXNECONVERT | FeatureAVXVNNIINT8;
constexpr FeatureBitset FeaturesGrandridge =
    FeaturesSierraforest | FeatureRAOINT;
constexpr FeatureBitset FeaturesArrowlakeS = FeaturesSierraforest |
    FeatureAVXVNNIINT16 | FeatureSHA512 | FeatureSM3 | FeatureSM4;
constexpr FeatureBitset FeaturesPantherlake =
    FeaturesArrowlakeS | FeaturePREFETCHI;
constexpr FeatureBitset FeaturesClearwaterforest =
    FeaturesArrowlakeS | FeatureUSERMSR | FeaturePREFETCHI;

// Geode Processor.
constexpr FeatureBitset FeaturesGeode =
    FeatureX87 | FeatureCMPXCHG8B | FeatureMMX | Feature3DNOW | Feature3DNOWA;

// K6 processor.
constexpr FeatureBitset FeaturesK6 = FeatureX87 | FeatureCMPXCHG8B | FeatureMMX;

// K7 and K8 architecture processors.
constexpr FeatureBitset FeaturesAthlon =
    FeatureX87 | FeatureCMPXCHG8B | FeatureMMX | Feature3DNOW | Feature3DNOWA;
constexpr FeatureBitset FeaturesAthlonXP =
    FeaturesAthlon | FeatureFXSR | FeatureSSE;
constexpr FeatureBitset FeaturesK8 =
    FeaturesAthlonXP | FeatureSSE2 | Feature64BIT;
constexpr FeatureBitset FeaturesK8SSE3 = FeaturesK8 | FeatureSSE3;
constexpr FeatureBitset FeaturesAMDFAM10 =
    FeaturesK8SSE3 | FeatureCMPXCHG16B | FeatureLZCNT | FeaturePOPCNT |
    FeaturePRFCHW | FeatureSAHF | FeatureSSE4_A;

// Bobcat architecture processors.
constexpr FeatureBitset FeaturesBTVER1 =
    FeatureX87 | FeatureCMPXCHG8B | FeatureCMPXCHG16B | Feature64BIT |
    FeatureFXSR | FeatureLZCNT | FeatureMMX | FeaturePOPCNT | FeaturePRFCHW |
    FeatureSSE | FeatureSSE2 | FeatureSSE3 | FeatureSSSE3 | FeatureSSE4_A |
    FeatureSAHF;
constexpr FeatureBitset FeaturesBTVER2 =
    FeaturesBTVER1 | FeatureAES | FeatureAVX | FeatureBMI | FeatureCRC32 |
    FeatureF16C | FeatureMOVBE | FeaturePCLMUL | FeatureXSAVE | FeatureXSAVEOPT;

// AMD Bulldozer architecture processors.
constexpr FeatureBitset FeaturesBDVER1 =
    FeatureX87 | FeatureAES | FeatureAVX | FeatureCMPXCHG8B |
    FeatureCMPXCHG16B | FeatureCRC32 | Feature64BIT | FeatureFMA4 |
    FeatureFXSR | FeatureLWP | FeatureLZCNT | FeatureMMX | FeaturePCLMUL |
    FeaturePOPCNT | FeaturePRFCHW | FeatureSAHF | FeatureSSE | FeatureSSE2 |
    FeatureSSE3 | FeatureSSSE3 | FeatureSSE4_1 | FeatureSSE4_2 | FeatureSSE4_A |
    FeatureXOP | FeatureXSAVE;
constexpr FeatureBitset FeaturesBDVER2 =
    FeaturesBDVER1 | FeatureBMI | FeatureFMA | FeatureF16C | FeatureTBM;
constexpr FeatureBitset FeaturesBDVER3 =
    FeaturesBDVER2 | FeatureFSGSBASE | FeatureXSAVEOPT;
constexpr FeatureBitset FeaturesBDVER4 = FeaturesBDVER3 | FeatureAVX2 |
                                         FeatureBMI2 | FeatureMOVBE |
                                         FeatureMWAITX | FeatureRDRND;

// AMD Zen architecture processors.
constexpr FeatureBitset FeaturesZNVER1 =
    FeatureX87 | FeatureADX | FeatureAES | FeatureAVX | FeatureAVX2 |
    FeatureBMI | FeatureBMI2 | FeatureCLFLUSHOPT | FeatureCLZERO |
    FeatureCMPXCHG8B | FeatureCMPXCHG16B | FeatureCRC32 | Feature64BIT |
    FeatureF16C | FeatureFMA | FeatureFSGSBASE | FeatureFXSR | FeatureLZCNT |
    FeatureMMX | FeatureMOVBE | FeatureMWAITX | FeaturePCLMUL | FeaturePOPCNT |
    FeaturePRFCHW | FeatureRDRND | FeatureRDSEED | FeatureSAHF | FeatureSHA |
    FeatureSSE | FeatureSSE2 | FeatureSSE3 | FeatureSSSE3 | FeatureSSE4_1 |
    FeatureSSE4_2 | FeatureSSE4_A | FeatureXSAVE | FeatureXSAVEC |
    FeatureXSAVEOPT | FeatureXSAVES;
constexpr FeatureBitset FeaturesZNVER2 = FeaturesZNVER1 | FeatureCLWB |
                                         FeatureRDPID | FeatureRDPRU |
                                         FeatureWBNOINVD;
static constexpr FeatureBitset FeaturesZNVER3 = FeaturesZNVER2 |
                                                FeatureINVPCID | FeaturePKU |
                                                FeatureVAES | FeatureVPCLMULQDQ;
static constexpr FeatureBitset FeaturesZNVER4 =
    FeaturesZNVER3 | FeatureAVX512F | FeatureEVEX512 | FeatureAVX512CD |
    FeatureAVX512DQ | FeatureAVX512BW | FeatureAVX512VL | FeatureAVX512IFMA |
    FeatureAVX512VBMI | FeatureAVX512VBMI2 | FeatureAVX512VNNI |
    FeatureAVX512BITALG | FeatureAVX512VPOPCNTDQ | FeatureAVX512BF16 |
    FeatureGFNI | FeatureSHSTK;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CPU_RYL
#define ENABLE_CPU_RYL
constexpr FeatureBitset FeaturesRoyal =
#endif // INTEL_FEATURE_CPU_RYL
#ifdef ENABLE_CPU_RYL
#if INTEL_FEATURE_ISA_AVX256P
    FeatureAVX256P |
#endif // INTEL_FEATURE_ISA_AVX256P
#endif // ENABLE_CPU_RYL
#if INTEL_FEATURE_CPU_RYL
    FeaturesAlderlake;
#undef ENABLE_CPU_RYL
#endif // INTEL_FEATURE_CPU_RYL
#endif // INTEL_CUSTOMIZATION

// D151696 tranplanted Mangling and OnlyForCPUDispatchSpecific from
// X86TargetParser.def to here. They are assigned by following ways:
// 1. Copy the mangling from the original CPU_SPEICIFC MACROs. If no, assign
// to '\0' by default, which means not support cpu_specific/dispatch feature.
// 2. set OnlyForCPUDispatchSpecific as true if this cpu name was not
// listed here before, which means it doesn't support -march, -mtune and so on.
// FIXME: Remove OnlyForCPUDispatchSpecific after all CPUs here support both
// cpu_dispatch/specific() feature and -march, -mtune, and so on.
// clang-format off
constexpr ProcInfo Processors[] = {
 // Empty processor. Include X87 and CMPXCHG8 for backwards compatibility.
  { {""}, CK_None, ~0U, FeatureX87 | FeatureCMPXCHG8B, '\0', false },
  { {"generic"}, CK_None, ~0U, FeatureX87 | FeatureCMPXCHG8B | Feature64BIT, 'A', true },
  // i386-generation processors.
  { {"i386"}, CK_i386, ~0U, FeatureX87, '\0', false },
  // i486-generation processors.
  { {"i486"}, CK_i486, ~0U, FeatureX87, '\0', false },
  { {"winchip-c6"}, CK_WinChipC6, ~0U, FeaturesPentiumMMX, '\0', false },
  { {"winchip2"}, CK_WinChip2, ~0U, FeaturesPentiumMMX | Feature3DNOW, '\0', false },
  { {"c3"}, CK_C3, ~0U, FeaturesPentiumMMX | Feature3DNOW, '\0', false },
  // i586-generation processors, P5 microarchitecture based.
  { {"i586"}, CK_i586, ~0U, FeatureX87 | FeatureCMPXCHG8B, '\0', false },
  { {"pentium"}, CK_Pentium, ~0U, FeatureX87 | FeatureCMPXCHG8B, 'B', false },
  { {"pentium-mmx"}, CK_PentiumMMX, ~0U, FeaturesPentiumMMX, '\0', false },
  { {"pentium_mmx"}, CK_PentiumMMX, ~0U, FeaturesPentiumMMX, 'D', true },
  // i686-generation processors, P6 / Pentium M microarchitecture based.
  { {"pentiumpro"}, CK_PentiumPro, ~0U, FeatureCMOV | FeatureX87 | FeatureCMPXCHG8B, 'C', false },
  { {"pentium_pro"}, CK_PentiumPro, ~0U, FeatureCMOV | FeatureX87 | FeatureCMPXCHG8B, 'C', true },
  { {"i686"}, CK_i686, ~0U, FeatureCMOV | FeatureX87 | FeatureCMPXCHG8B, '\0', false },
  { {"pentium2"}, CK_Pentium2, ~0U, FeaturesPentium2, 'E', false },
  { {"pentium_ii"}, CK_Pentium2, ~0U, FeaturesPentium2, 'E', true },
  { {"pentium3"}, CK_Pentium3, ~0U, FeaturesPentium3, 'H', false },
  { {"pentium3m"}, CK_Pentium3, ~0U, FeaturesPentium3, 'H', false },
  { {"pentium_iii"}, CK_Pentium3, ~0U, FeaturesPentium3, 'H', true },
  { {"pentium_iii_no_xmm_regs"}, CK_Pentium3, ~0U, FeaturesPentium3, 'H', true },
  { {"pentium-m"}, CK_PentiumM, ~0U, FeaturesPentium4, 'k', false }, // INTEL
  { {"pentium_m"}, CK_PentiumM, ~0U, FeaturesPentium4, 'K', true },
  { {"c3-2"}, CK_C3_2, ~0U, FeaturesPentium3, 'H', false }, // INTEL
  { {"yonah"}, CK_Yonah, ~0U, FeaturesPrescott, 'L', false },
  // Netburst microarchitecture based processors.
  { {"pentium4"}, CK_Pentium4, ~0U, FeaturesPentium4, 'J', false },
  { {"pentium4m"}, CK_Pentium4, ~0U, FeaturesPentium4, 'J', false },
  { {"pentium_4"}, CK_Pentium4, ~0U, FeaturesPentium4, 'J', true },
  { {"pentium_4_sse3"}, CK_Prescott, ~0U, FeaturesPrescott, 'L', true },
  { {"prescott"}, CK_Prescott, ~0U, FeaturesPrescott, 'L', false },
  { {"nocona"}, CK_Nocona, ~0U, FeaturesNocona, 'L', false },
  // Core microarchitecture based processors.
  { {"core2"}, CK_Core2, FEATURE_SSSE3, FeaturesCore2, 'M', false },
  { {"core_2_duo_ssse3"}, CK_Core2, ~0U, FeaturesCore2, 'M', true },
  { {"penryn"}, CK_Penryn, ~0U, FeaturesPenryn, 'N', false },
  { {"core_2_duo_sse4_1"}, CK_Penryn, ~0U, FeaturesPenryn, 'N', true },
  // Atom processors
  { {"bonnell"}, CK_Bonnell, FEATURE_SSSE3, FeaturesBonnell, 'O', false },
  { {"atom"}, CK_Bonnell, FEATURE_SSSE3, FeaturesBonnell, 'O', false },
  { {"silvermont"}, CK_Silvermont, FEATURE_SSE4_2, FeaturesSilvermont, 'c', false },
  { {"slm"}, CK_Silvermont, FEATURE_SSE4_2, FeaturesSilvermont, 'c', false },
  { {"atom_sse4_2"}, CK_Nehalem, FEATURE_SSE4_2, FeaturesNehalem, 'c', true },
  { {"atom_sse4_2_movbe"}, CK_Goldmont, FEATURE_SSE4_2, FeaturesGoldmont, 'd', true },
  { {"goldmont"}, CK_Goldmont, FEATURE_SSE4_2, FeaturesGoldmont, 'i', false },
  { {"goldmont-plus"}, CK_GoldmontPlus, FEATURE_SSE4_2, FeaturesGoldmontPlus, 'd', false }, // INTEL
  { {"goldmont_plus"}, CK_GoldmontPlus, FEATURE_SSE4_2, FeaturesGoldmontPlus, 'd', true },
  { {"tremont"}, CK_Tremont, FEATURE_SSE4_2, FeaturesTremont, 'd', false },
  // Nehalem microarchitecture based processors.
  { {"nehalem"}, CK_Nehalem, FEATURE_SSE4_2, FeaturesNehalem, 'P', false },
  { {"core_i7_sse4_2"}, CK_Nehalem, FEATURE_SSE4_2, FeaturesNehalem, 'P', true },
  { {"corei7"}, CK_Nehalem, FEATURE_SSE4_2, FeaturesNehalem, 'P', false },
  // Westmere microarchitecture based processors.
  { {"westmere"}, CK_Westmere, FEATURE_PCLMUL, FeaturesWestmere, 'Q', false },
  { {"core_aes_pclmulqdq"}, CK_Nehalem, FEATURE_SSE4_2, FeaturesNehalem, 'Q', true },
  // Sandy Bridge microarchitecture based processors.
  { {"sandybridge"}, CK_SandyBridge, FEATURE_AVX, FeaturesSandyBridge, 'R', false },
  { {"core_2nd_gen_avx"}, CK_SandyBridge, FEATURE_AVX, FeaturesSandyBridge, 'R', true },
  { {"corei7-avx"}, CK_SandyBridge, FEATURE_AVX, FeaturesSandyBridge, 'R', false }, // INTEL
  // Ivy Bridge microarchitecture based processors.
  { {"ivybridge"}, CK_IvyBridge, FEATURE_AVX, FeaturesIvyBridge, 'S', false },
  { {"core_3rd_gen_avx"}, CK_IvyBridge, FEATURE_AVX, FeaturesIvyBridge, 'S', true },
  { {"core-avx-i"}, CK_IvyBridge, FEATURE_AVX, FeaturesIvyBridge, 'S', false }, // INTEL
  // Haswell microarchitecture based processors.
  { {"haswell"}, CK_Haswell, FEATURE_AVX2, FeaturesHaswell, 'V', false },
  { {"core-avx2"}, CK_Haswell, FEATURE_AVX2, FeaturesHaswell, 'V', false }, // INTEL
  { {"core_4th_gen_avx"}, CK_Haswell, FEATURE_AVX2, FeaturesHaswell, 'V', true },
  { {"core_4th_gen_avx_tsx"}, CK_Haswell, FEATURE_AVX2, FeaturesHaswell, 'W', true },
  // Broadwell microarchitecture based processors.
  { {"broadwell"}, CK_Broadwell, FEATURE_AVX2, FeaturesBroadwell, 'X', false },
  { {"core_5th_gen_avx"}, CK_Broadwell, FEATURE_AVX2, FeaturesBroadwell, 'X', true },
  { {"core_5th_gen_avx_tsx"}, CK_Broadwell, FEATURE_AVX2, FeaturesBroadwell, 'Y', true },
  // Skylake client microarchitecture based processors.
  { {"skylake"}, CK_SkylakeClient, FEATURE_AVX2, FeaturesSkylakeClient, 'b', false },
  // Skylake server microarchitecture based processors.
  { {"skylake-avx512"}, CK_SkylakeServer, FEATURE_AVX512F, FeaturesSkylakeServer, 'a', false }, // INTEL
  { {"skx"}, CK_SkylakeServer, FEATURE_AVX512F, FeaturesSkylakeServer, 'a', false },
  { {"skylake_avx512"}, CK_SkylakeServer, FEATURE_AVX512F, FeaturesSkylakeServer, 'a', true },
  // Cascadelake Server microarchitecture based processors.
  { {"cascadelake"}, CK_Cascadelake, FEATURE_AVX512VNNI, FeaturesCascadeLake, 'o', false },
  // Cooperlake Server microarchitecture based processors.
  { {"cooperlake"}, CK_Cooperlake, FEATURE_AVX512BF16, FeaturesCooperLake, 'f', false },
  // Cannonlake client microarchitecture based processors.
  { {"cannonlake"}, CK_Cannonlake, FEATURE_AVX512VBMI, FeaturesCannonlake, 'e', false },
  // Icelake client microarchitecture based processors.
  { {"icelake-client"}, CK_IcelakeClient, FEATURE_AVX512VBMI2, FeaturesICLClient, 'k', false }, // INTEL
  { {"icelake_client"}, CK_IcelakeClient, FEATURE_AVX512VBMI2, FeaturesICLClient, 'k', true },
  // Rocketlake microarchitecture based processors.
  { {"rocketlake"}, CK_Rocketlake, FEATURE_AVX512VBMI2, FeaturesRocketlake, 'k', false },
  // Icelake server microarchitecture based processors.
  { {"icelake-server"}, CK_IcelakeServer, FEATURE_AVX512VBMI2, FeaturesICLServer, 'k', false }, // INTEL
  { {"icelake_server"}, CK_IcelakeServer, FEATURE_AVX512VBMI2, FeaturesICLServer, 'k', true },
  // Tigerlake microarchitecture based processors.
  { {"tigerlake"}, CK_Tigerlake, FEATURE_AVX512VP2INTERSECT, FeaturesTigerlake, 'l', false },
  // Sapphire Rapids microarchitecture based processors.
  { {"sapphirerapids"}, CK_SapphireRapids, FEATURE_AVX512BF16, FeaturesSapphireRapids, 'n', false },
  // Alderlake microarchitecture based processors.
  { {"alderlake"}, CK_Alderlake, FEATURE_AVX2, FeaturesAlderlake, 'p', false },
  // Raptorlake microarchitecture based processors.
  { {"raptorlake"}, CK_Raptorlake, FEATURE_AVX2, FeaturesAlderlake, 'p', false },
  // Meteorlake microarchitecture based processors.
  { {"meteorlake"}, CK_Meteorlake, FEATURE_AVX2, FeaturesAlderlake, 'p', false },
  // Arrowlake microarchitecture based processors.
  { {"arrowlake"}, CK_Arrowlake, FEATURE_AVX2, FeaturesSierraforest, 'p', false },
  { {"arrowlake-s"}, CK_ArrowlakeS, FEATURE_AVX2, FeaturesArrowlakeS, 'p', false }, // INTEL
  { {"arrowlake_s"}, CK_ArrowlakeS, FEATURE_AVX2, FeaturesArrowlakeS, 'p', true },
  // Lunarlake microarchitecture based processors.
  { {"lunarlake"}, CK_Lunarlake, FEATURE_AVX2, FeaturesArrowlakeS, 'p', false },
  // Gracemont microarchitecture based processors.
  { {"gracemont"}, CK_Gracemont, FEATURE_AVX2, FeaturesAlderlake, 'p', false },
  // Pantherlake microarchitecture based processors.
  { {"pantherlake"}, CK_Lunarlake, FEATURE_AVX2, FeaturesPantherlake, 'p', false },
  // Sierraforest microarchitecture based processors.
  { {"sierraforest"}, CK_Sierraforest, FEATURE_AVX2, FeaturesSierraforest, 'p', false },
  // Grandridge microarchitecture based processors.
  { {"grandridge"}, CK_Grandridge, FEATURE_AVX2, FeaturesGrandridge, 'p', false },
  // Granite Rapids microarchitecture based processors.
  { {"graniterapids"}, CK_Graniterapids, FEATURE_AVX512BF16, FeaturesGraniteRapids, 'n', false },
  // Granite Rapids D microarchitecture based processors.
  { {"graniterapids-d"}, CK_GraniterapidsD, FEATURE_AVX512BF16, FeaturesGraniteRapids | FeatureAMX_COMPLEX, 'n', false }, // INTEL
  { {"graniterapids_d"}, CK_GraniterapidsD, FEATURE_AVX512BF16, FeaturesGraniteRapids | FeatureAMX_COMPLEX, 'n', true },
  // Emerald Rapids microarchitecture based processors.
  { {"emeraldrapids"}, CK_Emeraldrapids, FEATURE_AVX512BF16, FeaturesSapphireRapids, 'n', false },
  // Clearwaterforest microarchitecture based processors.
  { {"clearwaterforest"}, CK_Lunarlake, FEATURE_AVX2, FeaturesClearwaterforest, 'p', false },
  // Knights Landing processor.
  { {"knl"}, CK_KNL, FEATURE_AVX512F, FeaturesKNL, 'Z', true }, // INTEL
  { {"mic_avx512"}, CK_KNL, FEATURE_AVX512F, FeaturesKNL, 'Z', true },
  // Knights Mill processor.
  { {"knm"}, CK_KNM, FEATURE_AVX5124FMAPS, FeaturesKNM, 'j', true }, // INTEL
  // Lakemont microarchitecture based processors.
  { {"lakemont"}, CK_Lakemont, ~0U, FeatureCMPXCHG8B, '\0', false },
  // K6 architecture processors.
  { {"k6"}, CK_K6, ~0U, FeaturesK6, '\0', false },
  { {"k6-2"}, CK_K6_2, ~0U, FeaturesK6 | Feature3DNOW, '\0', false },
  { {"k6-3"}, CK_K6_3, ~0U, FeaturesK6 | Feature3DNOW, '\0', false },
  // K7 architecture processors.
  { {"athlon"}, CK_Athlon, ~0U, FeaturesAthlon, '\0', false },
  { {"athlon-tbird"}, CK_Athlon, ~0U, FeaturesAthlon, '\0', false },
  { {"athlon-xp"}, CK_AthlonXP, ~0U, FeaturesAthlonXP, '\0', false },
  { {"athlon-mp"}, CK_AthlonXP, ~0U, FeaturesAthlonXP, '\0', false },
  { {"athlon-4"}, CK_AthlonXP, ~0U, FeaturesAthlonXP, '\0', false },
  // K8 architecture processors.
  { {"k8"}, CK_K8, ~0U, FeaturesK8, '\0', false },
  { {"athlon64"}, CK_K8, ~0U, FeaturesK8, '\0', false },
  { {"athlon-fx"}, CK_K8, ~0U, FeaturesK8, '\0', false },
  { {"opteron"}, CK_K8, ~0U, FeaturesK8, '\0', false },
  { {"k8-sse3"}, CK_K8SSE3, ~0U, FeaturesK8SSE3, '\0', false },
  { {"athlon64-sse3"}, CK_K8SSE3, ~0U, FeaturesK8SSE3, '\0', false },
  { {"opteron-sse3"}, CK_K8SSE3, ~0U, FeaturesK8SSE3, '\0', false },
  { {"amdfam10"}, CK_AMDFAM10, FEATURE_SSE4_A, FeaturesAMDFAM10, '\0', false },
  { {"barcelona"}, CK_AMDFAM10, FEATURE_SSE4_A, FeaturesAMDFAM10, '\0', false },
  // Bobcat architecture processors.
  { {"btver1"}, CK_BTVER1, FEATURE_SSE4_A, FeaturesBTVER1, '\0', false },
  { {"btver2"}, CK_BTVER2, FEATURE_BMI, FeaturesBTVER2, '\0', false },
  // Bulldozer architecture processors.
  { {"bdver1"}, CK_BDVER1, FEATURE_XOP, FeaturesBDVER1, '\0', false },
  { {"bdver2"}, CK_BDVER2, FEATURE_FMA, FeaturesBDVER2, '\0', false },
  { {"bdver3"}, CK_BDVER3, FEATURE_FMA, FeaturesBDVER3, '\0', false },
  { {"bdver4"}, CK_BDVER4, FEATURE_AVX2, FeaturesBDVER4, '\0', false },
  // Zen architecture processors.
  { {"znver1"}, CK_ZNVER1, FEATURE_AVX2, FeaturesZNVER1, '\0', false },
  { {"znver2"}, CK_ZNVER2, FEATURE_AVX2, FeaturesZNVER2, '\0', false },
  { {"znver3"}, CK_ZNVER3, FEATURE_AVX2, FeaturesZNVER3, '\0', false },
  { {"znver4"}, CK_ZNVER4, FEATURE_AVX512VBMI2, FeaturesZNVER4, '\0', false },
  // Generic 64-bit processor.
#if INTEL_CUSTOMIZATION
  { {"x86-64"}, CK_x86_64, FEATURE_SSE2, FeaturesX86_64, 'J', false },
  { {"x86-64-v2"}, CK_x86_64_v2, FEATURE_SSE4_2, FeaturesX86_64_V2, 'R', false },
  { {"x86-64-v3"}, CK_x86_64_v3, FEATURE_AVX2, FeaturesX86_64_V3, 'V', false },
  { {"x86-64-v4"}, CK_x86_64_v4, FEATURE_AVX512VL, FeaturesX86_64_V4, 'a', false },
  // Intersection of SKX and KNL.
  { {"common-avx512"}, CK_CommonAVX512, ~0U, FeaturesCommonAVX512, 'q', false },
  { {"common_avx512"}, CK_CommonAVX512, ~0U, FeaturesCommonAVX512, 'q', true },
#if INTEL_FEATURE_ISA_AVX256P
  // Intersection of AVX256
  { {"common-avx256"}, CK_CommonAVX256, ~0U, FeaturesCommonAVX256, '\0', false },
#endif // INTEL_FEATURE_ISA_AVX256P
#if INTEL_FEATURE_CPU_DMR
  // Diamondrapids microarchitecture based processors.
  { {"diamondrapids"}, CK_Diamondrapids, FEATURE_AVX512VP2INTERSECT, FeaturesDiamondRapids, 'n', false },
#endif // INTEL_FEATURE_CPU_DMR
#if INTEL_FEATURE_CPU_RYL
  // Royal microarchitecture based processors.
  { {"royal"}, CK_Royal, FEATURE_AVX2, FeaturesRoyal, '\0', false },
#endif // INTEL_FEATURE_CPU_RYL
#endif // INTEL_CUSTOMIZATION
  // Geode processors.
  { {"geode"}, CK_Geode, ~0U, FeaturesGeode, '\0', false },
};
// clang-format on

constexpr const char *NoTuneList[] = {"x86-64-v2", "x86-64-v3", "x86-64-v4"};

X86::CPUKind llvm::X86::parseArchX86(StringRef CPU, bool Only64Bit) {
  for (const auto &P : Processors)
    if (!P.OnlyForCPUDispatchSpecific && P.Name == CPU &&
        (P.Features[FEATURE_64BIT] || !Only64Bit))
      return P.Kind;

  return CK_None;
}

X86::CPUKind llvm::X86::parseTuneCPU(StringRef CPU, bool Only64Bit) {
  if (llvm::is_contained(NoTuneList, CPU))
    return CK_None;
  return parseArchX86(CPU, Only64Bit);
}

void llvm::X86::fillValidCPUArchList(SmallVectorImpl<StringRef> &Values,
                                     bool Only64Bit) {
  for (const auto &P : Processors)
    if (!P.OnlyForCPUDispatchSpecific && !P.Name.empty() &&
        (P.Features[FEATURE_64BIT] || !Only64Bit))
      Values.emplace_back(P.Name);
}

void llvm::X86::fillValidTuneCPUList(SmallVectorImpl<StringRef> &Values,
                                     bool Only64Bit) {
  for (const ProcInfo &P : Processors)
    if (!P.OnlyForCPUDispatchSpecific && !P.Name.empty() &&
        (P.Features[FEATURE_64BIT] || !Only64Bit) &&
        !llvm::is_contained(NoTuneList, P.Name))
      Values.emplace_back(P.Name);
}

ProcessorFeatures llvm::X86::getKeyFeature(X86::CPUKind Kind) {
  // FIXME: Can we avoid a linear search here? The table might be sorted by
  // CPUKind so we could binary search?
  for (const auto &P : Processors) {
    if (P.Kind == Kind) {
      assert(P.KeyFeature != ~0U && "Processor does not have a key feature.");
      return static_cast<ProcessorFeatures>(P.KeyFeature);
    }
  }

  llvm_unreachable("Unable to find CPU kind!");
}

// Features with no dependencies.
constexpr FeatureBitset ImpliedFeatures64BIT = {};
constexpr FeatureBitset ImpliedFeaturesADX = {};
constexpr FeatureBitset ImpliedFeaturesBMI = {};
constexpr FeatureBitset ImpliedFeaturesBMI2 = {};
constexpr FeatureBitset ImpliedFeaturesCLDEMOTE = {};
constexpr FeatureBitset ImpliedFeaturesCLFLUSHOPT = {};
constexpr FeatureBitset ImpliedFeaturesCLWB = {};
constexpr FeatureBitset ImpliedFeaturesCLZERO = {};
constexpr FeatureBitset ImpliedFeaturesCMOV = {};
constexpr FeatureBitset ImpliedFeaturesCMPXCHG16B = {};
constexpr FeatureBitset ImpliedFeaturesCMPXCHG8B = {};
constexpr FeatureBitset ImpliedFeaturesCRC32 = {};
constexpr FeatureBitset ImpliedFeaturesENQCMD = {};
constexpr FeatureBitset ImpliedFeaturesFSGSBASE = {};
constexpr FeatureBitset ImpliedFeaturesFXSR = {};
constexpr FeatureBitset ImpliedFeaturesINVPCID = {};
constexpr FeatureBitset ImpliedFeaturesLWP = {};
constexpr FeatureBitset ImpliedFeaturesLZCNT = {};
constexpr FeatureBitset ImpliedFeaturesMWAITX = {};
constexpr FeatureBitset ImpliedFeaturesMOVBE = {};
constexpr FeatureBitset ImpliedFeaturesMOVDIR64B = {};
constexpr FeatureBitset ImpliedFeaturesMOVDIRI = {};
constexpr FeatureBitset ImpliedFeaturesPCONFIG = {};
constexpr FeatureBitset ImpliedFeaturesPOPCNT = {};
constexpr FeatureBitset ImpliedFeaturesPKU = {};
constexpr FeatureBitset ImpliedFeaturesPREFETCHWT1 = {};
constexpr FeatureBitset ImpliedFeaturesPRFCHW = {};
constexpr FeatureBitset ImpliedFeaturesPTWRITE = {};
constexpr FeatureBitset ImpliedFeaturesRDPID = {};
constexpr FeatureBitset ImpliedFeaturesRDPRU = {};
constexpr FeatureBitset ImpliedFeaturesRDRND = {};
constexpr FeatureBitset ImpliedFeaturesRDSEED = {};
constexpr FeatureBitset ImpliedFeaturesRTM = {};
constexpr FeatureBitset ImpliedFeaturesSAHF = {};
constexpr FeatureBitset ImpliedFeaturesSERIALIZE = {};
constexpr FeatureBitset ImpliedFeaturesSGX = {};
constexpr FeatureBitset ImpliedFeaturesSHSTK = {};
constexpr FeatureBitset ImpliedFeaturesTBM = {};
constexpr FeatureBitset ImpliedFeaturesTSXLDTRK = {};
constexpr FeatureBitset ImpliedFeaturesUINTR = {};
constexpr FeatureBitset ImpliedFeaturesUSERMSR = {};
constexpr FeatureBitset ImpliedFeaturesWAITPKG = {};
constexpr FeatureBitset ImpliedFeaturesWBNOINVD = {};
constexpr FeatureBitset ImpliedFeaturesVZEROUPPER = {};
constexpr FeatureBitset ImpliedFeaturesX87 = {};
constexpr FeatureBitset ImpliedFeaturesXSAVE = {};

// Not really CPU features, but need to be in the table because clang uses
// target features to communicate them to the backend.
constexpr FeatureBitset ImpliedFeaturesRETPOLINE_EXTERNAL_THUNK = {};
constexpr FeatureBitset ImpliedFeaturesRETPOLINE_INDIRECT_BRANCHES = {};
constexpr FeatureBitset ImpliedFeaturesRETPOLINE_INDIRECT_CALLS = {};
constexpr FeatureBitset ImpliedFeaturesLVI_CFI = {};
constexpr FeatureBitset ImpliedFeaturesLVI_LOAD_HARDENING = {};

// XSAVE features are dependent on basic XSAVE.
constexpr FeatureBitset ImpliedFeaturesXSAVEC = FeatureXSAVE;
constexpr FeatureBitset ImpliedFeaturesXSAVEOPT = FeatureXSAVE;
constexpr FeatureBitset ImpliedFeaturesXSAVES = FeatureXSAVE;

// MMX->3DNOW->3DNOWA chain.
constexpr FeatureBitset ImpliedFeaturesMMX = {};
constexpr FeatureBitset ImpliedFeatures3DNOW = FeatureMMX;
constexpr FeatureBitset ImpliedFeatures3DNOWA = Feature3DNOW;

// SSE/AVX/AVX512F chain.
constexpr FeatureBitset ImpliedFeaturesSSE = {};
constexpr FeatureBitset ImpliedFeaturesSSE2 = FeatureSSE;
constexpr FeatureBitset ImpliedFeaturesSSE3 = FeatureSSE2;
constexpr FeatureBitset ImpliedFeaturesSSSE3 = FeatureSSE3;
constexpr FeatureBitset ImpliedFeaturesSSE4_1 = FeatureSSSE3;
constexpr FeatureBitset ImpliedFeaturesSSE4_2 = FeatureSSE4_1;
constexpr FeatureBitset ImpliedFeaturesAVX = FeatureSSE4_2;
constexpr FeatureBitset ImpliedFeaturesAVX2 = FeatureAVX;
constexpr FeatureBitset ImpliedFeaturesEVEX512 = {};
constexpr FeatureBitset ImpliedFeaturesAVX512F =
    FeatureAVX2 | FeatureF16C | FeatureFMA;

// Vector extensions that build on SSE or AVX.
constexpr FeatureBitset ImpliedFeaturesAES = FeatureSSE2;
constexpr FeatureBitset ImpliedFeaturesF16C = FeatureAVX;
constexpr FeatureBitset ImpliedFeaturesFMA = FeatureAVX;
constexpr FeatureBitset ImpliedFeaturesGFNI = FeatureSSE2;
constexpr FeatureBitset ImpliedFeaturesPCLMUL = FeatureSSE2;
constexpr FeatureBitset ImpliedFeaturesSHA = FeatureSSE2;
constexpr FeatureBitset ImpliedFeaturesVAES = FeatureAES | FeatureAVX2;
constexpr FeatureBitset ImpliedFeaturesVPCLMULQDQ = FeatureAVX | FeaturePCLMUL;
constexpr FeatureBitset ImpliedFeaturesSM3 = FeatureAVX;
constexpr FeatureBitset ImpliedFeaturesSM4 = FeatureAVX2;

// AVX512 features.
constexpr FeatureBitset ImpliedFeaturesAVX512CD = FeatureAVX512F;
constexpr FeatureBitset ImpliedFeaturesAVX512BW = FeatureAVX512F;
constexpr FeatureBitset ImpliedFeaturesAVX512DQ = FeatureAVX512F;
constexpr FeatureBitset ImpliedFeaturesAVX512ER = FeatureAVX512F;
constexpr FeatureBitset ImpliedFeaturesAVX512PF = FeatureAVX512F;
constexpr FeatureBitset ImpliedFeaturesAVX512VL = FeatureAVX512F;

constexpr FeatureBitset ImpliedFeaturesAVX512BF16 = FeatureAVX512BW;
constexpr FeatureBitset ImpliedFeaturesAVX512BITALG = FeatureAVX512BW;
constexpr FeatureBitset ImpliedFeaturesAVX512IFMA = FeatureAVX512F;
constexpr FeatureBitset ImpliedFeaturesAVX512VNNI = FeatureAVX512F;
constexpr FeatureBitset ImpliedFeaturesAVX512VPOPCNTDQ = FeatureAVX512F;
constexpr FeatureBitset ImpliedFeaturesAVX512VBMI = FeatureAVX512BW;
constexpr FeatureBitset ImpliedFeaturesAVX512VBMI2 = FeatureAVX512BW;
constexpr FeatureBitset ImpliedFeaturesAVX512VP2INTERSECT = FeatureAVX512F;

// FIXME: These two aren't really implemented and just exist in the feature
// list for __builtin_cpu_supports. So omit their dependencies.
constexpr FeatureBitset ImpliedFeaturesAVX5124FMAPS = {};
constexpr FeatureBitset ImpliedFeaturesAVX5124VNNIW = {};

// SSE4_A->FMA4->XOP chain.
constexpr FeatureBitset ImpliedFeaturesSSE4_A = FeatureSSE3;
constexpr FeatureBitset ImpliedFeaturesFMA4 = FeatureAVX | FeatureSSE4_A;
constexpr FeatureBitset ImpliedFeaturesXOP = FeatureFMA4;

// AMX Features
constexpr FeatureBitset ImpliedFeaturesAMX_TILE = {};
constexpr FeatureBitset ImpliedFeaturesAMX_BF16 = FeatureAMX_TILE;
constexpr FeatureBitset ImpliedFeaturesAMX_FP16 = FeatureAMX_TILE;
constexpr FeatureBitset ImpliedFeaturesAMX_INT8 = FeatureAMX_TILE;
constexpr FeatureBitset ImpliedFeaturesAMX_COMPLEX = FeatureAMX_TILE;
constexpr FeatureBitset ImpliedFeaturesHRESET = {};

#if INTEL_CUSTOMIZATION
constexpr FeatureBitset ImpliedFeaturesEGPR = {};
constexpr FeatureBitset ImpliedFeaturesPush2Pop2 = {};
constexpr FeatureBitset ImpliedFeaturesNDD = {};
constexpr FeatureBitset ImpliedFeaturesCCMP = {};
constexpr FeatureBitset ImpliedFeaturesCF = {};
#if INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
constexpr FeatureBitset ImpliedFeaturesAVXDOTPRODPHPS = FeatureAVX2;
#endif // INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
#if INTEL_FEATURE_ISA_AVX_CONVERT
constexpr FeatureBitset ImpliedFeaturesAVXCONVERT = FeatureAVX2;
#endif // INTEL_FEATURE_ISA_AVX_CONVERT
#if INTEL_FEATURE_ISA_AVX_BF16
constexpr FeatureBitset ImpliedFeaturesAVXBF16 = FeatureAVX2;
#endif // INTEL_FEATURE_ISA_AVX_BF16
#if INTEL_FEATURE_ISA_AVX_COMPRESS
constexpr FeatureBitset ImpliedFeaturesAVXCOMPRESS = FeatureAVX2;
#endif // INTEL_FEATURE_ISA_AVX_COMPRESS
#if INTEL_FEATURE_ISA_AVX512_MOVRS
constexpr FeatureBitset ImpliedFeaturesAVXMOVRS = FeatureAVX2;
constexpr FeatureBitset ImpliedFeaturesAVX512MOVRS = FeatureAVX512F;
#endif // INTEL_FEATURE_ISA_AVX512_MOVRS
#if INTEL_FEATURE_ISA_AVX512_MEDIAX
constexpr FeatureBitset ImpliedFeaturesAVX512MEDIAX = FeatureAVX512F |
    FeatureAVX512BW;
#endif // INTEL_FEATURE_ISA_AVX512_MEDIAX
#if INTEL_FEATURE_ISA_AVX_MOVGET
constexpr FeatureBitset ImpliedFeaturesAVXMOVGET = FeatureAVX2;
#endif // INTEL_FEATURE_ISA_AVX_MOVGET
#if INTEL_FEATURE_ISA_AVX512_MOVGET
constexpr FeatureBitset ImpliedFeaturesAVX512MOVGET = FeatureAVX512F;
#endif // INTEL_FEATURE_ISA_AVX512_MOVGET
#if INTEL_FEATURE_ISA_AVX512_VNNI_INT8
constexpr FeatureBitset ImpliedFeaturesAVX512VNNIINT8 = FeatureAVX512F |
    FeatureAVXVNNIINT8;
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT8
#if INTEL_FEATURE_ISA_AVX512_VNNI_FP16
constexpr FeatureBitset ImpliedFeaturesAVX512VNNIFP16 = FeatureAVX512F |
    FeatureAVX512FP16 | FeatureAVX512BW | FeatureAVX512DQ | FeatureAVX512VL |
    FeatureAVXDOTPRODPHPS;
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_FP16
#if INTEL_FEATURE_ISA_AVX512_CONVERT
constexpr FeatureBitset ImpliedFeaturesAVX512CONVERT = FeatureAVX512F |
    FeatureAVX512FP16 | FeatureAVX512BW | FeatureAVX512DQ | FeatureAVX512VL |
    FeatureAVXCONVERT | FeatureAVX512BF16;
#endif // INTEL_FEATURE_ISA_AVX512_CONVERT
#if INTEL_FEATURE_ISA_AMX_FUTURE
constexpr FeatureBitset ImpliedFeaturesAMX_REDUCE = FeatureAMX_TILE;
constexpr FeatureBitset ImpliedFeaturesAMX_MEMORY = FeatureAMX_TILE;
constexpr FeatureBitset ImpliedFeaturesAMX_FORMAT = FeatureAMX_TILE;
constexpr FeatureBitset ImpliedFeaturesAMX_ELEMENT = FeatureAMX_TILE;
#endif // INTEL_FEATURE_ISA_AMX_FUTURE
#if INTEL_FEATURE_ISA_AMX_TRANSPOSE
constexpr FeatureBitset ImpliedFeaturesAMX_TRANSPOSE = FeatureAMX_TILE;
#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE
#if INTEL_FEATURE_ISA_AMX_MEMORY2
constexpr FeatureBitset ImpliedFeaturesAMX_MEMORY2 = FeatureAMX_TILE;
#endif // INTEL_FEATURE_ISA_AMX_MEMORY2
#if INTEL_FEATURE_ISA_AMX_BF16_EVEX
constexpr FeatureBitset ImpliedFeaturesAMX_BF16_EVEX = FeatureAMX_TILE;
#endif // INTEL_FEATURE_ISA_AMX_BF16_EVEX
#if INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
constexpr FeatureBitset ImpliedFeaturesAMX_ELEMENT_EVEX = FeatureAMX_TILE;
#endif // INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
#if INTEL_FEATURE_ISA_AMX_INT8_EVEX
constexpr FeatureBitset ImpliedFeaturesAMX_INT8_EVEX = FeatureAMX_TILE;
#endif // INTEL_FEATURE_ISA_AMX_INT8_EVEX
#if INTEL_FEATURE_ISA_AMX_TILE_EVEX
constexpr FeatureBitset ImpliedFeaturesAMX_TILE_EVEX = FeatureAMX_TILE;
#endif // INTEL_FEATURE_ISA_AMX_TILE_EVEX
#if INTEL_FEATURE_ISA_AMX_TRANSPOSE2
constexpr FeatureBitset ImpliedFeaturesAMX_TRANSPOSE2 = FeatureAMX_TILE;
#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE2
#if INTEL_FEATURE_ISA_AMX_CONVERT
constexpr FeatureBitset ImpliedFeaturesAMX_CONVERT = FeatureAMX_TILE;
#endif // INTEL_FEATURE_ISA_AMX_CONVERT
#if INTEL_FEATURE_ISA_AMX_TILE2
constexpr FeatureBitset ImpliedFeaturesAMX_TILE2 = FeatureAMX_TILE;
#endif // INTEL_FEATURE_ISA_AMX_TILE2
#if INTEL_FEATURE_ISA_AMX_MOVRS
constexpr FeatureBitset ImpliedFeaturesAMX_MOVRS = FeatureAMX_TILE;
#endif // INTEL_FEATURE_ISA_AMX_MOVRS
#if INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
constexpr FeatureBitset ImpliedFeaturesAMX_MEMADVISE_EVEX = FeatureAMX_TILE;
#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
#if INTEL_FEATURE_ISA_AMX_TF32
constexpr FeatureBitset ImpliedFeaturesAMX_TF32 = FeatureAMX_TILE;
#endif // INTEL_FEATURE_ISA_AMX_TF32
#if INTEL_FEATURE_ISA_GPR_MOVGET
constexpr FeatureBitset ImpliedFeaturesGPRMOVGET = {};
#endif // INTEL_FEATURE_ISA_GPR_MOVGET
#if INTEL_FEATURE_ISA_MOVGET64B
constexpr FeatureBitset ImpliedFeaturesMOVGET64B = FeatureAVX512F;
#endif // INTEL_FEATURE_ISA_MOVGET64B
#if INTEL_FEATURE_ISA_AVX_RAO_INT
constexpr FeatureBitset ImpliedFeaturesAVXRAOINT = FeatureAVX2;
#endif // INTEL_FEATURE_ISA_AVX_RAO_INT
#if INTEL_FEATURE_ISA_AVX_RAO_FP
// AUTO GENERATED BY TOOL
constexpr FeatureBitset ImpliedFeaturesAVXRAOFP = FeatureAVX2;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_RAO_FP
#if INTEL_FEATURE_ISA_AVX512_RAO_INT
// AUTO GENERATED BY TOOL
constexpr FeatureBitset ImpliedFeaturesAVX512RAOINT = FeatureAVX512F;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_RAO_INT
#if INTEL_FEATURE_ISA_AVX512_RAO_FP
// AUTO GENERATED BY TOOL
constexpr FeatureBitset ImpliedFeaturesAVX512RAOFP = FeatureAVX512F;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_RAO_FP
#if INTEL_FEATURE_ISA_AMX_AVX512
// AUTO GENERATED BY TOOL
constexpr FeatureBitset ImpliedFeaturesAMX_AVX512 =
    FeatureAMX_TILE | FeatureAVX512F;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_AVX512
#if INTEL_FEATURE_ISA_AMX_AVX512_TILE16MOV
constexpr FeatureBitset ImpliedFeaturesAMX_AVX512_TILE16MOV =
    FeatureAMX_TILE | FeatureAVX512F;
#endif // INTEL_FEATURE_ISA_AMX_AVX512_TILE16MOV
#if INTEL_FEATURE_ISA_AVX512_NE_CONVERT
// AUTO GENERATED BY TOOL
constexpr FeatureBitset ImpliedFeaturesAVX512NECONVERT = FeatureAVX512F;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_NE_CONVERT
#if INTEL_FEATURE_ISA_DSPV1
// AUTO GENERATED BY TOOL
constexpr FeatureBitset ImpliedFeaturesDSPV1 = FeatureSSE3;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_DSPV1
#if INTEL_FEATURE_ISA_AVX512_VNNI_INT16
// AUTO GENERATED BY TOOL
constexpr FeatureBitset ImpliedFeaturesAVX512VNNIINT16 = FeatureAVX512F |
    FeatureAVXVNNIINT16;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT16
#if INTEL_FEATURE_ISA_AMX_SPARSE
// AUTO GENERATED BY TOOL
constexpr FeatureBitset ImpliedFeaturesAMX_SPARSE = FeatureAMX_TILE;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_SPARSE
#if INTEL_FEATURE_ISA_AMX_V3
// AUTO GENERATED BY TOOL
constexpr FeatureBitset ImpliedFeaturesAMX_V3 = FeatureAMX_TILE;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_V3
#if INTEL_FEATURE_ISA_AVX512_MOVZXC
static constexpr FeatureBitset ImpliedFeaturesAVX512MOVZXC = FeatureAVX512F;
#endif // INTEL_FEATURE_ISA_AVX512_MOVZXC
#if INTEL_FEATURE_ISA_VPINSR_VPEXTR
constexpr FeatureBitset ImpliedFeaturesVPINSR_VPEXTR = FeatureAVX512F;
#endif // INTEL_FEATURE_ISA_VPINSR_VPEXTR
#if INTEL_FEATURE_ISA_PREFETCHST2
constexpr FeatureBitset ImpliedFeaturesPREFETCHST2 = {};
#endif // INTEL_FEATURE_ISA_PREFETCHST2
#if INTEL_FEATURE_ISA_AVX512_SAT_CVT
constexpr FeatureBitset ImpliedFeaturesAVX512SATCVT = FeatureAVX512BF16 | FeatureAVX512FP16;
#endif // INTEL_FEATURE_ISA_AVX512_SAT_CVT
#if INTEL_FEATURE_ISA_AVX512_MINMAX
constexpr FeatureBitset ImpliedFeaturesAVX512MINMAX = FeatureAVX512BF16 | FeatureAVX512FP16;
#endif // INTEL_FEATURE_ISA_AVX512_MINMAX
#if INTEL_FEATURE_ISA_AVX512_COMPLEX
constexpr FeatureBitset ImpliedFeaturesAVX512COMPLEX = FeatureAVX512FP16;
#endif // INTEL_FEATURE_ISA_AVX512_COMPLEX
#if INTEL_FEATURE_ISA_AVX512_REDUCTION
constexpr FeatureBitset ImpliedFeaturesAVX512REDUCTION = FeatureAVX512F | FeatureAVX512VL | FeatureAVX512BW;
#endif // INTEL_FEATURE_ISA_AVX512_REDUCTION
#if INTEL_FEATURE_ISA_AVX512_REDUCTION
constexpr FeatureBitset ImpliedFeaturesAVX512REDUCTION2 = FeatureAVX512F | FeatureAVX512VL | FeatureAVX512BW;
#endif // INTEL_FEATURE_ISA_AVX512_REDUCTION
#if INTEL_FEATURE_ISA_AVX256P
constexpr FeatureBitset ImpliedFeaturesAVX256P = FeatureAVX2 | FeatureF16C | FeatureFMA;
#endif // INTEL_FEATURE_ISA_AVX256P
#if INTEL_FEATURE_ISA_AVX512_VNNI_FP8
// AUTO GENERATED BY TOOL
static constexpr FeatureBitset ImpliedFeaturesAVX512VNNIFP8 = FeatureAVX512F;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_FP8
#if INTEL_FEATURE_ISA_AVX512_BF16_NE
constexpr FeatureBitset ImpliedFeaturesAVX512BF16NE =
    FeatureAVX512BF16 | FeatureAVX512FP16 | FeatureAVX512BW | FeatureAVX512DQ | FeatureAVX512VL;
#endif // INTEL_FEATURE_ISA_AVX512_BF16_NE
#if INTEL_FEATURE_ISA_AMX_FP8
// AUTO GENERATED BY TOOL
constexpr FeatureBitset ImpliedFeaturesAMX_FP8 = FeatureAMX_TILE;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_FP8
#if INTEL_FEATURE_ISA_AVX512_NE_CONVERT_FP8
// AUTO GENERATED BY TOOL
constexpr FeatureBitset ImpliedFeaturesAVX512NECONVERTFP8 = FeatureAVX512FP16;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_NE_CONVERT_FP8
#if INTEL_FEATURE_ISA_AVX512_VPMM
constexpr FeatureBitset ImpliedFeaturesAVX512VPMM = FeatureAVX512F | FeatureAVX512BF16NE;
#endif // INTEL_FEATURE_ISA_AVX512_VPMM
#endif // INTEL_CUSTOMIZATION
constexpr FeatureBitset ImpliedFeaturesPREFETCHI = {};
constexpr FeatureBitset ImpliedFeaturesCMPCCXADD = {};
constexpr FeatureBitset ImpliedFeaturesRAOINT = {};
constexpr FeatureBitset ImpliedFeaturesAVXVNNIINT16 = FeatureAVX2;
constexpr FeatureBitset ImpliedFeaturesAVXVNNIINT8 = FeatureAVX2;
constexpr FeatureBitset ImpliedFeaturesAVXIFMA = FeatureAVX2;
constexpr FeatureBitset ImpliedFeaturesAVXNECONVERT = FeatureAVX2;
constexpr FeatureBitset ImpliedFeaturesSHA512 = FeatureAVX2;
constexpr FeatureBitset ImpliedFeaturesAVX512FP16 =
    FeatureAVX512BW | FeatureAVX512DQ | FeatureAVX512VL;
// Key Locker Features
constexpr FeatureBitset ImpliedFeaturesKL = FeatureSSE2;
constexpr FeatureBitset ImpliedFeaturesWIDEKL = FeatureKL;

// AVXVNNI Features
constexpr FeatureBitset ImpliedFeaturesAVXVNNI = FeatureAVX2;

// AVX10 Features
constexpr FeatureBitset ImpliedFeaturesAVX10_1 =
    FeatureAVX512CD | FeatureAVX512VBMI | FeatureAVX512IFMA |
    FeatureAVX512VNNI | FeatureAVX512BF16 | FeatureAVX512VPOPCNTDQ |
    FeatureAVX512VBMI2 | FeatureAVX512BITALG | FeatureVAES | FeatureVPCLMULQDQ |
    FeatureAVX512FP16;
constexpr FeatureBitset ImpliedFeaturesAVX10_1_512 =
    FeatureAVX10_1 | FeatureEVEX512;

constexpr FeatureInfo FeatureInfos[X86::CPU_FEATURE_MAX] = {
#define X86_FEATURE(ENUM, STR) {{"+" STR}, ImpliedFeatures##ENUM},
#include "llvm/TargetParser/X86TargetParser.def"
};

void llvm::X86::getFeaturesForCPU(StringRef CPU,
                                  SmallVectorImpl<StringRef> &EnabledFeatures,
                                  bool NeedPlus) {
  auto I = llvm::find_if(Processors,
                         [&](const ProcInfo &P) { return P.Name == CPU; });
  assert(I != std::end(Processors) && "Processor not found!");

  FeatureBitset Bits = I->Features;

  // Remove the 64-bit feature which we only use to validate if a CPU can
  // be used with 64-bit mode.
  Bits &= ~Feature64BIT;

  // Add the string version of all set bits.
  for (unsigned i = 0; i != CPU_FEATURE_MAX; ++i)
    if (Bits[i] && !FeatureInfos[i].getName(NeedPlus).empty())
      EnabledFeatures.push_back(FeatureInfos[i].getName(NeedPlus));
}

// For each feature that is (transitively) implied by this feature, set it.
static void getImpliedEnabledFeatures(FeatureBitset &Bits,
                                      const FeatureBitset &Implies) {
  // Fast path: Implies is often empty.
  if (!Implies.any())
    return;
  FeatureBitset Prev;
  Bits |= Implies;
  do {
    Prev = Bits;
    for (unsigned i = CPU_FEATURE_MAX; i;)
      if (Bits[--i])
        Bits |= FeatureInfos[i].ImpliedFeatures;
  } while (Prev != Bits);
}

/// Create bit vector of features that are implied disabled if the feature
/// passed in Value is disabled.
static void getImpliedDisabledFeatures(FeatureBitset &Bits, unsigned Value) {
  // Check all features looking for any dependent on this feature. If we find
  // one, mark it and recursively find any feature that depend on it.
  FeatureBitset Prev;
  Bits.set(Value);
  do {
    Prev = Bits;
    for (unsigned i = 0; i != CPU_FEATURE_MAX; ++i)
      if ((FeatureInfos[i].ImpliedFeatures & Bits).any())
        Bits.set(i);
  } while (Prev != Bits);
}

void llvm::X86::updateImpliedFeatures(
    StringRef Feature, bool Enabled,
    StringMap<bool> &Features) {
  auto I = llvm::find_if(FeatureInfos, [&](const FeatureInfo &FI) {
    return FI.getName() == Feature;
  });
  if (I == std::end(FeatureInfos)) {
    // FIXME: This shouldn't happen, but may not have all features in the table
    // yet.
    return;
  }

  FeatureBitset ImpliedBits;
  if (Enabled)
    getImpliedEnabledFeatures(ImpliedBits, I->ImpliedFeatures);
  else
    getImpliedDisabledFeatures(ImpliedBits,
                               std::distance(std::begin(FeatureInfos), I));

  // Update the map entry for all implied features.
  for (unsigned i = 0; i != CPU_FEATURE_MAX; ++i)
    if (ImpliedBits[i] && !FeatureInfos[i].getName().empty())
      Features[FeatureInfos[i].getName()] = Enabled;
}

#if INTEL_CUSTOMIZATION
bool llvm::X86::getCPUDispatchSupported(StringRef CPU) {
  auto I = llvm::find_if(Processors,
                         [&](const ProcInfo &P) { return P.Name == CPU; });
  return I != std::end(Processors) && I->Mangling != '\0';
}
#endif // INTEL_CUSTOMIZATION

char llvm::X86::getCPUDispatchMangling(StringRef CPU) {
  auto I = llvm::find_if(Processors,
                         [&](const ProcInfo &P) { return P.Name == CPU; });
  assert(I != std::end(Processors) && "Processor not found!");
  assert(I->Mangling != '\0' && "Processor dooesn't support function multiversion!");
  return I->Mangling;
}

bool llvm::X86::validateCPUSpecificCPUDispatch(StringRef Name) {
  auto I = llvm::find_if(Processors,
                         [&](const ProcInfo &P) { return P.Name == Name; });
  return I != std::end(Processors);
}

std::array<uint32_t, 4>
llvm::X86::getCpuSupportsMask(ArrayRef<StringRef> FeatureStrs) {
  // Processor features and mapping to processor feature value.
  std::array<uint32_t, 4> FeatureMask{};
  for (StringRef FeatureStr : FeatureStrs) {
    unsigned Feature = StringSwitch<unsigned>(FeatureStr)
#define X86_FEATURE_COMPAT(ENUM, STR, PRIORITY)                                \
  .Case(STR, llvm::X86::FEATURE_##ENUM)
#define X86_MICROARCH_LEVEL(ENUM, STR, PRIORITY)                               \
  .Case(STR, llvm::X86::FEATURE_##ENUM)
#include "llvm/TargetParser/X86TargetParser.def"
        ;
    assert(Feature / 32 < FeatureMask.size());
    FeatureMask[Feature / 32] |= 1U << (Feature % 32);
  }
  return FeatureMask;
}

unsigned llvm::X86::getFeaturePriority(ProcessorFeatures Feat) {
#ifndef NDEBUG
  // Check that priorities are set properly in the .def file. We expect that
  // "compat" features are assigned non-duplicate consecutive priorities
  // starting from zero (0, 1, ..., num_features - 1).
#define X86_FEATURE_COMPAT(ENUM, STR, PRIORITY) PRIORITY,
  unsigned Priorities[] = {
#include "llvm/TargetParser/X86TargetParser.def"
  };
  std::array<unsigned, std::size(Priorities)> HelperList;
  std::iota(HelperList.begin(), HelperList.end(), 0);
  assert(std::is_permutation(HelperList.begin(), HelperList.end(),
                             std::begin(Priorities), std::end(Priorities)) &&
         "Priorities don't form consecutive range!");
#endif

  switch (Feat) {
#define X86_FEATURE_COMPAT(ENUM, STR, PRIORITY)                                \
  case X86::FEATURE_##ENUM:                                                    \
    return PRIORITY;
#include "llvm/TargetParser/X86TargetParser.def"
  default:
    llvm_unreachable("No Feature Priority for non-CPUSupports Features");
  }
}

#if INTEL_CUSTOMIZATION
std::unique_ptr<const VectorAbiIsaInfo>
llvm::X86::VectorAbiIsaInfo::getByName(StringRef Name) {
  if (!validateCPUSpecificCPUDispatch(Name))
    return nullptr; // Unknown cpu name
  SmallVector<StringRef, 64> CPUFeatures;
  getFeaturesForCPU(Name, CPUFeatures, true);
  // Default values for ISA etc.
  char IntelIsa = 'x';
  char GnuIsa = 'b';
  size_t IntRegSize = 16;
  size_t FPRegSize = 16;

  if (llvm::find(CPUFeatures, "+avx512f") != CPUFeatures.end()) {
    IntelIsa = 'Z';
    GnuIsa = 'e';
    IntRegSize = 64;
    FPRegSize = 64;
  } else if (llvm::find(CPUFeatures, "+avx2") != CPUFeatures.end() ||
             llvm::find(CPUFeatures, "+avxvnni") != CPUFeatures.end()) {
    IntelIsa = 'Y';
    GnuIsa = 'd';
    IntRegSize = 32;
    FPRegSize = 32;
  } else if (llvm::find(CPUFeatures, "+avx") != CPUFeatures.end()) {
    IntelIsa = 'y';
    GnuIsa = 'c';
    IntRegSize = 16;
    FPRegSize = 32;
  }
  // default lowest, XMM ISA.

  return std::make_unique<const VectorAbiIsaInfo>(
      Name, IntelIsa, GnuIsa, IntRegSize, FPRegSize);
}
#endif // INTEL_CUSTOMIZATION


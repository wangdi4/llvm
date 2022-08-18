//===--- X86.cpp - Implement X86 target feature support -------------------===//
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
// This file implements X86 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/TargetBuiltins.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/X86TargetParser.h"

namespace clang {
namespace targets {

const Builtin::Info BuiltinInfoX86[] = {
#if INTEL_CUSTOMIZATION
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE)                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, FEATURE},
#define TARGET_HEADER_BUILTIN(ID, TYPE, ATTRS, HEADER, LANGS, FEATURE)         \
  {#ID, TYPE, ATTRS, HEADER, LANGS, FEATURE},

#include "clang/Basic/Intel_BuiltinsSVML.def"
#endif // INTEL_CUSTOMIZATION

#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE)                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, FEATURE},
#define TARGET_HEADER_BUILTIN(ID, TYPE, ATTRS, HEADER, LANGS, FEATURE)         \
  {#ID, TYPE, ATTRS, HEADER, LANGS, FEATURE},

#if INTEL_CUSTOMIZATION
#define LANGBUILTIN(ID, TYPE, ATTRS, LANGS)                                    \
  {#ID, TYPE, ATTRS, nullptr, LANGS, nullptr},
#endif // INTEL_CUSTOMIZATION

#include "clang/Basic/BuiltinsX86.def"

#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE)                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, FEATURE},
#define TARGET_HEADER_BUILTIN(ID, TYPE, ATTRS, HEADER, LANGS, FEATURE)         \
  {#ID, TYPE, ATTRS, HEADER, LANGS, FEATURE},

#if INTEL_CUSTOMIZATION
#define LANGBUILTIN(ID, TYPE, ATTRS, LANGS)                                    \
  {#ID, TYPE, ATTRS, nullptr, LANGS, nullptr},
#endif // INTEL_CUSTOMIZATION

#include "clang/Basic/BuiltinsX86_64.def"
};

static const char *const GCCRegNames[] = {
    "ax",    "dx",    "cx",    "bx",    "si",      "di",    "bp",    "sp",
    "st",    "st(1)", "st(2)", "st(3)", "st(4)",   "st(5)", "st(6)", "st(7)",
    "argp",  "flags", "fpcr",  "fpsr",  "dirflag", "frame", "xmm0",  "xmm1",
    "xmm2",  "xmm3",  "xmm4",  "xmm5",  "xmm6",    "xmm7",  "mm0",   "mm1",
    "mm2",   "mm3",   "mm4",   "mm5",   "mm6",     "mm7",   "r8",    "r9",
    "r10",   "r11",   "r12",   "r13",   "r14",     "r15",   "xmm8",  "xmm9",
    "xmm10", "xmm11", "xmm12", "xmm13", "xmm14",   "xmm15", "ymm0",  "ymm1",
    "ymm2",  "ymm3",  "ymm4",  "ymm5",  "ymm6",    "ymm7",  "ymm8",  "ymm9",
    "ymm10", "ymm11", "ymm12", "ymm13", "ymm14",   "ymm15", "xmm16", "xmm17",
    "xmm18", "xmm19", "xmm20", "xmm21", "xmm22",   "xmm23", "xmm24", "xmm25",
    "xmm26", "xmm27", "xmm28", "xmm29", "xmm30",   "xmm31", "ymm16", "ymm17",
    "ymm18", "ymm19", "ymm20", "ymm21", "ymm22",   "ymm23", "ymm24", "ymm25",
    "ymm26", "ymm27", "ymm28", "ymm29", "ymm30",   "ymm31", "zmm0",  "zmm1",
    "zmm2",  "zmm3",  "zmm4",  "zmm5",  "zmm6",    "zmm7",  "zmm8",  "zmm9",
    "zmm10", "zmm11", "zmm12", "zmm13", "zmm14",   "zmm15", "zmm16", "zmm17",
    "zmm18", "zmm19", "zmm20", "zmm21", "zmm22",   "zmm23", "zmm24", "zmm25",
    "zmm26", "zmm27", "zmm28", "zmm29", "zmm30",   "zmm31", "k0",    "k1",
    "k2",    "k3",    "k4",    "k5",    "k6",      "k7",
    "cr0",   "cr2",   "cr3",   "cr4",   "cr8",
    "dr0",   "dr1",   "dr2",   "dr3",   "dr6",     "dr7",
    "bnd0",  "bnd1",  "bnd2",  "bnd3",
    "tmm0",  "tmm1",  "tmm2",  "tmm3",  "tmm4",    "tmm5",  "tmm6",  "tmm7",
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AMX_LNC
    // Just align with ICC for tmm8-15
    "tmm8",  "tmm9",  "tmm10", "tmm11", "tmm12",   "tmm13", "tmm14", "tmm15",
    "tmm16", "tmm17", "tmm18", "tmm19", "tmm20",   "tmm21", "tmm22", "tmm23",
    "tmm24", "tmm25", "tmm26", "tmm27", "tmm28",   "tmm29", "tmm30", "tmm31",
#endif // INTEL_FEATURE_ISA_AMX_LNC
#endif // INTEL_CUSTOMIZATION
};

const TargetInfo::AddlRegName AddlRegNames[] = {
    {{"al", "ah", "eax", "rax"}, 0},
    {{"bl", "bh", "ebx", "rbx"}, 3},
    {{"cl", "ch", "ecx", "rcx"}, 2},
    {{"dl", "dh", "edx", "rdx"}, 1},
    {{"esi", "rsi"}, 4},
    {{"edi", "rdi"}, 5},
    {{"esp", "rsp"}, 7},
    {{"ebp", "rbp"}, 6},
    {{"r8d", "r8w", "r8b"}, 38},
    {{"r9d", "r9w", "r9b"}, 39},
    {{"r10d", "r10w", "r10b"}, 40},
    {{"r11d", "r11w", "r11b"}, 41},
    {{"r12d", "r12w", "r12b"}, 42},
    {{"r13d", "r13w", "r13b"}, 43},
    {{"r14d", "r14w", "r14b"}, 44},
    {{"r15d", "r15w", "r15b"}, 45},
};

} // namespace targets
} // namespace clang

using namespace clang;
using namespace clang::targets;

bool X86TargetInfo::setFPMath(StringRef Name) {
  if (Name == "387") {
    FPMath = FP_387;
    return true;
  }
  if (Name == "sse") {
    FPMath = FP_SSE;
    return true;
  }
  return false;
}

bool X86TargetInfo::initFeatureMap(
    llvm::StringMap<bool> &Features, DiagnosticsEngine &Diags, StringRef CPU,
    const std::vector<std::string> &FeaturesVec) const {
  // FIXME: This *really* should not be here.
  // X86_64 always has SSE2.
  if (getTriple().getArch() == llvm::Triple::x86_64)
    setFeatureEnabled(Features, "sse2", true);

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_XUCC
  // Enable xucc-mode for XuCC target.
  if (getTriple().getArch() == llvm::Triple::x86_64_xucc) {
    setFeatureEnabled(Features, "xucc-mode", true);
    setFeatureEnabled(Features, "sse2", true);
  }
#endif // INTEL_FEATURE_XUCC
#endif // INTEL_CUSTOMIZATION

  using namespace llvm::X86;

  SmallVector<StringRef, 16> CPUFeatures;
  getFeaturesForCPU(CPU, CPUFeatures);
  for (auto &F : CPUFeatures)
    setFeatureEnabled(Features, F, true);

  std::vector<std::string> UpdatedFeaturesVec;
  for (const auto &Feature : FeaturesVec) {
    // Expand general-regs-only to -x86, -mmx and -sse
    if (Feature == "+general-regs-only") {
      UpdatedFeaturesVec.push_back("-x87");
      UpdatedFeaturesVec.push_back("-mmx");
      UpdatedFeaturesVec.push_back("-sse");
      continue;
    }

    UpdatedFeaturesVec.push_back(Feature);
  }

  if (!TargetInfo::initFeatureMap(Features, Diags, CPU, UpdatedFeaturesVec))
    return false;

  // Can't do this earlier because we need to be able to explicitly enable
  // or disable these features and the things that they depend upon.

  // Enable popcnt if sse4.2 is enabled and popcnt is not explicitly disabled.
  auto I = Features.find("sse4.2");
  if (I != Features.end() && I->getValue() &&
      !llvm::is_contained(UpdatedFeaturesVec, "-popcnt"))
    Features["popcnt"] = true;

  // Additionally, if SSE is enabled and mmx is not explicitly disabled,
  // then enable MMX.
  I = Features.find("sse");
  if (I != Features.end() && I->getValue() &&
      !llvm::is_contained(UpdatedFeaturesVec, "-mmx"))
    Features["mmx"] = true;

  // Enable xsave if avx is enabled and xsave is not explicitly disabled.
  I = Features.find("avx");
  if (I != Features.end() && I->getValue() &&
      !llvm::is_contained(UpdatedFeaturesVec, "-xsave"))
    Features["xsave"] = true;

  // Enable CRC32 if SSE4.2 is enabled and CRC32 is not explicitly disabled.
  I = Features.find("sse4.2");
  if (I != Features.end() && I->getValue() &&
      !llvm::is_contained(UpdatedFeaturesVec, "-crc32"))
    Features["crc32"] = true;

  return true;
}

void X86TargetInfo::setFeatureEnabled(llvm::StringMap<bool> &Features,
                                      StringRef Name, bool Enabled) const {
  if (Name == "sse4") {
    // We can get here via the __target__ attribute since that's not controlled
    // via the -msse4/-mno-sse4 command line alias. Handle this the same way
    // here - turn on the sse4.2 if enabled, turn off the sse4.1 level if
    // disabled.
    if (Enabled)
      Name = "sse4.2";
    else
      Name = "sse4.1";
  }

  Features[Name] = Enabled;
  llvm::X86::updateImpliedFeatures(Name, Enabled, Features);
}

/// handleTargetFeatures - Perform initialization based on the user
/// configured set of features.
bool X86TargetInfo::handleTargetFeatures(std::vector<std::string> &Features,
                                         DiagnosticsEngine &Diags) {
  for (const auto &Feature : Features) {
    if (Feature[0] != '+')
      continue;

    if (Feature == "+aes") {
      HasAES = true;
    } else if (Feature == "+vaes") {
      HasVAES = true;
    } else if (Feature == "+pclmul") {
      HasPCLMUL = true;
    } else if (Feature == "+vpclmulqdq") {
      HasVPCLMULQDQ = true;
    } else if (Feature == "+lzcnt") {
      HasLZCNT = true;
    } else if (Feature == "+rdrnd") {
      HasRDRND = true;
    } else if (Feature == "+fsgsbase") {
      HasFSGSBASE = true;
    } else if (Feature == "+bmi") {
      HasBMI = true;
    } else if (Feature == "+bmi2") {
      HasBMI2 = true;
    } else if (Feature == "+popcnt") {
      HasPOPCNT = true;
    } else if (Feature == "+rtm") {
      HasRTM = true;
    } else if (Feature == "+prfchw") {
      HasPRFCHW = true;
    } else if (Feature == "+rdseed") {
      HasRDSEED = true;
    } else if (Feature == "+adx") {
      HasADX = true;
    } else if (Feature == "+tbm") {
      HasTBM = true;
    } else if (Feature == "+lwp") {
      HasLWP = true;
    } else if (Feature == "+fma") {
      HasFMA = true;
    } else if (Feature == "+f16c") {
      HasF16C = true;
    } else if (Feature == "+gfni") {
      HasGFNI = true;
    } else if (Feature == "+avx512cd") {
      HasAVX512CD = true;
    } else if (Feature == "+avx512vpopcntdq") {
      HasAVX512VPOPCNTDQ = true;
    } else if (Feature == "+avx512vnni") {
      HasAVX512VNNI = true;
    } else if (Feature == "+avx512bf16") {
      HasAVX512BF16 = true;
    } else if (Feature == "+avx512er") {
      HasAVX512ER = true;
    } else if (Feature == "+avx512fp16") {
      HasAVX512FP16 = true;
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX512_BF16_NE
    } else if (Feature == "+avx512bf16ne") {
      HasAVX512BF16NE = true;
#endif // INTEL_FEATURE_ISA_AVX512_BF16_NE
#endif // INTEL_CUSTOMIZATION
    } else if (Feature == "+avx512pf") {
      HasAVX512PF = true;
    } else if (Feature == "+avx512dq") {
      HasAVX512DQ = true;
    } else if (Feature == "+avx512bitalg") {
      HasAVX512BITALG = true;
    } else if (Feature == "+avx512bw") {
      HasAVX512BW = true;
    } else if (Feature == "+avx512vl") {
      HasAVX512VL = true;
    } else if (Feature == "+avx512vbmi") {
      HasAVX512VBMI = true;
    } else if (Feature == "+avx512vbmi2") {
      HasAVX512VBMI2 = true;
    } else if (Feature == "+avx512ifma") {
      HasAVX512IFMA = true;
    } else if (Feature == "+avx512vp2intersect") {
      HasAVX512VP2INTERSECT = true;
    } else if (Feature == "+sha") {
      HasSHA = true;
    } else if (Feature == "+shstk") {
      HasSHSTK = true;
    } else if (Feature == "+movbe") {
      HasMOVBE = true;
    } else if (Feature == "+sgx") {
      HasSGX = true;
    } else if (Feature == "+cx8") {
      HasCX8 = true;
    } else if (Feature == "+cx16") {
      HasCX16 = true;
    } else if (Feature == "+fxsr") {
      HasFXSR = true;
    } else if (Feature == "+xsave") {
      HasXSAVE = true;
    } else if (Feature == "+xsaveopt") {
      HasXSAVEOPT = true;
    } else if (Feature == "+xsavec") {
      HasXSAVEC = true;
    } else if (Feature == "+xsaves") {
      HasXSAVES = true;
    } else if (Feature == "+mwaitx") {
      HasMWAITX = true;
    } else if (Feature == "+pku") {
      HasPKU = true;
    } else if (Feature == "+clflushopt") {
      HasCLFLUSHOPT = true;
    } else if (Feature == "+clwb") {
      HasCLWB = true;
    } else if (Feature == "+wbnoinvd") {
      HasWBNOINVD = true;
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_PREFETCHI
    } else if (Feature == "+prefetchi") {
      HasPREFETCHI = true;
#endif // INTEL_FEATURE_ISA_PREFETCHI
#if INTEL_FEATURE_ISA_PREFETCHST2
    } else if (Feature == "+prefetchst2") {
      HasPREFETCHST2 = true;
#endif // INTEL_FEATURE_ISA_PREFETCHST2
#endif // INTEL_CUSTOMIZATION
    } else if (Feature == "+prefetchwt1") {
      HasPREFETCHWT1 = true;
    } else if (Feature == "+clzero") {
      HasCLZERO = true;
    } else if (Feature == "+cldemote") {
      HasCLDEMOTE = true;
    } else if (Feature == "+rdpid") {
      HasRDPID = true;
    } else if (Feature == "+rdpru") {
      HasRDPRU = true;
    } else if (Feature == "+kl") {
      HasKL = true;
    } else if (Feature == "+widekl") {
      HasWIDEKL = true;
    } else if (Feature == "+retpoline-external-thunk") {
      HasRetpolineExternalThunk = true;
    } else if (Feature == "+sahf") {
      HasLAHFSAHF = true;
    } else if (Feature == "+waitpkg") {
      HasWAITPKG = true;
    } else if (Feature == "+movdiri") {
      HasMOVDIRI = true;
    } else if (Feature == "+movdir64b") {
      HasMOVDIR64B = true;
    } else if (Feature == "+pconfig") {
      HasPCONFIG = true;
    } else if (Feature == "+ptwrite") {
      HasPTWRITE = true;
    } else if (Feature == "+invpcid") {
      HasINVPCID = true;
    } else if (Feature == "+enqcmd") {
      HasENQCMD = true;
#if INTEL_CUSTOMIZATION
    } else if (Feature == "+hreset") {
      HasHRESET = true;
    } else if (Feature == "+amx-bf16") {
      HasAMXBF16 = true;
    } else if (Feature == "+amx-int8") {
      HasAMXINT8 = true;
    } else if (Feature == "+amx-tile") {
      HasAMXTILE = true;
#if INTEL_FEATURE_ISA_AMX_BF8
    } else if (Feature == "+amx-bf8") {
      HasAMXBF8 = true;
#endif // INTEL_FEATURE_ISA_AMX_BF8
#if INTEL_FEATURE_ISA_AMX_MEMADVISE
    } else if (Feature == "+amx-memadvise") {
      HasAMXMEMADVISE = true;
#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE
#if INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
    } else if (Feature == "+amx-memadvise-evex") {
      HasAMXMEMADVISEEVEX = true;
#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
#if INTEL_FEATURE_ISA_AMX_FUTURE
    } else if (Feature == "+amx-reduce") {
      HasAMXREDUCE = true;
    } else if (Feature == "+amx-memory") {
      HasAMXMEMORY = true;
    } else if (Feature == "+amx-format") {
      HasAMXFORMAT = true;
    } else if (Feature == "+amx-element") {
      HasAMXELEMENT = true;
#endif // INTEL_FEATURE_ISA_AMX_FUTURE
#if INTEL_FEATURE_ISA_AMX_LNC
    } else if (Feature == "+amx-transpose") {
      HasAMXTRANSPOSE = true;
    } else if (Feature == "+amx-avx512") {
      HasAMXAVX512 = true;
#endif // INTEL_FEATURE_ISA_AMX_LNC
#if INTEL_FEATURE_ISA_AMX_FP16
    } else if (Feature == "+amx-fp16") {
      HasAMXFP16 = true;
#endif // INTEL_FEATURE_ISA_AMX_FP16
#if INTEL_FEATURE_ISA_AMX_MEMORY2
    } else if (Feature == "+amx-memory2") {
      HasAMXMEMORY2 = true;
#endif // INTEL_FEATURE_ISA_AMX_MEMORY2
#if INTEL_FEATURE_ISA_AMX_BF16_EVEX
    } else if (Feature == "+amx-bf16-evex") {
      HasAMXBF16EVEX = true;
#endif // INTEL_FEATURE_ISA_AMX_BF16_EVEX
#if INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
    } else if (Feature == "+amx-element-evex") {
      HasAMXELEMENTEVEX = true;
#endif // INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
#if INTEL_FEATURE_ISA_AMX_INT8_EVEX
    } else if (Feature == "+amx-int8-evex") {
      HasAMXINT8EVEX = true;
#endif // INTEL_FEATURE_ISA_AMX_INT8_EVEX
#if INTEL_FEATURE_ISA_AMX_TILE_EVEX
    } else if (Feature == "+amx-tile-evex") {
      HasAMXTILEEVEX = true;
#endif // INTEL_FEATURE_ISA_AMX_TILE_EVEX
#if INTEL_FEATURE_ISA_AMX_TRANSPOSE2
    } else if (Feature == "+amx-transpose2") {
      HasAMXTRANSPOSE2 = true;
#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE2
#if INTEL_FEATURE_ISA_AMX_CONVERT
    } else if (Feature == "+amx-convert") {
      HasAMXCONVERT = true;
#endif // INTEL_FEATURE_ISA_AMX_CONVERT
#if INTEL_FEATURE_ISA_AMX_TILE2
    } else if (Feature == "+amx-tile2") {
      HasAMXTILE2 = true;
#endif // INTEL_FEATURE_ISA_AMX_TILE2
#if INTEL_FEATURE_ISA_AMX_COMPLEX
    } else if (Feature == "+amx-complex") {
      HasAMXCOMPLEX = true;
#endif // INTEL_FEATURE_ISA_AMX_COMPLEX
#if INTEL_FEATURE_ISA_AMX_TF32
    } else if (Feature == "+amx-tf32") {
      HasAMXTF32 = true;
#endif // INTEL_FEATURE_ISA_AMX_TF32
#if INTEL_FEATURE_ISA_AVX512_VNNI_INT8
    } else if (Feature == "+avx512vnniint8") {
      HasAVX512VNNIINT8 = true;
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT8
#if INTEL_FEATURE_ISA_AVX512_VNNI_FP16
    } else if (Feature == "+avx512vnnifp16") {
      HasAVX512VNNIFP16 = true;
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_FP16
#if INTEL_FEATURE_ISA_AVX512_CONVERT
    } else if (Feature == "+avx512convert") {
      HasAVX512CONVERT = true;
#endif // INTEL_FEATURE_ISA_AVX512_CONVERT
#if INTEL_FEATURE_ISA_AVX_IFMA
    } else if (Feature == "+avxifma") {
      HasAVXIFMA = true;
#endif // INTEL_FEATURE_ISA_AVX_IFMA
#if INTEL_FEATURE_ISA_AVX_VNNI_INT8
    } else if (Feature == "+avxvnniint8") {
      HasAVXVNNIINT8 = true;
#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT8
#if INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
    } else if (Feature == "+avxdotprodphps") {
      HasAVXDOTPRODPHPS = true;
#endif // INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
#if INTEL_FEATURE_ISA_AVX_CONVERT
    } else if (Feature == "+avxconvert") {
      HasAVXCONVERT = true;
#endif // INTEL_FEATURE_ISA_AVX_CONVERT
#if INTEL_FEATURE_ISA_AVX_BF16
    } else if (Feature == "+avxbf16") {
      HasAVXBF16 = true;
#endif // INTEL_FEATURE_ISA_AVX_BF16
#if INTEL_FEATURE_ISA_AVX_COMPRESS
    } else if (Feature == "+avxcompress") {
      HasAVXCOMPRESS = true;
#endif // INTEL_FEATURE_ISA_AVX_COMPRESS
#if INTEL_FEATURE_ISA_AVX_MEMADVISE
    } else if (Feature == "+avxmemadvise") {
      HasAVXMEMADVISE = true;
    } else if (Feature == "+avx512memadvise") {
      HasAVX512MEMADVISE = true;
#endif // INTEL_FEATURE_ISA_AVX_MEMADVISE
#if INTEL_FEATURE_ISA_AVX512_MEDIAX
    } else if (Feature == "+avx512mediax") {
      HasAVX512MEDIAX = true;
#endif // INTEL_FEATURE_ISA_AVX512_MEDIAX
#if INTEL_FEATURE_ISA_AVX_MOVGET
    } else if (Feature == "+avxmovget") {
      HasAVXMOVGET = true;
#endif // INTEL_FEATURE_ISA_AVX_MOVGET
#if INTEL_FEATURE_ISA_AVX512_MOVGET
    } else if (Feature == "+avx512movget") {
      HasAVX512MOVGET = true;
#endif // INTEL_FEATURE_ISA_AVX512_MOVGET
#if INTEL_FEATURE_ISA_GPR_MOVGET
    } else if (Feature == "+gprmovget") {
      HasGPRMOVGET = true;
#endif // INTEL_FEATURE_ISA_GPR_MOVGET
#if INTEL_FEATURE_ISA_MOVGET64B
    } else if (Feature == "+movget64b") {
      HasMOVGET64B = true;
#endif // INTEL_FEATURE_ISA_MOVGET64B
#if INTEL_FEATURE_ISA_RAO_INT
    } else if (Feature == "+raoint") {
      HasRAOINT = true;
#endif // INTEL_FEATURE_ISA_RAO_INT
#if INTEL_FEATURE_ISA_AVX_RAO_INT
    } else if (Feature == "+avxraoint") {
      HasAVXRAOINT = true;
#endif // INTEL_FEATURE_ISA_AVX_RAO_INT
#if INTEL_FEATURE_ISA_AVX_RAO_FP
// AUTO GENERATED BY TOOL
    } else if (Feature == "+avxraofp") {
      HasAVXRAOFP = true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_RAO_FP
#if INTEL_FEATURE_ISA_AVX512_RAO_INT
// AUTO GENERATED BY TOOL
    } else if (Feature == "+avx512raoint") {
      HasAVX512RAOINT= true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_RAO_INT
#if INTEL_FEATURE_ISA_AVX512_RAO_FP
// AUTO GENERATED BY TOOL
    } else if (Feature == "+avx512raofp") {
      HasAVX512RAOFP= true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_RAO_FP
#if INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
// AUTO GENERATED BY TOOL
    } else if (Feature == "+amx-avx512-cvtrow") {
      HasAMXAVX512CVTROW= true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
#if INTEL_FEATURE_ISA_AVX_NE_CONVERT
// AUTO GENERATED BY TOOL
    } else if (Feature == "+avxneconvert") {
      HasAVXNECONVERT= true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_NE_CONVERT
#if INTEL_FEATURE_ISA_AVX512_NE_CONVERT
// AUTO GENERATED BY TOOL
    } else if (Feature == "+avx512neconvert") {
      HasAVX512NECONVERT = true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_NE_CONVERT
#if INTEL_FEATURE_ISA_SHA512
// AUTO GENERATED BY TOOL
    } else if (Feature == "+sha512") {
      HasSHA512 = true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SHA512
#if INTEL_FEATURE_ISA_SM3
// AUTO GENERATED BY TOOL
    } else if (Feature == "+sm3") {
      HasSM3 = true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SM3
#if INTEL_FEATURE_ISA_SM4
// AUTO GENERATED BY TOOL
    } else if (Feature == "+sm4") {
      HasSM4 = true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SM4
#if INTEL_FEATURE_ISA_DSPV1
// AUTO GENERATED BY TOOL
    } else if (Feature == "+dspv1") {
      HasDSPV1 = true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_DSPV1
#if INTEL_FEATURE_ISA_AVX_VNNI_INT16
// AUTO GENERATED BY TOOL
    } else if (Feature == "+avxvnniint16") {
      HasAVXVNNIINT16 = true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT16
#if INTEL_FEATURE_ISA_AVX512_VNNI_INT16
// AUTO GENERATED BY TOOL
    } else if (Feature == "+avx512vnniint16") {
      HasAVX512VNNIINT16 = true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT16
#if INTEL_FEATURE_ISA_AMX_SPARSE
// AUTO GENERATED BY TOOL
    } else if (Feature == "+amx-sparse") {
      HasAMXSPARSE = true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_SPARSE
#if INTEL_FEATURE_ISA_AMX_V3
// AUTO GENERATED BY TOOL
    } else if (Feature == "+amx-v3") {
      HasAMXV3 = true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_V3
#if INTEL_FEATURE_ISA_VPINSR_VPEXTR
    } else if (Feature == "+vpinsr-vpextr") {
      HasVPINSRVPEXTR = true;
#endif // INTEL_FEATURE_ISA_VPINSR_VPEXTR
#if INTEL_FEATURE_ISA_CMPCCXADD
// AUTO GENERATED BY TOOL
    } else if (Feature == "+cmpccxadd") {
      HasCMPCCXADD = true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_CMPCCXADD
#if INTEL_FEATURE_ISA_AVX512_SAT_CVT
// AUTO GENERATED BY TOOL
    } else if (Feature == "+avx512satcvt") {
      HasAVX512SATCVT = true;
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_SAT_CVT
#if INTEL_FEATURE_ISA_AVX256P
    } else if (Feature == "+avx256p") {
      HasAVX256P = true;
#endif // INTEL_FEATURE_ISA_AVX256P
#endif // INTEL_CUSTOMIZATION
    } else if (Feature == "+avxvnni") {
      HasAVXVNNI = true;
    } else if (Feature == "+serialize") {
      HasSERIALIZE = true;
    } else if (Feature == "+tsxldtrk") {
      HasTSXLDTRK = true;
    } else if (Feature == "+uintr") {
      HasUINTR = true;
    } else if (Feature == "+crc32") {
      HasCRC32 = true;
    } else if (Feature == "+x87") {
      HasX87 = true;
    }
    X86SSEEnum Level = llvm::StringSwitch<X86SSEEnum>(Feature)
                           .Case("+avx512f", AVX512F)
                           .Case("+avx2", AVX2)
                           .Case("+avx", AVX)
                           .Case("+sse4.2", SSE42)
                           .Case("+sse4.1", SSE41)
                           .Case("+ssse3", SSSE3)
                           .Case("+sse3", SSE3)
                           .Case("+sse2", SSE2)
                           .Case("+sse", SSE1)
                           .Default(NoSSE);
    SSELevel = std::max(SSELevel, Level);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_BF16_BASE
    HasBFloat16 = SSELevel >= SSE2;
#endif // INTEL_FEATURE_ISA_BF16_BASE
#endif // INTEL_CUSTOMIZATION

    HasFloat16 = SSELevel >= SSE2;

    MMX3DNowEnum ThreeDNowLevel = llvm::StringSwitch<MMX3DNowEnum>(Feature)
                                      .Case("+3dnowa", AMD3DNowAthlon)
                                      .Case("+3dnow", AMD3DNow)
                                      .Case("+mmx", MMX)
                                      .Default(NoMMX3DNow);
    MMX3DNowLevel = std::max(MMX3DNowLevel, ThreeDNowLevel);

    XOPEnum XLevel = llvm::StringSwitch<XOPEnum>(Feature)
                         .Case("+xop", XOP)
                         .Case("+fma4", FMA4)
                         .Case("+sse4a", SSE4A)
                         .Default(NoXOP);
    XOPLevel = std::max(XOPLevel, XLevel);
  }

  // LLVM doesn't have a separate switch for fpmath, so only accept it if it
  // matches the selected sse level.
  if ((FPMath == FP_SSE && SSELevel < SSE1) ||
      (FPMath == FP_387 && SSELevel >= SSE1)) {
    Diags.Report(diag::err_target_unsupported_fpmath)
        << (FPMath == FP_SSE ? "sse" : "387");
    return false;
  }

  SimdDefaultAlign =
      hasFeature("avx512f") ? 512 : hasFeature("avx") ? 256 : 128;

  // FIXME: We should allow long double type on 32-bits to match with GCC.
  // This requires backend to be able to lower f80 without x87 first.
  if (!HasX87 && LongDoubleFormat == &llvm::APFloat::x87DoubleExtended())
    HasLongDouble = false;

  return true;
}

/// X86TargetInfo::getTargetDefines - Return the set of the X86-specific macro
/// definitions for this particular subtarget.
void X86TargetInfo::getTargetDefines(const LangOptions &Opts,
                                     MacroBuilder &Builder) const {
  // Inline assembly supports X86 flag outputs.
  Builder.defineMacro("__GCC_ASM_FLAG_OUTPUTS__");

  std::string CodeModel = getTargetOpts().CodeModel;
  if (CodeModel == "default")
    CodeModel = "small";
  Builder.defineMacro("__code_model_" + CodeModel + "__");

  // Target identification.
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_XUCC
  if (getTriple().getArch() == llvm::Triple::x86_64_xucc) {
    Builder.defineMacro("__amd64__");
    Builder.defineMacro("__amd64");
    Builder.defineMacro("__x86_64");
    Builder.defineMacro("__x86_64__");
    Builder.defineMacro("__XUCC__");
  } else
#endif // INTEL_FEATURE_XUCC
#endif // INTEL_CUSTOMIZATION
  if (getTriple().getArch() == llvm::Triple::x86_64) {
    Builder.defineMacro("__amd64__");
    Builder.defineMacro("__amd64");
    Builder.defineMacro("__x86_64");
    Builder.defineMacro("__x86_64__");
    if (getTriple().getArchName() == "x86_64h") {
      Builder.defineMacro("__x86_64h");
      Builder.defineMacro("__x86_64h__");
    }
  } else {
    DefineStd(Builder, "i386", Opts);
  }

  Builder.defineMacro("__SEG_GS");
  Builder.defineMacro("__SEG_FS");
  Builder.defineMacro("__seg_gs", "__attribute__((address_space(256)))");
  Builder.defineMacro("__seg_fs", "__attribute__((address_space(257)))");

#if INTEL_CUSTOMIZATION
  // IntrinsicPromotion implementation.
  if (Opts.IntrinsicAutoPromote)
    // TODO: Does this have value?!
    Builder.defineMacro("__M_INTRINSIC_PROMOTE__");
#endif // INTEL_CUSTOMIZATION

  // Subtarget options.
  // FIXME: We are hard-coding the tune parameters based on the CPU, but they
  // truly should be based on -mtune options.
  using namespace llvm::X86;
  switch (CPU) {
  case CK_None:
    break;
  case CK_i386:
    // The rest are coming from the i386 define above.
    Builder.defineMacro("__tune_i386__");
    break;
  case CK_i486:
  case CK_WinChipC6:
  case CK_WinChip2:
  case CK_C3:
    defineCPUMacros(Builder, "i486");
    break;
  case CK_PentiumMMX:
    Builder.defineMacro("__pentium_mmx__");
    Builder.defineMacro("__tune_pentium_mmx__");
    LLVM_FALLTHROUGH;
  case CK_i586:
  case CK_Pentium:
    defineCPUMacros(Builder, "i586");
    defineCPUMacros(Builder, "pentium");
    break;
  case CK_Pentium3:
  case CK_PentiumM:
    Builder.defineMacro("__tune_pentium3__");
    LLVM_FALLTHROUGH;
  case CK_Pentium2:
  case CK_C3_2:
    Builder.defineMacro("__tune_pentium2__");
    LLVM_FALLTHROUGH;
  case CK_PentiumPro:
  case CK_i686:
    defineCPUMacros(Builder, "i686");
    defineCPUMacros(Builder, "pentiumpro");
    break;
  case CK_Pentium4:
    defineCPUMacros(Builder, "pentium4");
    break;
  case CK_Yonah:
  case CK_Prescott:
  case CK_Nocona:
    defineCPUMacros(Builder, "nocona");
    break;
  case CK_Core2:
  case CK_Penryn:
    defineCPUMacros(Builder, "core2");
    break;
  case CK_Bonnell:
    defineCPUMacros(Builder, "atom");
    break;
  case CK_Silvermont:
    defineCPUMacros(Builder, "slm");
    break;
  case CK_Goldmont:
    defineCPUMacros(Builder, "goldmont");
    break;
  case CK_GoldmontPlus:
    defineCPUMacros(Builder, "goldmont_plus");
    break;
  case CK_Tremont:
    defineCPUMacros(Builder, "tremont");
    break;
#if INTEL_CUSTOMIZATION
  case CK_Gracemont:
    defineCPUMacros(Builder, "gracemont");
    break;
#endif // INTEL_CUSTOMIZATION
  case CK_Nehalem:
  case CK_Westmere:
  case CK_SandyBridge:
  case CK_IvyBridge:
  case CK_Haswell:
  case CK_Broadwell:
  case CK_SkylakeClient:
  case CK_SkylakeServer:
  case CK_Cascadelake:
  case CK_Cooperlake:
  case CK_Cannonlake:
  case CK_IcelakeClient:
  case CK_Rocketlake:
  case CK_IcelakeServer:
  case CK_Tigerlake:
  case CK_SapphireRapids:
  case CK_Alderlake:
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CPU_RPL
  case CK_Raptorlake:
#endif // INTEL_FEATURE_CPU_RPL
#if INTEL_FEATURE_CPU_GNR
  case CK_Graniterapids:
#endif // INTEL_FEATURE_CPU_GNR
#if INTEL_FEATURE_CPU_DMR
  case CK_Diamondrapids:
#endif // INTEL_FEATURE_CPU_DMR
#endif // INTEL_CUSTOMIZATION
    // FIXME: Historically, we defined this legacy name, it would be nice to
    // remove it at some point. We've never exposed fine-grained names for
    // recent primary x86 CPUs, and we should keep it that way.
    defineCPUMacros(Builder, "corei7");
    break;
#if INTEL_CUSTOMIZATION
  case CK_CommonAVX512:
#if INTEL_FEATURE_ISA_AVX256
  case CK_CommonAVX256:
#endif // INTEL_FEATURE_ISA_AVX256
    break;
#endif // INTEL_CUSTOMIZATION
  case CK_KNL:
    defineCPUMacros(Builder, "knl");
    break;
  case CK_KNM:
    break;
  case CK_Lakemont:
    defineCPUMacros(Builder, "i586", /*Tuning*/false);
    defineCPUMacros(Builder, "pentium", /*Tuning*/false);
    Builder.defineMacro("__tune_lakemont__");
    break;
  case CK_K6_2:
    Builder.defineMacro("__k6_2__");
    Builder.defineMacro("__tune_k6_2__");
    LLVM_FALLTHROUGH;
  case CK_K6_3:
    if (CPU != CK_K6_2) { // In case of fallthrough
      // FIXME: GCC may be enabling these in cases where some other k6
      // architecture is specified but -m3dnow is explicitly provided. The
      // exact semantics need to be determined and emulated here.
      Builder.defineMacro("__k6_3__");
      Builder.defineMacro("__tune_k6_3__");
    }
    LLVM_FALLTHROUGH;
  case CK_K6:
    defineCPUMacros(Builder, "k6");
    break;
  case CK_Athlon:
  case CK_AthlonXP:
    defineCPUMacros(Builder, "athlon");
    if (SSELevel != NoSSE) {
      Builder.defineMacro("__athlon_sse__");
      Builder.defineMacro("__tune_athlon_sse__");
    }
    break;
  case CK_K8:
  case CK_K8SSE3:
  case CK_x86_64:
    defineCPUMacros(Builder, "k8");
    break;
  case CK_x86_64_v2:
  case CK_x86_64_v3:
  case CK_x86_64_v4:
    break;
  case CK_AMDFAM10:
    defineCPUMacros(Builder, "amdfam10");
    break;
  case CK_BTVER1:
    defineCPUMacros(Builder, "btver1");
    break;
  case CK_BTVER2:
    defineCPUMacros(Builder, "btver2");
    break;
  case CK_BDVER1:
    defineCPUMacros(Builder, "bdver1");
    break;
  case CK_BDVER2:
    defineCPUMacros(Builder, "bdver2");
    break;
  case CK_BDVER3:
    defineCPUMacros(Builder, "bdver3");
    break;
  case CK_BDVER4:
    defineCPUMacros(Builder, "bdver4");
    break;
  case CK_ZNVER1:
    defineCPUMacros(Builder, "znver1");
    break;
  case CK_ZNVER2:
    defineCPUMacros(Builder, "znver2");
    break;
  case CK_ZNVER3:
    defineCPUMacros(Builder, "znver3");
    break;
  case CK_Geode:
    defineCPUMacros(Builder, "geode");
    break;
  }

  // Target properties.
  Builder.defineMacro("__REGISTER_PREFIX__", "");

  // Define __NO_MATH_INLINES on linux/x86 so that we don't get inline
  // functions in glibc header files that use FP Stack inline asm which the
  // backend can't deal with (PR879).
  Builder.defineMacro("__NO_MATH_INLINES");

  if (HasAES)
    Builder.defineMacro("__AES__");

  if (HasVAES)
    Builder.defineMacro("__VAES__");

  if (HasPCLMUL)
    Builder.defineMacro("__PCLMUL__");

  if (HasVPCLMULQDQ)
    Builder.defineMacro("__VPCLMULQDQ__");

  // Note, in 32-bit mode, GCC does not define the macro if -mno-sahf. In LLVM,
  // the feature flag only applies to 64-bit mode.
  if (HasLAHFSAHF || getTriple().getArch() == llvm::Triple::x86)
    Builder.defineMacro("__LAHF_SAHF__");

  if (HasLZCNT)
    Builder.defineMacro("__LZCNT__");

  if (HasRDRND)
    Builder.defineMacro("__RDRND__");

  if (HasFSGSBASE)
    Builder.defineMacro("__FSGSBASE__");

  if (HasBMI)
    Builder.defineMacro("__BMI__");

  if (HasBMI2)
    Builder.defineMacro("__BMI2__");

  if (HasPOPCNT)
    Builder.defineMacro("__POPCNT__");

  if (HasRTM)
    Builder.defineMacro("__RTM__");

  if (HasPRFCHW)
    Builder.defineMacro("__PRFCHW__");

  if (HasRDSEED)
    Builder.defineMacro("__RDSEED__");

  if (HasADX)
    Builder.defineMacro("__ADX__");

  if (HasTBM)
    Builder.defineMacro("__TBM__");

  if (HasLWP)
    Builder.defineMacro("__LWP__");

  if (HasMWAITX)
    Builder.defineMacro("__MWAITX__");

  if (HasMOVBE)
    Builder.defineMacro("__MOVBE__");

  switch (XOPLevel) {
  case XOP:
    Builder.defineMacro("__XOP__");
    LLVM_FALLTHROUGH;
  case FMA4:
    Builder.defineMacro("__FMA4__");
    LLVM_FALLTHROUGH;
  case SSE4A:
    Builder.defineMacro("__SSE4A__");
    LLVM_FALLTHROUGH;
  case NoXOP:
    break;
  }

  if (HasFMA)
    Builder.defineMacro("__FMA__");

  if (HasF16C)
    Builder.defineMacro("__F16C__");

  if (HasGFNI)
    Builder.defineMacro("__GFNI__");

  if (HasAVX512CD)
    Builder.defineMacro("__AVX512CD__");
  if (HasAVX512VPOPCNTDQ)
    Builder.defineMacro("__AVX512VPOPCNTDQ__");
  if (HasAVX512VNNI)
    Builder.defineMacro("__AVX512VNNI__");
  if (HasAVX512BF16)
    Builder.defineMacro("__AVX512BF16__");
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX512_BF16_NE
  if (HasAVX512BF16NE)
    Builder.defineMacro("__AVX512BF16NE__");
#endif // INTEL_FEATURE_ISA_AVX512_BF16_NE
#endif // INTEL_CUSTOMIZATION
  if (HasAVX512ER)
    Builder.defineMacro("__AVX512ER__");
  if (HasAVX512FP16)
    Builder.defineMacro("__AVX512FP16__");
  if (HasAVX512PF)
    Builder.defineMacro("__AVX512PF__");
  if (HasAVX512DQ)
    Builder.defineMacro("__AVX512DQ__");
  if (HasAVX512BITALG)
    Builder.defineMacro("__AVX512BITALG__");
  if (HasAVX512BW)
    Builder.defineMacro("__AVX512BW__");
  if (HasAVX512VL)
    Builder.defineMacro("__AVX512VL__");
  if (HasAVX512VBMI)
    Builder.defineMacro("__AVX512VBMI__");
  if (HasAVX512VBMI2)
    Builder.defineMacro("__AVX512VBMI2__");
  if (HasAVX512IFMA)
    Builder.defineMacro("__AVX512IFMA__");
  if (HasAVX512VP2INTERSECT)
    Builder.defineMacro("__AVX512VP2INTERSECT__");
  if (HasSHA)
    Builder.defineMacro("__SHA__");

  if (HasFXSR)
    Builder.defineMacro("__FXSR__");
  if (HasXSAVE)
    Builder.defineMacro("__XSAVE__");
  if (HasXSAVEOPT)
    Builder.defineMacro("__XSAVEOPT__");
  if (HasXSAVEC)
    Builder.defineMacro("__XSAVEC__");
  if (HasXSAVES)
    Builder.defineMacro("__XSAVES__");
  if (HasPKU)
    Builder.defineMacro("__PKU__");
  if (HasCLFLUSHOPT)
    Builder.defineMacro("__CLFLUSHOPT__");
  if (HasCLWB)
    Builder.defineMacro("__CLWB__");
  if (HasWBNOINVD)
    Builder.defineMacro("__WBNOINVD__");
  if (HasSHSTK)
    Builder.defineMacro("__SHSTK__");
  if (HasSGX)
    Builder.defineMacro("__SGX__");
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_PREFETCHI
  if (HasPREFETCHI)
    Builder.defineMacro("__PREFETCHI__");
#endif // INTEL_FEATURE_ISA_PREFETCHI
#if INTEL_FEATURE_ISA_PREFETCHST2
  if (HasPREFETCHST2)
    Builder.defineMacro("__PREFETCHST2__");
#endif // INTEL_FEATURE_ISA_PREFETCHST2
#endif // INTEL_CUSTOMIZATION
  if (HasPREFETCHWT1)
    Builder.defineMacro("__PREFETCHWT1__");
  if (HasCLZERO)
    Builder.defineMacro("__CLZERO__");
  if (HasKL)
    Builder.defineMacro("__KL__");
  if (HasWIDEKL)
    Builder.defineMacro("__WIDEKL__");
  if (HasRDPID)
    Builder.defineMacro("__RDPID__");
  if (HasRDPRU)
    Builder.defineMacro("__RDPRU__");
  if (HasCLDEMOTE)
    Builder.defineMacro("__CLDEMOTE__");
  if (HasWAITPKG)
    Builder.defineMacro("__WAITPKG__");
  if (HasMOVDIRI)
    Builder.defineMacro("__MOVDIRI__");
  if (HasMOVDIR64B)
    Builder.defineMacro("__MOVDIR64B__");
  if (HasPCONFIG)
    Builder.defineMacro("__PCONFIG__");
  if (HasPTWRITE)
    Builder.defineMacro("__PTWRITE__");
  if (HasINVPCID)
    Builder.defineMacro("__INVPCID__");
  if (HasENQCMD)
    Builder.defineMacro("__ENQCMD__");
#if INTEL_CUSTOMIZATION
  if (HasHRESET)
    Builder.defineMacro("__HRESET__");
  if (HasAMXTILE)
    Builder.defineMacro("__AMXTILE__");
  if (HasAMXINT8)
    Builder.defineMacro("__AMXINT8__");
  if (HasAMXBF16)
    Builder.defineMacro("__AMXBF16__");
  Builder.defineMacro("__AMX_SUPPORTED__");
#if INTEL_FEATURE_ISA_AMX_BF8
  if (HasAMXBF8)
    Builder.defineMacro("__AMXBF8__");
  Builder.defineMacro("__AMXBF8_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_BF8
#if INTEL_FEATURE_ISA_AMX_MEMADVISE
  if (HasAMXMEMADVISE)
    Builder.defineMacro("__AMXMEMADVISE__");
  Builder.defineMacro("__AMXMEMADVISE_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE
#if INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
  if (HasAMXMEMADVISEEVEX)
    Builder.defineMacro("__AMXMEMADVISEEVEX__");
  Builder.defineMacro("__AMXMEMADVISEEVEX_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
#if INTEL_FEATURE_ISA_AMX_MEMORY2
  if (HasAMXMEMORY2)
    Builder.defineMacro("__AMXMEMORY2__");
  Builder.defineMacro("__AMXMEMORY2_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_MEMORY2
#if INTEL_FEATURE_ISA_AMX_BF16_EVEX
  if (HasAMXBF16EVEX)
    Builder.defineMacro("__AMXBF16EVEX__");
  Builder.defineMacro("__AMXBF16EVEX_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_BF16_EVEX
#if INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
  if (HasAMXELEMENTEVEX)
    Builder.defineMacro("__AMXELEMENTEVEX__");
  Builder.defineMacro("__AMXELEMENTEVEX_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
#if INTEL_FEATURE_ISA_AMX_INT8_EVEX
  if (HasAMXINT8EVEX)
    Builder.defineMacro("__AMXINT8EVEX__");
  Builder.defineMacro("__AMXINT8EVEX_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_INT8_EVEX
#if INTEL_FEATURE_ISA_AMX_TILE_EVEX
  if (HasAMXTILEEVEX)
    Builder.defineMacro("__AMXTILEEVEX__");
  Builder.defineMacro("__AMXTILEEVEX_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_TILE_EVEX
#if INTEL_FEATURE_ISA_AMX_TRANSPOSE2
  if (HasAMXTRANSPOSE2)
    Builder.defineMacro("__AMXTRANSPOSE2__");
  Builder.defineMacro("__AMXTRANSPOSE2_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE2
#if INTEL_FEATURE_ISA_AMX_FUTURE
  if (HasAMXREDUCE)
    Builder.defineMacro("__AMXREDUCE__");
  if (HasAMXMEMORY)
    Builder.defineMacro("__AMXMEMORY__");
  if (HasAMXFORMAT)
    Builder.defineMacro("__AMXFORMAT__");
  if (HasAMXELEMENT)
    Builder.defineMacro("__AMXELEMENT__");
  Builder.defineMacro("__AMXFUTURE_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_FUTURE
#if INTEL_FEATURE_ISA_AMX_LNC
  if (HasAMXTRANSPOSE)
    Builder.defineMacro("__AMXTRANSPOSE__");
  if (HasAMXAVX512)
    Builder.defineMacro("__AMXAVX512__");
  Builder.defineMacro("__AMXLNC_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_LNC
#if INTEL_FEATURE_ISA_AMX_FP16
  if (HasAMXFP16)
    Builder.defineMacro("__AMXFP16__");
  Builder.defineMacro("__AMXFP16_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_FP16
#if INTEL_FEATURE_ISA_AMX_CONVERT
  if (HasAMXCONVERT)
    Builder.defineMacro("__AMXCONVERT__");
  Builder.defineMacro("__AMXCONVERT_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_CONVERT
#if INTEL_FEATURE_ISA_AMX_TILE2
  if (HasAMXTILE2)
    Builder.defineMacro("__AMXTILE2__");
  Builder.defineMacro("__AMXTILE2_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_TILE2
#if INTEL_FEATURE_ISA_AMX_COMPLEX
  if (HasAMXCOMPLEX)
    Builder.defineMacro("__AMXCOMPLEX__");
  Builder.defineMacro("__AMXCOMPLEX_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_COMPLEX
#if INTEL_FEATURE_ISA_AMX_TF32
  if (HasAMXTF32)
    Builder.defineMacro("__AMXTF32__");
  Builder.defineMacro("__AMXTF32_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AMX_TF32
#if INTEL_FEATURE_ISA_AVX512_VNNI_INT8
  if (HasAVX512VNNIINT8)
    Builder.defineMacro("__AVX512VNNIINT8__");
  Builder.defineMacro("__AVX512VNNIINT8_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT8
#if INTEL_FEATURE_ISA_AVX512_VNNI_FP16
  if (HasAVX512VNNIFP16)
    Builder.defineMacro("__AVX512VNNIFP16__");
  Builder.defineMacro("__AVX512VNNIFP16_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_FP16
#if INTEL_FEATURE_ISA_AVX512_CONVERT
  if (HasAVX512CONVERT)
    Builder.defineMacro("__AVX512CONVERT__");
  Builder.defineMacro("__AVX512CONVERT_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX512_CONVERT
#if INTEL_FEATURE_ISA_AVX_IFMA
  if (HasAVXIFMA)
    Builder.defineMacro("__AVXIFMA__");
  Builder.defineMacro("__AVXIFMA_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX_IFMA
#if INTEL_FEATURE_ISA_AVX_VNNI_INT8
  if (HasAVXVNNIINT8)
    Builder.defineMacro("__AVXVNNIINT8__");
  Builder.defineMacro("__AVXVNNIINT8_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT8
#if INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
  if (HasAVXDOTPRODPHPS)
    Builder.defineMacro("__AVXDOTPRODPHPS__");
  Builder.defineMacro("__AVXDOTPRODPHPS_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
#if INTEL_FEATURE_ISA_AVX_CONVERT
  if (HasAVXCONVERT)
    Builder.defineMacro("__AVXCONVERT__");
  Builder.defineMacro("__AVXCONVERT_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX_CONVERT
#if INTEL_FEATURE_ISA_AVX_BF16
  if (HasAVXBF16)
    Builder.defineMacro("__AVXBF16__");
  Builder.defineMacro("__AVXBF16_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX_BF16
#if INTEL_FEATURE_ISA_AVX_COMPRESS
  if (HasAVXCOMPRESS)
    Builder.defineMacro("__AVXCOMPRESS__");
  Builder.defineMacro("__AVXCOMPRESS_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX_COMPRESS
#if INTEL_FEATURE_ISA_AVX_MEMADVISE
  if (HasAVXMEMADVISE)
    Builder.defineMacro("__AVXMEMADVISE__");
  Builder.defineMacro("__AVXMEMADVISE_SUPPORTED__");
  if (HasAVX512MEMADVISE)
    Builder.defineMacro("__AVX512MEMADVISE__");
  Builder.defineMacro("__AVX512MEMADVISE_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX_MEMADVISE
#if INTEL_FEATURE_ISA_AVX512_MEDIAX
  if (HasAVX512MEDIAX)
    Builder.defineMacro("__AVX512MEDIAX__");
  Builder.defineMacro("__AVX512MEDIAX_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX512_MEDIAX
#if INTEL_FEATURE_ISA_AVX_MOVGET
  if (HasAVXMOVGET)
    Builder.defineMacro("__AVXMOVGET__");
  Builder.defineMacro("__AVXMOVGET_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX_MOVGET
#if INTEL_FEATURE_ISA_AVX512_MOVGET
  if (HasAVX512MOVGET)
    Builder.defineMacro("__AVX512MOVGET__");
  Builder.defineMacro("__AVX512MOVGET_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX512_MOVGET
#if INTEL_FEATURE_ISA_GPR_MOVGET
  if (HasGPRMOVGET)
    Builder.defineMacro("__GPRMOVGET__");
  Builder.defineMacro("__GPRMOVGET_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_GPR_MOVGET
#if INTEL_FEATURE_ISA_MOVGET64B
  if (HasMOVGET64B)
    Builder.defineMacro("__MOVGET64B__");
  Builder.defineMacro("__MOVGET64B_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_MOVGET64B
#if INTEL_FEATURE_ISA_RAO_INT
  if (HasRAOINT)
    Builder.defineMacro("__RAOINT__");
  Builder.defineMacro("__RAOINT_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_RAO_INT
#if INTEL_FEATURE_ISA_AVX_RAO_INT
  if (HasAVXRAOINT)
    Builder.defineMacro("__AVXRAOINT__");
  Builder.defineMacro("__AVXRAOINT_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX_RAO_INT
#if INTEL_FEATURE_ISA_AVX_RAO_FP
// AUTO GENERATED BY TOOL
  if (HasAVXRAOFP)
    Builder.defineMacro("__AVXRAOFP__");
  Builder.defineMacro("__AVXRAOFP_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_RAO_FP
#if INTEL_FEATURE_ISA_AVX512_RAO_INT
// AUTO GENERATED BY TOOL
  if (HasAVX512RAOINT)
    Builder.defineMacro("__AVX512RAOINT__");
  Builder.defineMacro("__AVX512RAOINT_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_RAO_INT
#if INTEL_FEATURE_ISA_AVX512_RAO_FP
// AUTO GENERATED BY TOOL
  if (HasAVX512RAOFP)
    Builder.defineMacro("__AVX512RAOFP__");
  Builder.defineMacro("__AVX512RAOFP_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_RAO_FP
#if INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
// AUTO GENERATED BY TOOL
  if (HasAMXAVX512CVTROW)
    Builder.defineMacro("__AMXAVX512CVTROW__");
  Builder.defineMacro("__AMXAVX512CVTROW_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
#if INTEL_FEATURE_ISA_AVX_NE_CONVERT
// AUTO GENERATED BY TOOL
  if (HasAVXNECONVERT)
    Builder.defineMacro("__AVXNECONVERT__");
  Builder.defineMacro("__AVXNECONVERT_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_NE_CONVERT
#if INTEL_FEATURE_ISA_AVX512_NE_CONVERT
// AUTO GENERATED BY TOOL
  if (HasAVX512NECONVERT)
    Builder.defineMacro("__AVX512NECONVERT__");
  Builder.defineMacro("__AVX512NECONVERT_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_NE_CONVERT
#if INTEL_FEATURE_ISA_SHA512
// AUTO GENERATED BY TOOL
  if (HasSHA512)
    Builder.defineMacro("__SHA512__");
  Builder.defineMacro("__SHA512_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SHA512
#if INTEL_FEATURE_ISA_SM3
// AUTO GENERATED BY TOOL
  if (HasSM3)
    Builder.defineMacro("__SM3__");
  Builder.defineMacro("__SM3_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SM3
#if INTEL_FEATURE_ISA_SM4
// AUTO GENERATED BY TOOL
  if (HasSM4)
    Builder.defineMacro("__SM4__");
  Builder.defineMacro("__SM4_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SM4
#if INTEL_FEATURE_ISA_DSPV1
// AUTO GENERATED BY TOOL
  if (HasDSPV1)
    Builder.defineMacro("__DSPV1__");
  Builder.defineMacro("__DSPV1_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_DSPV1
#if INTEL_FEATURE_ISA_AVX_VNNI_INT16
// AUTO GENERATED BY TOOL
  if (HasAVXVNNIINT16)
    Builder.defineMacro("__AVXVNNIINT16__");
  Builder.defineMacro("__AVXVNNIINT16_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT16
#if INTEL_FEATURE_ISA_AVX512_VNNI_INT16
// AUTO GENERATED BY TOOL
  if (HasAVX512VNNIINT16)
    Builder.defineMacro("__AVX512VNNIINT16__");
  Builder.defineMacro("__AVX512VNNIINT16_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT16
#if INTEL_FEATURE_ISA_AMX_SPARSE
// AUTO GENERATED BY TOOL
  if (HasAMXSPARSE)
    Builder.defineMacro("__AMXSPARSE__");
  Builder.defineMacro("__AMXSPARSE_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_SPARSE
#if INTEL_FEATURE_ISA_AMX_V3
// AUTO GENERATED BY TOOL
  if (HasAMXV3)
    Builder.defineMacro("__AMXV3__");
  Builder.defineMacro("__AMXV3_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_V3
#if INTEL_FEATURE_ISA_VPINSR_VPEXTR
  if (HasVPINSRVPEXTR)
    Builder.defineMacro("__VPINSRVPEXTR__");
  Builder.defineMacro("__VPINSRVPEXTR_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_VPINSR_VPEXTR
#if INTEL_FEATURE_ISA_CMPCCXADD
// AUTO GENERATED BY TOOL
  if (HasCMPCCXADD)
    Builder.defineMacro("__CMPCCXADD__");
  Builder.defineMacro("__CMPCCXADD_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_CMPCCXADD
#if INTEL_FEATURE_ISA_AVX512_SAT_CVT
// AUTO GENERATED BY TOOL
  if (HasAVX512SATCVT)
    Builder.defineMacro("__AVX512SATCVT__");
  Builder.defineMacro("__AVX512SATCVT_SUPPORTED__");
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_SAT_CVT
#if INTEL_FEATURE_ISA_AVX256P
  if (HasAVX256P)
    Builder.defineMacro("__AVX256P__");
  Builder.defineMacro("__AVX256P_SUPPORTED__");
#endif // INTEL_FEATURE_ISA_AVX256P
#endif // INTEL_CUSTOMIZATION
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  // Disable setting of __SSE2_MATH__  as it enables guarded x86 inline asm in
  // bits/mathinline.hfor CSA and compiler errors on encountering the inline asm
  // Future we may was to disable only for CSA and disable other macro
  // defines not needed for CSA
  // TODO (vzakhari 11/14/2018): I do not understand why we call X86 target
  //       configuration on CSA.  This needs to be debugged.
  if (!Opts.OpenMPIsDevice) {
#endif  // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
  if (HasAVXVNNI)
    Builder.defineMacro("__AVXVNNI__");
  if (HasSERIALIZE)
    Builder.defineMacro("__SERIALIZE__");
  if (HasTSXLDTRK)
    Builder.defineMacro("__TSXLDTRK__");
  if (HasUINTR)
    Builder.defineMacro("__UINTR__");
  if (HasCRC32)
    Builder.defineMacro("__CRC32__");

  // Each case falls through to the previous one here.
  switch (SSELevel) {
  case AVX512F:
    Builder.defineMacro("__AVX512F__");
    LLVM_FALLTHROUGH;
  case AVX2:
    Builder.defineMacro("__AVX2__");
    LLVM_FALLTHROUGH;
  case AVX:
    Builder.defineMacro("__AVX__");
    LLVM_FALLTHROUGH;
  case SSE42:
    Builder.defineMacro("__SSE4_2__");
    LLVM_FALLTHROUGH;
  case SSE41:
    Builder.defineMacro("__SSE4_1__");
    LLVM_FALLTHROUGH;
  case SSSE3:
    Builder.defineMacro("__SSSE3__");
    LLVM_FALLTHROUGH;
  case SSE3:
    Builder.defineMacro("__SSE3__");
    LLVM_FALLTHROUGH;
  case SSE2:
    Builder.defineMacro("__SSE2__");
    Builder.defineMacro("__SSE2_MATH__"); // -mfp-math=sse always implied.
    LLVM_FALLTHROUGH;
  case SSE1:
    Builder.defineMacro("__SSE__");
    Builder.defineMacro("__SSE_MATH__"); // -mfp-math=sse always implied.
    LLVM_FALLTHROUGH;
  case NoSSE:
    break;
  }
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  }
#endif  // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

  if (Opts.MicrosoftExt && getTriple().getArch() == llvm::Triple::x86) {
    switch (SSELevel) {
    case AVX512F:
    case AVX2:
    case AVX:
    case SSE42:
    case SSE41:
    case SSSE3:
    case SSE3:
    case SSE2:
      Builder.defineMacro("_M_IX86_FP", Twine(2));
      break;
    case SSE1:
      Builder.defineMacro("_M_IX86_FP", Twine(1));
      break;
    default:
      Builder.defineMacro("_M_IX86_FP", Twine(0));
      break;
    }
  }

  // Each case falls through to the previous one here.
  switch (MMX3DNowLevel) {
  case AMD3DNowAthlon:
    Builder.defineMacro("__3dNOW_A__");
    LLVM_FALLTHROUGH;
  case AMD3DNow:
    Builder.defineMacro("__3dNOW__");
    LLVM_FALLTHROUGH;
  case MMX:
    Builder.defineMacro("__MMX__");
    LLVM_FALLTHROUGH;
  case NoMMX3DNow:
    break;
  }

  if (CPU >= CK_i486 || CPU == CK_None) {
    Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1");
    Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2");
    Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4");
  }
  if (HasCX8)
    Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8");
  if (HasCX16 && getTriple().getArch() == llvm::Triple::x86_64)
    Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_16");

  if (HasFloat128)
    Builder.defineMacro("__SIZEOF_FLOAT128__", "16");

#if INTEL_CUSTOMIZATION
  if (getTriple().getEnvironment() == llvm::Triple::IntelFPGA) {
    Builder.defineMacro("__fpga_reg", "__builtin_fpga_reg");
  }
#endif // INTEL_CUSTOMIZATION
}

bool X86TargetInfo::isValidFeatureName(StringRef Name) const {
  return llvm::StringSwitch<bool>(Name)
      .Case("3dnow", true)
      .Case("3dnowa", true)
      .Case("adx", true)
      .Case("aes", true)
      .Case("amx-bf16", true)
      .Case("amx-int8", true)
      .Case("amx-tile", true)
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AMX_BF8
      .Case("amx-bf8", true)
#endif // INTEL_FEATURE_ISA_AMX_BF8
#if INTEL_FEATURE_ISA_AMX_MEMADVISE
      .Case("amx-memadvise", true)
#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE
#if INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
      .Case("amx-memadvise-evex", true)
#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX

#if INTEL_FEATURE_ISA_AMX_FUTURE
      .Case("amx-reduce", true)
      .Case("amx-memory", true)
      .Case("amx-format", true)
      .Case("amx-element", true)
#endif // INTEL_FEATURE_ISA_AMX_FUTURE
#if INTEL_FEATURE_ISA_AMX_LNC
      .Case("amx-transpose", true)
      .Case("amx-avx512", true)
#endif // INTEL_FEATURE_ISA_AMX_LNC
#if INTEL_FEATURE_ISA_AMX_FP16
      .Case("amx-fp16", true)
#endif // INTEL_FEATURE_ISA_AMX_FP16
#if INTEL_FEATURE_ISA_AMX_MEMORY2
      .Case("amx-memory2", true)
#endif // INTEL_FEATURE_ISA_AMX_MEMORY2
#if INTEL_FEATURE_ISA_AMX_BF16_EVEX
      .Case("amx-bf16-evex", true)
#endif // INTEL_FEATURE_ISA_AMX_BF16_EVEX
#if INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
      .Case("amx-element-evex", true)
#endif // INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
#if INTEL_FEATURE_ISA_AMX_INT8_EVEX
      .Case("amx-int8-evex", true)
#endif // INTEL_FEATURE_ISA_AMX_INT8_EVEX
#if INTEL_FEATURE_ISA_AMX_TILE_EVEX
      .Case("amx-tile-evex", true)
#endif // INTEL_FEATURE_ISA_AMX_TILE_EVEX
#if INTEL_FEATURE_ISA_AMX_TRANSPOSE2
      .Case("amx-transpose2", true)
#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE2
#if INTEL_FEATURE_ISA_AMX_CONVERT
      .Case("amx-convert", true)
#endif // INTEL_FEATURE_ISA_AMX_CONVERT
#if INTEL_FEATURE_ISA_AMX_TILE2
      .Case("amx-tile2", true)
#endif // INTEL_FEATURE_ISA_AMX_TILE2
#if INTEL_FEATURE_ISA_AMX_COMPLEX
      .Case("amx-complex", true)
#endif // INTEL_FEATURE_ISA_AMX_COMPLEX
#if INTEL_FEATURE_ISA_AMX_TF32
      .Case("amx-tf32", true)
#endif // INTEL_FEATURE_ISA_AMX_TF32
#endif // INTEL_CUSTOMIZATION
      .Case("avx", true)
      .Case("avx2", true)
      .Case("avx512f", true)
      .Case("avx512cd", true)
      .Case("avx512vpopcntdq", true)
      .Case("avx512vnni", true)
      .Case("avx512bf16", true)
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX512_BF16_NE
      .Case("avx512bf16ne", true)
#endif // INTEL_FEATURE_ISA_AVX512_BF16_NE
#if INTEL_FEATURE_ISA_AVX_BF16
      .Case("avxbf16", true)
#endif // INTEL_FEATURE_ISA_AVX_BF16
#endif // INTEL_CUSTOMIZATION
      .Case("avx512er", true)
      .Case("avx512fp16", true)
      .Case("avx512pf", true)
      .Case("avx512dq", true)
      .Case("avx512bitalg", true)
      .Case("avx512bw", true)
      .Case("avx512vl", true)
      .Case("avx512vbmi", true)
      .Case("avx512vbmi2", true)
      .Case("avx512ifma", true)
      .Case("avx512vp2intersect", true)
      .Case("avxvnni", true)
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX_IFMA
      .Case("avxifma", true)
#endif // INTEL_FEATURE_ISA_AVX_IFMA
#endif // INTEL_CUSTOMIZATION
      .Case("bmi", true)
      .Case("bmi2", true)
      .Case("cldemote", true)
      .Case("clflushopt", true)
      .Case("clwb", true)
      .Case("clzero", true)
      .Case("crc32", true)
      .Case("cx16", true)
      .Case("enqcmd", true)
      .Case("f16c", true)
      .Case("fma", true)
      .Case("fma4", true)
      .Case("fsgsbase", true)
      .Case("fxsr", true)
      .Case("general-regs-only", true)
      .Case("gfni", true)
      .Case("hreset", true)
      .Case("invpcid", true)
      .Case("kl", true)
      .Case("widekl", true)
      .Case("lwp", true)
      .Case("lzcnt", true)
      .Case("mmx", true)
      .Case("movbe", true)
      .Case("movdiri", true)
      .Case("movdir64b", true)
      .Case("mwaitx", true)
      .Case("pclmul", true)
      .Case("pconfig", true)
      .Case("pku", true)
      .Case("popcnt", true)
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_PREFETCHI
      .Case("prefetchi", true)
#endif // INTEL_FEATURE_ISA_PREFETCHI
#if INTEL_FEATURE_ISA_PREFETCHST2
      .Case("prefetchst2", true)
#endif // INTEL_FEATURE_ISA_PREFETCHST2
#endif // INTEL_CUSTOMIZATION
      .Case("prefetchwt1", true)
      .Case("prfchw", true)
      .Case("ptwrite", true)
      .Case("rdpid", true)
      .Case("rdpru", true)
      .Case("rdrnd", true)
      .Case("rdseed", true)
      .Case("rtm", true)
      .Case("sahf", true)
      .Case("serialize", true)
      .Case("sgx", true)
      .Case("sha", true)
      .Case("shstk", true)
      .Case("sse", true)
      .Case("sse2", true)
      .Case("sse3", true)
      .Case("ssse3", true)
      .Case("sse4", true)
      .Case("sse4.1", true)
      .Case("sse4.2", true)
      .Case("sse4a", true)
      .Case("tbm", true)
      .Case("tsxldtrk", true)
      .Case("uintr", true)
      .Case("vaes", true)
      .Case("vpclmulqdq", true)
      .Case("wbnoinvd", true)
      .Case("waitpkg", true)
      .Case("x87", true)
      .Case("xop", true)
      .Case("xsave", true)
      .Case("xsavec", true)
      .Case("xsaves", true)
      .Case("xsaveopt", true)
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX512_VNNI_INT8
      .Case("avx512vnniint8", true)
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT8
#if INTEL_FEATURE_ISA_AVX512_VNNI_FP16
      .Case("avx512vnnifp16", true)
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_FP16
#if INTEL_FEATURE_ISA_AVX512_CONVERT
      .Case("avx512convert", true)
#endif // INTEL_FEATURE_ISA_AVX512_CONVERT
#if INTEL_FEATURE_ISA_AVX_VNNI_INT8
      .Case("avxvnniint8", true)
#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT8
#if INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
      .Case("avxdotprodphps", true)
#endif // INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
#if INTEL_FEATURE_ISA_AVX_CONVERT
      .Case("avxconvert", true)
#endif // INTEL_FEATURE_ISA_AVX_CONVERT
#if INTEL_FEATURE_ISA_AVX_COMPRESS
      .Case("avxcompress", true)
#endif // INTEL_FEATURE_ISA_AVX_COMPRESS
#if INTEL_FEATURE_ISA_AVX_MEMADVISE
      .Case("avxmemadvise", true)
      .Case("avx512memadvise", true)
#endif // INTEL_FEATURE_ISA_AVX_MEMADVISE
#if INTEL_FEATURE_ISA_AVX512_MEDIAX
      .Case("avx512mediax", true)
#endif // INTEL_FEATURE_ISA_AVX512_MEDIAX
#if INTEL_FEATURE_ISA_AVX_MOVGET
      .Case("avxmovget", true)
#endif // INTEL_FEATURE_ISA_AVX_MOVGET
#if INTEL_FEATURE_ISA_AVX512_MOVGET
      .Case("avx512movget", true)
#endif // INTEL_FEATURE_ISA_AVX512_MOVGET
#if INTEL_FEATURE_ISA_GPR_MOVGET
      .Case("gprmovget", true)
#endif // INTEL_FEATURE_ISA_GPR_MOVGET
#if INTEL_FEATURE_ISA_MOVGET64B
      .Case("movget64b", true)
#endif // INTEL_FEATURE_ISA_MOVGET64B
#if INTEL_FEATURE_ISA_RAO_INT
      .Case("raoint", true)
#endif // INTEL_FEATURE_ISA_RAO_INT
#if INTEL_FEATURE_ISA_AVX_RAO_INT
      .Case("avxraoint", true)
#endif // INTEL_FEATURE_ISA_AVX_RAO_INT
#if INTEL_FEATURE_ISA_AVX_RAO_FP
// AUTO GENERATED BY TOOL
      .Case("avxraofp", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_RAO_FP
#if INTEL_FEATURE_ISA_AVX512_RAO_INT
// AUTO GENERATED BY TOOL
      .Case("avx512raoint", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_RAO_INT
#if INTEL_FEATURE_ISA_AVX512_RAO_FP
// AUTO GENERATED BY TOOL
      .Case("avx512raofp", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_RAO_FP
#if INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
// AUTO GENERATED BY TOOL
      .Case("amx-avx512-cvtrow", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
#if INTEL_FEATURE_ISA_AVX_NE_CONVERT
// AUTO GENERATED BY TOOL
      .Case("avxneconvert", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_NE_CONVERT
#if INTEL_FEATURE_ISA_AVX512_NE_CONVERT
// AUTO GENERATED BY TOOL
      .Case("avx512neconvert", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_NE_CONVERT
#if INTEL_FEATURE_ISA_SHA512
// AUTO GENERATED BY TOOL
      .Case("sha512", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SHA512
#if INTEL_FEATURE_ISA_SM3
// AUTO GENERATED BY TOOL
      .Case("sm3", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SM3
#if INTEL_FEATURE_ISA_SM4
// AUTO GENERATED BY TOOL
      .Case("sm4", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SM4
#if INTEL_FEATURE_ISA_DSPV1
// AUTO GENERATED BY TOOL
      .Case("dspv1", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_DSPV1
#if INTEL_FEATURE_ISA_AVX_VNNI_INT16
// AUTO GENERATED BY TOOL
      .Case("avxvnniint16", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT16
#if INTEL_FEATURE_ISA_AVX512_VNNI_INT16
// AUTO GENERATED BY TOOL
      .Case("avx512vnniint16", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT16
#if INTEL_FEATURE_ISA_AMX_SPARSE
// AUTO GENERATED BY TOOL
      .Case("amx-sparse", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_SPARSE
#if INTEL_FEATURE_ISA_AMX_V3
// AUTO GENERATED BY TOOL
      .Case("amx-v3", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_V3
#if INTEL_FEATURE_ISA_VPINSR_VPEXTR
      .Case("vpinsr-vpextr", true)
#endif // INTEL_FEATURE_ISA_VPINSR_VPEXTR
#if INTEL_FEATURE_ISA_CMPCCXADD
// AUTO GENERATED BY TOOL
      .Case("cmpccxadd", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_CMPCCXADD
#if INTEL_FEATURE_ISA_AVX512_SAT_CVT
// AUTO GENERATED BY TOOL
      .Case("avx512satcvt", true)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_SAT_CVT
#if INTEL_FEATURE_ISA_AVX256P
      .Case("avx256p", true)
#endif // INTEL_FEATURE_ISA_AVX256P
#endif // INTEL_CUSTOMIZATION
      .Default(false);
}

bool X86TargetInfo::hasFeature(StringRef Feature) const {
  return llvm::StringSwitch<bool>(Feature)
      .Case("adx", HasADX)
      .Case("aes", HasAES)
      .Case("amx-bf16", HasAMXBF16)
      .Case("amx-int8", HasAMXINT8)
      .Case("amx-tile", HasAMXTILE)
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AMX_BF8
      .Case("amx-bf8", HasAMXBF8)
#endif // INTEL_FEATURE_ISA_AMX_BF8
#if INTEL_FEATURE_ISA_AMX_MEMADVISE
      .Case("amx-memadvise", HasAMXMEMADVISE)
#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE
#if INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
      .Case("amx-memadvise-evex", HasAMXMEMADVISEEVEX)
#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
#if INTEL_FEATURE_ISA_AMX_FUTURE
      .Case("amx-reduce", HasAMXREDUCE)
      .Case("amx-memory", HasAMXMEMORY)
      .Case("amx-format", HasAMXFORMAT)
      .Case("amx-element", HasAMXELEMENT)
#endif // INTEL_FEATURE_ISA_AMX_FUTURE
#if INTEL_FEATURE_ISA_AMX_LNC
      .Case("amx-transpose", HasAMXTRANSPOSE)
      .Case("amx-avx512", HasAMXAVX512)
#endif // INTEL_FEATURE_ISA_AMX_LNC
#if INTEL_FEATURE_ISA_AMX_FP16
      .Case("amx-fp16", HasAMXFP16)
#endif // INTEL_FEATURE_ISA_AMX_FP16
#if INTEL_FEATURE_ISA_AMX_MEMORY2
      .Case("amx-memory2", HasAMXMEMORY)
#endif // INTEL_FEATURE_ISA_AMX_MEMORY2
#if INTEL_FEATURE_ISA_AMX_BF16_EVEX
      .Case("amx-bf16-evex", HasAMXBF16EVEX)
#endif // INTEL_FEATURE_ISA_AMX_BF16_EVEX
#if INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
      .Case("amx-element-evex", HasAMXELEMENTEVEX)
#endif // INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
#if INTEL_FEATURE_ISA_AMX_INT8_EVEX
      .Case("amx-int8-evex", HasAMXINT8EVEX)
#endif // INTEL_FEATURE_ISA_AMX_INT8_EVEX
#if INTEL_FEATURE_ISA_AMX_TILE_EVEX
      .Case("amx-tile-evex", HasAMXTILEEVEX)
#endif // INTEL_FEATURE_ISA_AMX_TILE_EVEX
#if INTEL_FEATURE_ISA_AMX_TRANSPOSE2
      .Case("amx-transpose2", HasAMXTRANSPOSE2)
#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE2
#if INTEL_FEATURE_ISA_AMX_CONVERT
      .Case("amx-convert", HasAMXCONVERT)
#endif // INTEL_FEATURE_ISA_AMX_CONVERT
#if INTEL_FEATURE_ISA_AMX_TILE2
      .Case("amx-tile2", HasAMXTILE2)
#endif // INTEL_FEATURE_ISA_AMX_TILE2
#if INTEL_FEATURE_ISA_AMX_COMPLEX
      .Case("amx-complex", HasAMXCOMPLEX)
#endif // INTEL_FEATURE_ISA_AMX_COMPLEX
#if INTEL_FEATURE_ISA_AMX_TF32
      .Case("amx-tf32", HasAMXTF32)
#endif // INTEL_FEATURE_ISA_AMX_TF32
#if INTEL_FEATURE_ISA_AVX_IFMA
      .Case("avxifma", HasAVXIFMA)
#endif // INTEL_FEATURE_ISA_AVX_IFMA
#if INTEL_FEATURE_ISA_AVX_BF16
      .Case("avxbf16", HasAVXBF16)
#endif // INTEL_FEATURE_ISA_AVX_BF16
#endif // INTEL_CUSTOMIZATION
      .Case("avxvnni", HasAVXVNNI)
      .Case("avx", SSELevel >= AVX)
      .Case("avx2", SSELevel >= AVX2)
      .Case("avx512f", SSELevel >= AVX512F)
      .Case("avx512cd", HasAVX512CD)
      .Case("avx512vpopcntdq", HasAVX512VPOPCNTDQ)
      .Case("avx512vnni", HasAVX512VNNI)
      .Case("avx512bf16", HasAVX512BF16)
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX512_BF16_NE
      .Case("avx512bf16ne", HasAVX512BF16NE)
#endif // INTEL_FEATURE_ISA_AVX512_BF16_NE
#endif // INTEL_CUSTOMIZATION
      .Case("avx512er", HasAVX512ER)
      .Case("avx512fp16", HasAVX512FP16)
      .Case("avx512pf", HasAVX512PF)
      .Case("avx512dq", HasAVX512DQ)
      .Case("avx512bitalg", HasAVX512BITALG)
      .Case("avx512bw", HasAVX512BW)
      .Case("avx512vl", HasAVX512VL)
      .Case("avx512vbmi", HasAVX512VBMI)
      .Case("avx512vbmi2", HasAVX512VBMI2)
      .Case("avx512ifma", HasAVX512IFMA)
      .Case("avx512vp2intersect", HasAVX512VP2INTERSECT)
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX512_VNNI_INT8
      .Case("avx512vnniint8", HasAVX512VNNIINT8)
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT8
#if INTEL_FEATURE_ISA_AVX512_VNNI_FP16
      .Case("avx512vnnifp16", HasAVX512VNNIFP16)
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_FP16
#if INTEL_FEATURE_ISA_AVX512_CONVERT
      .Case("avx512convert", HasAVX512CONVERT)
#endif // INTEL_FEATURE_ISA_AVX512_CONVERT
#if INTEL_FEATURE_ISA_AVX_VNNI_INT8
      .Case("avxvnniint8", HasAVXVNNIINT8)
#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT8
#if INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
      .Case("avxdotprodphps", HasAVXDOTPRODPHPS)
#endif // INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
#if INTEL_FEATURE_ISA_AVX_CONVERT
      .Case("avxconvert", HasAVXCONVERT)
#endif // INTEL_FEATURE_ISA_AVX_CONVERT
#if INTEL_FEATURE_ISA_AVX_COMPRESS
      .Case("avxcompress", HasAVXCOMPRESS)
#endif // INTEL_FEATURE_ISA_AVX_COMPRESS
#if INTEL_FEATURE_ISA_AVX_MEMADVISE
      .Case("avxmemadvise", HasAVXMEMADVISE)
      .Case("avx512memadvise", HasAVX512MEMADVISE)
#endif // INTEL_FEATURE_ISA_AVX_MEMADVISE
#if INTEL_FEATURE_ISA_AVX512_MEDIAX
      .Case("avx512mediax", HasAVX512MEDIAX)
#endif // INTEL_FEATURE_ISA_AVX512_MEDIAX
#if INTEL_FEATURE_ISA_AVX_MOVGET
      .Case("avxmovget", HasAVXMOVGET)
#endif // INTEL_FEATURE_ISA_AVX_MOVGET
#if INTEL_FEATURE_ISA_AVX512_MOVGET
      .Case("avx512movget", HasAVX512MOVGET)
#endif // INTEL_FEATURE_ISA_AVX512_MOVGET
#if INTEL_FEATURE_ISA_GPR_MOVGET
      .Case("gprmovget", HasGPRMOVGET)
#endif // INTEL_FEATURE_ISA_GPR_MOVGET
#if INTEL_FEATURE_ISA_MOVGET64B
      .Case("movget64b", HasMOVGET64B)
#endif // INTEL_FEATURE_ISA_MOVGET64B
#if INTEL_FEATURE_ISA_RAO_INT
      .Case("raoint", HasRAOINT)
#endif // INTEL_FEATURE_ISA_RAO_INT
#if INTEL_FEATURE_ISA_AVX_RAO_INT
      .Case("avxraoint", HasAVXRAOINT)
#endif // INTEL_FEATURE_ISA_AVX_RAO_INT
#if INTEL_FEATURE_ISA_AVX_RAO_FP
// AUTO GENERATED BY TOOL
      .Case("avxraofp", HasAVXRAOFP)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_RAO_FP
#if INTEL_FEATURE_ISA_AVX512_RAO_INT
// AUTO GENERATED BY TOOL
      .Case("avx512raoint", HasAVX512RAOINT)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_RAO_INT
#if INTEL_FEATURE_ISA_AVX512_RAO_FP
// AUTO GENERATED BY TOOL
      .Case("avx512raofp", HasAVX512RAOFP)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_RAO_FP
#if INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
// AUTO GENERATED BY TOOL
      .Case("amx-avx512-cvtrow", HasAMXAVX512CVTROW)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
#if INTEL_FEATURE_ISA_AVX_NE_CONVERT
// AUTO GENERATED BY TOOL
      .Case("avxneconvert", HasAVXNECONVERT)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_NE_CONVERT
#if INTEL_FEATURE_ISA_AVX512_NE_CONVERT
// AUTO GENERATED BY TOOL
      .Case("avx512neconvert", HasAVX512NECONVERT)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_NE_CONVERT
#if INTEL_FEATURE_ISA_SHA512
// AUTO GENERATED BY TOOL
      .Case("sha512", HasSHA512)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SHA512
#if INTEL_FEATURE_ISA_SM3
// AUTO GENERATED BY TOOL
      .Case("sm3", HasSM3)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SM3
#if INTEL_FEATURE_ISA_SM4
// AUTO GENERATED BY TOOL
      .Case("sm4", HasSM4)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SM4
#if INTEL_FEATURE_ISA_DSPV1
// AUTO GENERATED BY TOOL
      .Case("dspv1", HasDSPV1)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_DSPV1
#if INTEL_FEATURE_ISA_AVX_VNNI_INT16
// AUTO GENERATED BY TOOL
      .Case("avxvnniint16", HasAVXVNNIINT16)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT16
#if INTEL_FEATURE_ISA_AVX512_VNNI_INT16
// AUTO GENERATED BY TOOL
      .Case("avx512vnniint16", HasAVX512VNNIINT16)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT16
#if INTEL_FEATURE_ISA_AMX_SPARSE
// AUTO GENERATED BY TOOL
      .Case("amx-sparse", HasAMXSPARSE)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_SPARSE
#if INTEL_FEATURE_ISA_AMX_V3
// AUTO GENERATED BY TOOL
      .Case("amx-v3", HasAMXV3)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_V3
#if INTEL_FEATURE_ISA_VPINSR_VPEXTR
      .Case("vpinsr-vpextr", HasVPINSRVPEXTR)
#endif // INTEL_FEATURE_ISA_VPINSR_VPEXTR
#if INTEL_FEATURE_ISA_CMPCCXADD
// AUTO GENERATED BY TOOL
      .Case("cmpccxadd", HasCMPCCXADD)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_CMPCCXADD
#if INTEL_FEATURE_ISA_AVX512_SAT_CVT
// AUTO GENERATED BY TOOL
      .Case("avx512satcvt", HasAVX512SATCVT)
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_SAT_CVT
#if INTEL_FEATURE_ISA_AVX256P
      .Case("avx256p", HasAVX256P)
#endif // INTEL_FEATURE_ISA_AVX256P
#endif // INTEL_CUSTOMIZATION
      .Case("bmi", HasBMI)
      .Case("bmi2", HasBMI2)
      .Case("cldemote", HasCLDEMOTE)
      .Case("clflushopt", HasCLFLUSHOPT)
      .Case("clwb", HasCLWB)
      .Case("clzero", HasCLZERO)
      .Case("crc32", HasCRC32)
      .Case("cx8", HasCX8)
      .Case("cx16", HasCX16)
      .Case("enqcmd", HasENQCMD)
      .Case("f16c", HasF16C)
      .Case("fma", HasFMA)
      .Case("fma4", XOPLevel >= FMA4)
      .Case("fsgsbase", HasFSGSBASE)
      .Case("fxsr", HasFXSR)
      .Case("gfni", HasGFNI)
      .Case("hreset", HasHRESET)
      .Case("invpcid", HasINVPCID)
      .Case("kl", HasKL)
      .Case("widekl", HasWIDEKL)
      .Case("lwp", HasLWP)
      .Case("lzcnt", HasLZCNT)
      .Case("mm3dnow", MMX3DNowLevel >= AMD3DNow)
      .Case("mm3dnowa", MMX3DNowLevel >= AMD3DNowAthlon)
      .Case("mmx", MMX3DNowLevel >= MMX)
      .Case("movbe", HasMOVBE)
      .Case("movdiri", HasMOVDIRI)
      .Case("movdir64b", HasMOVDIR64B)
      .Case("mwaitx", HasMWAITX)
      .Case("pclmul", HasPCLMUL)
      .Case("pconfig", HasPCONFIG)
      .Case("pku", HasPKU)
      .Case("popcnt", HasPOPCNT)
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_PREFETCHI
      .Case("prefetchi", HasPREFETCHI)
#endif // INTEL_FEATURE_ISA_PREFETCHI
#if INTEL_FEATURE_ISA_PREFETCHST2
      .Case("prefetchst2", HasPREFETCHST2)
#endif // INTEL_FEATURE_ISA_PREFETCHST2
#endif // INTEL_CUSTOMIZATION
      .Case("prefetchwt1", HasPREFETCHWT1)
      .Case("prfchw", HasPRFCHW)
      .Case("ptwrite", HasPTWRITE)
      .Case("rdpid", HasRDPID)
      .Case("rdpru", HasRDPRU)
      .Case("rdrnd", HasRDRND)
      .Case("rdseed", HasRDSEED)
      .Case("retpoline-external-thunk", HasRetpolineExternalThunk)
      .Case("rtm", HasRTM)
      .Case("sahf", HasLAHFSAHF)
      .Case("serialize", HasSERIALIZE)
      .Case("sgx", HasSGX)
      .Case("sha", HasSHA)
      .Case("shstk", HasSHSTK)
      .Case("sse", SSELevel >= SSE1)
      .Case("sse2", SSELevel >= SSE2)
      .Case("sse3", SSELevel >= SSE3)
      .Case("ssse3", SSELevel >= SSSE3)
      .Case("sse4.1", SSELevel >= SSE41)
      .Case("sse4.2", SSELevel >= SSE42)
      .Case("sse4a", XOPLevel >= SSE4A)
      .Case("tbm", HasTBM)
      .Case("tsxldtrk", HasTSXLDTRK)
      .Case("uintr", HasUINTR)
      .Case("vaes", HasVAES)
      .Case("vpclmulqdq", HasVPCLMULQDQ)
      .Case("wbnoinvd", HasWBNOINVD)
      .Case("waitpkg", HasWAITPKG)
      .Case("x86", true)
      .Case("x86_32", getTriple().getArch() == llvm::Triple::x86)
      .Case("x86_64", getTriple().getArch() == llvm::Triple::x86_64)
      .Case("x87", HasX87)
      .Case("xop", XOPLevel >= XOP)
      .Case("xsave", HasXSAVE)
      .Case("xsavec", HasXSAVEC)
      .Case("xsaves", HasXSAVES)
      .Case("xsaveopt", HasXSAVEOPT)
      .Default(false);
}

// We can't use a generic validation scheme for the features accepted here
// versus subtarget features accepted in the target attribute because the
// bitfield structure that's initialized in the runtime only supports the
// below currently rather than the full range of subtarget features. (See
// X86TargetInfo::hasFeature for a somewhat comprehensive list).
bool X86TargetInfo::validateCpuSupports(StringRef FeatureStr) const {
  return llvm::StringSwitch<bool>(FeatureStr)
#define X86_FEATURE_COMPAT(ENUM, STR, PRIORITY) .Case(STR, true)
#include "llvm/Support/X86TargetParser.def"
      .Default(false);
}

static llvm::X86::ProcessorFeatures getFeature(StringRef Name) {
  return llvm::StringSwitch<llvm::X86::ProcessorFeatures>(Name)
#define X86_FEATURE_COMPAT(ENUM, STR, PRIORITY)                                \
  .Case(STR, llvm::X86::FEATURE_##ENUM)

#include "llvm/Support/X86TargetParser.def"
      ;
  // Note, this function should only be used after ensuring the value is
  // correct, so it asserts if the value is out of range.
}

unsigned X86TargetInfo::multiVersionSortPriority(StringRef Name) const {
  // Valid CPUs have a 'key feature' that compares just better than its key
  // feature.
  using namespace llvm::X86;
  CPUKind Kind = parseArchX86(Name);
  if (Kind != CK_None) {
    ProcessorFeatures KeyFeature = getKeyFeature(Kind);
    return (getFeaturePriority(KeyFeature) << 1) + 1;
  }

  // Now we know we have a feature, so get its priority and shift it a few so
  // that we have sufficient room for the CPUs (above).
  return getFeaturePriority(getFeature(Name)) << 1;
}

bool X86TargetInfo::validateCPUSpecificCPUDispatch(StringRef Name) const {
  return llvm::StringSwitch<bool>(Name)
#define CPU_SPECIFIC(NAME, TUNE_NAME, MANGLING, FEATURES) .Case(NAME, true)
#define CPU_SPECIFIC_ALIAS(NEW_NAME, TUNE_NAME, NAME) .Case(NEW_NAME, true)
#include "llvm/Support/X86TargetParser.def"
      .Default(false);
}

static StringRef CPUSpecificCPUDispatchNameDealias(StringRef Name) {
  return llvm::StringSwitch<StringRef>(Name)
#define CPU_SPECIFIC_ALIAS(NEW_NAME, TUNE_NAME, NAME) .Case(NEW_NAME, NAME)
#include "llvm/Support/X86TargetParser.def"
      .Default(Name);
}

char X86TargetInfo::CPUSpecificManglingCharacter(StringRef Name) const {
  return llvm::StringSwitch<char>(CPUSpecificCPUDispatchNameDealias(Name))
#define CPU_SPECIFIC(NAME, TUNE_NAME, MANGLING, FEATURES) .Case(NAME, MANGLING)
#include "llvm/Support/X86TargetParser.def"
      .Default(0);
}

void X86TargetInfo::getCPUSpecificCPUDispatchFeatures(
    StringRef Name, llvm::SmallVectorImpl<StringRef> &Features) const {
  StringRef WholeList =
      llvm::StringSwitch<StringRef>(CPUSpecificCPUDispatchNameDealias(Name))
#define CPU_SPECIFIC(NAME, TUNE_NAME, MANGLING, FEATURES) .Case(NAME, FEATURES)
#include "llvm/Support/X86TargetParser.def"
          .Default("");
  WholeList.split(Features, ',', /*MaxSplit=*/-1, /*KeepEmpty=*/false);
}

StringRef X86TargetInfo::getCPUSpecificTuneName(StringRef Name) const {
  return llvm::StringSwitch<StringRef>(Name)
#define CPU_SPECIFIC(NAME, TUNE_NAME, MANGLING, FEATURES) .Case(NAME, TUNE_NAME)
#define CPU_SPECIFIC_ALIAS(NEW_NAME, TUNE_NAME, NAME) .Case(NEW_NAME, TUNE_NAME)
#include "llvm/Support/X86TargetParser.def"
      .Default("");
}

// We can't use a generic validation scheme for the cpus accepted here
// versus subtarget cpus accepted in the target attribute because the
// variables intitialized by the runtime only support the below currently
// rather than the full range of cpus.
bool X86TargetInfo::validateCpuIs(StringRef FeatureStr) const {
  return llvm::StringSwitch<bool>(FeatureStr)
#define X86_VENDOR(ENUM, STRING) .Case(STRING, true)
#define X86_CPU_TYPE_ALIAS(ENUM, ALIAS) .Case(ALIAS, true)
#define X86_CPU_TYPE(ENUM, STR) .Case(STR, true)
#define X86_CPU_SUBTYPE(ENUM, STR) .Case(STR, true)
#include "llvm/Support/X86TargetParser.def"
      .Default(false);
}

static unsigned matchAsmCCConstraint(const char *&Name) {
  auto RV = llvm::StringSwitch<unsigned>(Name)
                .Case("@cca", 4)
                .Case("@ccae", 5)
                .Case("@ccb", 4)
                .Case("@ccbe", 5)
                .Case("@ccc", 4)
                .Case("@cce", 4)
                .Case("@ccz", 4)
                .Case("@ccg", 4)
                .Case("@ccge", 5)
                .Case("@ccl", 4)
                .Case("@ccle", 5)
                .Case("@ccna", 5)
                .Case("@ccnae", 6)
                .Case("@ccnb", 5)
                .Case("@ccnbe", 6)
                .Case("@ccnc", 5)
                .Case("@ccne", 5)
                .Case("@ccnz", 5)
                .Case("@ccng", 5)
                .Case("@ccnge", 6)
                .Case("@ccnl", 5)
                .Case("@ccnle", 6)
                .Case("@ccno", 5)
                .Case("@ccnp", 5)
                .Case("@ccns", 5)
                .Case("@cco", 4)
                .Case("@ccp", 4)
                .Case("@ccs", 4)
                .Default(0);
  return RV;
}

bool X86TargetInfo::validateAsmConstraint(
    const char *&Name, TargetInfo::ConstraintInfo &Info) const {
  switch (*Name) {
  default:
    return false;
  // Constant constraints.
  case 'e': // 32-bit signed integer constant for use with sign-extending x86_64
            // instructions.
  case 'Z': // 32-bit unsigned integer constant for use with zero-extending
            // x86_64 instructions.
  case 's':
    Info.setRequiresImmediate();
    return true;
  case 'I':
    Info.setRequiresImmediate(0, 31);
    return true;
  case 'J':
    Info.setRequiresImmediate(0, 63);
    return true;
  case 'K':
    Info.setRequiresImmediate(-128, 127);
    return true;
  case 'L':
    Info.setRequiresImmediate({int(0xff), int(0xffff), int(0xffffffff)});
    return true;
  case 'M':
    Info.setRequiresImmediate(0, 3);
    return true;
  case 'N':
    Info.setRequiresImmediate(0, 255);
    return true;
  case 'O':
    Info.setRequiresImmediate(0, 127);
    return true;
  // Register constraints.
  case 'Y': // 'Y' is the first character for several 2-character constraints.
    // Shift the pointer to the second character of the constraint.
    Name++;
    switch (*Name) {
    default:
      return false;
    case 'z': // First SSE register.
    case '2':
    case 't': // Any SSE register, when SSE2 is enabled.
    case 'i': // Any SSE register, when SSE2 and inter-unit moves enabled.
    case 'm': // Any MMX register, when inter-unit moves enabled.
    case 'k': // AVX512 arch mask registers: k1-k7.
      Info.setAllowsRegister();
      return true;
    }
  case 'f': // Any x87 floating point stack register.
    // Constraint 'f' cannot be used for output operands.
    if (Info.ConstraintStr[0] == '=')
      return false;
    Info.setAllowsRegister();
    return true;
  case 'a': // eax.
  case 'b': // ebx.
  case 'c': // ecx.
  case 'd': // edx.
  case 'S': // esi.
  case 'D': // edi.
  case 'A': // edx:eax.
  case 't': // Top of floating point stack.
  case 'u': // Second from top of floating point stack.
  case 'q': // Any register accessible as [r]l: a, b, c, and d.
  case 'y': // Any MMX register.
  case 'v': // Any {X,Y,Z}MM register (Arch & context dependent)
  case 'x': // Any SSE register.
  case 'k': // Any AVX512 mask register (same as Yk, additionally allows k0
            // for intermideate k reg operations).
  case 'Q': // Any register accessible as [r]h: a, b, c, and d.
  case 'R': // "Legacy" registers: ax, bx, cx, dx, di, si, sp, bp.
  case 'l': // "Index" registers: any general register that can be used as an
            // index in a base+index memory access.
    Info.setAllowsRegister();
    return true;
  // Floating point constant constraints.
  case 'C': // SSE floating point constant.
  case 'G': // x87 floating point constant.
    return true;
  case '@':
    // CC condition changes.
    if (auto Len = matchAsmCCConstraint(Name)) {
      Name += Len - 1;
      Info.setAllowsRegister();
      return true;
    }
    return false;
  }
}

// Below is based on the following information:
// +------------------------------------+-------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------+
// |           Processor Name           | Cache Line Size (Bytes) |                                                                            Source                                                                            |
// +------------------------------------+-------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | i386                               |                      64 | https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-optimization-manual.pdf                                          |
// | i486                               |                      16 | "four doublewords" (doubleword = 32 bits, 4 bits * 32 bits = 16 bytes) https://en.wikichip.org/w/images/d/d3/i486_MICROPROCESSOR_HARDWARE_REFERENCE_MANUAL_%281990%29.pdf and http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.126.4216&rep=rep1&type=pdf (page 29) |
// | i586/Pentium MMX                   |                      32 | https://www.7-cpu.com/cpu/P-MMX.html                                                                                                                         |
// | i686/Pentium                       |                      32 | https://www.7-cpu.com/cpu/P6.html                                                                                                                            |
// | Netburst/Pentium4                  |                      64 | https://www.7-cpu.com/cpu/P4-180.html                                                                                                                        |
// | Atom                               |                      64 | https://www.7-cpu.com/cpu/Atom.html                                                                                                                          |
// | Westmere                           |                      64 | https://en.wikichip.org/wiki/intel/microarchitectures/sandy_bridge_(client) "Cache Architecture"                                                             |
// | Sandy Bridge                       |                      64 | https://en.wikipedia.org/wiki/Sandy_Bridge and https://www.7-cpu.com/cpu/SandyBridge.html                                                                    |
// | Ivy Bridge                         |                      64 | https://blog.stuffedcow.net/2013/01/ivb-cache-replacement/ and https://www.7-cpu.com/cpu/IvyBridge.html                                                      |
// | Haswell                            |                      64 | https://www.7-cpu.com/cpu/Haswell.html                                                                                                                       |
// | Boadwell                           |                      64 | https://www.7-cpu.com/cpu/Broadwell.html                                                                                                                     |
// | Skylake (including skylake-avx512) |                      64 | https://www.nas.nasa.gov/hecc/support/kb/skylake-processors_550.html "Cache Hierarchy"                                                                       |
// | Cascade Lake                       |                      64 | https://www.nas.nasa.gov/hecc/support/kb/cascade-lake-processors_579.html "Cache Hierarchy"                                                                  |
// | Skylake                            |                      64 | https://en.wikichip.org/wiki/intel/microarchitectures/kaby_lake "Memory Hierarchy"                                                                           |
// | Ice Lake                           |                      64 | https://www.7-cpu.com/cpu/Ice_Lake.html                                                                                                                      |
// | Knights Landing                    |                      64 | https://software.intel.com/en-us/articles/intel-xeon-phi-processor-7200-family-memory-management-optimizations "The Intel® Xeon Phi™ Processor Architecture" |
// | Knights Mill                       |                      64 | https://software.intel.com/sites/default/files/managed/9e/bc/64-ia-32-architectures-optimization-manual.pdf?countrylabel=Colombia "2.5.5.2 L1 DCache "       |
// +------------------------------------+-------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------+
Optional<unsigned> X86TargetInfo::getCPUCacheLineSize() const {
  using namespace llvm::X86;
  switch (CPU) {
    // i386
    case CK_i386:
    // i486
    case CK_i486:
    case CK_WinChipC6:
    case CK_WinChip2:
    case CK_C3:
    // Lakemont
    case CK_Lakemont:
      return 16;

    // i586
    case CK_i586:
    case CK_Pentium:
    case CK_PentiumMMX:
    // i686
    case CK_PentiumPro:
    case CK_i686:
    case CK_Pentium2:
    case CK_Pentium3:
    case CK_PentiumM:
    case CK_C3_2:
    // K6
    case CK_K6:
    case CK_K6_2:
    case CK_K6_3:
    // Geode
    case CK_Geode:
      return 32;

    // Netburst
    case CK_Pentium4:
    case CK_Prescott:
    case CK_Nocona:
    // Atom
    case CK_Bonnell:
    case CK_Silvermont:
    case CK_Goldmont:
    case CK_GoldmontPlus:
    case CK_Tremont:
#if INTEL_CUSTOMIZATION
    case CK_Gracemont:
#endif // INTEL_CUSTOMIZATION
    case CK_Westmere:
    case CK_SandyBridge:
    case CK_IvyBridge:
    case CK_Haswell:
    case CK_Broadwell:
    case CK_SkylakeClient:
    case CK_SkylakeServer:
    case CK_Cascadelake:
    case CK_Nehalem:
    case CK_Cooperlake:
    case CK_Cannonlake:
    case CK_Tigerlake:
    case CK_SapphireRapids:
    case CK_IcelakeClient:
    case CK_Rocketlake:
    case CK_IcelakeServer:
#if INTEL_CUSTOMIZATION
    case CK_CommonAVX512:
#if INTEL_FEATURE_ISA_AVX256
    case CK_CommonAVX256:
#endif // INTEL_FEATURE_ISA_AVX256
#endif // INTEL_CUSTOMIZATION
    case CK_Alderlake:
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CPU_RPL
    case CK_Raptorlake:
#endif // INTEL_FEATURE_CPU_RPL
#if INTEL_FEATURE_CPU_GNR
    case CK_Graniterapids:
#endif // INTEL_FEATURE_CPU_GNR
#if INTEL_FEATURE_CPU_DMR
    case CK_Diamondrapids:
#endif // INTEL_FEATURE_CPU_DMR
#endif // INTEL_CUSTOMIZATION
    case CK_KNL:
    case CK_KNM:
    // K7
    case CK_Athlon:
    case CK_AthlonXP:
    // K8
    case CK_K8:
    case CK_K8SSE3:
    case CK_AMDFAM10:
    // Bobcat
    case CK_BTVER1:
    case CK_BTVER2:
    // Bulldozer
    case CK_BDVER1:
    case CK_BDVER2:
    case CK_BDVER3:
    case CK_BDVER4:
    // Zen
    case CK_ZNVER1:
    case CK_ZNVER2:
    case CK_ZNVER3:
    // Deprecated
    case CK_x86_64:
    case CK_x86_64_v2:
    case CK_x86_64_v3:
    case CK_x86_64_v4:
    case CK_Yonah:
    case CK_Penryn:
    case CK_Core2:
      return 64;

    // The following currently have unknown cache line sizes (but they are probably all 64):
    // Core
    case CK_None:
      return None;
  }
  llvm_unreachable("Unknown CPU kind");
}

bool X86TargetInfo::validateOutputSize(const llvm::StringMap<bool> &FeatureMap,
                                       StringRef Constraint,
                                       unsigned Size) const {
  // Strip off constraint modifiers.
  while (Constraint[0] == '=' || Constraint[0] == '+' || Constraint[0] == '&')
    Constraint = Constraint.substr(1);

  return validateOperandSize(FeatureMap, Constraint, Size);
}

bool X86TargetInfo::validateInputSize(const llvm::StringMap<bool> &FeatureMap,
                                      StringRef Constraint,
                                      unsigned Size) const {
  return validateOperandSize(FeatureMap, Constraint, Size);
}

bool X86TargetInfo::validateOperandSize(const llvm::StringMap<bool> &FeatureMap,
                                        StringRef Constraint,
                                        unsigned Size) const {
  switch (Constraint[0]) {
  default:
    break;
  case 'k':
  // Registers k0-k7 (AVX512) size limit is 64 bit.
  case 'y':
    return Size <= 64;
  case 'f':
  case 't':
  case 'u':
    return Size <= 128;
  case 'Y':
    // 'Y' is the first character for several 2-character constraints.
    switch (Constraint[1]) {
    default:
      return false;
    case 'm':
      // 'Ym' is synonymous with 'y'.
    case 'k':
      return Size <= 64;
    case 'z':
      // XMM0/YMM/ZMM0
      if (hasFeatureEnabled(FeatureMap, "avx512f"))
        // ZMM0 can be used if target supports AVX512F.
        return Size <= 512U;
      else if (hasFeatureEnabled(FeatureMap, "avx"))
        // YMM0 can be used if target supports AVX.
        return Size <= 256U;
      else if (hasFeatureEnabled(FeatureMap, "sse"))
        return Size <= 128U;
      return false;
    case 'i':
    case 't':
    case '2':
      // 'Yi','Yt','Y2' are synonymous with 'x' when SSE2 is enabled.
      if (SSELevel < SSE2)
        return false;
      break;
    }
    break;
  case 'v':
  case 'x':
    if (hasFeatureEnabled(FeatureMap, "avx512f"))
      // 512-bit zmm registers can be used if target supports AVX512F.
      return Size <= 512U;
    else if (hasFeatureEnabled(FeatureMap, "avx"))
      // 256-bit ymm registers can be used if target supports AVX.
      return Size <= 256U;
    return Size <= 128U;

  }

  return true;
}

std::string X86TargetInfo::convertConstraint(const char *&Constraint) const {
  switch (*Constraint) {
  case '@':
    if (auto Len = matchAsmCCConstraint(Constraint)) {
      std::string Converted = "{" + std::string(Constraint, Len) + "}";
      Constraint += Len - 1;
      return Converted;
    }
    return std::string(1, *Constraint);
  case 'a':
    return std::string("{ax}");
  case 'b':
    return std::string("{bx}");
  case 'c':
    return std::string("{cx}");
  case 'd':
    return std::string("{dx}");
  case 'S':
    return std::string("{si}");
  case 'D':
    return std::string("{di}");
  case 'p': // Keep 'p' constraint (address).
    return std::string("p");
  case 't': // top of floating point stack.
    return std::string("{st}");
  case 'u':                        // second from top of floating point stack.
    return std::string("{st(1)}"); // second from top of floating point stack.
  case 'Y':
    switch (Constraint[1]) {
    default:
      // Break from inner switch and fall through (copy single char),
      // continue parsing after copying the current constraint into
      // the return string.
      break;
    case 'k':
    case 'm':
    case 'i':
    case 't':
    case 'z':
    case '2':
      // "^" hints llvm that this is a 2 letter constraint.
      // "Constraint++" is used to promote the string iterator
      // to the next constraint.
      return std::string("^") + std::string(Constraint++, 2);
    }
    LLVM_FALLTHROUGH;
  default:
    return std::string(1, *Constraint);
  }
}

void X86TargetInfo::fillValidCPUList(SmallVectorImpl<StringRef> &Values) const {
  bool Only64Bit = getTriple().getArch() != llvm::Triple::x86;
  llvm::X86::fillValidCPUArchList(Values, Only64Bit);
}

void X86TargetInfo::fillValidTuneCPUList(SmallVectorImpl<StringRef> &Values) const {
  llvm::X86::fillValidTuneCPUList(Values);
}

ArrayRef<const char *> X86TargetInfo::getGCCRegNames() const {
  return llvm::makeArrayRef(GCCRegNames);
}

ArrayRef<TargetInfo::AddlRegName> X86TargetInfo::getGCCAddlRegNames() const {
  return llvm::makeArrayRef(AddlRegNames);
}

ArrayRef<Builtin::Info> X86_32TargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfoX86, clang::X86::LastX86CommonBuiltin -
                                                Builtin::FirstTSBuiltin + 1);
}

ArrayRef<Builtin::Info> X86_64TargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfoX86,
                            X86::LastTSBuiltin - Builtin::FirstTSBuiltin);
}

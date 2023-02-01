//===--- Builtins.cpp - Builtin function implementation -------------------===//
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
//  This file implements various things for builtin functions.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/Builtins.h"
#include "BuiltinTargetFeatures.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "llvm/ADT/StringRef.h"
using namespace clang;

const char *HeaderDesc::getName() const {
  switch (ID) {
#define HEADER(ID, NAME)                                                       \
  case ID:                                                                     \
    return NAME;
#include "clang/Basic/BuiltinHeaders.def"
#undef HEADER
  };
}

static constexpr Builtin::Info BuiltinInfo[] = {
    {"not a builtin function", nullptr, nullptr, nullptr, HeaderDesc::NO_HEADER,
     ALL_LANGUAGES},
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, HeaderDesc::NO_HEADER, ALL_LANGUAGES},
#define LANGBUILTIN(ID, TYPE, ATTRS, LANGS)                                    \
  {#ID, TYPE, ATTRS, nullptr, HeaderDesc::NO_HEADER, LANGS},
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER, LANGS)                             \
  {#ID, TYPE, ATTRS, nullptr, HeaderDesc::HEADER, LANGS},
#include "clang/Basic/Builtins.def"
};

const Builtin::Info &Builtin::Context::getRecord(unsigned ID) const {
  if (ID < Builtin::FirstTSBuiltin)
    return BuiltinInfo[ID];
  assert(((ID - Builtin::FirstTSBuiltin) <
          (TSRecords.size() + AuxTSRecords.size())) &&
         "Invalid builtin ID!");
  if (isAuxBuiltinID(ID))
    return AuxTSRecords[getAuxBuiltinID(ID) - Builtin::FirstTSBuiltin];
  return TSRecords[ID - Builtin::FirstTSBuiltin];
}

void Builtin::Context::InitializeTarget(const TargetInfo &Target,
                                        const TargetInfo *AuxTarget) {
  assert(TSRecords.empty() && "Already initialized target?");
  TSRecords = Target.getTargetBuiltins();
  if (AuxTarget)
    AuxTSRecords = AuxTarget->getTargetBuiltins();
}

bool Builtin::Context::isBuiltinFunc(llvm::StringRef FuncName) {
  bool InStdNamespace = FuncName.consume_front("std-");
  for (unsigned i = Builtin::NotBuiltin + 1; i != Builtin::FirstTSBuiltin;
       ++i) {
    if (FuncName.equals(BuiltinInfo[i].Name) &&
        (bool)strchr(BuiltinInfo[i].Attributes, 'z') == InStdNamespace)
      return strchr(BuiltinInfo[i].Attributes, 'f') != nullptr;
  }

  return false;
}

#if INTEL_CUSTOMIZATION
static bool CheckIntelBuiltinSupported(bool IsOtherwiseSupported,
                                       const Builtin::Info &BuiltinInfo,
                                       const LangOptions &LangOpts) {
  if (!(BuiltinInfo.Langs & ICC_LANG))
    return IsOtherwiseSupported;
  bool IsIntelCompat = LangOpts.IntelCompat;

  // Intel Customization per-feature testing zone.  Future code should check to
  // see if it is their builtin.  If so, update IsIntelCompat, which defaults to
  // the global IntelCompat setting.
  StringRef Name = BuiltinInfo.Name;
  if (Name == "__builtin_va_arg_pack" || Name == "__builtin_va_arg_pack_len")
    IsIntelCompat = LangOpts.isIntelCompat(LangOptions::VaArgPack);

  if (BuiltinInfo.Langs == ICC_LANG) return IsIntelCompat;
  return IsIntelCompat || IsOtherwiseSupported;
}
#endif // INTEL_CUSTOMIZATION

/// Is this builtin supported according to the given language options?
static bool builtinIsSupported(const Builtin::Info &BuiltinInfo,
                               const LangOptions &LangOpts) {
#if INTEL_CUSTOMIZATION
  bool BuiltinsUnsupported =
      LangOpts.NoBuiltin && strchr(BuiltinInfo.Attributes, 'f') != nullptr;
  if (BuiltinsUnsupported)
    return false;
  bool CorBuiltinsUnsupported =
      !LangOpts.Coroutines && (BuiltinInfo.Langs & COR_LANG);
  if (CorBuiltinsUnsupported)
    return false;
<<<<<<< HEAD
  bool MathBuiltinsUnsupported =
      LangOpts.NoMathBuiltin && BuiltinInfo.HeaderName &&
      llvm::StringRef(BuiltinInfo.HeaderName).equals("math.h");
  if (MathBuiltinsUnsupported)
=======
  if (bool MathBuiltinsUnsupported =
          LangOpts.NoMathBuiltin && BuiltinInfo.Header.ID == HeaderDesc::MATH_H)
>>>>>>> 5a7f47cc021bd7a19cb70c9a30755d6b3cb67431
    return false;
  bool GnuModeUnsupported = !LangOpts.GNUMode && (BuiltinInfo.Langs & GNU_LANG);
  if (GnuModeUnsupported)
    return false;
  bool MSModeUnsupported =
      !LangOpts.MicrosoftExt && (BuiltinInfo.Langs & MS_LANG);
  if (MSModeUnsupported)
    return false;
  bool ObjCUnsupported = !LangOpts.ObjC && BuiltinInfo.Langs == OBJC_LANG;
  if (ObjCUnsupported)
    return false;
  bool OclCUnsupported =
      !LangOpts.OpenCL && (BuiltinInfo.Langs & ALL_OCL_LANGUAGES);
  if (OclCUnsupported)
    return false;
  bool OclGASUnsupported =
      !LangOpts.OpenCLGenericAddressSpace && (BuiltinInfo.Langs & OCL_GAS);
  if (OclGASUnsupported)
    return false;
  // Register target-specific pipe builtins
  bool OclPipeUnsupported =
      (!LangOpts.OpenCLPipes && (BuiltinInfo.Langs & OCL_PIPE)) ||
      ((LangOpts.OpenCLVersion / 100) != 1 &&
       (BuiltinInfo.Langs & ALL_OCL_PIPE) == INTEL_FPGA_PIPE1X);
  if (OclPipeUnsupported)
    return false;
  // Device side enqueue is not supported until OpenCL 2.0. In 2.0 and higher
  // support is indicated with language option for blocks.
  bool OclDSEUnsupported =
      (LangOpts.getOpenCLCompatibleVersion() < 200 || !LangOpts.Blocks) &&
      (BuiltinInfo.Langs & OCL_DSE);
  if (OclDSEUnsupported)
    return false;
  bool OpenMPUnsupported = !LangOpts.OpenMP && BuiltinInfo.Langs == OMP_LANG;
  if (OpenMPUnsupported)
    return false;
  bool CUDAUnsupported = !LangOpts.CUDA && BuiltinInfo.Langs == CUDA_LANG;
  if (CUDAUnsupported)
    return false;
  bool CPlusPlusUnsupported =
      !LangOpts.CPlusPlus && BuiltinInfo.Langs == CXX_LANG;
  if (CPlusPlusUnsupported)
    return false;
  // First parameter should be exactly the return statement from community.
  return CheckIntelBuiltinSupported(
      (!BuiltinsUnsupported && !CorBuiltinsUnsupported &&
       !MathBuiltinsUnsupported && !OclCUnsupported && !OclGASUnsupported &&
       !OclPipeUnsupported && !OclDSEUnsupported && !OpenMPUnsupported &&
       !GnuModeUnsupported && !MSModeUnsupported && !ObjCUnsupported &&
       !CPlusPlusUnsupported && !CUDAUnsupported),
      BuiltinInfo, LangOpts);
#endif // INTEL_CUSTOMIZATION
}

/// initializeBuiltins - Mark the identifiers for all the builtins with their
/// appropriate builtin ID # and mark any non-portable builtin identifiers as
/// such.
void Builtin::Context::initializeBuiltins(IdentifierTable &Table,
                                          const LangOptions& LangOpts) {
  // Step #1: mark all target-independent builtins with their ID's.
  for (unsigned i = Builtin::NotBuiltin+1; i != Builtin::FirstTSBuiltin; ++i)
    if (builtinIsSupported(BuiltinInfo[i], LangOpts)) {
      Table.get(BuiltinInfo[i].Name).setBuiltinID(i);
    }

  // Step #2: Register target-specific builtins.
  for (unsigned i = 0, e = TSRecords.size(); i != e; ++i)
    if (builtinIsSupported(TSRecords[i], LangOpts))
      Table.get(TSRecords[i].Name).setBuiltinID(i + Builtin::FirstTSBuiltin);

  // Step #3: Register target-specific builtins for AuxTarget.
  for (unsigned i = 0, e = AuxTSRecords.size(); i != e; ++i)
    Table.get(AuxTSRecords[i].Name)
        .setBuiltinID(i + Builtin::FirstTSBuiltin + TSRecords.size());

  // Step #4: Unregister any builtins specified by -fno-builtin-foo.
  for (llvm::StringRef Name : LangOpts.NoBuiltinFuncs) {
    bool InStdNamespace = Name.consume_front("std-");
    auto NameIt = Table.find(Name);
    if (NameIt != Table.end()) {
      unsigned ID = NameIt->second->getBuiltinID();
      if (ID != Builtin::NotBuiltin && isPredefinedLibFunction(ID) &&
          isInStdNamespace(ID) == InStdNamespace) {
        Table.get(Name).setBuiltinID(Builtin::NotBuiltin);
      }
    }
  }
}

unsigned Builtin::Context::getRequiredVectorWidth(unsigned ID) const {
  const char *WidthPos = ::strchr(getRecord(ID).Attributes, 'V');
  if (!WidthPos)
    return 0;

  ++WidthPos;
  assert(*WidthPos == ':' &&
         "Vector width specifier must be followed by a ':'");
  ++WidthPos;

  char *EndPos;
  unsigned Width = ::strtol(WidthPos, &EndPos, 10);
  assert(*EndPos == ':' && "Vector width specific must end with a ':'");
  return Width;
}

bool Builtin::Context::isLike(unsigned ID, unsigned &FormatIdx,
                              bool &HasVAListArg, const char *Fmt) const {
  assert(Fmt && "Not passed a format string");
  assert(::strlen(Fmt) == 2 &&
         "Format string needs to be two characters long");
  assert(::toupper(Fmt[0]) == Fmt[1] &&
         "Format string is not in the form \"xX\"");

  const char *Like = ::strpbrk(getRecord(ID).Attributes, Fmt);
  if (!Like)
    return false;

  HasVAListArg = (*Like == Fmt[1]);

  ++Like;
  assert(*Like == ':' && "Format specifier must be followed by a ':'");
  ++Like;

  assert(::strchr(Like, ':') && "Format specifier must end with a ':'");
  FormatIdx = ::strtol(Like, nullptr, 10);
  return true;
}

bool Builtin::Context::isPrintfLike(unsigned ID, unsigned &FormatIdx,
                                    bool &HasVAListArg) {
  return isLike(ID, FormatIdx, HasVAListArg, "pP");
}

bool Builtin::Context::isScanfLike(unsigned ID, unsigned &FormatIdx,
                                   bool &HasVAListArg) {
  return isLike(ID, FormatIdx, HasVAListArg, "sS");
}

bool Builtin::Context::performsCallback(unsigned ID,
                                        SmallVectorImpl<int> &Encoding) const {
  const char *CalleePos = ::strchr(getRecord(ID).Attributes, 'C');
  if (!CalleePos)
    return false;

  ++CalleePos;
  assert(*CalleePos == '<' &&
         "Callback callee specifier must be followed by a '<'");
  ++CalleePos;

  char *EndPos;
  int CalleeIdx = ::strtol(CalleePos, &EndPos, 10);
  assert(CalleeIdx >= 0 && "Callee index is supposed to be positive!");
  Encoding.push_back(CalleeIdx);

  while (*EndPos == ',') {
    const char *PayloadPos = EndPos + 1;

    int PayloadIdx = ::strtol(PayloadPos, &EndPos, 10);
    Encoding.push_back(PayloadIdx);
  }

  assert(*EndPos == '>' && "Callback callee specifier must end with a '>'");
  return true;
}

bool Builtin::Context::canBeRedeclared(unsigned ID) const {
  return ID == Builtin::NotBuiltin || ID == Builtin::BI__va_start ||
         ID == Builtin::BI__builtin_assume_aligned ||
         (!hasReferenceArgsOrResult(ID) && !hasCustomTypechecking(ID)) ||
         isInStdNamespace(ID);
}

bool Builtin::evaluateRequiredTargetFeatures(
    StringRef RequiredFeatures, const llvm::StringMap<bool> &TargetFetureMap) {
  // Return true if the builtin doesn't have any required features.
  if (RequiredFeatures.empty())
    return true;
  assert(!RequiredFeatures.contains(' ') && "Space in feature list");

  TargetFeatures TF(TargetFetureMap);
  return TF.hasRequiredFeatures(RequiredFeatures);
}

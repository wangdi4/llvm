//===--- SPIR.cpp - Implement SPIR target feature support -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements SPIR TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "SPIR.h"
#include "Targets.h"

#if INTEL_CUSTOMIZATION
#include "clang/Basic/TargetBuiltins.h"
#endif // INTEL_CUSTOMIZATION

using namespace clang;
using namespace clang::targets;

void SPIRTargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  DefineStd(Builder, "SPIR", Opts);
}

void SPIR32TargetInfo::getTargetDefines(const LangOptions &Opts,
                                        MacroBuilder &Builder) const {
  DefineStd(Builder, "SPIR32", Opts);
}

void SPIR64TargetInfo::getTargetDefines(const LangOptions &Opts,
                                        MacroBuilder &Builder) const {
  DefineStd(Builder, "SPIR64", Opts);
}

#if INTEL_CUSTOMIZATION
const Builtin::Info SPIR32INTELFpgaTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  { #ID, TYPE, ATTRS, nullptr, OCLC20_LANG, nullptr },
#include "clang/Basic/intel/BuiltinsSPIRINTELFpga.def"
};

ArrayRef<Builtin::Info>
SPIR32INTELFpgaTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo,
      clang::SPIRINTELFpga::LastTSBuiltin - Builtin::FirstTSBuiltin);
}

const Builtin::Info SPIR64INTELFpgaTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  { #ID, TYPE, ATTRS, nullptr, OCLC20_LANG, nullptr },
#include "clang/Basic/intel/BuiltinsSPIRINTELFpga.def"
};

ArrayRef<Builtin::Info>
SPIR64INTELFpgaTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo,
      clang::SPIRINTELFpga::LastTSBuiltin - Builtin::FirstTSBuiltin);
}
#endif // INTEL_CUSTOMIZATION

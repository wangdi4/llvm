//===--- SPIR.cpp - Implement SPIR target feature support -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
  SPIRTargetInfo::getTargetDefines(Opts, Builder);
  DefineStd(Builder, "SPIR32", Opts);
}

void SPIR64TargetInfo::getTargetDefines(const LangOptions &Opts,
                                        MacroBuilder &Builder) const {
  SPIRTargetInfo::getTargetDefines(Opts, Builder);
  DefineStd(Builder, "SPIR64", Opts);
}

#if INTEL_CUSTOMIZATION
const Builtin::Info SPIR32INTELFpgaTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  { #ID, TYPE, ATTRS, nullptr, ALL_OCLC_LANGUAGES, nullptr },
#define LANGBUILTIN(ID, TYPE, ATTRS, LANGS)                                    \
  { #ID, TYPE, ATTRS, nullptr, LANGS, nullptr },
#include "clang/Basic/intel/BuiltinsSPIRINTELFpga.def"
};

ArrayRef<Builtin::Info>
SPIR32INTELFpgaTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo,
      clang::SPIRINTELFpga::LastTSBuiltin - Builtin::FirstTSBuiltin);
}

const Builtin::Info SPIR64INTELFpgaTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  { #ID, TYPE, ATTRS, nullptr, ALL_OCLC_LANGUAGES, nullptr },
#define LANGBUILTIN(ID, TYPE, ATTRS, LANGS)                                    \
  { #ID, TYPE, ATTRS, nullptr, LANGS, nullptr },
#include "clang/Basic/intel/BuiltinsSPIRINTELFpga.def"
};

ArrayRef<Builtin::Info>
SPIR64INTELFpgaTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo,
      clang::SPIRINTELFpga::LastTSBuiltin - Builtin::FirstTSBuiltin);
}

static void defineFPGA(MacroBuilder &Builder) {
  Builder.defineMacro("__fpga_reg", "__builtin_fpga_reg");
}

void SPIR32INTELFpgaTargetInfo::getTargetDefines(
    const LangOptions &Opts, MacroBuilder &Builder) const {
  SPIR32TargetInfo::getTargetDefines(Opts, Builder);
  defineFPGA(Builder);
}

void SPIR64INTELFpgaTargetInfo::getTargetDefines(
    const LangOptions &Opts, MacroBuilder &Builder) const {
  SPIR64TargetInfo::getTargetDefines(Opts, Builder);
  defineFPGA(Builder);
}
#endif // INTEL_CUSTOMIZATION

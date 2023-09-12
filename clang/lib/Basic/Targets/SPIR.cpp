//===--- SPIR.cpp - Implement SPIR and SPIR-V target feature support ------===//
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
// This file implements SPIR and SPIR-V TargetInfo objects.
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

<<<<<<< HEAD
#if INTEL_CUSTOMIZATION
const Builtin::Info SPIR32INTELFpgaTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, HeaderDesc::NO_HEADER, ALL_OCL_LANGUAGES},
#define LANGBUILTIN(ID, TYPE, ATTRS, LANGS)                                    \
  {#ID, TYPE, ATTRS, nullptr, HeaderDesc::NO_HEADER, LANGS},
#include "clang/Basic/intel/BuiltinsSPIRINTELFpga.def"
};

ArrayRef<Builtin::Info>
SPIR32INTELFpgaTargetInfo::getTargetBuiltins() const {
  return llvm::ArrayRef(BuiltinInfo,
      clang::SPIRINTELFpga::LastTSBuiltin - Builtin::FirstTSBuiltin);
}

const Builtin::Info SPIR64INTELFpgaTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, HeaderDesc::NO_HEADER, ALL_OCL_LANGUAGES},
#define LANGBUILTIN(ID, TYPE, ATTRS, LANGS)                                    \
  {#ID, TYPE, ATTRS, nullptr, HeaderDesc::NO_HEADER, LANGS},
#include "clang/Basic/intel/BuiltinsSPIRINTELFpga.def"
};

ArrayRef<Builtin::Info>
SPIR64INTELFpgaTargetInfo::getTargetBuiltins() const {
  return llvm::ArrayRef(BuiltinInfo,
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

=======
void BaseSPIRVTargetInfo::getTargetDefines(const LangOptions &Opts,
                                           MacroBuilder &Builder) const {
  DefineStd(Builder, "SPIRV", Opts);
}

>>>>>>> 1b60fb4b3062da137b949fc85026a9c8b21b74eb
void SPIRVTargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  BaseSPIRVTargetInfo::getTargetDefines(Opts, Builder);
}

void SPIRV32TargetInfo::getTargetDefines(const LangOptions &Opts,
                                         MacroBuilder &Builder) const {
  BaseSPIRVTargetInfo::getTargetDefines(Opts, Builder);
  DefineStd(Builder, "SPIRV32", Opts);
}

void SPIRV64TargetInfo::getTargetDefines(const LangOptions &Opts,
                                         MacroBuilder &Builder) const {
  BaseSPIRVTargetInfo::getTargetDefines(Opts, Builder);
  DefineStd(Builder, "SPIRV64", Opts);
}

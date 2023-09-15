#if INTEL_FEATURE_CSA//===---------------------------------------*- C++ -*-===//
//===-- Intel_CSA.h - Declare CSA target feature support --------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_INTEL_CSA_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_INTEL_CSA_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
class StringRef;
} // namespace llvm

namespace clang {

class CSATargetInfo : public TargetInfo {
  static const Builtin::Info BuiltinInfo[];

  enum CSAKind {
    CSA_NONE,
    CSA_ORDERED,
    CSA_AUTOUNIT,
    CSA_AUTOMIN,
    CSA_CONFIG0,
    CSA_CONFIG1
  } CSA;

public:
  CSATargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : TargetInfo(Triple) {
    //
    // TODO (vzakhari 2/20/2018): we should support more host OSes eventually.
    //       See all possible OSes in llvm::Triple::*.
    //       The target setup below is specific to Linux Intel64 host.
    //
#if 0
    auto os = Triple.getOS();
    //
    // TODO (vzakhari 2/21/2018): figure out why bitcode emitter
    //       fails with "csa-intel-linux" triple and re-enable
    //       the assertion.
    //
    assert(os == llvm::Triple::Linux &&
           "CSA target is only supported for Linux host.");
#endif

    LongWidth = LongAlign = PointerWidth = PointerAlign = 64;
    LongDoubleFormat = &llvm::APFloat::x87DoubleExtended();
    LongDoubleWidth = 128;
    LongDoubleAlign = 128;
    LargeArrayMinWidth = 128;
    LargeArrayAlign = 128;
    SuitableAlign = 128;
    SizeType    = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType  = SignedLong;
    IntMaxType  = SignedLong;
    Int64Type   = SignedLong;
    HasFloat16  = true;

    // CSA supports atomics up to 8 bytes.
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 64;

    // Match lib/Target/Intel_CSA/CSASubtarget.cpp
    // Issue - does it need to match x86-64?
    resetDataLayout("e-m:e-i64:64-n32:64");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override {
    // CSA builds on X86.  We should really define everything for
    // the current x86 target, but this should get past initial include
    // file issues.
    Builder.defineMacro("__amd64__");
    Builder.defineMacro("__amd64");
    Builder.defineMacro("__x86_64");
    Builder.defineMacro("__x86_64__");

    Builder.defineMacro("__CSA__");
  }
  ArrayRef<Builtin::Info> getTargetBuiltins() const override {
    return llvm::makeArrayRef(BuiltinInfo,
                           clang::CSA::LastTSBuiltin-Builtin::FirstTSBuiltin);
  }
  ArrayRef<const char *> getGCCRegNames() const override {
    static const char * const GCCRegNames[] = { "dummy" };
    return llvm::makeArrayRef(GCCRegNames);
  }
  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return None;
  }
  bool validateAsmConstraint(const char *&Name,
                            TargetInfo::ConstraintInfo &Info) const override {
    // This is only used for validation on the front end.
    switch (*Name) {
      default:
        return false;
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'A':
      case 'B':
      case 'C':
      case 'D':
        Info.setAllowsRegister();
        return true;
    }
    return false;
  }

  const char *getClobbers() const override {
    return "";
  }
  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }
  bool setCPU(const std::string &Name) override {
    CSA = llvm::StringSwitch<CSAKind>(Name)
        .Case("ordered", CSA_ORDERED)
        .Case("autounit",CSA_AUTOUNIT)
        .Case("automin", CSA_AUTOMIN)
        .Case("config0", CSA_CONFIG0)
        .Case("config1", CSA_CONFIG1)
        .Default(CSA_NONE);
    return CSA != CSA_NONE;
  }
};

const Builtin::Info CSATargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS) \
  { #ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr },
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER) \
  { #ID, TYPE, ATTRS, HEADER, ALL_LANGUAGES, nullptr },
#include "clang/Basic/Intel_BuiltinsCSA.def"
};
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_INTEL_CSA_H
#endif // INTEL_FEATURE_CSA

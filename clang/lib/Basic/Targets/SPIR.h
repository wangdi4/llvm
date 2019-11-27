//===--- SPIR.h - Declare SPIR target feature support -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares SPIR TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_SPIR_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_SPIR_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/Compiler.h"
#include "OSTargets.h"

namespace clang {
namespace targets {

static const unsigned SPIRAddrSpaceMap[] = {
    0, // Default
    1, // opencl_global
    3, // opencl_local
    2, // opencl_constant
    0, // opencl_private
    4, // opencl_generic
    0, // cuda_device
    0, // cuda_constant
    0, // cuda_shared
    1, // sycl_global
    3, // sycl_local
    2, // sycl_constant
    0, // sycl_private
    4, // sycl_generic
};

static const unsigned SYCLAddrSpaceMap[] = {
    4, // Default
    1, // opencl_global
    3, // opencl_local
    2, // opencl_constant
    0, // opencl_private
    4, // opencl_generic
    0, // cuda_device
    0, // cuda_constant
    0, // cuda_shared
    1, // sycl_global
    3, // sycl_local
    2, // sycl_constant
    0, // sycl_private
    4, // sycl_generic
};

#if INTEL_COLLAB
static const unsigned SPIRAddrSpaceDefIsGenMap[] = {
    4, // Default
    1, // opencl_global
    3, // opencl_local
    2, // opencl_constant
    0, // opencl_private
    4, // opencl_generic
    0, // cuda_device
    0, // cuda_constant
    0, // cuda_shared
    1, // sycl_global
    3, // sycl_local
    2, // sycl_constant
    0, // sycl_private
    4, // sycl_generic
};
#endif // INTEL_COLLAB

class LLVM_LIBRARY_VISIBILITY SPIRTargetInfo : public TargetInfo {
#if INTEL_COLLAB
  bool UseAutoOpenCLAddrSpaceForOpenMP = false;
#endif  // INTEL_COLLAB
public:
  SPIRTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
#if INTEL_CUSTOMIZATION
    assert((getTriple().getEnvironment() == llvm::Triple::UnknownEnvironment ||
            getTriple().getEnvironment() == llvm::Triple::IntelFPGA ||
            getTriple().getEnvironment() == llvm::Triple::IntelEyeQ ||
            getTriple().getEnvironment() == llvm::Triple::SYCLDevice) &&
           "SPIR target must use unknown environment type");
#endif // INTEL_CUSTOMIZATION
    TLSSupported = false;
    VLASupported = false;
    LongWidth = LongAlign = 64;
    if (Triple.getEnvironment() == llvm::Triple::SYCLDevice &&
        !getenv("DISABLE_INFER_AS")) {
      AddrSpaceMap = &SYCLAddrSpaceMap;
    } else {
      AddrSpaceMap = &SPIRAddrSpaceMap;
    }
    UseAddrSpaceMapMangling = true;
    HasLegalHalfType = true;
    HasFloat16 = true;
    // Define available target features
    // These must be defined in sorted order!
    NoAsmVariants = true;
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

#if INTEL_COLLAB
  void adjust(LangOptions &Opts) override {
    TargetInfo::adjust(Opts);
    if (Opts.OpenMPLateOutline &&
        // FIXME: Temporarily quaery for ENABLE_INFER_AS environment variable.
        //        In the long term we should probably rely on
        //        UseAutoOpenCLAddrSpaceForOpenMP language option.
        //        The check for OpenMPLateOutline is also unnecessary.
        (Opts.UseAutoOpenCLAddrSpaceForOpenMP || getenv("ENABLE_INFER_AS"))) {
      // Use generic address space for all pointers except
      // globals and stack locals.
      Opts.UseAutoOpenCLAddrSpaceForOpenMP = true; // FIXME: remove this
      UseAutoOpenCLAddrSpaceForOpenMP = true;
      AddrSpaceMap = &SPIRAddrSpaceDefIsGenMap;
    }
  }

  llvm::Optional<LangAS> getConstantAddressSpace() const override {
    if (UseAutoOpenCLAddrSpaceForOpenMP)
      // Place constants into a global address space.
      return getLangASFromTargetAS(1);
    return LangAS::Default;
  }
#endif  // INTEL_COLLAB

  bool hasFeature(StringRef Feature) const override {
    return Feature == "spir";
  }

  // SPIR supports the half type and the only llvm intrinsic allowed in SPIR is
  // memcpy as per section 3 of the SPIR spec.
  bool useFP16ConversionIntrinsics() const override { return false; }

  ArrayRef<Builtin::Info> getTargetBuiltins() const override { return None; }

  const char *getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override { return None; }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override {
    return true;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return None;
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override {
    return (CC == CC_SpirFunction || CC == CC_OpenCLKernel) ? CCCR_OK
                                                            : CCCR_Warning;
  }

  CallingConv getDefaultCallingConv() const override {
    return CC_SpirFunction;
  }

  void setSupportedOpenCLOpts() override {
    // Assume all OpenCL extensions and optional core features are supported
    // for SPIR since it is a generic target.
    getSupportedOpenCLOpts().supportAll();
  }
};
class LLVM_LIBRARY_VISIBILITY SPIR32TargetInfo : public SPIRTargetInfo {
public:
  SPIR32TargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : SPIRTargetInfo(Triple, Opts) {
    PointerWidth = PointerAlign = 32;
    SizeType = TargetInfo::UnsignedInt;
    PtrDiffType = IntPtrType = TargetInfo::SignedInt;
    resetDataLayout("e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-"
                    "v96:128-v192:256-v256:256-v512:512-v1024:1024");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};

class LLVM_LIBRARY_VISIBILITY SPIR64TargetInfo : public SPIRTargetInfo {
public:
  SPIR64TargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : SPIRTargetInfo(Triple, Opts) {
    PointerWidth = PointerAlign = 64;
    SizeType = TargetInfo::UnsignedLong;
    PtrDiffType = IntPtrType = TargetInfo::SignedLong;

    resetDataLayout("e-i64:64-v16:16-v24:32-v32:32-v48:64-"
                    "v96:128-v192:256-v256:256-v512:512-v1024:1024");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};

class LLVM_LIBRARY_VISIBILITY SPIR32SYCLDeviceTargetInfo
    : public SPIR32TargetInfo {
public:
  SPIR32SYCLDeviceTargetInfo(const llvm::Triple &Triple,
                             const TargetOptions &Opts)
      : SPIR32TargetInfo(Triple, Opts) {
    // This is workaround for exception_ptr class.
    // Exceptions is not allowed in sycl device code but we should be able
    // to parse host code. So we allow compilation of exception_ptr but
    // if exceptions are used in device code we should emit a diagnostic.
    MaxAtomicInlineWidth = 32;
  }
};

class LLVM_LIBRARY_VISIBILITY SPIR64SYCLDeviceTargetInfo
    : public SPIR64TargetInfo {
public:
  SPIR64SYCLDeviceTargetInfo(const llvm::Triple &Triple,
                             const TargetOptions &Opts)
      : SPIR64TargetInfo(Triple, Opts) {
    // This is workaround for exception_ptr class.
    // Exceptions is not allowed in sycl device code but we should be able
    // to parse host code. So we allow compilation of exception_ptr but
    // if exceptions are used in device code we should emit a diagnostic.
    MaxAtomicInlineWidth = 64;
  }
};

#if INTEL_CUSTOMIZATION
class LLVM_LIBRARY_VISIBILITY SPIR32INTELFpgaTargetInfo
    : public SPIR32TargetInfo {
  static const Builtin::Info BuiltinInfo[];
public:
  SPIR32INTELFpgaTargetInfo(const llvm::Triple &Triple,
                            const TargetOptions &Opts)
      : SPIR32TargetInfo(Triple, Opts) {}
  ArrayRef<Builtin::Info> getTargetBuiltins() const override;
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};

class LLVM_LIBRARY_VISIBILITY SPIR64INTELFpgaTargetInfo
    : public SPIR64TargetInfo {
  static const Builtin::Info BuiltinInfo[];
public:
  SPIR64INTELFpgaTargetInfo(const llvm::Triple &Triple,
                            const TargetOptions &Opts)
      : SPIR64TargetInfo(Triple, Opts) {}
  ArrayRef<Builtin::Info> getTargetBuiltins() const override;
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};
#endif // INTEL_CUSTOMIZATION

// x86-32 SPIR Windows target
class LLVM_LIBRARY_VISIBILITY WindowsX86_32SPIRTargetInfo
    : public WindowsTargetInfo<SPIR32SYCLDeviceTargetInfo> {
public:
  WindowsX86_32SPIRTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : WindowsTargetInfo<SPIR32SYCLDeviceTargetInfo>(Triple, Opts) {
    DoubleAlign = LongLongAlign = 64;
    WCharType = UnsignedShort;
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::CharPtrBuiltinVaList;
  }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override {
    if (CC == CC_X86VectorCall)
      // Permit CC_X86VectorCall which is used in Microsoft headers
      return CCCR_OK;
    return (CC == CC_SpirFunction || CC == CC_OpenCLKernel) ? CCCR_OK
                                    : CCCR_Warning;
  }
};

// x86-32 SPIR Windows Visual Studio target
class LLVM_LIBRARY_VISIBILITY MicrosoftX86_32SPIRTargetInfo
    : public WindowsX86_32SPIRTargetInfo {
public:
  MicrosoftX86_32SPIRTargetInfo(const llvm::Triple &Triple,
                            const TargetOptions &Opts)
      : WindowsX86_32SPIRTargetInfo(Triple, Opts) {
    LongDoubleWidth = LongDoubleAlign = 64;
    LongDoubleFormat = &llvm::APFloat::IEEEdouble();
#if INTEL_COLLAB
    if (Triple.getEnvironment() != llvm::Triple::SYCLDevice)
      // Set Microsoft ABI in non-SYCL targetInfo compilations
      TheCXXABI.set(TargetCXXABI::Microsoft);
#endif  // INTEL_COLLAB
    assert(DataLayout->getPointerSizeInBits() == 32);
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override {
    WindowsX86_32SPIRTargetInfo::getTargetDefines(Opts, Builder);
    // The value of the following reflects processor type.
    // 300=386, 400=486, 500=Pentium, 600=Blend (default)
    // We lost the original triple, so we use the default.
    // TBD should we keep these lines?  Copied from X86.h.
    Builder.defineMacro("_M_IX86", "600");
  }

#if INTEL_CUSTOMIZATION
  virtual bool shouldDiagnoseVariadicCall(void) const {
    return false;
  }
#endif // INTEL_CUSTOMIZATION
};

// x86-64 SPIR64 Windows target
class LLVM_LIBRARY_VISIBILITY WindowsX86_64_SPIR64TargetInfo
    : public WindowsTargetInfo<SPIR64SYCLDeviceTargetInfo> {
public:
  WindowsX86_64_SPIR64TargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : WindowsTargetInfo<SPIR64SYCLDeviceTargetInfo>(Triple, Opts) {
    LongWidth = LongAlign = 32;
    DoubleAlign = LongLongAlign = 64;
    IntMaxType = SignedLongLong;
    Int64Type = SignedLongLong;
    SizeType = UnsignedLongLong;
    PtrDiffType = SignedLongLong;
    IntPtrType = SignedLongLong;
    WCharType = UnsignedShort;
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::CharPtrBuiltinVaList;
  }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override {
    if (CC == CC_X86VectorCall)
      // Permit CC_X86VectorCall which is used in Microsoft headers
      return CCCR_OK;
    return (CC == CC_SpirFunction || CC == CC_OpenCLKernel) ? CCCR_OK
                                    : CCCR_Warning;
  }
};

// x86-64 SPIR64 Windows Visual Studio target
class LLVM_LIBRARY_VISIBILITY MicrosoftX86_64_SPIR64TargetInfo
    : public WindowsX86_64_SPIR64TargetInfo {
public:
  MicrosoftX86_64_SPIR64TargetInfo(const llvm::Triple &Triple,
                            const TargetOptions &Opts)
      : WindowsX86_64_SPIR64TargetInfo(Triple, Opts) {
    LongDoubleWidth = LongDoubleAlign = 64;
    LongDoubleFormat = &llvm::APFloat::IEEEdouble();
#if INTEL_COLLAB
    if (Triple.getEnvironment() != llvm::Triple::SYCLDevice)
      // Set Microsoft ABI in non-SYCL targetInfo compilations
      TheCXXABI.set(TargetCXXABI::Microsoft);
#endif  // INTEL_COLLAB
    assert(DataLayout->getPointerSizeInBits() == 64);
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override {
    WindowsX86_64_SPIR64TargetInfo::getTargetDefines(Opts, Builder);
    Builder.defineMacro("_M_X64", "100");
    Builder.defineMacro("_M_AMD64", "100");
  }

#if INTEL_CUSTOMIZATION
  virtual bool shouldDiagnoseVariadicCall(void) const {
    return false;
  }
#endif // INTEL_CUSTOMIZATION
};

} // namespace targets
} // namespace clang
#endif // LLVM_CLANG_LIB_BASIC_TARGETS_SPIR_H

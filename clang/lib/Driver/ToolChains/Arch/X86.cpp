//===--- X86.cpp - X86 Helpers for Tools ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "ToolChains/CommonArgs.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/Host.h"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

#if INTEL_CUSTOMIZATION
const char *getCPUForIntel(StringRef Arch, const llvm::Triple &Triple,
                           bool IsArchOpt = false) {
  const char *CPU = nullptr;
  if (Triple.getArch() == llvm::Triple::x86 && !IsArchOpt) { // 32-bit-only
    CPU = llvm::StringSwitch<const char *>(Arch)
              .Case("A", "pentium")
              .CaseLower("sse", "pentium3")
              .CaseLower("sse2", "pentium4")
              .Default(nullptr);
  }
  if (CPU == nullptr) { // 32-bit and 64-bit
    CPU = llvm::StringSwitch<const char *>(Arch)
              .CaseLower("sse3", "nocona")
              .CaseLower("ssse3", "core2")
              .CaseLower("sse4.1", "penryn")
              .CaseLower("sse4.2", "corei7")
              .CaseLower("sandybridge", "corei7-avx")
              .CasesLower("core-avx2", "core_avx2", "haswell", "core-avx2")
              .CasesLower("core-avx-i", "core_avx_i", "ivybridge", "core-avx-i")
              .CasesLower("atom-ssse3", "atom_ssse3", "atom")
              .CasesLower("atom-sse4.2", "atom_sse4.2", "silvermont",
                          "silvermont")
              .CaseLower("goldmont", "goldmont")
              .CasesLower("goldmont-plus", "goldmont_plus", "goldmont-plus")
              .CaseLower("tremont", "tremont")
              .CasesLower("mic-avx512", "mic_avx512", "knl", "knl")
              .CaseLower("knm", "knm")
              .CasesLower("skylake", "kabylake", "coffeelake", "skylake")
              .CasesLower("amberlake", "whiskeylake", "skylake")
              .CasesLower("core-avx512", "core_avx512", "skylake-avx512",
                          "skylake_avx512", "skylake-avx512")
              .CasesLower("common-avx512", "common_avx512", "common-avx512")
              .CaseLower("broadwell", "broadwell")
              .CaseLower("cannonlake", "cannonlake")
              .CasesLower("icelake", "icelake-client", "icelake_client",
                          "icelake-client")
              .CasesLower("icelake-server", "icelake_server", "icelake-server")
              .CaseLower("cascadelake", "cascadelake")
              .CaseLower("tigerlake", "tigerlake")
              .CasesLower("sapphirerapids", "sapphire-rapids",
                          "sapphire_rapids", "sapphirerapids")
              .CaseLower("host", llvm::sys::getHostCPUName().data())
              .Default(nullptr);
  }
  // We check for valid /arch and /Qx values, so overlap values are covered
  // here.
  if (CPU == nullptr && !IsArchOpt) {
    CPU = llvm::StringSwitch<const char *>(Arch)
              .CaseLower("avx", "corei7-avx")
              .Default(nullptr);
  }
  if (!CPU) {
    // No match found.  Instead of erroring out with a bad language type, we
    // will pass the arg to the compiler to validate.
    if (!IsArchOpt && !types::lookupTypeForTypeSpecifier(Arch.data()))
      CPU = Arch.data();
  }
  return CPU;
}

bool x86::isValidIntelCPU(StringRef CPU, const llvm::Triple &Triple) {
  return getCPUForIntel(CPU, Triple) != nullptr;
}
#endif // INTEL_CUSTOMIZATION

const char *x86::getX86TargetCPU(const ArgList &Args,
                                 const llvm::Triple &Triple) {
#if INTEL_CUSTOMIZATION
  if (const Arg *A = Args.getLastArg(options::OPT_march_EQ, options::OPT_x)) {
    if (A->getOption().matches(options::OPT_x)) {
      // -x<code> handling for Intel Processors.
      StringRef Arch = A->getValue();
      const char *CPU = nullptr;
      CPU = getCPUForIntel(Arch, Triple);
      if (CPU)
        return CPU;
    }
  }
#endif // INTEL_CUSTOMIZATION
  if (const Arg *A = Args.getLastArg(options::OPT_march_EQ)) {
    if (StringRef(A->getValue()) != "native")
      return A->getValue();

    // FIXME: Reject attempts to use -march=native unless the target matches
    // the host.
    //
    // FIXME: We should also incorporate the detected target features for use
    // with -native.
    std::string CPU = std::string(llvm::sys::getHostCPUName());
    if (!CPU.empty() && CPU != "generic")
      return Args.MakeArgString(CPU);
  }

#if INTEL_CUSTOMIZATION
  if (const Arg *A = Args.getLastArgNoClaim(options::OPT__SLASH_arch,
                                            options::OPT__SLASH_Qx)) {
    if (A->getOption().matches(options::OPT__SLASH_Qx)) {
      // /Qx<code> handling for Intel Processors.
      StringRef Arch = A->getValue();
      const char *CPU = nullptr;
      CPU = getCPUForIntel(Arch, Triple);
      if (CPU) {
        A->claim();
        return CPU;
      }
    }
  }
#endif // INTEL_CUSTOMIZATION
  if (const Arg *A = Args.getLastArgNoClaim(options::OPT__SLASH_arch)) {
    // Mapping built by looking at lib/Basic's X86TargetInfo::initFeatureMap().
    StringRef Arch = A->getValue();
    const char *CPU = nullptr;
    if (Triple.getArch() == llvm::Triple::x86) {  // 32-bit-only /arch: flags.
      CPU = llvm::StringSwitch<const char *>(Arch)
                .Case("IA32", "i386")
                .Case("SSE", "pentium3")
                .Case("SSE2", "pentium4")
                .Default(nullptr);
    }
    if (CPU == nullptr) {  // 32-bit and 64-bit /arch: flags.
      CPU = llvm::StringSwitch<const char *>(Arch)
                .Case("AVX", "sandybridge")
                .Case("AVX2", "haswell")
                .Case("AVX512F", "knl")
                .Case("AVX512", "skylake-avx512")
                .Default(nullptr);
    }
#if INTEL_CUSTOMIZATION
    // Handle 'other' /arch variations that are allowed for icx/Intel
    if (CPU == nullptr)
      CPU = getCPUForIntel(Arch, Triple, true);
#endif // INTEL_CUSTOMIZATION
    if (CPU) {
      A->claim();
      return CPU;
    }
  }

  // Select the default CPU if none was given (or detection failed).

  if (!Triple.isX86())
    return nullptr; // This routine is only handling x86 targets.

  bool Is64Bit = Triple.getArch() == llvm::Triple::x86_64;

  // FIXME: Need target hooks.
  if (Triple.isOSDarwin()) {
    if (Triple.getArchName() == "x86_64h")
      return "core-avx2";
    // macosx10.12 drops support for all pre-Penryn Macs.
    // Simulators can still run on 10.11 though, like Xcode.
    if (Triple.isMacOSX() && !Triple.isOSVersionLT(10, 12))
      return "penryn";
    // The oldest x86_64 Macs have core2/Merom; the oldest x86 Macs have Yonah.
    return Is64Bit ? "core2" : "yonah";
  }

  // Set up default CPU name for PS4 compilers.
  if (Triple.isPS4CPU())
    return "btver2";

  // On Android use targets compatible with gcc
  if (Triple.isAndroid())
    return Is64Bit ? "x86-64" : "i686";

  // Everything else goes to x86-64 in 64-bit mode.
  if (Is64Bit)
    return "x86-64";

  switch (Triple.getOS()) {
  case llvm::Triple::FreeBSD:
  case llvm::Triple::NetBSD:
  case llvm::Triple::OpenBSD:
    return "i486";
  case llvm::Triple::Haiku:
    return "i586";
  default:
    // Fallback to p4.
    return "pentium4";
  }
}

void x86::getX86TargetFeatures(const Driver &D, const llvm::Triple &Triple,
                               const ArgList &Args,
                               std::vector<StringRef> &Features) {
  // If -march=native, autodetect the feature list.
  if (const Arg *A = Args.getLastArg(clang::driver::options::OPT_march_EQ)) {
    if (StringRef(A->getValue()) == "native") {
      llvm::StringMap<bool> HostFeatures;
      if (llvm::sys::getHostCPUFeatures(HostFeatures))
        for (auto &F : HostFeatures)
          Features.push_back(
              Args.MakeArgString((F.second ? "+" : "-") + F.first()));
    }
  }

  if (Triple.getArchName() == "x86_64h") {
    // x86_64h implies quite a few of the more modern subtarget features
    // for Haswell class CPUs, but not all of them. Opt-out of a few.
    Features.push_back("-rdrnd");
    Features.push_back("-aes");
    Features.push_back("-pclmul");
    Features.push_back("-rtm");
    Features.push_back("-fsgsbase");
  }

  const llvm::Triple::ArchType ArchType = Triple.getArch();
  // Add features to be compatible with gcc for Android.
  if (Triple.isAndroid()) {
    if (ArchType == llvm::Triple::x86_64) {
      Features.push_back("+sse4.2");
      Features.push_back("+popcnt");
      Features.push_back("+cx16");
    } else
      Features.push_back("+ssse3");
  }

  // Translate the high level `-mretpoline` flag to the specific target feature
  // flags. We also detect if the user asked for retpoline external thunks but
  // failed to ask for retpolines themselves (through any of the different
  // flags). This is a bit hacky but keeps existing usages working. We should
  // consider deprecating this and instead warn if the user requests external
  // retpoline thunks and *doesn't* request some form of retpolines.
  auto SpectreOpt = clang::driver::options::ID::OPT_INVALID;
  if (Args.hasArgNoClaim(options::OPT_mretpoline, options::OPT_mno_retpoline,
                         options::OPT_mspeculative_load_hardening,
                         options::OPT_mno_speculative_load_hardening)) {
    if (Args.hasFlag(options::OPT_mretpoline, options::OPT_mno_retpoline,
                     false)) {
      Features.push_back("+retpoline-indirect-calls");
      Features.push_back("+retpoline-indirect-branches");
      SpectreOpt = options::OPT_mretpoline;
    } else if (Args.hasFlag(options::OPT_mspeculative_load_hardening,
                            options::OPT_mno_speculative_load_hardening,
                            false)) {
      // On x86, speculative load hardening relies on at least using retpolines
      // for indirect calls.
      Features.push_back("+retpoline-indirect-calls");
      SpectreOpt = options::OPT_mspeculative_load_hardening;
    }
  } else if (Args.hasFlag(options::OPT_mretpoline_external_thunk,
                          options::OPT_mno_retpoline_external_thunk, false)) {
    // FIXME: Add a warning about failing to specify `-mretpoline` and
    // eventually switch to an error here.
    Features.push_back("+retpoline-indirect-calls");
    Features.push_back("+retpoline-indirect-branches");
    SpectreOpt = options::OPT_mretpoline_external_thunk;
  }

  auto LVIOpt = clang::driver::options::ID::OPT_INVALID;
  if (Args.hasFlag(options::OPT_mlvi_hardening, options::OPT_mno_lvi_hardening,
                   false)) {
    Features.push_back("+lvi-load-hardening");
    Features.push_back("+lvi-cfi"); // load hardening implies CFI protection
    LVIOpt = options::OPT_mlvi_hardening;
  } else if (Args.hasFlag(options::OPT_mlvi_cfi, options::OPT_mno_lvi_cfi,
                          false)) {
    Features.push_back("+lvi-cfi");
    LVIOpt = options::OPT_mlvi_cfi;
  }

  if (SpectreOpt != clang::driver::options::ID::OPT_INVALID &&
      LVIOpt != clang::driver::options::ID::OPT_INVALID) {
    D.Diag(diag::err_drv_argument_not_allowed_with)
        << D.getOpts().getOptionName(SpectreOpt)
        << D.getOpts().getOptionName(LVIOpt);
  }

  // Now add any that the user explicitly requested on the command line,
  // which may override the defaults.
  handleTargetFeaturesGroup(Args, Features, options::OPT_m_x86_Features_Group);
}

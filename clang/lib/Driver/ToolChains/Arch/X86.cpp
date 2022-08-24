//===--- X86.cpp - X86 Helpers for Tools ------------------------*- C++ -*-===//
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

#include "X86.h"
#include "ToolChains/CommonArgs.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/Host.h"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

#if INTEL_CUSTOMIZATION
std::string x86::getCPUForIntel(StringRef Arch, const llvm::Triple &Triple,
                                bool IsArchOpt) {
  StringRef CPU;
  if (Triple.getArch() == llvm::Triple::x86 && !IsArchOpt) { // 32-bit-only
    CPU = llvm::StringSwitch<StringRef>(Arch)
              .Case("A", "pentium")
              .CaseLower("sse", "pentium3")
              .CaseLower("sse2", "pentium4")
              .Default("");
  }
  if (CPU.empty()) { // 32-bit and 64-bit
    CPU = llvm::StringSwitch<StringRef>(Arch)
              .CaseLower("sse3", "nocona")
              .CaseLower("ssse3", "core2")
              .CaseLower("sse4.1", "penryn")
              .CaseLower("sse4.2", "corei7")
              .CaseLower("sandybridge", "corei7-avx")
              .CasesLower("core-avx2", "core_avx2", "haswell", "avx2",
                          "core-avx2")
              .CasesLower("core-avx-i", "core_avx_i", "ivybridge", "core-avx-i")
              .CasesLower("atom-ssse3", "atom_ssse3", "atom")
              .CasesLower("atom-sse4.2", "atom_sse4.2", "silvermont",
                          "silvermont")
              .CaseLower("goldmont", "goldmont")
              .CasesLower("goldmont-plus", "goldmont_plus", "goldmont-plus")
              .CaseLower("tremont", "tremont")
              .CasesLower("skylake", "kabylake", "coffeelake", "skylake")
              .CasesLower("amberlake", "whiskeylake", "skylake")
              .CasesLower("core-avx512", "core_avx512", "skylake-avx512",
                          "skylake_avx512", "skylake-avx512")
              .CasesLower("common-avx512", "common_avx512", "common-avx512")
#if INTEL_FEATURE_ISA_AVX256
              .CasesLower("common-avx256", "common_avx256", "common-avx256")
#endif // INTEL_FEATURE_ISA_AVX256
              .CaseLower("broadwell", "broadwell")
              .CaseLower("cannonlake", "cannonlake")
              .CasesLower("icelake", "icelake-client", "icelake_client",
                          "icelake-client")
              .CasesLower("icelake-server", "icelake_server", "icelake-server")
              .CaseLower("cascadelake", "cascadelake")
              .CaseLower("cooperlake", "cooperlake")
              .CaseLower("tigerlake", "tigerlake")
              .CasesLower("sapphirerapids", "sapphire-rapids",
                          "sapphire_rapids", "sapphirerapids")
              .CaseLower("alderlake", "alderlake")
              .CaseLower("rocketlake", "rocketlake")
#if INTEL_FEATURE_CPU_RPL
              .CaseLower("raptorlake", "raptorlake")
#endif // INTEL_FEATURE_CPU_RPL
#if INTEL_FEATURE_CPU_GNR
              .CaseLower("graniterapids", "graniterapids")
#endif // INTEL_FEATURE_CPU_GNR
#if INTEL_FEATURE_CPU_DMR
              .CaseLower("diamondrapids", "diamondrapids")
#endif // INTEL_FEATURE_CPU_DMR
              .CaseLower("host", llvm::sys::getHostCPUName())
              .Default("");
  }
  // We check for valid /arch and /Qx values, so overlap values are covered
  // here.
  if (CPU.empty() && !IsArchOpt) {
    CPU = llvm::StringSwitch<StringRef>(Arch)
              .CaseLower("avx", "corei7-avx")
              .Default("");
  }
  if (CPU.empty()) {
    // No match found.  Instead of erroring out with a bad language type, we
    // will pass the arg to the compiler to validate.
    if (!IsArchOpt && !types::lookupTypeForTypeSpecifier(Arch.data()))
      CPU = Arch;
  }
  return std::string(CPU);
}

bool x86::isValidIntelCPU(StringRef CPU, const llvm::Triple &Triple) {
  return !getCPUForIntel(CPU, Triple).empty();
}
#endif // INTEL_CUSTOMIZATION

std::string x86::getX86TargetCPU(const Driver &D, const ArgList &Args,
                                 const llvm::Triple &Triple) {
#if INTEL_CUSTOMIZATION
  auto ArchOverrideCheck = [&Args, &D](OptSpecifier Opt1, OptSpecifier Opt2) {
    Arg *Previous = nullptr;
    for (Arg *A : Args.filtered(Opt1, Opt2)) {
      bool IsSourceTypeOpt = A->getOption().matches(options::OPT_x) &&
          types::lookupTypeForTypeSpecifier(A->getValue());
      if (Previous && !IsSourceTypeOpt &&
          A->getAsString(Args) != Previous->getAsString(Args))
        D.Diag(clang::diag::warn_drv_overriding_flag_option)
            << Previous->getAsString(Args) << A->getAsString(Args);
      // Only capture the -x option if it is for arch setting
      if (IsSourceTypeOpt)
        continue;
      Previous = A;
    }
  };
  if (const Arg *A = clang::driver::getLastArchArg(Args)) {
    ArchOverrideCheck(options::OPT_x, options::OPT_march_EQ);
    if (A->getOption().matches(options::OPT_x)) {
      // -x<code> handling for Intel Processors.
      StringRef Arch = A->getValue();
      std::string CPU = getCPUForIntel(Arch, Triple);
      if (!CPU.empty())
        return CPU;
    }
  }
#endif // INTEL_CUSTOMIZATION
  if (const Arg *A = Args.getLastArg(clang::driver::options::OPT_march_EQ)) {
    StringRef CPU = A->getValue();
    if (CPU != "native")
      return std::string(CPU);

    // FIXME: Reject attempts to use -march=native unless the target matches
    // the host.
    //
    // FIXME: We should also incorporate the detected target features for use
    // with -native.
    CPU = llvm::sys::getHostCPUName();
    if (!CPU.empty() && CPU != "generic")
      return std::string(CPU);
  }

#if INTEL_CUSTOMIZATION
  if (const Arg *A = Args.getLastArgNoClaim(options::OPT__SLASH_arch,
                                            options::OPT__SLASH_Qx)) {
    ArchOverrideCheck(options::OPT__SLASH_arch, options::OPT__SLASH_Qx);
    if (A->getOption().matches(options::OPT__SLASH_Qx)) {
      // /Qx<code> handling for Intel Processors.
      StringRef Arch = A->getValue();
      std::string CPU = getCPUForIntel(Arch, Triple);
      if (!CPU.empty()) {
        A->claim();
        return CPU;
      }
    }
  }
#endif // INTEL_CUSTOMIZATION
  if (const Arg *A = Args.getLastArg(options::OPT__SLASH_arch)) {
    // Mapping built by looking at lib/Basic's X86TargetInfo::initFeatureMap().
    // The keys are case-sensitive; this matches link.exe.
    // 32-bit and 64-bit /arch: flags.
    llvm::StringMap<StringRef> ArchMap({
        {"AVX", "sandybridge"},
        {"AVX2", "haswell"},
#if !INTEL_CUSTOMIZATION
        {"AVX512F", "knl"},
#endif // !INTEL_CUSTOMIZATION
        {"AVX512", "skylake-avx512"},
    });
    if (Triple.getArch() == llvm::Triple::x86) {
      // 32-bit-only /arch: flags.
      ArchMap.insert({
          {"IA32", "i386"},
          {"SSE", "pentium3"},
          {"SSE2", "pentium4"},
      });
    }
    StringRef CPU = ArchMap.lookup(A->getValue());
    if (CPU.empty()) {
#if INTEL_CUSTOMIZATION
      // Handle 'other' /arch variations that are allowed for icx/Intel
      std::string IntelCPU = getCPUForIntel(A->getValue(), Triple, true);
      if (!IntelCPU.empty()) {
        A->claim();
        return IntelCPU;
      }
#endif // INTEL_CUSTOMIZATION
      std::vector<StringRef> ValidArchs{ArchMap.keys().begin(),
                                        ArchMap.keys().end()};
      sort(ValidArchs);
      D.Diag(diag::warn_drv_invalid_arch_name_with_suggestion)
          << A->getValue() << (Triple.getArch() == llvm::Triple::x86)
          << join(ValidArchs, ", ");
    }
    return std::string(CPU);
  }

  // Select the default CPU if none was given (or detection failed).

  if (!Triple.isX86())
    return ""; // This routine is only handling x86 targets.

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_XUCC
  bool Is64Bit = (Triple.getArch() == llvm::Triple::x86_64 ||
                  Triple.getArch() == llvm::Triple::x86_64_xucc);
#else // INTEL_FEATURE_XUCC
  bool Is64Bit = Triple.getArch() == llvm::Triple::x86_64;
#endif // INTEL_FEATURE_XUCC
#endif // INTEL_CUSTOMIZATION

  // FIXME: Need target hooks.
  if (Triple.isOSDarwin()) {
    if (Triple.getArchName() == "x86_64h")
      return "core-avx2";
    // macosx10.12 drops support for all pre-Penryn Macs.
    // Simulators can still run on 10.11 though, like Xcode.
    if (Triple.isMacOSX() && !Triple.isOSVersionLT(10, 12))
      return "penryn";

    if (Triple.isDriverKit())
      return "nehalem";

    // The oldest x86_64 Macs have core2/Merom; the oldest x86 Macs have Yonah.
    return Is64Bit ? "core2" : "yonah";
  }

  // Set up default CPU name for PS4/PS5 compilers.
  if (Triple.isPS4())
    return "btver2";
  if (Triple.isPS5())
    return "znver2";

  // On Android use targets compatible with gcc
  if (Triple.isAndroid())
    return Is64Bit ? "x86-64" : "i686";

  // Everything else goes to x86-64 in 64-bit mode.
  if (Is64Bit)
    return "x86-64";

  switch (Triple.getOS()) {
  case llvm::Triple::NetBSD:
    return "i486";
  case llvm::Triple::Haiku:
  case llvm::Triple::OpenBSD:
    return "i586";
  case llvm::Triple::FreeBSD:
    return "i686";
  default:
    // Fallback to p4.
    return "pentium4";
  }
}

void x86::getX86TargetFeatures(const Driver &D, const llvm::Triple &Triple,
                               const ArgList &Args,
                               std::vector<StringRef> &Features) {
  // If -march=native, autodetect the feature list.
#if INTEL_CUSTOMIZATION
  // if -xHost/QxHost, autodetect the feature list.
  if (const Arg *A =
        D.IsCLMode() ? Args.getLastArg(options::OPT_march_EQ,
                                       options::OPT__SLASH_Qx)
                     : clang::driver::getLastArchArg(Args)) {
    if ((A->getOption().matches(options::OPT_march_EQ) &&
         (StringRef(A->getValue()) == "native")) ||
        ((A->getOption().matches(options::OPT_x) ||
          A->getOption().matches(options::OPT__SLASH_Qx)) &&
         ((StringRef(A->getValue())).lower() == "host"))) {
#endif // INTEL_CUSTOMIZATION
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

  if (Args.hasFlag(options::OPT_m_seses, options::OPT_mno_seses, false)) {
    if (LVIOpt == options::OPT_mlvi_hardening)
      D.Diag(diag::err_drv_argument_not_allowed_with)
          << D.getOpts().getOptionName(options::OPT_mlvi_hardening)
          << D.getOpts().getOptionName(options::OPT_m_seses);

    if (SpectreOpt != clang::driver::options::ID::OPT_INVALID)
      D.Diag(diag::err_drv_argument_not_allowed_with)
          << D.getOpts().getOptionName(SpectreOpt)
          << D.getOpts().getOptionName(options::OPT_m_seses);

    Features.push_back("+seses");
    if (!Args.hasArg(options::OPT_mno_lvi_cfi)) {
      Features.push_back("+lvi-cfi");
      LVIOpt = options::OPT_mlvi_cfi;
    }
  }

  if (SpectreOpt != clang::driver::options::ID::OPT_INVALID &&
      LVIOpt != clang::driver::options::ID::OPT_INVALID) {
    D.Diag(diag::err_drv_argument_not_allowed_with)
        << D.getOpts().getOptionName(SpectreOpt)
        << D.getOpts().getOptionName(LVIOpt);
  }

  // Now add any that the user explicitly requested on the command line,
  // which may override the defaults.
  for (const Arg *A : Args.filtered(options::OPT_m_x86_Features_Group,
                                    options::OPT_mgeneral_regs_only)) {
    StringRef Name = A->getOption().getName();
    A->claim();

    // Skip over "-m".
    assert(Name.startswith("m") && "Invalid feature name.");
    Name = Name.substr(1);

    // Replace -mgeneral-regs-only with -x87, -mmx, -sse
    if (A->getOption().getID() == options::OPT_mgeneral_regs_only) {
      Features.insert(Features.end(), {"-x87", "-mmx", "-sse"});
      continue;
    }

    bool IsNegative = Name.startswith("no-");
    if (IsNegative)
      Name = Name.substr(3);
    Features.push_back(Args.MakeArgString((IsNegative ? "-" : "+") + Name));
  }

  // Enable/disable straight line speculation hardening.
  if (Arg *A = Args.getLastArg(options::OPT_mharden_sls_EQ)) {
    StringRef Scope = A->getValue();
    if (Scope == "all") {
      Features.push_back("+harden-sls-ijmp");
      Features.push_back("+harden-sls-ret");
    } else if (Scope == "return") {
      Features.push_back("+harden-sls-ret");
    } else if (Scope == "indirect-jmp") {
      Features.push_back("+harden-sls-ijmp");
    } else if (Scope != "none") {
      D.Diag(diag::err_drv_unsupported_option_argument)
          << A->getOption().getName() << Scope;
    }
  }
}

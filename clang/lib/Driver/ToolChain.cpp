//===- ToolChain.cpp - Collections of tools for one platform --------------===//
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

#include "clang/Driver/ToolChain.h"
#include "ToolChains/Arch/ARM.h"
#include "ToolChains/Clang.h"
#include "ToolChains/Flang.h"
#include "ToolChains/InterfaceStubs.h"
#include "clang/Basic/ObjCRuntime.h"
#include "clang/Basic/Sanitizers.h"
#include "clang/Config/config.h"
#include "clang/Driver/Action.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/InputInfo.h"
#include "clang/Driver/Job.h"
#include "clang/Driver/Options.h"
#include "clang/Driver/SanitizerArgs.h"
#include "clang/Driver/XRayArgs.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Option/Arg.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetParser.h"
#include "llvm/Support/VersionTuple.h"
#include "llvm/Support/VirtualFileSystem.h"
#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>

using namespace clang;
using namespace driver;
using namespace tools;
using namespace llvm;
using namespace llvm::opt;

static llvm::opt::Arg *GetRTTIArgument(const ArgList &Args) {
  return Args.getLastArg(options::OPT_mkernel, options::OPT_fapple_kext,
                         options::OPT_fno_rtti, options::OPT_frtti);
}

static ToolChain::RTTIMode CalculateRTTIMode(const ArgList &Args,
                                             const llvm::Triple &Triple,
                                             const Arg *CachedRTTIArg) {
  // Explicit rtti/no-rtti args
  if (CachedRTTIArg) {
    if (CachedRTTIArg->getOption().matches(options::OPT_frtti))
      return ToolChain::RM_Enabled;
    else
      return ToolChain::RM_Disabled;
  }

  // -frtti is default, except for the PS4.
  return (Triple.isPS4()) ? ToolChain::RM_Disabled : ToolChain::RM_Enabled;
}

ToolChain::ToolChain(const Driver &D, const llvm::Triple &T,
                     const ArgList &Args)
    : D(D), Triple(T), Args(Args), CachedRTTIArg(GetRTTIArgument(Args)),
      CachedRTTIMode(CalculateRTTIMode(Args, Triple, CachedRTTIArg)) {
  auto addIfExists = [this](path_list &List, const std::string &Path) {
    if (getVFS().exists(Path))
      List.push_back(Path);
  };

  for (const auto &Path : getRuntimePaths())
    addIfExists(getLibraryPaths(), Path);
  for (const auto &Path : getStdlibPaths())
    addIfExists(getFilePaths(), Path);
  addIfExists(getFilePaths(), getArchSpecificLibPath());
}

void ToolChain::setTripleEnvironment(llvm::Triple::EnvironmentType Env) {
  Triple.setEnvironment(Env);
  if (EffectiveTriple != llvm::Triple())
    EffectiveTriple.setEnvironment(Env);
}

ToolChain::~ToolChain() = default;

llvm::vfs::FileSystem &ToolChain::getVFS() const {
  return getDriver().getVFS();
}

bool ToolChain::useIntegratedAs() const {
  return Args.hasFlag(options::OPT_fintegrated_as,
                      options::OPT_fno_integrated_as,
                      IsIntegratedAssemblerDefault());
}

bool ToolChain::useRelaxRelocations() const {
  return ENABLE_X86_RELAX_RELOCATIONS;
}

bool ToolChain::defaultToIEEELongDouble() const {
  return PPC_LINUX_DEFAULT_IEEELONGDOUBLE && getTriple().isOSLinux();
}

SanitizerArgs
ToolChain::getSanitizerArgs(const llvm::opt::ArgList &JobArgs) const {
  SanitizerArgs SanArgs(*this, JobArgs, !SanitizerArgsChecked);
  SanitizerArgsChecked = true;
  return SanArgs;
}

const XRayArgs& ToolChain::getXRayArgs() const {
  if (!XRayArguments.get())
    XRayArguments.reset(new XRayArgs(*this, Args));
  return *XRayArguments.get();
}

namespace {

struct DriverSuffix {
  const char *Suffix;
  const char *ModeFlag;
};

} // namespace

static const DriverSuffix *FindDriverSuffix(StringRef ProgName, size_t &Pos) {
  // A list of known driver suffixes. Suffixes are compared against the
  // program name in order. If there is a match, the frontend type is updated as
  // necessary by applying the ModeFlag.
  static const DriverSuffix DriverSuffixes[] = {
      {"clang", nullptr},
      {"clang++", "--driver-mode=g++"},
      {"clang-c++", "--driver-mode=g++"},
      {"clang-cc", nullptr},
      {"clang-cpp", "--driver-mode=cpp"},
      {"clang-g++", "--driver-mode=g++"},
      {"clang-gcc", nullptr},
      {"clang-cl", "--driver-mode=cl"},
      {"cc", nullptr},
      {"cpp", "--driver-mode=cpp"},
      {"cl", "--driver-mode=cl"},
      {"++", "--driver-mode=g++"},
      {"flang", "--driver-mode=flang"},
      {"clang-dxc", "--driver-mode=dxc"},
  };

  for (size_t i = 0; i < llvm::array_lengthof(DriverSuffixes); ++i) {
    StringRef Suffix(DriverSuffixes[i].Suffix);
    if (ProgName.endswith(Suffix)) {
      Pos = ProgName.size() - Suffix.size();
      return &DriverSuffixes[i];
    }
  }
  return nullptr;
}

/// Normalize the program name from argv[0] by stripping the file extension if
/// present and lower-casing the string on Windows.
static std::string normalizeProgramName(llvm::StringRef Argv0) {
  std::string ProgName = std::string(llvm::sys::path::stem(Argv0));
  if (is_style_windows(llvm::sys::path::Style::native)) {
    // Transform to lowercase for case insensitive file systems.
    std::transform(ProgName.begin(), ProgName.end(), ProgName.begin(),
                   ::tolower);
  }
  return ProgName;
}

static const DriverSuffix *parseDriverSuffix(StringRef ProgName, size_t &Pos) {
  // Try to infer frontend type and default target from the program name by
  // comparing it against DriverSuffixes in order.

  // If there is a match, the function tries to identify a target as prefix.
  // E.g. "x86_64-linux-clang" as interpreted as suffix "clang" with target
  // prefix "x86_64-linux". If such a target prefix is found, it may be
  // added via -target as implicit first argument.
  const DriverSuffix *DS = FindDriverSuffix(ProgName, Pos);

  if (!DS) {
    // Try again after stripping any trailing version number:
    // clang++3.5 -> clang++
    ProgName = ProgName.rtrim("0123456789.");
    DS = FindDriverSuffix(ProgName, Pos);
  }

  if (!DS) {
    // Try again after stripping trailing -component.
    // clang++-tot -> clang++
    ProgName = ProgName.slice(0, ProgName.rfind('-'));
    DS = FindDriverSuffix(ProgName, Pos);
  }
  return DS;
}

ParsedClangName
ToolChain::getTargetAndModeFromProgramName(StringRef PN) {
  std::string ProgName = normalizeProgramName(PN);
  size_t SuffixPos;
  const DriverSuffix *DS = parseDriverSuffix(ProgName, SuffixPos);
  if (!DS)
    return {};
  size_t SuffixEnd = SuffixPos + strlen(DS->Suffix);

  size_t LastComponent = ProgName.rfind('-', SuffixPos);
  if (LastComponent == std::string::npos)
    return ParsedClangName(ProgName.substr(0, SuffixEnd), DS->ModeFlag);
  std::string ModeSuffix = ProgName.substr(LastComponent + 1,
                                           SuffixEnd - LastComponent - 1);

  // Infer target from the prefix.
  StringRef Prefix(ProgName);
  Prefix = Prefix.slice(0, LastComponent);
  std::string IgnoredError;
  bool IsRegistered =
      llvm::TargetRegistry::lookupTarget(std::string(Prefix), IgnoredError);
  return ParsedClangName{std::string(Prefix), ModeSuffix, DS->ModeFlag,
                         IsRegistered};
}

StringRef ToolChain::getDefaultUniversalArchName() const {
  // In universal driver terms, the arch name accepted by -arch isn't exactly
  // the same as the ones that appear in the triple. Roughly speaking, this is
  // an inverse of the darwin::getArchTypeForDarwinArchName() function.
  switch (Triple.getArch()) {
  case llvm::Triple::aarch64: {
    if (getTriple().isArm64e())
      return "arm64e";
    return "arm64";
  }
  case llvm::Triple::aarch64_32:
    return "arm64_32";
  case llvm::Triple::ppc:
    return "ppc";
  case llvm::Triple::ppcle:
    return "ppcle";
  case llvm::Triple::ppc64:
    return "ppc64";
  case llvm::Triple::ppc64le:
    return "ppc64le";
  default:
    return Triple.getArchName();
  }
}

std::string ToolChain::getInputFilename(const InputInfo &Input) const {
  return Input.getFilename();
}

bool ToolChain::IsUnwindTablesDefault(const ArgList &Args) const {
  return false;
}

Tool *ToolChain::getClang() const {
  if (!Clang)
    Clang.reset(new tools::Clang(*this, useIntegratedBackend()));
  return Clang.get();
}

Tool *ToolChain::getFlang() const {
  if (!Flang)
    Flang.reset(new tools::Flang(*this));
  return Flang.get();
}

Tool *ToolChain::buildAssembler() const {
  return new tools::ClangAs(*this);
}

Tool *ToolChain::buildLinker() const {
  llvm_unreachable("Linking is not supported by this toolchain");
}

Tool *ToolChain::buildBackendCompiler() const {
  llvm_unreachable("Backend Compilation is not supported by this toolchain");
}

Tool *ToolChain::buildStaticLibTool() const {
  llvm_unreachable("Creating static lib is not supported by this toolchain");
}

Tool *ToolChain::getAssemble() const {
  if (!Assemble)
    Assemble.reset(buildAssembler());
  return Assemble.get();
}

Tool *ToolChain::getClangAs() const {
  if (!Assemble)
    Assemble.reset(new tools::ClangAs(*this));
  return Assemble.get();
}

Tool *ToolChain::getLink() const {
  if (!Link)
    Link.reset(buildLinker());
  return Link.get();
}

Tool *ToolChain::getStaticLibTool() const {
  if (!StaticLibTool)
    StaticLibTool.reset(buildStaticLibTool());
  return StaticLibTool.get();
}

Tool *ToolChain::getIfsMerge() const {
  if (!IfsMerge)
    IfsMerge.reset(new tools::ifstool::Merger(*this));
  return IfsMerge.get();
}

Tool *ToolChain::getOffloadBundler() const {
  if (!OffloadBundler)
    OffloadBundler.reset(new tools::OffloadBundler(*this));
  return OffloadBundler.get();
}

Tool *ToolChain::getOffloadWrapper() const {
  if (!OffloadWrapper)
    OffloadWrapper.reset(new tools::OffloadWrapper(*this));
  return OffloadWrapper.get();
}

Tool *ToolChain::getOffloadDeps() const {
  if (!OffloadDeps)
    OffloadDeps.reset(new tools::OffloadDeps(*this));
  return OffloadDeps.get();
}

Tool *ToolChain::getSPIRVTranslator() const {
  if (!SPIRVTranslator)
    SPIRVTranslator.reset(new tools::SPIRVTranslator(*this));
  return SPIRVTranslator.get();
}

Tool *ToolChain::getSPIRCheck() const {
  if (!SPIRCheck)
    SPIRCheck.reset(new tools::SPIRCheck(*this));
  return SPIRCheck.get();
}

Tool *ToolChain::getSYCLPostLink() const {
  if (!SYCLPostLink)
    SYCLPostLink.reset(new tools::SYCLPostLink(*this));
  return SYCLPostLink.get();
}

Tool *ToolChain::getBackendCompiler() const {
  if (!BackendCompiler)
    BackendCompiler.reset(buildBackendCompiler());
  return BackendCompiler.get();
}

Tool *ToolChain::getAppendFooter() const {
  if (!AppendFooter)
    AppendFooter.reset(new tools::AppendFooter(*this));
  return AppendFooter.get();
}

Tool *ToolChain::getTableTform() const {
  if (!FileTableTform)
    FileTableTform.reset(new tools::FileTableTform(*this));
  return FileTableTform.get();
}

Tool *ToolChain::getSpirvToIrWrapper() const {
  if (!SpirvToIrWrapper)
    SpirvToIrWrapper.reset(new tools::SpirvToIrWrapper(*this));
  return SpirvToIrWrapper.get();
}

Tool *ToolChain::getLinkerWrapper() const {
  if (!LinkerWrapper)
    LinkerWrapper.reset(new tools::LinkerWrapper(*this, getLink()));
  return LinkerWrapper.get();
}

Tool *ToolChain::getTool(Action::ActionClass AC) const {
  switch (AC) {
  case Action::AssembleJobClass:
    return getAssemble();

  case Action::IfsMergeJobClass:
    return getIfsMerge();

  case Action::LinkJobClass:
    return getLink();

  case Action::StaticLibJobClass:
    return getStaticLibTool();

  case Action::InputClass:
  case Action::BindArchClass:
  case Action::OffloadClass:
  case Action::ForEachWrappingClass:
  case Action::LipoJobClass:
  case Action::DsymutilJobClass:
  case Action::VerifyDebugInfoJobClass:
    llvm_unreachable("Invalid tool kind.");

  case Action::CompileJobClass:
  case Action::PrecompileJobClass:
  case Action::HeaderModulePrecompileJobClass:
  case Action::PreprocessJobClass:
  case Action::ExtractAPIJobClass:
  case Action::AnalyzeJobClass:
  case Action::MigrateJobClass:
  case Action::VerifyPCHJobClass:
  case Action::BackendJobClass:
    return getClang();

  case Action::OffloadBundlingJobClass:
  case Action::OffloadUnbundlingJobClass:
    return getOffloadBundler();

  case Action::OffloadWrapperJobClass:
    return getOffloadWrapper();

  case Action::OffloadDepsJobClass:
    return getOffloadDeps();

  case Action::SPIRVTranslatorJobClass:
    return getSPIRVTranslator();

  case Action::SPIRCheckJobClass:
    return getSPIRCheck();

  case Action::SYCLPostLinkJobClass:
    return getSYCLPostLink();

  case Action::BackendCompileJobClass:
    return getBackendCompiler();

  case Action::AppendFooterJobClass:
    return getAppendFooter();

  case Action::FileTableTformJobClass:
    return getTableTform();

  case Action::SpirvToIrWrapperJobClass:
    return getSpirvToIrWrapper();

  case Action::LinkerWrapperJobClass:
    return getLinkerWrapper();
  }

  llvm_unreachable("Invalid tool kind.");
}

static StringRef getArchNameForCompilerRTLib(const ToolChain &TC,
                                             const ArgList &Args) {
  const llvm::Triple &Triple = TC.getTriple();
  bool IsWindows = Triple.isOSWindows();

  if (TC.getArch() == llvm::Triple::arm || TC.getArch() == llvm::Triple::armeb)
    return (arm::getARMFloatABI(TC, Args) == arm::FloatABI::Hard && !IsWindows)
               ? "armhf"
               : "arm";

  // For historic reasons, Android library is using i686 instead of i386.
  if (TC.getArch() == llvm::Triple::x86 && Triple.isAndroid())
    return "i686";

  if (TC.getArch() == llvm::Triple::x86_64 && Triple.isX32())
    return "x32";

  return llvm::Triple::getArchTypeName(TC.getArch());
}

StringRef ToolChain::getOSLibName() const {
  if (Triple.isOSDarwin())
    return "darwin";

  switch (Triple.getOS()) {
  case llvm::Triple::FreeBSD:
    return "freebsd";
  case llvm::Triple::NetBSD:
    return "netbsd";
  case llvm::Triple::OpenBSD:
    return "openbsd";
  case llvm::Triple::Solaris:
    return "sunos";
  case llvm::Triple::AIX:
    return "aix";
  default:
    return getOS();
  }
}

std::string ToolChain::getCompilerRTPath() const {
  SmallString<128> Path(getDriver().ResourceDir);
  if (Triple.isOSUnknown()) {
    llvm::sys::path::append(Path, "lib");
  } else {
    llvm::sys::path::append(Path, "lib", getOSLibName());
  }
  return std::string(Path.str());
}

std::string ToolChain::getCompilerRTBasename(const ArgList &Args,
                                             StringRef Component,
                                             FileType Type) const {
  std::string CRTAbsolutePath = getCompilerRT(Args, Component, Type);
  return llvm::sys::path::filename(CRTAbsolutePath).str();
}

std::string ToolChain::buildCompilerRTBasename(const llvm::opt::ArgList &Args,
                                               StringRef Component,
                                               FileType Type,
                                               bool AddArch) const {
  const llvm::Triple &TT = getTriple();
  bool IsITANMSVCWindows =
      TT.isWindowsMSVCEnvironment() || TT.isWindowsItaniumEnvironment();

  const char *Prefix =
      IsITANMSVCWindows || Type == ToolChain::FT_Object ? "" : "lib";
  const char *Suffix;
  switch (Type) {
  case ToolChain::FT_Object:
    Suffix = IsITANMSVCWindows ? ".obj" : ".o";
    break;
  case ToolChain::FT_Static:
    Suffix = IsITANMSVCWindows ? ".lib" : ".a";
    break;
  case ToolChain::FT_Shared:
    Suffix = TT.isOSWindows()
                 ? (TT.isWindowsGNUEnvironment() ? ".dll.a" : ".lib")
                 : ".so";
    break;
  }

  std::string ArchAndEnv;
  if (AddArch) {
    StringRef Arch = getArchNameForCompilerRTLib(*this, Args);
    const char *Env = TT.isAndroid() ? "-android" : "";
    ArchAndEnv = ("-" + Arch + Env).str();
  }
  return (Prefix + Twine("clang_rt.") + Component + ArchAndEnv + Suffix).str();
}

std::string ToolChain::getCompilerRT(const ArgList &Args, StringRef Component,
                                     FileType Type) const {
  // Check for runtime files in the new layout without the architecture first.
  std::string CRTBasename =
      buildCompilerRTBasename(Args, Component, Type, /*AddArch=*/false);
  for (const auto &LibPath : getLibraryPaths()) {
    SmallString<128> P(LibPath);
    llvm::sys::path::append(P, CRTBasename);
    if (getVFS().exists(P))
      return std::string(P.str());
  }

  // Fall back to the old expected compiler-rt name if the new one does not
  // exist.
  CRTBasename =
      buildCompilerRTBasename(Args, Component, Type, /*AddArch=*/true);
  SmallString<128> Path(getCompilerRTPath());
  llvm::sys::path::append(Path, CRTBasename);
  return std::string(Path.str());
}

const char *ToolChain::getCompilerRTArgString(const llvm::opt::ArgList &Args,
                                              StringRef Component,
                                              FileType Type) const {
  return Args.MakeArgString(getCompilerRT(Args, Component, Type));
}

ToolChain::path_list ToolChain::getRuntimePaths() const {
  path_list Paths;
  auto addPathForTriple = [this, &Paths](const llvm::Triple &Triple) {
    SmallString<128> P(D.ResourceDir);
    llvm::sys::path::append(P, "lib", Triple.str());
    Paths.push_back(std::string(P.str()));
  };

  addPathForTriple(getTriple());

  // Android targets may include an API level at the end. We still want to fall
  // back on a path without the API level.
  if (getTriple().isAndroid() &&
      getTriple().getEnvironmentName() != "android") {
    llvm::Triple TripleWithoutLevel = getTriple();
    TripleWithoutLevel.setEnvironmentName("android");
    addPathForTriple(TripleWithoutLevel);
  }

  return Paths;
}

ToolChain::path_list ToolChain::getStdlibPaths() const {
  path_list Paths;
  SmallString<128> P(D.Dir);
  llvm::sys::path::append(P, "..", "lib", getTripleString());
  Paths.push_back(std::string(P.str()));

  return Paths;
}

std::string ToolChain::getArchSpecificLibPath() const {
  SmallString<128> Path(getDriver().ResourceDir);
  llvm::sys::path::append(Path, "lib", getOSLibName(),
                          llvm::Triple::getArchTypeName(getArch()));
  return std::string(Path.str());
}

bool ToolChain::needsProfileRT(const ArgList &Args) {
  if (Args.hasArg(options::OPT_noprofilelib))
    return false;

  return Args.hasArg(options::OPT_fprofile_generate) ||
         Args.hasArg(options::OPT_fprofile_generate_EQ) ||
         Args.hasArg(options::OPT_fcs_profile_generate) ||
         Args.hasArg(options::OPT_fcs_profile_generate_EQ) ||
         Args.hasArg(options::OPT_fprofile_instr_generate) ||
         Args.hasArg(options::OPT_fprofile_instr_generate_EQ) ||
         Args.hasArg(options::OPT_fcreate_profile) ||
         Args.hasArg(options::OPT_forder_file_instrumentation);
}

bool ToolChain::needsGCovInstrumentation(const llvm::opt::ArgList &Args) {
  return Args.hasArg(options::OPT_coverage) ||
         Args.hasFlag(options::OPT_fprofile_arcs, options::OPT_fno_profile_arcs,
                      false);
}

Tool *ToolChain::SelectTool(const JobAction &JA) const {
  if (D.IsFlangMode() && getDriver().ShouldUseFlangCompiler(JA)) return getFlang();
  if (getDriver().ShouldUseClangCompiler(JA)) return getClang();
  Action::ActionClass AC = JA.getKind();
  if (AC == Action::AssembleJobClass && useIntegratedAs())
    return getClangAs();
  return getTool(AC);
}

std::string ToolChain::GetFilePath(const char *Name) const {
  return D.GetFilePath(Name, *this);
}

std::string ToolChain::GetProgramPath(const char *Name) const {
  return D.GetProgramPath(Name, *this);
}

std::string ToolChain::GetLinkerPath(bool *LinkerIsLLD) const {
  if (LinkerIsLLD)
    *LinkerIsLLD = false;

  // Get -fuse-ld= first to prevent -Wunused-command-line-argument. -fuse-ld= is
  // considered as the linker flavor, e.g. "bfd", "gold", or "lld".
  const Arg* A = Args.getLastArg(options::OPT_fuse_ld_EQ);
  StringRef UseLinker = A ? A->getValue() : CLANG_DEFAULT_LINKER;

  // --ld-path= takes precedence over -fuse-ld= and specifies the executable
  // name. -B, COMPILER_PATH and PATH and consulted if the value does not
  // contain a path component separator.
  if (const Arg *A = Args.getLastArg(options::OPT_ld_path_EQ)) {
    std::string Path(A->getValue());
    if (!Path.empty()) {
      if (llvm::sys::path::parent_path(Path).empty())
        Path = GetProgramPath(A->getValue());
      if (llvm::sys::fs::can_execute(Path))
        return std::string(Path);
    }
    getDriver().Diag(diag::err_drv_invalid_linker_name) << A->getAsString(Args);
    return GetProgramPath(getDefaultLinker());
  }
  // If we're passed -fuse-ld= with no argument, or with the argument ld,
  // then use whatever the default system linker is.
  if (UseLinker.empty() || UseLinker == "ld") {
    const char *DefaultLinker = getDefaultLinker();
    if (llvm::sys::path::is_absolute(DefaultLinker))
      return std::string(DefaultLinker);
    else
      return GetProgramPath(DefaultLinker);
  }

  // Extending -fuse-ld= to an absolute or relative path is unexpected. Checking
  // for the linker flavor is brittle. In addition, prepending "ld." or "ld64."
  // to a relative path is surprising. This is more complex due to priorities
  // among -B, COMPILER_PATH and PATH. --ld-path= should be used instead.
  if (UseLinker.contains('/'))
    getDriver().Diag(diag::warn_drv_fuse_ld_path);

  if (llvm::sys::path::is_absolute(UseLinker)) {
    // If we're passed what looks like an absolute path, don't attempt to
    // second-guess that.
    if (llvm::sys::fs::can_execute(UseLinker))
      return std::string(UseLinker);
  } else {
    llvm::SmallString<8> LinkerName;
    if (Triple.isOSDarwin())
      LinkerName.append("ld64.");
    else
      LinkerName.append("ld.");
    LinkerName.append(UseLinker);

    std::string LinkerPath(GetProgramPath(LinkerName.c_str()));
    if (llvm::sys::fs::can_execute(LinkerPath)) {
      if (LinkerIsLLD)
        *LinkerIsLLD = UseLinker == "lld";
      return LinkerPath;
    }
  }

  if (A)
    getDriver().Diag(diag::err_drv_invalid_linker_name) << A->getAsString(Args);

  return GetProgramPath(getDefaultLinker());
}

std::string ToolChain::GetStaticLibToolPath() const {
  // TODO: Add support for static lib archiving on Windows
  if (Triple.isOSDarwin())
    return GetProgramPath("libtool");
  return GetProgramPath("llvm-ar");
}

types::ID ToolChain::LookupTypeForExtension(StringRef Ext) const {
  types::ID id = types::lookupTypeForExtension(Ext);

  // Flang always runs the preprocessor and has no notion of "preprocessed
  // fortran". Here, TY_PP_Fortran is coerced to TY_Fortran to avoid treating
  // them differently.
  if (D.IsFlangMode() && id == types::TY_PP_Fortran)
    id = types::TY_Fortran;

  return id;
}

bool ToolChain::HasNativeLLVMSupport() const {
  return false;
}

bool ToolChain::isCrossCompiling() const {
  llvm::Triple HostTriple(LLVM_HOST_TRIPLE);
  switch (HostTriple.getArch()) {
  // The A32/T32/T16 instruction sets are not separate architectures in this
  // context.
  case llvm::Triple::arm:
  case llvm::Triple::armeb:
  case llvm::Triple::thumb:
  case llvm::Triple::thumbeb:
    return getArch() != llvm::Triple::arm && getArch() != llvm::Triple::thumb &&
           getArch() != llvm::Triple::armeb && getArch() != llvm::Triple::thumbeb;
  default:
    return HostTriple.getArch() != getArch();
  }
}

ObjCRuntime ToolChain::getDefaultObjCRuntime(bool isNonFragile) const {
  return ObjCRuntime(isNonFragile ? ObjCRuntime::GNUstep : ObjCRuntime::GCC,
                     VersionTuple());
}

llvm::ExceptionHandling
ToolChain::GetExceptionModel(const llvm::opt::ArgList &Args) const {
  return llvm::ExceptionHandling::None;
}

bool ToolChain::isThreadModelSupported(const StringRef Model) const {
  if (Model == "single") {
    // FIXME: 'single' is only supported on ARM and WebAssembly so far.
    return Triple.getArch() == llvm::Triple::arm ||
           Triple.getArch() == llvm::Triple::armeb ||
           Triple.getArch() == llvm::Triple::thumb ||
           Triple.getArch() == llvm::Triple::thumbeb || Triple.isWasm();
  } else if (Model == "posix")
    return true;

  return false;
}

std::string ToolChain::ComputeLLVMTriple(const ArgList &Args,
                                         types::ID InputType) const {
  switch (getTriple().getArch()) {
  default:
    return getTripleString();

  case llvm::Triple::x86_64: {
    llvm::Triple Triple = getTriple();
    if (!Triple.isOSBinFormatMachO())
      return getTripleString();

    if (Arg *A = Args.getLastArg(options::OPT_march_EQ)) {
      // x86_64h goes in the triple. Other -march options just use the
      // vanilla triple we already have.
      StringRef MArch = A->getValue();
      if (MArch == "x86_64h")
        Triple.setArchName(MArch);
    }
    return Triple.getTriple();
  }
  case llvm::Triple::aarch64: {
    llvm::Triple Triple = getTriple();
    if (!Triple.isOSBinFormatMachO())
      return getTripleString();

    if (Triple.isArm64e())
      return getTripleString();

    // FIXME: older versions of ld64 expect the "arm64" component in the actual
    // triple string and query it to determine whether an LTO file can be
    // handled. Remove this when we don't care any more.
    Triple.setArchName("arm64");
    return Triple.getTriple();
  }
  case llvm::Triple::aarch64_32:
    return getTripleString();
  case llvm::Triple::arm:
  case llvm::Triple::armeb:
  case llvm::Triple::thumb:
  case llvm::Triple::thumbeb: {
    llvm::Triple Triple = getTriple();
    tools::arm::setArchNameInTriple(getDriver(), Args, InputType, Triple);
    tools::arm::setFloatABIInTriple(getDriver(), Args, Triple);
    return Triple.getTriple();
  }
  }
}

std::string ToolChain::ComputeEffectiveClangTriple(const ArgList &Args,
                                                   types::ID InputType) const {
  return ComputeLLVMTriple(Args, InputType);
}

std::string ToolChain::computeSysRoot() const {
  return D.SysRoot;
}

void ToolChain::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                          ArgStringList &CC1Args) const {
  // Each toolchain should provide the appropriate include flags.
}

void ToolChain::addClangTargetOptions(
    const ArgList &DriverArgs, ArgStringList &CC1Args,
    Action::OffloadKind DeviceOffloadKind) const {}

void ToolChain::addClangWarningOptions(ArgStringList &CC1Args) const {}

void ToolChain::addProfileRTLibs(const llvm::opt::ArgList &Args,
                                 llvm::opt::ArgStringList &CmdArgs) const {
  if (!needsProfileRT(Args) && !needsGCovInstrumentation(Args))
    return;

  CmdArgs.push_back(getCompilerRTArgString(Args, "profile"));
}

ToolChain::RuntimeLibType ToolChain::GetRuntimeLibType(
    const ArgList &Args) const {
  if (runtimeLibType)
    return *runtimeLibType;

  const Arg* A = Args.getLastArg(options::OPT_rtlib_EQ);
  StringRef LibName = A ? A->getValue() : CLANG_DEFAULT_RTLIB;

  // Only use "platform" in tests to override CLANG_DEFAULT_RTLIB!
  if (LibName == "compiler-rt")
    runtimeLibType = ToolChain::RLT_CompilerRT;
  else if (LibName == "libgcc")
    runtimeLibType = ToolChain::RLT_Libgcc;
  else if (LibName == "platform")
    runtimeLibType = GetDefaultRuntimeLibType();
  else {
    if (A)
      getDriver().Diag(diag::err_drv_invalid_rtlib_name)
          << A->getAsString(Args);

    runtimeLibType = GetDefaultRuntimeLibType();
  }

  return *runtimeLibType;
}

ToolChain::UnwindLibType ToolChain::GetUnwindLibType(
    const ArgList &Args) const {
  if (unwindLibType)
    return *unwindLibType;

  const Arg *A = Args.getLastArg(options::OPT_unwindlib_EQ);
  StringRef LibName = A ? A->getValue() : CLANG_DEFAULT_UNWINDLIB;

  if (LibName == "none")
    unwindLibType = ToolChain::UNW_None;
  else if (LibName == "platform" || LibName == "") {
    ToolChain::RuntimeLibType RtLibType = GetRuntimeLibType(Args);
    if (RtLibType == ToolChain::RLT_CompilerRT) {
      if (getTriple().isAndroid() || getTriple().isOSAIX())
        unwindLibType = ToolChain::UNW_CompilerRT;
      else
        unwindLibType = ToolChain::UNW_None;
    } else if (RtLibType == ToolChain::RLT_Libgcc)
      unwindLibType = ToolChain::UNW_Libgcc;
  } else if (LibName == "libunwind") {
    if (GetRuntimeLibType(Args) == RLT_Libgcc)
      getDriver().Diag(diag::err_drv_incompatible_unwindlib);
    unwindLibType = ToolChain::UNW_CompilerRT;
  } else if (LibName == "libgcc")
    unwindLibType = ToolChain::UNW_Libgcc;
  else {
    if (A)
      getDriver().Diag(diag::err_drv_invalid_unwindlib_name)
          << A->getAsString(Args);

    unwindLibType = GetDefaultUnwindLibType();
  }

  return *unwindLibType;
}

ToolChain::CXXStdlibType ToolChain::GetCXXStdlibType(const ArgList &Args) const{
  if (cxxStdlibType)
    return *cxxStdlibType;

  const Arg *A = Args.getLastArg(options::OPT_stdlib_EQ);
  StringRef LibName = A ? A->getValue() : CLANG_DEFAULT_CXX_STDLIB;

  // Only use "platform" in tests to override CLANG_DEFAULT_CXX_STDLIB!
  if (LibName == "libc++")
    cxxStdlibType = ToolChain::CST_Libcxx;
  else if (LibName == "libstdc++")
    cxxStdlibType = ToolChain::CST_Libstdcxx;
  else if (LibName == "platform")
    cxxStdlibType = GetDefaultCXXStdlibType();
  else {
    if (A)
      getDriver().Diag(diag::err_drv_invalid_stdlib_name)
          << A->getAsString(Args);

    cxxStdlibType = GetDefaultCXXStdlibType();
  }

  return *cxxStdlibType;
}

/// Utility function to add a system include directory to CC1 arguments.
/*static*/ void ToolChain::addSystemInclude(const ArgList &DriverArgs,
                                            ArgStringList &CC1Args,
                                            const Twine &Path) {
  CC1Args.push_back("-internal-isystem");
  CC1Args.push_back(DriverArgs.MakeArgString(Path));
}

/// Utility function to add a system include directory with extern "C"
/// semantics to CC1 arguments.
///
/// Note that this should be used rarely, and only for directories that
/// historically and for legacy reasons are treated as having implicit extern
/// "C" semantics. These semantics are *ignored* by and large today, but its
/// important to preserve the preprocessor changes resulting from the
/// classification.
/*static*/ void ToolChain::addExternCSystemInclude(const ArgList &DriverArgs,
                                                   ArgStringList &CC1Args,
                                                   const Twine &Path) {
  CC1Args.push_back("-internal-externc-isystem");
  CC1Args.push_back(DriverArgs.MakeArgString(Path));
}

void ToolChain::addExternCSystemIncludeIfExists(const ArgList &DriverArgs,
                                                ArgStringList &CC1Args,
                                                const Twine &Path) {
  if (llvm::sys::fs::exists(Path))
    addExternCSystemInclude(DriverArgs, CC1Args, Path);
}

/// Utility function to add a list of system include directories to CC1.
/*static*/ void ToolChain::addSystemIncludes(const ArgList &DriverArgs,
                                             ArgStringList &CC1Args,
                                             ArrayRef<StringRef> Paths) {
  for (const auto &Path : Paths) {
    CC1Args.push_back("-internal-isystem");
    CC1Args.push_back(DriverArgs.MakeArgString(Path));
  }
}

std::string ToolChain::detectLibcxxVersion(StringRef IncludePath) const {
  std::error_code EC;
  int MaxVersion = 0;
  std::string MaxVersionString;
  SmallString<128> Path(IncludePath);
  llvm::sys::path::append(Path, "c++");
  for (llvm::vfs::directory_iterator LI = getVFS().dir_begin(Path, EC), LE;
       !EC && LI != LE; LI = LI.increment(EC)) {
    StringRef VersionText = llvm::sys::path::filename(LI->path());
    int Version;
    if (VersionText[0] == 'v' &&
        !VersionText.slice(1, StringRef::npos).getAsInteger(10, Version)) {
      if (Version > MaxVersion) {
        MaxVersion = Version;
        MaxVersionString = std::string(VersionText);
      }
    }
  }
  if (!MaxVersion)
    return "";
  return MaxVersionString;
}

void ToolChain::AddClangCXXStdlibIncludeArgs(const ArgList &DriverArgs,
                                             ArgStringList &CC1Args) const {
  // Header search paths should be handled by each of the subclasses.
  // Historically, they have not been, and instead have been handled inside of
  // the CC1-layer frontend. As the logic is hoisted out, this generic function
  // will slowly stop being called.
  //
  // While it is being called, replicate a bit of a hack to propagate the
  // '-stdlib=' flag down to CC1 so that it can in turn customize the C++
  // header search paths with it. Once all systems are overriding this
  // function, the CC1 flag and this line can be removed.
  DriverArgs.AddAllArgs(CC1Args, options::OPT_stdlib_EQ);
}

void ToolChain::AddClangCXXStdlibIsystemArgs(
    const llvm::opt::ArgList &DriverArgs,
    llvm::opt::ArgStringList &CC1Args) const {
  DriverArgs.ClaimAllArgs(options::OPT_stdlibxx_isystem);
  if (!DriverArgs.hasArg(options::OPT_nostdinc, options::OPT_nostdincxx,
                         options::OPT_nostdlibinc))
    for (const auto &P :
         DriverArgs.getAllArgValues(options::OPT_stdlibxx_isystem))
      addSystemInclude(DriverArgs, CC1Args, P);
}

bool ToolChain::ShouldLinkCXXStdlib(const llvm::opt::ArgList &Args) const {
  return getDriver().CCCIsCXX() &&
         !Args.hasArg(options::OPT_nostdlib, options::OPT_nodefaultlibs,
                      options::OPT_nostdlibxx);
}

void ToolChain::AddCXXStdlibLibArgs(const ArgList &Args,
                                    ArgStringList &CmdArgs) const {
  assert(!Args.hasArg(options::OPT_nostdlibxx) &&
         "should not have called this");
  CXXStdlibType Type = GetCXXStdlibType(Args);

  switch (Type) {
  case ToolChain::CST_Libcxx:
    CmdArgs.push_back("-lc++");
#if INTEL_CUSTOMIZATION
    if (Args.hasArg(options::OPT__intel))
      CmdArgs.push_back("-lc++abi");
#endif // INTEL_CUSTOMIZTION
    break;

  case ToolChain::CST_Libstdcxx:
    CmdArgs.push_back("-lstdc++");
    break;
  }
}

void ToolChain::AddFilePathLibArgs(const ArgList &Args,
                                   ArgStringList &CmdArgs) const {
  for (const auto &LibPath : getFilePaths())
    if(LibPath.length() > 0)
      CmdArgs.push_back(Args.MakeArgString(StringRef("-L") + LibPath));
}

void ToolChain::AddCCKextLibArgs(const ArgList &Args,
                                 ArgStringList &CmdArgs) const {
  CmdArgs.push_back("-lcc_kext");
}

#if INTEL_CUSTOMIZATION
void ToolChain::AddIntelLibimfLibArgs(const ArgList &Args,
                                      ArgStringList &CmdArgs) const {
  CmdArgs.push_back("-limf");
}

// takes any known library specification string and returns the associated
// option argument which can be used with the -no-intel-lib option.
static StringRef getIntelLibArgVal(StringRef LibName) {
  StringRef LibArg;
  LibArg =
      llvm::StringSwitch<StringRef>(LibName)
          .Cases("libirc", "-lirc", "-lirc_s", "-lintlc", "-lirc_pic", "libirc")
          .Cases("libimf", "-limf", "libimf")
          .Cases("libsvml", "-lsvml", "libsvml")
          .Cases("libirng", "-lirng", "libirng")
          .Default("");
  return LibArg;
}

// Based on input string, determine if the library should be added.
bool ToolChain::CheckAddIntelLib(StringRef LibName, const ArgList &Args) const {
  const Arg *A =
      Args.getLastArg(options::OPT_no_intel_lib, options::OPT_no_intel_lib_EQ);
  if (A) {
    if (A->getOption().matches(options::OPT_no_intel_lib))
      return false;
    for (StringRef Val : A->getValues()) {
      if (Val == getIntelLibArgVal(LibName))
        return false;
    }
  }
  return true;
}

static const std::string getIntelBasePath(const std::string DriverDir) {
  // Perf libs are located in different locations depending on the package
  // being used.  This is PSXE vs oneAPI installations.
  //  oneAPI: <install>/compiler/<ver>/<os>/bin
  //  PSXE: <install>/compiler/<os>/bin
  // FIXME - The path returned is based on oneAPI.  Additional checks need to
  // be added to properly enable for PSXE.
  const std::string BasePath(DriverDir);
  return BasePath + "/../../../../";
}

static std::string getIPPBasePath(const ArgList &Args,
                                  const std::string DriverDir) {
  const char * IPPRoot = getenv("IPPROOT");
  bool IsCrypto = false;
  if (const Arg *A = Args.getLastArg(options::OPT_qipp_EQ)) {
    if (A->getValue() == StringRef("crypto") ||
        A->getValue() == StringRef("nonpic_crypto")) {
      IsCrypto = true;
      IPPRoot = getenv("IPPCRYPTOROOT");
    }
  }
  SmallString<128> P;
  if (IPPRoot)
    P.append(IPPRoot);
  else {
    P.append(getIntelBasePath(DriverDir) + "ipp");
    if (IsCrypto)
      P.append("cp");
  }
  // Lib root could be set to the date based level or one above.  Check for
  // 'latest' and if it is there, use that.
  if (llvm::sys::fs::exists(P + "/latest"))
    llvm::sys::path::append(P, "latest");
  return std::string(P);
}

std::string ToolChain::GetIPPIncludePath(const ArgList &Args) const {
  SmallString<128> P(getIPPBasePath(Args, getDriver().Dir));
  llvm::sys::path::append(P, "include");
  return std::string(P);
}

// Add IPP specific performance library search path.  The different IPP libs
// that are brought in are differentiated by directory.
void ToolChain::AddIPPLibPath(const ArgList &Args, ArgStringList &CmdArgs,
                              std::string Opt) const {
  bool IsNonPIC = false;
  if (const Arg *A = Args.getLastArg(options::OPT_qipp_EQ))
    if (A->getValue() == StringRef("nonpic_crypto") ||
        A->getValue() == StringRef("nonpic"))
      IsNonPIC = true;
  SmallString<128> P(Opt);
  P.append(getIPPBasePath(Args, getDriver().Dir));
  if (getTriple().getArch() == llvm::Triple::x86_64)
    llvm::sys::path::append(P, "lib/intel64");
  else
    llvm::sys::path::append(P, "lib/ia32");
  const Arg *IL = Args.getLastArg(options::OPT_qipp_link_EQ);
  if (IsNonPIC && (!IL || (IL->getValue() == StringRef("static"))))
    llvm::sys::path::append(P, "nonpic");
  if (getTriple().isWindowsMSVCEnvironment()) {
    llvm::sys::path::replace_path_prefix(P, "//", "\\\\");
  }
  CmdArgs.push_back(Args.MakeArgString(P));
}

static std::string getMKLBasePath(const std::string DriverDir) {
  const char * MKLRoot = getenv("MKLROOT");
  SmallString<128> P;
  if (MKLRoot)
    P.append(MKLRoot);
  else
    P.append(getIntelBasePath(DriverDir) + "mkl");
  // Lib root could be set to the date based level or one above.  Check for
  // 'latest' and if it is there, use that.
  if (llvm::sys::fs::exists(P + "/latest"))
    llvm::sys::path::append(P, "latest");
  return std::string(P);
}

std::string ToolChain::GetMKLIncludePath(const ArgList &Args) const {
  SmallString<128> P(getMKLBasePath(getDriver().Dir));
  llvm::sys::path::append(P, "include");
  return std::string(P);
}

std::string ToolChain::GetMKLIncludePathExtra(const ArgList &Args) const {
  SmallString<128> P(GetMKLIncludePath(Args));
  if (getTriple().getArch() == llvm::Triple::x86_64)
    llvm::sys::path::append(P, "intel64/lp64");
  else
    llvm::sys::path::append(P, "ia32");
  return std::string(P);
}

std::string ToolChain::GetMKLLibPath(void) const {
  SmallString<128> P(getMKLBasePath(getDriver().Dir));
  llvm::Triple HostTriple(getAuxTriple() ? *getAuxTriple() : getTriple());
  if (HostTriple.getArch() == llvm::Triple::x86_64)
    llvm::sys::path::append(P, "lib/intel64");
  else
    llvm::sys::path::append(P, "lib/ia32");
  if (getTriple().isWindowsMSVCEnvironment()) {
    llvm::sys::path::replace_path_prefix(P, "//", "\\\\");
  }
  return std::string(P);
}

// Add MKL specific performance library search path
void ToolChain::AddMKLLibPath(const ArgList &Args, ArgStringList &CmdArgs,
                              std::string Opt) const {
  CmdArgs.push_back(Args.MakeArgString(Opt + GetMKLLibPath()));
}

static std::string getTBBBasePath(const std::string DriverDir) {
  const char * TBBRoot = getenv("TBBROOT");
  SmallString<128> P;
  if (TBBRoot)
    P.append(TBBRoot);
  else
    P.append(getIntelBasePath(DriverDir) + "tbb");
  // Lib root could be set to the date based level or one above.  Check for
  // 'latest' and if it is there, use that.
  if (llvm::sys::fs::exists(P + "/latest"))
    llvm::sys::path::append(P, "latest");
  return std::string(P);
}

std::string ToolChain::GetTBBIncludePath(const ArgList &Args) const {
  SmallString<128> P(getTBBBasePath(getDriver().Dir));
  llvm::sys::path::append(P, "include");
  return std::string(P);
}

// Add TBB specific performance library search path
void ToolChain::AddTBBLibPath(const ArgList &Args, ArgStringList &CmdArgs,
                              std::string Opt) const {
  SmallString<128> P(Opt);
  P.append(getTBBBasePath(getDriver().Dir));
  if (getTriple().getArch() == llvm::Triple::x86_64)
    llvm::sys::path::append(P, "lib/intel64");
  else
    llvm::sys::path::append(P, "lib/ia32");
  // FIXME - this only handles Linux and Windows for now
  if (getTriple().isWindowsMSVCEnvironment())
    llvm::sys::path::append(P, "vc14");
  else
    llvm::sys::path::append(P, "gcc4.8");
  if (getTriple().isWindowsMSVCEnvironment()) {
    llvm::sys::path::replace_path_prefix(P, "//", "\\\\");
  }
  CmdArgs.push_back(Args.MakeArgString(P));
}

static std::string getDAALBasePath(const std::string DriverDir) {
  const char * DAALRoot = getenv("DAALROOT");
  SmallString<128> P;
  if (DAALRoot)
    P.append(DAALRoot);
  else
    P.append(getIntelBasePath(DriverDir) + "dal");
  // Lib root could be set to the date based level or one above.  Check for
  // 'latest' and if it is there, use that.
  if (llvm::sys::fs::exists(P + "/latest"))
    llvm::sys::path::append(P, "latest");
  return std::string(P);
}

std::string ToolChain::GetDAALIncludePath(const ArgList &Args) const {
  SmallString<128> P(getDAALBasePath(getDriver().Dir));
  llvm::sys::path::append(P, "include");
  return std::string(P);
}

std::string ToolChain::GetDAALLibPath(void) const {
  SmallString<128> P(getDAALBasePath(getDriver().Dir));
  llvm::Triple HostTriple(getAuxTriple() ? *getAuxTriple() : getTriple());
  if (HostTriple.getArch() == llvm::Triple::x86_64)
    llvm::sys::path::append(P, "lib/intel64");
  else
    llvm::sys::path::append(P, "lib/ia32");
  if (getTriple().isWindowsMSVCEnvironment()) {
    llvm::sys::path::replace_path_prefix(P, "//", "\\\\");
  }
  return std::string(P);
}

// Add DAAL specific performance library search path
void ToolChain::AddDAALLibPath(const ArgList &Args, ArgStringList &CmdArgs,
                               std::string Opt) const {
  CmdArgs.push_back(Args.MakeArgString(Opt + GetDAALLibPath()));
}

// AC Types header and library setup.  Expected location when picked up
// from the compiler installation is in lib/oclfpga
static std::string getACTypesBasePath(const std::string DriverDir) {
  const char * ACTypesRoot = getenv("INTELFPGAOCLSDKROOT");
  SmallString<128> P;
  if (ACTypesRoot)
    P.append(ACTypesRoot);
  else {
    P.append(llvm::sys::path::parent_path(DriverDir));
    llvm::sys::path::append(P, "lib");
    llvm::sys::path::append(P, "oclfpga");
  }
  return std::string(P);
}

std::string ToolChain::GetACTypesIncludePath(const ArgList &Args) const {
  SmallString<128> P(getACTypesBasePath(getDriver().Dir));
  llvm::sys::path::append(P, "include");
  return std::string(P);
}

// The AC Types libraries are located in
// $INTELFPGAOCLSDKROOT/host/windows64/lib
std::string ToolChain::GetACTypesLibPath(void) const {
  SmallString<128> P(getACTypesBasePath(getDriver().Dir));
  llvm::sys::path::append(P, "host");
  llvm::sys::path::append(P,
      getTriple().isWindowsMSVCEnvironment() ? "windows64" : "linux64");
  llvm::sys::path::append(P, "lib");
  return std::string(P);
}

// Add AC Types library search path
void ToolChain::AddACTypesLibPath(const ArgList &Args, ArgStringList &CmdArgs,
                               std::string Opt) const {
  CmdArgs.push_back(Args.MakeArgString(Opt + GetACTypesLibPath()));
}

void ToolChain::AddIPPLibArgs(const ArgList &Args, ArgStringList &CmdArgs,
                              std::string Prefix) const {
  if (const Arg *A = Args.getLastArg(options::OPT_qipp_EQ)) {
    SmallVector<StringRef, 8> IPPLibs;
    if (A->getValue() == StringRef("crypto") ||
        A->getValue() == StringRef("nonpic_crypto"))
      IPPLibs.push_back("ippcp");
    else {
      IPPLibs.push_back("ippcv");
      IPPLibs.push_back("ippch");
      IPPLibs.push_back("ippcc");
      IPPLibs.push_back("ippdc");
      IPPLibs.push_back("ippe");
      IPPLibs.push_back("ippi");
      IPPLibs.push_back("ipps");
      IPPLibs.push_back("ippvm");
      IPPLibs.push_back("ippcore");
    }
    for (const auto &Lib : IPPLibs) {
      std::string LibName(Lib);
      if (Prefix.size() > 0)
        LibName.insert(0, Prefix);
      CmdArgs.push_back(Args.MakeArgString(LibName));
    }
  }
}

void ToolChain::AddMKLLibArgs(const ArgList &Args, ArgStringList &CmdArgs,
                              std::string Prefix) const {
  if (const Arg *A = Args.getLastArg(options::OPT_qmkl_EQ)) {
    // MKL Cluster library additions not supported for DPC++
    // MKL Parallel not supported with OpenMP and DPC++
    if (getDriver().IsDPCPPMode() &&
        (A->getValue() == StringRef("cluster") ||
         (A->getValue() == StringRef("parallel") &&
          Args.hasFlag(options::OPT_fopenmp, options::OPT_fopenmp_EQ,
                       options::OPT_fno_openmp, false))))
      return;
    SmallVector<StringRef, 8> MKLLibs;
    bool IsMSVC = getTriple().isWindowsMSVCEnvironment();
    if (Args.hasArg(options::OPT_fsycl)) {
      SmallString<32> LibName("mkl_sycl");
      if (IsMSVC && Args.hasArg(options::OPT__SLASH_MDd))
        LibName += "d";
      MKLLibs.push_back(Args.MakeArgString(LibName));
    }
    auto addMKLExt = [&Args, IsMSVC](std::string LN, const llvm::Triple &Triple) {
      std::string LibName(LN);
      if (Triple.getArch() == llvm::Triple::x86_64) {
        if (Args.hasArg(options::OPT_fsycl))
          LibName.append("_ilp64");
        else
          LibName.append("_lp64");
      } else if (IsMSVC && Triple.getArch() == llvm::Triple::x86)
        LibName.append("_c");
      return LibName;
    };
    MKLLibs.push_back(Args.MakeArgString(addMKLExt("mkl_intel", getTriple())));
    if (A->getValue() == StringRef("parallel")) {
      if (Args.hasArg(options::OPT_qtbb) || getDriver().IsDPCPPMode()) {
        // Use TBB when -tbb or DPC++
        SmallString<32> LibName("mkl_tbb_thread");
        if (IsMSVC && Args.hasArg(options::OPT__SLASH_MDd))
          LibName += "d";
        MKLLibs.push_back(Args.MakeArgString(LibName));
      } else
        MKLLibs.push_back("mkl_intel_thread");
    }
    if (A->getValue() == StringRef("cluster")) {
      MKLLibs.push_back("mkl_cdft_core");
      MKLLibs.push_back(Args.MakeArgString(addMKLExt("mkl_scalapack",
                        getTriple())));
      MKLLibs.push_back(Args.MakeArgString(addMKLExt("mkl_blacs_intelmpi",
                        getTriple())));
    }
    if (A->getValue() == StringRef("sequential") ||
        A->getValue() == StringRef("cluster"))
      MKLLibs.push_back("mkl_sequential");
    MKLLibs.push_back("mkl_core");
    for (const auto &Lib : MKLLibs) {
      std::string LibName(Lib);
      if (Prefix.size() > 0)
        LibName.insert(0, Prefix);
      CmdArgs.push_back(Args.MakeArgString(LibName));
    }
  }
}

void ToolChain::AddTBBLibArgs(const ArgList &Args, ArgStringList &CmdArgs,
                              std::string Prefix) const {
  std::string TBBLib("tbb");
  if (Prefix.size() > 0)
    TBBLib.insert(0, Prefix);
  CmdArgs.push_back(Args.MakeArgString(TBBLib));
}

void ToolChain::AddDAALLibArgs(const ArgList &Args, ArgStringList &CmdArgs,
                              std::string Prefix) const {
  if (const Arg *A = Args.getLastArg(options::OPT_qdaal_EQ)) {
    SmallVector<StringRef, 4> DAALLibs;
    if (Args.hasArg(options::OPT_fsycl))
      DAALLibs.push_back("onedal_sycl");
    DAALLibs.push_back("onedal_core");
    if (A->getValue() == StringRef("parallel"))
      DAALLibs.push_back("onedal_thread");
    if (A->getValue() == StringRef("sequential"))
      DAALLibs.push_back("onedal_sequential");
    for (const auto &Lib : DAALLibs) {
      std::string LibName(Lib);
      if (Prefix.size() > 0)
        LibName.insert(0, Prefix);
      CmdArgs.push_back(Args.MakeArgString(LibName));
    }
  }
}

void ToolChain::AddACTypesLibArgs(const ArgList &Args, ArgStringList &CmdArgs,
                                  std::string Prefix) const {
  SmallVector<StringRef, 4> ACTypesLibs;
  ACTypesLibs.push_back("dspba_mpir");
  ACTypesLibs.push_back("dspba_mpfr");
  ACTypesLibs.push_back("ac_types_fixed_point_math_x86");
  ACTypesLibs.push_back("ac_types_vpfp_library");
  for (const auto &Lib : ACTypesLibs) {
    std::string LibName(Lib);
    if (Prefix.size() > 0)
      LibName.insert(0, Prefix);
    CmdArgs.push_back(Args.MakeArgString(LibName));
  }
}
#endif // INTEL_CUSTOMIZATION

bool ToolChain::isFastMathRuntimeAvailable(const ArgList &Args,
                                           std::string &Path) const {
  // Do not check for -fno-fast-math or -fno-unsafe-math when -Ofast passed
  // (to keep the linker options consistent with gcc and clang itself).
  if (!isOptimizationLevelFast(getDriver(), Args)) {
    // Check if -ffast-math or -funsafe-math.
    Arg *A =
      Args.getLastArg(options::OPT_ffast_math, options::OPT_fno_fast_math,
                      options::OPT_funsafe_math_optimizations,
                      options::OPT_fno_unsafe_math_optimizations);

    if (!A || A->getOption().getID() == options::OPT_fno_fast_math ||
        A->getOption().getID() == options::OPT_fno_unsafe_math_optimizations)
      return false;
  }
  // If crtfastmath.o exists add it to the arguments.
  Path = GetFilePath("crtfastmath.o");
  return (Path != "crtfastmath.o"); // Not found.
}

bool ToolChain::addFastMathRuntimeIfAvailable(const ArgList &Args,
                                              ArgStringList &CmdArgs) const {
  std::string Path;
  if (isFastMathRuntimeAvailable(Args, Path)) {
    CmdArgs.push_back(Args.MakeArgString(Path));
    return true;
  }

  return false;
}

SanitizerMask ToolChain::getSupportedSanitizers() const {
  // Return sanitizers which don't require runtime support and are not
  // platform dependent.

  SanitizerMask Res =
      (SanitizerKind::Undefined & ~SanitizerKind::Vptr &
       ~SanitizerKind::Function) |
      (SanitizerKind::CFI & ~SanitizerKind::CFIICall) |
      SanitizerKind::CFICastStrict | SanitizerKind::FloatDivideByZero |
      SanitizerKind::UnsignedIntegerOverflow |
      SanitizerKind::UnsignedShiftBase | SanitizerKind::ImplicitConversion |
      SanitizerKind::Nullability | SanitizerKind::LocalBounds;
  if (getTriple().getArch() == llvm::Triple::x86 ||
      getTriple().getArch() == llvm::Triple::x86_64 ||
      getTriple().getArch() == llvm::Triple::arm || getTriple().isWasm() ||
      getTriple().isAArch64())
    Res |= SanitizerKind::CFIICall;
  if (getTriple().getArch() == llvm::Triple::x86_64 ||
      getTriple().isAArch64(64) || getTriple().isRISCV())
    Res |= SanitizerKind::ShadowCallStack;
  if (getTriple().isAArch64(64))
    Res |= SanitizerKind::MemTag;
  return Res;
}

void ToolChain::AddCudaIncludeArgs(const ArgList &DriverArgs,
                                   ArgStringList &CC1Args) const {}

void ToolChain::AddHIPIncludeArgs(const ArgList &DriverArgs,
                                  ArgStringList &CC1Args) const {}

llvm::SmallVector<ToolChain::BitCodeLibraryInfo, 12>
ToolChain::getHIPDeviceLibs(
    const ArgList &DriverArgs,
    const Action::OffloadKind DeviceOffloadingKind) const {
  return {};
}

void ToolChain::AddIAMCUIncludeArgs(const ArgList &DriverArgs,
                                    ArgStringList &CC1Args) const {}

static VersionTuple separateMSVCFullVersion(unsigned Version) {
  if (Version < 100)
    return VersionTuple(Version);

  if (Version < 10000)
    return VersionTuple(Version / 100, Version % 100);

  unsigned Build = 0, Factor = 1;
  for (; Version > 10000; Version = Version / 10, Factor = Factor * 10)
    Build = Build + (Version % 10) * Factor;
  return VersionTuple(Version / 100, Version % 100, Build);
}

VersionTuple
ToolChain::computeMSVCVersion(const Driver *D,
                              const llvm::opt::ArgList &Args) const {
  const Arg *MSCVersion = Args.getLastArg(options::OPT_fmsc_version);
  const Arg *MSCompatibilityVersion =
      Args.getLastArg(options::OPT_fms_compatibility_version);

  if (MSCVersion && MSCompatibilityVersion) {
    if (D)
      D->Diag(diag::err_drv_argument_not_allowed_with)
          << MSCVersion->getAsString(Args)
          << MSCompatibilityVersion->getAsString(Args);
    return VersionTuple();
  }

  if (MSCompatibilityVersion) {
    VersionTuple MSVT;
    if (MSVT.tryParse(MSCompatibilityVersion->getValue())) {
      if (D)
        D->Diag(diag::err_drv_invalid_value)
            << MSCompatibilityVersion->getAsString(Args)
            << MSCompatibilityVersion->getValue();
    } else {
      return MSVT;
    }
  }

  if (MSCVersion) {
    unsigned Version = 0;
    if (StringRef(MSCVersion->getValue()).getAsInteger(10, Version)) {
      if (D)
        D->Diag(diag::err_drv_invalid_value)
            << MSCVersion->getAsString(Args) << MSCVersion->getValue();
    } else {
      return separateMSVCFullVersion(Version);
    }
  }

  return VersionTuple();
}

llvm::opt::DerivedArgList *ToolChain::TranslateOffloadTargetArgs(
    const llvm::opt::DerivedArgList &Args, bool SameTripleAsHost,
    SmallVectorImpl<llvm::opt::Arg *> &AllocatedArgs,
    Action::OffloadKind DeviceOffloadKind) const {
  assert((DeviceOffloadKind == Action::OFK_OpenMP ||
          DeviceOffloadKind == Action::OFK_SYCL) &&
         "requires OpenMP or SYCL offload kind");
  DerivedArgList *DAL = new DerivedArgList(Args.getBaseArgs());
  const OptTable &Opts = getDriver().getOpts();
  bool Modified = false;

  // Handle -Xopenmp-target and -Xsycl-target-frontend flags
  for (auto *A : Args) {
    // Exclude flags which may only apply to the host toolchain.
    // Do not exclude flags when the host triple (AuxTriple)
    // matches the current toolchain triple. If it is not present
    // at all, target and host share a toolchain.
    if (A->getOption().matches(options::OPT_m_Group)) {
      // AMD GPU is a special case, as -mcpu is required for the device
      // compilation, except for SYCL which uses --offload-arch.
      if (SameTripleAsHost || (getTriple().getArch() == llvm::Triple::amdgcn &&
                               DeviceOffloadKind != Action::OFK_SYCL)) {
        DAL->append(A);
        continue;
      }
      // SPIR-V special case for -mlong-double
      if (getTriple().isSPIR() &&
          A->getOption().matches(options::OPT_LongDouble_Group)) {
        DAL->append(A);
        continue;
      }
      Modified = true;
      continue;
    }

    // Exclude -fsycl
#if INTEL_CUSTOMIZATION
    // Keep -fsycl around for OpenMP offload, we use it for SYCL/OpenMP
    // interoperability.
    if (A->getOption().matches(options::OPT_fsycl) &&
        DeviceOffloadKind != Action::OFK_OpenMP) {
#endif // INTEL_CUSTOMIZATION
      Modified = true;
      continue;
    }

    unsigned Index = 0;
    unsigned Prev;
    bool XOffloadTargetNoTriple;

    // TODO: functionality between OpenMP offloading and SYCL offloading
    // is similar, can be improved
    if (DeviceOffloadKind == Action::OFK_OpenMP) {
      XOffloadTargetNoTriple =
        A->getOption().matches(options::OPT_Xopenmp_target);
      if (A->getOption().matches(options::OPT_Xopenmp_target_EQ)) {
        llvm::Triple TT(getOpenMPTriple(A->getValue(0)));

        // Passing device args: -Xopenmp-target=<triple> -opt=val.
        if (TT.getTriple() == getTripleString())
          Index = Args.getBaseArgs().MakeIndex(A->getValue(1));
        else
          continue;
#if INTEL_CUSTOMIZATION
      } else if (A->getOption().matches(options::OPT_fopenmp_targets_EQ)) {
        // Capture options from -fopenmp-targets=<triple>=<opts>
        StringRef Val(A->getValue());
        std::pair<StringRef, StringRef> T = Val.split('=');
        if (!T.second.empty()) {
          // Add the the original -fopenmp-targets option without the additional
          // arguments, then add the arguments.
          Arg *ArgReplace =
              new Arg(A->getOption(), Args.MakeArgString(A->getSpelling()),
                      Args.getBaseArgs().MakeIndex(A->getSpelling()),
                      Args.MakeArgString(T.first));
          std::unique_ptr<llvm::opt::Arg> OffloadTargetArg(ArgReplace);
          OffloadTargetArg->setBaseArg(A);
          Arg *A4 = OffloadTargetArg.release();
          AllocatedArgs.push_back(A4);
          DAL->append(A4);
          Modified = true;
        } else
          DAL->append(A);
        continue;
#endif // INTEL_CUSTOMIZATION
      } else if (XOffloadTargetNoTriple) {
        // Passing device args: -Xopenmp-target -opt=val.
        Index = Args.getBaseArgs().MakeIndex(A->getValue(0));
      } else {
        DAL->append(A);
        continue;
      }
    } else if (DeviceOffloadKind == Action::OFK_SYCL) {
      XOffloadTargetNoTriple =
        A->getOption().matches(options::OPT_Xsycl_frontend);
      if (A->getOption().matches(options::OPT_Xsycl_frontend_EQ)) {
        // Passing device args: -Xsycl-target-frontend=<triple> -opt=val.
        if (getDriver().MakeSYCLDeviceTriple(A->getValue(0)) == getTriple())
          Index = Args.getBaseArgs().MakeIndex(A->getValue(1));
        else
          continue;
      } else if (XOffloadTargetNoTriple) {
        // Passing device args: -Xsycl-target-frontend -opt=val.
        Index = Args.getBaseArgs().MakeIndex(A->getValue(0));
      } else {
        DAL->append(A);
        continue;
      }
    }

    // Parse the argument to -Xopenmp-target.
    Prev = Index;
    std::unique_ptr<Arg> XOffloadTargetArg(Opts.ParseOneArg(Args, Index));
    if (!XOffloadTargetArg || Index > Prev + 1) {
      if (DeviceOffloadKind == Action::OFK_OpenMP) {
        getDriver().Diag(diag::err_drv_invalid_Xopenmp_target_with_args)
            << A->getAsString(Args);
      } else {
        getDriver().Diag(diag::err_drv_invalid_Xsycl_frontend_with_args)
            << A->getAsString(Args);
      }
      continue;
    }
    if (XOffloadTargetNoTriple && XOffloadTargetArg) {
      // TODO: similar behaviors with OpenMP and SYCL offloading, can be
      // improved upon
      auto SingleTargetTripleCount = [&Args](OptSpecifier Opt) {
        const Arg *TargetArg = Args.getLastArg(Opt);
        if (!TargetArg || TargetArg->getValues().size() == 1)
          return true;
        return false;
      };
      if (DeviceOffloadKind == Action::OFK_OpenMP &&
          !SingleTargetTripleCount(options::OPT_fopenmp_targets_EQ)) {
        getDriver().Diag(diag::err_drv_Xopenmp_target_missing_triple);
        continue;
      }
      if (DeviceOffloadKind == Action::OFK_SYCL &&
          !SingleTargetTripleCount(options::OPT_fsycl_targets_EQ)) {
        getDriver().Diag(diag::err_drv_Xsycl_target_missing_triple)
            << A->getSpelling();
        continue;
      }
    }

    if (!XOffloadTargetArg)
      continue;

    XOffloadTargetArg->setBaseArg(A);
    A = XOffloadTargetArg.release();
    AllocatedArgs.push_back(A);
    DAL->append(A);
    Modified = true;
  }
#if INTEL_CUSTOMIZATION
  // -fopenmp-targets=<triple>=<opts> support parsing.
  for (StringRef Val : Args.getAllArgValues(options::OPT_fopenmp_targets_EQ)) {
    // Capture options from -fopenmp-targets=<triple>=<opts>
    std::pair<StringRef, StringRef> T = Val.split('=');
    if (T.second.empty())
      continue;
    // Tokenize the string.
    SmallVector<const char *, 8> TargetArgs;
    llvm::BumpPtrAllocator BPA;
    llvm::StringSaver S(BPA);
    llvm::cl::TokenizeGNUCommandLine(T.second, S, TargetArgs);
    // Setup masks so Windows options aren't picked up for parsing
    // Linux options
    unsigned IncludedFlagsBitmask = 0;
    unsigned ExcludedFlagsBitmask = options::NoDriverOption;
    if (getDriver().IsCLMode()) {
      // Include CL and Core options.
      IncludedFlagsBitmask |= options::CLOption;
      IncludedFlagsBitmask |= options::CoreOption;
    } else
      ExcludedFlagsBitmask |= options::CLOption;
    unsigned MissingArgIndex, MissingArgCount;
    InputArgList NewArgs =
        Opts.ParseArgs(TargetArgs, MissingArgIndex, MissingArgCount,
                       IncludedFlagsBitmask, ExcludedFlagsBitmask);
    for (Arg *NA : NewArgs) {
      // Add the new arguments.
      Arg *OffloadArg;
      if (NA->getNumValues()) {
        StringRef Value(NA->getValue());
        OffloadArg = new Arg(
            NA->getOption(), Args.MakeArgString(NA->getSpelling()),
            Args.getBaseArgs().MakeIndex(NA->getSpelling()),
            Args.MakeArgString(Value.data()));
      } else {
        OffloadArg = new Arg(
            NA->getOption(), Args.MakeArgString(NA->getSpelling()),
            Args.getBaseArgs().MakeIndex(NA->getSpelling()));
      }
      DAL->append(OffloadArg);
      Modified = true;
    }
  }
#endif // INTEL_CUSTOMIZATION
  if (Modified)
    return DAL;

  delete DAL;
  return nullptr;
}

// TODO: Currently argument values separated by space e.g.
// -Xclang -mframe-pointer=no cannot be passed by -Xarch_. This should be
// fixed.
void ToolChain::TranslateXarchArgs(
    const llvm::opt::DerivedArgList &Args, llvm::opt::Arg *&A,
    llvm::opt::DerivedArgList *DAL,
    SmallVectorImpl<llvm::opt::Arg *> *AllocatedArgs) const {
  const OptTable &Opts = getDriver().getOpts();
  unsigned ValuePos = 1;
  if (A->getOption().matches(options::OPT_Xarch_device) ||
      A->getOption().matches(options::OPT_Xarch_host))
    ValuePos = 0;

  unsigned Index = Args.getBaseArgs().MakeIndex(A->getValue(ValuePos));
  unsigned Prev = Index;
  std::unique_ptr<llvm::opt::Arg> XarchArg(Opts.ParseOneArg(Args, Index));

  // If the argument parsing failed or more than one argument was
  // consumed, the -Xarch_ argument's parameter tried to consume
  // extra arguments. Emit an error and ignore.
  //
  // We also want to disallow any options which would alter the
  // driver behavior; that isn't going to work in our model. We
  // use options::NoXarchOption to control this.
  if (!XarchArg || Index > Prev + 1) {
    getDriver().Diag(diag::err_drv_invalid_Xarch_argument_with_args)
        << A->getAsString(Args);
    return;
  } else if (XarchArg->getOption().hasFlag(options::NoXarchOption)) {
    auto &Diags = getDriver().getDiags();
    unsigned DiagID =
        Diags.getCustomDiagID(DiagnosticsEngine::Error,
                              "invalid Xarch argument: '%0', not all driver "
                              "options can be forwared via Xarch argument");
    Diags.Report(DiagID) << A->getAsString(Args);
    return;
  }
  XarchArg->setBaseArg(A);
  A = XarchArg.release();
  if (!AllocatedArgs)
    DAL->AddSynthesizedArg(A);
  else
    AllocatedArgs->push_back(A);
}

llvm::opt::DerivedArgList *ToolChain::TranslateXarchArgs(
    const llvm::opt::DerivedArgList &Args, StringRef BoundArch,
    Action::OffloadKind OFK,
    SmallVectorImpl<llvm::opt::Arg *> *AllocatedArgs) const {
  DerivedArgList *DAL = new DerivedArgList(Args.getBaseArgs());
  bool Modified = false;

  bool IsGPU = OFK == Action::OFK_Cuda || OFK == Action::OFK_HIP;
  for (Arg *A : Args) {
    bool NeedTrans = false;
    bool Skip = false;
    if (A->getOption().matches(options::OPT_Xarch_device)) {
      NeedTrans = IsGPU;
      Skip = !IsGPU;
    } else if (A->getOption().matches(options::OPT_Xarch_host)) {
      NeedTrans = !IsGPU;
      Skip = IsGPU;
    } else if (A->getOption().matches(options::OPT_Xarch__) && IsGPU) {
      // Do not translate -Xarch_ options for non CUDA/HIP toolchain since
      // they may need special translation.
      // Skip this argument unless the architecture matches BoundArch
      if (BoundArch.empty() || A->getValue(0) != BoundArch)
        Skip = true;
      else
        NeedTrans = true;
    }
    if (NeedTrans || Skip)
      Modified = true;
    if (NeedTrans)
      TranslateXarchArgs(Args, A, DAL, AllocatedArgs);
    if (!Skip)
      DAL->append(A);
  }

  if (Modified)
    return DAL;

  delete DAL;
  return nullptr;
}

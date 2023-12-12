//===--- SYCL.cpp - SYCL Tool and ToolChain Implementations -----*- C++ -*-===//
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
#include "SYCL.h"
#include "CommonArgs.h"
#include "clang/Driver/Action.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/InputInfo.h"
#include "clang/Driver/Options.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include <sstream>

#if INTEL_CUSTOMIZATION
#include "llvm/Support/Process.h"
#endif // INTEL_CUSTOMIZATION

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

SYCLInstallationDetector::SYCLInstallationDetector(const Driver &D)
    : D(D), InstallationCandidates() {
  InstallationCandidates.emplace_back(D.Dir + "/..");
#if INTEL_DEPLOY_UNIFIED_LAYOUT
  InstallationCandidates.emplace_back(D.Dir + "/../..");
#endif // INTEL_DEPLOY_UNIFIED_LAYOUT
}

void SYCLInstallationDetector::getSYCLDeviceLibPath(
    llvm::SmallVector<llvm::SmallString<128>, 4> &DeviceLibPaths) const {
  for (const auto &IC : InstallationCandidates) {
    llvm::SmallString<128> InstallLibPath(IC.str());
    InstallLibPath.append("/lib");
    DeviceLibPaths.emplace_back(InstallLibPath);
#if INTEL_DEPLOY_UNIFIED_LAYOUT
    llvm::SmallString<128> InstallLibPathUnified(IC.str());
    llvm::sys::path::append(InstallLibPathUnified, "opt", "compiler", "lib");
    DeviceLibPaths.emplace_back(InstallLibPathUnified);
#endif // INTEL_DEPLOY_UNIFIED_LAYOUT
  }

  DeviceLibPaths.emplace_back(D.SysRoot + "/lib");
}

void SYCLInstallationDetector::print(llvm::raw_ostream &OS) const {
  if (!InstallationCandidates.size())
    return;
  OS << "SYCL Installation Candidates: \n";
  for (const auto &IC : InstallationCandidates) {
    OS << IC << "\n";
  }
}

static void addFPGATimingDiagnostic(std::unique_ptr<Command> &Cmd,
                                    Compilation &C) {
  const char *Msg = C.getArgs().MakeArgString(
      "The FPGA image generated during this compile contains timing violations "
      "and may produce functional errors if used. Refer to the Intel oneAPI "
      "DPC++ FPGA Optimization Guide section on Timing Failures for more "
      "information.");
  Cmd->addDiagForErrorCode(/*ErrorCode*/ 42, Msg);
  Cmd->addExitForErrorCode(/*ErrorCode*/ 42, false);
}

void SYCL::constructLLVMForeachCommand(Compilation &C, const JobAction &JA,
                                       std::unique_ptr<Command> InputCommand,
                                       const InputInfoList &InputFiles,
                                       const InputInfo &Output, const Tool *T,
                                       StringRef Increment, StringRef Ext,
                                       StringRef ParallelJobs) {
  // Construct llvm-foreach command.
  // The llvm-foreach command looks like this:
  // llvm-foreach --in-file-list=a.list --in-replace='{}' -- echo '{}'
  ArgStringList ForeachArgs;
  std::string OutputFileName(T->getToolChain().getInputFilename(Output));
  ForeachArgs.push_back(C.getArgs().MakeArgString("--out-ext=" + Ext));
  for (auto &I : InputFiles) {
    std::string Filename(T->getToolChain().getInputFilename(I));
    ForeachArgs.push_back(
        C.getArgs().MakeArgString("--in-file-list=" + Filename));
    ForeachArgs.push_back(
        C.getArgs().MakeArgString("--in-replace=" + Filename));
  }

  ForeachArgs.push_back(
      C.getArgs().MakeArgString("--out-file-list=" + OutputFileName));
  ForeachArgs.push_back(
      C.getArgs().MakeArgString("--out-replace=" + OutputFileName));
  if (!Increment.empty())
    ForeachArgs.push_back(
        C.getArgs().MakeArgString("--out-increment=" + Increment));
  if (!ParallelJobs.empty())
    ForeachArgs.push_back(C.getArgs().MakeArgString("--jobs=" + ParallelJobs));

  if (C.getDriver().isSaveTempsEnabled()) {
    SmallString<128> OutputDirName;
    if (C.getDriver().isSaveTempsObj()) {
      OutputDirName =
          T->getToolChain().GetFilePath(OutputFileName.c_str()).c_str();
      llvm::sys::path::remove_filename(OutputDirName);
    }
    // Use the current dir if the `GetFilePath` returned en empty string, which
    // is the case when the `OutputFileName` does not contain any directory
    // information, or if in CWD mode. This is necessary for `llvm-foreach`, as
    // it would disregard the parameter without it. Otherwise append separator.
    if (OutputDirName.empty())
      llvm::sys::path::native(OutputDirName = "./");
    else
      OutputDirName.append(llvm::sys::path::get_separator());
    ForeachArgs.push_back(
        C.getArgs().MakeArgString("--out-dir=" + OutputDirName));
  }

  ForeachArgs.push_back(C.getArgs().MakeArgString("--"));
  ForeachArgs.push_back(
      C.getArgs().MakeArgString(InputCommand->getExecutable()));

  for (auto &Arg : InputCommand->getArguments())
    ForeachArgs.push_back(Arg);

  SmallString<128> ForeachPath(C.getDriver().Dir);
  llvm::sys::path::append(ForeachPath, "llvm-foreach");
  const char *Foreach = C.getArgs().MakeArgString(ForeachPath);

  auto Cmd = std::make_unique<Command>(JA, *T, ResponseFileSupport::None(),
                                       Foreach, ForeachArgs, std::nullopt);
  // FIXME: Add the FPGA specific timing diagnostic to the foreach call.
  // The foreach call obscures the return codes from the tool it is calling
  // to the compiler itself.
  addFPGATimingDiagnostic(Cmd, C);
  C.addCommand(std::move(Cmd));
}

bool SYCL::shouldDoPerObjectFileLinking(const Compilation &C) {
  return !C.getArgs().hasFlag(options::OPT_fgpu_rdc, options::OPT_fno_gpu_rdc,
                              /*default=*/true);
}

// Return whether to use native bfloat16 library.
static bool selectBfloatLibs(const llvm::Triple &Triple, const Compilation &C,
                             bool &UseNative) {
  const llvm::opt::ArgList &Args = C.getArgs();
  bool NeedLibs = false;

  // spir64 target is actually JIT compilation, so we defer selection of
  // bfloat16 libraries to runtime. For AOT we need libraries, but skip
  // for Nvidia.
  NeedLibs =
      Triple.getSubArch() != llvm::Triple::NoSubArch && !Triple.isNVPTX();
  UseNative = false;
  if (NeedLibs && Triple.getSubArch() == llvm::Triple::SPIRSubArch_gen &&
      C.hasOffloadToolChain<Action::OFK_SYCL>()) {
    ArgStringList TargArgs;
    auto ToolChains = C.getOffloadToolChains<Action::OFK_SYCL>();
    // Match up the toolchain with the incoming Triple so we are grabbing the
    // expected arguments to scrutinize.
    for (auto TI = ToolChains.first, TE = ToolChains.second; TI != TE; ++TI) {
      llvm::Triple SYCLTriple = TI->second->getTriple();
      if (SYCLTriple == Triple) {
        const toolchains::SYCLToolChain *SYCLTC =
            static_cast<const toolchains::SYCLToolChain *>(TI->second);
        SYCLTC->TranslateBackendTargetArgs(Action::OFK_SYCL, Triple, Args, TargArgs);
        break;
      }
    }

    auto checkBF = [](StringRef Device) {
      return Device.starts_with("pvc") || Device.starts_with("ats");
    };

    std::string Params;
    for (const auto &Arg : TargArgs) {
      Params += " ";
      Params += Arg;
    }
    size_t DevicesPos = Params.find("-device ");
    UseNative = false;
    if (DevicesPos != std::string::npos) {
      UseNative = true;
      std::istringstream Devices(Params.substr(DevicesPos + 8));
      for (std::string S; std::getline(Devices, S, ',');)
        UseNative &= checkBF(S);
    }
  }
  return NeedLibs;
}

SmallVector<std::string, 8>
SYCL::getDeviceLibraries(const Compilation &C, const llvm::Triple &TargetTriple,
                         bool IsSpirvAOT) {
  SmallVector<std::string, 8> LibraryList;
  const llvm::opt::ArgList &Args = C.getArgs();

  struct DeviceLibOptInfo {
    StringRef DeviceLibName;
    StringRef DeviceLibOption;
  };

  bool NoDeviceLibs = false;
  // Currently, all SYCL device libraries will be linked by default. Linkage
  // of "internal" libraries cannot be affected via -fno-sycl-device-lib.
  llvm::StringMap<bool> DeviceLibLinkInfo = {
      {"libc", true},          {"libm-fp32", true},   {"libm-fp64", true},
      {"libimf-fp32", true},   {"libimf-fp64", true}, {"libimf-bf16", true},
      {"libm-bfloat16", true}, {"internal", true}};
  if (Arg *A = Args.getLastArg(options::OPT_fsycl_device_lib_EQ,
                               options::OPT_fno_sycl_device_lib_EQ)) {
    if (A->getValues().size() == 0)
      C.getDriver().Diag(diag::warn_drv_empty_joined_argument)
          << A->getAsString(Args);
    else {
      if (A->getOption().matches(options::OPT_fno_sycl_device_lib_EQ))
        NoDeviceLibs = true;

      for (StringRef Val : A->getValues()) {
        if (Val == "all") {
          for (const auto &K : DeviceLibLinkInfo.keys())
            DeviceLibLinkInfo[K] =
                true && (!NoDeviceLibs || K.equals("internal"));
          break;
        }
        auto LinkInfoIter = DeviceLibLinkInfo.find(Val);
        if (LinkInfoIter == DeviceLibLinkInfo.end() || Val.equals("internal")) {
          // TODO: Move the diagnostic to the SYCL section of
          // Driver::CreateOffloadingDeviceToolChains() to minimize code
          // duplication.
          C.getDriver().Diag(diag::err_drv_unsupported_option_argument)
              << A->getSpelling() << Val;
        }
        DeviceLibLinkInfo[Val] = true && !NoDeviceLibs;
      }
    }
  }
  StringRef LibSuffix =
      C.getDefaultToolChain().getTriple().isWindowsMSVCEnvironment() ? ".obj"
                                                                     : ".o";
  using SYCLDeviceLibsList = SmallVector<DeviceLibOptInfo, 5>;

  const SYCLDeviceLibsList SYCLDeviceWrapperLibs = {
      {"libsycl-crt", "libc"},
      {"libsycl-complex", "libm-fp32"},
      {"libsycl-complex-fp64", "libm-fp64"},
      {"libsycl-cmath", "libm-fp32"},
      {"libsycl-cmath-fp64", "libm-fp64"},
#if defined(_WIN32)
      {"libsycl-msvc-math", "libm-fp32"},
#endif
      {"libsycl-imf", "libimf-fp32"},
      {"libsycl-imf-fp64", "libimf-fp64"},
      {"libsycl-imf-bf16", "libimf-bf16"}};
  // For AOT compilation, we need to link sycl_device_fallback_libs as
  // default too.
  const SYCLDeviceLibsList SYCLDeviceFallbackLibs = {
      {"libsycl-fallback-cassert", "libc"},
      {"libsycl-fallback-cstring", "libc"},
      {"libsycl-fallback-complex", "libm-fp32"},
      {"libsycl-fallback-complex-fp64", "libm-fp64"},
      {"libsycl-fallback-cmath", "libm-fp32"},
      {"libsycl-fallback-cmath-fp64", "libm-fp64"},
      {"libsycl-fallback-imf", "libimf-fp32"},
      {"libsycl-fallback-imf-fp64", "libimf-fp64"},
      {"libsycl-fallback-imf-bf16", "libimf-bf16"}};
  const SYCLDeviceLibsList SYCLDeviceBfloat16FallbackLib = {
      {"libsycl-fallback-bfloat16", "libm-bfloat16"}};
  const SYCLDeviceLibsList SYCLDeviceBfloat16NativeLib = {
      {"libsycl-native-bfloat16", "libm-bfloat16"}};
  // ITT annotation libraries are linked in separately whenever the device
  // code instrumentation is enabled.
  const SYCLDeviceLibsList SYCLDeviceAnnotationLibs = {
      {"libsycl-itt-user-wrappers", "internal"},
      {"libsycl-itt-compiler-wrappers", "internal"},
      {"libsycl-itt-stubs", "internal"}};
#if !defined(_WIN32)
  const SYCLDeviceLibsList SYCLDeviceSanitizerLibs = {
      {"libsycl-sanitizer", "internal"}};
#endif

  auto addLibraries = [&](const SYCLDeviceLibsList &LibsList) {
    for (const DeviceLibOptInfo &Lib : LibsList) {
      if (!DeviceLibLinkInfo[Lib.DeviceLibOption])
        continue;
      SmallString<128> LibName(Lib.DeviceLibName);
      llvm::sys::path::replace_extension(LibName, LibSuffix);
      LibraryList.push_back(Args.MakeArgString(LibName));
    }
  };

  addLibraries(SYCLDeviceWrapperLibs);
  if (IsSpirvAOT || TargetTriple.isNVPTX())
    addLibraries(SYCLDeviceFallbackLibs);

  bool NativeBfloatLibs;
  bool NeedBfloatLibs = selectBfloatLibs(TargetTriple, C, NativeBfloatLibs);
  if (NeedBfloatLibs) {
    // Add native or fallback bfloat16 library.
    if (NativeBfloatLibs)
      addLibraries(SYCLDeviceBfloat16NativeLib);
    else
      addLibraries(SYCLDeviceBfloat16FallbackLib);
  }

  if (Args.hasFlag(options::OPT_fsycl_instrument_device_code,
                   options::OPT_fno_sycl_instrument_device_code, true))
    addLibraries(SYCLDeviceAnnotationLibs);

#if !defined(_WIN32)
  if (Arg *A = Args.getLastArg(options::OPT_fsanitize_EQ,
                               options::OPT_fno_sanitize_EQ)) {
    if (A->getOption().matches(options::OPT_fsanitize_EQ) &&
        A->getValues().size() == 1) {
      std::string SanitizeVal = A->getValue();
      if (SanitizeVal == "address")
        addLibraries(SYCLDeviceSanitizerLibs);
    }
  }
#endif
  return LibraryList;
}

// The list should match pre-built SYCL device library files located in
// compiler package. Once we add or remove any SYCL device library files,
// the list should be updated accordingly.
static llvm::SmallVector<StringRef, 16> SYCLDeviceLibList {
  "bfloat16", "crt", "cmath", "cmath-fp64", "complex", "complex-fp64",
#if defined(_WIN32)
      "msvc-math",
#endif
      "imf", "imf-fp64", "itt-compiler-wrappers", "itt-stubs",
      "itt-user-wrappers", "fallback-cassert", "fallback-cstring",
      "fallback-cmath", "fallback-cmath-fp64", "fallback-complex",
      "fallback-complex-fp64", "fallback-imf", "fallback-imf-fp64",
      "fallback-imf-bf16", "fallback-bfloat16", "native-bfloat16"
};

#if INTEL_CUSTOMIZATION
// The list should match pre-built OMP device library files located in
// compiler package. Once we add or remove any OMP device library files,
// the list should be updated accordingly.
// The spirvdevicertl library is not included here as it is required to
//  be linked in fully (without --only-needed).
// Some of the libraries are being linked conditionally (only device-svml
// for now) and we need to check that when considering whether to add
// one to the final list.
static llvm::SmallVector<
    std::pair<StringRef, std::function<bool(const ArgList &)>>, 10>
    OMPDeviceLibList{
        {"cmath", [](const ArgList &Args) { return true; }},
        {"cmath-fp64", [](const ArgList &Args) { return true; }},
        {"complex", [](const ArgList &Args) { return true; }},
        {"complex-fp64", [](const ArgList &Args) { return true; }},
        {"fallback-cassert", [](const ArgList &Args) { return true; }},
        {"fallback-cstring", [](const ArgList &Args) { return true; }},
        {"fallback-cmath", [](const ArgList &Args) { return true; }},
        {"fallback-cmath-fp64", [](const ArgList &Args) { return true; }},
        {"fallback-complex", [](const ArgList &Args) { return true; }},
        {"fallback-complex-fp64", [](const ArgList &Args) { return true; }},
        {"device-svml",
         [](const ArgList &Args) {
           return Args.hasArg(options::OPT_fopenmp_target_simd);
         }},
        {"itt-compiler-wrappers", [](const ArgList &Args) { return true; }},
        {"itt-stubs", [](const ArgList &Args) { return true; }},
        {"itt-user-wrappers", [](const ArgList &Args) { return true; }}};
#endif // INTEL_CUSTOMIZATION

const char *SYCL::Linker::constructLLVMLinkCommand(
    Compilation &C, const JobAction &JA, const InputInfo &Output,
    const ArgList &Args, StringRef SubArchName, StringRef OutputFilePrefix,
    const InputInfoList &InputFiles) const {
  // Split inputs into libraries which have 'archive' type and other inputs
  // which can be either objects or list files. Object files are linked together
  // in a usual way, but the libraries/list files need to be linked differently.
  // We need to fetch only required symbols from the libraries. With the current
  // llvm-link command line interface that can be achieved with two step
  // linking: at the first step we will link objects into an intermediate
  // partially linked image which on the second step will be linked with the
  // libraries with --only-needed option.
  ArgStringList Opts;
  ArgStringList Objs;
  ArgStringList Libs;
  ArgStringList OMPObjs;      //INTEL

  // Add the input bc's created by compile step.
  // When offloading, the input file(s) could be from unbundled partially
  // linked archives.  The unbundled information is a list of files and not
  // an actual object/archive.  Take that list and pass those to the linker
  // instead of the original object.
  if (JA.isDeviceOffloading(Action::OFK_SYCL) ||   // INTEL
      JA.isDeviceOffloading(Action::OFK_OpenMP)) { // INTEL
    bool IsRDC = !shouldDoPerObjectFileLinking(C);
    const bool IsSYCLNativeCPU = isSYCLNativeCPU(
        this->getToolChain(), *C.getSingleOffloadToolChain<Action::OFK_Host>());
    auto isNoRDCDeviceCodeLink = [&](const InputInfo &II) {
      if (IsRDC)
        return false;
      if (II.getType() != clang::driver::types::TY_LLVM_BC)
        return false;
      if (InputFiles.size() != 2)
        return false;
      return &II == &InputFiles[1];
    };
    auto isSYCLDeviceLib = [&](const InputInfo &II) {
      const ToolChain *HostTC = C.getSingleOffloadToolChain<Action::OFK_Host>();
      StringRef LibPostfix = ".o";
      if (HostTC->getTriple().isWindowsMSVCEnvironment() &&
          C.getDriver().IsCLMode())
        LibPostfix = ".obj";
      else if (isNoRDCDeviceCodeLink(II))
        LibPostfix = ".bc";

      std::string FileName = this->getToolChain().getInputFilename(II);
      StringRef InputFilename = llvm::sys::path::filename(FileName);
      const bool IsNVPTX = this->getToolChain().getTriple().isNVPTX();
      if (IsNVPTX || IsSYCLNativeCPU) {
        // Linking SYCL Device libs requires libclc as well as libdevice
        if ((InputFilename.find("libspirv") != InputFilename.npos ||
             InputFilename.find("libdevice") != InputFilename.npos))
          return true;
        if (IsNVPTX)
          LibPostfix = ".cubin";
      }
      StringRef LibSyclPrefix("libsycl-");
      if (!InputFilename.startswith(LibSyclPrefix) ||
          !InputFilename.endswith(LibPostfix) || (InputFilename.count('-') < 2))
        return false;
      // Skip the prefix "libsycl-"
      std::string PureLibName =
          InputFilename.substr(LibSyclPrefix.size()).str();
      if (isNoRDCDeviceCodeLink(II)) {
        // Skip the final - until the . because we linked all device libs into a
        // single BC in a previous action so we have a temp file name.
        auto FinalDashPos = PureLibName.find_last_of('-');
        auto DotPos = PureLibName.find_last_of('.');
        assert((FinalDashPos != std::string::npos &&
                DotPos != std::string::npos) &&
               "Unexpected filename");
        PureLibName =
            PureLibName.substr(0, FinalDashPos) + PureLibName.substr(DotPos);
      }
      for (const auto &L : SYCLDeviceLibList) {
        if (StringRef(PureLibName).startswith(L))
          return true;
      }
      return false;
    };
#if INTEL_CUSTOMIZATION
    auto isOMPDeviceLib = [&C, &Args](const InputInfo &II) {
      const ToolChain *HostTC = C.getSingleOffloadToolChain<Action::OFK_Host>();
      bool IsMSVC = HostTC->getTriple().isWindowsMSVCEnvironment();
      StringRef InputFilename =
          llvm::sys::path::filename(StringRef(II.getFilename()));
      if (!InputFilename.startswith("libomp-") ||
          (InputFilename.count('-') < 2))
        return false;
      size_t PureLibNameLen = InputFilename.find_last_of('-');
      // Skip the prefix "libomp-"
      StringRef PureLibName = InputFilename.substr(7, PureLibNameLen - 7);
      for (const auto &[Lib, Check] : OMPDeviceLibList) {
        if (PureLibName.startswith(Lib) && Check(Args))
          return true;
      }
      // Do a separate check for the CRT device lib, as it is a different name
      // depending on the target OS.
      StringRef LibCName = IsMSVC ? "msvc" : "glibc";
      if (PureLibName.compare(LibCName) == 0)
        return true;
      return false;
    };
#endif // INTEL_CUSTOMIZATION
    size_t InputFileNum = InputFiles.size();
    bool LinkSYCLDeviceLibs = (InputFileNum >= 2);
    LinkSYCLDeviceLibs = LinkSYCLDeviceLibs && !isSYCLDeviceLib(InputFiles[0]);
    for (size_t Idx = 1; Idx < InputFileNum; ++Idx)
      LinkSYCLDeviceLibs =
          LinkSYCLDeviceLibs && isSYCLDeviceLib(InputFiles[Idx]);
    // Go through the Inputs to the link.  When a listfile is encountered, we
    // know it is an unbundled generated list.
    if (LinkSYCLDeviceLibs) {
      Opts.push_back("-only-needed");
    }
    for (const auto &II : InputFiles) {
      std::string FileName = getToolChain().getInputFilename(II);

#if INTEL_CUSTOMIZATION
      if (isOMPDeviceLib(II)) {
        OMPObjs.push_back(II.getFilename());
      } else if (II.getType() == types::TY_Tempfilelist) {
#endif // INTEL_CUSTOMIZATION
        if (IsRDC) {
          // Pass the unbundled list with '@' to be processed.
          Libs.push_back(C.getArgs().MakeArgString("@" + FileName));
        } else {
          assert(InputFiles.size() == 2 &&
                 "Unexpected inputs for no-RDC with temp file list");
          // If we're in no-RDC mode and the input is a temp file list,
          // we want to link multiple object files each against device libs,
          // so we should consider this input as an object and not pass '@'.
          Objs.push_back(C.getArgs().MakeArgString(FileName));
        }
#if INTEL_CUSTOMIZATION
      } else if (isOMPDeviceLib(II)) {
        OMPObjs.push_back(II.getFilename());
#endif // INTEL_CUSTOMIZATION
      } else if (II.getType() == types::TY_Archive && !LinkSYCLDeviceLibs) {
        Libs.push_back(C.getArgs().MakeArgString(FileName));
      } else
        Objs.push_back(C.getArgs().MakeArgString(FileName));
    }
  } else
    for (const auto &II : InputFiles)
      Objs.push_back(
          C.getArgs().MakeArgString(getToolChain().getInputFilename(II)));

  // Get llvm-link path.
  SmallString<128> ExecPath(C.getDriver().Dir);
  llvm::sys::path::append(ExecPath, "llvm-link");
  const char *Exec = C.getArgs().MakeArgString(ExecPath);

  auto AddLinkCommand = [this, &C, &JA, Exec](const char *Output,
                                              const ArgStringList &Inputs,
                                              const ArgStringList &Options) {
    ArgStringList CmdArgs;
    llvm::copy(Options, std::back_inserter(CmdArgs));
    llvm::copy(Inputs, std::back_inserter(CmdArgs));
    CmdArgs.push_back("-o");
    CmdArgs.push_back(Output);
    // TODO: temporary workaround for a problem with warnings reported by
    // llvm-link when driver links LLVM modules with empty modules
    CmdArgs.push_back("--suppress-warnings");
    C.addCommand(std::make_unique<Command>(JA, *this,
                                           ResponseFileSupport::AtFileUTF8(),
                                           Exec, CmdArgs, std::nullopt));
  };

  // Add an intermediate output file.
  const char *OutputFileName =
      C.getArgs().MakeArgString(getToolChain().getInputFilename(Output));

#if INTEL_CUSTOMIZATION
  const char *TOutputFileName = OutputFileName;

  // Use a Temporary output file for OMP devices else use the Output file
  // provided
  if (!OMPObjs.empty()) {
    std::string OMPTempFile;
    OMPTempFile = C.getDriver().GetTemporaryPath(
        OutputFilePrefix.str() + "-linkomp", "bc");
    TOutputFileName = C.addTempFile(C.getArgs().MakeArgString(OMPTempFile));
  }
#endif // INTEL_CUSTOMIZATION

  if (Libs.empty())
    AddLinkCommand(TOutputFileName, Objs, Opts);  //INTEL
  else {
    assert(Opts.empty() && "unexpected options");

    // Linker will be invoked twice if inputs contain libraries. First time we
    // will link input objects into an intermediate temporary file, and on the
    // second invocation intermediate temporary object will be linked with the
    // libraries, but now only required symbols will be added to the final
    // output.
    std::string TempFile =
        C.getDriver().GetTemporaryPath(OutputFilePrefix.str() + "-link", "bc");
    const char *LinkOutput = C.addTempFile(C.getArgs().MakeArgString(TempFile));
    AddLinkCommand(LinkOutput, Objs, {});

    // Now invoke linker for the second time to link required symbols from the
    // input libraries.
    ArgStringList LinkInputs{LinkOutput};
    llvm::copy(Libs, std::back_inserter(LinkInputs));
    AddLinkCommand(TOutputFileName, LinkInputs, {"--only-needed"});  //INTEL
  }
#if INTEL_CUSTOMIZATION
  if (!OMPObjs.empty()) {
    ArgStringList OMPLinkInputs{TOutputFileName};
    llvm::copy(OMPObjs, std::back_inserter(OMPLinkInputs));
    AddLinkCommand(OutputFileName, OMPLinkInputs, {"--only-needed"});
    return OutputFileName;
  }
  return TOutputFileName;
#endif // INTEL_CUSTOMIZATION
}

void SYCL::Linker::constructLlcCommand(Compilation &C, const JobAction &JA,
                                       const InputInfo &Output,
                                       const char *InputFileName) const {
  // Construct llc command.
  // The output is an object file.
  ArgStringList LlcArgs{"-filetype=obj", "-o", Output.getFilename(),
                        InputFileName};
  SmallString<128> LlcPath(C.getDriver().Dir);
  llvm::sys::path::append(LlcPath, "llc");
  const char *Llc = C.getArgs().MakeArgString(LlcPath);
  C.addCommand(std::make_unique<Command>(JA, *this,
                                         ResponseFileSupport::AtFileUTF8(), Llc,
                                         LlcArgs, std::nullopt));
}

// For SYCL the inputs of the linker job are SPIR-V binaries and output is
// a single SPIR-V binary.  Input can also be bitcode when specified by
// the user.
void SYCL::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                const InputInfo &Output,
                                const InputInfoList &Inputs,
                                const ArgList &Args,
                                const char *LinkingOutput) const {

  assert((getToolChain().getTriple().isSPIR() ||
          getToolChain().getTriple().isNVPTX() ||
          getToolChain().getTriple().isAMDGCN() || isSYCLNativeCPU(Args)) &&
         "Unsupported target");

  std::string SubArchName =
      std::string(getToolChain().getTriple().getArchName());

  // Prefix for temporary file name.
  std::string Prefix = std::string(llvm::sys::path::stem(SubArchName));

  // For CUDA, we want to link all BC files before resuming the normal
  // compilation path
  if (getToolChain().getTriple().isNVPTX() ||
      getToolChain().getTriple().isAMDGCN()) {
    InputInfoList NvptxInputs;
    for (const auto &II : Inputs) {
      if (!II.isFilename())
        continue;
      NvptxInputs.push_back(II);
    }

    constructLLVMLinkCommand(C, JA, Output, Args, SubArchName, Prefix,
                             NvptxInputs);
    return;
  }

  InputInfoList SpirvInputs;
  for (const auto &II : Inputs) {
    if (!II.isFilename())
      continue;
    SpirvInputs.push_back(II);
  }

  constructLLVMLinkCommand(C, JA, Output, Args, SubArchName, Prefix,
                           SpirvInputs);
}

static const char *makeExeName(Compilation &C, StringRef Name) {
  llvm::SmallString<8> ExeName(Name);
  const ToolChain *HostTC = C.getSingleOffloadToolChain<Action::OFK_Host>();
  if (HostTC->getTriple().isWindowsMSVCEnvironment())
    ExeName.append(".exe");
  return C.getArgs().MakeArgString(ExeName);
}

void SYCL::fpga::BackendCompiler::constructOpenCLAOTCommand(
    Compilation &C, const JobAction &JA, const InputInfo &Output,
    const InputInfoList &Inputs, const ArgList &Args) const {
  // Construct opencl-aot command. This is used for FPGA AOT compilations
  // when performing emulation.  Input file will be a SPIR-V binary which
  // will be compiled to an aocx file.
  InputInfoList ForeachInputs;
  InputInfoList FPGADepFiles;
  ArgStringList CmdArgs{"-device=fpga_fast_emu"};

  for (const auto &II : Inputs) {
    if (II.getType() == types::TY_TempAOCOfilelist ||
        II.getType() == types::TY_FPGA_Dependencies ||
        II.getType() == types::TY_FPGA_Dependencies_List)
      continue;
    if (II.getType() == types::TY_Tempfilelist)
      ForeachInputs.push_back(II);
    CmdArgs.push_back(
        C.getArgs().MakeArgString("-spv=" + Twine(II.getFilename())));
  }
  CmdArgs.push_back(
      C.getArgs().MakeArgString("-ir=" + Twine(Output.getFilename())));

  StringRef ForeachExt = "aocx";
  if (Arg *A = Args.getLastArg(options::OPT_fsycl_link_EQ))
    if (A->getValue() == StringRef("early"))
      ForeachExt = "aocr";

  // Add any implied arguments before user defined arguments.
  Action::OffloadKind DeviceOffloadKind(JA.getOffloadingDeviceKind()); // INTEL
  const toolchains::SYCLToolChain &TC =
      static_cast<const toolchains::SYCLToolChain &>(getToolChain());
  llvm::Triple CPUTriple("spir64_x86_64");
#if INTEL_CUSTOMIZATION
  const ToolChain *HostTC = C.getSingleOffloadToolChain<Action::OFK_Host>();
  TC.AddImpliedTargetArgs(DeviceOffloadKind, CPUTriple, Args, CmdArgs, JA,
                          *HostTC);
#endif // INTEL_CUSTOMIZATION
  // Add the target args passed in
#if INTEL_CUSTOMIZATION
  TC.TranslateBackendTargetArgs(DeviceOffloadKind, CPUTriple, Args, CmdArgs);
  TC.TranslateLinkerTargetArgs(DeviceOffloadKind, CPUTriple, Args, CmdArgs);
#endif // INTEL_CUSTOMIZATION

  SmallString<128> ExecPath(
      getToolChain().GetProgramPath(makeExeName(C, "opencl-aot")));
  const char *Exec = C.getArgs().MakeArgString(ExecPath);
  auto Cmd = std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                       Exec, CmdArgs, std::nullopt);
  if (!ForeachInputs.empty()) {
    StringRef ParallelJobs =
#if INTEL_CUSTOMIZATION
        Args.getLastArgValue(DeviceOffloadKind == Action::OFK_SYCL
                                 ? options::OPT_fsycl_max_parallel_jobs_EQ
                                 : options::OPT_fopenmp_max_parallel_jobs_EQ);
#endif // INTEL_CUSTOMIZATION
    constructLLVMForeachCommand(C, JA, std::move(Cmd), ForeachInputs, Output,
                                this, "", ForeachExt, ParallelJobs);
  } else
    C.addCommand(std::move(Cmd));
}

void SYCL::fpga::BackendCompiler::ConstructJob(
    Compilation &C, const JobAction &JA, const InputInfo &Output,
    const InputInfoList &Inputs, const ArgList &Args,
    const char *LinkingOutput) const {
  assert((getToolChain().getTriple().getArch() == llvm::Triple::spir ||
          getToolChain().getTriple().getArch() == llvm::Triple::spir64) &&
         "Unsupported target");

  // Grab the -Xsycl-target* options.
  Action::OffloadKind DeviceOffloadKind(JA.getOffloadingDeviceKind()); // INTEL
  const toolchains::SYCLToolChain &TC =
      static_cast<const toolchains::SYCLToolChain &>(getToolChain());
  ArgStringList TargetArgs;
  TC.TranslateBackendTargetArgs(DeviceOffloadKind, TC.getTriple(), // INTEL
                                Args, TargetArgs);                 // INTEL

  // When performing emulation compilations for FPGA AOT, we want to use
  // opencl-aot instead of aoc.
  if (C.getDriver().IsFPGAEmulationMode()) {
    constructOpenCLAOTCommand(C, JA, Output, Inputs, Args);
    return;
  }

  InputInfoList ForeachInputs;
  InputInfoList FPGADepFiles;
  StringRef CreatedReportName;
  ArgStringList CmdArgs{"-o", Output.getFilename()};
  for (const auto &II : Inputs) {
    std::string Filename(II.getFilename());
    if (II.getType() == types::TY_Tempfilelist)
      ForeachInputs.push_back(II);
    if (II.getType() == types::TY_TempAOCOfilelist)
      // Add any FPGA library lists.  These come in as special tempfile lists.
      CmdArgs.push_back(Args.MakeArgString(Twine("-library-list=") + Filename));
    else if (II.getType() == types::TY_FPGA_Dependencies ||
             II.getType() == types::TY_FPGA_Dependencies_List)
      FPGADepFiles.push_back(II);
    else
      CmdArgs.push_back(C.getArgs().MakeArgString(Filename));
    // Check for any AOCR input, if found use that as the project report name
    StringRef Ext(llvm::sys::path::extension(Filename));
    if (Ext.empty())
      continue;
    if (getToolChain().LookupTypeForExtension(Ext.drop_front()) ==
        types::TY_FPGA_AOCR) {
      // Keep the base of the .aocr file name.  Input file is a temporary,
      // so we are stripping off the additional naming information for a
      // cleaner name.  The suffix being stripped from the name is the
      // added temporary string and the extension.
      StringRef SuffixFormat("-XXXXXX.aocr");
      SmallString<128> NameBase(
          Filename.substr(0, Filename.length() - SuffixFormat.size()));
      NameBase.append(".prj");
      CreatedReportName =
          Args.MakeArgString(llvm::sys::path::filename(NameBase));
    }
  }
  CmdArgs.push_back("-sycl");

  StringRef ForeachExt = "aocx";
  if (Arg *A = Args.getLastArg(options::OPT_fsycl_link_EQ))
    if (A->getValue() == StringRef("early")) {
      CmdArgs.push_back("-rtl");
      ForeachExt = "aocr";
    }

  for (auto *A : Args) {
    // Any input file is assumed to have a dependency file associated and
    // the report folder can also be named based on the first input.
    if (A->getOption().getKind() != Option::InputClass)
      continue;
    SmallString<128> ArgName(A->getSpelling());
    StringRef Ext(llvm::sys::path::extension(ArgName));
    if (Ext.empty())
      continue;
    types::ID Ty = getToolChain().LookupTypeForExtension(Ext.drop_front());
    if (Ty == types::TY_INVALID)
      continue;
    if (types::isSrcFile(Ty) || Ty == types::TY_Object) {
      // The project report is created in CWD, so strip off any directory
      // information if provided with the input file.
      StringRef TrimmedArgName = llvm::sys::path::filename(ArgName);
      if (types::isSrcFile(Ty)) {
        SmallString<128> DepName(
            C.getDriver().getFPGATempDepFile(std::string(TrimmedArgName)));
        if (!DepName.empty())
          FPGADepFiles.push_back(InputInfo(types::TY_Dependencies,
                                           Args.MakeArgString(DepName),
                                           Args.MakeArgString(DepName)));
      }
      if (CreatedReportName.empty()) {
        // Project report should be saved into CWD, so strip off any
        // directory information if provided with the input file.
        llvm::sys::path::replace_extension(ArgName, "prj");
        CreatedReportName = Args.MakeArgString(ArgName);
      }
    }
  }

  // Add any dependency files.
  if (!FPGADepFiles.empty()) {
    SmallString<128> DepOpt("-dep-files=");
    for (unsigned I = 0; I < FPGADepFiles.size(); ++I) {
      if (I)
        DepOpt += ',';
      if (FPGADepFiles[I].getType() == types::TY_FPGA_Dependencies_List)
        DepOpt += "@";
      DepOpt += FPGADepFiles[I].getFilename();
    }
    CmdArgs.push_back(C.getArgs().MakeArgString(DepOpt));
  }

  // Depending on output file designations, set the report folder
  SmallString<128> ReportOptArg;
  if (Arg *FinalOutput = Args.getLastArg(options::OPT_o, options::OPT__SLASH_o,
                                         options::OPT__SLASH_Fe)) {
    SmallString<128> FN(FinalOutput->getValue());
    // For "-o file.xxx" where the option value has an extension, if the
    // extension is one of .a .o .out .lib .obj .exe, the output project
    // directory name will be file.proj which omits the extension. Otherwise
    // the output project directory name will be file.xxx.prj which keeps
    // the original extension.
    StringRef Ext = llvm::sys::path::extension(FN);
    SmallVector<StringRef, 6> Exts = {".o",   ".a",   ".out",
                                      ".obj", ".lib", ".exe"};
    if (std::find(Exts.begin(), Exts.end(), Ext) != Exts.end())
      llvm::sys::path::replace_extension(FN, "prj");
    else
      FN.append(".prj");
    const char *FolderName = Args.MakeArgString(FN);
    ReportOptArg += FolderName;
  } else {
    // Default output directory should match default output executable name
    ReportOptArg += "a.prj";
  }
  if (!ReportOptArg.empty())
    CmdArgs.push_back(C.getArgs().MakeArgString(
        Twine("-output-report-folder=") + ReportOptArg));

#if INTEL_CUSTOMIZATION
  // Add any implied arguments before user defined arguments.
  const ToolChain *HostTC = C.getSingleOffloadToolChain<Action::OFK_Host>();
  TC.AddImpliedTargetArgs(DeviceOffloadKind, getToolChain().getTriple(), Args,
                          CmdArgs, JA, *HostTC);

  // Add -Xsycl-target* options.
  TC.TranslateBackendTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs);
  TC.TranslateLinkerTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs);
#endif // INTEL_CUSTOMIZATION

  // Look for -reuse-exe=XX option
  if (Arg *A = Args.getLastArg(options::OPT_reuse_exe_EQ)) {
    Args.ClaimAllArgs(options::OPT_reuse_exe_EQ);
    CmdArgs.push_back(Args.MakeArgString(A->getAsString(Args)));
  }

  SmallString<128> ExecPath(
      getToolChain().GetProgramPath(makeExeName(C, "aoc")));
  const char *Exec = C.getArgs().MakeArgString(ExecPath);
  auto Cmd = std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                       Exec, CmdArgs, std::nullopt);
  addFPGATimingDiagnostic(Cmd, C);
  if (!ForeachInputs.empty()) {
    StringRef ParallelJobs =
#if INTEL_CUSTOMIZATION
        Args.getLastArgValue(DeviceOffloadKind == Action::OFK_SYCL
                                 ? options::OPT_fsycl_max_parallel_jobs_EQ
                                 : options::OPT_fopenmp_max_parallel_jobs_EQ);
#endif // INTEL_CUSTOMIZATION
    constructLLVMForeachCommand(C, JA, std::move(Cmd), ForeachInputs, Output,
                                this, ReportOptArg, ForeachExt, ParallelJobs);
  } else
    C.addCommand(std::move(Cmd));
}

#if INTEL_CUSTOMIZATION
void SYCL::gen::BackendCompiler::constructOclocConcatCommand(Compilation &C,
    const JobAction &JA, const InputInfo &Output, const InputInfoList &Inputs,
    StringRef ExecPath) const {
  // Construct ocloc concat command.
  // ocloc concat <input> <input> ... -out <output>
  ArgStringList CmdArgs{"concat"};
  InputInfoList ForeachInputs;
  assert(Inputs.size() >= 2 && "Expecting 2 or more inputs to ocloc concat");
  for (const auto &II : Inputs) {
    std::string Filename(II.getFilename());
    if (II.getType() == types::TY_Tempfilelist)
      ForeachInputs.push_back(II);
    CmdArgs.push_back(C.getArgs().MakeArgString(Filename));
  }
  CmdArgs.push_back("-out");
  CmdArgs.push_back(C.getArgs().MakeArgString(Output.getFilename()));

  const char *Exec = C.getArgs().MakeArgString(ExecPath);
  auto Cmd = std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                       Exec, CmdArgs, std::nullopt);
  if (!ForeachInputs.empty()) {
    Action::OffloadKind DeviceOffloadKind(JA.getOffloadingDeviceKind());
    StringRef ParallelJobs =
        C.getArgs().getLastArgValue(DeviceOffloadKind == Action::OFK_SYCL
                                 ? options::OPT_fsycl_max_parallel_jobs_EQ
                                 : options::OPT_fopenmp_max_parallel_jobs_EQ);
    constructLLVMForeachCommand(C, JA, std::move(Cmd), ForeachInputs, Output,
                                this, "", "out", ParallelJobs);
  } else
    C.addCommand(std::move(Cmd));
}

struct OclocInfo {
  const char *DeviceName;
  const char *PackageName;
  const char *Version;
  SmallVector<int, 8> HexValues;
};

// The OclocDevices data structure is organized by device name, with the
// corresponding ocloc split release, version and possible Hex representations
// of a given device.  This information is gathered from the following:
// https://github.com/intel/compute-runtime/blob/master/shared/source/dll/devices/devices_base.inl
// https://github.com/intel/compute-runtime/blob/master/shared/source/dll/devices/devices_additional.inl
static OclocInfo OclocDevices[] = {
    {"bdw",
     "gen9-11",
     "8.0.0",
     {0x1602, 0x160A, 0x1606, 0x160E, 0x160D, 0x1612, 0x161A, 0x1616, 0x161E,
      0x161D, 0x1622, 0x162A, 0x1626, 0x162B, 0x162E, 0x162D}},
    {"skl", "gen9-11", "9.0.9", {0x1902, 0x190B, 0x190A, 0x1906, 0x190E, 0x1917,
                                 0x1913, 0x1915, 0x1912, 0x191B, 0x191A, 0x1916,
                                 0x191E, 0x191D, 0x1921, 0x9905, 0x192B, 0x192D,
                                 0x192A, 0x1923, 0x1926, 0x1927, 0x1932, 0x193B,
                                 0x193A, 0x193D}},
    {"kbl", "gen9-11", "9.1.9", {0x5902, 0x590B, 0x590A, 0x5906, 0x590E, 0x5908,
                                 0x5913, 0x5915, 0x5912, 0x591B, 0x5917, 0x591A,
                                 0x5916, 0x591E, 0x591D, 0x591C, 0x5921, 0x5926,
                                 0x5927, 0x592B, 0x592A, 0x5923, 0x5932, 0x593B,
                                 0x593A, 0x593D}},
    {"cfl",
     "gen9-11",
     "9.2.9",
     {0x3E90, 0x3E93, 0x3EA4, 0x3E99, 0x3EA1, 0x3E92, 0x3E9B, 0x3E94, 0x3E91,
      0x3E96, 0x3E9A, 0x3EA3, 0x3EA9, 0x3EA0, 0x3E98, 0x3E95, 0x3EA6, 0x3EA7,
      0x3EA8, 0x3EA5, 0x3EA2, 0x9B21, 0x9BAA, 0x9BAB, 0x9BAC, 0x9BA0, 0x9BA5,
      0x9BA8, 0x9BA4, 0x9BA2, 0x9B41, 0x9BCA, 0x9BCB, 0x9BCC, 0x9BC0, 0x9BC5,
      0x9BC8, 0x9BC4, 0x9BC2, 0x9BC6, 0x9BE6, 0x9BF6}},
    {"apl", "gen9-11", "9.3.0", {}},
    {"glk", "gen9-11", "9.4.0", {0x3184, 0x3185}},
    {"whl", "gen9-11", "9.5.0", {}},
    {"aml", "gen9-11", "9.6.0", {}},
    {"cml", "gen9-11", "9.7.0", {}},
    {"icllp",
     "gen9-11",
     "11.0.0",
     {0xFF05, 0x8A56, 0x8A58, 0x8A5C, 0x8A5A, 0x8A50, 0x8A52, 0x8A51}},
    {"lkf", "gen9-11", "11.1.0", {0x9840}},
    {"ehl",
     "gen9-11",
     "11.2.0",
     {0x4500, 0x4541, 0x4551, 0x4571, 0x4555, 0x4E51, 0x4E61, 0x4E71, 0x4E55}},
    {"gen9", "gen9-11", "", {}},
    {"gen11", "gen9-11", "", {}},
    {"tgl", "gen12+", "", {}},
    {"tgllp",
     "gen12+",
     "12.0.0",
     {0xFF20, 0x9A49, 0x9A40, 0x9A59, 0x9A60, 0x9A68, 0x9A70, 0x9A78}},

    {"rkl",
     "gen12+",
     "12.1.0",
     {0x4C80, 0x4C8A, 0x4C8B, 0x4C8C, 0x4C90, 0x4C9A}},
    {"adls",
     "gen12+",
     "12.2.0",
     {0x4680, 0x4682, 0x4688, 0x468A, 0x4690, 0x4692, 0x4693, 0xA780, 0xA781,
      0xA782, 0xA783, 0xA788, 0xA789, 0xA78B}},
    {"adl-s", "gen12+", "", {}},
    {"adlp", "gen12+", "12.3.0", {0x46A0, 0x46B0, 0x46A1, 0x46A2, 0x46A3,
                                  0x46A6, 0x46A8, 0x46AA, 0x462A, 0x4626,
                                  0x4628, 0x46B1, 0x46B2, 0x46B3, 0x46C0,
                                  0x46C1, 0x46C2, 0x46C3, 0xA7A0, 0xA720,
                                  0xA7A8, 0xA7A1, 0xA721, 0xA7A9}},
    {"adl-p", "gen12+", "", {}},
    {"adln", "gen12+", "12.4.0", {0x46D0, 0x46D1, 0x46D2}},
    {"adl-n", "gen12+", "", {}},
    {"dg1", "gen12+", "12.10.0", {0x4905, 0x4906, 0x4907, 0x4908}},
    {"dg2-g10-a0", "gen12+", "", {}},
    {"dg2-g10-a1", "gen12+", "", {}},
    {"dg2-g10-b0", "gen12+", "", {}},
    {"acm-g10", "gen12+", "12.55.8", {}},
    {"ats-m150", "gen12+", "", {}},
    {"dg2-g10",
     "gen12+",
     "",
     {0x4F80, 0x4F81, 0x4F82, 0x4F83, 0x4F84, 0x4F85, 0x5690, 0x5691, 0x5692,
      0x56A0, 0x56A1, 0x56A2, 0x56C0}},
    {"dg2-g10-c0", "gen12+", "", {}},
    {"dg2-g11-a0", "gen12+", "", {}},
    {"dg2-g11-b0", "gen12+", "", {}},
    {"acm-g11", "gen12+", "12.56.0", {}},
    {"ats-m75", "gen12+", "", {}},
    {"dg2-g11",
     "gen12+",
     "",
     {0x4F87, 0x4F88, 0x5693, 0x5694, 0x5695, 0x5696, 0x56B0, 0x56B1, 0x56C1}},
    {"dg2-g11-b1", "gen12+", "", {}},
    {"acm-g12", "gen12+", "12.57.0", {}},
    {"dg2-g12",
     "gen12+",
     "",
     {0x5696, 0x5697, 0x56A3, 0x56A4, 0x56B2, 0x56B3, 0x4F85, 0x4F86}},
    {"dg2-g12-a0", "gen12+", "", {}},
    {"acm-g20", "gen12+", "", {}},
    {"dg2-g20", "gen12+", "", {}},
    {"acm-g21", "gen12+", "", {}},
    {"dg2-g21", "gen12+", "", {}},
    {"dg2", "gen12+", "", {}},
    {"xehp-sdv",
     "gen12+",
     "12.1.0",
     {0x0201, 0x0202, 0x0203, 0x0204, 0x0205, 0x0206, 0x0207, 0x0208, 0x0209,
      0x020A, 0x020B, 0x020C, 0x020D, 0x020E, 0x020F, 0x0210}},
    {"pvc-sdv", "gen12+", "12.60.1", {}},
    {"pvc",
     "gen12+",
     "12.60.7",
     {0x0BD0, 0x0BD5, 0x0BD6, 0x0BD7, 0x0BD8, 0x0BD9, 0x0BDA, 0x0BDB}},
    {"gen12", "gen12+", "", {}},
    {"gen12lp", "gen12+", "", {}},
    {"xe", "gen12+", "", {}},
    {"xe-hpg", "gen12+", "", {}},
    {"xe-hpgplus", "gen12+", "", {}},
    {"xe_hp", "gen12+", "", {}},
    {"xe_hp_core", "gen12+", "", {}},
    {"xe_hpg_core", "gen12+", "", {}}};
static const unsigned numOclocDevices = std::size(OclocDevices);

static SmallVector<std::pair<StringRef, ArgStringList>> getOclocTargets(
    Compilation &C, const ArgStringList &CmdArgs) {
  SmallVector<std::pair<StringRef, ArgStringList>, 4> Targets;
  ArgStringList NewArgs;
  auto addToOcloc = [&](StringRef OclocTarget, StringRef Arg) {
    for (auto &Target : Targets) {
      if (Target.first.equals(OclocTarget)) {
        Target.second.push_back(C.getArgs().MakeArgString(Arg));
        return;
      }
    }
    // Target not found, add it here.
    ArgStringList NewList;
    NewList.push_back(C.getArgs().MakeArgString(Arg));
    Targets.push_back(
        std::make_pair(C.getArgs().MakeArgString(OclocTarget), NewList));
  };
  // Capture the argument for '-device'
  bool DeviceSeen = false;
  StringRef DeviceArg;
  for (StringRef Arg : CmdArgs) {
    if (DeviceSeen) {
      DeviceArg = Arg;
      break;
    }
    if (Arg.equals("-device"))
      DeviceSeen = true;
  }
  if (DeviceArg.empty())
    // No -device seen, return an empty vector
    return std::move(Targets);

  // Special case settings where we know exactly what oclocs to populate and
  // call with specific values.
  if (DeviceArg.equals("*")) {
    // -device * implies:
    //   gen9-11: -device *
    //   gen12+:  -device *
    addToOcloc("gen9-11", "-device");
    addToOcloc("gen9-11", "*");
    addToOcloc("gen12+", "-device");
    addToOcloc("gen12+", "*");
    return std::move(Targets);
  }
  if (DeviceArg.equals("gen9") || DeviceArg.equals("gen11")) {
    addToOcloc("gen9-11", "-device");
    addToOcloc("gen9-11", DeviceArg);
    return std::move(Targets);
  }

  // Here we parse the targets, tokenizing via ','
  SmallVector<StringRef> SplitArgs;
  // Holding vectors for the device values.
  // TODO: Improve handling of these - probably can be done a bit more
  // dynamically instead of fixed vectors.
  SmallVector<StringRef> Gen9_11;
  SmallVector<StringRef> Gen12plus;
  DeviceArg.split(SplitArgs, ",");
  for (auto SingleArg : SplitArgs) {
    StringRef OclocTarget;
    // Handle shortened versions.
    bool CheckShortVersion = true;
    for (auto Char : SingleArg.str()) {
      if (!(std::isdigit(Char) || Char == '.')) {
        CheckShortVersion = false;
        break;
      }
    }
    // Check for device, version or hex (literal values)
    for (unsigned int I = 0; I < numOclocDevices && OclocTarget.empty(); I++) {
      if (SingleArg.equals_insensitive(OclocDevices[I].DeviceName) ||
          SingleArg.equals_insensitive(OclocDevices[I].Version)) {
        OclocTarget = OclocDevices[I].PackageName;
        continue;
      }
      for (int HexVal : OclocDevices[I].HexValues) {
        int Value = 0;
        if (!SingleArg.getAsInteger(0, Value) && Value == HexVal) {
          OclocTarget = OclocDevices[I].PackageName;
          continue;
        }
      }
      // Check for ranges - just match a portion of the string or version
      // and send that full item to the specific ocloc.  Do not try and split
      // the ranges (possible future enhancement)
      if (SingleArg.contains('-') || SingleArg.contains(':')) {
        if (SingleArg.contains(OclocDevices[I].DeviceName) ||
            (OclocDevices[I].Version[0] &&
             SingleArg.contains(OclocDevices[I].Version))) {
          OclocTarget = OclocDevices[I].PackageName;
          continue;
        }
      }
      if (CheckShortVersion &&
          StringRef(OclocDevices[I].Version).startswith(SingleArg)) {
        OclocTarget = OclocDevices[I].PackageName;
        continue;
      }
    }
    if (!OclocTarget.empty()) {
      if (OclocTarget.equals("gen9-11"))
        Gen9_11.push_back(SingleArg);
      if (OclocTarget.equals("gen12+"))
        Gen12plus.push_back(SingleArg);
    } else {
      // Arg is not recognized.  We will default to adding to the gen12+ ocloc
      Gen12plus.push_back(SingleArg);
    }
  }
  auto addDevices = [&](SmallVector<StringRef> DeviceVec, StringRef Target) {
    if (!DeviceVec.empty()) {
      SmallString<32> Devices;
      int I = 0;
      for (auto DevArg : DeviceVec) {
        if (I)
          Devices += ",";
        Devices += DevArg;
        I++;
      }
      addToOcloc(Target, "-device");
      addToOcloc(Target, Devices);
    }
  };
  addDevices(Gen9_11, "gen9-11");
  addDevices(Gen12plus, "gen12+");

  return std::move(Targets);
}

void SYCL::gen::BackendCompiler::constructOclocCommand(Compilation &C,
    const JobAction &JA, const InputInfo &Output, const InputInfoList &Inputs,
    const ArgStringList &Args, StringRef ExecPath) const {
  // Construct ocloc command.
  // Input Args is expected to be the Args to ocloc, as the calling function
  // interprets the toolchain args prior to calling this.
  ArgStringList CmdArgs{"-output", Output.getFilename()};
  InputInfoList ForeachInputs;
  for (const auto &II : Inputs) {
    CmdArgs.push_back("-file");
    std::string Filename(II.getFilename());
    if (II.getType() == types::TY_Tempfilelist)
      ForeachInputs.push_back(II);
    CmdArgs.push_back(C.getArgs().MakeArgString(Filename));
  }

  // The next line prevents ocloc from modifying the image name
  CmdArgs.push_back("-output_no_suffix");
  CmdArgs.push_back("-spirv_input");

  // Add -Xsycl-target* options.
  CmdArgs.append(Args);

  const char *Exec = C.getArgs().MakeArgString(ExecPath);
  auto Cmd = std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                       Exec, CmdArgs, std::nullopt);
  if (!ForeachInputs.empty()) {
    Action::OffloadKind DeviceOffloadKind(JA.getOffloadingDeviceKind());
    StringRef ParallelJobs =
        C.getArgs().getLastArgValue(DeviceOffloadKind == Action::OFK_SYCL
                                 ? options::OPT_fsycl_max_parallel_jobs_EQ
                                 : options::OPT_fopenmp_max_parallel_jobs_EQ);
    constructLLVMForeachCommand(C, JA, std::move(Cmd), ForeachInputs, Output,
                                this, "", "out", ParallelJobs);
  } else
    C.addCommand(std::move(Cmd));
}

// Search for the location in which the split ocloc binaries reside.
// The location can be determined by the following:
//  - location set via $OCLOCROOT/$OCLOCVER
//  - values within the LIB environment variable
//  - location relative to compiler installation
static std::optional<std::string> getOclocLocation(Compilation &C) {
  std::optional<std::string> OclocRoot =
      llvm::sys::Process::GetEnv("OCLOCROOT");
  std::optional<std::string> OclocVer = llvm::sys::Process::GetEnv("OCLOCVER");
  if (OclocRoot && OclocVer) {
    SmallString<128> OclocLoc;
    llvm::sys::path::append(OclocLoc, *OclocRoot, *OclocVer);
    if (OclocLoc.size() > 1 && llvm::sys::fs::exists(OclocLoc))
      return std::string(OclocLoc.str());
  }
  std::optional<std::string> Lib = llvm::sys::Process::GetEnv("LIB");
  if (Lib) {
    SmallVector<StringRef, 8> SplitPaths;
    const char EnvPathSeparatorStr[] = {llvm::sys::EnvPathSeparator, '\0'};
    llvm::SplitString(*Lib, SplitPaths, EnvPathSeparatorStr);
    for (StringRef Path : SplitPaths) {
      SmallVector<std::string, 3> LibPaths;
      auto addDir = [&LibPaths](std::string Base, StringRef Path) -> void {
        SmallString<128> BD(Path);
        llvm::sys::path::append(BD, Base);
        LibPaths.emplace_back(BD);
      };
      // Check for the expected gen12+ and gen9-11 directories
      addDir("gen12+", Path.trim());
      addDir("gen9-11", Path.trim());
      bool ValidLoc = true;
      for (auto &Loc : LibPaths)
        ValidLoc &= llvm::sys::fs::exists(Loc);
      if (ValidLoc)
        return std::string(Path.str());
      // Non-split case, finding 'ocloc.exe' is valid.
      SmallString<128> LibDir(Path.trim());
      llvm::sys::path::append(LibDir, "ocloc.exe");
      if (llvm::sys::fs::exists(LibDir))
        return std::string(Path.str());
    }
  }
  SmallString<128> OclocDir(C.getDriver().Dir);
#if INTEL_DEPLOY_UNIFIED_LAYOUT
  // In the unified layout, the 'driver' is in bin/compiler
  llvm::sys::path::append(OclocDir, "..");
#endif // INTEL_DEPLOY_UNIFIED_LAYOUT
  llvm::sys::path::append(OclocDir, "..", "lib", "ocloc");
  llvm::sys::path::remove_dots(OclocDir, /*remove_dot_dot=*/true);
  if (llvm::sys::fs::exists(OclocDir))
    return std::string(OclocDir.str());
  return std::nullopt;
}

#endif // INTEL_CUSTOMIZATION

StringRef SYCL::gen::getGenGRFFlag(StringRef GRFMode) {
  return llvm::StringSwitch<StringRef>(GRFMode)
      .Case("auto", "-ze-intel-enable-auto-large-GRF-mode")
      .Case("small", "-ze-intel-128-GRF-per-thread")
      .Case("large", "-ze-opt-large-register-file")
      .Default("");
}

void SYCL::gen::BackendCompiler::ConstructJob(Compilation &C,
                                              const JobAction &JA,
                                              const InputInfo &Output,
                                              const InputInfoList &Inputs,
                                              const ArgList &Args,
                                              const char *LinkingOutput) const {
  assert((getToolChain().getTriple().getArch() == llvm::Triple::spir ||
          getToolChain().getTriple().getArch() == llvm::Triple::spir64) &&
         "Unsupported target");

#if INTEL_CUSTOMIZATION
  // When on Windows, and we have multiple ocloc installations available, we
  // have to break down which ocloc (or multiple oclocs) we are calling.
  // This path is only valid for Windows targets and if _all_ of the known
  // oclocs are found.
  const ToolChain *HostTC = C.getSingleOffloadToolChain<Action::OFK_Host>();
  bool isMSVC = HostTC->getTriple().isWindowsMSVCEnvironment();
  bool SplitOcloc = false;
  std::map<std::string, std::string> OclocDirs;
  // Allow for this behavior to trigger with --enable-ocloc-split for
  // testing purposes.
  if (isMSVC || Args.hasArg(options::OPT_enable_ocloc_split)) {
    SplitOcloc = true;
    std::string OclocDir(C.getDriver().GetFilePath("ocloc", *HostTC));
    if (!Args.hasArg(options::OPT_enable_ocloc_split))
      if (auto OclocLoc = getOclocLocation(C))
        OclocDir = *OclocLoc;
    auto addOclocDir = [&OclocDirs, &OclocDir](std::string BaseName) {
      SmallString<128> OD(OclocDir);
      llvm::sys::path::append(OD, BaseName, "ocloc.exe");
      OclocDirs[BaseName] = OD.str();
    };
    addOclocDir("gen9-11");
    addOclocDir("gen12+");
    for (auto &Loc : OclocDirs)
      SplitOcloc &= llvm::sys::fs::exists(Loc.second);
  }
  StringRef Device = JA.getOffloadingArch();
  Action::OffloadKind DeviceOffloadKind(JA.getOffloadingDeviceKind());
  const toolchains::SYCLToolChain &TC =
      static_cast<const toolchains::SYCLToolChain &>(getToolChain());
  ArgStringList CmdArgs;
  TC.AddImpliedTargetArgs(DeviceOffloadKind, getToolChain().getTriple(), Args,
                          CmdArgs, JA, *HostTC);
  TC.TranslateBackendTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs, Device);
  TC.TranslateLinkerTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs);
  // Strip out -cl-no-match-sincospi in case it was used to disable the
  // default setting.
  CmdArgs.erase(llvm::remove_if(CmdArgs,
                                [&](auto Cur) {
                                  return !strcmp(Cur, "-cl-no-match-sincospi");
                                }),
                CmdArgs.end());
  // Check for -device settings in the CmdArgs, split them out as needed.
  if (SplitOcloc) {
    // Determine which ocloc(s) we will be calling.  This will be done by
    // generating a tuple containing arguments for each set of devices.
    SmallVector<std::pair<StringRef, ArgStringList>>
        OclocTargets(getOclocTargets(C, CmdArgs));
    auto combineArgs = [&CmdArgs](const ArgStringList &DeviceArg)
                                                      -> ArgStringList {
      ArgStringList AllArgs;
      bool SkipNext = false;
      for (auto Arg : CmdArgs) {
        if (SkipNext) {
          SkipNext = false;
          continue;
        }
        if (StringRef(Arg).equals("-device")) {
          AllArgs.append(DeviceArg);
          SkipNext = true;
          continue;
        }
        AllArgs.push_back(Arg);
      }
      return AllArgs;
    };
    if (OclocTargets.size() > 1) {
      InputInfoList Outputs;
      for (auto &OclocItem : OclocTargets) {
        auto OclocDir = OclocDirs.find(OclocItem.first.data());
        assert(OclocDir != OclocDirs.end() && "Missing ocloc search location");
        // Create new output temporary file
        std::string OutputTempFile;
        OutputTempFile = C.getDriver().GetTemporaryPath(
            llvm::sys::path::stem(Output.getFilename()).str() + "-ocloc",
            types::getTypeTempSuffix(Output.getType()));
        const char *TempOutput = C.addTempFile(
            C.getArgs().MakeArgString(OutputTempFile));
        InputInfo OclocOutput(Inputs[0].getType(), TempOutput, TempOutput);
        Outputs.push_back(OclocOutput);
        constructOclocCommand(C, JA, OclocOutput, Inputs,
                              combineArgs(OclocItem.second), OclocDir->second);
      }
      // perform a concatenation.
      auto OclocDir = OclocDirs.find("gen12+");
      // 3 inputs for concat.  Perform a concat against the first 2.
      InputInfoList OutputsP2;
      assert((Outputs.size() == 2 || Outputs.size() == 3) && "Incorrect number "
             "of expected concat inputs.");
      if (Outputs.size() == 3) {
        // Create new output temporary file
        std::string OutputTempFile;
        OutputTempFile = C.getDriver().GetTemporaryPath(
            llvm::sys::path::stem(Output.getFilename()).str() + "-ocloc",
            types::getTypeTempSuffix(Output.getType()));
        const char *TempOutput = C.addTempFile(
            C.getArgs().MakeArgString(OutputTempFile));
        InputInfo OclocOutput(Outputs[1].getType(), TempOutput, TempOutput);
        InputInfoList OutputsP1;
        OutputsP1.push_back(Outputs[1]);
        OutputsP1.push_back(Outputs[2]);
        constructOclocConcatCommand(C, JA, OclocOutput, OutputsP1,
                                    OclocDir->second);
        OutputsP2.push_back(OclocOutput);
      } else
        OutputsP2.push_back(Outputs[1]);
      OutputsP2.push_back(Outputs[0]);
      constructOclocConcatCommand(C, JA, Output, OutputsP2, OclocDir->second);
      return;
    } else if (OclocTargets.size() == 1) {
      auto OclocItem = OclocTargets[0];
      auto OclocDir = OclocDirs.find(OclocItem.first.data());
      assert(OclocDir != OclocDirs.end() && "Missing ocloc search location");
      constructOclocCommand(C, JA, Output, Inputs,
                            combineArgs(OclocItem.second),
                            OclocDir->second);
      return;
    }
  }
  // Find ocloc via OCLOCROOT/OCLOCVER or LIB
  if (auto OclocLoc = getOclocLocation(C)) {
    SmallString<128> OD(*OclocLoc);
    llvm::sys::path::append(OD, "ocloc.exe");
    if (llvm::sys::fs::exists(OD)) {
      constructOclocCommand(C, JA, Output, Inputs, CmdArgs, OD);
      return;
    }
  }
  // Splitting environment not available and not found with OCLOCROOT/OCLOCVER
  // or LIB so do a regular run.
  auto OclocBin = llvm::sys::findProgramByName("ocloc");
  if (OclocBin.getError())
    C.getDriver().Diag(diag::warn_drv_aot_tool_not_found)
                   << "ocloc";

  SmallString<128> ExecPath(
      getToolChain().GetProgramPath(makeExeName(C, "ocloc")));
  constructOclocCommand(C, JA, Output, Inputs, CmdArgs, ExecPath);
#else // INTEL_CUSTOMIZATION
  ArgStringList CmdArgs{"-output", Output.getFilename()};
  InputInfoList ForeachInputs;
  for (const auto &II : Inputs) {
    CmdArgs.push_back("-file");
    std::string Filename(II.getFilename());
    if (II.getType() == types::TY_Tempfilelist)
      ForeachInputs.push_back(II);
    CmdArgs.push_back(C.getArgs().MakeArgString(Filename));
  }
  // The next line prevents ocloc from modifying the image name
  CmdArgs.push_back("-output_no_suffix");
  CmdArgs.push_back("-spirv_input");
  StringRef Device = JA.getOffloadingArch();

  // Add -Xsycl-target* options.
  Action::OffloadKind DeviceOffloadKind(JA.getOffloadingDeviceKind()); // INTEL
  const toolchains::SYCLToolChain &TC =
      static_cast<const toolchains::SYCLToolChain &>(getToolChain());
#if INTEL_CUSTOMIZATION
  const ToolChain *HostTC =
      C.getSingleOffloadToolChain<Action::OFK_Host>()
          TC.AddImpliedTargetArgs(DeviceOffloadKind, getToolChain().getTriple(),
                                  Args, CmdArgs, JA, *HostTC);
  TC.TranslateBackendTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs, Device);
  TC.TranslateLinkerTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs);
  // Strip out -cl-no-match-sincospi in case it was used to disable the
  // default setting.
  CmdArgs.erase(llvm::remove_if(CmdArgs,
                                [&](auto Cur) {
                                  return !strcmp(Cur, "-cl-no-match-sincospi");
                                }),
                CmdArgs.end());
#endif // INTEL_CUSTOMIZATION
  SmallString<128> ExecPath(
      getToolChain().GetProgramPath(makeExeName(C, "ocloc")));
  const char *Exec = C.getArgs().MakeArgString(ExecPath);

#if INTEL_CUSTOMIZATION
  auto OclocBin = llvm::sys::findProgramByName("ocloc");
  if (OclocBin.getError())
    C.getDriver().Diag(diag::warn_drv_aot_tool_not_found)
                   << "ocloc";
#endif // INTEL_CUSTOMIZATION
  auto Cmd = std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                       Exec, CmdArgs, std::nullopt);
  if (!ForeachInputs.empty()) {
    StringRef ParallelJobs =
#if INTEL_CUSTOMIZATION
        Args.getLastArgValue(DeviceOffloadKind == Action::OFK_SYCL
                                 ? options::OPT_fsycl_max_parallel_jobs_EQ
                                 : options::OPT_fopenmp_max_parallel_jobs_EQ);
#endif // INTEL_CUSTOMIZATION
    constructLLVMForeachCommand(C, JA, std::move(Cmd), ForeachInputs, Output,
                                this, "", "out", ParallelJobs);
  } else
    C.addCommand(std::move(Cmd));
#endif // INTEL_CUSTOMIZATION
}

StringRef SYCL::gen::resolveGenDevice(StringRef DeviceName) {
  StringRef Device;
  Device =
      llvm::StringSwitch<StringRef>(DeviceName)
          .Cases("intel_gpu_bdw", "intel_gpu_8_0_0", "bdw")
          .Cases("intel_gpu_skl", "intel_gpu_9_0_9", "skl")
          .Cases("intel_gpu_kbl", "intel_gpu_9_1_9", "kbl")
          .Cases("intel_gpu_cfl", "intel_gpu_9_2_9", "cfl")
          .Cases("intel_gpu_apl", "intel_gpu_bxt", "intel_gpu_9_3_0", "apl")
          .Cases("intel_gpu_glk", "intel_gpu_9_4_0", "glk")
          .Cases("intel_gpu_whl", "intel_gpu_9_5_0", "whl")
          .Cases("intel_gpu_aml", "intel_gpu_9_6_0", "aml")
          .Cases("intel_gpu_cml", "intel_gpu_9_7_0", "cml")
          .Cases("intel_gpu_icllp", "intel_gpu_11_0_0", "icllp")
          .Cases("intel_gpu_ehl", "intel_gpu_jsl", "ehl")
          .Cases("intel_gpu_tgllp", "intel_gpu_12_0_0", "tgllp")
          .Case("intel_gpu_rkl", "rkl")
          .Cases("intel_gpu_adl_s", "intel_gpu_rpl_s", "adl_s")
          .Case("intel_gpu_adl_p", "adl_p")
          .Case("intel_gpu_adl_n", "adl_n")
          .Cases("intel_gpu_dg1", "intel_gpu_12_10_0", "dg1")
          .Cases("intel_gpu_acm_g10", "intel_gpu_dg2_g10", "acm_g10")
          .Cases("intel_gpu_acm_g11", "intel_gpu_dg2_g11", "acm_g11")
          .Cases("intel_gpu_acm_g12", "intel_gpu_dg2_g12", "acm_g12")
          .Case("intel_gpu_pvc", "pvc")
          .Case("nvidia_gpu_sm_50", "sm_50")
          .Case("nvidia_gpu_sm_52", "sm_52")
          .Case("nvidia_gpu_sm_53", "sm_53")
          .Case("nvidia_gpu_sm_60", "sm_60")
          .Case("nvidia_gpu_sm_61", "sm_61")
          .Case("nvidia_gpu_sm_62", "sm_62")
          .Case("nvidia_gpu_sm_70", "sm_70")
          .Case("nvidia_gpu_sm_72", "sm_72")
          .Case("nvidia_gpu_sm_75", "sm_75")
          .Case("nvidia_gpu_sm_80", "sm_80")
          .Case("nvidia_gpu_sm_86", "sm_86")
          .Case("nvidia_gpu_sm_87", "sm_87")
          .Case("nvidia_gpu_sm_89", "sm_89")
          .Case("nvidia_gpu_sm_90", "sm_90")
          .Case("amd_gpu_gfx700", "gfx700")
          .Case("amd_gpu_gfx701", "gfx701")
          .Case("amd_gpu_gfx702", "gfx702")
          .Case("amd_gpu_gfx801", "gfx801")
          .Case("amd_gpu_gfx802", "gfx802")
          .Case("amd_gpu_gfx803", "gfx803")
          .Case("amd_gpu_gfx805", "gfx805")
          .Case("amd_gpu_gfx810", "gfx810")
          .Case("amd_gpu_gfx900", "gfx900")
          .Case("amd_gpu_gfx902", "gfx902")
          .Case("amd_gpu_gfx904", "gfx904")
          .Case("amd_gpu_gfx906", "gfx906")
          .Case("amd_gpu_gfx908", "gfx908")
          .Case("amd_gpu_gfx90a", "gfx90a")
          .Case("amd_gpu_gfx1010", "gfx1010")
          .Case("amd_gpu_gfx1011", "gfx1011")
          .Case("amd_gpu_gfx1012", "gfx1012")
          .Case("amd_gpu_gfx1013", "gfx1013")
          .Case("amd_gpu_gfx1030", "gfx1030")
          .Case("amd_gpu_gfx1031", "gfx1031")
          .Case("amd_gpu_gfx1032", "gfx1032")
          .Case("amd_gpu_gfx1034", "gfx1034")
          .Default("");
  return Device;
}

SmallString<64> SYCL::gen::getGenDeviceMacro(StringRef DeviceName) {
  SmallString<64> Macro;
  StringRef Ext = llvm::StringSwitch<StringRef>(DeviceName)
                      .Case("bdw", "INTEL_GPU_BDW")
                      .Case("skl", "INTEL_GPU_SKL")
                      .Case("kbl", "INTEL_GPU_KBL")
                      .Case("cfl", "INTEL_GPU_CFL")
                      .Case("apl", "INTEL_GPU_APL")
                      .Case("glk", "INTEL_GPU_GLK")
                      .Case("whl", "INTEL_GPU_WHL")
                      .Case("aml", "INTEL_GPU_AML")
                      .Case("cml", "INTEL_GPU_CML")
                      .Case("icllp", "INTEL_GPU_ICLLP")
                      .Case("ehl", "INTEL_GPU_EHL")
                      .Case("tgllp", "INTEL_GPU_TGLLP")
                      .Case("rkl", "INTEL_GPU_RKL")
                      .Case("adl_s", "INTEL_GPU_ADL_S")
                      .Case("adl_p", "INTEL_GPU_ADL_P")
                      .Case("adl_n", "INTEL_GPU_ADL_N")
                      .Case("dg1", "INTEL_GPU_DG1")
                      .Case("acm_g10", "INTEL_GPU_ACM_G10")
                      .Case("acm_g11", "INTEL_GPU_ACM_G11")
                      .Case("acm_g12", "INTEL_GPU_ACM_G12")
                      .Case("pvc", "INTEL_GPU_PVC")
                      .Case("sm_50", "NVIDIA_GPU_SM_50")
                      .Case("sm_52", "NVIDIA_GPU_SM_52")
                      .Case("sm_53", "NVIDIA_GPU_SM_53")
                      .Case("sm_60", "NVIDIA_GPU_SM_60")
                      .Case("sm_61", "NVIDIA_GPU_SM_61")
                      .Case("sm_62", "NVIDIA_GPU_SM_62")
                      .Case("sm_70", "NVIDIA_GPU_SM_70")
                      .Case("sm_72", "NVIDIA_GPU_SM_72")
                      .Case("sm_75", "NVIDIA_GPU_SM_75")
                      .Case("sm_80", "NVIDIA_GPU_SM_80")
                      .Case("sm_86", "NVIDIA_GPU_SM_86")
                      .Case("sm_87", "NVIDIA_GPU_SM_87")
                      .Case("sm_89", "NVIDIA_GPU_SM_89")
                      .Case("sm_90", "NVIDIA_GPU_SM_90")
                      .Case("gfx700", "AMD_GPU_GFX700")
                      .Case("gfx701", "AMD_GPU_GFX701")
                      .Case("gfx702", "AMD_GPU_GFX702")
                      .Case("gfx801", "AMD_GPU_GFX801")
                      .Case("gfx802", "AMD_GPU_GFX802")
                      .Case("gfx803", "AMD_GPU_GFX803")
                      .Case("gfx805", "AMD_GPU_GFX805")
                      .Case("gfx810", "AMD_GPU_GFX810")
                      .Case("gfx900", "AMD_GPU_GFX900")
                      .Case("gfx902", "AMD_GPU_GFX902")
                      .Case("gfx904", "AMD_GPU_GFX904")
                      .Case("gfx906", "AMD_GPU_GFX906")
                      .Case("gfx908", "AMD_GPU_GFX908")
                      .Case("gfx90a", "AMD_GPU_GFX90A")
                      .Case("gfx1010", "AMD_GPU_GFX1010")
                      .Case("gfx1011", "AMD_GPU_GFX1011")
                      .Case("gfx1012", "AMD_GPU_GFX1012")
                      .Case("gfx1013", "AMD_GPU_GFX1013")
                      .Case("gfx1030", "AMD_GPU_GFX1030")
                      .Case("gfx1031", "AMD_GPU_GFX1031")
                      .Case("gfx1032", "AMD_GPU_GFX1032")
                      .Case("gfx1034", "AMD_GPU_GFX1034")
                      .Default("");
  if (!Ext.empty()) {
    Macro = "__SYCL_TARGET_";
    Macro += Ext;
    Macro += "__";
  }
  return Macro;
}

void SYCL::x86_64::BackendCompiler::ConstructJob(
    Compilation &C, const JobAction &JA, const InputInfo &Output,
    const InputInfoList &Inputs, const ArgList &Args,
    const char *LinkingOutput) const {
  ArgStringList CmdArgs;
  CmdArgs.push_back(Args.MakeArgString(Twine("-o=") + Output.getFilename()));
  CmdArgs.push_back("--device=cpu");
  InputInfoList ForeachInputs;
  for (const auto &II : Inputs) {
    std::string Filename(II.getFilename());
    if (II.getType() == types::TY_Tempfilelist)
      ForeachInputs.push_back(II);
    CmdArgs.push_back(Args.MakeArgString(Filename));
  }
  // Add -Xsycl-target* options.
  Action::OffloadKind DeviceOffloadKind(JA.getOffloadingDeviceKind()); // INTEL
  const toolchains::SYCLToolChain &TC =
      static_cast<const toolchains::SYCLToolChain &>(getToolChain());

#if INTEL_CUSTOMIZATION
  const ToolChain *HostTC = C.getSingleOffloadToolChain<Action::OFK_Host>();
  TC.AddImpliedTargetArgs(DeviceOffloadKind, getToolChain().getTriple(), Args,
                          CmdArgs, JA, *HostTC);
  TC.TranslateBackendTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs);
  TC.TranslateLinkerTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs);
#endif // INTEL_CUSTOMIZATION
  SmallString<128> ExecPath(
      getToolChain().GetProgramPath(makeExeName(C, "opencl-aot")));
  const char *Exec = C.getArgs().MakeArgString(ExecPath);
  auto Cmd = std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                       Exec, CmdArgs, std::nullopt);
  if (!ForeachInputs.empty()) {
    StringRef ParallelJobs =
#if INTEL_CUSTOMIZATION
        Args.getLastArgValue(DeviceOffloadKind == Action::OFK_SYCL
                                 ? options::OPT_fsycl_max_parallel_jobs_EQ
                                 : options::OPT_fopenmp_max_parallel_jobs_EQ);
#endif // INTEL_CUSTOMIZATION
    constructLLVMForeachCommand(C, JA, std::move(Cmd), ForeachInputs, Output,
                                this, "", "out", ParallelJobs);
  } else
    C.addCommand(std::move(Cmd));
}

// Unsupported options for device compilation
//  -fcf-protection, -fsanitize, -fprofile-generate, -fprofile-instr-generate
//  -ftest-coverage, -fcoverage-mapping, -fcreate-profile, -fprofile-arcs
//  -fcs-profile-generate -forder-file-instrumentation
static std::vector<OptSpecifier> getUnsupportedOpts(void) {
  std::vector<OptSpecifier> UnsupportedOpts = {
      options::OPT_fsanitize_EQ,
      options::OPT_fcf_protection_EQ,
      options::OPT_fprofile_generate,
      options::OPT_fprofile_generate_EQ,
      options::OPT_fno_profile_generate,
      options::OPT_ftest_coverage,
      options::OPT_fno_test_coverage,
      options::OPT_fcoverage_mapping,
      options::OPT_fno_coverage_mapping,
      options::OPT_fprofile_instr_generate,
      options::OPT_fprofile_instr_generate_EQ,
      options::OPT_fprofile_arcs,
      options::OPT_fno_profile_arcs,
      options::OPT_fno_profile_instr_generate,
      options::OPT_fcreate_profile,
      options::OPT_fprofile_instr_use,
      options::OPT_fprofile_instr_use_EQ,
      options::OPT_forder_file_instrumentation,
      options::OPT_traceback, // INTEL
      options::OPT_fcs_profile_generate,
      options::OPT_fcs_profile_generate_EQ};
  return UnsupportedOpts;
}

SYCLToolChain::SYCLToolChain(const Driver &D, const llvm::Triple &Triple,
                             const ToolChain &HostTC, const ArgList &Args)
    : ToolChain(D, Triple, Args), HostTC(HostTC),
      IsSYCLNativeCPU(Triple == HostTC.getTriple()) {
  // Lookup binaries into the driver directory, this is used to
  // discover the clang-offload-bundler executable.
  getProgramPaths().push_back(getDriver().Dir);
#if INTEL_CUSTOMIZATION
  // getDriver() returns clang, which is not the Intel driver and may not be in
  // "bin". Ensure that we look in "bin" for programs. This is Intel-specific
  // because upstream doesn't typically have multiple program directories.
  SmallString<128> Bin(llvm::sys::path::parent_path(getDriver().Dir));
#if !INTEL_DEPLOY_UNIFIED_LAYOUT
  llvm::sys::path::append(Bin, "bin");
#endif // INTEL_DEPLOY_UNIFIED_LAYOUT
  getProgramPaths().push_back(std::string(Bin));
#endif // INTEL_CUSTOMIZATION

  // Diagnose unsupported options only once.
  for (OptSpecifier Opt : getUnsupportedOpts()) {
    if (const Arg *A = Args.getLastArg(Opt)) {
      D.Diag(clang::diag::warn_drv_unsupported_option_for_target)
          << A->getAsString(Args) << getTriple().str();
    }
  }
}

void SYCLToolChain::addClangTargetOptions(
    const llvm::opt::ArgList &DriverArgs, llvm::opt::ArgStringList &CC1Args,
    Action::OffloadKind DeviceOffloadingKind) const {
  HostTC.addClangTargetOptions(DriverArgs, CC1Args, DeviceOffloadingKind);
}

llvm::opt::DerivedArgList *
SYCLToolChain::TranslateArgs(const llvm::opt::DerivedArgList &Args,
                             StringRef BoundArch,
                             Action::OffloadKind DeviceOffloadKind) const {
  DerivedArgList *DAL =
      HostTC.TranslateArgs(Args, BoundArch, DeviceOffloadKind);

  bool IsNewDAL = false;
  if (!DAL) {
    DAL = new DerivedArgList(Args.getBaseArgs());
    IsNewDAL = true;
  }

  for (Arg *A : Args) {
    // Filter out any options we do not want to pass along to the device
    // compilation.
    auto Opt(A->getOption());
    bool Unsupported = false;
    for (OptSpecifier UnsupportedOpt : getUnsupportedOpts()) {
      if (Opt.matches(UnsupportedOpt)) {
        if (!IsNewDAL)
          DAL->eraseArg(Opt.getID());
        Unsupported = true;
      }
    }
    if (Unsupported)
      continue;
    if (IsNewDAL)
      DAL->append(A);
  }
  // Strip out -O0 for FPGA Hardware device compilation.
  if (getDriver().IsFPGAHWMode() &&
      getTriple().getSubArch() == llvm::Triple::SPIRSubArch_fpga)
    DAL->eraseArg(options::OPT_O0);

  const OptTable &Opts = getDriver().getOpts();
  if (!BoundArch.empty()) {
    DAL->eraseArg(options::OPT_march_EQ);
    DAL->AddJoinedArg(nullptr, Opts.getOption(options::OPT_march_EQ),
                      BoundArch);
  }
  return DAL;
}

static void parseTargetOpts(StringRef ArgString, const llvm::opt::ArgList &Args,
                            llvm::opt::ArgStringList &CmdArgs) {
  // Tokenize the string.
  SmallVector<const char *, 8> TargetArgs;
  llvm::BumpPtrAllocator A;
  llvm::StringSaver S(A);
  llvm::cl::TokenizeGNUCommandLine(ArgString, S, TargetArgs);
  for (StringRef TA : TargetArgs)
    CmdArgs.push_back(Args.MakeArgString(TA));
}

void SYCLToolChain::TranslateGPUTargetOpt(const llvm::opt::ArgList &Args,
                                          llvm::opt::ArgStringList &CmdArgs,
                                          OptSpecifier Opt_EQ) const {
  for (auto *A : Args) {
    if (A->getOption().matches(Opt_EQ)) {
      if (auto GpuDevice =
              tools::SYCL::gen::isGPUTarget<tools::SYCL::gen::AmdGPU>(
                  A->getValue())) {
        StringRef ArgString;
        SmallString<64> OffloadArch("--offload-arch=");
        OffloadArch += GpuDevice->data();
        ArgString = OffloadArch;
        parseTargetOpts(ArgString, Args, CmdArgs);
        A->claim();
      }
    }
  }
}

// Expects a specific type of option (e.g. -Xsycl-target-backend) and will
// extract the arguments.
#if INTEL_CUSTOMIZATION
void SYCLToolChain::TranslateTargetOpt(Action::OffloadKind DeviceOffloadKind,
    const llvm::opt::ArgList &Args, llvm::opt::ArgStringList &CmdArgs,
    OptSpecifier Opt, OptSpecifier Opt_EQ, StringRef Device) const {
#endif // INTEL_CUSTOMIZATION
  for (auto *A : Args) {
    bool OptNoTriple;
    OptNoTriple = A->getOption().matches(Opt);
    if (A->getOption().matches(Opt_EQ)) {
      // Passing device args: -X<Opt>=<triple> -opt=val.
      StringRef GenDevice = SYCL::gen::resolveGenDevice(A->getValue());
      bool IsGenTriple =
          getTriple().isSPIR() &&
          getTriple().getSubArch() == llvm::Triple::SPIRSubArch_gen;
      if (Device != GenDevice)
        continue;
      if (getDriver().MakeSYCLDeviceTriple(A->getValue()) != getTriple() &&
          (!IsGenTriple || (IsGenTriple && GenDevice.empty())))
        // Triples do not match, but only skip when we know we are not comparing
        // against intel_gpu_* and non-spir64_gen
        continue;
    } else if (!OptNoTriple)
      // Don't worry about any of the other args, we only want to pass what is
      // passed in -X<Opt>
      continue;

    // Add the argument from -X<Opt>
    StringRef ArgString;
    if (OptNoTriple) {
      // With multiple -fsycl-targets, a triple is required so we know where
      // the options should go.
#if INTEL_CUSTOMIZATION
      if (DeviceOffloadKind == Action::OFK_SYCL) {
        const Arg *TargetArg = Args.getLastArg(options::OPT_fsycl_targets_EQ);
        if (TargetArg && TargetArg->getValues().size() != 1) {
          getDriver().Diag(diag::err_drv_Xsycl_target_missing_triple)
              << A->getSpelling();
          continue;
        }
      } else {
        const Arg *TargetArg = Args.getLastArg(options::OPT_fopenmp_targets_EQ);
        if (TargetArg && TargetArg->getValues().size() != 1) {
          getDriver().Diag(diag::err_drv_Xopenmp_target_missing_triple)
              << A->getSpelling();
          continue;
        }
      }
#endif // INTEL_CUSTOMIZATION
      // No triple, so just add the argument.
      ArgString = A->getValue();
    } else
      // Triple found, add the next argument in line.
      ArgString = A->getValue(1);

    parseTargetOpts(ArgString, Args, CmdArgs);
    A->claim();
  }
}

void SYCLToolChain::AddImpliedTargetArgs(Action::OffloadKind DeviceOffloadKind,
                                         const llvm::Triple &Triple,
                                         const llvm::opt::ArgList &Args,
                                         llvm::opt::ArgStringList &CmdArgs,
                                         const JobAction &JA,
                                         const ToolChain &HostTC) const {
  // Current implied args are for debug information and disabling of
  // optimizations.  They are passed along to the respective areas as follows:
  // FPGA:  -g -cl-opt-disable
  // Default device AOT: -g -cl-opt-disable
  // SYCL Default device JIT: -g (-O0 is handled by the runtime)
  // OMP Offload Default device JIT: -g -cl-opt-disable
  // GEN:  -options "-g -O0"
  // CPU:  "--bo=-g -cl-opt-disable"
  llvm::opt::ArgStringList BeArgs;
  // Per-device argument vector storing the device name and the backend argument
  // string
  llvm::SmallVector<std::pair<StringRef, StringRef>, 16> PerDeviceArgs;
  bool IsGen = Triple.getSubArch() == llvm::Triple::SPIRSubArch_gen;
  if (Arg *A = Args.getLastArg(options::OPT_g_Group, options::OPT__SLASH_Z7))
    if (!A->getOption().matches(options::OPT_g0))
      BeArgs.push_back("-g");
#if INTEL_CUSTOMIZATION
  // FIXME: /Od is not translating to -O0 for OpenMP
  bool IsMSVCOd = false;
  if (Arg *A = Args.getLastArg(options::OPT__SLASH_O)) {
    StringRef OptStr = A->getValue();
    if (OptStr == "d")
      IsMSVCOd = true;
  }
  if (DeviceOffloadKind == Action::OFK_OpenMP) {
    if (IsGen) {
      // Add -cl-take-global-addresses by default for GEN/ocloc
      BeArgs.push_back("-cl-take-global-address");
      // Add -cl-match-sincospi by default for GEN/ocloc, but do a quick
      // check for -cl-no-match-sincospi if somebody passed it to disable.
      ArgStringList TargArgs;
      Args.AddAllArgValues(TargArgs, options::OPT_Xs, options::OPT_Xs_separate);
      Args.AddAllArgValues(TargArgs, options::OPT_Xopenmp_backend,
                           options::OPT_Xopenmp_backend_EQ);
      if (llvm::find_if(TargArgs, [&](auto Cur) {
            return !strcmp(Cur, "-cl-no-match-sincospi");
          }) == TargArgs.end())
        BeArgs.push_back("-cl-match-sincospi");
    }
    // -vc-codegen is the default with -fopenmp-target-simd
    if (Args.hasArg(options::OPT_fopenmp_target_simd) &&
        !Args.hasArg(options::OPT_fopenmp_target_simd_split) &&
        (Triple.getSubArch() == llvm::Triple::NoSubArch || IsGen))
      BeArgs.push_back("-vc-codegen");
    if (Arg *A = Args.getLastArg(options::OPT_fopenmp_target_buffers_EQ)) {
      StringRef BufArg = A->getValue();
      if (BufArg == "4GB") {
        // spir64_x86_64 is 'accept and ignore'.
        if (Triple.getSubArch() != llvm::Triple::SPIRSubArch_x86_64)
          BeArgs.push_back("-cl-intel-greater-than-4GB-buffer-required");
      } else if (BufArg != "default")
        getDriver().Diag(diag::err_drv_unsupported_option_argument)
            << A->getSpelling() << BufArg;
    }
  }
  if (IsMSVCOd )
      BeArgs.push_back("-cl-opt-disable");
  else
#endif // INTEL_CUSTOMIZATION
      // Only pass -cl-opt-disable for non-JIT or OMP Offload, as the SYCL
      // runtime handles O0 for the JIT case.
      if (Triple.getSubArch() != llvm::Triple::NoSubArch ||
          DeviceOffloadKind == Action::OFK_OpenMP)
      if (Arg *A = Args.getLastArg(options::OPT_O_Group))
        if (A->getOption().matches(options::OPT_O0))
          BeArgs.push_back("-cl-opt-disable");
  StringRef RegAllocModeOptName = "-ftarget-register-alloc-mode=";
  if (Arg *A = Args.getLastArg(options::OPT_ftarget_register_alloc_mode_EQ)) {
    StringRef RegAllocModeVal = A->getValue(0);
    auto ProcessElement = [&](StringRef Ele) {
      auto [DeviceName, RegAllocMode] = Ele.split(':');
      StringRef BackendOptName = SYCL::gen::getGenGRFFlag(RegAllocMode);
      bool IsDefault = RegAllocMode.equals("default");
      if (RegAllocMode.empty() || !DeviceName.equals("pvc") ||
          (BackendOptName.empty() && !IsDefault)) {
        getDriver().Diag(diag::err_drv_unsupported_option_argument)
            << A->getSpelling() << Ele;
      }
      // "default" means "provide no specification to the backend", so
      // we don't need to do anything here.
      if (IsDefault)
        return;
      if (IsGen) {
        // For AOT, Use ocloc's per-device options flag with the correct ocloc
        // option to honor the user's specification.
        PerDeviceArgs.push_back(
            {DeviceName, Args.MakeArgString("-options " + BackendOptName)});
      } else if (Triple.isSPIR() &&
                 Triple.getSubArch() == llvm::Triple::NoSubArch) {
        // For JIT, pass -ftarget-register-alloc-mode=Device:BackendOpt to
        // clang-offload-wrapper to be processed by the runtime.
        BeArgs.push_back(Args.MakeArgString(RegAllocModeOptName + DeviceName +
                                            ":" + BackendOptName));
      }
    };
    llvm::SmallVector<StringRef, 16> RegAllocModeArgs;
    RegAllocModeVal.split(RegAllocModeArgs, ',');
    for (StringRef Elem : RegAllocModeArgs)
      ProcessElement(Elem);
  } else if (DeviceOffloadKind == Action::OFK_OpenMP && !HostTC.getTriple().isWindowsMSVCEnvironment()) {
      // If -ftarget-register-alloc-mode is not specified, the default is
      // pvc:default on Windows and and pvc:auto otherwise.
      StringRef DeviceName = "pvc";
      StringRef BackendOptName = SYCL::gen::getGenGRFFlag("auto");
      if (IsGen)
        PerDeviceArgs.push_back(
            {DeviceName, Args.MakeArgString("-options " + BackendOptName)});
      else if (Triple.isSPIR() &&
               Triple.getSubArch() == llvm::Triple::NoSubArch) {
        BeArgs.push_back(Args.MakeArgString(RegAllocModeOptName + DeviceName +
                                            ":" + BackendOptName));
      }
    }
  if (IsGen) {
    // For GEN (spir64_gen) we have implied -device settings given usage
    // of intel_gpu_ as a target.  Handle those here, and also check that no
    // other -device was passed, as that is a conflict.
    StringRef DepInfo = JA.getOffloadingArch();
    if (!DepInfo.empty()) {
      ArgStringList TargArgs;
      Args.AddAllArgValues(TargArgs, options::OPT_Xs, options::OPT_Xs_separate);
      Args.AddAllArgValues(TargArgs, options::OPT_Xsycl_backend);
      // For -Xsycl-target-backend=<triple> we need to scrutinize the triple
      for (auto *A : Args) {
        if (!A->getOption().matches(options::OPT_Xsycl_backend_EQ))
          continue;
        if (StringRef(A->getValue()).startswith("intel_gpu"))
          TargArgs.push_back(A->getValue(1));
      }
      if (llvm::find_if(TargArgs, [&](auto Cur) {
            return !strncmp(Cur, "-device", sizeof("-device") - 1);
          }) != TargArgs.end()) {
        SmallString<64> Target("intel_gpu_");
        Target += DepInfo;
        getDriver().Diag(diag::err_drv_unsupported_opt_for_target)
            << "-device" << Target;
      }
      CmdArgs.push_back("-device");
      CmdArgs.push_back(Args.MakeArgString(DepInfo));
    }
    // -ftarget-compile-fast AOT
    if (Args.hasArg(options::OPT_ftarget_compile_fast))
      BeArgs.push_back("-igc_opts 'PartitionUnit=1,SubroutineThreshold=50000'");
    // -ftarget-export-symbols
    if (Args.hasFlag(options::OPT_ftarget_export_symbols,
                     options::OPT_fno_target_export_symbols, false))
      BeArgs.push_back("-library-compilation");
  } else if (Triple.getSubArch() == llvm::Triple::NoSubArch &&
             Triple.isSPIR()) {
    // -ftarget-compile-fast JIT
    Args.AddLastArg(BeArgs, options::OPT_ftarget_compile_fast);
  }
  if (IsGen) {
    for (auto [DeviceName, BackendArgStr] : PerDeviceArgs) {
      CmdArgs.push_back("-device_options");
      CmdArgs.push_back(Args.MakeArgString(DeviceName));
      CmdArgs.push_back(Args.MakeArgString(BackendArgStr));
    }
  }
  if (BeArgs.empty())
    return;
  if (Triple.getSubArch() == llvm::Triple::NoSubArch ||
      Triple.getSubArch() == llvm::Triple::SPIRSubArch_fpga) {
    for (StringRef A : BeArgs)
      CmdArgs.push_back(Args.MakeArgString(A));
    return;
  }
  SmallString<128> BeOpt;
  if (IsGen)
    CmdArgs.push_back("-options");
  else
    BeOpt = "--bo=";
  for (unsigned I = 0; I < BeArgs.size(); ++I) {
    if (I)
      BeOpt += ' ';
    BeOpt += BeArgs[I];
  }
  CmdArgs.push_back(Args.MakeArgString(BeOpt));
}

#if INTEL_CUSTOMIZATION
void SYCLToolChain::TranslateBackendTargetArgs(
    Action::OffloadKind DeviceOffloadKind, const llvm::Triple &Triple,
    const llvm::opt::ArgList &Args, llvm::opt::ArgStringList &CmdArgs,
    StringRef Device) const {
#endif // INTEL_CUSTOMIZATION
  // Handle -Xs flags.
  for (auto *A : Args) {
    // When parsing the target args, the -Xs<opt> type option applies to all
    // target compilations is not associated with a specific triple.  The
    // option can be used in 3 different ways:
    //   -Xs -DFOO -Xs -DBAR
    //   -Xs "-DFOO -DBAR"
    //   -XsDFOO -XsDBAR
    // All of the above examples will pass -DFOO -DBAR to the backend compiler.

    // Do not add the -Xs to the default SYCL triple (spir64) when we know we
    // have implied the setting.
    if ((A->getOption().matches(options::OPT_Xs) ||
         A->getOption().matches(options::OPT_Xs_separate)) &&
        Triple.getSubArch() == llvm::Triple::NoSubArch && Triple.isSPIR() &&
        getDriver().isSYCLDefaultTripleImplied())
      continue;

    if (A->getOption().matches(options::OPT_Xs)) {
      // Take the arg and create an option out of it.
      CmdArgs.push_back(Args.MakeArgString(Twine("-") + A->getValue()));
      A->claim();
      continue;
    }
    if (A->getOption().matches(options::OPT_Xs_separate)) {
      StringRef ArgString(A->getValue());
      parseTargetOpts(ArgString, Args, CmdArgs);
      A->claim();
      continue;
    }
  }
  // Do not process -Xsycl-target-backend for implied spir64
  if (Triple.getSubArch() == llvm::Triple::NoSubArch && Triple.isSPIR() &&
      getDriver().isSYCLDefaultTripleImplied())
    return;
#if INTEL_CUSTOMIZATION
  if (DeviceOffloadKind == Action::OFK_OpenMP)
    // Handle -Xopenmp-target-backend.
    TranslateTargetOpt(DeviceOffloadKind, Args, CmdArgs,
        options::OPT_Xopenmp_backend, options::OPT_Xopenmp_backend_EQ, Device);
  else
    // Handle -Xsycl-target-backend.
    TranslateTargetOpt(DeviceOffloadKind, Args, CmdArgs,
        options::OPT_Xsycl_backend, options::OPT_Xsycl_backend_EQ, Device);
#endif // INTEL_CUSTOMIZATION
  TranslateGPUTargetOpt(Args, CmdArgs, options::OPT_fsycl_targets_EQ);
}

#if INTEL_CUSTOMIZATION
void SYCLToolChain::TranslateLinkerTargetArgs(
    Action::OffloadKind DeviceOffloadKind, const llvm::Triple &Triple,
    const llvm::opt::ArgList &Args, llvm::opt::ArgStringList &CmdArgs) const {
  // Do not process -Xsycl-target-linker for implied spir64
  if (Triple.getSubArch() == llvm::Triple::NoSubArch && Triple.isSPIR() &&
      getDriver().isSYCLDefaultTripleImplied())
    return;
  if (DeviceOffloadKind == Action::OFK_OpenMP)
    // Handle -Xopenmp-target-linker.
    TranslateTargetOpt(DeviceOffloadKind, Args, CmdArgs,
        options::OPT_Xopenmp_linker, options::OPT_Xopenmp_linker_EQ,
        StringRef());
  else
    // Handle -Xsycl-target-linker.
    TranslateTargetOpt(DeviceOffloadKind, Args, CmdArgs,
        options::OPT_Xsycl_linker, options::OPT_Xsycl_linker_EQ, StringRef());
}
#endif // INTEL_CUSTOMIZATION

Tool *SYCLToolChain::buildBackendCompiler() const {
  if (getTriple().getSubArch() == llvm::Triple::SPIRSubArch_fpga)
    return new tools::SYCL::fpga::BackendCompiler(*this);
  if (getTriple().getSubArch() == llvm::Triple::SPIRSubArch_gen)
    return new tools::SYCL::gen::BackendCompiler(*this);
  // fall through is CPU.
  return new tools::SYCL::x86_64::BackendCompiler(*this);
}

Tool *SYCLToolChain::buildLinker() const {
  assert(getTriple().getArch() == llvm::Triple::spir ||
         getTriple().getArch() == llvm::Triple::spir64 || IsSYCLNativeCPU);
  return new tools::SYCL::Linker(*this);
}

void SYCLToolChain::addClangWarningOptions(ArgStringList &CC1Args) const {
  HostTC.addClangWarningOptions(CC1Args);
}

ToolChain::CXXStdlibType
SYCLToolChain::GetCXXStdlibType(const ArgList &Args) const {
  return HostTC.GetCXXStdlibType(Args);
}

void SYCLToolChain::AddSYCLIncludeArgs(const clang::driver::Driver &Driver,
                                       const ArgList &DriverArgs,
                                       ArgStringList &CC1Args) {
  // Add ../include/sycl, ../include/sycl/stl_wrappers and ../include (in that
  // order).
  SmallString<128> IncludePath(Driver.getInstalledDir());
  llvm::sys::path::append(IncludePath, "..");

#if INTEL_CUSTOMIZATION
#if INTEL_DEPLOY_UNIFIED_LAYOUT
  if (!llvm::sys::fs::exists(IncludePath + "/include/sycl")) {
    // Location of SYCL specific headers is <install>/include/sycl, which is
    // two levels up from the clang binary (Driver installed dir).
    llvm::sys::path::append(IncludePath, "..");
  }
#endif // INTEL_DEPLOY_UNIFIED_LAYOUT
#endif // INTEL_CUSTOMIZATION
  llvm::sys::path::append(IncludePath, "include");
  SmallString<128> SYCLPath(IncludePath);
  llvm::sys::path::append(SYCLPath, "sycl");
  // This is used to provide our wrappers around STL headers that provide
  // additional functions/template specializations when the user includes those
  // STL headers in their programs (e.g., <complex>).
  SmallString<128> STLWrappersPath(SYCLPath);
  llvm::sys::path::append(STLWrappersPath, "stl_wrappers");
  CC1Args.push_back("-internal-isystem");
  CC1Args.push_back(DriverArgs.MakeArgString(SYCLPath));
  CC1Args.push_back("-internal-isystem");
  CC1Args.push_back(DriverArgs.MakeArgString(STLWrappersPath));
  CC1Args.push_back("-internal-isystem");
  CC1Args.push_back(DriverArgs.MakeArgString(IncludePath));
}

void SYCLToolChain::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                              ArgStringList &CC1Args) const {
  HostTC.AddClangSystemIncludeArgs(DriverArgs, CC1Args);
}

void SYCLToolChain::AddClangCXXStdlibIncludeArgs(const ArgList &Args,
                                                 ArgStringList &CC1Args) const {
  HostTC.AddClangCXXStdlibIncludeArgs(Args, CC1Args);
}

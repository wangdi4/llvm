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
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/InputInfo.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

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
                                       Foreach, ForeachArgs, None);
  // FIXME: Add the FPGA specific timing diagnostic to the foreach call.
  // The foreach call obscures the return codes from the tool it is calling
  // to the compiler itself.
  addFPGATimingDiagnostic(Cmd, C);
  C.addCommand(std::move(Cmd));
}

// The list should match pre-built SYCL device library files located in
// compiler package. Once we add or remove any SYCL device library files,
// the list should be updated accordingly.
static llvm::SmallVector<StringRef, 16> SYCLDeviceLibList {
  "crt", "cmath", "cmath-fp64", "complex", "complex-fp64",
#if defined(_WIN32)
      "msvc-math",
#endif
      "imf", "imf-fp64", "itt-compiler-wrappers", "itt-stubs",
      "itt-user-wrappers", "fallback-cassert", "fallback-cstring",
      "fallback-cmath", "fallback-cmath-fp64", "fallback-complex",
      "fallback-complex-fp64", "fallback-imf", "fallback-imf-fp64"
};

#if INTEL_CUSTOMIZATION
// The list should match pre-built OMP device library files located in
// compiler package. Once we add or remove any OMP device library files,
// the list should be updated accordingly.
// The spirvdevicertl library is not included here as it is required to
//  be linked in fully (without --only-needed).
static llvm::SmallVector<StringRef, 10> OMPDeviceLibList{
    "cmath",
    "cmath-fp64",
    "complex",
    "complex-fp64",
    "fallback-cassert",
    "fallback-cstring",
    "fallback-cmath",
    "fallback-cmath-fp64",
    "fallback-complex",
    "fallback-complex-fp64",
    "itt-compiler-wrappers",
    "itt-stubs",
    "itt-user-wrappers"};
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
    auto isSYCLDeviceLib = [&C, this](const InputInfo &II) {
      const ToolChain *HostTC = C.getSingleOffloadToolChain<Action::OFK_Host>();
      StringRef LibPostfix = ".o";
      if (HostTC->getTriple().isWindowsMSVCEnvironment() &&
          C.getDriver().IsCLMode())
        LibPostfix = ".obj";
      std::string FileName = this->getToolChain().getInputFilename(II);
      StringRef InputFilename = llvm::sys::path::filename(FileName);
      if (this->getToolChain().getTriple().isNVPTX()) {
        // Linking SYCL Device libs requires libclc as well as libdevice
        if ((InputFilename.find("nvidiacl") != InputFilename.npos ||
             InputFilename.find("libdevice") != InputFilename.npos))
          return true;
        LibPostfix = ".cubin";
      }
      StringRef LibSyclPrefix("libsycl-");
      if (!InputFilename.startswith(LibSyclPrefix) ||
          !InputFilename.endswith(LibPostfix) || (InputFilename.count('-') < 2))
        return false;
      // Skip the prefix "libsycl-"
      StringRef PureLibName = InputFilename.substr(LibSyclPrefix.size());
      for (const auto &L : SYCLDeviceLibList) {
        if (PureLibName.startswith(L))
          return true;
      }
      return false;
    };
#if INTEL_CUSTOMIZATION
    auto isOMPDeviceLib = [&C](const InputInfo &II) {
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
      for (const auto &L : OMPDeviceLibList) {
        if (PureLibName.compare(L) == 0)
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
    if (LinkSYCLDeviceLibs)
      Opts.push_back("-only-needed");
    for (const auto &II : InputFiles) {
      std::string FileName = getToolChain().getInputFilename(II);

#if INTEL_CUSTOMIZATION
      if (isOMPDeviceLib(II)) {
        OMPObjs.push_back(II.getFilename());
      } else if (II.getType() == types::TY_Tempfilelist) {
#endif // INTEL_CUSTOMIZATION

        // Pass the unbundled list with '@' to be processed.
        Libs.push_back(C.getArgs().MakeArgString("@" + FileName));
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
    C.addCommand(std::make_unique<Command>(
        JA, *this, ResponseFileSupport::AtFileUTF8(), Exec, CmdArgs, None));
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
  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileUTF8(), Llc, LlcArgs, None));
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
          getToolChain().getTriple().isAMDGCN()) &&
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
  TC.AddImpliedTargetArgs(DeviceOffloadKind, CPUTriple, Args, CmdArgs); // INTEL
  // Add the target args passed in
#if INTEL_CUSTOMIZATION
  TC.TranslateBackendTargetArgs(DeviceOffloadKind, CPUTriple, Args, CmdArgs);
  TC.TranslateLinkerTargetArgs(DeviceOffloadKind, CPUTriple, Args, CmdArgs);
#endif // INTEL_CUSTOMIZATION

  SmallString<128> ExecPath(
      getToolChain().GetProgramPath(makeExeName(C, "opencl-aot")));
  const char *Exec = C.getArgs().MakeArgString(ExecPath);
  auto Cmd = std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                       Exec, CmdArgs, None);
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
  if (C.getDriver().isFPGAEmulationMode()) {
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
    // Output directory is based off of the first object name as captured
    // above.
    if (!CreatedReportName.empty())
      ReportOptArg += CreatedReportName;
  }
  if (!ReportOptArg.empty())
    CmdArgs.push_back(C.getArgs().MakeArgString(
        Twine("-output-report-folder=") + ReportOptArg));

#if INTEL_CUSTOMIZATION
  // Add any implied arguments before user defined arguments.
  TC.AddImpliedTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs);

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
                                       Exec, CmdArgs, None);
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

void SYCL::gen::BackendCompiler::ConstructJob(Compilation &C,
                                              const JobAction &JA,
                                              const InputInfo &Output,
                                              const InputInfoList &Inputs,
                                              const ArgList &Args,
                                              const char *LinkingOutput) const {
  assert((getToolChain().getTriple().getArch() == llvm::Triple::spir ||
          getToolChain().getTriple().getArch() == llvm::Triple::spir64) &&
         "Unsupported target");
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
  Action::OffloadKind DeviceOffloadKind(JA.getOffloadingDeviceKind()); // INTEL
  const toolchains::SYCLToolChain &TC =
      static_cast<const toolchains::SYCLToolChain &>(getToolChain());
#if INTEL_CUSTOMIZATION
  TC.AddImpliedTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs);
  TC.TranslateBackendTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs);
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
  auto Cmd = std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                       Exec, CmdArgs, None);
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
  TC.AddImpliedTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs);
  TC.TranslateBackendTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs);
  TC.TranslateLinkerTargetArgs(
      DeviceOffloadKind, getToolChain().getTriple(), Args, CmdArgs);
#endif // INTEL_CUSTOMIZATION
  SmallString<128> ExecPath(
      getToolChain().GetProgramPath(makeExeName(C, "opencl-aot")));
  const char *Exec = C.getArgs().MakeArgString(ExecPath);
  auto Cmd = std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                       Exec, CmdArgs, None);
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

SYCLToolChain::SYCLToolChain(const Driver &D, const llvm::Triple &Triple,
                             const ToolChain &HostTC, const ArgList &Args)
    : ToolChain(D, Triple, Args), HostTC(HostTC) {
  // Lookup binaries into the driver directory, this is used to
  // discover the clang-offload-bundler executable.
  getProgramPaths().push_back(getDriver().Dir);
#if INTEL_CUSTOMIZATION
  // getDriver() returns clang, which is not the Intel driver and may not be in
  // "bin". Ensure that we look in "bin" for programs. This is Intel-specific
  // because upstream doesn't typically have multiple program directories.
  SmallString<128> Bin(getDriver().Dir);
  llvm::sys::path::append(Bin, "..", "bin");
  llvm::sys::path::remove_dots(Bin, /*remove_dot_dot=*/ true);
  getProgramPaths().push_back(std::string(Bin));
#endif // INTEL_CUSTOMIZATION

  // Diagnose unsupported options only once.
  // All sanitizer options are not currently supported.
  for (auto A : Args.filtered(options::OPT_fsanitize_EQ))
    D.getDiags().Report(clang::diag::warn_drv_unsupported_option_for_target)
        << A->getAsString(Args) << getTriple().str();
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

  if (!DAL) {
    DAL = new DerivedArgList(Args.getBaseArgs());
    for (Arg *A : Args) {
      // Filter out any options we do not want to pass along to the device
      // compilation.
      switch ((options::ID)A->getOption().getID()) {
      case options::OPT_fsanitize_EQ:
        break;
      default:
        DAL->append(A);
        break;
      }
    }
  }
  // Strip out -O0 for FPGA Hardware device compilation.
  if (!getDriver().isFPGAEmulationMode() &&
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

// Expects a specific type of option (e.g. -Xsycl-target-backend) and will
// extract the arguments.
#if INTEL_CUSTOMIZATION
void SYCLToolChain::TranslateTargetOpt(Action::OffloadKind DeviceOffloadKind,
    const llvm::opt::ArgList &Args, llvm::opt::ArgStringList &CmdArgs,
    OptSpecifier Opt, OptSpecifier Opt_EQ) const {
#endif // INTEL_CUSTOMIZATION
  for (auto *A : Args) {
    bool OptNoTriple;
    OptNoTriple = A->getOption().matches(Opt);
    if (A->getOption().matches(Opt_EQ)) {
      // Passing device args: -X<Opt>=<triple> -opt=val.
#if INTEL_CUSTOMIZATION
      if (getDriver().MakeSYCLDeviceTriple(A->getValue()) != getTriple() &&
          A->getValue() != getTripleString())
#endif // INTEL_CUSTOMIZATION
        // Provided triple does not match current tool chain.
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

#if INTEL_CUSTOMIZATION
void SYCLToolChain::AddImpliedTargetArgs(
    Action::OffloadKind DeviceOffloadKind, const llvm::Triple &Triple,
    const llvm::opt::ArgList &Args, llvm::opt::ArgStringList &CmdArgs) const {
#endif // INTEL_CUSTOMIZATION
  // Current implied args are for debug information and disabling of
  // optimizations.  They are passed along to the respective areas as follows:
  //  FPGA and default device:  -g -cl-opt-disable
  //  GEN:  -options "-g -O0"
  //  CPU:  "--bo=-g -cl-opt-disable"
  llvm::opt::ArgStringList BeArgs;
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
  if (Args.getLastArg(options::OPT_O0) || IsMSVCOd)
#endif // INTEL_CUSTOMIZATION
    BeArgs.push_back("-cl-opt-disable");
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
    const llvm::opt::ArgList &Args, llvm::opt::ArgStringList &CmdArgs) const {
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
        options::OPT_Xopenmp_backend, options::OPT_Xopenmp_backend_EQ);
  else
    // Handle -Xsycl-target-backend.
    TranslateTargetOpt(DeviceOffloadKind, Args, CmdArgs,
        options::OPT_Xsycl_backend, options::OPT_Xsycl_backend_EQ);
#endif // INTEL_CUSTOMIZATION
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
        options::OPT_Xopenmp_linker, options::OPT_Xopenmp_linker_EQ);
  else
    // Handle -Xsycl-target-linker.
    TranslateTargetOpt(DeviceOffloadKind, Args, CmdArgs,
        options::OPT_Xsycl_linker, options::OPT_Xsycl_linker_EQ);
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
         getTriple().getArch() == llvm::Triple::spir64);
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
  // Add ../include/sycl and ../include (in that order)
  SmallString<128> P(Driver.getInstalledDir());
  llvm::sys::path::append(P, "..");
#if INTEL_CUSTOMIZATION
#if INTEL_DEPLOY_UNIFIED_LAYOUT
  if (!llvm::sys::fs::exists(P + "/include/sycl")) {
    // Location of SYCL specific headers is <install>/include/sycl, which is
    // two levels up from the clang binary (Driver installed dir).
    llvm::sys::path::append(P, "..");
  }
#endif // INTEL_DEPLOY_UNIFIED_LAYOUT
#endif // INTEL_CUSTOMIZATION
  llvm::sys::path::append(P, "include");
  SmallString<128> SYCLP(P);
  llvm::sys::path::append(SYCLP, "sycl");
  CC1Args.push_back("-internal-isystem");
  CC1Args.push_back(DriverArgs.MakeArgString(SYCLP));
  CC1Args.push_back("-internal-isystem");
  CC1Args.push_back(DriverArgs.MakeArgString(P));
}

void SYCLToolChain::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                              ArgStringList &CC1Args) const {
  HostTC.AddClangSystemIncludeArgs(DriverArgs, CC1Args);
}

void SYCLToolChain::AddClangCXXStdlibIncludeArgs(const ArgList &Args,
                                                 ArgStringList &CC1Args) const {
  HostTC.AddClangCXXStdlibIncludeArgs(Args, CC1Args);
}

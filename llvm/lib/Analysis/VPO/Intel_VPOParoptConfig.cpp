//===- Intel_VPOParoptConfig.cpp - Paropt Config Storage ------------------===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This is an immutable pass which stores the Paropt configuration
// information.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/Intel_VPOParoptConfig.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"

using namespace llvm;
using namespace llvm::yaml;

static cl::opt<std::string> ConfigFile(
    "vpo-paropt-config", cl::Hidden,
    cl::desc("A file with OpenMP outlining configuration"));

#define DEBUG_TYPE "vpo-paropt-config-analysis"

AnalysisKey VPOParoptConfigAnalysis::Key;

VPOParoptConfigAnalysis::Result VPOParoptConfigAnalysis::run(
    Module &M, ModuleAnalysisManager &AM) {
  return VPOParoptConfig(M.getContext());
}

char VPOParoptConfigWrapper::ID = 0;
INITIALIZE_PASS(VPOParoptConfigWrapper, DEBUG_TYPE,
                "VPO paropt config pass", false, true)

ImmutablePass *llvm::createVPOParoptConfigWrapperPass() {
  return new VPOParoptConfigWrapper();
}

VPOParoptConfigWrapper::VPOParoptConfigWrapper()
    : ImmutablePass(ID) {
  initializeVPOParoptConfigWrapperPass(*PassRegistry::getPassRegistry());
}

bool VPOParoptConfigWrapper::doInitialization(Module &M) {
  Result.reset(new VPOParoptConfig(M.getContext()));
  return false;
}

bool VPOParoptConfigWrapper::doFinalization(Module &M) {
  Result.reset();
  return false;
}

void VPOParoptConfigWrapper::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

namespace {
class VPCDiagInfo : public DiagnosticInfo {
  const SMDiagnostic &Diagnostic;

public:
  VPCDiagInfo(DiagnosticSeverity Severity, const SMDiagnostic &Diagnostic)
    : DiagnosticInfo(
          static_cast<DiagnosticKind>(getNextAvailablePluginDiagnosticKind()),
          Severity),
      Diagnostic(Diagnostic) {}

  void print(DiagnosticPrinter &DP) const override {
    DP << ConfigFile.ArgStr << ": " << Diagnostic;
  }
};
} // anonymous namespace

VPOParoptConfig::VPOParoptConfig(LLVMContext &Context) {
  auto ReportError =
      [&](const SMDiagnostic &Diag) {
        DiagnosticSeverity Severity;
        switch (Diag.getKind()) {
        case SourceMgr::DK_Error:
          Severity = DS_Error;
          break;
        case SourceMgr::DK_Warning:
          Severity = DS_Warning;
          break;
        default:
          llvm_unreachable("unsupported diag severity");
          break;
        }
        Context.diagnose(VPCDiagInfo(Severity, Diag));
      };

  if (ConfigFile.empty())
    return;

  ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
      MemoryBuffer::getFile(ConfigFile, /*IsText=*/true);
  if (std::error_code EC = FileOrErr.getError()) {
    ReportError(SMDiagnostic(ConfigFile, SourceMgr::DK_Error,
                             "Could not open input file: " + EC.message()));
    return;
  }

  yaml::Input YamlIn(FileOrErr.get()->getMemBufferRef());
  YamlIn >> Config;

  if (std::error_code EC = YamlIn.error())
    ReportError(SMDiagnostic(ConfigFile, SourceMgr::DK_Error,
                             "Could not parse YAML: " + EC.message()));
}

Optional<const vpo::KernelConfig>
    VPOParoptConfig::getKernelConfig(StringRef Name) const {
  for (const auto &KC : Config.KernelEntries)
    if (Name.str().find(KC.Name) != std::string::npos)
      return KC;

  return None;
}

uint8_t VPOParoptConfig::getKernelSPMDSIMDWidth(StringRef Name) const {
  auto KC = getKernelConfig(Name);
  if (!KC)
    return 0;

  return KC->SPMDSIMDWidth;
}

uint64_t VPOParoptConfig::getKernelThreadLimit(StringRef Name) const {
  auto KC = getKernelConfig(Name);
  if (!KC)
    return 0;

  return KC->ThreadLimit;
}

uint64_t VPOParoptConfig::getKernelNumTeams(StringRef Name) const {
  auto KC = getKernelConfig(Name);
  if (!KC)
    return 0;

  return KC->NumTeams;
}

vpo::RegisterAllocationMode
VPOParoptConfig::getRegisterAllocMode(StringRef Name) const {
  auto KC = getKernelConfig(Name);
  if (!KC)
    return vpo::RegisterAllocationMode::DEFAULT;

  return KC->RegisterAllocMode;
}

namespace llvm {
namespace yaml {

template <> struct ScalarEnumerationTraits<vpo::RegisterAllocationMode> {
  static void enumeration(IO &io, vpo::RegisterAllocationMode &value) {
    io.enumCase(value, "auto", vpo::RegisterAllocationMode::AUTO);
    io.enumCase(value, "small", vpo::RegisterAllocationMode::SMALL);
    io.enumCase(value, "large", vpo::RegisterAllocationMode::LARGE);
    io.enumCase(value, "default", vpo::RegisterAllocationMode::DEFAULT);
  }
};
} // namespace yaml
} // namespace llvm

// YAML IO mappings.
void MappingTraits<vpo::KernelConfig>::mapping(
    IO &IO, vpo::KernelConfig &KernelConfig) {
  IO.mapRequired("Name", KernelConfig.Name);
  IO.mapOptional("SPMDSIMDWidth", KernelConfig.SPMDSIMDWidth, 0);
  IO.mapOptional("ThreadLimit", KernelConfig.ThreadLimit, 0);
  IO.mapOptional("NumTeams", KernelConfig.NumTeams, 0);
  IO.mapOptional("RegisterAllocMode", KernelConfig.RegisterAllocMode,
                 vpo::RegisterAllocationMode::DEFAULT);
}

void MappingTraits<vpo::Config>::mapping(
    IO &IO, vpo::Config &Config) {
  IO.mapTag("!ParoptConfig", true);
  IO.mapOptional("Kernels", Config.KernelEntries);
}

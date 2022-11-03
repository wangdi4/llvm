// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

//===- NewPMDriver.cpp - Driver for opt with new PM -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
///
/// This file is just a split of the code that logically belongs in opt.cpp but
/// that includes the new pass manager headers.
///
//===----------------------------------------------------------------------===//

#include "NewPMDriver.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Bitcode/BitcodeWriterPass.h"
#include "llvm/Config/config.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/IPO/ThinLTOBitcodeWriter.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Utils/Debugify.h"

using namespace llvm;
using namespace opt_tool;

static cl::opt<bool>
    DebugPM("debug-pass-manager", cl::Hidden,
            cl::desc("Print pass management debugging information"));

// This flag specifies a textual description of the alias analysis pipeline to
// use when querying for aliasing information. It only works in concert with
// the "passes" flag above.
static cl::opt<std::string>
    AAPipeline("aa-pipeline",
               cl::desc("A textual description of the alias analysis "
                        "pipeline for handling managed aliasing queries"),
               cl::Hidden);

/// {{@ These options accept textual pipeline descriptions which will be
/// inserted into default pipelines at the respective extension points
static cl::opt<std::string> PeepholeEPPipeline(
    "passes-ep-peephole",
    cl::desc("A textual description of the function pass pipeline inserted at "
             "the Peephole extension points into default pipelines"),
    cl::Hidden);
static cl::opt<std::string> LateLoopOptimizationsEPPipeline(
    "passes-ep-late-loop-optimizations",
    cl::desc(
        "A textual description of the loop pass pipeline inserted at "
        "the LateLoopOptimizations extension point into default pipelines"),
    cl::Hidden);
static cl::opt<std::string> LoopOptimizerEndEPPipeline(
    "passes-ep-loop-optimizer-end",
    cl::desc("A textual description of the loop pass pipeline inserted at "
             "the LoopOptimizerEnd extension point into default pipelines"),
    cl::Hidden);
static cl::opt<std::string> ScalarOptimizerLateEPPipeline(
    "passes-ep-scalar-optimizer-late",
    cl::desc("A textual description of the function pass pipeline inserted at "
             "the ScalarOptimizerLate extension point into default pipelines"),
    cl::Hidden);
static cl::opt<std::string> CGSCCOptimizerLateEPPipeline(
    "passes-ep-cgscc-optimizer-late",
    cl::desc("A textual description of the cgscc pass pipeline inserted at "
             "the CGSCCOptimizerLate extension point into default pipelines"),
    cl::Hidden);
static cl::opt<std::string> VectorizerStartEPPipeline(
    "passes-ep-vectorizer-start",
    cl::desc("A textual description of the function pass pipeline inserted at "
             "the VectorizerStart extension point into default pipelines"),
    cl::Hidden);
enum PGOKind { NoPGO, InstrGen, InstrUse, SampleUse };
static cl::opt<PGOKind> PGOKindFlag(
    "pgo-kind", cl::init(NoPGO), cl::Hidden,
    cl::desc("The kind of profile guided optimization"),
    cl::values(clEnumValN(NoPGO, "nopgo", "Do not use PGO."),
               clEnumValN(InstrGen, "new-pm-pgo-instr-gen-pipeline",
                          "Instrument the IR to generate profile."),
               clEnumValN(InstrUse, "new-pm-pgo-instr-use-pipeline",
                          "Use instrumented profile to guide PGO."),
               clEnumValN(SampleUse, "new-pm-pgo-sample-use-pipeline",
                          "Use sampled profile to guide PGO.")));
static cl::opt<std::string>
    ProfileFile("profile-file", cl::desc("Path to the profile."), cl::Hidden);

static cl::opt<std::string>
    ProfileRemappingFile("profile-remapping-file",
                         cl::desc("Path to the profile remapping."),
                         cl::Hidden);

static cl::opt<bool> DebugInfoForProfiling(
    "new-pm-debug-info-for-profiling", cl::init(false), cl::Hidden,
    cl::desc("Emit special debug info to enable PGO profile generation."));
/// @}}

template <typename PassManagerT>
bool tryParsePipelineText(PassBuilder &PB, StringRef PipelineText) {
  if (PipelineText.empty())
    return false;

  // Verify the pipeline is parseable:
  PassManagerT PM;
  if (PB.parsePassPipeline(PM, PipelineText))
    return true;

  errs() << "Could not parse pipeline '" << PipelineText
         << "'. I'm going to igore it.\n";
  return false;
}

/// If one of the EPPipeline command line options was given, register callbacks
/// for parsing and inserting the given pipeline
static void registerEPCallbacks(PassBuilder &PB, bool /*DebugLogging*/) {
  if (tryParsePipelineText<FunctionPassManager>(PB, PeepholeEPPipeline))
    PB.registerPeepholeEPCallback(
        [&PB](FunctionPassManager &PM, OptimizationLevel /*Level*/) {
          (void)PB.parsePassPipeline(PM, PeepholeEPPipeline);
        });
  if (tryParsePipelineText<LoopPassManager>(PB,
                                            LateLoopOptimizationsEPPipeline))
    PB.registerLateLoopOptimizationsEPCallback(
        [&PB](LoopPassManager &PM, OptimizationLevel /*Level*/) {
          (void)PB.parsePassPipeline(PM, LateLoopOptimizationsEPPipeline);
        });
  if (tryParsePipelineText<LoopPassManager>(PB, LoopOptimizerEndEPPipeline))
    PB.registerLoopOptimizerEndEPCallback(
        [&PB](LoopPassManager &PM, OptimizationLevel /*Level*/) {
          (void)PB.parsePassPipeline(PM, LoopOptimizerEndEPPipeline);
        });
  if (tryParsePipelineText<FunctionPassManager>(PB,
                                                ScalarOptimizerLateEPPipeline))
    PB.registerScalarOptimizerLateEPCallback(
        [&PB](FunctionPassManager &PM, OptimizationLevel /*Level*/) {
          (void)PB.parsePassPipeline(PM, ScalarOptimizerLateEPPipeline);
        });
  if (tryParsePipelineText<CGSCCPassManager>(PB, CGSCCOptimizerLateEPPipeline))
    PB.registerCGSCCOptimizerLateEPCallback(
        [&PB](CGSCCPassManager &PM, OptimizationLevel /*Level*/) {
          (void)PB.parsePassPipeline(PM, CGSCCOptimizerLateEPPipeline);
        });
  if (tryParsePipelineText<FunctionPassManager>(PB, VectorizerStartEPPipeline))
    PB.registerVectorizerStartEPCallback(
        [&PB](FunctionPassManager &PM, OptimizationLevel /*Level*/) {
          (void)PB.parsePassPipeline(PM, VectorizerStartEPPipeline);
        });
}

#ifdef LINK_POLLY_INTO_TOOLS
namespace polly {
void RegisterPollyPasses(PassBuilder &);
}
#endif

bool llvm::runPassPipeline(StringRef Arg0, Module &M, TargetMachine *TM,
                           ToolOutputFile *Out, ToolOutputFile *ThinLTOLinkOut,
                           ToolOutputFile *OptRemarkFile,
                           StringRef PassPipeline, OutputKind OK,
                           VerifierKind VK,
                           bool ShouldPreserveAssemblyUseListOrder,
                           bool ShouldPreserveBitcodeUseListOrder,
                           bool EmitSummaryIndex, bool EmitModuleHash) {
  Optional<PGOOptions> P;
  switch (PGOKindFlag) {
  case InstrGen:
    P = PGOOptions(ProfileFile, "", "", PGOOptions::IRInstr);
    break;
  case InstrUse:
    P = PGOOptions(ProfileFile, "", ProfileRemappingFile, PGOOptions::IRUse);
    break;
  case SampleUse:
    P = PGOOptions(ProfileFile, "", ProfileRemappingFile,
                   PGOOptions::SampleUse);
    break;
  case NoPGO:
    if (DebugInfoForProfiling)
      P = PGOOptions("", "", "", PGOOptions::NoAction, PGOOptions::NoCSAction,
                     true);
    else
      P = None;
  }
  PassBuilder PB(TM, PipelineTuningOptions(), P);
  registerEPCallbacks(PB, DebugPM);

#ifdef LINK_POLLY_INTO_TOOLS
  polly::RegisterPollyPasses(PB);
#endif

  // Specially handle the alias analysis manager so that we can register
  // a custom pipeline of AA passes with it.
  AAManager AA;
  if (!PB.parseAAPipeline(AA, AAPipeline)) {
    errs() << Arg0 << ": unable to parse AA pipeline description.\n";
    return false;
  }

  LoopAnalysisManager LAM;
  FunctionAnalysisManager FAM;
  CGSCCAnalysisManager CGAM;
  ModuleAnalysisManager MAM;

  // Register the AA manager first so that our version is the one used.
  FAM.registerPass([&] { return std::move(AA); });

  // Register all the basic analyses with the managers.
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  ModulePassManager MPM;

  // Register a callback that creates the debugify passes as needed.
  PB.registerPipelineParsingCallback(
      [](StringRef Name, ModulePassManager &MPM,
         ArrayRef<PassBuilder::PipelineElement>) {
        if (Name == "debugify") {
          MPM.addPass(NewPMDebugifyPass());
          return true;
        } else if (Name == "check-debugify") {
          MPM.addPass(NewPMCheckDebugifyPass());
          return true;
        }
        return false;
      });

  if (VK > VK_NoVerifier)
    MPM.addPass(VerifierPass());

  if (!PB.parsePassPipeline(MPM, PassPipeline)) {
    errs() << Arg0 << ": unable to parse pass pipeline description.\n";
    return false;
  }

  if (VK > VK_NoVerifier)
    MPM.addPass(VerifierPass());

  // Add any relevant output pass at the end of the pipeline.
  switch (OK) {
  case OK_NoOutput:
    break; // No output pass needed.
  case OK_OutputAssembly:
    MPM.addPass(
        PrintModulePass(Out->os(), "", ShouldPreserveAssemblyUseListOrder));
    break;
  case OK_OutputBitcode:
    MPM.addPass(BitcodeWriterPass(Out->os(), ShouldPreserveBitcodeUseListOrder,
                                  EmitSummaryIndex, EmitModuleHash));
    break;
  case OK_OutputThinLTOBitcode:
    MPM.addPass(ThinLTOBitcodeWriterPass(
        Out->os(), ThinLTOLinkOut ? &ThinLTOLinkOut->os() : nullptr));
    break;
  }

  // Before executing passes, print the final values of the LLVM options.
  cl::PrintOptionValues();

  // Now that we have all of the passes ready, run them.
  MPM.run(M, MAM);

  // Declare success.
  if (OK != OK_NoOutput) {
    Out->keep();
    if (OK == OK_OutputThinLTOBitcode && ThinLTOLinkOut)
      ThinLTOLinkOut->keep();
  }

  if (OptRemarkFile)
    OptRemarkFile->keep();

  return true;
}

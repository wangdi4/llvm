//=== Intel_UnpredictableProfileLoader.cpp - Unpredictable Profile Loader -===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass reads a sample profile containing mispredict counts rather than
// execution counts and adds !unpredictable metadata to branch or select
// instructions with corresponding debug locations.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_UnpredictableProfileLoader.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/ProfileData/SampleProf.h"
#include "llvm/ProfileData/SampleProfReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;

#define DEBUG_TYPE "unpredictable-profile-loader"

static cl::opt<std::string> UnpredictableHintsFile(
    "unpredictable-hints-file",
    cl::desc("Path to the unpredictability hints profile"), cl::Hidden);

static cl::opt<unsigned> MinimumWeight(
    "unpredictable-hints-min-weight",
    cl::desc("Absolute minimum profile weight to apply MD_unpredictable from"),
    cl::init(0), cl::Hidden);

// Lookup samples for an Instruction's corresponding location in a
// FunctionSamples profile. The count returned is directly from the profile
// representing the number of samples seen.
ErrorOr<uint64_t> UnpredictableProfileLoaderPass::getUnpredictableHint(
    const FunctionSamples *TopSamples, const Instruction *I) {

  if (const auto &Loc = I->getDebugLoc())
    if (const ErrorOr<uint64_t> Samples = TopSamples->findSamplesAt(
            FunctionSamples::getOffset(Loc), Loc->getBaseDiscriminator()))
      if (uint64_t Count = Samples.get())
        return Count;

  return std::error_code();
}

// Examine all Select and BranchInsts in a function, adding !unpredictable
// metadata if they appear in the mispredict profile with sufficient weight.
bool UnpredictableProfileLoaderPass::addUpredictableMetadata(Function &F) {

  const FunctionSamples *Samples = Reader->getSamplesFor(F);
  if (!Samples)
    return false;

  bool MadeChange = false;
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      if (!isa<BranchInst>(&I) && !isa<SelectInst>(&I) && !isa<SwitchInst>(&I))
        continue;
      if (I.hasMetadata(LLVMContext::MD_unpredictable))
        continue;

      const ErrorOr<uint64_t> Hints = getUnpredictableHint(Samples, &I);
      if (!Hints)
        continue;

      LLVM_DEBUG(dbgs() << "Found a hint that the following instruction is "
                           "unpredictable with weight "
                        << Hints.get() << ":\n");
      LLVM_DEBUG(dbgs() << I << "\n");

      if (Hints.get() < MinimumWeight) {
        LLVM_DEBUG(dbgs() << "\tWeight " << Hints.get()
                          << " is below threshold of " << MinimumWeight
                          << "; ignoring.\n");
        continue;
      }

      // In the future we probably want to attach more information here, such as
      // the mispredict count or ratio.
      MDNode *MD = MDNode::get(I.getContext(), std::nullopt);
      I.setMetadata(LLVMContext::MD_unpredictable, MD);
      MadeChange = true;
    }
  }

  return MadeChange;
}

bool UnpredictableProfileLoaderPass::addUpredictableMetadata(Module &M) {
  bool MadeChange = false;

  for (Function &F : M)
    MadeChange |= addUpredictableMetadata(F);

  // Return an indication of whether we changed anything or not.
  return MadeChange;
}

bool UnpredictableProfileLoaderPass::loadSampleProfile(Module &M) {
  if (Reader)
    return true;

  if (UnpredictableHintsFile.empty())
    return false;

  LLVMContext &Ctx = M.getContext();
  auto FS = vfs::getRealFileSystem();
  ErrorOr<std::unique_ptr<SampleProfileReader>> ReaderOrErr =
      SampleProfileReader::create(UnpredictableHintsFile, Ctx, *FS);
  if (std::error_code EC = ReaderOrErr.getError()) {
    std::string Msg = "Could not open profile: " + EC.message();
    Ctx.diagnose(DiagnosticInfoSampleProfile(UnpredictableHintsFile, Msg,
                                             DiagnosticSeverity::DS_Warning));
    return false;
  }

  Reader = std::move(ReaderOrErr.get());
  Reader->read();
  LLVM_DEBUG(dbgs() << "Successfully read profile " << UnpredictableHintsFile
                    << "\n");
  return true;
}

PreservedAnalyses UnpredictableProfileLoaderPass::run(Module &M,
                                                      ModuleAnalysisManager &) {
  if (!loadSampleProfile(M))
    return PreservedAnalyses::all();

  if (addUpredictableMetadata(M)) {
    PreservedAnalyses PA;
    PA.preserveSet<CFGAnalyses>();
    return PA;
  }

  return PreservedAnalyses::all();
}

//===- Inference.cpp - Inference --------------------------------*- C++ -*-===//
//
// Copyright (C) 2023 Intel Corporation
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
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Instrumentation/Intel_MLPGO/Inference.h"

#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Comdat.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/EHPersonalities.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/ProfileSummary.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/ExtractFeatures.h"
#include "llvm/Transforms/Instrumentation/PGOInstrumentation.h"

#include <cmath>
#include <set>
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "mlpgo"

static void MLPGOInferFunction(Module *M, Function &F, CallGraph &CG,
                               std::unique_ptr<mlpgo::Model> &Model,
                               mlpgo::Parameters &Parameter) {
  // Analysis Requirement
  DominatorTree DT{F};
  PostDominatorTree PostDT{F};
  LoopInfo LI{DT};
  BranchProbabilityInfo BPI(F, LI, nullptr, &DT, &PostDT);
  BranchProbabilityInfo::SccInfo Scc = BranchProbabilityInfo::SccInfo(F);

  SmallVector<std::pair<const BasicBlock *, const BasicBlock *>> BackEdges;
  std::set<std::pair<const BasicBlock *, const BasicBlock *>> BackEdgesSet;

  FindFunctionBackedges(F, BackEdges);
  for (auto &BackEdge : BackEdges)
    BackEdgesSet.insert(BackEdge);

  mlpgo::ProcedureType ProcType;
  ProcType = mlpgo::GetProcedureType(F, CG);

  unsigned int EdgesCountInCFG = 0;
  mlpgo::CalcEdgesInFunction(F, EdgesCountInCFG);

  for (BasicBlock &BB : F) {
    Instruction *TI = BB.getTerminator();

    if (!mlpgo::TerminatorInst::isSupportedBrInst(TI))
      continue;

    std::optional<mlpgo::MLBrFeatureVec> IF =
        mlpgo::ExtractInstFeatures(TI, F, ProcType, LI, DT, PostDT, Scc,
                                   BackEdgesSet, Parameter, BPI, BPI, true);

    if (!IF)
      continue;

    IF->getSrcBBFeatures().srcFunctionEdgesSize = EdgesCountInCFG;

    MDBuilder MDB(M->getContext());

    std::vector<unsigned int> Probs = Model->inference(*IF);

    // Probabilities vector might be empty if there is an issue with inferencing
    if (Probs.empty())
      continue;

    TI->setMetadata(LLVMContext::MD_prof,
                    MDB.createBranchWeights(ArrayRef<uint32_t>(Probs)));
  }
}

PreservedAnalyses MLPGOInference::run(Module &M, ModuleAnalysisManager &AM) {
  auto &CG = AM.getResult<CallGraphAnalysis>(M);
  mlpgo::Parameters MLPGOParameters(M);

  std::unique_ptr<mlpgo::Model> Model(new mlpgo::Model());
  if (!Model->ok()) {
    LLVM_DEBUG(llvm::dbgs() << "Failed to load models.\n");
    return PreservedAnalyses::all();
  }

  for (Function &F : M) {
    if (F.isDeclaration())
      continue;

    MLPGOInferFunction(&M, F, CG, Model, MLPGOParameters);
  }

  return PreservedAnalyses::none();
}

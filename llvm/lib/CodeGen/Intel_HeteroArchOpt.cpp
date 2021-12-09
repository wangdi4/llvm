//===- Intel_HeteroArchOpt.cpp - Hetero Arch (such as ADL) Optimization ---===//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/CostModel.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

using namespace llvm;

#define DEBUG_TYPE "hetero-arch-opt"

// Defines the smallest loop depth in order to
// apply Hetero Arch Optimization.
static cl::opt<uint32_t>
    HeteroArchLoopDepthThreshold("hetero-arch-loop-depth-threshold",
                                 cl::init(2), cl::ReallyHidden);

// Defines the biggest basic block number in order to
// apply Hetero Arch Optimization.
static cl::opt<uint32_t>
    HeteroArchBBNumThreshold("hetero-arch-bb-num-threshold", cl::init(1),
                             cl::ReallyHidden);

// Defines the smallest percentage of gather instruction cost in order to
// apply Hetero Arch Optimization.
static cl::opt<uint32_t>
    HeteroArchGatherCostThreshold("hetero-arch-gather-cost-threshold",
                                  cl::init(45), cl::ReallyHidden);

// Defines the smallest density of gather instructions in order to
// apply Hetero Arch Optimization.
static cl::opt<uint32_t>
    HeteroArchGatherDensityThreshold("hetero-arch-gather-density-threshold",
                                     cl::init(8), cl::ReallyHidden);

namespace {

class HeteroArchOpt : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  HeteroArchOpt() : FunctionPass(ID) {
    initializeHeteroArchOptPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addPreserved<TargetTransformInfoWrapperPass>();
  }

  bool runOnFunction(Function &F) override;

private:
  // An extension of ValueToValueMapTy which is used for cloning function body
  // (added template to cast cloned value to the original value type). Defining
  // it here because function-level templates are not allowed.
  struct Value2CloneMapTy : public ValueToValueMapTy {
    template <typename T> T *getClone(const T *V) const {
      auto It = this->find(V);
      // assert(It != this->end() && "no clone for the given value");
      if (It == this->end()) {
        LLVM_DEBUG(dbgs() << "!!!no clone for the given value!!!\n");
        return nullptr;
      }
      return cast<T>(It->second);
    }
  };

  unsigned maxLoopDepth(Loop *L) {
    unsigned MaxDepth = 0;
    if (L->isInnermost())
      return 1;
    for (auto *SubLoop : *L) {
      unsigned Depth = maxLoopDepth(SubLoop);
      if (Depth > MaxDepth)
        MaxDepth = Depth;
    }
    return MaxDepth + 1;
  }

  void processLoop(Loop *L);
  void createMultiVersion();
  bool optGather();
  LoopInfo *LI = nullptr;
  TargetTransformInfo *TTI = nullptr;
  SmallVector<IntrinsicInst *, 8> CandidateInsts;
  Function *CurFn = nullptr;
};

} // end anonymous namespace

char HeteroArchOpt::ID = 0;

char &llvm::HeteroArchOptID = HeteroArchOpt::ID;

INITIALIZE_PASS(HeteroArchOpt, DEBUG_TYPE, "Hetero Arch Optimization", false,
                false)

FunctionPass *llvm::createHeteroArchOptPass() { return new HeteroArchOpt(); }

bool HeteroArchOpt::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  if (F.hasOptSize())
    return false; // This opt will bloat code.

  if (!F.hasFnAttribute("target-cpu"))
    return false;

  StringRef CPU = F.getFnAttribute("target-cpu").getValueAsString();
  if (CPU != "alderlake")
    return false;

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  CurFn = &F;

  bool MadeChange = false;
  if (optGather())
    MadeChange |= true;
  return MadeChange;
}

bool HeteroArchOpt::optGather() {
  for (auto *Lp : *LI)
    processLoop(Lp);

  if (!CandidateInsts.size())
    return false;

  createMultiVersion();
  return true;
}

void HeteroArchOpt::processLoop(Loop *L) {
  unsigned MaxLD = maxLoopDepth(L);
  if (MaxLD < HeteroArchLoopDepthThreshold)
    return;
  float Factor =
      std::min((float)4.0, (float)MaxLD / HeteroArchLoopDepthThreshold);
  std::map<Loop *, SmallSetVector<IntrinsicInst *, 8>> LoopCandidates;
  // Collect candidates per loop
  for (BasicBlock *BB : L->blocks()) {
    BasicBlock::iterator CurInstIterator = BB->begin();
    while (CurInstIterator != BB->end()) {
      if (CallInst *CI = dyn_cast<CallInst>(&*CurInstIterator++)) {
        IntrinsicInst *II = dyn_cast<IntrinsicInst>(CI);
        if (II && Intrinsic::masked_gather == II->getIntrinsicID()) {
          if (!TTI->shouldScalarizeMaskedGather(CI)) {
            Loop *InnermostLoop = LI->getLoopFor(BB);
            LoopCandidates[InnermostLoop].insert(II);
          }
        }
      }
    }
  }
  // Identify qualified candidates based on heuristics and cost model.
  for (auto LC : LoopCandidates) {
    auto LoopDepth = LC.first->getLoopDepth();
    if (LoopDepth < HeteroArchLoopDepthThreshold)
      continue;
    auto NumBB = LC.first->getNumBlocks();
    if (NumBB > HeteroArchBBNumThreshold)
      continue;

    unsigned int TotalCost = 0;
    unsigned int GatherCost = 0;
    unsigned int TotalInst = 0;
    unsigned int GatherInst = LC.second.size();
    for (BasicBlock *BB : LC.first->blocks()) {
      for (Instruction &Inst : *BB) {
        InstructionCost Cost = TTI->getInstructionCost(
            &Inst, TargetTransformInfo::TCK_RecipThroughput);
        auto CostVal = Cost.getValue();
        if (!CostVal) {
          LLVM_DEBUG(dbgs()
                     << "Invalid cost for instruction: " << Inst << "\n");
          continue;
        }
        LLVM_DEBUG(dbgs() << "Estimated cost: " << *CostVal
                          << " for instruction: " << Inst << "\n");
        TotalCost += *CostVal;
        if (*CostVal)
          TotalInst++; // only consider instructions resulting in code
        IntrinsicInst *II = dyn_cast<IntrinsicInst>(&Inst);
        if (II && LC.second.count(II))
          GatherCost += *CostVal;
      }
    }

    if (GatherCost * Factor * 100 < TotalCost * HeteroArchGatherCostThreshold &&
        GatherInst * Factor * HeteroArchGatherDensityThreshold < TotalInst)
      continue;

    LLVM_DEBUG(dbgs() << "\nHetero Arch Opt Candidate in " << CurFn->getName()
                      << "\nNumBB: " << NumBB << "\tLoopDepth: " << LoopDepth
                      << "\tTotalCost: " << TotalCost << "\tGatherCost: "
                      << GatherCost << "\tTotalInst: " << TotalInst
                      << "\tGatherInst: " << GatherInst << "\n");

    LLVM_DEBUG(for (BasicBlock *BB : LC.first->blocks()) BB->dump());

    for (auto II : LC.second)
      CandidateInsts.push_back(II);
  }
}

void HeteroArchOpt::createMultiVersion() {
  LLVM_DEBUG(dbgs() << "Hetero Arch Opt create MultiVersion for function: "
                    << CurFn->getName() << "\n");
  // Create a clone of the whole function body.
  SmallVector<BasicBlock *, 32> OrigBBs;
  SmallVector<BasicBlock *, 32> CloneBBs;
  Value2CloneMapTy VMap;

  for (auto &BB : *CurFn)
    OrigBBs.push_back(&BB);
  for (auto *OBB : OrigBBs) {
    auto *CBB = CloneBasicBlock(OBB, VMap, ".clone", CurFn);
    VMap[OBB] = CBB;
    CloneBBs.push_back(CBB);
  }
  remapInstructionsInBlocks(CloneBBs, VMap);

  // Create a new entry block with branching code which transfers control to
  // the 'true'/'false' specialization of the function body.
  auto *OldEntryBB = &CurFn->getEntryBlock();
  auto *NewEntryBB =
      BasicBlock::Create(CurFn->getContext(), "entry.new", CurFn, OldEntryBB);
  IRBuilder<> IRB(NewEntryBB);

  // Create runtime check based on hybrid information enumeration leaf.
  CallInst *CpuId = IRB.CreateIntrinsic(Intrinsic::x86_cpuid, None,
                                        {IRB.getInt32(0x1a), IRB.getInt32(0)});
  Value *EAX = IRB.CreateExtractValue(CpuId, 0, "eax");

  // Bits 31-24 is core type. 20H is intel atom. 40H is intel core.
  Value *CoreType = IRB.CreateLShr(EAX, IRB.getInt32(24));
  Value *IsCore = IRB.CreateICmpEQ(CoreType, IRB.getInt32(0x40));

  // True branch for core and False branch for atom.
  IRB.CreateCondBr(IsCore, OldEntryBB, VMap.getClone(OldEntryBB));

  // Create meta-data for cloned candidate gather instructions (on small core).
  for (IntrinsicInst *II : CandidateInsts) {
    auto *Clone = VMap.getClone(II);
    if (!Clone)
      continue;
    Clone->setMetadata("hetero.arch.opt.disable.gather",
                       MDNode::get(Clone->getContext(), {}));
    assert(TTI->shouldScalarizeMaskedGather(Clone));
  }

  CandidateInsts.clear();
}

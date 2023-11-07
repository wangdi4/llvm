//==--------------- ReduceCrossBarrierValues.cpp ---------------- C++ -*---==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/ReduceCrossBarrierValues.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/SYCLTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/DataPerBarrierPass.h"
#include "llvm/Transforms/SYCLTransforms/DataPerValuePass.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierRegionInfo.h"
#include "llvm/Transforms/SYCLTransforms/Utils/RuntimeService.h"
#include "llvm/Transforms/SYCLTransforms/WIRelatedValuePass.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-reduce-cross-barrier-values"

static cl::opt<unsigned> SYCLBarrierCopyInstructionThreshold(
    "sycl-barrier-copy-instruction-threshold", cl::init(15), cl::Hidden);

using UseSet = DataPerValue::UseSet;
using InstMap = DenseMap<Instruction *, Instruction *>;

static bool isSafeToCopy(Instruction *Inst, RuntimeService &RTS) {
  // Alloca can't be copied.
  if (isa<AllocaInst>(Inst))
    return false;

  // A few TID builtins's memory(none) attribute is removed in AddFunctionAttrs
  // pass. Check their names before checking if it may read or write memory.
  if (auto *Call = dyn_cast<CallBase>(Inst)) {
    Function *CalledFunc = Call->getCalledFunction();
    if (CalledFunc && RTS.isSafeToSpeculativeExecute(CalledFunc->getName()))
      return true;
  }

  // TODO:
  // If the instruction is a load, we must guarantee that all paths from where
  // it's defined to where it will be cloned have no write to the memory it
  // loads from. This could be time-consuming. But maybe we can do this if the
  // source and the target aren't far?
  if (Inst->mayReadOrWriteMemory())
    return false;

  if (Inst->isTerminator() || isa<PHINode>(Inst) || Inst->isEHPad() ||
      Inst->mayThrow())
    return false;

  // For other function calls, cloning them can be more expensive than save
  // to/load from special buffer. So we choose to bail out.
  if (isa<CallBase>(Inst))
    return false;

  return true;
}

/// Returns true if it's legal to clone all dependent instructions used by the
/// user of \p TheUse. All dependencies are saved into \p Uses.
static bool collectDependencies(Use *TheUse, size_t Threshold,
                                DataPerBarrier *DPB, WIRelatedValue *WIRV,
                                RuntimeService &RTS,
                                SmallVectorImpl<Use *> &Uses) {
  auto *User = TheUse->getUser();

  // If the user is a call instruction whose callee contains barriers, we
  // cannot sink cross-barrier values.
  CallBase *CB = dyn_cast<CallBase>(User);
  if (CB && DPB->hasSyncInstruction(CB->getCalledFunction()))
    return false;

  DenseSet<Value *> Visited;
  for (Use *DepUse : make_range(po_begin(TheUse), po_end(TheUse))) {
    Instruction *DepInst = dyn_cast<Instruction>(DepUse->get());
    if (!DepInst || !WIRV->isWIRelated(DepInst))
      continue;

    // TODO: Use a cost model to choose whether to bail out of copying.
    // TODO: If some of the instructions the use depends on are already copied,
    // maybe we can loosen the threshold.
    // E.g., Given Threshold = 2 and the following code,
    //   id = get_gid();
    //   i = id + 1;
    //   j = i * 10;
    //   barrier();
    //   a[i] = ...;
    //   b[j] = ...;
    // when visiting `i` in `a[i]`, we copy `id` and `i`, and when visiting `j`
    // in `b[j]`, we choose to bail out of copying since the number of
    // instructions to copy is larger than the Threshold, but actually we only
    // need to copy 1 more instruction, i.e., `j = id * 10`.
    // TODO: In the case above, what if `b[j]` is visited before `a[i]`?
    if (Visited.insert(DepInst).second && Visited.size() > Threshold) {
      Uses.clear();
      return false;
    }

    if (!isSafeToCopy(DepInst, RTS)) {
      Uses.clear();
      return false;
    }

    Uses.push_back(DepUse);
  }

  if (Visited.empty())
    return false;

  return true;
}

static void copyAndReplaceUses(
    SmallVectorImpl<Use *> &Uses, BasicBlock *RegionHeader,
    Instruction *InsertPt, DenseMap<BasicBlock *, InstMap> &OriginToCopyMaps,
    InstMap &CopyToOriginMap, SmallPtrSetImpl<Instruction *> &RemoveCandidates,
    WIRelatedValue *WIRV, BarrierRegionInfo *BRI) {

  auto IsDomFrontier = [](BasicBlock *RegionHeader) {
    return !BarrierUtils::isBarrierOrDummyBarrierCall(&*RegionHeader->begin());
  };

  InstMap &OriginToCopyMap = OriginToCopyMaps[RegionHeader];

  // 1. Copy instructions
  DenseSet<Value *> PHIs; // A set keeping insts whose clones are phi nodes
  for (Use *U : Uses) {
    auto *Inst = cast<Instruction>(U->get());
    // If the clone of the user is a phi node, we don't need to clone this
    // instruction.
    if (PHIs.count(U->getUser()))
      continue;
    LLVM_DEBUG(dbgs() << "Replace the use of "; Inst->dump());

    // If the Inst is a cloned one, find its original value, and if it's not,
    // set itself as it's original value.
    auto *OrigInst = CopyToOriginMap.insert({Inst, Inst}).first->second;
    SmallVector<std::pair<BasicBlock *, Instruction *>> IncomingValues;

    auto HasInstOrInstCopy = [&, OrigInst](BasicBlock *BB) {
      BasicBlock *RegionHeader = BRI->getRegionHeaderFor(BB);
      LLVM_DEBUG(dbgs() << "  Searching " << OrigInst->getName() << " in "
                        << RegionHeader->getName() << '\n');
      if (OrigInst->getParent() == RegionHeader) {
        LLVM_DEBUG(dbgs() << "  Found " << OrigInst->getName() << '\n');
        IncomingValues.emplace_back(BB, OrigInst);
        return true;
      }
      auto &M = OriginToCopyMaps[RegionHeader];
      auto It = M.find(OrigInst);
      if (It == M.end()) {
        LLVM_DEBUG(dbgs() << "  Found nothing\n");
        return false;
      }
      LLVM_DEBUG(dbgs() << "  Found " << It->second->getName() << '\n');
      IncomingValues.emplace_back(BB, It->second);
      return true;
    };

    auto It = OriginToCopyMap.find(OrigInst);
    Instruction *Clone;
    if (It != OriginToCopyMap.end()) {
      LLVM_DEBUG(
          dbgs() << "  It's already copied, so use the existing copy.\n");
      continue;
    }
    if (IsDomFrontier(RegionHeader) &&
        llvm::all_of(predecessors(RegionHeader), HasInstOrInstCopy)) {
      LLVM_DEBUG(dbgs() << "  Create a phi.\n");
      auto *Phi = PHINode::Create(OrigInst->getType(), IncomingValues.size());
      for (auto &KV : IncomingValues)
        Phi->addIncoming(KV.second, KV.first);
      Phi->insertBefore(&*RegionHeader->begin());
      Clone = Phi;
      PHIs.insert(Inst);
    } else {
      LLVM_DEBUG(dbgs() << "  Create a new copy.\n");
      Clone = OrigInst->clone();
      Clone->insertBefore(InsertPt);
      Clone->setDebugLoc(InsertPt->getDebugLoc());
    }
    OriginToCopyMap[OrigInst] = Clone;
    CopyToOriginMap[Clone] = OrigInst;
    Clone->setName(OrigInst->getName() + ".copy");
    WIRV->setWIRelated(Clone, true);
    // Cloned instruction may have no use.
    RemoveCandidates.insert(Clone);
  }

  // Query from CopyToOriginMap and OriginToCopyMap to get the unique copy of
  // instruction.
  auto GetTheUniqueCopyOfInst = [&](Instruction *Inst) {
    assert(Inst && "Inst cannot be null!");
    // If the user is already a copied instruction, get the original inst.
    // Otherwise, the lookup will return the instruction itself.
    auto Iter1 = CopyToOriginMap.find(Inst);
    if (Iter1 == CopyToOriginMap.end()) {
      LLVM_DEBUG(Inst->dump());
      llvm_unreachable("Inst not found in the map!");
    }
    auto *OrigInst = Iter1->getSecond();
    assert(OrigInst && "Inst cannot be null!");
    // Then find the unique copy that we should use.
    auto Iter2 = OriginToCopyMap.find(OrigInst);
    if (Iter2 == OriginToCopyMap.end()) {
      LLVM_DEBUG(OrigInst->dump());
      llvm_unreachable("Inst not found in the map!");
    }
    auto *TheCopy = Iter2->getSecond();
    assert(TheCopy && "Inst cannot be null!");
    return TheCopy;
  };

  // 2. Replace the operand of the original user
  Use *U = Uses.back();
  Instruction *UserInst = cast<Instruction>(U->getUser());
  unsigned OpIdx = U->getOperandNo();
  Instruction *Inst = cast<Instruction>(U->get());
  UserInst->setOperand(OpIdx, GetTheUniqueCopyOfInst(Inst));
  RemoveCandidates.insert(Inst);

  // 3. Fix operands for all cloned instructions
  for (auto II = Uses.rbegin() + 1, EE = Uses.rend(); II != EE; ++II) {
    U = *II;
    UserInst = GetTheUniqueCopyOfInst(cast<Instruction>(U->getUser()));
    if (isa<PHINode>(UserInst))
      continue;
    OpIdx = U->getOperandNo();
    Inst = cast<Instruction>(U->get());
    UserInst->setOperand(OpIdx, GetTheUniqueCopyOfInst(Inst));
    RemoveCandidates.insert(Inst);
  }
}

static bool runOnFunction(Function &F, BuiltinLibInfo *BLI, DataPerValue *DPV,
                          WIRelatedValue *WIRV, DataPerBarrier *DPB,
                          DominanceFrontier *DF, DominatorTree *DT,
                          OptimizationRemarkEmitter *ORE) {
  const auto *CrossBarrierUseMap = DPV->getCrossBarrierUses(&F);
  if (!CrossBarrierUseMap || CrossBarrierUseMap->empty())
    return false;

  auto &RTS = BLI->getRuntimeService();
  BarrierRegionInfo BRI(&F, DF, DT);

  bool Changed = false;

  // A map holding maps between original defs and their clones per region
  DenseMap<BasicBlock * /* RegionHeader */,
           DenseMap<Instruction * /*Origin*/, Instruction * /*Clone*/>>
      CopiedInsts;

  // A map between copied instructions and their original instructions. This is
  // used to avoid redundant copies. E.g.,
  //   id = get_gid();
  //   barrier();
  //   a = id + 1;
  //   barrier();
  //   b = a * 10;
  //   c = id + 2;
  // `id` may be copied twice after the 2nd barrier w/o this map,
  //   id = get_gid();
  //   barrier();
  //   id.cp = get_gid();
  //   a = id.cp + 1;
  //   barrier();
  //   id.cp.cp = get_gid();    // Cloned from id.cp
  //   a.cp = id.cp.cp + 1;
  //   b = a.cp * 10;
  //   id.cp2 = get_gid();      // Cloned from id
  //   c = id.cp2 + 1;
  InstMap CopyToOriginMap;
  DenseMap<BasicBlock * /*RegionHeader*/, Instruction * /*InsertPoint*/>
      InsertPoints;
  std::vector<Use *> CrossBarrierUses;
  for (const auto &KV : *CrossBarrierUseMap) {
    auto &Uses = KV.second;
    CrossBarrierUses.insert(CrossBarrierUses.end(), Uses.begin(), Uses.end());
  }
  llvm::sort(CrossBarrierUses, [&BRI](Use *LHS, Use *RHS) -> bool {
    auto *LInst = cast<Instruction>(LHS->getUser()),
         *RInst = cast<Instruction>(RHS->getUser());
    auto *LBB = LInst->getParent(), *RBB = RInst->getParent();
    if (LBB == RBB)
      return LInst->comesBefore(RInst);
    return BRI.compare(LBB, RBB);
  });

  unsigned NumReduced = 0;
  SmallPtrSet<Instruction *, 16> RemoveCandidates;
  for (Use *TheUse : CrossBarrierUses) {
    auto *UserInst = cast<Instruction>(TheUse->getUser());
    LLVM_DEBUG(dbgs() << "Handle instruction"; UserInst->dump());

    // If the user is a phi node, we should copy these instructions to the
    // region where the control flow comes from.
    BasicBlock *RegionBB;
    if (PHINode *PHI = dyn_cast<PHINode>(UserInst))
      RegionBB = PHI->getIncomingBlock(*TheUse);
    else
      RegionBB = UserInst->getParent();
    BasicBlock *RegionHeader = BRI.getRegionHeaderFor(RegionBB);

    SmallVector<Use *, 16> Uses;
    if (!collectDependencies(TheUse, SYCLBarrierCopyInstructionThreshold, DPB,
                             WIRV, RTS, Uses))
      continue;

    Instruction *&InsertPt = InsertPoints[RegionHeader];
    if (LLVM_UNLIKELY(!InsertPt)) {
      InsertPt = RegionHeader->getFirstNonPHI();
      if (BarrierUtils::isBarrierOrDummyBarrierCall(InsertPt))
        InsertPt = InsertPt->getNextNode();
    }

    copyAndReplaceUses(Uses, RegionHeader, InsertPt, CopiedInsts,
                       CopyToOriginMap, RemoveCandidates, WIRV, &BRI);
    NumReduced++;
    Changed = true;
  }

  if (Changed) {
    bool Removed;
    do {
      Removed = false;
      for (Instruction *I : make_early_inc_range(RemoveCandidates)) {
        if (I->use_empty()) {
          I->eraseFromParent();
          RemoveCandidates.erase(I);
          Removed = true;
        }
      }
    } while (Removed);

    ORE->emit([&]() {
      return OptimizationRemark(DEBUG_TYPE, "ReduceCrossBarrierValues", &F)
             << Twine(NumReduced).str()
             << " cross-barrier uses are erased in function " << F.getName();
    });
  }

  return Changed;
}

PreservedAnalyses
ReduceCrossBarrierValuesPass::run(Module &M, ModuleAnalysisManager &MAM) {
  auto *BLI = &MAM.getResult<BuiltinLibInfoAnalysis>(M);
  auto *DPV = &MAM.getResult<DataPerValueAnalysis>(M);
  auto *WIRV = &MAM.getResult<WIRelatedValueAnalysis>(M);
  auto *DPB = &MAM.getResult<DataPerBarrierAnalysis>(M);

  auto &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  bool Changed = false;
  for (auto &F : M) {
    if (F.hasOptNone() || F.isDeclaration())
      continue;
    auto &DF = FAM.getResult<DominanceFrontierAnalysis>(F);
    auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);
    auto &ORE = FAM.getResult<OptimizationRemarkEmitterAnalysis>(F);
    Changed |= runOnFunction(F, BLI, DPV, WIRV, DPB, &DF, &DT, &ORE);
  }

  if (!Changed)
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<DataPerBarrierAnalysis>();
  PA.preserve<DominanceFrontierAnalysis>();
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<WIRelatedValueAnalysis>();
  return PA;
}

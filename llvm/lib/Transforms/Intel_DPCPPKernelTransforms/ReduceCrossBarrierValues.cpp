//==--------------- ReduceCrossBarrierValues.cpp ---------------- C++ -*---==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ReduceCrossBarrierValues.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DataPerBarrierPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DataPerValuePass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/RuntimeService.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/WIRelatedValuePass.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-reduce-cross-barrier-values"

static cl::opt<unsigned> DPCPPBarrierCopyInstructionThreshold(
    "dpcpp-barrier-copy-instruction-threshold", cl::init(15), cl::Hidden);

namespace {

/// Legacy ReduceCrossBarrierValues pass.
class ReduceCrossBarrierValuesLegacy : public ModulePass {
  ReduceCrossBarrierValuesPass Impl;

public:
  static char ID;

  ReduceCrossBarrierValuesLegacy() : ModulePass(ID) {
    initializeReduceCrossBarrierValuesLegacyPass(
        *PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "ReduceCrossBarrierValuesLegacy";
  }

  bool runOnModule(Module &M) override {
    auto *BLI = &getAnalysis<BuiltinLibInfoAnalysisLegacy>().getResult();
    auto *DPV = &getAnalysis<DataPerValueWrapper>().getDPV();
    auto *WIRV = &getAnalysis<WIRelatedValueWrapper>().getWRV();
    auto *DPB = &getAnalysis<DataPerBarrierWrapper>().getDPB();
    auto GetDF = [this](Function &F) -> DominanceFrontier & {
      return this->getAnalysis<DominanceFrontierWrapperPass>(F)
          .getDominanceFrontier();
    };
    auto GetDT = [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };
    return Impl.runImpl(M, BLI, DPV, WIRV, DPB, GetDF, GetDT);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
    AU.addRequired<DataPerBarrierWrapper>();
    AU.addRequired<DataPerValueWrapper>();
    AU.addRequired<DominanceFrontierWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<WIRelatedValueWrapper>();

    AU.setPreservesCFG();
    AU.addPreserved<DataPerBarrierWrapper>();
    AU.addPreserved<DominanceFrontierWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<WIRelatedValueWrapper>();
  }
};
} // namespace

char ReduceCrossBarrierValuesLegacy::ID = 0;
INITIALIZE_PASS_BEGIN(ReduceCrossBarrierValuesLegacy, DEBUG_TYPE,
                      "Reduce cross barrier values", false, false)
INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy);
INITIALIZE_PASS_DEPENDENCY(DataPerBarrierWrapper);
INITIALIZE_PASS_DEPENDENCY(DataPerValueWrapper);
INITIALIZE_PASS_DEPENDENCY(DominanceFrontierWrapperPass);
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass);
INITIALIZE_PASS_DEPENDENCY(WIRelatedValueWrapper);
INITIALIZE_PASS_END(ReduceCrossBarrierValuesLegacy, DEBUG_TYPE,
                    "Reduce cross barrier values", false, false)
ModulePass *llvm::createReduceCrossBarrierValuesLegacyPass() {
  return new ReduceCrossBarrierValuesLegacy();
}

PreservedAnalyses
ReduceCrossBarrierValuesPass::run(Module &M, ModuleAnalysisManager &MAM) {
  auto *BLI = &MAM.getResult<BuiltinLibInfoAnalysis>(M);
  auto *DPV = &MAM.getResult<DataPerValueAnalysis>(M);
  auto *WIRV = &MAM.getResult<WIRelatedValueAnalysis>(M);
  auto *DPB = &MAM.getResult<DataPerBarrierAnalysis>(M);

  auto &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetDF = [&FAM](Function &F) -> DominanceFrontier & {
    return FAM.getResult<DominanceFrontierAnalysis>(F);
  };
  auto GetDT = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };
  if (!runImpl(M, BLI, DPV, WIRV, DPB, GetDF, GetDT))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<DataPerBarrierAnalysis>();
  PA.preserve<DominanceFrontierAnalysis>();
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<WIRelatedValueAnalysis>();
  return PA;
}

static bool isBarrierOrDummyBarrierCall(Value *Val) {
  static std::string Barriers[] = {
      CompilationUtils::mangledBarrier(),
      CompilationUtils::mangledWGBarrier(
          CompilationUtils::BarrierType::NoScope),
      CompilationUtils::mangledWGBarrier(
          CompilationUtils::BarrierType::WithScope),
      DUMMY_BARRIER_FUNC_NAME,
  };
  CallInst *CI;
  Function *F;
  if (!(CI = dyn_cast<CallInst>(Val)) || !(F = CI->getCalledFunction()))
    return false;
  StringRef FName = F->getName();
  return any_of(Barriers, [&](std::string &B) { return FName == B; });
}

namespace {
class BarrierRegionInfo {
public:
  BarrierRegionInfo(Function *F, DominanceFrontier *DF, DominatorTree *DT) {
    analyze(F, DF, DT);
  }

  inline bool isRegionHeader(BasicBlock *BB) {
    return Regions.find(BB) != Regions.end();
  }

  inline BasicBlock *getRegionHeaderFor(BasicBlock *BB) {
    if (isRegionHeader(BB))
      return BB;
    assert(HeaderMap.find(BB) != HeaderMap.end() &&
           "BB doesn't belong to any region?");
    return HeaderMap[BB];
  }

  /// Returns true if the region header of \p LHS comes before the one of \p
  /// RHS in member variable \p Regions. See comments of \p Regions for
  /// details.
  bool compare(BasicBlock *LHS, BasicBlock *RHS) {
    auto LIt = Regions.find(getRegionHeaderFor(LHS)),
         RIt = Regions.find(getRegionHeaderFor(RHS));
    assert(LIt != Regions.end() && RIt != Regions.end() &&
           "Expected region headers");
    return LIt < RIt;
  }

private:
  void analyze(Function *F, DominanceFrontier *DF, DominatorTree *DT) {
    SetVector<BasicBlock *> Headers = collectRegionHeaders(F, DF);
    constructRegions(F, Headers, DT);
  }

  void constructRegions(Function *F, SetVector<BasicBlock *> &Headers,
                        DominatorTree *DT) {
    // Create regions in order.
    for (auto *Header : Headers)
      Regions[Header];

    for (auto &BB : *F) {
      auto *HeaderNode = DT->getNode(&BB);
      assert(HeaderNode != nullptr && "Invalid header node!");
      BasicBlock *Header = HeaderNode->getBlock();
      while (!Headers.contains(Header)) {
        HeaderNode = HeaderNode->getIDom();
        Header = HeaderNode->getBlock();
      }

      if (Header != &BB) {
        auto &BlockSet = Regions[Header];
        BlockSet.insert(&BB);
        HeaderMap[&BB] = Header;
      }
    }
  }

  /// Returns all region headers in order. See comments of member variable
  /// \p Regions for details.
  static SetVector<BasicBlock *> collectRegionHeaders(Function *F,
                                                      DominanceFrontier *DF) {
    std::list<BasicBlock *> Headers;
    Headers.push_back(&F->getEntryBlock());

    DenseSet<BasicBlock *> Visited;
    for (BasicBlock &BB : *F) {
      if (isBarrierOrDummyBarrierCall(&*BB.begin())) {
        Headers.push_back(&BB);
        Visited.insert(&BB);
      }
    }

    auto FrontierEnd = --Headers.end();
    SmallVector<BasicBlock *, 16> WorkList(Headers.begin(), Headers.end());
    do {
      auto *BB = WorkList.pop_back_val();
      for (BasicBlock *Frontier : DF->find(BB)->second) {
        if (Visited.insert(Frontier).second) {
          Headers.push_back(Frontier);
          WorkList.push_back(Frontier);
        }
      }
    } while (!WorkList.empty());

    // Remove duplicate dominance frontiers. We don't prevent inserting
    // duplicated elements above because we need to preserve the topological
    // order (see comments of the member Regions), so only the last occurrence
    // should be kept.
    Visited.clear();
    auto It = --Headers.end();
    while (It != FrontierEnd) {
      auto Next = --It;
      if (!Visited.insert(*It).second)
        Headers.erase(It);
      It = Next;
    }
    return {Headers.begin(), Headers.end()};
  }

private:
  /// A map between region headers and basic blocks belonging to them.
  /// Region headers are sorted in the following order:
  ///   Entry, Sync BBs ..., Dominance frontiers ...
  /// Dominance frontiers are also in topological order (ignoring loop
  /// backedges).
  MapVector<BasicBlock *, DenseSet<BasicBlock *>> Regions;

  /// A map between basic blocks and the region headers they belong to
  DenseMap<BasicBlock *, BasicBlock *> HeaderMap;
};
} // namespace

using UseSet = DataPerValue::UseSet;
using InstMap = DenseMap<Instruction *, Instruction *>;

static bool isSafeToCopy(Instruction *Inst, RuntimeService &RTS) {
  // Alloca can't be copied.
  if (isa<AllocaInst>(Inst))
    return false;

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

  if (auto *Call = dyn_cast<CallBase>(Inst)) {
    Function *CalledFunc = Call->getCalledFunction();
    if (CalledFunc && RTS.isSafeToSpeculativeExecute(CalledFunc->getName()))
      return true;

    // For other function calls, cloning them can be more expensive than save
    // to/load from special buffer. So we choose to bail out.
    return false;
  }

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
    InstMap &CopyToOriginMap, WIRelatedValue *WIRV, BarrierRegionInfo *BRI) {

  auto IsDomFrontier = [](BasicBlock *RegionHeader) {
    return !isBarrierOrDummyBarrierCall(&*RegionHeader->begin());
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
    }
    OriginToCopyMap[OrigInst] = Clone;
    CopyToOriginMap[Clone] = OrigInst;
    Clone->setName(OrigInst->getName() + ".copy");
    WIRV->setWIRelated(Clone, true);
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
  // TODO: Erasing the dead instruction here can lead to invalid values in
  // CopyToOriginMap and assertion failures. So I just leave dead instructions
  // to DCE pass until there's a better solution.
  // if (Inst->use_empty())
  //   Inst->eraseFromParent();

  // 3. Fix operands for all cloned instructions
  for (auto II = Uses.rbegin() + 1, EE = Uses.rend(); II != EE; ++II) {
    U = *II;
    UserInst = GetTheUniqueCopyOfInst(cast<Instruction>(U->getUser()));
    if (isa<PHINode>(UserInst))
      continue;
    OpIdx = U->getOperandNo();
    Inst = cast<Instruction>(U->get());
    UserInst->setOperand(OpIdx, GetTheUniqueCopyOfInst(Inst));
    // if (Inst->use_empty())
    //   Inst->eraseFromParent();
  }
}

static bool runOnFunction(Function &F, BuiltinLibInfo *BLI, DataPerValue *DPV,
                          WIRelatedValue *WIRV, DataPerBarrier *DPB,
                          DominanceFrontier *DF, DominatorTree *DT) {
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
  for (auto KV : *CrossBarrierUseMap) {
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
    if (!collectDependencies(TheUse, DPCPPBarrierCopyInstructionThreshold, DPB,
                             WIRV, RTS, Uses))
      continue;

    Instruction *&InsertPt = InsertPoints[RegionHeader];
    if (LLVM_UNLIKELY(!InsertPt)) {
      InsertPt = RegionHeader->getFirstNonPHI();
      if (isBarrierOrDummyBarrierCall(InsertPt))
        InsertPt = InsertPt->getNextNode();
    }

    copyAndReplaceUses(Uses, RegionHeader, InsertPt, CopiedInsts,
                       CopyToOriginMap, WIRV, &BRI);
    Changed = true;
  }

  return Changed;
}

bool ReduceCrossBarrierValuesPass::runImpl(
    Module &M, BuiltinLibInfo *BLI, DataPerValue *DPV, WIRelatedValue *WIRV,
    DataPerBarrier *DPB, function_ref<DominanceFrontier &(Function &)> GetDF,
    function_ref<DominatorTree &(Function &)> GetDT) {
  bool Changed = false;
  for (auto &F : M) {
    if (F.hasOptNone() || F.isDeclaration())
      continue;
    Changed |= runOnFunction(F, BLI, DPV, WIRV, DPB, &GetDF(F), &GetDT(F));
  }
  return Changed;
}

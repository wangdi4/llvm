//=---- ReduceCrossBarrierValues.cpp ---------------------------*- C++ -*----=//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This pass reduce cross-barrier values by copying their definitions to where
/// they're used. E.g., given pseudo code as follows,
///
///   id = get_global_id(0);
///   i = id * 10;
///   ... a[i] ...
///   ....
///   barrier()
///   ... b[i] ...
///
/// we copy `id` and `i` below the barrier, and replace the `i` in `b[i]` to
/// the copied one,
///
///   id = get_global_id(0);
///   i = id * 10;
///   ... a[i] ...
///   ....
///   barrier()
///   id_copy = get_global_id(0);
///   i_copy = id_copy * 10;
///   ... b[i_copy] ...
///
/// The algorithm is described as follows.
///
/// Assumption
/// ==========
/// All barriers must be at the beginning of basic blocks. This is already done
/// by SplitBBonBarrier pass.
///
/// Terms
/// =====
/// - Sync basic block
///   A sync basic block (Sync BB) is a basic block who contains a barrier or a
///   dummy barrier. And the barrier is the first instruction of the basic
///   block according to the assumption above.
///
/// - Barrier region header
///   A basic block is a barrier region header iff it's
///     1) the entry block of a function, or
///     2) a sync BB, or
///     3) a dominance frontier of another region header.
///
/// - Barrier region
///   A barrier region starts with a region header, and contains a set of basic
///   blocks. A basic block (not a header) belongs to a barrier region iff its
///   idom
///     1) is the region header, or
///     2) belongs to the region.
///
/// Example
/// =======
/// Given a CFG as follows,
///
///       A
///   ,.-'|
///  |   *B
///  |   / \
///  |  C   D-.
///  |  |   |  | (backedge from F to D)
///  | *E  *F-'
///  |   \ /
///  |    G
///   `.  |
///     '-H
///       |
///      *I
///   (Blocks with * are sync BBs, i.e.,
///    block B, E, F and I start with barrier.)
///
/// barrier region headers are
///  - A (entry block)
///  - B, E, F, I (sync BB)
///  - D (dominance frontier of F)
///  - G (dominance frontier of A)
///  - H (dominance frontier of G)
///
/// so there are 8 barrier regions in total:
///  {A}, {B C}, {D}, {E}, {F}, {G}, {H} and {I}
///
/// Notes
/// =====
/// The reason to construct barrier region is to share a same copy of a def
/// among its uses within a region, and thus avoid duplicate copying.
///
/// Algorithm
/// =========
/// Collect all cross-barrier def-use's;
/// Sort all uses in topological order;
/// Found all barrier regions using DFS;
/// Init an empty map M to hold maps between original defs and their copies per
/// region;
/// For each cross-barrier use U:
///   Determine the region R it belongs to;
///   Iteratively copy U and its WI-related operands into the region (When
///   copying, if the region header is a dominance frontier, and all of its
///   predeceasing regions have already contained the instruction to copy or
///   its copies, just create a Phi node in R and stop copying instructions it
///   depends on instead of copying the instruction).
///
/// Author: Senran Zhang
///
// ===--------------------------------------------------------------------=== //

#include "ReduceCrossBarrierValues.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "OCLPassSupport.h"
#include "OpenclRuntime.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/KernelBarrierUtils.h"

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "B-ReduceCrossBarrierValues"

static cl::opt<unsigned> Threshold("barrier-copy-instruction-threshold",
                                   cl::init(15), cl::Hidden);

namespace {
struct OpUseIterator {
  OpUseIterator(Use *U) : U(U) {}
  OpUseIterator operator++(int) {
    OpUseIterator Ret(U);
    unsigned NextOpIdx = U->getOperandNo() + 1;
    User *TheUser = U->getUser();
    U = TheUser->getOperandList() + NextOpIdx;
    return Ret;
  }
  inline Use *operator*() { return U; }
  inline bool operator==(const OpUseIterator &RHS) const { return U == RHS.U; }
  inline bool operator!=(const OpUseIterator &RHS) const { return U != RHS.U; }
private:
  Use *U;
};
} // anonymous namespace

namespace llvm {
template <> struct GraphTraits<Use *> {
  using NodeRef = Use *;
  using ChildIteratorType = OpUseIterator;

  static inline NodeRef getEntryNode(NodeRef N) { return N; }

  static inline ChildIteratorType child_begin(NodeRef N) {
    if (auto *Inst = dyn_cast<Instruction>(N->get()))
      return Inst->op_begin();
    return nullptr;
  }

  static inline ChildIteratorType child_end(NodeRef N) {
    if (auto *Inst = dyn_cast<Instruction>(N->get()))
      return Inst->op_end();
    return nullptr;
  }
};
} // namespace llvm

using namespace Intel::OpenCL::DeviceBackend;

static bool isBarrierOrDummyBarrierCall(Value *Val) {
  static std::string Barriers[] = {
    CompilationUtils::mangledBarrier(),
    CompilationUtils::mangledWGBarrier(CompilationUtils::BARRIER_NO_SCOPE),
    CompilationUtils::mangledWGBarrier(CompilationUtils::BARRIER_WITH_SCOPE),
    DUMMY_BARRIER_FUNC_NAME,
  };
  CallInst *CI;
  Function *F;
  if (!(CI = dyn_cast<CallInst>(Val)) || !(F = CI->getCalledFunction()))
    return false;
  StringRef FName = F->getName();
  return any_of(Barriers, [&](std::string &B){ return FName == B; });
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
  static SetVector<BasicBlock *> collectRegionHeaders(
      Function *F, DominanceFrontier *DF) {
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
} // anonymous namespace


namespace intel {

OCL_INITIALIZE_PASS_BEGIN(
    ReduceCrossBarrierValues, "B-ReduceCrossBarrierValues",
    "Barrier Pass - Reduce Cross Barrier Values", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY_INTEL(BuiltinLibInfo);
OCL_INITIALIZE_PASS_DEPENDENCY(DataPerBarrierWrapper);
OCL_INITIALIZE_PASS_DEPENDENCY(DataPerValueWrapper);
OCL_INITIALIZE_PASS_DEPENDENCY(WIRelatedValueWrapper);
OCL_INITIALIZE_PASS_DEPENDENCY(DominanceFrontierWrapperPass);
OCL_INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass);
OCL_INITIALIZE_PASS_END(
    ReduceCrossBarrierValues, "B-ReduceCrossBarrierValues",
    "Barrier Pass - Reduce Cross Barrier Values", false, false)

ReduceCrossBarrierValues::ReduceCrossBarrierValues() : FunctionPass(ID) {
  initializeReduceCrossBarrierValuesPass(*PassRegistry::getPassRegistry());
}

using UseSet = DataPerValue::UseSet;
using InstMap = DenseMap<Instruction *, Instruction *>;

static bool isSafeToCopy(Instruction *Inst, OpenclRuntime *RuntimeServices) {
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
    if (CalledFunc &&
        RuntimeServices->isSafeToSpeculativeExecute(CalledFunc->getName().str()))
      return true;

    // For other function calls, cloning them can be more expensive than save
    // to/load from special buffer. So we choose to bail out.
    return false;
  }

  return true;
}

/// Returns true if it's legal to clone all dependent instructions used by the
/// user of \p TheUse. All dependencies are saved into \p Uses.
static bool collectDependencies(
    Use *TheUse, size_t Threshold, DataPerBarrier *DPB, WIRelatedValue *WIRV,
    OpenclRuntime *RuntimeServices, SmallVectorImpl<Use *> &Uses) {
  auto *User = TheUse->getUser();

  // If the user is a call instruction whose callee contains barriers, we
  // cannot sink cross-barrier values.
  CallBase *CB = dyn_cast<CallBase>(User);
  if (CB && DPB->hasSyncInstruction(CB->getCalledFunction()))
    return false;

  DenseSet<Value *> Visited;
  for (Use *DepUse :
          make_range(po_begin(TheUse), po_end(TheUse))) {
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

    if (!isSafeToCopy(DepInst, RuntimeServices)) {
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
    Instruction *InsertPt,
    DenseMap<BasicBlock *, InstMap> &OriginToCopyMaps,
    InstMap &CopyToOriginMap, WIRelatedValue *WIRV,
    BarrierRegionInfo *BRI) {

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
    LLVM_DEBUG(dbgs() << "Replace the use of ";
               Inst->dump());

    // If the Inst is a cloned one, find its original value, and if it's not,
    // set itself as it's original value.
    auto *OrigInst = CopyToOriginMap.insert({Inst, Inst}).first->second;
    SmallVector<std::pair<BasicBlock *, Instruction *>> IncomingValues;

    auto hasInstOrInstCopy = [&, OrigInst](BasicBlock *BB) {
      BasicBlock *RegionHeader = BRI->getRegionHeaderFor(BB);
      LLVM_DEBUG(dbgs() << "  Searching " << OrigInst->getName()
                        << " in " << RegionHeader->getName() << '\n');
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
      LLVM_DEBUG(dbgs() << "  It's already copied, so use the existing copy.\n");
      continue;
    } else if (IsDomFrontier(RegionHeader) &&
               llvm::all_of(predecessors(RegionHeader), hasInstOrInstCopy)) {
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

  // 2. Replace the operand of the original user
  Use *U = Uses.back();
  Instruction *UserInst = cast<Instruction>(U->getUser());
  unsigned OpIdx = U->getOperandNo();
  Instruction *Inst = cast<Instruction>(U->get());
  UserInst->setOperand(OpIdx, OriginToCopyMap[CopyToOriginMap[Inst]]);
  // TODO: Erasing the dead instruction here can lead to invalid values in
  // CopyToOriginMap and assertion failures. So I just leave dead instructions
  // to DCE pass until there's a better solution.
  // if (Inst->use_empty())
  //   Inst->eraseFromParent();

  // 3. Fix operands for all cloned instructions
  for (auto II = Uses.rbegin() + 1, EE = Uses.rend(); II != EE; ++II) {
    U = *II;
    UserInst =
        OriginToCopyMap[CopyToOriginMap[cast<Instruction>(U->getUser())]];
    if (isa<PHINode>(UserInst))
      continue;
    OpIdx = U->getOperandNo();
    Inst = cast<Instruction>(U->get());
    UserInst->setOperand(OpIdx, OriginToCopyMap[CopyToOriginMap[Inst]]);
    // if (Inst->use_empty())
    //   Inst->eraseFromParent();
  }
}

bool ReduceCrossBarrierValues::runOnFunction(Function &F) {
  if (F.hasOptNone())
    return false;
  auto *DPV = &getAnalysis<DataPerValueWrapper>().getDPV();
  const auto *CrossBarrierUseMap = DPV->getCrossBarrierUses(&F);
  if (!CrossBarrierUseMap || CrossBarrierUseMap->empty())
    return false;

  auto *WIRV = &getAnalysis<WIRelatedValueWrapper>().getWRV();
  auto *DPB = &getAnalysis<DataPerBarrierWrapper>().getDPB();
  auto *DF = &getAnalysis<DominanceFrontierWrapperPass>().getDominanceFrontier();
  auto *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto *RuntimeServices = static_cast<OpenclRuntime *>(
      getAnalysis<BuiltinLibInfo>().getRuntimeServices());
  BarrierRegionInfo BRI(&F, DF, DT);

  bool Changed = false;

  // A map holding maps between original defs and their clones per region
  DenseMap<BasicBlock * /* RegionHeader */,
    DenseMap<Instruction * /*Origin*/, Instruction* /*Clone*/>> CopiedInsts;

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
  DenseMap<BasicBlock * /*RegionHeader*/,
    Instruction * /*InsertPoint*/> InsertPoints;
  std::vector<Use *> CrossBarrierUses;
  for (auto KV : *CrossBarrierUseMap) {
    auto &Uses = KV.second;
    CrossBarrierUses.insert(CrossBarrierUses.end(), Uses.begin(), Uses.end());
  }
  llvm::sort(CrossBarrierUses,
             [&BRI](Use *LHS, Use *RHS) -> bool {
               auto *LInst = cast<Instruction>(LHS->getUser()),
                    *RInst = cast<Instruction>(RHS->getUser());
               auto *LBB = LInst->getParent(),
                    *RBB = RInst->getParent();
               if (LBB == RBB)
                 return LInst->comesBefore(RInst);
               return BRI.compare(LBB, RBB);
             });

  for (Use *TheUse : CrossBarrierUses) {
    auto *UserInst = cast<Instruction>(TheUse->getUser());
    LLVM_DEBUG(dbgs() << "Handle instruction";
               UserInst->dump());

    // If the user is a phi node, we should copy these instructions to the
    // region where the control flow comes from.
    BasicBlock *RegionBB;
    if (PHINode *PHI = dyn_cast<PHINode>(UserInst))
      RegionBB = PHI->getIncomingBlock(*TheUse);
    else
      RegionBB = UserInst->getParent();
    BasicBlock *RegionHeader = BRI.getRegionHeaderFor(RegionBB);

    SmallVector<Use *, 16> Uses;
    if (!collectDependencies(TheUse, Threshold, DPB, WIRV,
                             RuntimeServices, Uses))
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

char ReduceCrossBarrierValues::ID = 0;
} // namespace intel

extern "C" FunctionPass *createReduceCrossBarrierValuesPass() {
  return new intel::ReduceCrossBarrierValues();
}

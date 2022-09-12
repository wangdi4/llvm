#if INTEL_COLLAB
//===----- VPRenameOperands.cpp - Rename Operands Utility for OpenMP ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements VPO Rename Operands utility that replaces OpenMP clause
/// operands with a store-then-load. This is used to guard memory motion across
/// work regions. It also implements a standalone pass that can be used to
/// rename clause operands in WRNGuardMemMotionNodes.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Utils/VPORenameOperands.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-rename-operands"

static cl::opt<bool> DisablePass("disable-" DEBUG_TYPE, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable VPO Rename Operands pass"));

/// Create a pointer, store address of \p V to the pointer, and replace uses
/// of \p V with a load from that pointer.
///
/// \code
///   %v = alloca i32
///   ...
///   %v.addr = alloca i32*
///   ...
///   store i32* %v, i32** %v.addr
///   ; <InsertPtForStore>
///
///   +- <EntryBB>:
///   | ...
///   | %0 = llvm.region.entry() [... "PRIVATE" (i32* %v) ]
///   | ...
///   | %v1 = load i32*, i32** %v.addr
///   +-
///   ...
///   ; Replace uses of %v with %v1
///   ...
/// \endcode
///
/// If \p ReplaceUses is \b true (default), then original uses or %v are
/// replaced with %v1.
///
/// If \p EmitLoadEvenIfNoUses is \b true, then %v1 is emitted even if there
/// is no use of original %v in the region, otherwise not (default).
///
/// If \p InsertLoadInBeginningOfEntryBB is \b true, the load `%v1` is
/// inserted in the beginning on EntryBB (BBlock containing `%0`), and the
/// use of `%v` in `%0` is also replaced with `%v1`. Otherwise, by default,
/// `v1` is inserted at the end of EntryBB.
///
/// If \p SelectAllocaInsertPtBasedOnParentWRegion is \b true, then the
/// insertion point for `%v.addr` is determined based on whether a parent
/// WRegion would eventually be outlined, otherwise, the end of the Function's
/// entry block is used.
///
/// If \p CastToAddrSpaceGeneric is \b true, then `%v.addr` is casted to
/// address space generic (4) before doing the store/load.
///
/// \returns the pointer where \p V is stored (`%v.addr` above).
Value *VPOUtils::replaceWithStoreThenLoad(
    WRegionNode *W, Value *V, Instruction *InsertPtForStore, bool ReplaceUses,
    bool EmitLoadEvenIfNoUses, bool InsertLoadInBeginningOfEntryBB,
    bool SelectAllocaInsertPtBasedOnParentWRegion,
    bool CastToAddrSpaceGeneric) {
  Function *F = InsertPtForStore->getParent()->getParent();
  assert(F && "Function cannot be nullptr.");

  // Find instructions in W that use V
  SmallVector<Instruction *, 8> UserInsts;
  SmallPtrSet<ConstantExpr *, 8> UserExprs;
  if (ReplaceUses)
    WRegionUtils::findUsersInRegion(
        W, V, &UserInsts, !InsertLoadInBeginningOfEntryBB, &UserExprs);

  Instruction *AllocaInsertPt =
      SelectAllocaInsertPtBasedOnParentWRegion
          ? VPOParoptUtils::getInsertionPtForAllocas(W, F, true)
          : W->getEntryBBlock()->getParent()->getEntryBlock().getTerminator();

  IRBuilder<> AllocaBuilder(AllocaInsertPt);
  Value *VAddr = //                                       (1)
      AllocaBuilder.CreateAlloca(V->getType(), nullptr, V->getName() + ".addr");

  IRBuilder<> StoreBuilder(InsertPtForStore);
  if (CastToAddrSpaceGeneric) {
    Value *VAddrCasted = StoreBuilder.CreatePointerBitCastOrAddrSpaceCast(
        VAddr, V->getType()->getPointerTo(vpo::ADDRESS_SPACE_GENERIC),
        VAddr->getName() + ".ascast");
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Casted '";
               VAddr->printAsOperand(dbgs()); dbgs() << "' to '";
               VAddrCasted->printAsOperand(dbgs()); dbgs() << "'.\n";);
    VAddr = VAddrCasted;
  }

  StoreBuilder.CreateStore(V, VAddr); //                  (2)

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Stored '"; V->printAsOperand(dbgs());
             dbgs() << "' to '"; VAddr->printAsOperand(dbgs());
             dbgs() << "'.\n";);

  if (UserInsts.empty() && !EmitLoadEvenIfNoUses)
    return VAddr; // Nothing to replace inside the region. Just capture the
                  // address of V to VAddr and return it.

  BasicBlock *EntryBB = W->getEntryBBlock();
  Instruction *InsertPtForLoad = InsertLoadInBeginningOfEntryBB
                                     ? EntryBB->getFirstNonPHI()
                                     : EntryBB->getTerminator();
  IRBuilder<> BuilderInner(InsertPtForLoad);
  LoadInst *VRenamed = BuilderInner.CreateLoad(V->getType(), VAddr); // (3)
  if (!InsertLoadInBeginningOfEntryBB)
    // InstCombine may transform:
    //   %1 = load float*, float** %.addr
    //   store float* %1, float** %X
    // into:
    //   %1 = bitcast float** %.addr to i64*
    //   %2 = load i64, i64* %1
    //   %3 = bitcast float** %X to i64*
    //   store i64 %2, i64* %3
    //
    // In this case VRenamed will be the %2 load of type i64,
    // VOrig will have type float*, so we will not be able
    // to restore the operand with just BitCasting float*
    // value to i64. We could have used IntToPtr, but
    // this will never be optimized. So we mark the load
    // as volatile to prevent InstCombine transformation
    // for this load.
    VRenamed->setVolatile(true);

  VRenamed->setName(V->getName());

  // Replace uses of V with VRenamed
  while (!UserInsts.empty()) {
    Instruction *User = UserInsts.pop_back_val();
    User->replaceUsesOfWith(V, VRenamed);

    if (UserExprs.empty())
      continue;

    // Some uses of V are in a ConstantExpr, in which case the User is the
    // instruction using the ConstantExpr. For example, the use of @u below is
    // the GEP expression (a ConstantExpr), not the instruction itself, so
    // doing User->replaceUsesOfWith(V, NewI) does not replace @u
    //
    //     %12 = load i32, i32* getelementptr inbounds (%struct.t_union_,
    //           %struct.t_union_* @u, i32 0, i32 0), align 4
    //
    // The solution is to access the ConstantExpr as instruction(s) in order to
    // do the replacement. NewInstArr below keeps such instruction(s).
    SmallVector<Instruction *, 2> NewInstArr;
    GeneralUtils::breakExpressions(User, &NewInstArr, &UserExprs);
    for (Instruction *NewInst : NewInstArr)
      UserInsts.push_back(NewInst);
  }
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Loaded '";
             VAddr->printAsOperand(dbgs()); dbgs() << "' into '";
             VRenamed->printAsOperand(dbgs()); dbgs() << "'.\n";);
  return VAddr;
}

/// Rename operands of various clauses by replacing them with a
/// store-then-load, and adding operand-address pair to the entry directive.
/// This renaming is done to prevent CSE/Instcombine transformations which
/// break OpenMP semantics by combining/recomputing bitcasts/GEPs across
/// region boundaries.
/// This renaming is done for operands to private, firstprivate, lastprivate,
/// reduction, shared, and map clauses.
///
/// This renaming is done in the vpo-paropt-prepare phase, and is undone
/// in the vpo-paropt-restore phase beofore the vpo-paropt transformation
/// pass.
///
/// The IR before and after this renaming looks like:
///
/// \code
///            Before                   |            After
///          ---------------------------+---------------------------
///                                     |   store i32* %y, i32** %y.addr
///                                     |
///   %1 = begin_region[... %y...]      |   %1 = begin_region[... %y...
///                                     |        "QUAL.OMP.OPERAND.ADDR"
///                                     |         (i32* %y,i32** %y.addr)]
///                                     |
///                                     |   %y1 = load i32*, i32** %y.addr
///                                     |
///   ...                               |   ...
///   <%y used inside the region>       |   <%y1 used inside the region>
///                                     |
///                                     |
///   end_region(%1)                    |   end_region(%1)
/// \endcode
///
/// In the region, `%y1` is used to replace all uses of `$y`. If there is no
/// use of `%y` inside the region, then the load `%y` is not emitted.
/// The operand-addr pair in the auxiliary clause `QUAL.OMP.OPERAND.ADDR` is
/// used to undo the renaming in the VPORestoreOperandsPass.
/// \see VPOUtils::restoreOperands() for details on how the renaming is
/// undone and the original operands are restored.
bool VPOUtils::renameOperandsUsingStoreThenLoad(WRegionNode *W,
                                                DominatorTree *DT,
                                                LoopInfo *LI) {
  bool Changed = false;
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *NewEntryBB =
      SplitBlock(EntryBB, EntryBB->getFirstNonPHI(), DT, LI);
  Instruction *InsertBefore = EntryBB->getTerminator();

  W->setEntryBBlock(NewEntryBB);
  W->populateBBSet(true); // rebuild BBSet unconditionlly as EntryBB changed

  SmallPtrSet<Value *, 16> HandledVals;
  using OpndAddrPairTy = SmallVector<Value *, 2>;
  SmallVector<OpndAddrPairTy, 16> OpndAddrPairs;
  auto rename = [&](Value *Orig, bool CheckAlreadyHandled,
                    bool ReplaceUses = true) {
    if (CheckAlreadyHandled && HandledVals.find(Orig) != HandledVals.end())
      return false;

    HandledVals.insert(Orig);
    Value *RenamedAddr =
        replaceWithStoreThenLoad(W, Orig, InsertBefore, ReplaceUses);
    if (!RenamedAddr)
      return false;

    OpndAddrPairs.push_back({Orig, RenamedAddr});
    return true;
  };

  if (W->canHavePrivate()) {
    PrivateClause &PrivClause = W->getPriv();
    for (PrivateItem *PrivI : PrivClause.items())
      Changed |= rename(PrivI->getOrig(), false);
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items())
      Changed |= rename(FprivI->getOrig(), false);
  }

  if (W->canHaveShared()) {
    SharedClause &ShaClause = W->getShared();
    for (SharedItem *ShaI : ShaClause.items())
      Changed |= rename(ShaI->getOrig(), false);
  }

  if (W->canHaveReduction()) {
    ReductionClause &RedClause = W->getRed();
    for (ReductionItem *RedI : RedClause.items())
      Changed |= rename(RedI->getOrig(), false);
  }

  if (W->canHaveLivein()) {
    LiveinClause &LvClause = W->getLivein();
    for (LiveinItem *LvI : LvClause.items())
      Changed |= rename(LvI->getOrig(), false);
  }

  if (W->canHaveLastprivate()) {
    LastprivateClause &LprivClause = W->getLpriv();
    for (LastprivateItem *LprivI : LprivClause.items())
      Changed |= rename(LprivI->getOrig(), true);
  }

  if (W->canHaveLinear()) {
    LinearClause &LrClause = W->getLinear();
    for (LinearItem *LrI : LrClause.items())
      Changed |= rename(LrI->getOrig(), true);
  }

  if (W->canHaveMap()) {
    // We need to make sure that we don't introduce new live-ins when renaming
    // base/section ptrs, so we only do the replacement of base/section-ptrs in
    // the region if they are the same as the base of the map-chain.
    // ------------------------------------+------------------------------------
    //   Before                            |  After
    // ------------------------------------+------------------------------------
    //                                     | %x.addr = ...
    //                                     | %x.gep.addr = ...
    //                                     |
    //  MAP(i32* @x, i32* gep(@x, 0), ...) | MAP(i32* @x, i32* gep(@x, 0), ...
    //                                     |  OPND.ADDR(@x, %x.addr)
    //                                     |  OPND.ADDR(gep(@x, 0), %x.gep.addr)
    //                                     |
    //                                     | %x.renamed = load i32*, %x.addr
    //                                     |
    //  ... gep(@x, 0)                     | ... gep(%x.renamed, 0)
    MapClause const &MpClause = W->getMap();
    for (MapItem *MapI : MpClause.items()) {
      Value *MapIOrig = MapI->getOrig();
      if (MapI->getIsMapChain()) {
        MapChainTy const &MapChain = MapI->getMapChain();
        for (unsigned I = 0; I < MapChain.size(); ++I) {
          MapAggrTy *Aggr = MapChain[I];
          Value *SectionPtr = Aggr->getSectionPtr();
          Value *BasePtr = Aggr->getBasePtr();
          Changed |= rename(SectionPtr, true, SectionPtr == MapIOrig);
          Changed |= rename(BasePtr, true, BasePtr == MapIOrig);
        }
      }
      Changed |= rename(MapIOrig, true);
    }
  }

  if (W->canHaveUseDevicePtr()) {
    UseDevicePtrClause &UdpClause = W->getUseDevicePtr();
    for (UseDevicePtrItem *UdpI : UdpClause.items())
      Changed |= rename(UdpI->getOrig(), true);
  }

  // is_device_ptr() clauses must have been transformed into
  // map[+private], but W->getIsDevicePtr().items().empty()
  // is still not empty. Just do nothing for these items.

  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed

  if (!Changed)
    return false;

  assert(!OpndAddrPairs.empty() && "Something changed without any renaming.");
  CallInst *CI = cast<CallInst>(W->getEntryDirective());
  SmallVector<std::pair<StringRef, ArrayRef<Value *>>, 8> BundleOpndAddrs;
  StringRef OperandAddrClauseString =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_OPERAND_ADDR);
  for (auto &OpndAddrPair : OpndAddrPairs)
    BundleOpndAddrs.emplace_back(OperandAddrClauseString,
                                 makeArrayRef(OpndAddrPair));

  CI = VPOUtils::addOperandBundlesInCall(CI, BundleOpndAddrs);
  W->setEntryDirective(CI);

  return true;
}

namespace {
// Use with the WRNVisitor class (in WRegionUtils.h) to walk the WRGraph
// (DFS) to gather all WRegion Nodes;
class VPOWRegionVisitor {
public:
  WRegionListTy &WRNList;

  VPOWRegionVisitor(WRegionListTy &WL) : WRNList(WL) {}
  void preVisit(WRegionNode *W) {}
  // Use DFS visiting of WRegionNodes.
  void postVisit(WRegionNode *W) { WRNList.push_back(W); }
  bool quitVisit(WRegionNode *W) { return false; }
};

static bool renameOperands(Function &F, WRegionInfo &WI, DominatorTree *DT,
                           LoopInfo *LI) {
  bool Changed = false;

  // Pass is disabled, nothing to do.
  if (DisablePass)
    return Changed;

  WI.buildWRGraph();
  // No WRNs to process.
  if (WI.WRGraphIsEmpty())
    return Changed;

  WRegionListTy WRegionList;
  VPOWRegionVisitor Visitor(WRegionList);
  WRegionUtils::forwardVisit(Visitor, WI.getWRGraph());

  for (auto *W : WRegionList) {
    // TODO: Generalize this pass to run for all types of WRNs?
    auto *GuardMemMotionNode = dyn_cast<WRNGuardMemMotionNode>(W);
    if (!GuardMemMotionNode)
      continue;

    Changed |=
        VPOUtils::renameOperandsUsingStoreThenLoad(GuardMemMotionNode, DT, LI);
  }

  return Changed;
}
} // end of anonymous namespace

INITIALIZE_PASS_BEGIN(VPORenameOperands, "vpo-rename-operands",
                      "VPO Rename Operands Function Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(VPORenameOperands, "vpo-rename-operands",
                    "VPO Rename Operands Function Pass", false, false)

char VPORenameOperands::ID = 0;

VPORenameOperands::VPORenameOperands() : FunctionPass(ID) {
  initializeVPORenameOperandsPass(*PassRegistry::getPassRegistry());
}

bool VPORenameOperands::runOnFunction(Function &F) {
  if (VPOAnalysisUtils::skipFunctionForOpenmp(F) && skipFunction(F))
    return false;

  WRegionInfo &WI = getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();
  DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  return Impl.runImpl(F, WI, DT, LI);
}

bool VPORenameOperandsPass::runImpl(Function &F, WRegionInfo &WI,
                                    DominatorTree *DT, LoopInfo *LI) {
  return renameOperands(F, WI, DT, LI);
}

void VPORenameOperands::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<WRegionInfoWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addPreserved<WRegionInfoWrapperPass>();
}

PreservedAnalyses VPORenameOperandsPass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  WRegionInfo &WI = AM.getResult<WRegionInfoAnalysis>(F);
  DominatorTree *DT = &AM.getResult<DominatorTreeAnalysis>(F);
  LoopInfo *LI = &AM.getResult<LoopAnalysis>(F);

  bool Changed = runImpl(F, WI, DT, LI);

  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WRegionInfoAnalysis>();
  return PA;
}

FunctionPass *llvm::createVPORenameOperandsPass() {
  return new VPORenameOperands();
}
#endif // INTEL_COLLAB

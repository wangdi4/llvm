#if INTEL_COLLAB
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
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
//===-- VPOParoptTileLoops.cpp - Loop tiling of WRegion                 --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//
//===----------------------------------------------------------------------===//
///
/// \file
/// VPOParoptTileLoops.cpp handles omp tile pragma.
///
//===----------------------------------------------------------------------===//
#if INTEL_CUSTOMIZATION
///
/// Handles fortran pragma !$omp tile sizes (num).
///
/// For example, given a loop
///
/// - before tiling
/// \code
///  !$omp tile sizes(S)
///  do I = 0, N
/// \endcode
///
/// - after tiling (interim code before normalization of the floor loop)
/// \code
///   do II = 0, N, S
///      do I = II, min(II + S - 1, N)
/// \endcode
///
/// - after normalization of floor loop
/// \code
///   do II = 0, N/S                          <-- floor loop
///      do I = II * S, min(II*S + S - 1, N)  <-- tile loop
/// \endcode
///
/// The by-strip loop introduced by tiling is called a floor loop and the loop
/// iterates within a tile is called a tile loop.
///
/// Notice we always make the floor loop to be normalized. Depending on tile
/// pragmas in the input floor loop can be a target of subsequent tiling. Since
/// we always normalize floor loops, this simplifies the implementation.
///
/// All the floor loops introduced from a tile pragma are added as outer loops
/// of the original outermost loop of the tile pragma. For example,
///
/// - before tiling
/// \code
///  !$omp tile sizes(S1, S2, S3)
///  do I = 0, N
///   do J = 0, M
///    do K = 0, L
/// \endcode
///
/// - after tiling
/// \code
///  do II = 0, N/S1 <-- floor loop
///   do JJ = 0, M/S2 <-- floor loop
///    do KK = 0, L/S3 <-- floor loop
///     do I = II*S1, ..
///      do J = JJ*S2, ..
///       do K = KK*S3, ..
/// \endcode
///
/// This pass works on non-rotated loops, where loop header is the loop exiting
/// block. When a floor loop is added as a parent of a loop, the loop's header
/// exits to its floor loop's latch, which jumps back to floor loop's header.
//
//===----------------------------------------------------------------------===//
#endif // INTEL_CUSTOMIZATION

#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptAtomics.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptModuleTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"

#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/PredIteratorCache.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/GeneralUtils.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"
#include "llvm/Transforms/Utils/LoopRotationUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
#include "Intel_DTrans/Analysis/DTransTypeMetadataBuilder.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#endif // INTEL_FEATURE_SW_DTRANS
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#endif // INTEL_CUSTOMIZATION

#include <algorithm>
#include <set>
#include <unordered_set>
#include <vector>

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-tile"

static cl::opt<bool> DisableTiling("disable-" DEBUG_TYPE, cl::Hidden,
                                   cl::init(true),
                                   cl::desc("Disable paropt loop tiling"));
namespace {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void print(ArrayRef<Value *> IVsToFill, int Size, StringRef Header) {
  dbgs() << Header << " ";
  for (int i = 0; i < Size; i++)
    IVsToFill[i]->dump();
}

void printWRNLoopInfo(WRNLoopInfo &WLoopInfo) {
  formatted_raw_ostream OS(dbgs());

  dbgs() << "== Begin WRNLoopInfo== \n";
  WLoopInfo.print(OS, 1);
  dbgs() << "== End WRNLoopInfo == \n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

} // end of namespace

namespace {
// Stripmine a loop, i.e., one-level.
// WRegionNode passed to the constructor is for information,
// not for stripmining the whole WRegionNodes multiple times
// as prescribed in tile pragma.
class Stripminer {
public:
  Stripminer(unsigned LoopDepthIndex, Value *TileSize, WRegionNode *W)
      : LoopDepthIndex(LoopDepthIndex), TileSize(TileSize), W(W),
        IndVarTy(W->getWRNLoopInfo().getNormIVElemTy(LoopDepthIndex)),
        FloorLB(nullptr), FloorUB(nullptr), FloorIV(nullptr),
        OrigUpperBound(nullptr) {
    assert(IndVarTy == W->getWRNLoopInfo().getNormUBElemTy(LoopDepthIndex) &&
           "IV and UB types are should be the same");
  }

  std::tuple<Value *, Value *, Value *>
  addFloorLoopIVBounds(Instruction *InsertPos);

  // Update original loop preheader by adding  computation of tile loops's lower
  // and upper bounds (tile.lb, tile.ub). Store for tile.lb to original.norm.iv
  // & store of tile.ub to original.norm.ub also added. Original norm.iv and
  // norm.ub are reused as tile.iv, tile.ub \p InsertPos is original loop
  // preheader.
  void addTileLoopBounds(Instruction *InsertPos);

  // Add a floor loop around the given (tile) loop.
  // As a result, CFG is changed.
  // Given the current \p OutermostPreheader and \p OutermostHeader,
  // returns a new \p OutermostPreheader, \p OutermostHeader,
  // and \p OutermostLatch.
  // \p Pred is the unique predecessor of \p OutermostPreheader.
  // The returned results corresponde to those of added floor loop.
  // The new outermost preheader and header are added between
  // the current Entry block and outermost preheader.
  std::tuple<BasicBlock *, BasicBlock *, BasicBlock *>
  addFloorLoop(BasicBlock *Pred, BasicBlock *OutermostPreheader,
               BasicBlock *OutermostHeader);

private:
  unsigned LoopDepthIndex;
  Value *TileSize;
  WRegionNode *W;
  Type *IndVarTy;

  // outputs
  AllocaInst *FloorLB;
  AllocaInst *FloorUB;
  AllocaInst *FloorIV;

  // This is a copy of Orig UB. Needed because the orig ub is reused as tile ub.
  AllocaInst *OrigUpperBound;
};

// Top-level handler of tiling per WRegionNode, i.e. per omp tile pragma.
// Takes care of all tiles sizes given in the pragma.
// The parent WRegionNode is updated as needed.
class WRegionNodeTiler {

  typedef SmallVector<Value *, 4> BundleOprsTy;
  typedef SmallVectorImpl<Value *> BundleOprsImplTy;

public:
  WRegionNodeTiler(WRegionNode *W) : W(W) {}

  void run();

private:
  // Updates parent WRegionNode's LoopInfo.
  // Parent's LoopInfo needs to be updated when
  // tile pragmas are back to back on a loop nest.
  // In that case, before tiling, two or more WRegionNode will
  // contain the same outermost loop. However, as the inner WRegionNode
  // is handled with tiling, new outer loops are added, leading to
  // a new outermost loop for the parent WRegionNode. This change needs
  // to be reflected to the LoopInfo of the parent WRegionNode.
  // Notice only the minimum amount LoopInfo are updated.
  // Not all related information, DT, all basic blocks, and so on, are
  // kept up to date.
  void updateParentRegionLoopInfo(ArrayRef<Value *> GenIVs,
                                  ArrayRef<Value *> GenUBs, int NumToFill,
                                  ArrayRef<BasicBlock *> FloorLoopPreHeaders,
                                  ArrayRef<BasicBlock *> FloorLoopHeaders,
                                  ArrayRef<BasicBlock *> FloorLoopLatches);

  // Directly updates LLVM IR's call to llvm.directive.region.entry()
  // Needed with outer omp do or tile pragma
  void updateParentRegionEntry(ArrayRef<Value *> GenIVs,
                               ArrayRef<Value *> GenLBs,
                               ArrayRef<Value *> GenUBs, int NumToFill);

  void updateParentRegion(ArrayRef<Value *> GenIVs, ArrayRef<Value *> GenLBs,
                          ArrayRef<Value *> GenUBs,
                          ArrayRef<BasicBlock *> FloorLoopPreHeaders,
                          ArrayRef<BasicBlock *> FloorLoopHeaders,
                          ArrayRef<BasicBlock *> FloorLoopLatches);

private:
  WRegionNode *W;
};

// GenIVs, GenUBs are in inner to outer order.
// FloorLoop info are in inner to outer order.
void WRegionNodeTiler::updateParentRegionLoopInfo(
    ArrayRef<Value *> GenIVs, ArrayRef<Value *> GenUBs, int NumToFill,
    ArrayRef<BasicBlock *> FloorLoopPreHeaders,
    ArrayRef<BasicBlock *> FloorLoopHeaders,
    ArrayRef<BasicBlock *> FloorLoopLatches) {

  WRegionNode *WParent = W->getParent();
  auto &WParentLoopInfo = WParent->getWRNLoopInfo();
  bool BackToBackWRegions = WParentLoopInfo.getNormIVSize() == 0;

  // GenIVs, GenUBs, GenLBs, contain information from inner to outer.
  // Iterate them from back to front.
  for (int I = NumToFill - 1; I >= 0; I--) {
    auto *V = cast<AllocaInst>(GenIVs[I]);
    auto *VTy = V->getAllocatedType();
    WParentLoopInfo.addNormIV(V, VTy);
  }

  for (int I = NumToFill - 1; I >= 0; I--) {
    auto *V = cast<AllocaInst>(GenUBs[I]);
    auto *VTy = V->getAllocatedType();
    WParentLoopInfo.addNormUB(V, VTy);
  }

  // No need to worry for GenLBs

  //
  //  From
  //    ParentLoop --> Loop
  //  To
  //    ParentLoop --> New --> Loop
  //
  // All the floor loops are added from the outside of CurLoop
  Loop *CurLoop = (W->getWRNLoopInfo()).getLoop(0);

  unsigned I = 0;
  unsigned NumFloorLoops = FloorLoopHeaders.size();
  LoopInfo *LI = WParentLoopInfo.getLoopInfo();
  while (I < NumFloorLoops) {
    Loop *ParentLoop = CurLoop->getParentLoop();

    Loop *FloorLoop = LI->AllocateLoop();

    if (!ParentLoop) {
      LI->changeTopLevelLoop(CurLoop, FloorLoop);
    } else {
      ParentLoop->replaceChildLoopWith(CurLoop, FloorLoop);
      FloorLoop->addChildLoop(CurLoop);
      ParentLoop->addBasicBlockToLoop(FloorLoopPreHeaders[I], *LI);
    }

    //  Do the adding of basic block at the end because
    //  BasicBlocks are added on all outer loops of the loop.
    FloorLoop->addBasicBlockToLoop(FloorLoopHeaders[I], *LI);
    FloorLoop->addBasicBlockToLoop(FloorLoopLatches[I], *LI);

    CurLoop = FloorLoop;
    I++;
  }

  // If WParent and W were back-to-back, meaning
  // no do-loop between the two tile pragmas
  // WParent's outermost loop is also updated by the outermost
  // floor loop of W.
  if (BackToBackWRegions)
    WParentLoopInfo.setLoop(CurLoop);

  LLVM_DEBUG(printWRNLoopInfo(WParentLoopInfo));
}

// Copy existing type bundles or make it typed if they were not
// originally typed.
static void getTypedIVUBBundles(bool IsTyped, const OperandBundleDef &Bundle,
                                SmallVectorImpl<Value *> &NewTypedBundles) {
  if (IsTyped)
    NewTypedBundles.insert(NewTypedBundles.begin(), Bundle.input_begin(),
                           Bundle.input_end());
  else
    llvm::for_each(Bundle.inputs(), [&](Value *V) {
      NewTypedBundles.push_back(V);

      assert(!V->getType()->isOpaquePointerTy() &&
             "Need Typed IV/UB clauses for opaque pointers.");
      Type *VTy = isa<PointerType>(V->getType())
                      ? V->getType()->getNonOpaquePointerElementType()
                      : V->getType();

      NewTypedBundles.push_back(Constant::getNullValue(VTy));
    });
}

// Update Parent's WRNNode's intrinsic if NumToFill > 0
// GenIVs, LBs, and UBs contain all generated data.
// Their size is not smaller than NumToFill.
void WRegionNodeTiler::updateParentRegionEntry(ArrayRef<Value *> GenIVs,
                                               ArrayRef<Value *> GenLBs,
                                               ArrayRef<Value *> GenUBs,
                                               int NumToFill) {

  assert(NumToFill > 0 && "No information to add");

  WRegionNode *WParent = W->getParent();
  assert(WParent && "Parent WRNNode should exist");
  CallInst *EntryCI = cast<CallInst>(WParent->getEntryDirective());

  SmallVector<OperandBundleDef, 16> OpBundles;
  EntryCI->getOperandBundlesAsDefs(OpBundles);

  SmallVector<OperandBundleDef, 6> NewBundles;

  // Remaining original bundles other than normalized iv, lb, ub.
  // Notice these should be added first before normalized iv, lb, ub.
  // E.g., OMP.DO.LOOP should be the first bundle.
  SmallVector<Value *, 4> IVBundleVals;
  SmallVector<Value *, 4> UBBundleVals;
  for (auto &Bundle : OpBundles) {
    StringRef ClauseString = Bundle.getTag();

    if (!VPOAnalysisUtils::isOpenMPClause(ClauseString)) {
      NewBundles.emplace_back(ClauseString.str(), Bundle.inputs());
      continue;
    }

    ClauseSpecifier ClauseInfo(ClauseString);
    int ClauseId = ClauseInfo.getId();
    if (ClauseId == QUAL_OMP_NORMALIZED_IV) {
      getTypedIVUBBundles(ClauseInfo.getIsTyped(), Bundle, IVBundleVals);
      continue;
    }
    if (ClauseId == QUAL_OMP_NORMALIZED_UB) {
      getTypedIVUBBundles(ClauseInfo.getIsTyped(), Bundle, UBBundleVals);
      continue;
    }

    NewBundles.emplace_back(ClauseString.str(), Bundle.inputs());
  }

  // Add NumToFill GenIVs to IVBundleVals
  // GenIVs, GenUBs, GenLBs, contain information from inner to outer.
  // Iterate them from back to front.
  for (Value *GenIV : make_range(GenIVs.rbegin(), GenIVs.rend())) {
    IVBundleVals.push_back(GenIV);
    Value *TypeV =
        Constant::getNullValue(cast<AllocaInst>(GenIV)->getAllocatedType());
    IVBundleVals.push_back(TypeV);
  }
  NewBundles.emplace_back(
      VPOAnalysisUtils::getTypedClauseString(QUAL_OMP_NORMALIZED_IV),
      IVBundleVals);

  for (Value *GenUB : make_range(GenUBs.rbegin(), GenUBs.rend())) {
    UBBundleVals.push_back(GenUB);
    Value *TypeV =
        Constant::getNullValue(cast<AllocaInst>(GenUB)->getAllocatedType());
    UBBundleVals.push_back(TypeV);
  }
  NewBundles.emplace_back(
      VPOAnalysisUtils::getTypedClauseString(QUAL_OMP_NORMALIZED_UB),
      UBBundleVals);

  // LB as live-ins
  // Each LB is taken care of separately.
  Value *NumElemsOne =
      ConstantInt::get(Type::getInt32Ty(EntryCI->getContext()), 1);
  for (int I = NumToFill - 1; I >= 0; I--) {
    Value *TypeV =
        Constant::getNullValue(cast<AllocaInst>(GenLBs[I])->getAllocatedType());

    if (WParent->canHaveFirstprivate())
      NewBundles.emplace_back(
          VPOAnalysisUtils::getTypedClauseString(QUAL_OMP_FIRSTPRIVATE),
          makeArrayRef({GenLBs[I], TypeV, NumElemsOne}));
    else if (WParent->canHaveShared())
      NewBundles.emplace_back(
          VPOAnalysisUtils::getTypedClauseString(QUAL_OMP_SHARED),
          makeArrayRef({GenLBs[I], TypeV, NumElemsOne}));
    else if (WParent->canHaveLivein())
      NewBundles.emplace_back(
          VPOAnalysisUtils::getClauseString(QUAL_OMP_LIVEIN).str(), GenLBs[I]);
    else
      llvm_unreachable("Unexpected parent WRN");
  }

  // Recreate the directive call.
  SmallVector<Value *, 8> Args(EntryCI->arg_begin(), EntryCI->arg_end());
  auto *NewEntryCI =
      CallInst::Create(EntryCI->getFunctionType(), EntryCI->getCalledOperand(),
                       Args, NewBundles, "", EntryCI);
  NewEntryCI->takeName(EntryCI);
  NewEntryCI->setCallingConv(EntryCI->getCallingConv());
  NewEntryCI->setAttributes(EntryCI->getAttributes());
  NewEntryCI->setDebugLoc(EntryCI->getDebugLoc());
  EntryCI->replaceAllUsesWith(NewEntryCI);
  EntryCI->eraseFromParent();
}

void WRegionNodeTiler::updateParentRegion(
    ArrayRef<Value *> GenIVs, ArrayRef<Value *> GenLBs,
    ArrayRef<Value *> GenUBs, ArrayRef<BasicBlock *> FloorLoopPreHeaders,
    ArrayRef<BasicBlock *> FloorLoopHeaders,
    ArrayRef<BasicBlock *> FloorLoopLatches) {

  int NumToFill = 0;
  WRegionNode *WParent = W->getParent();
  assert(WParent && "Parent WRNNode should exist");
  if (WParent->getIsOmpLoopOrLoopTransform()) {
    auto ParentWLoopInfo = WParent->getWRNLoopInfo();
    NumToFill = WParent->getOmpLoopDepth() - ParentWLoopInfo.getNormIVSize();
  }
  assert(NumToFill >= 0 && "Invalid status");

  LLVM_DEBUG(dbgs() << "NumToFill: " << NumToFill << "\n");

  if (NumToFill == 0)
    return;

  assert(GenIVs.size() >= (unsigned)NumToFill &&
         "Not enough IVs are generated");

  LLVM_DEBUG(print(GenIVs, NumToFill, "GenIVs"));
  LLVM_DEBUG(print(GenLBs, NumToFill, "GenLBs"));
  LLVM_DEBUG(print(GenUBs, NumToFill, "GenUBs"));

  if (WParent->getIsOmpLoop()) // omp do
    updateParentRegionEntry(GenIVs, GenLBs, GenUBs, NumToFill);
  else if (WParent->getIsOmpLoopTransform()) // omp tile
    updateParentRegionLoopInfo(GenIVs, GenUBs, NumToFill, FloorLoopPreHeaders,
                               FloorLoopHeaders, FloorLoopLatches);
  else
    llvm_unreachable("omp tile: parent region should be either do or tile");
}

} // end of namespace

// Take care of loops from innermost to outermost
void WRegionNodeTiler::run() {
  // Get the size of Sizes - how many loops are tiled in this WRegionNode
  unsigned NumTileLoops = W->getSizes().size();

  LLVM_DEBUG(dbgs() << "NumTileLoops: " << NumTileLoops << "\n");

  // Outermost Alloca Pos
  auto &WLoopInfo = W->getWRNLoopInfo();

  LLVM_DEBUG(printWRNLoopInfo(WLoopInfo));

  auto *OutermostIV = WLoopInfo.getNormIV(0);
  auto *AllocaPos = cast<AllocaInst>(OutermostIV)->getParent()->getTerminator();

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Outermost Loop from LoopInfo:\n";
             auto *OL = WLoopInfo.getLoop(0); OL->dump();
             dbgs() << "Size of its subloop: " << OL->getSubLoops().size()
                    << "\n");

  auto *OutermostPreheader = WLoopInfo.getLoop(0)->getLoopPreheader();
  auto *OutermostHeader = WLoopInfo.getLoop(0)->getHeader();
  BasicBlock *OutermostLatch = nullptr;

  assert(OutermostPreheader && OutermostHeader &&
         "Outermost preheader and header should exist!");

  // TODO: See if there is accessor with direct indexing to Clause<>
  SmallVector<Value *, 4> TileSizes;
  for (auto *TileSize : W->getSizes().items())
    TileSizes.push_back(TileSize->getOrig());

  LLVM_DEBUG(dbgs() << "TileSizes.size: " << TileSizes.size() << "\n");
  assert(NumTileLoops == TileSizes.size() && "TileSize mismatch");

  // Loops are proccessed from the innermost to outermost direction.
  // Notice that the smaller LoopDepthIndex is the outer the loop is.
  // In other words, the innermost loop has the largest LoopDepthIndex,
  // and the outermost loop has the smallest LoopDepthIndex.
  // CollectAllNormIVs and keep the outermost NumNormIVsToFill
  // for the paretn WRegionNode
  // Data Structure for Parent WRegionNode
  SmallVector<Value *, 4> GenIVs;
  SmallVector<Value *, 4> GenLBs;
  SmallVector<Value *, 4> GenUBs;
  SmallVector<BasicBlock *, 4> FloorLoopPreHeaders;
  SmallVector<BasicBlock *, 4> FloorLoopHeaders;
  SmallVector<BasicBlock *, 4> FloorLoopLatches;

  for (int LoopDepthIndex = NumTileLoops - 1; LoopDepthIndex >= 0;
       LoopDepthIndex--) {

    LLVM_DEBUG(dbgs() << "LoopDepthIndex processed: " << LoopDepthIndex
                      << "\n");

    Value *TileSize = TileSizes[LoopDepthIndex];

    LLVM_DEBUG(dbgs() << "Tilesize processed: "; TileSize->dump());

    Stripminer SM(LoopDepthIndex, TileSize, W);

    Value *NormIV = nullptr;
    Value *NormLB = nullptr;
    Value *NormUB = nullptr;

    std::tie(NormIV, NormLB, NormUB) = SM.addFloorLoopIVBounds(AllocaPos);

    assert(NormIV && NormLB && NormUB &&
           "Some normalized loop info is missing");
    GenIVs.push_back(NormIV);
    GenLBs.push_back(NormLB);
    GenUBs.push_back(NormUB);

    LLVM_DEBUG(
        dbgs()
        << "TileBounds BB: "
        << WLoopInfo.getLoop(LoopDepthIndex)->getLoopPreheader()->getName()
        << "\n");

    auto TileBoundsPos =
        WLoopInfo.getLoop(LoopDepthIndex)->getLoopPreheader()->getTerminator();
    SM.addTileLoopBounds(TileBoundsPos);

    BasicBlock *Pred = OutermostPreheader->getUniquePredecessor();
    assert(Pred && "No predecessor block of preheader");
    std::tie(OutermostPreheader, OutermostHeader, OutermostLatch) =
        SM.addFloorLoop(Pred, OutermostPreheader, OutermostHeader);

    FloorLoopPreHeaders.push_back(OutermostPreheader);
    FloorLoopHeaders.push_back(OutermostHeader);
    FloorLoopLatches.push_back(OutermostLatch);
  }

  auto *WParent = W->getParent();
  if (WParent)
    updateParentRegion(GenIVs, GenLBs, GenUBs, FloorLoopPreHeaders,
                       FloorLoopHeaders, FloorLoopLatches);
}

std::tuple<Value *, Value *, Value *>
Stripminer::addFloorLoopIVBounds(Instruction *InsertPos) {

  auto &WLoopInfo = W->getWRNLoopInfo();

  IRBuilder<> AllocaBuilder(InsertPos);
  FloorLB = AllocaBuilder.CreateAlloca(IndVarTy, nullptr, "floor_lb");
  auto *Zero =
      AllocaBuilder.getIntN(cast<IntegerType>(IndVarTy)->getBitWidth(), 0);

  LLVM_DEBUG(dbgs() << "Zero: "; Zero->dump());
  LLVM_DEBUG(dbgs() << "FloorLB: "; FloorLB->dump());

  AllocaBuilder.CreateStore(Zero, FloorLB);
  FloorUB = AllocaBuilder.CreateAlloca(IndVarTy, nullptr, "floor_ub");

  // Get the Original UB
  // Currently no-optimization, just load from memory
  // Can be replaced with value stored to NormUB bar
  auto *OrigUB = WLoopInfo.getNormUB(LoopDepthIndex);
  assert(OrigUB && "Missing Normalized upper bound!");

  LLVM_DEBUG(dbgs() << "OrigUB: "; OrigUB->dump());

  auto *OrigUBLoad =
      AllocaBuilder.CreateLoad(IndVarTy, OrigUB, "norm.orig.ub.val");

  // floorUB  =  OrigUBLoad / TileSize
  // Notice possibility of type mismatch
  // e.g.) OrigUBLoad - 64bit, TileSize - 32 bit.
  TileSize = AllocaBuilder.CreateSExtOrTrunc(TileSize, IndVarTy, ".sext");
  auto *NormFloorUB =
      AllocaBuilder.CreateSDiv(OrigUBLoad, TileSize, "norm.floor.ub.val");

  LLVM_DEBUG(dbgs() << "NormFloorUB: "; NormFloorUB->dump());
  LLVM_DEBUG(dbgs() << "FloorUB: "; FloorUB->dump());

  AllocaBuilder.CreateStore(NormFloorUB, FloorUB);
  FloorIV = AllocaBuilder.CreateAlloca(IndVarTy, nullptr, "floor_iv");

  auto *FloorLBLoad = AllocaBuilder.CreateLoad(IndVarTy, FloorLB, "floor.lb");
  AllocaBuilder.CreateStore(FloorLBLoad, FloorIV);
  OrigUpperBound = AllocaBuilder.CreateAlloca(IndVarTy, nullptr, "orig_ub");
  AllocaBuilder.CreateStore(OrigUBLoad, OrigUpperBound);

  return {FloorIV, FloorLB, FloorUB};
}

void Stripminer::addTileLoopBounds(Instruction *InsertPos) {

  auto &WLoopInfo = W->getWRNLoopInfo();

  IRBuilder<> Builder(InsertPos);

  // Compute tile.lb.val = TileSize * FloorIV.val
  auto *FloorIVVal = Builder.CreateLoad(IndVarTy, FloorIV, "norm.floor.iv.val");
  auto *TileLB = Builder.CreateMul(TileSize, FloorIVVal, "tile.lb");

  // Store TileLB to OrigNormIV
  Builder.CreateStore(TileLB, WLoopInfo.getNormIV(LoopDepthIndex));

  // Compute tile.ub.val = min(tile.lb + TileSize - 1, OrigUB)
  auto *LoadOrigUB =
      Builder.CreateLoad(IndVarTy, OrigUpperBound, "norm.orig.ub.val");
  auto *Add = Builder.CreateAdd(TileLB, TileSize, "add");
  auto *Dec = Builder.CreateSub(
      Add, Builder.getIntN(cast<IntegerType>(IndVarTy)->getBitWidth(), 1),
      "dec");

  // TODO: figure out if the original loop's IV is signed or not
  // TODO: This is min(a,b)
  bool IsSigned = true;
  Value *Compare =
      Builder.CreateICmp(IsSigned ? ICmpInst::ICMP_SLE : ICmpInst::ICMP_ULE,
                         Dec, LoadOrigUB, "cond");
  Value *TileUB = Builder.CreateSelect(Compare, Dec, LoadOrigUB, "tile.ub");

  LLVM_DEBUG(dbgs() << "TileUB: "; TileUB->dump());

  // Store tile.ub.val to the original normUB
  Builder.CreateStore(TileUB, WLoopInfo.getNormUB(LoopDepthIndex));
}

std::tuple<BasicBlock *, BasicBlock *, BasicBlock *>
Stripminer::addFloorLoop(BasicBlock *Pred, BasicBlock *OutermostPreheader,
                         BasicBlock *OutermostHeader) {

  auto *ExitBlock = W->getExitBBlock();

  // 1. Fill in Floor Header
  // flooriv = load
  // floorub = load
  // br flooriv <= floorub OutermostPreheader, ExitBlock
  BasicBlock *FloorHeader = SplitEdge(Pred, OutermostPreheader, nullptr,
                                      nullptr, nullptr, "FLOOR.HEAD");
  auto *UncondBr = FloorHeader->getTerminator();
  IRBuilder<> Builder(UncondBr);
  auto *FloorIVVal = Builder.CreateLoad(IndVarTy, FloorIV, "floor.iv");
  auto *FloorUBVal = Builder.CreateLoad(IndVarTy, FloorUB, "floor.ub");
  // TODO: signed/unsigned comparison
  Builder.CreateCondBr(Builder.CreateICmp(ICmpInst::ICMP_SLE, FloorIVVal,
                                          FloorUBVal, "tile.loop.cond"),
                       OutermostPreheader, ExitBlock);
  // Replace uncond br to cond br
  UncondBr->eraseFromParent();

  LLVM_DEBUG(dbgs() << "FloorHeadBlock 2\n"; FloorHeader->dump());

  // Add reset of floor_iv between FloorHeader & its predecessor
  // I.e.
  //     $floor.lb.val = load %floor_lb
  //     store %floor.lb.val to %floor_iv
  // However, we always normalize floor-loops. Thus, it can simply be
  //      store 0 to %floor_iv
  // First split edge from FloorHeader's pred to FloorHeader
  BasicBlock *FloorPreHeader =
      SplitEdge(Pred, FloorHeader, nullptr, nullptr, nullptr, "FLOOR.PREHEAD");

  // Add store 0 to %floor_iv
  Builder.SetInsertPoint(FloorPreHeader->getTerminator());
  auto *Zero = Builder.getIntN(cast<IntegerType>(IndVarTy)->getBitWidth(), 0);
  Builder.CreateStore(Zero, FloorIV);

  // 2. Replace OutermostHeader's br to Floor Latch
  auto *LoopCondBr = cast<BranchInst>(OutermostHeader->getTerminator());
  unsigned ExitTargetId = LoopCondBr->getSuccessor(0) == ExitBlock ? 0 : 1;
  LoopCondBr->setSuccessor(ExitTargetId, FloorHeader);

  auto *FloorLatch = SplitEdge(OutermostHeader, FloorHeader, nullptr, nullptr,
                               nullptr, "FLOOR.LATCH");

  LLVM_DEBUG(dbgs() << "After splitting edge - floor latch\n";
             FloorHeader->dump());

  // 3. Fill in Floor Latch
  IRBuilder<> LatchBuilder(FloorLatch->getTerminator());
  FloorIVVal = LatchBuilder.CreateLoad(IndVarTy, FloorIV, "floor.iv");
  auto *Inc = LatchBuilder.CreateAdd(
      FloorIVVal,
      LatchBuilder.getIntN(cast<IntegerType>(IndVarTy)->getBitWidth(), 1),
      "inc");
  LatchBuilder.CreateStore(Inc, FloorIV);

  return {FloorPreHeader, FloorHeader, FloorLatch};
}

bool VPOParoptTransform::tileOmpLoops(WRegionNode *W) {

  if (DisableTiling)
    return false;

  auto Exiter = [FunctionName = __FUNCTION__, W](bool Changed) {
    W->resetBBSetIfChanged(Changed);
    (void)FunctionName;
    LLVM_DEBUG(dbgs() << FunctionName << ": finished loop tiling\n");
    return Changed;
  };

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": starting loops tiling\n");

  if (!W->getIsOmpLoopTransform()) {
    LLVM_DEBUG(dbgs() << "Not a loop transform construct.  Exiting.\n");
    return Exiter(false);
  }

  WRegionNodeTiler(W).run();

  // @entry/@exit still exists
  LLVM_DEBUG(dbgs() << "=== AFTER tile == \n"
                    << *(F->getParent()) << "\n === End === \n");

  unsigned NumLoops = W->getWRNLoopInfo().getNormIVSize();

  LLVM_DEBUG(dbgs() << "In Tile - NumLoops: "; dbgs() << NumLoops << "\n");
  // Notice that LoopInfo, or WRNRegionInfo did not get updated.
  LLVM_DEBUG(W->dump());

  if (NumLoops == 0) {
    // SIMD loops combined with other loop constructs do not have
    // associated loops.
    LLVM_DEBUG(dbgs() << "Loop is associated with an enclosing construct.\n");
    return Exiter(false);
  }

  return Exiter(true);
}
#endif // INTEL_COLLAB

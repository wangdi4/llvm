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
                                   cl::init(false),
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
  Stripminer(unsigned LoopDepthIndex, Value *TileSize, WRegionNode *W,
             bool IsSPIRVTarget)
      : LoopDepthIndex(LoopDepthIndex), TileSize(TileSize), W(W),
        IndVarTy(W->getWRNLoopInfo().getNormIVElemTy(LoopDepthIndex)),
        FloorLB(nullptr), FloorUB(nullptr), FloorIV(nullptr),
        ClonedOrigUpperBound(nullptr), IsSPIRVTarget(IsSPIRVTarget) {
    assert(IndVarTy == W->getWRNLoopInfo().getNormUBElemTy(LoopDepthIndex) &&
           "IV and UB types are should be the same");
  }

  void addFloorLoopIVBounds(Instruction *AllocaPos, Instruction *InitInsertPos);

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

public:
  Type *IndVarTy;

  // outputs
  Value *FloorLB;
  Value *FloorUB;
  Value *FloorIV;

  // This is a copy of Orig UB, which will perserve the original value. Needed
  // because the orig ub(e.g. %norm.pdo.norm.ub) is reused as tile ub after
  // tiling and doesn't continue to contain the original UB val.
  Value *ClonedOrigUpperBound;

private:
  bool IsSPIRVTarget;
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
                                  ArrayRef<Value *> GenUBs,
                                  ArrayRef<Type *> GenIVTypes, int NumToFill,
                                  ArrayRef<BasicBlock *> FloorLoopPreHeaders,
                                  ArrayRef<BasicBlock *> FloorLoopHeaders,
                                  ArrayRef<BasicBlock *> FloorLoopLatches);

  // Directly updates LLVM IR's call to llvm.directive.region.entry()
  // Needed with outer omp do or tile pragma
  void updateParentRegionEntry(ArrayRef<Value *> GenIVs,
                               ArrayRef<Value *> GenLBs,
                               ArrayRef<Value *> GenUBs,
                               ArrayRef<Type *> GenIVTypes, int NumToFill,
                               int StartingOverapIVIndex);

  void updateParentRegion(ArrayRef<Value *> GenIVs, ArrayRef<Value *> GenLBs,
                          ArrayRef<Value *> GenUBs, ArrayRef<Type *> GenIVTypes,
                          ArrayRef<BasicBlock *> FloorLoopPreHeaders,
                          ArrayRef<BasicBlock *> FloorLoopHeaders,
                          ArrayRef<BasicBlock *> FloorLoopLatches);

  void updateOuterRegions(DominatorTree *DT, BasicBlock *InitPtsBB,
                          WRegionNode *ParOutermostWRN, Value *GenIV,
                          Value *GenLB, Value *GenUB, Value *OrigIV,
                          Value *OrigLB, Value *OrigUB, Value *ClonedOrigUB);

private:
  WRegionNode *W;
};

// GenIVs, GenUBs are in outer to inner order (i.e. loop depth order).
// FloorLoop info are in outer to inner order.
void WRegionNodeTiler::updateParentRegionLoopInfo(
    ArrayRef<Value *> GenIVs, ArrayRef<Value *> GenUBs,
    ArrayRef<Type *> GenIVTypes, int NumToFill,
    ArrayRef<BasicBlock *> FloorLoopPreHeaders,
    ArrayRef<BasicBlock *> FloorLoopHeaders,
    ArrayRef<BasicBlock *> FloorLoopLatches) {

  WRegionNode *WParent = W->getParent();
  auto &WParentLoopInfo = WParent->getWRNLoopInfo();
  bool BackToBackWRegions = WParentLoopInfo.getNormIVSize() == 0;

  // GenIVs, GenUBs, GenLBs, contain information from inner to outer.
  for (int I = 0; I < NumToFill; I++) {
    WParentLoopInfo.addNormIV(GenIVs[I], GenIVTypes[I]);
  }

  for (int I = 0; I < NumToFill; I++) {
    WParentLoopInfo.addNormUB(GenUBs[I], GenIVTypes[I]);
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
      // Notice that changeTopLevelLoop() doesn't
      // make CurLoop as a subloop of FloorLoop.
      LI->changeTopLevelLoop(CurLoop, FloorLoop);
      FloorLoop->addChildLoop(CurLoop);
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

// Ernesto's implementation
// Given a TILE WRN "WTile" and its immediate parent construct "WParent",
// this util provides information about whether their loops overlap,
// and to what extent. It returns this pair of ints:
//
//   - NumberOfOverlappingIVs: number of IVs that overlap;
//                             0 if no overlap or parent is not loop-type.
//   - OverlapIVIdx: index of WParent's NormIV that is the same as WTile's
//                   first NormIV. Meaningless if NumberOfOverlappingIVs == 0.
//
// Examples:
//
// 1. NumberOfOverlappingIVs = 1, OverlapIVIdx = 0 (= index of i)
//
//     $omp do                       !  IV: i
//     $omp tile sizes(4)            !  IV: i
//     do i = ...
//
// 2. NumberOfOverlappingIVs = 1, OverlapIVIdx = 0
//
//     $omp do                       !  IV: i
//     $omp tile sizes(4, 2)         !  IV: i, j
//     do i = ...
//       do j = ...
//
// 3. NumberOfOverlappingIVs = 2, OverlapIVIdx = 0
//
//     $omp do collapse(2)           !  IV: i, j
//     $omp tile sizes(4, 2)         !  IV: i, j
//     do i = ...
//       do j = ...
//
// 4. NumberOfOverlappingIVs = 0 (NO OVERLAP)
//
//     $omp do                       !  IV: i
//     do i = ...
//       $omp tile sizes(4, 2)       !  IV:    j, k
//       do j = ...
//         do k = ...
//
// 5. NumberOfOverlappingIVs = 1, OverlapIVIdx = 1 (= index of j)
//
//     $omp do collapse(2)           !  IV: i, j
//     do i = ...
//       $omp tile sizes(4, 2)       !  IV:    j, k
//       do j = ...
//         do k = ...
//
// 6. NumberOfOverlappingIVs = 2, OverlapIVIdx = 1
//
//     $omp do collapse(3)           !  IV: i, j, k
//     do i = ...
//       $omp tile sizes(4, 2)       !  IV:    j, k
//       do j = ...
//         do k = ...
//
// 7. Assertion triggered. This case should have been filtered out by FFE.
//    This tile construct results in two floor loops and two tile loops.
//    The "do collapse(4)" will try to collapse the i-loop, the two floor loops,
//    and the outer tile loop. In general the tile loops don't have canonical
//    loop nest form (per OpenMP spec) so we don't support this case.
//
//     $omp do collapse(4)           !  IV: i, j, k, m
//     do i = ...
//       $omp tile sizes(4, 2)       !  IV:    j, k
//       do j = ...
//         do k = ...
//           do m = ...
//
static std::pair<int, int> getOverlapIVs(WRegionNode *WTile,
                                         WRegionNode *WParent) {
  int NumberOfOverlappingIVs = 0;
  int OverlapIVIdx = 0; // meaningful only if NumberOfOverlappingIVs > 0

  assert(isa<WRNTileNode>(WTile) &&
         "getOverlapIVs: WTile must be a TILE construct");

  // If Parent is not a loop-type construct then there is no overlap
  if (!WParent->getIsOmpLoopOrLoopTransform())
    return {NumberOfOverlappingIVs, OverlapIVIdx};

  int ParentNumIVs = WParent->getWRNLoopInfo().getNormIVSize();
  int TileNumIVs = WTile->getWRNLoopInfo().getNormIVSize();

  Value *TileFirstIV = WTile->getWRNLoopInfo().getNormIV(0);
  for (int i = 0; i < ParentNumIVs; ++i) {
    Value *ParentIV = WParent->getWRNLoopInfo().getNormIV(i);
    if (ParentIV == TileFirstIV) {
      OverlapIVIdx = i;
      NumberOfOverlappingIVs = ParentNumIVs - OverlapIVIdx;
      if (NumberOfOverlappingIVs > TileNumIVs)
        llvm_unreachable(
            "Unsupported: Parent loop nest goes into tile-loops generated by "
            "the TILE construct");
      break;
    }
  }

  return {NumberOfOverlappingIVs, OverlapIVIdx};
}

static bool isSIMDLoop(const WRegionNode *WNode) {
  if (const WRNVecLoopNode *VecLoop = dyn_cast<WRNVecLoopNode>(WNode))
    return VecLoop->isOmpSIMDLoop();

  return false;
}

// Find the outermost WRN associated with the same loop with LoopNode.
// LoopNode is a WRN that is associated with a loop.
// Roughly speaking, this is the immediate parent WRN of LoopNode with
// overlapping IVs. (Refer to the examples of function getOverlapIVs().
// In case when the immediate parent is a SIMD node, the parent of the
// SIMD node can be the outermose WRN of the interest.
//
// Refer to the following examples.
//
// #pragma omp for simd // "dir.omp.for" "normalized.iv"(%iv)
//                      // "dir.omp.simd" // no normalized.ivs/ubs here
// #pragma omp tile     // "dir.omp.tile" "normalized.iv"(%iv)
// omp-for is the outermost
//
// #pragma omp simd // "dir.omp.simd" "normalized.iv"(%iv)
// #pragma omp tile // "dir.omp.tile" "normalized.iv"(%iv)
// simd is the outermost
//
// For now, back-to-back tiles for one loop are NOT considered.
static WRegionNode *findOutermostWRNLoopNode(WRegionNode *TileLoopNode) {
  WRegionNode *CandidateWOutermost = TileLoopNode->getParent();
  if (!CandidateWOutermost)
    return TileLoopNode;

  WRegionNode *CurrWOutermost = TileLoopNode;

  while (CandidateWOutermost) {

    bool IsSIMDParent = isSIMDLoop(CandidateWOutermost);
    if (IsSIMDParent &&
        CandidateWOutermost->getWRNLoopInfo().getNormIVSize() == 0) {
      // SIMD - without norm IVs
      CandidateWOutermost = CandidateWOutermost->getParent();
      assert(CandidateWOutermost &&
             CandidateWOutermost->getWRNLoopInfo().getNormIVSize() != 0 &&
             "SIMD directive's parent not found.");
      continue;
    }

    if (!CandidateWOutermost->getIsOmpLoopTransform()) {
      const auto [NumOverlappingIVs, DummyIndex] =
          getOverlapIVs(TileLoopNode, CandidateWOutermost);

      if (!NumOverlappingIVs)
        break;
    } else {

      // getOverlapIVs WParent->getWRNLoopInfo().getNormIV(i)
      // may not work for tile+tile environment as new floor_loops are added
      // in-between the existing tile (before +) and tile (after +).
      // Thus, stick to the current working logic, which uses LoopInfo.
      auto ParentWLoopInfo = CandidateWOutermost->getWRNLoopInfo();
      int NumToFill = CandidateWOutermost->getOmpLoopDepth() -
                      ParentWLoopInfo.getNormIVSize();
      if (!NumToFill)
        break;
    }

    CurrWOutermost = CandidateWOutermost;
    CandidateWOutermost = CandidateWOutermost->getParent();
  }

  return CurrWOutermost;
}

// Copy existing type bundles or make it typed if they were not
// originally typed.
static int getTypedIVUBBundles(const OperandBundleDef &Bundle,
                               SmallVectorImpl<Value *> &NewTypedBundles,
                               int NumToCopy) {

  NumToCopy = std::min(NumToCopy, (int)(Bundle.inputs().size()));
  // The format has IV and its initial value as a pair
  // "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv1, i32 0,
  //                                ptr %omp.pdo.norm.iv, i32 0),
  for (int I = 0, NumItems = NumToCopy * 2; I < NumItems; I++)
    NewTypedBundles.push_back(Bundle.inputs()[I]);

  // Return the number of items(pairs) copied
  return NumToCopy;
}

// Recreate the directive call.
static void
replaceBundles(CallInst *EntryCI,
               const SmallVectorImpl<OperandBundleDef> &NewBundles) {
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

// Update Parent's WRNNode's intrinsic if NumToFill > 0
// GenIVs, LBs, and UBs contain all generated data.
// Their size is not smaller than NumToFill.
// Fill:    - how many IV/UB/LB tuples should be generated
// IVIndex: - From which existing index, the newly generated tuples should be
// added.
void WRegionNodeTiler::updateParentRegionEntry(ArrayRef<Value *> GenIVs,
                                               ArrayRef<Value *> GenLBs,
                                               ArrayRef<Value *> GenUBs,
                                               ArrayRef<Type *> GenIVTypes,
                                               int NumToFill,
                                               int StartingOverapIVIndex) {

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

  int NumIVsToCopy = StartingOverapIVIndex;
  int NumUBsToCopy = StartingOverapIVIndex;

  for (auto &Bundle : OpBundles) {
    StringRef ClauseString = Bundle.getTag();

    if (!VPOAnalysisUtils::isOpenMPClause(ClauseString)) {
      NewBundles.emplace_back(ClauseString.str(), Bundle.inputs());
      continue;
    }

    ClauseSpecifier ClauseInfo(ClauseString);
    int ClauseId = ClauseInfo.getId();

    if (ClauseId == QUAL_OMP_NORMALIZED_IV) {
      if (NumIVsToCopy > 0) {
        auto NumCopied =
            getTypedIVUBBundles(Bundle, IVBundleVals, NumIVsToCopy);
        NumIVsToCopy -= NumCopied;
      }
      continue;
    }

    if (ClauseId == QUAL_OMP_NORMALIZED_UB) {
      if (NumUBsToCopy > 0) {
        auto NumCopied =
            getTypedIVUBBundles(Bundle, UBBundleVals, NumUBsToCopy);
        NumUBsToCopy -= NumCopied;
      }
      continue;
    }

    NewBundles.emplace_back(ClauseString.str(), Bundle.inputs());
  }

  // Add NumToFill GenIVs to IVBundleVals
  // GenIVs, GenUBs, GenLBs, contain information from outer to inner.
  for (unsigned I = 0, Size = GenIVs.size(); I < Size; I++) {
    IVBundleVals.push_back(GenIVs[I]);
    Value *TypeV = Constant::getNullValue(GenIVTypes[I]);
    IVBundleVals.push_back(TypeV);
  }
  NewBundles.emplace_back(
      VPOAnalysisUtils::getTypedClauseString(QUAL_OMP_NORMALIZED_IV),
      IVBundleVals);

  for (unsigned I = 0, Size = GenUBs.size(); I < Size; I++) {
    UBBundleVals.push_back(GenUBs[I]);
    Value *TypeV = Constant::getNullValue(GenIVTypes[I]);
    UBBundleVals.push_back(TypeV);
  }
  NewBundles.emplace_back(
      VPOAnalysisUtils::getTypedClauseString(QUAL_OMP_NORMALIZED_UB),
      UBBundleVals);

  // LB as live-ins
  // Each LB is taken care of separately.
  Value *NumElemsOne =
      ConstantInt::get(Type::getInt32Ty(EntryCI->getContext()), 1);
  for (int I = 0; I < NumToFill; I++) {
    Value *TypeV = Constant::getNullValue(GenIVTypes[I]);

    if (WParent->canHaveFirstprivate())
      NewBundles.emplace_back(
          VPOAnalysisUtils::getTypedClauseString(QUAL_OMP_FIRSTPRIVATE),
          ArrayRef({GenLBs[I], TypeV, NumElemsOne}));
    else if (WParent->canHaveShared())
      NewBundles.emplace_back(
          VPOAnalysisUtils::getTypedClauseString(QUAL_OMP_SHARED),
          ArrayRef({GenLBs[I], TypeV, NumElemsOne}));
    else if (WParent->canHaveLivein())
      NewBundles.emplace_back(
          VPOAnalysisUtils::getClauseString(QUAL_OMP_LIVEIN).str(), GenLBs[I]);
    else
      llvm_unreachable("Unexpected parent WRN");
  }

  replaceBundles(EntryCI, NewBundles);
}

void WRegionNodeTiler::updateParentRegion(
    ArrayRef<Value *> GenIVs, ArrayRef<Value *> GenLBs,
    ArrayRef<Value *> GenUBs, ArrayRef<Type *> GenIVTypes,
    ArrayRef<BasicBlock *> FloorLoopPreHeaders,
    ArrayRef<BasicBlock *> FloorLoopHeaders,
    ArrayRef<BasicBlock *> FloorLoopLatches) {

  WRegionNode *WParent = W->getParent();
  if (WParent->getIsOmpLoop()) { // omp do
    const auto [NumToFill, OverlapStartingIVIndex] = getOverlapIVs(W, WParent);

    LLVM_DEBUG(dbgs() << "NumToFill: " << NumToFill << "\n");
    LLVM_DEBUG(dbgs() << "IVIndex: " << OverlapStartingIVIndex << "\n");

    if (NumToFill > 0) {

      LLVM_DEBUG(print(GenIVs, NumToFill, "GenIVs"));
      LLVM_DEBUG(print(GenLBs, NumToFill, "GenLBs"));
      LLVM_DEBUG(print(GenUBs, NumToFill, "GenUBs"));

      // Here the changes are for updating IR of the parent construct for
      // preparing next pass other than loop transformation pass. Only the
      // immediate parent WRN matters except for SIMD without norm IV.
      updateParentRegionEntry(GenIVs, GenLBs, GenUBs, GenIVTypes, NumToFill,
                              OverlapStartingIVIndex);
    }
  } else if (WParent->getIsOmpLoopTransform()) { // omp tile

    // getOverlapIVs WParent->getWRNLoopInfo().getNormIV(i)
    // may not work for tile+tile environment as new floor_loops are added
    // in-between the existing tile (before +) and tile (after +).
    // Thus, stick to the current working logic, which uses LoopInfo.
    auto ParentWLoopInfo = WParent->getWRNLoopInfo();
    int NumToFill =
        WParent->getOmpLoopDepth() - ParentWLoopInfo.getNormIVSize();

    LLVM_DEBUG(dbgs() << "-NumToFill: " << NumToFill << "\n");

    if (NumToFill > 0) {

      LLVM_DEBUG(print(GenIVs, NumToFill, "GenIVs"));
      LLVM_DEBUG(print(GenLBs, NumToFill, "GenLBs"));
      LLVM_DEBUG(print(GenUBs, NumToFill, "GenUBs"));

      // Here the changes are done one the LoopInfo data structure, and
      // preparing for the next iteration of loop transformation pass.
      // e.g. tile2+tile1.ll

      // !$omp do collapse(2) -- i_reverse, Floor_j
      // !$omp tile sizes(4,2) -- i_reverse, Floor_j -- new format
      // !$omp loop reverse -- i_reverse
      // do i = 1, 100
      //  !$omp tile sizes(8) <-- current tile
      //  do j = 1, 48
      //    call bar(i,j)
      //  end do
      // end do
      // It will probably only matters the immediate parent, as the
      // parent of the parent can be handled when the parent is
      // loop-transformed.
      updateParentRegionLoopInfo(GenIVs, GenUBs, GenIVTypes, NumToFill,
                                 FloorLoopPreHeaders, FloorLoopHeaders,
                                 FloorLoopLatches);
    }
  } else {
    llvm_unreachable("omp tile: parent region should be either do or tile");
  }
}

static BasicBlock *getInsertionBBAfterStore(Value *Alloca) {
  StoreInst *StoreI = nullptr;
  for (User *U : Alloca->users()) {
    if (Instruction *I = dyn_cast<Instruction>(U)) {
      if (StoreInst *SI = dyn_cast<StoreInst>(I)) {
        assert(!StoreI && "Only one store inst should exists.");
        StoreI = SI;
      }
    }
  }

  assert(StoreI && (StoreI->getPointerOperand() == Alloca) &&
         "Operand mis-matching.");
  return StoreI->getParent();
}

// Find a bundle in WRN's entry CI, containing \p ToFindVar, and make a copy
// of that bundle but replacing  \p ToFindVar to the element of \p ToAddVars.
static bool copyBundles(WRegionNode *ToUpdateWRN, Value *ToFindVar,
                        ArrayRef<Value *> ToAddVars,
                        SmallVectorImpl<OperandBundleDef> &NewBundles) {

  CallInst *EntryCI = cast<CallInst>(ToUpdateWRN->getEntryDirective());
  SmallVector<OperandBundleDef, 16> OrigBundles;
  EntryCI->getOperandBundlesAsDefs(OrigBundles);

  bool FoundBundle = false;
  for (auto &Bundle : OrigBundles) {
    StringRef ClauseString = Bundle.getTag();

    if (!VPOAnalysisUtils::isOpenMPClause(ClauseString))
      continue;

    // For UB, LB, IVs in the outer regions, the first var
    // should be the matching var.
    if (Bundle.inputs().front() != ToFindVar)
      continue;

    FoundBundle = true;

    // Copy this Bundle for each \p ToAddVars
    for (auto *ToAddVar : ToAddVars) {
      SmallVector<Value *, 16> Vals;
      Vals.push_back(ToAddVar);
      if (Bundle.inputs().size() > 1)
        Vals.append(Bundle.inputs().begin() + 1, Bundle.inputs().end());
      NewBundles.emplace_back(ClauseString.str(), Vals);
    }

    break;
  }

  return FoundBundle;
}

// AllocaBB --> StoreBB(or init BB) --> WRN_1 --> WRN_2-->.. WRN_m-1 --> WRN_m
// --> .. --> WRN_tile. WRN_m is the outermost wrn associated with the same loop
// with WRN_tile. In ohterwords, WRN_m and WRN_tile have overlapping IVs. WRN_m
// is called outermost WRN. For all interim WRNs from SToreBB to WRN_m
// (exclusively), [WRN_1, WRN_m-1], generated vars related to tiling have to be
// replicated in the clauses. Original Vars from which tile vars are induced are
// needed to find the type of Clause.
void WRegionNodeTiler::updateOuterRegions(
    DominatorTree *DT, BasicBlock *InitPtsBB, WRegionNode *ParOutermostWRN,
    Value *GenIV, Value *GenLB, Value *GenUB, Value *OrigIV, Value *OrigLB,
    Value *OrigUB, Value *ClonedOrigUB) {
  // Add GenLB, GenUB, ClonedOrigUB to all enclosing wrn from ParOutermostWRN
  // to upward until the WRN is strictly dominated by InitPtsBB.
  // If we knew the WRNNode where InitPtsBB belongs to, we could just use
  // WRN's parent chain using getParent().

  BasicBlock *WOutsideEntryBlock = ParOutermostWRN->getEntryBBlock();
  while (WOutsideEntryBlock && InitPtsBB != WOutsideEntryBlock &&
         DT->dominates(InitPtsBB, WOutsideEntryBlock)) {

    LLVM_DEBUG(dbgs() << "Updating one outer region:\n";
               if (WOutsideEntryBlock->hasName()) dbgs()
               << WOutsideEntryBlock->getName() << "\n";
               else ParOutermostWRN->getEntryDirective()->dump(););

    // OrigUB should be found from WOutsideEntryBlock's clauses
    SmallVector<OperandBundleDef, 4> NewBundles;

    LLVM_DEBUG(dbgs() << "OrigUB: "; OrigUB->dump(); dbgs() << "\n";);

    bool Copied =
        copyBundles(ParOutermostWRN, OrigUB, {GenUB, ClonedOrigUB}, NewBundles);
    assert(Copied && "FloorUB should have been added to enclosing WRN.");
    Copied = copyBundles(ParOutermostWRN, OrigLB, {GenLB}, NewBundles);
    assert(Copied && "FloorLB should have been added to enclosing WRN.");

    LLVM_DEBUG(dbgs() << "origIV -- " << *OrigIV << "\n");
    Copied = copyBundles(ParOutermostWRN, OrigIV, {GenIV}, NewBundles);
    assert(Copied && "FloorIV should have been added to enclosing WRN.");
    (void)Copied;

    // Actual addition to CI
    CallInst *EntryCI = cast<CallInst>(ParOutermostWRN->getEntryDirective());
    SmallVector<OperandBundleDef, 16> Bundles;
    EntryCI->getOperandBundlesAsDefs(Bundles);
    Bundles.append(NewBundles);
    replaceBundles(EntryCI, Bundles);

    ParOutermostWRN = ParOutermostWRN->getParent();
    if (!ParOutermostWRN)
      break;
    WOutsideEntryBlock = ParOutermostWRN->getEntryBBlock();

    LLVM_DEBUG(dbgs() << "Updating one outer region done\n";);
  }
}

static Value *getOrigLB(WRegionNode *W, unsigned LoopDepthIndex) {
  return VPOParoptUtils::getLoadFromLB(W, LoopDepthIndex)->getPointerOperand();
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
  //  This is under the assumption that all input normalized IVs associated
  //  with one TILE WRN are allocated in the same basic block.
  auto *AllocaPos =
      cast<Instruction>(OutermostIV)->getParent()->getTerminator();

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

  // Record OutermostWRN before starting transformation
  WRegionNode *OutermostWRN = findOutermostWRNLoopNode(W);
  WRegionNode *ParOutermostWRN = OutermostWRN->getParent();

  LLVM_DEBUG(if (ParOutermostWRN) {
    dbgs() << "OutermostWRN exists: \n";
    auto *ED = OutermostWRN->getEntryDirective();
    ED->dump();
    dbgs() << "ParOutermostWRN exists: \n";
    ED = ParOutermostWRN->getEntryDirective();
    ED->dump();
  });

  SmallVector<Value *, 4> OrigLBs(NumTileLoops, nullptr);
  SmallVector<Value *, 4> OrigUBs(NumTileLoops, nullptr);
  SmallVector<Value *, 4> OrigIVs(NumTileLoops, nullptr);
  SmallVector<Instruction *, 4> InitPts(NumTileLoops, nullptr);
  SmallVector<bool, 4> AddToEnclosingWRNs(NumTileLoops, false);
  auto *DT = W->getDT();
  // Populate information for OutermostWRN before transformation
  // Loops are proccessed from the innermost to outermost direction.
  for (int LoopDepthIndex = NumTileLoops - 1; LoopDepthIndex >= 0;
       LoopDepthIndex--) {
    OrigUBs[LoopDepthIndex] = WLoopInfo.getNormUB(LoopDepthIndex);
    auto *InitPos = getInsertionBBAfterStore(OrigUBs.back())->getTerminator();
    InitPts[LoopDepthIndex] = InitPos;

    // The following util can be called before any loop tiling transformation
    // happens. Once the transformation starts, CFG changes, and the basic block
    // the util was assuming will be no longer valid.
    OrigLBs[LoopDepthIndex] = getOrigLB(W, LoopDepthIndex);
    LLVM_DEBUG(dbgs() << "Found origLB: " << *OrigLBs[LoopDepthIndex] << "\n");

    OrigIVs[LoopDepthIndex] = WLoopInfo.getNormIV(LoopDepthIndex);
    LLVM_DEBUG(dbgs() << "Found origIV: " << *OrigIVs[LoopDepthIndex] << "\n");

    // See if InitPos is dominationg OutermostWRN's Parent WRN
    if (ParOutermostWRN &&
        InitPos->getParent() != ParOutermostWRN->getEntryBBlock() &&
        DT->dominates(InitPos->getParent(), ParOutermostWRN->getEntryBBlock()))
      AddToEnclosingWRNs[LoopDepthIndex] = true;
  }

  bool IsSPIRVTarget = VPOAnalysisUtils::isTargetSPIRV(
      W->getEntryBBlock()->getParent()->getParent());

  // Notice that the smaller LoopDepthIndex is the outer the loop is.
  // In other words, the innermost loop has the largest LoopDepthIndex,
  // and the outermost loop has the smallest LoopDepthIndex.
  // CollectAllNormIVs and keep the outermost NumNormIVsToFill
  // for the paretn WRegionNode
  // Data Structure for Parent WRegionNode
  SmallVector<Value *, 4> GenIVs(NumTileLoops, nullptr);
  SmallVector<Value *, 4> GenLBs(NumTileLoops, nullptr);
  SmallVector<Value *, 4> GenUBs(NumTileLoops, nullptr);
  SmallVector<Value *, 4> ClonedOrigUBs(NumTileLoops, nullptr);
  SmallVector<Type *, 4> GenIVTypes(NumTileLoops, nullptr);
  SmallVector<BasicBlock *, 4> FloorLoopPreHeaders;
  SmallVector<BasicBlock *, 4> FloorLoopHeaders;
  SmallVector<BasicBlock *, 4> FloorLoopLatches;

  // Loops are proccessed from the innermost to outermost direction.
  for (int LoopDepthIndex = NumTileLoops - 1; LoopDepthIndex >= 0;
       LoopDepthIndex--) {

    LLVM_DEBUG(dbgs() << "LoopDepthIndex processed: " << LoopDepthIndex
                      << "\n");

    Value *TileSize = TileSizes[LoopDepthIndex];

    LLVM_DEBUG(dbgs() << "Tilesize processed: "; TileSize->dump());

    Stripminer SM(LoopDepthIndex, TileSize, W, IsSPIRVTarget);

    // Gen and add Floor-loop vars
    SM.addFloorLoopIVBounds(AllocaPos, InitPts[LoopDepthIndex]);
    assert(SM.FloorIV && SM.FloorUB && SM.FloorLB && SM.ClonedOrigUpperBound &&
           "Tile floor vars are not generated.");

    GenIVs[LoopDepthIndex] = SM.FloorIV;
    GenLBs[LoopDepthIndex] = SM.FloorLB;
    GenUBs[LoopDepthIndex] = SM.FloorUB;
    ClonedOrigUBs[LoopDepthIndex] = SM.ClonedOrigUpperBound;
    // TODO: Make sure if it is alright to use IndVarTy
    GenIVTypes[LoopDepthIndex] = SM.IndVarTy;

    LLVM_DEBUG(
        dbgs()
        << "TileBounds BB: "
        << WLoopInfo.getLoop(LoopDepthIndex)->getLoopPreheader()->getName()
        << "\n");

    auto TileBoundsPos =
        WLoopInfo.getLoop(LoopDepthIndex)->getLoopPreheader()->getTerminator();

    // Add tile loop vars
    SM.addTileLoopBounds(TileBoundsPos);

    BasicBlock *Pred = OutermostPreheader->getUniquePredecessor();
    assert(Pred && "No predecessor block of preheader");

    // Add floor loop around tile loop
    std::tie(OutermostPreheader, OutermostHeader, OutermostLatch) =
        SM.addFloorLoop(Pred, OutermostPreheader, OutermostHeader);

    // By pushing at the back, innermost loop info added first.
    // Easier to process later.
    FloorLoopPreHeaders.push_back(OutermostPreheader);
    FloorLoopHeaders.push_back(OutermostHeader);
    FloorLoopLatches.push_back(OutermostLatch);
  }

  // This run() function is for one WRNNode.
  // Insertion of {FloorUB, FloorUB, ClonedOrigUB} can be done after all
  // TILE(size1, size2...) are done here outside the for-loop

  auto *WParent = W->getParent();
  if (WParent && WParent->getIsOmpLoopOrLoopTransform())
    updateParentRegion(GenIVs, GenLBs, GenUBs, GenIVTypes, FloorLoopPreHeaders,
                       FloorLoopHeaders, FloorLoopLatches);

  if (!ParOutermostWRN) {
    return;
  }

  for (int LoopDepthIndex = NumTileLoops - 1; LoopDepthIndex >= 0;
       LoopDepthIndex--) {
    if (!AddToEnclosingWRNs[LoopDepthIndex])
      continue;

    updateOuterRegions(DT, InitPts[LoopDepthIndex]->getParent(),
                       ParOutermostWRN, GenIVs[LoopDepthIndex],
                       GenLBs[LoopDepthIndex], GenUBs[LoopDepthIndex],
                       OrigIVs[LoopDepthIndex], OrigLBs[LoopDepthIndex],
                       OrigUBs[LoopDepthIndex], ClonedOrigUBs[LoopDepthIndex]);
  }
}

static Value *allocaVar(Instruction *InsertPt, bool IsSpirV, Type *Ty,
                        LLVMContext &Context, StringRef VarName) {

  IRBuilder<> Builder(InsertPt);

  if (!IsSpirV)
    return Builder.CreateAlloca(Ty, nullptr, VarName);

  Value *TempVal =
      Builder.CreateAlloca(Ty, nullptr, VarName.str() + Twine(".tmp"));
  Value *Val = Builder.CreateAddrSpaceCast(
      TempVal, PointerType::get(Context, vpo::ADDRESS_SPACE_GENERIC), VarName);
  return Val;
}

void Stripminer::addFloorLoopIVBounds(Instruction *AllocaInsertPos,
                                      Instruction *InitInsertPos) {

  auto &WLoopInfo = W->getWRNLoopInfo();

  LLVMContext &Context = AllocaInsertPos->getContext();

  FloorLB =
      allocaVar(AllocaInsertPos, IsSPIRVTarget, IndVarTy, Context, "floor_lb");

  IRBuilder<> InitializerBuilder(InitInsertPos);

  auto *Zero =
      InitializerBuilder.getIntN(cast<IntegerType>(IndVarTy)->getBitWidth(), 0);

  LLVM_DEBUG(dbgs() << "Zero: "; Zero->dump());
  LLVM_DEBUG(dbgs() << "FloorLB: "; FloorLB->dump());

  InitializerBuilder.CreateStore(Zero, FloorLB);

  FloorUB =
      allocaVar(AllocaInsertPos, IsSPIRVTarget, IndVarTy, Context, "floor_ub");

  auto *OrigUB = WLoopInfo.getNormUB(LoopDepthIndex);
  auto *OrigUBLoad =
      InitializerBuilder.CreateLoad(IndVarTy, OrigUB, "norm.orig.ub.val");

  // floorUB  =  OrigUBLoad / TileSize
  // Notice possibility of type mismatch
  // e.g.) OrigUBLoad - 64bit, TileSize - 32 bit.

  TileSize = InitializerBuilder.CreateSExtOrTrunc(TileSize, IndVarTy, ".sext");
  auto *NormFloorUB =
      InitializerBuilder.CreateSDiv(OrigUBLoad, TileSize, "norm.floor.ub.val");

  LLVM_DEBUG(dbgs() << "NormFloorUB: "; NormFloorUB->dump());
  LLVM_DEBUG(dbgs() << "FloorUB: "; FloorUB->dump());

  InitializerBuilder.CreateStore(NormFloorUB, FloorUB);

  FloorIV =
      allocaVar(AllocaInsertPos, IsSPIRVTarget, IndVarTy, Context, "floor_iv");

  // This storage of OrigUB is needed because OrigUB value won't be
  // preserved but reused as new upper bound of the tiled loop.
  ClonedOrigUpperBound = allocaVar(AllocaInsertPos, IsSPIRVTarget, IndVarTy,
                                   Context, "cloned_orig_ub");
  InitializerBuilder.CreateStore(OrigUBLoad, ClonedOrigUpperBound);
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
      Builder.CreateLoad(IndVarTy, ClonedOrigUpperBound, "norm.orig.ub.val");
  auto *Add = Builder.CreateAdd(TileLB, TileSize, "add");
  auto *Dec = Builder.CreateSub(
      Add, Builder.getIntN(cast<IntegerType>(IndVarTy)->getBitWidth(), 1),
      "dec");

  // TODO: figure out if the original loop's IV is signed or not
  // TODO: This is min(a,b)
  const bool IsSigned = true;
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

  Builder.SetInsertPoint(FloorPreHeader->getTerminator());
  // vpo-paropt-loop-collapse pass expects the first operand of this store
  // a load instruction.
  //     $floor.lb.val = load %floor_lb
  //     store %floor.lb.val to %floor_iv
  // We could just directly store 0 to %floor_iv since the loop is already
  // normalized. However, collapse pass expects a load instead of const 0.
  auto *FloorLBVal = Builder.CreateLoad(IndVarTy, FloorLB, "floor.lb");
  Builder.CreateStore(FloorLBVal, FloorIV);

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

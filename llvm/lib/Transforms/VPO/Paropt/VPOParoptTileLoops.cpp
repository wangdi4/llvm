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

typedef SmallVector<Value *, 4> BundleOprsTy;
typedef SmallVectorImpl<Value *> BundleOprsImplTy;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void print(const SmallVectorImpl<Value *> &IVsToFill, int Size,
           StringRef Header) {
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

void updateParentRegionLoopInfo(WRegionNode *WParent,
                                const BundleOprsImplTy &GenIVs,
                                const BundleOprsImplTy &GenUBs, int NumToFill) {

  auto &WLoopInfo = WParent->getWRNLoopInfo();

  for (int I = 0; I < NumToFill; I++) {
    auto *V = cast<AllocaInst>(GenIVs[I]);
    auto *VTy = V->getAllocatedType();
    WLoopInfo.addNormIV(V, VTy);
  }

  for (int I = 0; I < NumToFill; I++) {
    auto *V = cast<AllocaInst>(GenUBs[I]);
    auto *VTy = V->getAllocatedType();
    WLoopInfo.addNormUB(V, VTy);
  }

  // No need to worry for GenLBs

  // TODO: Loop and its preheader/header/latch
  // information should be updated as well.
  // Needs direct update on LLVM's Loop and LoopInfo
}

// Copy existing type bundles or make it typed if they were not
// originally typed.
void getTypedIVUBBundles(bool IsTyped, const OperandBundleDef &Bundle,
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
void updateParentRegionEntry(WRegionNode *WParent,
                             const BundleOprsImplTy &GenIVs,
                             const BundleOprsImplTy &GenLBs,
                             const BundleOprsImplTy &GenUBs, int NumToFill) {

  assert(NumToFill > 0 && "No information to add");

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
  for (Value *GenIV : GenIVs) {
    IVBundleVals.push_back(GenIV);
    Value *TypeV =
        Constant::getNullValue(cast<AllocaInst>(GenIV)->getAllocatedType());
    IVBundleVals.push_back(TypeV);
  }
  NewBundles.emplace_back(
      VPOAnalysisUtils::getTypedClauseString(QUAL_OMP_NORMALIZED_IV),
      IVBundleVals);

  for (Value *GenUB : GenUBs) {
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
  for (int I = 0; I < NumToFill; I++) {
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

void updateParentRegion(WRegionNode *WParent, BundleOprsTy &GenIVs,
                        BundleOprsTy &GenLBs, BundleOprsTy &GenUBs) {

  int NumToFill = 0;
  auto ParentWLoopInfo = WParent->getWRNLoopInfo();
  if (WParent->getIsOmpLoopOrLoopTransform())
    NumToFill = WParent->getOmpLoopDepth() - ParentWLoopInfo.getNormIVSize();
  assert(NumToFill >= 0 && "Invalid status");

  LLVM_DEBUG(dbgs() << "NumToFill: " << NumToFill << "\n");

  if (NumToFill == 0)
    return;

  assert(GenIVs.size() >= (unsigned)NumToFill &&
         "Not enough IVs are generated");

  std::reverse(GenIVs.begin(), GenIVs.end());
  std::reverse(GenLBs.begin(), GenLBs.end());
  std::reverse(GenUBs.begin(), GenUBs.end());

  LLVM_DEBUG(print(GenIVs, NumToFill, "GenIVs"));
  LLVM_DEBUG(print(GenLBs, NumToFill, "GenLBs"));
  LLVM_DEBUG(print(GenUBs, NumToFill, "GenUBs"));

  if (WParent->getIsOmpLoop()) // omp do
    updateParentRegionEntry(WParent, GenIVs, GenLBs, GenUBs, NumToFill);
  else if (WParent->getIsOmpLoopTransform()) // omp tile
    updateParentRegionLoopInfo(WParent, GenIVs, GenUBs, NumToFill);
  else
    llvm_unreachable("omp tile: parent region should be either do or tile");
}

} // end of namespace

namespace {
class Stripminer {
public:
  Stripminer(WRegionNode *W, unsigned LoopDepthIndex, Value *TileSize)
      : W(W), LoopDepthIndex(LoopDepthIndex), TileSize(TileSize),
        IndVarTy(W->getWRNLoopInfo().getNormIVElemTy(LoopDepthIndex)),
        FloorLB(nullptr), FloorUB(nullptr), FloorIV(nullptr),
        OrigUpperBound(nullptr) {
    assert(IndVarTy == W->getWRNLoopInfo().getNormUBElemTy(LoopDepthIndex) &&
           "IV and UB types are should be the same");
  }

  static std::tuple<BundleOprsTy, BundleOprsTy, BundleOprsTy, BasicBlock *,
                    BasicBlock *, BasicBlock *>
  driver(WRegionNode *W);

  std::tuple<Value *, Value *, Value *>
  addFloorLoopIVBounds(Instruction *InsertPos);

  // Update original loop preheader by adding
  // computation of tile loops's lower and upper bounds (tile.lb, tile.ub).
  // Store for tile.lb to original.norm.iv &
  // sore of tile.ub to original.norm.ub
  // also added.
  // Original norm.iv and norm.ub are reused as tile.iv, tile.ub
  // \p InsertPos is original loop preheader
  void addTileLoopBounds(Instruction *InsertPos);

  // Add a floor loop around the given (tile) loop.
  // As a result, CFG is changed.
  std::tuple<BasicBlock *, BasicBlock *, BasicBlock *>
  addFloorLoop(BasicBlock *OutermostPreheader, BasicBlock *OutermostHeader);

private:
  WRegionNode *W;
  unsigned LoopDepthIndex;
  Value *TileSize;
  Type *IndVarTy;

  // outputs
  AllocaInst *FloorLB;
  AllocaInst *FloorUB;
  AllocaInst *FloorIV;

  // This is a copy of Orig UB. Needed because the orig ub is reused as tile ub.
  AllocaInst *OrigUpperBound;
};

} // end of namespace

// Take care of loops from innermost to outermost
std::tuple<BundleOprsTy, BundleOprsTy, BundleOprsTy, BasicBlock *, BasicBlock *,
           BasicBlock *>
Stripminer::driver(WRegionNode *W) {
  // Get the size of Sizes - how many loops are tiled in this WRegionNode
  unsigned NumTileLoops = W->getSizes().size();

  LLVM_DEBUG(dbgs() << "NumTileLoops: " << NumTileLoops << "\n");

  // Outermost Alloca Pos
  auto &WLoopInfo = W->getWRNLoopInfo();

  LLVM_DEBUG(printWRNLoopInfo(WLoopInfo));

  auto *OutermostIV = WLoopInfo.getNormIV(0);
  auto *AllocaPos = cast<AllocaInst>(OutermostIV)->getParent()->getTerminator();

  // Keep this information in case the LoopInfo is affected by in-place
  // transformation
  SmallVector<BasicBlock *, 4> OrigPreheaders;
  SmallVector<BasicBlock *, 4> OrigHeaders;
  for (unsigned LoopDepthIndex = 0; LoopDepthIndex < NumTileLoops;
       LoopDepthIndex++) {
    OrigPreheaders.push_back(
        WLoopInfo.getLoop(LoopDepthIndex)->getLoopPreheader());
    OrigHeaders.push_back(WLoopInfo.getLoop(LoopDepthIndex)->getHeader());
  }

  auto *OutermostPreheader = OrigPreheaders.front();
  auto *OutermostHeader = OrigHeaders.front();
  BasicBlock *OutermostLatch = nullptr;

  auto *EntryBlock = W->getEntryBBlock();
  // TODO: tile1_tile1.f90 dies here. LoopInfo is not updated correctly
  //       after the tileing of inner WRNRegion.
  // Check Entry block i-dominates OutermostPreheader;
  assert(OutermostPreheader->getUniquePredecessor() == EntryBlock &&
         "Missing structural correctness");
  (void)EntryBlock;

  // TODO: See if there is accessor with direct indexing to Clause<>
  SmallVector<Value *, 4> TileSizes;
  for (auto *TileSize : W->getSizes().items())
    TileSizes.push_back(TileSize->getOrig());

  LLVM_DEBUG(dbgs() << "TileSizes.size: " << TileSizes.size() << "\n");
  assert(NumTileLoops == TileSizes.size() && "TileSize mismatch");

  // Loops are proeccessed from the innermost to outermost direction.
  // Notice that the smaller LoopDepthIndex is the outer loop is.
  // CollectAllNormIVs and keep the outermost NumNormIVsToFill
  // for the paretn WRegionNode
  // Data Structure for Parent WRegionNode
  SmallVector<Value *, 4> GenIVs;
  SmallVector<Value *, 4> GenLBs;
  SmallVector<Value *, 4> GenUBs;
  for (int LoopDepthIndex = NumTileLoops - 1; LoopDepthIndex >= 0;
       LoopDepthIndex--) {

    LLVM_DEBUG(dbgs() << "LoopDepthIndex processed: " << LoopDepthIndex
                      << "\n");

    Value *TileSize = TileSizes[LoopDepthIndex];

    LLVM_DEBUG(dbgs() << "Tilesize processed: "; TileSize->dump());

    Stripminer SM(W, LoopDepthIndex, TileSize);

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

    std::tie(OutermostPreheader, OutermostHeader, OutermostLatch) =
        SM.addFloorLoop(OutermostPreheader, OutermostHeader);
  }

  // Outermost preheader, header, latch
  return {GenIVs,          GenLBs,        GenUBs, OutermostPreheader,
          OutermostHeader, OutermostLatch};
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
Stripminer::addFloorLoop(BasicBlock *OutermostPreheader,
                         BasicBlock *OutermostHeader) {

  // TODO: Make them class member variable
  auto *EntryBlock = W->getEntryBBlock();
  auto *ExitBlock = W->getExitBBlock();

  // TODO: getSuccessor() is not a correct method.
  //       However, something similar might be useful.
  // assert(OrigHeader->getSuccessor(1) == ExitBlock ||
  //       OrigHeader->getSuccessor(0) == ExitBlock);

  // 1. Fill in Floor Header
  // flooriv = load
  // floorub = load
  // br flooriv <= floorub OutermostPreheader, ExitBlock
  BasicBlock *FloorHeader = SplitEdge(EntryBlock, OutermostPreheader, nullptr,
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

  LLVM_DEBUG(dbgs() << "FloorHeadBlock 2\n"; FloorHeader->dump();
             EntryBlock->dump(); ExitBlock->dump());

  // Add reset of floor_iv between FloorHeader & its predecessor
  // I.e.
  //     $floor.lb.val = load %floor_lb
  //     store %floor.lb.val to %floor_iv
  // However, we always normalize floor-loops. Thus, it can simply be
  //      store 0 to %floor_iv
  // First split edge from FloorHeader's pred to FloorHeader
  BasicBlock *FloorPreHeader = SplitEdge(EntryBlock, FloorHeader, nullptr,
                                         nullptr, nullptr, "FLOOR.PREHEAD");
  // Add store 0 to %floor_iv
  Builder.SetInsertPoint(FloorPreHeader->getTerminator());
  auto *Zero = Builder.getIntN(cast<IntegerType>(IndVarTy)->getBitWidth(), 0);
  Builder.CreateStore(Zero, FloorIV);

  // 2. Replace OutermostHeader's br to FloorLetch
  auto *LoopCondBr = cast<BranchInst>(OutermostHeader->getTerminator());
  unsigned ExitTargetId = LoopCondBr->getSuccessor(0) == ExitBlock ? 0 : 1;
  LoopCondBr->setSuccessor(ExitTargetId, FloorHeader);

  auto *FloorLatch = SplitEdge(OutermostHeader, FloorHeader, nullptr, nullptr,
                               nullptr, "FLOOR.LATCH");
  LLVM_DEBUG(dbgs() << "After splitting edge - floor latch\n";
             FloorHeader->dump(); EntryBlock->dump(); ExitBlock->dump());

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

  BundleOprsTy GenIVs;
  BundleOprsTy GenLBs;
  BundleOprsTy GenUBs;
  BasicBlock *OutermostLoopPreheader = nullptr;
  BasicBlock *OutermostLoopHeader = nullptr;
  BasicBlock *OutermostLoopLatch = nullptr;
  std::tie(GenIVs, GenLBs, GenUBs, OutermostLoopPreheader, OutermostLoopHeader,
           OutermostLoopLatch) = Stripminer::driver(W);

  auto *WParent = W->getParent();
  if (WParent)
    updateParentRegion(WParent, GenIVs, GenLBs, GenUBs);

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

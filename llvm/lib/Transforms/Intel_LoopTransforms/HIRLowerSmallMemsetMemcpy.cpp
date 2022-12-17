//===---------------- HIRLowerSmallMemsetMemcpy.cpp ----------------------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// ===--------------------------------------------------------------------===//
//
// The pass lowers a small sized (<=64) alloca memcpy/memset before
// hir-pre-vec-complete-unroll pass into a loop of store/load assignment. Not
// only will this help complete unroll's cost model, it will allow HIRLMM pass
// to optimize away more loads/stores after unroll.
//
// Ex.:
// Before:
// @llvm.memcpy.p0i8.p0i8.i64(&((i8*)(%dst)[0]),  &((i8*)(%src)[0]),  20,  0);
//
// After:
// DO i1 =0, 5
//   (%dst)[i1] = (%src)[i1]
// END DO
//
// TODO:
// 1) Support loating point type in memset.
// 2) Support non-constant memset.

#include "llvm/Transforms/Intel_LoopTransforms/HIRLowerSmallMemsetMemcpyPass.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#define OPT_SWITCH "hir-lower-small-memset-memcpy"
#define OPT_DESC "HIR Lower Small Memset/Memcpy Pass"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    DisableLowerSmallMemsetMemcpyPass("disable-" OPT_SWITCH, cl::init(false),
                                      cl::Hidden,
                                      cl::desc("Disable " OPT_DESC " pass"));

// The threshold is for size of the memop in bytes.
static cl::opt<int>
    SmallMemsetMemcpyThreshold("small-memset-memcpy-threshold", cl::init(64),
                               cl::Hidden,
                               cl::desc("Memset/memcpy size threshold."));

STATISTIC(NumLoweredMemsetMemcpy,
          "Number of memcpy/memset calls lowered to loops.");

class MemsetMemcpyCandidate {
public:
  const RegDDRef *DstOp = nullptr;
  const RegDDRef *SrcOrValOp = nullptr;
  bool IsMemset = false;
  int64_t TC = 0;
  Type *ElemTy = nullptr;
};

class MemsetMemcpyVisitor final : public HLNodeVisitorBase {
public:
  MemsetMemcpyVisitor(HIRFramework &HIRF) : HIRF(HIRF) {}

  bool visit(HLInst *Inst);
  void visit(HLNode *) {}
  void postVisit(HLNode *) {}

private:
  HIRFramework &HIRF;
  bool doAnalysis(const HLInst *Inst, MemsetMemcpyCandidate &MMC) const;
  void doTransform(HLInst *Inst, MemsetMemcpyCandidate &MMC);
  RegDDRef *createLoopMemref(const RegDDRef *Op, unsigned Level);
};

bool MemsetMemcpyVisitor::visit(HLInst *Inst) {

  MemsetMemcpyCandidate MMC;
  if (doAnalysis(Inst, MMC)) {
    doTransform(Inst, MMC);
    ++NumLoweredMemsetMemcpy;
    return true;
  }

  return false;
}

class HIRLowerSmallMemsetMemcpy {
public:
  HIRLowerSmallMemsetMemcpy(HIRFramework &HIRF) : HIRF(HIRF) {}
  bool run();

private:
  HIRFramework &HIRF;
};

static Type *findElementType(const RegDDRef *AddressOfRef) {
  Type *DimElemTy = AddressOfRef->getDimensionElementType(1);
  if (!DimElemTy) {
    return nullptr;
  }

  if (!DimElemTy->isSized()) {
    return nullptr;
  }

  if (DimElemTy->isArrayTy()) {
    return DimElemTy->getArrayElementType();
  }

  if (!DimElemTy->isStructTy()) {
    // If access is strided, check that stride is constant and match dimention
    // element type size.
    auto *DimStride = AddressOfRef->getDimensionStride(1);
    int64_t DimStrideConst;
    if (!DimStride->isIntConstant(&DimStrideConst) ||
        (DimStrideConst <= 0 ||
         ((uint64_t)DimStrideConst !=
          AddressOfRef->getDDRefUtils().getCanonExprUtils().getTypeSizeInBytes(
              DimElemTy)))) {
      return nullptr;
    }
    return DimElemTy;
  }

  // Last dimension element type is stucture. Check that it is a nest of
  // single-field structures and the innermost structure has single array field.
  while (DimElemTy->isStructTy()) {
    if (DimElemTy->getStructNumElements() != 1) {
      return nullptr;
    }
    DimElemTy = DimElemTy->getStructElementType(0);
  }

  // Innermost type should be array.
  if (DimElemTy->isArrayTy()) {
    return DimElemTy->getArrayElementType();
  }

  return nullptr;
}

bool MemsetMemcpyVisitor::doAnalysis(const HLInst *Inst,
                                     MemsetMemcpyCandidate &MMC) const {
  Intrinsic::ID IntrinID;
  if (!Inst->isIntrinCall(IntrinID)) {
    return false;
  }
  LLVM_DEBUG(dbgs() << "\nLower memset/memcpy: analyse ");
  LLVM_DEBUG(Inst->dump(););

  bool IsMemset = false;

  if (IntrinID == Intrinsic::memset) {
    IsMemset = true;
  } else if (IntrinID != Intrinsic::memcpy) {
    return false;
  }

  auto *DstOp = Inst->getOperandDDRef(0);
  auto *SrcOrValOp = Inst->getOperandDDRef(1);
  auto *SizeOp = Inst->getOperandDDRef(2);

  // Do not optimize memset/memcpy with unknown size.
  // Restrict current AccessSizeInBytes to 64 bytes for profitability purposes.
  int64_t AccessSizeInBytes = 0;
  if (!SizeOp->isIntConstant(&AccessSizeInBytes) ||
      (AccessSizeInBytes > SmallMemsetMemcpyThreshold)) {
    return false;
  }

  // The following restrictions applied for profitability purposes:
  // Dst should be either an alloca-based ref or a ref which accesses NoAlias
  // arg.
  if (!(DstOp->accessesAlloca() || DstOp->accessesNoAliasFunctionArgument())) {
    return false;
  }
  if (IsMemset) {
    if (!SrcOrValOp->isIntConstant()) {
      return false;
    }
  } else {
    // Src should be an alloca-based ref, constant array ref or a ref which
    // accesses NoAlias arg.
    if (!SrcOrValOp->accessesAlloca() && !SrcOrValOp->accessesConstantArray() &&
        !SrcOrValOp->accessesNoAliasFunctionArgument())
      return false;
  }

  Type *DstElemType = findElementType(DstOp);
  if (!DstElemType) {
    return false;
  }

  // We only handle integer memsets for now.
  if (IsMemset && !DstElemType->isIntegerTy()) {
    return false;
  }

  if (!IsMemset) {
    Type *SrcElemType = findElementType(SrcOrValOp);
    if (SrcElemType != DstElemType) {
      return false;
    }
  }

  auto *ElemTy = DstElemType;
  int64_t ElemSize = HIRF.getCanonExprUtils().getTypeSizeInBytes(ElemTy);

  // Do not proceed if element type does not divide memset/memcpy size evenly.
  if (AccessSizeInBytes % ElemSize) {
    return false;
  }

  // Calculate a trip count of a new loop.
  unsigned TC = AccessSizeInBytes / ElemSize;

  LLVM_DEBUG(dbgs() << "\nLower memset/memcpy: "
                    << (IsMemset ? "memset" : "memcpy") << "() is found: \n");
  LLVM_DEBUG(Inst->dump(););

  MMC.DstOp = DstOp;
  MMC.SrcOrValOp = SrcOrValOp;
  MMC.IsMemset = IsMemset;
  MMC.TC = TC;
  MMC.ElemTy = ElemTy;

  return true;
}

RegDDRef *
MemsetMemcpyVisitor::createLoopMemref(const RegDDRef *MemIntrinAddressOfOp,
                                      unsigned Level) {
  CanonExprUtils &CEU = HIRF.getCanonExprUtils();

  RegDDRef *MemRefOp = MemIntrinAddressOfOp->clone();
  Type *IVTy = HIRF.getDataLayout().getIndexType(MemRefOp->getBaseType());
  CanonExpr *IVCanonExpr = CEU.createCanonExpr(IVTy);
  IVCanonExpr->setIVCoeff(Level, 0, 1);

  auto *DimElemType = MemRefOp->getDimensionElementType(1);
  SmallVector<unsigned, 8> Offsets;
  while (DimElemType->isStructTy()) {
    DimElemType = DimElemType->getStructElementType(0);
    Offsets.push_back(0);
  }

  if (!Offsets.empty()) {
    // If reference element type is structure, add last subscript togather with
    // approprite number of zero offsets:
    //    &((%y)[0]) -> (%y)[0].0.0
    // where %y is a pointer to the type struct2 = { struct1 }
    //                              type struct1 = { [i8 * 20] }
    MemRefOp->setTrailingStructOffsets(1, Offsets);
    // Finally add last subscript with new IV.
    // (%y)[0].0.0.0 -> (%y)[0].0.0.0[i3]
    MemRefOp->addDimension(IVCanonExpr);
  } else if (DimElemType->isArrayTy()) {
    // The MemIntrinAddressOfOp is an address of array. Add new dimention.
    MemRefOp->addDimension(IVCanonExpr);
  } else {
    // Otherwise, it is a multi-dimensional array, last subscript should be
    // replaced with new IV:
    // &((%y)[i1][i2][0]) -> (%y)[i1][i2][i3]
    CanonExpr *CE = MemRefOp->getDimensionIndex(1);
    CE->setIVCoeff(Level, 0, 1);
  }

  MemRefOp->setAddressOf(false);
  MemRefOp->setBitCastDestVecOrElemType(nullptr);

  return MemRefOp;
}

void MemsetMemcpyVisitor::doTransform(HLInst *Inst,
                                      MemsetMemcpyCandidate &MMC) {
  // The Operand represents an 'address of' ref. Create a new ref with the
  // same base.
  LLVM_DEBUG(dbgs() << "\nLower memset/memcpy: transform "
                    << (MMC.IsMemset ? "memset" : "memcpy") << "()\n");
  DDRefUtils &DDRU = HIRF.getDDRefUtils();
  Type *IVTy = HIRF.getDataLayout().getIndexType(MMC.DstOp->getBaseType());

  // Create a new loop instead of intrinsic call.
  HLNodeUtils &HLNU = HIRF.getHLNodeUtils();
  HLLoop *NewLoop = HLNU.createHLLoop(nullptr, DDRU.createConstDDRef(IVTy, 0),
                                      DDRU.createConstDDRef(IVTy, MMC.TC - 1),
                                      DDRU.createConstDDRef(IVTy, 1));
  // Replace intrinsic call with the new loop.
  HLNodeUtils::insertBefore(Inst, NewLoop);

  // Create LHS DD ref based on corresponding operand DD ref of the original
  // call.
  unsigned NewLoopLevel = NewLoop->getNestingLevel();
  RegDDRef *DstOp = createLoopMemref(MMC.DstOp, NewLoopLevel);

  LLVM_DEBUG(dbgs() << "\t LHS: ");
  LLVM_DEBUG(DstOp->dump(););

  // Create RHS DD ref based on corresponding operand DD ref of the original
  // call.
  RegDDRef *SrcOrValOp = nullptr;
  if (MMC.IsMemset) {
    int64_t SrcOrValOpConst;
    MMC.SrcOrValOp->isIntConstant(&SrcOrValOpConst);
    APInt APVal(8, SrcOrValOpConst);
    auto WideVal = APInt::getSplat(MMC.ElemTy->getIntegerBitWidth(), APVal);
    SrcOrValOp = DDRU.createConstDDRef(MMC.ElemTy, WideVal.getSExtValue());
  } else {
    SrcOrValOp = createLoopMemref(MMC.SrcOrValOp, NewLoopLevel);
  }
  LLVM_DEBUG(dbgs() << "\t RHS: ");
  LLVM_DEBUG(SrcOrValOp->dump(););
  LLVM_DEBUG(dbgs() << "\n");

  // Create store instruction.
  HLInst *const StoreInst = HLNU.createStore(SrcOrValOp, "", DstOp);
  HLNodeUtils::insertAsFirstChild(NewLoop, StoreInst);

  NewLoop->addLiveInTemp(DstOp);
  if (!MMC.IsMemset) {
    NewLoop->addLiveInTemp(SrcOrValOp);
  }

  SrcOrValOp->makeConsistent(MMC.SrcOrValOp);
  DstOp->makeConsistent(MMC.DstOp);

  // Remove original memop intrinsic.
  HLNodeUtils::remove(Inst);

  // Mark region as modified.
  NewLoop->getParentRegion()->setGenCode();

  // Region should be invalidated.
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(NewLoop);
  return;
}

bool HIRLowerSmallMemsetMemcpy::run() {

  MemsetMemcpyVisitor MMV(HIRF);

  for (auto &RegIt : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    HLRegion &Reg = cast<HLRegion>(RegIt);
    HLNodeUtils::visitRange(MMV, Reg.child_begin(), Reg.child_end());
  }

  return NumLoweredMemsetMemcpy;
}

PreservedAnalyses
HIRLowerSmallMemsetMemcpyPass::runImpl(Function &F, FunctionAnalysisManager &AM,
                                       HIRFramework &HIRF) {
  if (DisableLowerSmallMemsetMemcpyPass) {
    return PreservedAnalyses::all();
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : " << F.getName() << "\n");
  HIRLowerSmallMemsetMemcpy(HIRF).run();

  return PreservedAnalyses::all();
}

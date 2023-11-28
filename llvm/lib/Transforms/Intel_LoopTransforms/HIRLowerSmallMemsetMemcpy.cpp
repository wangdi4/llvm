//===---------------- HIRLowerSmallMemsetMemcpy.cpp ----------------------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
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
// 1) Currently we only create single level loop. To avoid out-of-range access
//    we could consider creating a loop nest.

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
    SmallMemsetMemcpyThreshold("small-memset-memcpy-threshold", cl::init(800),
                               cl::Hidden,
                               cl::desc("Memset/memcpy size threshold."));

STATISTIC(NumLoweredMemsetMemcpy,
          "Number of memcpy/memset calls lowered to loops.");

class MemsetMemcpyCandidate {
public:
  const RegDDRef *DstOp = nullptr;
  RegDDRef *SrcOrValOp = nullptr;
  bool IsMemset = false;
  int64_t TC = 0;
  Type *ElemTy = nullptr;
};

class MemsetMemcpyVisitor final : public HLNodeVisitorBase {
public:
  MemsetMemcpyVisitor(HIRFramework &HIRF) : HIRF(HIRF) {}

  void visit(HLInst *Inst);
  void visit(HLNode *) {}
  void postVisit(HLNode *) {}

private:
  HIRFramework &HIRF;
  bool doAnalysis(const HLInst *Inst, MemsetMemcpyCandidate &MMC) const;
  void doTransform(HLInst *Inst, MemsetMemcpyCandidate &MMC);
  RegDDRef *createLoopMemref(const RegDDRef *Op, unsigned Level);
};

void MemsetMemcpyVisitor::visit(HLInst *Inst) {

  MemsetMemcpyCandidate MMC;
  if (doAnalysis(Inst, MMC)) {
    doTransform(Inst, MMC);
    ++NumLoweredMemsetMemcpy;
  }
}

class HIRLowerSmallMemsetMemcpy {
public:
  HIRLowerSmallMemsetMemcpy(HIRFramework &HIRF) : HIRF(HIRF) {}
  bool run();

private:
  HIRFramework &HIRF;
};

// Return element type of the AddressOf ref in memop.
static Type *findElementType(const RegDDRef *AddressOfRef,
                             unsigned &InnermostDimSize,
                             const CanonExpr *&InnermostDimIndex) {
  Type *DimElemTy = AddressOfRef->getDimensionElementType(1);
  if (!DimElemTy) {
    return nullptr;
  }

  if (!DimElemTy->isSized()) {
    return nullptr;
  }

  if (DimElemTy->isArrayTy()) {
    // Set number of elements if it is known.
    InnermostDimSize = DimElemTy->getArrayNumElements();
    InnermostDimIndex = nullptr;
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

    // Set number of elements if it is known.
    InnermostDimSize = AddressOfRef->getNumDimensionElements(1);
    InnermostDimIndex = AddressOfRef->getDimensionIndex(1);

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
    // Set number of elements if it is known.
    InnermostDimSize = DimElemTy->getArrayNumElements();
    InnermostDimIndex = nullptr;
    return DimElemTy->getArrayElementType();
  }

  return nullptr;
}

// Returns true if we can create an out-of-range array access.
static bool isOutOfRangeAccess(int64_t TC, int64_t MemOpInnermostDimSize,
                               const CanonExpr *InnermostDimIndex) {
  // Nullptr InnermostDimIndex means canon expr 0.
  if (!InnermostDimIndex) {
    if (TC > MemOpInnermostDimSize) {
      return true;
    }
    return false;
  }

  int64_t CEConst = 0;
  // Bail out for non-constant subcripts for now.
  if (!InnermostDimIndex->isIntConstant(&CEConst)) {
    return true;
  }
  if ((CEConst + TC) > MemOpInnermostDimSize) {
    return true;
  }
  return false;
}

// Function creates a constant DDRef of the type \pDstElemType from i8 constant
// DDRef \pSrcOrValOp.
RegDDRef *createConstRef(const RegDDRef *SrcOrValOp, Type *DstElemType) {
  RegDDRef *ConstRef = nullptr;
  DDRefUtils &DDRU = SrcOrValOp->getDDRefUtils();
  int64_t SrcOrValOpConst;
  (void)SrcOrValOp->isIntConstant(&SrcOrValOpConst);

  // Process pointer type first. We allow only memset with 0 for pointers.
  if (DstElemType->isPointerTy()) {
    if (SrcOrValOpConst == 0) {
      ConstRef = DDRU.createNullDDRef(DstElemType);
      return ConstRef;
    }
    return nullptr;
  }

  APInt APVal(8, SrcOrValOpConst);
  auto BitWidth = DstElemType->getPrimitiveSizeInBits();
  if (!BitWidth) {
    return nullptr;
  }

  auto WideVal = APInt::getSplat(BitWidth, APVal);
  if (DstElemType->isIntegerTy()) {
    ConstRef = DDRU.createConstDDRef(DstElemType, WideVal.getSExtValue());
    return ConstRef;
  }

  if (DstElemType->isFloatingPointTy()) {
    const fltSemantics &Semantics = DstElemType->getFltSemantics();
    APFloat APFloatVal(Semantics, 0);
    unsigned Res =
        APFloatVal.convertFromAPInt(WideVal, true, APFloat::rmTowardZero);
    if (Res != APFloat::opOK) {
      return nullptr;
    }
    ConstRef = DDRU.createConstDDRef(ConstantFP::get(DstElemType, APFloatVal));
    return ConstRef;
  }

  return nullptr;
}

// The main analysis function of the pass.
bool MemsetMemcpyVisitor::doAnalysis(const HLInst *Inst,
                                     MemsetMemcpyCandidate &MMC) const {
  Intrinsic::ID IntrinID = 0;
  if (!Inst->isIntrinCall(IntrinID)) {
    return false;
  }
  bool IsMemset = false;

  if (IntrinID == Intrinsic::memset) {
    IsMemset = true;
  } else if (IntrinID != Intrinsic::memcpy) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "\nHIR Lower Small Memset/Memcpy: analyse ");
  LLVM_DEBUG(Inst->dump(););

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

  // Skip memop intrinsics with null or undef operands.
  // TODO: add support for null refs.
  if ((!IsMemset && (SrcOrValOp->isNull() || SrcOrValOp->containsUndef())) ||
      DstOp->isNull() || DstOp->containsUndef()) {
    return false;
  }

  // The following restrictions applied for profitability purposes:
  // Dst should be either an alloca-based ref or a ref which accesses NoAlias
  // arg.
  if (!(DstOp->accessesAlloca() || DstOp->accessesNoAliasFunctionArgument())) {
    return false;
  }

  bool SrcOrValOpIsConst = false;
  if (IsMemset) {
    SrcOrValOpIsConst = SrcOrValOp->isIntConstant();
    if (!SrcOrValOpIsConst && !SrcOrValOp->isTerminalRef()) {
      return false;
    }
  } else {
    // Src should be an alloca-based ref, constant array ref or a ref which
    // accesses NoAlias arg.
    if (!SrcOrValOp->accessesAlloca() && !SrcOrValOp->accessesConstantArray() &&
        !SrcOrValOp->accessesNoAliasFunctionArgument())
      return false;
  }

  unsigned DstInnermostDimSize = 0;
  unsigned SrcInnermostDimSize = 0;
  const CanonExpr *DstInnermostDimIndex = nullptr;
  const CanonExpr *SrcInnermostDimIndex = nullptr;
  Type *DstElemType =
      findElementType(DstOp, DstInnermostDimSize, DstInnermostDimIndex);
  if (!DstElemType) {
    return false;
  }

  if (IsMemset && !SrcOrValOpIsConst) {
    if (DstElemType != Type::getInt8Ty(DstElemType->getContext())) {
      // We only allow non-constant memsets with int8 access type to avoid
      // SrcOrVal operand replication for wider types.
      return false;
    }
  }

  if (!IsMemset) {
    Type *SrcElemType =
        findElementType(SrcOrValOp, SrcInnermostDimSize, SrcInnermostDimIndex);
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

  // Bail out if we can create an out-of-range array access.
  if (DstInnermostDimSize &&
      isOutOfRangeAccess(TC, DstInnermostDimSize, DstInnermostDimIndex)) {
    return false;
  }

  if (SrcInnermostDimSize &&
      isOutOfRangeAccess(TC, SrcInnermostDimSize, SrcInnermostDimIndex)) {
    return false;
  }

  // Check that we can successfully create const dd ref for Memset.
  if (IsMemset && SrcOrValOpIsConst) {
    RegDDRef *ConstRef = createConstRef(SrcOrValOp, DstElemType);
    if (!ConstRef) {
      return false;
    }
    MMC.SrcOrValOp = ConstRef;
  } else {
    MMC.SrcOrValOp = const_cast<RegDDRef *>(SrcOrValOp);
  }

  LLVM_DEBUG(dbgs() << "\nHIR Lower Small Memset/Memcpy: "
                    << (IsMemset ? "memset" : "memcpy") << "() is eligible.\n");

  MMC.DstOp = DstOp;
  MMC.IsMemset = IsMemset;
  MMC.TC = TC;
  MMC.ElemTy = ElemTy;

  return true;
}

// Given an original AddressOf ref from memop(), function creates a
// corresponding array ref for the transformed loop.
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
  LLVM_DEBUG(dbgs() << "\nHIR Lower Small Memset/Memcpy: transform "
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
    if (!MMC.SrcOrValOp->isConstant()) {
      // unlink operand from memset before placing it into store.
      Inst->removeOperandDDRef(1);
    }
    SrcOrValOp = MMC.SrcOrValOp;
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
  if (!MMC.IsMemset || !SrcOrValOp->isConstant()) {
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

  unsigned PrevLowered = NumLoweredMemsetMemcpy;
  for (auto &RegIt : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    HLRegion &Reg = cast<HLRegion>(RegIt);
    HLNodeUtils::visitRange(MMV, Reg.child_begin(), Reg.child_end());
  }

  return NumLoweredMemsetMemcpy != PrevLowered;
}

PreservedAnalyses
HIRLowerSmallMemsetMemcpyPass::runImpl(Function &F, FunctionAnalysisManager &AM,
                                       HIRFramework &HIRF) {
  if (DisableLowerSmallMemsetMemcpyPass) {
    return PreservedAnalyses::all();
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : " << F.getName() << "\n");
  ModifiedHIR = HIRLowerSmallMemsetMemcpy(HIRF).run();

  return PreservedAnalyses::all();
}

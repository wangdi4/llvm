//===----- IntelVPOCodeGenHIR.cpp -----------------------------------------===//
//
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the HIR vector Code generation for VPlan.
///
//===----------------------------------------------------------------------===//

#include "IntelVPOCodeGenHIR.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"
#include "llvm/Transforms/Utils/Intel_GeneralUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#define DEBUG_TYPE "VPOCGHIR"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::vpo;

STATISTIC(LoopsVectorized, "Number of HIR loops vectorized");

static cl::opt<bool>
    DisableStressTest("disable-vplan-stress-test", cl::init(false), cl::Hidden,
                      cl::desc("Disable VPO Vectorizer Stress Testing"));

static cl::opt<bool>
    EnableNestedBlobVec("enable-nested-blob-vec", cl::init(true), cl::Hidden,
                        cl::desc("Enable vectorization of loops with nested blobs"));

static cl::opt<bool>
    EnableBlobCoeffVec("enable-blob-coeff-vec", cl::init(true), cl::Hidden,
                       cl::desc("Enable vectorization of loops with blob IV coefficients"));

/// Don't vectorize loops with a known constant trip count below this number if
/// set to a non zero value.
static cl::opt<unsigned> TinyTripCountThreshold(
    "vplan-vectorizer-min-trip-count", cl::init(0), cl::Hidden,
    cl::desc("Don't vectorize loops with a constant "
             "trip count that is smaller than this value."));

namespace llvm {

static RegDDRef *getConstantSplatDDRef(DDRefUtils &DDRU, Constant *ConstVal,
                                       unsigned VF) {
  Constant *ConstVec = ConstantVector::getSplat(VF, ConstVal);
  if (isa<ConstantDataVector>(ConstVec))
    return DDRU.createConstDDRef(cast<ConstantDataVector>(ConstVec));
  if (isa<ConstantAggregateZero>(ConstVec))
    return DDRU.createConstDDRef(cast<ConstantAggregateZero>(ConstVec));
  if (isa<ConstantVector>(ConstVec))
    return DDRU.createConstDDRef(cast<ConstantVector>(ConstVec));
  llvm_unreachable("Unhandled vector type");
}

static bool refIsUnit(unsigned Level, const RegDDRef *Ref, VPOCodeGenHIR *CG) {
  if (!Ref->isMemRef())
    return false;

  bool IsNegStride;
  if (!Ref->isUnitStride(Level, IsNegStride) || IsNegStride)
    return false;

  CG->addUnitStrideRef(Ref);
  return true;
}

// This class implements code generation for a nested blob.
class NestedBlobCG : public SCEVVisitor<NestedBlobCG, RegDDRef *> {
private:
  const RegDDRef *RDDR;
  HLNodeUtils &HNU;
  DDRefUtils &DDRU;
  VPOCodeGenHIR *ACG;
  RegDDRef *MaskDDRef;

  enum NewOpCodes {
    NewOpsStart = Instruction::OtherOpsEnd + 1,
    UMaxOp,
    SMaxOp,
    NewOpsEnd
  };

  // Add instruction at end of main loop after adding mask if mask is not null.
  void addInst(HLInst *Inst) {
    if (MaskDDRef)
      Inst->setMaskDDRef(MaskDDRef->clone());
    if (auto InsertRegion = dyn_cast<HLLoop>(ACG->getInsertRegion()))
      HLNodeUtils::insertAsLastChild(InsertRegion, Inst);
    else
      addInst(Inst, true);
  }

  // Insert instruction into HLIf region.
  void addInst(HLInst *Inst, const bool IsThenChild) {
    // Simply put MaskDDRef on each instruction under if. For innermost uniform
    // predicates it's responsibility of predicator to remove unnecessary
    // predicates.
    if (MaskDDRef)
      Inst->setMaskDDRef(MaskDDRef->clone());
    auto InsertRegion = dyn_cast<HLIf>(ACG->getInsertRegion());
    assert(InsertRegion && "HLIf is expected as insert region.");
    HLNodeUtils::insertAsLastChild(InsertRegion, Inst, IsThenChild);
  }

  RegDDRef *codegenStandAloneBlob(const SCEV *SC);
  RegDDRef *codegenNAryOp(const SCEVNAryExpr *SC, unsigned OpCode);
  RegDDRef *codegenUDivOp(const SCEVUDivExpr *SC);
  RegDDRef *codegenCoeff(Constant *Const);
  RegDDRef *codegenCoeff(int64_t Coeff, Type *Ty);
  RegDDRef *codegenConversion(RegDDRef *Src, unsigned ConvOpCode,
                              Type *DestType);

public:
  NestedBlobCG(const RegDDRef *R, HLNodeUtils &H, DDRefUtils &D, VPOCodeGenHIR *C,
               RegDDRef *M)
      : RDDR(R), HNU(H), DDRU(D), ACG(C), MaskDDRef(M) {}

  RegDDRef *visit(const SCEV *SC) {
    RegDDRef *RDDR;
    if ((RDDR = ACG->getWideRefForSCVal(SC)))
      return RDDR;

    RDDR = SCEVVisitor::visit(SC);
    ACG->addSCEVWideRefMapping(SC, RDDR);
    return RDDR;
  }

  RegDDRef *visitConstant(const SCEVConstant *Constant);
  RegDDRef *visitTruncateExpr(const SCEVTruncateExpr *Expr);
  RegDDRef *visitZeroExtendExpr(const SCEVZeroExtendExpr *Expr);
  RegDDRef *visitSignExtendExpr(const SCEVSignExtendExpr *Expr);
  RegDDRef *visitAddExpr(const SCEVAddExpr *Expr);
  RegDDRef *visitMulExpr(const SCEVMulExpr *Expr);
  RegDDRef *visitUDivExpr(const SCEVUDivExpr *Expr);
  RegDDRef *visitAddRecExpr(const SCEVAddRecExpr *Expr);
  RegDDRef *visitSMaxExpr(const SCEVSMaxExpr *Expr);
  RegDDRef *visitUMaxExpr(const SCEVUMaxExpr *Expr);
  RegDDRef *visitUnknown(const SCEVUnknown *Expr);
  RegDDRef *visitCouldNotCompute(const SCEVCouldNotCompute *Expr);
};

RegDDRef *NestedBlobCG::codegenCoeff(Constant *Const) {
  return getConstantSplatDDRef(DDRU, Const, ACG->getVF());
}

RegDDRef *NestedBlobCG::codegenCoeff(int64_t Coeff, Type *Ty) {
  Constant *ConstCoeff;

  // Null value for pointer types needs special treatment
  if (Coeff == 0 && Ty->isPointerTy()) {
    ConstCoeff = Constant::getNullValue(Ty);
  } else {
    ConstCoeff = ConstantInt::getSigned(Ty, Coeff);
  }

  return codegenCoeff(ConstCoeff);
}

RegDDRef *NestedBlobCG::codegenConversion(RegDDRef *Src, unsigned ConvOpCode,
                                          Type *DestType) {
  assert((ConvOpCode == Instruction::ZExt || ConvOpCode == Instruction::SExt ||
          ConvOpCode == Instruction::Trunc) &&
         "Unexpected conversion OpCode");

  Type *VecTy = VectorType::get(DestType, ACG->getVF());
  HLInst *WideInst =
      HNU.createCastHLInst(VecTy, ConvOpCode, Src->clone(), "NBConv");
  addInst(WideInst);
  return WideInst->getLvalDDRef();
}

// Given the SCEV expression for a standalone blob, return the widened Ref
// corresponding to the same.
RegDDRef *NestedBlobCG::codegenStandAloneBlob(const SCEV *SC) {
  unsigned BlobIndex = RDDR->getBlobUtils().findBlob(SC);
  assert(BlobIndex != InvalidBlobIndex && "SCEV is not a Blob");

  const BlobDDRef *BDDR = RDDR->getBlobDDRef(BlobIndex);
  assert(BDDR != nullptr && "BlobDDRef not found!");

  RegDDRef *WideRef;

  if (auto WInst = ACG->getWideInst(BDDR->getSymbase())) {
    WideRef = WInst->getLvalDDRef();
  } else {
    WideRef = DDRU.createScalarRegDDRef(BDDR->getSymbase(),
                                        BDDR->getSingleCanonExpr()->clone());
    WideRef = ACG->widenRef(WideRef);
  }

  return WideRef;
}

RegDDRef *NestedBlobCG::codegenNAryOp(const SCEVNAryExpr *SC, unsigned OpCode) {
  assert(SC->getNumOperands() && "Unexpected SCEV with no operands");
  auto SCOperands = SC->operands();

  // Initialize OpTree with the first operand
  RegDDRef *CurDDRef = visit(*SCOperands.begin());
  for (auto Op = std::next(SCOperands.begin()), OpEnd = SCOperands.end();
       Op != OpEnd; ++Op) {
    RegDDRef *InnerDDRef = visit(*Op);
    HLInst *WideInst;

    if (OpCode == UMaxOp) {
      WideInst = HNU.createSelect(CmpInst::ICMP_UGT, CurDDRef->clone(),
                                  InnerDDRef->clone(), CurDDRef->clone(),
                                  InnerDDRef->clone(), "NAry");
    } else if (OpCode == SMaxOp) {
      WideInst = HNU.createSelect(CmpInst::ICMP_SGT, CurDDRef->clone(),
                                  InnerDDRef->clone(), CurDDRef->clone(),
                                  InnerDDRef->clone(), "NAry");
    } else {
      WideInst = HNU.createBinaryHLInst(OpCode, CurDDRef->clone(),
                                        InnerDDRef->clone(), "NAry");
    }

    addInst(WideInst);
    CurDDRef = WideInst->getLvalDDRef();
  }

  return CurDDRef;
}

RegDDRef *NestedBlobCG::visitConstant(const SCEVConstant *Constant) {
  return codegenCoeff(Constant->getValue());
}

RegDDRef *NestedBlobCG::visitTruncateExpr(const SCEVTruncateExpr *Expr) {
  RegDDRef *Src = visit(Expr->getOperand());
  return codegenConversion(Src, Instruction::Trunc, Expr->getType());
}

RegDDRef *NestedBlobCG::visitZeroExtendExpr(const SCEVZeroExtendExpr *Expr) {
  RegDDRef *Src = visit(Expr->getOperand());
  return codegenConversion(Src, Instruction::ZExt, Expr->getType());
}

RegDDRef *NestedBlobCG::visitSignExtendExpr(const SCEVSignExtendExpr *Expr) {
  RegDDRef *Src = visit(Expr->getOperand());
  return codegenConversion(Src, Instruction::SExt, Expr->getType());
}

RegDDRef *NestedBlobCG::visitAddExpr(const SCEVAddExpr *Expr) {
  return codegenNAryOp(Expr, Instruction::Add);
}

RegDDRef *NestedBlobCG::visitMulExpr(const SCEVMulExpr *Expr) {
  return codegenNAryOp(Expr, Instruction::Mul);
}

RegDDRef *NestedBlobCG::visitUDivExpr(const SCEVUDivExpr *Expr) {
  RegDDRef *DivLHS = visit(Expr->getLHS());
  RegDDRef *DivRHS = visit(Expr->getRHS());
  HLInst *WideInst;

  WideInst = HNU.createBinaryHLInst(Instruction::UDiv, DivLHS->clone(),
                                    DivRHS->clone(), "UDiv");
  addInst(WideInst);
  return WideInst->getLvalDDRef();
}

RegDDRef *NestedBlobCG::visitAddRecExpr(const SCEVAddRecExpr *Expr) {
  llvm_unreachable("Expected add-recs to be broken by canon-expr");
}

RegDDRef *NestedBlobCG::visitSMaxExpr(const SCEVSMaxExpr *Expr) {
  return codegenNAryOp(Expr, SMaxOp);
}

RegDDRef *NestedBlobCG::visitUMaxExpr(const SCEVUMaxExpr *Expr) {
  return codegenNAryOp(Expr, UMaxOp);
}

RegDDRef *NestedBlobCG::visitUnknown(const SCEVUnknown *Expr) {
  return codegenStandAloneBlob(Expr);
}

RegDDRef *NestedBlobCG::visitCouldNotCompute(const SCEVCouldNotCompute *Expr) {
  llvm_unreachable("Attempt to use a SCEVCouldNotCompute object!");
}

namespace {
// Check if a scev expression can possibly result in a divide by zero.
class DivByZeroCheck {
private:
  bool DivByZero;

public:
  DivByZeroCheck() : DivByZero(false) {}
  bool follow(const SCEV *SC) {
    if (auto *DivExpr = dyn_cast<SCEVUDivExpr>(SC)) {
      const SCEV *RHS = DivExpr->getRHS();
      if (isa<SCEVConstant>(RHS)) {
        if (RHS->isZero())
          DivByZero = true;
      } else {
        DivByZero = true;
      }
    }

    return !isDone();
  }

  bool isDivByZeroPossible() const { return DivByZero; }
  bool isDone() const { return DivByZero; }
};

class HandledCheck final : public HLNodeVisitorBase {
private:
  bool IsHandled;
  const HLLoop *OrigLoop;
  TargetLibraryInfo *TLI;
  unsigned VF;
  bool UnitStrideRefSeen;
  bool MemRefSeen;
  bool NegativeIVCoeffSeen;
  bool FieldAccessSeen;
  unsigned LoopLevel;
  VPOCodeGenHIR *CG;

  void visitRegDDRef(RegDDRef *RegDD);
  void visitCanonExpr(CanonExpr *CExpr, bool InMemRef, bool InMaskedStmt);

  // For each blob of the Ref check whether it's changed somewhere in the loop.
  bool isUniform(const RegDDRef *Ref) const;

public:
  HandledCheck(const HLLoop *OrigLoop, TargetLibraryInfo *TLI, int VF,
               VPOCodeGenHIR *CG)
      : IsHandled(true), OrigLoop(OrigLoop), TLI(TLI), VF(VF),
        UnitStrideRefSeen(false), MemRefSeen(false), NegativeIVCoeffSeen(false),
        FieldAccessSeen(false), CG(CG) {
    LoopLevel = OrigLoop->getNestingLevel();
  }

  void visit(HLDDNode *Node);

  void visit(HLNode *Node) {
    // Current CG uses VPlan to generate the code, so Gotos should be ok to
    // support in CG.
    if (!CG->isSearchLoop()) {
      LLVM_DEBUG(
          dbgs() << "VPLAN_OPTREPORT: Loop not handled - unsupported HLNode\n");
      IsHandled = false;
    }
  }

  void postVisit(HLNode *Node) {}

  bool isDone() const { return false; }
  bool isHandled() { return IsHandled; }
  bool getUnitStrideRefSeen() { return UnitStrideRefSeen; }
  bool getMemRefSeen() { return MemRefSeen; }
  bool getNegativeIVCoeffSeen() { return NegativeIVCoeffSeen; }
  bool getFieldAccessSeen() { return FieldAccessSeen; }
};

class HLInstCounter final : public HLNodeVisitorBase {
private:
  const HLLoop *OrigLoop;
  unsigned NumInsts;

public:
  HLInstCounter(const HLLoop *OrigLoop) : OrigLoop(OrigLoop) { NumInsts = 0; }

  unsigned getNumInsts() { return NumInsts; }

  void postVisit(HLNode *Node) {}

  void visit(HLNode *Node) {
    if (isa<HLInst>(Node))
      NumInsts++;
  }
};
} // End anonymous namespace

void HandledCheck::visit(HLDDNode *Node) {
  if (!isa<HLInst>(Node) && !isa<HLIf>(Node)) {
    LLVM_DEBUG(
        dbgs() << "VPLAN_OPTREPORT: Loop not handled - only HLInst/HLIf are "
                  "supported\n");
    IsHandled = false;
    return;
  }

  // Calls supported are masked/non-masked svml and non-masked intrinsics.
  if (HLInst *Inst = dyn_cast<HLInst>(Node)) {
    auto LLInst = Inst->getLLVMInstruction();

    if (LLInst->mayThrow()) {
      LLVM_DEBUG(Inst->dump());
      LLVM_DEBUG(
          dbgs()
          << "VPLAN_OPTREPORT: Loop not handled - instruction may throw\n");
      IsHandled = false;
      return;
    }

    auto Opcode = LLInst->getOpcode();
    if ((Opcode == Instruction::UDiv || Opcode == Instruction::SDiv ||
         Opcode == Instruction::URem || Opcode == Instruction::SRem) &&
        (Inst->getParent() != OrigLoop)) {
      LLVM_DEBUG(Inst->dump());
      LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop not handled - masked DIV/REM "
                           "instruction\n");
      IsHandled = false;
      return;
    }

    auto TLval = Inst->getLvalDDRef();

    if (!CG->isSearchLoop() && TLval && TLval->isTerminalRef() &&
        OrigLoop->isLiveOut(TLval->getSymbase()) &&
        Inst->getParent() != OrigLoop) {
      LLVM_DEBUG(Inst->dump());
      LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: Liveout conditional scalar assign "
                           "not handled\n");
      IsHandled = false;
      return;
    }

    // FIXME: With proper support from CG this bail-out is not needed.
    if (TLval && TLval->isMemRef() && isUniform(TLval)) {
      LLVM_DEBUG(Inst->dump());
      LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: Uniform store is not handled\n");
      IsHandled = false;
      return;
    }

    if (Inst->isCallInst()) {
      const CallInst *Call = cast<CallInst>(Inst->getLLVMInstruction());
      Function *Fn = Call->getCalledFunction();
      if (!Fn) {
        LLVM_DEBUG(Inst->dump());
        LLVM_DEBUG(
            dbgs() << "VPLAN_OPTREPORT: Loop not handled - indirect call\n");
        IsHandled = false;
        return;
      }

      StringRef CalledFunc = Fn->getName();

      if (Inst->getParent() != OrigLoop &&
          (VF > 1 && !TLI->isFunctionVectorizable(CalledFunc, VF))) {
        // Masked svml calls are supported, but masked intrinsics are not at
        // the moment.
        LLVM_DEBUG(Inst->dump());
        LLVM_DEBUG(
            dbgs() << "VPLAN_OPTREPORT: Loop not handled - masked intrinsic\n");
        IsHandled = false;
        return;
      }

      // Quick hack to avoid loops containing fabs in 447.dealII from becoming
      // vectorized due to bug in unrolling. The problem involves loop index
      // variable that spans outside the array range, resulting in segfault.
      // floor calls are also temporarily disabled until FeatureOutlining is
      // fixed (CQ410864)
      if (CalledFunc == "fabs" || CalledFunc == "floor") {
        LLVM_DEBUG(Inst->dump());
        LLVM_DEBUG(
            dbgs() << "VPLAN_OPTREPORT: Loop not handled - fabs/floor call "
                      "disabled\n");
        IsHandled = false;
        return;
      }

      Intrinsic::ID ID = getVectorIntrinsicIDForCall(Call, TLI);
      if ((VF > 1 && !TLI->isFunctionVectorizable(CalledFunc, VF)) && !ID) {
        LLVM_DEBUG(
            dbgs()
            << "VPLAN_OPTREPORT: Loop not handled - call not vectorizable\n");
        IsHandled = false;
        return;
      }

      // These intrinsics need the second argument to remain scalar(consequently loop
      // invariant). Support to be added later.
      if (ID == Intrinsic::ctlz || ID == Intrinsic::cttz || ID == Intrinsic::powi) {
        LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop not handled - "
                             "ctlz/cttz/powi intrinsic\n");
        IsHandled = false;
        return;
      }
    }
  }

  for (auto Iter = Node->ddref_begin(), End = Node->ddref_end(); Iter != End;
       ++Iter) {
    visitRegDDRef(*Iter);
  }
}

// visitRegDDRef - Visits RegDDRef to visit the Canon Exprs
// present inside it.
void HandledCheck::visitRegDDRef(RegDDRef *RegDD) {

  if (!VectorType::isValidElementType(RegDD->getSrcType()) ||
      !VectorType::isValidElementType(RegDD->getDestType())) {
    LLVM_DEBUG(RegDD->getSrcType()->dump());
    LLVM_DEBUG(RegDD->getDestType()->dump());
    LLVM_DEBUG(
        dbgs() << "VPLAN_OPTREPORT: Loop not handled - invalid element type\n");
    IsHandled = false;
    return;
  }

  if (refIsUnit(LoopLevel, RegDD, CG))
    UnitStrideRefSeen = true;

  // Visit CanonExprs inside the RegDDRefs
  bool InMaskedStmt = RegDD->getHLDDNode()->getParent() != OrigLoop;
  for (auto Iter = RegDD->canon_begin(), End = RegDD->canon_end(); Iter != End;
       ++Iter) {
    visitCanonExpr(*Iter, RegDD->isMemRef(), InMaskedStmt);
  }

  // Visit GEP Base
  if (RegDD->hasGEPInfo()) {
    // Track if we see field accesses in the lowest dimension
    if (RegDD->hasTrailingStructOffsets(1))
      FieldAccessSeen = true;

    MemRefSeen = true;

    auto BaseCE = RegDD->getBaseCE();

    if (!BaseCE->isInvariantAtLevel(LoopLevel)) {
      LLVM_DEBUG(
          dbgs()
          << "VPLAN_OPTREPORT: Loop not handled - BaseCE not invariant\n");
      IsHandled = false;
      return;
    }
  }
}

// Checks Canon Expr to see if we support it. Currently, we do not
// support blob IV coefficients
void HandledCheck::visitCanonExpr(CanonExpr *CExpr, bool InMemRef,
                                  bool InMaskedStmt) {
  // Track if we see a memory reference with a negative IV coefficient
  if (InMemRef) {
    int64_t ConstCoeff = 0;
    CExpr->getIVCoeff(LoopLevel, nullptr, &ConstCoeff);
    if (ConstCoeff < 0)
      NegativeIVCoeffSeen = true;
  }
  if (!EnableBlobCoeffVec && CExpr->hasIVBlobCoeff(LoopLevel)) {
    LLVM_DEBUG(
        dbgs()
        << "VPLAN_OPTREPORT: Loop not handled - IV with blob coefficient\n");
    IsHandled = false;
    return;
  }

  // Skip the bailout for nested blobs if we are enabling vectorization for
  // loops with nested blobs. We still need to bail out for a possible divide by
  // zero until we add support for masked divides.
  SmallVector<unsigned, 8> BlobIndices;
  CExpr->collectBlobIndices(BlobIndices, true /* MakeUnique */);
  if (EnableNestedBlobVec) {
    if (InMaskedStmt) {
      for (auto &BI : BlobIndices) {
        auto TopBlob = CExpr->getBlobUtils().getBlob(BI);
        DivByZeroCheck ZChk;
        SCEVTraversal<DivByZeroCheck> ZeroCheck(ZChk);
        ZeroCheck.visitAll(TopBlob);

        if (ZChk.isDivByZeroPossible()) {
          LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: Masked divide support TBI\n");
          IsHandled = false;
          return;
        }
      }
    }

    return;
  }

  for (auto &BI : BlobIndices) {
    auto TopBlob = CExpr->getBlobUtils().getBlob(BI);

    if (CExpr->getBlobUtils().isNestedBlob(TopBlob)) {
      LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop not handled - nested blob\n");
      IsHandled = false;
      return;
    }
  }
}

// Return true if given memory reference is uniform.
bool HandledCheck::isUniform(const RegDDRef *Ref) const {
  assert(Ref->isMemRef() && "Given RegDDRef is not memory reference.");
  return Ref->isStructurallyInvariantAtLevel(LoopLevel);
}

// Return true if Loop is currently handled by HIR vector code generation.
bool VPOCodeGenHIR::loopIsHandled(HLLoop *Loop, unsigned int VF) {

  // Only handle normalized loops
  if (!Loop->isNormalized()) {
    LLVM_DEBUG(
        dbgs()
        << "VPLAN_OPTREPORT: Loop not handled - loop not in normalized form\n");
    return false;
  }

  // We are working with normalized loops, trip count is loop UpperBound + 1.
  auto UBRef = Loop->getUpperDDRef();
  int64_t UBConst;

  if (UBRef->isIntConstant(&UBConst)) {
    auto ConstTripCount = UBConst + 1;

    // Check for minimum trip count threshold
    if (TinyTripCountThreshold && ConstTripCount <= TinyTripCountThreshold) {
      LLVM_DEBUG(
          dbgs() << "VPLAN_OPTREPORT: Loop not handled - loop with small "
                    "trip count\n");
      return false;
    }

    // Check that main vector loop will have at least one iteration
    if (ConstTripCount < VF) {
      LLVM_DEBUG(
          dbgs()
          << "VPLAN_OPTREPORT: Loop not handled - zero iteration main loop\n");
      return false;
    }

    // Set constant trip count
    setTripCount((uint64_t)ConstTripCount);
  }

  HandledCheck NodeCheck(Loop, TLI, VF, this);
  HLNodeUtils::visitRange(NodeCheck, Loop->child_begin(), Loop->child_end());
  if (!NodeCheck.isHandled())
    return false;

  // If we are not in stress testing mode, only vectorize when some
  // unit stride refs are seen. Still vectorize the case when no mem refs
  // are seen. Remove this check once vectorizer cost model is fully
  // implemented.
  if (DisableStressTest && NodeCheck.getMemRefSeen() &&
      !NodeCheck.getUnitStrideRefSeen()) {
    LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop not handled - all mem refs non "
                         "unit-stride\n");
    return false;
  }

  // Workaround for performance regressions until cost model can be refined
  if (NodeCheck.getFieldAccessSeen() && NodeCheck.getNegativeIVCoeffSeen()) {
    LLVM_DEBUG(
        dbgs() << "VPLAN_OPTREPORT: Loop not handled - combination of field "
                  "accesses and negative IV coefficients seen\n");
    return false;
  }

  return true;
}

HLLoop *VPOCodeGenHIR::findRednHoistInsertionPoint(HLLoop *Lp) {
  // Inspired by hpo_huntHoist in icc - check to see at what loop it is safe
  // to hoist the reduction initializer and sink the last value compute
  // instructions. Basically a quick and dirty check to see that a parent
  // loop only contains a child loop, so no dependences prevent hoisting.
  if (!Lp->hasPreheader() && !Lp->hasPostexit()) {
    HLLoop *Parent = dyn_cast<HLLoop>(Lp->getParent());
    if (Parent && Parent->isDo() && Parent->getNumChildren() == 1) {
      return findRednHoistInsertionPoint(Parent);
    }
  }
  return Lp;
}

void VPOCodeGenHIR::initializeVectorLoop(unsigned int VF) {
  assert(VF > 1);
  setVF(VF);

  LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: VPlan handled loop, VF = " << VF << " "
                    << Fn.getName() << "\n");
  LLVM_DEBUG(dbgs() << "Handled loop before vec codegen: \n");
  LLVM_DEBUG(OrigLoop->dump());

  LoopsVectorized++;
  SRA->computeSafeReductionChains(OrigLoop);

  const SafeRedChainList &SRCL = SRA->getSafeReductionChain(OrigLoop);
  for (auto &SafeRedInfo : SRCL) {
    for (auto &Inst : SafeRedInfo.Chain) {
      // Make sure all reduction instructions are part of the OrigLoop that
      // was queried for safe reductions.
      assert(OrigLoop == Inst->getParentLoop() &&
             "Reduction inst should be in OrigLoop");
      (void)Inst;
    }
  }
  RednHoistLp = findRednHoistInsertionPoint(OrigLoop);

  // Setup main and remainder loops
  bool NeedRemainderLoop = false;
  auto MainLoop = HIRTransformUtils::setupMainAndRemainderLoops(
      OrigLoop, VF, NeedRemainderLoop, LORBuilder, OptimizationType::Vectorizer);

  // Do not attempt hoisting when a remainder loop is needed as remainder
  // loop updates the scalar reduction value. Since we blend in the scalar
  // reduction value in reduction initialization code, hoisting up the
  // initialization will generate incorrect code.

  if (NeedRemainderLoop)
    RednHoistLp = OrigLoop;

  // The reduction initializer hoist loop should point to the vector loop and
  // not the original scalar loop if it was found not to be safe to hoist
  // further.
  if (RednHoistLp == OrigLoop)
    RednHoistLp = MainLoop;

  MainLoop->extractZtt();
  setNeedRemainderLoop(NeedRemainderLoop);
  setMainLoop(MainLoop);
  addInsertRegion(MainLoop);

  // Disable further vectorization attempts on main and remainder loops
  MainLoop->markDoNotVectorize();
  if (NeedRemainderLoop) {
    OrigLoop->markDoNotVectorize();
  }
}

void VPOCodeGenHIR::finalizeVectorLoop(void) {
  LLVM_DEBUG(dbgs() << "\n\n\nHandled loop after: \n");
  LLVM_DEBUG(MainLoop->dump());
  if (NeedRemainderLoop)
    LLVM_DEBUG(OrigLoop->dump());

  if (!MainLoop->hasChildren()) {
    // TODO: Can this happen if HIR would never let "dead" loops to be in the
    //       input to the vectorizer? Currently it's not the case, e.g. the loop
    //       containing a single call to @llvm.assume is not considered as empty
    //       by HIR framework, but once this is fixed this condition might be
    //       changed to an assert.
    LLVM_DEBUG(dbgs() << "\n\n\nRemoving empty loop\n");
    HLNodeUtils::removeEmptyNodes(MainLoop, true);
  } else {
    // Prevent LLVM from possibly unrolling vectorized loops with non-constant
    // trip counts. See loop in function fxpAutoCorrelation() that is part of
    // telecom/autcor00data_1 (opt_base_st_64_hsw). Inner loop has max trip
    // count estimate of 16, VPO vectorizer chooses VF=4, and LLVM unrolls by 4.
    // However, the inner loop does not always have a constant 16 trip count,
    // leading to a performance degradation caused by entering the scalar code
    // path.
    if (!MainLoop->isConstTripLoop()) {
      const Loop *Lp = MainLoop->getLLVMLoop();
      LLVMContext &Context = Lp->getHeader()->getContext();
      SmallVector<Metadata *, 4> MDs;
      MDs.push_back(nullptr);
      SmallVector<Metadata *, 1> DisableOperands;
      DisableOperands.push_back(
          MDString::get(Context, "llvm.loop.unroll.disable"));
      MDNode *DisableUnroll = MDNode::get(Context, DisableOperands);
      MDs.push_back(DisableUnroll);
      MDNode *NewLoopID = MDNode::get(Context, MDs);
      NewLoopID->replaceOperandWith(0, NewLoopID);
      MainLoop->setLoopMetadata(NewLoopID);
    }
  }

  // If a remainder loop is not needed get rid of the OrigLoop at this point.
  // Replace calls in remainderloop for FP consistency
  if (NeedRemainderLoop) {
    HIRLoopVisitor LV(OrigLoop, this);
    LV.replaceCalls();
  } else {
    // NeedRemainderLoop is false so trip count % VF == 0. Also check to see
    // that the trip count is small and the loop body is small. If so, do
    // complete unroll of the vector loop.
    uint64_t TripCount = getTripCount();
    bool KnownTripCount = TripCount > 0 ? true : false;
    if (KnownTripCount && TripCount <= SmallTripThreshold &&
        OrigLoop->isInnermost()) {
      HLInstCounter InstCounter(OrigLoop);
      HLNodeUtils::visitRange(InstCounter, OrigLoop->child_begin(),
                              OrigLoop->child_end());
      if (InstCounter.getNumInsts() <= SmallLoopBodyThreshold)
        HIRTransformUtils::completeUnroll(MainLoop);
    }
    HLNodeUtils::remove(OrigLoop);
  }
}

void VPOCodeGenHIR::eraseLoopIntrinsImpl(bool BeginDir) {
  HLContainerTy::iterator StartIter;
  HLContainerTy::iterator EndIter;
  if (BeginDir) {
    auto BeginNode = WVecNode->getEntryHLNode();
    assert(BeginNode && "Unexpected null entry node in WRNVecLoopNode");
    StartIter = BeginNode->getIterator();
    EndIter = OrigLoop->getIterator();
  } else {
    auto ExitNode = WVecNode->getExitHLNode();
    assert(ExitNode && "Unexpected null exit node in WRNVecLoopNode");
    StartIter = ExitNode->getIterator();

    auto LastNode =
        HLNodeUtils::getLastLexicalChild(OrigLoop->getParent(), OrigLoop);
    EndIter = std::next(LastNode->getIterator());
  }

  SmallSet<int, 2> BeginOrEndDirIDs;
  if (BeginDir) {
    BeginOrEndDirIDs.insert(DIR_OMP_SIMD);
    BeginOrEndDirIDs.insert(DIR_VPO_AUTO_VEC);
  } else {
    BeginOrEndDirIDs.insert(DIR_OMP_END_SIMD);
    BeginOrEndDirIDs.insert(DIR_VPO_END_AUTO_VEC);
  }
  for (auto Iter = StartIter; Iter != EndIter;) {
    auto HInst = dyn_cast<HLInst>(&*Iter);

    if (!HInst) {
      break;
    }

    // Move to the next iterator now as HInst may get removed below
    ++Iter;

    Intrinsic::ID IntrinID;
    if (HInst->isIntrinCall(IntrinID)) {
      if (vpo::VPOAnalysisUtils::isIntelClause(IntrinID)) {
        HLNodeUtils::remove(HInst);
        continue;
      }

      if (vpo::VPOAnalysisUtils::isIntelDirective(IntrinID)) {
        auto Inst = cast<IntrinsicInst>(HInst->getLLVMInstruction());
        StringRef DirStr = vpo::VPOAnalysisUtils::getDirectiveMetadataString(
            const_cast<IntrinsicInst *>(Inst));

        int DirID = vpo::VPOAnalysisUtils::getDirectiveID(DirStr);

        if (BeginOrEndDirIDs.count(DirID)) {
          HLNodeUtils::remove(HInst);
        } else if (VPOAnalysisUtils::isListEndDirective(DirID)) {
          HLNodeUtils::remove(HInst);
          return;
        }
      }
    }
  }

  assert(false && "Missing SIMD Begin/End directive");
}

void VPOCodeGenHIR::eraseLoopIntrins() {
  eraseLoopIntrinsImpl(true /* Intrinsics before loop */);
  eraseLoopIntrinsImpl(false /* Intrinsics before loop */);
}

// This function replaces scalar math lib calls in the remainder loop with
// the svml version used in the main vector loop in order to maintain
// consistency of precision. See the example below:
//
// Original remainder loop:
//
// <14>  + DO i1 = 128, 130, 1   <DO_LOOP>
// <5>   |   %call = @sinf((%b)[i1]);
// <7>   |   (%a)[i1] = %call;
// <14>  + END LOOP
//
// Transformed remainder loop:
//
// <14>  + DO i1 = 128, 130, 1   <DO_LOOP>
// <15>  |   %load = (%b)[i1];
// <16>  |   %__svml_sinf48 = @__svml_sinf4(%load);
// <17>  |   %call = extractelement %__svml_sinf48,  0;
// <7>   |   (%a)[i1] = %call;
// <14>  + END LOOP
//
// Detailed HIR:
//
// <14>  + DO i64 i1 = 128, 130, 1   <DO_LOOP>
// <15>  |   %load = (%b)[i1];
// <15>  |   <LVAL-REG> NON-LINEAR float %load {sb:15}
// <15>  |   <RVAL-REG> {al:4}(LINEAR float* %b)[LINEAR i64 i1] !tbaa !5 {sb:12}
// <15>  |      <BLOB> LINEAR float* %b {sb:6}
// <15>  |
// <16>  |   %__svml_sinf48 = @__svml_sinf4(%load);
// <16>  |   <LVAL-REG> NON-LINEAR <4 x float> %__svml_sinf48 {sb:16}
// <16>  |   <RVAL-REG> NON-LINEAR bitcast.float.<4 x float>(%load) {sb:15}
// <16>  |      <BLOB> NON-LINEAR float %load {sb:15}
// <16>  |
// <17>  |   %call = extractelement %__svml_sinf48,  0;
// <17>  |   <LVAL-REG> NON-LINEAR float %call {sb:7}
// <17>  |   <RVAL-REG> NON-LINEAR <4 x float> %__svml_sinf48 {sb:16}
// <17>  |
// <7>   |   (%a)[i1] = %call;
// <7>   |   <LVAL-REG> {al:4}(LINEAR float* %a)[LINEAR i64 i1] !tbaa !5 {sb:13}
// <7>   |      <BLOB> LINEAR float* %a {sb:9}
// <7>   |   <RVAL-REG> NON-LINEAR float %call {sb:7}
// <7>   |
// <14>  + END LOOP

void VPOCodeGenHIR::replaceLibCallsInRemainderLoop(HLInst *HInst) {

  // Used to remove the original math calls after iterating over them.
  SmallVector<HLInst *, 1> InstsToRemove;

  const CallInst *Call = cast<CallInst>(HInst->getLLVMInstruction());
  Function *F = Call->getCalledFunction();
  assert(F && "Unexpected null called function");
  StringRef FnName = F->getName();

  // Check to see if the call was vectorized in the main loop.
  if (TLI->isFunctionVectorizable(FnName, VF)) {
    SmallVector<RegDDRef *, 1> CallArgs;
    SmallVector<Type *, 1> ArgTys;

    // For each call argument, insert a scalar load of the element,
    // broadcast it to a vector.
    for (auto It = HInst->rval_op_ddref_begin(),
              ItEnd = HInst->rval_op_ddref_end();
         It != ItEnd; ++It) {
      // TODO: it is assumed that call arguments need to become vector.
      // In the future, some vectorizable calls may contain scalar
      // arguments. Additional checking is needed for these cases.

      // The DDRef of the original scalar call instruction.
      RegDDRef *Ref = *It;

      // The resulting type of the widened ref/broadcast.
      auto VecDestTy = VectorType::get(Ref->getDestType(), VF);

      RegDDRef *WideRef = nullptr;
      HLInst *LoadInst = nullptr;

      // Create the scalar load of the call argument. This is done so that
      // we can clone the new LvalDDRef and change its type to force the
      // broadcast. See %load in the example above. Essentially, the original
      // scalar %load becomes bitcast.float.<4 x float>, which is how HIRCG
      // knows to do the broadcast.
      if (Ref->isMemRef()) {
        // Ref is a memory reference: %t = sinf(a[i]);
        LoadInst = HInst->getHLNodeUtils().createLoad(Ref->clone(), "load");
      } else {
        // Ref in this case is a temp from a previous load: %r = sinf(%t).
        // Create a new temp and broadcast it for the call argument.
        LoadInst = HInst->getHLNodeUtils().createCopyInst(Ref->clone(), "copy");
      }

      // Construct the new RegDDRef for the call argument. Set the dest
      // type to the vector type required to do a broadcast. So, for
      // example, source type is float, and dest type becomes <4 x float>.
      // This causes the RegDDRef to obtain a bitcast. Because of this,
      // the ref is no longer a self blob and we must copy the BlobDDRef
      // from the original reference to this one. This is what the call
      // to makeConsistent() does.
      //
      // e.g., %load is a self blob, bitcast.float.<4 x float>(%load) is
      // no longer a self blob due to the existence of the bitcast. So,
      // copy BlobDDRef from %load to bitcast.float.<4 x float>(%load).
      HLNodeUtils::insertBefore(HInst, LoadInst);
      WideRef = LoadInst->getLvalDDRef()->clone();
      auto CE = WideRef->getSingleCanonExpr();
      CE->setDestType(VecDestTy);
      const SmallVector<const RegDDRef *, 1> AuxRefs = {
          LoadInst->getLvalDDRef()};
      WideRef->makeConsistent(&AuxRefs, OrigLoop->getNestingLevel());

      // Collect call arguments and types so that the function declaration
      // and call instruction can be generated.
      CallArgs.push_back(WideRef);
      ArgTys.push_back(VecDestTy);
    }

    // Using the newly created vector call arguments, generate the vector
    // call instruction and extract the low element.
    Function *VectorF = getOrInsertVectorFunction(
        Call, VF, ArgTys, TLI, Intrinsic::not_intrinsic,
        nullptr /*simd function*/, false /*non-masked*/);
    assert(VectorF && "Can't create vector function.");

    HLInst *WideCall = HInst->getHLNodeUtils().createCall(
        VectorF, CallArgs, VectorF->getName(), nullptr);
    HLNodeUtils::insertBefore(HInst, WideCall);

    // TODO: Matt can you look into the following code review comment
    // from Pankaj?
    // Call instructions can have one fake memref for each AddressOf
    // operand that can be read from/written into. This is necessary to
    // preserve data dependence semantics. I would recommend that vectorizer
    // handle those as well. You can access them using
    // HInst->fake_ddref_begin(). Keep in mind that they may contain
    // 'undef' in the index. This is used to obtain DV of *.
    if (FnName.find("sincos") != StringRef::npos) {
      // Since we're in the remainder loop and scalarizing for now,
      // then set the call argument strides for the sin/cos results
      // to indirect to force scalarization in MapIntrinToIml. Later,
      // when we support remainder loop vectorization, swap out the
      // following loop with the call to analyzeCallArgMemoryReferences().
      Instruction *WideInst =
          const_cast<Instruction *>(WideCall->getLLVMInstruction());
      CallInst *VecCall = cast<CallInst>(WideInst);
      for (unsigned I = 1; I < 3; I++) {
        AttrBuilder AttrList;
        AttrList.addAttribute("stride", "indirect");
        VecCall->setAttributes(VecCall->getAttributes().addAttributes(
            VecCall->getContext(), I + 1, AttrList));
      }
      // analyzeCallArgMemoryReferences(HInst, WideCall, CallArgs);
    }

    InstsToRemove.push_back(HInst);

    if (auto LvalDDRef = HInst->getLvalDDRef()) {
      HLInst *ExtractInst = HInst->getHLNodeUtils().createExtractElementInst(
          WideCall->getLvalDDRef()->clone(), 0, "elem", LvalDDRef->clone());
      HLNodeUtils::insertAfter(WideCall, ExtractInst);
    }
  }

  // Remove the original scalar call(s) to clean up the IR.
  for (unsigned Idx = 0; Idx < InstsToRemove.size(); Idx++) {
    HLInst *Inst = InstsToRemove[Idx];
    HLNodeUtils::remove(Inst);
  }
}

void VPOCodeGenHIR::HIRLoopVisitor::replaceCalls() {
  for (unsigned i = 0; i < CallInsts.size(); i++) {
    CG->replaceLibCallsInRemainderLoop(CallInsts[i]);
  }
}

void VPOCodeGenHIR::HIRLoopVisitor::visitInst(HLInst *I) {
  // Check for function calls.
  if (I->isCallInst()) {
    CallInsts.push_back(I);
  }
}

void VPOCodeGenHIR::HIRLoopVisitor::visitIf(HLIf *If) {
  for (auto ThenIt = If->then_begin(), ThenEnd = If->then_end();
       ThenIt != ThenEnd; ++ThenIt) {
    visit(*ThenIt);
  }
  for (auto ElseIt = If->else_begin(), ElseEnd = If->else_end();
       ElseIt != ElseEnd; ++ElseIt) {
    visit(*ElseIt);
  }
}

void VPOCodeGenHIR::HIRLoopVisitor::visitLoop(HLLoop *L) {
  for (auto Iter = L->child_begin(), EndItr = L->child_end(); Iter != EndItr;
       ++Iter) {
    visit(*Iter);
  }
}

bool VPOCodeGenHIR::isReductionRef(const RegDDRef *Ref, unsigned &Opcode) {
  // When widening decomposed nested blobs, we create temp Refs without
  // an associated DDNode.
  if (!Ref->getHLDDNode())
    return false;

  return SRA->isReductionRef(Ref, Opcode);
}

RegDDRef *VPOCodeGenHIR::widenRef(const RegDDRef *Ref) {
  RegDDRef *WideRef;
  auto RefDestTy = Ref->getDestType();
  auto VecRefDestTy = VectorType::get(RefDestTy, VF);
  auto RefSrcTy = Ref->getSrcType();
  auto VecRefSrcTy = VectorType::get(RefSrcTy, VF);

  // If the DDREF has a widened counterpart, return the same after setting
  // SrcType/DestType appropriately.
  if (Ref->isTerminalRef()) {
    unsigned RedOpCode;

    if (WidenMap.find(Ref->getSymbase()) != WidenMap.end()) {
      auto WInst = WidenMap[Ref->getSymbase()];
      WideRef = WInst->getLvalDDRef()->clone();

      auto CE = WideRef->getSingleCanonExpr();
      CE->setDestType(VecRefDestTy);
      CE->setSrcType(VecRefSrcTy);
      CE->setExtType(Ref->getSingleCanonExpr()->isSExt());

      return WideRef;
    }

    // Check if Ref is a reduction - we create widened DDREF for a
    // reduction ref the first time it is encountered and use this to replace
    // all occurrences of Ref. The widened ref is added to the WidenMap
    // here to accomplish this.
    if (isReductionRef(Ref, RedOpCode)) {

      auto Identity = HLInst::getRecurrenceIdentity(RedOpCode, RefDestTy);
      auto RedOpVecInst = insertReductionInitializer(Identity, Ref->clone());

      // Add to WidenMap and handle generating code for building reduction tail
      addToMapAndHandleLiveOut(Ref, RedOpVecInst, RednHoistLp);

      // LVAL ref of the initialization instruction is the widened reduction
      // ref.
      return RedOpVecInst->getLvalDDRef()->clone();
    }

    // Lval terminal refs get the widened ref duing the widened HLInst creation
    // later - simply return NULL.
    if (Ref->getHLDDNode() && Ref->isLval())
      return nullptr;
  }

  WideRef = Ref->clone();

  // Set VectorType on WideRef base pointer - BaseDestType is set to pointer
  // type of VF-wide vector of Ref's DestType. For addressof DDRef, desttype
  // is set to vector of pointers(scalar desttype).
  if (WideRef->hasGEPInfo()) {
    auto AddressSpace = Ref->getPointerAddressSpace();

    // Omit the range metadata as is done in loop vectorize which does
    // not propagate the same. We get a compile time error otherwise about
    // type mismatch for range values.
    WideRef->setMetadata(LLVMContext::MD_range, nullptr);

    if (WideRef->isAddressOf()) {
      WideRef->setBitCastDestType(VecRefDestTy);

      // There is nothing more to do for opaque types as they can only occur in
      // this form: &p[0].
      if (Ref->isOpaqueAddressOf()) {
        return WideRef;
      }
    } else {
      WideRef->setBitCastDestType(PointerType::get(VecRefDestTy, AddressSpace));
      // When the original scalar ref does not have alignment information
      // LLVM defaults to the ABI alignment for the ref's type. During widening,
      // we need to set the widened ref's alignment to the ABI alignment for
      // the scalar ref's type for such cases.
      unsigned Alignment = Ref->getAlignment();
      if (!Alignment) {
        auto DL = Ref->getDDRefUtils().getDataLayout();
        Alignment = DL.getABITypeAlignment(RefDestTy);
        WideRef->setAlignment(Alignment);
      }
    }
  }

  unsigned NestingLevel = OrigLoop->getNestingLevel();
  // For unit stride ref, nothing else to do
  if (isUnitStrideRef(Ref))
    return WideRef;

  SmallVector<const RegDDRef *, 4> AuxRefs;
  // For cases other than unit stride refs, we need to widen the induction
  // variable and replace blobs in Canon Expr with widened equivalents.
  for (auto I = WideRef->canon_begin(), E = WideRef->canon_end(); I != E; ++I) {
    auto CE = *I;

    // Collect blob indices in canon expr before we start changing the same.
    SmallVector<unsigned, 8> BlobIndices;
    CE->collectBlobIndices(BlobIndices, true /* MakeUnique */);

    if (CE->hasIV(NestingLevel)) {
      SmallVector<Constant *, 4> CA;
      Type *Int64Ty = CE->getSrcType();
      unsigned BlobCoeff;
      int64_t ConstCoeff;

      CE->getIVCoeff(NestingLevel, &BlobCoeff, &ConstCoeff);

      for (unsigned i = 0; i < VF; ++i) {
        CA.push_back(ConstantInt::getSigned(Int64Ty, ConstCoeff * i));
      }
      ArrayRef<Constant *> AR(CA);
      auto CV = ConstantVector::get(AR);

      if (BlobCoeff != InvalidBlobIndex) {
        // Compute Addend = WidenedBlob * CV and add Addend to the canon expression
        NestedBlobCG CGBlob(Ref, MainLoop->getHLNodeUtils(),
                            WideRef->getDDRefUtils(), this, nullptr);

        auto NewRef = CGBlob.visit(WideRef->getBlobUtils().getBlob(BlobCoeff));
        auto CRef  = Ref->getDDRefUtils().createConstDDRef(CV);

        auto TWideInst = MainLoop->getHLNodeUtils().createBinaryHLInst(Instruction::Mul, 
                                                                       NewRef->clone(), CRef, ".BlobMul");
        addInst(TWideInst, nullptr);
        AuxRefs.push_back(TWideInst->getLvalDDRef());
        CE->addBlob(TWideInst->getLvalDDRef()->getSingleCanonExpr()->getSingleBlobIndex(), 1);
      } else {
        unsigned Idx = 0;
        CE->getBlobUtils().createBlob(CV, true, &Idx);
        CE->addBlob(Idx, 1);
      }
    }

    for (auto &BI : BlobIndices) {
      auto TopBlob = CE->getBlobUtils().getBlob(BI);

      // We do not need to widen invariant blobs - check for blob invariance
      // by comparing maxbloblevel against the loop's nesting level.
      if (WideRef->findMaxBlobLevel(BI) < NestingLevel)
        continue;

      if (CE->getBlobUtils().isNestedBlob(TopBlob)) {
        NestedBlobCG CGBlob(Ref, MainLoop->getHLNodeUtils(),
                            WideRef->getDDRefUtils(), this, nullptr);

        auto NewRef = CGBlob.visit(TopBlob);

        AuxRefs.push_back(NewRef);
        CE->replaceBlob(BI, NewRef->getSingleCanonExpr()->getSingleBlobIndex());
        continue;
      }

      assert(CE->getBlobUtils().isTempBlob(TopBlob) &&
             "Only temp blobs expected");

      auto OldSymbase = CE->getBlobUtils().getTempBlobSymbase(BI);

      // A temp blob not widened before is a loop invariant - it will be broadcast
      // in HIRCG when needed.
      if (WidenMap.find(OldSymbase) != WidenMap.end()) {
        auto WInst1 = WidenMap[OldSymbase];
        auto WRef = WInst1->getLvalDDRef();
        AuxRefs.push_back(WRef);
        CE->replaceBlob(BI, WRef->getSingleCanonExpr()->getSingleBlobIndex());
      }
    }

    auto VecCEDestTy = VectorType::get(CE->getDestType(), VF);
    auto VecCESrcTy = VectorType::get(CE->getSrcType(), VF);

    CE->setDestType(VecCEDestTy);
    CE->setSrcType(VecCESrcTy);
  }

  // The blobs in the scalar ref have been replaced by widened refs, call
  // the utility to update the widened Ref consistent.
  WideRef->makeConsistent(&AuxRefs, NestingLevel);
  return WideRef;
}

/// \brief Return result of combining horizontal vector binary operation with
/// initial value. Instead of splitting VecRef recursively into 2 parts of half
/// VF until the VF becomes 2, VecRef is shuffled in such a way that the
/// resulting vector stays VF-wide and the upper elements are shuffled down
/// into the lower positions to form a new vector. Then, the horizontal
/// operation is performed on VF-wide vectors, but the upper elements of the
/// operation are simply ignored. The final result of the horizontal operation
/// is then extracted from position 0, the leftmost position of the vector.
/// The rightmost position is 7. E.g., if VF=8, we will have 3 horizontal
/// operation stages. So, if we start with elements <0,2,1,4,5,1,3,0> and the
/// horizontal operation is add, we end up with the following sequence of
/// operations, where u is undefined.
///
/// <0,2,1,4,5,1,3,0> + <5,1,3,0,u,u,u,u> = <5,3,4,4,u,u,u,u>
/// Now, only the first 4 elements are considered for the next stage.
/// <5,3,4,4,u,u,u,u> + <4,4,u,u,u,u,u,u> = <9,7,u,u,u,u,u,u>
/// And, now just two.
/// <9,7,u,u,u,u,u,u> + <7,u,u,u,u,u,u,u> = <16,u,u,u,u,u,u,u>
/// extract<0> = 16 and add this value to the initial value.
///
/// This transformation is necessary so that X86/Target SAD idiom for
/// reductions is enabled.
static HLInst *buildReductionTail(HLContainerTy &InstContainer,
                                  unsigned BOpcode, const RegDDRef *VecRef,
                                  RegDDRef *InitValRefClone, HLLoop *HLLp,
                                  RegDDRef *ResultRefClone) {

  // Take Vector Length from the WideRedInst type
  Type *VecTy = VecRef->getDestType();

  // For Sub/FSub operation, we need to use Add/FAdd for the horizontal
  // vector and combine operations.
  if (BOpcode == Instruction::Sub)
    BOpcode = Instruction::Add;
  else if (BOpcode == Instruction::FSub)
    BOpcode = Instruction::FAdd;

  unsigned VF = cast<VectorType>(VecTy)->getNumElements();
  unsigned Stages = Log2_32(VF);
  unsigned MaskElems = VF / 2;
  const RegDDRef *LastVal = VecRef;
  const Loop *Lp = HLLp->getLLVMLoop();
  LLVMContext &Context = Lp->getHeader()->getContext();
  for (unsigned i = 0; i < Stages; i++) {
    SmallVector<Constant *, 16> ShuffleMask;
    unsigned MaskElemVal = MaskElems;
    for (unsigned j = 0; j < VF; j++) {
      if (j > MaskElems - 1) {
        // Use undef for the mask when we don't care what the result of the
        // horizontal operation is for that element.
        Constant *UndefVal = UndefValue::get(Type::getInt32Ty(Context));
        ShuffleMask.push_back(UndefVal);
      } else {
        Constant *Mask = ConstantInt::get(Type::getInt32Ty(Context),
                                          MaskElemVal);
        ShuffleMask.push_back(Mask);
        MaskElemVal++;
      }
    }
    MaskElems /= 2;
    Constant *MaskVec = ConstantVector::get(ShuffleMask);
    RegDDRef *MaskVecDDRef = VecRef->getDDRefUtils().createConstDDRef(MaskVec);
    HLInst *Shuffle = HLLp->getHLNodeUtils().createShuffleVectorInst(
        LastVal->clone(), LastVal->clone(), MaskVecDDRef, "rdx.shuf");
    HLInst *BinOp = HLLp->getHLNodeUtils().createBinaryHLInst(
        BOpcode, LastVal->clone(), Shuffle->getLvalDDRef()->clone(), "bin.rdx");
    InstContainer.push_back(*Shuffle);
    InstContainer.push_back(*BinOp);
    LastVal = BinOp->getLvalDDRef();
  }

  HLInst *Extract = HLLp->getHLNodeUtils().createExtractElementInst(
      LastVal->clone(), 0, "bin.final", ResultRefClone);
  InstContainer.push_back(*Extract);

  return Extract;
}

void VPOCodeGenHIR::analyzeCallArgMemoryReferences(
    const HLInst *OrigCall, HLInst *WideCall,
    SmallVectorImpl<RegDDRef *> &Args) {

  Instruction *Inst = const_cast<Instruction *>(WideCall->getLLVMInstruction());

  CallInst *VecCall = cast<CallInst>(Inst);

  HLLoop *L = cast<HLLoop>(OrigCall->getParentLoop());
  unsigned LoopLevel = L->getNestingLevel();

  // Analyze memory references for the arguments used to store sin/cos
  // results. This information will later be used to generate appropriate
  // store instructions.

  for (unsigned I = 0; I < Args.size(); I++) {

    // Only consider call arguments that involve address computations.
    // For example, this is limited at the moment to call arguments like:
    // sincos(..., &a[i], &b[i], ...). In order to extend to other memory
    // references, the type derivations below will need to change. Some
    // assumptions are made for addressOf references.
    if (Args[I]->isAddressOf()) {
      AttrBuilder AttrList;
      int64_t ByteStride;

      if (Args[I]->getConstStrideAtLevel(LoopLevel, &ByteStride)) {
        // Type of the argument will be something like <4 x double*>
        // The following code will yield a type of double. This type is used
        // to determine the stride in elements.
        Type *ArgTy = Args[I]->getDestType();
        PointerType *PtrTy = cast<PointerType>(ArgTy);
        VectorType *VecTy = cast<VectorType>(PtrTy->getElementType());
        PointerType *ElemPtrTy = cast<PointerType>(VecTy->getElementType());
        Type *ElemTy = ElemPtrTy->getElementType();
        unsigned ElemSize = ElemTy->getPrimitiveSizeInBits() / 8;
        unsigned ElemStride = ByteStride / ElemSize;
        AttrList.addAttribute("stride",
                              APInt(32, ElemStride).toString(10, false));
      } else {
        AttrList.addAttribute("stride", "indirect");
      }

      if (AttrList.hasAttributes()) {
        VecCall->setAttributes(VecCall->getAttributes().addAttributes(
            VecCall->getContext(), I + 1, AttrList));
      }
    }
  }
}

HLInst *VPOCodeGenHIR::widenIfPred(const HLIf *HIf, RegDDRef *Mask) {
  RegDDRef *WOp0, *WOp1;

  if (!Mask)
    Mask = CurMaskValue;

  // TODO - supports only single if predicate for now
  auto FirstPred = HIf->pred_begin();
  auto Op0 = HIf->getOperandDDRef(0);
  auto Op1 = HIf->getOperandDDRef(1);
  WOp0 = widenRef(Op0);
  WOp1 = widenRef(Op1);

  assert((WOp0 && WOp1) && "Unexpected null widened IF predicate operand(s)");
  auto WideInst =
      HIf->getHLNodeUtils().createCmp(*FirstPred, WOp0, WOp1, "wide.cmp.");
  addInst(WideInst, Mask);
  // For the search loop simply generate HLIf instruction and move
  // InsertionPoint into then-branch of that HLIf.
  // Blindly assume that this if is for exit block.
  // TODO: For general case of early exit vectorization, this code is not
  // correct for all ifs and it has to be changed.
  if (isSearchLoop()) {
    RegDDRef *Ref = WideInst->getLvalDDRef();
    Type *Ty = Ref->getDestType();
    LLVMContext &Context = Ty->getContext();
    Type *IntTy = IntegerType::get(Context, Ty->getPrimitiveSizeInBits());
    HLInst *BitCastInst = createBitCast(IntTy, Ref, "intmask");
    createHLIf(PredicateTy::ICMP_NE, BitCastInst->getLvalDDRef()->clone(),
               WideInst->getDDRefUtils().createConstDDRef(IntTy, 0));
  }
  return WideInst;
}

// Generate cttz(bsf) call for a given Ref.
//    %intmask = bitcast.i32(Ref);
//    %bsf = call intrinsic.cttz.i32(Ref);
HLInst *VPOCodeGenHIR::createCTTZCall(RegDDRef *Ref, const Twine &Name) {
  assert(Ref && "Ref is expected for this assignment.");

  HLNodeUtils &HNU = Ref->getHLDDNode()->getHLNodeUtils();
  DDRefUtils &DDRU = Ref->getHLDDNode()->getDDRefUtils();
  // It's necessary to bitcast mask to integer, otherwise it's not possible to
  // use it in cttz instruction.
  Type *RefTy = Ref->getDestType();
  LLVMContext &Context = RefTy->getContext();
  Type *IntTy = IntegerType::get(Context, RefTy->getPrimitiveSizeInBits());
  HLInst *BitCastInst = createBitCast(IntTy, Ref, Name + "intmask");

  RegDDRef *IntRef = BitCastInst->getLvalDDRef();
  // As soon as this code has to be under if condition, return value from cttz
  // can be undefined if mask is zero.
  Type *BoolTy = IntegerType::get(Context, 1);
  Function *BsfFunc =
      Intrinsic::getDeclaration(&HNU.getModule(), Intrinsic::cttz, {IntTy});
  SmallVector<RegDDRef *, 1> Args = {IntRef->clone(),
                                     DDRU.createConstDDRef(BoolTy, 0)};
  HLInst *BsfCall = HNU.createCall(BsfFunc, Args, Name);
  addInstUnmasked(BsfCall);
  return BsfCall;
}

// Generate correct last value computation of the linear variable for early exit
// loops:
//    %intmask = bitcast.i32(mask);
//    %bsf = call intrinsic.cttz.i32(mask);
//    %live-out-value = %i1 + %bsf * %linear-stride;
//
HLInst *VPOCodeGenHIR::handleLiveOutLinearInEarlyExit(HLInst *INode,
                                                      RegDDRef *Mask) {
  HLInst *BsfCall = createCTTZCall(Mask);

  // Finally update linear reference by number of executed iterations.
  RegDDRef *LinRef = INode->getRvalDDRef();
  assert(!LinRef->isNonLinear() && "Linear RegDDRef is expected.");

  CanonExpr *CE = LinRef->getSingleCanonExpr();
  unsigned Level = MainLoop->getNestingLevel();

  RegDDRef *ZExtRef;
  if (CE->getSrcType() != BsfCall->getLvalDDRef()->getDestType())
    ZExtRef =
        createZExt(CE->getSrcType(), BsfCall->getLvalDDRef())->getLvalDDRef();
  else
    ZExtRef = BsfCall->getLvalDDRef();

  unsigned Index = ZExtRef->getSingleCanonExpr()->getSingleBlobIndex();
  if (CE->hasIVBlobCoeff(Level)) {
    BlobUtils *BU = &CE->getBlobUtils();
    BU->createMulBlob(BU->getBlob(CE->getIVBlobCoeff(Level)),
                      BU->getBlob(Index), true, &Index);
  }
  CE->addBlob(Index, CE->getIVConstCoeff(Level), true);

  addInstUnmasked(INode);
  SmallVector<const RegDDRef *, 1> AuxRefs = {ZExtRef};
  INode->getRvalDDRef()->makeConsistent(&AuxRefs, Level);
  return INode;
}

HLInst *VPOCodeGenHIR::widenNode(const HLInst *INode, RegDDRef *Mask) {
  const HLInst *Node = INode;
  auto CurInst = INode->getLLVMInstruction();
  SmallVector<RegDDRef *, 6> WideOps;

  // Widened values for SCEV expressions cannot be reused across HIR
  // instructions as temps that are part of such SCEV expressions can be
  // assigned to before a use in a later HIR instruction.
  // t1 =
  //    = zext(t1)
  // t1 =
  //    = zext(t1)
  // We clear the map at the start of widening of each HLInst.
  SCEVWideRefMap.clear();

  if (!Mask)
    Mask = CurMaskValue;

  HLInst *WideInst = nullptr;

  LLVM_DEBUG(INode->dump(true));
  bool InsertInMap = true;

  if (isSearchLoop() && INode->hasLval() &&
      INode->getLvalDDRef()->isTerminalRef() &&
      INode->getLvalDDRef()->isLiveOutOfParentLoop() && INode->hasRval() &&
      !INode->getRvalDDRef()->isNonLinear()) {
    return handleLiveOutLinearInEarlyExit(INode->clone(), Mask);
  }

  // Widen instruction operands
  for (auto Iter = (INode)->op_ddref_begin(), End = (INode)->op_ddref_end();
       Iter != End; ++Iter) {
    RegDDRef *WideRef;
    WideRef = widenRef(*Iter);
    WideOps.push_back(WideRef);
  }

  // Generate the widened instruction using widened operands
  if (auto BOp = dyn_cast<BinaryOperator>(CurInst)) {
    WideInst = Node->getHLNodeUtils().createBinaryHLInst(
        BOp->getOpcode(), WideOps[1], WideOps[2], CurInst->getName() + ".vec",
        WideOps[0], BOp);
  } else if (isa<LoadInst>(CurInst)) {
    WideInst = Node->getHLNodeUtils().createLoad(
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<StoreInst>(CurInst)) {
    WideInst = Node->getHLNodeUtils().createStore(
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
    InsertInMap = false;
  } else if (isa<CastInst>(CurInst)) {
    assert(WideOps.size() == 2 && "invalid cast");

    WideInst = Node->getHLNodeUtils().createCastHLInst(
        VectorType::get(CurInst->getType(), VF), CurInst->getOpcode(),
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<SelectInst>(CurInst)) {
    WideInst = Node->getHLNodeUtils().createSelect(
        INode->getPredicate(), WideOps[1], WideOps[2], WideOps[3], WideOps[4],
        CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<CmpInst>(CurInst)) {
    WideInst = Node->getHLNodeUtils().createCmp(
        INode->getPredicate(), WideOps[1], WideOps[2],
        CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<GetElementPtrInst>(CurInst)) {
    // Gep Instructions in LLVM may have any number of operands but the HIR
    // representation for them is always a single rhs ddref - copy rval to
    // lval.
    WideInst = Node->getHLNodeUtils().createCopyInst(
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
  } else if (const CallInst *Call = dyn_cast<CallInst>(CurInst)) {

    Function *Fn = Call->getCalledFunction();
    assert(Fn && "Unexpected null called function");
    StringRef FnName = Fn->getName();

    // Default to svml. If svml is not available, try the intrinsic.
    Intrinsic::ID ID = Intrinsic::not_intrinsic;
    if (!TLI->isFunctionVectorizable(FnName, VF)) {
      ID = getVectorIntrinsicIDForCall(Call, TLI);
      // FIXME: We should scalarize calls to @llvm.assume intrinsic instead of
      //        completely ignoring them.
      if (ID && (ID == Intrinsic::assume || ID == Intrinsic::lifetime_end ||
                 ID == Intrinsic::lifetime_start)) {
        return const_cast<HLInst *>(INode);
      }
    }

    unsigned ArgOffset = 0;
    if (!Fn->getReturnType()->isVoidTy()) {
      ArgOffset = 1;
    }
    SmallVector<RegDDRef *, 1> CallArgs;
    SmallVector<Type *, 1> ArgTys;
    for (unsigned i = ArgOffset; i < WideOps.size(); i++) {
      CallArgs.push_back(WideOps[i]);
      ArgTys.push_back(WideOps[i]->getDestType());
    }

    bool Masked = false;
    if (Mask) {
      auto CE = Mask->getSingleCanonExpr();
      ArgTys.push_back(CE->getDestType());
      CallArgs.push_back(Mask->clone());
      Masked = true;
    }

    Function *VectorF =
        getOrInsertVectorFunction(Call, VF, ArgTys, TLI, ID, nullptr, Masked);
    assert(VectorF && "Can't create vector function.");

    WideInst = Node->getHLNodeUtils().createCall(
        VectorF, CallArgs, VectorF->getName(), WideOps[0]);
    Instruction *Inst =
        const_cast<Instruction *>(WideInst->getLLVMInstruction());

    if (isa<FPMathOperator>(Inst)) {
      Inst->copyFastMathFlags(Call);
    }

    if (FnName.find("sincos") != StringRef::npos) {
      analyzeCallArgMemoryReferences(INode, WideInst, CallArgs);
    }

    if (ArgOffset) {
      // If this is a void function, there will be no LVal DDRef for it, so
      // don't try to insert it in the map. i.e., there are no users of an
      // LVal for a void function.
      InsertInMap = true;
    } else {
      InsertInMap = false;
    }
  } else {
    llvm_unreachable("Unimplemented widening for inst");
  }

  assert(WideInst && "Expected non-null widened instruction");
  // Add to WidenMap and handle generating code for any liveouts
  if (InsertInMap) {
    addToMapAndHandleLiveOut(INode->getLvalDDRef(), WideInst, MainLoop);
    if (WideInst->getLvalDDRef()->isTerminalRef())
      WideInst->getLvalDDRef()->makeSelfBlob();
  }

  addInst(WideInst, Mask);
  return WideInst;
}

HLInst *VPOCodeGenHIR::insertReductionInitializer(Constant *Iden,
                                                  RegDDRef *ScalarRednRef) {

  // ScalarRednRef is the initial value that is assigned to result of the
  // reduction operation. We are blending in the initial value into the
  // identity vector in lane 0.

  auto IdentityVec = getConstantSplatDDRef(MainLoop->getDDRefUtils(), Iden, VF);

  HLInst *InsertElementInst =
      MainLoop->getHLNodeUtils().createInsertElementInst(
          IdentityVec, ScalarRednRef, 0, "result.vector");
  HLNodeUtils::insertBefore(RednHoistLp, InsertElementInst);

  auto LvalSymbase = InsertElementInst->getLvalDDRef()->getSymbase();
  // Add the reduction ref as a live-in for each loop up to and including
  // the hoist loop.
  HLLoop *ThisLoop = MainLoop;
  while (ThisLoop != RednHoistLp->getParentLoop()) {
    ThisLoop->addLiveInTemp(LvalSymbase);
    ThisLoop = ThisLoop->getParentLoop();
  }
  return InsertElementInst;
}

void VPOCodeGenHIR::addToMapAndHandleLiveOut(const RegDDRef *ScalRef,
                                             HLInst *WideInst,
                                             HLLoop *HoistLp) {
  auto ScalSymbase = ScalRef->getSymbase();

  // If already in WidenMap, nothing further to do
  if (WidenMap.count(ScalSymbase))
    return;

  // Insert in WidenMap
  WidenMap[ScalSymbase] = WideInst;

  // Generate any necessary code to handle loop liveout/reduction
  if (!MainLoop->isLiveOut(ScalSymbase))
    return;

  auto VecRef = WideInst->getLvalDDRef();

  // Add the ref as a live-out for each loop up to and including the hoist loop.
  HLLoop *ThisLoop = MainLoop;
  while (ThisLoop != HoistLp->getParentLoop()) {
    ThisLoop->addLiveOutTemp(VecRef->getSymbase());
    ThisLoop = ThisLoop->getParentLoop();
  }

  unsigned OpCode;

  RegDDRef *FinalLvalRef = ScalRef->clone();
  if (isReductionRef(ScalRef, OpCode)) {
    HLContainerTy Tail;

    buildReductionTail(Tail, OpCode, VecRef, ScalRef->clone(), MainLoop,
                       FinalLvalRef);
    HLNodeUtils::insertAfter(RednHoistLp, &Tail);
  } else {
    auto Extr = WideInst->getHLNodeUtils().createExtractElementInst(
        VecRef->clone(), VF - 1, "Last", FinalLvalRef);

    HLNodeUtils::insertAfter(MainLoop, Extr);
  }

  // The original ScalarRef may have linear/IV information from the loop. Final
  // value is computed after the loop, make the final lval ref a selfblob.
  if (FinalLvalRef->isTerminalRef()) {
    assert(FinalLvalRef->isLval() && "DDRef is expected to be an lval ref!");
    FinalLvalRef->makeSelfBlob();
  }
}
} // end namespace llvm

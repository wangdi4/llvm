//===----- IntelVPOCodeGenHIR.cpp -----------------------------------------===//
//
//   Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
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
#include "../IntelVPlanUtils.h"
#include "../IntelVPlanVLSAnalysis.h"
#include "IntelVPlanHCFGBuilderHIR.h"
#include "IntelVPlanVLSClientHIR.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"
#include "llvm/Transforms/Utils/GeneralUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#if INTEL_INCLUDE_DTRANS
#include "Intel_DTrans/Transforms/DTransPaddedMalloc.h"
#endif // INTEL_INCLUDE_DTRANS

#define DEBUG_TYPE "VPOCGHIR"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::vpo;

STATISTIC(LoopsVectorized, "Number of HIR loops vectorized");

static cl::opt<bool>
    DisableStressTest("disable-vplan-stress-test", cl::init(false), cl::Hidden,
                      cl::desc("Disable VPO Vectorizer Stress Testing"));

static cl::opt<bool> EnableNestedBlobVec(
    "enable-nested-blob-vec", cl::init(true), cl::Hidden,
    cl::desc("Enable vectorization of loops with nested blobs"));

static cl::opt<bool> EnableBlobCoeffVec(
    "enable-blob-coeff-vec", cl::init(true), cl::Hidden,
    cl::desc("Enable vectorization of loops with blob IV coefficients"));

static cl::opt<bool> EnableVPlanVLSCG("enable-vplan-vls-cg", cl::init(true),
                                      cl::Hidden,
                                      cl::desc("Enable VLS code generation"));

/// Command line switches to control VLS optimizations
static cl::opt<bool>
    EnableVPlanVLSLoads("enable-vplan-vls-loads", cl::init(true), cl::Hidden,
                        cl::desc("Enable VLS optimization for loads"));

static cl::opt<bool>
    EnableVPlanVLSStores("enable-vplan-vls-stores", cl::init(true), cl::Hidden,
                         cl::desc("Enable VLS optimization for stores"));

static cl::opt<int>
    VPlanVLSNumLoops("vplan-vls-num-loops", cl::init(-1), cl::Hidden,
                     cl::desc("If non-negative, limit VLS optimization to "
                              "specified number of vectorized loops"));

static cl::opt<bool> EnableFirstIterPeelMEVec(
    "enable-first-it-peel-me-vec", cl::init(true), cl::Hidden,
    cl::desc(
        "Enable first iteration peel loop for vectorized multi-exit loops."));

static cl::opt<bool> EnablePeelMEVec(
    "enable-peel-me-vec", cl::init(true), cl::Hidden,
    cl::desc("Enable peel loop for vectorized multi-exit loops."));

// Force full VPValue based code generation.
static cl::opt<bool> EnableVPValueCodegenHIR(
    "enable-vp-value-codegen-hir", cl::init(false), cl::Hidden,
    cl::desc("Enable VPValue based codegen for HIR vectorizer"));

extern cl::opt<bool> AllowMemorySpeculation;

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
    UMinOp,
    SMinOp,
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
    IsThenChild ? HLNodeUtils::insertAsLastThenChild(InsertRegion, Inst)
                : HLNodeUtils::insertAsLastElseChild(InsertRegion, Inst);
  }

  RegDDRef *codegenStandAloneBlob(const SCEV *SC);
  RegDDRef *codegenNAryOp(const SCEVNAryExpr *SC, unsigned OpCode);
  RegDDRef *codegenUDivOp(const SCEVUDivExpr *SC);
  RegDDRef *codegenCoeff(Constant *Const);
  RegDDRef *codegenCoeff(int64_t Coeff, Type *Ty);
  RegDDRef *codegenConversion(RegDDRef *Src, unsigned ConvOpCode,
                              Type *DestType);

public:
  NestedBlobCG(const RegDDRef *R, HLNodeUtils &H, DDRefUtils &D,
               VPOCodeGenHIR *C, RegDDRef *M)
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
  RegDDRef *visitSMinExpr(const SCEVSMinExpr *Expr);
  RegDDRef *visitUMinExpr(const SCEVUMinExpr *Expr);
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

  RegDDRef *WideRef = ACG->getWideRef(BDDR->getSymbase());
  if (!WideRef) {
    WideRef = DDRU.createScalarRegDDRef(BDDR->getSymbase(),
                                        BDDR->getSingleCanonExpr()->clone());
    WideRef = ACG->widenRef(WideRef, ACG->getVF());
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

    if (OpCode == UMaxOp || OpCode == SMaxOp ||
        OpCode == UMinOp || OpCode == SMinOp) {
      const CmpInst::Predicate Pred = [&OpCode]() {
        switch (OpCode) {
        case UMaxOp:
          return CmpInst::ICMP_UGT;
        case SMaxOp:
          return CmpInst::ICMP_SGT;
        case UMinOp:
          return CmpInst::ICMP_ULT;
        case SMinOp:
          return CmpInst::ICMP_SLT;
        default:
          llvm_unreachable("Unimplemented new OpCode");
        }
      } ();
      WideInst =
          HNU.createSelect(Pred, CurDDRef->clone(), InnerDDRef->clone(),
                           CurDDRef->clone(), InnerDDRef->clone(), "NAry");
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

RegDDRef *NestedBlobCG::visitSMinExpr(const SCEVSMinExpr *Expr) {
  return codegenNAryOp(Expr, SMinOp);
}

RegDDRef *NestedBlobCG::visitUMinExpr(const SCEVUMinExpr *Expr) {
  return codegenNAryOp(Expr, UMinOp);
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

    // Handling liveouts for privates/inductions is not implemented in
    // VPValue-based CG. The checks below enable the following -
    // 1. For full VPValue-based CG all reductions are supported.
    // 2. For mixed mode CG, minmax reductions are not supported if VPLoopEntity
    // representation is not used.
    // TODO: Revisit when VPLoopEntity representation for reductions is made
    // default.
    auto TLval = Inst->getLvalDDRef();
    if (TLval && TLval->isTerminalRef() &&
        OrigLoop->isLiveOut(TLval->getSymbase())) {
      unsigned RedOpcode = 0;
      if (CG->isReductionRef(TLval, RedOpcode)) {
        if (EnableVPValueCodegenHIR)
          // VPValue-based CG for reductions is supported.
          return;
        // Min/max reductions are not supported without VPLoopEntities.
        if (!VPlanUseVPEntityInstructions &&
            (RedOpcode == Instruction::Select ||
             RedOpcode == VPInstruction::UMin ||
             RedOpcode == VPInstruction::UMax ||
             RedOpcode == VPInstruction::SMin ||
             RedOpcode == VPInstruction::SMax ||
             RedOpcode == VPInstruction::FMin ||
             RedOpcode == VPInstruction::FMax))
          IsHandled = false;
      } else if (EnableVPValueCodegenHIR)
        IsHandled = false;

      if (!IsHandled) {
        LLVM_DEBUG(Inst->dump());
        LLVM_DEBUG(
            dbgs() << "VPLAN_OPTREPORT: VPValCG liveout induction/private or "
                      "min/max reduction not handled\n");
        return;
      }
    }

    unsigned MaskedRedOpcode = 0;
    if (!CG->isSearchLoop() && TLval && TLval->isTerminalRef() &&
        OrigLoop->isLiveOut(TLval->getSymbase()) &&
        Inst->getParent() != OrigLoop &&
        !CG->isReductionRef(TLval, MaskedRedOpcode)) {
      LLVM_DEBUG(Inst->dump());
      LLVM_DEBUG(
          dbgs()
          << "VPLAN_OPTREPORT: Liveout conditional non-reduction scalar assign "
             "not handled\n");
      IsHandled = false;
      return;
    }

    if (const CallInst *Call = Inst->getCallInst()) {
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

      // These intrinsics need the second argument to remain scalar(consequently
      // loop invariant). Support to be added later.
      if (ID == Intrinsic::ctlz || ID == Intrinsic::cttz ||
          ID == Intrinsic::powi) {
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
inline bool HandledCheck::isUniform(const RegDDRef *Ref) const {
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

bool VPOCodeGenHIR::initializeVectorLoop(unsigned int VF) {
  assert(VF > 1);
  setVF(VF);

  LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: VPlan handled loop, VF = " << VF << " "
                    << Fn.getName() << "\n");
  LLVM_DEBUG(dbgs() << "Handled loop before vec codegen: \n");
  LLVM_DEBUG(OrigLoop->dump());

  LoopsVectorized++;
  SRA->computeSafeReductionChains(OrigLoop);

  const SafeRedInfoList &SRCL = SRA->getSafeRedInfoList(OrigLoop);
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

  // Setup peel, main and remainder loops
  // TODO: Peeling decisions should be properly made in VPlan's cost model and
  // not during code generation. The following logic is a temporary workaround
  // to have peeling functionality working for vectorized search loops.

  bool SearchLoopNeedsPeeling =
      TTI->isAdvancedOptEnabled(
          TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2) &&
      (EnableFirstIterPeelMEVec || EnablePeelMEVec) && isSearchLoop() &&
      (getSearchLoopType() == VPlanIdioms::SearchLoopStrEq ||
       getSearchLoopType() == VPlanIdioms::SearchLoopStructPtrEq);

  if (isSearchLoop() &&
      getSearchLoopType() == VPlanIdioms::SearchLoopStructPtrEq) {
    assert(SearchLoopPeelArrayRef &&
           "StructPtrEq search loop does not have peel array ref.\n");
  }

  // We cannot peel any iteration of the loop when the trip count is constant
  // and lower or equal than VF since we have already made the decision of
  // vectorizing that loop with such a VF.
  uint64_t TripCount = 0;
  bool IsTCValidForPeeling =
      OrigLoop->isConstTripLoop(&TripCount) && TripCount <= VF ? false : true;
  if (SearchLoopNeedsPeeling && !IsTCValidForPeeling)
    LLVM_DEBUG(dbgs() << "Can't peel loop : VF(" << VF << ") >= TC("
                      << TripCount << ")!\n");

  bool NeedPeelLoop = SearchLoopNeedsPeeling & IsTCValidForPeeling;
  bool NeedRemainderLoop = false;
  HLLoop *PeelLoop = nullptr;
  SmallVector<std::tuple<HLPredicate, RegDDRef *, RegDDRef *>, 2> RTChecks;
  // Generate padding runtime check if OrigLoops requires it. Otherwise, nothing
  // is added to RTChecks.
  addPaddingRuntimeCheck(RTChecks);

  // Specialized first iteration peeling is needed only for the SearchLoopStrEq
  // idiom.
  bool NeedFirstIterationPeelLoop =
      NeedPeelLoop && getSearchLoopType() == VPlanIdioms::SearchLoopStrEq;

  auto MainLoop = HIRTransformUtils::setupPeelMainAndRemainderLoops(
      OrigLoop, VF, NeedRemainderLoop, LORBuilder, OptimizationType::Vectorizer,
      &PeelLoop, NeedFirstIterationPeelLoop, SearchLoopPeelArrayRef, &RTChecks);

  if (!MainLoop) {
    assert(false && "Main loop could not be setup.");
    // Bailout for prod builds
    return false;
  }

  if (PeelLoop) {
    setNeedPeelLoop(NeedPeelLoop);
    setPeelLoop(PeelLoop);
    if (TripCount > 0) {
      assert(TripCount > 1 && "Expected trip-count > 1!");
      if (NeedFirstIterationPeelLoop)
        setTripCount(TripCount - 1);
      // TODO : What about generic peel loop?
    }
  }

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

  return true;
}

void VPOCodeGenHIR::finalizeVectorLoop(void) {
  LLVM_DEBUG(dbgs() << "\n\n\nHandled loop after: \n");
  if (NeedPeelLoop)
    LLVM_DEBUG(PeelLoop->dump());
  LLVM_DEBUG(MainLoop->getParent()->dump());
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
      MainLoop->markDoNotUnroll();
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

    if (auto *Inst = HInst->getIntrinCall()) {
      Intrinsic::ID IntrinID = Inst->getIntrinsicID();

      if (vpo::VPOAnalysisUtils::isRegionDirective(IntrinID)) {
        StringRef DirStr = vpo::VPOAnalysisUtils::getDirectiveString(
            const_cast<IntrinsicInst *>(Inst));

        int DirID = vpo::VPOAnalysisUtils::getDirectiveID(DirStr);

        if (BeginOrEndDirIDs.count(DirID))
          HLNodeUtils::remove(HInst);
      }
    }
  }
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

  const CallInst *Call = HInst->getCallInst();
  assert(Call && "Unexpected null call");
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
      WideRef->makeConsistent(AuxRefs, OrigLoop->getNestingLevel());

      // Collect call arguments and types so that the function declaration
      // and call instruction can be generated.
      CallArgs.push_back(WideRef);
      ArgTys.push_back(VecDestTy);
    }

    // Using the newly created vector call arguments, generate the vector
    // call instruction and extract the low element.
    Function *VectorF = getOrInsertVectorFunction(
        F, VF, ArgTys, TLI, Intrinsic::not_intrinsic, nullptr /*simd function*/,
        false /*non-masked*/);
    assert(VectorF && "Can't create vector function.");

    HLInst *WideCall = HInst->getHLNodeUtils().createCall(
        VectorF, CallArgs, VectorF->getName(), nullptr);
    HLNodeUtils::insertBefore(HInst, WideCall);

    Instruction *Inst =
        const_cast<Instruction *>(WideCall->getLLVMInstruction());

    // Make sure we don't lose attributes at the call site. E.g., IMF
    // attributes are taken from call sites in MapIntrinToIml to refine
    // SVML calls for precision.
    cast<CallInst>(Inst)->setAttributes(Call->getAttributes());

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

    // Set calling conventions for SVML function calls
    if (isSVMLFunction(TLI, FnName, VectorF->getName())) {
      Instruction *WideInst =
          const_cast<Instruction *>(WideCall->getLLVMInstruction());
      cast<CallInst>(WideInst)->setCallingConv(CallingConv::SVML);
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

  // Check for explicit SIMD reductions.
  if (auto *Descr = HIRLegality->isReduction(Ref)) {
    Opcode = VPReduction::getReductionOpcode(Descr->Kind, Descr->MMKind);
    return true;
  }
  // Check for minmax+index idiom.
  if (HIRLegality->isMinMaxIdiomTemp(Ref, OrigLoop)) {
    Opcode = Instruction::Select;
    return true;
  }
  // Check for auto-recognized SafeReduction.
  return SRA->isReductionRef(Ref, Opcode);
}

static void setRefAlignment(Type *ScalRefTy, RegDDRef *WideRef) {
  // When the original ref does not have alignment information
  // LLVM defaults to the ABI alignment for the ref's type. During widening,
  // we need to set the widened ref's alignment to the ABI alignment for
  // the scalar ref's type for such cases.
  unsigned Alignment = WideRef->getAlignment();
  if (!Alignment) {
    auto DL = WideRef->getDDRefUtils().getDataLayout();
    Alignment = DL.getABITypeAlignment(ScalRefTy);
    WideRef->setAlignment(Alignment);
  }
}

RegDDRef *VPOCodeGenHIR::widenRef(const RegDDRef *Ref, unsigned VF,
                                  bool InterLeaveAccess) {
  RegDDRef *WideRef;
  auto RefDestTy = Ref->getDestType();
  auto VecRefDestTy = VectorType::get(RefDestTy, VF);
  auto RefSrcTy = Ref->getSrcType();
  auto VecRefSrcTy = VectorType::get(RefSrcTy, VF);

  // If the DDREF has a widened counterpart, return the same after setting
  // SrcType/DestType appropriately.
  if (Ref->isTerminalRef()) {
    unsigned RedOpCode;

    if ((WideRef = getWideRef(Ref->getSymbase()))) {
      WideRef = WideRef->clone();

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
    } else {
      WideRef->setBitCastDestType(PointerType::get(VecRefDestTy, AddressSpace));
      setRefAlignment(RefDestTy, WideRef);
    }
  }

  unsigned NestingLevel = OrigLoop->getNestingLevel();
  // For unit stride ref, nothing else to do. We assume unit stride for
  // interleaved access.
  if (isUnitStrideRef(Ref) || InterLeaveAccess)
    return WideRef;

  SmallVector<const RegDDRef *, 4> AuxRefs;
  RegDDRef::CanonExprsTy WideRefCEs;
  if (WideRef->hasGEPInfo()) {
    // If the base CE of this memref is loop variant then add it to the list
    // of CEs to be updated for widening.
    if (!WideRef->getBaseCE()->isInvariantAtLevel(NestingLevel)) {
      WideRefCEs.push_back(WideRef->getBaseCE());
    }
  }
  WideRefCEs.append(WideRef->canon_begin(), WideRef->canon_end());

  // For cases other than unit stride refs, we need to widen the induction
  // variable and replace blobs in Canon Expr with widened equivalents.
  for (auto CE : WideRefCEs) {

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
        // Compute Addend = WidenedBlob * CV and add Addend to the canon
        // expression
        NestedBlobCG CGBlob(Ref, MainLoop->getHLNodeUtils(),
                            WideRef->getDDRefUtils(), this, nullptr);

        auto NewRef = CGBlob.visit(WideRef->getBlobUtils().getBlob(BlobCoeff));
        auto CRef = Ref->getDDRefUtils().createConstDDRef(CV);

        auto TWideInst = MainLoop->getHLNodeUtils().createBinaryHLInst(
            Instruction::Mul, NewRef->clone(), CRef, ".BlobMul");
        addInst(TWideInst, nullptr);
        AuxRefs.push_back(TWideInst->getLvalDDRef());
        CE->addBlob(TWideInst->getLvalDDRef()
                        ->getSingleCanonExpr()
                        ->getSingleBlobIndex(),
                    1);
      } else {
        unsigned Idx = 0;
        CE->getBlobUtils().createConstantBlob(CV, true, &Idx);
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

      // A temp blob not widened before is a loop invariant - it will be
      // broadcast in HIRCG when needed.
      if (auto *WRef = getWideRef(OldSymbase)) {
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
  WideRef->makeConsistent(AuxRefs, NestingLevel);
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
  LLVMContext &Context = HLLp->getHLNodeUtils().getContext();
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
        Constant *Mask =
            ConstantInt::get(Type::getInt32Ty(Context), MaskElemVal);
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

/// \brief Create a call to llvm.experimental.vector.reduce intrinsic in order
/// to perform horizontal reduction on vector ref \p VecRef. The scalar return
/// value of this intrinsic call is stored back to the reduction descriptor
/// variable \p RednDescriptor. Overloaded parameters of intrinsic call is
/// determined based on type of reduction. Some examples -
/// 1. Integer add reduction for VF=4 will generate -
///    declare i32 @llvm.experimental.vector.reduce.add.i32.v4i32(<4 x i32>)
///
/// 2. FP add reduction for VF=4 will generate -
///    declare float @llvm.experimental.vector.reduce.fadd.f32.f32.v4f32(float,
///                                                                 <4 x float>)
static HLInst *createVectorReduce(Intrinsic::ID VecRedIntrin, RegDDRef *VecRef,
                                  RegDDRef *&Acc, RegDDRef *RednDescriptor,
                                  HLNodeUtils &HNU) {
  assert(isa<VectorType>(VecRef->getDestType()) &&
         "Ref to reduce is not a vector.");

  // 1. For FP reductions the accumulator's type is also needed
  // 2. All reduction intrinsics have vector type
  SmallVector<Type *, 2> Tys;
  Type *VecType = VecRef->getDestType();
  SmallVector<RegDDRef *, 2> Ops;

  switch (VecRedIntrin) {
  case Intrinsic::experimental_vector_reduce_v2_fadd:
  case Intrinsic::experimental_vector_reduce_v2_fmul:
    assert(Acc && "Expected initial value");
    assert(!isa<VectorType>(Acc->getDestType()) &&
           "Accumulator for FP reduction is not scalar.");
    Tys.insert(Tys.end(), {Acc->getDestType(), VecType});
    Ops.insert(Ops.end(), {Acc, VecRef});
    Acc = nullptr;
    break;
  case Intrinsic::experimental_vector_reduce_add:
  case Intrinsic::experimental_vector_reduce_mul:
  case Intrinsic::experimental_vector_reduce_and:
  case Intrinsic::experimental_vector_reduce_or:
  case Intrinsic::experimental_vector_reduce_xor:
    Tys.insert(Tys.end(), {VecType});
    Ops.insert(Ops.end(), {VecRef});
    break;
  case Intrinsic::experimental_vector_reduce_umax:
  case Intrinsic::experimental_vector_reduce_smax:
  case Intrinsic::experimental_vector_reduce_umin:
  case Intrinsic::experimental_vector_reduce_smin:
    assert(!Acc && "Unexpected initial value");
    // Since we use generic IRBuilder::CreateCall interface in HIR, signedness
    // does not need to be explicitly specified.
    Tys.insert(Tys.end(), {VecType});
    Ops.insert(Ops.end(), {VecRef});
    break;
  case Intrinsic::experimental_vector_reduce_fmax:
  case Intrinsic::experimental_vector_reduce_fmin:
    assert(!Acc && "Unexpected initial value");
    // TODO: Need processing to determine NoNaN.
    return HNU.createFPMinMaxVectorReduce(VecRef, VecRedIntrin, false /*NoNaN*/,
                                          RednDescriptor);
  default:
    llvm_unreachable("unsupported reduction");
    break;
  }

  Function *VecReduceFunc =
      Intrinsic::getDeclaration(&HNU.getModule(), VecRedIntrin, Tys);
  LLVM_DEBUG(dbgs() << "Vector reduce func: "; VecReduceFunc->dump());

  return HNU.createCall(VecReduceFunc, Ops, "vec.reduce", RednDescriptor);
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

HLInst *VPOCodeGenHIR::widenPred(const HLIf *HIf,
                                 HLIf::const_pred_iterator PredIt,
                                 RegDDRef *Mask) {

  auto PredLHS = HIf->getPredicateOperandDDRef(PredIt, true);
  auto PredRHS = HIf->getPredicateOperandDDRef(PredIt, false);
  RegDDRef *WideLHS = widenRef(PredLHS, getVF());
  RegDDRef *WideRHS = widenRef(PredRHS, getVF());

  assert((WideLHS && WideRHS) &&
         "Unexpected null widened IF predicate operand(s)");
  auto WideCmpInst =
      HIf->getHLNodeUtils().createCmp(*PredIt, WideLHS, WideRHS, "wide.cmp.");
  // Add the new wide compare instruction
  addInst(WideCmpInst, Mask);
  return WideCmpInst;
}

HLInst *VPOCodeGenHIR::widenIfNode(const HLIf *HIf, RegDDRef *Mask) {
  if (!Mask)
    Mask = CurMaskValue;

  auto FirstPred = HIf->pred_begin();
  HLInst *CurWideInst = widenPred(HIf, FirstPred, Mask);
  LLVM_DEBUG(dbgs() << "VPCodegen: First Pred WideInst: "; CurWideInst->dump());

  // Create Cmp HLInsts for remaining predicates in the HLIf
  for (auto It = HIf->pred_begin() + 1, E = HIf->pred_end(); It != E; ++It) {
    HLInst *NewWideInst = widenPred(HIf, It, Mask);
    LLVM_DEBUG(dbgs() << "VPCodegen: NewWideInst: "; NewWideInst->dump());
    // Conjoin the new wideInst with the current wideInst using implicit AND
    CurWideInst = HIf->getHLNodeUtils().createAnd(
        CurWideInst->getLvalDDRef()->clone(),
        NewWideInst->getLvalDDRef()->clone(), "wide.and.");
    LLVM_DEBUG(dbgs() << "VPCodegen: CurWideInst: "; CurWideInst->dump());
    addInst(CurWideInst, Mask);
  }

  assert(CurWideInst && "No HLInst generated for HLIf?");

  // For the search loop simply generate HLIf instruction and move
  // InsertionPoint into then-branch of that HLIf.
  // Blindly assume that this if is for exit block.
  // TODO: For general case of early exit vectorization, this code is not
  // correct for all ifs and it has to be changed.
  if (isSearchLoop()) {
    assert(
        HIf->getNumPredicates() == 1 &&
        "Search loop vectorization handles HLIfs with single predicate only.");
    RegDDRef *Ref = CurWideInst->getLvalDDRef();
    Type *Ty = Ref->getDestType();
    LLVMContext &Context = *Plan->getLLVMContext();
    Type *IntTy = IntegerType::get(Context, Ty->getPrimitiveSizeInBits());
    HLInst *BitCastInst = createBitCast(IntTy, Ref, "intmask");
    createHLIf(PredicateTy::ICMP_NE, BitCastInst->getLvalDDRef()->clone(),
               CurWideInst->getDDRefUtils().createConstDDRef(IntTy, 0));
  }

  return CurWideInst;
}

/// Create an interleave shuffle mask. This function mimics the interface in
/// VectorUtils.cpp which requires us to pass in an IRBuilder. This function
/// creates a shuffle mask for interleaving NumVecs vectors of vectorization
/// factor VF into a single wide vector. The mask is of the form: <0, VF, VF *
/// 2, ..., VF * (NumVecs - 1), 1, VF + 1, VF * 2 + 1, ...> For example, the
/// mask for VF = 4 and NumVecs = 2 is: <0, 4, 1, 5, 2, 6, 3, 7>.
static Constant *createInterleaveMask(LLVMContext &Context, unsigned VF,
                                      unsigned NumVecs) {
  SmallVector<Constant *, 16> Mask;
  for (unsigned I = 0; I < VF; ++I)
    for (unsigned J = 0; J < NumVecs; ++J)
      Mask.push_back(ConstantInt::get(Type::getInt32Ty(Context), J * VF + I));

  return ConstantVector::get(Mask);
}

/// Create a sequential shuffle mask. This function mimics the interface in
/// VectorUtils.cpp which requires us to pass in an IRBuilder. This function
/// creates shuffle mask whose elements are sequential and begin at Start. The
/// mask contains NumInts integers and is padded with NumUndefs undef values.
/// The mask is of the form: <Start, Start + 1, ... Start + NumInts - 1,
/// undef_1, ... undef_NumUndefs> For example, the mask for Start = 0, NumInsts
/// = 4, and NumUndefs = 4 is: <0, 1, 2, 3, undef, undef, undef, undef>
static Constant *createSequentialMask(unsigned Start, unsigned NumInts,
                                      unsigned NumUndefs,
                                      LLVMContext &Context) {
  SmallVector<Constant *, 16> Mask;
  for (unsigned I = 0; I < NumInts; ++I)
    Mask.push_back(ConstantInt::get(Type::getInt32Ty(Context), I));

  Constant *Undef = UndefValue::get(Type::getInt32Ty(Context));
  for (unsigned I = 0; I < NumUndefs; ++I)
    Mask.push_back(Undef);

  return ConstantVector::get(Mask);
}

void VPOCodeGenHIR::propagateMetadata(const OVLSGroup *Group,
                                      RegDDRef *NewRef) {
  SmallVector<const RegDDRef *, 4> MemDDRefVec;
  for (int64_t Index = 0, Size = Group->size(); Index < Size; ++Index) {
    auto *Memref = cast<VPVLSClientMemrefHIR>(Group->getMemref(Index));
    MemDDRefVec.push_back(Memref->getRegDDRef());
  }

  const RegDDRef *R0 = MemDDRefVec[0];
  for (auto Kind :
       {LLVMContext::MD_tbaa, LLVMContext::MD_alias_scope,
        LLVMContext::MD_noalias, LLVMContext::MD_fpmath,
        LLVMContext::MD_nontemporal, LLVMContext::MD_invariant_load}) {
    MDNode *MD = R0->getMetadata(Kind);

    for (int J = 1, E = MemDDRefVec.size(); MD && J != E; ++J) {
      const RegDDRef *RJ = MemDDRefVec[J];
      MDNode *MDJ = RJ->getMetadata(Kind);
      switch (Kind) {
      case LLVMContext::MD_tbaa:
        MD = MDNode::getMostGenericTBAA(MD, MDJ);
        break;
      case LLVMContext::MD_alias_scope:
        MD = MDNode::getMostGenericAliasScope(MD, MDJ);
        break;
      case LLVMContext::MD_fpmath:
        MD = MDNode::getMostGenericFPMath(MD, MDJ);
        break;
      case LLVMContext::MD_noalias:
      case LLVMContext::MD_nontemporal:
      case LLVMContext::MD_invariant_load:
        MD = MDNode::intersect(MD, MDJ);
        break;
      default:
        llvm_unreachable("unhandled metadata");
      }
    }

    NewRef->setMetadata(Kind, MD);
  }
}

RegDDRef *VPOCodeGenHIR::concatenateTwoVectors(RegDDRef *V1, RegDDRef *V2,
                                               RegDDRef *Mask) {
  VectorType *VecTy1 = dyn_cast<VectorType>(V1->getDestType());
  VectorType *VecTy2 = dyn_cast<VectorType>(V2->getDestType());
  assert(VecTy1 && VecTy2 &&
         VecTy1->getScalarType() == VecTy2->getScalarType() &&
         "Expect two vectors with the same element type");

  unsigned NumElts1 = VecTy1->getNumElements();
  unsigned NumElts2 = VecTy2->getNumElements();
  assert(NumElts1 >= NumElts2 &&
         "Unexpected the first vector has less elements");

  if (NumElts1 > NumElts2) {
    // Extend with UNDEFs.
    Constant *ExtMask =
        createSequentialMask(0, NumElts2, NumElts1 - NumElts2, Context);
    RegDDRef *ExtMaskDDRef = V1->getDDRefUtils().createConstDDRef(ExtMask);

    Constant *Undef = UndefValue::get(VecTy2);
    RegDDRef *UndefDDRef = V1->getDDRefUtils().createConstDDRef(Undef);

    HLInst *ExtShuffle = MainLoop->getHLNodeUtils().createShuffleVectorInst(
        V2->clone(), UndefDDRef, ExtMaskDDRef, "ext.shuf");
    addInst(ExtShuffle, Mask);
    V2 = ExtShuffle->getLvalDDRef();
  }

  // V1 and V2 are same length now - join the two vectors.
  Constant *CombMask = createSequentialMask(0, NumElts1 + NumElts2, 0, Context);
  RegDDRef *CombMaskDDRef = V1->getDDRefUtils().createConstDDRef(CombMask);

  HLInst *CombShuffle = MainLoop->getHLNodeUtils().createShuffleVectorInst(
      V1->clone(), V2->clone(), CombMaskDDRef, "comb.shuf");
  addInst(CombShuffle, Mask);
  return CombShuffle->getLvalDDRef();
}

RegDDRef *VPOCodeGenHIR::concatenateVectors(RegDDRef **VecsArray,
                                            int64_t NumVecs, RegDDRef *Mask) {
  assert(NumVecs > 1 && "Should be at least two vectors");

  SmallVector<RegDDRef *, 8> ResList;
  for (unsigned Index = 0; Index < NumVecs; ++Index)
    ResList.push_back(VecsArray[Index]);

  do {
    SmallVector<RegDDRef *, 8> TmpList;
    for (unsigned Index = 0; Index < NumVecs - 1; Index += 2) {
      RegDDRef *V0 = ResList[Index], *V1 = ResList[Index + 1];
      assert((V0->getDestType() == V1->getDestType() || Index == NumVecs - 2) &&
             "Only the last vector may have a different type");

      TmpList.push_back(concatenateTwoVectors(V0, V1, Mask));
    }

    // Push the last vector if the total number of vectors is odd.
    if (NumVecs % 2 != 0)
      TmpList.push_back(ResList[NumVecs - 1]);

    ResList = TmpList;
    NumVecs = ResList.size();
  } while (NumVecs > 1);

  return ResList[0];
}

HLInst *VPOCodeGenHIR::createInterleavedLoad(const RegDDRef *LvalRef,
                                             RegDDRef *WLoadRes,
                                             int64_t InterleaveFactor,
                                             int64_t InterleaveIndex,
                                             RegDDRef *Mask) {
  SmallVector<Constant *, 8> ShuffleMask;
  // Create the interleaved load shuffle for memory access at position
  // InterleaveIndex in the VLS group.
  for (unsigned Index = 0; Index < VF; ++Index) {
    Constant *Mask =
        ConstantInt::get(Type::getInt32Ty(Context),
                         InterleaveIndex + (InterleaveFactor * Index));
    ShuffleMask.push_back(Mask);
  }
  Constant *MaskVec = ConstantVector::get(ShuffleMask);
  RegDDRef *ShuffleMaskRef =
      WLoadRes->getDDRefUtils().createConstDDRef(MaskVec);

  Constant *UndefVal = UndefValue::get(WLoadRes->getDestType());
  RegDDRef *UndefRef = WLoadRes->getDDRefUtils().createConstDDRef(UndefVal);

  // Create shuffle instruction using the result of the wide load and the
  // computed shuffle mask.
  RegDDRef *WLvalRef = widenRef(LvalRef, getVF());
  HLInst *Shuffle = MainLoop->getHLNodeUtils().createShuffleVectorInst(
      WLoadRes->clone(), UndefRef, ShuffleMaskRef, "vls.shuf", WLvalRef);

  addInst(Shuffle, Mask);
  addToMapAndHandleLiveOut(LvalRef, Shuffle, MainLoop);

  // Shuffle instruction creation creates a new Lval ref if the passed in
  // Lval ref during HLInst creation is null.
  WLvalRef = Shuffle->getLvalDDRef();
  if (WLvalRef->isTerminalRef())
    WLvalRef->makeSelfBlob();
  return Shuffle;
}

HLInst *VPOCodeGenHIR::createInterleavedStore(RegDDRef **StoreVals,
                                              const RegDDRef *StorePtrRef,
                                              int64_t InterleaveFactor,
                                              RegDDRef *Mask) {
  RegDDRef *ConcatVec = concatenateVectors(StoreVals, InterleaveFactor, Mask);

  // Create interleaved store mask shuffle instruction to shuffle the
  // concatenated vectors in the desired order for the wide store.
  Constant *UndefVal = UndefValue::get(ConcatVec->getDestType());
  RegDDRef *UndefRef = StorePtrRef->getDDRefUtils().createConstDDRef(UndefVal);

  Constant *InterleaveMask =
      createInterleaveMask(Context, getVF(), InterleaveFactor);
  RegDDRef *InterleaveMaskRef =
      StorePtrRef->getDDRefUtils().createConstDDRef(InterleaveMask);

  HLInst *Shuffle = MainLoop->getHLNodeUtils().createShuffleVectorInst(
      ConcatVec->clone(), UndefRef, InterleaveMaskRef, "vls.interleave");
  addInst(Shuffle, Mask);

  // Create the wide store using the shuffled vector values and the widened
  // store pointer reference.
  RegDDRef *ShuffleRef = Shuffle->getLvalDDRef();
  RegDDRef *WStorePtrRef =
      widenRef(StorePtrRef, getVF() * InterleaveFactor, true);
  HLInst *WideStore = MainLoop->getHLNodeUtils().createStore(
      ShuffleRef->clone(), ".vls.store", WStorePtrRef);
  addInst(WideStore, Mask);

  return WideStore;
}

HLInst *VPOCodeGenHIR::widenInterleavedAccess(const HLInst *INode,
                                              RegDDRef *Mask,
                                              const OVLSGroup *Grp,
                                              int64_t InterleaveFactor,
                                              int64_t InterleaveIndex,
                                              const HLInst *GrpStartInst) {
  auto CurInst = INode->getLLVMInstruction();
  HLInst *WideInst = nullptr;

  if (isa<LoadInst>(CurInst)) {
    RegDDRef *WLoadRes;
    auto It = VLSGroupLoadMap.find(Grp);

    if (It == VLSGroupLoadMap.end()) {
      // We are encountering the first instruction of a load group. Generate a
      // wide load using the memory reference from the instruction starting the
      // group.
      const RegDDRef *MemRef =
          cast<VPVLSClientMemrefHIR>(Grp->getFirstMemref())->getRegDDRef();
      RegDDRef *WMemRef = widenRef(MemRef, getVF() * InterleaveFactor, true);
      HLInst *WideLoad = INode->getHLNodeUtils().createLoad(
          WMemRef, CurInst->getName() + ".vls.load");
      propagateMetadata(Grp, WMemRef);

      addInst(WideLoad, Mask);

      // Set the result of the wide load and add the same to VLS Group load map.
      WLoadRes = WideLoad->getLvalDDRef();
      VLSGroupLoadMap[Grp] = WLoadRes;
    } else
      WLoadRes = (*It).second;

    WideInst = createInterleavedLoad(INode->getLvalDDRef(), WLoadRes,
                                     InterleaveFactor, InterleaveIndex, Mask);
  } else {
    assert(isa<StoreInst>(CurInst) &&
           "Unexpected interleaved access instruction");
    RegDDRef **StoreValRefs;
    const RegDDRef *StoreValRef = INode->getOperandDDRef(1);
    RegDDRef *WStoreValRef = widenRef(StoreValRef, getVF());

    auto It = VLSGroupStoreMap.find(Grp);
    if (It == VLSGroupStoreMap.end()) {
      // We are encountering the first instruction of an OPTVLS store group.
      // Allocate an array of RegDDRef * that is big enough to store the values
      // being stored in InterleaveFactor number of stores in the group.
      // Initialize array elements to null and store in VLSGroupStoreMap.
      StoreValRefs = new RegDDRef *[InterleaveFactor];
      for (unsigned Index = 0; Index < InterleaveFactor; ++Index)
        StoreValRefs[Index] = nullptr;
      VLSGroupStoreMap[Grp] = StoreValRefs;
    } else
      StoreValRefs = (*It).second;

    // Store the widened store value into StoreValRefs array.
    StoreValRefs[InterleaveIndex] = WStoreValRef;

    // Check if we are at the point of the last store in the VLS group.
    unsigned Index;
    for (Index = 0; Index < InterleaveFactor; ++Index)
      if (StoreValRefs[Index] == nullptr)
        break;

    // If we have seen all the instructions in a store group, go ahead and
    // generate the interleaved store.
    if (Index == InterleaveFactor) {
      WideInst =
          createInterleavedStore(StoreValRefs, GrpStartInst->getOperandDDRef(0),
                                 InterleaveFactor, Mask);
      propagateMetadata(Grp, WideInst->getOperandDDRef(0));
    }
  }

  return WideInst;
}

// Generate cttz(bsf) call for a given Ref.
//    %intmask = bitcast.i32(Ref);
//    %bsf = call intrinsic.cttz.i32(Ref);
HLInst *VPOCodeGenHIR::createCTTZCall(RegDDRef *Ref, bool MaskIsNonZero,
                                      const Twine &Name) {
  assert(Ref && "Ref is expected for this assignment.");

  HLNodeUtils &HNU = OrigLoop->getHLNodeUtils();
  DDRefUtils &DDRU = OrigLoop->getDDRefUtils();
  // It's necessary to bitcast mask to integer, otherwise it's not possible to
  // use it in cttz instruction.
  Type *RefTy = Ref->getDestType();
  LLVMContext &Context = *Plan->getLLVMContext();
  Type *IntTy = IntegerType::get(Context, RefTy->getPrimitiveSizeInBits());
  HLInst *BitCastInst = createBitCast(IntTy, Ref, Name + "intmask");

  RegDDRef *IntRef = BitCastInst->getLvalDDRef();
  // As soon as this code has to be under if condition, return value from cttz
  // can be undefined if mask is zero. However if we know for a fact that mask
  // is non-zero, then inform intrinsic that zero produces defined result.
  Type *BoolTy = IntegerType::get(Context, 1);
  Function *BsfFunc =
      Intrinsic::getDeclaration(&HNU.getModule(), Intrinsic::cttz, {IntTy});
  SmallVector<RegDDRef *, 1> Args = {
      IntRef->clone(), DDRU.createConstDDRef(BoolTy, MaskIsNonZero)};
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
                                                      RegDDRef *Mask,
                                                      bool MaskIsNonZero) {
  HLInst *BsfCall = createCTTZCall(Mask, MaskIsNonZero);

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
  INode->getRvalDDRef()->makeConsistent(AuxRefs, Level);
  return INode;
}

// This visitor collects the reaching definitions of live-outs reaching a given
// HLIf. This is an ad-hoc implementation for search loop vectorization, where
// we need  to collect the reaching definitions of live-outs that are before an
// HLIf with an early exit.
class LiveOutReachDefsVisitor : public HLNodeVisitorBase {
private:
  // The original loop we are vectorizing.
  HLLoop *OrigLoop;

  // The HLIf with an early exit goto. The traversak is done once we reach this
  // node.
  HLIf *EarlyExitIf;

  // Flag to signal that we have hit the target EarlyExitIf during the
  // traversal.
  bool IsDone = false;

  // Map from live-out symbases to live-out reaching definitions. If multiple
  // definitions for the same symbase happen, only the last one will prevail
  // (the new definition kills the old one).
  SmallMapVector<unsigned, HLInst *, 4> &LiveOutReachDefs;

public:
  LiveOutReachDefsVisitor(HLLoop *OrigLp, HLIf *EEIf,
                          SmallMapVector<unsigned, HLInst *, 4> &LO)
      : OrigLoop(OrigLp), EarlyExitIf(EEIf), LiveOutReachDefs(LO) {
    assert(OrigLoop->getNumExits() == 2 && "Expected only two exits!");
  }

  // Return true if the traversal is done, i.e., we have reached EarlyExitIf.
  bool isDone() { return IsDone; }

  // Catch-all methods.
  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}

  void visit(HLInst *Inst) {
    // We are just looking for live-out HLInst like t1 = ...
    if (!Inst->hasLval())
      return;

    RegDDRef *LDDRef = Inst->getLvalDDRef();
    if (!LDDRef->isTerminalRef() || !OrigLoop->isLiveOut(LDDRef->getSymbase()))
      return;

    // HLInst is a live out terminal ref.
    // For current search loops, we should only visit plain HLInsts without any
    // other complex HLNode before the early exit HLIf.
    assert(Inst->getParent() == OrigLoop && "Expected loop as parent!");
    LiveOutReachDefs[LDDRef->getSymbase()] = Inst;
  }

  void visit(HLIf *If) { IsDone = If == EarlyExitIf; }

  // For now, we only support search loops idioms with live-outs in the loop
  // header so we shouldn't try to collect live-outs inside HLIf's or nested
  // HLLoops.
  void postvisit(HLIf *If) {
    assert(false && "Unexpected HLIf in search loop idiom!");
  }

  void postvisit(HLLoop *Lp) {
    assert(false && "Unexpected HLLoop in search loop idiom!");
  }
};

void VPOCodeGenHIR::handleNonLinearEarlyExitLiveOuts(const HLGoto *Goto) {
  assert(isSearchLoop() && "Expected search loop!");
  assert(Goto->isEarlyExit(OrigLoop) && "Expected early exit goto!");

  // Collect live-out definitions reaching Goto's (early exit) parent.
  // FIXME: Current implementation will work only for simple search loops with
  // single early exit HLIfs.
  HLNode *Parent = Goto->getParent();
  assert(isa<HLIf>(Parent) && "Expected HLIf as HLGoto's parent!");
  SmallMapVector<unsigned, HLInst *, 4> LiveOutReachDefs;
  LiveOutReachDefsVisitor LOVisitor(OrigLoop, cast<HLIf>(Parent),
                                    LiveOutReachDefs);
  Goto->getHLNodeUtils().visit(LOVisitor, OrigLoop);

  // Insert new live-out definitions in early exit branch, modifying linear
  // variables according to the first lane taking the early exit.
  for (auto &LiveOutPair : LiveOutReachDefs) {
    HLInst *LOInst = LiveOutPair.second;
    HLInst *NewLOInst = LOInst->clone();
    // Check in the live-out is supported by createLiveOutLinearForEE. At this
    // point, current support is limited to very specific simple live-outs in
    // search loops.
    RegDDRef *RvalRef = NewLOInst->getRvalDDRef();
    assert(RvalRef && "Unsupported live-out: expected a single RvalDDRef!");
    assert(RvalRef->isLinear() &&
           "Unsupported live-out: expected linear RvalDDRef!");
    (void)RvalRef;
    RegDDRef *LvalRef = NewLOInst->getLvalDDRef();
    assert(LvalRef && "Unsupported live-out: expected a single LvalDDRef!");
    assert(LvalRef->isNonLinear() &&
           "Unsupported live-out: expected non-linear LvalDDRef!");
    (void)LvalRef;
    // All linear live-outs for the SearchLoopStructPtrEq idiom are known to be
    // strictly inside the if-then block of mask check. Hence we know that mask
    // is non-zero for such live-out instructions.
    handleLiveOutLinearInEarlyExit(
        NewLOInst, CurMaskValue,
        SearchLoopType == VPlanIdioms::SearchLoopStructPtrEq /*NonZeroMask*/);
  }
}

HLInst *VPOCodeGenHIR::widenNonMaskedUniformStore(const HLInst *INode) {
  auto CurInst = INode->getLLVMInstruction();
  const RegDDRef *Lval = INode->getLvalDDRef();
  const RegDDRef *Rval = INode->getRvalDDRef();

  assert(Lval && Rval && "Operand of store is null.");

  LLVM_DEBUG(dbgs() << "VPCodegen: Lval : "; Lval->dump(); dbgs() << "\n");
  LLVM_DEBUG(dbgs() << "VPCodegen: Rval : "; Rval->dump(); dbgs() << "\n");

  // If Rval is not loop invariant, then we widen it and generate an
  // extractelement instruction
  // TODO: extractelement may not be efficient if the Rval is not a memory
  // reference. For example:
  //    DO i1 = 0, N, 1 <DO_LOOP>
  //      (%array)[0] = i1;
  //    END LOOP
  //
  // can be directly changed to
  //    DO i1 = 0, N, VF <DO_LOOP>
  //      (%array)[0] = i1 + VF -1;
  //    END LOOP
  //
  unsigned NestingLevel = OrigLoop->getNestingLevel();
  if (!Rval->isStructurallyInvariantAtLevel(NestingLevel)) {
    RegDDRef *WideRef = widenRef(Rval, getVF());
    assert(WideRef && "Unexpected null widened ref");
    // create an extractelement instruction to get last element of vector
    auto Extract = INode->getHLNodeUtils().createExtractElementInst(
        WideRef, VF - 1, "last");
    addInst(Extract, nullptr);
    // overwrite Rval for store with Lval of extractelement instruction
    Rval = Extract->getLvalDDRef();
    LLVM_DEBUG(dbgs() << "VPCodegen: widened Rval : "; Rval->dump();
               dbgs() << "\n");
  }

  HLInst *WideInst = INode->getHLNodeUtils().createStore(
      Rval->clone(), CurInst->getName() + ".uniform.store", Lval->clone());
  LLVM_DEBUG(dbgs() << "VPCodegen: WideInst : "; WideInst->dump();
             dbgs() << "\n");
  addInst(WideInst, nullptr);

  return WideInst;
}

HLInst *VPOCodeGenHIR::widenCall(const HLInst *INode,
                                 SmallVectorImpl<RegDDRef *> &WideOps,
                                 RegDDRef *Mask,
                                 SmallVectorImpl<RegDDRef *> &CallArgs,
                                 bool HasLvalArg) {
  const CallInst *Call = INode->getCallInst();
  assert(Call && "Unexpected null call");
  Function *Fn = Call->getCalledFunction();
  assert(Fn && "Unexpected null called function");
  StringRef FnName = Fn->getName();
  RegDDRef *WideLval = HasLvalArg ? WideOps[0] : nullptr;

  // Default to svml. If svml is not available, try the intrinsic.
  Intrinsic::ID ID = Intrinsic::not_intrinsic;
  if (!TLI->isFunctionVectorizable(FnName, VF)) {
    ID = getVectorIntrinsicIDForCall(Call, TLI);
    // FIXME: We should scalarize calls to @llvm.assume intrinsic instead of
    //        completely ignoring them.
    if (ID && (ID == Intrinsic::assume || ID == Intrinsic::lifetime_end ||
               ID == Intrinsic::lifetime_start)) {
      return nullptr;
    }
  }

  unsigned ArgOffset = 0;
  // If WideOps contains the widened Lval, the actual call arguments start
  // at position 1 in the WideOps vector.
  if (HasLvalArg) {
    ArgOffset = 1;
  }
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
      getOrInsertVectorFunction(Fn, VF, ArgTys, TLI, ID, nullptr, Masked);
  assert(VectorF && "Can't create vector function.");

  auto *WideInst = INode->getHLNodeUtils().createCall(
      VectorF, CallArgs, VectorF->getName(), WideLval);
  Instruction *Inst = const_cast<Instruction *>(WideInst->getLLVMInstruction());

  if (isa<FPMathOperator>(Inst)) {
    Inst->copyFastMathFlags(Call);
  }

  // Make sure we don't lose attributes at the call site. E.g., IMF
  // attributes are taken from call sites in MapIntrinToIml to refine
  // SVML calls for precision.
  cast<CallInst>(Inst)->setAttributes(Call->getAttributes());

  // Set calling conventions for SVML function calls
  if (isSVMLFunction(TLI, FnName, VectorF->getName())) {
    cast<CallInst>(Inst)->setCallingConv(CallingConv::SVML);
  }

  // We analyze memory references in call arguments to add stride information
  // for sincos calls. Clear call args for non-sincos calls. Caller analyzes
  // call args if they are non-empty.
  if (FnName.find("sincos") == StringRef::npos)
    CallArgs.clear();

  return WideInst;
}

void VPOCodeGenHIR::widenNodeImpl(const HLInst *INode, RegDDRef *Mask,
                                  const OVLSGroup *Grp,
                                  int64_t InterleaveFactor,
                                  int64_t InterleaveIndex,
                                  const HLInst *GrpStartInst,
                                  const VPInstruction *VPInst) {
  auto CurInst = INode->getLLVMInstruction();
  SmallVector<RegDDRef *, 6> WideOps;
  SmallVector<RegDDRef *, 1> CallArgs;

  if (!Mask)
    Mask = CurMaskValue;

  // Check if we want to widen the current Inst as an interleaved memory access.
  // TODO: Mask for the interleaved accesse must be shuffled as well. Currently
  // it's not done, thus disable CG for it.
  if (Grp && !Mask) {
    bool InterleaveAccess = EnableVPlanVLSCG;
    const RegDDRef *MemRef;

    // If the user is limiting the number of vectorized loops for which
    // VLS optimization is enabled, only interleave for the specified
    // number of loops.
    if (VPlanVLSNumLoops >= 0)
      InterleaveAccess &= LoopsVectorized <= ((unsigned)VPlanVLSNumLoops);

    // Check if the user is limiting interleaved accesses to loads or stores.
    if (isa<LoadInst>(CurInst)) {
      MemRef = INode->getOperandDDRef(1);
      InterleaveAccess &= EnableVPlanVLSLoads;
    } else {
      MemRef = INode->getOperandDDRef(0);
      InterleaveAccess &= EnableVPlanVLSStores;
    }

    // If the reference is unit strided, we do not need interleaving
    InterleaveAccess &= !refIsUnit(OrigLoop->getNestingLevel(), MemRef, this);
    if (InterleaveAccess) {
      widenInterleavedAccess(INode, Mask, Grp, InterleaveFactor,
                             InterleaveIndex, GrpStartInst);
      return;
    }
  }

  // Widened values for SCEV expressions cannot be reused across HIR
  // instructions as temps that are part of such SCEV expressions can be
  // assigned to before a use in a later HIR instruction.
  // t1 =
  //    = zext(t1)
  // t1 =
  //    = zext(t1)
  // We clear the map at the start of widening of each HLInst.
  SCEVWideRefMap.clear();

  HLInst *WideInst = nullptr;

  LLVM_DEBUG(INode->dump(true));
  bool InsertInMap = true;

  if (isSearchLoop() && INode->hasLval() &&
      !INode->getRvalDDRef()->isMemRef() &&
      INode->getLvalDDRef()->isTerminalRef() &&
      INode->getLvalDDRef()->isLiveOutOfParentLoop() && INode->hasRval() &&
      !INode->getRvalDDRef()->isNonLinear()) {
    assert(Mask && "HLInst with linear ref does not have mask.");
    handleLiveOutLinearInEarlyExit(
        INode->clone(), Mask,
        SearchLoopType == VPlanIdioms::SearchLoopStructPtrEq /*NonZeroMask*/);
    return;
  }

  // Check for non-masked uniform stores and handle codegen for them
  // separately
  if (isa<StoreInst>(CurInst) && !Mask) {
    // NOTE: isUniform is private to HandledCheck, so we have to use
    // isStructurallyInvariantAtLevel here in VPOCodeGenHIR
    auto TLval = INode->getLvalDDRef();
    unsigned NestingLevel = OrigLoop->getNestingLevel();
    if (TLval->isStructurallyInvariantAtLevel(NestingLevel)) {
      widenNonMaskedUniformStore(INode);
      return;
    }
  }

  // Widen instruction operands
  for (auto Iter = (INode)->op_ddref_begin(), End = (INode)->op_ddref_end();
       Iter != End; ++Iter) {
    RegDDRef *WideRef;
    WideRef = widenRef(*Iter, getVF());
    WideOps.push_back(WideRef);
  }

  // Generate the widened instruction using widened operands
  if (auto BOp = dyn_cast<BinaryOperator>(CurInst)) {
    WideInst = INode->getHLNodeUtils().createBinaryHLInst(
        BOp->getOpcode(), WideOps[1], WideOps[2], CurInst->getName() + ".vec",
        WideOps[0], BOp);
  } else if (auto UOp = dyn_cast<UnaryOperator>(CurInst)) {
    WideInst = INode->getHLNodeUtils().createUnaryHLInst(
        UOp->getOpcode(), WideOps[1], CurInst->getName() + ".vec", WideOps[0],
        nullptr, UOp);
  } else if (isa<LoadInst>(CurInst)) {
    WideInst = INode->getHLNodeUtils().createLoad(
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<StoreInst>(CurInst)) {
    WideInst = INode->getHLNodeUtils().createStore(
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
    InsertInMap = false;
  } else if (isa<CastInst>(CurInst)) {
    assert(WideOps.size() == 2 && "invalid cast");

    WideInst = INode->getHLNodeUtils().createCastHLInst(
        VectorType::get(CurInst->getType(), VF), CurInst->getOpcode(),
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<SelectInst>(CurInst)) {
    WideInst = INode->getHLNodeUtils().createSelect(
        INode->getPredicate(), WideOps[1], WideOps[2], WideOps[3], WideOps[4],
        CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<CmpInst>(CurInst)) {
    WideInst = INode->getHLNodeUtils().createCmp(
        INode->getPredicate(), WideOps[1], WideOps[2],
        CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<GEPOrSubsOperator>(CurInst)) {
    // Gep Instructions in LLVM may have any number of operands but the HIR
    // representation for them is always a single rhs ddref - copy rval to
    // lval.
    WideInst = INode->getHLNodeUtils().createCopyInst(
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
  } else if (INode->isCallInst()) {
    bool HasLvalArg = INode->hasLval();
    WideInst = widenCall(INode, WideOps, Mask, CallArgs, HasLvalArg);
    if (!WideInst)
      return;
    if (HasLvalArg) {
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
    addVPValueWideRefMapping(VPInst, WideInst->getLvalDDRef());
    if (WideInst->getLvalDDRef()->isTerminalRef())
      WideInst->getLvalDDRef()->makeSelfBlob();
  }

  addInst(WideInst, Mask);
  // We need to hook up WideInst before we call analyzeCallArgMemoryReferences
  // as getConstStrideAtLevel expects a ref whose stride is being checked to be
  // attached to HIR. We analyze memory references in call args for stride
  // information if CallArgs is non-empty.
  if (!CallArgs.empty())
    analyzeCallArgMemoryReferences(INode, WideInst, CallArgs);
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
  auto *VecRef = WideInst->getLvalDDRef();
  WidenMap[ScalSymbase] = VecRef;

  // Generate any necessary code to handle loop liveout/reduction
  if (!MainLoop->isLiveOut(ScalSymbase))
    return;

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

void VPOCodeGenHIR::addPaddingRuntimeCheck(
    SmallVectorImpl<std::tuple<HLPredicate, RegDDRef *, RegDDRef *>>
        &RTChecks) {
  if (getSearchLoopType() != VPlanIdioms::SearchLoopStrEq)
    return;

#if INTEL_INCLUDE_DTRANS
  // Generate runtime check __Intel_PaddedMallocCounter[0] < PaddedMallocLimit.
  // If condition is false, vector code is not safe to execute.
  HLNodeUtils &HNU = OrigLoop->getHLNodeUtils();
  if (GlobalVariable *PaddedMallocVariable =
          HNU.getModule().getGlobalVariable("__Intel_PaddedMallocCounter",
                                            true /*AllowInternal*/)) {
    LLVMContext &Context = *Plan->getLLVMContext();
    DDRefUtils &DDRU = OrigLoop->getDDRefUtils();
    CanonExprUtils &CEU = HNU.getCanonExprUtils();

    Type *IntTy = IntegerType::get(Context, 32);
    Type *BoolTy = IntegerType::get(Context, 1);

    // Construct memref __Intel_PaddedMallocCounter[0].
    unsigned PaddedMallocAddrIdx;
    CEU.getBlobUtils().createGlobalVarBlob(PaddedMallocVariable, true,
                                           &PaddedMallocAddrIdx);
    RegDDRef *PaddedMalloc = DDRU.createMemRef(PaddedMallocAddrIdx);
    PaddedMalloc->addDimension(CEU.createCanonExpr(IntTy));

    // Construct PaddedMallocLimit.
    RegDDRef *PaddedLimit =
        DDRU.createConstDDRef(IntTy, llvm::getPaddedMallocLimit());

    HLInst *PaddingIsValid = HNU.createCmp(
        PredicateTy::ICMP_ULT, PaddedMalloc, PaddedLimit, "valid.padding");
    HLNodeUtils::insertBefore(OrigLoop, PaddingIsValid);

    RegDDRef *ZeroRef = DDRU.createConstDDRef(BoolTy, 0);
    HLPredicate Pred(PredicateTy::ICMP_NE);
    auto Check = std::make_tuple(
        Pred, PaddingIsValid->getLvalDDRef()->clone(), ZeroRef);
    RTChecks.push_back(Check);
  }
#endif // INTEL_INCLUDE_DTRANS
}

RegDDRef *VPOCodeGenHIR::widenRef(const VPValue *VPVal, unsigned VF) {
  RegDDRef *WideRef = nullptr;
  DDRefUtils &DDU = OrigLoop->getDDRefUtils();
  HLNodeUtils &HNU = OrigLoop->getHLNodeUtils();
  CanonExprUtils &CEU = HNU.getCanonExprUtils();

  // If the DDREF has a widened counterpart, return the same.
  if ((WideRef = getWideRefForVPVal(VPVal)))
    return WideRef->clone();

  if (auto *VPExtDef = dyn_cast<VPExternalDef>(VPVal)) {
    const VPOperandHIR *HIROperand = VPExtDef->getOperandHIR();

    if (const auto *Blob = dyn_cast<VPBlob>(HIROperand)) {
      auto *BDDR = Blob->getBlob();
      WideRef = DDU.createSelfBlobRef(BDDR->getSelfBlobIndex(),
                                      BDDR->getDefinedAtLevel());
      WideRef = widenRef(WideRef, VF);
    } else {
      const auto *IV = cast<VPIndVar>(HIROperand);
      auto IVLevel = IV->getIVLevel();
      auto RefDestTy = VPExtDef->getType();
      auto VecRefDestTy = VectorType::get(RefDestTy, VF);

      auto *CE = CEU.createCanonExpr(VecRefDestTy);
      CE->addIV(IVLevel, InvalidBlobIndex /* no blob */,
                1 /* constant IV coefficient */);
      WideRef = DDU.createScalarRegDDRef(GenericRvalSymbase, CE);
    }
  } else {
    assert(isa<VPConstant>(VPVal) && "Expected a VPConstant");
    auto *VPConst = cast<VPConstant>(VPVal);
    WideRef = DDU.createConstDDRef(VPConst->getConstant());
    WideRef = widenRef(WideRef, VF);
  }

  assert(WideRef && "Expected non-null widened ref");
  LLVM_DEBUG(WideRef->dump(true));
  LLVM_DEBUG(errs() << "\n");
  return WideRef;
}

// Given a widened ref corresponding to the pointer operand of
// a load/store instruction, setup and return the pointer operand
// for use in generating the load/store HLInst.
static RegDDRef *getPointerOperand(RegDDRef *PtrOp, HLNodeUtils &HNU,
                                   DDRefUtils &DDU, CanonExprUtils &CEU,
                                   Type *VecRefDestTy, unsigned AddressSpace,
                                   unsigned VF) {
  RegDDRef *AddrRef;
  if (PtrOp->isAddressOf()) {
    // We generate an addressof ref from the GEP instruction. For
    // generating load/store instruction, we need to clear the
    // addressof.
    AddrRef = PtrOp;
    AddrRef->setAddressOf(false);
  } else {
    // PtrOp is an invariant blob - we need to generate a
    // reference of the form blob[0].
    assert(PtrOp->isSelfBlob() && "Expected self blob DDRef");
    auto &HIRF = HNU.getHIRFramework();
    llvm::Triple TargetTriple(HIRF.getModule().getTargetTriple());
    auto Is64Bit = TargetTriple.isArch64Bit();
    AddrRef = DDU.createMemRef(PtrOp->getSelfBlobIndex());
    auto Int32Ty = Type::getInt32Ty(HNU.getContext());
    auto Int64Ty = Type::getInt64Ty(HNU.getContext());
    auto Zero = CEU.createCanonExpr(Is64Bit ? Int64Ty : Int32Ty);

    // We need to set destination type of the created canon expression
    // to VF wide vector type.
    Zero->setDestType(VectorType::get(Zero->getSrcType(), VF));

    AddrRef->addDimension(Zero);
  }
  AddrRef->setBitCastDestType(PointerType::get(VecRefDestTy, AddressSpace));
  return AddrRef;
}

// Widen PHI instruction. The implementation here generates a sequence
// of selects if VPPhi is marked as blend. Consider the following scalar
// phi in bb0
//      vp1 = phi [vp2, bb1] [vp3, bb2] [vp4, bb3].
// The selects are generated as follows:
//      select1 = mask_for_edge(bb0, bb2) ? vp3_vec : vp2_vec
//      vp1_vec = mask_for_edge(bb0, bb3) ? vp4_vec : select1
// The implementation here assumes that any PHI not marked as a blend
// is the loop IV and widens it by generating the ref i1 + <0, 1, 2, .. VF-1>.
// This assumption needs to change when we start handling reductions/linears.
void VPOCodeGenHIR::widenPhiImpl(const VPPHINode *VPPhi, RegDDRef *Mask) {
  HLNodeUtils &HNU = OrigLoop->getHLNodeUtils();
  CanonExprUtils &CEU = HNU.getCanonExprUtils();
  DDRefUtils &DDU = OrigLoop->getDDRefUtils();

  // Blend marked PHIs using selects and incoming masks
  if (VPPhi->getBlend()) {
    unsigned NumIncomingValues = VPPhi->getNumIncomingValues();
    assert(NumIncomingValues > 0 && "Unexpected PHI with zero values");

    if (NumIncomingValues == 1) {
      // Blend phis should only be encountered in the linearized control flow.
      // However, currently some preceding transformations mark some
      // single-value phis as blends too (and codegen is probably relying on
      // that as well). Bail out right now because general processing of phis
      // with multiple incoming values relies on the control flow being
      // linearized.
      RegDDRef *Val = widenRef(VPPhi->getOperand(0), getVF());
      addVPValueWideRefMapping(VPPhi, Val);
      return;
    }

    // FIXME: Ugly hack - should probably have a single VPlan-to-VPlan pass
    // doing this and possible other preparations. Not UB because we always
    // create non-const instructions in PlainCFGBuilder.
    const_cast<VPPHINode *>(VPPhi)->sortIncomingBlocksForBlend();

    // Generate a sequence of selects.
    RegDDRef *BlendVal = nullptr;
    for (unsigned Idx = 0, End = VPPhi->getNumIncomingValues(); Idx < End;
         ++Idx) {
      VPBasicBlock *Block = VPPhi->getIncomingBlock(Idx);
      auto *IncomingVecVal = widenRef(VPPhi->getIncomingValue(Idx), getVF());
      if (!BlendVal) {
        BlendVal = IncomingVecVal;
        continue;
      }

      RegDDRef *Cond = widenRef(Block->getPredicate(), getVF());
      Type *CondTy = Cond->getDestType();
      Constant *OneVal = Constant::getAllOnesValue(CondTy->getScalarType());
      RegDDRef *OneValRef = getConstantSplatDDRef(DDU, OneVal, getVF());
      HLInst *BlendInst = HNU.createSelect(CmpInst::ICMP_EQ, Cond, OneValRef,
                                           IncomingVecVal, BlendVal);
      // TODO: Do we really need the Mask here?
      addInst(BlendInst, Mask);
      BlendVal = BlendInst->getLvalDDRef()->clone();
    }
    addVPValueWideRefMapping(VPPhi, BlendVal);
    return;
  }

  // Check if PHI corresponds to a reduction, if yes just use the corresponding
  // reduction ref as its widened ref.
  if (ReductionRefs.count(VPPhi)) {
    addVPValueWideRefMapping(VPPhi, ReductionRefs[VPPhi]);
    return;
  }

  // Assuming remaining PHIs to be loop induction - this will change with
  // support for live-outs.
  auto RefDestTy = VPPhi->getType();
  auto VecRefDestTy = VectorType::get(RefDestTy, VF);

  auto *CE = CEU.createCanonExpr(RefDestTy);
  CE->setSrcType(VecRefDestTy);
  CE->setDestType(VecRefDestTy);
  CE->addIV(OrigLoop->getNestingLevel(), InvalidBlobIndex /* no blob */,
            1 /* constant IV coefficient */);
  SmallVector<Constant *, 4> ConstVec;
  for (unsigned i = 0; i < VF; ++i)
    ConstVec.push_back(ConstantInt::getSigned(RefDestTy, i));
  unsigned Idx = 0;
  CE->getBlobUtils().createConstantBlob(ConstantVector::get(ConstVec), true,
                                        &Idx);
  CE->addBlob(Idx, 1);
  auto *NewRef = DDU.createScalarRegDDRef(GenericRvalSymbase, CE);
  addVPValueWideRefMapping(VPPhi, NewRef);
}

RegDDRef *VPOCodeGenHIR::getUniformScalarRef(const VPValue *VPVal) {
  RegDDRef *ScalarRef = nullptr;
  DDRefUtils &DDU = OrigLoop->getDDRefUtils();

  if (auto *VPExtDef = dyn_cast<VPExternalDef>(VPVal)) {
    const VPOperandHIR *HIROperand = VPExtDef->getOperandHIR();

    if (const auto *Blob = dyn_cast<VPBlob>(HIROperand)) {
      auto *BlobRef = Blob->getBlob();
      ScalarRef = DDU.createSelfBlobRef(BlobRef->getSelfBlobIndex(),
                                        BlobRef->getDefinedAtLevel());
    } else {
      llvm_unreachable("External def is not a VPBlob. Implement support to get "
                       "uniform scalar IV ref.");
    }
  } else if (auto *VPConst = dyn_cast<VPConstant>(VPVal)) {
    ScalarRef = DDU.createConstDDRef(VPConst->getConstant());
  } else {
    llvm_unreachable("Need uniform scalar ref for VPInstruction.");
  }

  return ScalarRef;
}

void VPOCodeGenHIR::widenLoopEntityInst(const VPInstruction *VPInst) {
  HLInst *WInst = nullptr; // Track the last generated wide inst for VPInst
  HLNodeUtils &HNU = OrigLoop->getHLNodeUtils();

  switch (VPInst->getOpcode()) {
  case VPInstruction::ReductionInit: {
    const VPReductionInit *RedInit = cast<VPReductionInit>(VPInst);
    assert(
        ReductionRefs.count(RedInit) &&
        "Reduction init instruction does not have a corresponding RegDDRef.");
    HLContainerTy RedInitHLInsts;
    RegDDRef *RedRef = ReductionRefs[RedInit];
    RegDDRef *IdentityRef = widenRef(RedInit->getIdentityOperand(), getVF());
    // Write the widened identity value into the reduction ref
    HLInst *CopyInst = HNU.createCopyInst(IdentityRef, "red.init", RedRef);
    WInst = CopyInst;
    RedInitHLInsts.push_back(*CopyInst);

    if (RedInit->getNumOperands() > 1) {
      auto *StartVPVal = RedInit->getStartValueOperand();
      assert((isa<VPExternalDef>(StartVPVal) || isa<VPConstant>(StartVPVal)) &&
             "Unsupported reduction start value.");
      // Insert start value into lane 0 of identity vector
      HLInst *InsertElementInst = HNU.createInsertElementInst(
          CopyInst->getLvalDDRef()->clone(), getUniformScalarRef(StartVPVal), 0,
          "red.init.insert", RedRef->clone());
      WInst = InsertElementInst;
      RedInitHLInsts.push_back(*InsertElementInst);
    }
    // TODO: This should be changed to addInst interface once it is aware of
    // Loop PH or Exit.
    HLNodeUtils::insertBefore(RednHoistLp, &RedInitHLInsts);

    // Add the reduction init ref as a live-in for each loop up to and including
    // the hoist loop.
    // TODO: The same ref should also be LiveOut, so that can also be done here
    // only. Any exceptions?
    auto LvalSymbase = WInst->getLvalDDRef()->getSymbase();
    HLLoop *ThisLoop = MainLoop;
    while (ThisLoop != RednHoistLp->getParentLoop()) {
      ThisLoop->addLiveInTemp(LvalSymbase);
      ThisLoop->addLiveOutTemp(LvalSymbase);
      ThisLoop = ThisLoop->getParentLoop();
    }

    addVPValueWideRefMapping(VPInst, WInst->getLvalDDRef());
    return;
  }

  case VPInstruction::ReductionFinal: {
    const VPReductionFinal *RedFinal = cast<VPReductionFinal>(VPInst);
    assert(
        ReductionRefs.count(RedFinal) &&
        "Reduction final instruction does not have a corresponding RegDDRef.");
    HLContainerTy RedTail;

    RegDDRef *VecRef = widenRef(RedFinal->getReducingOperand(), getVF());
    auto Intrin = RedFinal->getVectorReduceIntrinsic();
    Type *ElType = RedFinal->getReducingOperand()->getType();
    if (isa<VectorType>(ElType)) {
      // Incoming vector types is not supported for HIR
      llvm_unreachable("Unsupported vector data type for reducing operand.");
    }

    auto *StartVPVal = RedFinal->getStartValueOperand();
    RegDDRef *Acc = nullptr;

    if (StartVPVal) {
      assert(isa<VPExternalDef>(StartVPVal) &&
             "Unsupported reduction start value.");
      Acc = getUniformScalarRef(StartVPVal);
    }

    // 1. Generate vector reduce intrinsic call
    const VPReduction *RednEntity = VPLoopEntities->getReduction(RedFinal);
    assert(RednEntity && "Reduction final does not have a corresponding "
                         "VPReduction descriptor.");
    // Scalar result of vector reduce intrinsic should be written back to the
    // original reduction descriptor variable NOTE : We obtain this variable
    // from VPLoopEntity corresponding to the reduction since reduction-final
    // instruction may not have accumulators always.
    RegDDRef *RednDescriptor =
        getUniformScalarRef(RednEntity->getRecurrenceStartValue());
    HLInst *VecReduceCall =
        createVectorReduce(Intrin, VecRef, Acc, RednDescriptor, HNU);
    WInst = VecReduceCall;
    RedTail.push_back(*VecReduceCall);

    // 2. If the accumulator is not null, then last value scalar compute is
    // needed, which is of the form %red.init = %red.init OP %vec.reduce NOTE :
    // Acc will always be null for min/max reductions, createVectorReduce
    // asserts on that.
    if (Acc) {
      HLInst *ScalarLastVal = HNU.createBinaryHLInst(
          RedFinal->getBinOpcode(), Acc->clone(),
          VecReduceCall->getLvalDDRef()->clone(), "red.result", Acc);
      WInst = ScalarLastVal;
      RedTail.push_back(*ScalarLastVal);
    }

    // TODO: This should be changed to addInst interface once it is aware of
    // Loop PH or Exit.
    HLNodeUtils::insertAfter(RednHoistLp, &RedTail);
    return;
  }

  case VPInstruction::InductionInit:
  case VPInstruction::InductionInitStep:
  case VPInstruction::InductionFinal: {
    // TODO: Add codegen for induction initialization/finalization
    return;
  }

  default:
    llvm_unreachable("Unsupported VPLoopEntity instruction.");
  }
}

void VPOCodeGenHIR::widenNodeImpl(const VPInstruction *VPInst, RegDDRef *Mask,
                                  const OVLSGroup *Grp,
                                  int64_t InterleaveFactor,
                                  int64_t InterleaveIndex,
                                  const HLInst *GrpStartInst) {
  HLNodeUtils &HNU = OrigLoop->getHLNodeUtils();
  CanonExprUtils &CEU = HNU.getCanonExprUtils();
  DDRefUtils &DDU = OrigLoop->getDDRefUtils();
  HLInst *WInst = nullptr;
  SmallVector<RegDDRef *, 1> CallArgs;
  const HLInst *CallInst = nullptr;

  if (!Mask)
    Mask = CurMaskValue;

  if (auto *VPPhi = dyn_cast<VPPHINode>(VPInst)) {
    widenPhiImpl(VPPhi, Mask);
    return;
  }

  // Skip loop induction related VPInstructions.
  // TODO: Revisit when HIR vectorizer supports outer-loop vectorization.
  if (VPInst->isUnderlyingIRValid()) {
    const HLNode *HNode =
        VPInst->HIR.isMaster()
            ? VPInst->HIR.getUnderlyingNode()
            : VPInst->HIR.getMaster()->HIR.getUnderlyingNode();
    if (isa<HLLoop>(HNode))
      return;
  }

  switch (VPInst->getOpcode()) {
  case VPInstruction::ReductionInit:
  case VPInstruction::ReductionFinal:
  case VPInstruction::InductionInit:
  case VPInstruction::InductionInitStep:
  case VPInstruction::InductionFinal:
    widenLoopEntityInst(VPInst);
    return;
  }

  // Skip widening the first select operand. The select instruction is generated
  // using operands of the instruction corresponding to the select mask.
  bool SkipFirstSelectOp = VPInst->getOpcode() == Instruction::Select;
  SmallVector<RegDDRef *, 6> WideOps;
  for (const VPValue *Operand : VPInst->operands()) {
    if (SkipFirstSelectOp) {
      SkipFirstSelectOp = false;
      continue;
    }
    RegDDRef *WideRef = widenRef(Operand, getVF());
    WideOps.push_back(WideRef);
  }

  switch (VPInst->getOpcode()) {
  case Instruction::FNeg:
    WInst = HNU.createFNeg(WideOps[0], ".vec");
    break;

  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::SRem:
  case Instruction::URem:
  case Instruction::Add:
  case Instruction::FAdd:
  case Instruction::Sub:
  case Instruction::FSub:
  case Instruction::Mul:
  case Instruction::FMul:
  case Instruction::FDiv:
  case Instruction::FRem:
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor: {
    // If this binop instruction corresponds to a reduction, then we need to
    // write the result back to the corresponding reduction variable.
    RegDDRef *RedRef = nullptr;
    if (ReductionRefs.count(VPInst))
      RedRef = ReductionRefs[VPInst];
    WInst = HNU.createBinaryHLInst(VPInst->getOpcode(), WideOps[0], WideOps[1],
                                   ".vec", RedRef ? RedRef->clone() : nullptr);
    break;
  }

  case Instruction::ICmp:
  case Instruction::FCmp: {
    auto *VPCmp = cast<VPCmpInst>(VPInst);
    WInst =
        HNU.createCmp(VPCmp->getPredicate(), WideOps[0], WideOps[1], ".vec");
    break;
  }

  case Instruction::Select: {
    auto PredInst = cast<VPCmpInst>(VPInst->getOperand(0));
    RegDDRef *Pred0, *Pred1;

    // In order for HIR idiom recognition to work, when generating the widened
    // select we need to use operands of the compare instruction corresponding
    // to the select mask.
    Pred0 = widenRef(PredInst->getOperand(0), getVF());
    Pred1 = widenRef(PredInst->getOperand(1), getVF());

    // If this select instruction corresponds to a reduction (min/max), then we
    // need to write the result back to the corresponding reduction variable.
    RegDDRef *RedRef = nullptr;
    if (ReductionRefs.count(VPInst))
      RedRef = ReductionRefs[VPInst];
    WInst = HNU.createSelect(PredInst->getPredicate(), Pred0, Pred1, WideOps[0],
                             WideOps[1], ".vec",
                             RedRef ? RedRef->clone() : nullptr);
    break;
  }

  case Instruction::ZExt:
  case Instruction::SExt:
  case Instruction::FPToUI:
  case Instruction::FPToSI:
  case Instruction::FPExt:
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
  case Instruction::SIToFP:
  case Instruction::UIToFP:
  case Instruction::Trunc:
  case Instruction::FPTrunc:
  case Instruction::BitCast: {
    auto RefDestTy = VPInst->getType();
    auto VecRefDestTy = VectorType::get(RefDestTy, VF);

    // For bitcasts of addressof refs, it is enough to set the bitcast
    // destination type.
    if (VPInst->getOpcode() == Instruction::BitCast &&
        WideOps[0]->isAddressOf()) {
      WideOps[0]->setBitCastDestType(VecRefDestTy);
      addVPValueWideRefMapping(VPInst, WideOps[0]);
      return;
    }
    WInst = HNU.createCastHLInst(VecRefDestTy, VPInst->getOpcode(), WideOps[0],
                                 ".vec");
    break;
  }
  case VPInstruction::SMax:
    WInst = HNU.createSelect(CmpInst::ICMP_SGT, WideOps[0], WideOps[1],
                             WideOps[0]->clone(), WideOps[1]->clone(), ".vec");
    break;

  case VPInstruction::UMax:
    WInst = HNU.createSelect(CmpInst::ICMP_UGT, WideOps[0], WideOps[1],
                             WideOps[0]->clone(), WideOps[1]->clone(), ".vec");
    break;

  case Instruction::GetElementPtr: {
    auto RefDestTy = VPInst->getType();
    auto VecRefDestTy = VectorType::get(RefDestTy, VF);
    auto VPGEP = cast<VPGEPInstruction>(VPInst);

    auto *NewRef = DDU.createAddressOfRef(WideOps[0]->getSelfBlobIndex(),
                                          WideOps[0]->getDefinedAtLevel());
    NewRef->setBitCastDestType(VecRefDestTy);
    SmallVector<const RegDDRef *, 4> AuxRefs;

    // Widened operands contain reference dimensions and trailing struct
    // offsets. Add reference dimensions and trailing struct offsets.
    for (unsigned OpIdx = 1; OpIdx < WideOps.size();) {
      auto Operand = WideOps[OpIdx];
      AuxRefs.push_back(Operand);
      ++OpIdx;
      SmallVector<unsigned, 2> StructOffsets;
      while (OpIdx < WideOps.size() && VPGEP->isOperandStructOffset(OpIdx)) {
        const VPValue *VPOffset = VPInst->getOperand(OpIdx);
        ConstantInt *CVal =
            cast<ConstantInt>(cast<VPConstant>(VPOffset)->getConstant());
        unsigned Offset = CVal->getZExtValue();
        StructOffsets.push_back(Offset);
        ++OpIdx;
      }
      NewRef->addDimension(Operand->getSingleCanonExpr(), StructOffsets);
    }

    NewRef->makeConsistent(AuxRefs, OrigLoop->getNestingLevel());
    addVPValueWideRefMapping(VPInst, NewRef);
    return;
  }

  case Instruction::Load: {
    auto RefDestTy = VPInst->getType();
    auto VecRefDestTy = VectorType::get(RefDestTy, VF);
    unsigned AddressSpace =
        cast<PointerType>(VPInst->getOperand(0)->getType())->getAddressSpace();
    RegDDRef *AddrRef = getPointerOperand(WideOps[0], HNU, DDU, CEU,
                                          VecRefDestTy, AddressSpace, VF);
    // TODO - Alignment information needs to be obtained from VPInstruction.
    // For now we are forcing alignment based on RefDestTy.
    setRefAlignment(RefDestTy, AddrRef);
    WInst = HNU.createLoad(AddrRef, ".vec");
    break;
  }

  case Instruction::Store: {
    auto VecRefDestTy = WideOps[0]->getDestType();
    unsigned AddressSpace =
        cast<PointerType>(VPInst->getOperand(1)->getType())->getAddressSpace();
    RegDDRef *AddrRef = getPointerOperand(WideOps[1], HNU, DDU, CEU,
                                          VecRefDestTy, AddressSpace, VF);
    // TODO - Alignment information needs to be obtained from VPInstruction.
    // For now we are forcing alignment based on scalar type of value being
    // stored.
    setRefAlignment(VPInst->getOperand(0)->getType(), AddrRef);
    WInst = HNU.createStore(WideOps[0], ".vec", AddrRef);
    addInst(WInst, Mask);
    return;
  }

  case Instruction::Call: {
    // TODO - VPValue codegen for call instructions still accesses
    // underlying node. This needs to be changed when we add all
    // necessary information such as call properties to VPInstruction.
    CallInst = dyn_cast<HLInst>(VPInst->HIR.getUnderlyingNode());
    assert(CallInst && CallInst->isCallInst() &&
           "Expected non-null underlying call instruction");

    // The Lval is not represented as an explicit operand in VPInstructions.
    WInst =
        widenCall(CallInst, WideOps, Mask, CallArgs, false /* HasLvalArg */);
    if (!WInst)
      return;
    break;
  }

  case VPInstruction::Not:
    WInst = HNU.createNot(WideOps[0], ".vec");
    break;

  case VPInstruction::Pred:
    // Pred instruction is only used to mark the current block predicate. Simply
    // set the current mask value.
    setCurMaskValue(WideOps[0]);
    return;

  default:
    LLVM_DEBUG(VPInst->dump());
    llvm_unreachable("Unexpected VPInstruction opcode");
  }

  addInst(WInst, Mask);
  if (WInst->hasLval())
    addVPValueWideRefMapping(VPInst, WInst->getLvalDDRef());

  // We need to hook up WideInst before we call analyzeCallArgMemoryReferences
  // as getConstStrideAtLevel expects a ref whose stride is being checked to be
  // attached to HIR. We analyze memory references in call args for stride
  // information if CallArgs is non-empty.
  if (!CallArgs.empty())
    analyzeCallArgMemoryReferences(CallInst, WInst, CallArgs);
}

void VPOCodeGenHIR::createAndMapLoopEntityRefs() {
  HLNodeUtils &HNU = OrigLoop->getHLNodeUtils();

  // Process reductions. For each reduction variable in the loop we create a new
  // RegDDRef to represent it. Next we map the reduction's init, PHI and loop
  // exit instructions to have the new RegDDRef as its underlying reduction
  // RegDDRef. NOTE: The above mentioned instructions are expected to in
  // reduction's LinkedVPValues.
  for (VPReduction *Reduction : VPLoopEntities->vpreductions()) {
    if (Reduction->getIsMemOnly()) {
      VPValue *StartV = Reduction->getRecurrenceStartValue();
      assert(isa<VPExternalDef>(StartV) &&
             "Start value for in-memory reduction is not external def.");
      InMemoryReductionDescriptors.insert(cast<VPExternalDef>(StartV));
    }
    RegDDRef *RednRef = HNU.createTemp(
        VectorType::get(Reduction->getRecurrenceType(), VF), "red.var");

    LLVM_DEBUG(dbgs() << "VPReduction: "; Reduction->dump(dbgs());
               dbgs() << " gets the RegDDRef: "; RednRef->dump(true);
               dbgs() << "\n");

    const SmallVectorImpl<VPValue *> &LinkedVals =
        Reduction->getLinkedVPValues();
    for (auto *V : make_range(LinkedVals.begin(), LinkedVals.end())) {
      assert(ReductionRefs.find(V) == ReductionRefs.end() &&
             "VPValue is already mapped to a reduction RegDDRef.");
      ReductionRefs[V] = RednRef;

      LLVM_DEBUG(dbgs() << "VPValue: "; V->dump();
                 dbgs() << " has the underlying reduction ref: ";
                 RednRef->dump(true); dbgs() << "\n");
    }
  }

  // Process inductions
  // TODO
}

void VPOCodeGenHIR::widenNode(const VPInstruction *VPInst, RegDDRef *Mask,
                              const OVLSGroup *Grp, int64_t InterleaveFactor,
                              int64_t InterleaveIndex,
                              const HLInst *GrpStartInst) {
  if (EnableVPValueCodegenHIR) {
    widenNodeImpl(VPInst, Mask, Grp, InterleaveFactor, InterleaveIndex,
                  GrpStartInst);
    return;
  }

  HLInst *WInst = nullptr;
  const VPInstruction::HIRSpecifics &HIR = VPInst->HIR;
  if (!Mask)
    Mask = CurMaskValue;

  if (HIR.isDecomposed() && VPInst->isUnderlyingIRValid()) {
    // Skip decomposed VPInstruction with valid HIR. This will be codegen'ed by
    // its master VPInstruction.
    LLVM_DEBUG(dbgs() << "Skipping decomposed VPInstruction with valid HIR:"
                      << *VPInst << "\n");
    return;
  }

  LLVM_DEBUG(dbgs() << "Vectorizing: ");
  LLVM_DEBUG(VPInst->dump());

  if (auto Branch = dyn_cast<VPBranchInst>(VPInst)) {
    assert(Branch->getHLGoto() && "For HIR VPBranchInst must have HLGoto.");
    const HLGoto *HGoto = Branch->getHLGoto();
    assert(isSearchLoop() && HGoto->isEarlyExit(getOrigLoop()) &&
           "Only early exit gotos expected!");
    // FIXME: Temporary support for last value computation of live-outs in the
    // early exit branch. 'createNonLinearLiveOutsForEE' introduces the last
    // value computation instructions before the goto instruction for the
    // reaching definitions of the live-outs.
    handleNonLinearEarlyExitLiveOuts(HGoto);

    addInst(HGoto->clone(), nullptr);
    return;
  }

  if (VPInst->isUnderlyingIRValid()) {
    // Master VPInstruction with valid HIR.
    assert(HIR.isMaster() && "VPInstruction with valid HIR must be a Master "
                             "VPInstruction at this point.");
    const HLNode *HNode = HIR.getUnderlyingNode();

    // Generate code using underlying HLInst.
    if (auto *Inst = dyn_cast<HLInst>(HNode)) {
      widenNodeImpl(Inst, Mask, Grp, InterleaveFactor, InterleaveIndex,
                    GrpStartInst, VPInst);
      return;
    }

    if (auto *HIf = dyn_cast<HLIf>(HNode)) {
      // We generate a compare instruction from the IF predicate. The VPValue
      // corresponding to this instruction gets used as the condition bit
      // value for the conditional branch. We need a mapping between this
      // VPValue and the widened value so that we can generate code for the
      // predicate recipes.
      WInst = widenIfNode(HIf, nullptr);
      addVPValueWideRefMapping(VPInst, WInst->getOperandDDRef(0));
      return;
    }
    if (isa<HLLoop>(HNode)) {
      // Master VPInstructions with an attached HLLoop are IV-related or bottom
      // test instructions that don't have explicit instruction representation
      // in HIR. This information will be updated directly when processing the
      // HLLoop construct.
      return;
    }

    llvm_unreachable("Master VPInstruction with unexpected HLDDNode.");
  }

  // For mixed codegen, all instructions under if-else blocks are appropriately
  // masked, hence we don't need to explicitly generate a select for a masked
  // reduction's loop exit blend PHI instruction.
  if (isa<VPPHINode>(VPInst) && ReductionRefs.count(VPInst)) {
    auto *RednEntity = VPLoopEntities->getReduction(VPInst);
    assert(RednEntity && "VPReduction not found for PHI.");
    if (RednEntity->getLoopExitInstr() == VPInst) {
      VPPHINode *StartPhi = VPLoopEntities->getRecurrentVPHINode(*RednEntity);
      // Use the same map entry as the masked reduction's updating instruction
      // for the exiting blend PHI.
      assert(StartPhi && "Masked reduction does not have start PHI.");
      VPValue *UpdatingVPVal = VPInst->getOperand(0) == StartPhi
                                   ? VPInst->getOperand(1)
                                   : VPInst->getOperand(0);
      RegDDRef *Ref = getWideRefForVPVal(UpdatingVPVal);
      assert(Ref && "Wide DDRef not found for UpdatingVPVal.");
      addVPValueWideRefMapping(VPInst, Ref);

      return;
    }
  }

  widenNodeImpl(VPInst, Mask, Grp, InterleaveFactor, InterleaveIndex,
                GrpStartInst);
}

} // end namespace llvm

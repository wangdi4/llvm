//===----- IntelVPOCodeGenHIR.cpp -----------------------------------------===//
//
//   Copyright (C) 2017-2020 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TypeSize.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"
#include "llvm/Transforms/Utils/GeneralUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
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
static cl::opt<bool, true> EnableVPValueCodegenHIROpt(
    "enable-vp-value-codegen-hir", cl::Hidden,
    cl::location(EnableVPValueCodegenHIR),
    cl::desc("Enable VPValue based codegen for HIR vectorizer"));

extern cl::opt<bool> AllowMemorySpeculation;

/// Don't vectorize loops with a known constant trip count below this number if
/// set to a non zero value.
static cl::opt<unsigned> TinyTripCountThreshold(
    "vplan-vectorizer-min-trip-count", cl::init(0), cl::Hidden,
    cl::desc("Don't vectorize loops with a constant "
             "trip count that is smaller than this value."));

static cl::opt<bool> VPlanAssumeMaskedFabsProfitable(
    "vplan-assume-masked-fabs-profitable", cl::init(false), cl::Hidden,
    cl::desc("Allow VPlan codegen to vectorize masked fabs intrinsic assuming "
             "profitability."));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<bool> PrintHIRAfterVPlan(
    "print-hir-after-vplan", cl::init(false),
    cl::desc("Print vectorized HIR after VPlanDriverHIR, on per-HLLoop basis"));
#endif // !NDEBUG || LLVM_ENABLE_DUMP

namespace llvm {
namespace vpo {
bool EnableVPValueCodegenHIR = true;
} // namespace vpo
} // namespace llvm

namespace llvm {

static RegDDRef *getConstantSplatDDRef(DDRefUtils &DDRU, Constant *ConstVal,
                                       unsigned VF) {
  Constant *ConstVec =
      ConstantVector::getSplat(ElementCount(VF, false), ConstVal);
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

HLInst *VPOCodeGenHIR::createReverseVector(RegDDRef *ValRef) {
  unsigned NumElems = cast<VectorType>(ValRef->getDestType())->getNumElements();
  SmallVector<Constant *, 4> ShuffleMask;
  for (unsigned I = 0; I < NumElems; I++) {
    Constant *Mask =
        ConstantInt::get(Type::getInt32Ty(Context), NumElems - I - 1);
    ShuffleMask.push_back(Mask);
  }

  Constant *MaskVec = ConstantVector::get(ShuffleMask);
  RegDDRef *ShuffleMaskRef = DDRefUtilities.createConstDDRef(MaskVec);
  RegDDRef *UndefRef = DDRefUtilities.createUndefDDRef(ValRef->getDestType());

  HLInst *Shuffle = HLNodeUtilities.createShuffleVectorInst(
      ValRef, UndefRef, ShuffleMaskRef, "reverse");
  addInstUnmasked(Shuffle);
  return Shuffle;
}

// Utility to check if a given VPPHINode has been deconstructed via copies. A
// PHI is deconstructed if all its incoming values are copy instructions that
// are tagged with the same origin PHI ID.
static bool isDeconstructedPhi(const VPPHINode *VPPhi) {
  bool PhiBlendsOnlyCopyInsts =
      llvm::all_of(VPPhi->incoming_values(),
                   [](VPValue *V) { return isa<VPHIRCopyInst>(V); });

  if (!PhiBlendsOnlyCopyInsts)
    return false;

  int PhiId = cast<VPHIRCopyInst>(VPPhi->getOperand(0))->getOriginPhiId();
  if (PhiId == -1)
    return false;

  // Check if all incoming copy insts have same PHI ID.
  for (unsigned I = 1; I < VPPhi->getNumOperands(); ++I) {
    int Id = cast<VPHIRCopyInst>(VPPhi->getOperand(I))->getOriginPhiId();
    if (Id != PhiId)
      return false;
  }

  // All checks passed.
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

  Type *VecTy = FixedVectorType::get(DestType, ACG->getVF());
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
  unsigned NumInsts = 0;

public:

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

    if (Opcode == Instruction::Alloca) {
      LLVM_DEBUG(Inst->dump());
      LLVM_DEBUG(
          dbgs() << "VPLAN_OPTREPORT: Loop not handled - alloca instruction\n");
      IsHandled = false;
      return;
    }

    // Handling liveouts for privates/inductions is not implemented in
    // VPValue-based CG.
    auto TLval = Inst->getLvalDDRef();
    if (TLval && TLval->isTerminalRef() &&
        OrigLoop->isLiveOut(TLval->getSymbase())) {
      unsigned RedOpcode = 0;
      if (EnableVPValueCodegenHIR && !CG->isReductionRef(TLval, RedOpcode)) {
        LLVM_DEBUG(Inst->dump());
        LLVM_DEBUG(
            dbgs() << "VPLAN_OPTREPORT: VPValCG liveout "
                      "induction/private not handled - forcing mixed CG\n");
        CG->setForceMixedCG(true);
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
      Intrinsic::ID ID = getVectorIntrinsicIDForCall(Call, TLI);

      bool TrivialVectorIntrinsic =
          (ID != Intrinsic::not_intrinsic) && isTriviallyVectorizable(ID);
      // Prevent vectorization of loop if masked fabs intrinsic vectorization is
      // not profitable specifically for non-AVX512 targets. Check JIRA :
      // CMPLRLLVM-11468.
      if (TrivialVectorIntrinsic && ID == Intrinsic::fabs &&
          !VPlanAssumeMaskedFabsProfitable && !CG->targetHasAVX512())
        TrivialVectorIntrinsic = false;
      if (isa<HLIf>(Inst->getParent()) &&
          (VF > 1 && !TrivialVectorIntrinsic &&
           !TLI->isFunctionVectorizable(CalledFunc, VF))) {
        // Masked svml calls are supported, but masked non-trivially
        // vectorizable intrinsics are not at the moment.
        LLVM_DEBUG(Inst->dump());
        LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop not handled - masked "
                             "non-trivially vectorizable intrinsic\n");
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

      if ((VF > 1 && !TLI->isFunctionVectorizable(CalledFunc, VF)) && !ID) {
        LLVM_DEBUG(
            dbgs()
            << "VPLAN_OPTREPORT: Loop not handled - call not vectorizable\n");
        IsHandled = false;
        return;
      }

      // If the always scalar operand of vector intrinsic call is loop variant
      // then we bail out of vectorization.
      // TODO: In the future such calls should be serialized in outgoing vector
      // code.
      if (ID != Intrinsic::not_intrinsic) {
        unsigned ArgOffset = Inst->hasLval() ? 1 : 0;
        for (unsigned I = 0; I < Call->getNumArgOperands(); ++I) {
          if (hasVectorInstrinsicScalarOpd(ID, I)) {
            auto *OperandCE =
                Inst->getOperandDDRef(ArgOffset + I)->getSingleCanonExpr();
            if (!OperandCE->isInvariantAtLevel(OrigLoop->getNestingLevel())) {
              LLVM_DEBUG(dbgs()
                         << "VPLAN_OPTREPORT: Loop not handled - always scalar "
                            "operand of vector intrinsic is loop variant\n");
              IsHandled = false;
              return;
            }
          }
        }
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
  BlobUtils &BlobUtilities = CExpr->getBlobUtils();
  CExpr->collectBlobIndices(BlobIndices, true /* MakeUnique */);
  if (EnableNestedBlobVec) {
    if (InMaskedStmt) {
      for (auto &BI : BlobIndices) {
        auto TopBlob = BlobUtilities.getBlob(BI);
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
    auto TopBlob = BlobUtilities.getBlob(BI);

    if (BlobUtilities.isNestedBlob(TopBlob)) {
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
  // Search loop representation is not explicit. Force mixed CG.
  if (isSearchLoop())
    setForceMixedCG(true);

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

void VPOCodeGenHIR::setRednHoistPtForVectorLoop() {
  // If loop has any reductions then find appropriate HLLoop that reduction
  // init/finalize can be hoisted out to. If loop has no reductions then hoist
  // loop is set to current loop being vectorized.
  if (!ReductionRefs.empty())
    RednHoistLp = findRednHoistInsertionPoint(OrigLoop);
  else
    RednHoistLp = OrigLoop;
}

bool VPOCodeGenHIR::initializeVectorLoop(unsigned int VF, unsigned int UF) {
  assert(VF > 1);
  setVF(VF);
  assert(UF > 0);
  setUF(UF);
  assert(RednHoistLp &&
         "Decision about reduction hoist loop should be available here.");

  LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: VPlan handled loop, VF = " << VF << " "
                    << Fn.getName() << "\n");
  LLVM_DEBUG(dbgs() << "Handled loop before vec codegen: \n");
  LLVM_DEBUG(OrigLoop->dump(1));

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
  // and lower or equal than VF * UF since we have already made the decision of
  // vectorizing that loop with such a VF * UF.
  uint64_t TripCount = 0;
  bool IsTCValidForPeeling =
      OrigLoop->isConstTripLoop(&TripCount) && TripCount <= VF * UF ? false
                                                                    : true;
  if (SearchLoopNeedsPeeling && !IsTCValidForPeeling)
    LLVM_DEBUG(dbgs() << "Can't peel loop : VF(" << VF << ") * UF(" << UF
                      << ") >= TC(" << TripCount << ")!\n");

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

  assert(!(NeedFirstIterationPeelLoop && SearchLoopPeelArrayRef) &&
       "First iteration peeling idiom cannot have SearchLoopPeelArrayRef set.");

  if (NeedFirstIterationPeelLoop) {
    PeelLoop = OrigLoop->peelFirstIteration(
                false /*OrigLoop will also executed peeled its.*/);
  }

  auto MainLoop = HIRTransformUtils::setupPeelMainAndRemainderLoops(
      OrigLoop, VF * UF, NeedRemainderLoop, LORBuilder, OptimizationType::Vectorizer,
      &PeelLoop, SearchLoopPeelArrayRef, &RTChecks);

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

  // If reduction hoist loop is not the main vector loop, then we need to
  // invalidate parent region of reduction hoist loop as well since new
  // reduction initialization/finalization instructions are inserted into its
  // preheader/postexit.
  if (RednHoistLp != MainLoop)
    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(RednHoistLp);

  RedInitInsertPoint = RednHoistLp;
  RedFinalInsertPoint = RednHoistLp;

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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPOCodeGenHIR::dumpFinalHIR() {
  if (NeedPeelLoop)
    PeelLoop->dump();
  MainLoop->getParent()->dump();
  if (NeedRemainderLoop)
    OrigLoop->dump();
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

void VPOCodeGenHIR::finalizeVectorLoop(void) {
  finalizeGotos();

  LLVM_DEBUG(dbgs() << "\n\n\nHandled loop after: \n");
  LLVM_DEBUG(dumpFinalHIR());
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (PrintHIRAfterVPlan) {
    dbgs() <<"Handled loop after VPlan:\n";
    dumpFinalHIR();
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

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
      HLInstCounter InstCounter;
      HLNodeUtils::visitRange(InstCounter, OrigLoop->child_begin(),
                              OrigLoop->child_end());
      if (InstCounter.getNumInsts() <= SmallLoopBodyThreshold)
        HIRTransformUtils::completeUnroll(MainLoop);
    }
    HLNodeUtils::remove(OrigLoop);
  }
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
    ++OptRptStats.VectorMathCalls;

    SmallVector<RegDDRef *, 1> CallArgs;
    SmallVector<Type *, 1> ArgTys;
    SmallVector<AttributeSet, 1> ArgAttrs;

    // Glibc sincos function uses pointers as out parameters. They need to be
    // discarded since SVML sincos functions don't have them.
    auto ItEnd = HInst->rval_op_ddref_end();
    if (FnName.find("sincos") != StringRef::npos)
      ItEnd -= 2;

    // For each call argument, insert a scalar load of the element,
    // broadcast it to a vector.
    unsigned ArgNum = 0;
    for (auto It = HInst->rval_op_ddref_begin(); It != ItEnd; ++It, ++ArgNum) {
      // TODO: it is assumed that call arguments need to become vector.
      // In the future, some vectorizable calls may contain scalar
      // arguments. Additional checking is needed for these cases.

      // The DDRef of the original scalar call instruction.
      RegDDRef *Ref = *It;

      // The resulting type of the widened ref/broadcast.
      auto VecDestTy = FixedVectorType::get(Ref->getDestType(), VF);

      RegDDRef *WideRef = nullptr;
      HLInst *LoadInst = nullptr;

      // Create the scalar load of the call argument. This is done so that
      // we can clone the new LvalDDRef and change its type to force the
      // broadcast. See %load in the example above. Essentially, the original
      // scalar %load becomes bitcast.float.<4 x float>, which is how HIRCG
      // knows to do the broadcast.
      if (Ref->isMemRef()) {
        // Ref is a memory reference: %t = sinf(a[i]);
        LoadInst = HLNodeUtilities.createLoad(Ref->clone(), "load");
      } else {
        // Ref in this case is a temp from a previous load: %r = sinf(%t).
        // Create a new temp and broadcast it for the call argument.
        LoadInst = HLNodeUtilities.createCopyInst(Ref->clone(), "copy");
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
      ArgAttrs.push_back(Call->getAttributes().getParamAttributes(ArgNum));
    }

    // Using the newly created vector call arguments, generate the vector
    // call instruction and extract the low element.
    Function *VectorF = getOrInsertVectorFunction(
        F, VF, ArgTys, TLI, Intrinsic::not_intrinsic, nullptr /*simd function*/,
        false /*non-masked*/);
    assert(VectorF && "Can't create vector function.");

    FastMathFlags FMF =
        isa<FPMathOperator>(Call) ? Call->getFastMathFlags() : FastMathFlags();
    HLInst *WideCall = HLNodeUtilities.createCall(
        VectorF, CallArgs, VectorF->getName(), nullptr /*Lval*/, {} /*Bundle*/,
        {} /*BundleOps*/, FMF);
    HLNodeUtils::insertBefore(HInst, WideCall);

    // Make sure we don't lose attributes at the call site. E.g., IMF
    // attributes are taken from call sites in MapIntrinToIml to refine
    // SVML calls for precision.
    copyRequiredAttributes(Call,
                           cast<CallInst>(const_cast<Instruction *>(
                               WideCall->getLLVMInstruction())),
                           ArgAttrs);

    // Set calling conventions for SVML function calls
    if (isSVMLFunction(TLI, FnName, VectorF->getName())) {
      Instruction *WideInst =
          const_cast<Instruction *>(WideCall->getLLVMInstruction());
      cast<CallInst>(WideInst)->setCallingConv(CallingConv::SVML);
    }

    // Handle results of SVML sincos function calls
    if (VectorF->getName().startswith("__svml_sincos")) {
      generateStoreForSinCos(HInst, WideCall, nullptr /* Mask */,
                             true /* IsRemainderLoop */);
    }

    InstsToRemove.push_back(HInst);

    if (auto LvalDDRef = HInst->getLvalDDRef()) {
      HLInst *ExtractInst = HLNodeUtilities.createExtractElementInst(
          WideCall->getLvalDDRef()->clone(), unsigned(0), "elem",
          LvalDDRef->clone());
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
  auto VecRefDestTy = FixedVectorType::get(RefDestTy, VF);
  auto RefSrcTy = Ref->getSrcType();
  auto VecRefSrcTy = FixedVectorType::get(RefSrcTy, VF);

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
      llvm_unreachable(
          "HIR vectorizer is trying to handle reductions without entities.");

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

    propagateMetadata(WideRef, nullptr /*VLS Group*/, Ref);
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
  if (isUnitStrideRef(Ref) || InterLeaveAccess) {
    if (CurrentVPInstUnrollPart > 0)
      WideRef->shift(NestingLevel, CurrentVPInstUnrollPart * VF);
    return WideRef;
  }

  SmallVector<const RegDDRef *, 4> AuxRefs;
  RegDDRef::CanonExprsTy WideRefCEs;
  if (WideRef->hasGEPInfo()) {
    // If the base CE of this memref is loop variant then add it to the list
    // of CEs to be updated for widening.
    if (!WideRef->getBaseCE()->isInvariantAtLevel(NestingLevel)) {
      WideRefCEs.push_back(WideRef->getBaseCE());
    }

    // If the lower/stride for any of the dimensions of this memref are loop
    // variant, then add it to the list of CEs to be updated for widening.
    for (unsigned Dim = WideRef->getNumDimensions(); Dim > 0; --Dim) {
      auto *Lower = WideRef->getDimensionLower(Dim);
      auto *Stride = WideRef->getDimensionStride(Dim);
      if (!Lower->isInvariantAtLevel(NestingLevel))
        WideRefCEs.push_back(Lower);
      if (!Stride->isInvariantAtLevel(NestingLevel))
        WideRefCEs.push_back(Stride);
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
        CA.push_back(ConstantInt::getSigned(
            Int64Ty, ConstCoeff * (i + CurrentVPInstUnrollPart * VF)));
      }
      ArrayRef<Constant *> AR(CA);
      auto CV = ConstantVector::get(AR);

      if (BlobCoeff != InvalidBlobIndex) {
        // Compute Addend = WidenedBlob * CV and add Addend to the canon
        // expression
        NestedBlobCG CGBlob(Ref, HLNodeUtilities, DDRefUtilities, this,
                            nullptr);

        auto NewRef = CGBlob.visit(BlobUtilities.getBlob(BlobCoeff));
        auto CRef = DDRefUtilities.createConstDDRef(CV);

        auto TWideInst = HLNodeUtilities.createBinaryHLInst(
            Instruction::Mul, NewRef->clone(), CRef, ".BlobMul");
        addInst(TWideInst, nullptr);
        AuxRefs.push_back(TWideInst->getLvalDDRef());
        CE->addBlob(TWideInst->getLvalDDRef()
                        ->getSingleCanonExpr()
                        ->getSingleBlobIndex(),
                    1);
      } else {
        unsigned Idx = 0;
        BlobUtilities.createConstantBlob(CV, true, &Idx);
        CE->addBlob(Idx, 1);
      }
    }

    for (auto &BI : BlobIndices) {
      auto TopBlob = BlobUtilities.getBlob(BI);

      // We do not need to widen invariant blobs - check for blob invariance
      // by comparing maxbloblevel against the loop's nesting level.
      if (WideRef->findMaxBlobLevel(BI) < NestingLevel)
        continue;

      if (BlobUtilities.isNestedBlob(TopBlob)) {
        NestedBlobCG CGBlob(Ref, HLNodeUtilities, DDRefUtilities, this,
                            nullptr);

        auto NewRef = CGBlob.visit(TopBlob);

        AuxRefs.push_back(NewRef);
        CE->replaceBlob(BI, NewRef->getSingleCanonExpr()->getSingleBlobIndex());
        continue;
      }

      assert(BlobUtilities.isTempBlob(TopBlob) && "Only temp blobs expected");

      auto OldSymbase = BlobUtilities.getTempBlobSymbase(BI);

      // A temp blob not widened before is a loop invariant - it will be
      // broadcast in HIRCG when needed.
      if (auto *WRef = getWideRef(OldSymbase)) {
        AuxRefs.push_back(WRef);
        CE->replaceBlob(BI, WRef->getSingleCanonExpr()->getSingleBlobIndex());
      }
    }

    auto VecCEDestTy = FixedVectorType::get(CE->getDestType(), VF);
    auto VecCESrcTy = FixedVectorType::get(CE->getSrcType(), VF);

    CE->setDestType(VecCEDestTy);
    CE->setSrcType(VecCESrcTy);
  }

  // The blobs in the scalar ref have been replaced by widened refs, call
  // the utility to update the widened Ref consistent.
  WideRef->makeConsistent(AuxRefs, NestingLevel);
  return WideRef;
}

/// Return result of combining horizontal vector binary operation with initial
/// value. Instead of splitting VecRef recursively into 2 parts of half VF until
/// the VF becomes 2, VecRef is shuffled in such a way that the resulting vector
/// stays VF-wide and the upper elements are shuffled down into the lower
/// positions to form a new vector. Then, the horizontal operation is performed
/// on VF-wide vectors, but the upper elements of the operation are simply
/// ignored. The final result of the horizontal operation is then extracted from
/// position 0, the leftmost position of the vector. The rightmost position
/// is 7. E.g., if VF=8, we will have 3 horizontal operation stages. So, if we
/// start with elements <0,2,1,4,5,1,3,0> and the horizontal operation is add,
/// we end up with the following sequence of operations, where u is undefined.
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
  HLNodeUtils &HNU = HLLp->getHLNodeUtils();
  DDRefUtils &DDRU = HLLp->getDDRefUtils();
  LLVMContext &Context = HNU.getContext();
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
    RegDDRef *MaskVecDDRef = DDRU.createConstDDRef(MaskVec);
    HLInst *Shuffle = HNU.createShuffleVectorInst(
        LastVal->clone(), LastVal->clone(), MaskVecDDRef, "rdx.shuf");
    HLInst *BinOp = HNU.createBinaryHLInst(
        BOpcode, LastVal->clone(), Shuffle->getLvalDDRef()->clone(), "bin.rdx");
    InstContainer.push_back(*Shuffle);
    InstContainer.push_back(*BinOp);
    LastVal = BinOp->getLvalDDRef();
  }

  HLInst *Extract = HNU.createExtractElementInst(LastVal->clone(), unsigned(0),
                                                 "bin.final", ResultRefClone);
  InstContainer.push_back(*Extract);

  return Extract;
}

/// Create a call to llvm.experimental.vector.reduce intrinsic in order to
/// perform horizontal reduction on vector ref \p VecRef. The scalar return
/// value of this intrinsic call is stored back to the reduction descriptor
/// variable \p RednDescriptor. Overloaded parameters of intrinsic call is
/// determined based on type of reduction. Some examples -
/// 1. Integer add reduction for VF=4 will generate -
///    declare i32 @llvm.experimental.vector.reduce.add.i32.v4i32(<4 x i32>)
///
/// 2. FP add reduction for VF=4 will generate -
///    declare float @llvm.experimental.vector.reduce.fadd.f32.f32.v4f32(float,
///                                                                 <4 x float>)
static HLInst *createVectorReduce(const VPReductionFinal *RedFinal,
                                  RegDDRef *VecRef, RegDDRef *&Acc,
                                  RegDDRef *RednDescriptor,
                                  HLNodeUtils &HLNodeUtilities) {
  Intrinsic::ID VecRedIntrin = RedFinal->getVectorReduceIntrinsic();
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
    return HLNodeUtilities.createFPMinMaxVectorReduce(
        VecRef, VecRedIntrin, false /*NoNaN*/, RednDescriptor);
  default:
    llvm_unreachable("unsupported reduction");
    break;
  }

  Function *VecReduceFunc = Intrinsic::getDeclaration(
      &HLNodeUtilities.getModule(), VecRedIntrin, Tys);
  LLVM_DEBUG(dbgs() << "Vector reduce func: "; VecReduceFunc->dump());

  FastMathFlags FMF = RedFinal->hasFastMathFlags()
                          ? RedFinal->getFastMathFlags()
                          : FastMathFlags();

  return HLNodeUtilities.createCall(VecReduceFunc, Ops, "vec.reduce",
                                    RednDescriptor, {} /*Bundle*/,
                                    {} /*BundleOps*/, FMF);
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
      HLNodeUtilities.createCmp(*PredIt, WideLHS, WideRHS, "wide.cmp.");
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
    CurWideInst = HLNodeUtilities.createAnd(
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
    HLInst *BitCastInst =
        createBitCast(IntTy, Ref, nullptr /* Container */, "intmask");
    createHLIf(PredicateTy::ICMP_NE, BitCastInst->getLvalDDRef()->clone(),
               DDRefUtilities.createConstDDRef(IntTy, 0));
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

void VPOCodeGenHIR::propagateMetadata(RegDDRef *NewRef, const OVLSGroup *Group,
                                      const RegDDRef *OldRef) {
  SmallVector<unsigned, 6> PreservedMDKinds = {
      LLVMContext::MD_tbaa,        LLVMContext::MD_alias_scope,
      LLVMContext::MD_noalias,     LLVMContext::MD_fpmath,
      LLVMContext::MD_nontemporal, LLVMContext::MD_invariant_load};

  // Start out by clearing all non-debug related metadata. The metadata for
  // kinds in the preserved set is added later.
  RegDDRef::MDNodesTy MDs;
  NewRef->getAllMetadataOtherThanDebugLoc(MDs);
  for (auto It : MDs) {
    LLVM_DEBUG(dbgs() << "Cleared metadata kind: " << It.first << "\n");
    NewRef->setMetadata(It.first, nullptr);
  }

  // TODO - we need to ensure that metadata is being set properly with
  // VPValue based code generation.
  SmallVector<const RegDDRef *, 4> MemDDRefVec;
  if (Group)
    for (int64_t Index = 0, Size = Group->size(); Index < Size; ++Index) {
      auto *Memref = cast<VPVLSClientMemrefHIR>(Group->getMemref(Index));
      MemDDRefVec.push_back(Memref->getRegDDRef());
    }
  else {
    assert(OldRef && "Expected non null reference");
    assert(OldRef->hasGEPInfo() && "Expected reference with GEP info");
    MemDDRefVec.push_back(OldRef);
  }

  const RegDDRef *R0 = MemDDRefVec[0];
  for (auto Kind : PreservedMDKinds) {
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

void VPOCodeGenHIR::propagateDebugLocation(const VPInstruction *VPInst) {
  auto SetDebugLoc = [](RegDDRef *Ref, DebugLoc DbgLoc) {
    if (Ref->hasGEPInfo()) {
      // TODO: This should change when VPValue-based CG starts supporting
      // folding of memrefs i.e. %1 = A[i1] + B[i1].
      assert(Ref->isAddressOf() &&
             "Only address of memrefs are expected in WidenMap.");
      Ref->setGepDebugLoc(DbgLoc);
    } else if (Ref->getHLDDNode() == nullptr) {
      // Standalone RegDDRefs are created only when VPInsts are mapped to folded
      // CEs. Propagate debug location to the single CE attached to the Ref.
      assert(Ref->isTerminalRef() && "Terminal ref expected.");
      Ref->getSingleCanonExpr()->setDebugLoc(DbgLoc);
    } else {
      // All remaining Refs in the map are expected to be lval of generated
      // HLInsts.
      assert(Ref->isLval() && Ref->isTerminalRef() &&
             "Terminal lval ref expected.");
      auto *HInst = cast<HLInst>(Ref->getHLDDNode());
      if (isa<LoadInst>(HInst->getLLVMInstruction())) {
        // For a load instruction, set debug location on rval memref.
        RegDDRef *RvalRef = HInst->getRvalDDRef();
        assert(RvalRef->hasGEPInfo() &&
               "Memref expected here for load HLInst.");
        RvalRef->setMemDebugLoc(DbgLoc);
      } else {
        // For non-load instructions, DebugLoc should be set at HLInst level.
        // Since the instruction is known to be newly created set the debug
        // location directly on underlying dummy LLVMInst.
        Instruction *LLVMInst =
            const_cast<Instruction *>(HInst->getLLVMInstruction());
        LLVMInst->setDebugLoc(DbgLoc);
      }
    }
  };

  DebugLoc DbgLoc = VPInst->getDebugLocation();

  // Nothing to do if DbgLoc is not available.
  if (!DbgLoc)
    return;

  // Set debug location for widened component of VPInstruction.
  RegDDRef *VecRef = getWideRefForVPVal(VPInst);
  if (VecRef)
    SetDebugLoc(VecRef, DbgLoc);

  // Set debug location for scalarized component of VPInstruction.
  RegDDRef *ScalRef = getScalRefForVPVal(VPInst, 0 /*Lane*/);
  if (ScalRef)
    SetDebugLoc(ScalRef, DbgLoc);
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
    RegDDRef *ExtMaskDDRef = DDRefUtilities.createConstDDRef(ExtMask);
    RegDDRef *UndefDDRef = DDRefUtilities.createUndefDDRef(VecTy2);

    HLInst *ExtShuffle = HLNodeUtilities.createShuffleVectorInst(
        V2->clone(), UndefDDRef, ExtMaskDDRef, "ext.shuf");
    addInst(ExtShuffle, Mask);
    V2 = ExtShuffle->getLvalDDRef();
  }

  // V1 and V2 are same length now - join the two vectors.
  Constant *CombMask = createSequentialMask(0, NumElts1 + NumElts2, 0, Context);
  RegDDRef *CombMaskDDRef = DDRefUtilities.createConstDDRef(CombMask);

  HLInst *CombShuffle = HLNodeUtilities.createShuffleVectorInst(
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
  RegDDRef *ShuffleMaskRef = DDRefUtilities.createConstDDRef(MaskVec);
  RegDDRef *UndefRef = DDRefUtilities.createUndefDDRef(WLoadRes->getDestType());

  // Create shuffle instruction using the result of the wide load and the
  // computed shuffle mask.
  RegDDRef *WLvalRef = widenRef(LvalRef, getVF());
  HLInst *Shuffle = HLNodeUtilities.createShuffleVectorInst(
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
  RegDDRef *UndefRef =
      DDRefUtilities.createUndefDDRef(ConcatVec->getDestType());

  Constant *InterleaveMask =
      createInterleaveMask(Context, getVF(), InterleaveFactor);
  RegDDRef *InterleaveMaskRef = DDRefUtilities.createConstDDRef(InterleaveMask);

  HLInst *Shuffle = HLNodeUtilities.createShuffleVectorInst(
      ConcatVec->clone(), UndefRef, InterleaveMaskRef, "vls.interleave");
  addInst(Shuffle, Mask);

  // Create the wide store using the shuffled vector values and the widened
  // store pointer reference.
  RegDDRef *ShuffleRef = Shuffle->getLvalDDRef();
  RegDDRef *WStorePtrRef =
      widenRef(StorePtrRef, getVF() * InterleaveFactor, true);
  HLInst *WideStore = HLNodeUtilities.createStore(ShuffleRef->clone(),
                                                  ".vls.store", WStorePtrRef);
  addInst(WideStore, Mask);

  return WideStore;
}

HLInst *VPOCodeGenHIR::widenInterleavedAccess(
    const HLInst *INode, RegDDRef *Mask, const OVLSGroup *Grp,
    int64_t InterleaveFactor, int64_t InterleaveIndex,
    const HLInst *GrpStartInst, const VPInstruction *VPInst) {
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
      assert(WMemRef && "The memory reference should not be null pointer");
      HLInst *WideLoad =
          HLNodeUtilities.createLoad(WMemRef, CurInst->getName() + ".vls.load");
      propagateMetadata(WMemRef, Grp);

      addInst(WideLoad, Mask);

      // Set the result of the wide load and add the same to VLS Group load map.
      WLoadRes = WideLoad->getLvalDDRef();
      VLSGroupLoadMap[Grp] = WLoadRes;

      DEBUG_WITH_TYPE("ovls",
                      dbgs() << "Emitted a group-wide vector LOAD for Group#"
                             << Grp->getDebugId() << ":\n  ");
      DEBUG_WITH_TYPE("ovls", WideLoad->dump());
    } else
      WLoadRes = (*It).second;

    WideInst = createInterleavedLoad(INode->getLvalDDRef(), WLoadRes,
                                     InterleaveFactor, InterleaveIndex, Mask);
    // Map the generated DDRef to corresponding VPInstruction.
    addVPValueWideRefMapping(VPInst, WideInst->getLvalDDRef());
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
      propagateMetadata(WideInst->getOperandDDRef(0), Grp);

      DEBUG_WITH_TYPE("ovls",
                      dbgs() << "Emitted a group-wide vector STORE for Group#"
                             << Grp->getDebugId() << ":\n  ");
      DEBUG_WITH_TYPE("ovls", WideInst->dump());
    }
  }

  return WideInst;
}

HLInst *VPOCodeGenHIR::createBitCast(Type *Ty, RegDDRef *Ref,
                                     HLContainerTy *Container,
                                     const Twine &Name) {
  HLInst *BitCastInst = HLNodeUtilities.createBitCast(Ty, Ref->clone(), Name);
  if (Container)
    Container->push_back(*BitCastInst);
  else
    addInstUnmasked(BitCastInst);
  return BitCastInst;
}

// Generate cttz(bsf) call for a given Ref.
//    %intmask = bitcast.i32(Ref);
//    %bsf = call intrinsic.cttz.i32(Ref);
HLInst *VPOCodeGenHIR::createCTZCall(RegDDRef *Ref, llvm::Intrinsic::ID Id,
                                     bool MaskIsNonZero,
                                     HLContainerTy *Container,
                                     const Twine &Name) {
  assert(Ref && "Ref is expected for this assignment.");
  assert((Id == Intrinsic::cttz || Id == Intrinsic::ctlz) &&
         "Unexpected intrinsic");

  // It's necessary to bitcast mask to integer, otherwise it's not possible to
  // use it in cttz instruction.
  Type *RefTy = Ref->getDestType();
  LLVMContext &Context = *Plan->getLLVMContext();
  Type *IntTy = IntegerType::get(Context, RefTy->getPrimitiveSizeInBits());
  HLInst *BitCastInst = createBitCast(IntTy, Ref, Container, Name + "intmask");
  RegDDRef *IntRef = BitCastInst->getLvalDDRef();
  // As soon as this code has to be under if condition, return value from cttz
  // can be undefined if mask is zero. However if we know for a fact that mask
  // is non-zero, then inform intrinsic that zero produces defined result.
  Type *BoolTy = IntegerType::get(Context, 1);
  Function *BsfFunc =
      Intrinsic::getDeclaration(&HLNodeUtilities.getModule(), Id, {IntTy});
  SmallVector<RegDDRef *, 1> Args = {
      IntRef->clone(), DDRefUtilities.createConstDDRef(BoolTy, MaskIsNonZero)};
  HLInst *BsfCall = HLNodeUtilities.createCall(BsfFunc, Args, Name);
  if (Container)
    Container->push_back(*BsfCall);
  else
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
  HLInst *BsfCall = createCTZCall(Mask, Intrinsic::cttz, MaskIsNonZero);

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
    BlobUtilities.createMulBlob(
        BlobUtilities.getBlob(CE->getIVBlobCoeff(Level)),
        BlobUtilities.getBlob(Index), true, &Index);
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
  HLNodeUtilities.visit(LOVisitor, OrigLoop);

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
    auto Extract =
        HLNodeUtilities.createExtractElementInst(WideRef, VF - 1, "last");
    addInst(Extract, nullptr);
    // overwrite Rval for store with Lval of extractelement instruction
    Rval = Extract->getLvalDDRef();
    LLVM_DEBUG(dbgs() << "VPCodegen: widened Rval : "; Rval->dump();
               dbgs() << "\n");
  }

  HLInst *WideInst = HLNodeUtilities.createStore(
      Rval->clone(), CurInst->getName() + ".uniform.store", Lval->clone());
  LLVM_DEBUG(dbgs() << "VPCodegen: WideInst : "; WideInst->dump();
             dbgs() << "\n");
  addInst(WideInst, nullptr);

  return WideInst;
}

void VPOCodeGenHIR::addMaskToSVMLCall(
    Function *OrigF, AttributeList OrigAttrs,
    SmallVectorImpl<RegDDRef *> &VecArgs, SmallVectorImpl<Type *> &VecArgTys,
    SmallVectorImpl<AttributeSet> &VecArgAttrs, RegDDRef *MaskValue) {
  assert(MaskValue && "Expected mask to be present");
  VectorType *VecTy = cast<VectorType>(VecArgTys[0]);

  if (VecTy->getPrimitiveSizeInBits().getFixedSize() < 512) {
    // For 128-bit and 256-bit masked calls, mask value is appended to the
    // parameter list. For example:
    //
    //  %sin.vec = call <4 x float> @__svml_sinf4_mask(<4 x float>, <4 x i32>)
    VectorType *MaskTyExt =
        VectorType::get(IntegerType::get(Context, VecTy->getScalarSizeInBits()),
                        VecTy->getElementCount());
    HLInst *MaskValueExt =
        HLNodeUtilities.createSExt(MaskTyExt, MaskValue->clone());
    addInst(MaskValueExt, nullptr);
    VecArgTys.push_back(MaskTyExt);
    VecArgs.push_back(MaskValueExt->getLvalDDRef()->clone());
    VecArgAttrs.push_back(AttributeSet());
  } else {
    // Compared with 128-bit and 256-bit calls, 512-bit masked calls need extra
    // pass-through source parameters. We don't care about masked-out lanes, so
    // just pass undef for that parameter. For example:
    //
    // %sin.vec = call { <16 x float>, <16 x float> } @__svml_sinf16_mask(
    //            <16 x float>, <16 x i1>, <16 x float>)
    SmallVector<Type *, 1> NewArgTys;
    SmallVector<RegDDRef *, 1> NewArgs;
    SmallVector<AttributeSet, 1> NewArgAttrs;

    Type *SourceTy = VecTy;
    StringRef FnName = OrigF->getName();
    if (FnName == "sincos" || FnName == "sincosf")
      SourceTy = StructType::get(VecTy, VecTy);

    RegDDRef *UndefDDRef = DDRefUtilities.createUndefDDRef(SourceTy);

    NewArgTys.push_back(SourceTy);
    NewArgs.push_back(UndefDDRef);
    NewArgAttrs.push_back(AttributeSet());

    CanonExpr *CE = MaskValue->getSingleCanonExpr();
    NewArgTys.push_back(CE->getDestType());
    NewArgs.push_back(MaskValue->clone());
    NewArgAttrs.push_back(AttributeSet());

    NewArgTys.append(VecArgTys.begin(), VecArgTys.end());
    NewArgAttrs.append(VecArgAttrs.begin(), VecArgAttrs.end());
    for (RegDDRef *Arg : VecArgs)
      NewArgs.push_back(Arg->clone());

    VecArgTys = std::move(NewArgTys);
    VecArgs = std::move(NewArgs);
    VecArgAttrs = std::move(NewArgAttrs);
  }
}

void VPOCodeGenHIR::generateStoreForSinCos(const HLInst *HInst,
                                           HLInst *WideInst,
                                           RegDDRef *Mask,
                                           bool IsRemainderLoop) {
  // Track the latest instruction inserted by this function.
  // Only used for remainder loop. addInst() is used for main loop instead.
  HLInst *InsertionPoint = WideInst;

  auto AppendInst = [this, IsRemainderLoop, &InsertionPoint](
                        HLInst *NewHInst, RegDDRef *Mask) -> void {
    if (!IsRemainderLoop) {
      addInst(NewHInst, Mask);
    } else {
      // Append new instruction to the remainder loop (instead of main loop)
      HLNodeUtils::insertAfter(InsertionPoint, NewHInst);
      InsertionPoint = NewHInst;
    }
  };

  auto *ExtractSinInst = HLNodeUtilities.createExtractValueInst(
      WideInst->getLvalDDRef()->clone(), {0}, "sincos.sin");
  auto *ExtractCosInst = HLNodeUtilities.createExtractValueInst(
      WideInst->getLvalDDRef()->clone(), {1}, "sincos.cos");
  AppendInst(ExtractSinInst, nullptr);
  AppendInst(ExtractCosInst, nullptr);

  auto generateStore = [this, Mask, IsRemainderLoop, AppendInst](
                           const RegDDRef *VecValueRef, const RegDDRef *AddrRef,
                           const Twine &Name) {
    RegDDRef *MemRef = AddrRef->clone();
    MemRef->setAddressOf(false);

    // If we're generating code for remainder loop, only the first lane is
    // needed.
    HLInst *Store;
    if (!IsRemainderLoop) {
      bool IsUnitStride = false;
      int64_t Stride;
      // This should use RegDDRef::isUnitStride(), but neither MemRef nor
      // AddrRef is eligible for calling it.
      if (AddrRef->getConstStrideAtLevel(OrigLoop->getNestingLevel(), &Stride))
        IsUnitStride =
            (Stride == static_cast<int64_t>(MemRef->getDestTypeSizeInBytes()));
      RegDDRef *VecMemRef = widenRef(MemRef, getVF(), IsUnitStride);
      Store =
          HLNodeUtilities.createStore(VecValueRef->clone(), Name, VecMemRef);
    } else {
      HLInst *ExtractInst = HLNodeUtilities.createExtractElementInst(
          VecValueRef->clone(), unsigned(0), "elem");
      AppendInst(ExtractInst, nullptr);
      Store = HLNodeUtilities.createStore(ExtractInst->getLvalDDRef()->clone(),
                                          Name, MemRef);
    }
    AppendInst(Store, Mask);
  };

  generateStore(ExtractSinInst->getLvalDDRef(), HInst->getOperandDDRef(1),
                "sincos.sin.store");
  generateStore(ExtractCosInst->getLvalDDRef(), HInst->getOperandDDRef(2),
                "sincos.cos.store");
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
  unsigned ArgIgnored = 0;
  // glibc scalar sincos function has 2 pointer out parameters, but SVML sincos
  // functions return the results directly in a struct. The pointers should be
  // omitted in vectorized call.
  if (FnName == "sincos" || FnName == "sincosf")
    ArgIgnored = 2;

  AttributeList Attrs = Call->getAttributes();

  SmallVector<Type *, 1> ArgTys;
  SmallVector<AttributeSet, 1> ArgAttrs;
  for (unsigned i = ArgOffset; i < WideOps.size() - ArgIgnored; i++) {
    CallArgs.push_back(WideOps[i]);
    ArgTys.push_back(WideOps[i]->getDestType());
    ArgAttrs.push_back(Attrs.getParamAttributes(i - ArgOffset));
  }

  if (ID != Intrinsic::not_intrinsic) {
    // Scalarize argument operands of intrinsic calls, if needed.
    // TODO: For VPValue-based CG scalarization decision about operands should
    // be done in general earlier. We should not blindly widen all operands of
    // an instruction.
    for (unsigned I = 0; I < CallArgs.size(); ++I) {
      if (hasVectorInstrinsicScalarOpd(ID, I)) {
        assert(CallArgs[I]->isTerminalRef() &&
               "Scalar operand of intrinsic is not terminal ref.");
        auto *OperandCE = CallArgs[I]->getSingleCanonExpr();
        assert(OperandCE->isInvariantAtLevel(OrigLoop->getNestingLevel()) &&
               "Scalar operand of intrinsic is loop variant.");
        Type *OperandScalarDestTy = OperandCE->getDestType()->getScalarType();
        OperandCE->setSrcType(OperandCE->getSrcType()->getScalarType());
        OperandCE->setDestType(OperandScalarDestTy);
        ArgTys[I] = OperandScalarDestTy;
      }
    }
  }

  // Masked intrinsics will not have explicit mask parameter. They are handled
  // like other BinOp HLInsts i.e. execute on all lanes and extract active lanes
  // during HIR-CG.
  bool Masked = Mask && ID == Intrinsic::not_intrinsic;
  if (Masked) {
    StringRef VecFuncName =
        TLI->getVectorizedFunction(Fn->getName(), VF, Masked);
    // Masks of SVML function calls need special treatment, it's different from
    // the normal case for AVX512.
    if (!VecFuncName.empty() &&
        isSVMLFunction(TLI, Fn->getName(), VecFuncName)) {
      addMaskToSVMLCall(Fn, Attrs, CallArgs, ArgTys, ArgAttrs, Mask);
    } else {
      auto CE = Mask->getSingleCanonExpr();
      ArgTys.push_back(CE->getDestType());
      CallArgs.push_back(Mask->clone());
      ArgAttrs.push_back(AttributeSet());
    }
  }

  // TODO: Fix when vector-variants will become supported.
  ++OptRptStats.VectorMathCalls;
  Function *VectorF = getOrInsertVectorFunction(
      Fn, VF, ArgTys, TLI, ID, nullptr /* vector-variant */, Masked);
  assert(VectorF && "Can't create vector function.");

  FastMathFlags FMF =
      isa<FPMathOperator>(Call) ? Call->getFastMathFlags() : FastMathFlags();
  auto *WideInst = HLNodeUtilities.createCall(
      VectorF, CallArgs, VectorF->getName(), WideLval, {} /*Bundle*/,
      {} /*BundleOps*/, FMF);
  CallInst *VecCall =
      cast<CallInst>(const_cast<Instruction *>(WideInst->getLLVMInstruction()));

  // Make sure we don't lose attributes at the call site. E.g., IMF
  // attributes are taken from call sites in MapIntrinToIml to refine
  // SVML calls for precision.
  copyRequiredAttributes(Call, VecCall, ArgAttrs);

  // Set calling conventions for SVML function calls
  if (isSVMLFunction(TLI, FnName, VectorF->getName())) {
    VecCall->setCallingConv(CallingConv::SVML);
  }

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
                             InterleaveIndex, GrpStartInst, VPInst);
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
    WideInst = HLNodeUtilities.createBinaryHLInst(
        BOp->getOpcode(), WideOps[1], WideOps[2], CurInst->getName() + ".vec",
        WideOps[0], BOp);
  } else if (auto UOp = dyn_cast<UnaryOperator>(CurInst)) {
    WideInst = HLNodeUtilities.createUnaryHLInst(UOp->getOpcode(), WideOps[1],
                                                 CurInst->getName() + ".vec",
                                                 WideOps[0], nullptr, UOp);
  } else if (isa<LoadInst>(CurInst)) {
    WideInst = HLNodeUtilities.createLoad(
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<StoreInst>(CurInst)) {
    WideInst = HLNodeUtilities.createStore(
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
    InsertInMap = false;
  } else if (isa<CastInst>(CurInst)) {
    assert(WideOps.size() == 2 && "invalid cast");

    WideInst = HLNodeUtilities.createCastHLInst(
        FixedVectorType::get(CurInst->getType(), VF), CurInst->getOpcode(),
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<SelectInst>(CurInst)) {
    WideInst = HLNodeUtilities.createSelect(
        INode->getPredicate(), WideOps[1], WideOps[2], WideOps[3], WideOps[4],
        CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<CmpInst>(CurInst)) {
    WideInst =
        HLNodeUtilities.createCmp(INode->getPredicate(), WideOps[1], WideOps[2],
                                  CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<GEPOrSubsOperator>(CurInst)) {
    // Gep Instructions in LLVM may have any number of operands but the HIR
    // representation for them is always a single rhs ddref - copy rval to
    // lval.
    WideInst = HLNodeUtilities.createCopyInst(
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
  } else if (INode->isCopyInst()) {
    WideInst = HLNodeUtilities.createCopyInst(
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
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

  // Handle results of SVML sincos function calls
  // sincos function has two return values. The scalar sincos function uses
  // pointers as out-parameters. SVML sincos function, instead, returns them in
  // a struct directly. This bridges the gap between these two approaches.
  if (const CallInst *Call = WideInst->getCallInst())
    if (const Function *Fn = Call->getCalledFunction())
      if (Fn->getName().startswith("__svml_sincos")) {
        addInst(WideInst, nullptr);
        generateStoreForSinCos(INode, WideInst, Mask,
                               false /* IsRemainderLoop */);
        return;
      }

  addInst(WideInst, Mask);
}

HLInst *VPOCodeGenHIR::insertReductionInitializer(Constant *Iden,
                                                  RegDDRef *ScalarRednRef) {

  // ScalarRednRef is the initial value that is assigned to result of the
  // reduction operation. We are blending in the initial value into the
  // identity vector in lane 0.

  auto IdentityVec = getConstantSplatDDRef(DDRefUtilities, Iden, VF);

  HLInst *InsertElementInst = HLNodeUtilities.createInsertElementInst(
      IdentityVec, ScalarRednRef, 0, "result.vector");
  insertReductionInit(InsertElementInst);

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
    llvm_unreachable(
        "HIR vectorizer is trying to handle reductions without entities.");
    HLContainerTy Tail;

    buildReductionTail(Tail, OpCode, VecRef, ScalRef->clone(), MainLoop,
                       FinalLvalRef);
    insertReductionFinal(&Tail);
  } else {
    auto Extr = HLNodeUtilities.createExtractElementInst(
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
  if (GlobalVariable *PaddedMallocVariable =
          HLNodeUtilities.getModule().getGlobalVariable(
              "__Intel_PaddedMallocCounter", true /*AllowInternal*/)) {
    LLVMContext &Context = *Plan->getLLVMContext();
    Type *IntTy = IntegerType::get(Context, 32);
    Type *BoolTy = IntegerType::get(Context, 1);

    // Construct memref __Intel_PaddedMallocCounter[0].
    unsigned PaddedMallocAddrIdx;
    BlobUtilities.createGlobalVarBlob(PaddedMallocVariable, true,
                                      &PaddedMallocAddrIdx);
    RegDDRef *PaddedMalloc = DDRefUtilities.createMemRef(PaddedMallocAddrIdx);
    PaddedMalloc->addDimension(CanonExprUtilities.createCanonExpr(IntTy));

    // Construct PaddedMallocLimit.
    RegDDRef *PaddedLimit =
        DDRefUtilities.createConstDDRef(IntTy, llvm::getPaddedMallocLimit());

    HLInst *PaddingIsValid = HLNodeUtilities.createCmp(
        PredicateTy::ICMP_ULT, PaddedMalloc, PaddedLimit, "valid.padding");
    HLNodeUtils::insertBefore(OrigLoop, PaddingIsValid);

    RegDDRef *ZeroRef = DDRefUtilities.createConstDDRef(BoolTy, 0);
    HLPredicate Pred(PredicateTy::ICMP_NE);
    auto Check = std::make_tuple(
        Pred, PaddingIsValid->getLvalDDRef()->clone(), ZeroRef);
    RTChecks.push_back(Check);
  }
#endif // INTEL_INCLUDE_DTRANS
}

RegDDRef *VPOCodeGenHIR::getUniformScalarRef(const VPValue *VPVal) {
  if (!isa<VPExternalDef>(VPVal) && !isa<VPConstant>(VPVal))
    llvm_unreachable("Expected a VPExternalDef/VPConstant");

  RegDDRef *ScalarRef = nullptr;
  if ((ScalarRef = getScalRefForVPVal(VPVal, 0)))
    return ScalarRef->clone();

  if (auto *VPExtDef = dyn_cast<VPExternalDef>(VPVal)) {
    const VPOperandHIR *HIROperand = VPExtDef->getOperandHIR();

    if (const auto *Blob = dyn_cast<VPBlob>(HIROperand)) {
      const auto *BlobRef = Blob->getBlob();
      if (BlobRef->isSelfBlob())
        ScalarRef = DDRefUtilities.createSelfBlobRef(
            BlobRef->getSelfBlobIndex(), BlobRef->getDefinedAtLevel());
      else {
        unsigned BlobIndex = Blob->getBlobIndex();
        const RegDDRef *RDDR = cast<RegDDRef>(BlobRef);

        // Create a RegDDREF containing blob with index BlobIndex.
        auto *CE = CanonExprUtilities.createCanonExpr(VPExtDef->getType());
        CE->addBlob(BlobIndex, 1);
        ScalarRef = DDRefUtilities.createScalarRegDDRef(GenericRvalSymbase, CE);

        // Use RDDR to set proper defined at level for blob in WideRef.
        SmallVector<const RegDDRef *, 1> AuxRefs = {RDDR};
        ScalarRef->makeConsistent(AuxRefs, OrigLoop->getNestingLevel());
      }
    } else if (const auto *VPCE = dyn_cast<VPCanonExpr>(HIROperand)) {
      auto *CE = VPCE->getCanonExpr()->clone();
      auto *DDR = VPCE->getDDR();
      ScalarRef = DDRefUtilities.createScalarRegDDRef(GenericRvalSymbase, CE);
      SmallVector<const RegDDRef *, 1> AuxRefs = {DDR};
      ScalarRef->makeConsistent(AuxRefs, OrigLoop->getNestingLevel());
    } else {
      const auto *IV = cast<VPIndVar>(HIROperand);
      auto IVLevel = IV->getIVLevel();
      auto RefDestTy = VPExtDef->getType();

      auto *CE = CanonExprUtilities.createCanonExpr(RefDestTy);
      CE->addIV(IVLevel, loopopt::InvalidBlobIndex, 1);
      ScalarRef = DDRefUtilities.createScalarRegDDRef(
          DDRefUtilities.getNewSymbase(), CE);
    }
  } else {
    auto *VPConst = cast<VPConstant>(VPVal);
    ScalarRef = DDRefUtilities.createConstDDRef(VPConst->getConstant());
  }

  assert(ScalarRef && "Unexpected null scalar ref");
  addVPValueScalRefMapping(VPVal, ScalarRef, 0);

  // Clients can potentially modify the returned value. Return the cloned value.
  return ScalarRef->clone();
}

RegDDRef *VPOCodeGenHIR::widenRef(const VPValue *VPVal, unsigned VF) {
  RegDDRef *WideRef = nullptr;

  // If the DDREF has a widened counterpart, return the same.
  if ((WideRef = getWideRefForVPVal(VPVal)))
    return WideRef->clone();

  // VPVal is expected to be a VPExternalDef or VPConstant.
  WideRef = getUniformScalarRef(VPVal);
  WideRef = widenRef(WideRef, VF);
  assert(WideRef && "Expected non-null widened ref");
  LLVM_DEBUG(WideRef->dump(true));
  LLVM_DEBUG(errs() << "\n");
  addVPValueWideRefMapping(VPVal, WideRef);

  // Clients can potentially modify the returned value. Return the cloned value.
  return WideRef->clone();
}

RegDDRef *VPOCodeGenHIR::getMemoryRef(const VPValue *VPPtr,
                                      unsigned ScalSymbase,
                                      const AAMDNodes &AANodes,
                                      bool Lane0Value) {
  bool IsNegOneStride;
  bool IsUnitStride = isUnitStridePtr(VPPtr, IsNegOneStride);
  bool NeedScalarRef = IsUnitStride || Lane0Value;

  RegDDRef *PtrRef;
  if (NeedScalarRef)
    PtrRef = getOrCreateScalarRef(VPPtr);
  else
    PtrRef = widenRef(VPPtr, getVF());

  RegDDRef *MemRef;
  if (PtrRef->isAddressOf()) {
    // We generate an addressof ref from the GEP instruction. For
    // generating load/store instruction, we need to clear the
    // addressof.
    MemRef = PtrRef;
    MemRef->setAddressOf(false);
  } else {
    // PtrRef is an invariant blob or value computed inside loop - we need to
    // generate a reference of the form blob[0].
    assert(PtrRef->isSelfBlob() && "Expected self blob DDRef");
    auto &HIRF = HLNodeUtilities.getHIRFramework();
    llvm::Triple TargetTriple(HIRF.getModule().getTargetTriple());
    auto Is64Bit = TargetTriple.isArch64Bit();
    MemRef = DDRefUtilities.createMemRef(PtrRef->getSelfBlobIndex(),
                                         PtrRef->getDefinedAtLevel());
    auto Int32Ty = Type::getInt32Ty(HLNodeUtilities.getContext());
    auto Int64Ty = Type::getInt64Ty(HLNodeUtilities.getContext());
    auto Zero = CanonExprUtilities.createCanonExpr(Is64Bit ? Int64Ty : Int32Ty);

    // We need to set destination type of the created canon expression
    // to VF wide vector type if we are not setting up a scalar ref.
    if (!NeedScalarRef)
      Zero->setDestType(FixedVectorType::get(Zero->getSrcType(), VF));
    MemRef->addDimension(Zero);
  }

  PointerType *PtrTy = cast<PointerType>(VPPtr->getType());
  Type *ValTy = PtrTy->getElementType();
  if (!Lane0Value) {
    Type *VecValTy = FixedVectorType::get(ValTy, VF);

    // MemRef's bitcast type needs to be set to a pointer to <VF x ValType>.
    MemRef->setBitCastDestType(
        PointerType::get(VecValTy, PtrTy->getAddressSpace()));
  }
  MemRef->setSymbase(ScalSymbase);
  MemRef->setAAMetadata(AANodes);

  // Adjust the memory reference for the negative one stride case so that
  // the client can do a wide load/store.
  if (IsNegOneStride)
    MemRef->shift(MainLoop->getNestingLevel(), (int64_t)VF - 1);

  // TODO - Alignment information needs to be obtained from VPInstruction.
  // For now we are forcing alignment based on value type.
  setRefAlignment(ValTy, MemRef);
  return MemRef;
}

// Widen blend instruction. The implementation here generates a sequence
// of selects. Consider the following scalar blend in bb0:
//      vp1 = blend [vp2, bp2] [vp3, bp3] [vp4, bp4].
// The selects are generated as follows:
//      select1 = bp3_vec ? vp3_vec : vp2_vec
//      vp1_vec = bp4_vec ? vp4_vec : select1
void VPOCodeGenHIR::widenBlendImpl(const VPBlendInst *Blend, RegDDRef *Mask) {
  unsigned NumIncomingValues = Blend->getNumIncomingValues();
  assert(NumIncomingValues > 0 && "Unexpected blend with zero values");

  // Generate a sequence of selects.
  RegDDRef *BlendVal = nullptr;
  for (unsigned Idx = 0, End = NumIncomingValues; Idx < End;
       ++Idx) {
    auto *IncomingVecVal = widenRef(Blend->getIncomingValue(Idx), getVF());
    if (!BlendVal) {
      BlendVal = IncomingVecVal;
      continue;
    }

    // We are trying to blend same wide ref here, so select is not needed.
    // Leave BlendVal untouched.
    if (DDRefUtilities.areEqual(IncomingVecVal, BlendVal))
      continue;

    VPValue *BlockPred = Blend->getIncomingPredicate(Idx);
    assert(BlockPred && "block-predicate should not be null for select");
    RegDDRef *Cond = widenRef(BlockPred, getVF());
    Type *CondTy = Cond->getDestType();
    Constant *OneVal = Constant::getAllOnesValue(CondTy->getScalarType());
    RegDDRef *OneValRef =
        getConstantSplatDDRef(DDRefUtilities, OneVal, getVF());
    HLInst *BlendInst = HLNodeUtilities.createSelect(
        CmpInst::ICMP_EQ, Cond, OneValRef, IncomingVecVal, BlendVal);
    // TODO: Do we really need the Mask here?
    addInst(BlendInst, Mask);
    BlendVal = BlendInst->getLvalDDRef()->clone();
  }

  // If the blend instruction has an associated reduction ref, the blendval
  // needs to be copied to the same. The reduction ref becomes the blend result.
  BlendVal = createCopyForRednRef(Blend, BlendVal, Mask);
  addVPValueWideRefMapping(Blend, BlendVal);
}

// Widen PHI instruction.
void VPOCodeGenHIR::widenPhiImpl(const VPPHINode *VPPhi, RegDDRef *Mask) {
  // A phi can correspond to a reduction. This can either be a phi in the loop
  // header which is not deconstructed. It can also be a phi at a merge point
  // of uniform control flow within the loop. In case of a non-deconstructed PHI
  // corresponding to a reduction, just use the corresponding reduction ref as
  // its widened ref.
  if (!isDeconstructedPhi(VPPhi) && ReductionRefs.count(VPPhi)) {
    addVPValueWideRefMapping(VPPhi, ReductionRefs[VPPhi]);
    return;
  }

  // Check if  PHI is the main loop IV and widen it by generating the ref i1 +
  // <0, 1, 2, .. VF-1>. TODO: Need to handle all IVs in general here, for now
  // HIR is expected to have main loop IV only.
  if (MainLoopIVInsts.count(VPPhi)) {
    auto RefDestTy = VPPhi->getType();
    auto *NewRef = generateLoopInductionRef(RefDestTy);
    addVPValueWideRefMapping(VPPhi, NewRef);

    // Create a scalar value for IV as well.
    auto *CE = CanonExprUtilities.createCanonExpr(RefDestTy);
    CE->addIV(OrigLoop->getNestingLevel(), InvalidBlobIndex /* no blob */,
              1 /* constant IV coefficient */);
    auto *ScalRef = DDRefUtilities.createScalarRegDDRef(GenericRvalSymbase, CE);
    addVPValueScalRefMapping(VPPhi, ScalRef, 0);

    return;
  }

  assert(isDeconstructedPhi(VPPhi) &&
         "Non-reduction/induction PHI was not deconstructed.");
  // PHI that was deconstructed via copies during SSA deconstruction is mapped
  // to the common Lval temp that the copies write into.
  int OriginPhiId = cast<VPHIRCopyInst>(VPPhi->getOperand(0))->getOriginPhiId();
  RegDDRef *PhiTemp = getLValTempForPhiId(OriginPhiId);
  assert(PhiTemp && "Deconstructed PHI does not have a LVal temp.");

  // If the phi instruction has an associated reduction ref, PhiTemp needs to be
  // copied to the same. The reduction ref becomes the Phi result.
  PhiTemp = createCopyForRednRef(VPPhi, PhiTemp->clone(), Mask);
  addVPValueWideRefMapping(VPPhi, PhiTemp);
}

RegDDRef *VPOCodeGenHIR::generateLoopInductionRef(Type *RefDestTy) {
  auto VecRefDestTy = FixedVectorType::get(RefDestTy, VF);

  auto *CE = CanonExprUtilities.createCanonExpr(RefDestTy);
  CE->setSrcType(VecRefDestTy);
  CE->setDestType(VecRefDestTy);
  CE->addIV(OrigLoop->getNestingLevel(), InvalidBlobIndex /* no blob */,
            1 /* constant IV coefficient */);
  SmallVector<Constant *, 4> ConstVec;
  for (unsigned i = 0; i < VF; ++i)
    ConstVec.push_back(ConstantInt::getSigned(RefDestTy, i));
  unsigned Idx = 0;
  BlobUtilities.createConstantBlob(ConstantVector::get(ConstVec), true, &Idx);
  CE->addBlob(Idx, 1);
  auto *NewRef = DDRefUtilities.createScalarRegDDRef(GenericRvalSymbase, CE);
  return NewRef;
}

RegDDRef *VPOCodeGenHIR::getOrCreateScalarRef(const VPValue *VPVal) {
  RegDDRef *ScalarRef = nullptr;
  unsigned Lane = 0;
  if ((ScalarRef = getScalRefForVPVal(VPVal, Lane)))
    return ScalarRef->clone();

  if (isa<VPExternalDef>(VPVal) || isa<VPConstant>(VPVal))
    return getUniformScalarRef(VPVal);

  RegDDRef *WideRef = widenRef(VPVal, getVF());
  HLInst *ExtractInst = HLNodeUtilities.createExtractElementInst(
      WideRef->clone(), (unsigned)Lane, "uni.idx");
  addInstUnmasked(ExtractInst);
  ScalarRef = ExtractInst->getLvalDDRef();
  return ScalarRef->clone();
}

// For the code generated see comment in *.h file.
void VPOCodeGenHIR::generateMinMaxIndex(const VPReductionFinal *RedFinal,
                                        RegDDRef *RednVariable,
                                        HLContainerTy &RedTail,
                                        HLInst *&WInst) {
  RegDDRef *ReduceVal = widenRef(RedFinal->getReducingOperand(), getVF());
  RegDDRef *ParentExit = widenRef(RedFinal->getParentExitValOperand(), getVF());
  // Get DDRef for parent reduced value.
  RegDDRef *ParentFinal =
      widenRef(RedFinal->getParentFinalValOperand(), getVF());
  // Get broadcasted DDRef.
  ParentFinal = widenRef(ParentFinal, getVF());
  unsigned Opc = RedFinal->getBinOpcode();
  PredicateTy Pred = (Opc == VPInstruction::FMax || Opc == VPInstruction::FMin)
                         ? PredicateTy::FCMP_OEQ
                         : PredicateTy::ICMP_EQ;

  Type *IndexVecTy = ReduceVal->getDestType();
  assert(isa<IntegerType>(IndexVecTy->getScalarType()) &&
         "Index part of minmax+index idiom is not integer type.");

  bool NeedMaxIntVal =
      (Opc == VPInstruction::FMin || Opc == VPInstruction::SMin ||
       Opc == VPInstruction::UMin);

  Constant *MinMaxIntVec = VPOParoptUtils::getMinMaxIntVal(
      CanonExprUtilities.getContext(), IndexVecTy, !RedFinal->isSigned(),
      NeedMaxIntVal);
  RegDDRef *MinMaxIntVecRef = DDRefUtilities.createConstDDRef(MinMaxIntVec);
  HLInst *IndexBlend =
      HLNodeUtilities.createSelect(Pred, ParentFinal, ParentExit->clone(),
                                   ReduceVal, MinMaxIntVecRef, "idx.blend");
  RedTail.push_back(*IndexBlend);

  RegDDRef *Acc = nullptr;
  HLInst *IdxReduceCall =
      createVectorReduce(RedFinal, IndexBlend->getLvalDDRef()->clone(), Acc,
                         RednVariable, HLNodeUtilities);
  RedTail.push_back(*IdxReduceCall);
  WInst = IdxReduceCall;
}

void VPOCodeGenHIR::widenLoopEntityInst(const VPInstruction *VPInst) {
  HLInst *WInst = nullptr; // Track the last generated wide inst for VPInst

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
    HLInst *CopyInst =
        HLNodeUtilities.createCopyInst(IdentityRef, "red.init", RedRef);
    WInst = CopyInst;
    RedInitHLInsts.push_back(*CopyInst);

    if (RedInit->getNumOperands() > 1) {
      auto *StartVPVal = RedInit->getStartValueOperand();
      assert((isa<VPExternalDef>(StartVPVal) || isa<VPConstant>(StartVPVal)) &&
             "Unsupported reduction start value.");
      // Insert start value into lane 0 of identity vector
      HLInst *InsertElementInst = HLNodeUtilities.createInsertElementInst(
          CopyInst->getLvalDDRef()->clone(), getUniformScalarRef(StartVPVal), 0,
          "red.init.insert", RedRef->clone());
      WInst = InsertElementInst;
      RedInitHLInsts.push_back(*InsertElementInst);
    }
    // TODO: This should be changed to addInst interface once it is aware of
    // Loop PH or Exit.
    insertReductionInit(&RedInitHLInsts);

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
    HLContainerTy RedTail;

    RegDDRef *VecRef = widenRef(RedFinal->getReducingOperand(), getVF());
    Type *ElType = RedFinal->getReducingOperand()->getType();
    if (isa<VectorType>(ElType)) {
      // Incoming vector types is not supported for HIR
      llvm_unreachable("Unsupported vector data type for reducing operand.");
    }
    const VPReduction *RednEntity = VPLoopEntities->getReduction(RedFinal);
    assert(RednEntity && "Reduction final does not have a corresponding "
                         "VPReduction descriptor.");
    // Scalar result of vector reduce intrinsic should be written back to the
    // original reduction descriptor variable.
    // NOTE : We obtain this variable from VPLoopEntity corresponding to the
    // reduction since reduction-final instruction may not have accumulators
    // always.
    // TODO: This is the only user of VPLoopEntities in CG, this should be
    // removed once VPReductionFinal is updated to have the StartValue always.
    RegDDRef *RednDescriptor =
        getUniformScalarRef(RednEntity->getRecurrenceStartValue());

    if (RedFinal->isMinMaxIndex())
      generateMinMaxIndex(RedFinal, RednDescriptor, RedTail, WInst);
    else {
      auto *StartVPVal = RedFinal->getStartValueOperand();
      RegDDRef *Acc = nullptr;

      if (StartVPVal) {
        assert(isa<VPExternalDef>(StartVPVal) &&
               "Unsupported reduction start value.");
        Acc = getUniformScalarRef(StartVPVal);
      }

      // 1. Generate vector reduce intrinsic call
      HLInst *VecReduceCall = createVectorReduce(
          RedFinal, VecRef, Acc, RednDescriptor, HLNodeUtilities);
      WInst = VecReduceCall;
      RedTail.push_back(*VecReduceCall);

      // 2. If the accumulator is not null, then last value scalar compute is
      // needed, which is of the form %red.init = %red.init OP %vec.reduce NOTE
      // : Acc will always be null for min/max reductions, createVectorReduce
      // asserts on that.
      if (Acc) {
        HLInst *ScalarLastVal;
        if (RedFinal->hasFastMathFlags())
          ScalarLastVal = HLNodeUtilities.createFPMathBinOp(
              RedFinal->getBinOpcode(), Acc->clone(),
              VecReduceCall->getLvalDDRef()->clone(),
              RedFinal->getFastMathFlags(), "red.result", Acc);
        else
          ScalarLastVal = HLNodeUtilities.createBinaryHLInst(
              RedFinal->getBinOpcode(), Acc->clone(),
              VecReduceCall->getLvalDDRef()->clone(), "red.result", Acc);
        WInst = ScalarLastVal;
        RedTail.push_back(*ScalarLastVal);
      }
    }

    // TODO: This should be changed to addInst interface once it is aware of
    // Loop PH or Exit.
    insertReductionFinal(&RedTail);
    addVPValueWideRefMapping(VPInst, WInst->getLvalDDRef());
    return;
  }

  case VPInstruction::InductionInitStep: {
    // The only expected induction currently is for the implicit IV.
    auto *ConstStep = cast<VPConstant>(VPInst->getOperand(0));

    assert((cast<VPInductionInitStep>(VPInst)->getBinOpcode() ==
                Instruction::Add &&
            ConstStep->getConstant()->isOneValue()) &&
           "Expected an add induction with constant step of 1");
    auto *ScalRef =
        DDRefUtilities.createConstDDRef(ConstStep->getType(), getVF());
    auto *WideRef = widenRef(ScalRef, getVF());
    addVPValueWideRefMapping(VPInst, WideRef);
    addVPValueScalRefMapping(VPInst, ScalRef, 0);
    return;
  }

  case VPInstruction::InductionInit:
  case VPInstruction::InductionFinal: {
    // TODO: Add codegen for induction initialization/finalization
    return;
  }

  default:
    llvm_unreachable("Unsupported VPLoopEntity instruction.");
  }
}

RegDDRef *VPOCodeGenHIR::generateCompareToZero(RegDDRef *Value,
                                               RegDDRef *InstMask, bool Equal) {
  assert(Value && "Expected a non-null mask");
  Type *Ty = Value->getDestType();
  assert((Ty->isVectorTy() && Ty->getScalarType()->isIntegerTy(1)) &&
         "Expected <N x i1> type");

  // Bitcast <VF x i1> to an integer value VF bits long.
  Type *IntTy =
      IntegerType::get(Ty->getContext(), Ty->getPrimitiveSizeInBits());
  auto *BitCastInst = HLNodeUtilities.createCastHLInst(
      IntTy, Instruction::BitCast, Value->clone());
  addInst(BitCastInst, InstMask);

  // Compare that the bitcast value against zero. If Equal is true we check
  // for equality otherwise, check for inequality.
  PredicateTy Pred = Equal ? PredicateTy::ICMP_EQ : PredicateTy::ICMP_NE;
  auto *CmpInst =
      HLNodeUtilities.createCmp(Pred, BitCastInst->getLvalDDRef()->clone(),
                                DDRefUtilities.createNullDDRef(IntTy));
  addInst(CmpInst, InstMask);
  return CmpInst->getLvalDDRef();
}

void VPOCodeGenHIR::widenUniformLoadImpl(const VPInstruction *VPInst,
                                         RegDDRef *Mask) {
  // For a uniform load, do a scalar load followed by a broadcast. We need to
  // mask the scalar load appropriately. The mask to use is !AllZeroCheck(Mask)
  // which will ensure that we do the load if any vector lane is active. Using a
  // scalar load followed by broadcast is especially important for the masked
  // case as we need to ensure that lane 0 value is properly set as it can feed
  // unit stride memory references in cases like the following:
  //     t1 = load unif_ptr @mask = cond
  //     t2 = a[t1][i1]  @mask = cond
  if (Mask) {
    Mask =
        generateCompareToZero(Mask, nullptr /* InstMask */, false /* Equal */);
  }

  const VPValue *PtrOp = getLoadStorePointerOperand(VPInst);
  RegDDRef *MemRef = getMemoryRef(PtrOp, VPInst->getSymbase(),
                                  VPInst->HIR.AANodes, true /* Lane0Value */);
  auto *ScalarInst = HLNodeUtilities.createLoad(MemRef, ".unifload");
  if (Mask) {
    HLIf *If = HLNodeUtilities.createHLIf(
        PredicateTy::ICMP_EQ, Mask->clone(),
        DDRefUtilities.createConstDDRef(Mask->getDestType(), 1));
    addInst(If, nullptr /* Mask */);
    HLNodeUtils::insertAsFirstThenChild(If, ScalarInst);
  } else {
    addInstUnmasked(ScalarInst);
  }

  addVPValueScalRefMapping(VPInst, ScalarInst->getLvalDDRef(), 0);
  addVPValueWideRefMapping(
      VPInst, widenRef(ScalarInst->getLvalDDRef()->clone(), getVF()));
}

void VPOCodeGenHIR::widenLoadStoreImpl(const VPInstruction *VPInst,
                                       RegDDRef *Mask) {
  // Loads/stores need to be masked with current mask value if Mask is null.
  if (!Mask)
    Mask = CurMaskValue;

  auto Opcode = VPInst->getOpcode();
  assert((Opcode == Instruction::Load || Opcode == Instruction::Store) &&
         "Expected load/store instruction");

  // Handle uniform load
  const VPValue *PtrOp = getLoadStorePointerOperand(VPInst);
  if (Opcode == Instruction::Load && !Plan->getVPlanDA()->isDivergent(*PtrOp)) {
    widenUniformLoadImpl(VPInst, Mask);
    return;
  }

  RegDDRef *MemRef =
      getMemoryRef(PtrOp, VPInst->getSymbase(), VPInst->HIR.AANodes);
  HLInst *WInst;

  // Reverse mask for negative -1 stride.
  bool IsNegOneStride;
  isUnitStridePtr(PtrOp, IsNegOneStride);
  if (Mask && IsNegOneStride) {
    auto *RevInst = createReverseVector(Mask->clone());
    Mask = RevInst->getLvalDDRef();
  }

  if (Opcode == Instruction::Load) {
    WInst = HLNodeUtilities.createLoad(MemRef, ".vec");
    addInst(WInst, Mask);
    // Reverse the loaded value for negative -1 stride.
    if (IsNegOneStride)
      WInst = createReverseVector(WInst->getLvalDDRef()->clone());
    addVPValueWideRefMapping(VPInst, WInst->getLvalDDRef());
  } else {
    RegDDRef *StoreVal = widenRef(VPInst->getOperand(0), getVF());
    // Reverse the value to be stored for negative -1 stride.
    if (IsNegOneStride) {
      WInst = createReverseVector(StoreVal);
      StoreVal = WInst->getLvalDDRef()->clone();
    }
    WInst = HLNodeUtilities.createStore(StoreVal, ".vec", MemRef);
    addInst(WInst, Mask);
    // Stores are not added to widen/scalar maps. Explicitly set debug location
    // on lval memref.
    WInst->getLvalDDRef()->setMemDebugLoc(VPInst->getDebugLocation());
  }
}

void VPOCodeGenHIR::generateHIRForSubscript(const VPSubscriptInst *VPSubscript,
                                            RegDDRef *Mask, bool Widen) {
  auto RefDestTy = VPSubscript->getType();
  auto ResultRefTy = getResultRefTy(RefDestTy, VF, Widen);

  RegDDRef *PointerRef =
      getOrCreateRefForVPVal(VPSubscript->getPointerOperand(), Widen);
  // Base canon expression needs to be a self blob.
  if (!PointerRef->isSelfBlob()) {
    auto *CopyInst = HLNodeUtilities.createCopyInst(PointerRef, "nsbgepcopy");
    addInst(CopyInst, Mask);
    PointerRef = CopyInst->getLvalDDRef();
  }

  auto *NewRef = DDRefUtilities.createAddressOfRef(
      PointerRef->getSelfBlobIndex(), PointerRef->getDefinedAtLevel());
  NewRef->setInBounds(VPSubscript->isInBounds());
  NewRef->setBitCastDestType(ResultRefTy);
  SmallVector<const RegDDRef *, 4> AuxRefs;

  // Utility to specially handle lower/stride fields of a subscript. We ensure
  // that loop invariant lower/stride are kept scalar even in vectorized memref.
  auto generateLowerOrStride = [this](VPValue *V, bool Widen) {
    if (!Plan->getVPlanDA()->isDivergent(*V))
      return getOrCreateRefForVPVal(V, false /*always scalar*/);
    return getOrCreateRefForVPVal(V, Widen);
  };

  for (int Dim = VPSubscript->getNumDimensions() - 1; Dim >= 0; --Dim) {
    RegDDRef *Lower = generateLowerOrStride(VPSubscript->getLower(Dim), Widen);
    RegDDRef *Stride =
        generateLowerOrStride(VPSubscript->getStride(Dim), Widen);
    RegDDRef *Idx = getOrCreateRefForVPVal(VPSubscript->getIndex(Dim), Widen);
    AuxRefs.insert(AuxRefs.end(), {Idx, Lower, Stride});
    ArrayRef<unsigned> StructOffsets = VPSubscript->getStructOffsets(Dim);
    Type *DimTy = VPSubscript->getDimensionType(Dim);
    NewRef->addDimension(Idx->getSingleCanonExpr(), StructOffsets,
                         Lower->getSingleCanonExpr(),
                         Stride->getSingleCanonExpr(), DimTy);
  }

  makeConsistentAndAddToMap(NewRef, VPSubscript, AuxRefs, Widen);
  LLVM_DEBUG(dbgs() << "[VPOCGHIR] NewMemRef: "; NewRef->dump(1));
}

void VPOCodeGenHIR::generateHIR(const VPInstruction *VPInst, RegDDRef *Mask,
                                const OVLSGroup *Grp, int64_t InterleaveFactor,
                                int64_t InterleaveIndex,
                                const HLInst *GrpStartInst, bool Widen) {
  assert((Widen || isOpcodeForScalarInst(VPInst->getOpcode())) &&
         "Unxpected instruction for scalar constructs");

  HLInst *NewInst = nullptr;
  SmallVector<RegDDRef *, 1> CallArgs;
  const HLInst *CallInst = nullptr;
  const Twine InstName = ".vec";

  if (auto *VPPhi = dyn_cast<VPPHINode>(VPInst)) {
    widenPhiImpl(VPPhi, Mask);
    return;
  }

  if (auto *Blend = dyn_cast<VPBlendInst>(VPInst)) {
    widenBlendImpl(Blend, Mask);
    return;
  }

  // Skip loop related VPInstructions other than IV-updates from unroller.
  // TODO: Revisit when HIR vectorizer supports outer-loop vectorization.
  if (VPInst->isUnderlyingIRValid()) {
    const HLNode *HNode =
        VPInst->HIR.isMaster()
            ? VPInst->HIR.getUnderlyingNode()
            : VPInst->HIR.getMaster()->HIR.getUnderlyingNode();
    if (isa<HLLoop>(HNode))
      if (VPInst->getOpcode() != Instruction::Add ||
          !MainLoopIVInsts.count(VPInst))
        return;
  }

  auto Opcode = VPInst->getOpcode();
  switch (Opcode) {
  case Instruction::Load:
  case Instruction::Store:
    widenLoadStoreImpl(VPInst, Mask);
    return;
  case VPInstruction::Subscript:
    generateHIRForSubscript(cast<VPSubscriptInst>(VPInst), Mask, Widen);
    return;
  case VPInstruction::ReductionInit:
  case VPInstruction::ReductionFinal:
  case VPInstruction::InductionInit:
  case VPInstruction::InductionInitStep:
  case VPInstruction::InductionFinal:
    widenLoopEntityInst(VPInst);
    return;

  case Instruction::Br:
    // Do nothing.
    return;
  }

  // Skip widening the first select operand. The select instruction is generated
  // using operands of the instruction corresponding to the select mask.
  bool SkipFirstSelectOp = VPInst->getOpcode() == Instruction::Select;
  SmallVector<RegDDRef *, 6> RefOps;
  // For call instructions we need to widen only argument operands. Called
  // value/function operand should not be included here for widening.
  VPUser::const_operand_range OpRange =
      isa<VPCallInstruction>(VPInst)
          ? cast<VPCallInstruction>(VPInst)->arg_operands()
          : VPInst->operands();
  for (const VPValue *Operand : OpRange) {
    if (SkipFirstSelectOp) {
      SkipFirstSelectOp = false;
      continue;
    }

    RegDDRef *Ref;
    if (Widen)
      Ref = widenRef(Operand, getVF());
    else {
      // Obtain the scalar ref for lane 0 if one exists already
      if ((Ref = getScalRefForVPVal(Operand, 0)))
        Ref = Ref->clone();
      else if (isa<VPExternalDef>(Operand) || isa<VPConstant>(Operand))
        // Creation of a scalar ref for externaldefs/constants does not require
        // a new instruction - try to get uniform scalar ref if Ref is null.
        Ref = getUniformScalarRef(Operand);
    }

    // Ref can be null for a case where we do not have a scalar ref created.
    // For such cases, we avoid creating extracts for operands and bail out
    // of generating the scalar instruction. If a later use requires the
    // scalar version, an extract instruction will be generated at that point
    // from the widened instruction. Once we start using SVA results, this
    // should no longer be an issue as scalar refs should be created at the
    // def point when needed.
    if (!Ref)
      return;

    RefOps.push_back(Ref);
  }

  // RefOp0 points to the first operand when RefOps has atleast one operand.
  // RefOp1 points to the second operand when RefOps has atleast two operands.
  // They are null otherwise.
  RegDDRef *RefOp0 = nullptr, *RefOp1 = nullptr;
  unsigned RefOpsSize = RefOps.size();
  if (RefOpsSize == 1)
    RefOp0 = RefOps[0];
  else if (RefOpsSize >= 2) {
    RefOp0 = RefOps[0];
    RefOp1 = RefOps[1];
  }

  switch (VPInst->getOpcode()) {
  case Instruction::FNeg:
    NewInst = HLNodeUtilities.createFNeg(RefOp0, InstName, nullptr /*Lval*/,
                                         nullptr /*FPMathTag*/,
                                         VPInst->getFastMathFlags());
    break;

  case Instruction::Freeze:
    NewInst = HLNodeUtilities.createFreeze(RefOp0, InstName);
    break;

  case Instruction::Add: {
    // Try and fold the add operation into the canon expression for RefOps[0].
    // This helps preserve linear values and also avoids unnecessary HLInsts.
    // Reductions cannot be folded.
    assert(RefOp0->isTerminalRef() && RefOp1->isTerminalRef() &&
           "Expected terminal refs");

    auto *CE1 = RefOp0->getSingleCanonExpr();
    auto *CE2 = RefOp1->getSingleCanonExpr();
    if (!ReductionVPInsts.count(VPInst) &&
        CanonExprUtilities.canAdd(CE1, CE2)) {
      SmallVector<const RegDDRef *, 2> AuxRefs = {RefOp0->clone(), RefOp1};
      CanonExprUtilities.add(CE1, CE2);
      makeConsistentAndAddToMap(RefOp0, VPInst, AuxRefs, Widen);
      return;
    }

    // Do not create an explicit scalar instruction.
    if (!Widen)
      return;

    bool HasNUW = false, HasNSW = false;
    RegDDRef *RedRef = nullptr;
    getOverflowFlagsAndRednRef(VPInst, HasNUW, HasNSW, RedRef);

    NewInst = HLNodeUtilities.createOverflowingBinOp(
        VPInst->getOpcode(), RefOp0, RefOp1, HasNUW, HasNSW, ".vec",
        RedRef ? RedRef->clone() : nullptr);
    break;
  }

  case Instruction::UDiv:
  case Instruction::SDiv: {
    assert(RefOp0->isTerminalRef() && RefOp1->isTerminalRef() &&
           "Expected terminal refs");

    auto *CE1 = RefOp0->getSingleCanonExpr();

    // Try and fold the divide operation into the canon expression for RefOp0 if
    // it is linear. This helps preserve linear values and also avoids
    // unnecessary HLInsts.
    auto *ConstOp = dyn_cast<VPConstant>(VPInst->getOperand(1));
    if (CE1->isLinearAtLevel(MainLoop->getNestingLevel()) &&
        CE1->getDenominator() == 1 && ConstOp) {
      auto *CI = cast<ConstantInt>(ConstOp->getConstant());

      // The constant value needs to fit in 64 bits which is what setDenominator
      // takes. Try folding only if this is the case.
      if (CI->getBitWidth() <= 64) {
        SmallVector<const RegDDRef *, 2> AuxRefs = {RefOp0->clone()};

        CE1->setDenominator(CI->getSExtValue());
        CE1->setDivisionType(VPInst->getOpcode() == Instruction::SDiv);
        makeConsistentAndAddToMap(RefOp0, VPInst, AuxRefs, Widen);
        return;
      }
    }

    NewInst = HLNodeUtilities.createPossiblyExactBinOp(
        VPInst->getOpcode(), RefOp0, RefOp1, VPInst->isExact(), ".vec",
        nullptr);
    break;
  }

  case Instruction::Mul: {
    // Try and fold the mul operation into the canon expression. This helps
    // preserve linear values and also avoids unnecessary HLInsts. Reductions
    // cannot be folded.
    assert(RefOp0->isTerminalRef() && RefOp1->isTerminalRef() &&
           "Expected terminal refs");

    auto *CE1 = RefOp0->getSingleCanonExpr();
    auto *CE2 = RefOp1->getSingleCanonExpr();
    const VPConstant *ConstOp = nullptr;
    unsigned NonConstIndex = 0;

    if (!ReductionVPInsts.count(VPInst) &&
        CE1->isLinearAtLevel(MainLoop->getNestingLevel()) &&
        CE2->isLinearAtLevel(MainLoop->getNestingLevel()))
      if (ConstOp = dyn_cast<VPConstant>(VPInst->getOperand(0)))
        NonConstIndex = 1;
      else if (ConstOp = dyn_cast<VPConstant>(VPInst->getOperand(1)))
        NonConstIndex = 0;

    if (ConstOp) {
      auto *CI = cast<ConstantInt>(ConstOp->getConstant());
      // The constant value needs to fit in 64 bits which is what
      // multiplyByConstant takes. Try folding only if this is the case.
      if (CI->getBitWidth() <= 64) {
        auto *NonConstCE = NonConstIndex == 0 ? CE1 : CE2;
        auto *ResultRef = NonConstIndex == 0 ? RefOp0 : RefOp1;

        SmallVector<const RegDDRef *, 2> AuxRefs = {ResultRef->clone()};
        int64_t ConstVal = CI->getSExtValue();
        if (NonConstCE->multiplyByConstant(ConstVal)) {
          makeConsistentAndAddToMap(ResultRef, VPInst, AuxRefs, Widen);
          return;
        }
      }
    }

    if (!Widen)
      return;

    bool HasNUW = false, HasNSW = false;
    RegDDRef *RedRef = nullptr;
    getOverflowFlagsAndRednRef(VPInst, HasNUW, HasNSW, RedRef);

    NewInst = HLNodeUtilities.createOverflowingBinOp(
        VPInst->getOpcode(), RefOp0, RefOp1, HasNUW, HasNSW, ".vec",
        RedRef ? RedRef->clone() : nullptr);
    break;
  }

  case Instruction::SRem:
  case Instruction::URem:
  case Instruction::FAdd:
  case Instruction::Sub:
  case Instruction::FSub:
  case Instruction::FMul:
  case Instruction::FDiv:
  case Instruction::FRem:
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor: {
    bool HasNUW = false, HasNSW = false;
    RegDDRef *RedRef = nullptr;
    getOverflowFlagsAndRednRef(VPInst, HasNUW, HasNSW, RedRef);

    if (VPInst->hasFastMathFlags()) {
      NewInst = HLNodeUtilities.createFPMathBinOp(
          VPInst->getOpcode(), RefOp0, RefOp1, VPInst->getFastMathFlags(),
          InstName, RedRef ? RedRef->clone() : nullptr);
    } else if (HasNUW || HasNSW) {
      NewInst = HLNodeUtilities.createOverflowingBinOp(
          VPInst->getOpcode(), RefOp0, RefOp1, HasNUW, HasNSW, InstName);
    } else if (VPInst->isExact()) {
      NewInst = HLNodeUtilities.createPossiblyExactBinOp(
          VPInst->getOpcode(), RefOp0, RefOp1, VPInst->isExact(), InstName,
          RedRef ? RedRef->clone() : nullptr);
    } else {
      NewInst = HLNodeUtilities.createBinaryHLInst(
          VPInst->getOpcode(), RefOp0, RefOp1, InstName,
          RedRef ? RedRef->clone() : nullptr);
    }
    break;
  }

  case Instruction::ICmp:
  case Instruction::FCmp: {
    auto *VPCmp = cast<VPCmpInst>(VPInst);
    FastMathFlags FMF = VPInst->hasFastMathFlags() ? VPInst->getFastMathFlags()
                                                   : FastMathFlags();
    NewInst = HLNodeUtilities.createCmp(VPCmp->getPredicate(), RefOp0, RefOp1,
                                        InstName, nullptr /*Lval*/, FMF);
    break;
  }

  case VPInstruction::Abs: {
    assert(RefOp0->isTerminalRef() && "Expected terminal ref");

    // Generate DDRef for -RefOp0.
    SmallVector<const RegDDRef *, 2> AuxRefs = {RefOp0};
    RegDDRef *NegRef = RefOp0->clone();
    NegRef->getSingleCanonExpr()->negate();
    NegRef->makeConsistent(AuxRefs, OrigLoop->getNestingLevel());

    // Create the following select instruction:
    //    RefOp0 < 0 ? NegRef : RefOp0
    RegDDRef *ZeroRef =
        DDRefUtilities.createConstDDRef(RefOp0->getDestType(), (int64_t)0);
    NewInst =
        HLNodeUtilities.createSelect(CmpInst::ICMP_SLT, RefOp0, ZeroRef, NegRef,
                                     RefOp0->clone(), InstName, nullptr);
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
    FastMathFlags FMF = VPInst->hasFastMathFlags() ? VPInst->getFastMathFlags()
                                                   : FastMathFlags();
    NewInst = HLNodeUtilities.createSelect(
        PredInst->getPredicate(), Pred0, Pred1, RefOp0, RefOp1, InstName,
        RedRef ? RedRef->clone() : nullptr, FMF);
    break;
  }

  case Instruction::ZExt:
  case Instruction::SExt:
  case Instruction::Trunc: {
    auto RefDestTy = VPInst->getType();
    auto ResultRefTy = getResultRefTy(RefDestTy, VF, Widen);
    assert(RefOp0->isTerminalRef() && "Expected terminal ref");
    auto *CE = RefOp0->getSingleCanonExpr();

    // Try to fold the convert operation into the canon expression. This can
    // be done if the canon expression's source and dest type are the same.
    if (CE->getDestType() == CE->getSrcType()) {
      // Used to make the folded ref consistent.
      SmallVector<const RegDDRef *, 1> AuxRefs = {RefOp0->clone()};
      const VPValue *VPOp0 = VPInst->getOperand(0);

      // The IV in a canon expression is allowed to be of a type different from
      // the src type of the canon expression. When this is the case and
      // generating LLVM IR during HIRCG, the IV value is generated followed by
      // generation of an appropriate convert instruction to convert the value
      // to canon expression source type. In order to preserve linear canon
      // expressions, we flagged such converts during decomposition. Fold such
      // converts.
      if (VPInst->getFoldIVConvert()) {
        if (Widen) {
          // The type of blobs in the canon expression needs to match the source
          // type of canon expression. For a widened IV, the type of the blob
          // corresponding to the constant <0, 1, .., VF-1> would match the
          // current source type of canon expression. Since we are going to
          // change the source type, this is handled by getting the scalar ref,
          // changing the source type, and then widening this scalar ref.
          RefOp0 = getScalRefForVPVal(VPOp0, 0)->clone();
          CE = RefOp0->getSingleCanonExpr();
          CE->setSrcType(RefDestTy);
          CE->setDestType(RefDestTy);
          RefOp0 = widenRef(RefOp0, VF);
        } else {
          CE->setSrcType(ResultRefTy);
          CE->setDestType(ResultRefTy);
        }
        makeConsistentAndAddToMap(RefOp0, VPInst, AuxRefs, Widen);
        return;
      }

      CE->setDestType(ResultRefTy);
      CE->setExtType(VPInst->getOpcode() == Instruction::SExt);
      makeConsistentAndAddToMap(RefOp0, VPInst, AuxRefs, Widen);
      return;
    }

    // Do not create an explicit scalar instruction.
    if (!Widen)
      return;

    NewInst = HLNodeUtilities.createCastHLInst(ResultRefTy, VPInst->getOpcode(),
                                               RefOp0, InstName);
    break;
  }

  case Instruction::FPToUI:
  case Instruction::FPToSI:
  case Instruction::FPExt:
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
  case Instruction::SIToFP:
  case Instruction::UIToFP:
  case Instruction::FPTrunc:
  case Instruction::BitCast: {
    auto RefDestTy = VPInst->getType();
    auto ResultRefTy = getResultRefTy(RefDestTy, VF, Widen);

    // For bitcasts of addressof refs, it is enough to set the bitcast
    // destination type.
    if (VPInst->getOpcode() == Instruction::BitCast && RefOp0->isAddressOf()) {
      SmallVector<const RegDDRef *, 1> AuxRefs = {RefOp0->clone()};
      RefOp0->setBitCastDestType(ResultRefTy);
      makeConsistentAndAddToMap(RefOp0, VPInst, AuxRefs, Widen);
      return;
    }

    if (!Widen)
      return;

    NewInst = HLNodeUtilities.createCastHLInst(ResultRefTy, VPInst->getOpcode(),
                                               RefOp0, InstName);
    break;
  }
  case VPInstruction::SMax:
    NewInst = HLNodeUtilities.createSelect(CmpInst::ICMP_SGT, RefOp0, RefOp1,
                                           RefOp0->clone(), RefOp1->clone(),
                                           InstName);
    break;

  case VPInstruction::UMax:
    NewInst = HLNodeUtilities.createSelect(CmpInst::ICMP_UGT, RefOp0, RefOp1,
                                           RefOp0->clone(), RefOp1->clone(),
                                           InstName);
    break;

  case VPInstruction::SMin:
    NewInst = HLNodeUtilities.createSelect(CmpInst::ICMP_SLT, RefOp0, RefOp1,
                                           RefOp0->clone(), RefOp1->clone(),
                                           InstName);
    break;

  case VPInstruction::UMin:
    NewInst = HLNodeUtilities.createSelect(CmpInst::ICMP_ULT, RefOp0, RefOp1,
                                           RefOp0->clone(), RefOp1->clone(),
                                           InstName);
    break;

  case Instruction::GetElementPtr: {
    // Decomposer can generate GEPs only if underlying LLVMInst is GEP. This
    // happens for single operand GEP cases (check hir_oneopgep.ll).
    SmallVector<const RegDDRef *, 4> AuxRefs;

    // If we have a single operand GEP, we can simply reuse RefOp0
    if (VPInst->getNumOperands() == 1) {
      makeConsistentAndAddToMap(RefOp0, VPInst, AuxRefs, Widen);
      return;
    }

    // Any other case of GEP implies it was introduced after decomposer by some
    // VPlan-to-VPlan xform.
    assert(false && "VPlan-to-VPlan xform introduced a new IR-agnostic GEP.");

    auto RefDestTy = VPInst->getType();
    auto ResultRefTy = getResultRefTy(RefDestTy, VF, Widen);
    auto VPGEP = cast<VPGEPInstruction>(VPInst);
    RegDDRef *NewRef = RefOp0;

    // Base canon expression needs to be a self blob.
    if (!RefOp0->isSelfBlob()) {
      auto *CopyInst = HLNodeUtilities.createCopyInst(RefOp0, "nsbgepcopy");
      addInst(CopyInst, Mask);
      NewRef = CopyInst->getLvalDDRef();
    }

    NewRef = DDRefUtilities.createAddressOfRef(NewRef->getSelfBlobIndex(),
                                               NewRef->getDefinedAtLevel());
    NewRef->setBitCastDestType(ResultRefTy);

    // Widened operands contain reference dimensions and trailing struct
    // offsets. Add reference dimensions and trailing struct offsets.
    for (unsigned OpIdx = 1; OpIdx < RefOps.size();) {
      auto Operand = RefOps[OpIdx];
      AuxRefs.push_back(Operand);
      ++OpIdx;
      SmallVector<unsigned, 2> StructOffsets;
      while (OpIdx < RefOps.size() && VPGEP->isOperandStructOffset(OpIdx)) {
        const VPValue *VPOffset = VPInst->getOperand(OpIdx);
        ConstantInt *CVal =
            cast<ConstantInt>(cast<VPConstant>(VPOffset)->getConstant());
        unsigned Offset = CVal->getZExtValue();
        StructOffsets.push_back(Offset);
        ++OpIdx;
      }
      NewRef->addDimension(Operand->getSingleCanonExpr(), StructOffsets);
    }

    makeConsistentAndAddToMap(NewRef, VPInst, AuxRefs, Widen);
    return;
  }

  case Instruction::Call: {
    // Calls need to be masked with current mask value if Mask is null.
    if (!Mask)
      Mask = CurMaskValue;

    // TODO - VPValue codegen for call instructions still accesses
    // underlying node. This needs to be changed when we add all
    // necessary information such as call properties to VPInstruction.
    CallInst = dyn_cast<HLInst>(VPInst->HIR.getUnderlyingNode());
    assert(CallInst && CallInst->isCallInst() &&
           "Expected non-null underlying call instruction");

    // The Lval is not represented as an explicit operand in VPInstructions.
    NewInst =
        widenCall(CallInst, RefOps, Mask, CallArgs, false /* HasLvalArg */);
    if (!NewInst)
      return;
    break;
  }

  case VPInstruction::Not:
    NewInst = HLNodeUtilities.createNot(RefOp0, InstName);
    break;

  case VPInstruction::Pred:
    // Pred instruction is only used to mark the current block predicate. Simply
    // set the current mask value.
    setCurMaskValue(RefOp0);
    return;

  case VPInstruction::HIRCopy: {
    int OriginPhiId = cast<VPHIRCopyInst>(VPInst)->getOriginPhiId();
    RegDDRef *LValTmp = nullptr;
    if (OriginPhiId != -1) {
      if (auto *ExistingTmp = getLValTempForPhiId(OriginPhiId)) {
        // If a HIR temp was already created for this PHI ID, then re-use it
        // as Lval.
        LValTmp = ExistingTmp->clone();
      } else {
        // First occurrence of PHI ID, create a new HIR temp to be used as
        // Lval for all copies. TODO: Use SVA in future to decide between
        // vector/scalar type here.
        LValTmp = HLNodeUtilities.createTemp(
            FixedVectorType::get(VPInst->getType(), getVF()), "phi.temp");
        PhiIdLValTempsMap[OriginPhiId] = LValTmp;
      }
    }

    NewInst = HLNodeUtilities.createCopyInst(RefOp0, ".copy", LValTmp);
    break;
  }

  case VPInstruction::AllZeroCheck: {
    RegDDRef *A = RefOp0;
    if (Mask || CurMaskValue) {
      // TODO: Is this needed for HIR? The comment block in LLVM-IR CG adds this
      // case for masked inner-loops which we won't see in HIR. add A, Mask
      assert(false && "Mask supported for AllZeroCheck?");
    }

    RegDDRef *AllZeroCmp = generateCompareToZero(A, Mask, true /* Equal */);
    addVPValueScalRefMapping(VPInst, AllZeroCmp, 0);
    NewInst = HLNodeUtilities.createCopyInst(
        widenRef(AllZeroCmp->clone(), getVF()), "all.zero.check");
    break;
  }

  default:
    LLVM_DEBUG(VPInst->dump());
    llvm_unreachable("Unexpected VPInstruction opcode");
  }

  // Handle results of SVML sincos function calls
  // sincos function has two return values. The scalar sincos function uses
  // pointers as out-parameters. SVML sincos function, instead, returns them in
  // a struct directly. This bridges the gap between these two approaches.
  const class CallInst *Call = NewInst->getCallInst();
  if (Call &&
      Call->getCalledFunction()->getName().startswith("__svml_sincos")) {
    assert(CallInst && "Expected non-null CallInst");
    addInst(NewInst, nullptr);
    generateStoreForSinCos(CallInst, NewInst, Mask,
                           false /* IsRemainderLoop */);
    return;
  }

  addInst(NewInst, Mask);
  if (NewInst->hasLval())
    addVPValueWideRefMapping(VPInst, NewInst->getLvalDDRef());
}

void VPOCodeGenHIR::widenNodeImpl(const VPInstruction *VPInst, RegDDRef *Mask,
                                  const OVLSGroup *Grp,
                                  int64_t InterleaveFactor,
                                  int64_t InterleaveIndex,
                                  const HLInst *GrpStartInst) {
  // We treat select instruction in HIR specially. When generating code for
  // select instructions, the operands of the compare which generate the select
  // mask are part of the HIR select instruction. The HIR select instruction
  // also stores the compare predicate. As a result, we can avoid generating
  // code for a compare instruction if its only use is a select instruction.
  if (isa<VPCmpInst>(VPInst) && VPInst->getNumUsers() == 1) {
    auto *UserInst = cast<VPInstruction>(*(VPInst->users().begin()));
    if (UserInst->getOpcode() == Instruction::Select)
      return;
  }

  // Generate wide constructs for all VPInstuctions. This will be changed later
  // to use SVA information.
  generateHIR(VPInst, Mask, Grp, InterleaveFactor, InterleaveIndex,
              GrpStartInst, true /* Widen */);

  // Generate a scalar instruction for unitstride GEPs/subscripts. This will be
  // changed later to use SVA information.
  if (isa<VPGEPInstruction>(VPInst) || isa<VPSubscriptInst>(VPInst)) {
    bool IsNegOneStride;
    if (isUnitStridePtr(VPInst, IsNegOneStride))
      generateHIR(VPInst, Mask, Grp, InterleaveFactor, InterleaveIndex,
                  GrpStartInst, false /* Widen */);
    return;
  }

  // Generate a scalar value for some opcodes to avoid making references
  // non-linear. This is made possible due to support for folding such opcodes.
  // This will be changed later to use SVA information.
  if (isOpcodeForScalarInst(VPInst->getOpcode()))
    generateHIR(VPInst, Mask, Grp, InterleaveFactor, InterleaveIndex,
                GrpStartInst, false /* Widen */);
}

void VPOCodeGenHIR::createAndMapLoopEntityRefs(unsigned VF) {
  // Collect set of VPInstructions that are involved in a reduction. This set
  // is used to avoid folding instructions involved in reductions. This is
  // done by recursively collecting relevant instructions via def-use chains,
  // starting from the reduction initializers.
  std::function<void(const VPInstruction *)> collectRednVPInsts =
      [&](const VPInstruction *Inst) {
        // Do not process an already seen instruction.
        if (ReductionVPInsts.count(Inst)) {
          return;
        }

        ReductionVPInsts.insert(Inst);
        for (auto *User : Inst->users()) {
          if (!isa<VPInstruction>(User))
            continue;

          auto *UserInst = cast<VPInstruction>(User);
          // We are interested only in instructions that participate in
          // reduction, so return types should match.
          if (UserInst->getType() != Inst->getType())
            continue;

          collectRednVPInsts(UserInst);
        }
      };

  // Process reductions. For each reduction variable in the loop we create a new
  // RegDDRef to represent it. Next, we map the corresponding ReductionInit
  // user(PHI), and the PHI operands to the same RegDDRef. As part of mapping
  // the PHI operands, corresponding ReductionInit will also get mapped to the
  // same RegDDRef.
  std::function<void(VPReductionInit *, RegDDRef *)> mapRednToRednRef =
      [&](VPReductionInit *RedInit, RegDDRef *Ref) {
        assert(RedInit->getNumUsers() == 1 &&
               "Expected single user of reduction init");
        auto *RedPHI = cast<VPPHINode>(*(RedInit->users().begin()));
        ReductionRefs[RedPHI] = Ref;
        LLVM_DEBUG(dbgs() << "VPInst: "; RedPHI->dump();
                   dbgs() << " has the underlying reduction ref: ";
                   Ref->dump(true); dbgs() << "\n");

        for (const VPValue *Operand : RedPHI->operands()) {
          auto *PhiOpInst = cast<VPInstruction>(Operand);

          LLVM_DEBUG(dbgs() << "VPInst: "; PhiOpInst->dump();
                     dbgs() << " has the underlying reduction ref: ";
                     Ref->dump(true); dbgs() << "\n");
          ReductionRefs[PhiOpInst] = Ref;
        }
      };

  auto *VPLI = Plan->getVPLoopInfo();
  assert(std::distance(VPLI->begin(), VPLI->end()) == 1 &&
         "Expected single outermost loop!");
  VPLoop *OuterMostVPLoop = *VPLI->begin();
  VPBasicBlock *OuterMostLpPreheader =
      cast<VPBasicBlock>(OuterMostVPLoop->getLoopPreheader());

  for (VPInstruction &Inst : *OuterMostLpPreheader) {
    if (auto *RedInit = dyn_cast<VPReductionInit>(&Inst)) {
      RegDDRef *RednRef = HLNodeUtilities.createTemp(
          FixedVectorType::get(RedInit->getType(), VF), "red.var");
      mapRednToRednRef(RedInit, RednRef);
      collectRednVPInsts(RedInit);
    }
  }

  // Capture main loop IV instructions.
  bool MainLoopIVCaptured = false;
  for (VPInstruction &Inst : *OuterMostLpPreheader) {
    if (!isa<VPInductionInit>(&Inst))
      continue;

    auto *IndInit = cast<VPInductionInit>(&Inst);
    if (MainLoopIVCaptured)
      report_fatal_error(
          "HIR is expected to have only one loop induction variable.");
    MainLoopIVCaptured = true;

    std::function<void(VPInstruction *)> CaptureIVUpdates =
        [&](VPInstruction *Inst) {
          assert(Inst->getOpcode() == Instruction::Add &&
                 "Invalid induction variable.");
          assert(isa<VPInductionInitStep>(Inst->getOperand(0)) ||
                 isa<VPInductionInitStep>(Inst->getOperand(1)) &&
                     "One of the operands of IV update should be IV step.");
          MainLoopIVInsts.insert(Inst);
          for (auto *Op : Inst->operands()) {
            if (isa<VPPHINode>(Op) || isa<VPInductionInitStep>(Op)) {
              // Ignore IV step and PHI operands.
              continue;
            } else {
              CaptureIVUpdates(cast<VPInstruction>(Op));
            }
          }
        };

    assert(IndInit->getNumUsers() == 1 && "Invalid induction variable.");
    auto *User = *(IndInit->users().begin());
    VPPHINode *IndPHI = cast<VPPHINode>(User);
    MainLoopIVInsts.insert(IndPHI);
    VPInstruction *LastUpdate = cast<VPInstruction>(
        IndPHI->getOperand(0) == IndInit ? IndPHI->getOperand(1)
                                         : IndPHI->getOperand(0));
    CaptureIVUpdates(LastUpdate);
  }

  for (auto *V : MainLoopIVInsts) {
    LLVM_DEBUG(dbgs() << "VPInst:"; V->dump();
               dbgs() << " is associated with main loop IV.\n");
    (void)V;
  }

  // Process inductions
  // TODO
}

bool VPOCodeGenHIR::targetHasAVX512() const {
  return TTI->isAdvancedOptEnabled(
      TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX512);
}

void VPOCodeGenHIR::widenNode(const VPInstruction *VPInst, RegDDRef *Mask,
                              const OVLSGroup *Grp, int64_t InterleaveFactor,
                              int64_t InterleaveIndex,
                              const HLInst *GrpStartInst) {

  auto It = VPInstUnrollPart.find(VPInst);
  CurrentVPInstUnrollPart = It == VPInstUnrollPart.end() ? 0 : It->second;

  // Use VPValue based code generation if it is enabled and mixed CG has not
  // been forced
  if (EnableVPValueCodegenHIR && !getForceMixedCG()) {
    widenNodeImpl(VPInst, Mask, Grp, InterleaveFactor, InterleaveIndex,
                  GrpStartInst);
    return;
  }

  HLInst *WInst = nullptr;
  const VPInstruction::HIRSpecifics &HIR = VPInst->HIR;
  if (!Mask)
    Mask = CurMaskValue;

  // Generate code for IV related instructions so that we no longer rely
  // on using unrolled part. This now enables generating wide loads/stores
  // in mixed codegen path as well when the loads/stores are invalidated.
  if (MainLoopIVInsts.count(VPInst)) {
    widenNodeImpl(VPInst, Mask, Grp, InterleaveFactor, InterleaveIndex,
                  GrpStartInst);
    return;
  }

  // Always generate code for Phis/Blends in mixed code gen mode except for
  // search loops.
  if (!isSearchLoop() && (isa<VPPHINode>(VPInst) || isa<VPBlendInst>(VPInst))) {
    widenNodeImpl(VPInst, Mask, Grp, InterleaveFactor, InterleaveIndex,
                  GrpStartInst);
    return;
  }

  if (HIR.isDecomposed() && VPInst->isUnderlyingIRValid()) {
    // Skip decomposed non-PHI VPInstruction with valid HIR. This will be
    // codegen'ed by its master VPInstruction.
    LLVM_DEBUG(dbgs() << "Skipping decomposed VPInstruction with valid HIR:"
                      << *VPInst << "\n");
    return;
  }

  LLVM_DEBUG(dbgs() << "Vectorizing: ");
  LLVM_DEBUG(VPInst->dump());

  if (auto Branch = dyn_cast<VPBranchInst>(VPInst))
    if (const HLGoto *HGoto = Branch->getHLGoto()) {
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
      // predication related instructions.
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

  widenNodeImpl(VPInst, Mask, Grp, InterleaveFactor, InterleaveIndex,
                GrpStartInst);
}

void VPOCodeGenHIR::emitBlockLabel(const VPBasicBlock *VPBB) {
  // Nothing to do if no uniform control flow is seen
  if (!getUniformControlFlowSeen())
    return;

  if (VLoop->contains(VPBB)) {
    HLLabel *Label = HLNodeUtilities.createHLLabel(VPBB->getName());
    addInst(Label, nullptr /* Mask */);
    VPBBLabelMap[VPBB] = Label;
  }
}

void VPOCodeGenHIR::emitBlockTerminator(const VPBasicBlock *SourceBB) {
  // Nothing to do if no uniform control flow is seen
  if (!getUniformControlFlowSeen())
    return;

  // The loop backedge/exit is implicit in the vector loop. Do not emit
  // gotos in the latch block.
  if (VLoop->contains(SourceBB) && !VLoop->isLoopLatch(SourceBB)) {
    assert(SourceBB->getNumSuccessors() && "Expected at least one successor");
    const VPBasicBlock *Succ1 = SourceBB->getSuccessor(0);

    // If the block has two successors, we emit the following sequence
    // of code for 'SourceBB: (condbit: C1) Successors: Succ1, Succ2'.
    //
    //    Cond = extractelement C1_VEC, 0
    //    if (Cond == 1)
    //       goto Succ1_Label
    //    else
    //       goto Succ2_Label
    // If the block has a single succesor, we emit the following for
    // 'SourceBB: Successors: Succ1'.
    //
    //     goto Succ1_Label.
    // The gotos are created initially with a null target label. These are
    // fixed up at the end of vector code generation when the labels
    // are available for all basic blocks.
    if (SourceBB->getNumSuccessors() == 2) {
      const VPBasicBlock *Succ2 = SourceBB->getSuccessor(1);
      const VPValue *CondBit = SourceBB->getCondBit();
      auto *CondRef = getWideRefForVPVal(CondBit);
      assert(CondRef && "Missind widened DDRef!");
      HLInst *Extract = HLNodeUtilities.createExtractElementInst(
          CondRef->clone(), (unsigned)0, "unifcond");
      addInst(Extract, nullptr /* Mask */);
      CondRef = Extract->getLvalDDRef()->clone();
      HLIf *If = HLNodeUtilities.createHLIf(
          PredicateTy::ICMP_EQ, CondRef,
          DDRefUtilities.createConstDDRef(CondRef->getDestType(), 1));
      addInst(If, nullptr /* Mask */);

      HLGoto *ThenGoto = HLNodeUtilities.createHLGoto(nullptr);
      HLNodeUtils::insertAsFirstThenChild(If, ThenGoto);
      GotoTargetVPBBPairVector.push_back(std::make_pair(ThenGoto, Succ1));

      HLGoto *ElseGoto = HLNodeUtilities.createHLGoto(nullptr);
      HLNodeUtils::insertAsFirstElseChild(If, ElseGoto);
      GotoTargetVPBBPairVector.push_back(std::make_pair(ElseGoto, Succ2));
      LLVM_DEBUG(dbgs() << "Uniform IF seen\n");
    } else {
      HLGoto *Goto = HLNodeUtilities.createHLGoto(nullptr);
      addInst(Goto, nullptr /* Mask */);
      GotoTargetVPBBPairVector.push_back(std::make_pair(Goto, Succ1));
    }
  }
}

void VPOCodeGenHIR::finalizeGotos(void) {
  for (auto It : GotoTargetVPBBPairVector) {
    HLGoto *Goto = It.first;
    const VPBasicBlock *TargetBB = It.second;

    Goto->setTargetLabel(VPBBLabelMap[TargetBB]);
  }
}
} // end namespace llvm

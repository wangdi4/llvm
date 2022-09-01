//===----- IntelVPOCodeGenHIR.cpp -----------------------------------------===//
//
//   Copyright (C) 2017-2022 Intel Corporation. All rights reserved.
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
#include "../IntelLoopVectorizationPlanner.h"
#include "../IntelVPlanCallVecDecisions.h"
#include "../IntelVPlanUtils.h"
#include "../IntelVPlanVLSAnalysis.h"
#include "IntelVPlanHCFGBuilderHIR.h"
#include "IntelVPlanVLSClientHIR.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/Triple.h"
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
#if INTEL_FEATURE_SW_DTRANS
#include "Intel_DTrans/Transforms/DTransPaddedMalloc.h"
#endif // INTEL_FEATURE_SW_DTRANS

#define DEBUG_TYPE "VPOCGHIR"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::vpo;

STATISTIC(LoopsVectorized, "Number of HIR loops vectorized");

extern bool Usei1MaskForSimdFunctions;

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

static cl::opt<bool> DisableCondLastPrivCG(
    "disable-hir-cond-last-priv-cg", cl::init(false), cl::Hidden,
    cl::desc(
        "Disable HIR vector code generation for conditional last privates"));

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
static cl::opt<int> PrintHIRAfterVPlan(
    "print-hir-after-vplan", cl::init(0),
    cl::desc("Print vectorized HIR after VPlanDriverHIR, on per-HLLoop basis. "
             "The value of the switch: 0 - do not print, 1 means print w/o "
             "details, >1 causes printing HIR with details."));
#endif // !NDEBUG || LLVM_ENABLE_DUMP

namespace llvm {

// Helper method to determine if instruction is strictly identified as being
// scalar for first lane by SVA.
static bool instIsStrictlyFirstScalar(const VPInstruction *VPInst) {
  auto *Plan = cast<VPlanVector>(VPInst->getParent()->getParent());
  auto *SVA = Plan->getVPlanSVA();
  return SVA->instNeedsFirstScalarCode(VPInst) &&
         !SVA->instNeedsVectorCode(VPInst) &&
         !SVA->instNeedsLastScalarCode(VPInst);
}

static RegDDRef *getConstantSplatDDRef(DDRefUtils &DDRU, Constant *ConstVal,
                                       unsigned VF) {
  Constant *ConstVec =
      ConstantVector::getSplat(ElementCount::getFixed(VF), ConstVal);
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

/// Create a sequential shuffle mask. This function mimics the interface in
/// VectorUtils.cpp which requires us to pass in an IRBuilder. This function
/// creates shuffle mask whose elements are sequential and begin at Start. The
/// mask contains NumInts integers and is padded with NumUndefs undef values.
/// The mask is of the form: <Start, Start + 1, ... Start + NumInts - 1,
/// undef_1, ... undef_NumUndefs> For example, the mask for Start = 0, NumInts
/// = 4, and NumUndefs = 4 is: <0, 1, 2, 3, undef, undef, undef, undef>
static Constant *createSequentialMask(unsigned Start, unsigned NumInts,
                                      unsigned NumUndefs,
                                      LLVMContext &Context) {
  SmallVector<Constant *, 16> Mask;
  for (unsigned I = 0; I < NumInts; ++I)
    Mask.push_back(ConstantInt::get(Type::getInt32Ty(Context), Start + I));

  Constant *Undef = UndefValue::get(Type::getInt32Ty(Context));
  for (unsigned I = 0; I < NumUndefs; ++I)
    Mask.push_back(Undef);

  return ConstantVector::get(Mask);
}

HLInst *VPOCodeGenHIR::createReverseVector(RegDDRef *ValRef) {
  unsigned NumElems =
      cast<FixedVectorType>(ValRef->getDestType())->getNumElements();
  SmallVector<Constant *, 4> ShuffleMask;
  for (unsigned I = 0; I < NumElems; I++) {
    Constant *Mask =
        ConstantInt::get(Type::getInt32Ty(Context), NumElems - I - 1);
    ShuffleMask.push_back(Mask);
  }

  HLInst *Shuffle = createShuffleWithUndef(ValRef, ShuffleMask, "reverse");
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

// Utility to check if given VPInstruction has no external uses.
static bool hasNoExternalUsers(const VPInstruction *VPI) {
  return llvm::none_of(VPI->users(),
                       [](VPUser *U) { return isa<VPLiveOutValue>(U); });
}

// Utility to find the single VPExternalUse that uses the given VPInstruction.
static const VPExternalUse *getSingleExternalUse(const VPInstruction *VPI,
                                                 const VPlan *Plan) {
  auto Pred = [](VPUser *U) { return isa<VPLiveOutValue>(U); };
  auto It = llvm::find_if(VPI->users(), Pred);
  assert(
      (It != VPI->user_end() && std::none_of(It + 1, VPI->user_end(), Pred)) &&
      "VPInst has zero or more than one VPExternalUse");
  // VPInst has single external user.
  const VPExternalUse *EUse = Plan->getExternals().getVPExternalUse(
      cast<const VPLiveOutValue>(*It)->getMergeId());
  return EUse;
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
    ACG->addInst(Inst, MaskDDRef);
  }

  // Insert instruction into HLIf region.
  void addInst(HLInst *Inst, const bool IsThenChild) {
    ACG->addInst(Inst, MaskDDRef, IsThenChild);
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
  RegDDRef *visitPtrToIntExpr(const SCEVPtrToIntExpr *);
  RegDDRef *visitAddExpr(const SCEVAddExpr *Expr);
  RegDDRef *visitMulExpr(const SCEVMulExpr *Expr);
  RegDDRef *visitUDivExpr(const SCEVUDivExpr *Expr);
  RegDDRef *visitAddRecExpr(const SCEVAddRecExpr *Expr);
  RegDDRef *visitSMaxExpr(const SCEVSMaxExpr *Expr);
  RegDDRef *visitUMaxExpr(const SCEVUMaxExpr *Expr);
  RegDDRef *visitSMinExpr(const SCEVSMinExpr *Expr);
  RegDDRef *visitUMinExpr(const SCEVUMinExpr *Expr);
  RegDDRef *visitSequentialUMinExpr(const SCEVSequentialUMinExpr *Expr);
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
          ConvOpCode == Instruction::Trunc ||
          ConvOpCode == Instruction::PtrToInt) &&
         "Unexpected conversion OpCode");

  Type *VecTy = getWidenedType(DestType, ACG->getVF());
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

RegDDRef *NestedBlobCG::visitPtrToIntExpr(const SCEVPtrToIntExpr *Expr) {
  RegDDRef *Src = visit(Expr->getOperand());
  return codegenConversion(Src, Instruction::PtrToInt, Expr->getType());
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

RegDDRef *NestedBlobCG::visitSequentialUMinExpr(
    const SCEVSequentialUMinExpr *Expr) {
  llvm_unreachable("SCEVSequentialUMinExpr not implemented");
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
  bool VectorizableCallSeen;
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
        UnitStrideRefSeen(false), MemRefSeen(false),
        VectorizableCallSeen(false), CG(CG) {
    LoopLevel = OrigLoop->getNestingLevel();
  }

  void visit(HLDDNode *Node);

  void visit(HLNode *Node) {
    // Current CG uses VPlan to generate the code, so Gotos should be ok to
    // support in CG.
    if (!CG->isSearchLoop()) {
      DEBUG_WITH_TYPE(
          "VPOCGHIR-bailout",
          dbgs() << "VPLAN_OPTREPORT: Loop not handled - unsupported HLNode\n");
      IsHandled = false;
    }
  }

  void postVisit(HLNode *Node) {}

  bool isDone() const { return false; }
  bool isHandled() { return IsHandled; }
  bool getUnitStrideRefSeen() { return UnitStrideRefSeen; }
  bool getMemRefSeen() { return MemRefSeen; }
  bool getVectorizableCallSeen() { return VectorizableCallSeen; }
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
  if (!isa<HLInst>(Node) && !isa<HLIf>(Node) && !isa<HLLoop>(Node)) {
    DEBUG_WITH_TYPE("VPOCGHIR-bailout",
                    dbgs() << "VPLAN_OPTREPORT: Loop not handled - only "
                              "HLInst/HLIf/HLLoop are supported\n");
    IsHandled = false;
    return;
  }

  // Calls supported are masked/non-masked svml and non-masked intrinsics.
  if (HLInst *Inst = dyn_cast<HLInst>(Node)) {
    auto LLInst = Inst->getLLVMInstruction();

    bool HasVectorVariant = false;
    if (const CallInst *Call = Inst->getCallInst()) {
      Function *Fn = Call->getCalledFunction();
      if (Fn && Fn->hasFnAttribute("vector-variants"))
        HasVectorVariant = true;
    }

    // Vector functions cannot throw exceptions, but if the original function
    // is only declared (i.e., externed), then it will not be marked with the
    // nounwind attribute and an explicit "vector-variants" attribute check is
    // also necessary to avoid bailing out.
    if (LLInst->mayThrow() && !HasVectorVariant) {
      DEBUG_WITH_TYPE("VPOCGHIR-bailout", Inst->dump());
      DEBUG_WITH_TYPE(
          "VPOCGHIR-bailout",
          dbgs()
              << "VPLAN_OPTREPORT: Loop not handled - instruction may throw\n");
      IsHandled = false;
      return;
    }

    auto Opcode = LLInst->getOpcode();
    if (Opcode == Instruction::Alloca) {
      DEBUG_WITH_TYPE("VPOCGHIR-bailout", Inst->dump());
      DEBUG_WITH_TYPE(
          "VPOCGHIR-bailout",
          dbgs() << "VPLAN_OPTREPORT: Loop not handled - alloca instruction\n");
      IsHandled = false;
      return;
    }

    auto TLval = Inst->getLvalDDRef();
    unsigned MaskedRedOpcode = 0;
    if (DisableCondLastPrivCG && !CG->isSearchLoop() && TLval &&
        TLval->isTerminalRef() && OrigLoop->isLiveOut(TLval->getSymbase()) &&
        Inst->getParent() != OrigLoop &&
        !CG->isReductionRef(TLval, MaskedRedOpcode)) {
      DEBUG_WITH_TYPE("VPOCGHIR-bailout", Inst->dump());
      DEBUG_WITH_TYPE("VPOCGHIR-bailout",
                      dbgs() << "VPLAN_OPTREPORT: Liveout conditional "
                                "non-reduction scalar assign not handled\n");
      IsHandled = false;
      return;
    }

    if (const CallInst *Call = Inst->getCallInst()) {
      if (!CG->isIgnoredCall(Call) && (CG->getForceMixedCG())) {
        // TODO: Is this case possible? We use mixed CG only for search loops
        // now.
        LLVM_DEBUG(Inst->dump());
        DEBUG_WITH_TYPE("VPOCGHIR-bailout", Inst->dump());
        DEBUG_WITH_TYPE(
            "VPOCGHIR-bailout",
            dbgs() << "VPLAN_OPTREPORT: Loop not handled - call vectorization "
                      "not supported in mixed CG mode\n");
        IsHandled = false;
        return;
      }

      Function *Fn = Call->getCalledFunction();
      if (!Fn) {
        DEBUG_WITH_TYPE("VPOCGHIR-bailout", Inst->dump());
        DEBUG_WITH_TYPE("VPOCGHIR-bailout",
            dbgs() << "VPLAN_OPTREPORT: Loop not handled - indirect call\n");
        IsHandled = false;
        return;
      }

      StringRef CalledFunc = Fn->getName();
      Intrinsic::ID ID = getVectorIntrinsicIDForCall(Call, TLI);

      // Prevent vectorization of loop if masked fabs intrinsic vectorization is
      // not profitable specifically for ADL target. Masked fabs intrinsics is
      // used to catch the pattern. Slowdown is caused by gathers that are slow
      // on ADL-E cores but not CM'ed properly.
      // Check JIRAS : CMPLRLLVM-34347, CMPLRLLVM-35171.
      // Once issues with TTI cost modelling for ADL gathers are resolved this
      // bailout is expected to be removed.
      bool IsUnprofitableFabs = ID == Intrinsic::fabs &&
                                !VPlanAssumeMaskedFabsProfitable &&
                                CG->getFunction().getFnAttribute("target-cpu").
                                  getValueAsString() == "alderlake";
      if (isa<HLIf>(Inst->getParent()) && VF > 1 && IsUnprofitableFabs) {
        DEBUG_WITH_TYPE("VPOCGHIR-bailout", Inst->dump());
        DEBUG_WITH_TYPE("VPOCGHIR-bailout",
                        dbgs() << "VPLAN_OPTREPORT: Loop not handled - masked "
                        "fabs intrinsic for AlderLake.\n");
        IsHandled = false;
        return;
      }

      // Quick hack to avoid loops containing fabs in 447.dealII from becoming
      // vectorized due to bug in unrolling. The problem involves loop index
      // variable that spans outside the array range, resulting in segfault.
      // floor calls are also temporarily disabled until FeatureOutlining is
      // fixed (CQ410864)
      if (CalledFunc == "fabs" || CalledFunc == "floor") {
        DEBUG_WITH_TYPE("VPOCGHIR-bailout", Inst->dump());
        DEBUG_WITH_TYPE("VPOCGHIR-bailout",
            dbgs() << "VPLAN_OPTREPORT: Loop not handled - fabs/floor call "
                      "disabled\n");
        IsHandled = false;
        return;
      }

      if (!CG->isIgnoredCall(Call))
        VectorizableCallSeen = true;
    }

    // Checks for instructions that operate on VectorType.
    if (Opcode == Instruction::ExtractElement ||
        Opcode == Instruction::InsertElement) {
      // Index will be the last operand of insert/extractelement instruction.
      auto *IndexRef = Inst->getOperandDDRef(Inst->getNumOperands() - 1);
      if (!IndexRef->isIntConstant()) {
        DEBUG_WITH_TYPE("VPOCGHIR-bailout", Inst->dump());
        DEBUG_WITH_TYPE("VPOCGHIR-bailout",
                        dbgs()
                            << "VPLAN_OPTREPORT: Loop not handled - "
                               "insert/extractelement with variable index\n");
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

  bool NodeIsCall = false;
  if (auto *Inst = dyn_cast<HLInst>(RegDD->getHLDDNode()))
    NodeIsCall = Inst->isCallInst();

  // Call serialization is supported by CG, so don't check for non-vectorizable
  // types for calls.
  if (!NodeIsCall && !isVectorizableTy(RegDD->getDestType())) {
    DEBUG_WITH_TYPE("VPOCGHIR-bailout", RegDD->dump(); dbgs() << "\n");
    DEBUG_WITH_TYPE("VPOCGHIR-bailout", RegDD->getDestType()->dump());
    DEBUG_WITH_TYPE("VPOCGHIR-bailout",
                    dbgs() << "VPLAN_OPTREPORT: Loop not handled - "
                              "non-vectorizable destination type\n");
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
  if (RegDD->hasGEPInfo())
    MemRefSeen = true;
}

// Checks Canon Expr to see if we support it. Currently, we do not
// support blob IV coefficients
void HandledCheck::visitCanonExpr(CanonExpr *CExpr, bool InMemRef,
                                  bool InMaskedStmt) {
  // Track if we see a memory reference with a negative IV coefficient
  if (InMemRef) {
    int64_t ConstCoeff = 0;
    CExpr->getIVCoeff(LoopLevel, nullptr, &ConstCoeff);
  }
  if (!EnableBlobCoeffVec && CExpr->hasIVBlobCoeff(LoopLevel)) {
    DEBUG_WITH_TYPE("VPOCGHIR-bailout",
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
          DEBUG_WITH_TYPE("VPOCGHIR-bailout", dbgs() << "VPLAN_OPTREPORT: Masked divide support TBI\n");
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
      DEBUG_WITH_TYPE("VPOCGHIR-bailout", dbgs() << "VPLAN_OPTREPORT: Loop not handled - nested blob\n");
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
  else {
    // Check that all VPLoops are in loop simplified form and that Latch
    // has conditional branch.
    for (VPLoop *OuterLoop : *Plan->getVPLoopInfo()) {
      for (auto *VPLp : post_order(OuterLoop)) {
        // Essentially the same checks as the ones done by isLoopSimplifyForm
        // used by loopopt, except for the additional check for single exit
        // block.
        auto isLoopSimplifyForm = [](const VPLoop *VPLp) {
          return VPLp->getLoopPreheader() && VPLp->getLoopLatch() &&
                 VPLp->hasDedicatedExits() && VPLp->getExitBlock();
        };
        if (!isLoopSimplifyForm(VPLp)) {
          DEBUG_WITH_TYPE("VPOCGHIR-bailout",
                          dbgs()
                              << "VPLAN_OPTREPORT: Loop not handled - expect "
                                 "loopsimplify form\n");
          return false;
        }
        auto *Latch = VPLp->getLoopLatch();
        assert(Latch && "Expected non-null loop latch");
        if (Latch->getNumSuccessors() != 2) {
          DEBUG_WITH_TYPE("VPOCGHIR-bailout",
                          dbgs()
                              << "VPLAN_OPTREPORT: Loop not handled - expect "
                                 "latch with 2 successors\n");
          return false;
        }
      }
    }
  }

  // Only handle normalized loops
  if (!Loop->isNormalized()) {
    DEBUG_WITH_TYPE("VPOCGHIR-bailout",
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
      DEBUG_WITH_TYPE("VPOCGHIR-bailout",
          dbgs() << "VPLAN_OPTREPORT: Loop not handled - loop with small "
                    "trip count\n");
      return false;
    }

    // Check that main vector loop will have at least one iteration
    if (ConstTripCount < VF) {
      DEBUG_WITH_TYPE("VPOCGHIR-bailout",
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
    DEBUG_WITH_TYPE("VPOCGHIR-bailout", dbgs() << "VPLAN_OPTREPORT: Loop not handled - all mem refs non "
                         "unit-stride\n");
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
  if (!ReductionVPInsts.empty())
    RednHoistLp = findRednHoistInsertionPoint(OrigLoop);
  else
    RednHoistLp = OrigLoop;
}

VPValue *VPOCodeGenHIR::getLoopIVUpper(const VPLoop *VPLp,
                                       const VPPHINode *IVPhi) {
  assert(VPLp->getParentLoop() &&
         "Unexpected getLoopIVUpper call for outermost VPLoop");
  assert(VPLoopIVPhiMap[VPLp] == IVPhi &&
         "IVPhi is expected to be VPLp's IV PHI");
  auto *ExitingBlock = VPLp->getExitingBlock();
  assert(ExitingBlock && "Expected non-null exiting block");

  auto *ExitChk = ExitingBlock->getCondBit();
  assert(ExitChk && "Expected non-null exit check");

  // Backedge compare is expected to be in the following form to determine
  // IVUpper.
  //
  // BB5:
  //   [DA: Uni] i64 %ivphi = phi  [ i64 %vp31772, BB4 ],  [ i64 %vp31800, BB5 ]
  //   ...
  //   ...
  //   [DA: Uni] i64 %vp24002 = add i64 %ivphi i64 1
  //   [DA: Uni] i1 %vp25292 = icmp slt/ult i64 %vp24002 i64 100
  //   [DA: Uni] i64 %vp31800 = hir-copy i64 %vp24002 , OriginPhiId: 0
  //   [DA: Uni] br i1 %vp25292, BB5, BB6
  auto *CmpInst = dyn_cast<VPCmpInst>(ExitChk);
  if (!CmpInst)
    return nullptr;

  auto CmpPred = CmpInst->getPredicate();
  if (CmpPred != CmpInst::ICMP_SLT && CmpPred != CmpInst::ICMP_ULT)
    return nullptr;

  auto *CmpOp0 = CmpInst->getOperand(0);
  auto *CmpOp1 = CmpInst->getOperand(1);
  assert(!Plan->getVPlanDA()->isDivergent(*CmpInst) &&
         !Plan->getVPlanDA()->isDivergent(*CmpOp1) &&
         "Expected uniform loop bottom test");

  auto *AddInst = dyn_cast<VPInstruction>(CmpOp0);
  if (!AddInst || AddInst->getOpcode() != Instruction::Add)
    return nullptr;

  // Check that AddInst copy is used as IVPhi's incoming value for Latch.
  auto *Latch = VPLp->getLoopLatch();
  assert(Latch && "Expected non-null loop latch");
  VPHIRCopyInst *LatchCopy =
      cast<VPHIRCopyInst>(IVPhi->getIncomingValue(Latch));
  if (LatchCopy->getOperand(0) != AddInst)
    return nullptr;

  // The second operand of the compare is the IV upper value.
  return CmpOp1;
}

// Helper function to set up trip count estimates for non-constant trip loop to
// given TC and mark it as donotunroll. The loop gets marked donotvectorize.
static void setLoopTCEstimatesAndMarkers(HLLoop *Loop, unsigned TC) {
  // TODO - HIR transform utils also adjust profile data and other metadata.
  // Need to look into the same.
  if (!Loop->isConstTripLoop()) {
    Loop->setMaxTripCountEstimate(TC);
    Loop->setLegalMaxTripCount(TC);
    Loop->setPragmaBasedMaximumTripCount(TC);
    Loop->markDoNotUnroll();
  }

  Loop->clearPrefetchingPragmaInfo();
  Loop->markDoNotVectorize();
}

void VPOCodeGenHIR::setupHLLoop(const VPLoop *VPLp) {
  // For merged CFG we reuse the MainLoop created earlier for first top-level
  // loop, and for all other top-level loops (vectorized remainder for example)
  // we use an empty clone of MainLoop.
  auto getMergedCFGVecHLLoop = [&](const VPLoop *VLp) {
    if (*Plan->getVPLoopInfo()->begin() == VLp)
      return MainLoop;
    return MainLoop->cloneEmpty();
  };

  assert(MainLoop && "Expected non-null MainLoop");
  HLLoop *HLoop;
  if (!VPLp->getParentLoop()) {
    CurTopVPLoop = VPLp;
    HLoop = isSearchLoop() ? MainLoop : getMergedCFGVecHLLoop(VPLp);
    if (!isSearchLoop()) {
      // TODO - this code is making some implicit assumptions that the first
      // VPLoop is the main vector loop and using its VF(MaxVF) to set
      // trip count estimates. This needs to be improved to get the VF
      // information explicitly from the merger and to give better estimates
      // for the different vector loops.
      if (*Plan->getVPLoopInfo()->begin() == VPLp)
        HIRTransformUtils::adjustTCEstimatesForUnrollOrVecFactor(
            HLoop, getVF() * getUF());
      else
        setLoopTCEstimatesAndMarkers(HLoop, MaxVF / VF);
    }
  } else {
    Type *IVTy;
    VPValue *IVUpper = nullptr;
    auto Itr = VPLoopIVPhiMap.find(VPLp);
    if (Itr != VPLoopIVPhiMap.end()) {
      IVTy = Itr->second->getType();
      IVUpper = getLoopIVUpper(VPLp, Itr->second);
    } else {
      IVTy = Type::getInt64Ty(HLNodeUtilities.getContext());
    }

    // If we have a known IV upper bound, we can generate a DO loop with
    // lower(0), upper(IVUpper - 1), and stride(1).
    if (IVUpper) {
      auto *LowerRef = DDRefUtilities.createConstDDRef(IVTy, 0);
      auto *StrideRef = DDRefUtilities.createConstDDRef(IVTy, 1);
      auto *UpperRef = getOrCreateScalarRef(IVUpper, 0 /*ScalarLaneID*/);

      // HIR DO loop upper bound is inclusive, subtract 1 to account for
      // the same.
      if (UpperRef->isConstant()) {
        UpperRef->getSingleCanonExpr()->addConstant(-1, true);
      } else {
        HLInst *AddInst = HLNodeUtilities.createAdd(
            UpperRef, DDRefUtilities.createConstDDRef(IVTy, -1), "upper");
        addInstUnmasked(AddInst);
        auto *CurLoop = InsertPoint->getParentLoop();
        assert(CurLoop && "Unexpected null current loop");
        UpperRef = AddInst->getLvalDDRef()->clone();
        UpperRef->getSingleCanonExpr()->setDefinedAtLevel(
            CurLoop->getNestingLevel());
      }
      HLoop = HLNodeUtilities.createHLLoop(nullptr /* ZttIf */, LowerRef,
                                           UpperRef, StrideRef, 1 /* NumEx */);
      // Add UpperRef as livein for the non-constant case.
      if (!UpperRef->isConstant())
        HLoop->addLiveInTemp(UpperRef);
    } else {
      auto *DDR = DDRefUtilities.createConstDDRef(IVTy, 0);
      HLoop = HLNodeUtilities.createHLLoop(
          nullptr /* ZttIf */, DDR, DDR->clone(), DDR->clone(), 1 /* NumEx */);
    }
  }

  // Add HLLoop to the map.
  VPLoopHLLoopMap[VPLp] = HLoop;
}

void VPOCodeGenHIR::setupLoopsForLegacyCG(unsigned VF, unsigned UF) {
  // Setup peel, main and remainder loops
  // TODO: Peeling decisions should be properly made in VPlan's cost model and
  // not during code generation. The following logic is a temporary workaround
  // to have peeling functionality working for vectorized search loops.

  bool SearchLoopNeedsPeeling =
      TTI->isAdvancedOptEnabled(
          TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2) &&
      (EnableFirstIterPeelMEVec || EnablePeelMEVec) && isSearchLoop() &&
      (getSearchLoopType() == VPlanIdioms::SearchLoopStrEq ||
       getSearchLoopType() == VPlanIdioms::SearchLoopPtrEq);

  if (isSearchLoop() &&
      getSearchLoopType() == VPlanIdioms::SearchLoopPtrEq) {
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

  // For legacy CFG continue to use HIRTransformUtils to emit implicit peel,
  // vector and remainder loops.
  HLLoop *MainLoop = HIRTransformUtils::setupPeelMainAndRemainderLoops(
      OrigLoop, VF * UF, NeedRemainderLoop, ORBuilder,
      OptimizationType::Vectorizer, &PeelLoop, SearchLoopPeelArrayRef,
      &RTChecks);

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

  setNeedRemainderLoop(NeedRemainderLoop);
  setMainLoop(MainLoop);
}

void VPOCodeGenHIR::setupLoopsForMergedCFG() {
  // TODO: Port some functionalities like profiling data from
  // HIRTransformUtils::setupPeelMainAndRemainderLoops. For merged CFG just
  // create an empty clone of original scalar loop to correspond to the main
  // vector loop. We also extract preheader and postexit to preserve it in
  // generated vector HIR.
  OrigLoop->extractZttPreheaderAndPostexit();
  HLLoop *MainLoop = OrigLoop->cloneEmpty();
  setNeedPeelLoop(CFGInfo.isPeelLoopEmitted());
  setNeedRemainderLoop(CFGInfo.isRemainderLoopEmitted());
  setMainLoop(MainLoop);
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

  if (isSearchLoop())
    setupLoopsForLegacyCG(VF, UF);
  else
    setupLoopsForMergedCFG();

  if (!MainLoop) {
    assert(false && "Main loop could not be setup.");
    // Bailout for prod builds
    return false;
  }

  // Collect vploop preheader/header/exit blocks.
  for (VPLoop *OuterLoop : *Plan->getVPLoopInfo()) {
    for (auto *VPLp : post_order(OuterLoop)) {
      LoopPreheaderBlocks.insert(VPLp->getLoopPreheader());
      LoopHeaderBlocks.insert(VPLp->getHeader());
      LoopExitBlocks.insert(VPLp->getExitBlock());
    }
  }

  // Do not attempt hoisting when a remainder loop is needed as remainder
  // loop updates the scalar reduction value. Since we blend in the scalar
  // reduction value in reduction initialization code, hoisting up the
  // initialization will generate incorrect code. The same holds if we
  // generate a peel loop.
  if (NeedRemainderLoop || NeedPeelLoop)
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

  MainLoop->extractZtt();
  addInsertRegion(MainLoop);

  return true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPOCodeGenHIR::dumpFinalHIR() {
  bool Detailed = PrintHIRAfterVPlan > 1;
  if (!isSearchLoop()) {
    MainLoop->getParent()->dump(Detailed);
    return;
  }

  if (NeedPeelLoop)
    PeelLoop->dump(Detailed);
  MainLoop->getParent()->dump(Detailed);
  if (NeedRemainderLoop)
    OrigLoop->dump(Detailed);
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

void VPOCodeGenHIR::setupLiveInLiveOut() {
  if (isSearchLoop())
    return;

  auto addLiveInLiveOut = [this](const VPValue *Def, const VPUser *User,
                                 const RegDDRef *DefRef) -> void {
    const VPLoop *DefVPLoop = nullptr, *UseVPLoop = nullptr;
    HLLoop *DefHLLoop = nullptr, *UseHLLoop = nullptr;

    // DefHLLoop is set to the HLLoop corresponding to the VPLoop in which
    // Def occurs if Def is an instruction. Otherwise, it remains null(for
    // external def or def instruction outside the vector loop).
    if (auto *DefInst = dyn_cast<VPInstruction>(Def))
      DefVPLoop = Plan->getVPLoopInfo()->getLoopFor(DefInst->getParent());
    if (DefVPLoop)
      DefHLLoop = VPLoopHLLoopMap[DefVPLoop];

    // UseHLLoop is set to the HLLoop corresponding to the VPLoop in which
    // Use occurs if Use is an instruction. Otherwise, it remains null(for
    // external use or use instruction outside the vector loop).
    if (auto *UseInst = dyn_cast<VPInstruction>(User))
      UseVPLoop = Plan->getVPLoopInfo()->getLoopFor(UseInst->getParent());
    if (UseVPLoop)
      UseHLLoop = VPLoopHLLoopMap[UseVPLoop];

    // Simple case - no need to add livein/liveout information
    if (DefVPLoop == UseVPLoop)
      return;

    // Def outside the outer most loop. Propagate livein information
    // starting from use loop all the way up to and including MainLoop.
    if (!DefVPLoop) {
      assert(UseHLLoop && "Expected non-null use loop");
      while (UseHLLoop != MainLoop->getParentLoop()) {
        UseHLLoop->addLiveInTemp(DefRef);
        UseHLLoop = UseHLLoop->getParentLoop();
      }
      return;
    }

    // Use outside the outer most loop. Propagate liveout information
    // starting from  def loop all the way up to and including MainLoop.
    if (!UseVPLoop) {
      assert(DefHLLoop && "Expected non-null def loop");
      while (DefHLLoop != MainLoop->getParentLoop()) {
        DefHLLoop->addLiveOutTemp(DefRef);
        DefHLLoop = DefHLLoop->getParentLoop();
      }
      return;
    }

    // Update livein/liveout information for the case where the def and use
    // appear in different VPLoops.
    assert(UseHLLoop && DefHLLoop && "Expected non-null use and def loops");
    auto *LCAHLLoop =
        HLNodeUtils::getLowestCommonAncestorLoop(DefHLLoop, UseHLLoop);

    // Add DefRef as livein into UseHLLoop and recursively into its parent loop
    // until we reach LCAHLLoop.
    while (UseHLLoop != LCAHLLoop) {
      UseHLLoop->addLiveInTemp(DefRef);
      UseHLLoop = UseHLLoop->getParentLoop();
    }

    while (DefHLLoop != LCAHLLoop) {
      DefHLLoop->addLiveOutTemp(DefRef);
      DefHLLoop = DefHLLoop->getParentLoop();
    }
  };

  // For each definition in VPValWideRef map, get the users and add the
  // livein/liveout information
  for (auto &ValRef : VPValWideRefMap) {
    auto *Def = ValRef.first;
    auto *DefRef = ValRef.second;
    for (auto *User : Def->users())
      addLiveInLiveOut(Def, User, DefRef);
  }

  // For each external value in VPValsToFlushForVF map add the livein
  // information. VPExternals can't be liveout.
  for (auto &ValRef : VPValsToFlushForVF) {
    auto *Def = ValRef.first;
    if (isa<VPConstant>(Def))
      continue;

    for (auto *User : Def->users()) {
      VPInstruction *UserInst = dyn_cast<VPInstruction>(User);
      if (UserInst) {
        auto ParentBB = UserInst->getParent();

        // Can have a use of VPExternalDef in a VPlan that was not merged.
        if (ParentBB->getParent() != Plan)
          continue;

        // Get topmost VPloop (which can be peel/main/remainder) that contains
        // the use. We keep the DDRefs along with those VPloops in the map.
        const VPLoop *VLp = Plan->getVPLoopInfo()->getLoopFor(ParentBB);
        while (VLp && VLp->getParentLoop())
          VLp = VLp->getParentLoop();

        // User outside of any loop
        if (!VLp)
          continue;

        for (auto &DefRef : ValRef.second)
          if (VLp == DefRef.second) {
            addLiveInLiveOut(Def, User, DefRef.first);
          }
      }
    }
  }

  // For each definition in VPValScalRef map, get the users and add the
  // livein/liveout information
  for (auto &ValRef : VPValScalRefMap) {
    auto *Def = ValRef.first;
    auto &ScalMap = ValRef.second;
    for (auto &LaneRef : ScalMap) {
      auto *DefRef = LaneRef.second;
      for (auto *User : Def->users())
        addLiveInLiveOut(Def, User, DefRef);
    }
  }
}

void VPOCodeGenHIR::finalizeVectorLoop(void) {
  eliminateRedundantGotosLabels();
  setupLiveInLiveOut();

  if (!isSearchLoop()) {
    // Set the code gen for modified region. This will ensure outgoing HIR
    // region is lowered to LLVM-IR by HIR-CG.
    assert(MainLoop->getParentRegion() &&
           " Loop does not have a parent region.");
    MainLoop->getParentRegion()->setGenCode();

    // Mark parent for invalidation
    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(MainLoop);
  } else {
    // For non-merged CFG path, just track the scalar HLLoops emitted -
    // remainder is stored in OrigLoop.
    OutgoingScalarHLLoops.insert(OrigLoop);
    OutgoingScalarHLLoops.insert(PeelLoop);
  }

  LLVM_DEBUG(dbgs() << "\n\n\nHandled loop after: \n");
  LLVM_DEBUG(dumpFinalHIR());
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (PrintHIRAfterVPlan) {
    dbgs() << "Handled loop after VPlan: in Function: " << Fn.getName() << "\n";
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
  }

  // Mark generated loops as novectorize and nounroll as needed.
  for (auto &VPLpHLoop : VPLoopHLLoopMap) {
    HLLoop *HLoop = VPLpHLoop.second;
    HLoop->markDoNotVectorize();

    // Prevent LLVM from possibly unrolling vectorized loops with non-constant
    // trip counts. See loop in function fxpAutoCorrelation() that is part of
    // telecom/autcor00data_1 (opt_base_st_64_hsw). Inner loop has max trip
    // count estimate of 16, VPO vectorizer chooses VF=4, and LLVM unrolls by 4.
    // However, the inner loop does not always have a constant 16 trip count,
    // leading to a performance degradation caused by entering the scalar code
    // path.
    // TODO - currently this workaround only kicks in for loops at the
    // vectorization level. This marking may be extended to inner loops inside
    // a vectorized outer loop.
    if (!VPLpHLoop.first->getParentLoop() && !HLoop->isConstTripLoop())
      HLoop->markDoNotUnroll();
  }
  if (NeedRemainderLoop)
    OrigLoop->markDoNotVectorize();

  // Disable further vectorization/unroll attempts on main loop for search loop
  // case explicitly until VPlan representation is made explicit. TODO: Remove
  // this once search loop representation is made explicit.
  if (isSearchLoop()) {
    MainLoop->markDoNotVectorize();
    if (!MainLoop->isConstTripLoop())
      MainLoop->markDoNotUnroll();
  }

  // Lower remarks collected in VPLoops to outgoing vector/scalar HLLoops. This
  // is done always except for search loops. This should be done before
  // complete unroll optimization below since that would lead to loss of vector
  // loop.
  if (!isSearchLoop()) {
    emitRemarksForScalarLoops();
    lowerRemarksForVectorLoops();
  }

  // If a remainder loop is not needed get rid of the OrigLoop at this point.
  // Replace calls in remainderloop for FP consistency
  if (NeedRemainderLoop) {
    // Remainder loop is represented explicitly in merged CFG and we don't need
    // to repurpose OrigLoop as remainder.
    if (isSearchLoop()) {
      HIRLoopVisitor LV(OrigLoop, this);
      LV.replaceCalls();
    } else {
      HLNodeUtils::remove(OrigLoop);
    }
  }

  // Complete unroll optimization for small trip count vector loops. Also check
  // to see that the loop body is small. If so, do complete unroll of the vector
  // loop. We can only use the original trip count when a peel loop is not
  // needed. A generated peel loop will lead to a non-constant lower bound
  // leading to a crash in the complete unroller.
  bool KnownTripCount = getTripCount() > 0;
  if (!NeedPeelLoop && KnownTripCount && TripCount <= SmallTripThreshold &&
      OrigLoop->isInnermost() && !getTreeConflictsLowered()) {
    HLInstCounter InstCounter;
    HLNodeUtils::visitRange(InstCounter, OrigLoop->child_begin(),
                            OrigLoop->child_end());
    if (InstCounter.getNumInsts() <= SmallLoopBodyThreshold)
      HIRTransformUtils::completeUnroll(MainLoop);
  }

  // Remove the OrigLoop for merged CFG approach or if remainder is not needed.
  if ((!isSearchLoop() && OrigLoop->isAttached()) || !NeedRemainderLoop)
    HLNodeUtils::remove(OrigLoop);
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
// <16>  |   <RVAL-REG> NON-LINEAR <4 x float> %load {sb:15}
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
  if (TLI->isFunctionVectorizable(FnName, ElementCount::getFixed(MaxVF))) {
    // TODO: Call is vectorized in scalar remainder loop, currently ignored
    // since we don't have corresponding VPLoop.
    ++NonLoopInstStats.VectorMathCalls;

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
      auto VecDestTy = getWidenedType(Ref->getDestType(), MaxVF);

      RegDDRef *WideRef = nullptr;
      HLInst *LoadInst = nullptr;

      // Create the scalar load of the call argument. This is done so that
      // we can clone the new LvalDDRef and change its type to force the
      // broadcast. See %load in the example above. The cloned DDREF's source
      // and dest types are set appropriately(<4 x float> in the example above)
      // so that HIRCG knows to do the broadcast.
      if (Ref->isMemRef()) {
        // Ref is a memory reference: %t = sinf(a[i]);
        LoadInst = HLNodeUtilities.createLoad(Ref->clone(), "load");
      } else {
        // Ref in this case is a temp from a previous load: %r = sinf(%t).
        // Create a new temp and broadcast it for the call argument.
        LoadInst = HLNodeUtilities.createCopyInst(Ref->clone(), "copy");
      }

      // Construct the new RegDDRef for the call argument. Set the source and
      // dest types to the vector type required to do a broadcast. So, for
      // example, if source type is float, source/dest types become <4 x float>.
      // This causes HIRCG to do a broadcast when processing this RegDDRef.
      HLNodeUtils::insertBefore(HInst, LoadInst);
      WideRef = LoadInst->getLvalDDRef()->clone();
      auto CE = WideRef->getSingleCanonExpr();

      assert(CE->getSrcType() == CE->getDestType() &&
             "Expected CE src/dest type match");
      CE->setSrcAndDestType(VecDestTy);

      // Collect call arguments and types so that the function declaration
      // and call instruction can be generated.
      CallArgs.push_back(WideRef);
      ArgTys.push_back(VecDestTy);
      ArgAttrs.push_back(Call->getAttributes().getParamAttrs(ArgNum));
    }

    // Using the newly created vector call arguments, generate the vector
    // call instruction and extract the low element.
    Function *VectorF = getOrInsertVectorLibFunction(
        F, MaxVF, ArgTys, TLI, Intrinsic::not_intrinsic, false /*non-masked*/);
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
    setRequiredAttributes(Call->getAttributes(),
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
  if (auto *Descr = HIRLegality->getReductionDescr(Ref)) {
    Opcode = VPReduction::getReductionOpcode(Descr->getKind());
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
                                  bool LaneZeroOnly, bool IsUniform) {
  assert(Ref && "DDRef to be widened should not be null.");

  RegDDRef *WideRef;
  auto RefDestTy = Ref->getDestType();
  auto VecRefDestTy = getWidenedType(RefDestTy, VF);
  auto RefSrcTy = Ref->getSrcType();
  auto VecRefSrcTy = getWidenedType(RefSrcTy, VF);

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
      return nullptr;
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
    propagateMetadata(WideRef, Ref);
    WideRef->setBitCastDestVecOrElemType(VecRefDestTy);

    if (!WideRef->isAddressOf()) {
      setRefAlignment(RefDestTy, WideRef);
    }
  }

  // For unit stride ref, nothing else to do.
  if (isUnitStrideRef(Ref) || LaneZeroOnly)
    return WideRef;

  unsigned NestingLevel = OrigLoop->getNestingLevel();

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
        CA.push_back(ConstantInt::getSigned(Int64Ty, ConstCoeff * i));
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
      BlobTy TopBlob = BlobUtilities.getBlob(BI);

      // We do not need to widen invariant scalar blobs, invariant vector blobs
      // need to be replicated - check for blob invariance by comparing
      // maxbloblevel against the loop's nesting level or using IsUniform flag.
      if (IsUniform || WideRef->findMaxBlobLevel(BI) < NestingLevel) {
        if (TopBlob->getType()->isVectorTy()) {
          Constant *ConstVec = nullptr;
          UndefValue *UndefVec = nullptr;
          if (BlobUtils::isConstantVectorBlob(TopBlob, &ConstVec) ||
              BlobUtils::isUndefBlob(TopBlob, &UndefVec)) {
            // Replicate the constant/undef vector blob VF times.
            if (!ConstVec) {
              assert(UndefVec && "Only ConstVector or UndefValue is expected.");
              // UndefValue is a sub-class of Constant, so it's valid to upcast.
              ConstVec = UndefVec;
            }

            assert(ConstVec &&
                   "ConstantVector value not found for ConstantVector blob.");
            unsigned ReplBlobIdx;
            BlobUtilities.createReplicatedConstantBlob(
                ConstVec, VF, true /*Insert*/, &ReplBlobIdx);
            // Replace the original vector blob with replicated version.
            assert(BlobUtilities.isBlobIndexValid(ReplBlobIdx) &&
                   "Replicated blob does not have valid blob index.");
            CE->replaceBlob(BI, ReplBlobIdx);
          } else {
            assert(WideRef->getSingleCanonExpr()->getSingleBlobIndex() == BI &&
                   "Unexpected RegDDRef/Blob for invariant vector blob");
            // Replicate invariant vector blob VF times.
            HLInst *ReplBlobInst =
                replicateVector(WideRef, VF, NestingLevel - 1);
            // Insert the replicating instruction into vector loop at current
            // insertion point.
            addInstUnmasked(ReplBlobInst);
            auto NewRef = ReplBlobInst->getLvalDDRef();

            AuxRefs.push_back(NewRef);
            CE->replaceBlob(BI,
                            NewRef->getSingleCanonExpr()->getSingleBlobIndex());
          }
        }

        continue;
      }

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

      // A temp blob not widened before is scalarized value - we ignore scalar
      // types since it will be implicitly broadcasted in HIRCG when needed. For
      // vector type blobs we do an explicit replication.
      if (auto *WRef = getWideRef(OldSymbase)) {
        AuxRefs.push_back(WRef);
        CE->replaceBlob(BI, WRef->getSingleCanonExpr()->getSingleBlobIndex());
      } else if (TopBlob->getType()->isVectorTy()) {
        assert(WideRef->getSingleCanonExpr()->getSingleBlobIndex() == BI &&
               "Unexpected RegDDRef/Blob for scalarized vector blob");
        assert(WideRef->getDestType() != VecRefDestTy &&
               "Vector Blob was not scalarized.");
        // Replicate vector blob VF times.
        HLInst *ReplBlobInst = replicateVector(WideRef, VF, NestingLevel);
        // Insert the replicating instruction into vector loop.
        addInstUnmasked(ReplBlobInst);
        auto NewRef = ReplBlobInst->getLvalDDRef();
        AuxRefs.push_back(NewRef);
        CE->replaceBlob(BI, NewRef->getSingleCanonExpr()->getSingleBlobIndex());
      }
    }

    auto VecCEDestTy = getWidenedType(CE->getDestType(), VF);
    auto VecCESrcTy = getWidenedType(CE->getSrcType(), VF);

    CE->setDestType(VecCEDestTy);
    CE->setSrcType(VecCESrcTy);
  }

  // The blobs in the scalar ref have been replaced by widened refs, call
  // the utility to update the widened Ref consistent at the nesting level
  // of insertion point.
  WideRef->makeConsistent(AuxRefs, getNestingLevelFromInsertPoint());
  return WideRef;
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
  case Intrinsic::vector_reduce_fadd:
  case Intrinsic::vector_reduce_fmul:
    assert(Acc && "Expected initial value");
    assert(!isa<VectorType>(Acc->getDestType()) &&
           "Accumulator for FP reduction is not scalar.");
    Tys.insert(Tys.end(), {VecType});
    Ops.insert(Ops.end(), {Acc, VecRef});
    Acc = nullptr;
    break;
  case Intrinsic::vector_reduce_add:
  case Intrinsic::vector_reduce_mul:
  case Intrinsic::vector_reduce_and:
  case Intrinsic::vector_reduce_or:
  case Intrinsic::vector_reduce_xor:
    Tys.insert(Tys.end(), {VecType});
    Ops.insert(Ops.end(), {VecRef});
    break;
  case Intrinsic::vector_reduce_umax:
  case Intrinsic::vector_reduce_smax:
  case Intrinsic::vector_reduce_umin:
  case Intrinsic::vector_reduce_smin:
    assert(!Acc && "Unexpected initial value");
    // Since we use generic IRBuilder::CreateCall interface in HIR, signedness
    // does not need to be explicitly specified.
    Tys.insert(Tys.end(), {VecType});
    Ops.insert(Ops.end(), {VecRef});
    break;
  case Intrinsic::vector_reduce_fmax:
  case Intrinsic::vector_reduce_fmin:
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

  auto PredLHS = HIf->getLHSPredicateOperandDDRef(PredIt);
  auto PredRHS = HIf->getRHSPredicateOperandDDRef(PredIt);
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

template <class MDSource>
void VPOCodeGenHIR::propagateMetadata(RegDDRef *NewRef, const MDSource *SrcMD) {
  // Start out by clearing all non-debug related metadata.
  RegDDRef::MDNodesTy MDs;
  NewRef->getAllMetadataOtherThanDebugLoc(MDs);
  for (auto It : MDs) {
    LLVM_DEBUG(dbgs() << "Cleared metadata kind: " << It.first << "\n");
    NewRef->setMetadata(It.first, nullptr);
  }

  SmallVector<unsigned, 6> PreservedMDKinds = {
      LLVMContext::MD_tbaa,        LLVMContext::MD_alias_scope,
      LLVMContext::MD_noalias,     LLVMContext::MD_fpmath,
      LLVMContext::MD_nontemporal, LLVMContext::MD_invariant_load};

  for (auto Kind : PreservedMDKinds)
    NewRef->setMetadata(Kind, SrcMD->getMetadata(Kind));
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
  auto *VecTy1 = dyn_cast<FixedVectorType>(V1->getDestType());
  auto *VecTy2 = dyn_cast<FixedVectorType>(V2->getDestType());
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

RegDDRef *VPOCodeGenHIR::concatenateVectors(ArrayRef<RegDDRef *> VecsArray,
                                            RegDDRef *Mask) {
  int64_t NumVecs = VecsArray.size();
  assert(NumVecs > 1 && "Should be at least two vectors");

  SmallVector<RegDDRef *, 8> ResList(VecsArray.begin(), VecsArray.end());

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

    std::swap(ResList, TmpList);
    NumVecs = ResList.size();
  } while (NumVecs > 1);

  return ResList[0];
}

HLInst *VPOCodeGenHIR::createShuffleWithUndef(RegDDRef *Input,
                                              ArrayRef<Constant *> Mask,
                                              const StringRef &Name,
                                              RegDDRef *WLvalRef) {
  Constant *MaskVec = ConstantVector::get(Mask);
  RegDDRef *ShufMaskRef = DDRefUtilities.createConstDDRef(MaskVec);
  RegDDRef *UndefRef = DDRefUtilities.createUndefDDRef(Input->getDestType());

  return HLNodeUtilities.createShuffleVectorInst(Input->clone(), UndefRef,
                                                 ShufMaskRef, Name, WLvalRef);
}

HLInst *VPOCodeGenHIR::extendVector(RegDDRef *Input, unsigned TargetLength) {
  Type *OrigTy = Input->getDestType();
  unsigned OrigNumElts = cast<FixedVectorType>(OrigTy)->getNumElements();
  assert(TargetLength > OrigNumElts &&
         "TargetLength should be greater than OrigNumElts.");

  Constant *ShufMaskVec = createSequentialMask(
      0 /*Start*/, OrigNumElts, TargetLength - OrigNumElts /*NumUndefs*/,
      HLNodeUtilities.getContext());
  // TODO: modify createShuffleWithUndef to accept ConstantVector too?
  RegDDRef *ShufMaskRef = DDRefUtilities.createConstDDRef(ShufMaskVec);
  RegDDRef *UndefRef = DDRefUtilities.createUndefDDRef(Input->getDestType());

  return HLNodeUtilities.createShuffleVectorInst(Input->clone(), UndefRef,
                                                 ShufMaskRef, ".extended");
}

HLInst *VPOCodeGenHIR::replicateVector(RegDDRef *Input,
                                       unsigned ReplicationFactor,
                                       unsigned ReplNestingLvl) {
  assert(ReplicationFactor > 1 && "Unexpected replication factor.");
  auto *OrigTy = cast<FixedVectorType>(Input->getDestType());
  unsigned OrigNumElts = OrigTy->getNumElements();

  SmallVector<Constant *, 8> ShuffleMask;
  for (unsigned J = 0; J < ReplicationFactor; ++J) {
    for (unsigned I = 0; I < OrigNumElts; ++I) {
      Constant *Mask = ConstantInt::get(Type::getInt32Ty(Context), I);
      ShuffleMask.push_back(Mask);
    }
  }

  auto *ReplVecInst = createShuffleWithUndef(Input, ShuffleMask, ".replicated");
  LLVM_DEBUG(dbgs() << "[VPOCGHIR] ReplVecInst: "; ReplVecInst->dump();
             dbgs() << "\n");

  // Make the replicated sub-vector operand consistent based on nesting level
  // where ReplVecInst will be attached.
  RegDDRef *ReplOperand = ReplVecInst->getOperandDDRef(1);
  ReplOperand->makeConsistent({}, ReplNestingLvl);

  return ReplVecInst;
}

HLInst *VPOCodeGenHIR::replicateVectorElts(RegDDRef *Input,
                                           unsigned ReplicationFactor) {
  assert(ReplicationFactor > 1 && "Unexpected replication factor.");
  auto *OrigTy = cast<FixedVectorType>(Input->getDestType());
  unsigned OrigNumElts = OrigTy->getNumElements();

  SmallVector<Constant *, 8> ShuffleMask;
  for (unsigned J = 0; J < OrigNumElts; ++J) {
    for (unsigned I = 0; I < ReplicationFactor; ++I) {
      Constant *Mask = ConstantInt::get(Type::getInt32Ty(Context), J);
      ShuffleMask.push_back(Mask);
    }
  }

  auto *ReplVecInst =
      createShuffleWithUndef(Input, ShuffleMask, ".replicated.elts");
  LLVM_DEBUG(dbgs() << "[VPOCGHIR] ReplVecInst: "; ReplVecInst->dump();
             dbgs() << "\n");

  return ReplVecInst;
}

RegDDRef *VPOCodeGenHIR::extractSubVector(RegDDRef *Input, unsigned Part,
                                          unsigned NumParts,
                                          RegDDRef *LValRef) {
  if (!Input)
    return nullptr; // No vector to extract from.

  assert(NumParts > 0 && "Invalid number of subparts of vector.");

  if (NumParts == 1) {
    // Return the original vector as there is only one Part.
    return Input;
  }

  unsigned VecLen =
      cast<FixedVectorType>(Input->getDestType())->getNumElements();
  assert(VecLen % NumParts == 0 &&
         "Vector cannot be divided into unequal parts for extraction.");
  assert(Part < NumParts && "Invalid subpart to be extracted from vector.");

  unsigned SubVecLen = VecLen / NumParts;
  SmallVector<Constant *, 4> ShuffleMask;

  unsigned ElemIdx = Part * SubVecLen;

  for (unsigned K = 0; K < SubVecLen; K++) {
    Constant *Mask = ConstantInt::get(Type::getInt32Ty(Context), ElemIdx + K);
    ShuffleMask.push_back(Mask);
  }

  auto *SubVec =
      createShuffleWithUndef(Input, ShuffleMask, ".extracted.subvec", LValRef);
  LLVM_DEBUG(dbgs() << "[VPOCGHIR] ExtractSubVec: "; SubVec->dump();
             dbgs() << "\n");

  // Attach sub-vector instruction to HIR.
  addInstUnmasked(SubVec);
  return SubVec->getLvalDDRef()->clone();
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

  assert(getForceMixedCG() &&
         "Unexpected to be called in VPValue based CG mode");
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
        SearchLoopType == VPlanIdioms::SearchLoopPtrEq /*NonZeroMask*/);
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
      RegDDRef *VecMemRef = widenRef(MemRef, getVF(), IsUnitStride /* LaneZeroOnly */);
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

void VPOCodeGenHIR::widenVectorVariant(const VPCallInstruction *VPCall,
                                       RegDDRef *Mask) {
  assert(VPCall->getVectorizationScenario() ==
             VPCallInstruction::CallVecScenariosTy::VectorVariant &&
         "widenVectorVariant called for mismatched scenario.");

  // TODO: handle indirect calls - not yet supported for HIR since we bail
  // out in HandledCheck::visit()

  Function *CalledFunc = VPCall->getCalledFunction();
  assert(CalledFunc && "Unexpected null called function.");
  (void)CalledFunc;

  const VFInfo *MatchedVariant = VPCall->getVectorVariant();
  assert(MatchedVariant && "Unexpected null matched vector variant");

  // Create all-ones mask for masked vector variant when unmasked variant isn't
  // available.
  if (!Mask && VPCall->shouldUseMaskedVariantForUnmasked()) {
    RegDDRef *ConstOne =
        DDRefUtilities.createConstOneDDRef(Type::getInt1Ty(Context));
    Mask = widenRef(ConstOne, getVF());
  }

  unsigned PumpFactor = VPCall->getPumpFactor();
  SmallVector<HLInst *, 4> CallResults;
  generateWideCalls(VPCall, PumpFactor, Mask, MatchedVariant,
                    Intrinsic::not_intrinsic /*No vector intrinsic*/,
                    CallResults);
  // We assert in widenCallArgs for PumpFactor == 1, so CallResults will
  // have only 1 HLInst. Remove this comment after support for pumping
  // is added.
  HLInst *CallResult = CallResults[0];

  // Update wide-ref mapping.
  addVPValueWideRefMapping(VPCall, CallResult->getLvalDDRef());
}

void VPOCodeGenHIR::widenLibraryCall(const VPCallInstruction *VPCall,
                                     RegDDRef *Mask) {
  assert(VPCall->getVectorizationScenario() ==
             VPCallInstruction::CallVecScenariosTy::LibraryFunc &&
         "widenLibraryCall called for mismatched scenario.");
  Function *CalledFunc = VPCall->getCalledFunction();
  assert(CalledFunc && "Unexpected null called function.");
  unsigned PumpFactor = VPCall->getPumpFactor();

  SmallVector<HLInst *, 4> CallResults;
  generateWideCalls(VPCall, PumpFactor, Mask,
                    nullptr /*No vector variant*/,
                    Intrinsic::not_intrinsic /*No vector intrinsic*/,
                    CallResults);

  // Set calling conventions for SVML function calls
  for (auto *WideCall : CallResults) {
    assert(WideCall->isCallInst() && "Widened call instruction expected.");
    CallInst *WideLLVMCall = const_cast<CallInst *>(WideCall->getCallInst());
    Function *CalledLLVMFunc = WideLLVMCall->getCalledFunction();
    assert(CalledLLVMFunc && "Called function expected for the call");
    if (isSVMLFunction(TLI, CalledFunc->getName(), CalledLLVMFunc->getName())) {
      WideLLVMCall->setCallingConv(CallingConv::SVML);
    }
  }

  // Post process generated vector calls.
  HLInst *CombinedResult = nullptr;
  Type *ReturnTy = CallResults[0]->getLvalDDRef()->getDestType();
  if (PumpFactor > 1 && ReturnTy->isStructTy())
    CombinedResult = getCombinedCallResultsForStructTy(CallResults);
  else
    CombinedResult = getCombinedCallResults(CallResults, Mask);

  assert(CombinedResult && "Unexpected null combined result.");
  assert(CombinedResult->isAttached() &&
         "Combined result for calls is expected to be attached.");

  // Handle results of SVML sincos function calls
  // sincos function has two return values. The scalar sincos function uses
  // pointers as out-parameters. SVML sincos function, instead, returns them in
  // a struct directly. This bridges the gap between these two approaches.
  const class CallInst *Call = CallResults[0]->getCallInst();
  assert(Call && Call->getCalledFunction() &&
         "Unexpected null CalledFunction.");
  if (Call->getCalledFunction()->getName().startswith("__svml_sincos")) {
    // TODO: sincos handling uses underlying HLInst since it's designed to work
    // even for scalar remainder loop (replaceLibCallsInRemainderLoop).
    auto *UnderlyingHLInst = cast<HLInst>(VPCall->HIR().getUnderlyingNode());
    generateStoreForSinCos(UnderlyingHLInst, CombinedResult, Mask,
                           false /* IsRemainderLoop */);
    return;
  }

  // Update wide-ref mapping.
  addVPValueWideRefMapping(VPCall, CombinedResult->getLvalDDRef());
}

void VPOCodeGenHIR::widenTrivialIntrinsic(const VPCallInstruction *VPCall) {
  assert(VPCall->getVectorizationScenario() ==
             VPCallInstruction::CallVecScenariosTy::TrivialVectorIntrinsic &&
         "widenTrivialIntrinsic called for mismatched scenario.");
  unsigned PumpFactor = VPCall->getPumpFactor();
  Intrinsic::ID VectorIntrinID = VPCall->getVectorIntrinsic();
  assert(VectorIntrinID != Intrinsic::not_intrinsic &&
         "Unexpected non-intrinsic call.");

  // Trivial vector intrinsics are always unmasked.
  SmallVector<HLInst *, 4> CallResults;
  assert(PumpFactor == 1 &&
         "Pumping feature is not expected for trivial vector intrinsics.");
  generateWideCalls(VPCall, PumpFactor, nullptr /*Mask*/,
                    nullptr /*No vector variant*/, VectorIntrinID, CallResults);
  assert(CallResults.size() == 1 &&
         "Expected single widened call for trivial vector intrinsic.");
  HLInst *CallResult = CallResults[0];

  // Update wide-ref mapping.
  addVPValueWideRefMapping(VPCall, CallResult->getLvalDDRef());
}

RegDDRef* VPOCodeGenHIR::generateMaskArg(RegDDRef *Mask,
                                         const VFInfo *MatchedVariant,
                                         const VPCallInstruction *VPCall) {
  RegDDRef *NewMask = Mask;
  // For vector variants, use the characteristic type instead of i1
  // for the mask argument.
  if (MatchedVariant && !Usei1MaskForSimdFunctions) {
    auto *MaskTy = cast<VectorType>(Mask->getDestType());
    auto *CharacteristicTy =
        VPlanCallVecDecisions::calcCharacteristicType(
            const_cast<VPCallInstruction*>(VPCall), *MatchedVariant);
    // Promote i1 to an integer with same size as the characteristic type.
    VectorType *MaskTyExt =
        VectorType::get(
            IntegerType::get(Context,
                             CharacteristicTy->getPrimitiveSizeInBits()),
            MaskTy->getElementCount());

    HLInst *MaskValueExt =
        HLNodeUtilities.createSExt(MaskTyExt,
        Mask->clone());
    addInstUnmasked(MaskValueExt);
    NewMask = MaskValueExt->getLvalDDRef();

    auto *CharacteristicVecTy =
        VectorType::get(CharacteristicTy,
        MaskTy->getElementCount());

    // Bitcast to the characteristic type.
    if (MaskTyExt != CharacteristicVecTy) {
      auto *ConvertInst =
          HLNodeUtilities.createCastHLInst(CharacteristicVecTy,
          Instruction::BitCast,
          MaskValueExt->getLvalDDRef()->clone());
      addInstUnmasked(ConvertInst);
      NewMask = ConvertInst->getLvalDDRef();
    }
  }
  return NewMask;
}

void VPOCodeGenHIR::widenCallArgs(const VPCallInstruction *VPCall,
                                  RegDDRef *Mask, Intrinsic::ID VectorIntrinID,
                                  const VFInfo *MatchedVariant,
                                  unsigned PumpPart, unsigned PumpFactor,
                                  SmallVectorImpl<RegDDRef *> &CallArgs,
                                  SmallVectorImpl<Type *> &ArgTys,
                                  SmallVectorImpl<AttributeSet> &ArgAttrs) {
  unsigned PumpedVF = getVF() / PumpFactor;
  ArrayRef<VFParameter> Parms;
  if (MatchedVariant) {
    Parms = MatchedVariant->getParameters();
  }

  Function *Fn = VPCall->getCalledFunction();
  assert(Fn && "Unexpected null called function");
  StringRef FnName = Fn->getName();

  // Widen all arg operands of the call and adjust them based on masking.
  unsigned ArgIgnored = 0;
  // glibc scalar sincos function has 2 pointer out parameters, but SVML sincos
  // functions return the results directly in a struct. The pointers should be
  // omitted in vectorized call.
  if (FnName == "sincos" || FnName == "sincosf")
    ArgIgnored = 2;

  AttributeList Attrs = VPCall->getOrigCallAttrs();

  for (unsigned I = 0; I < VPCall->getNumArgOperands() - ArgIgnored; I++) {
    RegDDRef *WideArg = nullptr;
    if ((!MatchedVariant || Parms[I].isVector()) &&
        !isVectorIntrinsicWithScalarOpAtArg(VectorIntrinID, I)) {
      WideArg = widenRef(VPCall->getOperand(I), VF);
      WideArg = extractSubVector(WideArg, PumpPart, PumpFactor);
      assert(WideArg && "Vectorized call arg cannot be nullptr.");
    } else {
      // TODO: support pumping for vector variants
      assert(PumpFactor == 1 &&
             "Pumping feature is not expected.");
      WideArg = getOrCreateScalarRef(VPCall->getOperand(I), 0 /*Lane*/);
    }
    CallArgs.push_back(WideArg);
    ArgTys.push_back(WideArg->getDestType());
    ArgAttrs.push_back(Attrs.getParamAttrs(I));
  }

  bool Masked = Mask != nullptr;
  if (Masked) {
    // Masked intrinsics will not have explicit mask parameter. They are handled
    // like other BinOp HLInsts i.e. execute on all lanes and extract active
    // lanes during HIR-CG.
    assert(VectorIntrinID == Intrinsic::not_intrinsic &&
           "Vectorization of trivial intrinsics is not expected to be masked.");
    // Compute mask paramter for current part being pumped.
    RegDDRef *PumpPartMask = extractSubVector(Mask, PumpPart, PumpFactor);
    StringRef VecFuncName = TLI->getVectorizedFunction(
        Fn->getName(), ElementCount::getFixed(PumpedVF), Masked);
    // Masks of SVML function calls need special treatment, it's different from
    // the normal case for AVX512.
    if (!VecFuncName.empty() &&
        isSVMLFunction(TLI, Fn->getName(), VecFuncName)) {
      addMaskToSVMLCall(Fn, Attrs, CallArgs, ArgTys, ArgAttrs, PumpPartMask);
    } else {
      RegDDRef *MaskArg = generateMaskArg(PumpPartMask, MatchedVariant, VPCall);
      auto CE = MaskArg->getSingleCanonExpr();
      ArgTys.push_back(CE->getDestType());
      CallArgs.push_back(MaskArg->clone());
      ArgAttrs.push_back(AttributeSet());
    }
  }
}

HLInst *VPOCodeGenHIR::getCombinedCallResults(ArrayRef<HLInst *> CallResults,
                                              RegDDRef *Mask) {
  if (CallResults.size() == 1)
    return CallResults[0];

  assert(CallResults.size() >= 2 && isPowerOf2_32(CallResults.size()) &&
         "Number of pumped vector calls to combine must be a power of 2.");
  Type *RetTy = CallResults[0]->getLvalDDRef()->getDestType();
  if (RetTy->isVectorTy()) {
    SmallVector<RegDDRef *, 4> Lvals;
    for (auto *Call : CallResults)
      Lvals.push_back(Call->getLvalDDRef());
    RegDDRef *Combined = concatenateVectors(Lvals, Mask);
    assert(Combined->isLval() &&
           "Expected l-val as return value from concatenateVectors.");
    return cast<HLInst>(Combined->getHLDDNode());
  } else {
    llvm_unreachable("Expect vector result from vector calls.");
  }
}

HLInst *VPOCodeGenHIR::getCombinedCallResultsForStructTy(
    ArrayRef<HLInst *> CallResults) {
  assert(
      llvm::all_of(CallResults,
                   [](HLInst *Call) {
                     return Call->getLvalDDRef()->getDestType()->isStructTy();
                   }) &&
      "Calls returning StructTy expected here.");

  // If the return type is not a vector, then it must be a struct of vectors
  // (returned by sincos function). Create a widened struct type by widening
  // every element type.
  // For example, if CallResults[0] is { <2 x float>, <2 x float> } and
  // PumpFactor is 2, the combined type will be
  // { <4 x float>, <4 x float> }.
  auto *ReturnTy = CallResults[0]->getLvalDDRef()->getDestType();
  StructType *ReturnStructType = cast<StructType>(ReturnTy);
  SmallVector<Type *, 2> ElementTypes;
  for (unsigned I = 0; I < ReturnStructType->getStructNumElements(); I++) {
    VectorType *ElementType =
        cast<VectorType>(ReturnStructType->getStructElementType(I));
    ElementTypes.push_back(
        VectorType::get(ElementType->getElementType(),
                        ElementType->getElementCount() * CallResults.size()));
  }
  StructType *CombinedReturnType =
      StructType::get(ReturnTy->getContext(), ElementTypes);

  // Then combine the pumped call results: for each vector element of
  // struct, extract the result from the return value of pumped calls,
  // combine them and insert to the combined struct:
  //
  // ; extract and combine sin field of the return values
  // %sin0 = extractvalue { <2 x float>, <2 x float> } %callResults0, 0
  // %sin1 = extractvalue { <2 x float>, <2 x float> } %callResults1, 0
  // %sin.combined = shufflevector <2 x float> %sin0, <2 x float> %sin1,
  //                               <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  // %result.tmp = insertvalue { <4 x float>, <4 x float> } undef,
  //                           <4 x float> %sin.combined, 0
  //
  // ; extract and combine cos field of the return values
  // %cos0 = extractvalue { <2 x float>, <2 x float> } %callResults0, 1
  // %cos1 = extractvalue { <2 x float>, <2 x float> } %callResults1, 1
  // %cos.combined = shufflevector <2 x float> %cos0, <2 x float> %cos1,
  //                               <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  // %result = insertvalue { <4 x float>, <4 x float> } %result.tmp,
  //                       <4 x float> %cos.combined, 1
  RegDDRef *Combined = DDRefUtilities.createUndefDDRef(CombinedReturnType);
  for (unsigned I = 0; I < CombinedReturnType->getNumElements(); I++) {
    SmallVector<RegDDRef *, 4> Parts;
    for (unsigned J = 0; J < CallResults.size(); J++) {
      HLInst *ExtractVal = HLNodeUtilities.createExtractValueInst(
          CallResults[J]->getLvalDDRef()->clone(), I, "extract.result");
      addInstUnmasked(ExtractVal);
      Parts.push_back(ExtractVal->getLvalDDRef());
    }

    RegDDRef *CombinedVector = concatenateVectors(Parts, nullptr /*Mask*/);
    HLInst *InsertVal = HLNodeUtilities.createInsertValueInst(
        Combined->clone(), CombinedVector->clone(), I, "insert.result");
    addInstUnmasked(InsertVal);
    Combined = InsertVal->getLvalDDRef();
  }

  return cast<HLInst>(Combined->getHLDDNode());
}

void VPOCodeGenHIR::generateWideCalls(const VPCallInstruction *VPCall,
                                      unsigned PumpFactor, RegDDRef *Mask,
                                      const VFInfo *MatchedVariant,
                                      Intrinsic::ID VectorIntrinID,
                                      SmallVectorImpl<HLInst *> &CallResults) {
  Function *Fn = VPCall->getCalledFunction();
  assert(Fn && "Unexpected null called function");

  LLVM_DEBUG(dbgs() << "[VPOCGHIR] Function " << Fn->getName() << " is pumped "
                    << PumpFactor << "-way.\n");

  for (unsigned PumpPart = 0; PumpPart < PumpFactor; ++PumpPart) {
    LLVM_DEBUG(dbgs() << "[VPOCGHIR] Pumping part " << PumpPart << "/"
                      << PumpFactor << "\n");

    SmallVector<RegDDRef *, 4> CallArgs;
    SmallVector<Type *, 1> ArgTys;
    SmallVector<AttributeSet, 1> ArgAttrs;
    widenCallArgs(VPCall, Mask, VectorIntrinID, MatchedVariant, PumpPart,
                  PumpFactor, CallArgs, ArgTys, ArgAttrs);

    bool Masked = Mask != nullptr;
    Function *VectorF = nullptr;
    if (MatchedVariant) {
      VectorF = getOrInsertVectorVariantFunction(
        Fn, VF / PumpFactor, ArgTys, MatchedVariant, Masked);
    } else {
      VectorF = getOrInsertVectorLibFunction(
        Fn, VF / PumpFactor, ArgTys, TLI, VectorIntrinID, Masked);
    }
    assert(VectorF && "Can't create vector function.");

    FastMathFlags FMF = VPCall->hasFastMathFlags() ? VPCall->getFastMathFlags()
                                                   : FastMathFlags();
    auto *WideInst = HLNodeUtilities.createCall(
        VectorF, CallArgs, VectorF->getName(), nullptr /*Lval*/, {} /*Bundle*/,
        {} /*BundleOps*/, FMF);
    CallInst *VecCall = const_cast<CallInst *>(WideInst->getCallInst());
    assert(VecCall && "Call instruction is expected to be exist");

    // Make sure we don't lose attributes at the call site. E.g., IMF
    // attributes are taken from call sites in MapIntrinToIml to refine
    // SVML calls for precision.
    setRequiredAttributes(VPCall->getOrigCallAttrs(), VecCall, ArgAttrs);

    // Attach generated vector call. Mask is ignored here since they are
    // explicitly passed as operand to call.
    addInstUnmasked(WideInst);
    CallResults.push_back(WideInst);
  }
}

HLInst *VPOCodeGenHIR::generateScalarCall(const VPCallInstruction *VPCall,
                                          unsigned LaneID) {
  // Populate scalar arguments of call for given vector lane.
  SmallVector<RegDDRef *, 4> ScalarArgs;
  for (unsigned I = 0; I < VPCall->getNumArgOperands(); ++I) {
    RegDDRef *ScalarArg = getOrCreateScalarRef(VPCall->getOperand(I), LaneID);
    assert(ScalarArg && "Unexpected null scalar arg for call.");
    ScalarArgs.push_back(ScalarArg);
  }

  Function *ScalarF = VPCall->getCalledFunction();
  assert(ScalarF && "Expected only direct calls.");
  FastMathFlags FMF =
      VPCall->hasFastMathFlags() ? VPCall->getFastMathFlags() : FastMathFlags();
  SmallVector<OperandBundleDef, 4> Bundles;
  SmallVector<RegDDRef *, 4> BundleOps;
  if (ScalarF->getIntrinsicID() == Intrinsic::assume) {
    const CallInst *UnderlyingCI = VPCall->getUnderlyingCallInst();
    assert(UnderlyingCI && "Underlying call instruction expected here");

    UnderlyingCI->getOperandBundlesAsDefs(Bundles);
    unsigned NumOpBundles = UnderlyingCI->getNumOperandBundles();
    unsigned NumNonBundleOps = UnderlyingCI->arg_size();

    for (unsigned i = 0, OpBegin = NumNonBundleOps, OpEnd = 0; i < NumOpBundles;
         ++i, OpBegin = OpEnd) {
      unsigned NumCurrBundleOps =
          UnderlyingCI->getOperandBundleAt(i).Inputs.size();
      OpEnd = OpBegin + NumCurrBundleOps;
      for (unsigned j = OpBegin; j < OpEnd; ++j)
        BundleOps.push_back(ScalarArgs[j]);
    }
    ScalarArgs.resize(NumNonBundleOps);
  }
  auto *ScalarCall = HLNodeUtilities.createCall(
      ScalarF, ScalarArgs, ScalarF->getName(), nullptr /*Lval*/,
      Bundles /*Bundle*/, BundleOps /*BundleOps*/, FMF);
  CallInst *LLVMCall = const_cast<CallInst *>(ScalarCall->getCallInst());
  LLVMCall->setCallingConv(VPCall->getOrigCallingConv());
  LLVMCall->setAttributes(VPCall->getOrigCallAttrs());

  return ScalarCall;
}

void VPOCodeGenHIR::widenNodeImpl(const HLInst *INode, RegDDRef *Mask,
                                  const VPInstruction *VPInst) {
  auto CurInst = INode->getLLVMInstruction();
  SmallVector<RegDDRef *, 6> WideOps;
  SmallVector<RegDDRef *, 1> CallArgs;

  if (!Mask)
    Mask = CurMaskValue;

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
        SearchLoopType == VPlanIdioms::SearchLoopPtrEq /*NonZeroMask*/);
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
        getWidenedType(CurInst->getType(), VF), CurInst->getOpcode(),
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
    assert(isIgnoredCall(INode->getCallInst()) &&
           "Only ignored calls are handled in mixed CG.");
    return;
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
    return;
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

#if INTEL_FEATURE_SW_DTRANS
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
    RegDDRef *PaddedMalloc = DDRefUtilities.createMemRef(PaddedMallocVariable->getValueType(), PaddedMallocAddrIdx);
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
#endif // INTEL_FEATURE_SW_DTRANS
}

RegDDRef *VPOCodeGenHIR::getUniformScalarRef(const VPValue *VPVal) {
  // Check if VPValue already has scalar Ref for lane 0. Uniform instructions
  // that are scalarized are expected to be handled here.
  RegDDRef *ScalarRef = nullptr;

  if (auto *LiveOut = dyn_cast<VPLiveOutValue>(VPVal))
    return getUniformScalarRef(LiveOut->getOperand(0));

  if ((ScalarRef = getScalRefForVPVal(VPVal, 0)))
    return ScalarRef->clone();

  assert((isa<VPExternalDef>(VPVal) || isa<VPConstant>(VPVal) ||
          isa<VPExternalUse>(VPVal) || isa<VPMetadataAsValue>(VPVal)) &&
         "Expected a VPExternalDef/VPConstant/VPExternalUse/VPMetadataAsValue");

  auto GetScalarRefForExternal = [this](const VPOperandHIR *HIROperand,
                                        Type *VPValTy) -> RegDDRef * {
    RegDDRef *ScalarRef = nullptr;

    if (const auto *Blob = dyn_cast<VPBlob>(HIROperand)) {
      const auto *BlobRef = Blob->getBlob();
      if (BlobRef->isSelfBlob()) {
        ScalarRef = DDRefUtilities.createSelfBlobRef(
            BlobRef->getSelfBlobIndex(), BlobRef->getDefinedAtLevel());
        ScalarRef->makeConsistent({}, getNestingLevelFromInsertPoint());
      } else {
        unsigned BlobIndex = Blob->getBlobIndex();
        // If a valid blob index was not recorded for temp, then try to find the
        // index using its symbase. For example -
        // %t1 = %t2 (in preheader)
        // Here %t1 is not a self blob and VPExternalDef for it is created via
        // its symbase.
        if (BlobIndex == loopopt::InvalidBlobIndex)
          BlobIndex =
              BlobUtilities.findOrInsertTempBlobIndex(BlobRef->getSymbase());
        const RegDDRef *RDDR = cast<RegDDRef>(BlobRef);

        // Create a RegDDREF containing blob with index BlobIndex.
        auto *CE = CanonExprUtilities.createCanonExpr(VPValTy);
        CE->addBlob(BlobIndex, 1);
        ScalarRef = DDRefUtilities.createScalarRegDDRef(GenericRvalSymbase, CE);

        // Use RDDR to set proper defined at level for blob in WideRef.
        SmallVector<const RegDDRef *, 1> AuxRefs = {RDDR};
        ScalarRef->makeConsistent(AuxRefs, getNestingLevelFromInsertPoint());
      }
    } else if (const auto *VPCE = dyn_cast<VPCanonExpr>(HIROperand)) {
      auto *CE = VPCE->getCanonExpr()->clone();
      auto *DDR = VPCE->getDDR();
      ScalarRef = DDRefUtilities.createScalarRegDDRef(GenericRvalSymbase, CE);
      SmallVector<const RegDDRef *, 1> AuxRefs = {DDR};
      ScalarRef->makeConsistent(AuxRefs, getNestingLevelFromInsertPoint());
    } else if (const auto *If = dyn_cast<VPIfCond>(HIROperand)) {
      HLInst *Res = nullptr;
      const HLIf *HIf = If->getIf();
      for (auto PredIt = HIf->pred_begin(), End = HIf->pred_end();
           PredIt != End; ++PredIt) {
        auto *LHS = HIf->getLHSPredicateOperandDDRef(PredIt)->clone();
        auto *RHS = HIf->getRHSPredicateOperandDDRef(PredIt)->clone();
        auto *Cmp = HLNodeUtilities.createCmp(*PredIt, LHS, RHS);

        // Make LHS and RHS operands consistent at attached loop level.
        LHS->makeConsistent({LHS->clone()}, getNestingLevelFromInsertPoint());
        RHS->makeConsistent({RHS->clone()}, getNestingLevelFromInsertPoint());

        addInstUnmasked(Cmp);
        if (Res) {
          Res = HLNodeUtilities.createAnd(Res->getLvalDDRef()->clone(),
                                          Cmp->getLvalDDRef()->clone());
          addInstUnmasked(Res);
        } else {
          Res = Cmp;
        }
        ScalarRef = Res->getLvalDDRef()->clone();
      }
    } else {
      const auto *IV = cast<VPIndVar>(HIROperand);
      auto IVLevel = IV->getIVLevel();

      auto *CE = CanonExprUtilities.createCanonExpr(VPValTy);
      CE->addIV(IVLevel, loopopt::InvalidBlobIndex, 1);
      ScalarRef = DDRefUtilities.createScalarRegDDRef(GenericRvalSymbase, CE);
    }

    return ScalarRef;
  };

  if (auto *ExtDef = dyn_cast<VPExternalDef>(VPVal)) {
    ScalarRef =
        GetScalarRefForExternal(ExtDef->getOperandHIR(), ExtDef->getType());
    // HACK: Don't add to the mapping as emitting this at the insertion point
    // might not be dominating all the uses.
    if (isa<VPIfCond>(ExtDef->getOperandHIR()))
      return ScalarRef->clone();
  } else if (auto *VPExtUse = dyn_cast<VPExternalUse>(VPVal)) {
    ScalarRef =
        GetScalarRefForExternal(VPExtUse->getOperandHIR(), VPExtUse->getType());
  } else if (auto *VPMDAsVal = dyn_cast<VPMetadataAsValue>(VPVal)) {
    ScalarRef =
        DDRefUtilities.createConstDDRef(VPMDAsVal->getMetadataAsValue());
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

  // TODO: Probable improvement is to "inline" liveouts after CFG merge. Then we
  // don't need this code. Same for getUniformScalarRef.
  if (auto *LiveOut = dyn_cast<VPLiveOutValue>(VPVal))
    return widenRef(LiveOut->getOperand(0), VF);

  // If the DDREF has a widened counterpart, return the same.
  if ((WideRef = getWideRefForVPVal(VPVal)))
    return WideRef->clone();

  // VPVal is expected to be a VPExternalDef or VPConstant.
  WideRef = getUniformScalarRef(VPVal);
  WideRef = widenRef(WideRef, getVF(), false /* LaneZeroOnly */,
                     true /* IsUniform */);
  assert(WideRef && "Expected non-null widened ref");
  LLVM_DEBUG(WideRef->dump(true));
  LLVM_DEBUG(errs() << "\n");
  // Vector type VPExternalDef are explicitly replicated and inlined at use
  // points. Don't cache the wideref in maps.
  if (!isa<VPExternalDef>(VPVal) || !VPVal->getType()->isVectorTy())
    addVPValueWideRefMapping(VPVal, WideRef);

  // Keep the VPValue for dropping when we switch VF. TODO: Move it to
  // addVPValueWideRefMapping instead?
  if (!isa<VPInstruction>(VPVal)) {
    auto &Vec = VPValsToFlushForVF[VPVal];
    Vec.push_back(std::make_pair(WideRef, nullptr));
  }

  // Clients can potentially modify the returned value. Return the cloned value.
  return WideRef->clone();
}

RegDDRef *VPOCodeGenHIR::createMemrefFromBlob(RegDDRef *PtrRef,
                                              Type *ElementType, int Index,
                                              unsigned IdxBcastFactor) {
  assert((PtrRef->isSelfBlob() || PtrRef->isStandAloneUndefBlob()) &&
         "Expected self blob DDRef or undef");
  auto &HIRF = HLNodeUtilities.getHIRFramework();
  llvm::Triple TargetTriple(HIRF.getModule().getTargetTriple());
  auto Is64Bit = TargetTriple.isArch64Bit();
  unsigned BlobIndex = PtrRef->isSelfBlob()
                           ? PtrRef->getSelfBlobIndex()
                           : PtrRef->getSingleCanonExpr()->getSingleBlobIndex();
  RegDDRef *MemRef = DDRefUtilities.createMemRef(ElementType, BlobIndex,
                                                 PtrRef->getDefinedAtLevel());
  auto Int32Ty = Type::getInt32Ty(HLNodeUtilities.getContext());
  auto Int64Ty = Type::getInt64Ty(HLNodeUtilities.getContext());
  auto IndexCE =
      CanonExprUtilities.createCanonExpr(Is64Bit ? Int64Ty : Int32Ty);
  IndexCE->addConstant(Index, true /* IsMathAdd */);
  if (IdxBcastFactor > 1)
    IndexCE->setSrcAndDestType(
        getWidenedType(IndexCE->getSrcType(), IdxBcastFactor));
  MemRef->addDimension(IndexCE);
  return MemRef;
}

RegDDRef *
VPOCodeGenHIR::getWidenedAddressForScatterGather(const VPValue *VPPtr,
                                                 Type *ScalarAccessType) {
  assert(VPPtr->getType()->isPointerTy() && "Expected VPPtr to be PointerTy.");

  // Widened pointer.
  RegDDRef *WidePtr = widenRef(VPPtr, getVF());

  auto *PtrType = cast<PointerType>(VPPtr->getType());
  auto *VecType = dyn_cast<FixedVectorType>(ScalarAccessType);
  // No replication is needed for non-vector types.
  if (!VecType)
    return WidePtr;

  LLVM_DEBUG(dbgs() << "[VPOCGHIR] WidePtr for replication : ";
             WidePtr->dump(1); dbgs() << "\n");
  unsigned AddrSpace = PtrType->getAddressSpace();

  // Cast the inner vector-type to it's elemental scalar type.
  // e.g. - <VF x <OriginalVL x Ty> addrspace(x)*>
  //                          |
  //                          |
  //                          V
  //                <VF x Ty addrspace(x)*>
  Type *FlattenedTy = getWidenedType(
      VecType->getElementType()->getPointerTo(AddrSpace), getVF());
  WidePtr->setBitCastDestVecOrElemType(FlattenedTy);
  LLVM_DEBUG(dbgs() << "[VPOCGHIR] WidePtr after flattening : ";
             WidePtr->dump(1); dbgs() << "\n");

  // Replicate the base-address OriginalVL times
  //                <VF x Ty addrspace(x)*>
  //                          |
  //                          |
  //                          V
  //      < 0, 0, .., OriginalVL times, 1, 1, ..., OriginalVL times, ...>
  unsigned OriginalVL = VecType->getNumElements();
  HLInst *ReplWidePtrInst = replicateVectorElts(WidePtr, OriginalVL);
  addInstUnmasked(ReplWidePtrInst);

  // Create a vector of consecutive numbers from zero to OriginalVL-1 repeated
  // VF-times.
  SmallVector<Constant *, 32> Indices;
  for (unsigned J = 0; J < VF; ++J)
    for (unsigned I = 0; I < OriginalVL; ++I) {
      Indices.push_back(
          ConstantInt::get(Type::getInt64Ty(VecType->getContext()), I));
    }

  // Generate a ConstantVector of indices and build a corresponding CanonExpr.
  auto *Cv = ConstantVector::get(Indices);
  CanonExpr *IndexCE =
      CanonExprUtilities.createConstStandAloneBlobCanonExpr(Cv);

  // Create an address-of DDRef that would return the address of each elements
  // that is to be accessed. Generated address-of ref looks like -
  //
  // &(%ReplWidePtr)[< 0, 1, ..., OriginalVL - 1, ... VF times>]
  RegDDRef *ReplWidePtr = ReplWidePtrInst->getLvalDDRef();
  RegDDRef *ReplWidePtrAddr = DDRefUtilities.createAddressOfRef(VecType->getElementType(),
      ReplWidePtr->getSelfBlobIndex(), ReplWidePtr->getDefinedAtLevel());
  ReplWidePtrAddr->setInBounds(WidePtr->isInBounds());
  ReplWidePtrAddr->addDimension(IndexCE);
  LLVM_DEBUG(dbgs() << "[VPOCGHIR] ReplWidePtrAddr : ";
             ReplWidePtrAddr->dump(1); dbgs() << "\n");

  return ReplWidePtrAddr;
}

RegDDRef *VPOCodeGenHIR::getMemoryRef(const VPLoadStoreInst *VPLdSt,
                                      bool Lane0Value) {
  const VPValue *VPPtr = VPLdSt->getPointerOperand();
  bool IsNegOneStride;
  bool IsUnitStride = Plan->getVPlanDA()->isUnitStridePtr(
      VPPtr, VPLdSt->getValueType(), IsNegOneStride);
  bool NeedScalarRef = IsUnitStride || Lane0Value;

  Type *IndexedElementType = VPLdSt->getValueType();
  if (!NeedScalarRef)
    // Re-vectorizing vector load/store into a gather/scatter changes the
    // element type.
    IndexedElementType = IndexedElementType->getScalarType();

  unsigned ScalSymbase = VPLdSt->HIR().getSymbase();
  if (auto *Priv = getVPValuePrivateMemoryPtr(VPPtr)) {
    // For accesses to private memory we need to use new private alloca's
    // symbase.
    auto *PrivAlloca = cast<VPAllocatePrivate>(Priv);
    assert(PrivateMemBlobRefs.count(PrivAlloca) &&
           "Private memory pointer not widened.");
    ScalSymbase = PrivateMemBlobRefs[PrivAlloca].second;
  }
  Align Alignment = VPLdSt->getAlignment();
  AAMDNodes AANodes;
  VPLdSt->getAAMetadata(AANodes);

  RegDDRef *PtrRef;
  if (NeedScalarRef) {
    PtrRef = getOrCreateScalarRef(VPPtr, 0 /*LaneID*/);
    if (PtrRef->hasGEPInfo())
      PtrRef->setBitCastDestVecOrElemType(VPLdSt->getValueType());
  } else
    PtrRef = getWidenedAddressForScatterGather(VPPtr, VPLdSt->getValueType());

  RegDDRef *MemRef;
  if (PtrRef->isAddressOf()) {
    // We generate an addressof ref from the GEP instruction. For
    // generating load/store instruction, we need to clear the
    // addressof.
    MemRef = PtrRef;
    MemRef->setAddressOf(false);
  } else {
    MemRef = createMemrefFromBlob(PtrRef, IndexedElementType, 0,
                                  NeedScalarRef ? 1 : VF);
  }

  Type *ValTy = VPLdSt->getValueType();
  if (!Lane0Value) {
    Type *VecValTy = getWidenedType(ValTy, VF);

    // MemRef's bitcast type needs to be set to a pointer to <VF x ValType>.
    MemRef->setBitCastDestVecOrElemType(VecValTy);
  }
  MemRef->setSymbase(ScalSymbase);
  propagateMetadata(MemRef, VPLdSt);

  // Adjust the memory reference for the negative one stride case so that
  // the client can do a wide load/store.
  if (IsNegOneStride)
    MemRef->shift(getNestingLevelFromInsertPoint(), (int64_t)VF - 1);

  // Set ref alignment using original alignment.
  MemRef->setAlignment(Alignment.value());

  // Attach PreferredAlignmentMetadata if VPLdSt is a dynamic peeling candidate.
  if (NeedPeelLoop) {
    VPlanPeelingVariant *PreferredPeeling = Plan->getPreferredPeeling(VF);
    if (auto *DynPeeling =
            dyn_cast_or_null<VPlanDynamicPeeling>(PreferredPeeling)) {
      if (VPLdSt == DynPeeling->memref()) {
        LLVMContext &Context = *Plan->getLLVMContext();
        auto *CI = ConstantInt::get(Type::getInt32Ty(Context),
                                    DynPeeling->targetAlignment().value());
        SmallVector<Metadata *, 1> Ops{ConstantAsMetadata::get(CI)};
        MemRef->setMetadata("intel.preferred_alignment",
                            MDTuple::get(Context, Ops));
        auto DL = MemRef->getDDRefUtils().getDataLayout();
        auto LdStValAlignment = DL.getABITypeAlignment(VPLdSt->getValueType());

        // If memory ref is properly aligned on element boundary, we can set
        // its alignment to the target alignment from dynamic peeling.
        if (Alignment == LdStValAlignment)
          MemRef->setAlignment(DynPeeling->targetAlignment().value());
      }
    }
  }

  return MemRef;
}

template <class VLSOpTy>
RegDDRef *VPOCodeGenHIR::getVLSMemoryRef(const VLSOpTy *LoadStore) {
  RegDDRef *MemRef = getOrCreateScalarRef(LoadStore->getPointerOperand(), 0);
  if (MemRef->isAddressOf())
    MemRef->setAddressOf(false);
  else {
    auto *VecTy = cast<VectorType>(LoadStore->getValueType());
    MemRef = createMemrefFromBlob(MemRef, VecTy->getElementType(), 0 /*Idx*/,
                                  1 /*IdxBcastFactor*/);
  }
  MemRef->setBitCastDestVecOrElemType(LoadStore->getValueType());
  MemRef->setAlignment(LoadStore->getAlignment().value());
  MemRef->setSymbase(LoadStore->HIR().getSymbase());

  for (std::pair<unsigned, MDNode *> It : LoadStore->getMetadata())
    MemRef->setMetadata(It.first, It.second);

  return MemRef;
}

HLInst *VPOCodeGenHIR::createSelectHelper(const CmpInst::Predicate &CmpPred,
                                          RegDDRef *PredOp0, RegDDRef *PredOp1,
                                          RegDDRef *Val0, RegDDRef *Val1,
                                          unsigned ReplicateFactor,
                                          const Twine &Name, RegDDRef *LVal,
                                          FastMathFlags FMF) {
  assert(PredOp0 && "Expected non-null PredOp0");

  // PredOp0/PredOp1 vector elements need to be replicated by ReplicateFactor
  // times.
  if (ReplicateFactor > 1) {
    HLInst *ReplInst = replicateVectorElts(PredOp0, ReplicateFactor);
    addInstUnmasked(ReplInst);
    PredOp0 = ReplInst->getLvalDDRef()->clone();

    // Replicate PredOp1 vector elements if non-null
    if (PredOp1) {
      ReplInst = replicateVectorElts(PredOp1, ReplicateFactor);
      addInstUnmasked(ReplInst);
      PredOp1 = ReplInst->getLvalDDRef()->clone();
    }
  }

  // If PredOp1 is null we need to generate the equivalent of PredOp0 CmpPred
  // True
  if (!PredOp1) {
    assert(CmpPred == CmpInst::ICMP_EQ &&
           "Expected ICMP_EQ for compare predicate");
    auto *Op0VecTy = cast<FixedVectorType>(PredOp0->getDestType());
    Constant *OneVal = Constant::getAllOnesValue(Op0VecTy->getScalarType());

    // Broadcast OneVal to a vector with same number of elements as in PredOp0
    // whose vector elements have been replicated already if needed.
    PredOp1 = getConstantSplatDDRef(DDRefUtilities, OneVal,
                                    Op0VecTy->getNumElements());
  }

  HLInst *SelectInst = HLNodeUtilities.createSelect(
      CmpPred, PredOp0, PredOp1, Val0, Val1, Name, LVal, FMF);
  return SelectInst;
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

    auto *VecTy = dyn_cast<FixedVectorType>(Blend->getType());
    unsigned ReplicateFactor = VecTy ? VecTy->getNumElements() : 0;
    HLInst *BlendInst = createSelectHelper(CmpInst::ICMP_EQ /* CmpPred */, Cond,
                                           nullptr /* Pred1 */, IncomingVecVal,
                                           BlendVal, ReplicateFactor);
    addInstUnmasked(BlendInst);
    BlendVal = BlendInst->getLvalDDRef()->clone();
  }

  addVPValueWideRefMapping(Blend, BlendVal);
}

// Widen PHI instruction.
void VPOCodeGenHIR::widenPhiImpl(const VPPHINode *VPPhi, RegDDRef *Mask) {
  // Check if  PHI is the loop IV and widen it by generating the ref i1 +
  // <0, 1, 2, .. VF-1>. TODO: Need to handle all IVs in general here.
  if (LoopIVPhis.count(VPPhi)) {
    generateLoopInductionRef(VPPhi);
    return;
  }

  assert(isDeconstructedPhi(VPPhi) &&
         "Non-reduction/induction PHI was not deconstructed.");
  // PHI that was deconstructed via copies during SSA deconstruction is mapped
  // to the common Lval temp that the copies write into.
  auto *HIRCopy = cast<VPHIRCopyInst>(VPPhi->getOperand(0));
  int OriginPhiId = HIRCopy->getOriginPhiId();
  RegDDRef *PhiTemp = getLValTempForPhiId(OriginPhiId);
  assert(PhiTemp && "Deconstructed PHI does not have a LVal temp.");

  // Update mapping in scalar/vector maps based on nature of deconstructed
  // copies created for the PHI.
  if (instIsStrictlyFirstScalar(HIRCopy))
    addVPValueScalRefMapping(VPPhi, PhiTemp, 0 /*Lane*/);
  else
    addVPValueWideRefMapping(VPPhi, PhiTemp);

  if (!hasNoExternalUsers(VPPhi)) {
    // If it's used by an external, copy it to that temp.
    assert(instIsStrictlyFirstScalar(HIRCopy) && "Expected scalar");
    const VPExternalUse *ExtUse = getSingleExternalUse(VPPhi, Plan);
    auto PhiExtUse = getUniformScalarRef(ExtUse);
    HLInst *NewInst =
        HLNodeUtilities.createCopyInst(PhiTemp->clone(), ".copy", PhiExtUse);
    addInstUnmasked(NewInst);
  }
}

void VPOCodeGenHIR::generateLoopInductionRef(const VPPHINode *VPPhi) {
  auto *RefDestTy = VPPhi->getType();
  auto *VecRefDestTy = getWidenedType(RefDestTy, VF);
  auto *VPLp = Plan->getVPLoopInfo()->getLoopFor(VPPhi->getParent());
  assert(VPLp && "Unexpected null VPLoop for induction Phi");
  auto *HLoop = VPLoopHLLoopMap[VPLp];
  assert(HLoop && HLoop->isAttached() &&
         "Expected non-null attached HLLoop mapping");

  // Create a vector value for IV and add it to wide ref map.
  auto *VecCE = CanonExprUtilities.createCanonExpr(VecRefDestTy);
  VecCE->addIV(HLoop->getNestingLevel(), InvalidBlobIndex /* no blob */,
               1 /* constant IV coefficient */);

  // We only vectorize at outer most(candidate) loop level. Value increment
  // for each lane only applies at the candidate loop level.
  if (!VPLp->getParentLoop()) {
    SmallVector<Constant *, 4> ConstVec;
    for (unsigned i = 0; i < VF; ++i)
      ConstVec.push_back(ConstantInt::getSigned(RefDestTy, i));
    unsigned Idx = 0;
    BlobUtilities.createConstantBlob(ConstantVector::get(ConstVec), true, &Idx);
    VecCE->addBlob(Idx, 1);
  }

  auto *VecRef = DDRefUtilities.createScalarRegDDRef(GenericRvalSymbase, VecCE);
  addVPValueWideRefMapping(VPPhi, VecRef);

  // Create a scalar value for IV and add it to scalar ref map.
  auto *ScalCE = CanonExprUtilities.createCanonExpr(RefDestTy);
  ScalCE->addIV(HLoop->getNestingLevel(), InvalidBlobIndex /* no blob */,
                1 /* constant IV coefficient */);
  auto *ScalRef =
      DDRefUtilities.createScalarRegDDRef(GenericRvalSymbase, ScalCE);
  addVPValueScalRefMapping(VPPhi, ScalRef, 0);
}

RegDDRef *VPOCodeGenHIR::getOrCreateScalarRef(const VPValue *VPVal,
                                              unsigned ScalarLaneID) {
  RegDDRef *ScalarRef = nullptr;
  if ((ScalarRef = getScalRefForVPVal(VPVal, ScalarLaneID)))
    return ScalarRef->clone();

  if (isa<VPExternalDef>(VPVal) || isa<VPConstant>(VPVal) ||
      isa<VPMetadataAsValue>(VPVal) || isa<VPLiveOutValue>(VPVal))
    return getUniformScalarRef(VPVal);

  assert(ScalarLaneID < getVF() && "Invalid lane ID.");
  RegDDRef *WideRef = widenRef(VPVal, getVF());

  HLInst *ExtractInst = nullptr;

  // Decide the extraction scheme based on type of VPValue - we need to extract
  // subvector for re-vectorized incoming VectorType, while we do a simple
  // extractelement for scalar type.
  if (auto *VPValVecTy = dyn_cast<FixedVectorType>(VPVal->getType())) {
    // Here we assume that the widened vector we are extracting subvector from
    // is in AOS layout. Consider the below example -
    // VPVal              -  <v1, v2>
    // WideRef (VF=2)     -  <v1_lane0, v2_lane0, v1_lane1, v2_lane1>
    // ScalarRef (Lane=1) -  <v1_lane1, v2_lane1>
    unsigned OrigNumElts = VPValVecTy->getNumElements();
    SmallVector<Constant *, 8> ShuffleMask;
    for (unsigned Start = ScalarLaneID * OrigNumElts,
                  End = (ScalarLaneID * OrigNumElts) + OrigNumElts;
         Start != End; ++Start)
      ShuffleMask.push_back(ConstantInt::get(Type::getInt32Ty(Context), Start));

    ExtractInst =
        createShuffleWithUndef(WideRef->clone(), ShuffleMask, "extractsubvec.");
  } else {
    ExtractInst = HLNodeUtilities.createExtractElementInst(
        WideRef->clone(), (unsigned)ScalarLaneID,
        "extract." + Twine(ScalarLaneID) + ".");
  }

  addInstUnmasked(ExtractInst);
  return ExtractInst->getLvalDDRef()->clone();
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
  assert(ParentFinal && "trying to broadcast Lval?");
  unsigned Opc = RedFinal->getBinOpcode();
  PredicateTy Pred = ParentFinal->getDestType()->isFPOrFPVectorTy()
                         ? PredicateTy::FCMP_OEQ
                         : PredicateTy::ICMP_EQ;

  Type *IndexVecTy = ReduceVal->getDestType();
  if (RedFinal->isLinearIndex()) {
    assert(isa<IntegerType>(IndexVecTy->getScalarType()) &&
           "Index part of minmax+index idiom is not integer type.");

    bool NeedMaxIntVal =
        (Opc == VPInstruction::FMin || Opc == VPInstruction::SMin ||
         Opc == VPInstruction::UMin);

    Constant *MinMaxIntVec = VPOParoptUtils::getMinMaxIntVal(
        IndexVecTy, !RedFinal->isSigned(), NeedMaxIntVal);
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
  } else {
    HLInst *CmpInst = HLNodeUtilities.createCmp(
        Pred, ParentFinal, ParentExit->clone(), "mmidx.cmp.");
    RedTail.push_back(*CmpInst);

    // Call is pushed to container automatically
    HLInst *BsfCall =
        createCTZCall(CmpInst->getLvalDDRef()->clone(), Intrinsic::cttz,
                      true /* MaskIsNonZero */, &RedTail);

    HLInst *ExtractInst = HLNodeUtilities.createExtractElementInst(
        ReduceVal->clone(), BsfCall->getLvalDDRef()->clone(), "elem",
        RednVariable);
    RedTail.push_back(*ExtractInst);
    WInst = ExtractInst;
  }
}

RegDDRef *
VPOCodeGenHIR::createVectorPrivatePtrs(const VPAllocatePrivate *VPPvt) {
  assert(PrivateMemBlobRefs.count(VPPvt) &&
         "Private memory pointer not widened.");

  BlobDDRef *AllocaBlob = PrivateMemBlobRefs[VPPvt].first;
  unsigned AllocaMemRefSym = PrivateMemBlobRefs[VPPvt].second;
  PointerType *PvtTy = cast<PointerType>(VPPvt->getType());
  Type *AllocatedType = VPPvt->getAllocatedType();

  // In order to create a vector of pointers, we generate a scalar base pointer
  // and then use a vector of indices. Pseudo underlying LLVM-IR -
  // %priv.mem.bc = bitcast <VF x Ty>* %priv.mem to Ty*
  // %pvt.vec.ptrs = gep Ty, Ty* %priv.mem.bc, <VF x i32> <0, ..., VF-1>
  //
  // To achieve this sequence in HIR, we first create a self address-of DDRef
  // using the widened private blob and bitcast it to compute base-address. This
  // base-address is then used to generate another address-of DDRef with a
  // vector of indices in first dimension. For example -
  // AllocaBlob = <VF x i32>* %priv.mem
  // BaseRef = &((i32*)(<VF x i32>* %priv.mem)[i32 0])
  // VecPvtPtrs = &((<VF x i32*>)(i32* %priv.mem.bc)[<VFx i32> <0, .., VF-1])

  Type *WidenedType =
      AllocatedType->isAggregateType()
          ? static_cast<Type *>(ArrayType::get(AllocatedType, getVF()))
          : getWidenedType(AllocatedType, getVF());
  auto *BaseRef = DDRefUtilities.createSelfAddressOfRef(
      WidenedType, AllocaBlob->getSelfBlobIndex(),
      AllocaBlob->getDefinedAtLevel(), AllocaMemRefSym);
  BaseRef->setBitCastDestVecOrElemType(AllocatedType);
  // Need to create a copy inst to capture base-address since HIR does not allow
  // GEP Refs to be embedded into each other.
  HLInst *BaseRefCopy = HLNodeUtilities.createCopyInst(BaseRef, "priv.mem.bc");
  addInstUnmasked(BaseRefCopy);
  LLVM_DEBUG(dbgs() << "[VPOCGHIR] createVectorPrivatePtrs: BaseRef: ";
             BaseRef->dump(1); dbgs() << "\n");

  SmallVector<Constant *, 16> Indices;
  // Create a vector of consecutive numbers from zero to VF-1.
  // TODO: For allocas of the format -
  //     %priv = alloca i32, i32 %n
  // generating consecutive numbers is incorrect. We need -
  //     <%n * 0, %n * 1,..., %n * VF-1>
  // to correctly compute base addresses.
  for (unsigned I = 0; I < getVF(); ++I)
    Indices.push_back(
        ConstantInt::get(Type::getInt32Ty(PvtTy->getContext()), I));

  auto *Cv = ConstantVector::get(Indices);
  CanonExpr *IndexCE =
      CanonExprUtilities.createConstStandAloneBlobCanonExpr(Cv);

  // Create address-of ref to compute vector of pointers using base-address and
  // vector indices. The base ptr of this address-of ref will be defined in loop
  // preheader always, hence set defined at level as (LoopLevel - 1).
  auto *VecPvtPtrs = DDRefUtilities.createAddressOfRef(
      AllocatedType, BaseRefCopy->getLvalDDRef()->getSelfBlobIndex(),
      OrigLoop->getNestingLevel() - 1, AllocaMemRefSym);
  VecPvtPtrs->addDimension(IndexCE);
  VecPvtPtrs->setBitCastDestVecOrElemType(getWidenedType(PvtTy, getVF()));
  return VecPvtPtrs;
}

void VPOCodeGenHIR::insertReductionInit(HLContainerTy *List) {
  HLNode *Save = &List->back();

  // If the reductions are not being hoisted simply use the addInst interface.
  if (RednHoistLp == MainLoop) {
    addInst(List);
    return;
  }

  // The list of nodes are effectively added as the last pre-header nodes
  // either using the non-null insertion point or the pre-header of the
  // reduction hoist loop. Insertion point is updated appropriately for
  // subsequent reduction initializer instructions.
  if (!RednInitInsertPoint)
    HLNodeUtils::insertAsLastPreheaderNodes(cast<HLLoop>(RednHoistLp), List);
  else
    HLNodeUtils::insertAfter(RednInitInsertPoint, List);

  RednInitInsertPoint = Save;
}

void VPOCodeGenHIR::insertReductionFinal(HLContainerTy *List) {
  HLNode *Save = &List->back();

  // If the reductions are not being hoisted simply use the addInst interface.
  if (RednHoistLp == MainLoop) {
    addInst(List);
    return;
  }

  // The list of nodes are added after the non-null insertion point or
  // at the start of the post-exit of the reduction hoist loop if the
  // insertion point is null. Insertion point is updated appropriately for
  // subsequent reduction finalizer instructions.
  if (!RednFinalInsertPoint)
    HLNodeUtils::insertAsFirstPostexitNodes(RednHoistLp, List);
  else
    HLNodeUtils::insertAfter(RednFinalInsertPoint, List);

  RednFinalInsertPoint = Save;
}

void VPOCodeGenHIR::widenLoopEntityInst(const VPInstruction *VPInst) {
  HLInst *WInst = nullptr; // Track the last generated wide inst for VPInst

  switch (VPInst->getOpcode()) {
  case VPInstruction::ReductionInit: {
    const VPReductionInit *RedInit = cast<VPReductionInit>(VPInst);
    HLContainerTy RedInitHLInsts;
    RegDDRef *IdentityRef = widenRef(RedInit->getIdentityOperand(), getVF());
    // Create a copy for the widened identity value.
    HLInst *CopyInst = HLNodeUtilities.createCopyInst(IdentityRef, "red.init");
    WInst = CopyInst;
    RedInitHLInsts.push_back(*CopyInst);

    if (RedInit->getNumOperands() > 1) {
      auto *StartVPVal = RedInit->getStartValueOperand();
      // Insert start value into lane 0 of identity vector.
      HLInst *InsertElementInst = HLNodeUtilities.createInsertElementInst(
          CopyInst->getLvalDDRef()->clone(),
          getOrCreateScalarRef(StartVPVal, 0 /*Lane*/), 0, "red.init.insert");
      WInst = InsertElementInst;
      RedInitHLInsts.push_back(*InsertElementInst);
    }
    insertReductionInit(&RedInitHLInsts);
    addVPValueWideRefMapping(VPInst, WInst->getLvalDDRef());
    return;
  }

  case VPInstruction::ReductionFinal: {
    const VPReductionFinal *RedFinal = cast<VPReductionFinal>(VPInst);
    HLContainerTy RedTail;

    RegDDRef *VecRef = widenRef(RedFinal->getReducingOperand(), getVF());

    // Add the reduction final vector ref as a live-out for each loop up to and
    // including the hoist loop starting from MainLoop's parent loop. The
    // liveout information for MainLoop will be added by setupLiveInLiveOut
    // call.
    auto RvalSymbase = VecRef->getSymbase();
    HLLoop *ThisLoop = MainLoop->getParentLoop();
    while (ThisLoop != RednHoistLp->getParentLoop()) {
      ThisLoop->addLiveOutTemp(RvalSymbase);
      ThisLoop = ThisLoop->getParentLoop();
    }

    Type *ElType = RedFinal->getReducingOperand()->getType();
    if (isa<VectorType>(ElType)) {
      // Incoming vector types is not supported for HIR
      llvm_unreachable("Unsupported vector data type for reducing operand.");
    }

    // Scalar result of vector reduce intrinsic should be written back to the
    // original reduction descriptor variable, obtained from external user.
    // If a reduction is not live-out then it's essentially dead code, finalized
    // value can be written to a new temp since the value is not going to be
    // used anywhere.
    RegDDRef *RednDescriptor = nullptr;
    if (!hasNoExternalUsers(RedFinal)) {
      const VPExternalUse *ExtUse = getSingleExternalUse(RedFinal, Plan);
      RednDescriptor = getUniformScalarRef(ExtUse);
    }

    if (RedFinal->isMinMaxIndex())
      generateMinMaxIndex(RedFinal, RednDescriptor, RedTail, WInst);
    else {
      auto *StartVPVal = RedFinal->getStartValueOperand();
      RegDDRef *Acc = nullptr;

      if (StartVPVal) {
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

    insertReductionFinal(&RedTail);
    addVPValueScalRefMapping(VPInst, WInst->getLvalDDRef(), 0);
    return;
  }

  case VPInstruction::ReductionFinalUdr: {
    // Call combiner for each pointer in private memory and accumulate the
    // results in original DDRef corresponding to the UDR.
    auto *Orig = getOrCreateScalarRef(VPInst->getOperand(1), 0);
    auto *Priv = cast<VPAllocatePrivate>(VPInst->getOperand(0));
    Function *CombinerFn = cast<VPReductionFinalUDR>(VPInst)->getCombiner();

    for (unsigned Lane = 0; Lane < getVF(); Lane++) {
      auto *LanePvtPtr = getOrCreateScalarRef(Priv, Lane);
      auto *CombinerCall =
          HLNodeUtilities.createCall(CombinerFn, {Orig->clone(), LanePvtPtr});
      addInstUnmasked(CombinerCall);
    }

    return;
  }

  case VPInstruction::InductionInitStep: {
    const VPInductionInitStep *IndInitStep = cast<VPInductionInitStep>(VPInst);
    if (IndInitStep->isMainLoopIV()) {
      // Special handling done for the implicit IV.
      auto *ConstStep = cast<VPConstant>(VPInst->getOperand(0));

      assert((IndInitStep->getBinOpcode() == Instruction::Add &&
              ConstStep->getConstant()->isOneValue()) &&
             "Expected an add induction with constant step of 1");
      auto *ScalRef =
          DDRefUtilities.createConstDDRef(ConstStep->getType(), getVF());
      auto *WideRef = widenRef(ScalRef, getVF());
      addVPValueWideRefMapping(VPInst, WideRef);
      addVPValueScalRefMapping(VPInst, ScalRef, 0);
      return;
    }

    // For generating step values for induction step initailization
    // we expect: step * VF
    unsigned Opc = IndInitStep->getBinOpcode();
    bool isMult = Opc == Instruction::Mul || Opc == Instruction::FMul ||
                  Opc == Instruction::SDiv || Opc == Instruction::UDiv ||
                  Opc == Instruction::FDiv;
    bool IsFloat = VPInst->getType()->isFloatingPointTy();
    auto *StartVal = getOrCreateScalarRef(VPInst->getOperand(0), 0);

    unsigned StepOpc = IsFloat ? Instruction::FMul : Instruction::Mul;
    RegDDRef *MulVF = StartVal;
    HLInst *MulInst = nullptr;
    assert(!isMult && "Mutiplication induction is not supported.");
    (void)isMult;
    RegDDRef *VFVal = nullptr;
    if (IsFloat) {
      VFVal = DDRefUtilities.createConstDDRef(VPInst->getType(), VF);
      MulInst = HLNodeUtilities.createFPMathBinOp(
          static_cast<Instruction::BinaryOps>(StepOpc), MulVF, VFVal,
          IndInitStep->getFastMathFlags(), "ind.step.init");
    } else {
      VFVal = DDRefUtilities.createConstDDRef(StartVal->getSrcType(), VF);
      MulInst = HLNodeUtilities.createBinaryHLInst(
          static_cast<Instruction::BinaryOps>(StepOpc), MulVF, VFVal,
          "ind.step.init");
    }
    addInstUnmasked(MulInst);
    MulVF = MulInst->getLvalDDRef()->clone();
    auto *WideRef = widenRef(MulVF, VF);
    addVPValueWideRefMapping(VPInst, WideRef);
    addVPValueScalRefMapping(VPInst, MulVF, 0);
    return;
  }

  case VPInstruction::InductionInit: {
    // We expect induction-init to be only used to initialize the lower bound of
    // vector loops, so we need to generate a scalar copy of the value for 0th
    // lane.

    const VPInductionInit *IndInit = cast<VPInductionInit>(VPInst);
    // Ignore induction-inits where start value is 0 since that is the default
    // lower bound we use while emitting a HLLoop.
    auto *ConstStart = dyn_cast<VPConstant>(IndInit->getOperand(0));
    if (IndInit->isMainLoopIV() && ConstStart) {
      assert(ConstStart->getConstant()->isZeroValue() &&
             "Expected 0 as Start for main loop IV.");
      return;
    }

    auto *StartVal = getOrCreateScalarRef(VPInst->getOperand(0), 0);
    auto *VectorStart = widenRef(StartVal, VF);
    // For generating step values for induction initailization we expect
    // following -
    // < 0, step, 2*step, 3*step> for +/- (or)
    // < 1, step, step^2, step^3> for */div
    auto *StepVal = getOrCreateScalarRef(VPInst->getOperand(1), 0);

    unsigned Opc = IndInit->getBinOpcode();
    bool isMult = Opc == Instruction::Mul || Opc == Instruction::FMul ||
                  Opc == Instruction::SDiv || Opc == Instruction::UDiv ||
                  Opc == Instruction::FDiv;
    bool IsFloat = VPInst->getType()->isFloatingPointTy();
    int StartConst = isMult ? 1 : 0;
    Constant *StartCoeff =
        IsFloat ? ConstantFP::get(VPInst->getType(), StartConst)
                : ConstantInt::get(StepVal->getSrcType(), StartConst);
    RegDDRef *VectorStep;
    assert(!isMult && "Mutiplication induction is not supported.");
    // Generate sequence of vector operations:
    // %i_seq = {0, 1, 2, 3, ...VF-1}
    // %bcst_step = broadcast step
    // %vector_step = mul %i_seq, %bcst_step
    SmallVector<Constant *, 32> IndStep;
    IndStep.push_back(StartCoeff);
    for (unsigned I = 1; I < VF; I++) {
      Constant *ConstVal =
          IsFloat ? ConstantFP::get(VPInst->getType(), I)
                  : ConstantInt::getSigned(StepVal->getSrcType(), I);
      IndStep.push_back(ConstVal);
    }
    RegDDRef *VecConst =
        DDRefUtilities.createConstDDRef(ConstantVector::get(IndStep));
    RegDDRef *WidenStep = widenRef(StepVal, VF);
    int64_t StepConst;
    if (StepVal->isIntConstant(&StepConst) && StepConst == 1) {
      VectorStep = VecConst;
    } else {
      HLInst *VectorStepInst = nullptr;
      if (IsFloat) {
        VectorStepInst = HLNodeUtilities.createFPMathBinOp(
            static_cast<Instruction::BinaryOps>(Instruction::FMul), WidenStep,
            VecConst, VPInst->getFastMathFlags(), "ind.vec.step");
      } else {
        VectorStepInst = HLNodeUtilities.createBinaryHLInst(
            static_cast<Instruction::BinaryOps>(Instruction::Mul), WidenStep,
            VecConst, "ind.vec.step");
      }
      addInstUnmasked(VectorStepInst);
      VectorStep = VectorStepInst->getLvalDDRef()->clone();
    }
    HLInst *RetInst = HLNodeUtilities.createBinaryHLInst(
        static_cast<Instruction::BinaryOps>(Opc), VectorStart, VectorStep);
    addInstUnmasked(RetInst);

    addVPValueWideRefMapping(VPInst, RetInst->getLvalDDRef()->clone());
    addVPValueScalRefMapping(VPInst, StartVal, 0);
    return;
  }

  case VPInstruction::InductionFinal: {
    if (isSearchLoop()) {
      // We use another mechanism to set upper bounds so that does not work w/o
      // CFG merger.
      return;
    }
    RegDDRef *LastValue = nullptr;
    if (VPInst->getNumOperands() == 1) {
      // One operand - extract from vector
      RegDDRef *VecVal = widenRef(VPInst->getOperand(0), VF);
      HLInst *LastValueExtract = HLNodeUtilities.createExtractElementInst(
          VecVal, getVF() - 1, "extracted.lval");
      addInstUnmasked(LastValueExtract);
      LastValue = LastValueExtract->getLvalDDRef()->clone();
    } else {
      // Otherwise calculate by formulas
      //  for post increment liveouts LV = start + step*upper_bound,
      //  for pre increment liveouts LV = start + step*(upper_bound-1)
      //
      assert(VPInst->getNumOperands() == 2 && "Incorrect number of operands");
      const VPInductionFinal *IndFini = cast<VPInductionFinal>(VPInst);
      unsigned Opc = IndFini->getBinOpcode();
      assert(!(Opc == Instruction::Mul || Opc == Instruction::FMul ||
               Opc == Instruction::SDiv || Opc == Instruction::UDiv ||
               Opc == Instruction::FDiv) &&
             "Unsupported induction final form");

      bool IsFloat = IndFini->getType()->isFloatingPointTy();
      auto *Step = getOrCreateScalarRef(IndFini->getOperand(1), 0);
      auto *Start = getOrCreateScalarRef(IndFini->getOperand(0), 0);

      unsigned StepOpc = IsFloat ? Instruction::FMul : Instruction::Mul;

      // Get the latch comparison of the loop. We know that
      // VPInductionFinal is created in the loop exit so get its parent
      // predecessor to find a loop. The upper bound of the loop is used in the
      // latch comparison (this is ensured either by the check for normalized IV
      // or by emission of the new IV).
      VPLoop *L = nullptr;
      VPBasicBlock *VPIndFinalBB =
          *IndFini->getParent()->getPredecessors().begin();
      L = Plan->getVPLoopInfo()->getLoopFor(VPIndFinalBB);
      while (!L) {
        VPIndFinalBB = *VPIndFinalBB->getPredecessors().begin();
        L = Plan->getVPLoopInfo()->getLoopFor(VPIndFinalBB);
      }
      bool ExactUB = L->exactUB();
      VPCmpInst *Cond = L->getLatchComparison();
      HLInst *TripInst = nullptr;
      assert(Cond && "Latch comparison is expected.");
      VPValue *VPTripCount = L->isDefOutside(Cond->getOperand(0))
                                 ? Cond->getOperand(0)
                                 : Cond->getOperand(1);
      RegDDRef *TripCnt = getOrCreateScalarRef(VPTripCount, 0);
      int64_t TripCntConst, StartConst, StepConst;
      if (TripCnt->isIntConstant(&TripCntConst) &&
          Start->isIntConstant(&StartConst) &&
          Step->isIntConstant(&StepConst)) {
        if (IndFini->isLastValPreIncrement())
          TripCntConst -= 1;
        if (!ExactUB)
          TripCntConst += 1;
        int64_t LastValueConst = StartConst + StepConst * TripCntConst;
        LastValue = DDRefUtilities.createConstDDRef(Start->getSrcType(),
                                                    LastValueConst);
      } else {
        RegDDRef *ConstOne =
            DDRefUtilities.createConstDDRef(TripCnt->getSrcType(), 1);
        if (IndFini->isLastValPreIncrement()) {
          TripInst = HLNodeUtilities.createSub(TripCnt, ConstOne, "sub.tripcnt",
                                               TripCnt);
          addInstUnmasked(TripInst);
        }
        if (!ExactUB) {
          TripInst = HLNodeUtilities.createAdd(TripCnt, ConstOne, "add.tripcnt",
                                               TripCnt);
          addInstUnmasked(TripInst);
        }
        assert(TripCnt->getSrcType() == Step->getSrcType() &&
               "Trip count type must be equal to step type.");
        RegDDRef *MulRef;
        if (Step->isIntConstant(&StepConst) && StepConst == 1) {
          MulRef = TripCnt;
        } else {
          HLInst *MulInst = HLNodeUtilities.createBinaryHLInst(
              static_cast<Instruction::BinaryOps>(StepOpc), Step, TripCnt);
          addInstUnmasked(MulInst);
          MulRef = MulInst->getLvalDDRef()->clone();
        }
        assert(!VPInst->getType()->isPointerTy() &&
               "Pointer type is not supported.");
        HLInst *LastValInst = HLNodeUtilities.createBinaryHLInst(
            static_cast<Instruction::BinaryOps>(Opc), Start, MulRef,
            "ind.final");
        addInstUnmasked(LastValInst);
        LastValue = LastValInst->getLvalDDRef()->clone();
      }
    }
    // The value is scalar
    addVPValueScalRefMapping(VPInst, LastValue, 0);
    return;
  }

  case VPInstruction::AllocatePrivate: {
    auto *VPAllocaPriv = cast<VPAllocatePrivate>(VPInst);
    Type *OrigTy = VPAllocaPriv->getAllocatedType();

    // We need to allocate VF copies of element based on privatized memory type.
    // For simple scalars we use <VF x Ty>, while for aggregate types we use [VF
    // x Ty].
    // TODO: Extend support for privates optimized using SOA layout.
    Type *VecTyForAlloca = nullptr;
    if (OrigTy->isAggregateType())
      VecTyForAlloca = ArrayType::get(OrigTy, getVF());
    else {
      assert(!isa<VectorType>(OrigTy) &&
             "VectorType for AllocatePrivate not supported.");
      VecTyForAlloca = getWidenedType(OrigTy, getVF());
    }

    // Create a new vector blob to represent the widened private memory and map
    // it to the AllocatePrivate instruction.
    unsigned AllocaBlobIdx = HLNodeUtilities.createAlloca(
        VecTyForAlloca, OrigLoop->getParentRegion(), "priv.mem");
    BlobDDRef *AllocaBlob =
        DDRefUtilities.createBlobDDRef(AllocaBlobIdx, 0 /* Linear Level */);
    // Obtain a new symbase to assign for all memory operations based on the new
    // alloca.
    unsigned AllocaMemRefSym = DDRefUtilities.getNewSymbase();
    LLVM_DEBUG(dbgs() << "Private memory: "; VPInst->dump();
               dbgs() << " got the BlobDDRef: "; AllocaBlob->dump(1);
               dbgs() << "\n and unique memref symbase: " << AllocaMemRefSym
                      << "\n");
    PrivateMemBlobRefs[VPAllocaPriv] =
        std::make_pair(AllocaBlob, AllocaMemRefSym);
    // Since we are adding a new r-val use of temp outside the loop (check
    // createVectorPrivatePtrs), make the temp live-in at all parent loop
    // levels.
    makeSymLiveInForParentLoops(AllocaBlob->getSymbase());

    // Generated blob provides a pointer to the widened memory, here we generate
    // code to compute a vector of pointers pointing to each element of widened
    // memory. For example -
    // OrigTy = i32*
    // VecTyForAlloca = <VF x i32>
    // AllocaBlob = <VF x i32>* %priv.mem
    // VecPvtPtrs = &((<VF x i32*>)(i32* %priv.mem.bc)[<VFx i32> <0, .., VF-1])
    // ScalPvtPtr = &((i32 *)(<VF x i32>* %priv.mem)[i32 0])
    RegDDRef *VecPvtPtrs = createVectorPrivatePtrs(VPAllocaPriv);
    RegDDRef *ScalPvtPtr = DDRefUtilities.createSelfAddressOfRef(
        VecTyForAlloca, AllocaBlob->getSelfBlobIndex(),
        AllocaBlob->getDefinedAtLevel(), AllocaMemRefSym);
    ScalPvtPtr->setBitCastDestVecOrElemType(VPAllocaPriv->getAllocatedType());

    LLVM_DEBUG(dbgs() << "Private memory: "; VPInst->dump();
               dbgs() << " got the vector of private pointers: ";
               VecPvtPtrs->dump(1); dbgs() << "\n"
                                           << "and scalar private pointer: ";
               ScalPvtPtr->dump(1); dbgs() << "\n");

    addVPValueWideRefMapping(VPInst, VecPvtPtrs);
    addVPValueScalRefMapping(VPInst, ScalPvtPtr, 0 /*Lane*/);
    return;
  }

  case VPInstruction::PrivateFinalUncond:
  case VPInstruction::PrivateFinalUncondMem: {
    RegDDRef *OrigPrivDescr = nullptr;
    if (VPInst->getOpcode() == VPInstruction::PrivateFinalUncond) {
      // Scalar result of private finalization should be written back to
      // original private descriptor variable, obtained from external user.
      assert(!hasNoExternalUsers(VPInst) &&
             "Private final does not have any external uses.");
      const VPExternalUse *ExtUse = getSingleExternalUse(VPInst, Plan);
      OrigPrivDescr = getUniformScalarRef(ExtUse);
    }
    RegDDRef *VecRef = widenRef(VPInst->getOperand(0), getVF());

    RegDDRef *ResultRef;
    if (VPInst->getOperand(0)->getType()->isVectorTy()) {
      ResultRef = extractSubVector(VecRef, getVF() - 1, getVF(), OrigPrivDescr);
    } else {
      HLInst *PrivExtract = HLNodeUtilities.createExtractElementInst(
          VecRef, getVF() - 1, "extracted.priv", OrigPrivDescr);
      addInstUnmasked(PrivExtract);
      ResultRef = PrivExtract->getLvalDDRef();
    }
    // Make the original private descriptor non-linear since we have a
    // definition to the temp in loop post-exit.
    if (OrigPrivDescr)
      OrigPrivDescr->getSingleCanonExpr()->setNonLinear();
    addVPValueScalRefMapping(VPInst, ResultRef, 0 /*Lane*/);

    return;
  }

  case VPInstruction::PrivateFinalCondMem:
  case VPInstruction::PrivateFinalCond: {
    // Pseudo HIR generated to finalize conditional last private entity -
    // %priv.final = private-final-c %exit, %idx, %orig
    //
    // ; Find max index where condition was true
    // %idx.reduce = llvm.vector.reduce.smax.v2i64(%idx.vec)
    // ; Identify where max index is set in final vector
    // %max.idx.cmp = %idx.reduce == %idx.vec
    // ; Obtain lane for extraction
    // %bsfintmask = bitcast.<2 x i1>.i2(%max.idx.cmp);
    // %lane = @llvm.cttz.i2(%bsfintmask,  1);
    // ; Extract final value and store back to original
    // %orig = extractelement %exit.vec, %lane
    //
    if (isa<VPPrivateFinalCond>(VPInst))
      insertPrivateFinalCond<VPPrivateFinalCond>(VPInst);
    else if (isa<VPPrivateFinalCondMem>(VPInst))
      insertPrivateFinalCond<VPPrivateFinalCondMem>(VPInst);
    return;
  }

  case VPInstruction::PrivateFinalMaskedMem:
  case VPInstruction::PrivateFinalMasked: {
    // Pseudo HIR generated to finalize masked last private entity,
    // %priv.final = private-masked %exit, %mask, %orig
    // ==>
    // ; Obtain lane for extraction
    // %bsfintmask = bitcast.<2 x i1>.i2(%mask);
    // %lz = @llvm.ctlz.i2(%bsfintmask,  1);
    // %lane = VF - 1 - %lz;
    // ; Extract final value and store back to original
    // %orig = extractelement %exit, %lane
    //
    // In case of in-memory private the %orig is unknown - we don't have that
    // operand in the PrivateFinalMaskedMem instruction. In this case we just
    // create a new temp.
    //
    HLContainerTy PrivFinalInsts;
    RegDDRef *VecMask = widenRef(VPInst->getOperand(1), getVF());

    HLInst *BsfCall = createCTZCall(VecMask->clone(),
                                    Intrinsic::ctlz, true, &PrivFinalInsts);

    // Scalar result of registerized private finalization should be written back
    // to original private descriptor variable.
    RegDDRef *OrigPrivDescr = nullptr;
    if (VPInst->getOpcode() == VPInstruction::PrivateFinalMasked) {
      OrigPrivDescr = getUniformScalarRef(VPInst->getOperand(2));
      if (OrigPrivDescr->isConstant())
        OrigPrivDescr = nullptr;
    }

    RegDDRef *VecExit = widenRef(VPInst->getOperand(0), getVF());

    RegDDRef *BsfLhs = BsfCall->getLvalDDRef();
    HLInst *SubInst = HLNodeUtilities.createSub(
        DDRefUtilities.createConstDDRef(BsfLhs->getDestType(), VF - 1),
        BsfLhs->clone(), "ext.lane");
    PrivFinalInsts.push_back(*SubInst);

    HLInst *PrivExtract = HLNodeUtilities.createExtractElementInst(
        VecExit->clone(), SubInst->getLvalDDRef()->clone(), "priv.extract",
        OrigPrivDescr);
    PrivFinalInsts.push_back(*PrivExtract);

    // Make the original private descriptor non-linear since we have a
    // definition to the temp in loop post-exit.
    PrivExtract->getLvalDDRef()->getSingleCanonExpr()->setNonLinear();

    addInst(&PrivFinalInsts);
    addVPValueScalRefMapping(VPInst, PrivExtract->getLvalDDRef(), 0 /*Lane*/);
    return;
  }

  case VPInstruction::PrivateFinalArray: {
    VPAllocatePrivate *Priv = cast<VPAllocatePrivate>(VPInst->getOperand(0));
    Type *ElementType = Priv->getAllocatedType();
    const DataLayout &DL = DDRefUtilities.getDataLayout();

    // We need to copy array from last private allocated memory into the
    // original array location.
    RegDDRef *Orig = getOrCreateScalarRef(VPInst->getOperand(1), 0);
    auto *OrigAddr = DDRefUtilities.createSelfAddressOfRef(
        ElementType, Orig->getSelfBlobIndex(), Orig->getDefinedAtLevel());
    OrigAddr->setAlignment(DL.getPrefTypeAlign(Orig->getSrcType()).value());

    // TODO: Add support for SOA layout in HIR (CMPLRLLVM-9193).
    assert(!Priv->isSOALayout() && "SOA layout is not supported for HIR.");

    // In case of non-SOA layout it will be enough to copy memory from last
    // private into the original array.
    RegDDRef *ResAddr = getOrCreateScalarRef(Priv, 0);
    auto IndexCE = CanonExprUtilities.createCanonExpr(
        ResAddr->getSingleCanonExpr()->getDestType());
    IndexCE->addConstant(getVF() - 1, true /* IsMathAdd */);
    ResAddr->addDimension(IndexCE);
    ResAddr->setAlignment(Priv->getOrigAlignment().value());

    Type *SizeTy = Type::getInt64Ty(HLNodeUtilities.getContext());
    RegDDRef *Size = DDRefUtilities.createConstDDRef(
        SizeTy, DL.getTypeAllocSize(ElementType));

    auto *PrivMemcpy = HLNodeUtilities.createMemcpy(OrigAddr, ResAddr, Size);
    addInstUnmasked(PrivMemcpy);
    return;
  }

  case VPInstruction::PrivateFinalArrayMasked: {
    HLContainerTy PrivFinalArrayInsts;
    VPAllocatePrivate *Priv = cast<VPAllocatePrivate>(VPInst->getOperand(0));
    Type *ElementType = Priv->getAllocatedType();
    const DataLayout &DL = DDRefUtilities.getDataLayout();

    // We need to copy array from last private allocated memory into the
    // original array location.
    RegDDRef *Orig = getOrCreateScalarRef(VPInst->getOperand(1), 0);
    auto *OrigAddr = DDRefUtilities.createSelfAddressOfRef(
        ElementType, Orig->getSelfBlobIndex(), Orig->getDefinedAtLevel());
    OrigAddr->setAlignment(DL.getPrefTypeAlign(Orig->getSrcType()).value());

    // TODO: Add support for SOA layout in HIR (CMPLRLLVM-9193).
    assert(!Priv->isSOALayout() && "SOA layout is not supported for HIR.");

    // In case of non-SOA layout it will be enough to copy memory from last
    // private into the original array.
    RegDDRef *VecMask = widenRef(VPInst->getOperand(2), getVF());
    HLInst *BsfCall = createCTZCall(VecMask->clone(), Intrinsic::ctlz, true,
                                    &PrivFinalArrayInsts);
    RegDDRef *BsfLhs = BsfCall->getLvalDDRef();

    HLInst *SubInst = HLNodeUtilities.createSub(
        DDRefUtilities.createConstDDRef(BsfLhs->getDestType(), VF - 1),
        BsfLhs->clone(), "ext.lane");
    SubInst->getLvalDDRef()->makeSelfBlob();
    PrivFinalArrayInsts.push_back(*SubInst);

    RegDDRef *ResAddr = getOrCreateScalarRef(Priv, 0);
    auto IndexRef = SubInst->getLvalDDRef()->clone();
    ResAddr->addDimension(IndexRef->getSingleCanonExpr());
    ResAddr->setAlignment(Priv->getOrigAlignment().value());
    SmallVector<const RegDDRef *, 1> AuxRefs = {IndexRef};
    // Invoke makeConsitent() because dimensions of RegDDRef are modified.
    ResAddr->makeConsistent(AuxRefs, getNestingLevelFromInsertPoint());

    Type *SizeTy = Type::getInt64Ty(HLNodeUtilities.getContext());
    RegDDRef *Size = DDRefUtilities.createConstDDRef(
        SizeTy, DL.getTypeAllocSize(ElementType));

    auto *PrivMemcpy = HLNodeUtilities.createMemcpy(OrigAddr, ResAddr, Size);
    PrivFinalArrayInsts.push_back(*PrivMemcpy);
    addInst(&PrivFinalArrayInsts);
    return;
  }

  case VPInstruction::PrivateLastValueNonPOD: {
    RegDDRef *Orig = getOrCreateScalarRef(VPInst->getOperand(1), 0);
    VPAllocatePrivate *Priv = cast<VPAllocatePrivate>(VPInst->getOperand(0));
    RegDDRef *Res = getOrCreateScalarRef(Priv, getVF() - 1);
    auto *CopyAssignFn =
        cast<VPPrivateLastValueNonPODInst>(VPInst)->getCopyAssign();
    auto *PrivCopyAssign =
        HLNodeUtilities.createCall(CopyAssignFn, {Orig, Res});
    addInstUnmasked(PrivCopyAssign);
    return;
  }

  case VPInstruction::PrivateLastValueNonPODMasked: {
    // Pseudo HIR generated to non-POD masked last private entity,
    // private-last-value-nonpod-masked %exit, %orig, %mask
    // ==>
    // ; Obtain lane for extraction
    // %bsfintmask = bitcast.<2 x i1>.i2(%mask);
    // %lz = @llvm.ctlz.i2(%bsfintmask,  1);
    // %lane = VF - 1 - %lz;
    // ; Extract final value and store back to original
    // %priv_extract = extractelement %exit, %lane
    // call omp.copy_assign(%orig, %priv_extract)
    //
    HLContainerTy PrivLastValNonPODInsts;
    RegDDRef *VecMask = widenRef(VPInst->getOperand(2), getVF());
    HLInst *BsfCall = createCTZCall(VecMask->clone(), Intrinsic::ctlz, true,
                                    &PrivLastValNonPODInsts);
    RegDDRef *OrigPrivDescr = getOrCreateScalarRef(VPInst->getOperand(1), 0);
    RegDDRef *VecExit = widenRef(VPInst->getOperand(0), getVF());
    RegDDRef *BsfLhs = BsfCall->getLvalDDRef();

    HLInst *SubInst = HLNodeUtilities.createSub(
        DDRefUtilities.createConstDDRef(BsfLhs->getDestType(), VF - 1),
        BsfLhs->clone(), "ext.lane");
    PrivLastValNonPODInsts.push_back(*SubInst);

    HLInst *PrivExtract = HLNodeUtilities.createExtractElementInst(
        VecExit->clone(), SubInst->getLvalDDRef()->clone(), "priv.extract",
        nullptr);
    PrivLastValNonPODInsts.push_back(*PrivExtract);

    auto *CopyAssignFn =
        cast<VPPrivateLastValueNonPODMaskedInst>(VPInst)->getCopyAssign();
    HLInst *PrivCopyAssign = HLNodeUtilities.createCall(
        CopyAssignFn,
        {OrigPrivDescr->clone(), PrivExtract->getLvalDDRef()->clone()});
    PrivLastValNonPODInsts.push_back(*PrivCopyAssign);
    addInst(&PrivLastValNonPODInsts);
    return;
  }

  case VPInstruction::CompressExpandIndexInit: {
    // Generate a vector of zeros, but put instruction's operand into the first
    // position of the vector.
    Type *ElType = VPInst->getType();
    VectorType *VecType = getWidenedType(ElType, getVF());

    RegDDRef *Zeros =
        DDRefUtilities.createConstDDRef(Constant::getNullValue(VecType));
    RegDDRef *InitRef = getOrCreateScalarRef(VPInst->getOperand(0), 0);

    HLInst *InsertElementInst =
        HLNodeUtilities.createInsertElementInst(Zeros, InitRef, 0);
    addInstUnmasked(InsertElementInst);

    addVPValueWideRefMapping(VPInst, InsertElementInst->getLvalDDRef());
    return;
  }

  case VPInstruction::CompressExpandIndexFinal: {
    // CompressExpandIndexFinal returns its operand.
    RegDDRef *ScalRef = getOrCreateScalarRef(VPInst->getOperand(0), 0);
    addVPValueScalRefMapping(VPInst, ScalRef, 0);

    if (!hasNoExternalUsers(VPInst)) {
      const VPExternalUse *ExtUse = getSingleExternalUse(VPInst, Plan);
      RegDDRef *OutRef = getUniformScalarRef(ExtUse);
      auto *Inst = HLNodeUtilities.createCopyInst(ScalRef, ".copy", OutRef);
      addInstUnmasked(Inst);
    }
    return;
  }

  case VPInstruction::CompressStore: {
    // Just generate compress-store intrinsic, mask is the current execution
    // mask.
    //
    // compress-store %value, %ptr
    // =>
    // call <VF x type> @llvm.masked.compressstore.XXXX(<VF x type> %value, type* %ptr, <VF x i1> %mask)
    Type *ElType = VPInst->getOperand(0)->getType();
    VectorType *VecType = getWidenedType(ElType, getVF());
    Function *CompressStoreFunc =
        Intrinsic::getDeclaration(&HLNodeUtilities.getModule(),
                                  Intrinsic::masked_compressstore, {VecType});

    RegDDRef *ValueRef = widenRef(VPInst->getOperand(0), getVF());
    RegDDRef *PtrRef = getOrCreateScalarRef(VPInst->getOperand(1), 0);
    HLInst *CompressStoreCall = HLNodeUtilities.createCall(
        CompressStoreFunc, {ValueRef, PtrRef, getCurMaskValueOrAllOnes()},
        "comp.store");

    addInstUnmasked(CompressStoreCall);
    return;
  }

  case VPInstruction::ExpandLoad: {
    // Just generate expand-load intrinsic, mask is the current execution mask.
    //
    // type %ld = expand-load type, type* %ptr
    // =>
    // %Tmp = call <VF x type> @llvm.masked.expandload.XXXX(type* %ptr, <VF x i1> %Mask, <VF x type> undef)
    VectorType *VecType = getWidenedType(VPInst->getType(), getVF());
    Function *ExpandLoadFunc = Intrinsic::getDeclaration(
        &HLNodeUtilities.getModule(), Intrinsic::masked_expandload, {VecType});

    RegDDRef *PtrRef = getOrCreateScalarRef(VPInst->getOperand(0), 0);
    RegDDRef *Undef = DDRefUtilities.createUndefDDRef(VecType);

    HLInst *ExpandLoadCall = HLNodeUtilities.createCall(
        ExpandLoadFunc, {PtrRef, getCurMaskValueOrAllOnes(), Undef},
        "exp.load");
    addInstUnmasked(ExpandLoadCall);

    addVPValueWideRefMapping(VPInst, ExpandLoadCall->getLvalDDRef());
    return;
  }

  case VPInstruction::CompressStoreNonu: {
    // First, compress value to store, mask is the current execution mask.
    //
    // compress-store-nonu %value, %ptr
    // =>
    // %val.to.store = call <VF x type> @x86.masked.compress.XXXXX(<VF x type> %value, <VF x type> undef, <VF x i1> %Mask)
    Type *ElType = VPInst->getOperand(0)->getType();
    VectorType *VecType = getWidenedType(ElType, getVF());
    Function *CompressFunc = getCompressExpandIntrinsicDeclaration(
        Intrinsic::x86_avx512_mask_compress, VecType);

    RegDDRef *ValueRef = widenRef(VPInst->getOperand(0), getVF());
    RegDDRef *Undef = DDRefUtilities.createUndefDDRef(ValueRef->getDestType());
    HLInst *CompressCall = HLNodeUtilities.createCall(
        CompressFunc, {ValueRef, Undef, getCurMaskValueOrAllOnes()},
        "compress");
    addInstUnmasked(CompressCall);

    // Mask for strided store is the last instruction operand.
    RegDDRef *Mask = getOrCreateScalarRef(
        VPInst->getOperand(VPInst->getNumOperands() - 1), 0);

    // Last step, store all selected elements consecutively using index/ptr
    // calculated by CompressExpandIndex.
    //
    // call void @llvm.masked.scatter.XXXX.XXXX(<VF x type> %val.to.store, <VF x type*> %ptr, i32 align, <VF x i1> %exec.mask)
    RegDDRef *PtrRef = widenRef(VPInst->getOperand(1), 0);
    Function *ScatterFunc = Intrinsic::getDeclaration(
        &HLNodeUtilities.getModule(), Intrinsic::masked_scatter,
        {VecType, PtrRef->getDestType()});

    RegDDRef *Align = DDRefUtilities.createConstDDRef(Type::getInt32Ty(Context),
                                                      PtrRef->getAlignment());
    HLInst *ScatterCall = HLNodeUtilities.createCall(
        ScatterFunc,
        {CompressCall->getLvalDDRef()->clone(), PtrRef, Align, Mask},
        "scatter");
    addInstUnmasked(ScatterCall);
    return;
  }

  case VPInstruction::ExpandLoadNonu: {
    // Mask for strided load is the last instruction operand.
    RegDDRef *Mask = getOrCreateScalarRef(
        VPInst->getOperand(VPInst->getNumOperands() - 1), 0);

    // Load the needed number of elements from array B.
    //
    // %load.val = call void @llvm.masked.gather.XXXX.XXXX(<VF x type*> %ptr, i32 align, <VF x i1> %exec.mask)
    VectorType *VecType = getWidenedType(VPInst->getType(), getVF());
    RegDDRef *PtrRef = widenRef(VPInst->getOperand(0), 0);
    Function *GatherFunc = Intrinsic::getDeclaration(
        &HLNodeUtilities.getModule(), Intrinsic::masked_gather,
        {VecType, PtrRef->getDestType()});

    RegDDRef *Align = DDRefUtilities.createConstDDRef(Type::getInt32Ty(Context),
                                                      PtrRef->getAlignment());
    RegDDRef *Undef = DDRefUtilities.createUndefDDRef(VecType);
    HLInst *GatherCall = HLNodeUtilities.createCall(
        GatherFunc, {PtrRef, Align, Mask, Undef}, "gather");
    addInstUnmasked(GatherCall);

    // Last step, expand loaded value.
    //
    // %value = call <VF x type> @x86.masked.expand.XXXX(%load.val, <VF x type> undef, <VF x i1> %mask)
    Function *ExpandFunc = getCompressExpandIntrinsicDeclaration(
        Intrinsic::x86_avx512_mask_expand, VecType);
    HLInst *ExpandCall =
        HLNodeUtilities.createCall(ExpandFunc,
                                   {GatherCall->getLvalDDRef()->clone(),
                                    Undef->clone(), getCurMaskValueOrAllOnes()},
                                   "expand");
    addInstUnmasked(ExpandCall);

    addVPValueWideRefMapping(VPInst, ExpandCall->getLvalDDRef());
    return;
  }

  case VPInstruction::CompressExpandIndex: {
    // CompressExpandIndex creates a vector with stride-step sequence of values
    // and adds it to the original index.
    //
    // compress-expand-index %orig.index, %stride
    // =>
    // %bcast = broadcast %orig.index
    // %index = add %bcast, <0, %stride, %stride * 2, %stride * 3, ...>
    Type *ElType = VPInst->getType();
    VectorType *VecType = getWidenedType(ElType, getVF());

    RegDDRef *OrigIndex = widenRef(VPInst->getOperand(0), 0);
    RegDDRef *Undef = DDRefUtilities.createUndefDDRef(VecType);
    RegDDRef *Zeros =
        DDRefUtilities.createConstDDRef(Constant::getNullValue(VecType));

    HLInst *Bcast =
        HLNodeUtilities.createShuffleVectorInst(OrigIndex, Undef, Zeros);
    addInstUnmasked(Bcast);

    int64_t Stride = cast<VPConstant>(VPInst->getOperand(1))->getSExtValue();
    SmallVector<Constant *, 8> Offsets;
    for (unsigned I = 0; I < getVF(); I++)
      Offsets.push_back(ConstantInt::get(ElType, I * Stride));

    RegDDRef *OffsetsRef =
        DDRefUtilities.createConstDDRef(ConstantVector::get(Offsets));
    HLInst *Add =
        HLNodeUtilities.createAdd(Bcast->getLvalDDRef()->clone(), OffsetsRef);
    addInstUnmasked(Add);

    addVPValueWideRefMapping(VPInst, Add->getLvalDDRef());
    return;
  }

  case VPInstruction::CompressExpandIndexInc: {
    // Sum elements of resulting indices vector and put the resulting value into
    // the the first vector element. All the rest elements become zeros.
    VectorType *VecType = getWidenedType(VPInst->getType(), getVF());
    Function *VecReduceFunc = Intrinsic::getDeclaration(
        &HLNodeUtilities.getModule(), Intrinsic::vector_reduce_add, {VecType});

    RegDDRef *OrigIndex = widenRef(VPInst->getOperand(0), getVF());
    HLInst *VecReduceCall =
        HLNodeUtilities.createCall(VecReduceFunc, {OrigIndex}, "vec.reduce");
    addInstUnmasked(VecReduceCall);

    RegDDRef *Zeros =
        DDRefUtilities.createConstDDRef(Constant::getNullValue(VecType));
    HLInst *InsertElementInst = HLNodeUtilities.createInsertElementInst(
        Zeros, VecReduceCall->getLvalDDRef()->clone(), 0);
    addInstUnmasked(InsertElementInst);

    addVPValueWideRefMapping(VPInst, InsertElementInst->getLvalDDRef());
    return;
  }

  case VPInstruction::CompressExpandMask: {
    RegDDRef *Mask = generateMaskForCompressExpandLoadStoreNonu();
    addVPValueScalRefMapping(VPInst, Mask, 0);
    return;
  }

  default:
    llvm_unreachable("Unsupported VPLoopEntity instruction.");
  }
}

RegDDRef *VPOCodeGenHIR::getCurMaskValueOrAllOnes() const {

  return CurMaskValue ? CurMaskValue->clone()
                      : getConstantSplatDDRef(
                            DDRefUtilities,
                            Constant::getAllOnesValue(Type::getInt1Ty(Context)),
                            getVF());
}

RegDDRef *VPOCodeGenHIR::generateMaskForCompressExpandLoadStoreNonu() {

  if (!CurMaskValue)
    return getConstantSplatDDRef(
        DDRefUtilities, Constant::getAllOnesValue(Type::getInt1Ty(Context)),
        getVF());

  // Calculate mask for strided load/store. We store first consecutive elements
  // from compressed value, their number is equal to number of bits in the
  // current execution mask.
  // Note: four first instructions below are scalar.
  //
  // %mask.i = bitcast <VF x i1> %mask to i8
  // %mask.i.popcnt = call i8 @llvm.ctpop.i8(i8 %mask.i)
  // %store.mask.high.bits = shl i64 -1, i64 %mask.i.popcnt
  // %store.mask = xor i64 %store.mask.high.bits, -1
  // %exec.mask = bitcast i64 %store.mask to <VF x i1>

  Type *IntTy = IntegerType::get(Context, getVF());
  HLInst *BitCastInst = createBitCast(IntTy, CurMaskValue);
  RegDDRef *IntRef = BitCastInst->getLvalDDRef();

  Function *CtpopFunc = Intrinsic::getDeclaration(&HLNodeUtilities.getModule(),
                                                  Intrinsic::ctpop, {IntTy});
  HLInst *CtpopCall =
      HLNodeUtilities.createCall(CtpopFunc, {IntRef->clone()}, "popcnt");
  addInstUnmasked(CtpopCall);

  RegDDRef *PopCnt = CtpopCall->getLvalDDRef();
  RegDDRef *NegOne = DDRefUtilities.createConstDDRef(PopCnt->getDestType(), -1);
  HLInst *Shl = HLNodeUtilities.createShl(NegOne, PopCnt->clone());
  addInstUnmasked(Shl);

  HLInst *Xor =
      HLNodeUtilities.createXor(Shl->getLvalDDRef()->clone(), NegOne->clone());
  addInstUnmasked(Xor);

  Type *VectorTy = FixedVectorType::get(Type::getInt1Ty(Context), getVF());
  HLInst *BitCastVec = createBitCast(VectorTy, Xor->getLvalDDRef());

  return BitCastVec->getLvalDDRef()->clone();
}

Function *
VPOCodeGenHIR::getCompressExpandIntrinsicDeclaration(Intrinsic::ID IntrinsicId,
                                                     VectorType *VecType) {

  assert(IntrinsicId == Intrinsic::x86_avx512_mask_compress ||
         IntrinsicId == Intrinsic::x86_avx512_mask_expand &&
             "Expected compress/expand intrinsic ID.");
  assert(TTI->hasVLX() &&
         "AVX-512VL instructions are expected to be available.");

  // For the correct intrinsics code generation +avx512f,+avx512vl attributes
  // are required (could be copied from the current function).
  Function *F = Intrinsic::getDeclaration(&HLNodeUtilities.getModule(),
                                          IntrinsicId, {VecType});
  for (StringRef AttrName : {"target-cpu", "target-features"})
    F->addFnAttr(AttrName,
                 getFunction().getFnAttribute(AttrName).getValueAsString());

  // Too small min-legal-vector-width attribute value could prevent proper
  // @llvm.x86.avx512.mask.compress.xxx/@llvm.x86.avx512.mask.expand.xxx
  // intrinsics code generation, so upgrading it.
  AttributeFuncs::updateMinLegalVectorWidthAttr(getFunction(), 512);

  return F;
}

template <typename FinalInstType>
void VPOCodeGenHIR::insertPrivateFinalCond(const VPInstruction *VPInst) {
  auto *CondPrivateFinal = cast<FinalInstType>(VPInst);
  RegDDRef *VecExit = widenRef(CondPrivateFinal->getExit(), getVF());
  RegDDRef *VecIndex = widenRef(CondPrivateFinal->getIndex(), getVF());

  HLContainerTy CondPrivFinalInsts;

  Function *IdxReduceFunc = Intrinsic::getDeclaration(
      &HLNodeUtilities.getModule(), Intrinsic::vector_reduce_smax,
      {VecIndex->getDestType()});
  HLInst *IdxReduceCall = HLNodeUtilities.createCall(
      IdxReduceFunc, {VecIndex->clone()}, "priv.idx.max");
  CondPrivFinalInsts.push_back(*IdxReduceCall);

  RegDDRef *MaxIdxBcast =
      widenRef(IdxReduceCall->getLvalDDRef()->clone(), getVF());
  HLInst *CmpInst = HLNodeUtilities.createCmp(
      PredicateTy::ICMP_EQ, VecIndex->clone(), MaxIdxBcast, "priv.idx.cmp");
  CondPrivFinalInsts.push_back(*CmpInst);

  HLInst *BsfCall = createCTZCall(CmpInst->getLvalDDRef()->clone(),
                                  Intrinsic::cttz, true, &CondPrivFinalInsts);

  // Scalar result of private finalization should be written back to original
  // private descriptor variable.
  RegDDRef *OrigPrivDescr = nullptr;
  if (std::is_same<FinalInstType, VPPrivateFinalCond>::value)
    OrigPrivDescr = getUniformScalarRef(CondPrivateFinal->getOrig());
  HLInst *PrivExtract = HLNodeUtilities.createExtractElementInst(
      VecExit->clone(), BsfCall->getLvalDDRef()->clone(), "priv.extract",
      OrigPrivDescr);
  CondPrivFinalInsts.push_back(*PrivExtract);

  // Make the original private descriptor non-linear since we have a
  // definition to the temp in loop post-exit.
  PrivExtract->getLvalDDRef()->getSingleCanonExpr()->setNonLinear();

  addInst(&CondPrivFinalInsts);
  addVPValueScalRefMapping(VPInst, PrivExtract->getLvalDDRef(), 0 /*Lane*/);
  return;
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

void VPOCodeGenHIR::generateUniformScalarLoad(const VPLoadStoreInst *VPLoad) {
  // For a uniform load, do a scalar load.
  RegDDRef *MemRef = getMemoryRef(VPLoad, true /* Lane0Value */);
  auto *ScalarInst = HLNodeUtilities.createLoad(MemRef, ".unifload");
  addInstUnmasked(ScalarInst);
  addVPValueScalRefMapping(VPLoad, ScalarInst->getLvalDDRef(), 0);
}

void VPOCodeGenHIR::widenUnmaskedUniformStoreImpl(
    const VPLoadStoreInst *VPStore) {
  const VPValue *ValOp = VPStore->getOperand(0);
  RegDDRef *MemRef = getMemoryRef(VPStore, true /* Lane0Value */);
  RegDDRef *ValRef = nullptr;
  if (!Plan->getVPlanDA()->isDivergent(*ValOp))
    // Value being stored is uniform - use lane 0 value as the value to store.
    ValRef = getOrCreateScalarRef(ValOp, 0 /*LaneID*/);
  else {
    const VPPHINode *VPPhi = dyn_cast<VPPHINode>(ValOp);
    if (VPPhi && LoopIVPhis.count(VPPhi)) {
      // If the value being stored is the loop IV, generate IV + (VF - 1).
      auto RefDestTy = VPPhi->getType();
      auto *CE = CanonExprUtilities.createCanonExpr(RefDestTy);
      CE->addIV(OrigLoop->getNestingLevel(), InvalidBlobIndex /* no blob */,
                1 /* constant IV coefficient */);
      CE->addConstant(getVF() - 1, false /* IsMathAdd */);
      ValRef = DDRefUtilities.createScalarRegDDRef(GenericRvalSymbase, CE);
    } else {
      // Value being stored is divergent non loop IV - use last lane value as
      // the value to store.
      ValRef = getOrCreateScalarRef(ValOp, getVF() - 1);
    }
  }

  HLInst *WideInst =
      HLNodeUtilities.createStore(ValRef, "uniform.store", MemRef);
  addInstUnmasked(WideInst);
}

void VPOCodeGenHIR::widenLoadStoreImpl(const VPLoadStoreInst *VPLoadStore,
                                       RegDDRef *Mask) {
  // Loads/stores need to be masked with current mask value if Mask is null.
  if (!Mask)
    Mask = CurMaskValue;

  auto Opcode = VPLoadStore->getOpcode();

  const VPValue *PtrOp = VPLoadStore->getPointerOperand();
  if (!Plan->getVPlanDA()->isDivergent(*PtrOp)) {
    // Handle uniform load
    if (Opcode == Instruction::Load) {
      scalarizePredicatedUniformInst(VPLoadStore, Mask);
      return;
    } else if (!Mask) {
      // Handle unmasked uniform store
      widenUnmaskedUniformStoreImpl(VPLoadStore);
      return;
    }
  }

  RegDDRef *MemRef = getMemoryRef(VPLoadStore);
  HLInst *WInst;

  // Reverse mask for negative -1 stride.
  bool IsNegOneStride;
  bool IsUnitStride = Plan->getVPlanDA()->isUnitStridePtr(
      PtrOp, VPLoadStore->getValueType(), IsNegOneStride);
  if (Mask && IsNegOneStride) {
    auto *RevInst = createReverseVector(Mask->clone());
    Mask = RevInst->getLvalDDRef();
  }

  // If the load/store element type is a vector, we need to replicate mask
  // vector elements if non-null.
  if (Mask) {
    auto *ValueTy = VPLoadStore->getValueType();
    if (auto *ValueVecTy = dyn_cast<FixedVectorType>(ValueTy)) {
      HLInst *ReplMaskInst =
          replicateVectorElts(Mask, ValueVecTy->getNumElements());
      addInstUnmasked(ReplMaskInst);
      Mask = ReplMaskInst->getLvalDDRef();
    }
  }

  auto &OptRptStats = getOptReportStats(VPLoadStore);

  if (Opcode == Instruction::Load) {
    if (IsUnitStride)
      ++(Mask ? OptRptStats.MaskedUnalignedUnitStrideLoads
              : OptRptStats.UnmaskedUnalignedUnitStrideLoads);
    else
      ++(Mask ? OptRptStats.MaskedGathers : OptRptStats.UnmaskedGathers);

    WInst = HLNodeUtilities.createLoad(MemRef, ".vec");
    addInst(WInst, Mask);
    // Reverse the loaded value for negative -1 stride.
    if (IsNegOneStride)
      WInst = createReverseVector(WInst->getLvalDDRef()->clone());
    addVPValueWideRefMapping(VPLoadStore, WInst->getLvalDDRef());
  } else {
    if (IsUnitStride)
      ++(Mask ? OptRptStats.MaskedUnalignedUnitStrideStores
              : OptRptStats.UnmaskedUnalignedUnitStrideStores);
    else
      ++(Mask ? OptRptStats.MaskedScatters : OptRptStats.UnmaskedScatters);

    RegDDRef *StoreVal = widenRef(VPLoadStore->getOperand(0), getVF());
    // Reverse the value to be stored for negative -1 stride.
    if (IsNegOneStride) {
      WInst = createReverseVector(StoreVal);
      StoreVal = WInst->getLvalDDRef()->clone();
    }
    WInst = HLNodeUtilities.createStore(StoreVal, ".vec", MemRef);
    addInst(WInst, Mask);
    // Stores are not added to widen/scalar maps. Explicitly set debug location
    // on lval memref.
    WInst->getLvalDDRef()->setMemDebugLoc(VPLoadStore->getDebugLocation());
  }
}

void VPOCodeGenHIR::generateHIRForSubscript(const VPSubscriptInst *VPSubscript,
                                            RegDDRef *Mask, bool Widen,
                                            unsigned ScalarLaneID) {
  auto Dim0 = VPSubscript->dim(0);
  Type *DestTy = Dim0.DimElementType;
  assert(DestTy && "Expected an actual destination type.");
  for (uint64_t Idx : Dim0.StructOffsets) {
    DestTy = GetElementPtrInst::getTypeAtIndex(DestTy, Idx);
    assert(DestTy && "Expected an actual destination type.");
  }
  Type *ResultRefTy = DestTy;
  if (Widen) {
    // See documentation for
    //
    //   Type *BitCastDestVecOrElemTy;
    //
    // in loopopt::RegDDRef. For a vector of pointers they want that vector
    // instead of element types as "an exception".
    ResultRefTy = getWidenedType(
        DestTy->getScalarType()->getPointerTo(
            cast<PointerType>(VPSubscript->getType())->getAddressSpace()),
        VF);
  }

  RegDDRef *PointerRef = getOrCreateRefForVPVal(
      VPSubscript->getPointerOperand(), Widen, ScalarLaneID);
  // Base canon expression needs to be a self blob.
  if (!PointerRef->isSelfBlob()) {
    auto *CopyInst = HLNodeUtilities.createCopyInst(PointerRef, "nsbgepcopy");
    addInst(CopyInst, Mask);
    PointerRef = CopyInst->getLvalDDRef();
  }

  auto *NewRef = DDRefUtilities.createAddressOfRef(
      VPSubscript->dim(VPSubscript->getNumDimensions() - 1).DimElementType,
      PointerRef->getSelfBlobIndex(), PointerRef->getDefinedAtLevel());
  NewRef->setInBounds(VPSubscript->isInBounds());
  NewRef->setBitCastDestVecOrElemType(ResultRefTy);
  SmallVector<const RegDDRef *, 4> AuxRefs;

  // Utility to specially handle lower/stride fields of a subscript. We ensure
  // that loop invariant lower/stride are kept scalar even in vectorized memref.
  auto generateLowerOrStride = [this](VPValue *V, bool Widen,
                                      unsigned ScalarLaneID) {
    if (!Plan->getVPlanDA()->isDivergent(*V))
      return getOrCreateRefForVPVal(V, false /*always scalar*/, 0 /*LaneID*/);
    return getOrCreateRefForVPVal(V, Widen, ScalarLaneID);
  };

  for (int Dim = VPSubscript->getNumDimensions() - 1; Dim >= 0; --Dim) {
    auto DimInfo = VPSubscript->dim(Dim);
    RegDDRef *Lower =
        generateLowerOrStride(DimInfo.LowerBound, Widen, ScalarLaneID);
    RegDDRef *Stride =
        generateLowerOrStride(DimInfo.StrideInBytes, Widen, ScalarLaneID);
    RegDDRef *Idx =
        getOrCreateRefForVPVal(DimInfo.Index, Widen, ScalarLaneID);
    AuxRefs.insert(AuxRefs.end(), {Idx, Lower, Stride});
    ArrayRef<unsigned> StructOffsets = DimInfo.StructOffsets;
    Type *DimTy = DimInfo.DimType;
    Type *DimElemTy = DimInfo.DimElementType;
    NewRef->addDimension(Idx->getSingleCanonExpr(), StructOffsets,
                         Lower->getSingleCanonExpr(),
                         Stride->getSingleCanonExpr(), DimTy, DimElemTy);
  }

  makeConsistentAndAddToMap(NewRef, VPSubscript, AuxRefs, Widen, ScalarLaneID);
  LLVM_DEBUG(dbgs() << "[VPOCGHIR] NewMemRef: "; NewRef->dump(1);
             dbgs() << "\n");
}

RegDDRef *VPOCodeGenHIR::getVLSLoadStoreMask(VectorType *WideValueType,
                                             int GroupSize) {
  unsigned NumElements = cast<FixedVectorType>(WideValueType)->getNumElements();
  if (!CurMaskValue) {
    if (VF * GroupSize == NumElements)
      // No mask needed.
      return nullptr;

    auto *True = ConstantInt::getTrue(WideValueType->getContext());
    auto *False = ConstantInt::getFalse(WideValueType->getContext());
    SmallVector<Constant *, 32> Mask;
    unsigned Idx;
    for (Idx = 0; Idx < VF*GroupSize; ++Idx)
      Mask.push_back(True);
    for (; Idx < NumElements; ++Idx)
      Mask.push_back(False);

    return DDRefUtilities.createConstDDRef(ConstantVector::get(Mask));
  }

  auto *Int32Ty = Type::getInt32Ty(WideValueType->getContext());
  SmallVector<Constant *, 32> ShuffleMask;
  for (unsigned Lane = 0; Lane < VF; ++Lane)
    for (int Elt = 0; Elt < GroupSize; ++Elt)
      ShuffleMask.push_back(ConstantInt::get(Int32Ty, Lane));

  for (unsigned Idx = VF * GroupSize; Idx < NumElements; ++Idx)
    ShuffleMask.push_back(ConstantInt::get(Int32Ty, VF));

  auto *ShuffleMaskRef =
      DDRefUtilities.createConstDDRef(ConstantVector::get(ShuffleMask));

  auto *False = DDRefUtilities.createConstDDRef(
      ConstantInt::getFalse(CurMaskValue->getDestType()));

  HLInst *Shuffle = HLNodeUtilities.createShuffleVectorInst(
      CurMaskValue->clone(), False, ShuffleMaskRef, "vls.mask");
  addInstUnmasked(Shuffle);

  return Shuffle->getLvalDDRef();
}

bool VPOCodeGenHIR::serializeDivRem(const VPInstruction *VPInst, RegDDRef *Mask,
                                    OptReportStatsTracker &OptRptStats) {
  bool DivisorIsSafe = isDivisorSpeculationSafeForDivRem(VPInst->getOpcode(),
                                                         VPInst->getOperand(1));
  if (Mask && !DivisorIsSafe) {
    if (Plan->getVPlanDA()->isUniform(*VPInst)) {
      scalarizePredicatedUniformInst(VPInst, Mask);
      return true;
    } else if (!EnableIntDivRemBlendWithSafeValue) {
      serializeInstruction(VPInst, Mask);
      // Remark: division was scalarized due to fp-model requirements
      OptRptStats.SerializedInstRemarks.emplace_back(
          15566, Instruction::getOpcodeName(VPInst->getOpcode()));
      return true;
    }
  }
  return false;
}

static bool isIntDivRemOpcode(unsigned Opcode) {
  switch (Opcode) {
  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::URem:
  case Instruction::SRem:
    return true;
  default:
    return false;
  }
}

void VPOCodeGenHIR::generateHIR(const VPInstruction *VPInst, RegDDRef *Mask,
                                bool Widen, unsigned ScalarLaneID,
                                bool OnlyExistingScalarOps) {
  assert((Widen || isOpcodeForScalarInst(VPInst->getOpcode()) ||
          isIntDivRemOpcode(VPInst->getOpcode()) ||
          isa<VPCallInstruction>(VPInst) || isa<VPHIRCopyInst>(VPInst)) &&
         "Unxpected instruction for scalar constructs");

  HLInst *NewInst = nullptr;
  const Twine InstName = Widen ? ".vec" : ".scal";

  if (auto *VPPhi = dyn_cast<VPPHINode>(VPInst)) {
    widenPhiImpl(VPPhi, Mask);
    return;
  }

  if (auto *Blend = dyn_cast<VPBlendInst>(VPInst)) {
    widenBlendImpl(Blend, Mask);
    return;
  }

  // TODO: VPValue based code generation should stop relying on underlying
  // IR validity information. The following special casing is expected to
  // be removed as part of the move to merged CFG currently in progress.
  // To avoid duplication of this work for the non-merged CFG path, we avoid
  // generating code for loop related instructions outside the outermost
  // loop as the loop generation is done using HIR utilities in this path.
  if (VPInst->isUnderlyingIRValid()) {
    const HLNode *HNode =
        VPInst->HIR().isMaster()
            ? VPInst->HIR().getUnderlyingNode()
            : VPInst->HIR().getMaster()->HIR().getUnderlyingNode();
    if (isa<HLLoop>(HNode) && HNode == OrigLoop) {
      // Ignore backedge compare.
      if (VPInst->getOpcode() == Instruction::ICmp)
        return;

      if (isSearchLoop() &&
          !Plan->getVPLoopInfo()->getLoopFor(VPInst->getParent()))
        return;
    }
  }

  auto &OptRptStats = getOptReportStats(VPInst);

  auto Opcode = VPInst->getOpcode();
  switch (Opcode) {
  case Instruction::Load:
  case Instruction::Store:
    widenLoadStoreImpl(cast<VPLoadStoreInst>(VPInst), Mask);
    return;
  case VPInstruction::Subscript:
    generateHIRForSubscript(cast<VPSubscriptInst>(VPInst), Mask, Widen,
                            ScalarLaneID);
    return;
  case VPInstruction::ReductionInit:
  case VPInstruction::ReductionFinal:
  case VPInstruction::ReductionFinalUdr:
  case VPInstruction::InductionInit:
  case VPInstruction::InductionInitStep:
  case VPInstruction::InductionFinal:
  case VPInstruction::AllocatePrivate:
  case VPInstruction::PrivateFinalUncond:
  case VPInstruction::PrivateFinalUncondMem:
  case VPInstruction::PrivateFinalCondMem:
  case VPInstruction::PrivateFinalCond:
  case VPInstruction::PrivateFinalMaskedMem:
  case VPInstruction::PrivateFinalMasked:
  case VPInstruction::PrivateFinalArray:
  case VPInstruction::PrivateFinalArrayMasked:
  case VPInstruction::PrivateLastValueNonPOD:
  case VPInstruction::PrivateLastValueNonPODMasked:
  case VPInstruction::CompressStore:
  case VPInstruction::CompressStoreNonu:
  case VPInstruction::ExpandLoad:
  case VPInstruction::ExpandLoadNonu:
  case VPInstruction::CompressExpandIndexInit:
  case VPInstruction::CompressExpandIndexFinal:
  case VPInstruction::CompressExpandIndex:
  case VPInstruction::CompressExpandIndexInc:
  case VPInstruction::CompressExpandMask:
    widenLoopEntityInst(VPInst);
    return;
  case Instruction::ShuffleVector: {
    if (!Plan->getVPlanDA()->isDivergent(*VPInst)) {
      auto *Op0 = getOrCreateScalarRef(VPInst->getOperand(0), 0);
      auto *Op1 = getOrCreateScalarRef(VPInst->getOperand(1), 0);
      auto *Mask = getOrCreateScalarRef(VPInst->getOperand(2), 0);
      auto *Shuffle = HLNodeUtilities.createShuffleVectorInst(Op0, Op1, Mask);
      addInstUnmasked(Shuffle);
      addVPValueScalRefMapping(VPInst, Shuffle->getLvalDDRef(), 0);
      return;
    }

    auto *Op0 = widenRef(VPInst->getOperand(0), VF);
    auto *Op1 = widenRef(VPInst->getOperand(1), VF);
    auto *Mask = cast<VPConstant>(VPInst->getOperand(2))->getConstant();

    unsigned OrigSrcVL = cast<FixedVectorType>(VPInst->getOperand(0)->getType())
                             ->getNumElements();
    int OrigDstVL = cast<FixedVectorType>(VPInst->getType())->getNumElements();

    SmallVector<Constant *, 16> MaskIndices;
    for (unsigned LogicalLane = 0; LogicalLane < VF; ++LogicalLane) {
      for (int Idx = 0; Idx < OrigDstVL; ++Idx) {
        auto *MaskElt = Mask->getAggregateElement(Idx);
        if (isa<UndefValue>(MaskElt) || isa<PoisonValue>(MaskElt)) {
          // From the LangRef: If the shuffle mask selects an undefined element
          // from one of the input vectors, the resulting element is undefined.
          // An undefined element in the mask vector specifies that the
          // resulting element is undefined. An undefined element in the mask
          // vector prevents a poisoned vector element from propagating.
          //
          // For poison: Vector elements may be independently poisoned.
          // Therefore, transforms on instructions such as shufflevector must be
          // careful to propagate poison across values or elements only as
          // allowed by the original code... An instruction that depends on a
          // poison value, produces a poison value itself.
          MaskIndices.push_back(MaskElt);
          continue;
        }
        unsigned OrigIdx = cast<ConstantInt>(MaskElt)->getZExtValue();
        unsigned NewIdx;
        if (OrigIdx < OrigSrcVL) {
          NewIdx = OrigSrcVL * LogicalLane + OrigIdx;
        } else {
          NewIdx =
              OrigSrcVL * VF + OrigSrcVL * LogicalLane + (OrigIdx - OrigSrcVL);
        }
        MaskIndices.push_back(ConstantInt::get(MaskElt->getType(), NewIdx));
      }
    }
    auto *Shuffle = HLNodeUtilities.createShuffleVectorInst(
        Op0, Op1,
        DDRefUtilities.createConstDDRef(ConstantVector::get(MaskIndices)));
    addInstUnmasked(Shuffle);
    addVPValueWideRefMapping(VPInst, Shuffle->getLvalDDRef());

    return;
  }
  case VPInstruction::VLSLoad: {
    auto *VLSLoad = cast<VPVLSLoad>(VPInst);
    RegDDRef *MemRef = getVLSMemoryRef(VLSLoad);
    HLInst *WideLoad = HLNodeUtilities.createLoad(MemRef, ".vls.load");
    for (unsigned FakeSymbase : VLSLoad->HIR().fakeSymbases()) {
      RegDDRef *FakeRef = MemRef->clone();
      FakeRef->setSymbase(FakeSymbase);
      WideLoad->addFakeRvalDDRef(FakeRef);
    }
    RegDDRef *Mask = getVLSLoadStoreMask(cast<VectorType>(VPInst->getType()),
                                         VLSLoad->getGroupSize());
    addInst(WideLoad, Mask);

    if (Mask)
      OptRptStats.MaskedVLSLoads += VLSLoad->getNumOrigLoads();
    else
      OptRptStats.UnmaskedVLSLoads += VLSLoad->getNumOrigLoads();

    addVPValueScalRefMapping(VLSLoad, WideLoad->getLvalDDRef(), 0);
    return;
  }
  case VPInstruction::VLSExtract: {
    auto *Extract = cast<VPVLSExtract>(VPInst);

    auto NumEltsPerValue = Extract->getNumGroupEltsPerValue();

    auto Offset = Extract->getOffset();
    auto GroupSize = Extract->getGroupSize();

    auto *Int32Ty = Type::getInt32Ty(Extract->getType()->getContext());

    SmallVector<Constant *, 32> ShuffleMask;
    for (unsigned Lane = 0; Lane < VF; ++Lane)
      for (unsigned Part = 0; Part < NumEltsPerValue; ++Part)
        ShuffleMask.push_back(
            ConstantInt::get(Int32Ty, Lane * GroupSize + Offset + Part));

    RegDDRef *WideValue = getOrCreateScalarRef(Extract->getOperand(0), 0);
    HLInst *Shuffle = HLNodeUtilities.createShuffleVectorInst(
        WideValue, WideValue->clone(),
        DDRefUtilities.createConstDDRef(ConstantVector::get(ShuffleMask)),
        "vls.extract");
    addInstUnmasked(Shuffle);

    auto *ResultRef = Shuffle->getLvalDDRef();
    auto *ResultType = getWidenedType(Extract->getType(), VF);
    if (ResultType != ResultRef->getDestType()) {
      auto *Cast = HLNodeUtilities.createBitCast(ResultType, ResultRef->clone(),
                                                 "vls.extract.cast");
      addInstUnmasked(Cast);
      ResultRef = Cast->getLvalDDRef();
    }
    addVPValueWideRefMapping(Extract, ResultRef);

    return;
  }
  case VPInstruction::VLSInsert: {
    auto *Insert = cast<VPVLSInsert>(VPInst);
    RegDDRef *OrigWideValue = getOrCreateScalarRef(Insert->getOperand(0), 0);
    RegDDRef *ValueToInsert = widenRef(Insert->getOperand(1), VF);

    auto NumEltsPerValue = Insert->getNumGroupEltsPerValue();
    auto *GroupType = cast<FixedVectorType>(Insert->getOperand(0)->getType());
    Type *GroupEltType = GroupType->getElementType();
    auto NumEltsInGroup = GroupType->getNumElements();

    auto *CastedType = getWidenedType(GroupEltType, VF * NumEltsPerValue);
    RegDDRef *Casted = ValueToInsert;
    if (CastedType != Casted->getDestType()) {
      auto *Cast =
          HLNodeUtilities.createBitCast(CastedType, Casted, "vls.insert.cast");
      addInstUnmasked(Cast);
      Casted = Cast->getLvalDDRef()->clone();
    }

    auto *Extend = extendVector(Casted, NumEltsInGroup);
    addInstUnmasked(Extend);

    auto Offset = Insert->getOffset();
    auto GroupSize = Insert->getGroupSize();
    auto *Int32Ty = Type::getInt32Ty(Insert->getType()->getContext());
    SmallVector<Constant *, 32> ShuffleMask;

    // Initialize as if we'd want to keep OrigWideValue only.
    for (unsigned Idx = 0; Idx < NumEltsInGroup; ++Idx)
      ShuffleMask.push_back(ConstantInt::get(Int32Ty, Idx));

    // Now update indices where we'd like to change the data.
    for (unsigned Lane = 0; Lane < VF; ++Lane)
      for (unsigned Part = 0; Part < NumEltsPerValue; ++Part) {
        auto TargetIdx = GroupSize * Lane + Offset + Part;
        auto SrcIdx = NumEltsInGroup + Lane * NumEltsPerValue + Part;
        ShuffleMask[TargetIdx] = ConstantInt::get(Int32Ty, SrcIdx);
      }

    auto *Result = HLNodeUtilities.createShuffleVectorInst(
        OrigWideValue, Extend->getLvalDDRef()->clone(),
        DDRefUtilities.createConstDDRef(ConstantVector::get(ShuffleMask)));
    addInstUnmasked(Result);
    addVPValueScalRefMapping(Insert, Result->getLvalDDRef(), 0);
    return;
  }
  case VPInstruction::VLSStore: {
    auto *VLSStore = cast<VPVLSStore>(VPInst);
    RegDDRef *MemRef = getVLSMemoryRef(VLSStore);
    HLInst *WideStore = HLNodeUtilities.createStore(
        getOrCreateScalarRef(VPInst->getOperand(0), 0), ".vls.store", MemRef);
    for (unsigned FakeSymbase : VLSStore->HIR().fakeSymbases()) {
      RegDDRef *FakeRef = MemRef->clone();
      FakeRef->setSymbase(FakeSymbase);
      WideStore->addFakeLvalDDRef(FakeRef);
    }
    RegDDRef *Mask = getVLSLoadStoreMask(
        cast<VectorType>(VLSStore->getOperand(0)->getType()),
        VLSStore->getGroupSize());
    addInst(WideStore, Mask);
    if (Mask)
      OptRptStats.MaskedVLSStores += VLSStore->getNumOrigStores();
    else
      OptRptStats.UnmaskedVLSStores += VLSStore->getNumOrigStores();

    return;
  }
  case Instruction::Br:
    // Do nothing.
    return;

  case VPInstruction::HIRCopy: {
    int OriginPhiId = cast<VPHIRCopyInst>(VPInst)->getOriginPhiId();
    VPValue *CopiedVPVal = VPInst->getOperand(0);
    RegDDRef *LValTmp = nullptr;
    if (OriginPhiId != -1) {
      if (auto *ExistingTmp = getLValTempForPhiId(OriginPhiId)) {
        // If a HIR temp was already created for this PHI ID, then re-use it
        // as Lval.
        LValTmp = ExistingTmp->clone();
      } else {
        // First occurrence of PHI ID, create a new HIR temp to be used as
        // Lval for all copies.
        Type *TmpTy = Widen ? getWidenedType(VPInst->getType(), getVF())
                            : VPInst->getType();
        LValTmp = HLNodeUtilities.createTemp(TmpTy, "phi.temp");
        PhiIdLValTempsMap[OriginPhiId] = LValTmp;
      }
    }

    RegDDRef *CopiedRef = Widen
                              ? widenRef(CopiedVPVal, getVF())
                              : getOrCreateScalarRef(CopiedVPVal, ScalarLaneID);

    HLInst *NewInst =
        HLNodeUtilities.createCopyInst(CopiedRef, ".copy", LValTmp);

    // If this is a copy of reduction-init value then it should be inserted
    // along with other reduction initialization related instructions i.e based
    // on hoist loop. We also conservatively mark the copy's temp as live-in for
    // loop nest.
    if (isa<VPReductionInit>(CopiedVPVal)) {
      assert(Widen &&
             "Copies from reduction-init expected to be widened only.");
      HLContainerTy RedInitCopy;
      RedInitCopy.push_back(*NewInst);
      insertReductionInit(&RedInitCopy);

      // Add the reduction init ref as a live-in for each loop up to and
      // including the hoist loop starting from MainLoop's parent loop. The
      // livein information for MainLoop will be added by setupLiveInLiveOut
      // call.
      auto LvalSymbase = NewInst->getLvalDDRef()->getSymbase();
      assert(LValTmp && LValTmp->getSymbase() == LvalSymbase &&
             "Inconsistent lval/symbase used for copies.");
      HLLoop *ThisLoop = OrigLoop->getParentLoop();
      while (ThisLoop != RednHoistLp->getParentLoop()) {
        ThisLoop->addLiveInTemp(LvalSymbase);
        ThisLoop = ThisLoop->getParentLoop();
      }

      addVPValueWideRefMapping(VPInst, NewInst->getLvalDDRef());
      return;
    }

    auto IsExtDefUsedForCondLastPriv = [](VPExternalDef *ExtDef) {
      return llvm::any_of(ExtDef->users(),
                          [](VPUser *U) { return isa<VPPrivateFinalCond>(U); });
    };

    // Special case handling of copies generated for conditional last private
    // recurrent PHIs. Conditional last private entity is recognized by
    // inspecting the RHS value of this hir-copy instruction.
    // TODO: This special casing should be removed after updating VPLoopEntities
    // & PrivateFinalInstruction processing transforms to use undef to intialize
    // conditional last privates instead of original start value.
    if (isa<VPExternalDef>(CopiedVPVal) &&
        IsExtDefUsedForCondLastPriv(cast<VPExternalDef>(CopiedVPVal))) {
      auto *RValTmp = NewInst->getRvalDDRef();
      if (RValTmp->isSelfBlob()) {
        unsigned RvalSym = RValTmp->getSymbase();
        // Make the r-val temp of pre-loop copies non-linear if we know that
        // temp will be liveout of current loop. Pre-loop copies are introduced
        // by deconstruction of loop header reccurrent PHIs. Liveout r-val temps
        // have to be made non-linear since they will have a definition in the
        // loop post-exit (finalization) -
        // DO i1
        //     %vec = %t
        //   DO i2
        //     %vec =
        //   END DO
        //     %t = %vec // This definition makes %t non-linear
        // END DO
        if (MainLoop->isLiveOut(RvalSym)) {
          auto *RvalTempCE = RValTmp->getSingleCanonExpr();
          RvalTempCE->setNonLinear();
        }
        // Since we are adding a new r-val use of temp outside the loop, make
        // the temp live-in at all parent loop levels.
        makeSymLiveInForParentLoops(RvalSym);
      }
      addInstUnmasked(NewInst);
      addVPValueRefToMaps(VPInst, NewInst->getLvalDDRef(), Widen, ScalarLaneID);
      return;
    }

    addInst(NewInst, Mask);
    addVPValueRefToMaps(VPInst, NewInst->getLvalDDRef(), Widen, ScalarLaneID);

    return;
  }

  case VPInstruction::VectorTripCountCalculation: {
    auto *VPVectorTC = cast<VPVectorTripCountCalculation>(VPInst);
    VPValue *VPOrigTC = VPVectorTC->getOperand(0);
    auto *ConstIntTC = dyn_cast<VPConstantInt>(VPOrigTC);

    // When the vectortripcountcalculation instruction has two operands,
    // the vector trip count calculation needs to be adjusted using the
    // second operand which specifies the number of peel iteration. We
    // cannot treat the loop as having a known constant trip count.
    if (ConstIntTC && VPVectorTC->getNumOperands() == 1) {
      auto VFUF = getVF() * getUF();
      auto TGU = ConstIntTC->getValue().getZExtValue() / VFUF;
      auto ConstVecTC = TGU * VFUF;
      RegDDRef *ConstVecTCRef =
          DDRefUtilities.createConstDDRef(VPVectorTC->getType(), ConstVecTC);
      addVPValueScalRefMapping(VPVectorTC, ConstVecTCRef, 0);
      return;
    }

    // Adjust original trip count by subtracting the number of peeled iterations
    // using the second operand of VPVectorTC if available.
    RegDDRef *OrigTC = getUniformScalarRef(VPOrigTC);
    if (VPVectorTC->getNumOperands() > 1) {
      RegDDRef *AdjRef =
          getOrCreateScalarRef(VPVectorTC->getOperand(1), 0 /*Lane*/);
      HLInst *AdjTCInst =
          HLNodeUtilities.createSub(OrigTC->clone(), AdjRef->clone(), "adj.tc");
      addInstUnmasked(AdjTCInst);
      OrigTC = AdjTCInst->getLvalDDRef();
    }

    RegDDRef *UBRef = OrigTC->clone();

    // For given original loop TC say %N, we emit following sequence of
    // instructions to compute vector TC -
    // %tgu = %N /u VF*UF
    // %vec.tc = %tgu * VF*UF

    assert(!Mask && "Vector TC calculation should not be masked.");
    RegDDRef *VFUFDD = DDRefUtilities.createConstDDRef(VPVectorTC->getType(),
                                                       getVF() * getUF());
    auto *DivInst = HLNodeUtilities.createUDiv(UBRef, VFUFDD, "tgu");
    addInstUnmasked(DivInst);

    auto *MulInst = HLNodeUtilities.createMul(DivInst->getLvalDDRef()->clone(),
                                              VFUFDD->clone(), "vec.tc");
    addInstUnmasked(MulInst);

    // We need to add back the number of peeled iteration to the value
    // computed above.
    if (VPVectorTC->getNumOperands() > 1) {
      RegDDRef *AdjRef =
          getOrCreateScalarRef(VPVectorTC->getOperand(1), 0 /*Lane*/);
      auto *AdjTCInst = HLNodeUtilities.createAdd(
          MulInst->getLvalDDRef()->clone(), AdjRef->clone(), "adj.tc");
      addInstUnmasked(AdjTCInst);
      MulInst = AdjTCInst;
    }

    addVPValueScalRefMapping(VPVectorTC, MulInst->getLvalDDRef(), 0);
    return;
  }

  case VPInstruction::ActiveLane: {
    assert(!Mask && "ActiveLane calculation is expected to be unmasked!");
    assert(Plan->getVPlanDA()->isUniform(*VPInst) &&
           "ActiveLane instruction is expected to be uniform!");
    VPValue *MaskOp = VPInst->getOperand(0);
    RegDDRef *WideMaskOp = widenRef(MaskOp, getVF());
    HLInst *CTTZ =
        createCTZCall(WideMaskOp, Intrinsic::cttz, false /*MaskIsNonZero*/);
    addVPValueScalRefMapping(VPInst, CTTZ->getLvalDDRef(), 0);
    return;
  }

  case VPInstruction::ActiveLaneExtract: {
    RegDDRef *Val = widenRef(VPInst->getOperand(0), getVF());
    RegDDRef *ActiveLane =
        getScalRefForVPVal(VPInst->getOperand(1), 0 /*Lane*/)->clone();

    if (!VPInst->getType()->isVectorTy()) {
      HLInst *Extract = HLNodeUtilities.createExtractElementInst(
          Val, ActiveLane, "active.ln.extract");
      addInstUnmasked(Extract);
      addVPValueScalRefMapping(VPInst, Extract->getLvalDDRef(), 0);
      return;
    }

    // Original type was vector. Suppose it was <2 x type>, VF == 2 and active
    // lane is equals to "1" (in runtime). We'd need to extract elements (2, 3)
    // from the wide vector in this case:
    //
    // VecValue: |0.1, 0.2 | 1.1, 1.2 |
    //
    // ActiveScalar: <1.1, 1.2>
    //
    // To emulate this we first multiply the ActiveLane by OrigNumElements and
    // increment it while emitting sequence of extracts + inserts to obtain
    // final result.

    auto *I32Ty = IntegerType::getInt32Ty(HLNodeUtilities.getContext());
    if (ActiveLane->getDestType()->getScalarSizeInBits() < 32) {
      auto *ActiveLaneZext = createZExt(I32Ty, ActiveLane);
      ActiveLane = ActiveLaneZext->getLvalDDRef();
    } else if (ActiveLane->getDestType()->getScalarSizeInBits() > 32) {
      auto *ActiveLaneTrunc = HLNodeUtilities.createTrunc(I32Ty, ActiveLane);
      addInstUnmasked(ActiveLaneTrunc);
      ActiveLane = ActiveLaneTrunc->getLvalDDRef();
    }
    unsigned OrigNumElts =
        cast<FixedVectorType>(VPInst->getType())->getNumElements();
    RegDDRef *Undef = DDRefUtilities.createUndefDDRef(VPInst->getType());
    auto *Init = HLNodeUtilities.createCopyInst(Undef, "active.ln.result");
    addInstUnmasked(Init);
    RegDDRef *Result = Init->getLvalDDRef();

    // TODO: Should we fold the mul/adds into CanonExprs directly instead of
    // standalone HLInsts?
    HLInst *IndexBaseMul = HLNodeUtilities.createMul(
        ActiveLane->clone(),
        DDRefUtilities.createConstDDRef(I32Ty, OrigNumElts));
    addInstUnmasked(IndexBaseMul);
    RegDDRef *IndexBase = IndexBaseMul->getLvalDDRef();

    for (unsigned EltIdx = 0; EltIdx < OrigNumElts; ++EltIdx) {
      auto *Index = HLNodeUtilities.createAdd(
          IndexBase->clone(), DDRefUtilities.createConstDDRef(I32Ty, EltIdx));
      addInstUnmasked(Index);
      auto *Extract = HLNodeUtilities.createExtractElementInst(
          Val->clone(), Index->getLvalDDRef()->clone());
      addInstUnmasked(Extract);
      auto *Insert = HLNodeUtilities.createInsertElementInst(
          Result->clone(), Extract->getLvalDDRef()->clone(), EltIdx,
          "active.ln.insert", Result->clone());
      addInstUnmasked(Insert);
    }

    addVPValueScalRefMapping(VPInst, Result, 0);
    return;
  }

  case VPInstruction::ScalarPeelHIR: {
    auto *VPPeelLp = cast<VPScalarPeelHIR>(VPInst);
    HLLoop *ScalarPeel = VPPeelLp->getLoop()->clone();
    OutgoingScalarHLLoops.insert(ScalarPeel);
    OutgoingScalarHLLoopsMap[VPPeelLp] = ScalarPeel;

    // Emit scalar peel loop at current insertion point and initialize UB temp.
    // We also set scalar loop's upper bound captured in corresponding
    // VPScalarPeelHIR instruction. For example simple scalar peel loop that
    // looks like -
    // %scal.peel = scalar-peel-hir <HLLoop>, TempInitMap:
    //   { Initialize temp %peel.ub with -> i64 %vp.peel.ub }
    //
    // is lowered into HIR as -
    //
    // %peel.ub = ...
    // + DO i1 = 0, %peel.ub, 1   <DO_LOOP> <vectorize>
    // |   %A.i = (%A)[i1];
    // |   %red.var = %A.i  +  %red.var;
    // + END LOOP

    assert(VPPeelLp->getNumOperands() == 1 &&
           "Scalar peel loop is expected to have only one operand.");
    RegDDRef *UBTemp = cast<RegDDRef>(VPPeelLp->getUpperBoundTemp());
    RegDDRef *InitValue = getOrCreateScalarRef(VPPeelLp->getOperand(0), 0);
    HLInst *UBInitInst =
        HLNodeUtilities.createCopyInst(InitValue, "temp.init", UBTemp->clone());
    addInstUnmasked(UBInitInst);

    // Upper bound of HLLoop is inclusive - subtract 1 to account for the same.
    HLInst *AddInst = HLNodeUtilities.createSub(
        UBTemp, DDRefUtilities.createConstDDRef(UBTemp->getDestType(), 1),
        "peel.ub");
    addInstUnmasked(AddInst);
    UBTemp = AddInst->getLvalDDRef()->clone();

    // Initialized UB temp will be live-in to scalar loop.
    ScalarPeel->addLiveInTemp(UBTemp);

    // Insert the scalar peel loop into outgoing HIR.
    LLVM_DEBUG(dbgs() << "[VPOCGHIR] ScalarPeel:\n"; ScalarPeel->dump();
               dbgs() << "\n");
    addInst(ScalarPeel, nullptr /*Mask*/);

    // Update upper DDRef of scalar peel.
    ScalarPeel->setUpperDDRef(UBTemp);
    // Sets the defined at level of new bound to (nesting level - 1) as the
    // bound temp is defined just before the loop.
    UBTemp->getSingleCanonExpr()->setDefinedAtLevel(
        ScalarPeel->getNestingLevel() - 1);

    // As we peel maximum for the size of one register, the max TC for peel
    // can be MaxVF.
    setLoopTCEstimatesAndMarkers(ScalarPeel, MaxVF - 1);
    HIRLoopVisitor LV(ScalarPeel, this);
    LV.replaceCalls();
    return;
  }

  case VPInstruction::ScalarRemainderHIR: {
    auto *VPRemLp = cast<VPScalarRemainderHIR>(VPInst);
    HLLoop *ScalarRem = VPRemLp->getLoop()->clone();
    OutgoingScalarHLLoops.insert(ScalarRem);
    OutgoingScalarHLLoopsMap[VPRemLp] = ScalarRem;

    // Emit scalar remainder loop at current insertion point and initialize
    // required live-in temps before the loop. We also set scalar loop's lower
    // bound based on temps captured in corresponding VPScalarRemainderHIR
    // instruction. For example simple scalar remainder that looks like -
    // %scal.rem = scalar-remainder-hir <HLLoop>, TempInitMap:
    //   { Initialize temp %rem.lb with -> i64 %vp.rem.lb }
    //   { Initialize temp %red.var with -> i32 %vp.red.var }
    //
    // is lowered into HIR as -
    //
    // %rem.lb = %extract.vec.rem.lb;
    // %red.var = %extract.vec.red.var;
    // + DO i1 = %rem.lb, %N - 1, 1   <DO_LOOP> <vectorize>
    // |   %A.i = (%A)[i1];
    // |   %red.var = %A.i  +  %red.var;
    // + END LOOP
    VPValue *LBTempInitVal = nullptr;
    auto *LBTemp = cast<RegDDRef>(VPRemLp->getLowerBoundTemp());
    for (unsigned I = 0; I < VPRemLp->getNumOperands(); ++I) {
      RegDDRef *TempToInit = cast<RegDDRef>(VPRemLp->getLiveIn(I))->clone();
      RegDDRef *InitValue = getOrCreateScalarRef(VPRemLp->getOperand(I), 0);
      HLInst *InitInst = HLNodeUtilities.createCopyInst(InitValue, "temp.init",
                                                        TempToInit);
      if (DDRefUtils::areEqual(TempToInit, LBTemp))
        LBTempInitVal = VPRemLp->getOperand(I);
      addInstUnmasked(InitInst);
      if (TempToInit->isTerminalRef())
        TempToInit->makeSelfBlob();

      // Initialized temp will be live-in to scalar loop.
      ScalarRem->addLiveInTemp(TempToInit);
    }

    LLVM_DEBUG(dbgs() << "[VPOCGHIR] ScalarRemainder:\n"; ScalarRem->dump();
               dbgs() << "\n");
    addInst(ScalarRem, nullptr /*Mask*/);

    // Update lower DDRef of scalar peel. We try and set the lower DDRef to
    // a constant if we know that the lower bound temp is initialized with
    // a constant value to enable better downstream optimizations.
    if (isa<VPLiveOutValue>(LBTempInitVal) &&
        getUniformScalarRef(LBTempInitVal)->isConstant()) {
      ScalarRem->setLowerDDRef(getUniformScalarRef(LBTempInitVal));
    } else {
      ScalarRem->setLowerDDRef(LBTemp);
      // Sets the defined at level of new bound to (nesting level - 1) as the
      // bound temp is defined just before the loop.
      LBTemp->getSingleCanonExpr()->setDefinedAtLevel(
          ScalarRem->getNestingLevel() - 1);
    }

    // TODO: For the case of vector remainder followed by a scalar
    // remainder, we should be able to use remainder VF - 1 for
    // the trip count estimates. To be done later.
    setLoopTCEstimatesAndMarkers(ScalarRem, MaxVF * UF - 1);
    HIRLoopVisitor LV(ScalarRem, this);
    LV.replaceCalls();
    return;
  }

  case VPInstruction::PeelOrigLiveOutHIR: {
    handleScalarLoopOrigLiveOut<VPPeelOrigLiveOutHIR>(VPInst);
    return;
  }

  case VPInstruction::RemOrigLiveOutHIR: {
    handleScalarLoopOrigLiveOut<VPRemainderOrigLiveOutHIR>(VPInst);
    return;
  }

  case VPInstruction::InvSCEVWrapper: {
    auto *InvScev = cast<VPInvSCEVWrapper>(VPInst)->getSCEV();
    VPlanAddRecHIR *AddRecHIR =
        VPlanScalarEvolutionHIR::toVPlanAddRecHIR(InvScev);

    // AddRecHIR contains a canon expression for the adjusted base pointer of
    // a memory ref and the actual memory ref itself. As an example, for the
    // memory ref: A[n1 + 2 * i1 + i3] where A is an array of longs(element size
    // 8), vectorization is happening at i3 level, the canon expression for the
    // adjusted base would be: (A + 8 * n1 + 16 * i1). However, HIR framework
    // does not allow such canon expression in actual generated code although
    // it is ok for analysis purposes. When generating code, we need to convert
    // the adjusted base back to: &A[n1 + 2 * i1]. We were doing this by using
    // the adjusted base earlier, but we now effectively do the same by removing
    // vectorization level IV from the original memory ref - this is needed
    // for correctness(preserve lower/stride info) as well as for easier
    // handling of multi dimensional array accesses.
    // TODO: Look into avoiding using canon expr for adjusted base in SCEV
    // analysis and rely on adjusted RegDDRef if it serves the same purpose.
    const RegDDRef *ParentRef = AddRecHIR->Ref;
    RegDDRef *AddrOfRef = ParentRef->clone();
    AddrOfRef->setAddressOf(true);
    AddrOfRef->getDimensionIndex(1)->removeIV(OrigLoop->getNestingLevel());
    AddrOfRef->makeConsistent({}, getNestingLevelFromInsertPoint());
    addVPValueScalRefMapping(VPInst, AddrOfRef, 0);
    return;
  }
  }

  // Skip widening the first select operand. The select instruction is generated
  // using operands of the instruction corresponding to the select mask.
  bool SkipFirstSelectOp = VPInst->getOpcode() == Instruction::Select;
  SmallVector<RegDDRef *, 6> RefOps;
  // Do not widen any operands for call instruction. They are explicitly handled
  // during instruction widening.
  VPUser::const_operand_range OpRange =
      isa<VPCallInstruction>(VPInst)
          ? make_range(VPInst->op_end(), VPInst->op_end())
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
      // TODO: The whole IF below should be replaced by getOrCreateScalarRef
      // call when we switch to SVA-based CG, see widenNodeImpl, the last call
      // of generateHIR(). The OnlyExistingScalarOps argument will be not needed
      // then.

      // Obtain the scalar ref for lane ScalarLaneID if one exists already
      if ((Ref = getScalRefForVPVal(Operand, ScalarLaneID))) {
        Ref = Ref->clone();
      } else if (isa<VPExternalDef>(Operand) || isa<VPConstant>(Operand)) {
        // Creation of a scalar ref for externaldefs/constants does not
        // require a new instruction - try to get uniform scalar ref if Ref is
        // null.
        Ref = getUniformScalarRef(Operand);
      } else if (!OnlyExistingScalarOps) {
        Ref = getOrCreateScalarRef(Operand, ScalarLaneID);
      }
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
    // Reductions cannot be folded. Also, we don't fold for VPInstructions
    // that don't have any uses, that might lead to an empty loop generation
    // and assertion in HIR verifier.
    assert(RefOp0->isTerminalRef() && RefOp1->isTerminalRef() &&
           "Expected terminal refs");

    auto *CE1 = RefOp0->getSingleCanonExpr();
    auto *CE2 = RefOp1->getSingleCanonExpr();

    // When we do add folding with denominator(s), the add operation effectively
    // does the following:
    //
    //      CE1/D1 + CE2/D2 ==> ((LCM/D1 * CE1) + (LCM/D2 * CE2))/LCM
    //
    //  This cannot be done blindly as we need to make sure that no overflow
    //  occurs. To prevent issues, we avoid folding when we have a denominator.
    if (!ReductionVPInsts.count(VPInst) && CE1->getDenominator() == 1 &&
        CE2->getDenominator() == 1 && CanonExprUtilities.canAdd(CE1, CE2) &&
        VPInst->getNumUsers()) {
      SmallVector<const RegDDRef *, 2> AuxRefs = {RefOp0->clone(), RefOp1};
      CanonExprUtilities.add(CE1, CE2);
      makeConsistentAndAddToMap(RefOp0, VPInst, AuxRefs, Widen, ScalarLaneID);
      return;
    }

    bool HasNUW = false, HasNSW = false;
    getOverflowFlags(VPInst, HasNUW, HasNSW);

    NewInst = HLNodeUtilities.createOverflowingBinOp(
        VPInst->getOpcode(), RefOp0, RefOp1, HasNUW, HasNSW, InstName);
    break;
  }

  case Instruction::UDiv:
  case Instruction::SDiv: {
    if (Widen &&
        serializeDivRem(VPInst, Mask ? Mask : CurMaskValue, OptRptStats))
      return;
    assert(RefOp0->isTerminalRef() && RefOp1->isTerminalRef() &&
           "Expected terminal refs");

    auto *CE1 = RefOp0->getSingleCanonExpr();

    // Try and fold the divide operation into the canon expression for RefOp0 if
    // it is linear. This helps preserve linear values and also avoids
    // unnecessary HLInsts. This is limited to instructions inside a VPLoop.
    // Also, we don't fold for VPInstructions that don't have any uses, that
    // might lead to an empty loop generation and assertion in HIR verifier.
    auto *ConstOp = dyn_cast<VPConstant>(VPInst->getOperand(1));
    if (Plan->getVPLoopInfo()->getLoopFor(VPInst->getParent()) &&
        CE1->isLinearAtLevel(getNestingLevelFromInsertPoint()) &&
        CE1->getDenominator() == 1 && ConstOp && VPInst->getNumUsers()) {
      auto *CI = cast<ConstantInt>(ConstOp->getConstant());

      // The constant value needs to fit in 64 bits which is what setDenominator
      // takes. Try folding only if this is the case.
      if (CI->getBitWidth() <= 64) {
        SmallVector<const RegDDRef *, 2> AuxRefs = {RefOp0->clone()};

        if (Opcode == Instruction::UDiv)
          CE1->setDenominator(CI->getZExtValue());
        else
          CE1->setDenominator(CI->getSExtValue());

        CE1->setDivisionType(VPInst->getOpcode() == Instruction::SDiv);
        makeConsistentAndAddToMap(RefOp0, VPInst, AuxRefs, Widen, ScalarLaneID);
        return;
      }
    }

    NewInst = HLNodeUtilities.createPossiblyExactBinOp(
        VPInst->getOpcode(), RefOp0, RefOp1, VPInst->isExact(), InstName,
        nullptr);
    break;
  }

  case Instruction::Mul: {
    // Try and fold the mul operation into the canon expression. This helps
    // preserve linear values and also avoids unnecessary HLInsts. Reductions
    // cannot be folded. This is limited to instructions inside a VPLoop.
    // Also, we don't fold for VPInstructions that don't have any uses, that
    // might lead to an empty loop generation and assertion in HIR verifier.
    assert(RefOp0->isTerminalRef() && RefOp1->isTerminalRef() &&
           "Expected terminal refs");

    auto *CE1 = RefOp0->getSingleCanonExpr();
    auto *CE2 = RefOp1->getSingleCanonExpr();
    const VPConstant *ConstOp = nullptr;
    unsigned NonConstIndex = 0;

    if (Plan->getVPLoopInfo()->getLoopFor(VPInst->getParent()) &&
        !ReductionVPInsts.count(VPInst) && VPInst->getNumUsers() &&
        CE1->isLinearAtLevel(getNestingLevelFromInsertPoint()) &&
        CE2->isLinearAtLevel(getNestingLevelFromInsertPoint())) {
      if ((ConstOp = dyn_cast<VPConstant>(VPInst->getOperand(0))))
        NonConstIndex = 1;
      else if ((ConstOp = dyn_cast<VPConstant>(VPInst->getOperand(1))))
        NonConstIndex = 0;
    }
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
          makeConsistentAndAddToMap(ResultRef, VPInst, AuxRefs, Widen,
                                    ScalarLaneID);
          return;
        }
      }
    }

    bool HasNUW = false, HasNSW = false;
    getOverflowFlags(VPInst, HasNUW, HasNSW);

    NewInst = HLNodeUtilities.createOverflowingBinOp(
        VPInst->getOpcode(), RefOp0, RefOp1, HasNUW, HasNSW, InstName);
    break;
  }

  case Instruction::SRem:
  case Instruction::URem:
    if (Widen &&
        serializeDivRem(VPInst, Mask ? Mask : CurMaskValue, OptRptStats))
      return;
    LLVM_FALLTHROUGH;
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
    getOverflowFlags(VPInst, HasNUW, HasNSW);

    if (VPInst->hasFastMathFlags()) {
      NewInst = HLNodeUtilities.createFPMathBinOp(
          VPInst->getOpcode(), RefOp0, RefOp1, VPInst->getFastMathFlags(),
          InstName);
    } else if (HasNUW || HasNSW) {
      NewInst = HLNodeUtilities.createOverflowingBinOp(
          VPInst->getOpcode(), RefOp0, RefOp1, HasNUW, HasNSW, InstName);
    } else if (VPInst->isExact()) {
      NewInst = HLNodeUtilities.createPossiblyExactBinOp(
          VPInst->getOpcode(), RefOp0, RefOp1, VPInst->isExact(), InstName);
    } else {
      NewInst = HLNodeUtilities.createBinaryHLInst(VPInst->getOpcode(), RefOp0,
                                                   RefOp1, InstName);
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
    NegRef->makeConsistent(AuxRefs, getNestingLevelFromInsertPoint());

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
    auto PredInst = dyn_cast<VPCmpInst>(VPInst->getOperand(0));
    CmpInst::Predicate Pred;
    RegDDRef *Pred0, *Pred1;

    // In order for HIR idiom recognition to work, when generating the widened
    // select we need to use operands of the compare instruction corresponding
    // to the select mask when possible.
    if (PredInst) {
      Pred0 = widenRef(PredInst->getOperand(0), getVF());
      Pred1 = widenRef(PredInst->getOperand(1), getVF());
      Pred = PredInst->getPredicate();
    } else {
      // Used Pred0 == AllOnes as the select mask
      Pred0 = widenRef(VPInst->getOperand(0), getVF());
      Pred1 = nullptr;
      Pred = CmpInst::ICMP_EQ;
    }

    FastMathFlags FMF = VPInst->hasFastMathFlags() ? VPInst->getFastMathFlags()
                                                   : FastMathFlags();

    // Replicate Pred0/Pred1 if the mask is Scalar and instruction type is
    // vector
    auto *VecTy = dyn_cast<FixedVectorType>(VPInst->getType());
    unsigned ReplicateFactor =
        VecTy && !VPInst->getOperand(0)->getType()->isVectorTy()
            ? VecTy->getNumElements()
            : 0;
    NewInst =
        createSelectHelper(Pred, Pred0, Pred1, RefOp0, RefOp1, ReplicateFactor,
                           InstName, nullptr /*LVal*/, FMF);
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
    // Also, we don't fold for VPInstructions that don't have any uses, that
    // might lead to an empty loop generation and assertion in HIR verifier.
    if (CE->getDestType() == CE->getSrcType() && VPInst->getNumUsers()) {
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
      if (VPInst->HIR().getFoldIVConvert()) {
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
        makeConsistentAndAddToMap(RefOp0, VPInst, AuxRefs, Widen, ScalarLaneID);
        return;
      }

      CE->setDestType(ResultRefTy);
      CE->setExtType(VPInst->getOpcode() == Instruction::SExt);
      makeConsistentAndAddToMap(RefOp0, VPInst, AuxRefs, Widen, ScalarLaneID);
      return;
    }

    // Do not create an explicit scalar instruction.
    if (!Widen)
      return;

    NewInst = HLNodeUtilities.createCastHLInst(ResultRefTy, VPInst->getOpcode(),
                                               RefOp0, InstName);
    break;
  }

  case Instruction::AddrSpaceCast:
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
      auto *BitCastDestTy = isa<PointerType>(ResultRefTy)
                                ? ResultRefTy->getPointerElementType()
                                : ResultRefTy;
      RefOp0->setBitCastDestVecOrElemType(BitCastDestTy);
      makeConsistentAndAddToMap(RefOp0, VPInst, AuxRefs, Widen, ScalarLaneID);
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

  case VPInstruction::UMinSeq: {
    // The code we emit for SCEVSequentialUMinExpr:
    // if (op1 == 0 || op2 == 0)
    //   return 0;
    // else
    //   return UMin(op1, op2);
    //
    // Such lowering provides UMinSeq(0, poison) = 0 semantics, meanwhile
    // UMin(0, poison) = poison.

    Type *VTy = getWidenedType(VPInst->getType(), VF);
    Value *SaturationPoint = MinMaxIntrinsic::getSaturationPoint(
      Intrinsic::umin, VTy);
    RegDDRef *SatDDRef = DDRefUtilities.createConstDDRef(SaturationPoint);
    RegDDRef *OneDDRef = DDRefUtilities.createConstDDRef(getWidenedType(
                           Type::getInt1Ty(Context), VF), 1);

    // Generate compares with Saturation point for each operand.
    HLInst *CmpInst0 = HLNodeUtilities.createCmp(
      ICmpInst::ICMP_EQ, RefOp0->clone(), SatDDRef->clone(), InstName);
    HLInst *CmpInst1 = HLNodeUtilities.createCmp(
      ICmpInst::ICMP_EQ, RefOp1->clone(), SatDDRef->clone(), InstName);

    HLInst *SelInst = HLNodeUtilities.createSelect(
      ICmpInst::ICMP_EQ, CmpInst0->getLvalDDRef()->clone(), OneDDRef->clone(),
      OneDDRef->clone(), CmpInst1->getLvalDDRef()->clone(), InstName);

    HLInst *ClassicUMinInst = HLNodeUtilities.createSelect(
      CmpInst::ICMP_ULT, RefOp0->clone(), RefOp1->clone(),
      RefOp0->clone(), RefOp1->clone(), InstName);

    addInst(CmpInst0, Mask);
    addInst(CmpInst1, Mask);
    addInst(SelInst, Mask);
    addInst(ClassicUMinInst, Mask);

    NewInst = HLNodeUtilities.createSelect(
      ICmpInst::ICMP_EQ, SelInst->getLvalDDRef()->clone(), OneDDRef->clone(),
      SatDDRef->clone(), ClassicUMinInst->getLvalDDRef()->clone(), InstName);

    break;
  }

  case Instruction::GetElementPtr: {
    auto *GEP = cast<VPGEPInstruction>(VPInst);
    if (VPInst->getNumOperands() == 2) {
      // VPlan VLS transformation introduces this kind of GEPs:
      //   %base = ....
      //   %gep = getelementptr %base, i32 offset ; usually negative
      auto SVA = Plan->getVPlanSVA();
      (void)SVA;
      assert(SVA->retValNeedsFirstScalarCode(VPInst) &&
             !SVA->retValNeedsLastScalarCode(VPInst) &&
             !SVA->retValNeedsVectorCode(VPInst) &&
             "GetElementPtr not fully supported!");
      if (!Widen) {
        // FIXME: generateHIR is called two times. Non-widen case has more
        // conditions, so generate during the widening call.
        return;
      }

      RegDDRef *PointerRef = getOrCreateScalarRef(VPInst->getOperand(0), 0);
      if (PointerRef->hasGEPInfo() && PointerRef->getNumDimensions() > 0 &&
          !PointerRef->hasTrailingStructOffsets()) {
        if (auto *Const = dyn_cast<VPConstant>(VPInst->getOperand(1))) {
          // Consider VPInst being
          //   %gep = getelementptr i32* %base i64 -1
          // with PointerRef for %base
          //   &((i32*)(@arr1)[0][3 * i1 + 1])
          //
          // Instead of storing that to a temp and then creating a new compute
          // based on that temp, we can fix the CannonExpr in place resulting in
          // a nicer outgoing HIR, i.e. generate simply
          //   &((i32*)(@arr1)[0][3 * i1]).
          CanonExpr *CE = PointerRef->getDimensionIndex(1);
          CE->addConstant(Const->getSExtValue(), true /* IsMathAdd */);
          addVPValueScalRefMapping(VPInst, PointerRef, 0);
          return;
        }
      }
      auto *CopyInst = HLNodeUtilities.createCopyInst(PointerRef, "gep.base");
      addInstUnmasked(CopyInst);
      PointerRef = CopyInst->getLvalDDRef()->clone();

      auto *NewRef = DDRefUtilities.createAddressOfRef(
          GEP->getSourceElementType(), PointerRef->getSelfBlobIndex(),
          PointerRef->getDefinedAtLevel());
      RegDDRef *Idx = getOrCreateScalarRef(VPInst->getOperand(1), 0);
      NewRef->addDimension(Idx->getSingleCanonExpr());
      addVPValueScalRefMapping(VPInst, NewRef, 0);
      return;
    }

    // Any other case of GEP implies it was introduced after decomposer by some
    // VPlan-to-VPlan xform.
    assert(false && "VPlan-to-VPlan xform introduced a new IR-agnostic GEP.");
    return;
  }

  case Instruction::Call: {
    auto *VPCall = cast<VPCallInstruction>(VPInst);

    // Handle call that should not be widended for given lane ID.
    if (!Widen) {
      HLInst *ScalarCall = generateScalarCall(VPCall, ScalarLaneID);
      addInstUnmasked(ScalarCall);
      assert((CurMaskValue == nullptr || isa<HLIf>(ScalarCall->getParent())) &&
             "Scalar calls can be generated only in unmasked context.");
      if (ScalarCall->hasLval())
        addVPValueScalRefMapping(VPCall, ScalarCall->getLvalDDRef(),
                                 ScalarLaneID);
      return;
    }

    // Calls need to be masked with current mask value if Mask is null.
    if (!Mask)
      Mask = CurMaskValue;

    // For all calls vectorization scenario should be available for current VF.
    assert(VPCall->getVFForScenario() == VF &&
           "Cannot find call vectorization scenario for VF.");

    // Skip ignored calls (for example, lifetime intrinsics).
    if (VPCall->getUnderlyingCallInst() &&
        isIgnoredCall(VPCall->getUnderlyingCallInst()))
      return;

    switch (VPCall->getVectorizationScenario()) {
    case VPCallInstruction::CallVecScenariosTy::LibraryFunc: {
      widenLibraryCall(VPCall, Mask);
      ++OptRptStats.VectorMathCalls;
      return;
    }
    case VPCallInstruction::CallVecScenariosTy::TrivialVectorIntrinsic: {
      widenTrivialIntrinsic(VPCall);
      ++OptRptStats.VectorMathCalls;
      return;
    }
    case VPCallInstruction::CallVecScenariosTy::VectorVariant: {
      widenVectorVariant(VPCall, Mask);
      ++OptRptStats.VectorVariantCalls;
      return;
    }
    case VPCallInstruction::CallVecScenariosTy::Serialization: {
      serializeInstruction(VPCall, Mask);
      ++OptRptStats.SerializedCalls;
      // Since multiple new instructions are created for this VPInst, explicit
      // addInst and mapping is not needed. It is handled internally in
      // serializeInstruction.
      return;
    }
    case VPCallInstruction::CallVecScenariosTy::DoNotWiden: {
      scalarizePredicatedUniformInst(VPCall, Mask);
      return;
    }
    default: {
      llvm_unreachable("VPCallInstruction does not have a valid decision for "
                       "HIR vectorizer.");
    }
    }

    break;
  }

  // CG support for opcodes that operate on VectorType.
  case Instruction::ExtractElement: {
    assert(isa<VectorType>(RefOp0->getDestType()) &&
           "First operand of extractelement was not widened.");
    VPValue *VPIndexVal = VPInst->getOperand(1);
    assert(isa<VPConstantInt>(VPIndexVal) &&
           "Only constant index extractelements are supported.");

    RegDDRef *ExtrFrom = RefOp0;
    unsigned IndexVal =
        cast<VPConstantInt>(VPIndexVal)->getValue().getLimitedValue();

    // Extract subvector from widened first operand. Subvector should include VF
    // elements.
    SmallVector<Constant *, 8> ShufMask;
    unsigned OrigNumElts =
        cast<FixedVectorType>(VPInst->getOperand(0)->getType())
            ->getNumElements();
    unsigned WideNumElts = getVF() * OrigNumElts;

    for (unsigned Idx = IndexVal; Idx < WideNumElts; Idx += OrigNumElts)
      ShufMask.push_back(ConstantInt::get(Type::getInt32Ty(Context), Idx));

    assert(ShufMask.size() == getVF() &&
           "Subvector should contain VF elements.");

    NewInst = createShuffleWithUndef(ExtrFrom, ShufMask, "wide.extract");
    break;
  }

  case Instruction::InsertElement: {
    assert(isa<VectorType>(RefOp0->getDestType()) &&
           isa<VectorType>(RefOp1->getDestType()) &&
           "First/second operand of insertelement was not widened.");
    VPValue *VPIndexVal = VPInst->getOperand(2);
    assert(isa<VPConstantInt>(VPIndexVal) &&
           "Only constant index insertelements are supported.");

    RegDDRef *InsertInto = RefOp0;
    RegDDRef *SubVecToInsert = RefOp1;
    unsigned IndexVal =
        cast<VPConstantInt>(VPIndexVal)->getValue().getLimitedValue();
    unsigned OrigNumElts =
        cast<FixedVectorType>(VPInst->getOperand(0)->getType())
            ->getNumElements();
    unsigned WideNumElts = getVF() * OrigNumElts;

    // Widening of insertelements is handled based on 2 cases -
    // 1. Insert into empty/undef vector.
    // 2. Generic case where values are inserted into non-empty vector.
    //
    // Examples used to explain cases below -
    // Incoming OrigNumElts= 2 (complex type), VF = 2
    // %real = opcode ...
    // %imag = opcode ...
    // %complex.1 = insertelement <undef, undef>, %real, 0  ---> Case 1
    // %complex.2 = insertelement %complex.1, %imag, 1      ---> Case 2

    // Case 1:
    // Generate a shuffle that selects elements of SubVecToInsert at appropriate
    // indices and uses undef for other elements of outgoing vector. The example
    // is revectorized as -
    // %real.vec = opcode ...
    // %complex.1.vec = shuffle <2 x float> %real.vec, <undef, undef>,
    //                          <4 x i32 > < 0, undef, 1, undef >
    if (InsertInto->isStandAloneUndefBlob()) {
      SmallVector<Constant *, 8> ShufMask;
      // Fill all elements of mask with undef, size is WideNumElts.
      ShufMask.resize(WideNumElts, UndefValue::get(Type::getInt32Ty(Context)));
      // For each vector lane set mask bits based on IndexVal.
      for (unsigned Lane = 0; Lane < getVF(); ++Lane)
        ShufMask[Lane * OrigNumElts + IndexVal] =
            ConstantInt::get(Type::getInt32Ty(Context), Lane);

      // Emit shuffle to emulate the wide insert operation.
      NewInst = createShuffleWithUndef(SubVecToInsert, ShufMask, "wide.insert");
      break;
    }

    assert(!InsertInto->isStandAloneUndefBlob() && "Unexpected InsertInto.");

    // Case 2:
    // The vector we need to insert into is already of appropriate length and
    // may contain elements. To generically insert new elements into this
    // vector, we first extend the subvector to insert (with undefs) and then
    // generate a shuffle to select elements from the main vector or subvector
    // based on current index being processed. The example above is revectorized
    // as -
    // %imag.vec = opcode ...
    // %imag.vec.extend = shuffle %imag.vec, <undef, undef>,
    //                            <4 x i32> < 0, 1, undef, undef >
    // %complex.2.vec = shuffle %complex.1.vec, %imag.vec.extended,
    //                          <4 x i32> < 0, 4, 2, 5 >
    HLInst *ExtendedSubVec = extendVector(SubVecToInsert, WideNumElts);
    // Add the extended subvector into outgoing HIR. No need to update any
    // internal maps, its only user is the shuffle generated below.
    addInst(ExtendedSubVec, Mask);

    // Create shuffle mask to select elements from InsertInto or ExtendedSubVec
    // based on index value of the original insertelement.
    SmallVector<Constant *, 8> ShufMask;
    for (unsigned InsIntoVecIdx = 0, ExtSubVecIdx = WideNumElts;
         InsIntoVecIdx < WideNumElts; ++InsIntoVecIdx) {
      if (InsIntoVecIdx % OrigNumElts == IndexVal) {
        // Index matches, so insert element from ExtendedSubVec
        ShufMask.push_back(
            ConstantInt::get(Type::getInt32Ty(Context), ExtSubVecIdx));
        ExtSubVecIdx++;
      } else {
        // Index doesn't match, so retain element from InsertInto vector.
        ShufMask.push_back(
            ConstantInt::get(Type::getInt32Ty(Context), InsIntoVecIdx));
      }
    }

    Constant *MaskVec = ConstantVector::get(ShufMask);
    RegDDRef *ShufMaskRef = DDRefUtilities.createConstDDRef(MaskVec);
    NewInst = HLNodeUtilities.createShuffleVectorInst(
        InsertInto->clone(), ExtendedSubVec->getLvalDDRef()->clone(),
        ShufMaskRef, "wide.insert");
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

  case VPInstruction::AllZeroCheck: {
    RegDDRef *A = RefOp0;
    if (!Mask)
      Mask = CurMaskValue;
    if (Mask) {
      HLInst *And =
          HLNodeUtilities.createAnd(Mask->clone(), RefOp0, "allz.and.");
      addInstUnmasked(And);
      A = And->getLvalDDRef()->clone();
    }

    RegDDRef *AllZeroCmp = generateCompareToZero(A, nullptr, true /* Equal */);
    addVPValueScalRefMapping(VPInst, AllZeroCmp, 0);
    NewInst = HLNodeUtilities.createCopyInst(
        widenRef(AllZeroCmp->clone(), getVF()), "all.zero.check");

    // The generated copy instruction needs to be inserted unmasked so that
    // all vector lanes get the same value.
    assert(Plan->getVPlanDA()->isUniform(*VPInst) &&
           "Expected AllZeroCheck to be uniform");
    addInstUnmasked(NewInst);
    addVPValueWideRefMapping(VPInst, NewInst->getLvalDDRef());
    return;
  }

  case VPInstruction::ConflictInsn: {
    auto *VPConflict = cast<VPConflictInsn>(VPInst);
    unsigned TypeSize =
        VPConflict->getOperand(0)->getType()->getPrimitiveSizeInBits();
    Intrinsic::ID ConflictIntrin =
        VPConflict->getConflictIntrinsic(getVF(), TypeSize);
    assert(ConflictIntrin != Intrinsic::not_intrinsic &&
           "Valid conflict intrinsic expected here.");
    Function *ConflictFunc =
        Intrinsic::getDeclaration(&HLNodeUtilities.getModule(), ConflictIntrin);
    LLVM_DEBUG(dbgs() << "Conflict func: "; ConflictFunc->dump());
    // TODO: FMF for these conflict intrinsics?
    NewInst = HLNodeUtilities.createCall(ConflictFunc, {RefOp0}, "conflicts");
    break;
  }

  case VPInstruction::Permute: {
    auto *Permute = cast<VPPermute>(VPInst);
    Intrinsic::ID PermuteIntrin =
        Permute->getPermuteIntrinsic(getVF());
    assert(PermuteIntrin != Intrinsic::not_intrinsic &&
           "Valid permute intrinsic expected here.");
    Function *PermuteFunc =
        Intrinsic::getDeclaration(&HLNodeUtilities.getModule(), PermuteIntrin);
    LLVM_DEBUG(dbgs() << "Permute func: "; PermuteFunc->dump());
    NewInst = HLNodeUtilities.createCall(PermuteFunc, {RefOp0, RefOp1},
                                         "permute");
    break;
  }

  case VPInstruction::CvtMaskToInt: {
    // Bitcast <VF x i1> to an integer value VF-bits long.
    Type *MaskTy = IntegerType::get(Context, getVF());
    auto *BitCastInst =
        HLNodeUtilities.createCastHLInst(MaskTy, Instruction::BitCast, RefOp0);
    addInst(BitCastInst, Mask);
    // Zext the casted value to target integer type.
    auto *ZextInst =
        createZExt(VPInst->getType(), BitCastInst->getLvalDDRef()->clone());
    // Produced value is uniform and scalar, update entry in scalar map.
    addVPValueScalRefMapping(VPInst, ZextInst->getLvalDDRef(), 0 /*Lane*/);
    return;
  }

  case VPInstruction::PushVF: {
    assert(!isSearchLoop() && "PushVF not expected for search loops.");
    unsigned NewVF = cast<VPPushVF>(VPInst)->getVF();
    unsigned NewUF = cast<VPPushVF>(VPInst)->getUF();
    assert((NewVF != 0 && NewUF != 0) && "expected nonzero VF and UF");
    VFStack.emplace_back(getVF(), getUF());
    dropExternalValsFromMaps();
    setVF(NewVF);
    setUF(NewUF);
    return;
  }

  case VPInstruction::PopVF: {
    assert(!isSearchLoop() && "PopVF not expected for search loops.");
    assert(!VFStack.empty() && "unexpected PopVF");
    auto V = VFStack.pop_back_val();
    dropExternalValsFromMaps();
    setVF(V.first);
    setUF(V.second);
    return;
  }

  default:
    LLVM_DEBUG(VPInst->dump());
    llvm_unreachable("Unexpected VPInstruction opcode");
  }

  addInst(NewInst, Mask);
  if (NewInst->hasLval())
    addVPValueRefToMaps(VPInst, NewInst->getLvalDDRef(), Widen, ScalarLaneID);
}

void VPOCodeGenHIR::makeConsistentAndAddToMap(
    RegDDRef *Ref, const VPInstruction *VPInst,
    SmallVectorImpl<const RegDDRef *> &AuxRefs, bool Widen,
    unsigned ScalarLaneID) {
  // Use AuxRefs if it is not empty to make Ref consistent
  if (!AuxRefs.empty())
    Ref->makeConsistent(AuxRefs, getNestingLevelFromInsertPoint());
  if (Widen) {
    RegDDRef *WideRef = Ref;

    auto GetOuterMostVPLoop =
        [this](const VPBasicBlock *VPBB) -> const VPLoop * {
      auto *VLoop = Plan->getVPLoopInfo()->getLoopFor(VPBB);
      if (!VLoop)
        return nullptr;

      while (VLoop->getParentLoop())
        VLoop = VLoop->getParentLoop();

      return VLoop;
    };

    auto *OuterMostLoop = GetOuterMostVPLoop(VPInst->getParent());
    if (OuterMostLoop && OuterMostLoop->isLiveOut(VPInst)) {
      // Emit a copy instruction to prevent invalid folding during live-out
      // finalization.
      assert(Ref->getHLDDNode() == nullptr &&
             "Only unattached DDRefs are expected.");
      auto *LiveOutCopy = HLNodeUtilities.createCopyInst(Ref, "liveoutcopy");
      // TODO: Is Mask needed for adding the copy?
      addInstUnmasked(LiveOutCopy);
      WideRef = LiveOutCopy->getLvalDDRef();
    }

    addVPValueWideRefMapping(VPInst, WideRef);
  } else
    addVPValueScalRefMapping(VPInst, Ref, ScalarLaneID);
}

void VPOCodeGenHIR::widenNodeImpl(const VPInstruction *VPInst, RegDDRef *Mask) {
  // We treat select instruction in HIR specially. When generating code for
  // select instructions, the operands of the compare which generate the select
  // mask are part of the HIR select instruction. The HIR select instruction
  // also stores the compare predicate. As a result, we can avoid generating
  // code for a compare instruction if its only use is a select instruction.
  if (isa<VPCmpInst>(VPInst) && VPInst->getNumUsers() == 1) {
    auto *UserInst = cast<VPInstruction>(*(VPInst->users().begin()));
    if (UserInst->getOpcode() == Instruction::Select &&
        UserInst->getOperand(0) == VPInst &&
        llvm::count(UserInst->operands(), VPInst) == 1)
      return;
  }

  // Use SVA knowledge to scalarize HIR copy instructions. This is part of
  // initial efforts to keep instructions outside vector loops scalar.
  if (isa<VPHIRCopyInst>(VPInst) && instIsStrictlyFirstScalar(VPInst)) {
    generateHIR(VPInst, Mask, false /*Widen*/, 0 /*LaneID*/);
    return;
  }

  // Generate wide constructs for all VPInstuctions. This will be changed later
  // to use SVA information.
  generateHIR(VPInst, Mask, true /*Widen*/);

  // Generate a scalar instruction for strided/uniform GEPs/subscripts. This is
  // needed to avoid generating extractelement instructions for unit strided
  // pointers and pointers used in VLS interleaved accesses. This
  // will be changed later to use SVA information.
  if (isa<VPGEPInstruction>(VPInst) || isa<VPSubscriptInst>(VPInst)) {
    if (Plan->getVPlanDA()->getVectorShape(*VPInst).hasKnownStride() ||
        !Plan->getVPlanDA()->isDivergent(*VPInst))
      generateHIR(VPInst, Mask, false /*Widen*/, 0 /*LaneID*/);
    return;
  }

  // Generate a scalar value corresponding to lane 0 for some opcodes to avoid
  // making references non-linear. This is made possible due to support for
  // folding such opcodes. This will be changed later to use SVA information.
  if (isOpcodeForScalarInst(VPInst->getOpcode()))
    generateHIR(VPInst, Mask, false /*Widen*/, 0 /*LaneID*/);
}

void VPOCodeGenHIR::captureCanonicalIV(VPLoop *VPLp) {
  assert(VPLp->getParentLoop() &&
         "Unexpected canonical IV capture call for outermost loop");
  VPBasicBlock *Header = VPLp->getHeader();
  VPBasicBlock *PreHeader = VPLp->getLoopPreheader();
  VPBasicBlock *Latch = VPLp->getLoopLatch();

  // Loop over all of the PHI nodes in the header, looking for a canonical
  // induction variable.
  for (VPPHINode &VPPhi : Header->getVPPhis()) {
    // Search for a PHI of the form and add it as canonical IV for VPLp
    //  BB4: # preds: BB3
    //    ...
    //    [DA: Uni] i64 %vp29572 = hir-copy i64 0 , OriginPhiId: 0
    //    [DA: Uni] br BB5
    //
    //  BB5: # preds: BB4, BB5
    //    [DA: Uni] i64 %vp22876 = phi [i64 %vp29572, BB4], [i64 %vp29600, BB5]
    //    ...
    //    [DA: Uni] i64 %vp21868 = add i64 %vp22876 i64 1
    //    ...
    //    [DA: Uni] i64 %vp29600 = hir-copy i64 %vp21868 , OriginPhiId: 0
    //    [DA: Uni] br i1 %vp23080, BB5, BB6
    //
    //  BB6: # preds: BB5
    if (!VPPhi.getType()->isIntegerTy() ||
        Plan->getVPlanDA()->isDivergent(VPPhi))
      continue;

    // Check for constant start value of 0.
    VPHIRCopyInst *PHCopy =
        dyn_cast<VPHIRCopyInst>(VPPhi.getIncomingValue(PreHeader));
    if (!PHCopy)
      continue;
    VPConstant *StartVPConst = dyn_cast<VPConstant>(PHCopy->getOperand(0));
    if (!StartVPConst || !StartVPConst->getConstant()->isNullValue())
      continue;

    // Check for constant stride of 1.
    VPHIRCopyInst *LatchCopy =
        dyn_cast<VPHIRCopyInst>(VPPhi.getIncomingValue(Latch));
    if (!LatchCopy)
      continue;

    VPInstruction *Inc = dyn_cast<VPInstruction>(LatchCopy->getOperand(0));
    if (!Inc || Inc->getOpcode() != Instruction::Add)
      continue;

    VPConstant *IncConst;
    if (Inc->getOperand(0) == &VPPhi)
      IncConst = dyn_cast<VPConstant>(Inc->getOperand(1));
    else if (Inc->getOperand(1) == &VPPhi)
      IncConst = dyn_cast<VPConstant>(Inc->getOperand(0));
    else
      continue;

    // HIR loops can have only one IV. If a VPlan transformation for some
    // reason introduces more than one IV, it would still be correct to
    // capture the first one. The other inductions would be handled using
    // the ref for SSA deconstructed copies. However, there could be cases
    // where capturing for example the widest type or the IV used in the
    // latch compare check would be better. This code can be revisited if
    // we such a need.
    if (IncConst && IncConst->getConstant()->isOneValue()) {
      LoopIVPhis.insert(&VPPhi);
      VPLoopIVPhiMap[VPLp] = &VPPhi;
      return;
    }
  }
}

void VPOCodeGenHIR::collectLoopEntityInsts() {
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

  // Capture outer loop IV instructions.
  auto captureOuterLoopIVPhi = [&](VPBasicBlock *OuterLpPreheader,
                                   VPLoop *OuterLp) {
    bool LoopIVCaptured = false;
    for (VPInstruction &Inst : *OuterLpPreheader) {
      if (!isa<VPInductionInit>(&Inst))
        continue;

      auto *IndInit = cast<VPInductionInit>(&Inst);
      if (!isSearchLoop() && !IndInit->isMainLoopIV())
        continue;
      if (LoopIVCaptured)
        report_fatal_error(
            "HIR is expected to have only one loop induction variable.");
      LoopIVCaptured = true;

      assert(IndInit->getNumUsers() == 1 && "Invalid induction variable.");
      auto *User = *(IndInit->users().begin());
      VPPHINode *IndPHI = cast<VPPHINode>(User);
      LoopIVPhis.insert(IndPHI);
      VPLoopIVPhiMap[OuterLp] = IndPHI;
    }
  };

  auto *VPLI = Plan->getVPLoopInfo();
  for (auto *OuterLp : *VPLI) {
    VPBasicBlock *OuterLpPreheader =
        cast<VPBasicBlock>(OuterLp->getLoopPreheader());
    for (VPInstruction &Inst : *OuterLpPreheader) {
      if (auto *RedInit = dyn_cast<VPReductionInit>(&Inst)) {
        collectRednVPInsts(RedInit);
      }
    }
    captureOuterLoopIVPhi(OuterLpPreheader, OuterLp);

    // Capture inner loop canonical IV
    for (auto *VPLp : post_order(OuterLp)) {
      if (VPLp == OuterLp)
        continue;
      captureCanonicalIV(VPLp);
    }
  }

  for (auto *V : LoopIVPhis) {
    LLVM_DEBUG(dbgs() << "VPPhi:"; V->dump(); dbgs() << " is loop IV.\n");
    (void)V;
  }

  // Process inductions
  // TODO
}

bool VPOCodeGenHIR::targetHasIntelAVX512() const {
  return TTI->isAdvancedOptEnabled(
      TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX512);
}

void VPOCodeGenHIR::widenNode(const VPInstruction *VPInst, RegDDRef *Mask) {

  // Use VPValue based code generation if mixed CG has not been forced
  if (!getForceMixedCG()) {
    widenNodeImpl(VPInst, Mask);
    return;
  }

  HLInst *WInst = nullptr;
  const HIRSpecifics HIR = VPInst->HIR();
  if (!Mask)
    Mask = CurMaskValue;

  // Always generate code for Phis/Blends in mixed code gen mode except for
  // search loops.
  if (!isSearchLoop() && (isa<VPPHINode>(VPInst) || isa<VPBlendInst>(VPInst))) {
    widenNodeImpl(VPInst, Mask);
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
      widenNodeImpl(Inst, Mask, VPInst);
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

  widenNodeImpl(VPInst, Mask);
}

void VPOCodeGenHIR::serializeInstruction(const VPInstruction *VPInst,
                                         RegDDRef *Mask) {
  // Temp that represents the final value of VPInst after serialization, it
  // remains null for void type instructions.
  RegDDRef *SerialTemp = nullptr;

  // Initialize the temp to undef if it is known to produce a l-val i.e.
  // non-void return type.
  if (!VPInst->getType()->isVoidTy()) {
    RegDDRef *Undef = DDRefUtilities.createUndefDDRef(
        getWidenedType(VPInst->getType(), getVF()));
    auto SerialTempInit = HLNodeUtilities.createCopyInst(Undef, "serial.temp");
    addInstUnmasked(SerialTempInit);
    SerialTemp = SerialTempInit->getLvalDDRef();
    // Map VPInst to SerialTemp in vector map.
    addVPValueWideRefMapping(VPInst, SerialTemp);
  }

  // Genrate serial instruction for each vector lane.
  for (unsigned Lane = 0; Lane < getVF(); ++Lane) {
    // Track marker and HLIf potentially created for masked serialization.
    HLNode *Marker = nullptr;
    HLIf *If = nullptr;

    // If the instruction to be serialized is masked then we create HLIf with
    // predicate "LaneMask == 1" and insert possible extractelements plus the
    // scalar HLInst and corresponding insertelement into the then branch of the
    // HLIf. Pseudo-IR -
    // if (%mask.lane.x == 1) {
    //   %scalar.operand = extract %vec.op, x
    //   %scalar.lane.x = opcode %vec.op, ...
    //   %serial.temp = insert %serial.temp, %scalar.lane.x, x
    // }
    if (Mask) {
      auto ExtractMask = HLNodeUtilities.createExtractElementInst(
          Mask->clone(), Lane, "mask." + Twine(Lane) + ".");
      addInstUnmasked(ExtractMask);
      RegDDRef *LaneMask = ExtractMask->getLvalDDRef();

      // We create the HLIf here and set the insertion point appropriately.
      If = HLNodeUtilities.createHLIf(
          PredicateTy::ICMP_EQ, LaneMask->clone(),
          DDRefUtilities.createConstDDRef(LaneMask->getDestType(), 1));
      addInst(If, nullptr /*Mask*/);
      Marker = HLNodeUtilities.getOrCreateMarkerNode();
      HLNodeUtils::insertAsFirstThenChild(If, Marker);
      InsertPoint = Marker;
    }

    // Generate a scalar HLInst for given VPInstruction using operands for
    // corresponding lane.
    generateHIR(VPInst, nullptr /*Mask*/, false /*Widen*/, Lane,
                false /* OnlyExistingScalarOps */);

    // If the scalar HLInst produces an l-val then we need to insert it into
    // SerialTemp at appropriate lane.
    RegDDRef *ScalarRef = getScalRefForVPVal(VPInst, Lane);
    if (ScalarRef) {
      assert(SerialTemp && "Non-null serial temp expected when serialized "
                           "instruction produces l-val");
      HLInst *InsertInst = nullptr;
      if (auto *VecTy = dyn_cast<FixedVectorType>(ScalarRef->getDestType())) {
        unsigned OrigNumElts = VecTy->getNumElements();
        InsertInst = HLNodeUtilities.createVectorInsert(
            SerialTemp->clone(), ScalarRef->clone(), Lane * OrigNumElts,
            "subvec.insert", SerialTemp->clone());
      } else {
        InsertInst = HLNodeUtilities.createInsertElementInst(
            SerialTemp->clone(), ScalarRef->clone(), Lane, "serial.insert",
            SerialTemp->clone());
      }
      addInstUnmasked(InsertInst);
    }

    // If the serialization is done under a mask, we would have created an HLIf
    // to place the generated instructions. We can safely delete the Marker
    // created inside this HLIf. Subsequent instructions need to be placed after
    // this HLIf and we update the insertion point accordingly.
    if (Mask) {
      assert(If && Marker &&
             "Expected an HLIf and a Marker for masked serialization.");
      HLNodeUtils::remove(Marker);
      InsertPoint = If;
    }
  }

  LLVM_DEBUG(dbgs() << "\n[VPOCGHIR] After serialization"; MainLoop->dump();
             dbgs() << "\n");
}

void VPOCodeGenHIR::scalarizePredicatedUniformInst(const VPInstruction *VPInst,
                                                   RegDDRef *Mask) {
  // Track marker and HLIf potentially created for masked scalarization.
  HLNode *Marker = nullptr;
  HLIf *If = nullptr;

  // We need to mask the scalar instruction appropriately. The mask to use is
  // !AllZeroCheck(Mask) which will ensure that we do the operation if any
  // vector lane is active.
  if (Mask) {
    Mask =
        generateCompareToZero(Mask, nullptr /* InstMask */, false /* Equal */);

    // Consider the case of a uniform load under a mask and the subsequent
    // use of the loaded value.
    //   if (cond) {
    //      v = *unifp;
    //        = v + 1
    //   }
    //
    // The generated HIR will look like the following:
    //     %0 = bitcast.<4 x i1>.i4(cond.vec);
    //     %cmp = %0 != 0;
    //     if (%cmp == 1) {
    //        %.unifload = (unifp)[0];
    //     }
    //        = add %.unifload, 1
    //

    If = HLNodeUtilities.createHLIf(
        PredicateTy::ICMP_EQ, Mask->clone(),
        DDRefUtilities.createConstDDRef(Mask->getDestType(), 1));
    addInst(If, nullptr /* Mask */);
    Marker = HLNodeUtilities.getOrCreateMarkerNode();
    HLNodeUtils::insertAsFirstThenChild(If, Marker);
    InsertPoint = Marker;
  }

  // Generate a scalar HLInst for given VPInstruction using operands for lane 0.
  if (VPInst->getOpcode() == Instruction::Load)
    generateUniformScalarLoad(cast<VPLoadStoreInst>(VPInst));
  else
    generateHIR(VPInst, nullptr /*Mask*/, false /*Widen*/, 0 /*Lane*/);

  // Get l-val that scalar HLInst produces, if any.
  RegDDRef *ScalarRef = getScalRefForVPVal(VPInst, 0 /*Lane*/);
  if (ScalarRef) {
    // Explicit bcast of uniform value to all vector lanes.
    addVPValueWideRefMapping(VPInst, widenRef(ScalarRef->clone(), getVF()));
  }

  // If the scalarization is done under a mask, we would have created an HLIf
  // to place the generated instructions. We can safely delete the Marker
  // created inside this HLIf. Subsequent instructions need to be placed after
  // this HLIf and we update the insertion point accordingly. We also generate
  // l-val initialization with undef here if needed.
  if (Mask) {
    assert(If && Marker &&
           "Expected an HLIf and Marker for masked scalarization.");
    HLNodeUtils::remove(Marker);

    // For the pseudo-example described earlier, note that the load itself is
    // done conditionally but the uses of the loaded value can be unmasked if
    // the use itself is safe such as the use in an add instruction. However,
    // when we generate the equivalent LLVM IR, we end with unnecessary PHIs in
    // the loop header due to what appears to be a potential use of the value
    // assigned in a previous iteration. These unnecessary PHIs increase
    // register pressure and we avoid this by assigning undef to %.unifload
    // before the conditional load.
    //
    //     %.unifload = undef
    //     if (%cmp == 1) {
    //        %.unifload = (unifp)[0];
    //     }
    if (ScalarRef) {
      HLInst *InitInst = generateInitWithUndef(ScalarRef);
      HLNodeUtils::insertBefore(If, InitInst);
    }

    InsertPoint = If;
  }
}

HLLabel *VPOCodeGenHIR::getBlockLabel(const VPBasicBlock *VPBB) {
  auto Itr = VPBBLabelMap.find(VPBB);
  if (Itr != VPBBLabelMap.end())
    return Itr->second;
  else
    return nullptr;
}

HLLabel *VPOCodeGenHIR::createBlockLabel(const VPBasicBlock *VPBB) {
  assert(!VPBBLabelMap.count(VPBB) && "Unexpected block with existing label");
  HLLabel *Label = HLNodeUtilities.createHLLabel(VPBB->getName());
  VPBBLabelMap[VPBB] = Label;
  return Label;
}

HLLabel *VPOCodeGenHIR::getOrCreateBlockLabel(const VPBasicBlock *VPBB) {
  HLLabel *Label;
  if (Label = getBlockLabel(VPBB))
    return Label;

  return createBlockLabel(VPBB);
}

void VPOCodeGenHIR::setBoundsForVectorLoop(VPLoop *VPLp) {
  assert(!isSearchLoop() && "Search loop not supported here");
  HLLoop *VecLoop = VPLoopHLLoopMap[VPLp];

  // Upper bound for the vectorized HLLoop is obtained from VPLoop's utility.
  // Lower bound is obtained from the induction-init instruction in preheader.
  VPValue *VectorTC;
  std::tie(VectorTC, std::ignore) = VPLp->getLoopUpperBound();
  assert(VectorTC && "Vector TC computation not found vector loop.");

  assert(VPLoopIVPhiMap.count(VPLp) && "IV Phi not found for vectorized loop.");
  auto *IVPhi = VPLoopIVPhiMap[VPLp];
  auto IVInitIt = llvm::find_if(
      IVPhi->operands(), [](VPValue *Op) { return isa<VPInductionInit>(Op); });

  assert(IVInitIt != IVPhi->op_end() &&
         "Induction-init not found for vector loop via def-use chain");
  auto *VectorIVInit = cast<VPInductionInit>(*IVInitIt);

  RegDDRef *UBRef = getOrCreateScalarRef(VectorTC, 0 /*Lane*/);
  assert(UBRef && "Non-null ref expected to compute TC.");

  unsigned LoopLevel = VecLoop->getNestingLevel();

  auto *UBCanonExpr = UBRef->getSingleCanonExpr();
  // Subtract 1 from UB.
  if (!UBCanonExpr->isIntConstant()) {
    HLInst *SubInst = HLNodeUtilities.createSub(
        UBRef->clone(),
        DDRefUtilities.createConstDDRef(UBRef->getDestType(), 1), "loop.ub");
    addInstUnmasked(SubInst);
    UBRef = SubInst->getLvalDDRef()->clone();

    // Initialized UB temp will be live-in.
    VecLoop->addLiveInTemp(UBRef);

    assert(UBRef->getSingleCanonExpr()->isNonLinear() && "Non linear UB!");

    // Set the defined at level of new bound to (nesting level - 1) as the
    // bound temp is defined just before the loop.
    UBRef->getSingleCanonExpr()->setDefinedAtLevel(LoopLevel - 1);
  } else {
    UBCanonExpr->addConstant(-1, true);
  }
  VecLoop->setUpperDDRef(UBRef);

  // Set LB of loop if available. We adjust def@level since LB is defined
  // outside the loop.
  RegDDRef *LBRef = getScalRefForVPVal(VectorIVInit, 0 /*Lane*/);
  if (LBRef) {
    VecLoop->setLowerDDRef(LBRef);
    auto *LBCanonExpr = LBRef->getSingleCanonExpr();
    if (!LBRef->isIntConstant())
      LBCanonExpr->setDefinedAtLevel(LoopLevel - 1);
  }

  // TODO: Use PH induction-init-step to set stride? Currently we are assuming
  // VF*UF by default.
  VecLoop->getStrideDDRef()->getSingleCanonExpr()->setConstant(getVF() *
                                                               getUF());
}

void VPOCodeGenHIR::emitBlockLabel(const VPBasicBlock *VPBB) {
  // TODO - search loop representation is currently not explicit.
  if (isSearchLoop())
    return;

  HLLabel *Label = getBlockLabel(VPBB);
  // If we have a block label and it is already attached, just update the
  // instruction insertion point and return.
  if (Label && Label->getParent()) {
    InsertPoint = Label;
    return;
  }
  if (!Label)
    Label = createBlockLabel(VPBB);

  // NOTE - the implementation assumes that the blocks are traversed in RPO
  // order.
  // -- We start the insertion point before the original loop and set the same
  // appropriately once we hit the loop header and exit blocks.
  // -- Reduction related instructions are placed either at current insertion
  // point or reduction related insertion points depending on whether
  // they are hoisted.
  if (!InsertPoint) {
    // We are encountering the first VPBB in merged CFG, insert its label
    // before the original scalar loop.
    HLNodeUtilities.insertBefore(OrigLoop, Label);
  } else if (LoopHeaderBlocks.count(VPBB)) {
    auto *CurVPLoop = Plan->getVPLoopInfo()->getLoopFor(VPBB);
    assert(CurVPLoop && "Non-null CurVPLoop expected.");
    setupHLLoop(CurVPLoop);
    auto *CurHLLoop = VPLoopHLLoopMap[CurVPLoop];
    // Main vector loop is already linked in (except in case merged CFG)
    if (!isSearchLoop() || CurVPLoop->getLoopDepth() > 1)
      HLNodeUtilities.insertAfter(InsertPoint, CurHLLoop);
    HLNodeUtilities.insertAsFirstChild(CurHLLoop, Label);
    // Setup lower/upper bound and stride for vectorized loop i.e. outer most
    // loops.
    if (!isSearchLoop() && CurVPLoop->getLoopDepth() == 1)
      setBoundsForVectorLoop(CurVPLoop);
  } else if (LoopExitBlocks.count(VPBB)) {
    auto *CurVPLoop =
        Plan->getVPLoopInfo()->getLoopFor(VPBB->getSinglePredecessor());
    auto *CurHLLoop = VPLoopHLLoopMap[CurVPLoop];
    HLNodeUtilities.insertAfter(CurHLLoop, Label);
  } else
    HLNodeUtilities.insertAfter(InsertPoint, Label);

  InsertPoint = Label;
}

void VPOCodeGenHIR::emitBlockTerminator(const VPBasicBlock *SourceBB) {
  // TODO - search loop representation is currently not explicit.
  if (isSearchLoop())
    return;

  // Get or create the label corresponding to Block start and create a HLGoto
  // with target set to this label. Return the created HLGoto after adding it
  // to the needed vector.
  auto createGotoAndSetTargetLabel = [this](const VPBasicBlock *Block) {
    HLLabel *Label = getOrCreateBlockLabel(Block);
    HLGoto *Goto = HLNodeUtilities.createHLGoto(Label);
    GotosVector.push_back(Goto);
    return Goto;
  };

  // Insert the target label of Goto immediately after Goto if the label
  // has not been inserted already.
  auto insertUnlinkedGotoLabel = [](HLGoto *Goto) {
    HLLabel *Label = Goto->getTargetLabel();
    if (!Label->getParent())
      HLNodeUtils::insertAfter(Goto, Label);
  };

  // The loop backedge/exit is implicit in DO loops. Do not emit
  // gotos in the vector DO-loop latch block. TODO - look into why we need
  // to suppress goto in PreHeader.
  auto *VLoop = Plan->getVPLoopInfo()->getLoopFor(SourceBB);
  bool IsDoLoopLatch = VLoop != nullptr && VLoop->isLoopLatch(SourceBB) &&
                       VPLoopHLLoopMap[VLoop]->isDo();
  if (SourceBB->getNumSuccessors() && !IsDoLoopLatch &&
      !LoopPreheaderBlocks.count(SourceBB)) {
    const VPBasicBlock *Succ1 = SourceBB->getSuccessor(0);

    // Generate an appropriate if/else or goto target block depending on the
    // block's successor node(s).
    if (SourceBB->getNumSuccessors() == 2) {
      const VPBasicBlock *Succ2 = SourceBB->getSuccessor(1);
      const VPValue *CondBit = SourceBB->getCondBit();
      HLIf *If = nullptr;

      // HIR does not allow gotos between then and else children of an HLIf.
      // Our merged CFG representation uses such gotos and this causes
      // issues when we try to place then and else children properly. For
      // conditional branches outside outermost VPLoop, we resort to generating
      // HLIfs in the following way:
      //    Cond = scalar ref for lane 0
      //    if (Cond == 1)
      //       goto Succ1_Label
      //    else
      //       goto Succ2_Label
      //
      // TODO: We can try restricting this to only HLIfs outside outermost
      // VPLoop that need this. Also, once we start supporting gotos in incoming
      // HIR, HLIfs inside VPLoop will have the same issue. We need to mimic HIR
      // framework to support such cases.
      if (!Plan->getVPLoopInfo()->getLoopFor(SourceBB)) {
        auto *CondRef = getOrCreateScalarRef(CondBit, 0 /*ScalarLaneID*/);
        assert(CondRef && "Null scalar condition ref!");
        If = HLNodeUtilities.createHLIf(
            PredicateTy::ICMP_EQ, CondRef,
            DDRefUtilities.createConstDDRef(CondRef->getDestType(), 1));
        addInst(If, nullptr /* Mask */);

        HLGoto *ThenGoto = createGotoAndSetTargetLabel(Succ1);
        HLNodeUtils::insertAsFirstThenChild(If, ThenGoto);

        HLGoto *ElseGoto = createGotoAndSetTargetLabel(Succ2);
        HLNodeUtils::insertAsFirstElseChild(If, ElseGoto);
        return;
      }

      //
      // Check and set flags to see if then or else successors are fall through.
      //
      // -- neither fall through
      // br cond succ1 succ2
      // succ1
      //   ...
      //   br ifmerge
      // succ2
      //   ...
      //   br ifmerge
      // ifmerge:
      //   ...
      //
      // -- then successor falls through
      // br cond ifmerge succ2
      // succ2:
      //    ...
      //    br ifmerge
      // ifmerge:
      //    ...
      //
      // - else successor falls through
      // br cond succ1 ifmerge
      // succ1:
      //    ...
      //    br ifmerge
      // ifmerge:
      //    ...
      auto *PDT = cast<VPlanVector>(Plan)->getPDT();
      bool ThenFallThru = PDT->dominates(Succ1, SourceBB);
      bool ElseFallThru = PDT->dominates(Succ2, SourceBB);

      // Predicate to use in if condition
      PredicateTy Pred = PredicateTy::ICMP_EQ;

      // Swap the successors for the case where then successor falls through to
      // avoid ifs like:  if (cond) {} else { ... }
      if (ThenFallThru && !ElseFallThru) {
        ThenFallThru = false;
        ElseFallThru = true;
        std::swap(Succ1, Succ2);
        Pred = PredicateTy::ICMP_NE;
      }

      if (auto *Ext = dyn_cast<VPExternalDef>(CondBit)) {
        auto *ScalarIf = cast<VPIfCond>(Ext->getOperandHIR())->getIf();
        If = ScalarIf->cloneEmpty();
        for (RegDDRef *Ref : If->op_ddrefs())
          MainLoop->addLiveInTemp(Ref);
      } else {
        auto *CondRef = getOrCreateScalarRef(CondBit, 0 /*ScalarLaneID*/);
        assert(CondRef && "Null scalar condition ref!");
        If = HLNodeUtilities.createHLIf(
            Pred, CondRef,
            DDRefUtilities.createConstDDRef(CondRef->getDestType(), 1));
      }
      addInst(If, nullptr /* Mask */);

      HLGoto *ThenGoto = createGotoAndSetTargetLabel(Succ1);
      HLNodeUtils::insertAsFirstThenChild(If, ThenGoto);
      // If then does not fall through, insert Succ1 label after then
      // goto if the label has not been inserted earlier.
      if (!ThenFallThru)
        insertUnlinkedGotoLabel(ThenGoto);

      // If else does not fall through generate the else part
      if (!ElseFallThru) {
        HLGoto *ElseGoto = createGotoAndSetTargetLabel(Succ2);
        HLNodeUtils::insertAsFirstElseChild(If, ElseGoto);
        assert(!ThenFallThru && "if-else with unexpected empty if-then");
        insertUnlinkedGotoLabel(ElseGoto);
      }

      // Insert the if-merge block label if not inserted already. This is
      // placed after the if or after the loop if the conditional branch
      // is in a loop latch(check that Succ1 is a loop header block).
      assert(PDT->getNode(SourceBB) && "SourceBB should be in PDT");
      auto *IfSuccBlock =
          ElseFallThru ? Succ2 : PDT->getNode(SourceBB)->getIDom()->getBlock();
      HLNode *IfSuccLabel = getOrCreateBlockLabel(IfSuccBlock);
      if (!IfSuccLabel->getParent()) {
        if (LoopHeaderBlocks.count(Succ1)) {
          auto *CurVPLoop = Plan->getVPLoopInfo()->getLoopFor(Succ1);
          auto *CurHLLoop = VPLoopHLLoopMap[CurVPLoop];
          HLNodeUtils::insertAfter(CurHLLoop, IfSuccLabel);
        } else {
          HLNodeUtils::insertAfter(If, IfSuccLabel);
        }
      }

      LLVM_DEBUG(dbgs() << "Uniform IF seen\n");
    } else {
      HLGoto *Goto = createGotoAndSetTargetLabel(Succ1);
      addInst(Goto, nullptr /* Mask */);
    }
  }
}

void VPOCodeGenHIR::eliminateRedundantGotosLabels(void) {
  LLVM_DEBUG(dbgs() << "Loop before redundant gotos/labels removal: \n");
  LLVM_DEBUG(MainLoop->getParent()->dump());

  // Eliminate redundant Gotos.
  HLNodeUtils::RequiredLabelsTy RequiredLabels;
  HLNodeUtils::eliminateRedundantGotos(GotosVector, RequiredLabels);

  // Eliminate redundant labels.
  for (auto It : VPBBLabelMap) {
    auto *Label = It.second;
    if (!RequiredLabels.count(Label))
      HLNodeUtils::remove(Label);
  }

  LLVM_DEBUG(dbgs() << "Loop after redundant gotos/labels removal: \n");
  LLVM_DEBUG(MainLoop->getParent()->dump());
}

void VPOCodeGenHIR::setIsVecMDForHLLoops() {
  for (auto *HLp : OutgoingScalarHLLoops) {
    if (HLp && HLp->isAttached()) {
      if (HLp->getLoopStringMetadata("llvm.loop.vectorize.enable") != nullptr)
        setHLLoopMD(HLp, "llvm.loop.isvectorized");
    }
  }
}

bool VPOCodeGenHIR::getTreeConflictsLowered() {
  return TreeConflictsLowered;
}

void VPOCodeGenHIR::setTreeConflictsLowered(bool Lowered) {
  TreeConflictsLowered = Lowered;
}

void VPOCodeGenHIR::lowerRemarksForVectorLoops() {
  auto LowerRemarksForLoop = [this](VPLoop *VPL) {
    // Emit statistics related remarks for the loop.
    Plan->getOptRptStatsForLoop(VPL).emitRemarks(
        ORBuilder, VPL, const_cast<VPLoopInfo *>(Plan->getVPLoopInfo()));

    auto OR = VPL->getOptReport();
    if (!OR)
      return;

    // Identify the HIR loop that corresponds to current VPLoop.
    assert(VPLoopHLLoopMap.count(VPL) &&
           "Could not find HIR loop corresponding to VPLoop.");
    HLLoop *HIRLp = VPLoopHLLoopMap[VPL];

    // Drop any existing opt-report and re-add the opt-report tracked
    // for current VPLoop to corresponding HIR loop. Remarks from prior
    // components are all captured in VPLoop's opt-report during VPlan CFG
    // construction.
    HIRLp->eraseOptReport();
    HIRLp->setOptReport(OR);
  };

  for (VPLoop *OuterLoop : *Plan->getVPLoopInfo())
    for (auto *VLP : post_order(OuterLoop))
      LowerRemarksForLoop(VLP);
}

void VPOCodeGenHIR::emitRemarksForScalarLoops() {
  // Naive visitor to erase opt-reports from all loops in given loop-nest.
  struct EraseOptReportLoopVisitor
      : public HIRVisitor<EraseOptReportLoopVisitor> {
    void visitLoop(HLLoop *L) { L->eraseOptReport(); }
  };

  for (auto &ScalarLoopPair : OutgoingScalarHLLoopsMap) {
    auto *ScalarLpVPI = ScalarLoopPair.first;
    HLLoop *ScalarHLp = ScalarLoopPair.second;

    // Remove all opt-reports from scalar loop nest. They have been moved to
    // vectorized loops.
    ScalarHLp->eraseOptReport();
    EraseOptReportLoopVisitor ELV;
    ELV.visit(ScalarHLp);

    // Emit remarks collected for scalar loop instruction into outgoing scalar
    // loop's opt-report.
    auto EmitScalarLpVPIRemarks = [this, ScalarHLp](auto *LpVPI) {
      for (auto R : LpVPI->getOriginRemarks())
        ORBuilder(*ScalarHLp).addOrigin(R.RemarkID);

      for (auto R : LpVPI->getGeneralRemarks())
        ORBuilder(*ScalarHLp).addRemark(R.MessageVerbosity, R.RemarkID, R.Arg);
    };

    if (auto *RemLp = dyn_cast<VPScalarRemainderHIR>(ScalarLpVPI))
      EmitScalarLpVPIRemarks(RemLp);
    else
      EmitScalarLpVPIRemarks(cast<VPScalarPeelHIR>(ScalarLpVPI));
  }
}
} // end namespace llvm

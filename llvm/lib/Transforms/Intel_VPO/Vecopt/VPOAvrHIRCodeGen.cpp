//===-- VPOAvrHIRCodeGen.cpp ----------------------------------------------===//
//
//   Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the HIR vector Code generation from AVR.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_VPO/Vecopt/VPOAvrHIRCodeGen.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrDecomposeHIR.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrVisitor.h"
#include "llvm/IR/Intel_LoopIR/HIRVisitor.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/Intel_GeneralUtils.h"

#define DEBUG_TYPE "VPODriver"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

STATISTIC(LoopsVectorized, "Number of HIR loops vectorized");

static cl::opt<bool>
    DisableStressTest("disable-vpo-stress-test", cl::init(false), cl::Hidden,
                      cl::desc("Disable VPO Vectorizer Stress Testing"));

/// Don't vectorize loops with a known constant trip count below this number if
/// set to a non zero value.
static cl::opt<unsigned> TinyTripCountThreshold(
    "vpo-vectorizer-min-trip-count", cl::init(0), cl::Hidden,
    cl::desc("Don't vectorize loops with a constant "
             "trip count that is smaller than this value."));

static RegDDRef *getConstantSplatDDRef(DDRefUtils &DDRU, Constant *ConstVal,
                                       unsigned VL);

namespace llvm {
namespace vpo {

class AVRCGVisit {
private:
  AVRCodeGenHIR *ACG;
  RegDDRef *MaskDDRef;
  std::string NodeName;
  HLNodeUtils &HNU;
  DDRefUtils &DDRU;

  // Add instruction at end of main loop after adding mask if mask is not null.
  void addInst(HLInst *Inst) {
    if (MaskDDRef)
      Inst->setMaskDDRef(MaskDDRef->clone());
    HNU.insertAsLastChild(ACG->getMainLoop(), Inst);
  }

public:
  AVRCGVisit(AVRCodeGenHIR *ACG, RegDDRef *MaskDDRef, const Twine &Name)
      : ACG(ACG), MaskDDRef(MaskDDRef), NodeName(Name.str()),
        HNU(ACG->getMainLoop()->getHLNodeUtils()),
        DDRU(ACG->getMainLoop()->getDDRefUtils()) {}

  const std::string &getNodeName() const { return NodeName; }

  // Visit Functions
  void visit(AVR *ANode){};
  void visit(AVRValueHIR *AVal);
  void visit(AVRValue *AVal);

  void postVisit(AVR *ANode){};
  void postVisit(AVRExpression *AExpr);
  void postVisit(AVRPredicate *APredicate);

  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }
};
} // End vpo namespace
} // End llvm namespace

void AVRCGVisit::visit(AVRValueHIR *AVal) {
  if (ACG->findWideAvrRef(AVal->getNumber()))
    return;

  auto RefVal = AVal->getValue();
  RegDDRef *WideRef = nullptr;

  if (auto RDDRef = dyn_cast<RegDDRef>(RefVal)) {
    WideRef = ACG->widenRef(RDDRef);
  } else {
    assert(isa<BlobDDRef>(RefVal) && "Expected Blob DDRef");

    auto BRefVal = cast<BlobDDRef>(RefVal);
    if (auto WInst = ACG->findWideInst(BRefVal->getSymbase())) {
      WideRef = WInst->getLvalDDRef();
    } else {
      WideRef = DDRU.createScalarRegDDRef(
          BRefVal->getSymbase(),
          const_cast<CanonExpr *>(BRefVal->getCanonExpr()));
      WideRef = ACG->widenRef(WideRef);
    }
  }

  ACG->setWideAvrRef(AVal->getNumber(), WideRef);
}

void AVRCGVisit::visit(AVRValue *AVal) {
  if (ACG->findWideAvrRef(AVal->getNumber()))
    return;

  RegDDRef *WideRef = nullptr;
  Constant *CVal = const_cast<Constant *>(AVal->getConstant());

  if (CVal) {
    WideRef = getConstantSplatDDRef(DDRU, CVal, ACG->getVL());
  } else {
    for (AVRExpression *ReachingDef : AVal->getReachingDefs()) {
      auto Num = ReachingDef->getNumber();
      auto TRef = ACG->getWideAvrRef(Num);

      if (WideRef) {
        auto TInst =
            HNU.createBinaryHLInst(Instruction::Or, WideRef->clone(),
                                   TRef->clone(), getNodeName() + "RDef");
        addInst(TInst);
        WideRef = TInst->getLvalDDRef();
      } else {
        WideRef = TRef;
      }
    }
  }

  ACG->setWideAvrRef(AVal->getNumber(), WideRef);
}

void AVRCGVisit::postVisit(AVRExpression *AExpr) {
  if (ACG->findWideAvrRef(AExpr->getNumber()))
    return;

  SmallVector<RegDDRef *, 2> Operands;
  auto NumOperands = AExpr->getNumOperands();
  HLInst *WideInst;

  for (unsigned Index = 0; Index < NumOperands; ++Index) {
    auto Op = AExpr->getOperand(Index);
    auto OpRef = ACG->getWideAvrRef(Op->getNumber());

    Operands.push_back(OpRef);
  }

  if (NumOperands == 1) {
    auto VecTy = VectorType::get(AExpr->getType(), ACG->getVL());
    WideInst = HNU.createCastHLInst(VecTy, AExpr->getOperation(),
                                    Operands[0]->clone(), getNodeName());
  } else if (NumOperands == 2) {
    auto Condition = AExpr->getCondition();

    // Compare instruction
    if (Condition != CmpInst::BAD_ICMP_PREDICATE) {
      WideInst = HNU.createCmp(Condition, Operands[0]->clone(),
                               Operands[1]->clone(), getNodeName());
    } else {
      auto Op = AExpr->getOperation();

      if (Op == AVRExpression::UMax) {
        WideInst = HNU.createSelect(CmpInst::ICMP_UGT, Operands[0]->clone(),
                                    Operands[1]->clone(), Operands[0]->clone(),
                                    Operands[1]->clone(), getNodeName());
      } else if (Op == AVRExpression::SMax) {
        WideInst = HNU.createSelect(CmpInst::ICMP_SGT, Operands[0]->clone(),
                                    Operands[1]->clone(), Operands[0]->clone(),
                                    Operands[1]->clone(), getNodeName());
      } else {
        WideInst =
            HNU.createBinaryHLInst(AExpr->getOperation(), Operands[0]->clone(),
                                   Operands[1]->clone(), getNodeName());
      }
    }
  }

  addInst(WideInst);
  ACG->setWideAvrRef(AExpr->getNumber(), WideInst->getLvalDDRef());
}

void AVRCGVisit::postVisit(AVRPredicate *APredicate) {
  if (ACG->findWideAvrRef(APredicate->getNumber()))
    return;

  const SmallVectorImpl<AVRPredicate::IncomingTy> &IncomingPreds =
      APredicate->getIncoming();

  unsigned IncomingNum = IncomingPreds.size();

  HLInst *VecInst;
  RegDDRef *WideRef = nullptr;

  if (IncomingNum == 0) {
    auto OneVal =
        ConstantInt::get(Type::getInt1Ty(ACG->getFunction().getContext()), 1);

    // Assign true to predicate
    WideRef = getConstantSplatDDRef(DDRU, OneVal, ACG->getVL());
  } else {
    for (unsigned Index = 0; Index < IncomingNum; ++Index) {
      auto &Incoming = IncomingPreds[Index];
      auto FirstNum = Incoming.first->getNumber();
      auto PredRef = ACG->getWideAvrRef(FirstNum);

      if (Incoming.second) {
        auto SecondRef = ACG->getWideAvrRef(Incoming.second->getNumber());

        VecInst = HNU.createBinaryHLInst(Instruction::And, PredRef->clone(),
                                         SecondRef->clone(), getNodeName());
        addInst(VecInst);
        PredRef = VecInst->getLvalDDRef();
      }

      if (WideRef) {
        VecInst = HNU.createBinaryHLInst(Instruction::Or, WideRef->clone(),
                                         PredRef->clone(), getNodeName());

        addInst(VecInst);
        WideRef = VecInst->getLvalDDRef();
      } else {
        WideRef = PredRef;
      }
    }
  }

  ACG->setWideAvrRef(APredicate->getNumber(), WideRef);
}

static RegDDRef *getConstantSplatDDRef(DDRefUtils &DDRU, Constant *ConstVal,
                                       unsigned VL) {
  Constant *ConstVec = ConstantVector::getSplat(VL, ConstVal);
  if (isa<ConstantDataVector>(ConstVec))
    return DDRU.createConstDDRef(cast<ConstantDataVector>(ConstVec));
  if (isa<ConstantAggregateZero>(ConstVec))
    return DDRU.createConstDDRef(cast<ConstantAggregateZero>(ConstVec));
  if (isa<ConstantVector>(ConstVec))
    return DDRU.createConstDDRef(cast<ConstantVector>(ConstVec));
  llvm_unreachable("Unhandled vector type");
}

static RegDDRef *getConstantSplatDDRef(const RegDDRef *Op, unsigned VL) {
  assert((Op->isIntConstant() || Op->isFPConstant()) &&
         "Unexpected operand for getConstantSplatDDRef()");
  Constant *ConstVal = nullptr;
  int64_t ConstValInt;
  ConstantFP *ConstValFp;
  if (Op->isFPConstant(&ConstValFp))
    ConstVal = ConstValFp;
  else if (Op->isIntConstant(&ConstValInt))
    ConstVal = ConstantInt::get(Op->getDestType(), ConstValInt);
  else
    return nullptr;
  return getConstantSplatDDRef(Op->getDDRefUtils(), ConstVal, VL);
}

ReductionHIRMngr::ReductionHIRMngr(AVR *Avr) {
  ReductionClause &RC = cast<AVRWrn>(Avr)->getWrnNode()->getRed();
  for (ReductionItem *Ri : RC.items()) {

    auto usedInOnlyOnePhiNode = [](Value *V) {
      PHINode *Phi = 0;
      for (auto U : V->users())
        if (isa<PHINode>(U)) {
          if (Phi) // More than one Phi node
            return (PHINode *)nullptr;
          Phi = cast<PHINode>(U);
        }
      return Phi;
    };

    Value *RedVarPtr = Ri->getOrig();
    assert(isa<PointerType>(RedVarPtr->getType()) &&
           "Variable specified in Reduction directive should be a pointer");

    for (auto U : RedVarPtr->users()) {
      if (!isa<LoadInst>(U))
        continue;
      if (auto PhiNode = usedInOnlyOnePhiNode(U)) {
        Ri->setInitializer(U);
        if (PhiNode->getIncomingValue(0) == U)
          Ri->setCombiner(PhiNode->getIncomingValue(1));
        else
          Ri->setCombiner(PhiNode->getIncomingValue(0));
        break;
      }
    }

    ReductionMap[Ri->getCombiner()] = Ri;
  }
}

bool ReductionHIRMngr::isReductionVariable(const Value *Val) {
  return ReductionMap.find(Val) != ReductionMap.end();
}

ReductionItem *ReductionHIRMngr::getReductionInfo(const Value *Val) {
  return ReductionMap[Val];
}

Constant *ReductionHIRMngr::getRecurrenceIdentity(ReductionItem *RedItem,
                                                  Type *Ty) {

  assert((Ty->isFloatTy() || Ty->isIntegerTy()) &&
         "Expected FP or Integer scalar type");
  ReductionItem::WRNReductionKind RKind = RedItem->getType();
  RecurrenceDescriptor::RecurrenceKind RDKind;
  switch (RKind) {
  case ReductionItem::WRNReductionBxor:
    RDKind = RecurrenceDescriptor::RK_IntegerXor;
    break;
  case ReductionItem::WRNReductionBand:
    RDKind = RecurrenceDescriptor::RK_IntegerAnd;
    break;
  case ReductionItem::WRNReductionBor:
    RDKind = RecurrenceDescriptor::RK_IntegerOr;
    break;
  case ReductionItem::WRNReductionSum:
    RDKind = Ty->isFloatTy() ? RecurrenceDescriptor::RK_FloatAdd
                             : RecurrenceDescriptor::RK_IntegerAdd;
    break;
  case ReductionItem::WRNReductionMult:
    RDKind = Ty->isFloatTy() ? RecurrenceDescriptor::RK_FloatMult
                             : RecurrenceDescriptor::RK_IntegerMult;
    break;
  default:
    llvm_unreachable("Unknown recurrence kind");
  }
  return RecurrenceDescriptor::getRecurrenceIdentity(RDKind, Ty);
}

// TBD - once we update to the latest loopopt sources, make use of
// getStrideAtLevel utility
bool AVRCodeGenHIR::isConstStrideRef(const RegDDRef *Ref, unsigned NestingLevel,
                                     int64_t *CoeffPtr) {
  if (Ref->isTerminalRef())
    return false;

  if (Ref->isAddressOf())
    return false;

  const CanonExpr *FirstCE = nullptr;

  // Return false for cases where the lowest dimension has trailing struct
  // field offsets.
  if (Ref->hasTrailingStructOffsets(1))
    return false;

  // Check that canon exprs for dimensions other than the first are
  // invariant.
  for (auto I = Ref->canon_begin(), E = Ref->canon_end(); I != E; ++I) {
    if (!FirstCE) {
      FirstCE = *I;
      continue;
    }

    if (!(*I)->isInvariantAtLevel(NestingLevel))
      return false;
  }

  // Consider a[(i1 + 1) & 3], this is changed to a[zext.i2.i64(i1 + 1)] - we
  // do not want to treat this reference as unit stride.
  if (FirstCE->isSExt() || FirstCE->isZExt()) {
    auto SrcTy = FirstCE->getSrcType();
    auto DestTy = FirstCE->getDestType();

    // Do not go conservative for the common case.
    bool OK = false;
    if (SrcTy->isIntegerTy(32) && DestTy->isIntegerTy(64)) {
      // Check for simple sum of IVs
      OK = true;
      for (auto IV = FirstCE->iv_begin(), IVE = FirstCE->iv_end(); IV != IVE;
           ++IV) {
        int64_t Coeff;
        unsigned BlobCoeff;
        FirstCE->getIVCoeff(IV, &BlobCoeff, &Coeff);

        if (Coeff == 0)
          continue;

        if (BlobCoeff != 0 || Coeff != 1) {
          OK = false;
          break;
        }
      }
    }

    if (!OK)
      return false;
  }

  if (FirstCE->isNonLinear() || FirstCE->getDefinedAtLevel() >= NestingLevel)
    return false;

  if (FirstCE->hasIVBlobCoeff(NestingLevel))
    return false;

  auto IVConstCoeff = FirstCE->getIVConstCoeff(NestingLevel);
  if (IVConstCoeff == 0)
    return false;

  if (CoeffPtr)
    *CoeffPtr = IVConstCoeff;

  return true;
}

namespace {
class HandledCheck final : public HLNodeVisitorBase {
private:
  bool IsHandled;
  const HLLoop *OrigLoop;
  TargetLibraryInfo *TLI;
  unsigned VL;
  bool UnitStrideRefSeen;
  bool MemRefSeen;
  unsigned LoopLevel;

  void visitRegDDRef(RegDDRef *RegDD);
  void visitCanonExpr(CanonExpr *CExpr);

public:
  HandledCheck(const HLLoop *OrigLoop, TargetLibraryInfo *TLI, int VL)
      : IsHandled(true), OrigLoop(OrigLoop), TLI(TLI), VL(VL),
        UnitStrideRefSeen(false), MemRefSeen(false) {
    LoopLevel = OrigLoop->getNestingLevel();
  }

  void visit(HLDDNode *Node);

  void visit(HLNode *Node) {
    DEBUG(errs() << "VPO_OPTREPORT: Loop not handled - unsupported HLNode\n");
    IsHandled = false;
  }

  void postVisit(HLNode *Node) {}

  bool isDone() const override { return (!IsHandled); }
  bool isHandled() { return IsHandled; }
  bool getUnitStrideRefSeen() { return UnitStrideRefSeen; }
  bool getMemRefSeen() { return MemRefSeen; }
};
} // End anonymous namespace

void HandledCheck::visit(HLDDNode *Node) {

  if (!isa<HLInst>(Node) && !isa<HLIf>(Node)) {
    DEBUG(errs() << "VPO_OPTREPORT: Loop not handled - only HLInst/HLIf are "
                    "supported\n");
    IsHandled = false;
    return;
  }

  // Calls supported are masked/non-masked svml and non-masked intrinsics.
  if (HLInst *Inst = dyn_cast<HLInst>(Node)) {
    if (Inst->isCallInst()) {
      const CallInst *Call = cast<CallInst>(Inst->getLLVMInstruction());
      StringRef CalledFunc = Call->getCalledFunction()->getName();

      if (Inst->getParent() != OrigLoop &&
          (VL > 1 && !TLI->isFunctionVectorizable(CalledFunc, VL))) {
        // Masked svml calls are supported, but masked intrinsics are not at
        // the moment.
        DEBUG(Inst->dump());
        DEBUG(errs() << "VPO_OPTREPORT: Loop not handled - masked intrinsic\n");
        IsHandled = false;
        return;
      }

      // Quick hack to avoid loops containing fabs in 447.dealII from becoming
      // vectorized due to bug in unrolling. The problem involves loop index
      // variable that spans outside the array range, resulting in segfault. 
      // floor calls are also temporarily disabled until FeatureOutlining is
      // fixed (CQ410864)
      if (CalledFunc == "fabs" || CalledFunc == "floor") {
        DEBUG(Inst->dump());
        DEBUG(errs() <<
          "VPO_OPTREPORT: Loop not handled - fabs/floor call disabled\n");
        IsHandled = false;
        return;
      }

      Intrinsic::ID ID = getVectorIntrinsicIDForCall(Call, TLI);
      if ((VL > 1 && !TLI->isFunctionVectorizable(CalledFunc, VL)) && !ID) {
        DEBUG(errs()
              << "VPO_OPTREPORT: Loop not handled - call not vectorizable\n");
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
  int64_t IVConstCoeff;

  if (!VectorType::isValidElementType(RegDD->getSrcType())) {
    DEBUG(RegDD->getSrcType()->dump());
    DEBUG(errs() << "VPO_OPTREPORT: Loop not handled - invalid element type\n");
    IsHandled = false;
    return;
  }

  if (AVRCodeGenHIR::isConstStrideRef(RegDD, LoopLevel, &IVConstCoeff) &&
      IVConstCoeff == 1)
    UnitStrideRefSeen = true;

  // Visit CanonExprs inside the RegDDRefs
  for (auto Iter = RegDD->canon_begin(), End = RegDD->canon_end(); Iter != End;
       ++Iter) {
    visitCanonExpr(*Iter);
  }

  // Visit GEP Base
  if (RegDD->hasGEPInfo()) {
    MemRefSeen = true;

    auto BaseCE = RegDD->getBaseCE();

    if (!BaseCE->isInvariantAtLevel(LoopLevel)) {
      DEBUG(
          errs() << "VPO_OPTREPORT: Loop not handled - BaseCE not invariant\n");
      IsHandled = false;
      return;
    }

    visitCanonExpr(RegDD->getBaseCE());
  }
}

// Checks Canon Expr to see if we support it. Currently, we do not
// support blob IV coefficients
void HandledCheck::visitCanonExpr(CanonExpr *CExpr) {
  if (CExpr->hasIVBlobCoeff(LoopLevel)) {
    DEBUG(errs()
          << "VPO_OPTREPORT: Loop not handled - IV with blob coefficient\n");
    IsHandled = false;
    return;
  }
}

// TODO: Take as input a VPOVecContext that indicates which AVRLoop(s)
// is (are) to be vectorized, as identified by the vectorization scenario
// evaluation.
// FORNOW there is only one AVRLoop per region, so we will re-discover
// the same AVRLoop that the vecScenarioEvaluation had "selected".
bool AVRCodeGenHIR::loopIsHandled(unsigned int VF) {
  AVRWrn *AWrn = nullptr;
  WRNVecLoopNode *WVecNode;
  HLLoop *Loop = nullptr;

  // We expect avr to be a AVRWrn node
  if (!(AWrn = dyn_cast<AVRWrn>(Avr))) {
    DEBUG(errs() << "VPO_OPTREPORT: Loop not handled - expected AVRWrn node\n");
    return false;
  }

  WVecNode = AWrn->getWrnNode();

  if (!ALoop)
    ALoop = AVRUtils::findAVRLoop(AWrn);

  // Check that we have an AVRLoop
  if (!ALoop) {
    DEBUG(errs()
          << "VPO_OPTREPORT: Loop not handled - AVRLoop child not found\n");
    return false;
  }

  Loop = WVecNode->getHLLoop();
  assert(Loop && "Null HLLoop.");
  setOrigLoop(Loop);

  // Only handle normalized loops
  if (!OrigLoop->isNormalized()) {
    DEBUG(errs()
          << "VPO_OPTREPORT: Loop not handled - loop not in normalized form\n");
    return false;
  }

  // We are working with normalized loops, trip count is loop UpperBound + 1.
  auto UBRef = Loop->getUpperDDRef();
  int64_t UBConst;

  if (UBRef->isIntConstant(&UBConst)) {
    auto ConstTripCount = UBConst + 1;

    // Check for minimum trip count threshold
    if (TinyTripCountThreshold && ConstTripCount <= TinyTripCountThreshold) {
      DEBUG(
          errs()
          << "VPO_OPTREPORT: Loop not handled - loop with small trip count\n");
      return false;
    }

    // Check that main vector loop will have atleast one iteration
    if (ConstTripCount < VL) {
      DEBUG(errs()
            << "VPO_OPTREPORT: Loop not handled - zero iteration main loop\n");
      return false;
    }

    // Set constant trip count
    setTripCount((uint64_t)ConstTripCount);
  }

  bool UnitStrideSeen = false;
  bool MemRefSeen = false;
  for (auto Itr = ALoop->child_begin(), End = ALoop->child_end(); Itr != End;
       ++Itr) {
    // Itr->dump(PrintNumber);
    if (auto AAssign = dyn_cast<AVRAssignHIR>(Itr)) {
      auto HInst = AAssign->getHIRInstruction();
      auto Inst = HInst->getLLVMInstruction();

      if (Inst->mayThrow()) {
        DEBUG(HInst->dump());
        DEBUG(errs()
              << "VPO_OPTREPORT: Loop not handled - instruction may throw\n");
        return false;
      }

      auto Opcode = Inst->getOpcode();

      if ((Opcode == Instruction::UDiv || Opcode == Instruction::SDiv ||
           Opcode == Instruction::URem || Opcode == Instruction::SRem) &&
          (HInst->getParent() != OrigLoop)) {
        DEBUG(HInst->dump());
        DEBUG(errs()
              << "VPO_OPTREPORT: Loop not handled - DIV/REM instruction\n");
        return false;
      }

      auto TLval = HInst->getLvalDDRef();

      if (TLval && TLval->isTerminalRef() &&
          OrigLoop->isLiveOut(TLval->getSymbase()) &&
          HInst->getParent() != OrigLoop) {
        DEBUG(HInst->dump());
        DEBUG(errs() << "VPO_OPTREPORT: Liveout conditional scalar assign "
                        "not handled\n");
        return false;
      }

      HandledCheck NodeCheck(OrigLoop, TLI, VL);
      HLDDNode *INode = AAssign->getHIRInstruction();

      OrigLoop->getHLNodeUtils().visit(NodeCheck, INode);
      if (!NodeCheck.isHandled())
        return false;

      if (NodeCheck.getUnitStrideRefSeen())
        UnitStrideSeen = true;

      if (NodeCheck.getMemRefSeen())
        MemRefSeen = true;
    } else if (isa<AVRPredicate>(Itr)) {
      ;
    } else if (auto AIf = dyn_cast<AVRIfHIR>(Itr)) {
      HandledCheck NodeCheck(OrigLoop, TLI, VL);
      HLDDNode *INode = AIf->getCompareInstruction();

      OrigLoop->getHLNodeUtils().visit(NodeCheck, INode);
      if (!NodeCheck.isHandled())
        return false;
    } else {
      DEBUG(Itr->dump());
      DEBUG(
          errs() << "VPO_OPTREPORT: Loop not handled - unsupported AVR kind\n");
      return false;
    }
  }

  // If we are not in stress testing mode, only vectorize when some
  // unit stride refs are seen. Still vectorize the case when no mem refs
  // are seen. Remove this check once vectorizer cost model is fully
  // implemented.
  if (DisableStressTest && MemRefSeen && !UnitStrideSeen) {
    DEBUG(
        errs()
        << "VPO_OPTREPORT: Loop not handled - all mem refs non unit-stride\n");
    return false;
  }

  setALoop(ALoop);
  setWVecNode(WVecNode);

  // DEBUG(errs() << "Handled loop\n");
  return true;
}

bool AVRCodeGenHIR::isSmallShortAddRedLoop() {
  // Return false if loop does not have any reductions
  auto SRCL = SRA->getSafeReductionChain(OrigLoop);
  if (SRCL.empty())
    return false;

  unsigned Count = 0;
  bool Found = false;

  // Check for loop with at most two instructions of which at least one
  // is an add reduction of short type values into an I32/I64.
  for (auto Itr = ALoop->child_begin(), End = ALoop->child_end(); Itr != End;
       ++Itr) {
    ++Count;
    if (Count > 2)
      return false;

    auto AAssign = dyn_cast<AVRAssignHIR>(Itr);
    // Return false if the loop has anything other than AVRAssigns
    if (!AAssign)
      return false;

    auto HInst = AAssign->getHIRInstruction();
    if (HInst->getLLVMInstruction()->getOpcode() != Instruction::Add)
      continue;

    if (!SRA->isSafeReduction(HInst))
      continue;

    auto Lval = HInst->getLvalDDRef();
    auto Op1 = HInst->getOperandDDRef(1);
    auto Op2 = HInst->getOperandDDRef(2);

    if ((Lval->getDestType()->isIntegerTy(32) ||
         Lval->getDestType()->isIntegerTy(64)) &&
        (Op1->getSrcType()->isIntegerTy(16) ||
         Op2->getSrcType()->isIntegerTy(16)))
      Found = true;
  }

  return Found;
}

// TODO: Change all VL occurences with VF
// TODO: Have this method take a VecContext as input, which indicates which
// AVRLoops in the region to vectorize, and how (using what VF).
bool AVRCodeGenHIR::vectorize(unsigned int VL) {
  bool LoopHandled;

  setVL(VL);
  assert(VL >= 1);
  if (VL == 1) {
    DEBUG(errs() << "VPO_OPTREPORT: Loop not handled - VL is 1\n");
    return false;
  }

  LoopHandled = loopIsHandled(VL);
  if (!LoopHandled)
    return false;

  SRA->computeSafeReductionChains(OrigLoop);

  // Workaround for perf regressions - suppress vectorization of some small
  // loops with add reduction of short values until cost model can be refined.
  if (isSmallShortAddRedLoop()) {
    DEBUG(errs()
          << "VPO_OPTREPORT: Suppress vectorization - SmallShortAddRedLoop\n");
    return false;
  }

  DEBUG(errs() << "VPO_OPTREPORT: VPO handled loop, VF = " << VL << "\n");
  DEBUG(errs() << "Handled loop before vec codegen: \n");
  DEBUG(OrigLoop->dump());

  RHM.mapHLNodes(OrigLoop);

  processLoop();

  DEBUG(errs() << "\n\n\nHandled loop after: \n");
  DEBUG(MainLoop->dump());
  if (!MainLoop->hasChildren()) {
    DEBUG(errs() << "\n\n\nRemoving empty loop\n");
    MainLoop->getHLNodeUtils().remove(MainLoop);
  }
  if (NeedRemainderLoop)
    DEBUG(OrigLoop->dump());

  return true;
}

void AVRCodeGenHIR::eraseLoopIntrinsImpl(bool BeginDir) {
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

    auto LastNode = OrigLoop->getHLNodeUtils().getLastLexicalChild(
        OrigLoop->getParent(), OrigLoop);
    EndIter = std::next(LastNode->getIterator());
  }

  int BeginOrEndDirID = BeginDir ? DIR_OMP_SIMD : DIR_OMP_END_SIMD;
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
        OrigLoop->getHLNodeUtils().remove(HInst);
        continue;
      }

      if (vpo::VPOAnalysisUtils::isIntelDirective(IntrinID)) {
        auto Inst = cast<IntrinsicInst>(HInst->getLLVMInstruction());
        StringRef DirStr = vpo::VPOAnalysisUtils::getDirectiveMetadataString(
            const_cast<IntrinsicInst *>(Inst));

        int DirID = vpo::VPOAnalysisUtils::getDirectiveID(DirStr);

        if (DirID == BeginOrEndDirID) {
          OrigLoop->getHLNodeUtils().remove(HInst);
        } else if (VPOAnalysisUtils::isListEndDirective(DirID)) {
          OrigLoop->getHLNodeUtils().remove(HInst);
          return;
        }
      }
    }
  }

  assert(false && "Missing SIMD Begin/End directive");
}

void AVRCodeGenHIR::eraseLoopIntrins() {
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

void AVRCodeGenHIR::replaceLibCallsInRemainderLoop(HLInst *HInst) {

  // Used to remove the original math calls after iterating over them.
  SmallVector<HLInst*, 1> InstsToRemove;

  const CallInst *Call = cast<CallInst>(HInst->getLLVMInstruction());
  Function *F = Call->getCalledFunction();
  StringRef FnName = F->getName();

  // Check to see if the call was vectorized in the main loop.
  if (TLI->isFunctionVectorizable(FnName, VL)) {
    unsigned ArgIdx = 0;
    if (!F->getReturnType()->isVoidTy()) {
      // In HIR, call argument operands for non-void functions begin at
      // index position 1 in the DDRef operand list.
      ArgIdx = 1;
    }

    SmallVector<RegDDRef *, 1> CallArgs;
    SmallVector<Type*, 1> ArgTys;
    int NumOps = HInst->getNumOperands();

    // For each call argument, insert a scalar load of the element,
    // broadcast it to a vector.
    for (int I = ArgIdx; I < NumOps; I++) {

      // TODO: it is assumed that call arguments need to become vector.
      // In the future, some vectorizable calls may contain scalar
      // arguments. Additional checking is needed for these cases.

      // The DDRef of the original scalar call instruction.
      RegDDRef *Ref = HInst->getOperandDDRef(I);

      // The resulting type of the widened ref/broadcast.
      auto VecDestTy = VectorType::get(Ref->getDestType(), VL);

      RegDDRef *WideRef = nullptr;
      HLInst *LoadInst = nullptr;

      // Create the scalar load of the call argument. This is done so that
      // we can clone the new LvalDDRef and change its type to force the
      // broadcast. See %load in the example above. Essentially, the original
      // scalar %load becomes bitcast.float.<4 x float>, which is how HIRCG
      // knows to do the broadcast.
      if (Ref->isMemRef() && !Ref->isAddressOf()) {
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
      HInst->getHLNodeUtils().insertBefore(HInst, LoadInst);
      WideRef = LoadInst->getLvalDDRef()->clone();
      auto CE = WideRef->getSingleCanonExpr();
      CE->setDestType(VecDestTy);
      const SmallVector<const RegDDRef*, 1> AuxRefs =
        { LoadInst->getLvalDDRef() }; 
      WideRef->makeConsistent(&AuxRefs, OrigLoop->getNestingLevel());

      // Collect call arguments and types so that the function declaration
      // and call instruction can be generated.
      CallArgs.push_back(WideRef);
      ArgTys.push_back(VecDestTy);
    }

    // Using the newly created vector call arguments, generate the vector
    // call instruction and extract the low element.
    Function *VectorF =
      getOrInsertVectorFunction(F, VL, ArgTys, TLI,
                                Intrinsic::not_intrinsic,
                                nullptr/*simd function*/,
                                false/*non-masked*/);
    assert(VectorF && "Can't create vector function.");

    HLInst *WideCall =
      HInst->getHLNodeUtils().createCall(VectorF, CallArgs, VectorF->getName(),
                                       nullptr);
    HInst->getHLNodeUtils().insertBefore(HInst, WideCall);

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
          VecCall->getContext(), I + 1,
          AttributeSet::get(VecCall->getContext(), I + 1, AttrList)));
      }
      //analyzeCallArgMemoryReferences(HInst, WideCall, CallArgs);
    }

    InstsToRemove.push_back(HInst);

    if (!F->getReturnType()->isVoidTy()) {
      HLInst *ExtractInst = HInst->getHLNodeUtils().createExtractElementInst(
        WideCall->getLvalDDRef()->clone(), 0, "elem",
        HInst->getLvalDDRef()->clone());
      HInst->getHLNodeUtils().insertAfter(WideCall, ExtractInst);
    }
  }

  // Remove the original scalar call(s) to clean up the IR.
  for (unsigned Idx = 0; Idx < InstsToRemove.size(); Idx++) {
    HLInst *Inst = InstsToRemove[Idx];
    Inst->getHLNodeUtils().remove(Inst);
  }
}

void AVRCodeGenHIR::HIRLoopVisitor::replaceCalls() {
  for (unsigned i = 0; i < CallInsts.size(); i++) {
    CG->replaceLibCallsInRemainderLoop(CallInsts[i]);
  }
}

void AVRCodeGenHIR::HIRLoopVisitor::visitInst(HLInst *I) {
  // Check for function calls.
  if (I->isCallInst()) {
    CallInsts.push_back(I);
  }
}

void AVRCodeGenHIR::HIRLoopVisitor::visitIf(HLIf *If) {
  for (auto ThenIt = If->then_begin(), ThenEnd = If->then_end();
       ThenIt != ThenEnd; ++ThenIt) {
    visit(*ThenIt);
  }
  for (auto ElseIt = If->else_begin(), ElseEnd = If->else_end();
       ElseIt != ElseEnd; ++ElseIt) {
    visit(*ElseIt);
  }
}

void AVRCodeGenHIR::HIRLoopVisitor::visitLoop(HLLoop *L) {
  for (auto Iter = L->child_begin(), EndItr = L->child_end();
       Iter != EndItr; ++Iter) {
    visit(*Iter);
  }
}

void AVRCodeGenHIR::processLoop() {
  LoopsVectorized++;
  eraseLoopIntrins();

  // Setup main and remainder loops
  bool NeedRemainderLoop = false;
  auto MainLoop = HIRTransformUtils::setupMainAndRemainderLoops(
      OrigLoop, VL, NeedRemainderLoop, true /* VecMode */);

  setNeedRemainderLoop(NeedRemainderLoop);
  setMainLoop(MainLoop);

  for (auto Iter = ALoop->child_begin(), EndItr = ALoop->child_end();
       Iter != EndItr; ++Iter) {
    if (auto AvrAssign = dyn_cast<AVRAssignHIR>(Iter)) {
      RegDDRef *MaskDDRef = nullptr; 
      if (auto APred = AvrAssign->getPredicate()) {
        MaskDDRef = getWideAvrRef(APred->getNumber());
      }
      auto VecInst = widenNode(AvrAssign, MaskDDRef);
      if (MaskDDRef) {
        VecInst->setMaskDDRef(MaskDDRef->clone());
      }
    } else if (auto APredicate = dyn_cast<AVRPredicate>(Iter)) {
      AVRCGVisit CGVisit(this, nullptr /* MaskDDRef */,
                         "Pred" + Twine(APredicate->getNumber()) + "_");
      AVRVisitor<AVRCGVisit> AVisitor(CGVisit);

      AVisitor.visit(APredicate, true /*Recursive*/,
                     true /*RecurseInsideLoops*/, false /*RecurseInsideValues*/,
                     true /*Forward*/);
    } else if (auto AIf = dyn_cast<AVRIfHIR>(Iter)) {
      // We currently expect if/else to be linearized.
      assert(!AIf->hasThenChildren() && !AIf->hasElseChildren() &&
             "Empty if expected");

      RegDDRef *MaskDDRef = nullptr;
      if (auto IfPred = AIf->getPredicate()) {
        MaskDDRef = getWideAvrRef(IfPred->getNumber());
      }
      AVRCGVisit CGVisit(this, MaskDDRef,
                         "Cond" + Twine(AIf->getCondition()->getNumber()) +
                             "_");
      AVRVisitor<AVRCGVisit> AVisitor(CGVisit);

      AVisitor.visit(AIf->getCondition(), true /*Recursive*/,
                     true /*RecurseInsideLoops*/, false /*RecurseInsideValues*/,
                     true /*Forward*/);
    } else {
      assert(false && "Unexpected AVR kind");
    }
  }

  MainLoop->markDoNotVectorize();

  // If a remainder loop is not needed get rid of the OrigLoop at this point.
  if (NeedRemainderLoop) {
    HIRLoopVisitor LV(OrigLoop, this);
    LV.replaceCalls();
    OrigLoop->markDoNotVectorize();
  } else {
    MainLoop->getHLNodeUtils().remove(OrigLoop);
  }
}

bool AVRCodeGenHIR::isReductionRef(const RegDDRef *Ref, unsigned &Opcode) {
  // When widening decomposed nested blobs, we create temp Refs without
  // an associated DDNode.
  if (!Ref->getHLDDNode())
    return false;

  return SRA->isReductionRef(Ref, Opcode);
}

RegDDRef *AVRCodeGenHIR::widenRef(const RegDDRef *Ref) {
  RegDDRef *WideRef;
  int64_t IVConstCoeff;
  auto RefDestTy = Ref->getDestType();
  auto VecRefDestTy = VectorType::get(RefDestTy, VL);
  auto RefSrcTy = Ref->getSrcType();
  auto VecRefSrcTy = VectorType::get(RefSrcTy, VL);

  // If the DDREF has a widened counterpart, return the same after setting
  // SrcType/DestType appropriately.
  if (Ref->isTerminalRef()) {
    unsigned RedOpCode;

    if (WidenMap.find(Ref->getSymbase()) != WidenMap.end()) {
      auto WInst = WidenMap[Ref->getSymbase()];
      // TODO - look into reusing instead of cloning (Pankaj's suggestion)
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
      auto RedOpVecInst = insertReductionInitializer(Identity);

      // Add to WidenMap and handle generating code for building reduction tail
      addToMapAndHandleLiveOut(Ref, RedOpVecInst);

      // LVAL ref of the initialization instruction is the widened reduction
      // ref.
      return RedOpVecInst->getLvalDDRef()->clone();
    }
  }

  // Lval terminal refs get the widened ref duing the widened HLInst creation
  // later - simply return NULL.
  if (Ref->getHLDDNode() && Ref->isLval() && Ref->isTerminalRef())
    return nullptr;

  // TODO - look into reusing instead of cloning (Pankaj's suggestion)
  WideRef = Ref->clone();

  // Set VectorType on WideRef base pointer - BaseDestType is set to pointer
  // type of VL-wide vector of Ref's DestType. For addressof DDRef, desttype
  // is set to vector of pointers(scalar desttype).
  if (WideRef->hasGEPInfo()) {
    PointerType *PtrType = cast<PointerType>(Ref->getBaseDestType());
    auto AddressSpace = PtrType->getAddressSpace();

    // Omit the range metadata as is done in loop vectorize which does
    // not propagate the same. We get a compile time error otherwise about
    // type mismatch for range values.
    WideRef->setMetadata(LLVMContext::MD_range, nullptr);

    if (WideRef->isAddressOf())
      WideRef->setBaseDestType(VecRefDestTy);
    else
      WideRef->setBaseDestType(PointerType::get(VecRefDestTy, AddressSpace));
  }

  // For unit stride ref, nothing else to do
  if (isConstStrideRef(Ref, OrigLoop->getNestingLevel(), &IVConstCoeff) &&
      IVConstCoeff == 1)
    return WideRef;

  // For cases other than unit stride refs, we need to widen the induction
  // variable and replace blobs in Canon Expr with widened equivalents.
  for (auto I = WideRef->canon_begin(), E = WideRef->canon_end(); I != E; ++I) {
    auto CE = *I;
    bool AnyChange = true;

    if (CE->hasIV(OrigLoop->getNestingLevel())) {
      SmallVector<Constant *, 4> CA;
      Type *Int64Ty = CE->getSrcType();

      CE->getIVCoeff(OrigLoop->getNestingLevel(), nullptr, &IVConstCoeff);

      for (unsigned i = 0; i < VL; ++i) {
        CA.push_back(ConstantInt::getSigned(Int64Ty, IVConstCoeff * i));
      }
      ArrayRef<Constant *> AR(CA);
      auto CV = ConstantVector::get(AR);

      unsigned Idx = 0;
      CE->getBlobUtils().createBlob(CV, true, &Idx);
      CE->addBlob(Idx, 1);
      AnyChange = true;
    }

    SmallVector<unsigned, 8> BlobIndices;
    CE->collectBlobIndices(BlobIndices, false);

    for (auto &BI : BlobIndices) {
      auto TopBlob = CE->getBlobUtils().getBlob(BI);

      // We do not need to widen invariant blobs - check for blob invariance
      // by comparing maxbloblevel against the loop's nesting level.
      if (WideRef->findMaxBlobLevel(BI) < OrigLoop->getNestingLevel())
        continue;

      if (CE->getBlobUtils().isNestedBlob(TopBlob)) {

        AVR *BlobTree =
            decomposeBlob(WideRef, BI, 1, Fn.getParent()->getDataLayout());
        AVRCGVisit CGVisit(this, nullptr /* MaskDDRef */,
                           "Blob" + Twine(BI) + "_");
        AVRVisitor<AVRCGVisit> AVisitor(CGVisit);

        AVisitor.visit(BlobTree, true /*Recursive*/,
                       true /*RecurseInsideLoops*/,
                       false /*RecurseInsideValues*/, true /*Forward*/);

        auto TRef = getWideAvrRef(BlobTree->getNumber());

        CE->replaceBlob(BI, TRef->getSingleCanonExpr()->getSingleBlobIndex());
        continue;
      }

      assert(CE->getBlobUtils().isTempBlob(TopBlob) &&
             "Only temp blobs expected");

      auto OldSymbase = CE->getBlobUtils().getTempBlobSymbase(BI);

      if (WidenMap.find(OldSymbase) != WidenMap.end()) {
        auto WInst1 = WidenMap[OldSymbase];
        auto WRef = WInst1->getLvalDDRef()->clone();
        CE->replaceBlob(BI, WRef->getSingleCanonExpr()->getSingleBlobIndex());
        AnyChange = true;
      }
    }

    if (AnyChange) {
      auto VecCEDestTy = VectorType::get(CE->getDestType(), VL);
      auto VecCESrcTy = VectorType::get(CE->getSrcType(), VL);

      CE->setDestType(VecCEDestTy);
      CE->setSrcType(VecCESrcTy);
    }
  }

  // The blobs in the scalar ref have been replaced by widened refs, call
  // the utility to update the blob DDRefs in the widened Ref.
  SmallVector<BlobDDRef *, 8> NewBlobs;
  WideRef->updateBlobDDRefs(NewBlobs);

  return WideRef;
}

RegDDRef *AVRCodeGenHIR::getVectorValue(const RegDDRef *Op) {

  if (WidenMap.count(Op->getSymbase()))
    return WidenMap[Op->getSymbase()]->getLvalDDRef()->clone();

  if (Op->isIntConstant() || Op->isFPConstant())
    return getConstantSplatDDRef(Op, VL);

  // TODO: Create spat vector for loop invariant values
  assert(true && "Can't get vector value");
  return nullptr;
}

/// \brief Return result of combining horizontal vector binary operation with
/// initial value. Horizontal binary operation splits VecRef recursively
/// into 2 parts until the VL becomes 2. Then we extract elements from the
/// vector and perform scalar operation, the result of which is then
/// combined with the initial value and assigned to ResultRef. The created
/// instructions are added to the InstContainer initially and are added
/// after Loop at the end after generating the combined result.
static HLInst *buildReductionTail(HLContainerTy &InstContainer,
                                  unsigned BOpcode, const RegDDRef *VecRef,
                                  const RegDDRef *InitValRef, HLLoop *Loop,
                                  const RegDDRef *ResultRef) {

  // Take Vector Length from the WideRedInst type
  Type *VecTy = VecRef->getDestType();

  // For Sub/FSub operation, we need to use Add/FAdd for the horizontal
  // vector and combine operations.
  if (BOpcode == Instruction::Sub)
    BOpcode = Instruction::Add;
  else if (BOpcode == Instruction::FSub)
    BOpcode = Instruction::FAdd;

  unsigned VL = cast<VectorType>(VecTy)->getNumElements();
  if (VL == 2) {
    HLInst *Lo = Loop->getHLNodeUtils().createExtractElementInst(
        VecRef->clone(), 0, "Lo");
    HLInst *Hi = Loop->getHLNodeUtils().createExtractElementInst(
        VecRef->clone(), 1, "Hi");

    HLInst *Combine = Loop->getHLNodeUtils().createBinaryHLInst(
        BOpcode, Lo->getLvalDDRef()->clone(), Hi->getLvalDDRef()->clone(),
        "reduced");
    InstContainer.push_back(*Lo);
    InstContainer.push_back(*Hi);
    InstContainer.push_back(*Combine);

    RegDDRef *ScalarValue = Combine->getLvalDDRef();

    // Combine with initial value
    auto FinalInst = Loop->getHLNodeUtils().createBinaryHLInst(
        BOpcode, ScalarValue->clone(), InitValRef->clone(), "final" /* Name */,
        ResultRef->clone());
    InstContainer.push_back(*FinalInst);
    return FinalInst;
  }
  SmallVector<uint32_t, 16> LoMask, HiMask;
  for (unsigned i = 0; i < VL / 2; ++i)
    LoMask.push_back(i);
  for (unsigned i = VL / 2; i < VL; ++i)
    HiMask.push_back(i);
  HLInst *Lo = Loop->getHLNodeUtils().createShuffleVectorInst(
      VecRef->clone(), VecRef->clone(), LoMask, "Lo");
  HLInst *Hi = Loop->getHLNodeUtils().createShuffleVectorInst(
      VecRef->clone(), VecRef->clone(), HiMask, "Hi");
  HLInst *Result = Loop->getHLNodeUtils().createBinaryHLInst(
      BOpcode, Lo->getLvalDDRef()->clone(), Hi->getLvalDDRef()->clone(),
      "reduce");
  InstContainer.push_back(*Lo);
  InstContainer.push_back(*Hi);
  InstContainer.push_back(*Result);
  return buildReductionTail(InstContainer, BOpcode, Result->getLvalDDRef(),
                            InitValRef, Loop, ResultRef);
}

// Find RegDDref of address, where the reduction variable is stored.
// This functionality we need while DDG is not fully implemented.
// TODO- Concerns from Pankaj related to this function - these need to be
// addressed in the full reduction implementation.
// 1) The traversal should be backwards starting from the loop, not forwards
//    starting from the parent.
// 2) Not sure if preheader/postexit of the loop has been extracted already.
//    If not, we are missing looking in the preheader first. (Preheader
//    extraction has not happened at this point.)
// 3) The comparison for LLVM instructions below is making me uneasy. I don't
//    think this is the right way to check things in HIR.
void ReductionHIRMngr::mapHLNodes(const HLLoop *OrigLoop) {
  const HLNode *Parent = OrigLoop->getParent();
  auto FChild = OrigLoop->getHLNodeUtils().getFirstLexicalChild(Parent);

  for (auto RedItr : ReductionMap) {
    ReductionItem *RI = RedItr.second;
    const Value *Initializer = RI->getInitializer();
    bool Success = false;
    for (auto Itr = FChild->getIterator(), End = OrigLoop->getIterator();
         Itr != End; ++Itr)
      if (auto HInst = dyn_cast<HLInst>(Itr))
        if (HInst->getLLVMInstruction() == Initializer) {
          Initializers[RI] = HInst->getRvalDDRef();
          Success = true;
          break;
        }
    assert(Success && "Can't find HIR initializer for reduction item");
  }
}

const RegDDRef *ReductionHIRMngr::getReductionValuePtr(ReductionItem *RI) {
  assert(Initializers.count(RI) && "Uncompleted Initializers map");
  return Initializers[RI];
}

HLInst *AVRCodeGenHIR::widenReductionNode(const HLNode *Node) {
  const HLInst *INode = cast<HLInst>(Node);
  const Instruction *CurInst = INode->getLLVMInstruction();

  // We handle only Binary Operators here
  auto BOp = cast<BinaryOperator>(CurInst);
  const RegDDRef *Op1 = INode->getOperandDDRef(1);
  const RegDDRef *Op2 = INode->getOperandDDRef(2);
  const RegDDRef *LVal = INode->getLvalDDRef();
  const RegDDRef *RedOp;
  const RegDDRef *FreeOp;
  // Find reduction operand. We assume that the binary operation has 2 operands
  // The reduction Op and LVal of the instruction should have the same name.
  if (LVal->getSymbase() == Op1->getSymbase()) {
    RedOp = Op1;
    FreeOp = Op2;
  } else {
    assert(LVal->getSymbase() == Op2->getSymbase() &&
           "Unexpected reduction operand");
    RedOp = Op2;
    FreeOp = Op1;
  }
  // The free (non-reduction) operand should be widened in a regular way
  RegDDRef *FreeOpVec = widenRef(FreeOp);

  // Build Identity vector. It depends of recurrence kind and the type of the
  // operand.
  ReductionItem *RI = RHM.getReductionInfo(CurInst);

  Constant *Identity =
      ReductionHIRMngr::getRecurrenceIdentity(RI, RedOp->getDestType());
  auto RedOpVecInst = insertReductionInitializer(Identity);

  // Create a wide reduction instruction
  HLInst *WideInst = Node->getHLNodeUtils().createBinaryHLInst(
      BOp->getOpcode(), RedOpVecInst->getLvalDDRef()->clone(), FreeOpVec,
      "red" /* Name */, RedOpVecInst->getLvalDDRef()->clone(), BOp);

  // Build the tail - horizontal operation that converts vector to scalar
  HLContainerTy Tail;
  const RegDDRef *Address = RHM.getReductionValuePtr(RI);
  HLInst *LoadInitValInst =
      Node->getHLNodeUtils().createLoad(Address->clone(), "init");
  Tail.push_back(*LoadInitValInst);

  RegDDRef *InitValue = LoadInitValInst->getLvalDDRef();
  RegDDRef *VecRef = WideInst->getLvalDDRef();

  buildReductionTail(Tail, BOp->getOpcode(), VecRef, InitValue, MainLoop,
                     RedOp);
  Node->getHLNodeUtils().insertAfter(MainLoop, &Tail);
  return WideInst;
}

void AVRCodeGenHIR::analyzeCallArgMemoryReferences(
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
    if (Args[I]->hasGEPInfo() && Args[I]->isAddressOf()) {
      AttrBuilder AttrList;
      int64_t ByteStride;
      CanonExpr *CE = Args[I]->getStrideAtLevel(LoopLevel);

      if (CE->isLinearAtLevel() && CE->isIntConstant(&ByteStride)) {
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
            VecCall->getContext(), I + 1,
            AttributeSet::get(VecCall->getContext(), I + 1, AttrList)));
      }
    }
  }
}

HLInst *AVRCodeGenHIR::widenNode(AVRAssignHIR *AvrNode, RegDDRef *Mask) {
  const HLNode *Node = AvrNode->getHIRInstruction();
  const HLInst *INode;
  INode = dyn_cast<HLInst>(Node);
  auto CurInst = INode->getLLVMInstruction();
  SmallVector<RegDDRef *, 6> WideOps;

  HLInst *WideInst = nullptr;

  if (isa<BinaryOperator>(CurInst)) {
    if (RHM.isReductionVariable(CurInst)) {
      WideInst = widenReductionNode(Node);
      Node->getHLNodeUtils().insertAsLastChild(MainLoop, WideInst);
      return WideInst;
    }
  }

  DEBUG(errs() << "DDRef ");
  DEBUG(INode->dump());
  bool InsertInMap = true;
  for (auto Iter = INode->op_ddref_begin(), End = INode->op_ddref_end();
       Iter != End; ++Iter) {
    RegDDRef *WideRef, *Ref;

    Ref = *Iter;

    WideRef = widenRef(Ref);
    WideOps.push_back(WideRef);
  }

  DEBUG(Node->dump(true));

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
        VectorType::get(CurInst->getType(), VL), CurInst->getOpcode(),
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<SelectInst>(CurInst)) {
    WideInst = Node->getHLNodeUtils().createSelect(
        INode->getPredicate(), WideOps[1], WideOps[2], WideOps[3], WideOps[4],
        CurInst->getName() + ".vec", WideOps[0], INode->getPredicateFMF());
  } else if (isa<CmpInst>(CurInst)) {
    WideInst = Node->getHLNodeUtils().createCmp(
        INode->getPredicate(), WideOps[1], WideOps[2],
        CurInst->getName() + ".vec", WideOps[0], INode->getPredicateFMF());
  } else if (isa<GetElementPtrInst>(CurInst)) {
    // Gep Instructions in LLVM may have any number of operands but the HIR
    // representation for them is always a single rhs ddref - copy rval to
    // lval.
    WideInst = Node->getHLNodeUtils().createCopyInst(
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
  } else if (const CallInst *Call = dyn_cast<CallInst>(CurInst)) {

    Function *Fn = Call->getCalledFunction();
    StringRef FnName = Fn->getName();

    // Default to svml. If svml is not available, try the intrinsic.
    Intrinsic::ID ID = Intrinsic::not_intrinsic;
    if (!TLI->isFunctionVectorizable(FnName, VL)) {
      ID = getVectorIntrinsicIDForCall(Call, TLI);
      if (ID && (ID == Intrinsic::assume || ID == Intrinsic::lifetime_end ||
                 ID == Intrinsic::lifetime_start)) {
        return const_cast<HLInst*>(INode);
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

    Function *VectorF = getOrInsertVectorFunction(Fn, VL, ArgTys, TLI, ID,
                                                  nullptr, Masked);
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

  // Add to WidenMap and handle generating code for any liveouts
  if (InsertInMap) {
    addToMapAndHandleLiveOut(INode->getLvalDDRef(), WideInst);
    if (WideInst->getLvalDDRef()->isTerminalRef())
      WideInst->getLvalDDRef()->makeSelfBlob();
  }

  Node->getHLNodeUtils().insertAsLastChild(MainLoop, WideInst);
  return WideInst;
}

HLInst *AVRCodeGenHIR::insertReductionInitializer(Constant *Iden) {
  auto IdentityVec = getConstantSplatDDRef(MainLoop->getDDRefUtils(), Iden, VL);
  HLInst *RedOpVecInst =
      MainLoop->getHLNodeUtils().createCopyInst(IdentityVec, "RedOp");
  MainLoop->getHLNodeUtils().insertBefore(MainLoop, RedOpVecInst);

  auto LvalSymbase = RedOpVecInst->getLvalDDRef()->getSymbase();
  MainLoop->addLiveInTemp(LvalSymbase);
  return RedOpVecInst;
}

void AVRCodeGenHIR::addToMapAndHandleLiveOut(const RegDDRef *ScalRef,
                                             HLInst *WideInst) {
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

  MainLoop->addLiveOutTemp(VecRef->getSymbase());

  unsigned OpCode;

  if (isReductionRef(ScalRef, OpCode)) {
    HLContainerTy Tail;

    buildReductionTail(Tail, OpCode, VecRef, ScalRef, MainLoop, ScalRef);
    WideInst->getHLNodeUtils().insertAfter(MainLoop, &Tail);
  } else {
    auto Extr = WideInst->getHLNodeUtils().createExtractElementInst(
        VecRef->clone(), VL - 1, "Last", ScalRef->clone());
    auto Lval = Extr->getLvalDDRef();

    // Convert to selfblob if Lval has IV at Loop level since last value
    // extract instruction is added after the Loop.
    if (Lval->getSingleCanonExpr()->hasIV(MainLoop->getNestingLevel()))
      Lval->makeSelfBlob();

    WideInst->getHLNodeUtils().insertAfter(MainLoop, Extr);
  }
}

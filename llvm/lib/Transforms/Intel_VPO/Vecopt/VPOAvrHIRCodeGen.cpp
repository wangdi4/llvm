//===-- VPOAvrHIRCodeGen.cpp ----------------------------------------------===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRLoopTransformUtils.h"

#define DEBUG_TYPE "VPODriver"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

static cl::opt<bool>
  DisableStressTest("disable-vpo-stress-test", cl::init(true),
                    cl::Hidden,
                    cl::desc("Disable VPO Vectorizer Stress Testing"));

static RegDDRef *getConstantSplatDDRef(Constant *ConstVal, unsigned VL) {
  Constant *ConstVec = ConstantVector::getSplat(VL, ConstVal);
  if (isa<ConstantDataVector>(ConstVec))
    return DDRefUtils::createConstDDRef(cast<ConstantDataVector>(ConstVec));
  if (isa<ConstantAggregateZero>(ConstVec))
    return DDRefUtils::createConstDDRef(cast<ConstantAggregateZero>(ConstVec));
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
  return getConstantSplatDDRef(ConstVal, VL);
}

ReductionHIRMngr::ReductionHIRMngr(AVR *Avr) {
  ReductionClause *RC = cast<AVRWrn>(Avr)->getWrnNode()->getRed();
  if (!RC)
    return;
  for (ReductionItem *Ri : RC->items()) {

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

RegDDRef *
ReductionHIRMngr::getRecurrenceIdentityVector(ReductionItem *RedItem,
                                              Type *Ty, unsigned VL) {

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
    RDKind = Ty->isFloatTy() ? RecurrenceDescriptor::RK_FloatAdd :
      RecurrenceDescriptor::RK_IntegerAdd;
    break;
  case ReductionItem::WRNReductionMult:
    RDKind = Ty->isFloatTy() ? RecurrenceDescriptor::RK_FloatMult :
      RecurrenceDescriptor::RK_IntegerMult;
    break;
  default:
    llvm_unreachable("Unknown recurrence kind");
  }
  Constant *Iden = RecurrenceDescriptor::getRecurrenceIdentity(RDKind, Ty);
  return getConstantSplatDDRef(Iden, VL);
}

// TBD - once we update to the latest loopopt sources, make use of
// getStrideAtLevel utility
bool AVRCodeGenHIR::isConstStrideRef(const RegDDRef *Ref,
                                     unsigned NestingLevel,
                                     int64_t *CoeffPtr) {
  if (Ref->isTerminalRef())
    return false;

  const CanonExpr *FirstCE = nullptr;

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
  if (FirstCE->isSExt() || FirstCE->isZExt())
    return false;

  if (FirstCE->isNonLinear() ||
      FirstCE->getDefinedAtLevel() >= NestingLevel)
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
  unsigned LoopLevel;
  bool UnitStrideRefSeen;
  bool MemRefSeen;

  void visitRegDDRef(RegDDRef *RegDD);
  void visitCanonExpr(CanonExpr *CExpr);

public:
  HandledCheck(unsigned Level) : IsHandled(true), LoopLevel(Level),
    UnitStrideRefSeen(false), MemRefSeen(false) {}

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

  if (!isa<HLInst>(Node)) {
    DEBUG(errs() <<
          "VPO_OPTREPORT: Loop not handled - only HLInst supported\n");
    IsHandled = false;
    return;
  }

  auto Inst = cast<HLInst>(Node);

  // Calls are not supported for now
  if (Inst->isCallInst()) {
    DEBUG(errs() << "VPO_OPTREPORT: Loop not handled - calls not supported\n");
    IsHandled = false;
    return;
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

    // Addressof computation not supported for now.
    if (RegDD->isAddressOf()) {
      DEBUG(errs() <<
            "VPO_OPTREPORT: Loop not handled - addressof computation\n");
      IsHandled = false;
      return;
    }

    auto BaseCE = RegDD->getBaseCE();

    if (!BaseCE->isInvariantAtLevel(LoopLevel)) {
      DEBUG(errs() <<
            "VPO_OPTREPORT: Loop not handled - BaseCE not invariant\n");
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
    DEBUG(errs() <<
          "VPO_OPTREPORT: Loop not handled - IV with blob coefficient\n");
    IsHandled = false;
    return;
  }
  

  // TODO: Handle the case when we have a denominator
  if (CExpr->getDenominator() != 1) {
    DEBUG(errs() <<
          "VPO_OPTREPORT: Loop not handled - IV with denominator\n");
    IsHandled = false;
    return;
  }
  
  SmallVector<unsigned, 8> BlobIndices;
  CExpr->collectBlobIndices(BlobIndices, false);

  // Workaround for now until we have a way to handle nested blobs
  DEBUG(errs() << "Top blobs: \n");
  for (auto &BI : BlobIndices) {
    auto TopBlob = BlobUtils::getBlob(BI);

    DEBUG(TopBlob->dump());

    if (BlobUtils::isNestedBlob(TopBlob)) {
      DEBUG(errs() << "Nested blob: ");
      DEBUG(TopBlob->dump());

      DEBUG(errs() <<
            "VPO_OPTREPORT: Loop not handled - nested blob\n");
      IsHandled = false;
      return;
    }
  }
}

// TODO: Take as input a VPOVecContext that indicates which AVRLoop(s)
// is (are) to be vectorized, as identified by the vectorization scenario
// evaluation.
// FORNOW there is only one AVRLoop per region, so we will re-discover
// the same AVRLoop that the vecScenarioEvaluation had "selected".
bool AVRCodeGenHIR::loopIsHandled(unsigned int VF) {
  AVRWrn *AWrn = nullptr;
  AVRLoop *ALoop = nullptr;
  WRNVecLoopNode *WVecNode;
  HLLoop *Loop = nullptr;

  // We expect avr to be a AVRWrn node
  if (!(AWrn = dyn_cast<AVRWrn>(Avr))) {
    DEBUG(errs() << "VPO_OPTREPORT: Loop not handled - expected AVRWrn node\n");
    return false;
  }

  WVecNode = AWrn->getWrnNode();

  // An AVRWrn node is expected to have only one AVRLoop child
  // FIXME?: This expectation was already checked by the VecScenarioEvaluation.
  for (auto Itr = AWrn->child_begin(), End = AWrn->child_end(); Itr != End;
       ++Itr) {
    if (AVRLoop *TempALoop = dyn_cast<AVRLoop>(Itr)) {
      if (ALoop) {
        DEBUG(errs() << 
              "VPO_OPTREPORT: Loop not handled - expected one AVRLoop child\n");
        return false;
      }

      ALoop = TempALoop;
    }
  }

  // Check that we have an AVRLoop
  if (!ALoop) {
    DEBUG(errs() << 
          "VPO_OPTREPORT: Loop not handled - AVRLoop child not found\n");
    return false;
  }

  Loop = WVecNode->getHLLoop();
  assert(Loop && "Null HLLoop.");
  setOrigLoop(Loop);

  // Only handle normalized loops
  if (!OrigLoop->isNormalized()) {
    DEBUG(errs() << 
          "VPO_OPTREPORT: Loop not handled - loop not in normalized form\n");
    return false;
  }

  // We are working with normalized loops, trip count is loop UpperBound + 1.
  auto UBRef = Loop->getUpperDDRef();
  int64_t UBConst;

  if (UBRef->isIntConstant(&UBConst)) {
    auto ConstTripCount = UBConst + 1;

    // Check that main vector loop will have atleast one iteration
    if (ConstTripCount < VL) {
      DEBUG(errs() << 
            "VPO_OPTREPORT: Loop not handled - zero iteration main loop\n");
      return false;
    }

    // Set constant trip count
    setTripCount((uint64_t) ConstTripCount);
  }

  bool UnitStrideSeen = false;
  bool MemRefSeen = false;
  for (auto Itr = ALoop->child_begin(), End = ALoop->child_end(); Itr != End;
       ++Itr) {
    if (!isa<AVRAssignHIR>(Itr)) {
      DEBUG(errs() << 
            "VPO_OPTREPORT: Loop not handled - only AVRAssign is supported\n");
      return false;
    }

    HandledCheck NodeCheck(OrigLoop->getNestingLevel());
    HLDDNode *INode = cast<AVRAssignHIR>(Itr)->getHIRInstruction();

    HLNodeUtils::visit(NodeCheck, INode);
    if (!NodeCheck.isHandled())
      return false;

    if (NodeCheck.getUnitStrideRefSeen())
      UnitStrideSeen = true;

    if (NodeCheck.getMemRefSeen())
      MemRefSeen = true;
  }

  // If we are not in stress testing mode, only vectorize when some
  // unit stride refs are seen. Still vectorize the case when no mem refs
  // are seen. Remove this check once vectorizer cost model is fully
  // implemented.
  if (DisableStressTest && MemRefSeen && !UnitStrideSeen) {
    DEBUG(errs() << 
          "VPO_OPTREPORT: Loop not handled - all mem refs non unit-stride\n");
    return false;
  }

  // TODO - Explicit reduction implementation needs to be extended for
  // cases where the parent is not a region - while I look into how to
  // do this, I am retaining the restriction for the parent to be a
  // region for reduction support for now.
  HLRegion *Parent = dyn_cast<HLRegion>(Loop->getParent());

  // TODO - HIR added support for recognizing reductions and we now
  // mark loops with self reductions as vectorizable. However, we do
  // not handle these in code generation. The check below will mark
  // these cases as not handled for now.
  if (Parent) {
    // Live out for reduction only
    for (auto LiveOut : Parent->live_out())
      if (!RHM.isReductionVariable(LiveOut.second)) {
        DEBUG(errs() << 
              "VPO_OPTREPORT: Loop not handled - liveouts not supported\n");
        return false;
      }
  }
  else {
    // No loop live out support for now.
    if (Loop->hasLiveOutTemps()) {
      DEBUG(errs() << 
            "VPO_OPTREPORT: Loop not handled - liveouts not supported\n");
      return false;
    }
  }

  setALoop(ALoop);

  DEBUG(errs() << "Handled loop\n");
  return true;
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

  DEBUG(errs() << "Handled loop before vec codegen: \n");
  DEBUG(OrigLoop->dump(true));

  RHM.mapHLNodes(OrigLoop);

  processLoop();

  DEBUG(errs() << "\n\n\nHandled loop after: \n");
  DEBUG(MainLoop->dump(true));
  if (NeedRemainderLoop)
    DEBUG(OrigLoop->dump(true));

  return true;
}

int AVRCodeGenHIR::getRemainderLoopCost(HLLoop *Loop, unsigned int VF, 
                                        unsigned int &ConstTripCount) {
  ConstTripCount = TripCount;
  // Check for positive trip count and that trip count is a multiple of vector
  // length. Otherwise a remainder loop is needed. Since CG currently does not
  // support remainder loops, return a dummy high cost to make sure this VF will
  // not be selected as vectorization factor.
  if (TripCount == 0 || TripCount % VF) {
    return 1000;
  }

  return 0;
}

void AVRCodeGenHIR::eraseIntrinsBeforeLoop() {
  // Erase intrinsics before the Loop - the code below mimics the code
  // to check for a SIMD loop (HLLoop::isSIMD).
  auto FirstChild = HLNodeUtils::getFirstLexicalChild(OrigLoop->getParent(),
                                                      OrigLoop);
  HLContainerTy::iterator FIter(*FirstChild);
  HLContainerTy::iterator Iter(OrigLoop);

  bool FirstDirItSet = false;
  HLContainerTy::iterator FirstDirIt;
  HLContainerTy::iterator LoopIt(OrigLoop);

  while (Iter != FIter) {
    --Iter;

    auto Inst = dyn_cast<HLInst>(Iter);
    if (!Inst)
      break; // Loop, IF, Switch, etc.

    Intrinsic::ID IntrinID;
    // Expecting just directives and clauses between SIMD directive and Loop.
    if (!Inst->isIntrinCall(IntrinID) ||
        !vpo::VPOUtils::isIntelDirectiveOrClause(IntrinID))
      break; 
    
    FirstDirItSet = true;
    FirstDirIt = Iter;
  }

  // In cases where we have other HLInsts between the SIMD related directives
  // and HLLoop, we will hit the following assert. As a workaround for now,
  // do not assert. These directives will be deleted by the intrinsic cleanup
  // pass that runs later.
  // TODO: Modify this function to look for the first/last SIMD directive
  // before a HLLoop ignoring other HLInsts before the loop before we hit the
  // first SIMD related directive.
  // assert(FirstDirItSet && "Expected SIMD directive not found");

  if (FirstDirItSet)
    // Remove intrinsics and clauses before the loop
    HLNodeUtils::remove(FirstDirIt, LoopIt);
}

void AVRCodeGenHIR::processLoop() {
  eraseIntrinsBeforeLoop();

  // Setup main and remainder loops
  bool NeedRemainderLoop = false;
  auto MainLoop = 
    HIRLoopTransformUtils::setupMainAndRemainderLoops(OrigLoop,
                                                      VL,
                                                      NeedRemainderLoop,
                                                      true /* VecMode */);

  setNeedRemainderLoop(NeedRemainderLoop);
  setMainLoop(MainLoop);

  for (auto Iter = ALoop->child_begin(), EndItr = ALoop->child_end();
       Iter != EndItr; ++Iter) {
    AVRAssignHIR *AvrAssign;

    AvrAssign = cast<AVRAssignHIR>(Iter);
    widenNode(AvrAssign->getHIRInstruction());
  }

  MainLoop->markDoNotVectorize();

  // If a remainder loop is not needed get rid of the OrigLoop at this point.
  if (NeedRemainderLoop) {
    OrigLoop->markDoNotVectorize();
  } else {
    HLNodeUtils::remove(OrigLoop);
  }
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
  if (Ref->isSelfBlob()) {
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
  }

  // TODO - look into reusing instead of cloning (Pankaj's suggestion)
  WideRef = Ref->clone();

  // Set VectorType on WideRef base pointer - BaseDestType is set to pointer
  // type of VL-wide vector of Ref's DestType.
  if (WideRef->hasGEPInfo()) {
    PointerType *PtrType = cast<PointerType>(Ref->getBaseDestType());
    auto AddressSpace = PtrType->getAddressSpace();

    // Omit the range metadata as is done in loop vectorize which does
    // not propagate the same. We get a compile time error otherwise about
    // type mismatch for range values.
    WideRef->setMetadata(LLVMContext::MD_range, nullptr);

    WideRef->setBaseDestType(PointerType::get(VecRefDestTy, AddressSpace));
  }

  // For unit stride ref, nothing else to do
  if (isConstStrideRef(Ref, OrigLoop->getNestingLevel(), &IVConstCoeff) &&
      IVConstCoeff == 1)
    return WideRef;

  // For cases other than unit stride refs, we need to widen the induction
  // variable and replace blobs in Canon Expr with widened equivalents.
  for (auto B = WideRef->blob_cbegin(), BE = WideRef->blob_cend(); B != BE;
       ++B) {
    if (WidenMap.find((*B)->getSymbase()) != WidenMap.end()) {
      auto WInst1 = WidenMap[(*B)->getSymbase()];
      auto WideBlobRef = WInst1->getLvalDDRef()->clone();

      (*B)->replaceBlob(
          WideBlobRef->getSingleCanonExpr()->getSingleBlobIndex());
    }
  }

  for (auto I = WideRef->canon_begin(), E = WideRef->canon_end(); I != E; ++I) {
    auto CE = *I;
    bool AnyChange = true;

    if (CE->hasIV(OrigLoop->getNestingLevel())) {
      SmallVector<Constant *, 4> CA;
      Type *Int64Ty = CE->getSrcType();

      CE->getIVCoeff(OrigLoop->getNestingLevel(), nullptr, &IVConstCoeff);

      for (int i = 0; i < VL; ++i) {
        CA.push_back(ConstantInt::getSigned(Int64Ty, IVConstCoeff * i));
      }
      ArrayRef<Constant *> AR(CA);
      auto CV = ConstantVector::get(AR);

      unsigned Idx = 0;
      BlobUtils::createBlob(CV, true, &Idx);
      CE->addBlob(Idx, 1);
      AnyChange = true;
    }

    SmallVector<unsigned, 8> BlobIndices;
    CE->collectTempBlobIndices(BlobIndices, false);

    for (auto &BI : BlobIndices) {
      auto OldSymbase = BlobUtils::getTempBlobSymbase(BI);
      
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

/// \brief Return scalar result of horizontal vector binary operation.
/// Horizontal binary operation splits the vector recursively
/// into 2 parts until the VL becomes 2. Then we extract elements from the
/// vector and perform scalar operation.
static HLInst * buildReductionTail(HLContainerTy& InstContainer,
                                   unsigned BOpcode, HLInst *Inst) {

  // Take Vector Length from the WideRedInst type
  Type *InstTy = Inst->getLvalDDRef()->getDestType();

  unsigned VL = cast<VectorType>(InstTy)->getNumElements();
  if (VL == 2) {
    HLInst *Lo =
      HLNodeUtils::CreateExtractElementInst(Inst->getLvalDDRef()->clone(),
                                           0, "Lo");
    HLInst *Hi =
      HLNodeUtils::CreateExtractElementInst(Inst->getLvalDDRef()->clone(),
                                           1, "Hi");

    HLInst *Combine = 
      HLNodeUtils::createBinaryHLInst(BOpcode, Lo->getLvalDDRef()->clone(),
                                      Hi->getLvalDDRef()->clone(), "reduced");
    InstContainer.push_back(Lo);
    InstContainer.push_back(Hi);
    InstContainer.push_back(Combine);
    return Combine;
  }
  SmallVector<uint32_t, 16> LoMask, HiMask;
  for (unsigned i = 0; i < VL/2; ++i)
    LoMask.push_back(i);
  for (unsigned i = VL/2; i < VL; ++i)
    HiMask.push_back(i);
  HLInst *Lo =
    HLNodeUtils::CreateShuffleVectorInst(Inst->getLvalDDRef()->clone(),
                                         Inst->getLvalDDRef()->clone(),
                                         LoMask, "Lo");
  HLInst *Hi =
    HLNodeUtils::CreateShuffleVectorInst(Inst->getLvalDDRef()->clone(),
                                         Inst->getLvalDDRef()->clone(),
                                         HiMask, "Hi");
  HLInst *Result =
    HLNodeUtils::createBinaryHLInst(BOpcode, Lo->getLvalDDRef()->clone(),
                                    Hi->getLvalDDRef()->clone(), "reduce");
  InstContainer.push_back(Lo);
  InstContainer.push_back(Hi);
  InstContainer.push_back(Result);
  return buildReductionTail(InstContainer, BOpcode, Result);
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
  auto FChild = HLNodeUtils::getFirstLexicalChild(Parent);

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

  RegDDRef *IdentityVec =
    ReductionHIRMngr::getRecurrenceIdentityVector(RI, RedOp->getDestType(), VL);

  HLInst *RedOpVecInst = HLNodeUtils::createCopyInst(IdentityVec, "RedOp");

  HLNodeUtils::insertBefore(MainLoop, RedOpVecInst);

  // Create a wide reduction instruction
  HLInst *WideInst =
    HLNodeUtils::createBinaryHLInst(BOp->getOpcode(),
                                    RedOpVecInst->getLvalDDRef()->clone(),
                                    FreeOpVec,
                                    ""/* Name */,
                                    RedOpVecInst->getLvalDDRef()->clone(), BOp);

  // Build the tail - horizontal operation that converts vector to scalar
  HLContainerTy Tail;
  HLInst *LastScalarInst = buildReductionTail(Tail, BOp->getOpcode(), WideInst);
  RegDDRef *ScalarValue = LastScalarInst->getLvalDDRef()->clone();

  // Combine with initial value
  const RegDDRef *Address = RHM.getReductionValuePtr(RI);

  HLInst *LoadInitValInst = HLNodeUtils::createLoad(Address->clone());
  Tail.push_back(LoadInitValInst);
  RegDDRef *InitValue = LoadInitValInst->getLvalDDRef()->clone();

  Tail.push_back(HLNodeUtils::createBinaryHLInst(BOp->getOpcode(),
                                                 ScalarValue, InitValue,
                                                 ""/* Name */,
                                                 RedOp->clone()));

  HLNodeUtils::insertAfter(MainLoop, &Tail);
  return WideInst;
}

void AVRCodeGenHIR::widenNode(const HLNode *Node) {
  const HLInst *INode;
  INode = dyn_cast<HLInst>(Node);
  auto CurInst = INode->getLLVMInstruction();
  SmallVector<RegDDRef *, 6> WideOps;

  DEBUG(errs() << "DDRef ");
  DEBUG(INode->dump());
  for (auto Iter = INode->op_ddref_begin(), End = INode->op_ddref_end();
       Iter != End; ++Iter) {
    RegDDRef *WideRef, *Ref;

    Ref = *Iter;

    // Lval terminal refs get the widened ref duing the HLInst creation
    // later.
    if (Ref->isLval() && Ref->isTerminalRef())
      WideOps.push_back(nullptr);
    else {
      WideRef = widenRef(Ref);
      WideOps.push_back(WideRef);
      
      DEBUG(errs() << "Orig Ref: " << Ref << "\n");
      DEBUG(errs() << "Wide Ref: " << WideRef << "\n");
    }
  }

  DEBUG(Node->dump(true));

  HLInst *WideInst = nullptr;
  bool InsertInMap = true;

  if (auto BOp = dyn_cast<BinaryOperator>(CurInst)) {
    if (RHM.isReductionVariable(CurInst))
      WideInst = widenReductionNode(Node);
    else
      WideInst = HLNodeUtils::createBinaryHLInst(
        BOp->getOpcode(), WideOps[1], WideOps[2],
        CurInst->getName() + ".vec",  WideOps[0], BOp);
  } else if (isa<LoadInst>(CurInst)) {
    WideInst = HLNodeUtils::createLoad(WideOps[1],
                                       CurInst->getName() + ".vec",
                                       WideOps[0]);
  } else if (isa<StoreInst>(CurInst)) {
    WideInst = HLNodeUtils::createStore(WideOps[1],
                                        CurInst->getName() + ".vec",
                                        WideOps[0]);
    InsertInMap = false;
  } else if (isa<CastInst>(CurInst)) {
    assert(WideOps.size() == 2 && "invalid cast");

    WideInst =
        HLNodeUtils::createCastHLInst(VectorType::get(CurInst->getType(), VL),
                                      CurInst->getOpcode(), WideOps[1],
                                      CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<SelectInst>(CurInst)) {
    WideInst = HLNodeUtils::createSelect(
        INode->getPredicate(), WideOps[1], WideOps[2], WideOps[3], WideOps[4],
        CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<CmpInst>(CurInst)) {
    WideInst =
        HLNodeUtils::createCmp(INode->getPredicate(), WideOps[1], WideOps[2],
                               CurInst->getName() + ".vec", WideOps[0]);
  } else {
    llvm_unreachable("Unimplemented widening for inst");
  }

  // Add to WidenMap
  if (InsertInMap)
    WidenMap[INode->getLvalDDRef()->getSymbase()] = WideInst;

  HLNodeUtils::insertAsLastChild(MainLoop, WideInst);
}

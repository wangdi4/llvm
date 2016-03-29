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
#include "llvm/IR/Intel_LoopIR/HIRVerifier.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"

#define DEBUG_TYPE "VPODriver"

static cl::opt<unsigned> DefaultVL("default-vpo-vl", cl::init(4),
                                   cl::desc("Default vector length"));

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

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
bool AVRCodeGenHIR::isConstStrideRef(const RegDDRef *Ref, int64_t *CoeffPtr) {
  unsigned NestingLevel = OrigLoop->getNestingLevel();

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

  void visitRegDDRef(RegDDRef *RegDD);
  void visitCanonExpr(CanonExpr *CExpr);

public:
  HandledCheck(unsigned Level) : IsHandled(true), LoopLevel(Level) {}

  void visit(HLDDNode *Node);

  void visit(HLNode *Node) { IsHandled = false; }

  void postVisit(HLNode *Node) {}

  bool isDone() const override { return (!IsHandled); }
  bool isHandled() { return IsHandled; }
};
} // End anonymous namespace

void HandledCheck::visit(HLDDNode *Node) {

  if (!isa<HLInst>(Node)) {
    IsHandled = false;
    return;
  }

  auto Inst = cast<HLInst>(Node);

  // Calls are not supported for now
  if (Inst->isCallInst()) {
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
  // Visit CanonExprs inside the RegDDRefs
  for (auto Iter = RegDD->canon_begin(), End = RegDD->canon_end(); Iter != End;
       ++Iter) {
    visitCanonExpr(*Iter);
  }

  // Visit GEP Base
  if (RegDD->hasGEPInfo()) {
    // Addressof computation not supported for now.
    if (RegDD->isAddressOf()) {
      IsHandled = false;
      return;
    }

    auto BaseCE = RegDD->getBaseCE();

    if (!BaseCE->isInvariantAtLevel(LoopLevel)) {
      IsHandled = false;
      return;
    }

    visitCanonExpr(RegDD->getBaseCE());
  }
}

// Checks Canon Expr to see if we support it. Currently, we do not
// support blob IV coefficients
void HandledCheck::visitCanonExpr(CanonExpr *CExpr) {
  if (CExpr->hasIVBlobCoeff(LoopLevel))
    IsHandled = false;
}

bool AVRCodeGenHIR::loopIsHandled() {
  AVRWrn *AWrn = nullptr;
  AVRLoop *ALoop = nullptr;
  int VL = 0;
  unsigned int TripCount = 0;
  WRNVecLoopNode *WVecNode;
  HLLoop *Loop = nullptr;

  // We expect avr to be a AVRWrn node
  if (!(AWrn = dyn_cast<AVRWrn>(Avr)))
    return false;

  WVecNode = AWrn->getWrnNode();

  // An AVRWrn node is expected to have only one AVRLoop child
  for (auto Itr = AWrn->child_begin(), End = AWrn->child_end(); Itr != End;
       ++Itr) {
    if (AVRLoop *TempALoop = dyn_cast<AVRLoop>(Itr)) {
      if (ALoop)
        return false;

      ALoop = TempALoop;
    }
  }

  // Check that we have an AVRLoop
  if (!ALoop)
    return false;

  Loop = WVecNode->getHLLoop();
  assert(Loop && "Null HLLoop.");
  setOrigLoop(Loop);

  for (auto Itr = ALoop->child_begin(), End = ALoop->child_end(); Itr != End;
       ++Itr) {
    if (!isa<AVRAssignHIR>(Itr))
      return false;

    HandledCheck NodeCheck(OrigLoop->getNestingLevel());
    HLDDNode *INode = cast<AVRAssignHIR>(Itr)->getHIRInstruction();

    HLNodeUtils::visit(NodeCheck, INode);
    if (!NodeCheck.isHandled())
      return false;
  }

  VL = AWrn->getSimdVectorLength();

  // Assume the default vectorization factor when VL is 0
  if (VL == 0)
    VL = DefaultVL;

  // Loop parent is expected to be an HLRegion
  HLRegion *Parent = dyn_cast<HLRegion>(Loop->getParent());
  if (!Parent)
    return false;

  // Live out for reduction only
  for (auto LiveOut : Parent->live_out())
    if (!RHM.isReductionVariable(LiveOut.second))
      return false;

  // Check for unit stride and constant trip count
  const RegDDRef *UBRef = Loop->getUpperDDRef();
  assert(UBRef && " Loop UpperBound not found.");

  const RegDDRef *LBRef = Loop->getLowerDDRef();
  assert(LBRef && " Loop LowerBound not found.");

  const RegDDRef *StrideRef = Loop->getStrideDDRef();
  assert(StrideRef && " Loop Stride not found.");

  // Check if UB is Constant or not.
  int64_t UBConst;
  if (!UBRef->isIntConstant(&UBConst))
    return false;

  // Check if LB is Constant or not.
  int64_t LBConst;
  if (!LBRef->isIntConstant(&LBConst))
    return false;

  // Check if StepVal is Constant or not.
  int64_t StepConst;
  if (!StrideRef->isIntConstant(&StepConst))
    return false;

  if (StepConst != 1)
    return false;

  // TripCount is (Upper -Lower)/Stride + 1.
  int64_t ConstTripCount = (int64_t)((UBConst - LBConst) / StepConst) + 1;

  // Check for positive trip count and that  trip count is a multiple of vector
  // length.
  // No remainder loop is generated currently.
  if (ConstTripCount <= 0 || ConstTripCount % VL)
    return false;

  setALoop(ALoop);
  setTripCount(TripCount);
  setVL(VL);

  DEBUG(errs() << "Handled loop\n");
  return true;
}

bool AVRCodeGenHIR::vectorize() {
  bool RetVal, LoopHandled;

  LoopHandled = loopIsHandled();
  if (!LoopHandled)
    return false;

  DEBUG(errs() << "Handled loop before vec codegen: \n");
  DEBUG(OrigLoop->dump(true));

  RHM.mapHLNodes(cast<HLRegion>(OrigLoop->getParent()));

  RetVal = processLoop();

  DEBUG(errs() << "\n\n\nHandled loop after: \n");
  DEBUG(OrigLoop->dump(true));

  return RetVal;
}

bool AVRCodeGenHIR::processLoop() {
  HLLoop *LoopX = const_cast<HLLoop *>(OrigLoop);
  HLRegion *Parent = dyn_cast<HLRegion>(OrigLoop->getParent());
  HLContainerTy::iterator It1(Parent->child_begin());
  HLContainerTy::iterator It2(LoopX);

  // Erase intrinsics at the beginning of the region
  HLNodeUtils::erase(It1, It2);

  auto Begin = LoopX->child_begin();
  auto End = LoopX->child_end();

  for (auto Iter = ALoop->child_begin(), EndItr = ALoop->child_end();
       Iter != EndItr; ++Iter) {
    AVRAssignHIR *AvrAssign;

    AvrAssign = cast<AVRAssignHIR>(Iter);
    widenNode(AvrAssign->getHIRInstruction(), &*Begin);
  }

  // Get rid of the scalar children
  HLNodeUtils::erase(Begin, End);

  // Mark region for HIR code generation
  LoopX->getParentRegion()->setGenCode();
  LoopX->getStrideDDRef()->getSingleCanonExpr()->setConstant((int64_t)VL);
  return true;
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
    if (WidenMap.find(Ref->getSymbase()) != WidenMap.end()) {
      auto WInst = WidenMap[Ref->getSymbase()];
      
      WideRef = WInst->getLvalDDRef()->clone();
      WideRef->getSingleCanonExpr()->setDestType(VecRefDestTy);
      WideRef->getSingleCanonExpr()->setSrcType(VecRefSrcTy);
      return WideRef;
    }
  }

  WideRef = Ref->clone();

  // Set VectorType on WideRef base pointer - BaseDestType is set to pointer
  // type of VL-wide vector of Ref's DestType.
  if (WideRef->hasGEPInfo()) {
    PointerType *PtrType = cast<PointerType>(Ref->getBaseDestType());
    auto AddressSpace = PtrType->getAddressSpace();

    WideRef->setBaseDestType(PointerType::get(VecRefDestTy, AddressSpace));
  }

  // For unit stride ref, nothing else to do
  if (isConstStrideRef(Ref, &IVConstCoeff) && IVConstCoeff == 1)
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
      auto OldSymbase = BlobUtils::getBlobSymbase(BI);
      
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
  SmallVector<int, 16> LoMask, HiMask;
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
void ReductionHIRMngr::mapHLNodes(const HLRegion *HRegion) {
  for (auto RedItr : ReductionMap) {
    ReductionItem *RI = RedItr.second;
    const Value *Initializer = RI->getInitializer();
    bool Success = false;
    for (auto HLInstItr = HRegion->child_begin(), End = HRegion->child_end();
         HLInstItr != End; ++HLInstItr)
      if (auto HInst = dyn_cast<HLInst>(HLInstItr))
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

HLInst *AVRCodeGenHIR::widenReductionNode(const HLNode *Node, HLNode *Anchor) {
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

  HLNodeUtils::insertBefore(const_cast<HLLoop *>(OrigLoop), RedOpVecInst);

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

  HLNodeUtils::insertAfter(OrigLoop, &Tail);
  return WideInst;
}

void AVRCodeGenHIR::widenNode(const HLNode *Node, HLNode *Anchor) {
  const HLInst *INode;
  INode = dyn_cast<HLInst>(Node);
  auto CurInst = INode->getLLVMInstruction();
  SmallVector<RegDDRef *, 6> WideOps;

#if DEBUG
  HIRVerifier::verifyAll();
#endif

  DEBUG(errs() << "DDRef ");
  DEBUG(INode->dump());
  for (auto Iter = INode->op_ddref_begin(), End = INode->op_ddref_end();
       Iter != End; ++Iter) {
    RegDDRef *WideRef, *Ref;

    Ref = *Iter;

    // Lval SelfBlob refs get the widened ref duing the HLInst creation
    // later.
    if (Ref->isLval() && Ref->isSelfBlob())
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
      WideInst = widenReductionNode(Node, Anchor);
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

  HLNodeUtils::insertBefore(Anchor, WideInst);
}

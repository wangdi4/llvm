//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOAvrHIRCodeGen.cpp -- HIR vectior Code generation from AVR
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/VPO/Vecopt/VPOAvrHIRCodeGen.h"

#include "llvm/IR/Intrinsics.h"

#define DEBUG_TYPE "VPODriver"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

bool AVRCodeGenHIR::unitStrideRef(const RegDDRef *Ref) {
  if (Ref->getNumDimensions() != 1) {
    return false;
  }

  auto CE = Ref->getSingleCanonExpr();
  if (CE->getDefinedAtLevel() != 0) {
    return false;
  }

  if (CE->getIVConstCoeff(1) != 1) {
    return false;
  }

  if (CE->hasBlobIVCoeffs()) {
    return false;
  }

  return true;
}

bool AVRCodeGenHIR::loopIsHandled() {
  AVRWrn *AWrn = nullptr;
  AVRLoop *ALoop = nullptr;
  int VL = 0;
  unsigned int TripCount = 0;
  WRNVecLoopNode *WVecNode;
  HLLoop *Loop = nullptr;

  // We expect avr to be a AVRWrn node
  if (!(AWrn = dyn_cast<AVRWrn>(Avr))) {
    return false;
  }

  WVecNode = AWrn->getWrnNode();

  // An AVRWrn node is expected to have only one AVRLoop child
  for (auto Itr = AWrn->child_begin(), E = AWrn->child_end(); Itr != E; ++Itr) {
    if (AVRLoop *TempALoop = dyn_cast<AVRLoop>(Itr)) {
      if (ALoop) {
        return false;
      }
      ALoop = TempALoop;
    }
  }

  // Check that we have an AVRLoop
  if (!ALoop) {
    return false;
  }

  // Currently we only handle AVRAssignHIR, give up if we see any
  // other AVRs
  for (auto Itr = ALoop->child_begin(), E = ALoop->child_end(); 
       Itr != E; ++Itr) {
    if (isa<AVRAssignHIR>(Itr)) {
      // TBD: For now we only handle unit stride load/stores and instructions
      // whose operands are defined earlier in the loop. Are these checks 
      // sufficient?
      const HLInst *INode = 
        dyn_cast<const HLInst>(dyn_cast<AVRAssignHIR>(Itr)->getHIRInstruction());
      auto CurInst = const_cast<Instruction *>(INode->getLLVMInstruction());
      BinaryOperator *BOp;

      if ((BOp = dyn_cast<BinaryOperator>(CurInst)) &&
          (BOp->getOpcode() == Instruction::FAdd)) {
        // Check for form of %x = %y Bop %z
        for (unsigned OpIndex = 0, LastIndex = INode->getNumOperands(); OpIndex < LastIndex;
             ++OpIndex) {
          if (!INode->getOperandDDRef(OpIndex)->isSelfBlob()) {
            return false;
          }
        }
      }
      else if (isa<StoreInst>(CurInst)) {
        // Check for a[i] = %x
        auto Rval = INode->getRvalDDRef();
        auto Lval = INode->getLvalDDRef();

        if (!Rval->isSelfBlob()) {
          return false;
        }
        if (!unitStrideRef(Lval)) {
          return false;
        }
      }
      else if (isa<LoadInst>(CurInst)) {
        // Check for %x = a[i]
        auto Rval = INode->getRvalDDRef();
        auto Lval = INode->getLvalDDRef();

        if (!Lval->isSelfBlob()) {
          return false;
        }
        if (!unitStrideRef(Rval)) {
          return false;
        }
      }
      else {
        return false;
      }
    } else {
      return false;
    }
  }

  VL = AWrn->getSimdVectorLength();

  // Assume a default vectorization factor of 4
  if (VL == 0) {
    VL = 4;
  }

  Loop = WVecNode->getHLLoop();

  if (!Loop) {
    return false;
  }

  // Loop parent is expected to be an HLRegion
  HLRegion *Parent = dyn_cast<HLRegion>(Loop->getParent());
  if (!Parent) {
    return false;
  }

  // No live in/out for now
#if 0
  if (Parent->live_in_begin() != Parent->live_in_end()) {
    return false;
  }
#endif

  if (Parent->live_out_begin() != Parent->live_out_end()) {
    return false;
  }

  // Check for unit stride and constant trip count
  const RegDDRef *UBRef = Loop->getUpperDDRef();
  assert(UBRef && " Loop UpperBound not found.");

  const RegDDRef *LBRef = Loop->getLowerDDRef();
  assert(LBRef && " Loop LowerBound not found.");

  const RegDDRef *StrideRef = Loop->getStrideDDRef();
  assert(StrideRef && " Loop Stride not found.");

  // Check if UB is Constant or not.
  int64_t UBConst;
  if (!UBRef->isIntConstant(&UBConst)) {
    return false;
  }

  // Check if LB is Constant or not.
  int64_t LBConst;
  if (!LBRef->isIntConstant(&LBConst)) {
    return false;
  }

  // Check if StepVal is Constant or not.
  int64_t StepConst;
  if (!StrideRef->isIntConstant(&StepConst)) {
    return false;
  }
  if (StepConst != 1) {
    return false;
  }

  // TripCount is (Upper -Lower)/Stride + 1.
  int64_t ConstTripCount = (int64_t)((UBConst - LBConst) / StepConst) + 1;

  // Check for positive trip count and that  trip count is a multiple of vector length.
  // No remainder loop is generated currently.
  if (ConstTripCount <= 0 ||
      ConstTripCount % VL) {
    return false;
  }

  setALoop(ALoop);
  setOrigLoop(Loop);
  setTripCount(TripCount);
  setVL(VL);

  DEBUG(errs() << "Legal loop\n");
  return true;
}

bool AVRCodeGenHIR::vectorize() {
  bool RetVal;

  if (!loopIsHandled()) {
    return false;
  }

  RetVal = processLoop();

  DEBUG(OrigLoop->dump(true));

  return RetVal;
}

bool AVRCodeGenHIR::processLoop() {
  HLLoop *LoopX = const_cast<HLLoop *>(OrigLoop);
  HLRegion *Parent = dyn_cast<HLRegion>(OrigLoop->getParent());

  //Erase intrinsics at the beginning of the region
  HLNodeUtils::erase(Parent->child_begin(), LoopX);

  auto Begin = LoopX->child_begin();
  auto End = LoopX->child_end();

  for (auto Iter = ALoop->child_begin(), E = ALoop->child_end(); 
       Iter != E; ++Iter) {
    AVRAssignHIR *AvrAssign;

    AvrAssign = dyn_cast<AVRAssignHIR>(Iter);
    widenNode(AvrAssign->getHIRInstruction(), Begin);
  }

  // Get rid of the scalar children
  HLNodeUtils::erase(Begin, End);

  // Mark region for HIR code generation
  LoopX->getParentRegion()->setGenCode();
  LoopX->getStrideDDRef()->getSingleCanonExpr()->setConstant((int64_t)4);
  return true;
}

void AVRCodeGenHIR::widenNode(const HLNode *Node, HLNode *Anchor) {
  const HLInst *INode;
  INode = dyn_cast<HLInst>(Node);

  DEBUG(errs() << "DDRef ");
  DEBUG(INode->dump());
  for (auto Iter = INode->op_ddref_begin(), End = INode->op_ddref_end();
       Iter != End; ++Iter) {
    DEBUG(errs() << *Iter << "\n");
  }

  DEBUG(Node->dump(true));
  auto CurInst = const_cast<Instruction *>(INode->getLLVMInstruction());
  
  BinaryOperator *BOp;

  if ((BOp = dyn_cast<BinaryOperator>(CurInst)) &&
      (BOp->getOpcode() == Instruction::FAdd)) {

    HLInst *WInst1, *WInst2;

    assert(WidenMap.find(INode->getOperandDDRef(1)->getSymbase()) != WidenMap.end() &&
           "Value1 being added is expected to be widened already");
    assert(WidenMap.find(INode->getOperandDDRef(2)->getSymbase()) != WidenMap.end() &&
           "Value2 being added is expected to be widened already");

    // Get widened values
    WInst1 = WidenMap[INode->getOperandDDRef(1)->getSymbase()];
    WInst2 = WidenMap[INode->getOperandDDRef(2)->getSymbase()];

    auto Rval1 = WInst1->getLvalDDRef()->clone();
    auto Rval2 = WInst2->getLvalDDRef()->clone();

    auto WideInst = HLNodeUtils::createFAdd(Rval1, Rval2);

    // Add to WidenMap
    WidenMap[INode->getLvalDDRef()->getSymbase()] = WideInst;

    HLNodeUtils::insertBefore(Anchor, WideInst);
  }


#if 0
  // The widening of cast instruction assumes RVAL is the loop induction var
  if (isa<CastInst>(CurInst)) {
    auto Rval = dyn_cast<HLInst>(Node)->getRvalDDRef()->clone();
    auto RvalDestTy = Rval->getDestType();
    auto RvalSrcTy = Rval->getSrcType();

    Constant *CA[4];
    for (int i = 0; i < 4; ++i) {
      CA[i] = ConstantInt::getSigned(RvalDestTy, i);
    }
    ArrayRef<Constant *> AR(CA, 4);
    auto CV = ConstantVector::get(AR);

    unsigned Idx = 0;
    CanonExprUtils::createBlob(CV, true, &Idx);

    Rval->getSingleCanonExpr()->addBlob(Idx, 1);

    auto VecTyDestS = VectorType::get(RvalDestTy, VL);
    auto VecTySrcS = VectorType::get(RvalSrcTy, VL);

    // Set VectorType to Rval.
    Rval->getSingleCanonExpr()->setDestType(VecTyDestS);
    Rval->getSingleCanonExpr()->setSrcType(VecTySrcS);

    auto VecTyD = VectorType::get(CurInst->getType(), VL);
    auto WideInst = HLNodeUtils::createSIToFP(VecTyD, Rval);

    DEBUG(WideInst->dump(true));

    // Add to WidenMap
    WidenMap[INode->getLvalDDRef()->getSymbase()] = WideInst;

    HLNodeUtils::insertBefore(Anchor, WideInst);
  }
#endif

  if (isa<LoadInst>(CurInst)) {
    DEBUG(errs() << "Load inst: ");
    DEBUG(Node->dump(true));

    auto Rval = INode->getRvalDDRef()->clone();
    auto RvalDestTy = Rval->getDestType();
    auto VecTyDestS = VectorType::get(RvalDestTy, VL);

    // Set VectorType on Rval.
    Rval->setBaseDestType(PointerType::get(VecTyDestS, 0));

    auto WideInst = HLNodeUtils::createLoad(Rval, nullptr);

    // Store in widen map
    WidenMap[INode->getLvalDDRef()->getSymbase()] = WideInst;

    HLNodeUtils::insertBefore(Anchor, WideInst);
  }

  if (isa<StoreInst>(CurInst)) {
    HLInst *WInst;

    assert(WidenMap.find(INode->getRvalDDRef()->getSymbase()) != WidenMap.end() &&
           "Value being stored is expected to be widened already");

    WInst = WidenMap[INode->getRvalDDRef()->getSymbase()];

    auto Lval = dyn_cast<HLInst>(Node)->getLvalDDRef()->clone();
    auto Rval = WInst->getLvalDDRef()->clone();
    Lval->setBaseDestType(PointerType::get(Rval->getDestType(), 0));
    auto WideInst = HLNodeUtils::createStore(Rval, Lval);

#if 0
    Lval = WideInst->getLvalDDRef();
    Lval->setBaseDestType(PointerType::get(Rval->getDestType(), 0));
#endif

    HLNodeUtils::insertBefore(Anchor, WideInst);
  }
}

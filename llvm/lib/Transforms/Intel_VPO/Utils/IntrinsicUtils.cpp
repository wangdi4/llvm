//==-- IntrinsicUtils.cpp - Utilities for VPO related intrinsics -*- C++ -*-==//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file provides a set of utilities for VPO-based intrinsic function
/// calls. E.g., directives that mark the beginning and end of SIMD and
/// parallel regions.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"

#define DEBUG_TYPE "VPOIntrinsicUtils"

using namespace llvm;
using namespace llvm::vpo;

bool VPOUtils::stripDirectives(WRegionNode *WRN) {
  bool success = true;
  BasicBlock *EntryBB = WRN->getEntryBBlock();
  BasicBlock *ExitBB = WRN->getExitBBlock();

  success = success && VPOUtils::stripDirectives(*EntryBB);
  success = success && VPOUtils::stripDirectives(*ExitBB);

  return success;
}

bool VPOUtils::stripDirectives(BasicBlock &BB) {
  SmallVector<IntrinsicInst *, 4> IntrinsicsToRemove;
  IntrinsicInst *IntrinCall = nullptr;

  for (Instruction &I : BB) {
    if ((IntrinCall = dyn_cast<IntrinsicInst>(&I))) {
      Intrinsic::ID Id = IntrinCall->getIntrinsicID();
      if (VPOAnalysisUtils::isIntelDirectiveOrClause(Id)) {
        IntrinsicsToRemove.push_back(IntrinCall);
      }
    }
  }

  // Remove the directive intrinsics.
  // SimplifyCFG will remove any blocks that become empty.
  unsigned Idx = 0;
  for (Idx = 0; Idx < IntrinsicsToRemove.size(); ++Idx) {
    IntrinsicsToRemove[Idx]->eraseFromParent();
  }

  // Returns true if any elimination happens.
  return Idx > 0;
}

bool VPOUtils::stripDirectives(Function &F) {
  bool changed = false;

  for (BasicBlock &BB: F) {
    changed |= stripDirectives(BB);
  }

  return changed;
}

<<<<<<< HEAD
void VPOUtils::addNoFeatureOutline(CallInst *Call) {
  Call->setMetadata(IGNORE_FEATURE_OUTLINING_STRING,
                    MDNode::get(Call->getContext(), {}));
}

CallInst *VPOUtils::createMaskedGatherCall(Value *VecPtr,
                                           IRBuilder<> &Builder,
                                           unsigned Alignment,
                                           Value *Mask,
                                           Value *PassThru) {
  auto NewCallInst = Builder.CreateMaskedGather(VecPtr, Alignment, Mask,
                                                PassThru);
  addNoFeatureOutline(NewCallInst);
  return NewCallInst;
}

CallInst *VPOUtils::createMaskedScatterCall(Value *VecPtr,
                                            Value *VecData,
                                            IRBuilder<> &Builder,
                                            unsigned Alignment,
                                            Value *Mask) {
  auto NewCallInst = Builder.CreateMaskedScatter(VecData, VecPtr, Alignment,
                                                 Mask);
  addNoFeatureOutline(NewCallInst);
=======
CallInst *VPOUtils::createMaskedGatherCall(Module *M,
                                                   Value *VecPtr,
                                                   IRBuilder<> &Builder,
                                                   unsigned Alignment,
                                                   Value *Mask,
                                                   Value *PassThru)
{
  CallInst *NewCallInst;
  Intrinsic::ID IntrinsicID = Intrinsic::masked_gather;

  Type *Int1Ty = Type::getInt1Ty(M->getContext()); // Mask type
  Type *Int32Ty = Type::getInt32Ty(M->getContext()); // Alignment type

  Type *VectorOfPointersType = VecPtr->getType();
  Type *ElementPointerType = VectorOfPointersType->getVectorElementType();
  Type *ElementType = ElementPointerType->getPointerElementType();
  unsigned VL = VectorOfPointersType->getVectorNumElements();
  Type *VectorOfElementsType = VectorType::get(ElementType, VL);

  SmallVector<Type *, 4> ArgumentTypes;
  SmallVector<Value *, 4> Arguments;

  ArgumentTypes.push_back(VectorOfElementsType);
  Function *Intrinsic = Intrinsic::getDeclaration(M, 
                                                  IntrinsicID, 
                                                  ArrayRef<Type *>(ArgumentTypes));
  assert(Intrinsic &&
         "Expected to have an intrinsic for this memory operation");

  // Vector of pointers to load
  Arguments.push_back(VecPtr);

  // Alignment argument
  Arguments.push_back(ConstantInt::get(Int32Ty, Alignment));

  // Mask argument
  if (!Mask) 
    Mask = ConstantVector::getSplat(VL, ConstantInt::get(Int1Ty, 1));

  Arguments.push_back(Mask);

  // Passthru argument
  if (!PassThru) 
    PassThru = UndefValue::get(VectorOfElementsType);

  Arguments.push_back(PassThru);

  NewCallInst = Builder.CreateCall(Intrinsic, Arguments);
  return NewCallInst;
}

CallInst *VPOUtils::createMaskedScatterCall(Module *M,
                                                    Value *VecPtr,
                                                    Value *VecData,
                                                    IRBuilder<> &Builder,
                                                    unsigned Alignment,
                                                    Value *Mask)
{
  CallInst *NewCallInst;
  Intrinsic::ID IntrinsicID = Intrinsic::masked_scatter;

  Type *Int1Ty = Type::getInt1Ty(M->getContext()); // Mask type
  Type *Int32Ty = Type::getInt32Ty(M->getContext()); // Alignment type

  Type *VectorOfPointersType = VecPtr->getType();
  Type *ElementPointerType = VectorOfPointersType->getVectorElementType();
  Type *ElementType = ElementPointerType->getPointerElementType();
  unsigned VL = VectorOfPointersType->getVectorNumElements();
  Type *VectorOfElementsType = VectorType::get(ElementType, VL);

  SmallVector<Type *, 4> ArgumentTypes;
  SmallVector<Value *, 4> Arguments;

  ArgumentTypes.push_back(VectorOfElementsType);
  Function *Intrinsic = Intrinsic::getDeclaration(M, 
                                                  IntrinsicID, 
                                                  ArrayRef<Type *>(ArgumentTypes));
  assert(Intrinsic &&
         "Expected to have an intrinsic for this memory operation");

  // Vector of pointers to load
  Arguments.push_back(VecData);
  Arguments.push_back(VecPtr);

  // Alignment argument
  Arguments.push_back(ConstantInt::get(Int32Ty, Alignment));

  // Mask argument
  if (!Mask) 
    Mask = ConstantVector::getSplat(VL, ConstantInt::get(Int1Ty, 1));

  Arguments.push_back(Mask);

  NewCallInst = Builder.CreateCall(Intrinsic, Arguments);
>>>>>>> 6aecc073a16f8def9a529e6ef37cfe1d6ab582fd
  return NewCallInst;
}

CallInst *VPOUtils::createMaskedLoadCall(Value *VecPtr,
                                         IRBuilder<> &Builder,
                                         unsigned Alignment,
                                         Value *Mask,
                                         Value *PassThru) {
  auto NewCallInst = Builder.CreateMaskedLoad(VecPtr, Alignment, Mask,
                                               PassThru);
  return NewCallInst;
}

CallInst *VPOUtils::createMaskedStoreCall(Value *VecPtr,
                                          Value *VecData,
                                          IRBuilder<> &Builder,
                                          unsigned Alignment,
                                          Value *Mask) {
  auto NewCallInst = Builder.CreateMaskedStore(VecData, VecPtr, Alignment,
                                                Mask);
  return NewCallInst;
}

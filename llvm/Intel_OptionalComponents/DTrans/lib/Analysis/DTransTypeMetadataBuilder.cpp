//===-------DTransTypeMetadataBuilder.cpp --Builder for DTrans metadata ---===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransTypeMetadataBuilder.h"

#include "Intel_DTrans/Analysis/DTransOPUtils.h"
#include "Intel_DTrans/Analysis/DTransTypeMetadataConstants.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/MDBuilder.h"

namespace llvm {
namespace dtransOP {

void DTransTypeMetadataBuilder::addDTransMDNode(Value &V, MDNode *MD) {
  if (auto *F = dyn_cast<Function>(&V))
    F->setMetadata(DTransFuncTypeMDTag, MD);
  else if (auto *I = dyn_cast<Instruction>(&V))
    I->setMetadata(MDDTransTypeTag, MD);
  else if (auto *G = dyn_cast<GlobalObject>(&V))
    G->setMetadata(MDDTransTypeTag, MD);
  else
    llvm_unreachable("Unexpected Value type passed into addDTransMDNode");
}

void DTransTypeMetadataBuilder::setDTransFuncMetadata(
    Function *F, DTransFunctionType *FnType) {

  auto RemoveDTransFuncIndexAttribute = [](Function *F, unsigned Index) {
    F->removeAttributeAtIndex(Index, DTransFuncIndexTag);
  };

  // Add a DTrans function index attribute to 'F' if 'Ty' requires an attribute
  // because it refers to a pointer type, and update the MDTypeList with the
  // metadata reference to add to the function. 'Index' is used to specify the
  // return type or argument number the attribute will be attached to.
  auto AddAttributeIfNeeded = [](Function *F, DTransType *Ty, unsigned Index,
                                 SmallVectorImpl<Metadata *> &MDTypeList) {
    if (hasPointerType(Ty)) {
      Metadata *RetMD = Ty->createMetadataReference();
      MDTypeList.push_back(RetMD);
      // Attribute numbering starts with 1.
      unsigned AttrNumber = MDTypeList.size();
      std::string Label = std::to_string(AttrNumber);
      Attribute Attr =
          Attribute::get(F->getContext(), DTransFuncIndexTag, Label);
      F->addAttributeAtIndex(Index, Attr);
    }
  };

  // Clear any existing DTrans attributes for the function
  F->setMetadata(DTransFuncTypeMDTag, nullptr);
  RemoveDTransFuncIndexAttribute(F, AttributeList::ReturnIndex);
  unsigned NumArgs = F->arg_size();
  for (unsigned ArgIdx = 0; ArgIdx < NumArgs; ++ArgIdx)
    RemoveDTransFuncIndexAttribute(F, AttributeList::FirstArgIndex + ArgIdx);

  if (!FnType)
    return;

  // Build the new attribute and metadata information for the function type.
  DTransType *RetTy = FnType->getReturnType();
  assert(FnType->getNumArgs() == NumArgs && "Invalid FnType");
  assert(RetTy && "Invalid FnType");

  SmallVector<Metadata *, 8> MDTypeList;
  AddAttributeIfNeeded(F, RetTy, AttributeList::ReturnIndex, MDTypeList);
  for (unsigned ArgIdx = 0; ArgIdx < NumArgs; ++ArgIdx) {
    DTransType *ArgTy = FnType->getArgType(ArgIdx);
    assert(ArgTy && "Invalid FnType");
    AddAttributeIfNeeded(F, ArgTy, AttributeList::FirstArgIndex + ArgIdx,
                         MDTypeList);
  }

  if (!MDTypeList.empty()) {
    auto *MDTypes = MDTuple::getDistinct(F->getContext(), MDTypeList);
    F->addMetadata(DTransFuncTypeMDTag, *MDTypes);
  }
}

MDTuple *DTransTypeMetadataBuilder::createLiteralStructMetadata(
    LLVMContext &Ctx, ArrayRef<Metadata *> MDTypeList) {
  SmallVector<Metadata *, 16> FieldEncodings;
  MDBuilder MDB(Ctx);
  FieldEncodings.push_back(MDB.createString("L"));
  FieldEncodings.push_back(MDB.createConstant(
      ConstantInt::get(Type::getInt32Ty(Ctx), MDTypeList.size())));
  for (auto *TypeNode : MDTypeList)
    FieldEncodings.push_back(TypeNode);
  MDTuple *LiteralMD = MDTuple::get(Ctx, FieldEncodings);
  return LiteralMD;
}

} // end namespace dtransOP
} // end namespace llvm

//===------DTransAnnotator.cpp - Annotation utilities for DTrans ----------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
// Utility routines for getting/setting DTrans annotations used to convey
// information from one transformation back to the analysis or to another
// transformation. (metadata or intrinsics)
///
// ===--------------------------------------------------------------------=== //
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
namespace dtrans {

const char *DTransAnnotator::AnnotNames[DTransAnnotator::DPA_Last] = {
    // metadata attachment that associates an instruction as
    // being a specific type.
    "dtrans-type"};

// Annotate an instruction to indicate that the value should be treated as a
// specific type for DTrans. Currently, this attaches metadata to the
// instruction, but could be changed to insert an intrinsic in the IR in the
// future. We currently limit this to holding a single type.
//
// The format of the metadata is to store a null value of the specified type
// as follows:
//   { Ty null }
//
// The use of a null value of the type enables the type to be kept up-to-date
// when DTrans transformations run because when the instruction referencing the
// metadata is remapped, the type within the metadata will be remapped as well,
// if the type changes.
void DTransAnnotator::createDTransTypeAnnotation(Instruction *I,
                                                 llvm::Type *Ty) {
  assert(Ty->isPointerTy() && "Annotation type must be pointer type");
  assert(I->getMetadata(DTransAnnotator::AnnotNames[DPA_DTransType]) ==
             nullptr &&
         "Only a single dtrans type metadata attachment allowed.");

  LLVMContext &Ctx = I->getContext();
  MDNode *MD =
      MDNode::get(Ctx, {ConstantAsMetadata::get(Constant::getNullValue(Ty))});
  I->setMetadata(DTransAnnotator::AnnotNames[DPA_DTransType], MD);
}

void DTransAnnotator::removeDTransTypeAnnotation(Instruction *I) {
  I->setMetadata(DTransAnnotator::AnnotNames[DPA_DTransType], nullptr);
}

// Get the type that exists in an annotation, if one exists, for the
// instruction.
llvm::Type *DTransAnnotator::lookupDTransTypeAnnotation(Instruction *I) {
  auto *MD = I->getMetadata(DTransAnnotator::AnnotNames[DPA_DTransType]);
  if (!MD)
    return nullptr;

  assert(MD->getNumOperands() == 1 && "Unexpected metadata operand count");
  auto &MDOpp1 = MD->getOperand(0);
  auto *TyMD = dyn_cast<ConstantAsMetadata>(MDOpp1);
  if (!TyMD)
    return nullptr;

  return TyMD->getType();
}

} // namespace dtrans
} // namespace llvm

//===----DTransTypeMetadataPropagator.cpp - DTrans metadata propagation----===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTransTypeMetadataPropagator class, which is used to
// propagate DTrans type metadata.
//
//===----------------------------------------------------------------------===//
#include "Intel_DTrans/Analysis/DTransTypeMetadataPropagator.h"

#include "Intel_DTrans/Analysis/DTransTypeMetadataBuilder.h"
#include "Intel_DTrans/Analysis/DTransTypeMetadataConstants.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#define DEBUG_TYPE "dtrans-typemetadata-propagator"

namespace llvm {
namespace dtransOP {

// Populate the StructToMDDescriptor mapping with the list of structures
// from the intel.dtrans.types and the associated metadata node.
void DTransTypeMetadataPropagator::initialize(Module &M) {
  if (Initialized)
    return;

  Initialized = true;
  NamedMDNode *DTransMD = TypeMetadataReader::getDTransTypesMetadata(M);
  if (!DTransMD)
    return;

  DTransTypeMetadataAvailable = true;
  for (auto *MD : DTransMD->operands()) {
    if (MD->getNumOperands() < DTransStructMDConstants::MinOperandCount)
      continue;

    if (auto *MDS = dyn_cast<MDString>(
            MD->getOperand(DTransStructMDConstants::RecTypeOffset)))
      if (!MDS->getString().equals("S"))
        continue;

    // Ignore opaque structure definitions or malformed metadata
    auto *FieldCountMD = dyn_cast<ConstantAsMetadata>(
        MD->getOperand(DTransStructMDConstants::FieldCountOffset));
    if (!FieldCountMD)
      continue;
    int32_t FieldCount =
        cast<ConstantInt>(FieldCountMD->getValue())->getSExtValue();
    if (FieldCount == -1)
      continue;

    auto *TyMD = dyn_cast<ConstantAsMetadata>(
        MD->getOperand(DTransStructMDConstants::StructTypeOffset));
    if (!TyMD)
      continue;
    llvm::StructType *StTy = cast<llvm::StructType>(TyMD->getType());
    auto Res = StructToMDDescriptor.insert({StTy, MD});
    if (!Res.second && Res.first->second != MD)
      LLVM_DEBUG(dbgs() << "Structure " << *TyMD
                        << " described by more than one metadata node");
  }
}

// Return the MDNode that describes the structure.
MDNode *
DTransTypeMetadataPropagator::getDTransMDNode(llvm::StructType *Ty) const {
  assert(!Ty->isLiteral() && "Lookup is for named structures only");
  return StructToMDDescriptor.lookup(Ty);
}

void DTransTypeMetadataPropagator::updateDTransMetadata(AllocaInst *NewAI,
                                                        AllocaInst &OrigAI,
                                                        uint64_t Offset,
                                                        uint64_t Size) {
  llvm::Type *NewTy = NewAI->getAllocatedType();
  if (!dtrans::hasPointerType(NewTy))
    return;

  // This method only supports taking information from a structure allocation.
  llvm::Type *OrigTy = OrigAI.getAllocatedType();
  if (!OrigTy->isStructTy())
    return;

  // Only named structures are currently supported, but this could be
  // extended to take a metadata descriptor used on an alloca of a literal
  // structure, if necessary.
  auto *OrigStructTy = cast<llvm::StructType>(OrigTy);
  if (OrigStructTy->isLiteral()) {
    LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported - "
                         "Literal struct not currently supported:"
                      << OrigAI << " to " << *NewAI << "\n");
    return;
  }

  if (!Initialized)
    initialize(*OrigAI.getModule());

  // Check for the metadata representation of the original type.
  MDNode *StructMD = getDTransMDNode(OrigStructTy);
  if (!StructMD) {
    if (DTransTypeMetadataAvailable)
      LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported - No "
                           "metadata for original structure:"
                        << OrigAI << " to " << *NewAI << "\n");
    return;
  }

  // Determine which structure metadata descriptor to use to get the information
  // about the types of the extracted fields, and set it in 'StructMD'. The
  // structure metadata may be different than the current structure being
  // allocated due to nested structures. 'BeginFieldNum' will be the field
  // number of the first field extracted. 'EndFieldNum' will be 1 past the
  // actual field number because the 'Size' represents the number of bytes being
  // extracted, so will point to where the next field begins.
  const DataLayout &DL = OrigAI.getModule()->getDataLayout();
  unsigned BeginFieldNum = 0;
  unsigned EndFieldNum = 0;
  if (!identifyFieldRange(DL, OrigStructTy, &StructMD, Offset, Size,
                          &BeginFieldNum, &EndFieldNum, OrigAI, *NewAI))
    return;

  if (EndFieldNum - BeginFieldNum == 1) {
    // Single element being extracted.
    unsigned MDIndex = BeginFieldNum + DTransStructMDConstants::FieldNodeOffset;
    if (MDIndex > StructMD->getNumOperands()) {
      LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported "
                           "- MD operands do match type "
                        << OrigAI << " to " << *NewAI << "\n");
      return;
    }

    auto *FieldMD = dyn_cast<MDNode>(StructMD->getOperand(MDIndex));
    DTransTypeMetadataBuilder::addDTransMDNode(*NewAI, FieldMD);
    LLVM_DEBUG(dbgs() << "Updated alloca: " << OrigAI << " to  " << *NewAI
                      << "\n"
                      << *OrigStructTy << " @ " << BeginFieldNum << "\n");
    return;
  }

  // Multiple fields extracted, expect to be forming a literal structure.
  // If the new type is a literal struct type, construct a new literal struct
  // metadata node for the set of fields.
  if (!NewTy->isStructTy() || !cast<llvm::StructType>(NewTy)->isLiteral()) {
    LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported for "
                         "- Non-literal struct target: "
                      << OrigAI << " to " << *NewAI << "\n");
    return;
  }

  unsigned BeginMDIndex =
      BeginFieldNum + DTransStructMDConstants::FieldNodeOffset;
  unsigned EndMDIndex = EndFieldNum + DTransStructMDConstants::FieldNodeOffset;
  if (EndMDIndex > StructMD->getNumOperands()) {
    LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported - "
                         "Metadata indices do not match original type: "
                      << OrigAI << " to " << *NewAI << "\n"
                      << *OrigStructTy << " [" << BeginFieldNum << ".."
                      << EndFieldNum << ")\n"
                      << *StructMD << " [" << BeginMDIndex << ".." << EndMDIndex
                      << ")\n");
    return;
  }

  LLVMContext &Ctx = NewTy->getContext();
  SmallVector<Metadata *, 16> TypeMD;
  for (unsigned Idx = BeginMDIndex; Idx < EndMDIndex; ++Idx)
    TypeMD.push_back(dyn_cast<MDNode>(StructMD->getOperand(Idx)));

  MDTuple *LiteralMD =
      DTransTypeMetadataBuilder::createLiteralStructMetadata(Ctx, TypeMD);
  DTransTypeMetadataBuilder::addDTransMDNode(*NewAI, LiteralMD);
  LLVM_DEBUG(dbgs() << "Updated alloca: " << OrigAI << " to " << *NewAI << "\n"
                    << *OrigStructTy << " [" << BeginFieldNum << ".."
                    << EndFieldNum << ")\n");
}

// Starting from the initial value of 'StructMD', identify the set of fields
// that metadata needs to be copied for. When the location accessed by Offset
// is within a nested element, 'StructMD' will be updated to refer to the
// actual structure being used.
bool DTransTypeMetadataPropagator::identifyFieldRange(
    const DataLayout &DL, llvm::Type *Ty, MDNode **StructMD, uint64_t Offset,
    uint64_t Size, unsigned *ActualBeginFieldNum, unsigned *ActualEndFieldNum,
    const AllocaInst &OrigAI, const AllocaInst &NewAI) {

  if (*StructMD == nullptr) {
    LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported - "
                         "no structure metadata available: "
                      << OrigAI << " to " << NewAI << "\n");
    return false;
  }

  // Currently, only structure types are supported. This may need to be extended
  // for arrays.
  if (!Ty->isStructTy()) {
    LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported - "
                         "Only structure types supported: "
                      << OrigAI << " to " << NewAI << "\n");
    return false;
  }

  auto *OrigStructTy = cast<llvm::StructType>(Ty);
  const StructLayout *SL = DL.getStructLayout(OrigStructTy);
  if (Offset >= SL->getSizeInBytes()) {
    // This should not occur when processing SROA generated types.
    LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported - "
                         "offset out of range: "
                      << OrigAI << " to " << NewAI << "\n");
    return false;
  }

  uint64_t EndOffset = Offset + Size;
  if (EndOffset > SL->getSizeInBytes()) {
    // This should not occur when processing SROA generated types.
    LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported - "
                         "size out of range: "
                      << OrigAI << " to " << NewAI << "\n");
    return false;
  }

  // Identify the range of fields being moved for the original allocation to the
  // new allocation.
  unsigned BeginFieldNum = SL->getElementContainingOffset(Offset);
  Type *ElementTy = OrigStructTy->getElementType(BeginFieldNum);
  uint64_t ElementOffset = Offset - SL->getElementOffset(BeginFieldNum);
  uint64_t ElementSize = DL.getTypeAllocSize(ElementTy).getFixedSize();
  if (ElementOffset >= ElementSize) {
    // This should not occur when processing SROA generated types.
    LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported - offset "
                         "is within field padding: "
                      << OrigAI << " to " << NewAI << "\n");

    return false;
  }

  // See if any partition can be contained by the element.
  // -  ElementOffset > 0 => Extraction starts somewhere after the first member
  //                         of the element.
  // -  Size < ElementSize => A subset of the field member is being extracted.
  if (ElementOffset > 0 || Size < ElementSize) {
    if ((ElementOffset + Size) > ElementSize) {
      // This should not occur when processing SROA generated types.
      LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported - "
                           "extraction spans nested element: "
                        << OrigAI << " to " << NewAI << "\n");
      return false;
    }

    // Examine an aggregate type inside the structure currently being examined.
    llvm::Type *FieldTy = OrigStructTy->getElementType(BeginFieldNum);
    if (FieldTy->isAggregateType()) {
      if (auto *InnerStructTy = dyn_cast<llvm::StructType>(FieldTy)) {
        if (InnerStructTy->hasName()) {
          *StructMD = getDTransMDNode(InnerStructTy);
          return identifyFieldRange(DL, InnerStructTy, StructMD, ElementOffset,
                                    Size, ActualBeginFieldNum,
                                    ActualEndFieldNum, OrigAI, NewAI);
        }
      }
    }

    return false;
  }

  assert(ElementOffset == 0 &&
         "ElementOffset must start from a field boundary");
  if (Size == ElementSize) {
    // Single element being extracted.
    // It's possible the element is nested within the specific field.
    llvm::Type *FieldTy = OrigStructTy->getElementType(BeginFieldNum);
    if (FieldTy->isAggregateType()) {
      if (auto *InnerStructTy = dyn_cast<llvm::StructType>(FieldTy)) {
        if (InnerStructTy->hasName()) {
          *StructMD = getDTransMDNode(InnerStructTy);
          return identifyFieldRange(DL, InnerStructTy, StructMD, ElementOffset,
                                    Size, ActualBeginFieldNum,
                                    ActualEndFieldNum, OrigAI, NewAI);
        }
      }

      LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported for "
                           " - nested element is not named structure: "
                        << OrigAI << " to " << NewAI << "\n"
                        << *OrigStructTy << "@" << Offset << " Bytes: " << Size
                        << "\n");
      return false;
    }

    *ActualBeginFieldNum = BeginFieldNum;
    *ActualEndFieldNum = BeginFieldNum + 1;
    return true;
  }

  // 'EndFieldNum' will be 1 past the actual field number because the EndOffset
  // represents the number of bytes being extracted, so will point to where the
  // next field begins.
  unsigned EndFieldNum = OrigStructTy->getNumElements();
  if (EndOffset < SL->getSizeInBytes()) {
    EndFieldNum = SL->getElementContainingOffset(EndOffset);
    if (BeginFieldNum == EndFieldNum - 1) {
      // This should not occur when processing SROA generated types.
      LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported - "
                           "Within a single element and its padding: "
                        << OrigAI << " to " << NewAI << "\n");
      return false;
    }
    // The SROA does not peel sub-elements out of the last element, so the final
    // offset should match the location where the field ends.
    if (SL->getElementOffset(EndFieldNum) != EndOffset) {
      // This should not occur when processing SROA generated types.
      LLVM_DEBUG(dbgs() << "Warning: Metadata propagation not supported - "
                           "Size does not end at field boundary: "
                        << OrigAI << " to " << NewAI << "\n");
      return false;
    }
  }

  *ActualBeginFieldNum = BeginFieldNum;
  *ActualEndFieldNum = EndFieldNum;
  return true;
}

void DTransTypeMetadataPropagator::copyDTransMetadata(Value *DestValue,
                                                      const Value *SrcValue) {
  // Functions use a different type of metadata. For now, those need to be dealt
  // with differently, such as by using
  // DTransTypeMetadataBuilder::setDTransFuncMetadata.
  assert(!isa<Function>(SrcValue) && "Function values not currently supported");

  if (MDNode *MD = dtransOP::TypeMetadataReader::getDTransMDNode(*SrcValue))
    DTransTypeMetadataBuilder::addDTransMDNode(*DestValue, MD);
}

} // end namespace dtransOP
} // end namespace llvm

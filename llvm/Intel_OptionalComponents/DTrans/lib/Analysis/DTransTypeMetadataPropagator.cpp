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
  bool HasDTransMD = TypeMetadataReader::mapStructsToMDNodes(
      M, StructToMDDescriptor, /*IncludeOpaque=*/false);
  if (HasDTransMD)
    DTransTypeMetadataAvailable = true;
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

// Create new intel_dtrans_type metadata for NewGV which is created by
// WholeProgramDevirt.
// Type of NewGV: { [i8 x B.Before.Bytes], Type of OldGV, [i8 x AfterBytesSize]
// }
void DTransTypeMetadataPropagator::setDevirtVarDTransMetadata(
    GlobalVariable *OldGV, GlobalVariable *NewGV, uint64_t BeforeBytesSize,
    uint64_t AfterBytesSize) {
  MDNode *MMD = dtransOP::TypeMetadataReader::getDTransMDNode(*OldGV);
  if (!MMD)
    return;
  LLVMContext &Ctx = OldGV->getType()->getContext();
  auto CharMD = llvm::MDNode::get(
      Ctx, {llvm::ConstantAsMetadata::get(
                llvm::Constant::getNullValue(llvm::Type::getInt8Ty(Ctx))),
            llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), 0))});

  llvm::SmallVector<llvm::Metadata *> NewMD;
  auto Arr1MD = MDNode::get(Ctx, {MDString::get(Ctx, "A"),
                                  ConstantAsMetadata::get(ConstantInt::get(
                                      Type::getInt32Ty(Ctx), BeforeBytesSize)),
                                  CharMD});
  NewMD.push_back(Arr1MD);
  if (auto *RefMD = dyn_cast<MDNode>(MMD->getOperand(0)))
    MMD = RefMD;
  NewMD.push_back(MMD);

  auto Arr2MD = MDNode::get(Ctx, {MDString::get(Ctx, "A"),
                                  ConstantAsMetadata::get(ConstantInt::get(
                                      Type::getInt32Ty(Ctx), AfterBytesSize)),
                                  CharMD});
  NewMD.push_back(Arr2MD);
  MDTuple *LiteralMD =
      DTransTypeMetadataBuilder::createLiteralStructMetadata(Ctx, NewMD);
  NewGV->setMetadata(MDDTransTypeTag, nullptr);
  NewGV->setMetadata(llvm::dtransOP::MDDTransTypeTag, LiteralMD);
}

// Create new intel_dtrans_type metadata for NewGV which is created by
// GlobalOpt.
//  Type of NewGV: [i8* x NewArrSize]
void DTransTypeMetadataPropagator::setGlobUsedVarDTransMetadata(
    GlobalVariable *OldGV, GlobalVariable *NewGV, uint64_t NewArrSize) {

  // Skip if DTransMDNode is not attached to OldGV.
  MDNode *MD = dtransOP::TypeMetadataReader::getDTransMDNode(*OldGV);
  if (!MD)
    return;

  // Get metadata for Int8Ptr.
  LLVMContext &Ctx = NewGV->getType()->getContext();
  auto CharPtrMD = llvm::MDNode::get(
      Ctx, {llvm::ConstantAsMetadata::get(
                llvm::Constant::getNullValue(llvm::Type::getInt8Ty(Ctx))),
            llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), 1))});

  // Create metadata for [i8* x NewArrSize]
  auto ArrMD = MDNode::get(Ctx, {MDString::get(Ctx, "A"),
                                 ConstantAsMetadata::get(ConstantInt::get(
                                     Type::getInt32Ty(Ctx), NewArrSize)),
                                 CharPtrMD});
  NewGV->setMetadata(llvm::dtransOP::MDDTransTypeTag, ArrMD);
}

// Create new intel_dtrans_type metadata for NewGV which is created during
// IRMover.
//   Type of NewGV: [EltTy x NewArrSize]
void DTransTypeMetadataPropagator::setGlobAppendingVarDTransMetadata(
    const GlobalVariable *SrcGV, GlobalVariable *DstGV, GlobalVariable *NewGV,
    uint64_t NewArrSize) {
  // Makes sure both SrcGV and DstGV have intel_dtrans_type metadata.
  MDNode *MD = dtransOP::TypeMetadataReader::getDTransMDNode(*SrcGV);
  if (!MD)
    return;
  if (DstGV) {
    MDNode *DMD = dtransOP::TypeMetadataReader::getDTransMDNode(*DstGV);
    if (!DMD)
      return;
  }
  assert(isa<MDString>(MD->getOperand(0)) && "Expected MD String");
  assert(cast<MDString>(MD->getOperand(0))->getString().equals("A") &&
         "Expected Array");

  LLVMContext &Ctx = NewGV->getType()->getContext();
  auto *RefMD = dyn_cast<MDNode>(MD->getOperand(2));
  assert(RefMD && "Expected metadata constant");

  // Create MD for [EltTy x NewArrSize]
  auto ArrMD = MDNode::get(Ctx, {MDString::get(Ctx, "A"),
                                 ConstantAsMetadata::get(ConstantInt::get(
                                     Type::getInt32Ty(Ctx), NewArrSize)),
                                 RefMD});
  NewGV->setMetadata(llvm::dtransOP::MDDTransTypeTag, ArrMD);
}

} // end namespace dtransOP
} // end namespace llvm

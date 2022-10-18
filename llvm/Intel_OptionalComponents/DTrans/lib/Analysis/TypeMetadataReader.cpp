//===-----------TypeMetadataReader.cpp - Decode metadata annotations-------===//
//
// Copyright (C) 2019-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/Analysis/DTransOPUtils.h"
#include "Intel_DTrans/Analysis/DTransTypeMetadataBuilder.h"
#include "Intel_DTrans/Analysis/DTransTypeMetadataConstants.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "dtrans-typemetadatareader"

static llvm::cl::opt<bool> EnableStrictCheck(
    "dtrans-typemetadatareader-strict-check", llvm::cl::Hidden,
    llvm::cl::init(true),
    llvm::cl::desc("verify that DTrans "
                   "metadata was collected for all structures"));

namespace llvm {
namespace dtransOP {

bool TypeMetadataReader::hasDTransTypesMetadata(Module &M) {
  return getDTransTypesMetadata(M) != nullptr;
}

NamedMDNode *TypeMetadataReader::getDTransTypesMetadata(Module &M) {
  NamedMDNode *DTMDTypes = M.getNamedMetadata(MDStructTypesTag);
  return DTMDTypes;
}

bool TypeMetadataReader::mapStructsToMDNodes(
    Module &M, MapVector<StructType *, MDNode *> &Mapping, bool IncludeOpaque) {
  NamedMDNode *DTransMD = TypeMetadataReader::getDTransTypesMetadata(M);
  if (!DTransMD)
    return false;

  MapVector<StructType *, MDNode *> OpaqueTyMap;
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

    auto *TyMD = dyn_cast<ConstantAsMetadata>(
        MD->getOperand(DTransStructMDConstants::StructTypeOffset));
    if (!TyMD)
      continue;
    llvm::StructType *StTy = cast<llvm::StructType>(TyMD->getType());

    int32_t FieldCount =
        cast<ConstantInt>(FieldCountMD->getValue())->getSExtValue();
    bool IsOpaqueStructTy = (FieldCount == -1);
    if (IsOpaqueStructTy) {
      // Opaque structure types will get added later, if requested when there is
      // not a non-opaque version available.
      OpaqueTyMap.insert({StTy, MD});
      continue;
    }

    auto Res = Mapping.insert({StTy, MD});
    if (!Res.second && Res.first->second != MD)
      LLVM_DEBUG(dbgs() << "Structure " << *TyMD
                        << " described by more than one metadata node");
  }

  if (IncludeOpaque) {
    // Add any types that are not already in the mapping because only opaque
    // type definitions were seen.
    for (auto &KV : OpaqueTyMap)
      Mapping.insert({KV.first, KV.second});
  }

  return true;
}

MDNode *TypeMetadataReader::getDTransMDNode(const Value &V) {
  if (auto *F = dyn_cast<Function>(&V)) {
    MDNode *MD = F->getMetadata(DTransFuncTypeMDTag);
    return MD;
  }

  if (auto *I = dyn_cast<Instruction>(&V)) {
    MDNode *MD = I->getMetadata(MDDTransTypeTag);
    return MD;
  }

  if (auto *G = dyn_cast<GlobalObject>(&V)) {
    MDNode *MD = G->getMetadata(MDDTransTypeTag);
    return MD;
  }

  return nullptr;
}

bool TypeMetadataReader::initialize(Module &M, bool StrictCheck) {
  NamedMDNode *DTMDTypes = getDTransTypesMetadata(M);
  if (!DTMDTypes)
    return false;

  // Find all the structure types that we expect to create DTransStructTypes
  // for and keep track of whether metadata needs to be found for each type.
  // This will be used for error checking of whether there are types that may be
  // important which were not matched in the metadata.
  enum MetadataStatus {
    MS_RecoveryNeeded,
    MS_RecoveryNotNeeded,
    MS_RecoveryComplete
  };
  DenseMap<llvm::StructType *, MetadataStatus> TypeRecoveryState;
  for (auto *StTy : M.getIdentifiedStructTypes()) {
    // Any structure which contains pointer elements needs to have a
    // metadata description so that DTrans can know what types the fields
    // are expected to be used as.
    bool HasPointer = false;
    for (auto *FieldTy : StTy->elements()) {
      HasPointer = dtrans::hasPointerType(FieldTy);
      if (HasPointer)
        break;
    }

    if (!HasPointer) {
      TypeRecoveryState.insert(std::make_pair(StTy, MS_RecoveryNotNeeded));
    } else {
      TypeRecoveryState.insert(std::make_pair(StTy, MS_RecoveryNeeded));
    }
  }

  // Create all the DTransStructType elements for any structures described in
  // the metadata. This is done without populating the field member types
  // because a field may refer to structures that have not been created yet.
  // For example:
  //   %struct.foo = { %struct.bar* }
  //   %struct.bar = { %struct.foo* }
  // To populate the fields of these structures requires a DTransPointerType
  // object containing a DTransStructType object to exist for the other type. To
  // simplify this, all the structure types are created first.
  //
  // MDToStruct is used to keep track of already visited nodes because when
  // modules are merged together by the IRMover, the named metadata nodes get
  // concatenated to each either, which may produce duplicated entries in the
  // list.
  DenseMap<MDNode *, DTransStructType *> MDToStruct;
  for (auto *MD : DTMDTypes->operands())
    if (!MDToStruct.count(MD)) {
      DTransStructType *DTStTy = constructDTransStructType(MD);
      if (DTStTy)
        MDToStruct.insert(std::make_pair(MD, DTStTy));
    }

  // Create DTransStructType objects for any structures lacking metadata that
  // do not contain pointer types. These types can be created directly from
  // the llvm::StructType object because there are no unknown pointer types
  // within them. Creating the here will allow detection of any metadata that
  // does not agree with the IR on these types.
  DenseMap<llvm::StructType *, DTransStructType *> LLVMToDTransTypeMap;
  for (auto &KV : TypeRecoveryState)
    if (KV.second == MS_RecoveryNotNeeded && KV.first->hasName()) {
      auto *DTy = cast<DTransStructType>(TM.getOrCreateSimpleType(KV.first));
      assert(DTy && "Could not create DTransType for structure");
      LLVMToDTransTypeMap[KV.first] = DTy;
    }

  // All DTrans structure types required should have been created by now, either
  // because the DTrans metadata referred to them, or because it was a simple
  // structure that does not involve pointer types. Populate
  // the ones that did not need metadata. This needs to be done after the type
  // creation to ensure any nested structures have been created before we
  // begin populating. The map 'LLVMToDTransTypeMap' only contains the simple
  // structure types that do not involve pointers, so iterate through all of
  // them.
  for (auto &KV : LLVMToDTransTypeMap) {
    populateDTransStructTypeFromLLVMType(KV.first, KV.second);
    TypeRecoveryState[KV.first] = MS_RecoveryComplete;
  }

  // Process all of the structure types to populate the structure bodies.
  for (auto &MDTypePair : MDToStruct) {
    MDNode *MD = MDTypePair.first;
    DTransStructType *DTStTy = MDTypePair.second;
    // If the type corresponds to an existing LLVM struct type, mark it as
    // recovered. In the current metadata encoding, we expect a type to exist
    // because we are referencing the type within the metadata. If the encoding
    // is switched to just use string names for the types, then the type may no
    // longer exist.
    llvm::StructType *StTy = populateDTransStructType(M, MD, DTStTy);
    if (StTy)
      TypeRecoveryState[StTy] = MS_RecoveryComplete;
  }

  // Check if metadata was available for everything that was needed.
  bool AllRecovered = true;
  bool RecoveryErrors = false;
  for (auto &P : TypeRecoveryState) {
    if (P.second == MS_RecoveryNeeded) {
      LLVM_DEBUG(dbgs() << "Failed to recover type info for struct: "
                        << *P.first << "\n");
      AllRecovered = false;
      continue;
    }

    DTransStructType *DTStTy = TM.getStructType(P.first->getName());
    if (!DTStTy) {
      if ((!StrictCheck || !EnableStrictCheck) &&
          !dtrans::hasOpaquePointerFields(P.first))
        continue;

      RecoveryErrors = true;
      LLVM_DEBUG(dbgs() << "DTransStructType not created for: " << *P.first
                        << "\n");
      continue;
    }

    if (DTStTy->getReconstructError()) {
      LLVM_DEBUG(dbgs() << "Errors with type info for struct: " << *P.first
                        << "\n");
      RecoveryErrors = true;
    }
  }

  // Build a cache of function signature types for the functions with DTrans
  // metadata.
  buildFunctionTypeTable(M);

  return AllRecovered && !RecoveryErrors;
}

// This method creates a DTransStructType that corresponds to the metadata node.
// The metdata field members are not parsed to fill in the body during this
// routine because we want all structures types created first.
DTransStructType *TypeMetadataReader::constructDTransStructType(MDNode *MD) {
  // Parse the metadata to allocate DTransStructType, if the node describes a
  // structure. Should be of the form:
  //   { !"S", struct.type zeroinitializer, i32 NumFields [,!MDRef[,!MDRef]* ] }
  if (MD->getNumOperands() < 3) {
    LLVM_DEBUG(dbgs() << "Incorrect MD encoding for structure description:\n"
                      << *MD << "\n");
    return nullptr;
  }

  if (MD->getOperand(0))
    if (auto *MDS = dyn_cast<MDString>(MD->getOperand(0)))
      if (!MDS->getString().equals("S")) {
        LLVM_DEBUG(dbgs() << "Expected structure descriptor:\n" << *MD << "\n");
        return nullptr;
      }

  auto *FieldCountMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(2));
  assert(FieldCountMD && "Expected metadata constant");
  int32_t FieldCount =
      cast<ConstantInt>(FieldCountMD->getValue())->getSExtValue();

  // With the current metadata encoding format, we expect the structure type to
  // be embedded within the metadata itself. If we need to change the metadata
  // representation to use strings instead, then this will need to change to
  // accommodate the possibility that the type being created does not exist in
  // the Module's list of structure types.
  auto *TyMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(1));
  assert(TyMD && isa<llvm::StructType>(TyMD->getType()) &&
         "Expected struct type");
  llvm::StructType *StTy = cast<llvm::StructType>(TyMD->getType());

  // Check for an opaque type description. If the structure is expected to
  // be an opaque type, then create it now. Otherwise, wait for an actual
  // structure definition to be seen.
  if (FieldCount == -1) {
    if (StTy->isOpaque()) {
      // Get a DTransStructType*, creating it, if necessary.
      auto *DTStTy = DTransStructType::get(TM, StTy);
      cacheMDDecoding(MD, DTStTy);

      LLVM_DEBUG(dbgs() << DEBUG_TYPE << ": Created structure: "
                        << DTStTy->getName() << "\n");

      return DTStTy;
    }

    return nullptr;
  }

  // Need to convert to unsigned type to avoid sign comparison errors.
  assert(FieldCount >= 0 && "Must have positive field count if not opaque");
  assert(FieldCount < std::numeric_limits<int32_t>::max() &&
         "Field count out of range");
  unsigned FieldCountU = FieldCount;

  // Check if a structure has already been created for the type.
  StringRef StructName = StTy->getName();
  auto *ExistingType = TM.getStructType(StructName);
  if (ExistingType) {
    if (FieldCountU != ExistingType->getNumFields()) {
      LLVM_DEBUG({
        dbgs() << DEBUG_TYPE << ": Conflicting structure: ";
        dbgs() << "Current definition: " << *ExistingType << "\n";
        dbgs() << "New               : " << *MD << "\n";
      });

      ExistingType->setReconstructError();
      // Add additional uninitialized fields to the structure to be resolved
      // when parsing the field representation of the metadata.
      if (FieldCountU > ExistingType->getNumFields())
        ExistingType->resizeFieldCount(FieldCountU);
    }

    cacheMDDecoding(MD, ExistingType);
    return ExistingType;
  }

  // Create a new DTransStructType that will be used to represent the structure.
  DTransStructType *DTStTy = DTransStructType::get(TM, StTy);
  cacheMDDecoding(MD, DTStTy);

  // The number of fields described in the metadata should match the number of
  // fields in the llvm::StructType.
  if (FieldCountU != StTy->getNumElements()) {
    DTStTy->setReconstructError();

    LLVM_DEBUG({
      dbgs() << DEBUG_TYPE << ": Conflicting structure: ";
      dbgs() << "IR Structure:       " << *StTy << "\n";
      dbgs() << "Metadata structure: " << *MD << "\n";
    });

    if (FieldCountU > DTStTy->getNumFields())
      DTStTy->resizeFieldCount(FieldCountU);
  }

  LLVM_DEBUG(dbgs() << DEBUG_TYPE
                    << ": Created structure: " << DTStTy->getName() << "\n");

  return DTStTy;
}

// Populate the body of the \p DTStTy based on the metadata. If a body is
// populated and the type corresponds to a llvm::StructType, return it.
llvm::StructType *
TypeMetadataReader::populateDTransStructType(Module &M, MDNode *MD,
                                             DTransStructType *DTStTy) {
  // Metadata is of the form:
  //   { !"S", struct.type zeroinitializer, i32 NumFields [,!MDRef[,!MDRef]* ] }
  const unsigned FieldTyStartPos = DTransStructMDConstants::FieldNodeOffset;

  if (MD->getNumOperands() < DTransStructMDConstants::MinOperandCount) {
    LLVM_DEBUG(dbgs() << "Metadata node for structure is incomplete: "
                      << *DTStTy << ". MDNode = " << *MD << "\n");
    return nullptr;
  }

  unsigned RecTypeOffset = DTransStructMDConstants::RecTypeOffset;
  if (!isa<MDString>(MD->getOperand(RecTypeOffset))) {
    LLVM_DEBUG(dbgs() << "Metadata encoding incorrect for structure: "
                      << *DTStTy << ". MDNode = " << *MD << "\n");
    return nullptr;
  }

  auto RecType = cast<MDString>(MD->getOperand(RecTypeOffset));
  if (!RecType->getString().equals("S")) {
    LLVM_DEBUG(dbgs() << "Metadata encoding incorrect for structure: "
                      << *DTStTy << ". MDNode = " << *MD << "\n");
    return nullptr;
  }

  unsigned FieldCountOffset = DTransStructMDConstants::FieldCountOffset;
  auto *FieldCountMD =
      dyn_cast<ConstantAsMetadata>(MD->getOperand(FieldCountOffset));
  assert(FieldCountMD && "Expected metadata constant");
  int32_t FieldCount =
      cast<ConstantInt>(FieldCountMD->getValue())->getSExtValue();

  // Nothing to do for opaque structure descriptions.
  if (FieldCount < 0)
    return nullptr;

  unsigned FieldCountU = static_cast<unsigned>(FieldCount);
  if (FieldCountU > DTStTy->getNumFields() ||
      FieldCountU != MD->getNumOperands() - FieldTyStartPos) {
    DTStTy->setReconstructError();
    LLVM_DEBUG({
      dbgs() << "Incorrect field count for structure:\n" << *MD << "\n";
      DTStTy->dump();
    });
    return nullptr;
  }

  llvm::StructType *StTy = cast<llvm::StructType>(DTStTy->getLLVMType());
  assert(StTy && "DTransStructType incorrectly initialized during "
                 "constructDTransStructType");

  // Check that the metadata information matches with the structure definition.
  if (FieldCountU != StTy->getNumElements()) {
    LLVM_DEBUG(dbgs() << "Incorrect field count for structure: " << *StTy
                      << " - " << *MD << "\n ";);
    DTStTy->setReconstructError();
    return nullptr;
  }

  unsigned FieldNum = 0;
  unsigned NumOps = MD->getNumOperands();
  for (unsigned Idx = FieldTyStartPos; Idx < NumOps; ++Idx, ++FieldNum) {
    auto *FieldMD = dyn_cast<MDNode>(MD->getOperand(Idx));
    if (!FieldMD) {
      LLVM_DEBUG(dbgs() << "Metadata encoding incorrect for structure: "
                        << *DTStTy << ". MDNode = " << *MD << "\n");
      return nullptr;
    }

    DTransType *DTFieldTy = decodeMDNode(FieldMD);
    if (!DTFieldTy) {
      DTStTy->setReconstructError();
      LLVM_DEBUG(dbgs() << *DTStTy << " :Error decoding field: " << FieldNum
                        << "\n");
      continue;
    }

    llvm::Type *IRFieldType = StTy->getElementType(FieldNum);
    if (!validateMDFieldType(DTFieldTy, IRFieldType)) {
      DTStTy->setReconstructError();
      LLVM_DEBUG(dbgs() << "Mismatch occurred:\n  Expected: " << *IRFieldType
                        << "\n  Found: " << *DTFieldTy << "\n");
      continue;
    }

    DTransFieldMember &Field = DTStTy->getField(FieldNum);
    DTransType *ExistingType = Field.getType();
    Field.addResolvedType(DTFieldTy);

    if (ExistingType && ExistingType != DTFieldTy) {
      DTStTy->setReconstructError();
      LLVM_DEBUG(
          dbgs() << "Conflicting types for field from metadata:\n  Expected: "
                 << *ExistingType << "\n  Found: " << *DTFieldTy << "\n");
    }
  }

  LLVM_DEBUG({
    dbgs() << "Reconstructed structure: ";
    DTStTy->dump();
    dbgs() << "\n";
    if (StTy) {
      dbgs() << " IR structure type: ";
      StTy->dump();
    }
  });

  return StTy;
}

// Populate the body of \p DSTy based on the members of STy. All members of STy
// must be simple types. i.e. none of the members contain pointer references.
void TypeMetadataReader::populateDTransStructTypeFromLLVMType(
    llvm::StructType *STy, DTransStructType *DSTy) {
  for (auto &Elem : enumerate(STy->elements())) {
    DTransType *DTFieldTy = TM.getOrCreateSimpleType(Elem.value());
    assert(DTFieldTy && "Expected structure fields to be simple types");
    DTransFieldMember &Field = DSTy->getField(Elem.index());
    Field.addResolvedType(DTFieldTy);
  }
}

// This method is used when populating structure bodies based on the DTrans
// metadata to check whether the field type in the metadata matches the expected
// type of the field in the LLVM structure type. Return 'false' if the
// DTransType 'DTy' from the metadata is not compatible with the LLVM type.
bool TypeMetadataReader::validateMDFieldType(DTransType *DTy, llvm::Type *LTy) {
  assert(DTy && LTy && "Invalid parameters to validateMDFieldType");
  switch (DTy->getTypeID()) {
  case DTransType::DTransAtomicTypeID:
    if (DTy->getLLVMType() != LTy)
      return false;
    break;
  case DTransType::DTransPointerTypeID:
    if (!LTy->isPointerTy())
      return false;
    break;
  case DTransType::DTransStructTypeID: {
    auto *LStructTy = dyn_cast<llvm::StructType>(LTy);
    if (!LStructTy)
      return false;
    if (LStructTy->hasName()) {
      auto *DStructTy = cast<DTransStructType>(DTy);
      if (LStructTy->getName().compare(DStructTy->getName()) != 0)
        return false;
    }
    // TODO: Assume literal types are equal. This can be expanded to check
    // each field type later.
    break;
  }
  case DTransType::DTransArrayTypeID: {
    auto *LArrTy = dyn_cast<llvm::ArrayType>(LTy);
    if (!LArrTy)
      return false;
    auto *DArrTy = cast<DTransArrayType>(DTy);
    if (LArrTy->getArrayNumElements() != DArrTy->getNumElements())
      return false;
    return validateMDFieldType(DArrTy->getArrayElementType(),
                               LArrTy->getArrayElementType());
  }
  case DTransType::DTransVectorTypeID: {
    auto *LVecTy = dyn_cast<llvm::VectorType>(LTy);
    if (!LVecTy)
      return false;
    auto *DVecTy = cast<DTransVectorType>(DTy);
    if (LVecTy->getElementCount().getKnownMinValue() !=
        DVecTy->getNumElements())
      return false;
    return validateMDFieldType(DVecTy->getElementType(),
                               LVecTy->getElementType());
    break;
  }
  case DTransType::DTransFunctionTypeID:
    auto *LFnTy = dyn_cast<llvm::FunctionType>(LTy);
    if (!LFnTy)
      return false;

    // TODO: This could be extended to do additional checking to verify the
    // types within the function signature match.
    break;
  }

  return true;
}

// This method is the publicly visible method that will check whether a Value
// has DTransType metadata, and returns it if available.
// Otherwise, returns nullptr.
DTransType *TypeMetadataReader::getDTransTypeFromMD(const Value *V) {
  if (auto *F = dyn_cast<Function>(V)) {
    // Functions that had metadata were decoded during the initialize() method,
    // and the results stored in a table.
    DTransFunctionType *Ty = getDTransType(F);
    if (Ty)
      return Ty;
    // Try to get type of newly created functions from MD for now
    // since newly created functions are not in the table.
    auto *MDTypeListNode = F->getMetadata(DTransFuncTypeMDTag);
    if (!MDTypeListNode)
      return nullptr;
    return decodeDTransFuncType(*(const_cast<Function *>(F)), *MDTypeListNode);
  }

  MDNode *MD = getDTransMDNode(*V);
  if (MD)
    return decodeMDNode(MD);

  return nullptr;
}

// This method returns a DTransType* by decoding the information the metadata
// node.
DTransType *TypeMetadataReader::decodeMDNode(MDNode *MD) {
  assert(MD && "Expected metadata constant");

  // Return previously decoded item, if available.
  auto It = MDToDTransTypeMap.find(MD);
  if (It != MDToDTransTypeMap.end())
    return It->second;

  assert(MD->getOperand(0) && "Unexpected empty node");

  if (auto *MDS = dyn_cast<MDString>(MD->getOperand(0))) {
    StringRef Tag = MDS->getString();
    if (Tag.equals("S"))
      // Structures should have been decoded before calling this routine.
      llvm_unreachable(
          "MDToDTransTypeMap should have decoded information cached.");

    if (Tag.equals("F"))
      return decodeMDFunctionNode(MD);
    else if (Tag.equals("void"))
      return decodeMDVoidNode(MD);
    else if (Tag.equals("A"))
      return decodeMDArrayNode(MD);
    else if (Tag.equals("V"))
      return decodeMDVectorNode(MD);
    else if (Tag.equals("L"))
      return decodeMDLiteralStructNode(MD);
    else if (Tag.equals("metadata"))
      return TM.getOrCreateAtomicType(Type::getMetadataTy(MD->getContext()));
  }

  // If the first field of the metadata node is a metadata node
  // then we are dealing with a pointer to a another metadata element type.
  //   !{!MDNode, i32 <pointer level> }
  if (auto *RefMD = dyn_cast<MDNode>(MD->getOperand(0))) {
    DTransType *RefTy = decodeMDNode(RefMD);
    if (!RefTy) {
      LLVM_DEBUG(dbgs() << "Decoding reference type failed: " << *MD << "\n");
      return nullptr;
    }
    auto *PtrLevelMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(1));
    assert(PtrLevelMD && "Expected metadata constant");
    unsigned PtrLevel =
        cast<ConstantInt>(PtrLevelMD->getValue())->getZExtValue();
    DTransType *Result = createPointerToLevel(RefTy, PtrLevel);
    cacheMDDecoding(MD, Result);
    return Result;
  }

  // If we get here, then we are dealing with a first class type, or pointer to
  // first class type.
  //   !{<type> zeroinitializer, i32 <pointer level> }
  auto *TyMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(0));
  assert(TyMD && "Expected constant type");
  llvm::Type *Ty = TyMD->getType();
  if (Ty->isStructTy())
    return decodeMDStructRefNode(MD);

  if (Ty->isPointerTy()) {
    // Abort compilation on assertion enabled builds.
    llvm_unreachable("Pointer type is not allowed in DTrans metadata");
    return nullptr;
  }

  auto *PtrLevelMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(1));
  if (!PtrLevelMD || !isa<ConstantInt>(PtrLevelMD->getValue())) {
    llvm_unreachable("Expected metadata constant");
    return nullptr;
  }
  unsigned PtrLevel = cast<ConstantInt>(PtrLevelMD->getValue())->getZExtValue();
  auto *SimpleType = DTransAtomicType::get(TM, Ty);
  if (!SimpleType) {
    llvm_unreachable("Type was not first class type");
    return nullptr;
  }

  DTransType *Result = createPointerToLevel(SimpleType, PtrLevel);
  cacheMDDecoding(MD, Result);
  return Result;
}

// Decode a metadata description for a function signature.
// Metadata is of the form:
//    !{!"F", i1 <isVarArg>, i32 <numParam>, !MDNode_RetTy,
//            !MDNode_Param1Ty, ... !MDNode_ParamNTy }
//
DTransType *TypeMetadataReader::decodeMDFunctionNode(MDNode *MD) {
  const unsigned IsVarArgPos = 1;
  const unsigned NumArgsPos = 2;
  const unsigned RetTyPos = 3;
  const unsigned ArgTyStartPos = 4;

  if (MD->getNumOperands() < ArgTyStartPos) {
    LLVM_DEBUG(dbgs() << "Incorrect operand count for function encoding:" << *MD
                      << "\n");
    return nullptr;
  }

  auto *IsVarArgsMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(IsVarArgPos));
  assert(IsVarArgsMD && "Expected metadata constant");
  auto *NumArgsMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(NumArgsPos));
  assert(NumArgsMD && "Expected metadata constant");

  bool IsVarArg = cast<ConstantInt>(IsVarArgsMD->getValue())->getZExtValue();
  unsigned ArgCount = cast<ConstantInt>(NumArgsMD->getValue())->getZExtValue();

  unsigned NumOps = MD->getNumOperands();
  if (NumOps != ArgTyStartPos + ArgCount) {
    LLVM_DEBUG(dbgs() << "Incorrect operand count for function encoding:" << *MD
                      << "\n");
    return nullptr;
  }

  auto *RetTyMD = dyn_cast<MDNode>(MD->getOperand(RetTyPos));
  assert(RetTyMD && "Expected metadata constant");
  DTransType *RetTy = decodeMDNode(RetTyMD);
  if (!RetTy) {
    LLVM_DEBUG(dbgs() << "Failed to decode return type of: " << *MD << "\n");
    return nullptr;
  }

  SmallVector<DTransType *, 8> ParamTypes;
  for (unsigned OpIdx = ArgTyStartPos; OpIdx < NumOps; ++OpIdx) {
    auto *ArgTyMD = dyn_cast<MDNode>(MD->getOperand(OpIdx));
    assert(ArgTyMD && "Expected metadata constant");
    DTransType *ArgTy = decodeMDNode(ArgTyMD);
    if (!ArgTy) {
      LLVM_DEBUG(dbgs() << "Failed to decode argument type of: " << *MD
                        << ": MDOperand( " << OpIdx << ")\n");
      return nullptr;
    }
    ParamTypes.push_back(ArgTy);
  }
  DTransFunctionType *FnTy =
      DTransFunctionType::get(TM, RetTy, ParamTypes, IsVarArg);
  cacheMDDecoding(MD, FnTy);
  return FnTy;
}

// Decode a metadata description for a 'void' type.
// Metadata is of the form:
//     !{!"void", i32 <pointer level> }
//
DTransType *TypeMetadataReader::decodeMDVoidNode(MDNode *MD) {
  if (MD->getNumOperands() != 2) {
    LLVM_DEBUG(dbgs() << "Incorrect operand count for void type encoding:"
                      << *MD << "\n");
    return nullptr;
  }
  DTransType *DTVoidTy =
      DTransAtomicType::get(TM, llvm::Type::getVoidTy(MD->getContext()));

  auto *PtrLevelMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(1));
  assert(PtrLevelMD && "Expected metadata constant");
  unsigned PtrLevel = cast<ConstantInt>(PtrLevelMD->getValue())->getZExtValue();

  DTransType *Result = createPointerToLevel(DTVoidTy, PtrLevel);
  cacheMDDecoding(MD, Result);
  return Result;
}

// Decode a metadata description for a literal struct type.
// Metadata is of the form:
//     !{!"L", i32 <numElem>, !MDNodefield1, !MDNodefield2, ...}
//
DTransType *TypeMetadataReader::decodeMDLiteralStructNode(MDNode *MD) {
  if (MD->getNumOperands() < 2) {
    LLVM_DEBUG(
        dbgs() << "Incorrect operand count for literal struct type encoding:"
               << *MD << "\n");
    return nullptr;
  }

  const unsigned FieldTyStartPos = 2;
  auto *NumElemMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(1));
  assert(NumElemMD && "Expected metadata constant");
  unsigned NumElem = cast<ConstantInt>(NumElemMD->getValue())->getZExtValue();

  unsigned NumOps = MD->getNumOperands();
  if (NumOps != FieldTyStartPos + NumElem) {
    LLVM_DEBUG(
        dbgs() << "Incorrect operand count for literal struct type encoding:"
               << *MD << "\n");
    return nullptr;
  }

  SmallVector<DTransType *, 4> FieldTypes;
  for (unsigned OpIdx = FieldTyStartPos; OpIdx < NumOps; ++OpIdx) {
    auto *FieldMD = dyn_cast<MDNode>(MD->getOperand(OpIdx));
    assert(FieldMD && "Expected metadata constant");
    auto *DTFieldTy = decodeMDNode(FieldMD);
    FieldTypes.push_back(DTFieldTy);
  }
  DTransType *DTStTy =
      TM.getOrCreateLiteralStructType(MD->getContext(), FieldTypes);
  cacheMDDecoding(MD, DTStTy);
  return DTStTy;
}

// Decode a metadata description for an array type.
// Metadata is of the form:
//   !{!"A", i32 <numElem>, !MDNode }
//
DTransType *TypeMetadataReader::decodeMDArrayNode(MDNode *MD) {
  if (MD->getNumOperands() < 3) {
    LLVM_DEBUG(dbgs() << "Incorrect operand count for array type encoding:"
                      << *MD << "\n");
    return nullptr;
  }

  auto *NumElemMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(1));
  assert(NumElemMD && "Expected metadata constant");

  auto *RefMD = dyn_cast<MDNode>(MD->getOperand(2));
  assert(RefMD && "Expected metadata constant");

  auto *ElemType = decodeMDNode(RefMD);
  if (!ElemType)
    return nullptr;

  unsigned NumElem = cast<ConstantInt>(NumElemMD->getValue())->getZExtValue();
  auto Result = DTransArrayType::get(TM, ElemType, NumElem);
  cacheMDDecoding(MD, Result);
  return Result;
}

// Decode a metadata description for a vector type.
// Metadata is of the form:
//   !{!"V", i32 <numElem>, !MDNode }
//
DTransType *TypeMetadataReader::decodeMDVectorNode(MDNode *MD) {
  if (MD->getNumOperands() < 3) {
    LLVM_DEBUG(dbgs() << "Incorrect operand count for vector type encoding:"
                      << *MD << "\n");
    return nullptr;
  }

  auto *NumElemMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(1));
  assert(NumElemMD && "Expected metadata constant");

  auto *RefMD = dyn_cast<MDNode>(MD->getOperand(2));
  assert(RefMD && "Expected metadata constant");

  auto *ElemType = decodeMDNode(RefMD);
  if (!ElemType)
    return nullptr;

  unsigned NumElem = cast<ConstantInt>(NumElemMD->getValue())->getZExtValue();
  auto Result = DTransVectorType::get(TM, ElemType, NumElem);
  cacheMDDecoding(MD, Result);
  return Result;
}

// Decode a metadata description for a structure reference node.
// Metadata is of the form:
//     !{<type> zeroinitializer, i32 <pointer level> }
//
DTransType *TypeMetadataReader::decodeMDStructRefNode(MDNode *MD) {
  if (MD->getNumOperands() < 2) {
    LLVM_DEBUG(dbgs() << "Incorrect operand count for reference type encoding:"
                      << *MD << "\n");
    return nullptr;
  }

  int TypeIndex = 0;
  int PtrLevelIndex = 1;
  llvm::StructType *StTy = nullptr;
  auto *TyMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(TypeIndex));
  assert(TyMD && "Expected type");
  StTy = cast<StructType>(TyMD->getType());

  assert(StTy && "Expected structure type");

  // The structure type should already be created, so do a lookup
  // for the existing type. Do not try to create a type for a type
  // that doesn't exist yet. If it does not exist, then the metadata
  // is invalid.
  DTransType *DTStTy = TM.getStructType(StTy->getName());
  if (!DTStTy) {
    LLVM_DEBUG(
        dbgs() << "Incorrect reference type encoding. Type does not exist:"
               << *MD << "\n");
    return nullptr;
  }

  auto *PtrLevelMD =
      dyn_cast<ConstantAsMetadata>(MD->getOperand(PtrLevelIndex));
  assert(PtrLevelMD && "Expected metadata constant");
  unsigned PtrLevel = cast<ConstantInt>(PtrLevelMD->getValue())->getZExtValue();

  DTransType *Result = createPointerToLevel(DTStTy, PtrLevel);
  cacheMDDecoding(MD, Result);
  return Result;
}

void TypeMetadataReader::buildFunctionTypeTable(Module &M) {
  // Walk all the functions and build up a table of function signatures based on
  // the metadata.
  for (auto &F : M) {
    MDNode *MDTypeListNode = F.getMetadata(DTransFuncTypeMDTag);
    if (!MDTypeListNode)
      continue;

    DTransFunctionType *DTransFuncTy = decodeDTransFuncType(F, *MDTypeListNode);
    FunctionToDTransTypeMap[&F] = DTransFuncTy;
  }
}

// Decode the metadata to produce the signature for a Function
// definition/declaration.
DTransFunctionType *
TypeMetadataReader::decodeDTransFuncType(Function &F,
                                         const MDNode &MDTypeListNode) {
  // Any parameters that have lost their metadata tags will get i8* types
  // substituted in the signature table.
  llvm::Type *LLVMI8Type = llvm::Type::getInt8Ty(F.getContext());
  DTransType *DTransI8PtrType =
      TM.getOrCreatePointerType(TM.getOrCreateAtomicType(LLVMI8Type));

  // There are 2 components that make up the type descriptor for function
  // parameter/return types involving pointers.
  //   1. An attribute, "intel_dtrans_func_index" which contains an operand
  //   index to use from a metadata node attached to the function,
  //   !intel.dtrans.func.type, for the type. Index values start with 1.
  //
  //   2. A metadata node, !intel.dtrans.func.type, that contains a list of
  //   metadata nodes for the encoded types.
  //
  // For example:
  //
  // ; %struct.op* bitop(%struct.op*, i32, i32*)
  // define intel_dtrans_func_index(1) p0 @bitop(
  //     p0 intel_dtrans_func_index(2) %0, i32 %1
  //     p0 intel_dtrans_func_index(3) %2)
  //     !intel.dtrans.func.type !3
  //
  // !1 = !{%struct.op zeroinitializer, i32 1} ; %struct.op*
  // !2 = !{i32 0, i32 1}                      ; i32*
  // !3 = distinct !{!1, !1, !2}               ; list of type encodings.

  // Extract the index value from the intel_dtrans_func_index attribute. Return
  // 0, if the value attribute is not present.
  auto GetMetadataIndex = [](AttributeSet &Attrs) -> uint64_t {
    Attribute Attr;
    Attr = Attrs.getAttribute(DTransFuncIndexTag);
    if (Attr.isValid()) {
      StringRef TagName = Attr.getValueAsString();
      uint64_t Index = stoi(TagName.str());
      assert(Index >= 1 && "Expected 1 based indexing");
      return Index;
    }

    return 0;
  };

  // Walk the items of the list to form a mapping of the indices used by the
  // attribute tags of the current function to their MDNode.
  DenseMap<unsigned, DTransType *> IndexToType;

  unsigned Count = MDTypeListNode.getNumOperands();
  for (unsigned Idx = 0; Idx < Count; ++Idx) {
    auto *TypeNode = dyn_cast<MDNode>(MDTypeListNode.getOperand(Idx));
    assert(TypeNode && "Expected metadata constant");
    DTransType *Ty = decodeMDNode(TypeNode);
    if (!Ty)
      Ty = DTransI8PtrType;
    // Add 1 to the index because the values to look up from the attribute are
    // from 1 to n.
    IndexToType[Idx + 1] = Ty;
  }

  // Match the value used in an Attribute to the mapping of MDNodes.
  AttributeList Attrs = F.getAttributes();
  llvm::FunctionType *FuncTy = cast<llvm::FunctionType>(F.getValueType());
  llvm::Type *RetTy = FuncTy->getReturnType();
  DTransType *DTransRetTy = nullptr;
  if (!dtrans::hasPointerType(RetTy)) {
    DTransRetTy = TM.getOrCreateSimpleType(RetTy);
  } else {
    AttributeSet RetAttrs = Attrs.getRetAttrs();
    uint64_t Index = GetMetadataIndex(RetAttrs);
    if (Index) {
      auto It = IndexToType.find(Index);
      if (It != IndexToType.end())
        DTransRetTy = It->second;
      else
        LLVM_DEBUG(dbgs() << DEBUG_TYPE
                          << ": Warning: Invalid metadata index on: "
                          << F.getName() << "\n");
    }
  }

  if (!DTransRetTy) {
    DTransRetTy = DTransI8PtrType;
    LLVM_DEBUG(
        dbgs() << DEBUG_TYPE
               << ": Warning: Function type recovery missing return type for: "
               << F.getName() << "\n");
  }

  SmallVector<DTransType *, 8> ParamTypes;
  unsigned ArgCount = F.arg_size();
  for (unsigned Idx = 0; Idx < ArgCount; ++Idx) {
    llvm::Type *ParamTy = FuncTy->getParamType(Idx);
    DTransType *DTransParamTy = nullptr;
    if (!dtrans::hasPointerType(ParamTy)) {
      DTransParamTy = TM.getOrCreateSimpleType(ParamTy);
    } else {
      AttributeSet ParamAttrs = Attrs.getParamAttrs(Idx);
      uint64_t Index = GetMetadataIndex(ParamAttrs);
      if (Index) {
        auto It = IndexToType.find(Index);
        if (It != IndexToType.end())
          DTransParamTy = It->second;
        else
          LLVM_DEBUG(dbgs()
                     << DEBUG_TYPE << ": Warning: Invalid metadata index on: "
                     << F.getName() << "\n");
      }
    }
    if (!DTransParamTy) {
      DTransParamTy = DTransI8PtrType;
      LLVM_DEBUG(
          dbgs()
          << DEBUG_TYPE
          << ": Warning: Function type recovery missing parameter type for: "
          << F.getName() << "@" << Idx << "\n");
    }

    ParamTypes.push_back(DTransParamTy);
  }

  DTransFunctionType *DTransFuncTy =
      DTransFunctionType::get(TM, DTransRetTy, ParamTypes, F.isVarArg());
  return DTransFuncTy;
}

DTransFunctionType *TypeMetadataReader::getDTransType(const Function *F) const {
  auto It = FunctionToDTransTypeMap.find(F);
  if (It != FunctionToDTransTypeMap.end())
    return It->second;
  return nullptr;
}

// Helper to create a possible pointer type, or pointer-to-pointer, etc to the
// \p DTTy type.
// \p PtrLevel controls the number of levels of indirection the pointer has.
DTransType *TypeMetadataReader::createPointerToLevel(DTransType *DTTy,
                                                     unsigned PtrLevel) {
  while (PtrLevel--) {
    DTTy = DTransPointerType::get(TM, DTTy);
  }
  return DTTy;
}

// Add an mapping between the \p MD node and the \p DTTy to the cache so that
// subsequent requests will not need to decode the item again.
void TypeMetadataReader::cacheMDDecoding(MDNode *MD, DTransType *DTTy) {
  MDToDTransTypeMap.insert(std::make_pair(MD, DTTy));
}

} // end namespace dtransOP
} // end namespace llvm

#if !INTEL_PRODUCT_RELEASE

namespace llvm {

static cl::opt<bool> ReportDecodedValues(
    "dtrans-typemetadatareader-values", cl::ReallyHidden, cl::init(false),
    cl::desc("Report types decoded by type metadata reader test pass"));

static cl::opt<bool> ReportMissing(
    "dtrans-typemetadatareader-missing", cl::ReallyHidden, cl::init(true),
    cl::desc("Report missing metadata encountered by the reader test pass"));

static cl::opt<bool> ReportErrors(
    "dtrans-typemetadatareader-errors", cl::ReallyHidden, cl::init(true),
    cl::desc("Report decoding errors and mismatches between the metadata "
             "and the actual type. Mismatch reporting only supported "
             "when using typed-pointers"));

namespace dtransOP {

class TypeMetadataTester {
private:
  DTransTypeManager TM;
  TypeMetadataReader Reader;

public:
  TypeMetadataTester(LLVMContext &Ctx) : TM(Ctx), Reader(TM) {}
  void runTest(Module &M) {
    bool AllResolved = Reader.initialize(M);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (ReportDecodedValues)
      TM.printTypes();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    dbgs() << DEBUG_TYPE << ": All structures types "
           << (AllResolved ? "" : "NOT ") << "populated\n";

    bool Ok = checkModule(M);
    dbgs() << DEBUG_TYPE << ": " << (Ok ? "NO " : "")
           << "Errors while checking IR annotations\n";
  }

  // Check the module for missing metadata on items that are expected to have
  // metadata:
  // - Types of global variables that involve pointer types
  // - Function signatures that involve pointer types
  // - Values produced by 'alloca' instructions that involve pointer types
  // Return 'true' if no errors are found.
  bool checkModule(Module &M) {
    LLVM_DEBUG(dbgs() << DEBUG_TYPE
                      << ": Checking module for DTrans metadata\n");
    bool ErrorsFound = false;

    // Determine whether the IR is using opaque pointers or not. When opaque
    // pointers are in use, all pointers of an address space should be
    // equivalent. Until opaque pointers become enabled, it's possible to check
    // whether the metadata information matches the pointer type in the IR.
    LLVMContext &Ctx = M.getContext();
    bool OpaquePointersEnabled = !Ctx.supportsTypedPointers();

    // Check whether all the global variables that are expected to have metadata
    // have it.
    for (auto &GV : M.globals()) {
      llvm::Type *GVType = GV.getValueType();
      if (dtrans::hasPointerType(GVType)) {
        MDNode *MD = Reader.getDTransMDNode(GV);
        if (!MD) {
          ErrorsFound = true;
          if (ReportMissing)
            dbgs() << "ERROR: Missing var type metadata: " << GV << "\n";
          continue;
        }

        DTransType *DType = Reader.decodeMDNode(MD);
        if (!DType) {
          ErrorsFound = true;
          if (ReportErrors)
            dbgs() << "ERROR: Failed to decode var type metadata: " << GV
                   << " - " << *MD << "\n";
        } else {
          if (ReportDecodedValues)
            dbgs() << "Decoded var type metadata: " << GV << " - " << *DType
                   << "\n";

          if (!OpaquePointersEnabled && GVType != DType->getLLVMType()) {
            ErrorsFound = true;
            if (ReportErrors)
              dbgs() << "ERROR: Metadata type does not match expected type: "
                     << GV.getName() << "\n   IR: " << *GV.getValueType()
                     << "\n   MD: " << *DType << "\n";
          }
        }
      }
    }

    for (auto &F : M) {
      // Intrinsics do not currently get metadata info from the FE.
      if (F.isIntrinsic())
        continue;

      // We need to invert the result of check function, because a result of
      // 'false' means there was an error found.
      ErrorsFound |= !checkFunction(F, OpaquePointersEnabled);
    }

    return !ErrorsFound;
  }

  // Check the function signature and instructions within the body for
  // DTrans metadata. Return 'true' if no errors are found.
  bool checkFunction(Function &F, bool OpaquePointersEnabled) {
    LLVM_DEBUG(dbgs() << DEBUG_TYPE
                      << ": Checking function for DTrans metadata: "
                      << F.getName() << "\n");
    bool ErrorsFound = false;

    llvm::Type *FnType = F.getValueType();
    if (dtrans::hasPointerType(FnType)) {
      DTransType *DType = Reader.getDTransType(&F);
      if (DType) {
        if (ReportDecodedValues)
          dbgs() << "Decoded fn type metadata: " << F.getName() << " : "
                 << *DType << "\n";
        if (!OpaquePointersEnabled && DType->getLLVMType() != FnType) {
          ErrorsFound = true;
          if (ReportErrors)
            dbgs() << "ERROR: Metadata type does not match expected type: "
                   << F.getName() << "\n   IR: " << *FnType
                   << "\n   MD: " << *DType << "\n";
        }
      } else {
        ErrorsFound = true;
        if (ReportMissing)
          dbgs() << "ERROR: Missing fn type metadata for: " << F.getName()
                 << "\n";
      }
    }

    for (auto &I : instructions(F)) {
      if (auto *AI = dyn_cast<AllocaInst>(&I)) {
        llvm::Type *AllocType = AI->getAllocatedType();
        if (dtrans::hasPointerType(AllocType)) {
          MDNode *MD = Reader.getDTransMDNode(*AI);
          if (!MD) {
            ErrorsFound = true;
            if (ReportMissing)
              dbgs() << "ERROR: Missing metadata: " << F.getName() << " : "
                     << *AI << "\n";
            continue;
          }

          DTransType *DType = Reader.decodeMDNode(MD);
          if (!DType) {
            ErrorsFound = true;
            if (ReportErrors)
              dbgs() << "ERROR: Failed to decode metadata: " << F.getName()
                     << " : " << *AI << " - " << *MD << "\n";
          } else {
            if (ReportDecodedValues)
              dbgs() << "Decoded alloca metadata : " << F.getName() << " : "
                     << *AI << " - " << *DType << "\n";

            if (!OpaquePointersEnabled && DType->getLLVMType() != AllocType) {
              ErrorsFound = true;
              if (ReportErrors)
                dbgs() << F.getName()
                       << "ERROR: Metadata type does not match expected type: "
                       << F.getName() << " : " << *AI
                       << "\n   IR: " << *AI->getAllocatedType()
                       << "\n   MD: " << *DType << "\n";
            }
          }
        }
      } else if (auto *Call = dyn_cast<CallBase>(&I)) {
        if (Call->isIndirectCall() &&
            dtrans::hasPointerType(Call->getFunctionType())) {
          MDNode *MD = Reader.getDTransMDNode(*Call);
          if (!MD) {
            ErrorsFound = true;
            if (ReportMissing)
              dbgs() << "ERROR: Missing metadata: " << F.getName() << " : "
                     << *Call << "\n";
            continue;
          }

          DTransType *DType = Reader.decodeMDNode(MD);
          if (!DType) {
            ErrorsFound = true;
            if (ReportErrors)
              dbgs() << "ERROR: Failed to decode metadata: " << F.getName()
                     << " : " << *Call << " - " << *MD << "\n";
          } else {
            if (ReportDecodedValues)
              dbgs() << "Decoded call metadata   : " << F.getName() << " : "
                     << *Call << " - " << *DType << "\n";

            if (!OpaquePointersEnabled &&
                DType->getLLVMType() != Call->getFunctionType()) {
              if (ReportErrors)
                dbgs() << "ERROR:  Metadata type does not match expected type: "
                       << F.getName() << " : " << *Call
                       << "\n   IR: " << *Call->getFunctionType()
                       << "\n   MD: " << *DType << "\n";
            }
          }
        }
      }
    }

    return !ErrorsFound;
  }
};
} // end namespace dtransOP
} // end namespace llvm

using namespace llvm;

// This pass is just for testing the TypeMetadataReader class. This pass will
// not be part of the compiler pipeline.
class DTransTypeMetadataReaderTestWrapper : public ModulePass {

public:
  static char ID;

  DTransTypeMetadataReaderTestWrapper() : ModulePass(ID) {
    initializeDTransTypeMetadataReaderTestWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    dtransOP::TypeMetadataTester Tester(M.getContext());
    Tester.runTest(M);
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

char DTransTypeMetadataReaderTestWrapper::ID = 0;
INITIALIZE_PASS(DTransTypeMetadataReaderTestWrapper,
                "dtrans-typemetadatareader",
                "DTrans type metadata reader tester", false, false)

ModulePass *llvm::createDTransMetadataReaderTestWrapperPass() {
  return new DTransTypeMetadataReaderTestWrapper();
}

namespace llvm {
namespace dtransOP {

PreservedAnalyses
DTransTypeMetadataReaderTestPass::run(Module &M, ModuleAnalysisManager &MAM) {
  TypeMetadataTester Tester(M.getContext());
  Tester.runTest(M);
  return PreservedAnalyses::all();
}
} // end namespace dtransOP
} // end namespace llvm

#endif // !INTEL_PRODUCT_RELEASE

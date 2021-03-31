//===-----------TypeMetadataReader.cpp - Decode metadata annotations-------===//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"

#define DEBUG_TYPE "dtrans-typemetadatareader"

namespace llvm {
namespace dtransOP {
// The tag name for the named metadata nodes that contains the list of structure
// types. This node is used to identify all the nodes that describe the fields
// of the structure so that we will know what all the original pointer type
// fields were.
const char *MDStructTypesTag = "intel.dtrans.types";

// Deprecated:
// TODO: Remove this when the test cases are updated.
// This tag was used for a named metadata node to get to the list of structure
// types in the LIT tests.
const char *MDStructTypesTagLegacy = "dtrans_types";

// The tag name used for variables and instructions marked with DTrans type
// information for pointer type recovery.
const char *MDDTransTypeTag = "intel_dtrans_type";

// Deprecated:
// TODO: Remove this when the test cases are updated.
const char *MDDTransTypeTagLegacy = "dtrans_type";

// Deprecated:
// TODO: Remove when LIT tests are updated.
// Tag associated with the metadata node that represents declaration types for
// declared functions and global variables.
const char *MDDeclTypesTag = "dtrans_decl_types";

// Tag used for metadata on a Function declaration/definition to map a set
// of metadata nodes of encoded types to attributes used on the return type
// and parameters.
const char *DTransFuncTypeMDTag = "intel.dtrans.func.type";

NamedMDNode *TypeMetadataReader::getDTransTypesMetadata(Module &M) {
  NamedMDNode *DTMDTypes = M.getNamedMetadata(MDStructTypesTag);
  if (DTMDTypes)
    return DTMDTypes;

  // Temporary fallback to the legacy tag name for existing LIT tests.
  // TODO: Remove when LIT tests are updated.
  DTMDTypes = M.getNamedMetadata(MDStructTypesTagLegacy);
  return DTMDTypes;
}

MDNode *TypeMetadataReader::getDTransMDNode(const Value &V) {
  if (auto *F = dyn_cast<Function>(&V)) {
    MDNode *MD = F->getMetadata(DTransFuncTypeMDTag);
    if (MD)
      return MD;

    // Temporary fallback to the legacy tag name for existing LIT tests.
    // TODO: Remove when LIT tests are updated.
    return F->getMetadata(MDDTransTypeTagLegacy);
  }

  if (auto *I = dyn_cast<Instruction>(&V)) {
    MDNode *MD = I->getMetadata(MDDTransTypeTag);
    // Temporary fallback to the legacy tag name for existing LIT tests.
    // TODO: Remove when LIT tests are updated.
    if (!MD)
      MD = I->getMetadata(MDDTransTypeTagLegacy);

    return MD;
  }

  if (auto *G = dyn_cast<GlobalObject>(&V)) {
    MDNode *MD = G->getMetadata(MDDTransTypeTag);

    // Temporary fallback to the legacy tag name for existing LIT tests.
    // TODO: Remove when LIT tests are updated.
    if (!MD)
      MD = G->getMetadata(MDDTransTypeTagLegacy);
    return MD;
  }

  return nullptr;
}

bool TypeMetadataReader::initialize(Module &M) {
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
    assert(DTStTy && "Expected recovered type to have been created");
    if (DTStTy->getReconstructError()) {
      LLVM_DEBUG(dbgs() << "Errors with type info for struct: " << *P.first
                        << "\n");
      RecoveryErrors = true;
    }
  }

  // Deprecated:
  // TODO: Remove this when the test cases are updated.
  //
  // Build a table to map symbol names for functions and variables to metadata
  // nodes about the symbol's type. This is to handle function and global
  // variable declarations since these cannot have metadata directly attached
  // to them in the IR.
  NamedMDNode *DTDeclTypes = M.getNamedMetadata(MDDeclTypesTag);
  if (DTDeclTypes) {
    for (auto *MD : DTDeclTypes->operands()) {
      // Each operand should be a pair of the form: { "symbol name", MDid }
      // where MDid is the MDNode that describes the symbol's type.
      if (MD->getNumOperands() != 2) {
        LLVM_DEBUG(
            dbgs() << "Incorrect MD encoding for declaration description:\n"
                   << *MD << "\n");
        continue;
      }

      auto *MDS = dyn_cast<MDString>(MD->getOperand(0));
      if (!MDS) {
        LLVM_DEBUG(
            dbgs() << "Incorrect MD encoding for declaration description:\n"
                   << *MD << "\n");
        continue;
      }

      auto *TypeID = dyn_cast<MDNode>(MD->getOperand(1));
      if (!TypeID) {
        LLVM_DEBUG(
            dbgs() << "Incorrect MD encoding for declaration description:\n"
                   << *MD << "\n");
        continue;
      }

      // If the symbol is locally defined, ignore the node and instead rely on
      // the type attached to the Function or GlobalVariable. This is necessary
      // because the type for the declaration may have different types in this
      // metadata due to the IRMover creating multiple types when it fails to
      // merge common types. However, the type where a definition for the symbol
      // exists would match the type used in the IR for the symbol.
      StringRef SymName = MDS->getString();
      Function *F = M.getFunction(SymName);
      GlobalVariable *GV = M.getGlobalVariable(SymName, /*AllowInternal=*/true);
      if ((F && !F->isDeclaration()) || (GV && !GV->isDeclaration()))
        continue;

      // The symbol may have declarations in multiple files. Due to type
      // renaming in the IRMover, there could be conflicting descriptions.
      auto It = SymbolNameToMDNodeMap.find(SymName);
      if (It != SymbolNameToMDNodeMap.end() && It->second != TypeID) {
        LLVM_DEBUG(dbgs() << "  WARNING: Multiple types for symbol " << SymName
                          << ":" << *MD << "\n");
        // Reset the type information because it is ambiguous.
        SymbolNameToMDNodeMap[SymName] = nullptr;
        continue;
      }

      SymbolNameToMDNodeMap[SymName] = TypeID;
    }
  }
  // End of deprecated region

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
  const unsigned FieldTyStartPos = 3;

  assert(MD->getOperand(0) && isa<MDString>(MD->getOperand(0)) &&
         (cast<MDString>(MD->getOperand(0)))->getString().equals("S") &&
         "improper MD node");
  assert(MD->getNumOperands() >= 3 &&
         "Incorrect MD encoding for structure description");

  auto *FieldCountMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(2));
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

  // If the module has the corresponding structure, we will use it to check for
  // possible incorrect metadata information.
  llvm::StructType *StTy =
      StructType::getTypeByName(M.getContext(), DTStTy->getName());

  unsigned FieldNum = 0;
  unsigned NumOps = MD->getNumOperands();
  for (unsigned Idx = FieldTyStartPos; Idx < NumOps; ++Idx, ++FieldNum) {
    auto *FieldMD = dyn_cast<MDNode>(MD->getOperand(Idx));
    assert(FieldMD && "Incorrect MD encoding for structure fields");

    DTransType *DTFieldTy = decodeMDNode(FieldMD);
    if (!DTFieldTy) {
      DTStTy->setReconstructError();
      LLVM_DEBUG(dbgs() << *DTStTy << " :Error decoding field: " << FieldNum
                        << "\n");
      continue;
    }

    if (StTy) {
      llvm::Type *IRFieldType = StTy->getElementType(FieldNum);
      bool ExpectPointer = IRFieldType->isPointerTy();
      bool GotPointer = DTFieldTy->isPointerTy();
      if (ExpectPointer ^ GotPointer) {
        DTStTy->setReconstructError();
        LLVM_DEBUG(dbgs() << "Mismatch occurred:\n  Expected: " << *IRFieldType
                          << "\n  Found: " << *DTFieldTy << "\n");
      }
    }

    DTransFieldMember &Field = DTStTy->getField(FieldNum);
    Field.addResolvedType(DTFieldTy);
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

// This method is the publicly visible method that will check whether a Value
// has DTransType metadata, and returns it if available.
// Otherwise, returns nullptr.
DTransType *TypeMetadataReader::getDTransTypeFromMD(Value *V) {
  if (auto *F = dyn_cast<Function>(V)) {
    // Functions that had metadata were decoded during the initialize() method,
    // and the results stored in a table.
    DTransFunctionType *Ty = getDTransType(F);
    if (Ty)
      return Ty;
  }

  MDNode *MD = getDTransMDNode(*V);

  // Deprecated:
  // TODO: Remove when LIT tests are updated.
  // Try to find a type from the table of declaration types.
  if (!MD)
    if (auto *G = dyn_cast<GlobalObject>(V)) {
      auto It = SymbolNameToMDNodeMap.find(G->getName());
      if (It != SymbolNameToMDNodeMap.end())
        MD = It->second;
    }

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
    // Deprecated: TODO: Remove when LIT tests are updated.
    else if (Tag.equals("R"))
      return decodeMDStructRefNode(MD);
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

  auto *PtrLevelMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(1));
  assert(PtrLevelMD && "Expected metadata constant");
  unsigned PtrLevel = cast<ConstantInt>(PtrLevelMD->getValue())->getZExtValue();

  auto *SimpleType = DTransAtomicType::get(TM, Ty);
  assert(SimpleType && "Type was not first class type");

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
  unsigned NumElem = cast<ConstantInt>(NumElemMD->getValue())->getZExtValue();

  auto Result = DTransVectorType::get(TM, ElemType, NumElem);
  cacheMDDecoding(MD, Result);
  return Result;
}

// Decode a metadata description for a structure reference node.
// Metadata is of the form:
//     !{<type> zeroinitializer, i32 <pointer level> }
//
// Deprecated format:
// TODO: Remove when LIT tests are updated.
//     !{!"R", <type> zeroinitializer, i32 <pointer level> }
//
DTransType *TypeMetadataReader::decodeMDStructRefNode(MDNode *MD) {
  if (MD->getNumOperands() < 2) {
    LLVM_DEBUG(dbgs() << "Incorrect operand count for reference type encoding:"
                      << *MD << "\n");
    return nullptr;
  }

  int TypeIndex = 0;
  int PtrLevelIndex = 1;

  // Fallback to deprecated "R" format.
  // TODO: Remove when LIT tests are updated.
  if (MD->getNumOperands() == 3) {
    TypeIndex = 1;
    PtrLevelIndex = 2;
  }

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
    // TODO: Remove the macro test when the attribute is availabile. Until then,
    // we can simulate the behavior by using a string attribute.
    Attribute Attr;
#if defined(DTRANS_FUNC_INDEX_ATTR_AVAILABLE)
    Attr = Attrs.getAttribute(Attribute::DTransFuncIndex);
    if (Attr.isValid()) {
      uint64_t Index = Attr.getValueAsInt();
      assert(Index >= 1 && "Expected 1 based indexing");
      return Index;
    }
#endif // DTRANS_FUNC_INDEX_ATTR_AVAILABLE

    // Temporary fallback to the use a string attribute for testing.
    // TODO: Remove when LIT tests are updated, and the real attribute is added
    // to the IR.
    Attr = Attrs.getAttribute("intel_dtrans_func_index");
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
    AttributeSet RetAttrs = Attrs.getRetAttributes();
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
      AttributeSet ParamAttrs = Attrs.getParamAttributes(Idx);
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

DTransFunctionType *TypeMetadataReader::getDTransType(Function *F) const {
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
    TM.printTypes();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    dbgs() << DEBUG_TYPE << ": All structures types "
           << (AllResolved ? "" : " NOT ") << "populated\n";

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
    // whether the metadata information mataches the pointer type in the IR.
    bool OpaquePointersEnabled = false;
    LLVMContext &Ctx = M.getContext();
    llvm::Type *I8Ptr = llvm::Type::getInt8Ty(Ctx)->getPointerTo();
    llvm::Type *I16Ptr = llvm::Type::getInt16Ty(Ctx)->getPointerTo();
    if (I8Ptr == I16Ptr)
      OpaquePointersEnabled = true;

    // Check whether all the global variables that are expected to have metadata
    // have it.
    for (auto &GV : M.globals()) {
      llvm::Type *GVType = GV.getValueType();
      if (dtrans::hasPointerType(GVType)) {
        MDNode *MD = Reader.getDTransMDNode(GV);
        if (!MD) {
          ErrorsFound = true;
          LLVM_DEBUG(dbgs()
                     << DEBUG_TYPE
                     << ":   ERROR: Missing var type metadata: " << GV << "\n");
          continue;
        }

        DTransType *DType = Reader.decodeMDNode(MD);
        if (!DType) {
          ErrorsFound = true;
          LLVM_DEBUG(dbgs() << DEBUG_TYPE
                            << ":   ERROR: Failed to decode var type metadata: "
                            << GV << " - " << *MD << "\n");
        } else {
          LLVM_DEBUG(dbgs() << DEBUG_TYPE << ":  Decoded var type metadata: "
                            << GV << " - " << *DType << "\n");

          if (!OpaquePointersEnabled && GVType != DType->getLLVMType()) {
            ErrorsFound = true;
            LLVM_DEBUG(
                dbgs()
                << DEBUG_TYPE
                << ":   ERROR: Metadata type does not match expected type: "
                << GV.getName() << "\n  IR: " << *GV.getValueType()
                << "\n  MD: " << *DType << "\n");
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
      if (!DType) {
        // TODO: Remove check for metadata attachment when LIT tests are updated
        // to use new form of tagging.
        MDNode *MD = Reader.getDTransMDNode(F);
        if (!MD) {
          ErrorsFound = true;
          LLVM_DEBUG(dbgs() << DEBUG_TYPE
                            << ":   ERROR: Missing fn type metadata for: "
                            << F.getName() << "\n");
        } else {
          assert(F.getMetadata(DTransFuncTypeMDTag) == nullptr &&
                 "Expected a legacy DTrans encoding format to be in use");
          DType = Reader.decodeMDNode(MD);
          if (!DType) {
            ErrorsFound = true;
            LLVM_DEBUG(dbgs() << DEBUG_TYPE
                              << ":  ERROR: Failed to decode fn type metadata: "
                              << " - " << *MD << "\n");
          }
        }
      }

      if (DType) {
        LLVM_DEBUG(dbgs() << DEBUG_TYPE << ":   Decoded fn type metadata: "
                          << *DType << "\n");
        if (!OpaquePointersEnabled && DType->getLLVMType() != FnType) {
          ErrorsFound = true;
          LLVM_DEBUG(
              dbgs()
              << DEBUG_TYPE
              << ":   ERROR: Metadata type does not match expected type: "
              << F.getName() << "\n  IR: " << *FnType << "\n  MD: " << *DType
              << "\n");
        }
      }
    }

    for (auto &I : instructions(F)) {
      if (auto *AI = dyn_cast<AllocaInst>(&I)) {
        llvm::Type *AllocType = AI->getAllocatedType();
        if (dtrans::hasPointerType(AllocType)) {
          MDNode *MD = Reader.getDTransMDNode(*AI);
          if (!MD) {
            ErrorsFound = true;
            LLVM_DEBUG(dbgs() << DEBUG_TYPE << ":   ERROR: Missing metadata: "
                              << *AI << "\n");
            continue;
          }

          DTransType *DType = Reader.decodeMDNode(MD);
          if (!DType) {
            ErrorsFound = true;
            LLVM_DEBUG(dbgs() << DEBUG_TYPE
                              << ":   ERROR: Failed to decode metadata: " << *AI
                              << " - " << *MD << "\n");
          } else {
            LLVM_DEBUG(dbgs() << DEBUG_TYPE << ":  Decoded metadata: " << *AI
                              << " - " << *DType << "\n");
            if (!OpaquePointersEnabled && DType->getLLVMType() != AllocType) {
              ErrorsFound = true;
              LLVM_DEBUG(dbgs() << DEBUG_TYPE
                                << ":  ERROR: Metadata type does not match "
                                   "expected type: "
                                << *AI << "\n  IR: " << *AI->getAllocatedType()
                                << "\n  MD: " << *DType << "\n");
            }
          }
        }
      } else if (auto *Call = dyn_cast<CallBase>(&I)) {
        if (Call->isIndirectCall() &&
            dtrans::hasPointerType(Call->getFunctionType())) {
          MDNode *MD = Reader.getDTransMDNode(*Call);
          if (!MD) {
            ErrorsFound = true;
            LLVM_DEBUG(dbgs() << DEBUG_TYPE << ":   ERROR: Missing metadata: "
                              << *Call << "\n");
            continue;
          }

          DTransType *DType = Reader.decodeMDNode(MD);
          if (!DType) {
            ErrorsFound = true;
            LLVM_DEBUG(dbgs()
                       << DEBUG_TYPE << ":   ERROR: Failed to decode metadata: "
                       << *Call << " - " << *MD << "\n");
          } else {
            LLVM_DEBUG(dbgs() << DEBUG_TYPE << ":  Decoded metadata: " << *Call
                              << " - " << *DType << "\n");
            if (!OpaquePointersEnabled &&
                DType->getLLVMType() != Call->getFunctionType()) {
              LLVM_DEBUG(dbgs()
                         << DEBUG_TYPE
                         << ":   ERROR:  Metadata type does not match "
                            "expected type: "
                         << *Call << "\n  IR: " << *Call->getFunctionType()
                         << "\nMD: " << *DType << "\n");
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

//===--- DTransArraysWithConstant.cpp - Arrays with constant entries -*---===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
// This is an extension to the DTrans analysis that checks if a field in a
// structure is an array with constant entries. If so, then collect the
// constant values and store them in the DTransInfo. For example:
//
//   %class.TestClass = type <{i32, [4 x i32]}>
//
//   define void @foo(%class.TestClass* %0, i32 %var) {
//     %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
//             i64 0, i32 1
//     %tmp2 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i32 0
//     store i32 1, i32* %tmp2
//     %tmp3 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i32 1
//     store i32 2, i32* %tmp3
//     %tmp4 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i32 2
//     store i32 %var, i32* %tmp4
//     %tmp5 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i32 3
//     store i32 %var, i32* %tmp5
//     ret void
//   }
//
//   define i32 @bar(%class.TestClass* %0) {
//   entry:
//     %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
//             i64 0, i32 1
//     br label %bb1
//
//   bb1:
//     %phi1 = phi i32 [ 0, %entry], [ %var, %bb1]
//     %tmp2 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1,
//             i64 0, i32 %phi1
//     %tmp3 = load i32, i32* %tmp2
//     %var = add nuw i32 1, %phi1
//     %tmp4 = icmp eq i32 4, %var
//     br i1 %tmp4, label %bb2, label %bb1
//
//   bb2:
//     ret i32 %tmp3
//   }
//
// In the example above, field 1 in the class %class.TestClass is an array of
// integers. In function @foo, entry 0 of this array is set to 1 and entry
// 1 is set to 2. Later, function @bar loads from this array. This means that
// when %var is 0 or 1, the values loaded will be 1 and 2. These constant
// values are stored once and won't change during the entire execution of the
// program. The dtrans::FieldInfo for %class.TestClass, field 1, will be
// updated to include the constant entries collected. This information will
// be passed to the DTransImmutable analysis, which is used later by the loop
// optimizer, to collect the constant values. For example, the loop optimizer
// can unroll the loop in @bar, and then convert the loads to entries 0 and 1
// into constant values.
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransArraysWithConstant.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "dtrans-arrays-with-const-entries"

// Debug type for verbose information
#define DTRANS_ARRCONST_VERBOSE "dtrans-arrays-with-const-entries-verbose"

namespace llvm {

namespace dtrans {

static ConstantInt *getIndexAccessedFromGEP(GetElementPtrInst *GEP);
static ConstantInt *getFieldAndStructureAccessed(GetElementPtrInst *GEP,
                                                 llvm::StructType **StructTy);

// Implementation of the helper class FieldWithConstantArray

// Collect the GEP that accesses the current field, and will be used for storing
// the constant values.
void dtrans::FieldWithConstantArray::addGEPForStore(
    GetElementPtrInst *GEPStore) {

  // The field number that is being accessed should match.
  llvm::StructType *Structure = nullptr;
  ConstantInt *FieldNum = getFieldAndStructureAccessed(GEPStore, &Structure);
  if (FieldNum != FieldNumber || !GEPStore->isInBounds()) {
    disableField();
    return;
  }

  if (GEPsForStore.count(GEPStore) > 0) {
    disableField();
    return;
  }

  GEPsForStore.insert(GEPStore);
}

// Given the constant integers Index and ConstValue, pair them and
// add them in ConstantEntries. If the Index is already is inserted
// then ConstValue must match with the constant value, else set it
// as nullptr. If Index is null it means that we don't have any
// information of which field is being accessed, disable the whole field.
// If ConstValue is nullptr then it means that Index is not constant.
void dtrans::FieldWithConstantArray::addConstantEntry(ConstantInt *Index,
                                                      ConstantInt *ConstValue) {
  if (!Index) {
    disableField();
    return;
  }
  for (auto &ConstEntry : ConstantEntries) {
    if (ConstEntry.first == Index) {
      if (!ConstEntry.second)
        return;
      if (ConstEntry.second != ConstValue) {
        ConstEntry.second = nullptr;
        return;
      }
      return;
    }
  }
  ConstantEntries.push_back({Index, ConstValue});
}

// Return true if the Integer type was set or is the same as the input
// InType, else return false.
bool dtrans::FieldWithConstantArray::setIntegerType(llvm::IntegerType *InType) {
  // Check that the input type is there
  if (!InType)
    return false;

  // Set IntType for the first time
  if (!IntType) {
    IntType = InType;
    return true;
  }

  // Check that IntType is the same as the input type
  return IntType == InType;
}

// Clean all the information stored for the field and disable it for
// storing any constant information
void dtrans::FieldWithConstantArray::disableField() {
  ConstantEntries.clear();
  GEPsForStore.clear();
  LoadInstructions.clear();
  FieldDisabled = true;
  IntType = nullptr;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Print all the information collected. This is useful for debugging.
void dtrans::FieldWithConstantArray::printFieldFull() {
  dbgs() << "  Field number: " << FieldNumber->getZExtValue() << "\n";
  dbgs() << "    Field available: " << (!FieldDisabled ? "YES" : "NO") << "\n";
  dbgs() << "    Constants:";
  if (ConstantEntries.empty() || !hasAtLeastOneConstantEntry()) {
    dbgs() << " No constant data found";
  } else {
    dbgs() << "\n";
    for (auto ConstantEntry : ConstantEntries) {
      if (ConstantEntry.second)
        dbgs() << "      Index: " << *ConstantEntry.first
               << "      Value: " << *ConstantEntry.second << "\n";
    }
  }

  dbgs() << "\n    GEPs used for store: ";
  if (GEPsForStore.empty()) {
    dbgs() << "      No GEP for store found";
  } else {
    for (auto *GEP : GEPsForStore)
      dbgs() << "      " << *GEP << "\n";
  }

  dbgs() << "\n    Data loaded in:";
  if (LoadInstructions.empty()) {
    dbgs() << "      No load instruction found\n";
  } else {
    dbgs() << "\n";
    for (auto Load : LoadInstructions)
      dbgs() << "      " << *Load << "\n";
  }
  dbgs() << "\n";
}

// Print the information for the field simplified
void dtrans::FieldWithConstantArray::printFieldSimple() {
  dbgs() << "  Field number: " << FieldNumber->getZExtValue() << "\n";
  dbgs() << "    Field available: " << (!FieldDisabled ? "YES" : "NO") << "\n";
  dbgs() << "    Constants:";
  if (ConstantEntries.empty() || !hasAtLeastOneConstantEntry()) {
    dbgs() << " No constant data found";
  } else {
    dbgs() << "\n";
    for (auto ConstantEntry : ConstantEntries) {
      if (ConstantEntry.second)
        dbgs() << "      Index: " << *ConstantEntry.first
               << "      Value: " << *ConstantEntry.second << "\n";
    }
  }
  dbgs() << "\n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Implementation for StructWithArrayFields

// Return true if a new field that could be an array with constant entries
// was added succesfully, else return false.
bool dtrans::StructWithArrayFields::addField(FieldWithConstantArray *NewField) {
  ConstantInt *NewFieldConst = NewField->getFieldNumber();
  if (!NewFieldConst) {
    StructureDisabled = true;
    return false;
  }
  // If the field is already inserted then disable the structure.
  // NOTE: This is conservative, we could merge information
  for (auto *Field : FieldsWithConstArrayVector) {
    if (NewFieldConst == Field->getFieldNumber()) {
      Field->disableField();
      return false;
    }
  }
  // The field must be within the bounds of the structure
  unsigned NewFieldNum = NewFieldConst->getZExtValue();
  unsigned NumElements = Struct->getNumElements();
  if (NewFieldNum >= NumElements) {
    StructureDisabled = true;
    return false;
  }
  FieldsWithConstArrayVector.insert(NewField);
  return true;
}

// Given a field number return the field information stored.
FieldWithConstantArray *
dtrans::StructWithArrayFields::getField(ConstantInt *FieldNumber) {
  for (auto *Field : FieldsWithConstArrayVector)
    if (Field->getFieldNumber() == FieldNumber)
      return Field;
  return nullptr;
}

// Disable the structure if we find something that couldn't be analyzed.
// For example:
//
//   %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
//           i64 0, i32 %VAR
//
// The instruction in the example above is a GEP which we don't know the
// value of %VAR. Therefore we don't know which field will be collected. The
// structure will be disabled.
void dtrans::StructWithArrayFields::disableStructure() {
  for (auto *Field : FieldsWithConstArrayVector)
    Field->disableField();

  StructureDisabled = true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Print the structure with the full information for all fields
void dtrans::StructWithArrayFields::printStructureFull() {
  dbgs() << "Type: " << *Struct << "\n";
  dbgs() << "  Is structure available: " << (!StructureDisabled ? "YES" : "NO")
         << "\n";
  if (FieldsWithConstArrayVector.empty())
    dbgs() << "  Empty data\n";
  else
    for (auto *Field : FieldsWithConstArrayVector)
      Field->printFieldFull();
}

// Print the structure with the fields information simplified
void dtrans::StructWithArrayFields::printStructureSimple(bool PrintAll) {
  dbgs() << "Type: " << *Struct << "\n";
  dbgs() << "  Is structure available: " << (!StructureDisabled ? "YES" : "NO")
         << "\n";
  if (FieldsWithConstArrayVector.empty()) {
    dbgs() << "  Empty data\n";
  } else {
    for (auto *Field : FieldsWithConstArrayVector) {
      if (Field->isFieldDisabled() && !PrintAll)
        continue;
      Field->printFieldSimple();
    }
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Given a GEP, return the ConstantInt that is used to access an element
static ConstantInt *getIndexAccessedFromGEP(GetElementPtrInst *GEP) {
  if (!GEP)
    return nullptr;

  // If the GEP is not composed of constants or is not in bounds return
  // nullptr
  if (!GEP->hasAllConstantIndices() || !GEP->isInBounds())
    return nullptr;

  // Make sure the first operand is 0
  unsigned NumIndices = GEP->getNumIndices();
  if (NumIndices < 2 || !match(GEP->getOperand(1), m_Zero()))
    return nullptr;

  // Return the constant integer
  return cast<ConstantInt>(GEP->getOperand(NumIndices));
}

// Given a GEP, check if the source type is a structure and traverse through the
// indices of the GEP. Return the index that accesses a field in a structure
// that is NOT a structure, or a field that is a structure that wraps an array
// of integers. The structure found will be stored in StructTy.
static ConstantInt *getFieldAndStructureAccessed(GetElementPtrInst *GEP,
                                                 llvm::StructType **StructTy) {

  // Return true if the input Type is a structure, with one field
  // and that field is an array of integers.
  auto IsWrappingStruct = [](llvm::Type *InTy) {
    if (!InTy)
      return false;

    auto *StructTy = dyn_cast<llvm::StructType>(InTy);
    if (!StructTy || StructTy->getNumElements() != 1)
      return false;

    auto *ArrField = dyn_cast<llvm::ArrayType>(StructTy->getElementType(0));
    if (!ArrField || !ArrField->getElementType()->isIntegerTy())
      return false;

    return true;
  };

  if (!GEP)
    return nullptr;

  llvm::Type *SrcTy = GEP->getSourceElementType();

  if (!SrcTy->isStructTy())
    return nullptr;

  unsigned NumIndices = GEP->getNumIndices();

  if (NumIndices < 2)
    return nullptr;

  llvm::StructType *CurrStruct = nullptr;
  llvm::Type *GEPType = GEP->getOperand(0)->getType();
  if (GEPType->isPointerTy())
    CurrStruct = dyn_cast<llvm::StructType>(GEPType->getPointerElementType());
  if (!CurrStruct)
    return nullptr;

  // Start at index 2 since index 1 is reserved for arrays.
  unsigned Pos = 2;
  ConstantInt *ConstIndex = nullptr;
  llvm::StructType *PrevStruct = CurrStruct;
  while (CurrStruct && Pos <= NumIndices) {
    ConstIndex = dyn_cast<ConstantInt>(GEP->getOperand(Pos));
    if (!ConstIndex)
      return nullptr;
    unsigned Index = ConstIndex->getZExtValue();

    if (Index > CurrStruct->getNumElements()) {
      ConstIndex = nullptr;
      *StructTy = PrevStruct;
      return nullptr;
    }

    llvm::Type *FieldTy = CurrStruct->getElementType(Index);

    // This is a special case to catch when a structure wraps an array
    if (IsWrappingStruct(FieldTy)) {
      break;
    } if (FieldTy->isStructTy()) {
      Pos++;
      PrevStruct = CurrStruct;
      CurrStruct = cast<llvm::StructType>(FieldTy);
    } else {
      break;
    }
  }

  *StructTy = PrevStruct;
  return ConstIndex;
}

// Given an GEP and the structure that the GEP is accessing, return
// the same GEP if it is accessing an array. If the GEP is used to
// access an inner structure, then recurse. This is needed to catch
// special cases where an array is stored inside a structure,
// for example:
//
//   %class.TestClass = type <{i32, %"class.boost::array.2"}>
//   %"class.boost::array.2" = type {[4 x i32]}
//
//   %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
//           i64 0, i32 1
//   %tmp2 = getelementptr inbounds %"class.boost::array.2",
//           %"class.boost::array.2"* %tmp1, i64 0, i64 0
//   %tmp3 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp2, i64 0, i64 0
//   store i32 1, i32* %tmp3
//
// In the example above, the field 1 in %class.TestClass is another structure
// that wraps an array. In this case, the input GEP will be %tmp1 and the
// input structure type will be %class.TestClass. This function will
// return %tmp2, which is the actual GEP that accesses the array.
static GetElementPtrInst *getGEPAccessingArray(GetElementPtrInst *GEP,
                                               llvm::StructType *GEPType,
                                               DTransAnalysisInfo *DTInfo) {

  if (!GEP || !GEPType)
    return nullptr;

  // If the type doesn't pass the safety checks for arrays with constant
  // information then return nullptr.
  dtrans::TypeInfo *TI = DTInfo->getTypeInfo(GEPType);
  if (!TI || TI->testSafetyData(dtrans::SDArraysWithConstantEntries))
    return nullptr;

  // The field accessed must be constant and within the bounds of the
  // structure
  ConstantInt *FieldNumConst = getIndexAccessedFromGEP(GEP);
  if (!FieldNumConst)
    return nullptr;

  unsigned FieldNum = FieldNumConst->getZExtValue();
  if (FieldNum >= GEPType->getNumElements())
    return nullptr;

  // If the field is an array, then return GEP, if the field is a
  // structure, then make sure there is only one use for GEP
  // and that use is for accessing the inner array.
  llvm::Type *FieldType = GEPType->getElementType(FieldNum);
  GetElementPtrInst *RetGEP = nullptr;
  if (FieldType->isArrayTy()) {
    RetGEP = GEP;
  } else if (auto *FieldStruct = dyn_cast<StructType>(FieldType)) {
    if (FieldStruct->getNumElements() != 1)
      return nullptr;

    if (!GEP->hasOneUse())
      return nullptr;

    GetElementPtrInst *UserGEP = dyn_cast<GetElementPtrInst>(GEP->user_back());
    if (!UserGEP)
      return nullptr;

    RetGEP = getGEPAccessingArray(UserGEP, FieldStruct, DTInfo);
  }

  return RetGEP;
}

// Return true in the input store instruction doesn't invalidate the input
// field or structure, and is used to store an integer in an array. The GEP
// is used to find which index is being accessed in the array.
static bool checkStoreInst(StoreInst *Store, llvm::ArrayType *SrcArr,
                           FieldWithConstantArray *FieldData,
                           StructWithArrayFields *StructData,
                           GetElementPtrInst *EntryGEP) {
  if (!Store || !FieldData || !StructData || !EntryGEP)
    return false;

  // The index must be a constant integer
  ConstantInt *Index = getIndexAccessedFromGEP(EntryGEP);
  if (!Index) {
    FieldData->disableField();
    return false;
  }

  unsigned IndexAccessed = Index->getZExtValue();

  if (IndexAccessed > SrcArr->getNumElements()) {
    // the index must be within the size of the array.
    StructData->disableStructure();
    return false;
  }

  // Store the constant integer for the index (NOTE: ConstValue can be nullptr,
  // it means that a variable is being stored in that entry).
  ConstantInt *ConstValue = dyn_cast<ConstantInt>(Store->getValueOperand());
  FieldData->addConstantEntry(Index, ConstValue);
  return true;
}

// Return false if the input GEP is used for anything else than loading or
// storing constant integers, else return true. Also, update the information in
// StructData and FieldData with the Load and Store instructions found.
static bool checkGEPUsers(GetElementPtrInst *GEP, llvm::ArrayType *ArrayTy,
                          StructWithArrayFields *StructData,
                          FieldWithConstantArray *FieldData, bool *StoreFound,
                          bool *LoadFound) {

  if (!GEP || !ArrayTy || !StructData || !FieldData)
    return false;

  // Each user of the GEP that accesses the entries in the array must
  // be a load or store instruction.
  for (User *UserEntry : GEP->users()) {
    if (auto *Store = dyn_cast<StoreInst>(UserEntry)) {
      if (!checkStoreInst(Store, ArrayTy, FieldData, StructData, GEP))
        return false;
      *StoreFound = true;
    } else if (auto *Load = dyn_cast<LoadInst>(UserEntry)) {
      FieldData->insertLoadInstruction(Load);
      *LoadFound = true;
    } else {
      return false;
    }
    // This is conservative, for now all the stores must happen in one
    // function and all the loads must happen in other functions. In
    // reality we can have load and stores mixed together.
    if (*StoreFound && *LoadFound)
      return false;
  }

  return true;
}

// Collect the data from a GEP that has multiple indices, for example:
//
//   define void @foo(%class.TestClass %0) {
//     %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
//             i64 0, i32 12, i64 0
//     store i32 1, i32* %tmp1
//     ret void
//   }
//
// In the example above, the GEP's indices represent the following:
// field that is an array (Operand 2) and index accessed (Operand 3). This
// means that one GEP is used to access the field in the structure that is
// an array, and access the entry in the array.
static void collectFromMultipleIndicesGEP(GetElementPtrInst *GEP,
                                          llvm::StructType *Structure,
                                          DTransAnalysisInfo *DTInfo,
                                          StructWithArrayFields *StructData,
                                          FieldWithConstantArray *FieldData) {

  if (!GEP || !StructData || !FieldData || !Structure || !DTInfo)
    return;

  unsigned NumIndices = GEP->getNumIndices();

  assert(NumIndices > 2 && "Number of indices lower than the required");

  if (!match(GEP->getOperand(1), m_Zero())) {
    StructData->disableStructure();
    return;
  }

  unsigned FieldNum = FieldData->getFieldNumber()->getZExtValue();
  llvm::Type *FieldType = Structure->getElementType(FieldNum);
  llvm::IntegerType *IntType = nullptr;
  llvm::ArrayType *ArrayTy = nullptr;

  // Check if the GEP is in the following form:
  //
  //   %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
  //           i64 0, i32 12, i64 0
  //
  // Operand 2 in %tmp1 is the field in a structure that is an array, and
  // Operand 3 is the index that is being accessed in the array.
  //
  // NOTE: This is conservative, we can relax it in the future.
  if (auto *ArrTy = dyn_cast<llvm::ArrayType>(FieldType)) {
    if (NumIndices != 3 || !ArrTy->getElementType()->isIntegerTy()) {
      FieldData->disableField();
      return;
    }

    ArrayTy = ArrTy;
    IntType = cast<IntegerType>(ArrTy->getElementType());
  } else if (auto *StrTy = dyn_cast<llvm::StructType>(FieldType)) {
    // Check if the GEP is in the following form:
    //
    //   %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
    //           i64 0, i32 12, i64 0, i32 0
    //
    // Operand 2 in %tmp1 is the field in a structure that encapsulates another
    // structure, Operand 3 is an array and Operand 4 is the index that is
    // being accessed in the array. This is for handling special cases like
    // boost arrays.
    //
    // NOTE: This is conservative, we can relax it in the future.
    if (NumIndices != 4 || StrTy->getNumElements() != 1) {
      FieldData->disableField();
      return;
    }

    ConstantInt *GEPOperand3 = dyn_cast<ConstantInt>(GEP->getOperand(3));
    if (!GEPOperand3 || !GEPOperand3->isZero()) {
      FieldData->disableField();
      return;
    }

    llvm::ArrayType *ArrTy = dyn_cast<ArrayType>(StrTy->getElementType(0));
    if (!ArrTy || !ArrTy->getElementType()->isIntegerTy()) {
      FieldData->disableField();
      return;
    }

    ArrayTy = ArrTy;
    IntType = cast<IntegerType>(ArrTy->getElementType());
  }

  // Check that the array and the integer types were found
  if (!ArrayTy || !IntType) {
    FieldData->disableField();
    return;
  }

  // The GEP must be used only for store
  bool StoreFound = false;
  // The GEP must be used only for loads
  bool LoadFound = false;

  if (!FieldData->setIntegerType(IntType)) {
    FieldData->disableField();
    return;
  }

  // Check if the GEP is used for loading or storing constant integers
  if (!checkGEPUsers(GEP, ArrayTy, StructData, FieldData, &StoreFound,
                     &LoadFound)) {
    FieldData->disableField();
    return;
  }

  // Collect the GEP if it is used for storing data in an array
  if (StoreFound && !LoadFound)
    FieldData->addGEPForStore(GEP);
}

// Collect the data if it is inside multiple GEPs, for example:
//
//   define void @foo(%class.TestClass %0) {
//     %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
//             i64 0, i32 12
//     %tmp2 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i64 0
//     store i32 1, i32* %tmp2
//     ret void
//   }
//
// In the example above, we need two GEPs to access the index we want in the
// array. The first GEP (%tmp1) accesses the field in the structure that is
// an array and the second GEP (%tmp2) accesses the index in the array.
static void collectFromMultipleGEPs(GetElementPtrInst *GEP,
                                    llvm::StructType *Structure,
                                    DTransAnalysisInfo *DTInfo,
                                    StructWithArrayFields *StructData,
                                    FieldWithConstantArray *FieldData) {

  if (!GEP || !StructData || !FieldData || !Structure || !DTInfo)
    return;


  assert(GEP->getNumIndices() == 2 && "Number of indices different than "
                                      "the required");

  if (StructData->isStructureDisabled() || FieldData->isFieldDisabled())
    return;

  // If the field is an array of integers, then ArrayGEP will be GEP. Else,
  // if the field is a structure with one field, then we need to collect
  // the GetElementPtrInst instruction that accesses the entries in the
  // array. For example:
  //
  //   %class.TestClass = type <{i32, %"class.boost::array"}>
  //   %"class.boost::array" = type <{[4 x i32]}>
  //
  //   %tmp0 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
  //           i64 0, i32 1
  //   %tmp1 = getelementptr inbounds %"class.boost::array",
  //           %"class.boost::array"* %tmp0, i64 0, i32 0
  //
  // In the example above, %tmp0 is the instruction that is accessing the field
  // that is a structure (%class.TestClass field 1). The instruction %tmp1
  // is the GetElementPtrInst that accesses the entry in the array. In this
  // case, GEP will be %tmp0 and ArrayGEP will be %tmp1. If nothing is found
  // then ArrayGEP will be nullptr. This is to catch special cases like boost
  // libraries.
  GetElementPtrInst *ArrayGEP = getGEPAccessingArray(GEP, Structure, DTInfo);
  if (!ArrayGEP) {
    FieldData->disableField();
    return;
  }

  // The GEP must be used only for store
  bool StoreFound = false;
  // The GEP must be used only for loads
  bool LoadFound = false;

  // This loop checks that the users of ArrayGEP are GEPs accessing entries
  // in an array, and they are only used for load or store instructions.
  for (User *U : ArrayGEP->users()) {
    // Each user of the GEP is another GEP that is accessing the entry
    // in an array.
    if (auto *EntryGEP = dyn_cast<GetElementPtrInst>(U)) {

      if (!EntryGEP->isInBounds() ||
          !EntryGEP->getSourceElementType()->isArrayTy()) {
        StructData->disableStructure();
        return;
      }

      // The GEP must be accessing an entry in an array of integers
      llvm::ArrayType *SrcTy =
          dyn_cast<llvm::ArrayType>(EntryGEP->getSourceElementType());
      llvm::IntegerType *ResType =
          dyn_cast<llvm::IntegerType>(EntryGEP->getResultElementType());

      if (!SrcTy || !ResType) {
        FieldData->disableField();
        return;
      }

      if (!FieldData->setIntegerType(ResType)) {
        // If the type couldn't be set or compared with the type in the
        // the field, then invalidate the field
        FieldData->disableField();
        return;
      }

      // Check if EntryGEP is used for load or store constant integers
      if (!checkGEPUsers(EntryGEP, SrcTy, StructData, FieldData, &StoreFound,
                         &LoadFound)) {
        FieldData->disableField();
        return;
      }
    } else {
      FieldData->disableField();
      return;
    }
  }

  // Collect the GEP if it is used for storing data in an array
  if (StoreFound && !LoadFound)
    FieldData->addGEPForStore(GEP);
}

// Given a GEP and the structure type that it is accessing, update the
// information for the field that the GEP is accessing. This is the
// place where we are going to make sure that all accesses for the
// arrays are loading entries from them, or storing values. For example:
//
//   define void @foo(%class.TestClass %0) {
//     %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
//             i64 0, i32 12
//     %tmp2 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i64 0
//     store i32 1, i32* %tmp2
//     ret void
//   }
//
//   define i32 @bar(%class.TestClass %0) {
//     %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
//             i64 0, i32 12
//     %tmp2 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i64 0
//     %tmp3 = load i32, i32* %tmp2
//     ret i32 %tmp3
//   }
//
// If GEP is %tmp1 in @foo, from the example above, then this function will
// collect the constant entries that are being stored (i32 1 in index 0 for
// field 12). Else, if GEP is %tmp1 in @bar, then this function will collect
// the GEP that is used for loading data (%tmp2 in @bar).
static void checkAndUpdateStructure(GetElementPtrInst *GEP,
                                    ConstantInt *FieldNumConst,
                                    StructWithArrayFields *StructData,
                                    DTransAnalysisInfo *DTInfo) {
  if (!GEP)
    return;

  // If the structure is disabled then there is nothing to do
  if (!StructData || StructData->isStructureDisabled())
    return;

  if (!FieldNumConst) {
    StructData->disableStructure();
    return;
  }

  // The GEP must access information within the bounds of the structure
  unsigned FieldNum = FieldNumConst->getZExtValue();
  llvm::StructType *Structure = StructData->getStructure();
  if (FieldNum >= Structure->getNumElements()) {
    StructData->disableStructure();
    return;
  }

  // Collect the constant information for the field that is being accessed
  FieldWithConstantArray *FieldData = StructData->getField(FieldNumConst);
  if (!FieldData) {
    FieldData = new FieldWithConstantArray(FieldNumConst);
    StructData->addField(FieldData);
  }

  // If the field is disabled then there is nothing to do
  if (FieldData->isFieldDisabled())
    return;

  // If the GEP has 2 indices, then we are going to check if the information
  // is inside multiple GEPs
  if (GEP->getNumIndices() == 2)
    collectFromMultipleGEPs(GEP, Structure, DTInfo, StructData, FieldData);

  // Else, if there are more than 2 indices, we are going to check if the GEP
  // accesses the data directly
  else if (GEP->getNumIndices() > 2)
    collectFromMultipleIndicesGEP(GEP, Structure, DTInfo, StructData,
                                  FieldData);

  // Byte flattened GEPs or GEPs with one index aren't allowed
  else
    StructData->disableStructure();

  // NOTE: The previous checks are conservative. We could have a byte flattened
  // GEP that accesses the data we want, for example:
  //
  // %class.OuterClass = type <{ %class.TestClass }>
  // %class.TestClass = type <{ [4 x i32], i32 }>
  //
  // define i32 @foo(i64* %0) {
  //   %tmp1 = getelementptr inbounds i64, i64* %0, i64 8
  //   %tmp2 = bitcast i64* %tmp1 to i32*
  //   %tmp3 = load i32, i32* %tmp2
  //   ret i32 %tmp3
  // }
  //
  // define void @bar(%class.OuterClass* %0) {
  //   %tmp1 = bitcast %class.OuterClass* %0 to i64*
  //   %tmp2 = call i32 @foo(i64* %tmp1)
  //   ret void
  // }
  //
  // In the example above, function @bar converts the pointer to the structure
  // %%class.OuterClass (%0) into an i64*. Then, it calls @foo, which accesses
  // entry 1 of the array inside %class.TestClass. This is a form of loading
  // information from an array which is a field from a structure.
}

// Given a structure, return the StructWithArrayFields in the map
// StructsWithConstArrays. If an entry for this structure is not found, then
// make a new entry and return it. This function will return nullptr if a
// structure is not provided or if the structure doesn't pass the safety
// checks for arrays with constant entries.
static StructWithArrayFields *
getStructWithArrayFields(llvm::StructType *Structure,
                         DTransAnalysisInfo *DTInfo,
                         DenseMap<llvm::StructType *, StructWithArrayFields *>
                             &StructsWithConstArrays) {

  if (!Structure)
    return nullptr;

  dtrans::TypeInfo *TI = DTInfo->getTypeInfo(Structure);
  if (!TI || TI->testSafetyData(dtrans::SDArraysWithConstantEntries))
    return nullptr;

  StructWithArrayFields *StructData = nullptr;
  if (StructsWithConstArrays.find(Structure) == StructsWithConstArrays.end()) {
    StructData = new StructWithArrayFields(Structure);
    StructsWithConstArrays.insert({Structure, StructData});
  } else {
    StructData = StructsWithConstArrays[Structure];
  }

  return StructData;
}

// Check if the input GEP instruction is used for loading or storing an entry
// in an array of integers, which is a field in a structure. If so, update
// the structure and field with the information collected.
static void
analyzeGEPInstruction(GetElementPtrInst *GEP, DTransAnalysisInfo *DTInfo,
                      DenseMap<llvm::StructType *, StructWithArrayFields *>
                          &StructsWithConstArrays) {

  if (!GEP)
    return;

  llvm::StructType *Structure = nullptr;
  // Collect the field and structure type that the GEP is accessing
  ConstantInt *FieldNumConst = getFieldAndStructureAccessed(GEP, &Structure);
  if (!Structure)
    return;

  StructWithArrayFields *StructData =
      getStructWithArrayFields(Structure, DTInfo, StructsWithConstArrays);
  if (!StructData || StructData->isStructureDisabled())
    return;

  // If the GEP is not in bounds then we disable the structure
  if (!GEP->isInBounds()) {
    StructData->disableStructure();
    return;
  }

  // Check if we can collect any load or store to an array of integers,
  // which is a field of the structure Structure. If so, then
  // update the information for the structure, else invalidate the
  // information for the field or the structure.
  checkAndUpdateStructure(GEP, FieldNumConst, StructData, DTInfo);
}

// Check if the load or store instruction is accessing a field in a structure.
// If so then invalidate the field. This is to catch any possible read or
// write to a field that we expect to be a constant array. For example:
//
//   %class.TestClass = type <{i32, [4 x i32]}>
//
//   %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
//           i64 0, i32 1
//   %tmp2 = load [4 x i32], [4 x i32]* %tmp1
//
// In the example above, the Load instruction in %tmp2 is loading the pointer
// to field 1 in %class.TestClass. This means that the field will be address
// taken and anything could happen to it. In this case, the constant
// information will be disabled for this field.
static void analyzeLoadOrStoreInstruction(
    Instruction *I, DTransAnalysisInfo *DTInfo,
    DenseMap<llvm::StructType *, StructWithArrayFields *>
        &StructsWithConstArrays) {

  if (!I)
    return;

  llvm::StructType *Structure = nullptr;
  size_t FieldNumber = 0;

  // Get which structure and field is being accessed
  if (auto *Store = dyn_cast<StoreInst>(I)) {
    auto StoreInfo = DTInfo->getStoreElement(Store);
    Structure = dyn_cast_or_null<llvm::StructType>(StoreInfo.first);
    FieldNumber = StoreInfo.second;
  } else if (auto *Load = dyn_cast<LoadInst>(I)) {
    auto LoadInfo = DTInfo->getLoadElement(Load);
    Structure = dyn_cast_or_null<llvm::StructType>(LoadInfo.first);
    FieldNumber = LoadInfo.second;
  }

  if (!Structure)
    return;

  StructWithArrayFields *StructData =
      getStructWithArrayFields(Structure, DTInfo, StructsWithConstArrays);

  // If the structure is disabled already then there is nothing to do
  if (!StructData || StructData->isStructureDisabled())
    return;

  // If we can't find any constant from the field number, disable the
  // structure
  llvm::Type *IntType = llvm::IntegerType::getInt32Ty(I->getContext());
  llvm::Constant *Const = llvm::ConstantInt::get(IntType, FieldNumber);
  llvm::ConstantInt *FieldNum = dyn_cast<ConstantInt>(Const);
  if (!FieldNum) {
    StructData->disableStructure();
    return;
  }

  // If the constant FieldNum is available, then it means that we know
  // which field is being used for a Load or Store instruction. In this
  // case disable the field.
  FieldWithConstantArray *FieldData = StructData->getField(FieldNum);
  if (!FieldData) {
    // If the field information is not in the structure data, then create
    // a new field entry and disable it.
    FieldData = new FieldWithConstantArray(FieldNum);
    StructData->addField(FieldData);
  }

  // Disable the field
  FieldData->disableField();
}

// Check if the call instruction calls a memfunc which can invalidate a
// structure or a field.
static void
analyzeCallInstruction(Instruction *I, DTransAnalysisInfo *DTInfo,
                       DenseMap<llvm::StructType *, StructWithArrayFields *>
                           &StructsWithConstArrays,
                       bool *ProcessDisabled) {

  // This function wipes all the data in case we find something that
  // we can't handle at the moment.
  auto DisableAll = [&StructsWithConstArrays, I]() {
    for (auto StructData : StructsWithConstArrays) {
      StructData.second->clean();
      delete StructData.second;
    }
    StructsWithConstArrays.clear();
    LLVM_DEBUG({
      dbgs() << "Disabling collection for arrays with constant entries\n";
      dbgs() << "  Reason: memfunc operating over an array\n";
      dbgs() << "  Instruction: " << *I << "\n";
      dbgs() << "  Function: " << I->getFunction()->getName() << "\n";
    });

    DEBUG_WITH_TYPE(DTRANS_ARRCONST_VERBOSE, {
      dbgs() << "Disabling collection for arrays with constant entries\n";
      dbgs() << "  Reason: memfunc operating over an array\n";
      dbgs() << "  Instruction: " << *I << "\n";
      dbgs() << "  Function: " << I->getFunction()->getName() << "\n";
    });
  };

  if (!I)
    return;

  auto *CInfo = DTInfo->getCallInfo(I);
  if (!CInfo)
    return;

  if (CInfo->getCallInfoKind() != dtrans::CallInfo::CIK_Memfunc)
    return;

  dtrans::MemfuncCallInfo *MemFuncInfo = cast<dtrans::MemfuncCallInfo>(CInfo);
  auto &MemRefTypes = MemFuncInfo->getElementTypesRef();

  if (MemRefTypes.getNumTypes() == 0)
    return;

  // If the callsite refers to multiple types, then disable the constant
  // information for all types. There should be only one type for the
  // call site.
  if (MemRefTypes.getNumTypes() != 1) {
    bool ProcDisable = false;
    for (auto &TypeRef : MemRefTypes.getElemTypes()) {
      if (llvm::StructType *Struct = dyn_cast<StructType>(TypeRef)) {
        if (auto *StructData = getStructWithArrayFields(Struct, DTInfo,
                                                        StructsWithConstArrays))
          StructData->disableStructure();

      } else if (llvm::ArrayType *ArrTy = dyn_cast<ArrayType>(TypeRef)) {
        // TODO: This is conservative, we can expand the analysis to check if
        // the current type, which is an array of integers, is the field of a
        // structure and just disable that field.
        if (ArrTy->getElementType()->isIntegerTy()) {
          ProcDisable = true;
          break;
        }
      }
    }

    if (ProcDisable) {
      DisableAll();
      *ProcessDisabled = true;
    }
    return;
  }

  llvm::Type *MemFuncType = MemRefTypes.getElemType(0);
  if (llvm::ArrayType *ArrTy = dyn_cast<ArrayType>(MemFuncType)) {
    // TODO: This is conservative, we can expand the analysis to check if
    // the current type, which is an array of integers, is the field of a
    // structure and just disable that field.
    if (ArrTy->getElementType()->isIntegerTy()) {
      DisableAll();
      *ProcessDisabled = true;
    }
    return;
  }

  llvm::StructType *Struct = dyn_cast<StructType>(MemFuncType);
  if (!Struct)
    return;

  auto *StructData =
      getStructWithArrayFields(Struct, DTInfo, StructsWithConstArrays);
  if (!StructData || StructData->isStructureDisabled())
    return;

  // We don't need to check memcpy or memmove, since we know that the source
  // and destination types are the same in this case. For memset we need to
  // check that the pointer to the structure is being set to 0.
  if (MemFuncInfo->getMemfuncCallInfoKind() ==
      dtrans::MemfuncCallInfo::MK_Memset) {
    llvm::CallBase *Call = cast<CallBase>(I);
    llvm::ConstantInt *Const = dyn_cast<ConstantInt>(Call->getArgOperand(1));
    if (!Const || !Const->isZero())
      StructData->disableStructure();
  }
}

// Walk through the functions in the module and collect the structures with
// fields that are arrays with constant entries.
static void collectData(Module &M, DTransAnalysisInfo *DTInfo,
                        DenseMap<llvm::StructType *, StructWithArrayFields *>
                            &StructsWithConstArrays) {

  for (auto &F : M) {
    for (auto &II : instructions(F)) {
      if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&II)) {
        // We only care about direct access to the fields (GEPs) since
        // any indirect access was covered in the DTrans analysis. These
        // indirect access are related to zero element access.
        analyzeGEPInstruction(GEP, DTInfo, StructsWithConstArrays);
      } else if (isa<StoreInst>(&II) || isa<LoadInst>(&II)) {
        // Check if the load or store instruction can affect a field of
        // interest
        analyzeLoadOrStoreInstruction(&II, DTInfo, StructsWithConstArrays);
      } else if (isa<CallBase>(&II)) {
        // Check if the memfunc can affect a field of interest.
        // TODO: The variable ProcessDisabled is a conservative check in
        // case the memfunc affects an array of integers. We can expand the
        // analysis to disable only the field.
        bool ProcessDisabled = false;
        analyzeCallInstruction(&II, DTInfo, StructsWithConstArrays,
                               &ProcessDisabled);
        if (ProcessDisabled)
          return;
      }
    }
  }
}

// Return true if all the data related to loading and storing of constant values
// are set correctly for the input field. Else, return false
static bool analyzeField(FieldWithConstantArray *Field) {
  if (!Field)
    return false;

  if (Field->isFieldDisabled())
    return false;

  if (Field->getGEPsUsedForStore().empty())
    return false;

  auto &ConstantEntries = Field->getConstantEntires();
  if (Field->getLoadInstructions().empty() || ConstantEntries.empty())
    return false;

  if (!Field->getIntegerType())
    return false;

  // Check if at least one entry is constant.
  for (auto &ConstEntry : ConstantEntries) {
    if (ConstEntry.first && ConstEntry.second)
      return true;
  }

  return false;
}

// Given a StructType, go through the fields that are structures and disable
// the constant data for them. Recurse to disable the information for nested
// structures.
static void
cascadeBadResult(llvm::StructType *StructTy,
                 SetVector<llvm::StructType *> VisitedStructs,
                 DenseMap<llvm::StructType *, StructWithArrayFields *>
                     &StructsWithConstArrays) {

  if (!StructTy || StructsWithConstArrays.empty())
    return;

  if (!VisitedStructs.insert(StructTy))
    return;

  auto StructsWithConstArraysEnd = StructsWithConstArrays.end();

  // Traverse through the fields
  for (unsigned I = 0, E = StructTy->getNumElements(); I < E; I++) {
    llvm::StructType *FieldStruct =
        dyn_cast<llvm::StructType>(StructTy->getElementType(I));
    if (!FieldStruct)
      continue;

    if (StructsWithConstArrays.find(FieldStruct) == StructsWithConstArraysEnd)
      continue;

    // Set the structure information for the current field as
    // disabled
    auto FieldStructData = StructsWithConstArrays[FieldStruct];
    if (!FieldStructData->isStructureDisabled()) {
      DEBUG_WITH_TYPE(DTRANS_ARRCONST_VERBOSE, {
        dbgs() << "  Removing: " << FieldStruct->getName() << "\n"
               << "  Reason: Cascading bad results from " << StructTy->getName()
               << "\n\n";
      });
      FieldStructData->disableStructure();
    }

    // Recurse
    cascadeBadResult(FieldStruct, VisitedStructs, StructsWithConstArrays);
  }
}

// Return false if the input StructInfo doesn't pass the safety test
// for arrays with constant entries, or if at least one field in
// the structure is address taken. Else, return true.
static bool checkStructure(dtrans::StructInfo *StrInfo) {
  if (!StrInfo)
    return false;

  for (auto &FI : StrInfo->getFields())
    if (FI.isAddressTaken())
      return false;

  return true;
}

// This function is used for analyzing the related types. Related types are
// types that have a base form and a padded form. For example:
//
//   %class.A.base = type <{ %"class.boost::array", [2 x i8],
//                           %"class.std::vector", i32 }>
//   %class.A = type <{ %"class.boost::array", [2 x i8],
//                      %"class.std::vector", i32, [4 x i8] }>
//
// In the example above, both types are the same, one is used as base for
// nested structures (%class.A.base) and the other one is used when there is
// an instantiation (%class.A).
//
// The following function will return false if the relationship mentioned
// above invalidates the data for arrays with constant entries in the input
// StructWithArrayFields. Else, return true.
static bool
analyzeRelatedType(dtrans::StructInfo *RelatedInfo,
                   StructWithArrayFields *CurrStructData,
                   DenseMap<llvm::StructType *, StructWithArrayFields *>
                       &StructsWithConstArrays) {
  if (!RelatedInfo || StructsWithConstArrays.empty())
    return false;

  if (!checkStructure(RelatedInfo))
    return false;

  llvm::StructType *RelatedType =
      dyn_cast<llvm::StructType>(RelatedInfo->getLLVMType());
  assert(RelatedType && "Related Type without a structure type");

  // If the input related type is not in the map StructsWithConstArrays,
  // then there is nothing to check
  if (StructsWithConstArrays.find(RelatedType) == StructsWithConstArrays.end())
    return true;

  auto RelatedStructData = StructsWithConstArrays[RelatedType];

  // If the data for the related structure is disabled then cascade
  // the bad results and return false.
  if (RelatedStructData->isStructureDisabled()) {
    return false;
  }

  // Go through the fields in the data for RelatedType, if there is
  // any information in that field, then invalidate the same field
  // in CurrStructData.
  // NOTE: This is conservative, we might be able to add an extra
  // analysis to merge the information.
  for (auto RelatedField : RelatedStructData->getFields()) {
    auto *FieldNum = RelatedField->getFieldNumber();
    auto *Field = CurrStructData->getField(FieldNum);
    if (Field) {
      DEBUG_WITH_TYPE(DTRANS_ARRCONST_VERBOSE, {
        llvm::StructType *CurrStruct =
            cast<llvm::StructType>(CurrStructData->getStructure());
        dbgs() << "  Removing Field: " << FieldNum->getZExtValue() << " from "
               << RelatedType->getName() << "\n"
               << "  Reason: Mismatch with related type "
               << CurrStruct->getName() << "\n\n";
      });
      RelatedField->disableField();
      Field->disableField();
    }
  }

  return true;
}

// Analyze the structures collected with fields that could be arrays with
// constant entries. A structure will be invalidated if:
//
//   * It doesn't pass the safety checks
//   * If a structure doesn't pass the analysis then the inner structures
//     will be invalidated
//   * The information collected for the related type invalidates the
//     current structure.
//   * All the fields in the structure aren't candidates for arrays with
//     constant entries.
static void analyzeData(DTransAnalysisInfo *DTInfo,
                        DenseMap<llvm::StructType *, StructWithArrayFields *>
                            &StructsWithConstArrays) {

  auto StructsWithConstArraysEnd = StructsWithConstArrays.end();
  SetVector<StructWithArrayFields *> StructsNotNeeded;

  DEBUG_WITH_TYPE(DTRANS_ARRCONST_VERBOSE,
                  { dbgs() << "Analyzing results:\n"; });

  for (auto &StructureData : StructsWithConstArrays) {

    dtrans::StructInfo *STInfo = dyn_cast_or_null<dtrans::StructInfo>(
        DTInfo->getTypeInfo(StructureData.first));

    // If there is no DTrans information for the structure then disable
    // the data
    if (!STInfo)
      StructureData.second->disableStructure();

    // If the structure is marked as disabled then we don't need to check it,
    // but we need to disable the inner structures
    if (StructureData.second->isStructureDisabled()) {
      SetVector<llvm::StructType *> VisitedStructs;
      cascadeBadResult(StructureData.first, VisitedStructs,
                       StructsWithConstArrays);
      continue;
    }

    // If the structure doesn't pass any of the safety information then
    // disable it.
    if (!checkStructure(STInfo)) {
      DEBUG_WITH_TYPE(DTRANS_ARRCONST_VERBOSE, {
        dbgs() << "  Removing: " << StructureData.first->getName() << "\n"
               << "  Reason: Structure didn't pass check\n\n";
      });
      StructureData.second->disableStructure();
      SetVector<llvm::StructType *> VisitedStructs;
      cascadeBadResult(StructureData.first, VisitedStructs,
                       StructsWithConstArrays);
      continue;
    }

    // Check if there is any information in the related type that could
    // invalidate current type
    auto *RelatedInfo = STInfo->getRelatedType();
    if (RelatedInfo && !analyzeRelatedType(RelatedInfo, StructureData.second,
                                           StructsWithConstArrays)) {
      DEBUG_WITH_TYPE(DTRANS_ARRCONST_VERBOSE, {
        dbgs() << "  Removing: " << StructureData.first->getName() << "\n"
               << "  Reason: Structure didn't pass the related types check\n\n";
      });
      StructureData.second->disableStructure();
      SetVector<llvm::StructType *> VisitedStructs;
      cascadeBadResult(StructureData.first, VisitedStructs,
                       StructsWithConstArrays);
      continue;
    }

    // If all fields are disabled, then disable the structure. This is some
    // form of filtering.
    bool AllFieldsDisabled = true;
    llvm::StructType *CurrStruct =
        cast<llvm::StructType>(STInfo->getLLVMType());
    for (auto *Field : StructureData.second->getFields()) {

      if (Field->isFieldDisabled())
        continue;

      // If the field is a structure, and the structure is marked as disabled
      // then disable the field
      unsigned FieldNum = Field->getFieldNumber()->getZExtValue();

      llvm::StructType *FieldStruct =
          dyn_cast<llvm::StructType>(CurrStruct->getElementType(FieldNum));
      bool StructInfoDisabled = false;
      if (FieldStruct &&
          StructsWithConstArrays.find(FieldStruct) != StructsWithConstArraysEnd)
        StructInfoDisabled =
            StructsWithConstArrays[FieldStruct]->isStructureDisabled();

      if (StructInfoDisabled || !analyzeField(Field)) {
        DEBUG_WITH_TYPE(DTRANS_ARRCONST_VERBOSE, {
          dbgs() << "  Removing Field: " << FieldNum << " from "
                 << CurrStruct->getName() << "\n"
                 << "  Reason: Field didn't pass checks\n\n";
        });
        Field->disableField();
        continue;
      }
      AllFieldsDisabled = false;
    }

    // If all fields are disabled, then we don't need the structure. We don't
    // mark it as disabled inside this loop, since the structure is not wrong
    // and it could invalidate the analysis for other structures we collected.
    // This basically means that the structure won't be needed.
    if (AllFieldsDisabled)
      StructsNotNeeded.insert(StructureData.second);
  }

  // NOTE: We don't need to cascade the results here because there is nothing
  // wrong with the structures. This check is for filtering those structures
  // where all the fields don't qualify as array with constant entries.
  for (auto *Struct : StructsNotNeeded) {
    DEBUG_WITH_TYPE(DTRANS_ARRCONST_VERBOSE, {
      dbgs() << "  Removing: " << Struct->getStructure()->getName() << "\n"
             << "  Reason: None of the fields qualify as array with "
             << "constant entries\n\n";
    });
    Struct->disableStructure();
  }

  DEBUG_WITH_TYPE(DTRANS_ARRCONST_VERBOSE, {
    dbgs() << "\n";
    for (auto &Struct : StructsWithConstArrays) {
      if (!Struct.second->isStructureDisabled())
        dbgs() << "  Structure: " << Struct.first->getName() << " Pass\n";
    }
    dbgs() << "\n";
  });
}

// Insert the information collected for arrays with constant values into
// the DTransAnalysisInfo
static void
insertConstantData(DTransAnalysisInfo *DTInfo,
                   DenseMap<llvm::StructType *, StructWithArrayFields *>
                       &StructsWithConstArrays) {

  if (!DTInfo || StructsWithConstArrays.empty())
    return;

  for (auto &Struct : StructsWithConstArrays) {
    if (Struct.second->isStructureDisabled())
      continue;

    auto *STInfo =
        dyn_cast<dtrans::StructInfo>(DTInfo->getTypeInfo(Struct.first));
    assert(STInfo && "Inserting constant data in empty information");

    for (auto *Field : Struct.second->getFields()) {
      if (Field->isFieldDisabled())
        continue;
      unsigned FieldNum = Field->getFieldNumber()->getZExtValue();
      auto &FieldInfo = STInfo->getField(FieldNum);
      for (auto ConstEntry : Field->getConstantEntires()) {
        if (ConstEntry.first && ConstEntry.second)
          FieldInfo.addConstantEntryIntoTheArray(
              cast<Constant>(ConstEntry.first),
              cast<Constant>(ConstEntry.second));
      }
    }
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Print the results stored in StructsWithConstArrays. If PrintAll is true,
// then it will print all the information in StructsWithConstArrays. If it
// is false, then it will only print the structures with fields that
// are arrays with constant entries.
void printResults(DenseMap<llvm::StructType *, StructWithArrayFields *>
                      &StructsWithConstArrays,
                  bool PrintAll) {

  if (StructsWithConstArrays.empty()) {
    dbgs() << " No structure found\n";
    return;
  }

  dbgs() << "\n";
  bool NoStructs = false;
  for (auto Struct : StructsWithConstArrays) {
    if (Struct.second->isStructureDisabled() && !PrintAll)
      continue;
    Struct.second->printStructureSimple(PrintAll);
    NoStructs = true;
    dbgs() << "\n";
  }

  if (!NoStructs)
    dbgs() << " No structure found\n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Actual implementation of constant arrays metadata
void dtrans::DtransArraysWithConstant::runArraysWithConstAnalysis(
    Module &M, DTransAnalysisInfo *DTInfo) {

  if (!DTInfo)
    return;

  DenseMap<llvm::StructType *, StructWithArrayFields *>
      TempStructsWithConstArrays;

  LLVM_DEBUG({
    dbgs() << "Analyzing fields that are arrays with constant entries\n\n";
  });

  DEBUG_WITH_TYPE(DTRANS_ARRCONST_VERBOSE, {
    dbgs() << "Analyzing fields that are arrays with constant entries "
           << "(verbose print)\n\n";
  });

  // Collect the structures with possible fields that could be arrays with
  // constant entries
  collectData(M, DTInfo, TempStructsWithConstArrays);

  DEBUG_WITH_TYPE(DTRANS_ARRCONST_VERBOSE, {
    dbgs() << "Result after data collection:";
    printResults(TempStructsWithConstArrays, true /* PrintAll */);
  });

  if (TempStructsWithConstArrays.empty())
    return;

  // Check if there is any information that could invalidate
  // any possible candidate for arrays with constant entries
  analyzeData(DTInfo, TempStructsWithConstArrays);

  // Insert the information collected into the DTransAnalysis.
  insertConstantData(DTInfo, TempStructsWithConstArrays);

  LLVM_DEBUG({
    dbgs() << "Final result for fields that are arrays with "
           << "constant entries: ";
    printResults(TempStructsWithConstArrays, false /* PrintAll */);
  });

  DEBUG_WITH_TYPE(DTRANS_ARRCONST_VERBOSE, {
    dbgs() << "Final result for fields that are arrays with "
           << "constant entries: ";
    printResults(TempStructsWithConstArrays, false /* PrintAll */);
  });

  // Clean data since it isn't needed anymore
  for (auto &Struct : TempStructsWithConstArrays) {
    Struct.second->clean();
    delete Struct.second;
  }

  TempStructsWithConstArrays.clear();
}

} // namespace dtrans
} // namespace llvm
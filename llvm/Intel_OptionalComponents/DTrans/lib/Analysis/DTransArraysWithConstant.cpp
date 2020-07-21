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

namespace llvm {

namespace dtrans {

static ConstantInt *getIndexAccessedFromGEP(GetElementPtrInst *GEP);

// Implementation of the helper class FieldWithConstantArray

// Set the GEP that accesses the current field, and will be used for storing
// the constant values. If a GEPForStore is already set then we disable
// the field for the constant array optimization.
void dtrans::FieldWithConstantArray::setGEPForStore(
    GetElementPtrInst *GEPStore) {
  // If GEPForStore is already set then disable the field for array with
  // constant entries. (NOTE: This is conservative, we can assign multiple
  // stores in different places if they don't touch the same index).
  if (GEPForStore) {
    disableField();
    return;
  }
  // The field number that is being accessed should match.
  ConstantInt *FieldNum = getIndexAccessedFromGEP(GEPStore);
  if (FieldNum != FieldNumber) {
    disableField();
    return;
  }
  GEPForStore = GEPStore;
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
  if (!InType)
    return false;
  if (!IntType) {
    IntType = InType;
    return true;
  }
  return IntType == InType;
}

// Clean all the information stored for the field and disable it for
// storing any constant information
void dtrans::FieldWithConstantArray::disableField() {
  ConstantEntries.clear();
  GEPForStore = nullptr;
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
  if (ConstantEntries.empty()) {
    dbgs() << " No constant data found";
  } else {
    dbgs() << "\n";
    for (auto ConstantEntry : ConstantEntries) {
      if (ConstantEntry.second)
        dbgs() << "      Index: " << *ConstantEntry.first
               << "      Value: " << *ConstantEntry.second << "\n";
    }
  }
  dbgs() << "\n    GEP used for store: ";
  if (GEPForStore)
    dbgs() << *GEPForStore;
  else
    dbgs() << "      No GEP for store found";
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
  if (ConstantEntries.empty()) {
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
void dtrans::StructWithArrayFields::printStructureSimple() {
  dbgs() << "Type: " << *Struct << "\n";
  dbgs() << "  Is structure available: " << (!StructureDisabled ? "YES" : "NO")
         << "\n";
  if (FieldsWithConstArrayVector.empty())
    dbgs() << "  Empty data\n";
  else
    for (auto *Field : FieldsWithConstArrayVector)
      Field->printFieldSimple();
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
  if (NumIndices != 2 || !match(GEP->getOperand(1), m_Zero()))
    return nullptr;

  // Return the constant integer
  return cast<ConstantInt>(GEP->getOperand(2));
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

// Return true if the GEP is used to access an entry in an array of integers.
// Also, this integer type must match with integer type stored in FieldData.
static bool analyzeGEP(GetElementPtrInst *GEP,
                       FieldWithConstantArray *FieldData) {
  if (!GEP || !FieldData)
    return false;

  // The GEP must be accessing an entry in an array of integers
  llvm::ArrayType *SrcTy =
      dyn_cast<llvm::ArrayType>(GEP->getSourceElementType());
  llvm::IntegerType *ResType =
      dyn_cast<llvm::IntegerType>(GEP->getResultElementType());

  if (!SrcTy || !ResType) {
    FieldData->disableField();
    return false;
  }

  if (!FieldData->setIntegerType(ResType)) {
    // If the type couldn't be set or compared with the type in the
    // the field, then invalidate the field
    FieldData->disableField();
    return false;
  }

  return true;
}

// Return true in the input store instruction doesn't invalidate the input
// field or structure, and is used to store an integer in an array. The GEP
// is used to find which index is being accessed in the array.
static bool checkStoreInst(StoreInst *Store, FieldWithConstantArray *FieldData,
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

  llvm::ArrayType *SrcTy =
      dyn_cast<llvm::ArrayType>(EntryGEP->getSourceElementType());
  unsigned IndexAccessed = Index->getZExtValue();
  if (!analyzeGEP(EntryGEP, FieldData)) {
    FieldData->disableField();
    return false;
  }

  if (IndexAccessed > SrcTy->getNumElements()) {
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

// Return true if the input load instruction doesn't invalidate the information
// for the field and the structure, else return false. The parameter EntryGEP
// is used to check if the GEP is accessing an entry in a array of integers.
static bool checkLoadInst(LoadInst *Load, FieldWithConstantArray *FieldData,
                          StructWithArrayFields *StructData,
                          GetElementPtrInst *EntryGEP) {
  if (!Load || !FieldData || !StructData || !EntryGEP)
    return false;

  if (!analyzeGEP(EntryGEP, FieldData)) {
    FieldData->disableField();
    return false;
  }

  // NOTE: We don't check for constant entries because there is a chance that
  // we are loading variable entries in the array (e.g. load used inside a
  // loop).

  FieldData->insertLoadInstruction(Load);
  return true;
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
                                    StructWithArrayFields *StructData,
                                    DTransAnalysisInfo *DTInfo) {
  if (!GEP)
    return;

  // If the structure is disabled then there is nothing to do
  if (!StructData || StructData->isStructureDisabled())
    return;

  // Collect the field that is being accessed.
  ConstantInt *FieldNumConst = getIndexAccessedFromGEP(GEP);
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
  // is the GetElementPtrInst that accesses the entry in the array. In this case,
  // GEP will be %tmp0 and ArrayGEP will be %tmp1. If nothing is found then
  // ArrayGEP will be nullptr. This is to catch special cases like boost
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

      // Each user of the GEP that accesses the entries in the array must
      // be a load or store instruction.
      for (User *UserEntry : EntryGEP->users()) {
        if (auto *Store = dyn_cast<StoreInst>(UserEntry)) {
          if (!checkStoreInst(Store, FieldData, StructData, EntryGEP))
            return;
          StoreFound = true;
        } else if (auto *Load = dyn_cast<LoadInst>(UserEntry)) {
          if (!checkLoadInst(Load, FieldData, StructData, EntryGEP))
            return;
          LoadFound = true;
        } else {
          FieldData->disableField();
          return;
        }
        // This is conservative, for now all the stores must happen in one
        // function and all the loads must happen in other functions. In
        // reality we can have load and stores mixed together.
        if (StoreFound && LoadFound)
          FieldData->disableField();
      }
    } else {
      FieldData->disableField();
      return;
    }
  }

  // The GEP is used for storing data in an array
  if (StoreFound && !LoadFound)
    FieldData->setGEPForStore(GEP);
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
  llvm::Type *GEPType = GEP->getOperand(0)->getType();
  if (GEPType->isPointerTy())
    Structure = dyn_cast<llvm::StructType>(GEPType->getPointerElementType());
  if (!Structure)
    return;

  StructWithArrayFields *StructData =
      getStructWithArrayFields(Structure, DTInfo, StructsWithConstArrays);
  if (!StructData || StructData->isStructureDisabled())
    return;

  // Check if we can collect any load or store to an array of integers,
  // which is a field of the structure Structure. If so, then
  // update the information for the structure, else invalidate the
  // information for the field or the structure.
  checkAndUpdateStructure(GEP, StructData, DTInfo);
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

// Actual implementation of constant arrays metadata
void dtrans::DtransArraysWithConstant::runArraysWithConstAnalysis(
    Module &M, DTransAnalysisInfo *DTInfo) {

  if (!DTInfo)
    return;

  DenseMap<llvm::StructType *, StructWithArrayFields *>
      TempStructsWithConstArrays;

  // Collect the structures with possible fields that could be arrays with
  // constant entries
  collectData(M, DTInfo, TempStructsWithConstArrays);

  LLVM_DEBUG({
    dbgs() << "Result after data collection:";
    if (TempStructsWithConstArrays.empty()) {
      dbgs() << " No structure found\n";
    } else {
      dbgs() << "\n";
      for (auto Struct : TempStructsWithConstArrays) {
        Struct.second->printStructureSimple();
        dbgs() << "\n";
      }
    }
  });

  // TODO:
  //   1) Add the quick analysis to filter related types
  //   2) Filter TempStructsWithConstArrays to keep those
  //      structures we actually need in StructsWithConstArrays
  //   3) Write the results back to DTInfo
}

} // namespace dtrans
} // namespace llvm
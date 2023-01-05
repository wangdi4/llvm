//===----  DTransArraysWithConstant.h - Arrays with constant entries  -----===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
// This header file contains helper classes used for checking if a field in
// an structure is an array with constant entries. If so, then collect the
// constant values and store them in the FieldInfo for the DtransInfo analysis.
// This information will be passed to the DTransImmutable analysis, which
// can be used by the loop optimizer, constant propagation, or any other
// transformation that depends on constant values and DTrans.
//===---------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransArraysWithConstant.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSARRAYSWITHCONSTANT_H
#define INTEL_DTRANS_ANALYSIS_DTRANSARRAYSWITHCONSTANT_H

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

namespace dtrans {

// Helper class used for collecting the information related to a field
// that could be an array with constant entries. For example:
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
// The instruction %tmp1 in function @foo, from the example above, is a GEP
// pointing to field 1 in the structure %class.TestClass. This field is an
// array of integers. Then, 1 and 2 are stored in entries 0 and 1 respectively.
// This class will create the information related to indicate that field 1 in
// %class.TestClass is an array where entry 0 is a constant integer with value
// 1 and entry 1 is 2.
class FieldWithConstantArray {
public:
  FieldWithConstantArray(ConstantInt *FieldNumber)
      : FieldNumber(FieldNumber), FieldDisabled(false),
        IntType(nullptr) {}

  ~FieldWithConstantArray() { clean(); }

  // Add the GEP that accesses the current field, and will be used for storing
  // the constant values.
  void addGEPForStore(GetElementPtrInst *GEPStore);

  // Given the constant integers Index and ConstValue, pair them and
  // add them in ConstantEntries. If the Index is already is inserted
  // then ConstValue must match with the constant value, else set it
  // as nullptr. If Index is null it means that we don't have any
  // information of which field is being accessed, disable the whole field.
  // If ConstValue is nullptr then it means that Index is not constant.
  void addConstantEntry(ConstantInt *Index, ConstantInt *ConstValue);

  // Return true if the Integer type was set or is the same as the input
  // InType, else return false.
  bool setIntegerType(llvm::IntegerType *InType);

  // Insert a new Load instruction
  void insertLoadInstruction(LoadInst *Load) { LoadInstructions.insert(Load); }

  SetVector<GetElementPtrInst *>
      &getGEPsUsedForStore() { return GEPsForStore; }
  IntegerType *getIntegerType() { return IntType; }
  ConstantInt *getFieldNumber() { return FieldNumber; }
  SmallVectorImpl< std::pair<ConstantInt *, ConstantInt *> >
      &getConstantEntires() { return ConstantEntries; }
  SetVector<LoadInst *> &getLoadInstructions() { return LoadInstructions; }

  // If we find something that is not a load or a store from the array
  // in the field then disable it. For example:
  //
  //   %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
  //           i64 0, i32 12
  //   %tmp2 = call i32 @baz([4 x i32]* %tmp1)
  //
  // In the example above, the field result from the GEP is passed to a call
  // instruction. We don't have enough information what will happen here,
  // therefore disable the field.
  bool isFieldDisabled() { return FieldDisabled; }
  void disableField();

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Print all the information collected. This is useful for debugging.
  void printFieldFull();

  // Print a simplified version of the information for the field
  void printFieldSimple();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // Destroy the data
  void clean() {
    FieldNumber = nullptr;
    ConstantEntries.clear();
    GEPsForStore.clear();
    LoadInstructions.clear();
    FieldDisabled = true;
    IntType = nullptr;
  }

private:
  // NOTE: ConstantInt was used rather than unsigned or integers because it is
  // easier to reuse this information when inserting the information in
  // DTransInfo and DTransImmutable.

  // A ConstantInt* that identifies the field number (i32 12 from the example)
  ConstantInt *FieldNumber;

  // A SmallVector of std::pair, which pairs an array index (ConstantInt*) with
  // a constant value (ConstantInt*). From the example, it pairs index 0 with
  // value 1 ({i64 0, i32 1}).
  SmallVector<std::pair<ConstantInt *, ConstantInt *>, 8> ConstantEntries;

  // SetVector that holds the GEPs used by StoreInst instructions to store
  // constants in the array. From the example, %tmp1 in @foo.
  SetVector<GetElementPtrInst *> GEPsForStore;

  // Store the load instructions that are used for collecting the entry in
  // the array
  SetVector<LoadInst *> LoadInstructions;

  // If there is a use rather than load and store for the field that disables
  // the constant information, this will be true.
  bool FieldDisabled;

  // The integer type that is being stored in the array.
  IntegerType *IntType;

  // Return true if at least one entry in ConstantEntries is not nullptr
  bool hasAtLeastOneConstantEntry() {
    for (const auto &Entry : ConstantEntries)
      if (Entry.second)
        return true;
    return false;
  }
};

// Helper class to handle the structures with possible fields that could be
// arrays with constant entries.
class StructWithArrayFields {
public:
  StructWithArrayFields(llvm::StructType *Struct)
      : Struct(Struct), StructureDisabled(false) {}

  ~StructWithArrayFields() { clean(); }

  llvm::StructType *getStructure() { return Struct; }

  // Return true if a new field that could be an array with constant entries
  // was added successfully, else return false.
  bool addField(FieldWithConstantArray *NewField);

  // Given a field number return the field information stored.
  FieldWithConstantArray *getField(ConstantInt *FieldNumber);

  auto &getFields() { return FieldsWithConstArrayVector; }

  // Disable the structure if we find something that couldn't be analyzed.
  void disableStructure();
  bool isStructureDisabled() { return StructureDisabled; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Print the structure with the full information for all fields
  void printStructureFull();

  // Print the structure with the fields information simplified. If PrintAll
  // is true then all the structures collected will be printed, even
  // structures that were invalidated.
  void printStructureSimple(bool PrintAll);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // Destroy the data
  void clean() {
    Struct = nullptr;
    for (auto *Field : FieldsWithConstArrayVector) {
      Field->clean();
      delete Field;
    }
    StructureDisabled = true;
    FieldsWithConstArrayVector.clear();
  }

private:
  // LLVM StructType for the current structure
  llvm::StructType *Struct;

  // True if we can't use any of the fields in the structure for the
  // analysis
  bool StructureDisabled;

  // SetVector that holds the possible fields that could be arrays with
  // constant entries.
  SetVector<FieldWithConstantArray *> FieldsWithConstArrayVector;
};

// Helper class to collect constant entries in an array
class DtransArraysWithConstant {

public:
  DtransArraysWithConstant() {}
  void runArraysWithConstAnalysis(Module &M, DTransAnalysisInfo *DTInfo);
};

} // namespace dtrans
} // namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSARRAYSWITHCONSTANT_H

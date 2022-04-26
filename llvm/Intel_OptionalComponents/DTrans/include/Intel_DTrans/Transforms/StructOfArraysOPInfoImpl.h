//==StructOfArraysOPInfoImpl.h - common for SOAToAOSOP and MemInitTrimDownOP==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file is mainly used to collect candidates and their info for
// Initial Memory Allocation Trim Down optimization pass in both pre-LTO and
// LTO phases. In Pre-LTO phase, this is used to disable inlining for
// member functions that are related to potential candidates.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_STRUCTOFARRAYSOPINFOIMPL_H
#define INTEL_DTRANS_TRANSFORMS_STRUCTOFARRAYSOPINFOIMPL_H

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/Transforms/ClassInfoOPUtils.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"

#define DTRANS_STRUCTOFARRAYSOPINFO "dtrans-structofarraysopinfo"

namespace llvm {

class DominatorTree;

namespace dtransOP {

using SOADominatorTreeType = std::function<DominatorTree &(Function &)>;
using SOAGetTLITy = std::function<const TargetLibraryInfo &(const Function &)>;

// This is used to collect candidate for SOAToAOS or MemInitTrimDown and
// maintain information related to the candidate.
class SOACandidateInfo {

  // Max limit for number of fields in candidate struct.
  constexpr static int MaxNumElemsInCandidate = 4;

  // Min limit for number of fields in candidate struct.
  constexpr static int MinNumElemsInCandidate = 3;

  // Max limit: Number methods for candidate struct.
  constexpr static int MaxNumStructMethods = 16;

  // Max limit: Number methods for candidate vector field class.
  constexpr static int MaxNumVectorMethods = 16;

  // Min limit: Number methods for candidate vector field class.
  constexpr static int MinNumVectorMethods = 4;

  // Max limit: Number of BBs in methods of vector field class.
  constexpr static int MaxNumVectorMethodBBlocks = 32;

  // Min limit for number of fields in candidate struct.
  constexpr static int MinNumCandidateVectors = 2;

public:
  SOACandidateInfo(TypeMetadataReader &MDReader) : MDReader(MDReader){};
  inline bool isCandidateType(DTransType *Ty);
  inline DTransType *isSimpleVectorType(DTransType *STy, int32_t Offset,
                                        bool AllowOnlyDerived);
  inline bool collectMemberFunctions(Module &M, bool AtLTO = true);
  inline void collectFuncs(Module &M, SmallSet<Function *, 32> *SOACallSites);
  inline void printCandidateInfo(void);

  using FieldPositionTy = SmallVector<int32_t, MaxNumElemsInCandidate>;
  using StructMethodSetTy = SmallSetVector<Function *, MaxNumStructMethods>;
  using VectorMethodSetTy = SmallSetVector<Function *, MaxNumVectorMethods>;
  using VectorFieldTypeSetTy = SmallSetVector<DTransType *, 2>;

  // Returns SType.
  inline DTransStructType *getStructTy() { return SType; }

  // Returns MemInterfaceType.
  inline DTransStructType *getMemInterfaceType() { return MemInterfaceType; }

  // Returns array element type of candidate field at FI index.
  inline DTransType *getFieldElemTy(int32_t FI) {
    return CandidateFieldElemTy[FI];
  }

  // Returns true if DTy is class type of any candidate fields.
  inline bool isCandidateFieldDerivedTy(DTransType *DTy) {
    return CandidateFieldDerivedTy.count(DTy);
  }

  // Iterator for candidate field positions.
  typedef FieldPositionTy::const_iterator f_const_iterator;
  inline iterator_range<f_const_iterator> candidate_fields() {
    return make_range(CandidateFieldPositions.begin(),
                      CandidateFieldPositions.end());
  }

  // Member function iterator for given candidate field position FI.
  typedef VectorMethodSetTy::const_iterator m_const_iterator;
  inline iterator_range<m_const_iterator> member_functions(int32_t FI) {
    return make_range(CandidateFieldMemberFuncs[FI].begin(),
                      CandidateFieldMemberFuncs[FI].end());
  }

  // Member function iterator for struct methods.
  inline iterator_range<m_const_iterator> struct_functions() {
    return make_range(StructMethods.begin(), StructMethods.end());
  }

  // Returns true if F is member function of candidate class
  // at field position FI.
  inline bool isMemberFunction(Function *F, int32_t FI) {
    return CandidateFieldMemberFuncs[FI].count(F);
  }

  inline bool isStructMethod(Function *F) { return StructMethods.count(F); }

private:
  // Candidate struct.
  DTransStructType *SType = nullptr;

  // Candidate struct and vector field classes all have same pointer
  // type field that is used to allocate/deallocate memory to manage
  // the vectors.
  DTransStructType *MemInterfaceType = nullptr;

  // Set of positions of fields that are potential vector field classes.
  FieldPositionTy CandidateFieldPositions;

  // Set of methods of candidate struct.
  StructMethodSetTy StructMethods;

  // Mapping between vector candidate class position and set of
  // vector class types. If candidate field class is derived from
  // a vector class, both derived and base classes are considered
  // as types of the candidate field.
  // Ex:
  //  Derived: %"RefArrayVectorOf" = type { %"BaseRefVectorOf.5" }
  //
  //  Base: %"BaseRefVectorOf.5" = type { i32 (...)**, i8, i32, i32,
  //                                      i16**, %"MemoryManager"* }
  DenseMap<int32_t, VectorFieldTypeSetTy> CandidateFieldTypeSets;

  // List of class types of candidate vector fields.
  SmallPtrSet<DTransType *, 4> CandidateFieldDerivedTy;

  // Mapping between vector candidate class position and set of
  // methods of the vector class.
  DenseMap<int32_t, VectorMethodSetTy> CandidateFieldMemberFuncs;

  // Mapping between candidate vector field location and type of array
  // element of the vector.
  DenseMap<int32_t, DTransType *> CandidateFieldElemTy;

  TypeMetadataReader &MDReader;

  inline bool isStructWithNoRealData(DTransType *Ty);
  inline DTransType *getBaseClassOfSimpleDerivedClass(DTransType *Ty);
  inline bool isVectorLikeClass(DTransType *Ty, DTransType **ElemTy);
  inline bool collectTypesIfVectorClass(DTransType *VTy, int32_t pos);
};

// Returns true if 'Ty' is a struct that doesn't have any real data
// except vftable.
// Ex:
//      %"MemoryManager" = type { i32 (...)** }
//
bool SOACandidateInfo::isStructWithNoRealData(DTransType *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy || STy->getNumFields() != 1)
    return false;
  if (!isPtrToVFTable(STy->getFieldType(0)))
    return false;
  if (!MemInterfaceType)
    MemInterfaceType = STy;
  else if (MemInterfaceType != STy)
    return false;
  return true;
}

// Returns true if 'Ty' is a struct that looks like vector type class.
//
// Ex:
//  %"ValueVectorOf.6" = type { i8, i32, i32, %"IC_Fld"**, %"MemoryManager"* }
//
bool SOACandidateInfo::isVectorLikeClass(DTransType *Ty, DTransType **ElemTy) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumDataPointers = 0;
  unsigned NumCounters = 0;
  unsigned NumFlags = 0;
  unsigned NumNoDataPointers = 0;
  unsigned NumVtablePtr = 0;
  *ElemTy = nullptr;
  for (unsigned I = 0, E = STy->getNumFields(); I != E; ++I) {
    auto *ETy = STy->getFieldType(I);
    assert(ETy && "DTransType was not initialized properly");
    if (isPotentialPaddingField(ETy))
      continue;
    if (isPtrToVFTable(ETy)) {
      NumVtablePtr++;
      continue;
    }
    if (ETy->getLLVMType()->isIntegerTy(8)) {
      NumFlags++;
      continue;
    }
    if (ETy->getLLVMType()->isIntegerTy(32)) {
      NumCounters++;
      continue;
    }
    auto *PTy = getPointeeType(ETy);
    if (!PTy)
      return false;

    if (isStructWithNoRealData(PTy)) {
      NumNoDataPointers++;
      continue;
    }
    // Allow both pointer or struct as "data". Due to SOAToAOS
    // transformation, it is possible that "pointer to data" will
    // be converted to "struct that has pointer to data".
    if (getPointeeType(PTy) || getValidStructTy(PTy)) {
      NumDataPointers++;
      *ElemTy = PTy;
      continue;
    }
    return false;
  }
  if (NumDataPointers != 1 || NumCounters != 2 || NumFlags != 1 ||
      NumNoDataPointers != 1 || NumVtablePtr > 1)
    return false;
  return true;
}

// Returns base class type if 'Ty' is derived from the base class and
// derived class doesn't have its own fields.
//
// Ex:
//  %"RefArrayVectorOf" = type { %"BaseRefVectorOf.5" }
//
DTransType *SOACandidateInfo::getBaseClassOfSimpleDerivedClass(DTransType *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return nullptr;

  if (STy->getNumFields() != 1)
    return nullptr;

  auto *ETy = STy->getFieldType(0);
  assert(ETy && "DTransType was not initialized properly");
  return ETy;
}

// Returns true if given type is candidate for MemInitTrimDown or SOAToAOS.
//
// Ex:
//   %"FieldValueMap" = type { %"ValueVectorOf.6"*, %"ValueVectorOf.7"*,
//                             %"RefArrayVectorOf"*, %"MemoryManager"* }
//
//   %"FieldValueMap" will be considered as candidate for MemInitTrimDown
//   or SOAToAOS because it has only pointer fields that point to either
//   potential vector classes (%"ValueVectorOf.6", %"ValueVectorOf.7" and
//   %"RefArrayVectorOf") or dummy class with no real data (%"MemoryManager").
//
//   %"ValueVectorOf.6", %"ValueVectorOf.7" and %"RefArrayVectorOf" classes
//   are considered as potential vector classes because they have
//    1. Two integer fields that represent values of Size and Capacity.
//    2. A Pointer field that represents element array
//    3. Dummy struct with no real data
//    4. A Bool flag
//
//   Examples of potential vector classes:
//   %"ValueVectorOf.6" = type { i8, i32, i32, %"IC_Field"**, %"MemoryManager"*
//   }
//   %"ValueVectorOf.7" = type { i8, i32, i32, float**, %"MemoryManager"* }
//   %"RefArrayVectorOf" = type { %"BaseVec" }
//   %"BaseVec" = type { i32 (...)**, i8, i32, i32, i16**, %"MemoryManager"* }
//
//   %"MemoryManager" = type { i32 (...)** }
//
//   %"MemoryManager" is considered as dummy struct with no real data.
//
bool SOACandidateInfo::isCandidateType(DTransType *Ty) {

  auto STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  auto NumElems = STy->getNumFields();
  if (NumElems > MaxNumElemsInCandidate || NumElems < MinNumElemsInCandidate)
    return false;

  int32_t Pos = -1;
  unsigned NumNoDataPointers = 0;
  size_t NumFields = STy->getNumFields();
  for (size_t Idx = 0; Idx < NumFields; ++Idx) {
    DTransType *FTy = STy->getFieldType(Idx);
    assert(FTy && "DTransType was not initialized properly");
    Pos++;
    // Ignore potential padding added to fill gaps in structs.
    // Later at LTO, check that it doesn't have any uses.
    if (isPotentialPaddingField(FTy))
      continue;

    // Expect all other fields are pointers.
    auto *VTy = getPointeeType(FTy);
    if (!VTy)
      return false;

    // Ignore if it is pointer to a struct that doesn't have any real
    // data except vtable.
    if (isStructWithNoRealData(VTy)) {
      NumNoDataPointers++;
      continue;
    }
    if (!collectTypesIfVectorClass(VTy, Pos))
      return false;
  }
  if (CandidateFieldPositions.size() < MinNumCandidateVectors) {
    DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSOPINFO, {
      dbgs() << "  Failed: doesn't have minimum candidate vectors.\n";
    });
    return false;
  }
  if (NumNoDataPointers != 1) {
    DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSOPINFO, {
      dbgs() << "  Failed: Unexpected MemoryInterface Type.\n";
    });
    return false;
  }
  SType = STy;
  return true;
}

// Collects the element type of VTy at Pos if element type is a pointer
// to vector class.
bool SOACandidateInfo::collectTypesIfVectorClass(DTransType *VTy, int32_t Pos) {
  // Check if it is simple derived class that doesn't have its own
  // fields.
  //
  // Ex:
  //  Derived: %"RefArrayVectorOf" = type { %"BaseRefVectorOf.5" }
  //
  //  Base: %"BaseRefVectorOf.5" = type { i32 (...)**, i8, i32, i32,
  //                                      i16**, %"MemoryManager"* }
  auto *DerivedTy = VTy;
  if (auto *BaseTy = getBaseClassOfSimpleDerivedClass(VTy))
    VTy = BaseTy;
  DTransType *VecElemTy;
  if (!isVectorLikeClass(VTy, &VecElemTy))
    return false;

  CandidateFieldPositions.push_back(Pos);
  CandidateFieldTypeSets[Pos].insert(DerivedTy);
  CandidateFieldDerivedTy.insert(DerivedTy);
  CandidateFieldElemTy[Pos] = VecElemTy;
  if (VTy != DerivedTy)
    CandidateFieldTypeSets[Pos].insert(VTy);
  return true;
}

// AllowOnlyDerived is true: If element type of Ty at Offset is a simple derived
// class of vector base class, it returns the derived class. Otherwise, returns
// nullptr. AllowOnlyDerived is false: If element type of Ty at Offset is a
// simple vector class, it returns type of the vector class. Otherwise, it
// returns nullptr. Ex:
//  Derived: %"RefArrayVectorOf" = type { %"BaseRefVectorOf.5" }
//
//  Base: %"BaseRefVectorOf.5" = type { i32 (...)**, i8, i32, i32,
//                                      i16**, %"MemoryManager"* }
DTransType *SOACandidateInfo::isSimpleVectorType(DTransType *Ty, int32_t Offset,
                                                 bool AllowOnlyDerived) {
  auto STy = getValidStructTy(Ty);
  if (!STy)
    return nullptr;
  auto *FTy = STy->getFieldType(Offset);
  assert(FTy && "DTransType was not initialized properly");
  auto *VTy = getPointeeType(FTy);
  if (!VTy)
    return nullptr;
  if (AllowOnlyDerived && !getBaseClassOfSimpleDerivedClass(VTy))
    return nullptr;
  if (!collectTypesIfVectorClass(VTy, Offset))
    return nullptr;
  SType = STy;
  return VTy;
}

// Collect member functions of candidate struct and member functions of
// candidate vector field classes. 'AtLTO' indicates whether this routine
// is called from LTO pass or not.
bool SOACandidateInfo::collectMemberFunctions(Module &M, bool AtLTO) {
  std::function<bool(Function * F, bool AtLTO,
                     SmallPtrSet<Function *, 32> &ProcessedFuncs)>
      CollectVectorMemberFunctions;

  // If 'F' is a member function of candidate vector field, add
  // it to the corresponding member function set.
  auto CheckVectorMemberFunction = [this](Function *F) -> void {
    if (!F)
      return;
    auto *ThisTy = getClassType(F, MDReader);
    if (!ThisTy)
      return;
    for (auto Loc : CandidateFieldPositions)
      for (auto *Ty : CandidateFieldTypeSets[Loc])
        if (Ty == ThisTy) {
          CandidateFieldMemberFuncs[Loc].insert(F);
          break;
        }
  };

  // This is a recursive function to collect member functions of all
  // candidate vector fields that can be called from 'F'. 'AtLTO'
  // indicates whether it is called from LTO pass or not.
  CollectVectorMemberFunctions =
      [&CollectVectorMemberFunctions, &CheckVectorMemberFunction](
          Function *F, bool AtLTO,
          SmallPtrSet<Function *, 32> &ProcessedFunctions) -> bool {
    if (!F || F->isDeclaration())
      return true;
    // Check if it is already processed.
    if (!ProcessedFunctions.insert(F).second)
      return true;
    for (const auto &I : instructions(F))
      if (auto *CB = dyn_cast<CallBase>(&I)) {
        auto *Callee = dtrans::getCalledFunction(*CB);

        // At LTO, only direct calls are expected in the member functions.
        if (AtLTO && !Callee) {
          DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSOPINFO, {
            dbgs() << "  Failed: No indirect call is allowed.\n";
          });
          return false;
        }
        CheckVectorMemberFunction(Callee);
        if (!CollectVectorMemberFunctions(Callee, AtLTO, ProcessedFunctions))
          return false;
      }
    return true;
  };

  // Walk through module and collect member functions of
  // candidate struct.
  for (auto &F : M) {
    // Skip unused prototypes.
    if (F.isDeclaration() && F.use_empty())
      continue;
    if (auto *CTy = getClassType(&F, MDReader))
      if (CTy == SType)
        StructMethods.insert(&F);
  }

  if (StructMethods.size() > MaxNumStructMethods) {
    DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSOPINFO, {
      dbgs() << "  Failed: Exceeding max limit for struct methods.\n";
    });
    return false;
  }

  SmallPtrSet<Function *, 32> ProcessedFunctions;
  // Collect member functions of all candidate vector fields.
  for (auto *F : StructMethods) {
    // Skip unused prototypes.
    if (F->isDeclaration() && F->use_empty())
      continue;
    if (AtLTO && F->isDeclaration()) {
      DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSOPINFO, {
        dbgs() << "  Failed: Missing definition for struct methods.\n";
      });
      return false;
    }
    if (!CollectVectorMemberFunctions(F, AtLTO, ProcessedFunctions))
      return false;
  }

  if (!AtLTO)
    return true;

  // At LTO, do more checks.
  for (auto Loc : CandidateFieldPositions) {
    if (CandidateFieldMemberFuncs[Loc].size() > MaxNumVectorMethods ||
        CandidateFieldMemberFuncs[Loc].size() < MinNumVectorMethods) {
      DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSOPINFO, {
        dbgs() << "  Failed: Unexpected number of vector methods.\n";
      });
      return false;
    }
    for (auto *F : CandidateFieldMemberFuncs[Loc]) {
      if (F->isDeclaration()) {
        DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSOPINFO, {
          dbgs() << "  Failed: Missing definition for vector method.\n";
        });
        return false;
      }
      if (F->size() > MaxNumVectorMethodBBlocks) {
        DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSOPINFO, {
          dbgs() << "  Failed: Exceeding vector method size limit.\n";
        });
        return false;
      }
      // TODO: Put limit on number of times each vector method is
      // called.
    }
  }
  return true;
}

// Collect all member functions of
//   1. Candidate Struct
//   2. All candidate array field structs (even if member functions are not
//   called directly)
//   3. In struct methods, calls that are passed as arguments to other
//      direct calls.
void SOACandidateInfo::collectFuncs(Module &M,
                                    SmallSet<Function *, 32> *SOAFuncs) {
  SmallPtrSet<DTransType *, 4> InterestedClasses;

  InterestedClasses.insert(SType);
  for (auto Loc : CandidateFieldPositions)
    for (auto *Ty : CandidateFieldTypeSets[Loc])
      InterestedClasses.insert(Ty);

  for (auto &F : M)
    if (auto *CTy = getClassType(&F, MDReader))
      if (InterestedClasses.count(CTy))
        SOAFuncs->insert(&F);

  for (auto *F : struct_functions())
    for (Instruction &I : instructions(F)) {
      auto *CB = dyn_cast<CallBase>(&I);
      if (!CB)
        continue;
      for (auto &A : CB->args()) {
        auto *ACB = dyn_cast<CallBase>(&A);
        if (!ACB)
          continue;
        auto *Callee = dtrans::getCalledFunction(*ACB);
        if (!Callee || Callee->isDeclaration())
          continue;
        SOAFuncs->insert(Callee);
      }
    }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

void SOACandidateInfo::printCandidateInfo(void) {
  dbgs() << "    Candidate: " << dtrans::getStructName(SType->getLLVMType())
         << "\n";
  for (auto Loc : CandidateFieldPositions) {
    dbgs() << " Member functions for " << Loc << " candidate field: \n";
    for (auto *F : CandidateFieldMemberFuncs[Loc])
      dbgs() << "       " << F->getName() << "\n";
    dbgs() << " \n";
  }
}

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

} // namespace dtransOP

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_STRUCTOFARRAYSOPINFOIMPL_H

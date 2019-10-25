//===--------- MemInitTrimDownInfoImpl.h - DTransMemInitTrimDownInfoImpl --===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
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

#ifndef INTEL_DTRANS_TRANSFORMS_MEMINITTRIMDOWNINFOIMPL_H
#define INTEL_DTRANS_TRANSFORMS_MEMINITTRIMDOWNINFOIMPL_H

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"

#define DTRANS_MEMINITTRIMDOWN "dtrans-meminittrimdown"

namespace llvm {

class DominatorTree;

namespace dtrans {

using MemInitDominatorTreeType = std::function<DominatorTree &(Function &)>;
using MemGetTLITy = std::function<const TargetLibraryInfo &(const Function &)>;

// Get class type of the given function if there is one.
inline StructType *getClassType(const Function *F) {
  FunctionType *FunTy = F->getFunctionType();
  if (FunTy->getNumParams() == 0)
    return nullptr;
  // Get class type from "this" pointer that is passed as 1st
  // argument.
  if (auto *PTy = dyn_cast<PointerType>(FunTy->getParamType(0)))
    if (auto *STy = dyn_cast<StructType>(PTy->getPointerElementType()))
      return STy;
  return nullptr;
}

// Returns true if Ty is pointer to pointer to a function.
inline bool isPtrToVFTable(Type *Ty) {
  Type *ETy = nullptr;
  if (auto *PPETy = dyn_cast<PointerType>(Ty))
    if (auto *PETy = dyn_cast<PointerType>(PPETy->getElementType()))
      ETy = PETy->getElementType();
  if (!ETy || !ETy->isFunctionTy())
    return false;
  return true;
}

// Returns field type of DTy struct if it has only one field.
inline Type *getMemInitSimpleBaseType(Type *DTy) {
  assert(isa<StructType>(DTy) && "Expected StructType");
  StructType *STy = cast<StructType>(DTy);
  if (STy->getNumElements() != 1)
    return nullptr;
  return STy->getElementType(0);
}

// This is used to collect candidate for MemInitTrimDown and
// maintain information related to the candidate.
class MemInitCandidateInfo {

  // Max limit for number of fields in candidate struct.
  constexpr static int MaxNumElemsInCandidate = 4;

  // Min limit for number of fields in candidate struct.
  constexpr static int MinNumElemsInCandidate = 3;

  // Max limit: Number methods for candidate struct.
  constexpr static int MaxNumStructMethods = 10;

  // Max limit: Number methods for candidate vector field class.
  constexpr static int MaxNumVectorMethods = 10;

  // Min limit: Number methods for candidate vector field class.
  constexpr static int MinNumVectorMethods = 4;

  // Max limit: Number of BBs in methods of vector field class.
  constexpr static int MaxNumVectorMethodBBlocks = 32;

  // Min limit for number of fields in candidate struct.
  constexpr static int MinNumCandidateVectors = 2;

public:
  inline bool isCandidateType(Type *Ty);
  inline Type *isSimpleDerivedVectorType(Type *STy, int32_t Offset);
  inline bool collectMemberFunctions(Module &M, bool AtLTO = true);
  inline void collectFuncs(SmallSet<Function *, 32> *MemInitCallSites);
  inline void printCandidateInfo(void);

  using FieldPositionTy = SmallVector<int32_t, MaxNumElemsInCandidate>;
  using StructMethodSetTy = SmallPtrSet<Function *, MaxNumStructMethods>;
  using VectorMethodSetTy = SmallPtrSet<Function *, MaxNumVectorMethods>;
  using VectorFieldTypeSetTy = SmallPtrSet<Type *, 2>;

  // Returns SType.
  inline StructType *getStructTy() { return SType; }

  // Returns MemInterfaceType.
  inline StructType *getMemInterfaceType() { return MemInterfaceType; }

  // Returns array element type of candidate field at FI index.
  inline Type *getFieldElemTy(int32_t FI) { return CandidateFieldElemTy[FI]; }

  // Returns true if DTy is class type of any candidate fields.
  inline bool isCandidateFieldDerivedTy(Type *DTy) {
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
  StructType *SType = nullptr;

  // Candidate struct and vector field classes all have same pointer
  // type field that is used to allocate/deallocate memory to manage
  // the vectors.
  StructType *MemInterfaceType = nullptr;

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
  SmallPtrSet<Type *, 4> CandidateFieldDerivedTy;

  // Mapping between vector candidate class position and set of
  // methods of the vector class.
  DenseMap<int32_t, VectorMethodSetTy> CandidateFieldMemberFuncs;

  // Mapping between candidate vector field location and type of array
  // element of the vector.
  DenseMap<int32_t, Type *> CandidateFieldElemTy;

  inline StructType *getValidStructTy(Type *Ty);
  inline Type *getPointeeType(Type *Ty);
  inline bool isPotentialPaddingField(Type *Ty);
  inline bool isStructWithNoRealData(Type *Ty);
  inline Type *getBaseClassOfSimpleDerivedClass(Type *Ty);
  inline bool isVectorLikeClass(Type *Ty, Type **ElemTy);
  inline bool collectTypesIfVectorClass(Type *VTy, int32_t pos);
};

StructType *MemInitCandidateInfo::getValidStructTy(Type *Ty) {
  StructType *STy = dyn_cast<StructType>(Ty);
  if (!STy || STy->isLiteral() || !STy->isSized())
    return nullptr;
  return STy;
}

// Returns type of pointee if 'Ty' is pointer.
Type *MemInitCandidateInfo::getPointeeType(Type *Ty) {
  if (auto *PTy = dyn_cast_or_null<PointerType>(Ty))
    return PTy->getElementType();
  return nullptr;
}

// Returns true if 'Ty' is potential padding field that
// is created to fill gaps in structs.
bool MemInitCandidateInfo::isPotentialPaddingField(Type *Ty) {
  ArrayType *ATy = dyn_cast<ArrayType>(Ty);
  if (!ATy || !ATy->getElementType()->isIntegerTy(8))
    return false;
  return true;
}

// Returns true if 'Ty' is a struct that doesn't have any real data
// except vftable.
// Ex:
//      %"MemoryManager" = type { i32 (...)** }
//
bool MemInitCandidateInfo::isStructWithNoRealData(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy || STy->getNumElements() > 1)
    return false;
  if (STy->getNumElements() == 1 && !isPtrToVFTable(STy->getElementType(0)))
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
bool MemInitCandidateInfo::isVectorLikeClass(Type *Ty, Type **ElemTy) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumDataPointers = 0;
  unsigned NumCounters = 0;
  unsigned NumFlags = 0;
  unsigned NumNoDataPointers = 0;
  unsigned NumVtablePtr = 0;
  *ElemTy = nullptr;
  for (auto *ETy : STy->elements()) {
    if (isPotentialPaddingField(ETy))
      continue;
    if (isPtrToVFTable(ETy)) {
      NumVtablePtr++;
      continue;
    }
    if (ETy->isIntegerTy(8)) {
      NumFlags++;
      continue;
    }
    if (ETy->isIntegerTy(32)) {
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
Type *MemInitCandidateInfo::getBaseClassOfSimpleDerivedClass(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return nullptr;

  if (STy->getNumElements() != 1)
    return nullptr;

  return STy->getElementType(0);
}

// Returns true if given type is candidate for MemInitTrimDown.
//
// Ex:
//   %"FieldValueMap" = type { %"ValueVectorOf.6"*, %"ValueVectorOf.7"*,
//                             %"RefArrayVectorOf"*, %"MemoryManager"* }
//
//   %"FieldValueMap" will be considered as candidate for MemInitTrimDown
//   because it has only pointer fields that point to either potential
//   vector classes (%"ValueVectorOf.6", %"ValueVectorOf.7" and
//   %"RefArrayVectorOf") or dummy class with no real data (%"MemoryManager").
//
//
//   %"ValueVectorOf.6" = type { i8, i32, i32, %"IC_Field"**, %"MemoryManager"*
//   }
//
//   %"ValueVectorOf.6", %"ValueVectorOf.7" and %"RefArrayVectorOf" classes
//   are considered as potential vector classes because they have
//    1. Two integer fields that represent values of Size and Capacity.
//    2. A Pointer field that represents element array
//    3. Dummy struct with no real data
//    4. A Bool flag
//
//
//   %"MemoryManager" = type { i32 (...)** }
//
//   %"MemoryManager" is considered as dummy struct with no real data.
//
bool MemInitCandidateInfo::isCandidateType(Type *Ty) {

  auto STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  auto NumElems = STy->getNumElements();
  if (NumElems > MaxNumElemsInCandidate || NumElems < MinNumElemsInCandidate)
    return false;

  int32_t Pos = -1;
  unsigned NumNoDataPointers = 0;
  for (auto *FTy : STy->elements()) {
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
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
      dbgs() << "  Failed: doesn't have minimum candidate vectors.\n";
    });
    return false;
  }
  if (NumNoDataPointers != 1) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
      dbgs() << "  Failed: Unexpected MemoryInterface Type.\n";
    });
    return false;
  }
  SType = STy;
  return true;
}

// Collects the element type of VTy at Pos if element type is a pointer
// to vector class.
bool MemInitCandidateInfo::collectTypesIfVectorClass(Type *VTy, int32_t Pos) {
  // Check if it is simple derived class that doesn't have its own
  // fields.
  //
  // Ex:
  //  Derived: %"RefArrayVectorOf" = type { %"BaseRefVectorOf.5" }
  //
  //  Base: %"BaseRefVectorOf.5" = type { i32 (...)**, i8, i32, i32,
  //                                      i16**, %"MemoryManager"* }
  auto *DerivedTy = VTy;
  if (auto *BaseTy = getBaseClassOfSimpleDerivedClass(VTy)) {
    VTy = BaseTy;
  }
  Type *VecElemTy;
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

// If element type of Ty at Offset is a simple derived class of vector base
// class, it returns the derived class. Otherwise, returns nullptr.
// Ex:
//  Derived: %"RefArrayVectorOf" = type { %"BaseRefVectorOf.5" }
//
//  Base: %"BaseRefVectorOf.5" = type { i32 (...)**, i8, i32, i32,
//                                      i16**, %"MemoryManager"* }
Type *MemInitCandidateInfo::isSimpleDerivedVectorType(Type *Ty,
                                                      int32_t Offset) {
  auto STy = getValidStructTy(Ty);
  if (!STy)
    return nullptr;
  auto *FTy = STy->getElementType(Offset);
  auto *VTy = getPointeeType(FTy);
  if (!VTy)
    return nullptr;
  if (!getBaseClassOfSimpleDerivedClass(VTy))
    return nullptr;
  if (!collectTypesIfVectorClass(VTy, Offset))
    return nullptr;
  SType = STy;
  return VTy;
}

// Collect member functions of candidate struct and member functions of
// candidate vector field classes. 'AtLTO' indicates whether this routine
// is called from LTO pass or not.
bool MemInitCandidateInfo::collectMemberFunctions(Module &M, bool AtLTO) {
  std::function<bool(Function * F, bool AtLTO,
                     SmallPtrSet<Function *, 32> &ProcessedFuncs)>
      CollectVectorMemberFunctions;

  // If 'F' is a member function of candidate vector field, add
  // it to the corresponding member function set.
  auto CheckVectorMemberFunction = [this](Function *F) -> void {
    if (!F)
      return;
    auto *ThisTy = getClassType(F);
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
          DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
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
  for (auto &F : M)
    if (auto *CTy = getClassType(&F))
      if (CTy == SType)
        StructMethods.insert(&F);

  if (StructMethods.size() > MaxNumStructMethods) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
      dbgs() << "  Failed: Exceeding max limit for struct methods.\n";
    });
    return false;
  }

  SmallPtrSet<Function *, 32> ProcessedFunctions;
  // Collect member functions of all candidate vector fields.
  for (auto *F : StructMethods) {
    if (AtLTO && F->isDeclaration()) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
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
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
        dbgs() << "  Failed: Unexpected number of vector methods.\n";
      });
      return false;
    }
    for (auto *F : CandidateFieldMemberFuncs[Loc]) {
      if (F->isDeclaration()) {
        DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
          dbgs() << "  Failed: Missing definition for vector method.\n";
        });
        return false;
      }
      if (F->size() > MaxNumVectorMethodBBlocks) {
        DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
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

// Collect callsites for all member functions of
//   1. Candidate Struct
//   2. Candidate array field structs
void MemInitCandidateInfo::collectFuncs(
    SmallSet<Function *, 32> *MemInitFuncs) {
  for (auto *F : StructMethods)
    MemInitFuncs->insert(F);
  for (auto Loc : CandidateFieldPositions)
    for (auto *F : CandidateFieldMemberFuncs[Loc])
      MemInitFuncs->insert(F);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

void MemInitCandidateInfo::printCandidateInfo(void) {
  dbgs() << "    Candidate: " << getStructName(SType) << "\n";
  for (auto Loc : CandidateFieldPositions) {
    dbgs() << " Member functions for " << Loc << " candidate field: \n";
    for (auto *F : CandidateFieldMemberFuncs[Loc])
      dbgs() << "       " << F->getName() << "\n";
    dbgs() << " \n";
  }
}

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

} // namespace dtrans

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_MEMINITTRIMDOWNINFOIMPL_H

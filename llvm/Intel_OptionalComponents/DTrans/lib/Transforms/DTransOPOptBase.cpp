//===--- DTransOPOptBase.cpp - Base class for DTrans Transforms -----==//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file provides the base classes for DTrans Transformations that provide
// the common functionality needed for rewriting dependent data types and
// functions that change as the result of DTrans modifying a structure
// definition. This is to work with an opaque pointer representation.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DTransOPOptBase.h"

#include "Intel_DTrans/Analysis/DTransOPUtils.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "dtransop-optbase"

namespace llvm {
namespace dtransOP {

bool DTransOPOptBase::run(Module &M) {
  if (!prepareTypesBaseImpl(M))
    return false;

  // TODO: Implement the calls to collect types the derived class is changing
  // and perform the transformation.
  return false;
}

// Identify and create new types for any types the child class is going
// to replace.
bool DTransOPOptBase::prepareTypesBaseImpl(Module &M) {
  KnownStructTypes = TM.getIdentifiedStructTypes();

  // Compute the set of types that each type is a dependee of. This will
  // enable the determination of other types that need to be rewritten when one
  // type changes.
  buildTypeDependencyMapping();

  return true;
}

// Identify and create types that need to be remapped because of an
// existing type that contains a reference to a type being changed by
// the transformation.
//
// For each type, compute the set of structure types that either directly or
// indirectly use the type. For example:
//   struct.test01 = { i32, i32 }
//   struct.test02 = { struct.test01, struct.test02* }
//   struct.test03 = { struct.test01*, struct.test02* }
//
// struct.test01 is directly used by struct.test02 and indirectly used by
// struct.test03.
// struct.test02 is indirectly used by struct.test03.
// struct.test03 is not used by other types.
//
// The map created will be used for populating the list of additional types that
// may need to be converted when changing a specific type.
void DTransOPOptBase::buildTypeDependencyMapping() {
  for (auto *StTy : KnownStructTypes)
    collectDependenciesForType(StTy);

#if !defined(NDEBUG)
  LLVM_DEBUG({
    dumpTypeToTypeSetMapping("\nType dependency direct mapping table:",
                             TypeToDirectDependentTypes);
    dumpTypeToTypeSetMapping("\nType dependency pointer mapping table:",
                             TypeToPtrDependentTypes);
    dbgs() << "\n";
  });
#endif // !defined(NDEBUG)
}

// Collect the dependency mapping information by analyzing the body of
// 'Ty' to add 'Ty' to the dependency sets for each type contained within it.
// For example, processing the structure definition:
//   %struct.test08b = type { i32, %struct.test08c, void (%struct.test08a*)* }
// will result in the sets maintained for 'struct.test08c' and
// 'struct.test08a' containing 'struct.test08b' as a type that may need to be
// replaced if either of those types are replaced. (When opaque pointers are not
// used, changing either type will cause 'struct.test08b' to need to be changed.
// When opaque pointers are used, only changes to 'struct.test08c' would require
// 'struct.test08b' to be changed.
void DTransOPOptBase::collectDependenciesForType(DTransStructType *StructTy) {

  // Add a 'Dependee' to the set of types tracked for the 'Depender'.
  // 'IsPtrDep' controls whether it is a direct dependency (nested relationship)
  // or indirect dependency (pointer relationship).
  auto UpdateTypeToDependentTypeMap =
      [this](DTransType *Depender, DTransType *Dependee, bool IsPtrDep) {
        if (Depender == Dependee)
          return;

        if (!Dependee->isAggregateType() || !Depender->isAggregateType())
          return;

        LLVM_DEBUG({
          dbgs() << "DTRANS-OPTBASE: Type dependency: ";
          Depender->print(dbgs(), false);
          dbgs() << " used by ";
          Dependee->print(dbgs(), false);
          dbgs() << (IsPtrDep ? " [pointer]" : " [nesting]") << "\n";
        });

        // Pointer dependencies affect the description of the types in metadata.
        //    %struct.ptr_dep = type { p0 } ; Where p0 represents %struct.foo*
        // Direct dependencies affect both the type in IR and the description of
        // the type in metadata.
        //    %struct.outer = type { %struct.inner }
        if (IsPtrDep)
          TypeToPtrDependentTypes[Depender].insert(Dependee);
        else
          TypeToDirectDependentTypes[Depender].insert(Dependee);
      };

  // Update either the TypeToPtrDependentTypes or TypeToDirectDependentTypes set
  // to contain 'Dependee' as a member of the base type of 'Ty' based on the
  // relationship between the types. Examples:
  //  struct.B = type { struct.A }
  //    -> struct.B is a direct dependee of struct.A
  //  struct.B = type { [ 4 x struct.A* ] }
  //    -> struct.B is an indirect dependee of struct.A
  //  struct.B = type { struct A*(struct C*)* }
  //    -> struct.B is an indirect dependee of struct.A and struct.C
  //
  std::function<void(DTransType *, DTransType *)> AddDependentTypeEntry =
      [&UpdateTypeToDependentTypeMap,
       &AddDependentTypeEntry](DTransType *Ty, DTransType *Dependee) -> void {
    // If the dependent type is a literal struct, add the members of the
    // literal struct, rather than the literal struct itself.
    if (auto *StTy = dyn_cast<DTransStructType>(Ty)) {
      if (StTy->isLiteralStruct()) {
        for (auto &FieldMember : StTy->elements()) {
          DTransType *FieldTy = FieldMember.getType();
          AddDependentTypeEntry(FieldTy, Dependee);
        }
        return;
      }
    }

    if (auto *FuncTy = dyn_cast<DTransFunctionType>(Ty)) {
      DTransType *RetTy = FuncTy->getReturnType();
      assert(RetTy && "Incomplete function type");
      AddDependentTypeEntry(RetTy, Dependee);

      unsigned NumParams = FuncTy->getNumArgs();
      for (unsigned Idx = 0; Idx < NumParams; ++Idx) {
        DTransType *ArgTy = FuncTy->getArgType(Idx);
        assert(ArgTy && "Incomplete function type");
        AddDependentTypeEntry(ArgTy, Dependee);
      }

      return;
    }

    if (hasPointerType(Ty)) {
      DTransType *BaseTy = unwrapDTransType(Ty);
      if (auto *FunctionTy = dyn_cast<DTransFunctionType>(BaseTy))
        AddDependentTypeEntry(FunctionTy, Dependee);
      else
        UpdateTypeToDependentTypeMap(BaseTy, Dependee, /*IsPtrDep=*/true);
    } else {
      DTransType *BaseTy = unwrapDTransType(Ty);
      UpdateTypeToDependentTypeMap(BaseTy, Dependee, /*IsPtrDep=*/false);
    }
  };

  for (auto &FieldMember : StructTy->elements()) {
    DTransType *FieldTy = FieldMember.getType();
    assert(FieldTy && "Metadata for structure has ambiguous types");
    AddDependentTypeEntry(FieldTy, StructTy);
  }
}

#if !defined(NDEBUG)
// Print the table of type dependences
void DTransOPOptBase::dumpTypeToTypeSetMapping(
    StringRef Header, DTransTypeToTypeSetMap &TypeToDependentTypes) {

  auto DTransTypeToString = [](DTransType *Ty) {
    std::string OutputVal;
    raw_string_ostream OutputStream(OutputVal);
    Ty->print(OutputStream, false);
    OutputStream.flush();
    return OutputVal;
  };

  dbgs() << Header << "\n";
  for (auto &TySetTyPair : TypeToDependentTypes) {
    DTransType *Ty = TySetTyPair.first;
    Ty->print(dbgs(), false);
    dbgs() << ": ";

    dtrans::printCollectionSorted(dbgs(), TySetTyPair.second.begin(),
                                  TySetTyPair.second.end(), ", ",
                                  DTransTypeToString);
    dbgs() << "\n";
  }
}
#endif // !defined(NDEBUG)

} // namespace dtransOP
} // namespace llvm

//===------------------------PtrTypeAnalyzer.h----------------------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

// This file defines the pointer type analyzer classes and interfaces that are
// used by the DTrans safety checks and transformations to perform type recovery
// from opaque pointers in order to identify the types of object that a pointer
// may be referring to.

#if !INTEL_INCLUDE_DTRANS
#error PtrTypeAnalyzer.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_PTRYPEANALYZER_H
#define INTEL_DTRANS_ANALYSIS_PTRYPEANALYZER_H

#include "llvm//ADT/SmallPtrSet.h"
#include <functional>
#include <memory>
#include <set>

namespace llvm {
class DataLayout;
class Function;
class Module;
class TargetLibraryInfo;
class User;
class Value;
class raw_ostream;

namespace dtrans {

class DTransType;
class DTransTypeManager;
class PtrTypeAnalyzerImpl;
class TypeMetadataReader;

// This class is for keeping track of the source object type that a Value refers
// to, and what type of object it is used as. If the Value is the address of an
// element within a structure or array, that will be tracked as well.
class ValueTypeInfo {
public:
  // For each Value tracked, information will be tracked about the type it is
  // believed to be declared as, and the types of object it is used as. In the
  // current IR, the declared type is explicit in the IR, however when opaque
  // pointers are introduced, this will no longer be the case.
  //
  // For example:
  //   %struct.node = { i32, p0, p0 } ; Actual def: { i32, i32*, %struct.edge* }
  //   %22 = getelementptr %struct.node, p0 %21, i64 0, i32 2
  //   %24 = load i64, p0 %22
  //
  // - Based on the type of field #2 within %struct.node, %22 is declared as
  //   being a %struct.edge**.
  // - The pointer, %22, is used as an i64* during the load to %24.
  //
  // Another example is:
  //   %struct.test01 = type { i64, p0 } ; Actual def: { i64, %struct.test01* }
  //   %struct.test02 = type { i64, p0 } ; Actual def: { i64, %struct.test02* }
  //
  //  define void @test01(p0 %in) {
  //    %f1 = getelementptr %struct.test01, p0 %in, i64 0, i32 1
  //    %v1 = load p0, p0 %f1
  //    %f2 = getelementptr %struct.test02, p0 %v1, i64 0, i32 1
  //
  // %v1 appears to be declared as a %struct.test01* from the definition of %f1.
  // %v1 gets used as a %struct.test02* during the definition of %f2
  //
  // The current DTrans safety analysis is operating upon the 'usage' type, so
  // the 'declared' type may turn out to be unnecessary when the implementation
  // is complete. Although the current IR will only allow a single type to be
  // the 'declared' type, a set of types will be collected because it may not be
  // unambiguous with opaque pointers.
  enum ValueAnalysisType { VAT_Decl, VAT_Use };
  enum LPIState {
    LPIS_NotAnalyzed,       // No analysis done yet.
    LPIS_PartiallyAnalyzed, // Analysis is in progress. Needed to handle cyclic
                            // dependencies.
    LPIS_CompletelyAnalyzed // Type has been resolved.
  };

  // Used to describe the field or byte offset into an array or structure. When
  // the element pointee address starts on a field boundary, PLK_Field will be
  // used. For the rare cases where an byte offset does not start on a field
  // boundary (when address is passed into a memory intrinsic call), PLK_Offset
  // will be used.
  struct PointeeLoc {
    enum PointeeLocKind { PLK_Field, PLK_Offset };
    PointeeLocKind Kind;
    union {
      size_t ElementNum;
      size_t ByteOffset;
    } Loc;

    PointeeLoc(PointeeLocKind Kind, size_t Val) : Kind(Kind) {
      if (Kind == PLK_Field)
        Loc.ElementNum = Val;
      else
        Loc.ByteOffset = Val;
    }
    PointeeLocKind getKind() const { return Kind; }
    size_t getElementNum() const { return Loc.ElementNum; }
    size_t getByteOffset() const { return Loc.ByteOffset; }
  };

  // Track the aggregate type, and the offset into the type.
  typedef std::pair<DTransType *, PointeeLoc> TypeAndPointeeLocPair;

  // Using std::set, instead of SmallPtrSet for ElementPointees because in most
  // cases there will not be any elements placed into these sets, so there is no
  // need to preallocate memory.
  typedef std::set<TypeAndPointeeLocPair> ElementPointeeSet;
  typedef std::set<TypeAndPointeeLocPair> &ElementPointeeSetRef;

  // Most Values will have very few possible types.
  typedef SmallPtrSet<DTransType *, 2> PointerTypeAliasSet;
  typedef SmallPtrSetImpl<DTransType *> &PointerTypeAliasSetRef;

  ValueTypeInfo(Value *V) : V(V) {}

  // @param Kind        - Indicates whether this modification is for the
  //                      'declared' type set or the 'usage' type set. An item
  //                      added as a 'declared' type will also be inserted into
  //                      the 'usage' type set, but an object added as a 'usage'
  //                      type will only go to the 'usage' type set.
  // @param Ty          - Type to be added to the PointerTypeAliasSet.
  // @return            - 'true' if the sets changed as a result of the addition
  bool addTypeAlias(ValueAnalysisType Kind, dtrans::DTransType *Ty);

  // This function is used to capture that a value is the address of some
  // field within an aggregate type.
  //
  // @param Kind        - Indicates whether this modification is for the
  //                      'declared' type set or the 'usage' type set. An item
  //                      added as a 'declared' type will also be inserted into
  //                      the 'usage' type set, but an object added as a 'usage'
  //                      type will only go to the 'usage' type set.
  // @param BaseTy      - Aggregate type that an address is being obtained for.
  // @param ElemIdx     - Element number of aggregate.
  // @return            - 'true' if the sets changed as a result of the
  //                      addition. Otherwise false.
  bool addElementPointee(ValueAnalysisType Kind, dtrans::DTransType *BaseTy,
                         size_t ElemIdx);

  // This function is used to capture that a value is the address on some
  // location that does not being a field boundary within an aggregate type.
  //
  // @param Kind        - Indicates whether this modification is for the
  //                      'declared' type set or the 'usage' type set. An item
  //                      added as a 'declared' type will also be inserted into
  //                      the 'usage' type set, but an object added as a 'usage'
  //                      type will only go to the 'usage' type set.
  // @param BaseTy      - Aggregate type that an address is being obtained for.
  // @param ByteOffset  - Byte offset from start of aggregate.
  // @return            - 'true' if the sets changed as a result of the
  //                      addition. Otherwise false.
  bool addElementPointeeByOffset(ValueAnalysisType Kind,
                                 dtrans::DTransType *BaseTy, size_t ByteOffset);

  // Retrieve the (declared or usage type) alias set for iterating.
  PointerTypeAliasSetRef getPointerTypeAliasSet(ValueAnalysisType Kind) {
    return PointerTypeAliases[Kind];
  }

  // Retrieve the (declared or usage type) element pointee set for iterating.
  ElementPointeeSetRef getElementPointeeSet(ValueAnalysisType Kind) {
    return ElementPointees[Kind];
  }

  // Indicates that an element in the alias set is an aggregate type.
  bool canAliasToAggregatePointer() { return AliasesToAggregatePointer; }

  // Indicate that the Value object could not be processed during type
  // determination. For example, a global pointer variable which should have had
  // a metadata attachment to define the type, but was missing the metadata.
  void setUnhandled();
  bool getUnhandled() const { return Unhandled; }

  // Indicate the Value object type may require information about a Value which
  // could not be processed.
  void setDependsOnUnhandled();
  bool getDependsOnUnhandled() const { return DependsOnUnhandled; }

  // These control the state machine that tracks the progress of analyzing the
  // value.
  void setPartiallyAnalyzed();
  void setCompletelyAnalyzed();
  bool isPartiallyAnalyzed();
  bool isCompletelyAnalyzed() const;
  static const char *LPIStateToString(LPIState S);

  bool empty() {
    // Only check the 'VAT_Use' set, since this is a superset of the 'VAT_Decl'
    // set.
    return PointerTypeAliases[VAT_Use].empty() &&
           ElementPointees[VAT_Use].empty();
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Print declared and usage type information.
  void dump() const;

  // @param OS        - Output stream.
  // @param Combined  - When 'true' output will combine the 'declared' type set
  //                    and usage 'type' set into a single group. Otherwise,
  //                    these will be printed separately.
  // @param Prefix    - String to prefix lines by, such as a comment character
  //                    or indentation.
  void print(raw_ostream &OS, bool Combined = false,
             const char *Prefix = "") const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  // Internal implementation for updating the ElementPointees set.
  bool addElementPointeeImpl(ValueAnalysisType Kind, dtrans::DTransType *BaseTy,
                             PointeeLoc &Loc);

  // The value object this type information is for.
  Value *V = nullptr;

  // Keep a set of values for the type aliases and element-of types for both the
  // 'declared' type and the 'usage' type.
  PointerTypeAliasSet PointerTypeAliases[2];
  ElementPointeeSet ElementPointees[2];

  // Indicates that at least one of the types within the PointerTypeAliases set
  // is an aggregate type.
  bool AliasesToAggregatePointer = false;

  // An instruction that declared this value was not supported by the analysis,
  // which means the type recovery information may be incomplete.
  bool Unhandled = false;

  // The value object had a dependency on a value that could not be handled by
  // the analysis, which means the type recovery information may be incomplete.
  bool DependsOnUnhandled = false;

  // Used for handling cyclic dependencies that require multiple rounds of the
  // analysis routine.
  LPIState AnalysisState = LPIS_NotAnalyzed;
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static raw_ostream &operator<<(raw_ostream &OS, const ValueTypeInfo &Info) {
  Info.print(OS);
  return OS;
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Comparator for ValueTypeInfo::PointeeLoc to enable using type within
// std::set.
inline bool operator<(const ValueTypeInfo::PointeeLoc &A,
                      const ValueTypeInfo::PointeeLoc &B) {
  if (A.Kind == B.Kind)
    return A.Kind == ValueTypeInfo::PointeeLoc::PLK_Field
               ? A.getElementNum() < B.getElementNum()
               : A.getByteOffset() < B.getByteOffset();

  // Treat field elements as sorting before non-field elements
  if (A.Kind == ValueTypeInfo::PointeeLoc::PLK_Field)
    return true;

  return false;
}

// This class provides the interface to the information collected by the
// pointer type analyzer for use by the DTrans safety analysis and DTrans
// transformation passes.
class PtrTypeAnalyzer {
public:
  PtrTypeAnalyzer(
      DTransTypeManager &TM, TypeMetadataReader &MDReader, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI);

  ~PtrTypeAnalyzer();

  // Copying is not allowed, but moving should be.
  PtrTypeAnalyzer(const PtrTypeAnalyzer &) = delete;
  PtrTypeAnalyzer(PtrTypeAnalyzer &&);
  PtrTypeAnalyzer &operator=(const PtrTypeAnalyzer &) = delete;
  PtrTypeAnalyzer &operator=(PtrTypeAnalyzer &&);

  // This method is the entry point to perform analysis of the module to
  // identify pointer types.
  void run(Module &M);

  // The following methods are the interfaces for querying the type information
  // by the safety analysis and transformation.
  //
  // Retrieve the ValueTypeInfo object, if it exists, for V. If V is 'null' or
  // 'undef' then the function that takes the User and operand number
  // needs to be used because in those case the type of 'null' or 'undef' is
  // going to be context specific.
  //
  // All Value objects for the 'null' pointer value within a function are the
  // same Value object. For type recovery, instances of constants such as 'null'
  // or 'undef' need to be tracked as specific types, so will be kept in a
  // different map than the map of Values to ValueTypeInfo. For example:
  //   %ptr1 = alloca %struct.foo
  //   %ptr2 = alloca %struct.bar
  //   store p0 null, p0 %ptr1
  //   store p0 null, p0 %ptr2
  //
  // In this case, the Value object representing 'null' in one case means
  // %struct.foo* and in the other case %struct.bar*. Without tracking these
  // separately, there could be safety violations about the types when
  // propagating type information about Value objects.
  ValueTypeInfo *getValueTypeInfo(const Value *V) const;
  ValueTypeInfo *getValueTypeInfo(const User *U, unsigned OpNum) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dumpPTA(Module &M);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  std::unique_ptr<PtrTypeAnalyzerImpl> Impl;
};

} // end namespace dtrans
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_PTRYPEANALYZER_H

//===----------------------DTransSafetyAnalyzer.cpp-----------------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"

#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransDebug.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/DTransCommon.h"

#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "dtrans-safetyanalyzer"

// Print detailed messages regarding the safety checks.
#define SAFETY_VERBOSE "dtrans-safetyanalyzer-verbose"

using namespace llvm;
using namespace dtrans;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Comma separated list of function names that the verbose debug output should
// be reported for. If empty, all verbose messages are generated when the
// appropriate -debug-only value is set. Otherwise, messages are only emitted
// when processing functions matching the name of an entry in the list.
static cl::list<std::string> DTransSAFilterPrintFuncs(
    "dtrans-safetyanalyzer-filter-print-funcs", cl::CommaSeparated,
    cl::ReallyHidden,
    cl::desc("Filter emission of safety analyzer verbose debug messages to "
             "specific functions"));

// A trace filter that will be enabled/disabled based on the function name.
static llvm::dtrans::debug::DebugFilter FNFilter;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// This class is responsible for analyzing the LLVM IR using information
// collected by the PtrTypeAnalyzer class to mark the DTrans safety bits on the
// TypeInfo objects managed by the DTransSafetyInfo class. This will also
// collect the field usage information for structure fields.
class DTransSafetyInstVisitor : public InstVisitor<DTransSafetyInstVisitor> {
public:
  DTransSafetyInstVisitor(LLVMContext &Ctx, const DataLayout &DL,
                          DTransSafetyInfo &DTInfo, PtrTypeAnalyzer &PTA,
                          DTransTypeManager &TM)
      : DL(DL), DTInfo(DTInfo), PTA(PTA), TM(TM) {
    DTransI8Type = TM.getOrCreateAtomicType(llvm::Type::getInt8Ty(Ctx));
    DTransI8PtrType = TM.getOrCreatePointerType(DTransI8Type);
    DTransPtrSizedIntType = TM.getOrCreateAtomicType(
        llvm::Type::getIntNTy(Ctx, DL.getPointerSizeInBits()));
    DTransPtrSizedIntPtrType = TM.getOrCreatePointerType(DTransPtrSizedIntType);
  }

  DTransType *getDTransI8Type() const { return DTransI8Type; }
  DTransPointerType *getDTransI8PtrType() const { return DTransI8PtrType; }
  DTransType *getDTransPtrSizedIntType() const { return DTransPtrSizedIntType; }
  DTransPointerType *getDTransPtrSizedIntPtrType() const {
    return DTransPtrSizedIntPtrType;
  }

  void visitModule(Module &M) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    // Reset the state of the trace filter to the default value each time we
    // start visiting a module.
    FNFilter.reset();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    for (StructType *Ty : M.getIdentifiedStructTypes())
      if (auto *DTStructType = TM.getStructType(Ty->getStructName()))
        DTInfo.getOrCreateTypeInfo(DTStructType);

    // Analyze definitions of the StructInfo types collected.
    for (dtrans::TypeInfo *TI : DTInfo.type_info_entries())
      if (auto *StructInfo = dyn_cast<dtrans::StructInfo>(TI))
        analyzeStructureType(StructInfo);
  }

  // Analyze the structure to collect basic information, such as whether it is
  // nested or contains a vtable, etc.
  void analyzeStructureType(dtrans::StructInfo *StructInfo) {
    size_t NumElements = StructInfo->getNumFields();
    if (NumElements == 0) {
      DEBUG_WITH_TYPE(SAFETY_VERBOSE,
                      dbgs() << "dtrans-safety: No fields in structure: "
                             << *StructInfo->getDTransType() << "\n");
      StructInfo->setSafetyData(dtrans::NoFieldsInStruct);
      return;
    }

    // Check whether the structure ends with an array containing zero elements.
    // This is necessary to allow for treating memory allocation sizes that are
    // not an exact multiple of the structure as safe in some instances. We only
    // check that the last field is an array, not the possibility that the last
    // field is a nested structure which ends with a zero-sized array, because
    // it not permitted to nest a flexible structure within an array or another
    // structure.
    FieldInfo &LastField = StructInfo->getField(NumElements - 1);
    DTransType *LastFieldType = LastField.getDTransType();
    if (auto *ArType = dyn_cast<DTransArrayType>(LastFieldType))
      if (ArType->getNumElements() == 0) {
        DEBUG_WITH_TYPE(SAFETY_VERBOSE,
                        dbgs() << "dtrans-safety: Type has zero-sized array: "
                               << *StructInfo->getDTransType() << "\n");
        StructInfo->setSafetyData(dtrans::HasZeroSizedArray);
      }

    for (auto &FI : StructInfo->getFields()) {
      // Check for possible nested structures. If the field is an array or
      // vector, find the underlying element type, which could be a nested
      // structure.
      DTransType *FieldType = FI.getDTransType();
      DTransType *UnderlyingType = FI.getDTransType();
      while (auto *SeqTy = dyn_cast<DTransSequentialType>(UnderlyingType))
        UnderlyingType = SeqTy->getTypeAtIndex(0);

      if (auto StTy = dyn_cast<DTransStructType>(UnderlyingType)) {
        DEBUG_WITH_TYPE(SAFETY_VERBOSE,
                        dbgs() << "dtrans-safety: Contains nested structure: "
                               << *StructInfo->getDTransType() << "\n"
                               << "  child : " << *UnderlyingType << "\n");

        StructInfo->setSafetyData(dtrans::ContainsNestedStruct);
        dtrans::TypeInfo *ContainedStInfo = DTInfo.getOrCreateTypeInfo(StTy);
        ContainedStInfo->setSafetyData(dtrans::NestedStruct);
      }

      // Check for function pointer fields and potential vtable fields.
      if (FieldType->isPointerTy()) {
        if (FieldType->getPointerElementType()->isFunctionTy()) {
          DEBUG_WITH_TYPE(SAFETY_VERBOSE,
                          dbgs() << "dtrans-safety: Has function ptr: "
                                 << *StructInfo->getDTransType() << "\n");
          StructInfo->setSafetyData(dtrans::HasFnPtr);
        } else if (auto *PtrTy = dyn_cast<dtrans::DTransPointerType>(
                       FieldType->getPointerElementType())) {
          if (auto *FnTy = dyn_cast<dtrans::DTransFunctionType>(
                  PtrTy->getPointerElementType()))
            if (FnTy->getNumArgs() == 0 && FnTy->isVarArg()) {
              // Fields matching this check might not actually be vtable
              // pointers, but we will treat them as though they are since the
              // false positives do not affect any cases we are currently
              // interested in.
              DEBUG_WITH_TYPE(SAFETY_VERBOSE,
                              dbgs()
                                  << "dtrans-safety: Has vtable-like element: "
                                  << *StructInfo->getDTransType() << "\n"
                                  << "  field: " << *FieldType << "\n");
              StructInfo->setSafetyData(dtrans::HasVTable);
            }
        }
      }
    }
  }

  void visitFunction(Function &F) {
    LLVM_DEBUG(dbgs() << "visitFunction: " << F.getName() << "\n");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    // Enable verbose trace filter predicate, if necessary.
    if (!DTransSAFilterPrintFuncs.empty()) {
      FNFilter.setEnabled(false);
      for (auto &N : DTransSAFilterPrintFuncs)
        if (F.getName() == N) {
          FNFilter.setEnabled(true);
          break;
        }
    }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  }

  // Check "alloca" instructions for the construction of aggregate types to set
  // the "Local Instance" and "Local Pointer" safety bits.
  void visitAlloca(AllocaInst &I) {
    ValueTypeInfo *Info = PTA.getValueTypeInfo(&I);
    if (!Info) {
      // There should always be a ValueTypeInfo object available for an
      // AllocaInst because the result type of an "alloca" is a pointer type.
      DTInfo.setUnhandledPtrType(&I);
      return;
    }

    // We are expecting the pointer type analyzer to have the type, so we are
    // only looking at elements from the 'VAT_Decl' list. If the pointer type
    // analyzer could not handle it, mark the DTrans safety info as not being
    // valid.
    if (Info->getUnhandled())
      DTInfo.setUnhandledPtrType(&I);

    for (auto *AliasTy :
         Info->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl)) {
      assert(AliasTy->isPointerTy() &&
             "Expecting alloca to produce a pointer type");

      DTransType *ElemTy = AliasTy->getPointerElementType();
      if (auto *ArTy = dyn_cast<DTransArrayType>(ElemTy)) {
        // For arrays, we need to find the actual type that makes up the array.
        // If the array is made up of pointer elements, then our analysis will
        // effectively treat that type as a "Local Pointer"
        //   For example: [4 x %struct.T1*]
        // Otherwise, only set "Local instance" on the array itself.
        DTransType *UnitTy = getArrayUnitType(ArTy);
        if (UnitTy->isPointerTy())
          setBaseTypeInfoSafetyData(UnitTy, dtrans::LocalPtr,
                                    "Array of pointers to type allocated", &I);
        else if (UnitTy->isVectorTy())
          setBaseTypeInfoSafetyData(AliasTy, dtrans::UnhandledUse,
                                    "Array of vector allocated", &I);
        else
          setBaseTypeInfoSafetyData(AliasTy, dtrans::LocalInstance,
                                    "Array of type allocated", &I);
        continue;
      }

      if (ElemTy->isPointerTy())
        setBaseTypeInfoSafetyData(AliasTy, dtrans::LocalPtr,
                                  "Pointer allocated", &I);
      else if (ElemTy->isVectorTy())
        setBaseTypeInfoSafetyData(AliasTy, dtrans::UnhandledUse,
                                  "Array of vector allocated", &I);
      else
        setBaseTypeInfoSafetyData(AliasTy, dtrans::LocalInstance,
                                  "Instance allocated", &I);
    }
  }

private:
  // private methods

  // Helper to get the deepest type that makes up an array.
  DTransType *getArrayUnitType(DTransArrayType *ArTy) {
    DTransType *BaseTy = ArTy;
    while (BaseTy->isArrayTy())
      BaseTy = BaseTy->getArrayElementType();

    return BaseTy;
  }

  // Return true if 'Data' is a safety condition that should be cascaded
  // across pointer field members within a structure. Most safety conditions
  // detected on a structure do not apply to structures that are only
  // referenced by pointer fields (as opposed to fully nested structures)
  // because changing the layout of the parent structure does not affect
  // the type pointed to. However, safety conditions involving accesses that
  // cannot be fully analyzed must be transferred to the contained pointers.
  bool isPointerCarriedSafetyCondition(dtrans::SafetyData Data) {
    switch (Data) {
    case dtrans::AddressTaken:
    case dtrans::AmbiguousPointerTarget:
    case dtrans::BadCasting:
    case dtrans::BadCastingPending:
    case dtrans::BadMemFuncManipulation:
    case dtrans::SystemObject:
    case dtrans::UnsafePtrMerge:
    case dtrans::UnhandledUse:
      return true;

    case dtrans::UnsafePointerStore:
    case dtrans::UnsafePointerStoreConditional:
      // TODO: Unsafe pointer store was not treated as pointer carried in
      // the legacy DTransAnalysis pass because some programs could not
      // be optimized. This may require more analysis for cases where
      // UnsafePointerStore is applied to a structure. For now, take
      // the conservative approach.
      return true;

    case dtrans::FieldAddressTaken:
      // FieldAddressTaken is treated as a pointer carried condition based on
      // how out of bounds field accesses is set because the access is not
      // permitted under the C/C++ rules, but is allowed within the definition
      // of llvm IR. If an out of bounds access is permitted, then it would be
      // possible to access elements of pointed-to objects, as well, in ways
      // that DTrans would not be able to analyze.
      return dtrans::DTransOutOfBoundsOK;
    case dtrans::AmbiguousGEP:
    case dtrans::BadAllocSizeArg:
    case dtrans::BadCastingConditional:
    case dtrans::BadMemFuncSize:
    case dtrans::BadPtrManipulation:
    case dtrans::ContainsNestedStruct:
    case dtrans::DopeVector:
    case dtrans::GlobalArray:
    case dtrans::GlobalInstance:
    case dtrans::GlobalPtr:
    case dtrans::HasCppHandling:
    case dtrans::HasInitializerList:
    case dtrans::HasZeroSizedArray:
    case dtrans::HasFnPtr:
    case dtrans::HasVTable:
    case dtrans::LocalInstance:
    case dtrans::LocalPtr:
    case dtrans::MemFuncPartialWrite:
    case dtrans::MismatchedArgUse:
    case dtrans::MismatchedElementAccess:
    case dtrans::NestedStruct:
    case dtrans::NoFieldsInStruct:
    case dtrans::WholeStructureReference:
    case dtrans::UnsafePointerStorePending:
    case dtrans::VolatileData:
      // These cases do not need to be pointer carried
      return false;
    }

    llvm_unreachable("Fully covered switch isn't fully covered?");
  }

  // Return true if 'Data' should be propagated down to all types nested
  // within some type for which the safety condition was found to hold.
  // The motivation for this propagation is that a user may access outside
  // the bounds of a structure. This is strictly not allowed in C/C++, but
  // is allowed under the definition of LLVM IR.
  bool isCascadingSafetyCondition(dtrans::SafetyData Data) {
    if (dtrans::DTransOutOfBoundsOK)
      return true;

    switch (Data) {
      // We can add additional cases here to reduce the conservative behavior
      // as needs dictate.
    case dtrans::FieldAddressTaken:
    case dtrans::HasZeroSizedArray:
      return false;

    case dtrans::AddressTaken:
    case dtrans::AmbiguousGEP:
    case dtrans::AmbiguousPointerTarget:
    case dtrans::BadAllocSizeArg:
    case dtrans::BadCasting:
    case dtrans::BadCastingConditional:
    case dtrans::BadCastingPending:
    case dtrans::BadMemFuncManipulation:
    case dtrans::BadMemFuncSize:
    case dtrans::BadPtrManipulation:
    case dtrans::ContainsNestedStruct:
    case dtrans::DopeVector:
    case dtrans::GlobalArray:
    case dtrans::GlobalInstance:
    case dtrans::GlobalPtr:
    case dtrans::HasCppHandling:
    case dtrans::HasInitializerList:
    case dtrans::HasFnPtr:
    case dtrans::HasVTable:
    case dtrans::LocalInstance:
    case dtrans::LocalPtr:
    case dtrans::MemFuncPartialWrite:
    case dtrans::MismatchedArgUse:
    case dtrans::MismatchedElementAccess:
    case dtrans::NestedStruct:
    case dtrans::NoFieldsInStruct:
    case dtrans::SystemObject:
    case dtrans::WholeStructureReference:
    case dtrans::UnhandledUse:
    case dtrans::UnsafePtrMerge:
    case dtrans::UnsafePointerStore:
    case dtrans::UnsafePointerStoreConditional:
    case dtrans::UnsafePointerStorePending:
    case dtrans::VolatileData:
      return true;
    }

    llvm_unreachable("Fully covered switch isn't fully covered?");
  }

  // This function will identify the aggregate type corresponding to 'Ty',
  // allowing for levels of pointer indirection, and set the SafetyData on the
  // type. Nested and referenced types within the aggregate may also be set
  // based on the pointer-carried and cascading safety data rules. 'Reason' and
  // 'V' are optional parameters that provide for debug traces.
  void setBaseTypeInfoSafetyData(DTransType *Ty, dtrans::SafetyData Data,
                                 const char *Reason, Value *V) {
    DEBUG_WITH_TYPE_P(FNFilter, SAFETY_VERBOSE, {
      if (Reason) {
        dbgs() << "dtrans-safety: " << getSafetyDataName(Data) << " -- "
               << Reason << "\n";
        if (V) {
          Function *F = nullptr;
          if (auto *I = dyn_cast<Instruction>(V))
            F = I->getFunction();
          else if (auto *Arg = dyn_cast<Argument>(V))
            F = Arg->getParent();
          if (F)
            dbgs() << "  [" << F->getName() << "] ";
          dbgs() << *V << "\n";
        }
      }
    });

    setBaseTypeInfoSafetyData(Ty, Data, isCascadingSafetyCondition(Data),
                              isPointerCarriedSafetyCondition(Data));
  }

  // Set the safety data on all the aliased types of 'PtrInfo'
  // 'Reason' and 'V' are optional parameters that provide for debug traces.
  void setBaseTypeInfoSafetyData(ValueTypeInfo *PtrInfo,
                                 dtrans::SafetyData Data, const char *Reason,
                                 Value *V) {
    DEBUG_WITH_TYPE_P(FNFilter, SAFETY_VERBOSE, {
      if (Reason) {
        dbgs() << "dtrans-safety: " << getSafetyDataName(Data) << " -- "
               << Reason << "\n";
        if (V) {
          Function *F = nullptr;
          if (auto *I = dyn_cast<Instruction>(V))
            F = I->getFunction();
          else if (auto *Arg = dyn_cast<Argument>(V))
            F = Arg->getParent();
          if (F)
            dbgs() << "  [" << F->getName() << "] ";
          dbgs() << *V << "\n";
        }
      }
    });

    for (auto *AliasTy :
         PtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
      setBaseTypeInfoSafetyData(AliasTy, Data, nullptr, nullptr);
  }

  // This is a helper function that retrieves the aggregate type through
  // zero or more layers of indirection and sets the specified safety data
  // for that type.
  //
  // When 'IsCascading' is set, the safety data will also
  // be set for fields nested (and possibly pointers) within a structure type.
  //
  // When 'IsPointerCarried' is set, the safety data will also be cascaded to
  // the types referenced via the pointer. This parameter is only used, when
  // 'IsCascading' is set to true.
  //
  // Note, descending into types for cascading of the safety data stops
  // when a type is encountered that already contains the safety data value
  // because all elements reached from it should also already have the safety
  // bit set.
  void setBaseTypeInfoSafetyData(DTransType *Ty, dtrans::SafetyData Data,
                                 bool IsCascading, bool IsPointerCarried) {

    // Get the underlying element type by stripping away the pointer levels.
    // Because we do not have TypeInfo objects for vector types, we also need to
    // iterate through the vector levels to detect the type for
    // "<2 x struct.T1*>"
    DTransType *BaseTy = Ty;
    while (BaseTy->isPointerTy() || BaseTy->isVectorTy())
      if (BaseTy->isPointerTy())
        BaseTy = BaseTy->getPointerElementType();
      else
        BaseTy = BaseTy->getVectorElementType();

    dtrans::TypeInfo *TI = DTInfo.getOrCreateTypeInfo(BaseTy);
    TI->setSafetyData(Data);
    if (!IsCascading)
      return;

    // This lambda encapsulates the logic for propagating safety conditions to
    // structure field or array element types. If the field or element is an
    // instance of a type of interest, and not if it is merely a pointer to
    // such a type, the condition is propagated. If the field is a pointer,
    // we call a helper function to see if this is a condition which requires
    // propagation through pointer fields. Propagation is done via a recursive
    // call to setBaseTypeInfoSafetyData in order to handle additional levels
    // of nesting.
    auto MaybePropagateSafetyCondition = [this, BaseTy](DTransType *FieldTy,
                                                        dtrans::SafetyData Data,
                                                        bool IsCascading,
                                                        bool IsPointerCarried) {
      // If FieldTy is not a type of interest, there's no need to propagate.
      if (!isTypeOfInterest(FieldTy))
        return;
      // If the field is an instance of the type, propagate the condition.
      if (!FieldTy->isPointerTy()) {
        setBaseTypeInfoSafetyData(FieldTy, Data, IsCascading, IsPointerCarried);
      } else if (IsPointerCarried) {
        // In some cases we need to propagate the condition even to fields
        // that are pointers to structures, but in order to avoid infinite
        // loops in the case where two structures each have pointers to the
        // other we need to avoid doing this for structures that already have
        // the condition set.
        DTransType *FieldBaseTy = FieldTy;
        while (FieldBaseTy->isPointerTy())
          FieldBaseTy = FieldBaseTy->getPointerElementType();
        dtrans::TypeInfo *FieldTI = DTInfo.getOrCreateTypeInfo(FieldBaseTy);
        if (!FieldTI->testSafetyData(Data)) {
          DEBUG_WITH_TYPE_P(FNFilter, SAFETY_VERBOSE, {
            dbgs()
                << "dtrans-safety: Cascading pointer carried safety condition: "
                << "From: " << *BaseTy << " To: " << *FieldBaseTy
                << " :: " << dtrans::getSafetyDataName(Data) << "\n";
          });
          setBaseTypeInfoSafetyData(FieldBaseTy, Data, IsCascading,
                                    IsPointerCarried);
        }
      }
    };

    // Propagate this condition to any nested types.
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI))
      for (dtrans::FieldInfo &FI : StInfo->getFields())
        MaybePropagateSafetyCondition(FI.getDTransType(), Data, IsCascading,
                                      IsPointerCarried);
    else if (isa<dtrans::ArrayInfo>(TI))
      MaybePropagateSafetyCondition(BaseTy->getArrayElementType(), Data,
                                    IsCascading, IsPointerCarried);
  }

  // Retrun 'true' if the DTransType is something that may require safety data
  // to be set on it.
  bool isTypeOfInterest(DTransType *Ty) {
    DTransType *BaseTy = Ty;

    // For pointers, see what they point to.
    while (BaseTy->isPointerTy())
      BaseTy = cast<DTransPointerType>(BaseTy)->getPointerElementType();

    if (auto *ArTy = dyn_cast<DTransArrayType>(BaseTy))
      return isTypeOfInterest(ArTy->getArrayElementType());
    if (auto *VecTy = dyn_cast<DTransVectorType>(BaseTy))
      return isTypeOfInterest(VecTy->getTypeAtIndex(0));
    else if (auto *StTy = dyn_cast<DTransStructType>(BaseTy))
      if (!StTy->isOpaque())
        return true;

    return false;
  }

private:
  // private data members
  const DataLayout &DL;
  DTransSafetyInfo &DTInfo;
  PtrTypeAnalyzer &PTA;
  DTransTypeManager &TM;

  // Types that are frequently needed for comparing type aliases against
  // known types.
  DTransType *DTransI8Type = nullptr;                    // i8
  DTransPointerType *DTransI8PtrType = nullptr;          // i8*
  DTransType *DTransPtrSizedIntType = nullptr;           // i64 or i32
  DTransPointerType *DTransPtrSizedIntPtrType = nullptr; // i64* or i32*
};

DTransSafetyInfo::DTransSafetyInfo(DTransSafetyInfo &&Other)
    : TM(std::move(Other.TM)), MDReader(std::move(Other.MDReader)),
      PtrAnalyzer(std::move(Other.PtrAnalyzer)),
      DTransSafetyAnalysisRan(Other.DTransSafetyAnalysisRan) {}

DTransSafetyInfo::~DTransSafetyInfo() { reset(); }

DTransSafetyInfo &DTransSafetyInfo::operator=(DTransSafetyInfo &&Other) {
  reset();
  TM = std::move(Other.TM);
  MDReader = std::move(Other.MDReader);
  PtrAnalyzer = std::move(Other.PtrAnalyzer);
  UnhandledPtrType = Other.UnhandledPtrType;
  DTransSafetyAnalysisRan = Other.DTransSafetyAnalysisRan;
  return *this;
}

void DTransSafetyInfo::analyzeModule(
    Module &M,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
    WholeProgramInfo &WPInfo,
    function_ref<BlockFrequencyInfo &(Function &)> GetBFI) {

  LLVM_DEBUG(dbgs() << "DTransSafetyInfo::analyzeModule running\n");
  if (!WPInfo.isWholeProgramSafe()) {
    LLVM_DEBUG(dbgs() << "  DTransSafetyInfo: Not Whole Program Safe\n");
    return;
  }

  LLVMContext &Ctx = M.getContext();
  const DataLayout &DL = M.getDataLayout();
  TM = std::make_unique<DTransTypeManager>(Ctx);
  MDReader = std::make_unique<TypeMetadataReader>(*TM);
  if (!MDReader->initialize(M)) {
    LLVM_DEBUG(dbgs() << "DTransSafetyInfo: Type metadata reader did not find "
                         "structure type metadata\n");
    return;
  }

  PtrAnalyzer =
      std::make_unique<PtrTypeAnalyzer>(Ctx, *TM, *MDReader, DL, GetTLI);
  PtrAnalyzer->run(M);
  LLVM_DEBUG(dbgs() << "DTransSafetyInfo: PtrTypeAnalyzer complete\n");

  DTransSafetyInstVisitor Visitor(Ctx, DL, *this, *PtrAnalyzer, *TM);
  Visitor.visit(M);

  LLVM_DEBUG({
    dbgs() << "DTransSafetyInfo: Module visited\n";
    if (getUnhandledPtrType())
      dbgs() << "DTransSafetyInfo: Unhandled Value from pointer type "
                "analyzer found\n";
  });

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (DTransPrintAnalyzedTypes)
    printAnalyzedTypes();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
}

void DTransSafetyInfo::reset() {
  // Clean up the TypeInfoMap.
  // DTransSafetyInfo owns the TypeInfo objects in the TypeInfoMap.
  // The DTransType pointers are owned by the DTransTypeManager, and
  // will be cleaned up when that object is reset.
  for (auto Entry : TypeInfoMap) {
    switch (Entry.second->getTypeInfoKind()) {
    case dtrans::TypeInfo::NonAggregateInfo:
      delete cast<dtrans::NonAggregateTypeInfo>(Entry.second);
      break;
    case dtrans::TypeInfo::PtrInfo:
      delete cast<dtrans::PointerInfo>(Entry.second);
      break;
    case dtrans::TypeInfo::StructInfo:
      delete cast<dtrans::StructInfo>(Entry.second);
      break;
    case dtrans::TypeInfo::ArrayInfo:
      delete cast<dtrans::ArrayInfo>(Entry.second);
      break;
    }
  }
  TypeInfoMap.clear();

  TM.reset();
  MDReader.reset();
  PtrAnalyzer.reset();
  UnhandledPtrType = false;
  DTransSafetyAnalysisRan = false;
}

void DTransSafetyInfo::setUnhandledPtrType(Value *V) {
  LLVM_DEBUG({
    dbgs() << "DTransSafetyInfo: Unhandled Value from pointer type analyzer\n";
    if (V)
      dbgs() << *V;
    dbgs() << "\n";
  });

  UnhandledPtrType = true;
}

bool DTransSafetyInfo::useDTransSafetyAnalysis() const {
  return !UnhandledPtrType && DTransSafetyAnalysisRan;
}

TypeInfo *DTransSafetyInfo::getOrCreateTypeInfo(DTransType *Ty) {
  // If we already have this type in our map, just return it.
  auto *TI = getTypeInfo(Ty);
  if (TI)
    return TI;

  // Create the DTrans TypeInfo object for this type and any sub-types.
  TypeInfo *DTransTI;
  if (Ty->isPointerTy()) {
    // For pointer types, we want to record the pointer type info
    // and then record what it points to. We must add the pointer to the
    // map early to avoid infinite recursion.
    DTransTI = new dtrans::PointerInfo(Ty);
    TypeInfoMap[Ty] = DTransTI;
    getOrCreateTypeInfo(cast<DTransPointerType>(Ty)->getPointerElementType());
    return DTransTI;
  } else if (Ty->isArrayTy()) {
    dtrans::TypeInfo *ElementInfo =
        getOrCreateTypeInfo(Ty->getArrayElementType());
    DTransTI = new dtrans::ArrayInfo(
        Ty, ElementInfo, cast<DTransArrayType>(Ty)->getNumElements());
  } else if (Ty->isStructTy()) {
    DTransStructType *STy = cast<DTransStructType>(Ty);
    SmallVector<dtrans::AbstractType, 16> FieldTypes;
    unsigned NumFields = STy->getNumFields();
    for (unsigned Idx = 0; Idx < NumFields; ++Idx) {
      DTransType *FieldTy = STy->getFieldType(Idx);
      assert(FieldTy && "Expected non-null field type");
      getOrCreateTypeInfo(FieldTy);
      FieldTypes.push_back(FieldTy);
    }
    // TODO: StructInfo constructor should be cleaned up to not require
    // the IgnoreFor parameter, because they are always created with 0,
    // and then updated with a call to the SetIgnoreFor method.
    DTransTI = new dtrans::StructInfo(Ty, FieldTypes, /*IgnoreFor=*/0);
  } else {
    // TODO: TypeInfo does not support vector types, so they will be stored
    // within NonAggregateTypeInfo objects currently.
    assert(!Ty->isAggregateType() &&
           "DTransAnalysisInfo::getOrCreateTypeInfo unexpected aggregate type");
    DTransTI = new dtrans::NonAggregateTypeInfo(Ty);
  }

  TypeInfoMap[Ty] = DTransTI;
  return DTransTI;
}

TypeInfo *DTransSafetyInfo::getTypeInfo(DTransType *Ty) const {
  // If we have this type in our map, return it.
  auto It = TypeInfoMap.find(Ty);
  if (It != TypeInfoMap.end())
    return It->second;

  return nullptr;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransSafetyInfo::printAnalyzedTypes() {
  std::vector<dtrans::TypeInfo *> TypeInfoEntries;
  for (auto *TI : type_info_entries())
    if (isa<dtrans::ArrayInfo>(TI) || isa<dtrans::StructInfo>(TI))
      TypeInfoEntries.push_back(TI);

  std::sort(TypeInfoEntries.begin(), TypeInfoEntries.end(),
            [](dtrans::TypeInfo *A, dtrans::TypeInfo *B) {
              std::string TypeStrA;
              llvm::raw_string_ostream RSO_A(TypeStrA);
              A->getDTransType()->print(RSO_A);
              std::string TypeStrB;
              llvm::raw_string_ostream RSO_B(TypeStrB);
              B->getDTransType()->print(RSO_B);
              return RSO_A.str().compare(RSO_B.str()) < 0;
            });

  dbgs() << "================================\n";
  dbgs() << " DTRANS Analysis Types Created\n";
  dbgs() << "================================\n\n";

  for (auto TI : TypeInfoEntries)
    if (auto *AI = dyn_cast<dtrans::ArrayInfo>(TI))
      AI->print(dbgs());
    else if (auto *SI = dyn_cast<dtrans::StructInfo>(TI))
      SI->print(dbgs());

  dbgs().flush();
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Provide a definition for the static class member used to identify passes.
AnalysisKey DTransSafetyAnalyzer::Key;

DTransSafetyInfo DTransSafetyAnalyzer::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetBFI = [&FAM](Function &F) -> BlockFrequencyInfo & {
    return FAM.getResult<BlockFrequencyAnalysis>(F);
  };
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  WholeProgramInfo &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  DTransSafetyInfo DTResult;
  DTResult.analyzeModule(M, GetTLI, WPInfo, GetBFI);
  return DTResult;
}

namespace {
class DTransSafetyAnalyzerWrapper : public ModulePass {

public:
  static char ID;

  DTransSafetyAnalyzerWrapper() : ModulePass(ID) {
    initializeDTransSafetyAnalyzerWrapperPass(*PassRegistry::getPassRegistry());
  }

  DTransSafetyInfo &getDTransSafetyInfo(Module &M) { return Result; }

  bool runOnModule(Module &M) override {
    auto GetBFI = [this](Function &F) -> BlockFrequencyInfo & {
      return this->getAnalysis<BlockFrequencyInfoWrapperPass>(F).getBFI();
    };
    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    WholeProgramInfo &WPInfo =
        getAnalysis<WholeProgramWrapperPass>().getResult();
    Result.analyzeModule(M, GetTLI, WPInfo, GetBFI);
    return false;
  }

  bool doFinalization(Module &M) override {
    Result.reset();
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<BlockFrequencyInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
  }

private:
  DTransSafetyInfo Result;
};
} // end anonymous namespace

INITIALIZE_PASS_BEGIN(DTransSafetyAnalyzerWrapper, "dtrans-safetyanalyzer",
                      "Data transformation safety analyzer", false, true)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(BlockFrequencyInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransSafetyAnalyzerWrapper, "dtrans-safetyanalyzer",
                    "Data transformation safety analyzer", false, true)

char DTransSafetyAnalyzerWrapper::ID = 0;

ModulePass *llvm::createDTransSafetyAnalyzerTestWrapperPass() {
  return new DTransAnalysisWrapper();
}

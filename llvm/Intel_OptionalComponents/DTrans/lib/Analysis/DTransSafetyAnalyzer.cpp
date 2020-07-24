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

// Helper method to check whether 'Ty' is a pointer to a pointer.
static bool isPtrToPtr(const DTransType *Ty) {
  return Ty->isPointerTy() && Ty->getPointerElementType()->isPointerTy();
}

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

    for (auto &GV : M.globals()) {
      analyzeGlobalVariable(GV);

      for (auto *U : GV.users())
        if (auto *CE = dyn_cast<ConstantExpr>(U))
          analyzeConstantExpr(CE);
    }
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

  // Analyze global variables to mark "Global array", Global instance", and
  // "Global pointer" safety flags on types of interest. Also, analyze the
  // initializer value to collect constants, and check for incompatible types
  // being stored into the the global.
  void analyzeGlobalVariable(GlobalVariable &GV) {
    // Declarations do not need to be analyzed
    if (GV.isDeclaration())
      return;

    ValueTypeInfo *Info = PTA.getValueTypeInfo(&GV);
    assert(Info &&
           "PtrTypeAnalyzer failed to construct ValueTypeInfo for global");

    // We are expecting the pointer type analyzer to have the type, so we are
    // only looking at elements from the 'VAT_Decl' list. If the pointer type
    // analyzer could not handle it, mark the DTrans safety info as not being
    // valid. This can occur due to the losing the type metadata attached to the
    // GlobalVariable.
    if (Info->getUnhandled())
      DTInfo.setUnhandledPtrType(&GV);

    // These are conservative conditions meant to restrict us to
    // global variables that are definitely handled. Additional analysis
    // may be required to safely support these cases.
    //
    // hasLocalLinkage() indicates that the linkage is either internal or
    //   private. This should be the case for all program defined variables
    //   during LTO. The primary intention of this check is to eliminate
    //   externally accessible variables, but we're using a more general
    //   check to defer decisions about other linkage types until they
    //   are encountered.
    //
    // isThreadLocal() may be acceptable, but is included here so that
    //   consideration of its implications can be deferred until it must
    //   be handled.
    //
    if (!GV.hasLocalLinkage() || GV.isThreadLocal()) {
      setAllAliasedTypeSafetyData(Info, dtrans::UnhandledUse,
                                  "Unsupported global variable use", &GV);
      return;
    }

    // Look at the information set by the PtrTypeAnalyzer for the variable.
    // There generally should only be a single declared type. We need to rely on
    // the PtrTypeAnalyzer rather than looking at the type returned by
    // GV.getValueType() because all pointer types will just be 'p0' when opaque
    // pointers are used.
    auto &Pointees = Info->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl);
    for (auto *AliasTy : Pointees) {
      DTransType *ElemTy = AliasTy->getPointerElementType();
      if (!isTypeOfInterest(ElemTy))
        continue;

      // TODO: When 'GlobalInstance' is set below, any pointers contained within
      // the aggregate type being created should be marked with the 'GlobalPtr'
      // flag to indicate there is a pointer to those types at the module level.

      if (auto *ArTy = dyn_cast<DTransArrayType>(ElemTy)) {
        setBaseTypeInfoSafetyData(ArTy, dtrans::GlobalArray,
                                  "Array of type of interest", &GV);

        // For arrays, we need to find the actual type that makes up the
        // array. If the array is made up of pointer elements, then our
        // analysis will effectively treat that type as a "Global Pointer"
        //   For example: [4 x %struct.T1*]
        // Otherwise, only set "Global instance" on the array itself.
        // TODO: The following is similar to the handling of "alloca"
        // instructions, so it may be good to unify the code in the future.
        DTransType *UnitTy = getArrayUnitType(ArTy);
        if (UnitTy->isPointerTy())
          setBaseTypeInfoSafetyData(UnitTy, dtrans::GlobalPtr,
                                    "Global array of pointers to type defined",
                                    &GV);
        else if (UnitTy->isVectorTy())
          setBaseTypeInfoSafetyData(AliasTy, dtrans::UnhandledUse,
                                    "Global array of vector type defined", &GV);
        else
          setBaseTypeInfoSafetyData(AliasTy, dtrans::GlobalInstance,
                                    "Global array of type defined", &GV);
        continue;
      }

      if (ElemTy->isPointerTy())
        setBaseTypeInfoSafetyData(AliasTy, dtrans::GlobalPtr,
                                  "Pointer allocated", &GV);
      else if (ElemTy->isVectorTy())
        setBaseTypeInfoSafetyData(AliasTy, dtrans::UnhandledUse,
                                  "Vector allocated", &GV);
      else
        setBaseTypeInfoSafetyData(AliasTy, dtrans::GlobalInstance,
                                  "Instance allocated", &GV);
    }

    // TODO: Analyze the initializer values for constant values or incompatible
    // types.
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
    assert(Info &&
           "PtrTypeAnalyzer failed to construct ValueTypeInfo for alloca");

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

      // TODO: When 'LocalInstance' is set below, any pointers contained within
      // the aggregate type being created should be marked with the
      // 'LocalPointer' flag.

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
                                  "Vector allocated", &I);
      else
        setBaseTypeInfoSafetyData(AliasTy, dtrans::LocalInstance,
                                  "Instance allocated", &I);
    }
  }

  void visitGetElementPtrInst(GetElementPtrInst &I) {
    analyzeGEPOperator(cast<GEPOperator>(&I));
  }

  // Check the GEP to ensure that the pointer operand is not ambiguous.
  // Also, check for byte flattened GEPs that could not be resolved to a
  // specific value. If the GEP is safe, check whether all the uses are for
  // load/store instructions to determine whether the FieldInfo object needs to
  // be marked as 'ComplexUse'
  void analyzeGEPOperator(GEPOperator *GEP) {
    Value *PtrOp = GEP->getPointerOperand();
    ValueTypeInfo *PtrInfo = PTA.getValueTypeInfo(PtrOp);
    if (!PtrInfo) {
      DTInfo.setUnhandledPtrType(GEP);
      return;
    }

    if (PtrInfo->getUnhandled() || PtrInfo->getDependsOnUnhandled()) {
      DTInfo.setUnhandledPtrType(GEP);
      setAllAliasedTypeSafetyData(PtrInfo, dtrans::UnhandledUse,
                                  "PointerTypeAnalyzer could not resolve all "
                                  "potential types for pointer",
                                  GEP);
      return;
    }

    // If the pointer type analyzer identified this GEP as performing what looks
    // like a byte-flattened GEP, but it was unable to resolve the element
    // indexed, then we need to flag the pointer as "Bad pointer manipulation"
    ValueTypeInfo *GEPInfo = PTA.getValueTypeInfo(GEP);
    assert(GEPInfo &&
           "Pointer type analyzer should have info about all GEP pointers");
    if (GEPInfo->getUnknownByteFlattenedGEP()) {
      setAllAliasedTypeSafetyData(PtrInfo, dtrans::BadPtrManipulation,
                                  "Byte flattened GEP could not be resolved",
                                  GEP);
      return;
    }

    if (!PtrInfo->canAliasToAggregatePointer())
      return;

    // The GEP is indexing an aggregate type, check that the type is
    // unambiguous.
    auto PtrDomTy = PTA.getDominantAggregateUsageType(*PtrInfo);
    if (!PtrDomTy)
      setAllAliasedTypeSafetyData(
          PtrInfo, dtrans::AmbiguousGEP,
          "Multiple potential types for GEP pointer operand", GEP);

    // If there are multiple aggregate types, we need to check that
    // the element can be safely resolved. Normally, multiple aggregate pointer
    // types is an unsafe situation. However, due to the nature of nested types,
    // a pointer to an aggregate is equivalent to a pointer to the zeroth
    // element of the aggregate, and will still have a dominant type as the
    // outer type of the nesting, so we need to deal with possibility there will
    // be multiple aggregate type pointers to be sure the pointer is being used
    // in a compatible way. Having a dominant type is not enough because we need
    // to consider that the pointer is being "downcast" to be used as a derived
    // type
    if (PtrInfo->canAliasMultipleAggregatePointers()) {
      llvm::Type *SrcType = GEP->getSourceElementType();
      if (TM.isSimpleType(SrcType)) {
        DTransType *ExpectedType = TM.getOrCreateSimpleType(SrcType);
        // %pField0 = getelementptr %class.test02.base, %class.test02.base*
        //                 %merge, i64 0, i32 0
        // if %merge could be a derived type, it is an unsafe use.
        if (hasIncompatibleAggregateDecl(ExpectedType, PtrInfo)) {
          setAllAliasedTypeSafetyData(PtrInfo, dtrans::AmbiguousGEP,
                                      "Dominant type does not match expected "
                                      "type for GEP pointer operand",
                                      GEP);
        }
      }
    }

    // Next check the uses of the GEP for updating flags that need to be set on
    // the FieldInfo object is a structure is being indexed.
    //
    // When a GEP points to some element of an aggregate, we need to consider
    // the following cases:
    // - The byte-offset corresponds to the start of a field.
    //    In this case, we need to check how the GEP gets used because some
    //    structure transformations can only process the GEP instruction
    //    when the value is used to load or store the field value. For other
    //    uses, the 'ComplexUse' property is set on the field to inhibit
    //    transformations of the structure.
    // - The byte-offset does not correspond to the start of a field.
    //     In this case, it will generally be a 'Bad pointer manipulation'.
    //     However, we allow a special case where the GEP is used for a
    //     'memset' instruction, to allow the offset to correspond to padding
    //     that may occur between fields. If the offset turns out to not be a
    //     padding location, the 'memset' analyzer will flag the aggregate
    //     with a safety violation.
    if (GEPInfo->pointsToSomeElement()) {
      // Check the uses of this GEP element. If it is used by anything other
      // than casts, loads, and stores.
      std::function<bool(Value *)> hasNonCastLoadStoreUses =
          [&hasNonCastLoadStoreUses](Value *V) {
            for (auto *U : V->users()) {
              if (isa<LoadInst>(U) || isa<StoreInst>(U))
                continue;
              if (isa<CastInst>(U)) {
                if (hasNonCastLoadStoreUses(U))
                  return true;
                continue;
              }
              // Anything else is the "other" use we were looking for.
              DEBUG_WITH_TYPE_P(FNFilter, SAFETY_VERBOSE,
                                dbgs()
                                    << "dtrans-field-info: Complex use of GEP: "
                                    << *U << "\n");
              return true;
            }
            // If we get here, all users were load, store, or cast.
            return false;
          };

      // Check if any of element pointees do not correspond to a element
      // boundary.
      auto HasNonFieldAddress = [](ValueTypeInfo *Info) {
        return any_of(
            Info->getElementPointeeSet(ValueTypeInfo::VAT_Use).begin(),
            Info->getElementPointeeSet(ValueTypeInfo::VAT_Use).end(),
            [](const ValueTypeInfo::TypeAndPointeeLocPair &PointeePair) {
              return PointeePair.second.getKind() ==
                     ValueTypeInfo::PointeeLoc::PLK_Offset;
            });
      };

      if (HasNonFieldAddress(GEPInfo)) {
        // The pointer type analyzer should only have set a non-field offset
        // for the case of a GEP that is passed to a memset and the offset
        // does not correspond to a field boundary.
        assert(valueOnlyUsedForMemset(GEP) &&
               "Non-field accesses expected to only occur for memset operand");
      } else if (hasNonCastLoadStoreUses(GEP)) {
        auto &Pointees = GEPInfo->getElementPointeeSet(ValueTypeInfo::VAT_Use);
        for (auto &PointeePair : Pointees) {
          DTransType *ParentTy = PointeePair.first;
          if (ParentTy->isStructTy()) {
            assert(PointeePair.second.getKind() !=
                       ValueTypeInfo::PointeeLoc::PLK_Offset &&
                   "Unexpected use of invalid element");

            auto *ParentStInfo =
                cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(ParentTy));
            dtrans::FieldInfo &FI =
                ParentStInfo->getField(PointeePair.second.getElementNum());
            FI.setComplexUse(true);
          }
        }
      }
    }
  }

  void visitSelectInst(SelectInst &Sel) { analyzeSelectOrPhi(&Sel); }

  void visitPHINode(PHINode &Phi) { analyzeSelectOrPhi(&Phi); }

  // If the select or phi involves pointer types or element pointees, then
  // the ValueTypeInfo for the instruction will hold the merge results of all
  // the source operands. For type safety, if there are aggregate types
  // involved, we need to be sure there is a unique dominant type when aggregate
  // types are involved, otherwise it is an unsafe pointer merge.
  void analyzeSelectOrPhi(Instruction *I) {
    // If the select/phi instruction was not identified as a type of interest by
    // the PtrTypeAnalyzer there will not be a ValueTypeInfo collected for it.
    ValueTypeInfo *Info = PTA.getValueTypeInfo(I);
    if (!Info || Info->empty())
      return;

    if (Info->getUnhandled() || Info->getDependsOnUnhandled())
      DTInfo.setUnhandledPtrType(I);

    if (!Info->canAliasToAggregatePointer())
      return;

    if (!PTA.getDominantAggregateUsageType(*Info))
      setAllAliasedTypeSafetyData(Info, dtrans::UnsafePtrMerge,
                                  "Merge of conflicting pointer types", I);
  }

  // Check whether the type the value loaded gets used as is compatible with the
  // type that is expected to be loaded. The type expected to be loaded may be
  // based on the type that the pointer operand was identified to be by the
  // pointer type analyzer, or the type of a field member within an aggregate
  // type. When an incompatibility is detected this will set the 'Mismatched
  // element access' and/or 'Bad casting' safety checks. Even though there may
  // not be actual 'bitcast' instructions with opaque pointers, we will use the
  // 'Bad casting' safety bit to indicate that a type is used as if it were cast
  // to a different type.
  void visitLoadInst(LoadInst &I) {
    Value *Ptr = I.getPointerOperand();
    ValueTypeInfo *PtrInfo = PTA.getValueTypeInfo(Ptr);
    ValueTypeInfo *ValInfo = PTA.getValueTypeInfo(&I);

    assert(PtrInfo &&
           "PtrTypeAnalyzer failed to construct ValueTypeInfo for load "
           "pointer operand");

    if (isValueTypeInfoUnhandled(*PtrInfo)) {
      DTInfo.setUnhandledPtrType(&I);
      setAllAliasedAndPointeeTypeSafetyData(
          PtrInfo, dtrans::UnhandledUse,
          "PointerTypeAnalyzer failed to collect type for load", &I);

      if (ValInfo)
        setAllAliasedAndPointeeTypeSafetyData(
            ValInfo, dtrans::UnhandledUse,
            "PointerTypeAnalyzer could not analyze load pointer operand", &I);
      return;
    }

    if (PtrInfo->pointsToSomeElement()) {
      analyzeElementLoadOrStore(I, *PtrInfo, ValInfo);
      return;
    }

    // If the load is not an element pointee and does not involve any aggregate
    // types for the value loaded or pointer operand, then there is nothing left
    // to do.
    if (!PtrInfo->canAliasToAggregatePointer() &&
        (!ValInfo || !ValInfo->canAliasToAggregatePointer()))
      return;

    if (I.isVolatile())
      for (auto *AliasTy :
           PtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
        if (!isPtrToPtr(AliasTy))
          setBaseTypeInfoSafetyData(AliasTy, dtrans::VolatileData,
                                    "volatile load", &I);

    // Start of the checks to determine whether load is a mismatched access to
    // the first element of the structure, or a load of a type that is not
    // compatible with the pointer type. First, get the type being loaded. It
    // may be a pointer, an aggregate type, vector, or a scalar type. It may
    // be unknown due to a pointer being used as multiple types.
    DTransType *ValTy = getLoadStoreValueType(I, ValInfo, /*IsLoad=*/true);
    if (!ValTy) {
      setAllAliasedTypeSafetyData(PtrInfo, dtrans::BadCasting,
                                  "Cannot resolve type of value loaded", &I);
      if (ValInfo)
        setAllAliasedTypeSafetyData(ValInfo, dtrans::BadCasting,
                                    "Cannot resolve type of value loaded", &I);
      return;
    }

    // Check the load instruction for safety. The following are safe uses for a
    // load instruction:
    //
    // glossary:
    //   <NonAggTy> - A type that is not  an aggregate type
    //   <AggTy>    - A type that is an aggregate type or pointer to an
    //                aggregate type
    //   <Ty>       - Any type
    //
    // For the purpose these examples, the type placeholders mean the type that
    // has been collected by the PointerTypeAnalyzer to represent the type that
    // an object is believed to represent. Therefore, these may be different
    // than the types seen in an IR instruction. For example: load i64, i64* %P
    // could mean load a pointer-sized integer for the pointer-to-pointer to an
    // aggregate type if %P is believed to be a pointer-to-pointer to an
    // aggregate type, and pointer-sized integers are 64 bits, which would be
    // case 5 below, because the value loaded would effectively be a pointer to
    // an aggregate type.
    //
    // Note, each of these cases requires that unambiguous types can be
    // resolved for the value loaded and pointer operand.
    //
    // (1)  V = load <NonAggTy>, <NonAggTy>* P
    //       Always safe. DTrans does not care if pointers to scalars are
    //       loaded with mismatched types. (This case should not reach here,
    //       because the above conditions handle it.)
    //
    // (2)  V = load <Ty>*,   <NonAggTy>** P
    //       Safe, provided that <Ty> is not known to alias an aggregate type.
    //
    // (3)  V = load <AggTy>, <AggTy>* P
    //       Safe when AggTy loaded matches pointer's aggregate type.
    //
    // (4)  V = load <Ty>,    <AggTy>* P
    //       Safe, when Ty is compatible with element zero of AggTy.
    //       Compatible means more than just being the exact same type.
    //       If element zero is a pointer type, then i8* or pointer-sized int
    //       types are also compatible with it.
    //
    // (5)  V = load <Ty>*,   <AggTy>** P
    //        Only safe when Ty loaded is compatible with pointer's aggregate
    //        type.
    //
    bool IsWholeStructureRead = false;
    bool IsMismatched = false;
    StringRef BadcastReason;
    DTransType *PtrDomTy = PTA.getDominantAggregateUsageType(*PtrInfo);
    if (PtrInfo->canAliasToDirectAggregatePointer()) {
      // The only things that are valid to directly alias a load from the
      // aggregate pointer are:
      //   - A whole structure load
      //       V = load <AggTy>, <AggTy>* P
      //   - An element zero load of an aggregate type
      //       V = load <Ty>,    <AggTy>* P
      if (ValTy->isStructTy()) {
        setBaseTypeInfoSafetyData(ValTy, dtrans::WholeStructureReference,
                                  "load of structure type", &I);
        IsWholeStructureRead = true;
      } else if (!PtrDomTy || !PTA.isPointeeElementZeroAccess(
                                  PtrDomTy->getPointerElementType(), ValTy)) {
        IsMismatched = true;
        BadcastReason = "Pointer to aggregate not used as element zero load";
      }
    } else if (PtrInfo->canAliasToAggregatePointer()) {
      // The pointer operand must be multiple levels of indirection, so the
      // value loaded needs to be a pointer that is compatible with the pointer
      // type. Check that the dominant types are related as pointer/pointee
      // pairs.
      // - There must be a dominant type for the pointer operand
      // - The pointer and pointee dominant types need to be properly
      //   related to allow a pointer type to be loaded from the
      //   pointer-to-pointer.
      //     V = load <AggTy>*,   <AggTy>** P
      if (!PtrDomTy) {
        IsMismatched = true;
        BadcastReason = "Dominant type of pointer not resolved";
      } else if (PtrDomTy->getPointerElementType() != ValTy ||
                 (PtrInfo->canAliasMultipleAggregatePointers() &&
                  hasIncompatibleAggregateDecl(PtrDomTy, PtrInfo))) {
        IsMismatched = true;
        BadcastReason =
            "Dominant types for value and pointer are not compatible";
      } else if (!ValInfo || !ValInfo->canAliasToAggregatePointer()) {
        // If we reach here, the pointer has multiple levels of indirection to
        // an aggregate type, but the value loaded did not involve an aggregate
        // type.
        IsMismatched = true;
        BadcastReason =
            "Load of non-aggregate from pointer-to-pointer of aggregate";
      }
    } else if (ValInfo && ValInfo->canAliasToAggregatePointer()) {
      // Load does not involve an aggregate for the pointer operand.
      //   V = load <Ty>*,   <NonAggTy>** P
      //
      // This case can only be reached when opaque pointers are available,
      // because that will eliminate the need to bitcast the types and allow the
      // value type to differ from the pointer operand type.
      //
      // For example:
      //   %v = alloca p0
      //   %ld = load p0, p0 %v, align 8
      //   %gep = getelementptr %struct.test01a, p0 %ld, i64 0, i32 0
      //
      // In this case, if %v were to be created as an i64*, but then loaded as a
      // pointer to an aggregate type, it would reach this.
      IsMismatched = true;
      BadcastReason = "Load of aggregate from non-aggregate pointer";
    }

    if (IsMismatched) {
      setAllAliasedTypeSafetyData(PtrInfo, dtrans::BadCasting, BadcastReason,
                                  &I);
      if (ValInfo)
        setAllAliasedTypeSafetyData(ValInfo, dtrans::BadCasting, BadcastReason,
                                    &I);

      TypeSize ValSize = DL.getTypeSizeInBits(I.getType());
      for (auto *PtrAliasTy :
           PtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use)) {

        // Don't set a mismatch on any of the aliases that are
        // pointer-to-pointer because those are not potential zero-element
        // accesses.
        if (isPtrToPtr(PtrAliasTy))
          continue;

        setFieldMismatchedElementAccess(PtrAliasTy, ValSize, ValTy,
                                        /*FieldNum=*/0, I);
      }
    } else if (PtrDomTy && !isPtrToPtr(PtrDomTy)) {
      // Treat this as an element zero access if the pointer operand alias info
      // is not for a pointer-to-pointer access.
      if (auto *StructTy =
              dyn_cast<DTransStructType>(PtrDomTy->getPointerElementType())) {
        dtrans::StructInfo *SI =
            cast<StructInfo>(DTInfo.getOrCreateTypeInfo(StructTy));
        if (SI->getNumFields() != 0) {
          collectReadInfo(I, SI, /*FieldNum=*/0, !IsWholeStructureRead);

          if (IsWholeStructureRead) {
            // Note: For whole structure reference, DTrans does not fill in all
            // the field info details for each field, such as frequency or
            // single value because we do not have any transforms that can
            // handle them.
            for (auto &FI : SI->getFields()) {
              FI.setRead(I);
              FI.setValueUnused(false);
            }
          }
        }
      }
    }
  }

  void visitStoreInst(StoreInst &I) {
    Value *Ptr = I.getPointerOperand();
    Value *Val = I.getValueOperand();
    ValueTypeInfo *PtrInfo = PTA.getValueTypeInfo(Ptr);
    ValueTypeInfo *ValInfo = PTA.getValueTypeInfo(&I, 0);

    assert(PtrInfo &&
           "PtrTypeAnalyzer failed to construct ValueTypeInfo for store "
           "pointer operand");

    if (isValueTypeInfoUnhandled(*PtrInfo) ||
        (ValInfo && isValueTypeInfoUnhandled(*ValInfo))) {
      DTInfo.setUnhandledPtrType(&I);
      setAllAliasedAndPointeeTypeSafetyData(
          PtrInfo, dtrans::UnhandledUse,
          "PointerTypeAnalyzer failed to collect types for store instruction",
          &I);

      if (ValInfo)
        setAllAliasedAndPointeeTypeSafetyData(
            ValInfo, dtrans::UnhandledUse,
            "PointerTypeAnalyzer failed to collect types for store instruction",
            &I);
      return;
    }

    // Check that the address of a structure field is not being stored to
    // memory. This is done up front, so that we can return as soon as
    // analyzing any element pointees of the pointer operand.
    if (ValInfo && ValInfo->pointsToSomeElement())
      for (auto &PointeePair :
           ValInfo->getElementPointeeSet(ValueTypeInfo::VAT_Use)) {
        setBaseTypeInfoSafetyData(PointeePair.first, dtrans::FieldAddressTaken,
                                  "Address of member stored to memory", &I);

        dtrans::TypeInfo *ParentTI =
            DTInfo.getOrCreateTypeInfo(PointeePair.first);
        if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI))
          ParentStInfo->getField(PointeePair.second.getElementNum())
              .setAddressTaken();
      }

    if (PtrInfo->pointsToSomeElement()) {
      analyzeElementLoadOrStore(I, *PtrInfo, ValInfo);
      return;
    }

    // If the store is not an element pointee and does not involve any aggregate
    // types for the value stored or pointer operand, then nothing else needs to
    // be analyzed for the instruction because it cannot trigger any safety
    // flags.
    if (!PtrInfo->canAliasToAggregatePointer() &&
        (!ValInfo || !ValInfo->canAliasToAggregatePointer()))
      return;

    if (I.isVolatile())
      for (auto *AliasTy :
           PtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
        if (!isPtrToPtr(AliasTy))
          setBaseTypeInfoSafetyData(AliasTy, dtrans::VolatileData,
                                    "volatile store", &I);

    DTransType *PtrDomTy = PTA.getDominantAggregateUsageType(*PtrInfo);
    DTransType *ValTy = getLoadStoreValueType(*Val, ValInfo, /*IsLoad=*/false);
    if (!ValTy) {
      // If we cannot determine a single specific type being stored, then we need
      // to mark a safety violation.
      //
      // If the value operand or pointer operand as known to directly alias an
      // aggregate type, mark them as "unsafe pointer store" because we cannot
      // determine that they are compatible. Mark "Bad casting" because the
      // types are not being used with their expected types. This may be
      // redundant, but it is consistent with the way the LocalPointerAnalyzer
      // marked the types.
      if ((ValInfo && ValInfo->canAliasToDirectAggregatePointer()) ||
          PtrInfo->canAliasToDirectAggregatePointer()) {
        setAllAliasedTypeSafetyData(PtrInfo, dtrans::UnsafePointerStore,
                                    "Cannot resolve type of value stored", &I);
        if (ValInfo)
          setAllAliasedTypeSafetyData(ValInfo, dtrans::UnsafePointerStore,
                                      "Cannot resolve type of value stored",
                                      &I);
      }

      setAllAliasedTypeSafetyData(PtrInfo, dtrans::BadCasting,
                                  "Cannot resolve type of value stored", &I);
      if (ValInfo)
        setAllAliasedTypeSafetyData(ValInfo, dtrans::BadCasting,
                                    "Cannot resolve type of value stored", &I);
      return;
    }

    // Check the store instruction for safety. The following are safe uses for a
    // store instruction:
    //
    // glossary:
    //   <NonAggTy> - A type that is not  an aggregate type
    //   <AggTy>    - A type that is an aggregate type or pointer to an
    //                aggregate type
    //   <Ty>       - Any type
    //
    // (1) store <NonAggTy> V, <NonAggTy>* P
    //       Always safe. DTrans does not care if pointers to scalars are
    //       stored with mismatched types. (This case should not reach here,
    //       because the above conditions handle it.)

    // (2) store <Ty>*,    <NonAggTy>** P
    //       Safe, provided that <Ty> is not known to alias an aggregate type.
    //
    // (3) store <AggTy>,  <AggTy>* P
    //       Safe when AggTy stored matches pointer's aggregate type
    //
    // (4) store <Ty> V,   <AggTy>* P
    //   or
    //     store <Ty>* V,  <AggTy>* P
    //       Safe when Ty is compatible with element zero of AggTy.
    //       Compatible means more than just being the exact same type.
    //       If element zero is a pointer type, then i8* or pointer-sized int
    //       types are also compatible with it.
    //
    // (5) store <Ty>* V, <AggTy>** P
    //        Only safe when Ty is compatible with pointer's aggregate type.
    //        This also requires the value operand pointer type to be able to be
    //        resolved unambiguously.
    //
    bool IsWholeStructureWrite = false;
    bool IsMismatched = false;
    StringRef BadcastReason;
    if (PtrDomTy && !PtrDomTy->isPointerTy()) {
      // This is to handle a rare case, where the pointer type analyzer sees the
      // pointer operand being used as both a pointer and non-pointer type. This
      // can occur due to trying to infer types based on usage.
      IsMismatched = true;
      BadcastReason = "Ptr operand not resolved as dominant pointer type";
    } else if (!PtrInfo->canAliasToAggregatePointer()) {
      // Check that there is not a store of an aggregate pointer type to a
      // non-aggregate pointer location.
      //   store i64 %val, i64* %ptr
      //   store i64* %val, i64** %ptr
      //
      // If %val represented something that pointed to an aggregate type, it
      // would be unsafe.
      if (ValInfo && ValInfo->canAliasToAggregatePointer()) {
        IsMismatched = true;
        BadcastReason = "Pointer to aggregate type stored to non-aggregate "
                        "pointer location";
      }
    } else if (PtrInfo->canAliasToDirectAggregatePointer()) {
      // The only things that are valid to directly alias a store to the
      // aggregate pointer are:
      //   - A whole structure store
      //   - An element zero load of the aggregate type
      llvm::Type *ValOpType = Val->getType();
      if (!PtrDomTy) {
        IsMismatched = true;
        BadcastReason = "Dominant type of pointer not resolved";
      } else if (ValOpType->isStructTy() &&
                 PtrDomTy->getPointerElementType()->getLLVMType() ==
                     ValOpType) {
        setBaseTypeInfoSafetyData(ValTy, dtrans::WholeStructureReference,
                                  "store of structure type", &I);
        IsWholeStructureWrite = true;
      }
      // It's not a whole structure store, check for compatibility with element
      // zero of the aggregate type.
      else if (!PTA.isPointeeElementZeroAccess(
                   PtrDomTy->getPointerElementType(), ValTy)) {
        IsMismatched = true;
        BadcastReason = "Pointer to aggregate not used as element zero store";
      }
    } else if (PtrInfo->canAliasToAggregatePointer()) {
      // The pointer operand must be multiple levels of indirection to an
      // aggregate type, so the value stored needs to be a pointer that is
      // compatible with the pointer type. Check that the dominant types are
      // related as pointer/pointee pairs.
      // - There must be a dominant type for the pointer operand
      // - The pointer and pointee dominant types need to be properly
      //   related to allow a pointer type to be loaded from the
      //   pointer-to-pointer.
      // - There must be an unambiguous aggregate pointer type for the value
      //   operand
      if (!PtrDomTy) {
        IsMismatched = true;
        BadcastReason = "Dominant type of pointer not resolved";
      } else if (PtrDomTy->getPointerElementType() != ValTy ||
                 (PtrInfo->canAliasMultipleAggregatePointers() &&
                  hasIncompatibleAggregateDecl(PtrDomTy, PtrInfo))) {
        IsMismatched = true;
        BadcastReason =
            "Dominant types for value and pointer are not compatible";
      } else if (!ValInfo) {
        IsMismatched = true;
        BadcastReason =
            "Dominant type of value pointer being stored not resolved";
      } else {
        DTransType *ValDomTy = PTA.getDominantAggregateUsageType(*ValInfo);
        if (!ValDomTy->isPointerTy() &&
            hasIncompatibleAggregateDecl(ValDomTy->getPointerElementType(),
                                         ValInfo)) {
          IsMismatched = true;
          BadcastReason = "Incompatible type for ptr-to-ptr store";
        }
      }
    }

    if (IsMismatched) {
      // If the value being stored is a pointer to an aggregate type, or the
      // location being stored to is expecting a pointer to aggregate type, then
      // we may have an Unsafe Pointer Store.
      markUnsafePointerStore(nullptr, ValInfo, PtrInfo, &I);
      setAllAliasedTypeSafetyData(PtrInfo, dtrans::BadCasting, BadcastReason,
                                  &I);
      if (ValInfo)
        setAllAliasedTypeSafetyData(ValInfo, dtrans::BadCasting, BadcastReason,
                                    &I);
      return;
    }

    // Treat this as a safe store. If the pointer operand alias info is not for
    // a pointer-to-pointer access, then treat it as an element-zero access and
    // update the FieldInfo for the element.
    if (PtrDomTy && !isPtrToPtr(PtrDomTy))
      if (auto *StructTy =
              dyn_cast<DTransStructType>(PtrDomTy->getPointerElementType())) {
        dtrans::StructInfo *SI =
            cast<StructInfo>(DTInfo.getOrCreateTypeInfo(StructTy));
        if (SI->getNumFields() != 0) {
          collectWriteInfo(I, SI, /*FieldNum=*/0, Val, !IsWholeStructureWrite);

          if (IsWholeStructureWrite) {
            // Note: For whole structure reference, DTrans does not fill in all
            // the field info details for each field, such as frequency or
            // single value because we do not have any transforms that can
            // handle them.
            for (auto &FI : SI->getFields())
              FI.setWritten(I);
          }
        }
      }
  }

  // The element access analysis for load and store instructions are nearly
  // identical, so we use this helper function to perform the task for both.
  void analyzeElementLoadOrStore(Instruction &I, ValueTypeInfo &PtrInfo,
                                 ValueTypeInfo *ValInfo) {
    bool IsLoad = isa<LoadInst>(&I);
    Value *PtrOp = IsLoad ? cast<LoadInst>(&I)->getPointerOperand()
                          : cast<StoreInst>(&I)->getPointerOperand();
    Value *ValOp = IsLoad ? &I : cast<StoreInst>(I).getValueOperand();
    llvm::Type *ValTy = ValOp->getType();
    bool IsVolatile = IsLoad ? cast<LoadInst>(&I)->isVolatile()
                             : cast<StoreInst>(&I)->isVolatile();

    for (auto &PointeePair :
         PtrInfo.getElementPointeeSet(ValueTypeInfo::VAT_Use)) {
      DTransType *ParentTy = PointeePair.first;
      assert((ParentTy->isStructTy() || ParentTy->isArrayTy()) &&
             "Expect pointee pair only for structure/array types");

      if (IsVolatile)
        setBaseTypeInfoSafetyData(ParentTy, dtrans::VolatileData,
                                  "Marked as volatile", &I);

      if (PointeePair.second.getKind() !=
          ValueTypeInfo::PointeeLoc::PLK_Field) {
        // A GEP that indexes a location that is not a field boundary should
        // only be allowed for access to the padding bytes for a memfunc
        // intrinsic, and not a load/store instruction.
        setBaseTypeInfoSafetyData(ParentTy, dtrans::UnhandledUse,
                                  "load/store from non-field boundary", &I);
        continue;
      }

      // Identify the field type or array member type being accessed
      DTransType *IndexedType = nullptr;
      size_t ElementNum = PointeePair.second.getElementNum();
      if (auto *StTy = dyn_cast<DTransStructType>(ParentTy))
        IndexedType = StTy->getFieldType(ElementNum);
      else if (auto *ArTy = dyn_cast<DTransArrayType>(ParentTy))
        IndexedType = ArTy->getArrayElementType();

      if (!IndexedType) {
        // If the IndexedType was not resolved, then there either a problem
        // with setting the field types in the structure when reading the
        // metadata, or the element was a byte offset that does not correspond
        // to a field boundary.
        setBaseTypeInfoSafetyData(ParentTy, dtrans::UnhandledUse,
                                  "type recovery problem for load/store", &I);
        continue;
      }

      // Check for the whole structure being loaded/stored with the single
      // instruction.
      bool IsWholeStructure = false;
      if (IndexedType->isStructTy() && IndexedType->getLLVMType() == ValTy) {
        setBaseTypeInfoSafetyData(IndexedType, dtrans::WholeStructureReference,
                                  "load/store of structure type", &I);
        IsWholeStructure = true;

        // Note: For whole structure reference, DTrans does not fill in all the
        // field info details for each field, such as frequency or single value
        // because we do not have any transforms that can handle them.
        StructInfo *IndexedStTI =
            cast<StructInfo>(DTInfo.getOrCreateTypeInfo(IndexedType));
        for (auto &FI : IndexedStTI->getFields())
          if (IsLoad) {
            FI.setRead(I);
            FI.setValueUnused(false);
          } else {
            FI.setWritten(I);
          }
      }

      if (auto *StTy = dyn_cast<DTransStructType>(ParentTy)) {
        auto *ParentStInfo =
            cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(StTy));
        // Update the read/write info for structure FieldInfo objects. If it is
        // a whole structure reference, don't descend into the nested elements,
        // otherwise walk the nested types to find the actual field being
        // accessed..
        if (IsLoad)
          collectReadInfo(I, ParentStInfo, ElementNum, !IsWholeStructure);
        else
          collectWriteInfo(I, ParentStInfo, ElementNum, ValOp,
                           !IsWholeStructure);
      }

      // Check if the types for the value loaded or value operand of the store
      // are compatible with the type of the indexed element. If they are not,
      // this could also indicate a bad casting operation is taking place if
      // - a pointer is used when one wasn't expected
      // - a pointer was expected, but the value was not used as a pointer,
      //   except in the case that a pointer-sized int is used
      // - a pointer was used, but not with the expected type
      bool TypesCompatible = true;
      bool BadCasting = false;

      DTransType *ValTy = getLoadStoreValueType(*ValOp, ValInfo, IsLoad);
      if (!ValTy) {
        TypesCompatible = false;
        BadCasting = true;
      }
      // Check whether the value being loaded or stored is compatible with
      // expected type for the indexed element.
      else if (!isElementLoadStoreTypeCompatible(IndexedType, ValTy)) {
        TypesCompatible = false;
        // If the expected type was a pointer or the used type was a pointer,
        // then mark it as bad casting.
        BadCasting = (IndexedType->isPointerTy() &&
                      ValTy != getDTransPtrSizedIntPtrType()) ||
                     ValTy->isPointerTy();
      }
      // Check whether the value is compatible with the pointer operand
      else if (!areLoadStoreTypesCompatible(IndexedType, ValInfo, ValOp,
                                            PtrInfo, PtrOp)) {
        TypesCompatible = false;
        BadCasting = true;
      }

      if (!IsLoad && ValInfo) {
        // Make sure that a pointer to an aggregate type is not being stored
        // where the original declaration of the type was not an aggregate type.
        // For example:
        //   %x = bitcast i64* %p to %struct*
        //   store %struct* %x, %struct** %y
        if (ValTy && ValTy->isPointerTy())
          if (ValTy->getPointerElementType()->isAggregateType()) {
            if (hasIncompatibleAggregateDecl(ValTy->getPointerElementType(),
                                             ValInfo)) {
              TypesCompatible = false;
              BadCasting = true;
            }
          } else if (IndexedType->isPointerTy() &&
                     IndexedType->getPointerElementType()->isAggregateType()) {
            // Non-aggregate pointer stored to aggregate pointer location
            TypesCompatible = false;
            BadCasting = true;
          }
      }

      // With opaque pointers, we will not see explicit pointer bitcasts
      // occurring, but the effect here is the same as is a bitcast had
      // occurred, so we need to set the BadCasting safety bit for
      // compatibility with the old LocalPointerAnalysis method, and because
      // that safety bit has different semantics regarding propagation than
      // the MismatchedElementAccess safety, which are necessary to get all
      // the necessary structures marked.
      if (BadCasting) {
        setBaseTypeInfoSafetyData(
            ParentTy, dtrans::BadCasting,
            "Incompatible pointer type for field load/store", &I);
        if (ValInfo)
          for (auto *ValAliasTy :
               ValInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
            setBaseTypeInfoSafetyData(
                ValAliasTy, dtrans::BadCasting,
                "Incompatible pointer type for field load/store", &I);
      }

      if (!TypesCompatible) {
        if (!IsLoad) {
          // When the types don't match, and one or both of the types are
          // pointers to aggregate types, that triggers the Unsafe Pointer
          // Store bit on all the aggregate types the object is used as, and the
          // type of the element indexed.
          bool PtrToAggregateFound = false;
          if (IndexedType->isPointerTy() &&
              IndexedType->getPointerElementType()->isAggregateType())
            PtrToAggregateFound = true;

          if (ValInfo) {
            auto &AliasSet =
                ValInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use);
            PtrToAggregateFound |= ValInfo->canAliasToDirectAggregatePointer();
            if (PtrToAggregateFound) {
              for (auto *ValAliasTy : AliasSet)
                setBaseTypeInfoSafetyData(
                    ValAliasTy, dtrans::UnsafePointerStore,
                    "Incompatible type for field load/store", &I);
            }
          }

          if (PtrToAggregateFound)
            setBaseTypeInfoSafetyData(IndexedType, dtrans::UnsafePointerStore,
                                      "Incompatible type for field load/store",
                                      &I);
        }

        // Finally, mark the structure as having a mismatched element access.
        TypeSize ValSize = DL.getTypeSizeInBits(ValOp->getType());
        setFieldMismatchedElementAccess(ParentTy, ValSize, IndexedType,
                                        ElementNum, I);
      }
    }
  }

  // This helper is to get the type of object that is being used for a 'load' or
  // 'store' instruction. For a 'load', 'V' is the LoadInst. For a 'store', 'V'
  // is the value operand of the StoreInst. Returns 'nullptr' if an unambiguous
  // type cannot be determined.
  DTransType *getLoadStoreValueType(Value &V, ValueTypeInfo *Info,
                                    bool IsLoad) const {
    ValueTypeInfo::ValueAnalysisType Kind =
        IsLoad ? ValueTypeInfo::VAT_Use : ValueTypeInfo::VAT_Decl;

    // The pointer type analyzer only collects types for Values that are
    // potentially pointers. For simple scalar types, it will not have the type,
    // so just take the type from the IR to handle cases such as:
    //   %y = load i32, p0 %x
    if (!Info || Info->empty()) {
      if (TM.isSimpleType(V.getType()))
        return TM.getOrCreateSimpleType(V.getType());
      return nullptr;
    }
    auto &Aliases = Info->getPointerTypeAliasSet(Kind);
    // If the pointer type analyzer did not collect the declared type for the
    // Value, but the stored value is only used as a single type, return that.
    // This does not need to be done for load instructions because in that case
    // we are using the VAT_Use set which is a superset of the VAT_Decl set.
    if (!IsLoad && Aliases.empty()) {
      auto &UseAliases = Info->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use);
      if (UseAliases.size() == 1)
        return *UseAliases.begin();
      return nullptr;
    }

    return PTA.getDominantType(*Info, Kind);
  }

  // Return 'true' if a value used as 'UsageType' is compatible with
  // 'ExpectedType'. To be compatible, one of the following conditions must be
  // met:
  // - The 'UsageType' and 'ExpectedTypes' are the same.
  // - An expected pointer type can be used as an pointer sized int type.
  // - The usage type matches the element zero of the expected type.
  bool isElementLoadStoreTypeCompatible(DTransType *ExpectedType,
                                        DTransType *UsageType) {
    if (!ExpectedType || !UsageType)
      return false;

    if (ExpectedType == UsageType)
      return true;

    if (ExpectedType->isPointerTy() &&
        (UsageType == getDTransI8PtrType() ||
         UsageType == getDTransPtrSizedIntType()))
      return true;

    // Allow an access to the element zero type, such as:
    //   %struct.A = type { %struct.B }
    //   %struct.B = type{ i32, i32, i32 }
    // The 'expected type' of a load from %struct.A* would be %struct.B, but
    // because this is a nested structure, a value used as an 'i32' would be
    // valid.
    if (PTA.isPointeeElementZeroAccess(ExpectedType, UsageType))
      return true;

    return false;
  }

  bool areLoadStoreTypesCompatible(DTransType *ExpectedType,
                                   ValueTypeInfo *ValInfo, Value *ValOp,
                                   ValueTypeInfo &PtrInfo, Value *PtrOp) {

    // When a scalar type is being loaded or stored, there will not be
    // ValueTypeInfo. Make sure that a pointer type was not expected. For
    // example, a pointer is often cast to pointer sized int to allow a value to
    // be stored, which is a safe. Or a field may be a scalar type, and a scalar
    // is being stored. However, if a pointer type was expected, and the value
    // stored cannot be resolved as being the right type, such as if it came
    // from a function argument, then we need to treat it as unsafe.
    //
    // Example: Assume %arg0 is an incoming i64 argument.
    //   %pField.as.p64 = bitcast %struct.A** %pField to i64*
    //   store i64 %arg0, %i64* pField.as.p64
    //
    if (!ValInfo || ValInfo->empty()) {
      if (ExpectedType->isPointerTy() || PTA.isPtrToPtr(PtrInfo))
        return false;

      // Check that the size loaded/stored matches the expected type's size.
      // In the case of an aggregate type being stored at the address, the
      // load/store could be the first element and not the aggregate itself, so
      // don't require the sizes to match.
      if (!ExpectedType->isAggregateType())
        if (DL.getTypeStoreSize(ValOp->getType()) !=
            DL.getTypeStoreSize(ExpectedType->getLLVMType()))
          return false;

      // Otherwise, the types are compatible.
      return true;
    }

    // Storing a 'null' pointer is safe whenever a pointer is expected.
    if (ExpectedType->isPointerTy() && isa<ConstantPointerNull>(ValOp))
      return true;

    if (!ValInfo->canAliasToAggregatePointer()) {
      // The value info only contains pointers to scalar types or generic
      // types. If the pointer location is for an aggregate type, it is not
      // safe, unless it is the element zero type of the aggregate. For example,
      // storing an arbitrary i8* to a %struct.A* will be unsafe.
      if (PtrInfo.canAliasToAggregatePointer()) {
        DTransType *PtrDomTy = PTA.getDominantAggregateUsageType(PtrInfo);
        if (!PtrDomTy || !PtrDomTy->isPointerTy())
          return false;

        DTransType *PtrDomTyPtrTy = PtrDomTy->getPointerElementType();

        if (!PtrDomTyPtrTy->isAggregateType())
          return false;

        if (!PTA.isPointeeElementZeroAccess(PtrDomTyPtrTy, ExpectedType))
          return false;

        return true;
      }

      // Check that none of the alias types are incompatible, such as storing a
      // i16* where a i32* is expected.
      auto &Aliases = ValInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use);
      for (auto *Alias : Aliases)
        if (!isElementLoadStoreTypeCompatible(ExpectedType, Alias))
          return false;

      return true;
    }

    // The Value Type Info indicates the Value is used as an aggregate type.
    // Check that there is a type compatible with the expected type.
    auto DomTy = PTA.getDominantAggregateUsageType(*ValInfo);
    if (!DomTy)
      return false;

    if (ValInfo->canAliasMultipleAggregatePointers() &&
        hasIncompatibleAggregateDecl(ExpectedType, &PtrInfo))
      return false;

    if (!isElementLoadStoreTypeCompatible(ExpectedType, DomTy))
      return false;

    return true;
  }

  void setFieldMismatchedElementAccess(DTransType *ParentTy,
                                       TypeSize AccessSize,
                                       DTransType *AccessTy,
                                       unsigned int ElementNum,
                                       Instruction &I) {
    auto *TI = DTInfo.getOrCreateTypeInfo(ParentTy);

    if (DTransOutOfBoundsOK) {
      // Assuming out of bound access, set safety issue for the entire
      // ParentTy.
      setBaseTypeInfoSafetyData(ParentTy, dtrans::MismatchedElementAccess,
                                "Incompatible type for field load/store", &I);
    } else {
      // Set safety issue to AccessTy only, because out of bounds accesses are
      // known to not occur based on the DTransOutOfBoundsOK definition.
      TI->setSafetyData(dtrans::MismatchedElementAccess);
      if (AccessTy)
        setBaseTypeInfoSafetyData(AccessTy, dtrans::MismatchedElementAccess,
                                  "Incompatible type for field load/store", &I);
    }

    if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      TypeSize FieldSize = DL.getTypeSizeInBits(
          ParentStInfo->getField(ElementNum).getLLVMType());
      if (DTransOutOfBoundsOK || AccessSize > FieldSize) {
        for (auto &FI : ParentStInfo->getFields())
          FI.setMismatchedElementAccess();
      } else {
        dtrans::FieldInfo &FI = ParentStInfo->getField(ElementNum);
        FI.setMismatchedElementAccess();
      }
    }
  }

  // This routine is called when a stored value is identified as not being the
  // expected type for the pointer operand. Check if the value being stored is a
  // pointer to an aggregate type, or the location being stored to is expecting
  // a pointer to aggregate type. If so, we have an Unsafe Pointer Store.
  void markUnsafePointerStore(DTransType *ExpectedTy,
                              ValueTypeInfo *StoreValInfo,
                              ValueTypeInfo *StorePtrInfo, StoreInst *SI) {
    bool PtrToAggregateFound = false;
    if (ExpectedTy && ExpectedTy->isPointerTy() &&
        ExpectedTy->getPointerElementType()->isAggregateType())
      PtrToAggregateFound = true;

    PtrToAggregateFound |= StorePtrInfo->canAliasToDirectAggregatePointer();
    if (StoreValInfo) {
      PtrToAggregateFound |= StoreValInfo->canAliasToDirectAggregatePointer();
      if (PtrToAggregateFound)
        setAllAliasedAndPointeeTypeSafetyData(
            StoreValInfo, dtrans::UnsafePointerStore,
            "Incompatible type for field load/store", SI);
    }

    if (PtrToAggregateFound)
      setAllAliasedAndPointeeTypeSafetyData(
          StorePtrInfo, dtrans::UnsafePointerStore,
          "Incompatible type for field load/store", SI);
  }

  void collectReadInfo(Instruction &I, dtrans::StructInfo *StInfo,
                       size_t FieldNum, bool DescendIntoNested) {
    if (DescendIntoNested) {
      dtrans::FieldInfo &FI = getDeepestNestedField(StInfo, FieldNum);
      FI.setRead(I);
      if (!dtrans::isLoadedValueUnused(&I,
                                       cast<LoadInst>(&I)->getPointerOperand()))
        FI.setValueUnused(false);
    } else {
      dtrans::FieldInfo &FI = StInfo->getField(FieldNum);
      FI.setRead(I);
      FI.setValueUnused(false);
    }
    // TODO: set other attributes like usage frequency, etc.
  }

  void collectWriteInfo(Instruction &I, dtrans::StructInfo *StInfo,
                        size_t FieldNum, Value *WriteVal,
                        bool DescendIntoNested) {
    if (DescendIntoNested) {
      dtrans::FieldInfo &FI = getDeepestNestedField(StInfo, FieldNum);
      FI.setWritten(I);
    } else {
      dtrans::FieldInfo &FI = StInfo->getField(FieldNum);
      FI.setWritten(I);
    }

    // TODO: Update field usage frequency, field single value, field single
    // allocation functions fields
  }

  // When the pointer type analyzer detects an inner element of a nested type
  // being loaded via a pointer to the container type, it records the container
  // type as the element pointee. In these cases the type nest needs to be
  // traversed to mark the actual field as being 'read' or 'written'. This
  // function walks the structure to return the field accessed.
  dtrans::FieldInfo &getDeepestNestedField(dtrans::StructInfo *StInfo,
                                           size_t FieldNum) {
    DTransType *ElemTy = StInfo->getField(FieldNum).getDTransType();
    while (ElemTy->isAggregateType()) {
      if (auto *StElemTy = dyn_cast<DTransStructType>(ElemTy)) {
        if (StElemTy->getNumFields() == 0)
          break;

        // For a nested structure, the field to be examined will be the first
        // element because that field will have the same address as the
        // structure itself.
        //   %struct.A = { i32, %struct.B }
        //   %struct.B = { i32, i64 }
        // The address of Field 1 of %struct.A is also the address of the 'i32'
        // within %struct.B
        FieldNum = 0;
        StInfo = cast<StructInfo>(DTInfo.getOrCreateTypeInfo(StElemTy));
        ElemTy = StInfo->getField(FieldNum).getDTransType();
      } else if (auto *ArElemTy = dyn_cast<DTransArrayType>(ElemTy)) {
        // Handle the case of the element being an array of nested structures by
        // finding the structure type. If it's not an array of structures, then
        // the access is to the first element of the array, and there is no need
        // to continue deeper into the nesting.
        // Examples:
        //   [4 x %struct.A] - iterate over element that starts %struct.A
        //   [4 x i32] - cannot continue iterating
        DTransType *MemberTy = getArrayUnitType(ArElemTy);
        if (MemberTy->isStructTy())
          ElemTy = MemberTy;
        else
          break;
      }
    }

    FieldInfo &FI = StInfo->getField(FieldNum);
    return FI;
  }

private:
  // private methods

  // Return 'true' if the ValueTypeInfo object is invalid for safety analysis
  // due to the PointerTypeAnalyzer not being able to collect types for it.
  bool isValueTypeInfoUnhandled(const ValueTypeInfo &Info) {
    return Info.getUnhandled() || Info.getDependsOnUnhandled();
  }

  // Helper to get the deepest type that makes up an array.
  DTransType *getArrayUnitType(DTransArrayType *ArTy) {
    DTransType *BaseTy = ArTy;
    while (BaseTy->isArrayTy())
      BaseTy = BaseTy->getArrayElementType();

    return BaseTy;
  }

  // Constant expressions can be used to access global variables and fields
  // contained within them. Make sure the accesses are safe.
  void analyzeConstantExpr(ConstantExpr *CE) {
    // Ignore bitcast operators because any problems should be detected when the
    // result of the bitcast is used. The ValueTypeInfo object will contain the
    // types of the source pointer and the type it gets used as. With opaque
    // pointers, we don't expect to see bitcast operators, just uses of the
    // pointer as the type that it would have been bitcast to.
    if (!isa<BitCastOperator>(CE)) {
      // TODO: Extend this to handle other constant expression types, as needed.
      if (auto *GEPOp = dyn_cast<GEPOperator>(CE)) {
        analyzeGEPOperator(GEPOp);
      } else {
        ValueTypeInfo *CEInfo = PTA.getValueTypeInfo(CE);
        if (CEInfo)
          setAllAliasedTypeSafetyData(
              CEInfo, dtrans::UnhandledUse,
              "Analysis of constant expression not implemented yet", CE);
      }
    }

    for (auto *U : CE->users())
      if (auto *UCE = dyn_cast<ConstantExpr>(U))
        analyzeConstantExpr(UCE);
  }

  // Check whether any of the aliases that were collected for the declaration of
  // 'V' are incompatible when used as pointers to an object of type
  // 'ExpectedTy'. This is used when a object has a dominant aggregate type, but
  // can alias multiple aggregate types. This can occur because of nested types,
  // where the address of an element is shared with the container itself.
  // For example:
  //   %struct.inside = { i32 }
  //   %struct.outside = { %struct.inside, i64 }
  //   %inside = alloca %struct.inside
  //   %t = getelementptr %struct.outside, p0 %inside, i64 0, i32 1
  // A %struct.outside* value can be used to access either type. However, a
  // %struct.inside* that has been cast to be used as an %struct.outside* should
  // trigger a safety violation. This function is to check for such a case. In
  // the above, when analyzing the getelementptr instruction, the 'ExpectedTy'
  // parameter will be %struct.outside because that is the type specified as
  // being indexed. However, the pointer operand, %inside, whose alias set is
  // passed in the 'Info' parameter, was originally defined as a %struct.inside.
  // Returns 'true' to indicate the use is incompatible.
  bool hasIncompatibleAggregateDecl(DTransType *ExpectedTy,
                                    ValueTypeInfo *Info) {
    auto AreTypesCompatible = [this](DTransType *AliasTy,
                                     DTransType *ExpectedTy) {
      if (!AliasTy || !ExpectedTy)
        return false;

      if (ExpectedTy == AliasTy)
        return true;

      // Any pointer object may be safely used as a pointer sized int type.
      if (AliasTy->isPointerTy() && ExpectedTy == getDTransPtrSizedIntType())
        return true;

      // Check if the 'Expected' type can be used for the element zero type of
      // 'AliasTy'
      if (PTA.isPointeeElementZeroAccess(AliasTy, ExpectedTy))
        return true;

      return false;
    };

    if (!Info)
      return true;

    auto &AliasSet = Info->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl);
    // If a declaration type is not available, then we cannot tell whether the
    // type is compatible.
    if (AliasSet.empty())
      return true;

    bool SawAggregate = false;
    for (auto *ValAliasTy : AliasSet) {
      if (!ValAliasTy->isPointerTy())
        return true;

      DTransType *BaseTy = ValAliasTy->getPointerElementType();
      while (BaseTy->isPointerTy())
        BaseTy = BaseTy->getPointerElementType();
      if (!BaseTy->isAggregateType())
        continue;

      SawAggregate = true;
      if (!AreTypesCompatible(ValAliasTy->getPointerElementType(),
                              ExpectedTy)) {
        return true;
      }
    }

    // If an aggregate type was expected and the declaration type was not known
    // to be an aggregate type, then it must be a pointer to a scalar type,
    // which would not be compatible with an aggregate type.
    if (ExpectedTy->isAggregateType() && !SawAggregate)
      return true;

    return false;
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
                                 StringRef Reason, Value *V) {
    DEBUG_WITH_TYPE_P(FNFilter, SAFETY_VERBOSE, {
      if (!Reason.empty()) {
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

    setBaseTypeInfoSafetyDataImpl(Ty, Data, isCascadingSafetyCondition(Data),
                                  isPointerCarriedSafetyCondition(Data));
  }

  // Set the safety data on all the aliased types of 'PtrInfo'
  // 'Reason' and 'V' are optional parameters that provide for debug traces.
  void setAllAliasedTypeSafetyData(ValueTypeInfo *PtrInfo,
                                   dtrans::SafetyData Data, StringRef Reason,
                                   Value *V) {
    DEBUG_WITH_TYPE_P(FNFilter, SAFETY_VERBOSE, {
      if (!Reason.empty()) {
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
        PtrInfo->print(dbgs(), /*Combined=*/false, "    ");
      }
    });

    for (auto *AliasTy :
         PtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
      setBaseTypeInfoSafetyData(AliasTy, Data, "", nullptr);
  }

  // Set the safety data on all the aliased types of 'PtrInfo' and all element
  // pointee member types.
  //
  // For example, a Value may have the following pointer info to indicate that
  // it is for a pointer to a structure field.  The value is a pointer to a
  // %struct.B*, which is a member field of %struct.A.
  //   Aliased types:
  //     %struct.B**
  //   Element pointees:
  //     %struct.A @ 0
  //
  // This routine will set the safety information set on both %struct.B and
  // %struct.A
  void setAllAliasedAndPointeeTypeSafetyData(ValueTypeInfo *PtrInfo,
                                             dtrans::SafetyData Data,
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
        PtrInfo->print(dbgs(), /*Combined=*/false, "    ");
      }
    });

    for (auto *AliasTy :
         PtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
      setBaseTypeInfoSafetyData(AliasTy, Data, "", nullptr);

    for (auto &PointeePair :
         PtrInfo->getElementPointeeSet(ValueTypeInfo::VAT_Use))
      setBaseTypeInfoSafetyData(PointeePair.first, Data, "", nullptr);
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
  void setBaseTypeInfoSafetyDataImpl(DTransType *Ty, dtrans::SafetyData Data,
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

    if (!BaseTy->isAggregateType())
      return;

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
        setBaseTypeInfoSafetyDataImpl(FieldTy, Data, IsCascading,
                                      IsPointerCarried);
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
          setBaseTypeInfoSafetyDataImpl(FieldBaseTy, Data, IsCascading,
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

  // Return 'true' if the DTransType is something that may require safety data
  // to be set on it.
  bool isTypeOfInterest(DTransType *Ty) {
    DTransType *BaseTy = Ty;
    while (BaseTy->isPointerTy() || BaseTy->isVectorTy())
      if (BaseTy->isPointerTy())
        BaseTy = BaseTy->getPointerElementType();
      else
        BaseTy = BaseTy->getVectorElementType();

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
    DTransTI = new dtrans::StructInfo(Ty, FieldTypes);
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

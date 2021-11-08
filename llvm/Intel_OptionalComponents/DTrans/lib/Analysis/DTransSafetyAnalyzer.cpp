//===----------------------DTransSafetyAnalyzer.cpp-----------------------===//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"

#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAllocAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransDebug.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/DTransCommon.h"

#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/Intel_LangRules.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FormattedStream.h"

#define DEBUG_TYPE "dtrans-safetyanalyzer"

// Print detailed messages regarding the safety checks.
#define SAFETY_VERBOSE "dtrans-safetyanalyzer-verbose"

// Print messages regarding the collection of values stored to fields of
// structures.
#define SAFETY_VALUES "dtrans-safetyanalyzer-values"

// Debug type for verbose field single alloc function analysis output.
#define SAFETY_FSAF "dtrans-safety-analyzer-fsaf"

using namespace llvm;
using namespace dtransOP;

// Control whether field frequency info should use the BlockFrequencyInfo class
// to estimate access frequencies for the structure fields. When false, a simple
// static instruction count is used instead.
static cl::opt<bool> DTransUseBlockFreq(
    "dtrans-use-block-freq", cl::init(false), cl::ReallyHidden,
    cl::desc("Use BlockFrequencyInfo counters instead of static instruction "
             "counts for field frequency info"));

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

// Trace option to print the IR instructions with comments inserted that
// show the safety flags that are triggered by the instruction.
static cl::opt<bool> PrintSafetyAnalyzerIR(
    "dtrans-print-safetyanalyzer-ir", cl::ReallyHidden,
    cl::desc("Print IR with safety analyzer comments after "
             "instructions that trigger safety flags"));

// A trace filter that will be enabled/disabled based on the function name.
static llvm::dtrans::debug::DebugFilter FNFilter;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Helper method to check whether 'Ty' is a pointer to a pointer.
static bool isPtrToPtr(const DTransType *Ty) {
  return Ty->isPointerTy() && Ty->getPointerElementType()->isPointerTy();
}

static bool isCompilerConstantData(Value *V) {
  return isa<ConstantData>(V);
}

// Helper to print a Value object (and optionally the name of the function it
// belongs to, if there is one). If the Value represents a GlobalValue, such as
// a Function or GlobalVariable, just print the object's name instead of the
// complete definition, otherwise print the Value.
static void printValue(raw_ostream &OS, Value *V, bool ReportFuncName = true) {
  if (!V)
    return;

  if (ReportFuncName) {
    Function *F = nullptr;
    if (auto *I = dyn_cast<Instruction>(V))
      F = I->getFunction();
    else if (auto *Arg = dyn_cast<Argument>(V))
      F = Arg->getParent();
    if (F)
      dbgs() << "[" << F->getName() << "] ";
  }

  if (auto *GV = dyn_cast<GlobalObject>(V))
    OS << "@" << GV->getName();
  else
    OS << *V;
}

// Type used for callback function for reporting additional details in the debug
// trace messages when setting safety flags.
using SafetyInfoReportCB = std::function<void()>;

// This structure is used to store information about a safety flag being set on
// a type to allow printing the IR with the comment message that indicates the
// safety conditions triggered by an instruction. In debug builds, a mapping can
// be kept to map a Value object to a set of SafeInfoLog objects that were
// triggered by that Value.
struct SafetyInfoLog {
  DTransType *Ty;
  dtrans::SafetyData SafetyFlag;
  bool IsCascaded;
  bool IsPointerCarried;

  SafetyInfoLog(DTransType *Ty, dtrans::SafetyData SafetyFlag, bool IsCascaded,
                bool IsPointerCarried)
      : Ty(Ty), SafetyFlag(SafetyFlag), IsCascaded(IsCascaded),
        IsPointerCarried(IsPointerCarried) {}

  // This comparison operator is only suitable for avoiding duplicates when
  // storing this object in a container. It is not suitable for maintaining a
  // deterministic printing order because it is relying on the address of
  // objects.
  bool operator<(const SafetyInfoLog &Other) const {
    if (Ty != Other.Ty)
      return Ty < Other.Ty;
    if (SafetyFlag != Other.SafetyFlag)
      return SafetyFlag < Other.SafetyFlag;
    if (IsCascaded != Other.IsCascaded)
      return IsCascaded < Other.IsCascaded;
    return IsPointerCarried < Other.IsPointerCarried;
  }
};

// Implementation of a DTransSafetyLogger that the DTransSafetyInstVisitor and
// SafetyAnnotationWriter classes will use to save and retrieve log messages for
// use in a debug build. In the debug build this will store a mapping of Value
// objects to SafetyInfoLog objects for printing as comments with IR when using
// the flag -print-dtrans-safetyanalyzer-ir.
class DTransSafetyLogger {
public:
  void log(Value *V, const SafetyInfoLog &Log) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (V)
      ValueToSafetyInfo[V].insert(Log);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  }

  void lookupSafetyFlagsForValue(Value *V, std::set<SafetyInfoLog> &SD) const {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    auto It = ValueToSafetyInfo.find(V);
    if (It != ValueToSafetyInfo.end())
      SD = It->second;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  }

private:
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::map<Value *, std::set<SafetyInfoLog>> ValueToSafetyInfo;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Class that can be called by the AsmWriter to print comments about the DTrans
// SafetyData values triggered by an instruction.
class SafetyAnnotationWriter : public AssemblyAnnotationWriter {
public:
  SafetyAnnotationWriter(const DTransSafetyLogger &Log) : Log(Log) {}

  // Emit a comment after an Instruction or GlobalValue is printed.
  void printInfoComment(const Value &CV, formatted_raw_ostream &OS) override {
    // Lambda used to print the comment to a string buffer to allow sorting when
    // there are multiple comments to be emitted for the instruction.
    auto LogToString = [](const SafetyInfoLog &Log) {
      std::string OutputVal;
      raw_string_ostream OutputStream(OutputVal);
      OutputStream << "\n    ; -> Safety data: ";
      Log.Ty->print(OutputStream, false);
      OutputStream << " : " << dtrans::getSafetyDataName(Log.SafetyFlag);
      if (Log.IsCascaded)
        OutputStream << "[Cascaded]";
      if (Log.IsPointerCarried)
        OutputStream << "[PtrCarried]";

      OutputStream.flush();
      return OutputVal;
    };

    Value *V = const_cast<Value *>(&CV);
    std::set<SafetyInfoLog> Data;
    Log.lookupSafetyFlagsForValue(V, Data);
    if (!Data.empty())
      dtrans::printCollectionSorted(OS, Data.begin(), Data.end(), "",
                                    LogToString);
  }

private:
  const DTransSafetyLogger &Log;
};
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// This class is responsible for analyzing the LLVM IR using information
// collected by the PtrTypeAnalyzer class to mark the DTrans safety bits on the
// TypeInfo objects managed by the DTransSafetyInfo class. This will also
// collect the field usage information for structure fields.
class DTransSafetyInstVisitor : public InstVisitor<DTransSafetyInstVisitor> {

  // Value type written for field value analysis by a memfunc call to indicate
  // whether a zero value, non-zero value, or the existing value of a field is
  // being written by the call.
  enum FieldWriteType { FWT_ZeroValue, FWT_NonZeroValue, FWT_ExistingValue };

public:
  DTransSafetyInstVisitor(
      LLVMContext &Ctx, const DataLayout &DL,
      DTransSafetyInfo::GetTLIFnType GetTLI,
      function_ref<BlockFrequencyInfo &(Function &)> &GetBFI,
      DTransSafetyInfo &DTInfo, PtrTypeAnalyzer &PTA, DTransTypeManager &TM,
      TypeMetadataReader &MDReader, DTransSafetyLogger &Log)
      : DL(DL), GetTLI(GetTLI), GetBFI(GetBFI), DTInfo(DTInfo), PTA(PTA),
        MDReader(MDReader), TM(TM), Log(Log) {
    DTransI8Type = TM.getOrCreateAtomicType(llvm::Type::getInt8Ty(Ctx));
    DTransI8PtrType = TM.getOrCreatePointerType(DTransI8Type);
    LLVMPtrSizedIntType = llvm::Type::getIntNTy(Ctx, DL.getPointerSizeInBits());
    DTransPtrSizedIntType = TM.getOrCreateAtomicType(LLVMPtrSizedIntType);
    DTransPtrSizedIntPtrType = TM.getOrCreatePointerType(DTransPtrSizedIntType);
  }

  DTransType *getDTransI8Type() const { return DTransI8Type; }
  DTransPointerType *getDTransI8PtrType() const { return DTransI8PtrType; }
  DTransType *getDTransPtrSizedIntType() const { return DTransPtrSizedIntType; }
  DTransPointerType *getDTransPtrSizedIntPtrType() const {
    return DTransPtrSizedIntPtrType;
  }

  llvm::Type *getLLVMPtrSizedIntType() const { return LLVMPtrSizedIntType; }

  void visitModule(Module &M) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    // Reset the state of the trace filter to the default value each time we
    // start visiting a module.
    FNFilter.reset();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    // Before visiting each Function, ensure that the DTransTypes seen from the
    // PtrTypeAnalyzer are in the type_info_entries, so that remaing visit*
    // functions are just looking up types and not creating new TypeInfo
    // objects. This is absolutely needed so we can determine if any actual
    // argument of an indirect or external call could be subject to an
    // AddressTaken safety violation due to an actual/formal argument mismatch,
    // otherwise we would need to walk all the IR now looking for indirect
    // function calls to create types for all the type aliases that may be
    // passed.
    for (auto* DPTy : TM.dtrans_types())
      (void)DTInfo.getOrCreateTypeInfo(DPTy);

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
    dtrans::FieldInfo &LastField = StructInfo->getField(NumElements - 1);
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
        dtrans::TypeInfo *ContainedStInfo = DTInfo.getTypeInfo(StTy);
        assert(ContainedStInfo &&
               "visitModule() should create all TypeInfo objects");
        ContainedStInfo->setSafetyData(dtrans::NestedStruct);
      }

      // Check for function pointer fields and potential vtable fields.
      if (FieldType->isPointerTy()) {
        if (FieldType->getPointerElementType()->isFunctionTy()) {
          DEBUG_WITH_TYPE(SAFETY_VERBOSE,
                          dbgs() << "dtrans-safety: Has function ptr: "
                                 << *StructInfo->getDTransType() << "\n");
          StructInfo->setSafetyData(dtrans::HasFnPtr);
        } else if (isa<DTransPointerType>(FieldType->getPointerElementType()) &&
                   isVTableType(FieldType)) {
          // Fields matching this check might not actually be vtable
          // pointers, but we will treat them as though they are since the
          // false positives do not affect any cases we are currently
          // interested in.
          DEBUG_WITH_TYPE(SAFETY_VERBOSE,
                          dbgs() << "dtrans-safety: Has vtable-like element: "
                                 << *StructInfo->getDTransType() << "\n"
                                 << "  field: " << *FieldType << "\n");
          StructInfo->setSafetyData(dtrans::HasVTable);
        }
      }
    }
  }

  // Analyze global variables to mark "Global array", Global instance", and
  // "Global pointer" safety flags on types of interest. Also, analyze the
  // initializer value to collect constants, and check for incompatible types
  // being stored into the global.
  void analyzeGlobalVariable(GlobalVariable &GV) {
    // Check whether the initialization value is a global object that
    // is being used as a type that is not compatible with the expected type
    // based on the DTrans type information for a structure field or array
    // element. If so, set a safety flag.
    //
    auto CheckInitializerCompatibility = [this](GlobalVariable &GV,
                                                DTransType *DType,
                                                Constant *ConstVal) {
      assert(DType && "Expected valid DTransType object");
      assert(ConstVal && "Expected valid Constant object");

      // No need to check 'zeroinitializer' for compatibility.
      if (ConstVal->isZeroValue())
        return;

      // If the initializer is getting the address from another initialized
      // field, try to check if whether address type matches the type expected.
      // If it matches here, then no safety flag needs to be set. Otherwise,
      // fallthru to the remaining checks of this function, which may result in
      // the "Unsafe pointer store" safety flag being set.
      if (auto *GEP = dyn_cast<GEPOperator>(ConstVal)) {
        ValueTypeInfo *GepInfo = PTA.getValueTypeInfo(GEP);
        if (GepInfo) {
          DTransType *InitializerType =
              PTA.getDominantType(*GepInfo, ValueTypeInfo::VAT_Decl);
          if (InitializerType && InitializerType == DType)
            return;
        }
      }

      // When opaque pointers are in use, the Constant can be used without the
      // intervening bitcasts, but there still may be a getelementptr operator
      // that converts from an address of a variable that is an array to an
      // address of the first element of the array, so strip these away.
      if (ConstVal->getType()->isPointerTy())
        ConstVal = ConstVal->stripPointerCasts();

      if (auto *InitializerGlobObj = dyn_cast<GlobalObject>(ConstVal)) {
        ValueTypeInfo *Info = PTA.getValueTypeInfo(InitializerGlobObj);
        assert(Info && "Expected pointer type analyzer to collect type "
                       "for GlobalObject");
        if (!Info->canAliasToAggregatePointer()) {
          DTransType *BaseType = DType;
          while (BaseType->isPointerTy() || BaseType->isArrayTy())
            if (BaseType->isPointerTy())
              BaseType = BaseType->getPointerElementType();
            else
              BaseType = BaseType->getArrayElementType();

          bool ExpectStructPtr = BaseType->isStructTy();
          if (ExpectStructPtr) {
            ValueTypeInfo *InitializeeInfo = PTA.getValueTypeInfo(&GV);
            assert(InitializeeInfo && "Expected pointer type analyzer to "
                                      "collect type for GlobalVariable");
            setAllAliasedTypeSafetyData(InitializeeInfo,
                                        dtrans::UnsafePointerStore,
                                        "Object used to initialize an "
                                        "incompatible structure field",
                                        &GV);
          }
          return;
        }

        DTransType *InitializerType =
            PTA.getDominantType(*Info, ValueTypeInfo::VAT_Decl);
        if (InitializerType) {
          if (InitializerType == DType)
            return;

          // Allow a pointer type to be initialized with a pointer to an array
          // of the type. For example: An i8* structure field can be initialized
          // with
          //   i8* getelementptr ([4 x i8], [4 x i8]* @.str.27.213, i32 0, i32 0
          if (DType->isPointerTy() && InitializerType->isPointerTy() &&
              InitializerType->getPointerElementType()->isArrayTy() &&
              InitializerType->getPointerElementType()->getArrayElementType() ==
                  DType->getPointerElementType())
            return;
        }

        // Mark the value being stored and the global variable that was
        // being initialized with an appropriate safety flag.
        setAllAliasedTypeSafetyData(Info, dtrans::BadCasting,
                                    "Object used to initialize an "
                                    "incompatible structure field",
                                    &GV);

        ValueTypeInfo *InitializeeInfo = PTA.getValueTypeInfo(&GV);
        assert(InitializeeInfo && "Expected pointer type analyzer to "
                                  "collect type for GlobalVariable");
        setAllAliasedTypeSafetyData(InitializeeInfo, dtrans::UnsafePointerStore,
                                    "Object used to initialize an "
                                    "incompatible structure field",
                                    &GV);
      }
    };

    // Analyze the initializer of a global variable, updating the field value
    // analysis and safety data. Return 'false' if the initializer cannot be
    // processed.
    std::function<bool(GlobalVariable &, DTransType *, llvm::Constant *)>
        AnalyzeInitializer =
            [this, &AnalyzeInitializer, &CheckInitializerCompatibility](
                GlobalVariable &GV, DTransType *DTy,
                llvm::Constant *Init) -> bool {
      assert(Init && "Initializer constant must not be nullptr");

      // No value collection or checks are needed for 'undef'
      if (isa<UndefValue>(Init))
        return true;

      if (auto *DTransStTy = dyn_cast<DTransStructType>(DTy)) {
        dtrans::TypeInfo *TI = DTInfo.getTypeInfo(DTransStTy);
        assert(TI && "visitModule() should create all TypeInfo objects");
        auto *StInfo = cast<dtrans::StructInfo>(TI);
        unsigned NumFields = StInfo->getNumFields();
        if (!isa<ConstantAggregateZero>(Init) &&
            NumFields != Init->getNumOperands())
          return false;

        for (unsigned I = 0; I < NumFields; ++I) {
          llvm::Constant *ConstVal = Init->getAggregateElement(I);
          dtrans::FieldInfo &FI = StInfo->getField(I);
          if (FI.getLLVMType()->isAggregateType()) {
            AnalyzeInitializer(GV, FI.getDTransType(), ConstVal);
          } else {
            updateFieldValueTracking(*StInfo, FI, I, ConstVal, &GV);
            CheckInitializerCompatibility(GV, FI.getDTransType(), ConstVal);
          }
          if (!isa<ConstantPointerNull>(ConstVal)) {
            DEBUG_WITH_TYPE(SAFETY_FSAF, {
              if (!FI.isBottomAllocFunction())
                dbgs() << "dtrans-fsaf: " << *(StInfo->getLLVMType()) << " ["
                       << I << "] <BOTTOM>\n";
            });
            FI.setBottomAllocFunction();
          }
        }
      } else if (auto *DTransArTy = dyn_cast<DTransArrayType>(DTy)) {
        if (!isa<ConstantAggregateZero>(Init) && !isa<ConstantArray>(Init))
          return false;

        // An array of scalar elements does not need further evaluation because
        // DTrans does not make use of the values that are stored in the
        // array. However, if there is an array of structures or structure
        // pointers, then analysis of those needs to be done to find any
        // initializers of the structure fields, or incompatible initializers.
        auto *ElemTy = DTransArTy->getArrayElementType();
        for (unsigned I = 0, E = DTransArTy->getNumElements(); I < E; ++I) {
          llvm::Constant *ConstVal = Init->getAggregateElement(I);
          if (ElemTy->isAggregateType())
            AnalyzeInitializer(GV, ElemTy, ConstVal);
          else
            CheckInitializerCompatibility(GV, ElemTy, ConstVal);
        }
      }

      return true;
    };

    // Declarations do not need to be analyzed
    if (GV.isDeclaration())
      return;

    DEBUG_WITH_TYPE(SAFETY_VERBOSE,
                    dbgs() << "Analyzing global var: " << GV << "\n");

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

    // The local linkage check should guarantee a unique and definitive
    // initializer.
    assert((GV.hasUniqueInitializer() && GV.hasDefinitiveInitializer()) &&
           "Expected initializer");

    // The global variables that are going to be processed will either have a
    // 'zeroinitializer', 'undef', or an initializer list that specifies the
    // value for the variable.
    Constant *Initializer = GV.getInitializer();
    bool HasNonZeroInitializer = !isa<ConstantAggregateZero>(Initializer) &&
                                 !isa<UndefValue>(Initializer);

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
        if (UnitTy->isPointerTy()) {
          setBaseTypeInfoSafetyData(UnitTy, dtrans::GlobalPtr,
                                    "Global array of pointers to type defined",
                                    &GV);
          if (HasNonZeroInitializer) {
            DTransType *ElemTy = ArTy->getArrayElementType();
            for (unsigned I = 0, E = ArTy->getNumElements(); I < E; ++I) {
              llvm::Constant *ConstVal = Initializer->getAggregateElement(I);
              CheckInitializerCompatibility(GV, ElemTy, ConstVal);
            }
          }
        } else if (UnitTy->isVectorTy()) {
          setBaseTypeInfoSafetyData(AliasTy, dtrans::UnhandledUse,
                                    "Global array of vector type defined", &GV);
        } else {
          setBaseTypeInfoSafetyData(AliasTy, dtrans::GlobalInstance,
                                    "Global array of type defined", &GV);
          if (!AnalyzeInitializer(GV, ElemTy, Initializer))
            setBaseTypeInfoSafetyData(
                AliasTy, dtrans::UnhandledUse,
                "dtrans-safety: Initializer list does not match expected type",
                &GV);

          if (HasNonZeroInitializer)
            setBaseTypeInfoSafetyData(AliasTy, dtrans::HasInitializerList,
                                      "dtrans-safety: Has initializer list",
                                      &GV);
        }
        continue;
      }

      if (ElemTy->isPointerTy()) {
        setBaseTypeInfoSafetyData(AliasTy, dtrans::GlobalPtr,
                                  "Pointer allocated", &GV);
      } else if (ElemTy->isVectorTy()) {
        setBaseTypeInfoSafetyData(AliasTy, dtrans::UnhandledUse,
                                  "Vector allocated", &GV);
      } else {
        setBaseTypeInfoSafetyData(AliasTy, dtrans::GlobalInstance,
                                  "Instance allocated", &GV);
        if (!AnalyzeInitializer(GV, ElemTy, Initializer))
          setBaseTypeInfoSafetyData(
              AliasTy, dtrans::UnhandledUse,
              "dtrans-safety: Initializer list does not match expected type",
              &GV);

        if (HasNonZeroInitializer)
          setBaseTypeInfoSafetyData(AliasTy, dtrans::HasInitializerList,
                                    "dtrans-safety: Has initializer list", &GV);
      }
    }
  }

  void visitFunction(Function &F) {
    LLVM_DEBUG(dbgs() << "visitFunction: " << F.getName() << "\n");

    // Get BFI if available.
    BFI = (!F.isDeclaration()) ? &(GetBFI(F)) : nullptr;

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

    if (isValueTypeInfoUnhandled(*PtrInfo)) {
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
      std::function<bool(Value *)> HasNonCastLoadStoreUses =
          [&HasNonCastLoadStoreUses](Value *V) {
            for (auto *U : V->users()) {
              if (isa<LoadInst>(U) || isa<StoreInst>(U))
                continue;
              if (isa<CastInst>(U)) {
                if (HasNonCastLoadStoreUses(U))
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
              return !PointeePair.second.isField();
            });
      };

      auto &Pointees = GEPInfo->getElementPointeeSet(ValueTypeInfo::VAT_Use);
      if (HasNonFieldAddress(GEPInfo)) {
#if !defined(NDEBUG)
        for (auto &PointeePair : Pointees) {
          // The pointer type analyzer should only have set a non-field byte
          // offset for the case of a GEP that is passed to a memset and the
          // offset does not correspond to a field boundary.
          if (PointeePair.second.isByteOffset())
            assert(
                dtrans::valueOnlyUsedForMemset(GEP) &&
                "Non-field accesses expected to only occur for memset operand");
        }
#endif // !defined(NDEBUG)
      } else if (HasNonCastLoadStoreUses(GEP)) {
        // Set GEPs for fields of structures that are used for something other
        // than a Load/Store as ComplexUse.
        for (auto &PointeePair : Pointees) {
          DTransType *ParentTy = PointeePair.first;
          if (ParentTy->isStructTy() && PointeePair.second.isField()) {
            dtrans::TypeInfo *TI = DTInfo.getTypeInfo(ParentTy);
            assert(TI && "visitModule() should create all TypeInfo objects");
            auto *ParentStInfo = cast<dtrans::StructInfo>(TI);
            dtrans::FieldInfo &FI =
                ParentStInfo->getField(PointeePair.second.getElementNum());
            FI.setComplexUse(true);
          }
        }
      }

      // A runtime dependent index of an array cannot be guaranteed to be within
      // the bounds. When LangRuleOutOfBoundsOK is not set, we are explicitly
      // asserting that the access cannot go out of bounds.
      if (getLangRuleOutOfBoundsOK())
        for (auto &PointeePair : Pointees) {
          if (PointeePair.second.getKind() ==
              ValueTypeInfo::PointeeLoc::PLK_UnknownOffset) {
            setBaseTypeInfoSafetyData(PointeePair.first,
                                      dtrans::BadPtrManipulation,
                                      "Runtime dependent offset", GEP);
            for (auto &ElementOfPair : PointeePair.second.getElementOf())
              setBaseTypeInfoSafetyData(ElementOfPair.first,
                                        dtrans::BadPtrManipulation,
                                        "Runtime dependent offset", GEP);
          }
        }
    }
  }

  void visitSelectInst(SelectInst &Sel) {
    SmallVector<Value *, 4> IncomingVals;
    IncomingVals.push_back(Sel.getTrueValue());
    IncomingVals.push_back(Sel.getFalseValue());

    analyzeSelectOrPhi(&Sel, IncomingVals);
  }

  void visitPHINode(PHINode &Phi) {
    SmallVector<Value *, 4> IncomingVals;
    for (Value *Val : Phi.incoming_values())
      IncomingVals.push_back(Val);

    analyzeSelectOrPhi(&Phi, IncomingVals);
  }

  // If the select or phi involves pointer types or element pointees, then
  // the ValueTypeInfo for the instruction will hold the merge results of all
  // the source operands. For type safety, if there are aggregate types
  // involved, we need to be sure there is a unique dominant type when aggregate
  // types are involved, otherwise it is an unsafe pointer merge.
  void analyzeSelectOrPhi(Instruction *I,
                          SmallVectorImpl<Value *> &IncomingVals) {
    // If the select/phi instruction was not identified as a type of interest by
    // the PtrTypeAnalyzer there will not be a ValueTypeInfo collected for it.
    ValueTypeInfo *Info = PTA.getValueTypeInfo(I);
    if (!Info || Info->empty())
      return;

    if (isValueTypeInfoUnhandled(*Info))
      DTInfo.setUnhandledPtrType(I);

    if (!Info->canAliasToAggregatePointer())
      return;

    if (!PTA.getDominantAggregateUsageType(*Info))
      setAllAliasedTypeSafetyData(Info, dtrans::UnsafePtrMerge,
                                  "Merge of conflicting pointer types", I);

    // The merge is taking place on a type that was known to alias a pointer to
    // an aggregate type. If this is being done with the pointer having been
    // converted to an integer, check to ensure that all Values being merged
    // were for pointers to aggregate types. There is no need to check the
    // specific types of the aliases because the above check for the dominant
    // type would have determined that. For example, the following would be
    // unsafe because %n was not known to be an aggregate type.
    //   define void @test01(%struct.test01* %pStruct1, i64 %n) {
    //     %tmp = ptrtoint % struct.test01* %pStruct1 to i64
    //     %sel = select i1 undef, i64 %tmp, i64 %n
    //     ret void
    //   }
    if (!I->getType()->isPointerTy())
      for (auto *ValIn : IncomingVals) {
        if (isa<ConstantData>(ValIn)) {
          setAllAliasedTypeSafetyData(
              Info, dtrans::UnsafePtrMerge,
              "Merge of conflicting types during integer merge", I);
          return;
        }

        ValueTypeInfo *SrcInfo = PTA.getValueTypeInfo(ValIn);
        if (!SrcInfo || !SrcInfo->canAliasToAggregatePointer()) {
          setAllAliasedTypeSafetyData(
              Info, dtrans::UnsafePtrMerge,
              "Merge of conflicting types during integer merge", I);
          return;
        }
      }
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
    // Ignore loads from constant locations like null or undef.
    if (isCompilerConstantData(Ptr))
      return;

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
      } else if ((PtrDomTy->getPointerElementType() != ValTy &&
                  !PtrInfo->getIsPartialPointerUse()) ||
                 (PtrInfo->canAliasMultipleAggregatePointers() &&
                  hasIncompatibleAggregateDecl(PtrDomTy, PtrInfo))) {
        IsMismatched = true;
        BadcastReason =
            "Dominant types for value and pointer are not compatible";
      } else if (!PtrInfo->getIsPartialPointerUse() &&
                 (!ValInfo || !ValInfo->canAliasToAggregatePointer())) {
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
        dtrans::TypeInfo *TI = DTInfo.getTypeInfo(StructTy);
        assert(TI && "visitModule() should create all TypeInfo objects");
        dtrans::StructInfo *SI = cast<dtrans::StructInfo>(TI);
        if (SI->getNumFields() != 0) {
          collectReadInfo(I, SI, /*FieldNum=*/0, IsWholeStructureRead,
                          /*ForElementZeroAccess=*/true);

          if (IsWholeStructureRead) {
            // Note: For whole structure reference, DTrans does not fill in all
            // the field info details for each field, such as frequency or
            // single value because we do not have any transforms that can
            // handle them.
            for (auto &FI : SI->getFields()) {
              FI.setRead(I);
              FI.setValueUnused(false);
              updateFieldFrequency(FI, I);
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

    // Callback method for when a safety flag is set to report the ValueTypeInfo
    // object for the value and store operands of the StoreInst.
    auto DumpCallback = [ValInfo, PtrInfo]() {
      (void)ValInfo;
      (void)PtrInfo;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      dbgs() << "  Value op info:\n";
      if (ValInfo)
        ValInfo->print(dbgs(), /*CombinedTypes=*/false, "    ");
      dbgs() << "  Ptr op info:\n";
      if (PtrInfo)
        PtrInfo->print(dbgs(), /*CombinedTypes=*/false, "    ");
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    };

    // Check that the address of a structure field is not being stored to
    // memory. This is done up front, so that we can return as soon as
    // analyzing any element pointees of the pointer operand.
    if (ValInfo && ValInfo->pointsToSomeElement())
      markFieldAddressTaken(ValInfo, "Address of member stored to memory", &I,
                            dtrans::FieldAddressTakenMemory, DumpCallback);

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
                                    "volatile store", &I, DumpCallback);

    DTransType *PtrDomTy = PTA.getDominantAggregateUsageType(*PtrInfo);
    DTransType *ValTy = getLoadStoreValueType(*Val, ValInfo, /*IsLoad=*/false);
    if (!ValTy) {
      // If we cannot determine a single specific type being stored, then we
      // need to mark a safety violation.
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
                                    "Cannot resolve type of value stored", &I,
                                    DumpCallback);
        if (ValInfo)
          setAllAliasedTypeSafetyData(ValInfo, dtrans::UnsafePointerStore,
                                      "Cannot resolve type of value stored", &I,
                                      DumpCallback);
      }

      setAllAliasedTypeSafetyData(PtrInfo, dtrans::BadCasting,
                                  "Cannot resolve type of value stored", &I,
                                  DumpCallback);
      if (ValInfo)
        setAllAliasedTypeSafetyData(ValInfo, dtrans::BadCasting,
                                    "Cannot resolve type of value stored", &I,
                                    DumpCallback);
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
                                  "store of structure type", &I, DumpCallback);
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
      } else if ((PtrDomTy->getPointerElementType() != ValTy &&
                  !PtrInfo->getIsPartialPointerUse()) ||
                 (PtrInfo->canAliasMultipleAggregatePointers() &&
                  hasIncompatibleAggregateDecl(PtrDomTy, PtrInfo))) {
        IsMismatched = true;
        BadcastReason =
            "Dominant types for value and pointer are not compatible";
      } else if (!ValInfo && !PtrInfo->getIsPartialPointerUse()) {
        IsMismatched = true;
        BadcastReason =
            "Dominant type of value pointer being stored not resolved";
      }
    }

    if (IsMismatched) {
      // If the value being stored is a pointer to an aggregate type, or the
      // location being stored to is expecting a pointer to aggregate type, then
      // we may have an Unsafe Pointer Store.
      markUnsafePointerStore(nullptr, ValInfo, PtrInfo, &I);
      setAllAliasedTypeSafetyData(PtrInfo, dtrans::BadCasting, BadcastReason,
                                  &I, DumpCallback);
      if (ValInfo)
        setAllAliasedTypeSafetyData(ValInfo, dtrans::BadCasting, BadcastReason,
                                    &I, DumpCallback);
      return;
    }

    // Treat this as a safe store. If the pointer operand alias info is not for
    // a pointer-to-pointer access, then treat it as an element-zero access and
    // update the FieldInfo for the element.
    if (PtrDomTy && !isPtrToPtr(PtrDomTy))
      if (auto *StructTy =
              dyn_cast<DTransStructType>(PtrDomTy->getPointerElementType())) {
        dtrans::TypeInfo *TI = DTInfo.getTypeInfo(StructTy);
        assert(TI && "visitModule() should create all TypeInfo objects");
        dtrans::StructInfo *SI = cast<dtrans::StructInfo>(TI);
        if (SI->getNumFields() != 0) {
          collectWriteInfo(I, SI, /*FieldNum=*/0, Val, IsWholeStructureWrite,
                           /*ForElementZeroAccess=*/true);

          if (IsWholeStructureWrite) {
            // Note: For whole structure reference, DTrans does not fill in all
            // the field info details for each field, such as frequency or
            // single value because we do not have any transforms that can
            // handle them. It's possible that the write is just copying an
            // existing version of the structure, but it's also possible to
            // perform the write using new constant values, such as:
            //   store %struct.test01 { i32 100, i32 0 }, %struct.test01* %tmp
            // For this reason, we will set all the fields as multiple value.
            for (auto &Field : enumerate(SI->getFields())) {
              dtrans::FieldInfo &FI = Field.value();
              FI.setWritten(I);
              updateFieldFrequency(FI, I);
              if (!FI.isMultipleValue()) {
                FI.setMultipleValue();
                DEBUG_WITH_TYPE(SAFETY_VALUES, {
                  dbgs() << "dtrans-values: ";
                  SI->getDTransType()->print(dbgs(), /*Detailed=*/false);
                  dbgs() << "@" << Field.index() << ": <MULTIPLE>\n  ";
                  printValue(dbgs(), &I);
                  dbgs() << "\n";
                });
              }
            }
          }
        }
      }
  }

  // The element access analysis for load and store instructions are nearly
  // identical, so we use this helper function to perform the task for both.
  void analyzeElementLoadOrStore(Instruction &I,
                                 ValueTypeInfo &PtrInfo,
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

      if (PointeePair.second.isByteOffset()) {
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
        dtrans::TypeInfo *IndexedTI = DTInfo.getTypeInfo(IndexedType);
        assert(IndexedTI && "visitModule() should create all TypeInfo objects");
        dtrans::StructInfo *IndexedStTI = cast<dtrans::StructInfo>(IndexedTI);
        for (auto &FI : IndexedStTI->getFields()) {
          if (IsLoad) {
            FI.setRead(I);
            FI.setValueUnused(false);
          } else {
            FI.setWritten(I);
          }
          updateFieldFrequency(FI, I);
        }
      }
      if (auto *StTy = dyn_cast<DTransStructType>(ParentTy)) {
        dtrans::TypeInfo *ParentInfo = DTInfo.getTypeInfo(StTy);
        assert(ParentInfo &&
               "visitModule() should create all TypeInfo objects");
        auto *ParentStInfo = cast<dtrans::StructInfo>(ParentInfo);
        // Update the read/write info for structure FieldInfo objects. If it is
        // a whole structure reference, don't descend into the nested elements,
        // otherwise walk the nested types to find the actual field being
        // accessed..
        if (IsLoad)
          collectReadInfo(*cast<LoadInst>(&I), ParentStInfo, ElementNum,
                          IsWholeStructure,
                          /*ForElementZeroAccess=*/false);
        else
          collectWriteInfo(*cast<StoreInst>(&I), ParentStInfo, ElementNum,
                           ValOp, IsWholeStructure,
                           /*ForElementZeroAccess=*/false);
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
      // If the pointer location represents a vtable, then we only need to
      // check that the value operand is not a structure type, which should
      // not happen. However, if it does, set a safety bit.
      else if (isVTableType(IndexedType)) {
        if (!ValInfo || ValInfo->canAliasToAggregatePointer()) {
          TypesCompatible = false;
          BadCasting = true;
        }
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
        if (ValTy && ValTy->isPointerTy()) {
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

  // Return 'true' if 'DTy' is 'i32 (...)**'
  bool isVTableType(DTransType *DTy) {
    if (!DTy->isPointerTy())
      return false;
    auto *DTy2 = DTy->getPointerElementType();
    if (!DTy2->isPointerTy())
      return false;
    auto *DFnTy = dyn_cast<DTransFunctionType>(DTy2->getPointerElementType());
    if (!DFnTy)
      return false;
    if (DFnTy->getNumArgs() != 0 || !DFnTy->isVarArg())
      return false;
    if (!DFnTy->getReturnType() || !DFnTy->getReturnType()->isIntegerTy())
      return false;

    return true;
  }

  void setFieldMismatchedElementAccess(DTransType *ParentTy,
                                       TypeSize AccessSize,
                                       DTransType *AccessTy,
                                       unsigned int ElementNum,
                                       Instruction &I) {
    auto *TI = DTInfo.getTypeInfo(ParentTy);
    assert(TI && "visitModule() should create all TypeInfo objects");
    if (getLangRuleOutOfBoundsOK()) {
      // Assuming out of bound access, set safety issue for the entire
      // ParentTy.
      setBaseTypeInfoSafetyData(ParentTy, dtrans::MismatchedElementAccess,
                                "Incompatible type for field load/store", &I);
    } else {
      // Set safety issue to AccessTy only, because out of bounds accesses are
      // known to not occur based on the LangRuleOutOfBoundsOK definition.
      TI->setSafetyData(dtrans::MismatchedElementAccess);
      if (AccessTy)
        setBaseTypeInfoSafetyData(AccessTy, dtrans::MismatchedElementAccess,
                                  "Incompatible type for field load/store", &I);
    }

    if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      TypeSize FieldSize = DL.getTypeSizeInBits(
          ParentStInfo->getField(ElementNum).getLLVMType());
      if (getLangRuleOutOfBoundsOK() || AccessSize > FieldSize) {
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

  // Process the ElementPointees of 'Info' to mark FieldAddressTaken on the
  // StructInfo and FieldInfo objects. For ElementPointees that are an
  // ArrayType, this also needs to consider the case that element zero of the
  // structure may also be the address of a field within a structure when the
  // array is a member of a structure.
  // Note, when LangRuleOutOfBoundsOK is off, all runtime dependent indices of
  // an array are considered to be element zero, rather than BadPtrManipulation.
  // The argument FieldAddressTakenType is used to identify which form of
  // FieldAddrssTaken we want set.
  void markFieldAddressTaken(ValueTypeInfo *Info, StringRef Reason, Value *V,
                             dtrans::SafetyData FieldAddressTakenType,
                             SafetyInfoReportCB Callback = nullptr) {
    auto MarkStructField = [this, &Reason, &V, &Callback,
                            FieldAddressTakenType](dtrans::TypeInfo *TI,
                                                   size_t FieldNum) {
      assert(isa<dtrans::StructInfo>(TI) && "Expected struct type info*");

      auto *ParentStInfo = cast<dtrans::StructInfo>(TI);
      setBaseTypeInfoSafetyData(TI->getDTransType(), FieldAddressTakenType,
                                Reason, V, Callback);
      ParentStInfo->getField(FieldNum).setAddressTaken();
    };

    switch (FieldAddressTakenType) {
    case dtrans::FieldAddressTakenMemory:
    case dtrans::FieldAddressTakenCall:
    case dtrans::FieldAddressTakenReturn:
      break;
    default:
      llvm_unreachable("Expected a valid FieldAddressTaken safety mask");
    }

    for (auto &PointeePair :
         Info->getElementPointeeSet(ValueTypeInfo::VAT_Use)) {
      dtrans::TypeInfo *ParentTI = DTInfo.getTypeInfo(PointeePair.first);
      assert(ParentTI && "visitModule() should create all TypeInfo objects");
      if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI)) {
        MarkStructField(ParentStInfo, PointeePair.second.getElementNum());
        continue;
      }

      // Handle array types. An array indexed by the zeroth element, requires
      // marking a structure field as "Field address taken" when the array is an
      // element of the structure.
      auto &ElementOfTypes = PointeePair.second.getElementOf();
      if ((PointeePair.second.isField() &&
           PointeePair.second.getElementNum() == 0) ||
          (!getLangRuleOutOfBoundsOK() &&
           PointeePair.second.isUnknownOffset()))
        for (auto &ElementOfPair : ElementOfTypes) {
          dtrans::TypeInfo *ElementOfTI =
              DTInfo.getTypeInfo(ElementOfPair.first);
          assert(ElementOfTI &&
                 "visitModule() should create all TypeInfo objects");
          if (auto *ElementStInfo = dyn_cast<dtrans::StructInfo>(ElementOfTI)) {
            MarkStructField(ElementStInfo, ElementOfPair.second);
            // We only mark field address taken on the closest structure that
            // contains the array, so stop walking the ElementOf list once one
            // is found.
            break;
          }
        }
    }
  }

  void collectReadInfo(LoadInst &I, dtrans::StructInfo *StInfo,
                       size_t FieldNum, bool IsWholeStructure,
                       bool ForElementZeroAccess) {
    if (!IsWholeStructure) {
      // When a whole structure is not being read, the read may be using a
      // pointer to a structure to access the element at index 0 of a contained
      // structure. Get the corresponding structure type and field being
      // read.
      dtrans::StructInfo *ReadStInfo = nullptr;
      size_t ReadFieldNum = 0;
      bool Descended = false;
      getDeepestNestedField(StInfo, FieldNum, &ReadStInfo, &ReadFieldNum,
                            &Descended);
      dtrans::FieldInfo &FI = ReadStInfo->getField(ReadFieldNum);
      FI.setRead(I);
      updateFieldFrequency(FI, I);
      DTInfo.addLoadMapping(&I, { ReadStInfo->getLLVMType(), ReadFieldNum });
      if (Descended || ForElementZeroAccess)
        FI.setNonGEPAccess();
      if (!dtrans::isLoadedValueUnused(&I,
                                       cast<LoadInst>(&I)->getPointerOperand()))
        FI.setValueUnused(false);
    } else {
      dtrans::FieldInfo &FI = StInfo->getField(FieldNum);
      FI.setRead(I);
      FI.setValueUnused(false);
      updateFieldFrequency(FI, I);
    }
  }

  void collectWriteInfo(StoreInst &I, dtrans::StructInfo *StInfo,
                        size_t FieldNum, Value *WriteVal, bool IsWholeStructure,
                        bool ForElementZeroAccess) {

    // Returns true if "WriteVal" is a SelectInst and the true and false values
    // are both constants.
    auto IsConstantResultSelectInst = [](Value *WriteVal) {
      if (auto *SI = dyn_cast<SelectInst>(WriteVal))
        return isa<Constant>(SI->getTrueValue()) &&
               isa<Constant>(SI->getFalseValue());
      return false;
    };

    auto SetFieldInfo = [this, &IsConstantResultSelectInst](
                            StoreInst &I, dtrans::StructInfo &StInfo,
                            dtrans::FieldInfo &FI, size_t FieldNum,
                            Value *WriteVal) {
      FI.setWritten(I);
      updateFieldFrequency(FI, I);
      DTInfo.addStoreMapping(&I, { StInfo.getLLVMType(), FieldNum });

      // For simple cases where the stored value is the result of select
      // instruction that always produces a constant result, save both
      // constants, instead of letting the field value info be marked as
      // 'incomplete'. For example:
      //   %70 = select i1 %69, i16 1, i16 2
      //   store i16 %70, i16* %67
      if (IsConstantResultSelectInst(WriteVal)) {
        auto *SelInst = cast<SelectInst>(WriteVal);
        updateFieldValueTracking(StInfo, FI, FieldNum,
                                 cast<Constant>(SelInst->getTrueValue()), &I);
        updateFieldValueTracking(StInfo, FI, FieldNum,
                                 cast<Constant>(SelInst->getFalseValue()), &I);
        updateFieldFrequency(FI, I);
        return;
      }

      auto *ConstVal = dyn_cast<llvm::Constant>(WriteVal);
      updateFieldValueTracking(StInfo, FI, FieldNum, ConstVal, &I);
    };

    // Set FI to bottom alloc function if 'FI' is being assigned a constant
    // value other than nullptr.
    auto CheckWriteValue = [](Constant *ConstVal, dtrans::FieldInfo &FI,
                              size_t FieldNum,
                              dtrans::StructInfo *ParentStInfo) {
      if (!isa<ConstantPointerNull>(ConstVal)) {
        DEBUG_WITH_TYPE(SAFETY_FSAF, {
          if (!FI.isBottomAllocFunction())
            dbgs() << "dtrans-fsaf: " << *(ParentStInfo->getLLVMType()) << " ["
                   << FieldNum << "] <BOTTOM>\n";
        });
        FI.setBottomAllocFunction();
      }
    };

    if (IsWholeStructure) {
      // Note: For a whole structure reference, DTrans does not fill in all
      // the field values for each field because we do not have any transforms
      // that can handle them. Instead, set all the fields as multiple value.
      for (auto &Field : enumerate(StInfo->getFields())) {
        dtrans::FieldInfo &FI = Field.value();
        FI.setWritten(I);
        updateFieldFrequency(FI, I);
        if (!FI.isMultipleValue()) {
          FI.setMultipleValue();
          DEBUG_WITH_TYPE(SAFETY_VALUES, {
            dbgs() << "dtrans-values (whole-structure-write): ";
            StInfo->getDTransType()->print(dbgs(), /*Detailed=*/false);
            dbgs() << "@" << Field.index() << ": <MULTIPLE>\n";
          });
        }
      }
      return;
    }

    // When a whole structure is not being written, the write may be using a
    // pointer to a structure to access the element at index 0 of a contained
    // structure, get the corresponding structure type and field being written.
    dtrans::StructInfo *WrittenStInfo = nullptr;
    size_t WrittenFieldNum = 0;
    bool Descended = false;
    getDeepestNestedField(StInfo, FieldNum, &WrittenStInfo, &WrittenFieldNum,
                          &Descended);
    dtrans::FieldInfo &FI = WrittenStInfo->getField(WrittenFieldNum);
    SetFieldInfo(I, *WrittenStInfo, FI, WrittenFieldNum, WriteVal);
    if (Descended || ForElementZeroAccess)
      FI.setNonGEPAccess();

    // Update field single allocation function info
    if (auto *ConstVal = dyn_cast<llvm::Constant>(WriteVal)) {
      CheckWriteValue(ConstVal, FI, FieldNum, WrittenStInfo);
    } else if (auto *Call = dyn_cast<CallBase>(WriteVal)) {
      if (PTA.getAllocationCallKind(Call) != dtrans::AK_NotAlloc) {
        Function *Callee = Call->getCalledFunction();
        if (FI.processNewSingleAllocFunction(Callee)) {
          DEBUG_WITH_TYPE(SAFETY_FSAF, {
            dbgs() << "dtrans-fsaf: " << *(WrittenStInfo->getLLVMType())
                   << " [" << FieldNum << "] ";
            if (FI.isSingleAllocFunction())
              Callee->printAsOperand(dbgs());
            else
              dbgs() << "<BOTTOM>";
            dbgs() << "\n";
          });
        }
      } else {
        DEBUG_WITH_TYPE(SAFETY_FSAF, {
          if (!FI.isBottomAllocFunction())
            dbgs() << "dtrans-fsaf: " << *(WrittenStInfo->getLLVMType())
                   << " [" << FieldNum << "] <BOTTOM>\n";
        });
        FI.setBottomAllocFunction();
      }
    } else {
      DEBUG_WITH_TYPE(SAFETY_FSAF, {
        if (!FI.isBottomAllocFunction())
          dbgs() << "dtrans-fsaf: " << *(WrittenStInfo->getLLVMType())
                 << " [" << FieldNum << "] <BOTTOM>\n";
      });
      FI.setBottomAllocFunction();
    }
  }

  // If C is a Constant, add it to the list of values tracked for a field.
  // Otherwise, mark the field info as being an incomplete multiple value set.
  // 'StInfo', 'FieldNum', and 'Consumer' are just used for providing
  // information in the traces regarding which structure, field number and
  // memory write were responsible for field update.
  void updateFieldValueTracking(dtrans::StructInfo &StInfo,
                                dtrans::FieldInfo &FI, size_t FieldNum,
                                Constant *C, Value *Consumer) {
    if (!C) {
      if (!FI.isMultipleValue()) {
        FI.setMultipleValue();
        DEBUG_WITH_TYPE(SAFETY_VALUES, {
          dbgs() << "dtrans-values: ";
          StInfo.getDTransType()->print(dbgs(), /*Detailed=*/false);
          dbgs() << "@" << FieldNum << ": <MULTIPLE>\n  ";
          printValue(dbgs(), Consumer);
          dbgs() << "\n";
        });
      }
      return;
    }

    if (FI.processNewSingleValue(C))
      DEBUG_WITH_TYPE(SAFETY_VALUES, {
        dbgs() << "dtrans-values: ";
        StInfo.getDTransType()->print(dbgs(), /*Detailed=*/false);
        dbgs() << "@" << FieldNum << ": New value: ";
        printValue(dbgs(), C);
        dbgs() << "\n  ";
        printValue(dbgs(), Consumer);
        dbgs() << "\n";
      });
  }

  void updateFieldFrequency(dtrans::FieldInfo &FI, Instruction &I) {
    assert(BFI && "BFI should be set when visitFunction begins");
    uint64_t InstFreq = DTransUseBlockFreq
                            ? BFI->getBlockFreq(I.getParent()).getFrequency()
                            : 1;
    uint64_t TFreq = InstFreq + FI.getFrequency();
    // Set it to 64-bit unsigned Max value if there is overflow.
    TFreq = TFreq < InstFreq ? std::numeric_limits<uint64_t>::max() : TFreq;
    FI.setFrequency(TFreq);
  }

  // When the pointer type analyzer detects an inner element of a nested type
  // being loaded via a pointer to the container type, it records the container
  // type as the element pointee. In these cases the type nest needs to be
  // traversed to mark the actual field as being 'read' or 'written'. This
  // function walks the structure to return the actual structure and field
  // number accessed.
  // 'ActualStInfo', 'ActualFieldNum' and 'Descended' are outputs of this
  // function. 'ActualStInfo' and 'FieldNum' will record the structure and
  // field number actually referenced. 'Descended' will be set to 'true' if
  // the actual element referenced is a nested member of 'StInfo'
  void getDeepestNestedField(dtrans::StructInfo *StInfo, size_t FieldNum,
                             dtrans::StructInfo **ActualStInfo,
                             size_t *ActualFieldNum, bool *Descended) {
    *Descended = false;
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
        dtrans::TypeInfo *TI = DTInfo.getTypeInfo(StElemTy);
        assert(TI && "visitModule() should create all TypeInfo objects");
        StInfo = cast<dtrans::StructInfo>(TI);
        ElemTy = StInfo->getField(FieldNum).getDTransType();
        *Descended = true;
      } else if (auto *ArElemTy = dyn_cast<DTransArrayType>(ElemTy)) {
        // Handle the case of the element being an array of nested structures by
        // finding the structure type. If it's not an array of structures, then
        // the access is to the first element of the array, and there is no need
        // to continue deeper into the nesting.
        // Examples:
        //   [4 x %struct.A] - iterate over element that starts %struct.A
        //   [4 x i32] - cannot continue iterating
        DTransType *MemberTy = getArrayUnitType(ArElemTy);
        *Descended = true;
        if (MemberTy->isStructTy())
          ElemTy = MemberTy;
        else
          break;
      }
    }

    *ActualStInfo = StInfo;
    *ActualFieldNum = FieldNum;
  }

  void visitBitCastInst(BitCastInst &I) {
    // Bitcast instructions can be ignored because all the safety checks are
    // based on how the result of the bitcast gets used.
  }

  void visitPtrToIntInst(PtrToIntInst &I) {
    ValueTypeInfo *Info = PTA.getValueTypeInfo(&I);
    assert(Info &&
           "PtrTypeAnalyzer failed to construct ValueTypeInfo for ptrtoint");

    if (isValueTypeInfoUnhandled(*Info))
      DTInfo.setUnhandledPtrType(&I);

    if (I.getType() != getLLVMPtrSizedIntType()) {
      setAllAliasedTypeSafetyData(Info, dtrans::BadCasting,
                                  "ptrtoint cast to non-ptr-sized integer type",
                                  &I);
    }

    // If there is an aggregate type, make sure it is unambiguous.
    if (Info->canAliasToAggregatePointer()) {
      DTransType *DomTy = PTA.getDominantAggregateUsageType(*Info);
      if (!DomTy)
        setAllAliasedTypeSafetyData(Info, dtrans::BadCasting,
                                    "ptrtoint with unknown type", &I);
    }
  }

  void visitBinaryOperator(BinaryOperator &I) {
    auto SetBinaryOperatorUnhandledUse = [this](BinaryOperator &I) {
      for (Value *Arg : I.operands()) {
        ValueTypeInfo *Info = PTA.getValueTypeInfo(Arg);
        if (Info && !Info->empty())
          setAllAliasedAndPointeeTypeSafetyData(
              Info, dtrans::UnhandledUse, "Unhandled binary operator", &I);
      }
    };

    auto HasNonDivBySizeUses = [](Value *V, uint64_t Size) {
      for (auto *U : V->users()) {
        if (auto *BinOp = dyn_cast<BinaryOperator>(U)) {
          if (BinOp->getOpcode() != Instruction::SDiv &&
              BinOp->getOpcode() != Instruction::UDiv)
            return true;
          if (BinOp->getOperand(0) != V)
            return true;
          if (!dtrans::isValueMultipleOfSize(BinOp->getOperand(1), Size))
            return true;
          continue;
        }

        return true;
      }

      return false;
    };

    auto AnalyzeSubtraction = [this,
                               &HasNonDivBySizeUses](BinaryOperator &Sub) {
      assert(Sub.getOpcode() == Instruction::Sub &&
             "AnalyzeSubtraction() called with unexpected opcode");

      Value *LHS = Sub.getOperand(0);
      Value *RHS = Sub.getOperand(1);
      ValueTypeInfo *LHSInfo = PTA.getValueTypeInfo(LHS);
      ValueTypeInfo *RHSInfo = PTA.getValueTypeInfo(RHS);
      // The PointerTypeAnalyzer may create a ValueTypeInfo for scalar types
      // that never get used, so we need to check if they exist and are
      // non-empty to determine whether the instruction needs to be analyzed.
      if ((!LHSInfo || LHSInfo->empty()) && (!RHSInfo || RHSInfo->empty()))
        return;

      if (!LHSInfo || !RHSInfo) {
        if (LHSInfo)
          setAllAliasedAndPointeeTypeSafetyData(
              LHSInfo, dtrans::BadPtrManipulation,
              "Subtraction between pointer and non-pointer", &Sub);
        if (RHSInfo)
          setAllAliasedAndPointeeTypeSafetyData(
              RHSInfo, dtrans::BadPtrManipulation,
              "Subtraction between pointer and non-pointer", &Sub);
        return;
      }

      if (LHSInfo->pointsToSomeElement() || RHSInfo->pointsToSomeElement()) {
        setAllElementPointeeSafetyData(
            LHSInfo, dtrans::BadPtrManipulation,
            "Subtraction using aggregate field address", &Sub);
        setAllElementPointeeSafetyData(
            RHSInfo, dtrans::BadPtrManipulation,
            "Subtraction using aggregate field address", &Sub);
        return;
      }

      // Both operands need to be pointers to aggregate types, or neither is
      // allowed to be a pointer to an aggregate type.
      bool LHSIsAggregate = LHSInfo->canAliasToAggregatePointer();
      bool RHSIsAggregate = RHSInfo->canAliasToAggregatePointer();
      if (LHSIsAggregate != RHSIsAggregate) {
        setAllAliasedTypeSafetyData(
            LHSInfo, dtrans::BadPtrManipulation,
            "Subtraction between pointer and non-pointer", &Sub);
        setAllAliasedTypeSafetyData(
            RHSInfo, dtrans::BadPtrManipulation,
            "Subtraction between pointer and non-pointer", &Sub);
        return;
      }

      // Neither operand is a pointer to an aggregate type.
      if (!LHSIsAggregate)
        return;

      // Check that the same aggregate pointer type is being used for the same
      // operand types.
      DTransType *LHSDomTy = PTA.getDominantAggregateUsageType(*LHSInfo);
      DTransType *RHSDomTy = PTA.getDominantAggregateUsageType(*RHSInfo);
      if (!LHSDomTy || LHSDomTy != RHSDomTy) {
        setAllAliasedTypeSafetyData(LHSInfo, dtrans::BadPtrManipulation,
                                    "Subtraction between unrelated types",
                                    &Sub);
        setAllAliasedTypeSafetyData(RHSInfo, dtrans::BadPtrManipulation,
                                    "Subtraction between unrelated types",
                                    &Sub);
        return;
      }

      // Next, we need to verify that the pointer arithmetic is only used to
      // feed a computation that computes the number of elements between the two
      // pointers by performing a division using the element's size. For
      // ptr-to-ptr, this is not necessary because it would not be dividing by
      // the element's size.
      if (isPtrToPtr(LHSDomTy))
        return;

      // We expect the dominant type to be a pointer type.
      if (!LHSDomTy->isPointerTy()) {
        setAllAliasedTypeSafetyData(LHSInfo, dtrans::UnhandledUse,
                                    "Unexpected aggregate type for subtraction",
                                    &Sub);
        setAllAliasedTypeSafetyData(LHSInfo, dtrans::UnhandledUse,
                                    "Unexpected aggregate type for subtraction",
                                    &Sub);
        return;
      }

      // Check that the uses are restricted to computing an element count. If
      // so, we treat it is as safe. We assume the arithmetic is being
      // performed for two elements from the same allocation chunk (or static
      // array) because that is the only case that logically makes sense to
      // compute the distance between elements.
      DTransType *ElementTy = LHSDomTy->getPointerElementType();
      assert(ElementTy->isAggregateType() && "Expected aggregate type");
      llvm::Type *LLVMElementTy = ElementTy->getLLVMType();
      uint64_t ElementSize = DL.getTypeAllocSize(LLVMElementTy);
      if (HasNonDivBySizeUses(&Sub, ElementSize)) {
        setAllAliasedTypeSafetyData(
            LHSInfo, dtrans::BadPtrManipulation,
            "Pointer subtract result has non-divide by size use", &Sub);
        setAllAliasedTypeSafetyData(
            LHSInfo, dtrans::BadPtrManipulation,
            "Pointer subtract result has non-divide by size use", &Sub);
        return;
      }

      // The subtraction is safe. Save the type information for queries by the
      // transformations.
      DTInfo.addPtrSubMapping(&Sub, ElementTy);
    };

    // Check that one operand is an instruction and the other is an integer
    // constant.
    auto HasInstAndConstOperand = [](Instruction *I) {
      assert((isa<BinaryOperator>(I) || isa<ICmpInst>(I)) &&
             "Expected BinaryOperator");
      Value *Op0 = I->getOperand(0);
      Value *Op1 = I->getOperand(1);
      return ((isa<Instruction>(Op0) && isa<ConstantInt>(Op1)) ||
              (isa<Instruction>(Op1) && isa<ConstantInt>(Op0)));
    };

    // This function is checking that the only uses of the binary operation are
    // one of the following:
    // 1) another bitmask binary operator, which applies a constant
    //    value for the mask
    // 2) a comparison to a constant.
    auto IsBitmaskAndCompareSequenceOnly =
        [&HasInstAndConstOperand](BinaryOperator *I) {
          SmallVector<Instruction *, 4> WorkList;
          WorkList.push_back(I);

          while (!WorkList.empty()) {
            Instruction *NextI = WorkList.back();
            WorkList.pop_back();
            switch (NextI->getOpcode()) {
            default:
              return false;
            case Instruction::Or:
            case Instruction::And:
            case Instruction::Xor:
              if (!HasInstAndConstOperand(NextI))
                return false;
              for (auto *U : NextI->users()) {
                // We don't need to check the users of the ICmp.
                if (auto *Cmp = dyn_cast<ICmpInst>(U)) {
                  if (!HasInstAndConstOperand(Cmp))
                    return false;
                  continue;
                }
                // We do need to check the users of binary operators.
                if (isa<BinaryOperator>(U)) {
                  WorkList.push_back(cast<Instruction>(U));
                  continue;
                }
                // Anything else breaks the pattern.
                return false;
              }
            }
          }

          return true;
        };

    // Check whether the bitmask instruction is supported. If not, mark the
    // operands as UnhandledUse
    //
    // BitMask computations that appear to be used for checking the alignment of
    // a pointer value that is being tracked are allowed. Other cases will be
    // marked as Unhandled.
    //
    // Allow a case such as:
    //   %5 = ptrtoint p0 %0 to i64
    //   %6 = or i64 %5, 8
    //   %7 = and i64 %6, 7
    //   %8 = icmp eq i64 %7, 0
    auto AnalyzeBitMask =
        [this, &HasInstAndConstOperand, &SetBinaryOperatorUnhandledUse,
         &IsBitmaskAndCompareSequenceOnly](BinaryOperator &BinOp) {
          Value *LHS = BinOp.getOperand(0);
          Value *RHS = BinOp.getOperand(1);
          ValueTypeInfo *LHSInfo = PTA.getValueTypeInfo(LHS);
          ValueTypeInfo *RHSInfo = PTA.getValueTypeInfo(RHS);

          // The PointerTypeAnalyzer may create a ValueTypeInfo object for
          // some values that do not represent pointer types, so we use
          // this check to determine whether the instruction has some
          // pointer type information associated with it.
          if ((!LHSInfo || LHSInfo->empty()) && (!RHSInfo || RHSInfo->empty()))
            return;

          // Only support a binary operation with a constant integer
          if (!HasInstAndConstOperand(&BinOp)) {
            SetBinaryOperatorUnhandledUse(BinOp);
            return;
          }

          // Do not allow alignment checks on the address of field members
          ValueTypeInfo *InfoOfInterest =
              (LHSInfo && !LHSInfo->empty()) ? LHSInfo : RHSInfo;
          assert(InfoOfInterest && "Expected operand of interest");
          if (InfoOfInterest->pointsToSomeElement()) {
            SetBinaryOperatorUnhandledUse(BinOp);
            return;
          }

          if (IsBitmaskAndCompareSequenceOnly(&BinOp))
            return;

          SetBinaryOperatorUnhandledUse(BinOp);
        };

    switch (I.getOpcode()) {
    case Instruction::Sub:
      AnalyzeSubtraction(I);
      break;
    case Instruction::Or:
    case Instruction::And:
    case Instruction::Xor:
      AnalyzeBitMask(I);
      break;
    default:
      SetBinaryOperatorUnhandledUse(I);
      break;
    }
  }

  // See typesMayBeCRuleCompatible() immediately below for explanation of
  // this function.
  static bool typesMayBeCRuleCompatibleX(DTransType *T1, DTransType *T2,
                                         SmallPtrSetImpl<DTransType *> &Tstack,
                                         bool IgnorePointees) {

    // Enum indicating that on the particular predicate being compared for
    // T1 and T2, the types have the opposite value of the predicate
    // (TME_OPPOSITE), the types both have the predicate (TME_YES), or
    // neither type has the predicate (TME_YES).
    enum TME { TME_OPPOSITE, TME_YES, TME_NO };

    // Lambda to match the result of testing the same predicate for B1 and B2
    auto boolT = [](bool B1, bool B2) -> TME {
      return (B1 && B2) ? TME_YES : (!B1 && !B2) ? TME_NO : TME_OPPOSITE;
    };

    // Returns true if T is a struct type with no elements.
    auto isEmptyStructType = [](DTransType *T) -> bool {
      auto ST = dyn_cast<DTransStructType>(T);
      return ST && ST->getNumFields() == 0;
    };

    // Typedef for const pointer to member function which returns a bool
    typedef bool (DTransType::*MFP)() const;

    // Lambda to compare an predicate indicated by Fp for T1 and T2
    auto typeTest = [&boolT](DTransType *T1, DTransType *T2, MFP Fp) -> TME {
      return boolT((T1->*Fp)(), (T2->*Fp)());
    };

    // An array of predicate conditions for which T1 and T2 will be tested
    // for compatibility. The predicate "isIntegerTy" and "isFloatingPointTy"
    // are base properties that cannot be further refined.
    MFP F1Array[] = {&DTransType::isIntegerTy, &DTransType::isFloatingPointTy};

    // A Type is always compatible with itself.
    if (T1 == T2)
      return true;

    // CMPLRS-15252: We have seen that an empty struct type can be generated by
    // the clang front end to represent a Function type. Since this will not be
    // fixed in the clang front end, assume that an empty struct type is
    // compatible with a Function type.
    if ((T1->isFunctionTy() && isEmptyStructType(T2)) ||
        (T2->isFunctionTy() && isEmptyStructType(T1)))
      return true;

    // Test some fundamental and complicated predicates. Return false if they
    // don't match, true if they do.
    for (auto Fx : F1Array) {
      TME R = typeTest(T1, T2, Fx);
      if (R == TME_OPPOSITE)
        return false;
      if (R == TME_YES)
        // FIXME: check DataLayout::getTypeStoreSize
        return true;
    }

    // Two pointer Types are compatible if their element types are compatible.
    TME R = typeTest(T1, T2, &DTransType::isPointerTy);
    if (R == TME_OPPOSITE)
      return false;
    if (R == TME_YES) {
      //
      // In the non-opaque pointer case, there was a check that the address
      // spaces of T1 and T2 matched. But a simple check like:
      //   unsigned P1 = T1->getLLVMType()->getPointerAddressSpace();
      //   unsigned P2 = T2->getLLVMType()->getPointerAddressSpace();
      //   if (P1 != P2)
      //     return false;
      // will not work here, because conversion of a DTransPointerType to
      // a LLVMType will always result in having the default address space.
      // To support address spaces will require additional metdata. But
      // DTrans only runs on the default address space now, so a check is not
      // really necessary.
      //
      if (IgnorePointees)
        return true;

      //
      // Pointers to StructTypes are a tricky case, as the definition could
      // be recursive.  This could be resolved by struct TAGs, but these are
      // not strictly preserved in LLVM. We could try to derive them from
      // the StructType's name, but in LLVM even anonymous types get a name.
      // (albeit a recognizable one because it is either %struct.anon or
      // of the form %struct.anon.*).
      //
      // So we keep a stack of pointers to Types that we have already seen,
      // and give up whenever we encounter one again. This is conservative
      // and could be improved.
      //
      auto *T3 = T1->getPointerElementType();
      auto *T4 = T2->getPointerElementType();

      // CMPLRS-15252: Perform the same empty struct/Function type test
      // above on the pointer element types.
      if ((T3->isFunctionTy() && isEmptyStructType(T4)) ||
          (T4->isFunctionTy() && isEmptyStructType(T3)))
        return true;

      // Should contain all checks resulting in recursive calls.
      MFP CompoundChecks[] = {&DTransType::isArrayTy, &DTransType::isStructTy,
                              &DTransType::isFunctionTy};

      for (auto Cx : CompoundChecks) {
        TME S = typeTest(T3, T4, Cx);
        if (S == TME_OPPOSITE)
          return false;
        if (S == TME_YES) {
          if (Tstack.find(T3) != Tstack.end())
            return true;
          if (Tstack.find(T4) != Tstack.end())
            return true;
          Tstack.insert(T3);
          Tstack.insert(T4);
        }
      }
      return typesMayBeCRuleCompatibleX(T1->getPointerElementType(),
                                        T2->getPointerElementType(), Tstack,
                                        IgnorePointees);
    }

    R = typeTest(T1, T2, &DTransType::isFunctionTy);
    if (R == TME_OPPOSITE)
      return false;
    if (R == TME_YES) {
      auto *FT1 = cast<DTransFunctionType>(T1);
      auto *FT2 = cast<DTransFunctionType>(T2);
      if (!typesMayBeCRuleCompatibleX(FT1->getReturnType(),
                                      FT2->getReturnType(), Tstack,
                                      IgnorePointees))
        return false;

      // FIXME: extend to handle left over parameters and var-arg.
      for (auto Pair : zip(FT1->args(), FT2->args())) {
        auto *PT1 = std::get<0>(Pair);
        auto *PT2 = std::get<1>(Pair);
        if (!typesMayBeCRuleCompatibleX(PT1, PT2, Tstack, IgnorePointees))
          return false;
      }
    }

    // Two array Types are compatible if they have the same number of elements
    // and their element types are compatible.
    R = typeTest(T1, T2, &DTransType::isArrayTy);
    if (R == TME_OPPOSITE)
      return false;
    if (R == TME_YES) {
      auto *AT1 = cast<DTransArrayType>(T1);
      auto *AT2 = cast<DTransArrayType>(T2);
      if (AT1->getNumElements() != AT2->getNumElements())
        return false;
      return typesMayBeCRuleCompatibleX(AT1->getElementType(),
                                        AT2->getElementType(),
                                        Tstack, IgnorePointees);
    }

    // Two struct Types are compatible if they have the same number of
    // elements, and corresponding elements are compatible with one another.
    R = typeTest(T1, T2, &DTransType::isStructTy);
    if (R == TME_OPPOSITE)
      return false;
    if (R == TME_YES) {
      auto *ST1 = cast<DTransStructType>(T1);
      auto *ST2 = cast<DTransStructType>(T2);
      if (ST1->getNumFields() != ST2->getNumFields())
        return false;
      for (unsigned I = 0; I < ST1->getNumFields(); ++I)
        if (!typesMayBeCRuleCompatibleX(ST1->getFieldType(I),
                                        ST2->getFieldType(I), Tstack,
                                        IgnorePointees))
          return false;
      return true;
    }

    // None of the rules cited above were able to determine the Types are
    // not compatible with one another. Conservatively assume they are.
    return true;
  }

  // Return true if Types T1 and T2 may be compatible by C language rules.
  // The full C language rules for Type compatibility are not implemented
  // here.  This is a conservative test.  It will return true in some cases
  // where the T1 and T2 are not compatible. Here are some examples:
  //   (1) When the names of structure fields do not match. Since the LLVM
  //       IR does not include info about the structure field names, this
  //       function must return a conservative result here.
  //   (2) When the structure tags do not match. (See the comment in the
  //       code for typesMayBeCRuleCompatibleX.) In the absence of completely
  //       reliable structure tags, we keep a stack (Tstack) of Types that
  //       we have already seen to avoid recursion.
  //   (3) Vector types and function types with different numbers of
  //       parameters and var arg. This could be implemented in the future.
  //       We are not doing it now because there is no immediate need.
  //
  // IgnorePointees only checks that pointers are layout compatible,
  // i.e. they are from the same address space.
  static bool typesMayBeCRuleCompatible(DTransType *T1, DTransType *T2,
                                        bool IgnorePointees = false) {
    SmallPtrSet<DTransType *, 4> Tstack;
    return typesMayBeCRuleCompatibleX(T1, T2, Tstack, IgnorePointees);
  }

  // Return true if the Type T may have a distinct compatible Type by
  // C language rules. Before this function is executed, we must ensure
  // that all Types against which we test it have TypeInfos created for
  // them.  This is done in analyzeModule().
  bool mayHaveDistinctCompatibleCType(DTransType *T) {
    dtrans::TypeInfo *TIN = DTInfo.getTypeInfo(T);
    assert(TIN && "visitModule() should create all TypeInfo objects");
    switch (TIN->getCRuleTypeKind()) {
    case dtrans::CRT_False:
      return false;
    case dtrans::CRT_True:
      return true;
    case dtrans::CRT_Unknown:
      for (auto *TI : DTInfo.type_info_entries()) {
        DTransType *U = TI->getDTransType();
        if (U != T && typesMayBeCRuleCompatible(T, U)) {
          LLVM_DEBUG(dbgs()
                     << "dtrans-crule-compat: " << *T << " (X) " << *U << "\n");
          TIN->setCRuleTypeKind(dtrans::CRT_True);
          return true;
        }
      }
      TIN->setCRuleTypeKind(dtrans::CRT_False);
      LLVM_DEBUG(dbgs() << "dtrans-crule-nocompat: " << *T << "\n");
      return false;
    }
    return true;
  }

  void visitCallBase(CallBase &Call) {

    // Return true if \p Call represents an indirect call, and there is a
    // an address taken external function that matches it.
    auto HasIndirectCallMatch = [this](CallBase *Call) -> bool {
      // No point in doing this if the C language compatibility rule is not
      // enforced.
      if (!DTInfo.getDTransUseCRuleCompat())
        return true;
      // Check if this is an indirect call site.
      if (isa<Function>(Call->getCalledOperand()))
        return false;
      DTransType *ActualType = MDReader.getDTransTypeFromMD(Call);
      auto ActualFType = dyn_cast_or_null<DTransFunctionType>(ActualType);
      if (!ActualFType ||
          Call->getFunctionType()->getNumParams() != ActualFType->getNumArgs())
        return true;
      // Look for a matching address taken external call.
      for (auto &F : Call->getModule()->functions()) {
        if (isExternalAddressTakenFunction(&F)) {
          // The standard test for an indirect call match.
          if ((F.arg_size() == ActualFType->getNumArgs()) ||
              (F.isVarArg() && F.arg_size() <= ActualFType->getNumArgs())) {
            bool IsFunctionMatch = true;
            DTransType *FormalType = MDReader.getDTransTypeFromMD(&F);
            auto FormalFType = dyn_cast_or_null<DTransFunctionType>(FormalType);
            if (!FormalFType)
              return true;
            unsigned I = 0;
            for (; I < F.arg_size(); ++I) {
              DTransType *AArgType = ActualFType->getArgType(I);
              DTransType *FArgType = FormalFType->getArgType(I);
              if (!typesMayBeCRuleCompatible(AArgType, FArgType)) {
                IsFunctionMatch = false;
                break;
              }
            }
            if (IsFunctionMatch) {
              LLVM_DEBUG({
                dbgs() << "dtrans-ic-match: ";
                F.printAsOperand(dbgs());
                dbgs() << ":: " << I << "\n";
              });
              return true;
            }
          }
        }
      }
      return false;
    };

    // If the called function is a known allocation function, we need to check
    // that it only gets used as at most 1 aggregate type, and the size is a
    // multiple of the aggregate. Also construct the CallInfo object for the
    // transformations.
    SmallPtrSet<const Value *, 3> SpecialArguments;
    const TargetLibraryInfo &TLI = GetTLI(*Call.getFunction());
    dtrans::AllocKind AKind = PTA.getAllocationCallKind(&Call);
    if (AKind != dtrans::AK_NotAlloc) {
      analyzeAllocationCall(&Call, AKind);

      // Get the list of arguments that don't need to be checked for type
      // compatibility.
      dtrans::collectSpecialAllocArgs(AKind, &Call, SpecialArguments, TLI);
    }

    // If this is a call to the "free" lib function,  the call is safe, but
    // we analyze the instruction for the purpose of capturing the argument
    // TypeInfo, which will be needed by some of the transformations when
    // rewriting allocations and frees.
    dtrans::FreeKind FKind = PTA.getFreeCallKind(&Call);
    if (FKind != dtrans::FK_NotFree) {
      analyzeFreeCall(&Call, FKind);

      // Get the list of arguments that don't need to be checked for type
      // compatibility.
      dtrans::collectSpecialFreeArgs(FKind, &Call, SpecialArguments, TLI);
    }

    // If the call returns a type of interest, then we need to analyze the
    // return value for safety bits that need to be set
    Function *F = dtrans::getCalledFunction(Call);
    bool IsFnLocal =
        F ? (!F->isDeclaration() && !F->hasDLLExportStorageClass()) : false;
    bool IsIndirect = Call.isIndirectCall();
    LibFunc TheLibFunc = NotLibFunc;
    bool IsLibFunc = false;
    if (F && F->hasName())
      IsLibFunc =
          TLI.getLibFunc(F->getName(), TheLibFunc) && TLI.has(TheLibFunc);
    bool IsInvoke = isa<InvokeInst>(&Call);
    if (!Call.getType()->isVoidTy()) {
      ValueTypeInfo *Info = PTA.getValueTypeInfo(&Call);
      if (Info && Info->canAliasToAggregatePointer()) {
        if (IsInvoke)
          setAllAliasedTypeSafetyData(Info, dtrans::HasCppHandling,
                                      "Type returned by invoke call", &Call);

        // If the value was declared as returning an 'i8*', then check that it
        // does not get used as an aggregate type. This is only done for i8*
        // return values because that is a generic type that is generally
        // allowed as a safe alias type when analyzing instructions that use the
        // value. Other type mismatches should be detected when the value gets
        // used.
        if (PTA.getDominantType(*Info, ValueTypeInfo::VAT_Decl) ==
                getDTransI8PtrType() &&
            AKind == dtrans::AK_NotAlloc)
          setAllAliasedTypeSafetyData(
              Info, dtrans::BadCasting,
              "i8* type returned by call used as aggregate pointer type",
              &Call);

        // Types returned by non-local functions should be treated as
        // system objects since we cannot transform them.
        if ((!IsFnLocal || IsLibFunc) && AKind == dtrans::AK_NotAlloc)
          setAllAliasedTypeSafetyData(Info, dtrans::SystemObject,
                                      "Type returned by non-local function",
                                      &Call);
      }
    }

    // Do not check and potentially set the mismatched arg safety condition on
    // the 'this' pointer argument of a call created by devirtualization. The
    // devirtualizer has already proven the argument type is the correct type
    // for the member function.
    if (Call.getMetadata("_Intel.Devirt.Call") && Call.arg_size() >= 1)
      SpecialArguments.insert(Call.getArgOperand(0));

    if (SpecialArguments.size() == Call.arg_size())
      return;

    DTransType *TargetType = nullptr;
    if (F) {
      if (TM.isSimpleType(F->getValueType()))
        TargetType = TM.getOrCreateSimpleType(F->getValueType());
      else
        TargetType = MDReader.getDTransTypeFromMD(F);
    } else {
      // An indirect call should have metadata info to define the expected type
      // of the call.
      TargetType = MDReader.getDTransTypeFromMD(&Call);
    }
    DTransFunctionType *FuncExpectedDTransTy =
        dyn_cast_or_null<DTransFunctionType>(TargetType);

    // If this is an indirect call site, find out if there is a matching
    // address taken external call.  In this case, the indirect call must
    // be treated like an external call for the purpose of generating
    // the AddressTaken safety check.
    bool HasICMatch = HasIndirectCallMatch(&Call);
    size_t CallArgCnt = Call.arg_size();
    for (unsigned ArgNum = 0; ArgNum < CallArgCnt; ++ArgNum) {
      Value *Param = Call.getArgOperand(ArgNum);
      if (SpecialArguments.count(Param))
        continue;

      // Callback method to report specific parameter that is triggering the
      // safety flag. This callback will be invoked when a safety flag is being
      // set, and debug trace filtering is enabled for the function being
      // analyzed.
      auto DumpCallback = [Param, ArgNum]() {
        (void)Param;
        (void)ArgNum;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
        dbgs() << "  Arg#" << ArgNum << ": " << *Param << "\n";
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      };

      // The pointer type analyzer should have collected value type info for all
      // parameters that are types of interest. If there is no info, then it
      // must not be a parameter of interest.
      ValueTypeInfo *ParamInfo = PTA.getValueTypeInfo(Param);
      if (!ParamInfo)
        continue;

      // Perform checks that are the same regardless of whether the function is
      // locally defined or external.
      if (isValueTypeInfoUnhandled(*ParamInfo)) {
        DTInfo.setUnhandledPtrType(&Call);
        setAllAliasedTypeSafetyData(ParamInfo, dtrans::UnhandledUse,
                                    "PointerTypeAnalyzer could not resolve all "
                                    "potential types for pointer",
                                    &Call, DumpCallback);
      }

      if (ParamInfo->pointsToSomeElement())
        markFieldAddressTaken(ParamInfo, "Address of member passed to function",
                              &Call, dtrans::FieldAddressTakenCall,
                              DumpCallback);

      // We need to know the signature of the called function in order the check
      // for functions that take i8* parameter types.
      if (!FuncExpectedDTransTy) {
        setAllAliasedTypeSafetyData(
            ParamInfo, dtrans::UnhandledUse,
            "Unable to determine expected type for function", &Call,
            DumpCallback);
        continue;
      }

      // Try to get the expected argument type to determine whether the function
      // takes that pointer as an i8*.
      DTransType *ArgExpectedDTransTy = nullptr;
      bool IsVarArgParam = ArgNum >= FuncExpectedDTransTy->getNumArgs();
      if (!IsVarArgParam) {
        ArgExpectedDTransTy = FuncExpectedDTransTy->getArgType(ArgNum);
        if (!ArgExpectedDTransTy) {
          setAllAliasedTypeSafetyData(
              ParamInfo, dtrans::UnhandledUse,
              "Unable to determine expected type for function argument", &Call,
              DumpCallback);
          continue;
        }
      }

      if (!ParamInfo->canAliasToAggregatePointer()) {
        if (!IsVarArgParam && ArgExpectedDTransTy == getDTransI8PtrType()) {
          // TODO: The caller did not see any aliased types, but the i8* could
          // be passed to multiple functions and used as different types for
          // each them. Add checks to detect an unsafe use, such as if the
          // following, if the functions used %p as different structure types.
          //   call void @use_test01a(i8* %p) ; uses %p as %struct.foo
          //   call void @use_test01b(i8* %p) ; uses %p as %struct.bar
          // For now we will not set any safety data here because the original
          // LocalPointerAnalyzer did not mark these cases as unsafe.
        }
        continue;
      }

      // If we reach here, we know that some aggregate pointer type is
      // involved in the parameter that needs to be checked for safety.

      // Mark HasCppHandling on any aggregate pointer passed via InvokeInst.
      if (IsInvoke)
        setAllAliasedTypeSafetyData(ParamInfo, dtrans::HasCppHandling,
                                    "Type passed to invoke call", &Call,
                                    DumpCallback);

      if (IsFnLocal && !IsLibFunc) {
        if (IsVarArgParam) {
          // TODO: Add checking of whether the argument is used in the called
          // function with a type that is compatible with the passed value to
          // make this less conservative.
          setAllAliasedTypeSafetyData(
              ParamInfo, dtrans::UnhandledUse,
              "Type passed in vararg parameter to function", &Call,
              DumpCallback);
          continue;
        }

        // Any argument in the VarArgs should have been processed above.
        // Check whether the callee uses the argument, if it doesn't there is no
        // need for the remaining safety flag checks.
        assert(ArgNum < F->arg_size() && "Unexpected operand");
        Argument *TargetArg = F->getArg(ArgNum);
        if (TargetArg->user_empty()) {
          DEBUG_WITH_TYPE_P(FNFilter, SAFETY_VERBOSE,
                            dbgs()
                                << "dtrans-safety: Ignoring unused argument #"
                                << ArgNum << " in call: " << Call << "\n");
          continue;
        }

        // If the argument is a pointer-sized int, then mark the element as
        // AddressTaken because the analysis of the callee would have treated
        // the argument as a scalar.
        llvm::Type *ArgType = F->getArg(ArgNum)->getType();
        if (ArgType == getLLVMPtrSizedIntType()) {
          setAllAliasedTypeSafetyData(
              ParamInfo, dtrans::AddressTaken,
              "Type passed to pointer-sized int argument of local function",
              &Call, DumpCallback);
          continue;
        }

        // Check whether the parameter type matches the expected argument type.
        // The PtrTypeAnalyzer will have populated the ArgInfo with the type
        // declared in the signature of the called function, so if we can
        // resolve a unique usage type for the parameter, then the parameter
        // matches the expected type.
        DTransType *DomTy = PTA.getDominantAggregateUsageType(*ParamInfo);
        if (ArgExpectedDTransTy == getDTransI8PtrType()) {
          Value *TargetArg = F->getArg(ArgNum);
          ValueTypeInfo *ArgInfo = PTA.getValueTypeInfo(TargetArg);
          assert(ArgInfo &&
                 "Expected pointer type analyzer to analyze pointer");

          DTransType *TargDomTy =
              PTA.getDominantType(*ArgInfo, ValueTypeInfo::VAT_Use);
          if (DomTy && TargDomTy == DomTy)
            continue;

          // The dominant type the pointer is used as in the callee does not
          // match the type of the parameter is used as elsewhere.
          setAllAliasedTypeSafetyData(
              ParamInfo, dtrans::MismatchedArgUse,
              "Type passed to i8* argument of local function", &Call,
              DumpCallback);

          // The safety data also needs to be set on the type aliases within
          // the callee, because the ValueTypeInfo for the parameter only knows
          // about the type aliases used within the caller.
          setAllAliasedTypeSafetyData(
              ArgInfo, dtrans::MismatchedArgUse,
              "Type passed to i8* argument of local function", &Call,
              DumpCallback);
          continue;
        }

        // If the Dominant type was identified, and the function didn't take an
        // i8*, then it is safe because the dominant type is formed as a union
        // of the type the parameter is used as within the calling function and
        // the type that function is declared as taking.
        if (DomTy)
          continue;

        setAllAliasedTypeSafetyData(ParamInfo, dtrans::BadCasting,
                                    "Incorrect type passed to local function",
                                    &Call, DumpCallback);

        // End of processing of values passed to locally defined functions.
        continue;
      }

      // Process indirect and external calls.
      DTransType *DomTy = PTA.getDominantAggregateUsageType(*ParamInfo);
      if (!DomTy) {
        setAllAliasedTypeSafetyData(
            ParamInfo, dtrans::BadCasting,
            "Ambiguous type passed to indirect or non-local function", &Call,
            DumpCallback);
      }

      if (IsIndirect) {
        // Improved handling of parameters passed to indirect calls.
        // "Address Taken" is only necessary when one of the following is met:
        // - There is an address taken externally defined function that
        //   matches the signature of the call.
        // - There is another type that is structurally compatible with the
        //   parameter type used for the call.
        //   Example:
        //      %struct.test01a = type { i32, i32 }
        //      %struct.test01b = type { i32, i32 }
        //    Passing a pointer to %struct.test01a
        auto &Aliases =
            ParamInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use);
        for (auto *AliasTy : Aliases) {
          if (DTInfo.getDTransUseCRuleCompat() && !HasICMatch && 
              !mayHaveDistinctCompatibleCType(AliasTy))
            continue;
          setBaseTypeInfoSafetyData(AliasTy, dtrans::AddressTaken,
                                    "Type passed to indirect function call",
                                    &Call, DumpCallback);
        }
        continue;
      }

      if (IsLibFunc) {
        // Special handling is needed for i8* parameter types when the address
        // is the first field of a structure, and the field type is an array of
        // i8 elements. The address of 1st array element is also the address of
        // the structure.
        //   %struct.net = type { [200 x i8], i64, ... }
        //
        // A call made with the following parameter needs to be handled:
        //   getelementptr %struct.net, %struct.net, i64 0, i32 0, i32 0
        //
        // The structure will have been marked as "Field address taken" above,
        // we need to avoid also marking the structure type as "Address taken"
        // when the call is just using the i8 array. We allow this when passing
        // the pointer to LibFuncs that we know are using the parameters as a
        // i8* type.

        // Return 'true' if the LibFunc is safe to call with a parameter that
        // aliases a structure type which starts with an array of i8 elements in
        // a VarArg parameter. For non-VarArg parameters we should know the
        // expected type for the parameter, but we would not know this for the
        // VarArg parameters, so this function is used to treat some calls as
        // safe.
        auto IsSafeVarArgLibFunc = [](LibFunc TheLibFunc) {
          switch (TheLibFunc) {
          default:
            break;
          case LibFunc_dunder_isoc99_fscanf:
          case LibFunc_dunder_isoc99_scanf:
          case LibFunc_dunder_isoc99_sscanf:
          case LibFunc_fprintf:
          case LibFunc_fscanf:
          case LibFunc_printf:
          case LibFunc_scanf:
          case LibFunc_snprintf:
          case LibFunc_sscanf:
          case LibFunc_sprintf:
          case LibFunc_vfprintf:
          case LibFunc_vprintf:
            // NOTE: This is not a complete set of routines that may be safe.
            // Rather, it is just a set of routines that are important at the
            // moment.
            return true;
          }

          return false;
        };

        if (((IsVarArgParam && IsSafeVarArgLibFunc(TheLibFunc)) ||
             ArgExpectedDTransTy == getDTransI8PtrType()) &&
            PTA.isElementZeroAccess(DomTy, getDTransI8PtrType())) {
          DEBUG_WITH_TYPE_P(
              FNFilter, SAFETY_VERBOSE,
              dbgs()
                  << "dtrans-safety: Allowing pointer passed to LibFunc: Arg# "
                  << ArgNum << ": " << Call << "\n");
          continue;
        }
      }

      // There is no way to analyze the type a VarArg parameter may get used as
      // when the function definition is not available. Mark it as AddressTaken.
      if (IsVarArgParam) {
        setAllAliasedTypeSafetyData(
            ParamInfo, dtrans::AddressTaken,
            "Type passed as VarArg parameter to non-local function", &Call,
            DumpCallback);
        continue;
      }

      // An aggregate type that escapes to the called function needs to be
      // marked with a safety flag. If we know the function is a library
      // function that expected a pointer to an aggregate type then we treat the
      // type as being a "System object".
      if (IsLibFunc && ArgExpectedDTransTy &&
          isTypeOfInterest(ArgExpectedDTransTy))
        setAllAliasedTypeSafetyData(ParamInfo, dtrans::SystemObject,
                                    "Type passed to LibFunc", &Call,
                                    DumpCallback);

      setAllAliasedTypeSafetyData(ParamInfo, dtrans::AddressTaken,
                                  "Type passed to non-local function", &Call,
                                  DumpCallback);
    }
  }

  // For an allocation call, we want to check that the value produced does not
  // get used as multiple types. Also, collect information for use by the
  // transformations that need to rewrite allocation and free calls. This is
  // just a convenience for migrating legacy code, to avoid the transformations
  // needing to look at the ValueTypeInfo object.
  void analyzeAllocationCall(CallBase *Call, dtrans::AllocKind Kind) {
    // This function supports allocation functions that use the return value to
    // return the pointer to the memory allocated. If another type of allocation
    // function is supported in the future that returns the memory address via a
    // passed in pointer, this function will need to be changed. This assertion
    // is to ensure that any new allocation types handled are also considered
    // here.
    assert((Kind == dtrans::AK_Malloc || Kind == dtrans::AK_Calloc ||
            Kind == dtrans::AK_Realloc || dtrans::isUserAllocKind(Kind) ||
            Kind == dtrans::AK_New) &&
           "Only functions that use return value of call for allocation "
           "supported");

    ValueTypeInfo *Info = PTA.getValueTypeInfo(Call);
    assert(Info && "PtrTypeAnalyzer failed to construct ValueTypeInfo for "
                   "allocation call");
    if (isValueTypeInfoUnhandled(*Info))
      DTInfo.setUnhandledPtrType(Call);

    if (Kind == dtrans::AK_Calloc && Info->canAliasToAggregatePointer())
      for (auto *Ty : Info->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use)) {
        if (!Ty->isPointerTy())
          continue;

        DTransType *ElemTy = Ty->getPointerElementType();
        dtrans::TypeInfo *ParentTI = DTInfo.getTypeInfo(ElemTy);
        assert(ParentTI && "visitModule() should create all TypeInfo objects");
        markAllFieldsWritten(ParentTI, *Call, FWT_ZeroValue);
      }

    dtrans::AllocCallInfo *ACI = DTInfo.createAllocCallInfo(Call, Kind);
    populateCallInfo(*Info, ACI);

    // If the allocation wasn't cast to a type of interest, then nothing more
    // needs to be done.
    if (!Info->canAliasToAggregatePointer())
      return;

    if (Kind == dtrans::AK_New) {
      setAllAliasedTypeSafetyData(Info, dtrans::HasCppHandling,
                                  "Allocation of using 'new'", Call);
    }

    DTransType *DomTy = PTA.getDominantAggregateUsageType(*Info);
    if (!DomTy) {
      setAllAliasedTypeSafetyData(Info, dtrans::BadCasting,
                                  "Allocation of ambiguous type", Call);
      return;
    }

    if (!isValidAllocationSize(Call, Kind, DomTy))
      setBaseTypeInfoSafetyData(
          DomTy, dtrans::BadAllocSizeArg,
          "Allocation size does not match expected type size", Call);
  }

  // Return 'true' if the allocation size is valid for the type being allocated.
  bool isValidAllocationSize(CallBase *Call, dtrans::AllocKind Kind,
                             DTransType *Ty) {
    assert(Ty->isPointerTy() && "Expected pointer type");
    DTransType *AllocType = Ty->getPointerElementType();

    // No need to check pointer-to-pointer allocations, because DTrans is only
    // concerned with modifying uses of an aggregate type.
    if (AllocType->isPointerTy())
      return true;

    const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
    uint64_t ElementSize = DL.getTypeAllocSize(AllocType->getLLVMType());
    unsigned AllocSizeInd = 0;
    unsigned AllocCountInd = 0;
    getAllocSizeArgs(Kind, Call, AllocSizeInd, AllocCountInd, TLI);
    auto *AllocSizeVal = Call->getArgOperand(AllocSizeInd);
    auto *AllocCountVal =
        AllocCountInd != -1U ? Call->getArgOperand(AllocCountInd) : nullptr;

    // If either AllocSizeVal or AllocCountVal can be proven to be a multiple
    // of the element size, the size arguments are acceptable.
    if (dtrans::isValueMultipleOfSize(AllocSizeVal, ElementSize) ||
        dtrans::isValueMultipleOfSize(AllocCountVal, ElementSize))
      return true;

    // If the allocation is cast as a pointer to a fixed size array, and
    // one argument is a multiple of the array's element size and the other
    // is a multiple of the number of elements in the array, the size arguments
    // are acceptable.
    if (AllocType->isArrayTy() && (AllocCountVal != nullptr)) {
      auto *ArrTy = cast<DTransArrayType>(AllocType);
      uint64_t NumArrElements = ArrTy->getNumElements();
      uint64_t ArrElementSize =
          DL.getTypeAllocSize(ArrTy->getElementType()->getLLVMType());
      if ((dtrans::isValueMultipleOfSize(AllocSizeVal, ArrElementSize) &&
           dtrans::isValueMultipleOfSize(AllocCountVal, NumArrElements)) ||
          (dtrans::isValueMultipleOfSize(AllocCountVal, ArrElementSize) &&
           dtrans::isValueMultipleOfSize(AllocSizeVal, NumArrElements)))
        return true;
    }

    // If allocation size is not constant we can try tracing it back to the
    // constant
    uint64_t Res;
    dtrans::TypeInfo *TI = DTInfo.getTypeInfo(AllocType);
    bool EndsInZeroSizedArray = TI ? TI->hasZeroSizedArrayAsLastField() : false;
    if (!dtrans::isValueConstant(AllocSizeVal, &Res) &&
        dtrans::traceNonConstantValue(AllocSizeVal, ElementSize,
                                      EndsInZeroSizedArray)) {
      setBaseTypeInfoSafetyData(AllocType, dtrans::ComplexAllocSize,
                                "Allocation is not direct multiple of size",
                                Call);
      return true;
    }

    return false;
  }

  // For a "free" call, we just collect information for use by the
  // transformations that need to rewrite allocation and free calls. This is
  // just a convenience for migrating legacy code, since the transformation
  // could directly obtain the information from the DTransSafetyInfo class.
  void analyzeFreeCall(CallBase *Call, dtrans::FreeKind FK) {
    dtrans::FreeCallInfo *FCI = DTInfo.createFreeCallInfo(Call, FK);
    unsigned PtrArgInd = -1U;
    const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
    getFreePtrArg(FK, Call, PtrArgInd, TLI);
    assert(PtrArgInd != -1U && "getFreePtrArg failed to set PtrArgInd");

    ValueTypeInfo *Info = PTA.getValueTypeInfo(Call, PtrArgInd);
    assert(Info && "Expected PtrTypeAnalyzer to have ValueTypeInfo for "
                   "free call argument");

    ValueTypeInfo::PointerTypeAliasSetRef &AliasSet =
        Info->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use);
    if (AliasSet.empty())
      return;

    if (FK == dtrans::FK_Delete)
      for (auto *Ty : AliasSet) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: C++ handling -- "
                          << "delete/delete[] function call:\n  " << *Call
                          << "\n");
        setBaseTypeInfoSafetyData(Ty, dtrans::HasCppHandling,
                                  "Type used by delete", Call);
      }

    populateCallInfo(*Info, FCI);
  }

  // This function is used to set the type information captured in the
  // ValueTypeInfo structure into the CallInfo structure that is going to be
  // exposed to the transforms.
  void populateCallInfo(ValueTypeInfo &Info, dtrans::CallInfo *CI) {
    CI->setAnalyzed(true);
    if (Info.canAliasToAggregatePointer()) {
      CI->setAliasesToAggregateType(true);
      ValueTypeInfo::PointerTypeAliasSetRef &AliasSet =
          Info.getPointerTypeAliasSet(ValueTypeInfo::VAT_Use);
      for (auto *Ty : AliasSet) {
        if (!Ty->isPointerTy())
          continue;

        // Add this to our type info list.
        DTransType *ElemTy = Ty->getPointerElementType();
        if (isTypeOfInterest(ElemTy))
          CI->addElemType(ElemTy);
      }
    }
  }

  void visitIntrinsicInst(IntrinsicInst &I) {
    Intrinsic::ID Intrin = I.getIntrinsicID();
    switch (Intrin) {
    default:
      break;

    case Intrinsic::dbg_declare:
    case Intrinsic::dbg_value:
    case Intrinsic::lifetime_end:
    case Intrinsic::lifetime_start:
    case Intrinsic::ptr_annotation:
    case Intrinsic::var_annotation:
      // These intrinsics do not affect the safety checks of
      // the DTrans analysis for any of their arguments.
      return;

    case Intrinsic::memset:
      analyzeMemset(*(cast<MemSetInst>(&I)));
      return;

    case Intrinsic::memcpy:
    case Intrinsic::memmove:
      analyzeMemcpyOrMemmove(*(cast<MemTransferInst>(&I)));
      return;
    }

    // Mark all other intrinsic calls with UnhandledUse for the arguments that
    // are types of interest.
    for (Value *Arg : I.arg_operands()) {
      ValueTypeInfo *Info = PTA.getValueTypeInfo(Arg);
      if (!Info)
        continue;
      setAllAliasedAndPointeeTypeSafetyData(Info, dtrans::UnhandledUse,
                                            "Value passed to intrinsic", &I);
    }
  }

  // Check the destination of a call to memset for safety.
  //
  // A safe call is one where it can be resolved that the operand to the
  // memset meets the following conditions:
  //   - The operand does not affect an aggregate data type.
  //  or
  //   - If the operand is not a pointer to a field within an aggregate, then
  //     the size must be a multiple of the aggregate object's size, or if the
  //     size is smaller than the aggregate type and a specific subset of fields
  //     is set, the memfunc partial write safety bit will be set.
  //   - If the operand is a pointer to a field within an aggregate, then
  //     the size operand must cover the size of one or more fields, in which
  //     case the memfunc partial write safety bit will be set on the containing
  //     structure.
  //  or
  //  - The size operand is 0.
  //
  // A necessary precursor for most of these rules is that the operand type
  // is able to be resolved to a unique dominant type for the pointer.
  //
  // For a safe call, the field information tracking of the aggregate type
  // will be updated to indicate the field is written to.
  void analyzeMemset(MemSetInst &I) {
    DEBUG_WITH_TYPE_P(FNFilter, SAFETY_VERBOSE,
                      dbgs()
                          << "dtrans: Analyzing memset call: ["
                          << I.getFunction()->getName() << "] " << I << "\n");
    Value *DestArg = I.getRawDest();
    // Ignore the memset if the destination is a constant location like null or
    // undef.
    if (isCompilerConstantData(DestArg))
      return;

    Value *SetSize = I.getLength();
    Value *SetValue = I.getValue();
    bool IsSettingNullValue = isa<ConstantInt>(SetValue) &&
                              (cast<ConstantInt>(SetValue))->isZeroValue();

    // A memset of 0 bytes will not affect the safety of any data structure.
    if (dtrans::isValueEqualToSize(SetSize, 0))
      return;

    ValueTypeInfo *DstInfo = PTA.getValueTypeInfo(DestArg);
    assert(DstInfo &&
           "PointerTypeAnalyzer should have collected info for pointer");
    if (!DstInfo->canAliasToAggregatePointer() &&
        !DstInfo->pointsToSomeElement())
      return;

    if (DstInfo->pointsToSomeElement()) {
      DTransStructType *StructTy = nullptr;
      size_t FieldNum = 0;
      uint64_t PrePadBytes = 0;

      // Try to collect a structure type, field and any padding bytes being
      // written for the ElementPointees of DstInfo.
      if (isSimpleStructureMember(DstInfo, &StructTy, &FieldNum,
                                  &PrePadBytes)) {

        // Check the structure, and mark any safety flags needed.
        dtrans::MemfuncRegion RegionDesc;
        if (analyzeMemfuncStructureMemberParam(
                I, StructTy, FieldNum, PrePadBytes, SetSize, RegionDesc,
                IsSettingNullValue ? FWT_ZeroValue : FWT_NonZeroValue))
          createMemsetCallInfo(I, StructTy, RegionDesc);

        return;
      }

      // The element pointee was not able to be analyzed.

      // If the PointeePair is for an array element, propagate the safety data
      // to any structures the array is a part of.
      auto SetSafetyDataOnElementPointees = [this](ValueTypeInfo *PtrInfo,
                                                   dtrans::SafetyData Data,
                                                   StringRef Reason,
                                                   Instruction *I) {
        auto &ElementPointees =
            PtrInfo->getElementPointeeSet(ValueTypeInfo::VAT_Use);
        for (auto PointeePair : ElementPointees)
          if (isa<DTransArrayType>(PointeePair.first)) {
            auto &ElementOfTypes = PointeePair.second.getElementOf();
            for (auto &ElementOfPair : ElementOfTypes)
              if (auto *StTy = dyn_cast<DTransStructType>(ElementOfPair.first))
                setBaseTypeInfoSafetyData(StTy, Data, Reason, I);
          }
      };

      dtrans::SafetyData Data;
      StringRef Reason;
      auto &ElementPointees =
          DstInfo->getElementPointeeSet(ValueTypeInfo::VAT_Use);
      if (ElementPointees.size() != 1) {
        Data = dtrans::AmbiguousPointerTarget;
        Reason = "memset with multiple element pointees";
      } else {
        Data = dtrans::BadMemFuncSize;
        Reason = "memset with array, invalid offset or size";

        DTransType *DestPointeeTy = ElementPointees.begin()->first;
        processBadMemFuncSize(I, DestPointeeTy);
      }
      setAllElementPointeeSafetyData(DstInfo, Data, Reason, &I);
      SetSafetyDataOnElementPointees(DstInfo, Data, Reason, &I);
      return;
    }

    // No need to check types that do not involve aggregates or that are only
    // for pointer-to-pointer types.
    if (!DstInfo->canAliasToDirectAggregatePointer())
      return;

    auto DestParentTy = PTA.getDominantAggregateUsageType(*DstInfo);
    if (!DestParentTy || !DestParentTy->isPointerTy()) {
      setAllAliasedTypeSafetyData(DstInfo, dtrans::AmbiguousPointerTarget,
                                  "memset could not identify dominant type",
                                  &I);
      return;
    }

    auto *DestPointeeTy = DestParentTy->getPointerElementType();
    uint64_t ElementSize = DL.getTypeAllocSize(DestPointeeTy->getLLVMType());
    if (dtrans::isValueMultipleOfSize(SetSize, ElementSize)) {
      dtrans::TypeInfo *ParentTI = DTInfo.getTypeInfo(DestPointeeTy);
      assert(ParentTI && "visitModule() should create all TypeInfo objects");
      markAllFieldsWritten(
          ParentTI, I, IsSettingNullValue ? FWT_ZeroValue : FWT_NonZeroValue);
      dtrans::MemfuncRegion RegionDesc;
      RegionDesc.IsCompleteAggregate = true;
      createMemsetCallInfo(I, DestPointeeTy, RegionDesc);
      return;
    }

    // Check for the case where a portion of a structure is being set, starting
    // from field number zero.
    if (auto *StructTy = dyn_cast<DTransStructType>(DestPointeeTy)) {
      dtrans::MemfuncRegion RegionDesc;
      if (analyzeMemfuncStructureMemberParam(
              I, StructTy, /*FieldNum=*/0,
              /*PrePadBytes=*/0, SetSize, RegionDesc,
              IsSettingNullValue ? FWT_ZeroValue : FWT_NonZeroValue)) {

        createMemsetCallInfo(I, StructTy, RegionDesc);
        return;
      }
    }

    setAllAliasedAndPointeeTypeSafetyData(
        DstInfo, dtrans::BadMemFuncSize,
        "memset could not match size to aggregate type", &I);
    processBadMemFuncSize(I, DestPointeeTy);
  }

  // Check the source and destination pointer parameters of a call to memcpy or
  // memmove call for safety.
  //
  // A safe call is one where it can be resolved that the operands to the
  // call meets one of the following conditions:
  //   - Neither operand affects an aggregate data type.
  //
  //   - For pointers to fields within an aggregate type, a subset of fields may
  //   be copied from the source to the destination, provided that both the
  //   source and destination fields are members of same aggregate type, and
  //   start on the same field number.
  //
  //   - For pointers that point to an aggregate type, both source and
  //   destination must be of the same type, and the size parameter must be a
  //   multiple of the aggregate size.
  //
  //   - For cases where one pointer is a field within aggregate type A and
  //   the other is a pointer to aggregate type B, a copy will be permitted
  //   when the field type is aggregate type B and the size parameter is the
  //   exact size of the field.
  //
  void analyzeMemcpyOrMemmove(MemTransferInst &I) {
    DEBUG_WITH_TYPE_P(FNFilter, SAFETY_VERBOSE,
                      dbgs()
                          << "dtrans: Analyzing memcpy/memmove call: ["
                          << I.getFunction()->getName() << "] " << I << "\n");

    Value *SetSize = I.getLength();
    dtrans::MemfuncCallInfo::MemfuncKind Kind =
        isa<MemCpyInst>(&I) ? dtrans::MemfuncCallInfo::MK_Memcpy
                            : dtrans::MemfuncCallInfo::MK_Memmove;

    auto *DestArg = I.getRawDest();
    auto *SrcArg = I.getRawSource();
    // Ignore the memcpy if the source or destination is a constant location
    // like null or undef.
    if (isCompilerConstantData(DestArg) || isCompilerConstantData(SrcArg))
      return;

    ValueTypeInfo *DstInfo = PTA.getValueTypeInfo(DestArg);
    ValueTypeInfo *SrcInfo = PTA.getValueTypeInfo(SrcArg);
    assert(DstInfo && SrcInfo &&
           "PointerTypeAnalyzer should have collected info for pointers");

    bool DstOfInterest =
        DstInfo->canAliasToAggregatePointer() || DstInfo->pointsToSomeElement();
    bool SrcOfInterest =
        SrcInfo->canAliasToAggregatePointer() || SrcInfo->pointsToSomeElement();
    if (!DstOfInterest && !SrcOfInterest)
      return;

    // Do not support copy between type that is tracked to an aggregate and one
    // that isn't.
    if (!DstOfInterest || !SrcOfInterest) {
      setAllAliasedAndPointeeTypeSafetyData(
          DstInfo, dtrans::BadMemFuncManipulation,
          "memcpy/memmove - src and dest must both be of interest", &I);
      setAllAliasedAndPointeeTypeSafetyData(
          SrcInfo, dtrans::BadMemFuncManipulation,
          "memcpy/memmove - src and dest must both be of interest", &I);
      processBadMemFuncManipulation(I, DstInfo);
      return;
    }

    bool DstPtrToMember = DstInfo->pointsToSomeElement();
    bool SrcPtrToMember = SrcInfo->pointsToSomeElement();
    if (DstPtrToMember || SrcPtrToMember) {

      // Helper function for handling propagation when ValueTypeInfo has an
      // element pointee type. If the PointeePair is an array element, this
      // supports propagating the safety data to a structure which the array is
      // an element of, when LangRuleOutOfBoundsOK is enabled. If
      // LangRuleOutOfBoundsOK is not enabled, then it is assumed that any
      // safety conditions on the array only affect the array, and not any
      // structures it may be part of.
      auto SetSafetyDataOnElementPointees = [this](ValueTypeInfo *PtrInfo,
                                                   dtrans::SafetyData Data,
                                                   StringRef Reason,
                                                   Instruction *I) {
        auto &ElementPointees =
            PtrInfo->getElementPointeeSet(ValueTypeInfo::VAT_Use);
        for (auto PointeePair : ElementPointees) {
          setBaseTypeInfoSafetyData(PointeePair.first, Data, Reason, I);
          if (getLangRuleOutOfBoundsOK() &&
              isa<DTransArrayType>(PointeePair.first)) {
            auto &ElementOfTypes = PointeePair.second.getElementOf();
            for (auto &ElementOfPair : ElementOfTypes)
              if (auto *StTy = dyn_cast<DTransStructType>(ElementOfPair.first))
                setBaseTypeInfoSafetyData(StTy, Data, Reason, I);
          }
        }
      };

      // It is possible that one of the pointers is not an element pointee.
      //
      // For example:
      //   %struct.test01a = type { i32, i32, i32, i32, i32 }
      //   %struct.test01b = type { i32, %struct.test01a }
      //
      // Copy field 1 of %struct.test01b, which is of type %struct.test01a to a
      // pointer that points to a value of type %struct.test01a should be
      // permitted.
      //
      if (!(DstPtrToMember && SrcPtrToMember)) {
        // TODO: Update the field value tracking for the structure fields.

        // Do not set safety issue when the memfunc is for an element pointee
        // that is to/from a single field which is an aggregate type, where
        // the field type matches the expected structure type.
        auto *DestParentTy = PTA.getDominantAggregateUsageType(*DstInfo);
        auto *SrcParentTy = PTA.getDominantAggregateUsageType(*SrcInfo);
        if (DestParentTy == SrcParentTy) {
          DTransStructType *OuterStructType;
          size_t FieldNum;
          uint64_t PrePadBytes;
          uint64_t AccessSize;
          bool IsConstantSize = dtrans::isValueConstant(SetSize, &AccessSize);
          bool IsSimple = isSimpleStructureMember(
              DstPtrToMember ? DstInfo : SrcInfo, &OuterStructType, &FieldNum,
              &PrePadBytes);

          if (IsConstantSize && IsSimple && PrePadBytes == 0) {
            DTransType *ElemTy = OuterStructType->getFieldType(FieldNum);
            assert(ElemTy && "Expected non-null field type");
            if (DL.getTypeStoreSize(ElemTy->getLLVMType()) == AccessSize) {
              dtrans::MemfuncRegion RegionDesc;
              RegionDesc.IsCompleteAggregate = true;
              createMemcpyOrMemmoveCallInfo(I, ElemTy, Kind, RegionDesc,
                                            RegionDesc);
              auto *ElemInfo = DTInfo.getTypeInfo(ElemTy);
              assert(ElemInfo &&
                     "visitModule() should create all TypeInfo objects");
              markAllFieldsWritten(ElemInfo, I, FWT_ExistingValue);
              return;
            }
          }
        }
        dtrans::SafetyData Data = dtrans::BadMemFuncManipulation;
        StringRef Reason =
            "memcpy/memmove - Element pointee and non-Element pointee";
        setAllAliasedAndPointeeTypeSafetyData(DstInfo, Data, Reason, &I);
        setAllAliasedAndPointeeTypeSafetyData(SrcInfo, Data, Reason, &I);
        SetSafetyDataOnElementPointees(DstInfo, Data, Reason, &I);
        SetSafetyDataOnElementPointees(SrcInfo, Data, Reason, &I);
        return;
      }

      // Handle the case where both pointers are element pointees.
      // In this case, require that they are both members of the same type and
      // point to the same field number.
      size_t DstPointeeCount =
          DstInfo->getElementPointeeSet(ValueTypeInfo::VAT_Use).size();
      size_t SrcPointeeCount =
          SrcInfo->getElementPointeeSet(ValueTypeInfo::VAT_Use).size();
      if (DstPointeeCount != 1 || SrcPointeeCount != 1) {
        StringRef AmbigMsg = "memcpy/memmove - multiple element pointees";
        StringRef BadManipMsg = "memcpy/memmove - src/dest not supported";
        if (DstPointeeCount != 1) {
          SetSafetyDataOnElementPointees(
              DstInfo, dtrans::AmbiguousPointerTarget, AmbigMsg, &I);
          SetSafetyDataOnElementPointees(
              SrcInfo, dtrans::BadMemFuncManipulation, BadManipMsg, &I);
          processBadMemFuncManipulation(I, DstInfo);
        } else {
          SetSafetyDataOnElementPointees(
              SrcInfo, dtrans::AmbiguousPointerTarget, AmbigMsg, &I);
          SetSafetyDataOnElementPointees(
              DstInfo, dtrans::BadMemFuncManipulation, BadManipMsg, &I);
        }
        return;
      }

      DTransStructType *DstStructTy = nullptr;
      size_t DstFieldNum = 0;
      uint64_t DstPrePadBytes = 0;
      bool DstSimple = isSimpleStructureMember(DstInfo, &DstStructTy,
                                               &DstFieldNum, &DstPrePadBytes);
      if (!DstSimple) {
        SetSafetyDataOnElementPointees(
            DstInfo, dtrans::BadMemFuncSize,
            "memcpy/memmove - array, invalid offset or size", &I);
        SetSafetyDataOnElementPointees(
            SrcInfo, dtrans::BadMemFuncManipulation,
            "memcpy/memmove - dest was not supported", &I);

        // Treat all the fields of the destination as being written with an
        // unknown value.
        markAllFieldsWritten(I, DstInfo);
        return;
      }

      DTransStructType *SrcStructTy = nullptr;
      size_t SrcFieldNum = 0;
      uint64_t SrcPrePadBytes = 0;
      bool SrcSimple = isSimpleStructureMember(SrcInfo, &SrcStructTy,
                                               &SrcFieldNum, &SrcPrePadBytes);
      if (!SrcSimple) {
        SetSafetyDataOnElementPointees(DstInfo, dtrans::BadMemFuncManipulation,
                                       "memcpy/memmove - src was not supported",
                                       &I);
        SetSafetyDataOnElementPointees(
            SrcInfo, dtrans::BadMemFuncSize,
            "memcpy/memmove - array, invalid offset or size", &I);
        processBadMemFuncManipulation(I, DstInfo);
        return;
      }

      // Copying values from one structure type to another, or from one set of
      // fields to a different set of fields is not supported because the
      // complications this would cause for the transformations. Such as, if one
      // structure had fields deleted because they were write-only, but the
      // other structure didn't have the fields deleted.
      if (!(DstStructTy == SrcStructTy && DstFieldNum == SrcFieldNum &&
            DstPrePadBytes == SrcPrePadBytes)) {
        dtrans::SafetyData Data = dtrans::BadMemFuncManipulation;
        StringRef Reason =
            "memcpy/memmove - non-identical src and dest element pointees";
        SetSafetyDataOnElementPointees(DstInfo, Data, Reason, &I);
        SetSafetyDataOnElementPointees(SrcInfo, Data, Reason, &I);
        processBadMemFuncManipulation(I, DstInfo);
        return;
      }

      // Identify the set of fields affected.
      dtrans::MemfuncRegion RegionDesc;
      if (!analyzeMemfuncStructureMemberParam(I, DstStructTy, DstFieldNum,
                                              DstPrePadBytes, SetSize,
                                              RegionDesc, FWT_ExistingValue)) {
        SetSafetyDataOnElementPointees(
            DstInfo, dtrans::BadMemFuncSize,
            "memcpy/memmove - unsupport array, or invalid offset/size", &I);
        processBadMemFuncSize(I, DstStructTy);
        return;
      }

      // The call is safe for the ElementPointee.
      createMemcpyOrMemmoveCallInfo(I, DstStructTy, Kind,
                                    /*RegionDescDest=*/RegionDesc,
                                    /*RegionDescSrc=*/RegionDesc);

      // NOTE: For memcpy/memmove, we do not mark the "Read" property in the
      // FieldInfo objects. This is to allow for field deletion to identify the
      // field as potentially unneeded.

      // TODO: For ModRef information of the field, we add the function to the
      // "Readers" list in the FieldInfo to record that the field may be
      // referenced.
      return;
    }

    // Start of handling the case where both parameters are pointers to
    // aggregate types.

    // No need to check types that do not involve aggregates or that are only
    // using pointer-to-pointer types.
    if (!(DstInfo->canAliasToDirectAggregatePointer() ||
          SrcInfo->canAliasToDirectAggregatePointer()))
      return;

    auto DestParentTy = PTA.getDominantAggregateUsageType(*DstInfo);
    auto SrcParentTy = PTA.getDominantAggregateUsageType(*SrcInfo);
    if (!DestParentTy || !DestParentTy->isPointerTy() || !SrcParentTy ||
        !SrcParentTy->isPointerTy()) {
      dtrans::SafetyData Data = dtrans::AmbiguousPointerTarget;
      StringRef Reason = "memcpy/memmove - unidentified type for src or dest";
      setAllAliasedTypeSafetyData(DstInfo, Data, Reason, &I);
      setAllAliasedTypeSafetyData(SrcInfo, Data, Reason, &I);
      return;
    }

    if (DestParentTy != SrcParentTy) {
      dtrans::SafetyData Data = dtrans::BadMemFuncManipulation;
      StringRef Reason = "memcpy/memmove - different types for src and dest";
      setAllAliasedTypeSafetyData(DstInfo, Data, Reason, &I);
      setAllAliasedTypeSafetyData(SrcInfo, Data, Reason, &I);
      processBadMemFuncManipulation(I, DstInfo);
      return;
    }

    auto *DestPointeeTy = DestParentTy->getPointerElementType();
    uint64_t ElementSize = DL.getTypeAllocSize(DestPointeeTy->getLLVMType());
    if (dtrans::isValueMultipleOfSize(SetSize, ElementSize)) {
      dtrans::TypeInfo *ParentTI = DTInfo.getTypeInfo(DestPointeeTy);
      assert(ParentTI && "visitModule() should create all TypeInfo objects");
      // The call is safe, and is using the entire structure
      markAllFieldsWritten(ParentTI, I, FWT_ExistingValue);
      dtrans::MemfuncRegion RegionDesc;
      RegionDesc.IsCompleteAggregate = true;
      createMemcpyOrMemmoveCallInfo(I, DestPointeeTy, Kind,
                                    /*RegionDescDest=*/RegionDesc,
                                    /*RegionDescSrc=*/RegionDesc);
      // TODO: For ModRef information of the field, we add the function to the
      // "Readers" list in the FieldInfo to record that the field may be
      // referenced.
      return;
    }

    // Check for the case where a portion of a structure is being set, starting
    // from field number zero.
    if (auto *StructTy = dyn_cast<DTransStructType>(DestPointeeTy)) {
      dtrans::MemfuncRegion RegionDesc;
      if (analyzeMemfuncStructureMemberParam(I, StructTy, /*FieldNum=*/0,
                                             /*PrePadBytes=*/0, SetSize,
                                             RegionDesc, FWT_ExistingValue)) {
        // The call is safe, and affects the region described in RegionDesc
        createMemcpyOrMemmoveCallInfo(I, StructTy, Kind,
                                      /*RegionDescDest=*/RegionDesc,
                                      /*RegionDescSrc=*/RegionDesc);

        // TODO: For ModRef information of the field, we add the function to the
        // "Readers" list in the FieldInfo to record that the field may be
        // referenced.
        return;
      }
    }

    setAllAliasedAndPointeeTypeSafetyData(
        DstInfo, dtrans::BadMemFuncSize,
        "memcpy/memmove - could not match size to aggregate type", &I);
    setAllAliasedAndPointeeTypeSafetyData(
        SrcInfo, dtrans::BadMemFuncSize,
        "memcpy/memmove - could not match size to aggregate type", &I);
    processBadMemFuncSize(I, DestPointeeTy);
  }

  // Helper function for retrieving information when the \p ValueTypeInfo
  // argument refers to a pointer to some element within an aggregate type. This
  // function checks that the pointer to member is a referencing a single member
  // from a single structure. If so, it returns 'true'. Otherwise, return
  // 'false'. When returning 'true', the following output parameters are set:
  //
  // \p StructTy    - The structure type in the \p StructTy.
  // \p FieldNum    - Field number of the first complete field that may be
  //                  accessed.
  // \p PrePadBytes - Number of padding bytes prior to \p FieldNum
  //                  accessed.
  bool isSimpleStructureMember(ValueTypeInfo *Info, DTransStructType **StructTy,
                               size_t *FieldNum, uint64_t *PrePadBytes) {
    assert(Info->pointsToSomeElement() && "Expected pointer to an element");

    auto &ElementPointees = Info->getElementPointeeSet(ValueTypeInfo::VAT_Use);
    if (ElementPointees.size() != 1)
      return false;

    auto &PointeePair = *(ElementPointees.begin());
    if (PointeePair.second.isUnknownOffset())
      return false;

    // Check whether the address is to a location that is not the start of a
    // field. In this case, we need to identify the first field that follows the
    // offset, and the number of bytes to reach it.
    DTransType *Ty = PointeePair.first;
    if (PointeePair.second.isByteOffset()) {
      if (auto *StTy = dyn_cast<DTransStructType>(Ty)) {
        llvm::StructType *LLVMStTy =
            cast<llvm::StructType>(StTy->getLLVMType());
        const StructLayout *SL = DL.getStructLayout(LLVMStTy);
        uint64_t AccessOffset = PointeePair.second.getByteOffset();
        if (AccessOffset < SL->getSizeInBytes()) {
          uint64_t Elem = SL->getElementContainingOffset(AccessOffset);
          uint64_t FieldStart = SL->getElementOffset(Elem);

          // getElementContainingOffset returns the field member prior to any
          // pad bytes if the offset falls into a padding region.
          if (AccessOffset > FieldStart) {
            // Make sure the offset is beyond the end of the prior field member.
            uint64_t FieldSize =
                DL.getTypeStoreSize(LLVMStTy->getElementType(Elem));
            if (AccessOffset < FieldStart + FieldSize)
              return false;

            // Advance to the next field, if possible.
            ++Elem;
            if (Elem == LLVMStTy->getNumElements())
              return false;

            FieldStart = SL->getElementOffset(Elem);
          }
          *StructTy = StTy;
          *FieldNum = Elem;
          *PrePadBytes = FieldStart - AccessOffset;
          return true;
        }
      }
      return false;
    }

    // If the element is an array type, it's possible that the address is for a
    // structure field that is the first element of an aggregate structure. For
    // example:
    //
    //   %struct.network = type { [200 x i8], [200 x i8], i64 }
    //   call void @llvm.memset.p0i8.i64(i8* getelementptr(
    //        %struct.network, %struct.network* @net, i64 0, i32 0, i64 0),
    //       i8 0, i64 408, i1 false)
    //
    // The analysis will later use the size parameter for the call to determine
    // whether this call is only writing to the array element, or to multiple
    // elements of the structure.
    if (isa<DTransArrayType>(Ty)) {
      auto &ElementOfTypes = PointeePair.second.getElementOf();
      if (ElementOfTypes.empty())
        return false;

      if (PointeePair.second.isField() &&
          PointeePair.second.getElementNum() == 0)
        for (auto &ElementOfPair : ElementOfTypes) {
          if (auto *StTy = dyn_cast<DTransStructType>(ElementOfPair.first)) {
            *StructTy = StTy;
            *FieldNum = ElementOfPair.second;
            *PrePadBytes = 0;
            return true;
          }
        }

      return false;
    }

    // It's not a special case. Just get the type and field number if it's a
    // structure field.
    if (auto *StTy = dyn_cast<DTransStructType>(PointeePair.first)) {
      assert(PointeePair.second.isField() && "Expected structure field access");
      *StructTy = StTy;
      *FieldNum = PointeePair.second.getElementNum();
      *PrePadBytes = 0;
      return true;
    }

    return false;
  }

  // Analyze a structure pointer that is passed to memfunc call, possibly using
  // a pointer to one of the fields within the structure to determine which
  // fields are modified, and whether it is a safe usage. Return 'true' if safe
  // usage, and populate the \p RegionDesc with the results.
  bool analyzeMemfuncStructureMemberParam(Instruction &I,
                                          DTransStructType *StructTy,
                                          size_t FieldNum, uint64_t PrePadBytes,
                                          Value *SetSize,
                                          dtrans::MemfuncRegion &RegionDesc,
                                          FieldWriteType WriteType) {
    dtrans::TypeInfo *ParentTI = DTInfo.getTypeInfo(StructTy);
    assert(ParentTI && "visitModule() should create all TypeInfo objects");
    llvm::StructType *LLVMTy = cast<llvm::StructType>(StructTy->getLLVMType());

    // Try to determine if a set of fields in a structure is being written.
    if (analyzePartialStructUse(DL, LLVMTy, FieldNum, PrePadBytes, SetSize,
                                &RegionDesc)) {
      // If not all members of the structure were set, mark it as a partial
      // write.
      if (!RegionDesc.IsCompleteAggregate) {
        setBaseTypeInfoSafetyData(
            StructTy, dtrans::MemFuncPartialWrite,
            "size covers subset of fields of the structures", &I);
      }

      markStructFieldsWritten(ParentTI, RegionDesc.FirstField,
                              RegionDesc.LastField, I, WriteType);
      return true;
    }

    // TODO: Add checks for cases that can be marked with
    // MemFuncNestedStructsPartialWrite instead of BadMemFuncSize.

    // The size could not be matched to the fields of the structure.
    setBaseTypeInfoSafetyData(StructTy, dtrans::BadMemFuncSize,
                              "size does not equal member field type(s) size",
                              &I);
    processBadMemFuncSize(I, StructTy);
    return false;
  }

  // Treat 'Ty' which is the target of a memfunc call as having all the fields
  // 'incomplete' for value tracking. This is a conservative behavior that may
  // need to be relaxed for some cases in the future with an approach that only
  // marks selected fields that are impacted by the call as 'incomplete'
  void processBadMemFuncSize(Instruction &I, DTransType *Ty) {
    dtrans::TypeInfo *ParentTI = DTInfo.getTypeInfo(Ty);
    assert(ParentTI && "visitModule() should create all TypeInfo objects");
    markAllFieldsWritten(ParentTI, I, FWT_NonZeroValue);
  }

  // The legacy DTrans local pointer analyzer implementation only marked
  // selected fields as 'incomplete' when a bad memfunc manipulation was
  // encountered. It's not clear that all cases of bad memfunc manipulation
  // would leave a complete known value set for some fields, so for now go
  // conservative and set them all to 'incomplete'.
  void processBadMemFuncManipulation(Instruction &I, ValueTypeInfo *Info) {
    markAllFieldsWritten(I, Info);
  }

  // For ReturnInst, we need to perform the following checks:
  // - The address of structure is not being returned as a generic type, as this
  // would allow the caller to use the object in a way that is not tracked by
  // the analysis.
  // - The address of a field is not returned, because this would allow
  // reads/writes to the field to be performed without the analysis tracking the
  // location as an element pointee
  // - The type returned is unambiguous and matches the expected type for the
  // function.
  void visitReturnInst(ReturnInst &I) {
    Value *RetVal = I.getReturnValue();
    if (!RetVal)
      return;

    // No need to check type for constants like null or undef values being
    // returned.
    if (isa<ConstantData>(RetVal))
      return;

    // Identify the type that is expected to be returned.
    llvm::Type *ExpectedRetTy = RetVal->getType();
    DTransType *ExpectedRetDTransTy = nullptr;
    if (TM.isSimpleType(ExpectedRetTy)) {
      ExpectedRetDTransTy = TM.getOrCreateSimpleType(ExpectedRetTy);
    } else {
      DTransType *FnTy = MDReader.getDTransTypeFromMD(I.getFunction());
      if (FnTy)
        ExpectedRetDTransTy = cast<DTransFunctionType>(FnTy)->getReturnType();
    }

    // Check for a structure type being returned
    if (ExpectedRetDTransTy) {
      DTransType *BaseTy = ExpectedRetDTransTy;
      while (BaseTy->isArrayTy())
        BaseTy = BaseTy->getArrayElementType();

      if (isa<DTransStructType>(BaseTy))
        setBaseTypeInfoSafetyData(BaseTy, dtrans::WholeStructureReference,
                                  "return of structure type", &I);
    }

    // If the value is not a type of interest, then it will not affect any
    // safety data.
    if (!isPossiblePtrValue(RetVal))
      return;

    // Now find the types collected by the PtrTypeAnalyzer for the value being
    // returned, and start the checks for whether it is safe for the expected
    // type to be returned.
    ValueTypeInfo *Info = PTA.getValueTypeInfo(RetVal);
    if (!Info)
      return;

    if (isValueTypeInfoUnhandled(*Info)) {
      DTInfo.setUnhandledPtrType(RetVal);
      setAllAliasedAndPointeeTypeSafetyData(
          Info, dtrans::UnhandledUse,
          "PointerTypeAnalyzer could not analyze return instruction value", &I);
      return;
    }

    // Nothing more necessary if there are no types associated with the return
    // value.
    if (Info->empty())
      return;

    if (!ExpectedRetDTransTy) {
      setAllAliasedTypeSafetyData(Info, dtrans::UnhandledUse,
                                  "Return of unknown type", &I);
      return;
    }

    // Check whether the address of a field is being returned
    if (Info->pointsToSomeElement()) {
      bool MismatchedType = false;
      for (auto PointeePair :
           Info->getElementPointeeSet(ValueTypeInfo::VAT_Use)) {
        dtrans::TypeInfo *ParentTI = DTInfo.getTypeInfo(PointeePair.first);
        assert(ParentTI && "visitModule() should create all TypeInfo objects");
        if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI)) {
          assert(PointeePair.second.isField() &&
                 "Unexpected use of non-field offset");
          size_t Idx = PointeePair.second.getElementNum();
          setBaseTypeInfoSafetyData(PointeePair.first,
                                    dtrans::FieldAddressTakenReturn,
                                    "Field address returned", &I);
          ParentStInfo->getField(Idx).setAddressTaken();

          // Check that the field type matches the returned type
          if (TM.getOrCreatePointerType(
                  ParentStInfo->getField(Idx).getDTransType()) !=
              ExpectedRetDTransTy)
            MismatchedType = true;
        }
      }

      if (MismatchedType) {
        setAllAliasedAndPointeeTypeSafetyData(
            Info, dtrans::BadCasting, "Return of field using mismatched type",
            &I);
        // We also need to set the expected type with the safety flag because it
        // did not match the field type.
        setBaseTypeInfoSafetyData(ExpectedRetDTransTy, dtrans::BadCasting,
                                  "Return value type did not match this type",
                                  &I);
      }
    }

    if (Info->canAliasToAggregatePointer()) {
      // Check that the type being returned matches the expected type for the
      // function
      DTransType *PtrDomTy = PTA.getDominantAggregateUsageType(*Info);
      if (!PtrDomTy)
        setAllAliasedTypeSafetyData(Info, dtrans::BadCasting,
                                    "Return of ambiguous type", &I);

      // If a pointer to an aggregate type is being returned as a generic type
      // (i8* or pointer-sized-int), then the AddressTaken safety bit will be
      // applied because the caller will treating the returned pointer as just a
      // generic pointer, rather than the actual type.
      if (ExpectedRetDTransTy == getDTransI8PtrType() ||
          ExpectedRetDTransTy == getDTransPtrSizedIntType()) {
        setAllAliasedTypeSafetyData(
            Info, dtrans::AddressTaken,
            "Return of aggregate pointer as a generic type", &I);
      }
      // If a pointer to an aggregate type is being returned as a type that is
      // different than the expected type, we will treat it as BadCasting.
      // TODO: There is the special case where a pointer to an aggregate is
      // returned as the element zero type, which could be treated as
      // FieldAddressTaken in the future, which may be less conservative,
      // because of differences in how the safety conditions cascade and
      // propagate.
      else if (PtrDomTy != ExpectedRetDTransTy) {
        setAllAliasedAndPointeeTypeSafetyData(
            Info, dtrans::BadCasting,
            "Return of aggregate type that does not match expected type", &I);
        setBaseTypeInfoSafetyData(ExpectedRetDTransTy, dtrans::BadCasting,
                                  "Return value type did not match this type",
                                  &I);
      }
    }
  }

  void visitICmpInst(ICmpInst &I) {
    // Compare instructions are always safe because the comparison is not
    // accessing any memory. It does not matter if the pointers represent
    // different types. This visitor method is included here so that base
    // visitInstruction does not mark operands as unhandled.
  }

  void visitIntToPtrInst(IntToPtrInst &I) {
    ValueTypeInfo *Info = PTA.getValueTypeInfo(&I);
    assert(Info && "Expected pointer type analyzer to analyze pointer");

    // TODO: There may be some cases where a pointer to integer conversion can
    // be analyzed as safe if the integer can be traced back to a pointer of
    // the same type as the type this instruction will be used as, but offset
    // by a multiple of the size of the structure.
    setAllAliasedAndPointeeTypeSafetyData(Info, dtrans::BadCasting,
                                          "Integer type cast to pointer", &I);
  }

  void visitLandingPad(LandingPadInst& I) {
    // This instruction results in a type containing a pointer, but there are no
    // safety flags affected.
    return;
  }

  void visitExtractValueInst(ExtractValueInst& I) {
    // This instruction may result in a type containing a pointer, but there are no
    // safety flags affected.
    return;
  }

  void visitResume(ResumeInst& I) {
    // This instruction has an operand containing a pointer type, but there are
    // no safety flags affected.
    return;
  }

  // Process any instruction not handled by other visit functions.
  void visitInstruction(Instruction &I) {
    ValueTypeInfo *Info = PTA.getValueTypeInfo(&I);
    assert((!dtrans::hasPointerType(I.getType()) || Info) &&
           "Expected pointer type analyzer to analyze pointer result");

    if (Info && Info->canAliasToAggregatePointer()) {
      setAllAliasedAndPointeeTypeSafetyData(Info, dtrans::UnhandledUse,
                                            "Unhandled instruction", &I);
    }

    for (unsigned OpNum = 0; OpNum < I.getNumOperands(); ++OpNum)
      if (IsPossiblePtrValue(I.getOperand(OpNum))) {
        ValueTypeInfo *OpInfo = PTA.getValueTypeInfo(&I, OpNum);
        if (OpInfo && OpInfo->canAliasToAggregatePointer())
          setAllAliasedAndPointeeTypeSafetyData(
              OpInfo, dtrans::UnhandledUse,
              "Operand used in unhandled instruction", &I);
      }
  }

  bool IsPossiblePtrValue(Value *V) {
    if (dtrans::hasPointerType(V->getType()))
      return true;

    if (isa<PtrToIntOperator>(V))
      return true;

    // If the value is not a pointer and is not a pointer-sized integer,
    // it is not a value we will track as a pointer.
    if (V->getType() != getLLVMPtrSizedIntType())
      return false;

    // If it is a pointer-sized integer, we need may need to analyze it if
    // it is the result of a load, select or PHI node.
    if (isa<LoadInst>(V) || isa<SelectInst>(V) || isa<PHINode>(V))
      return true;

    // Otherwise, we don't need to analyze it as a pointer.
    return false;
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

  // Check whether the type for the Value object is something that needs to be
  // analyzed for potential pointer types. This differs from just checking the
  // Type of V with isTypeOfInterest, because this also needs to consider the
  // case of a pointer converted into a pointer sized integer.
  bool isPossiblePtrValue(Value *V) const {
    llvm::Type *ValueTy = V->getType();

    // If the value is a pointer or the result of a pointer-to-int cast
    // it definitely is a pointer.
    if (ValueTy->isPointerTy() || isa<PtrToIntOperator>(V))
      return true;

    // A vector of pointers should be analyzed to track the pointer type.
    if (ValueTy->isVectorTy() &&
        cast<VectorType>(ValueTy)->getElementType()->isPointerTy())
      return true;

    // If the value is not a pointer and is not a pointer-sized integer, it
    // is definitely not a value we will track as a pointer.
    if (ValueTy != getLLVMPtrSizedIntType())
      return false;

    // If it is a pointer-sized integer, we may need to analyze it if
    // it is the result of a load, select or PHI node.
    if (isa<LoadInst>(V) || isa<SelectInst>(V) || isa<PHINode>(V))
      return true;

    // Otherwise, we don't need to analyze it as a pointer.
    return false;
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

    case dtrans::FieldAddressTakenMemory:
    case dtrans::FieldAddressTakenCall:
    case dtrans::FieldAddressTakenReturn:
      // Any form of FieldAddressTaken is treated as a pointer carried
      // condition based on how out of bounds field accesses is set because
      // the access is not permitted under the C/C++ rules, but is allowed
      // within the definition of llvm IR. If an out of bounds access is
      // permitted, then it would be possible to access elements of
      // pointed-to objects, as well, in ways that DTrans would not be
      // able to analyze.
      return getLangRuleOutOfBoundsOK();
    case dtrans::AmbiguousGEP:
    case dtrans::BadAllocSizeArg:
    case dtrans::BadCastingConditional:
    case dtrans::BadMemFuncSize:
    case dtrans::BadPtrManipulation:
    case dtrans::ComplexAllocSize:
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

    switch (Data) {
    case dtrans::FieldAddressTakenMemory:
    case dtrans::FieldAddressTakenCall:
    case dtrans::FieldAddressTakenReturn:
    case dtrans::HasZeroSizedArray:
      // We can add additional cases here to reduce the conservative behavior
      // as needs dictate. These cases should not cascade to nested elements,
      // unless we are allowing that the address of one field can be used to
      // access a disjoint field.
      if (getLangRuleOutOfBoundsOK())
        return true;
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
    case dtrans::ComplexAllocSize:
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

    case dtrans::MemFuncPartialWrite:
      // A partial write to a one structure does not imply contained structures
      // are also partially written. Those contained structures should be marked
      // on a case by case basis.
      return false;
    }

    llvm_unreachable("Fully covered switch isn't fully covered?");
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printSafetyDataDebugMessage(dtrans::SafetyData Data, StringRef Reason,
                                   Value *V, ValueTypeInfo *Info,
                                   SafetyInfoReportCB Callback) {
    DEBUG_WITH_TYPE_P(FNFilter, SAFETY_VERBOSE, {
      if (!Reason.empty()) {
        dbgs() << "dtrans-safety: " << dtrans::getSafetyDataName(Data) << " -- "
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
        if (Callback)
          Callback();
        if (Info)
          Info->print(dbgs(), /*Combined=*/false, "    ");
      }
    });
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // This function will identify the aggregate type corresponding to 'Ty',
  // allowing for levels of pointer indirection, and set the SafetyData on the
  // type. Nested and referenced types within the aggregate may also be set
  // based on the pointer-carried and cascading safety data rules.
  // 'Reason' and 'V' are optional parameters (i.e. may be empty or nullptr)
  // that provide information for the debug traces.
  // 'Callback' is an optional callback function that can be used to report
  // additional information about the safety check than just the reason and
  // value. For example, a safety flag set on a parameter of a call may have a
  // callback function to additionally report the argument and argument
  // position. This function will only be called when the debug traces are
  // enabled for the function being analyzed.
  // TODO: Combine 'Reason', 'V' and 'Callback' into single class that can
  // be passed around, and more easily customized for information dumped based
  // on context.
  void setBaseTypeInfoSafetyData(DTransType *Ty, dtrans::SafetyData Data,
                                 StringRef Reason, Value *V,
                                 SafetyInfoReportCB Callback = nullptr) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    printSafetyDataDebugMessage(Data, Reason, V, nullptr, Callback);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    setBaseTypeInfoSafetyDataImpl(Ty, Data, isCascadingSafetyCondition(Data),
                                  isPointerCarriedSafetyCondition(Data), V,
                                  /*ForCascade=*/false,
                                  /*ForPtrCarried=*/false);
  }

  // Set the safety data on all the aliased types of 'PtrInfo'
  // 'Reason' and 'V' are optional parameters (i.e. may be empty or nullptr)
  // that provide information for the debug traces.
  // 'Callback' is an optional callback function that can be used to report
  // additional information about the safety check than just the reason and
  // value. For example, a safety flag set on a parameter of a call, may have a
  // callback function to additionally report the argument and argument
  // position. This function will only be called when the debug traces are
  // enabled for the function being analyzed.
  void setAllAliasedTypeSafetyData(ValueTypeInfo *PtrInfo,
                                   dtrans::SafetyData Data, StringRef Reason,
                                   Value *V,
                                   SafetyInfoReportCB Callback = nullptr) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    printSafetyDataDebugMessage(Data, Reason, V, PtrInfo, Callback);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    setAliasedOrPointeeTypeSafetyDataImpl(PtrInfo, Data, V, /*Aliases=*/true,
                                          /*Pointees=*/false);
  }

  // Set the safety data for only the aggregate types referenced in the
  // ElementPointees list of 'PtrInfo'.
  //
  // 'Reason' and 'V' are optional parameters (i.e. may be empty or nullptr)
  // that provide information for the debug traces.
  // 'Callback' is an optional callback function that can be used to report
  // additional information about the safety check than just the reason and
  // value. For example, a safety flag set on a parameter of a call, may have a
  // callback function to additionally report the argument and argument
  // position. This function will only be called when the debug traces are
  // enabled for the function being analyzed.
  void setAllElementPointeeSafetyData(ValueTypeInfo *PtrInfo,
                                      dtrans::SafetyData Data, StringRef Reason,
                                      Value *V,
                                      SafetyInfoReportCB Callback = nullptr) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    printSafetyDataDebugMessage(Data, Reason, V, PtrInfo, Callback);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    setAliasedOrPointeeTypeSafetyDataImpl(PtrInfo, Data, V, /*Aliases=*/false,
                                          /*Pointees=*/true);
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
  void setAllAliasedAndPointeeTypeSafetyData(
      ValueTypeInfo *PtrInfo, dtrans::SafetyData Data, StringRef Reason,
      Value *V, SafetyInfoReportCB Callback = nullptr) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    printSafetyDataDebugMessage(Data, Reason, V, PtrInfo, nullptr);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    setAliasedOrPointeeTypeSafetyDataImpl(PtrInfo, Data, V, /*Aliases=*/true,
                                          /*Pointees=*/true);
  }

  // Sets the safety data on either the Aliases or ElementPointees of PtrInfo
  // depending on boolean inputs.
  void setAliasedOrPointeeTypeSafetyDataImpl(ValueTypeInfo *PtrInfo,
                                             dtrans::SafetyData Data, Value *V,
                                             bool Aliases, bool Pointees) {
    assert((Aliases || Pointees) &&
           "Expected aliases and/or pointees to be specified");

    if (Aliases)
      for (auto *AliasTy :
           PtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
        setBaseTypeInfoSafetyData(AliasTy, Data, "", V);

    if (Pointees)
      for (auto &PointeePair :
           PtrInfo->getElementPointeeSet(ValueTypeInfo::VAT_Use))
        setBaseTypeInfoSafetyData(PointeePair.first, Data, "", V);
  }

  // This is a helper function that retrieves the aggregate type through
  // zero or more layers of indirection and sets the specified safety data
  // for that type.
  //
  // When 'DoCascade' is set, the safety data will also be set for fields nested
  // (and possibly pointers) within a structure type.
  //
  // When 'DoPtrCarried' is set, the safety data will also be cascaded to
  // the types referenced via the pointer. This parameter is only used, when
  // 'DoCascade' is set to true.
  //
  // 'TriggerValue', 'ForCascade' and 'ForPtrCarried' are informational fields
  // used for the log messages to report the instruction that caused the safety
  // flag, and whether the flag is being set on an inner type due to a safety
  // condition of the outer type.
  //
  // Note, descending into types for cascading of the safety data stops when a
  // type is encountered that already contains the safety data value because all
  // elements reached from it should also already have the safety bit set.
  void setBaseTypeInfoSafetyDataImpl(DTransType *Ty, dtrans::SafetyData Data,
                                     bool DoCascade, bool DoPtrCarried,
                                     Value *TriggerValue, bool ForCascade,
                                     bool ForPtrCarried) {

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

    dtrans::TypeInfo *TI = DTInfo.getTypeInfo(BaseTy);
    assert(TI && "visitModule() should create all TypeInfo objects");
    TI->setSafetyData(Data);
    Log.log(TriggerValue,
            SafetyInfoLog(BaseTy, Data, ForCascade, ForPtrCarried));
    if (!DoCascade)
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
                                                        bool DoCascade,
                                                        bool DoPtrCarried,
                                                        Value *TriggerValue,
                                                        bool ForCascade,
                                                        bool ForPtrCarried) {
      // If FieldTy is not a type of interest, there's no need to propagate.
      if (!isTypeOfInterest(FieldTy))
        return;
      // If the field is a pointer or aggregate type, propagate the condition.
      if (!FieldTy->isPointerTy()) {
        setBaseTypeInfoSafetyDataImpl(FieldTy, Data, DoCascade, DoPtrCarried,
                                      TriggerValue, ForCascade, ForPtrCarried);
      } else if (DoPtrCarried) {
        // In some cases we need to propagate the condition even to fields
        // that are pointers to structures, but in order to avoid infinite
        // loops in the case where two structures each have pointers to the
        // other we need to avoid doing this for structures that already have
        // the condition set.
        DTransType *FieldBaseTy = FieldTy;
        while (FieldBaseTy->isPointerTy())
          FieldBaseTy = FieldBaseTy->getPointerElementType();
        dtrans::TypeInfo *FieldTI = DTInfo.getTypeInfo(FieldBaseTy);
        assert(FieldTI && "visitModule() should create all TypeInfo objects");
        if (!FieldTI->testSafetyData(Data)) {
          (void)BaseTy;
          DEBUG_WITH_TYPE_P(FNFilter, SAFETY_VERBOSE, {
            dbgs()
                << "dtrans-safety: Cascading pointer carried safety condition: "
                << "From: " << *BaseTy << " To: " << *FieldBaseTy
                << " :: " << dtrans::getSafetyDataName(Data) << "\n";
          });
          setBaseTypeInfoSafetyDataImpl(FieldBaseTy, Data, DoCascade,
                                        DoPtrCarried, TriggerValue, ForCascade,
                                        /*ForPtrCarried=*/true);
        }
      }
    };

    // Propagate this condition to any nested types.
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI))
      for (dtrans::FieldInfo &FI : StInfo->getFields())
        MaybePropagateSafetyCondition(FI.getDTransType(), Data, DoCascade,
                                      DoPtrCarried, TriggerValue,
                                      /*ForCascade=*/true, ForPtrCarried);
    else if (isa<dtrans::ArrayInfo>(TI))
      MaybePropagateSafetyCondition(BaseTy->getArrayElementType(), Data,
                                    DoCascade, DoPtrCarried, TriggerValue,
                                    /*ForCascade=*/true, ForPtrCarried);
  }

  // Mark all the fields of the type, and fields of aggregates the type contains
  // as written.
  void markAllFieldsWritten(dtrans::TypeInfo *TI, Instruction &I,
                            FieldWriteType WriteType) {
    if (TI == nullptr)
      return;

    if (!TI->getLLVMType()->isAggregateType()) {
      return;
    }

    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      for (auto &Field : enumerate(StInfo->getFields())) {
        dtrans::FieldInfo &FI = Field.value();
        size_t Idx = Field.index();
        FI.setWritten(I);
        updateFieldFrequency(FI, I);
        DEBUG_WITH_TYPE(SAFETY_FSAF, {
          if (!FI.isBottomAllocFunction())
            dbgs() << "dtrans-fsaf: " << *(StInfo->getLLVMType()) << " ["
                   << Idx << "] <BOTTOM>\n";
        });
        FI.setBottomAllocFunction();
        if (WriteType != FWT_ExistingValue) {
          Constant *C = (WriteType == FWT_ZeroValue)
                            ? Constant::getNullValue(FI.getLLVMType())
                            : nullptr;
          updateFieldValueTracking(*StInfo, FI, Idx, C, &I);
        }

        // TODO: Update frequency count for field info
        auto *ComponentTI = DTInfo.getTypeInfo(FI.getDTransType());
        assert(ComponentTI &&
               "visitModule() should create all TypeInfo objects");
        markAllFieldsWritten(ComponentTI, I, WriteType);
      }
    } else if (auto *AInfo = dyn_cast<dtrans::ArrayInfo>(TI)) {
      auto *ComponentTI = AInfo->getElementDTransInfo();
      markAllFieldsWritten(ComponentTI, I, WriteType);
    }
  }

  // Mark all the fields of any aggregate types contained in the alias and
  // element pointer type sets as written.
  void markAllFieldsWritten(Instruction &I, ValueTypeInfo *Info) {
    for (auto *AliasTy : Info->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
      if (AliasTy->isPointerTy()) {
        dtrans::TypeInfo *TI =
            DTInfo.getTypeInfo(AliasTy->getPointerElementType());
        assert(TI && "visitModule() should create all TypeInfo objects");
        markAllFieldsWritten(TI, I, FWT_NonZeroValue);
      }
    for (auto &PointeePair :
         Info->getElementPointeeSet(ValueTypeInfo::VAT_Use)) {
      dtrans::TypeInfo *TI = DTInfo.getTypeInfo(PointeePair.first);
      assert(TI && "visitModule() should create all TypeInfo objects");
      markAllFieldsWritten(TI, I, FWT_NonZeroValue);
    }
  }

  // A specialized form of the MarkAllFieldsWritten that is used to mark a
  // subset of fields of a structure type as written. Any contained aggregates
  // within the subset are marked as completely written.
  void markStructFieldsWritten(dtrans::TypeInfo *TI, unsigned int FirstField,
                               unsigned int LastField, Instruction &I,
                               FieldWriteType WriteType) {
    assert(TI && isa<dtrans::StructInfo>(TI) &&
           "markStructFieldsWritten requires Structure type");

    auto *StInfo = cast<dtrans::StructInfo>(TI);
    assert(LastField >= FirstField && LastField < StInfo->getNumFields() &&
           "markStructFieldsWritten with invalid field index");

    for (unsigned int Idx = FirstField; Idx <= LastField; ++Idx) {
      auto &FI = StInfo->getField(Idx);
      FI.setWritten(I);
      updateFieldFrequency(FI, I);
      if (WriteType != FWT_ExistingValue) {
        Constant *C = (WriteType == FWT_ZeroValue)
                          ? Constant::getNullValue(FI.getLLVMType())
                          : nullptr;
        updateFieldValueTracking(*StInfo, FI, Idx, C, &I);
      }

      // TODO: Update frequency count for field info
      auto *ComponentTI = DTInfo.getTypeInfo(FI.getDTransType());
      assert(ComponentTI && "visitModule() should create all TypeInfo objects");
      markAllFieldsWritten(ComponentTI, I, WriteType);
    }
  }

  // Create a MemfuncCallInfo object that will store the details about a safe
  // memset call.
  void createMemsetCallInfo(Instruction &I, DTransType *ElemTy,
                            dtrans::MemfuncRegion &RegionDesc) {
    // Add this to our type info list.
    dtrans::MemfuncCallInfo *MCI = DTInfo.createMemfuncCallInfo(
        &I, dtrans::MemfuncCallInfo::MK_Memset, RegionDesc);
    MCI->setAliasesToAggregateType(true);
    MCI->setAnalyzed(true);
    MCI->addElemType(ElemTy);

    // Some transformations may only support memset calls that do not cover all
    // the fields of the structure on the fields not touched by the partial
    // memset.
    if (!RegionDesc.IsCompleteAggregate)
      markFieldsComplexUse(ElemTy, RegionDesc.FirstField, RegionDesc.LastField);
  }

  // Create a MemfuncCallInfo object that will store the details about a safe
  // memcpy/memmove call.
  void createMemcpyOrMemmoveCallInfo(Instruction &I, DTransType *ElemTy,
                                     dtrans::MemfuncCallInfo::MemfuncKind Kind,
                                     dtrans::MemfuncRegion &RegionDescDest,
                                     dtrans::MemfuncRegion &RegionDescSrc) {
    // Add this to our type info list.
    dtrans::MemfuncCallInfo *MCI =
        DTInfo.createMemfuncCallInfo(&I, Kind, RegionDescDest, RegionDescSrc);
    MCI->setAliasesToAggregateType(true);
    MCI->setAnalyzed(true);
    MCI->addElemType(ElemTy);

    // Some transformations may only support memcpy/memmove calls that do not
    // cover all the fields of the structure on the fields not touched by the
    // partial memory write.
    if (!RegionDescDest.IsCompleteAggregate)
      markFieldsComplexUse(ElemTy, RegionDescDest.FirstField,
                           RegionDescDest.LastField);
  }

  // Set the ComplexUse member for the fields from 'First' to 'Last' inclusive
  // on 'AggType' if it is a structure type.
  void markFieldsComplexUse(DTransType *AggType, unsigned First,
                            unsigned Last) {
    dtrans::TypeInfo *TI = DTInfo.getTypeInfo(AggType);
    assert(TI && "visitModule() should create all TypeInfo objects");
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      for (auto I = First, E = Last + 1; I != E; ++I)
        StInfo->getField(I).setComplexUse(true);
    }
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

  bool isExternalAddressTakenFunction(Function *F) {
    if (!ExternalAddressTakenFunctionsComputed) {
      // Cache the set of external address taken functions to avoid needing the
      // compute whether a function is address taken every time an indirect
      // function call is encountered.
      ExternalAddressTakenFunctionsComputed = true;
      Module *M = F->getParent();
      for (auto &F : *M)
        if (F.isDeclaration() && F.hasAddressTaken())
          ExternalAddressTakenFunctions.insert(&F);
    }

    return ExternalAddressTakenFunctions.count(F);
  }

private:
  // private data members
  const DataLayout &DL;
  DTransSafetyInfo::GetTLIFnType GetTLI;
  function_ref<BlockFrequencyInfo &(Function &)> &GetBFI;
  DTransSafetyInfo &DTInfo;
  PtrTypeAnalyzer &PTA;
  TypeMetadataReader &MDReader;
  DTransTypeManager &TM;
  DTransSafetyLogger &Log;

  // Set at the start of each visitFunction call.
  BlockFrequencyInfo *BFI;

  // Set of external functions that are address taken.
  DenseSet<Function*> ExternalAddressTakenFunctions;

  // Indicates whether the ExternalAddressTakenFunctions has been computed yet.
  bool ExternalAddressTakenFunctionsComputed = false;

  // Types that are frequently needed for comparing type aliases against
  // known types.
  DTransType *DTransI8Type = nullptr;                    // i8
  DTransPointerType *DTransI8PtrType = nullptr;          // i8*
  DTransType *DTransPtrSizedIntType = nullptr;           // i64 or i32
  DTransPointerType *DTransPtrSizedIntPtrType = nullptr; // i64* or i32*
  llvm::Type *LLVMPtrSizedIntType = nullptr;             // i64 or i32
};

DTransSafetyInfo::DTransSafetyInfo(DTransSafetyInfo &&Other)
    : TM(std::move(Other.TM)), MDReader(std::move(Other.MDReader)),
      PtrAnalyzer(std::move(Other.PtrAnalyzer)),
      TypeInfoMap(std::move(Other.TypeInfoMap)), CIM(std::move(Other.CIM)),
      MaxTotalFrequency(Other.MaxTotalFrequency),
      UnhandledPtrType(Other.UnhandledPtrType),
      DTransSafetyAnalysisRan(Other.DTransSafetyAnalysisRan) {
  PtrSubInfoMap.insert(Other.PtrSubInfoMap.begin(), Other.PtrSubInfoMap.end());
  Other.PtrSubInfoMap.clear();
  LoadInfoMap.insert(Other.LoadInfoMap.begin(), Other.LoadInfoMap.end());
  Other.LoadInfoMap.clear();
  StoreInfoMap.insert(Other.StoreInfoMap.begin(), Other.StoreInfoMap.end());
  Other.StoreInfoMap.clear();
  MultiElemLoadStoreInfo.insert(Other.MultiElemLoadStoreInfo.begin(),
                                Other.MultiElemLoadStoreInfo.end());
  Other.MultiElemLoadStoreInfo.clear();
  SawFortran = Other.SawFortran;
}

DTransSafetyInfo::~DTransSafetyInfo() { reset(); }

DTransSafetyInfo &DTransSafetyInfo::operator=(DTransSafetyInfo &&Other) {
  reset();
  TM = std::move(Other.TM);
  MDReader = std::move(Other.MDReader);
  PtrAnalyzer = std::move(Other.PtrAnalyzer);
  TypeInfoMap = std::move(Other.TypeInfoMap);
  CIM = std::move(Other.CIM);
  PtrSubInfoMap.insert(Other.PtrSubInfoMap.begin(), Other.PtrSubInfoMap.end());
  Other.PtrSubInfoMap.clear();
  LoadInfoMap.insert(Other.LoadInfoMap.begin(), Other.LoadInfoMap.end());
  Other.LoadInfoMap.clear();
  StoreInfoMap.insert(Other.StoreInfoMap.begin(), Other.StoreInfoMap.end());
  Other.StoreInfoMap.clear();
  MultiElemLoadStoreInfo.insert(Other.MultiElemLoadStoreInfo.begin(),
    Other.MultiElemLoadStoreInfo.end());
  Other.MultiElemLoadStoreInfo.clear();
  MaxTotalFrequency = Other.MaxTotalFrequency;
  UnhandledPtrType = Other.UnhandledPtrType;
  DTransSafetyAnalysisRan = Other.DTransSafetyAnalysisRan;
  SawFortran = Other.SawFortran;
  return *this;
}

void DTransSafetyInfo::analyzeModule(
    Module &M, GetTLIFnType GetTLI, WholeProgramInfo &WPInfo,
    function_ref<BlockFrequencyInfo &(Function &)> GetBFI) {

  // Initialize the DTransTypeManager & TypeMetadataReader classes first, so
  // that the DTrans base class can be run without the complete pointer type
  // analysis being run because there are some transformations that can be done
  // without the complete analysis.
  LLVMContext &Ctx = M.getContext();
  const DataLayout &DL = M.getDataLayout();
  TM = std::make_unique<DTransTypeManager>(Ctx);
  MDReader = std::make_unique<TypeMetadataReader>(*TM);
  if (!MDReader->initialize(M)) {
    LLVM_DEBUG(dbgs() << "DTransSafetyInfo: Type metadata reader did not find "
                         "structure type metadata\n");
    return;
  }

  LLVM_DEBUG(dbgs() << "DTransSafetyInfo::analyzeModule running\n");
  if (!WPInfo.isWholeProgramSafe()) {
    LLVM_DEBUG(dbgs() << "  DTransSafetyInfo: Not Whole Program Safe\n");
    return;
  }

  PtrAnalyzer =
      std::make_unique<PtrTypeAnalyzer>(Ctx, *TM, *MDReader, DL, GetTLI);
  PtrAnalyzer->run(M);
  LLVM_DEBUG(dbgs() << "DTransSafetyInfo: PtrTypeAnalyzer complete\n");

  if (PtrAnalyzer->getUnsupportedAddressSpaceSeen()) {
    LLVM_DEBUG(
        dbgs() << "  DTransSafetyInfo: Unsupported address space seen\n");
    return;
  }

  DTransSafetyLogger Log;
  DTransSafetyInstVisitor Visitor(Ctx, DL, GetTLI, GetBFI, *this, *PtrAnalyzer,
                                  *TM, *MDReader, Log);
  checkLanguages(M);
  Visitor.visit(M);
  PostProcessFieldValueInfo();
  DTransSafetyAnalysisRan = true;

  // Computes TotalFrequency for each StructInfo and MaxTotalFrequency.
  uint64_t MaxTFrequency = 0;
  for (auto *TI : type_info_entries()) {
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      computeStructFrequency(StInfo);
      MaxTFrequency = std::max(MaxTFrequency, StInfo->getTotalFrequency());
    }
  }
  setMaxTotalFrequency(MaxTFrequency);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (PrintSafetyAnalyzerIR) {
    SafetyAnnotationWriter Annotator(Log);
    if (DTransSAFilterPrintFuncs.empty()) {
      M.print(dbgs(), &Annotator);
    } else {
      for (auto &Name : DTransSAFilterPrintFuncs) {
        Function *F = M.getFunction(Name);
        if (F)
          F->print(dbgs(), &Annotator);
      }
    }
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  LLVM_DEBUG({
    dbgs() << "DTransSafetyInfo: Module visited\n";
    if (getUnhandledPtrType())
      dbgs() << "DTransSafetyInfo: Unhandled Value from pointer type "
                "analyzer found\n";
  });

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (dtrans::DTransPrintAnalyzedTypes)
    printAnalyzedTypes();

  if (dtrans::DTransPrintAnalyzedCalls)
    printCallInfo();
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

bool DTransSafetyInfo::getDTransOutOfBoundsOK() const {
  return getLangRuleOutOfBoundsOK();
}

bool DTransSafetyInfo::getDTransUseCRuleCompat() const {
  return !SawFortran && dtrans::DTransUseCRuleCompat;
}

bool DTransSafetyInfo::testSafetyData(dtrans::TypeInfo *TyInfo,
                                      dtrans::Transform Transform) {
  assert(!(Transform & ~dtrans::DT_Legal) && "Illegal transform");

  dtrans::SafetyData Conditions =
      dtrans::getConditionsForTransform(Transform, getDTransOutOfBoundsOK());
  return TyInfo->testSafetyData(Conditions);
}

// Set 'SawFortran' if there is a Fortran function. This will disable
// settings like DTransOutOfBoundsOK.
void DTransSafetyInfo::checkLanguages(Module &M) {
  for (auto &F : M.functions())
    if (F.isFortran()) {
      SawFortran = true;
      break;
    }
}

dtrans::TypeInfo *DTransSafetyInfo::getOrCreateTypeInfo(DTransType *Ty) {
  // If we already have this type in our map, just return it.
  dtrans::TypeInfo *TI = getTypeInfo(Ty);
  if (TI)
    return TI;

  // Create the DTrans TypeInfo object for this type and any sub-types.
  dtrans::TypeInfo *DTransTI;
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

dtrans::TypeInfo *DTransSafetyInfo::getTypeInfo(DTransType *Ty) const {
  // If we have this type in our map, return it.
  auto It = TypeInfoMap.find(Ty);
  if (It != TypeInfoMap.end())
    return It->second;

  return nullptr;
}

dtrans::StructInfo *
DTransSafetyInfo::getStructInfo(llvm::StructType *STy) const {
  assert(!STy->isLiteral() && "Literal types not supported");
  dtransOP::DTransStructType *DTy = TM->getStructType(STy->getName());
  assert(DTy && "Expected existing structure type");
  dtrans::TypeInfo *TI = getTypeInfo(DTy);
  assert(TI && "visitModule() should create all TypeInfo objects");
  auto *StInfo = cast<dtrans::StructInfo>(TI);
  assert(StInfo && "Expected safety analyzer to create TypeInfo");
  return StInfo;
}

DTransStructType *DTransSafetyInfo::getPtrToStructTy(Value *V) {
  auto *Info = getPtrTypeAnalyzer().getValueTypeInfo(V);
  if (!Info)
    return nullptr;
  PtrTypeAnalyzer &PTA = getPtrTypeAnalyzer();
  DTransType* DTransTy = PTA.getDominantAggregateUsageType(*Info);
  if (!DTransTy || !DTransTy->isPointerTy())
    return nullptr;
  return dyn_cast<DTransStructType>(DTransTy->getPointerElementType());
}

bool DTransSafetyInfo::isPtrToStruct(Value *V) {
  return getPtrToStructTy(V);
}

DTransType *DTransSafetyInfo::getFieldTy(StructType *STy, unsigned Idx) {
  if (Idx >= STy->getNumElements())
    return nullptr;
  dtrans::StructInfo *StInfo = getStructInfo(STy);
  dtrans::FieldInfo &FI = StInfo->getField(Idx);
  return FI.getDTransType();
}

DTransType *DTransSafetyInfo::getFieldPETy(StructType *STy, unsigned Idx) {
  DTransType *DTFTy = getFieldTy(STy, Idx);
  if (!DTFTy || !DTFTy->isPointerTy())
    return nullptr;
  return DTFTy->getPointerElementType();
}

bool DTransSafetyInfo::isFunctionPtr(StructType *STy, unsigned Idx) {
  DTransType *DTFPETy = getFieldPETy(STy, Idx);
  return DTFPETy && DTFPETy->isFunctionTy();
} 

StructType *DTransSafetyInfo::getPtrToStructElementType(Value *V) {
  DTransStructType *DTSTy = getPtrToStructTy(V);
  if (!DTSTy)
    return nullptr;
  return dyn_cast_or_null<StructType>(DTSTy->getLLVMType());
} 

bool DTransSafetyInfo::isPtrToStructWithI8StarFieldAt(Value *V,
                                                      unsigned StructIndex) {
  DTransStructType *DTSTy = getPtrToStructTy(V);
  if (!DTSTy || StructIndex >= DTSTy->getNumFields())
    return false;
  DTransType *DTSFTy = DTSTy->getFieldType(StructIndex);
  if (!DTSFTy || !DTSFTy->isPointerTy())
    return false;
  DTransType *DTSFPETy = DTSFTy->getPointerElementType();
  if (!DTSFPETy || !DTSFPETy->isIntegerTy())
    return false;
  return DTSFPETy->getLLVMType()->isIntegerTy(8);
}

bool DTransSafetyInfo::isPtrToIntOrFloat(Value *V) {
  PtrTypeAnalyzer &PTA = getPtrTypeAnalyzer();
  auto *Info = PTA.getValueTypeInfo(V);
  if (!Info)
    return false;
  return PTA.isPtrToIntOrFloat(*Info);
}

bool DTransSafetyInfo::hasPtrToIntOrFloatReturnType(Function *F) {
  DTransType *FnTy = MDReader->getDTransTypeFromMD(F);
  if (!FnTy)
    return false;   
  DTransType *DTy = cast<DTransFunctionType>(FnTy)->getReturnType();
  if (!DTy || !DTy->isPointerTy())
    return false;
  DTransType *DPETy = DTy->getPointerElementType();
  return DPETy && (DPETy->isIntegerTy() || DPETy->isFloatingPointTy());
}

bool DTransSafetyInfo::isPtrToIntOrFloat(dtrans::FieldInfo &FI) {
  DTransType *DTy = FI.getDTransType();
  if (!DTy || !DTy->isPointerTy())
    return false;
  DTransType *DPETy = DTy->getPointerElementType();
  return DPETy && (DPETy->isIntegerTy() || DPETy->isFloatingPointTy());
}

void DTransSafetyInfo::addPtrSubMapping(llvm::BinaryOperator *BinOp,
                                        DTransType *Ty) {
  DEBUG_WITH_TYPE(SAFETY_VERBOSE, {
    dbgs() << "addPtrSubMapping: ";
    if (auto *I = dyn_cast<Instruction>(BinOp))
      dbgs() << I->getFunction()->getName() << ": ";
    dbgs() << *BinOp << " -- " << *Ty << "\n";
  });
  PtrSubInfoMap[BinOp] = Ty;
}

DTransType *DTransSafetyInfo::getResolvedPtrSubType(BinaryOperator *BinOp) {
  return PtrSubInfoMap.lookup(BinOp);
}

std::pair<DTransType *, size_t>
DTransSafetyInfo::getByteFlattenedGEPElement(GEPOperator *GEP) {
  return PtrAnalyzer->getByteFlattenedGEPElement(GEP);
}

std::pair<DTransType *, size_t>
DTransSafetyInfo::getByteFlattenedGEPElement(GetElementPtrInst *GEP) {
  return getByteFlattenedGEPElement(cast<GEPOperator>(GEP));
}

void DTransSafetyInfo::addLoadMapping(LoadInst *LdInst,
                                      std::pair<llvm::Type *, size_t> Pointee) {
  if (LoadInfoMap.count(LdInst)) {
    // If the load is already being tracked for a structure field being
    // accessed, then move it to the MultiElemLoadStoreInfo mapping, as the
    // instruction will require special processing during the transformation
    // analysis.
    DEBUG_WITH_TYPE_P(FNFilter, DEBUG_TYPE,
                      dbgs() << "Moved to MultiElemLoadStore: ["
                             << LdInst->getFunction()->getName() << "] "
                             << *LdInst << ": " << *Pointee.first << "@"
                             << Pointee.second << "\n");

    LoadInfoMap.erase(LdInst);
    addMultiElemLoadStore(LdInst);
    return;
  }
  DEBUG_WITH_TYPE_P(FNFilter, DEBUG_TYPE,
                    dbgs() << "Added to LoadInfoMapping: ["
                           << LdInst->getFunction()->getName() << "] "
                           << *LdInst << ": " << *Pointee.first << "@"
                           << Pointee.second << "\n");
  LoadInfoMap[LdInst] = Pointee;
}

void DTransSafetyInfo::addStoreMapping(
    StoreInst *StInst, std::pair<llvm::Type *, size_t> Pointee) {
  if (StoreInfoMap.count(StInst)) {
    // If the store is already being tracked for a structure field being
    // accessed, then move it to the MultiElemLoadStoreInfo mapping, as the
    // instruction will require special processing during the transformation
    // analysis.
    DEBUG_WITH_TYPE_P(FNFilter, DEBUG_TYPE,
                      dbgs() << "Moved to MultiElemLoadStore: ["
                             << StInst->getFunction()->getName() << "] "
                             << *StInst << ": " << *Pointee.first << "@"
                             << Pointee.second << "\n");

    StoreInfoMap.erase(StInst);
    addMultiElemLoadStore(StInst);
    return;
  }
  DEBUG_WITH_TYPE_P(FNFilter, DEBUG_TYPE,
                    dbgs() << "Added to StoreInfoMapping: ["
                           << StInst->getFunction()->getName() << "] "
                           << *StInst << ": " << *Pointee.first << "@"
                           << Pointee.second << "\n");
  StoreInfoMap[StInst] = Pointee;
}

std::pair<llvm::Type *, size_t>
DTransSafetyInfo::getStoreElement(StoreInst *StInst) {
  auto It = StoreInfoMap.find(StInst);
  if (It == StoreInfoMap.end())
    return std::make_pair(nullptr, 0);
  return It->second;
}

void DTransSafetyInfo::addMultiElemLoadStore(Instruction *I) {
  DEBUG_WITH_TYPE_P(FNFilter, DEBUG_TYPE,
                    dbgs() << "Multi-load-store: "
                           << "[" << I->getFunction()->getName() << "] " << *I
                           << "\n";);
  MultiElemLoadStoreInfo.insert(I);
}

std::pair<llvm::Type *, size_t>
DTransSafetyInfo::getLoadElement(LoadInst *LdInst) {
  auto It = LoadInfoMap.find(LdInst);
  if (It == LoadInfoMap.end())
    return std::make_pair(nullptr, 0);
  return It->second;
}

bool DTransSafetyInfo::isMultiElemLoadStore(Instruction *I) {
  if (MultiElemLoadStoreInfo.count(I))
    return true;
  return false;
}

std::pair<dtrans::StructInfo *, uint64_t>
DTransSafetyInfo::getInfoFromLoad(LoadInst *Load) {
  if (!Load)
    return std::make_pair(nullptr, 0);

  auto GEP = dyn_cast<GEPOperator>(Load->getPointerOperand());
  auto StructField = getStructField(GEP);
  if (!StructField.first)
    return std::make_pair(nullptr, 0);

  dtrans::StructInfo *StInfo = getStructInfo(StructField.first);
  if (!StInfo)
    return std::make_pair(nullptr, 0);

  return std::make_pair(StInfo, StructField.second);
}

// Interface routine to check if the field that is supposed to be loaded in the
// instruction is only read and its parent structure has no safety data
// violations.
//
bool DTransSafetyInfo::isReadOnlyFieldAccess(LoadInst *Load) {
  std::pair<dtrans::StructInfo *, uint64_t> Res = getInfoFromLoad(Load);
  if (!Res.first)
    return false;

  if (testSafetyData(Res.first, dtrans::DT_ElimROFieldAccess))
    return false;

  dtrans::FieldInfo &FI = Res.first->getField(Res.second);
  return FI.isRead() && !FI.isWritten();
}

std::pair<llvm::StructType *, uint64_t>
DTransSafetyInfo::getStructField(GEPOperator *GEP) {
  if (!GEP || !GEP->hasAllConstantIndices())
    return std::make_pair(nullptr, 0);

  if (GEP->getNumIndices() == 1) {
    auto StructField = getByteFlattenedGEPElement(GEP);
    auto StructTy = dyn_cast_or_null<DTransStructType>(StructField.first);
    if (!StructTy)
      return std::make_pair(nullptr, 0);

    return std::make_pair(cast<llvm::StructType>(StructTy->getLLVMType()), StructField.second);
  }

  auto StructTy = dyn_cast<StructType>(GEP->getSourceElementType());
  if (!StructTy)
    return std::make_pair(nullptr, 0);

  if (!cast<ConstantInt>(GEP->getOperand(1))->isZeroValue())
    return std::make_pair(nullptr, 0);

  uint64_t FieldIndex = 0;
  for (unsigned NI = 2; NI <= GEP->getNumIndices(); ++NI) {
    auto IndexConst = cast<ConstantInt>(GEP->getOperand(NI));
    FieldIndex = IndexConst->getLimitedValue();
    if (FieldIndex >= StructTy->getNumElements())
      return std::make_pair(nullptr, 0);
    if (NI == GEP->getNumIndices())
      break;
    auto *Ty = StructTy->getElementType(FieldIndex);
    auto *NewStructTy = dyn_cast<StructType>(Ty);
    if (!NewStructTy)
      return std::make_pair(nullptr, 0);
    StructTy = NewStructTy;
  }
  return std::make_pair(StructTy, FieldIndex);
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

  dbgs() << "\n MaxTotalFrequency: " << getMaxTotalFrequency() << "\n\n";
  dbgs().flush();
}

// Print the call info data
void DTransSafetyInfo::printCallInfo() {

  // To get consistency in the printing order, populate a tuple
  // that can be sorted, then output the sorted list.
  // Sort order is: Function name, CallInfoKind, Instruction
  std::vector<std::tuple<StringRef, dtrans::CallInfo::CallInfoKind, std::string,
                         dtrans::CallInfo *>>
      Entries;

  for (auto *CI : call_info_entries()) {
    Instruction *I = CI->getInstruction();
    std::string InstStr;
    raw_string_ostream Stream(InstStr);
    I->printAsOperand(Stream);
    Stream.flush();
    Entries.emplace_back(std::make_tuple(I->getFunction()->getName(),
                                         CI->getCallInfoKind(), InstStr, CI));
  }

  std::sort(Entries.begin(), Entries.end());
  for (auto &Entry : Entries) {
    dtrans::CallInfo *CI = std::get<3>(Entry);
    Instruction *I = CI->getInstruction();
    dbgs() << "Function: " << I->getFunction()->getName() << "\n";
    dbgs() << "Instruction: " << *I << "\n";
    CI->print(dbgs());
    dbgs() << "\n";
  }
  dbgs().flush();
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Fields of structures that do not meet the necessary safety conditions need to
// be invalidated to prevent them from being marked as containing a complete set
// of values.
void DTransSafetyInfo::PostProcessFieldValueInfo() {
  bool UsingOutOfBoundsOK = getDTransOutOfBoundsOK();

  for (auto *TI : type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    // There following conditions are reasons to invalidate the constant values
    // as holding a complete set of possible values.
    // - The safety flags indicate the structure is used in a non-supported way.
    //   Note: The safety flags allowed are dependent on the value that
    //   LangRuleOutOfBoundsOK is set to.
    // - The field is address taken, and the value of LangRuleOutOfBoundsOK
    //   indicates the address could be used to access other field members.
    // - The type is an aggregate type. We keep values for individual fields,
    //   but not for an array or structure element as a whole.
    bool FSV_Unsafe = testSafetyData(TI, dtrans::DT_FieldSingleValue);
    bool FSAF_Unsafe = testSafetyData(TI, dtrans::DT_FieldSingleAllocFunction);
    for (unsigned I = 0, E = StInfo->getNumFields(); I != E; ++I) {
      if (FSV_Unsafe ||
          (!UsingOutOfBoundsOK && StInfo->getField(I).isAddressTaken()) ||
          StInfo->getField(I).getLLVMType()->isAggregateType())
        StInfo->getField(I).setMultipleValue();
      if (FSAF_Unsafe ||
          (!UsingOutOfBoundsOK && StInfo->getField(I).isAddressTaken())) {
        DEBUG_WITH_TYPE(SAFETY_FSAF, {
          if (!StInfo->getField(I).isBottomAllocFunction())
            dbgs() << "dtrans-fsaf: " << *(StInfo->getLLVMType()) << " ["
                   << I << "] <BOTTOM>\n";
        });
        StInfo->getField(I).setBottomAllocFunction();
      }
    }
  }
}

// Computes total frequency of all fields and sets TotalFrequency of
// 'StInfo'.
void DTransSafetyInfo::computeStructFrequency(dtrans::StructInfo *StInfo) {
  uint64_t StructFreq = 0;
  for (dtrans::FieldInfo &FI : StInfo->getFields()) {
    uint64_t TFreq = StructFreq + FI.getFrequency();
    // Check for overflow.
    if (TFreq < StructFreq) {
      StructFreq = std::numeric_limits<uint64_t>::max();
      break;
    }
    StructFreq = TFreq;
  }
  StInfo->setTotalFrequency(StructFreq);
}

/// Handle invalidation events in the new pass manager.
bool DTransSafetyInfo::invalidate(Module &M, const PreservedAnalyses &PA,
                                  ModuleAnalysisManager::Invalidator &Inv) {
  auto PAC = PA.getChecker<DTransSafetyAnalyzer>();
  return !PAC.preservedWhenStateless();
}


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

INITIALIZE_PASS_BEGIN(DTransSafetyAnalyzerWrapper, "dtrans-safetyanalyzer",
                      "Data transformation safety analyzer", false, true)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(BlockFrequencyInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransSafetyAnalyzerWrapper, "dtrans-safetyanalyzer",
                    "Data transformation safety analyzer", false, true)

char DTransSafetyAnalyzerWrapper::ID = 0;
DTransSafetyAnalyzerWrapper::DTransSafetyAnalyzerWrapper() : ModulePass(ID) {
  initializeDTransSafetyAnalyzerWrapperPass(*PassRegistry::getPassRegistry());
}

DTransSafetyInfo &DTransSafetyAnalyzerWrapper::getDTransSafetyInfo(Module &M) {
  return Result;
}

bool DTransSafetyAnalyzerWrapper::runOnModule(Module &M) {
  auto GetBFI = [this](Function &F) -> BlockFrequencyInfo & {
    return this->getAnalysis<BlockFrequencyInfoWrapperPass>(F).getBFI();
  };
  auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
    return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  };

  WholeProgramInfo &WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
  Result.analyzeModule(M, GetTLI, WPInfo, GetBFI);
  return false;
}

bool DTransSafetyAnalyzerWrapper::doFinalization(Module &M) {
  Result.reset();
  return false;
}

void DTransSafetyAnalyzerWrapper::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<BlockFrequencyInfoWrapperPass>();
  AU.addRequired<WholeProgramWrapperPass>();
}

ModulePass *llvm::createDTransSafetyAnalyzerTestWrapperPass() {
  return new dtransOP::DTransSafetyAnalyzerWrapper();
}

//===-----------------------PtrTypeAnalyzer.cpp---------------------------===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"

#include "Intel_DTrans/Analysis/DTransAllocCollector.h"
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/Analysis/DTransDebug.h"
#include "Intel_DTrans/Analysis/DTransLibraryInfo.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FormattedStream.h"

#include <deque>

using namespace llvm::PatternMatch;

#define DEBUG_TYPE "dtrans-pta"

#define DTRANS_PARTIALPTR "dtrans-partialptr"

// Trace messages for the pointer type analyzer when new information is
// associated with a Value object.
#define VERBOSE_TRACE "dtrans-pta-verbose"

// Trace message about information collected for dependent values.
#define VERBOSE_DEP_TRACE "dtrans-pta-dep-verbose"

namespace llvm {
namespace dtransOP {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// A trace filter that will be enabled/disabled based on the function name.
static llvm::dtrans::debug::DebugFilter FNFilter;

// Comma separated list of function names that the verbose debug output should
// be reported for. If empty, all verbose messages are generated when the
// appropriate -debug-only value is set. Otherwise, messages are only emitted
// when processing functions matching the name of an entry in the list.
static cl::list<std::string> DTransPTAFilterPrintFuncs(
    "dtrans-pta-filter-print-funcs", cl::CommaSeparated, cl::ReallyHidden,
    cl::desc(
        "Filter emission of verbose debug messages to specific functions"));

// When 'true', print the IR intermixed with comments indicating the types
// resolved by the pointer type analyzer.
static cl::opt<bool> PrintPTAResults(
    "dtrans-print-pta-results", cl::ReallyHidden,
    cl::desc("Print IR with pointer type analyzer results as comments"));

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Control whether the 'declared' type and 'usage' type sets are reported
// separately (false), or combined into a single set (true) when dumping the
// IR intermixed with the collected information.
static cl::opt<bool> PTAEmitCombinedSets(
    "dtrans-pta-emit-combined-sets", cl::init(true), cl::ReallyHidden,
    cl::desc("Print merged results of declaration and usage types"));

// Helper function to check if \p is 'null', 'undef' or other compiler constant.
static bool isCompilerConstant(const Value *V) {
  if (!V)
    return false;

  // Checks for Null, Undef, Int, FP, ...
  return isa<ConstantData>(V);
}

// Return true if the Type is something that needs to be analyzed for type
// recovery.
//
// In general, this is checking for a pointer type, an array/vector of pointer
// types, or a structure type.
//
static bool isTypeOfInterest(llvm::Type *Ty) {
  if (Ty->isPointerTy())
    return true;

  if (Ty->isVectorTy() && cast<VectorType>(Ty)->getElementType()->isPointerTy())
    return true;

  if (Ty->isArrayTy())
    return isTypeOfInterest(Ty->getArrayElementType());

  // Non-opaque named and literal structures should be types of interest.
  if (Ty->isStructTy() && !cast<llvm::StructType>(Ty)->isOpaque())
    return true;

  return false;
}

// Return 'true' if the Function is marked with the attribute that indicates it
// is a function created by PAROPT to be used as callback target.
static bool isPAROPTCreatedFunction(Function &F) {
  return F.hasFnAttribute("processed-by-vpo");
}

// Helper to get the base object type that makes up an array/vector/array nest
// type.
static DTransType *getSequentialObjectBaseType(DTransSequentialType *Ty) {
  DTransType *BaseTy = Ty;
  while (auto SeqTy = dyn_cast<DTransSequentialType>(BaseTy))
    BaseTy = SeqTy->getTypeAtIndex(0);

  return BaseTy;
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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// This class is used to annotate the IR dump output with the information
// collected by the pointer analyzer for pointers, or values that may have been
// cast from pointers printed as comments within the listing.
class PtrTypeAnalyzerAnnotationWriter : public AssemblyAnnotationWriter {
public:
  // @param Analyzer          - Hold results of pointer type analysis
  // @param CombineUseAndDecl - When 'true' don't print declaration set types
  //                            separate from 'usage' types.
  PtrTypeAnalyzerAnnotationWriter(PtrTypeAnalyzer &Analyzer,
                                  bool CombineUseAndDecl)
      : Analyzer(Analyzer), CombineUseAndDecl(CombineUseAndDecl) {}

  // Dump the list of input parameter types detected for the function.
  void emitFunctionAnnot(const Function *F,
                         formatted_raw_ostream &OS) override {
    // We don't have information for parameters of function declarations.
    if (F->isDeclaration())
      return;

    OS << ";  Input Parameters: " << F->getName() << "\n";
    for (auto &A : F->args()) {
      OS << ";    Arg " << A.getArgNo() << ": " << A << "\n";

      // Report the type info if the object is known to be a pointer. Also,
      // report any information that may have been collected due to an inttoptr
      // conversion detected on a non-pointer argument.
      auto Info = Analyzer.getValueTypeInfo(&A);
      if (Info) {
        Info->print(OS, CombineUseAndDecl, ";    ");
        OS << ";      ";
        printDominantUsageType(OS, *Info);
      } else if (A.getType()->isPointerTy()) {
        OS << ";    <NO PTR INFO AVAILABLE>\n";
      }
    }
  }

  // For pointers and values of interest, print the type information determined
  // for Value \p CV
  void printInfoComment(const Value &CV, formatted_raw_ostream &OS) override {
    std::function<void(formatted_raw_ostream &, ConstantExpr *)>
        PrintConstantExpr =
            [&PrintConstantExpr, this](formatted_raw_ostream &OS,
                                       ConstantExpr *CE) -> void {
      OS << "\n;        CE: " << *CE << "\n";
      auto *Info = Analyzer.getValueTypeInfo(CE);
      if (Info) {
        Info->print(OS, CombineUseAndDecl, ";          ");
        OS << ";            ";
        printDominantUsageType(OS, *Info);
      } else if (CE->getType()->isPointerTy()) {
        OS << ";          <NO PTR INFO AVAILABLE FOR ConstantExpr>\n";
      }

      // There may be constant expressions nested within this CE that should be
      // reported.
      for (auto *Op : CE->operand_values())
        if (auto *InnerCE = dyn_cast<ConstantExpr>(Op))
          PrintConstantExpr(OS, InnerCE);
    };

    Value *V = const_cast<Value *>(&CV);

    // Check for any constant expressions being used for the instruction, and
    // report types for those, if available.
    if (auto *I = dyn_cast<Instruction>(V))
      for (auto *Op : I->operand_values())
        if (auto *CE = dyn_cast<ConstantExpr>(Op))
          PrintConstantExpr(OS, CE);

    // Report the information about the value produced by the instruction.
    bool ExpectPointerInfo =
        isTypeOfInterest(V->getType()) || isa<PtrToIntInst>(V);

    // The value is being produced by an instruction, so can use
    // getValueTypeInfo, without checking if it is 'null'/'undef' here.
    auto *Info = Analyzer.getValueTypeInfo(V);
    if (ExpectPointerInfo && !Info) {
      OS << "\n;    <NO PTR INFO AVAILABLE>\n";
      return;
    }

    // Info objects may be created for some Values that are not pointers. Skip
    // printing in these cases to allow better comparisons with legacy
    // LocalPointerAnalyzer dumps.
    if (Info && (ExpectPointerInfo || !Info->empty())) {
      OS << "\n";
      if (auto *GEP = dyn_cast<GEPOperator>(V)) {
        auto FlatGepInfo = Analyzer.getFlattenedGEPElement(GEP);
        if (FlatGepInfo)
          OS << ";      <FLATTENED GEP>\n";
      }
      Info->print(OS, CombineUseAndDecl, ";    ");
      OS << ";      ";
      printDominantUsageType(OS, *Info);
    }
  }

  void printDominantUsageType(raw_ostream &OS, ValueTypeInfo &Info) {
    DTransType *DomTy = Analyzer.getDominantType(Info, ValueTypeInfo::VAT_Use);
    if (DomTy)
      OS << "DomTy: " << *DomTy << "\n";
    else if (Info.canAliasToAggregatePointer())
      OS << "Ambiguous Dominant Type\n";
    else
      OS << "No Dominant Type\n";
  }

private:
  PtrTypeAnalyzer &Analyzer;
  bool CombineUseAndDecl;
};
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// This class defines the implementation for the PtrTypeAnalyzer.
class PtrTypeAnalyzerImpl {
public:
  PtrTypeAnalyzerImpl(
      LLVMContext &Ctx, DTransTypeManager &TM, TypeMetadataReader &MDReader,
      const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI)
      : TM(TM), MDReader(MDReader), DL(DL), GetTLI(GetTLI),
        DTransLibInfo(TM, GetTLI) {
    LLVMI8Type = llvm::Type::getInt8Ty(Ctx);
    LLVMPointerSizedIntType =
        llvm::Type::getIntNTy(Ctx, DL.getPointerSizeInBits());

    DTransI8Type = TM.getOrCreateAtomicType(LLVMI8Type);
    DTransI8PtrType = TM.getOrCreatePointerType(DTransI8Type);
    DTransPtrSizedIntType = TM.getOrCreateAtomicType(LLVMPointerSizedIntType);
    DTransPtrSizedIntPtrType = TM.getOrCreatePointerType(DTransPtrSizedIntType);
  }

  ~PtrTypeAnalyzerImpl();

  void run(Module &M);

  // Get the ValueTypeInfo object for the Value, creating it if necessary.
  ValueTypeInfo *getOrCreateValueTypeInfo(Value *V);

  // Get the ValueTypeInfo object for the specified operand of the User.
  // This method must be used for a Value object that represents 'null'. It may
  // be used for other Value objects, in which case it just redirects to the
  // above overload. Creates a new ValueTypeInfo object if necessary.
  ValueTypeInfo *getOrCreateValueTypeInfo(const User *U, unsigned OpNum);

  // Get the ValueTypeInfo object, if it exists.
  ValueTypeInfo *getValueTypeInfo(const Value *V) const;
  ValueTypeInfo *getValueTypeInfo(const User *U, unsigned OpNum) const;

  // Record that the GEPOperator is using an i8* type to access the element at
  // 'Idx' of the specified aggregate type 'Ty'
  void addByteFlattenedGEPMapping(GEPOperator *GEP, DTransType *Ty, size_t Idx);

  std::pair<DTransType *, size_t>
  getByteFlattenedGEPElement(GEPOperator *GEP) const;

  // Record that a GEP result for the field was obtained using a GEP that
  // indexed the field by using multiples of the size of a pointer. This is
  // similar to a byte-flattened GEP access, but the offset is computing using a
  // multiple of the size of a pointer (or some other type), rather than
  // requiring it to be a single byte.
  //
  // For example, given:
  //   %struct.BMLog = type { ptr, ptr, ptr, %struct.ListBase, ptr }
  //   %struct.ListBase = type { ptr, ptr }
  //
  // If %in was resolved to be a pointer to %struct.BMLog, then all the
  // following are equivalent, and resolve to the address of the last
  // field of %struct.BMLog:
  //   1)  %field4 = getelementptr %struct.BMLog, ptr %in, i64 0, i32 4
  //   2)  %field4 = getelementptr ptr, ptr %in, i64 5
  //   3)  %field4 = getelementptr i8, ptr %in, i64 40
  //
  // 1. uses normal structure addressing
  // 2. uses flattened addressing
  // 3. uses byte-flattened GEP addressing. (this is just a specific form of
  //    flattened indexing that the transformations currently understand)
  //
  // TODO: It would be nice to unify this with the byte-flattened GEP tracking,
  // instead of having a separate map for the cases that were identified as
  // being a byte-flattened GEP.
  void addFlattenedGEPMapping(GEPOperator *GEP, DTransType *Ty, size_t Idx,
                              size_t Multiplier);

  // Query function for saved results about flattened GEPs.
  llvm::Optional<PtrTypeAnalyzer::FlattenedGEPInfoType>
  getFlattenedGEPElement(GEPOperator *GEP) const;

  void addAllocationCall(CallBase *Call, dtrans::AllocKind Kind) {
    AllocationCalls.insert({Call, Kind});
  }

  void addFreeCall(CallBase *Call, dtrans::FreeKind Kind) {
    FreeCalls.insert({Call, Kind});
  }

  dtrans::AllocKind getAllocationCallKind(CallBase *Call) const {
    auto It = AllocationCalls.find(Call);
    if (It == AllocationCalls.end())
      return dtrans::AK_NotAlloc;
    return It->second;
  }

  dtrans::FreeKind getFreeCallKind(CallBase *Call) const {
    auto It = FreeCalls.find(Call);
    if (It == FreeCalls.end())
      return dtrans::FK_NotFree;
    return It->second;
  }

  // Set Ty as the declaration type of value V, and mark the ValueTypeInfo as
  // completely analyzed.
  void setDeclaredType(Value *V, DTransType *Ty);

  llvm::Type *getLLVMI8Type() const { return LLVMI8Type; }
  llvm::Type *getLLVMPointerSizedIntType() const {
    return LLVMPointerSizedIntType;
  }

  DTransType *getDTransI8Type() const { return DTransI8Type; }
  DTransPointerType *getDTransI8PtrType() const { return DTransI8PtrType; }

  DTransType *getDTransPtrSizedIntType() const { return DTransPtrSizedIntType; }

  DTransType *getDTransPtrSizedIntPtrType() const {
    return DTransPtrSizedIntPtrType;
  }

  // If the type is used for a single aggregate type, return the type, provided
  // that any other pointer types are generic equivalents for the type, such as
  // i8* or a pointer-sized integer pointer, or they are equivalent types that
  // allow access to the element-zero  member. If the case, where there are
  // multiple aggregate types,  which are related based on nesting of the types
  // at the element-zero position, return the outermost aggregate type for the
  // nesting. Otherwise, return nullptr meaning either the value does not alias
  // an aggregate type (i.e. canAliasToAggregatePointer() == false), or there is
  // not a single dominant aggregate type.
  DTransType *getDominantAggregateUsageType(ValueTypeInfo &Info) const;

  // Like getDominantAggregateUsageType, but allows specifying whether the
  // VAT_Decl or VAT_Use alias set should be analyzed to find the type.
  DTransType *
  getDominantAggregateType(ValueTypeInfo &Info,
                           ValueTypeInfo::ValueAnalysisType Kind) const;

  // Like getDominantAggregateType, but can return a dominant type among a set
  // of pointers to scalar types when there is no aggregate type.
  DTransType *getDominantType(ValueTypeInfo &Info,
                              ValueTypeInfo::ValueAnalysisType Kind) const;

  // Returns 'true' if the dominant type is a pointer-to-pointer type.
  bool isPtrToPtr(ValueTypeInfo &Info) const;

  bool isPtrToIntOrFloat(ValueTypeInfo &Info) const;

  // Return 'true' if the dominant type is a array of 'i8' elements, or pointer
  // to array of 'i8' elements. If 'AggArType' is supplied, populate it with
  // the actual array type.
  bool isPtrToCharArray(ValueTypeInfo &Info,
                        DTransArrayType **AggArType = nullptr) const;

  // This function is called to determine if 'DestTy' could be used to access
  // element 0 of 'SrcTy'. If 'DestTy' is a pointer type whose element type is
  // the same as the type of element zero of the aggregate pointed to by the
  // 'SrcTy' pointer type, then it would be a valid element zero access.
  bool isElementZeroAccess(DTransType *SrcTy, DTransType *DestTy,
                           DTransType **AccessedTy = nullptr) const;

  // Return 'true' if a pointer to 'DestPointeeTy' can be used to access element
  // zero of 'SrcPointeeTy'. This is similar to 'isElementZeroAccess', except
  // that it operates on the type the pointers point to.
  bool isPointeeElementZeroAccess(DTransType *SrcTy, DTransType *DestTy,
                                  DTransType **AccessedType = nullptr) const;

  // Get the element zero type as a pair, where the first element returns the
  // deepest nested aggregate type, and the second element returns the type of
  // element zero of that type.
  using AggregateElementPair = std::pair<DTransType *, DTransType *>;
  Optional<AggregateElementPair> getElementZeroType(DTransType *Ty);

  void setUnsupportedAddressSpaceSeen() { UnsupportedAddressSpaceSeen = true; }
  bool getUnsupportedAddressSpaceSeen() const {
    return UnsupportedAddressSpaceSeen;
  }

  // If there is not element-zero info tracked for the instruction, start
  // tracking it. Otherwise, if the 'Ty' or 'Depth' values differ from what is
  // tracked, mark the tracked data as an unknown type.
  void addElementZeroPointer(Instruction *I, DTransType *Ty,
                             unsigned int Depth) {
    auto It = ElementZeroPointers.find(I);
    if (It != ElementZeroPointers.end()) {
      if (It->second.Ty != Ty || It->second.Depth != Depth) {
        Ty = nullptr;
        Depth = 0;
        DEBUG_WITH_TYPE_P(
            FNFilter, VERBOSE_TRACE,
            dbgs() << "Warning: Element zero type already exists for: ["
                   << I->getFunction()->getName() << "]" << *I << "\n");
      }
    }

    ElementZeroPointers[I] = PtrTypeAnalyzer::ElementZeroInfo(Ty, Depth);
  }

  PtrTypeAnalyzer::ElementZeroInfo getElementZeroPointer(Instruction *I) const {
    auto It = ElementZeroPointers.find(I);
    if (It != ElementZeroPointers.end())
      return It->second;
    return PtrTypeAnalyzer::ElementZeroInfo(nullptr, 0);
  }

private:
  DTransTypeManager &TM;
  TypeMetadataReader &MDReader;
  const DataLayout &DL;
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;

  // Class that provides information about DTransTypes for library functions.
  DTransLibraryInfo DTransLibInfo;

  // We cannot use DenseMap or ValueMap here because we are inserting values
  // during recursive calls to analyzeValue() and with a DenseMap or
  // ValueMap that would cause the LocalPointerInfo to be copied and our
  // local reference to it to be invalidated.
  //
  // Separate maps will be used for each Function, along with a map for Global
  // objects (uses 'nullptr' as the 'LocalMaps' lookup key) to avoid having to
  // search a single map of all Value objects when looking up the ValueTypeInfo
  // object for a Value.
  using PerFunctionLocalMapType = std::map<const Value*, ValueTypeInfo*>;
  std::map<const Function*, PerFunctionLocalMapType> LocalMaps;

  // Compiler constants such as 'undef' and 'null' need to have context
  // sensitive information. For example:
  //   %ptr1 = alloca %struct.foo
  //   %ptr2 = alloca %struct.bar
  //   store ptr null, ptr %ptr1
  //   store ptr null, ptr %ptr2
  // The IR only has a single Value object instantiated to represent a null
  // pointer, but for the purpose of our analysis we need to track them as
  // representing different types.
  std::map<std::pair<const User *, unsigned>, ValueTypeInfo *>
      LocalMapForConstant;

  // A mapping from GEPOperators that have been identified as being structure
  // element accesses in byte-flattened form to a type-index pair for the
  // element being accessed.
  using ByteFlattenedGEPInfoMapType =
      DenseMap<GEPOperator *, std::pair<DTransType *, size_t>>;
  ByteFlattenedGEPInfoMapType ByteFlattenedGEPInfoMap;

  // A mapping from GEPs that have been identified as being structure
  // element accesses in flattened form to map the GEP to:
  //   <Type, Field number, Index multiplier>
  //
  // For example:
  // Given %in that represents a pointer to a structure that is 48 bytes long,
  // the following would be accessing a field within the structure using a
  // flattened form:
  //
  //   %field = getelementptr ptr, ptr %in, i64 5
  // or
  //   %gep = getelementptr double, ptr %in, i64 5
  //
  using FlattenedGEPInfoMapType =
      DenseMap<GEPOperator *, PtrTypeAnalyzer::FlattenedGEPInfoType>;
  FlattenedGEPInfoMapType FlattenedGEPInfoMap;

  // Map of calls identified as being memory allocation calls to the allocation
  // kind.
  DenseMap<CallBase *, dtrans::AllocKind> AllocationCalls;

  // Map of calls identified as being memory free calls to the free kind.
  DenseMap<CallBase *, dtrans::FreeKind> FreeCalls;

  // LLVM Type for 'i8'
  llvm::Type *LLVMI8Type;

  // LLVM integer type that is the same size as a pointer in address space 0.
  llvm::Type *LLVMPointerSizedIntType;

  // Representation of the 'i8' type in DTransType system
  DTransType *DTransI8Type;

  // Representation of the 'i8*' type in DTransType system
  DTransPointerType *DTransI8PtrType;

  // Representation of an integer type that is the same size as a pointer in the
  // DTransType system.
  DTransType *DTransPtrSizedIntType;

  // Representation of a pointer to an integer type that is the same size as a
  // pointer in the DTransType system.
  DTransPointerType *DTransPtrSizedIntPtrType;

  // Indicates that a pointer was seen that used the non-default address space.
  // The DTrans analysis does not take into account pointers to different
  // address spaces, so we need to detect this and disable DTrans in those
  // cases.
  bool UnsupportedAddressSpaceSeen = false;

  // Map of instructions that were identified as using element-zero of a
  // contained type to information about element-zero type.
  DenseMap<Instruction *, PtrTypeAnalyzer::ElementZeroInfo> ElementZeroPointers;
};

////////////////////////////////////////////////////////////////////////////////
//
// Implementation of PtrTypeAnalyzerInstVisitor class
//
////////////////////////////////////////////////////////////////////////////////

// This class will walk the IR, updating the type information for each
// important Value in the PtrTypeAnalyzer.
class PtrTypeAnalyzerInstVisitor
    : public InstVisitor<PtrTypeAnalyzerInstVisitor> {

  // This enum type is used to define how the level of indirection will be
  // modified when propagating type aliases from one ValueTypeInfo set to
  // another.
  enum class DerefType {
    DT_PointeeType,  /* Get the type of object pointed to (i.e. Decrease
                       indirection by 1 level) */
    DT_SameType,     /* Keep the same level of indirection */
    DT_PointerToType /* Create a pointer to the type (i.e. Increase indirection
                        by 1 level) */
  };

  // Enum to describe result of byte-flattened GEP analysis.
  enum BFG_Kind {
    BFG_None,    // Not recognized as a byte flattened GEP to an aggregate type.
    BFG_Unknown, // Could be a byte flattened GEP, but index not matched a known
                 // element.
    BFG_Identified // Identified as a byte flattened GEP to member of an
                   // aggregate type.
  };

public:
  PtrTypeAnalyzerInstVisitor(
      PtrTypeAnalyzerImpl &PTA, DTransTypeManager &TM,
      TypeMetadataReader &MDReader, DTransLibraryInfo &DTransLibInfo,
      DTransAllocCollector &DTAC, LLVMContext &Ctx, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI)
      : PTA(PTA), TM(TM), MDReader(MDReader), DTransLibInfo(DTransLibInfo),
        DTAC(DTAC), DL(DL), GetTLI(GetTLI) {

    // The landingpad instruction will always result in the literal structure:
    //   { i8*, i32 }
    // Build the type that will be used when analyzing the instruction.
    DTransType *FieldTypes[2] = {
        PTA.getDTransI8PtrType(),
        TM.getOrCreateAtomicType(llvm::Type::getInt32Ty(Ctx)) };
    DTransLandingPadTy = TM.getOrCreateLiteralStructType(Ctx, FieldTypes);
  }

  void visitModule(Module &M) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    // Reset the state of the trace filter to the default value in case another
    // call to visitModule is started.
    FNFilter.reset();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    // Get the type for all the Function definitions.
    for (auto &F : M) {
      DTransType *DType = MDReader.getDTransTypeFromMD(&F);
      if (DType) {
        PTA.setDeclaredType(&F, TM.getOrCreatePointerType(DType));
        continue;
      }

      llvm::Type *FnType = F.getValueType();
      if (TM.isSimpleType(FnType)) {
        DTransType *DType = TM.getOrCreateSimpleType(FnType);
        assert(DType && "Expected simple type");
        PTA.setDeclaredType(&F, TM.getOrCreatePointerType(DType));
        continue;
      }

      // Check whether this is a standard library function that we have type
      // information for. This commonly occurs when a function which was not
      // present when the front-end generated metadata gets inserted from a
      // transformation pass, such as when replacing a 'printf' call with a
      // 'puts' call.
      DTransFunctionType *DFnTy = DTransLibInfo.getDTransFunctionType(&F);
      if (DFnTy) {
        PTA.setDeclaredType(&F, TM.getOrCreatePointerType(DFnTy));
        continue;
      }

      ValueTypeInfo *Info = PTA.getOrCreateValueTypeInfo(&F);

      // We can skip marking the Function type info as 'unhandled' when
      // encountering PAROPT created functions that don't have metadata, because
      // they are not directly called.
      if (isPAROPTCreatedFunction(F))
        continue;

      Info->setUnhandled();
      LLVM_DEBUG(dbgs() << "Unable to set declared type for function: "
                        << F.getName() << "\n");
    }

    // Get the type of all the global variable pointers that were annotated with
    // metadata. Note, GlobalVariables are pointers to a value type. The type
    // recovered is the variable's value type, so we need to make a pointer to
    // it when saving the pointer info.
    for (auto &GV : M.globals()) {
      DTransType *DType = MDReader.getDTransTypeFromMD(&GV);
      if (DType) {
        PTA.setDeclaredType(&GV, TM.getOrCreatePointerType(DType));
        continue;
      }

      // Variable types that don't involve pointer types can be directly
      // reconstructed into a DTransType.
      llvm::Type *ValTy = GV.getValueType();
      if (TM.isSimpleType(ValTy)) {
        DTransType *Ty = TM.getOrCreateSimpleType(ValTy);
        assert(Ty && "Expected simple type");
        PTA.setDeclaredType(&GV, TM.getOrCreatePointerType(Ty));
        continue;
      }

      ValueTypeInfo *Info = PTA.getOrCreateValueTypeInfo(&GV);
      if (GV.isDeclaration() && handleLibraryGlobal(GV, Info))
        continue;

      Info->setUnhandled();
      LLVM_DEBUG(dbgs() << "Unable to set declared type for global variable: "
                        << GV.getName() << "\n");
    }

    // Now that types have been set up for the functions and globals, process
    // the uses of them within constant expressions.
    for (auto &F : M)
      for (auto *U : F.users())
        if (auto *CE = dyn_cast<ConstantExpr>(U))
          analyzeConstantExpr(CE);

    for (auto &GV : M.globals())
      for (auto *U : GV.users())
        if (auto *CE = dyn_cast<ConstantExpr>(U))
          analyzeConstantExpr(CE);
  }

  // For certain library global variables that are of known types, set them
  // here.
  bool handleLibraryGlobal(GlobalVariable &GV, ValueTypeInfo *ResultInfo) {
    // TODO: In the future, we may need metadata from the FE about external
    // declarations. Once there are opaque pointers, we do not expect
    // to see a structure type defined for struct._IO_FILE, and these will
    // just be defined as being of type 'ptr'
    DTransType *DTy = StringSwitch<DTransType *>(GV.getName())
                          .Case("stdout", DTransLibInfo.getDTransIOPtrType())
                          .Case("stderr", DTransLibInfo.getDTransIOPtrType())
                          .Default(nullptr);

    if (DTy) {
      // Global variables are pointers to the object type.
      ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl,
                               TM.getOrCreatePointerType(DTy));
      ResultInfo->setCompletelyAnalyzed();
      return true;
    }

    return false;
  }

  void visitFunction(Function &F) {
    LLVM_DEBUG(dbgs() << "visitFunction: " << F.getName() << "\n");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    // Check and enable for verbose filter predicate, if necessary.
    if (!DTransPTAFilterPrintFuncs.empty()) {
      FNFilter.setEnabled(false);
      for (auto &N : DTransPTAFilterPrintFuncs)
        if (F.getName() == N) {
          FNFilter.setEnabled(true);
          break;
        }
    }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    if (!F.isDeclaration()) {
      IdentifyPartialPointerOperations(F);

      for (auto &A : F.args())
        if (dtrans::hasPointerType(A.getType()))
          analyzeValue(&A);
    }
  }

  // There is a very specific pattern that we need to be able to identify
  // where we have pointer-to-pointer values and the pointers being pointed
  // to are swapped by copying partial chunks of the pointer (either i8 or i32)
  // We don't want to track the loads, stores or any associated bitcasts as
  // potential safety violations in this case.
  //
  // The pattern we want to match looks like this for i32 swaps:
  //
  //   Block1:
  //     %Cast1 = bitcast i8* %PtrToPtr to i32*
  //     %Cast2 = bitcast i8* %OtherPtrToPtr to i32*
  //     br label %Block2
  //
  //   Block2:
  //     %Count = phi i64 [ 2, %Block1 ], [ %NextCount, %Block2 ]
  //     %HalfPtr1 = phi i32* [ %Cast1, %Block1 ], [ %NextHalf1, %Block2 ]
  //     %HalfPtr2 = phi i32* [ %Cast2, %Block1 ], [ %NextHalf2, %Block2 ]
  //     %HalfVal1 = load i32, i32* %HalfPtr1
  //     %HalfVal2 = load i32, i32* %HalfPtr2
  //     %NextHalf1 = getelementptr inbounds i32, i32* %HalfPtr1, i64 1
  //     store i32 %HalfVal2, i32* %HalfPtr1
  //     %NextHalf2 = getelementptr inbounds i32, i32* %HalfPtr2, i64 1
  //     store i32 %HalfVal1, i32* %HalfPtr2
  //     %NextCount = add nsw i64 %Count, -1
  //     %Cmp = icmp sgt i64 %Count, 1
  //     br i1 %Cmp, label %Block2, label %ExitBlock
  //
  //
  // There are a couple of forms of the comparison instruction supported for
  // this pattern. In the form shown above, the current loop iteration %Count
  // value is being compared, but a form that compares the %NextCount variable
  // as greater than 0 is also supported. In both cases, we can also support an
  // unsigned greater than comparison type.
  //
  // For i8 swaps, it looks similar without the bitcasts in Block1 and with
  // a count constant of 8 rather than 2.
  //
  // (Note: Sometimes we can't identify that the incoming count value for the
  //  32-bit case is not constant.)
  //
  // Even though the treatment of the two values is symmetric, we need to
  // check them both together because we need to be sure that the partial-values
  // are being written to adjacent memory locations.
  //
  // Here we attempt to match the pattern starting with one of the load
  // instructions. For the pattern to match, the following conditions must be
  // met:
  //
  //   1. The pointer operand of the load must be a PHI node with two incoming
  //      values.
  //   2. One of the incoming values must be from the block containing the
  //      PHI, which loops back on itself.
  //   3. The incoming value must be a GEP which increments the PHI pointer.
  //   4. The PHI node must have three users, a load, a store, and a GEP.
  //   5. The store must be storing a value loaded from the "partner PHI"
  //   5. The load must have a single user, a store in the same block.
  //   6. The load must be stored to the "partner PHI"
  //   7. The GEP must be the other incoming value to the PHI.
  //   8. The block containing this code must loop back on itself based on
  //      a count value which is decremented each time the block executes.
  void IdentifyPartialPointerOperations(Function &F) {

    // Check that \p V is a PHI, which takes an input of itself, incremented by
    // one element via a GEP which is within \p LoopBB.
    //   bb1053:
    //     %i1055 = phi ptr [ %i1039, %bb1050 ], [ %i1059, %bb1053 ]
    //     ...
    //     %i1059 = getelementptr i32, ptr %1055, i64 1
    //
    auto verifyPHI = [](Value *V, BasicBlock *LoopBB) {
      auto *PN = dyn_cast<PHINode>(V);
      if (!PN)
        return false;

      // The PHI must have two incoming values. We know one is the
      // value we're here to check. We'll check the other below.
      if (PN->getNumIncomingValues() != 2)
        return false;

      // The block containing the PHI must loop back on itself and the incoming
      // value from that block must be a GEP that increments the PHI pointer.
      Value *SelfInVal = nullptr;
      if (PN->getIncomingBlock(0) == LoopBB)
        SelfInVal = PN->getIncomingValue(0);
      else if (PN->getIncomingBlock(1) == LoopBB)
        SelfInVal = PN->getIncomingValue(1);
      if (!SelfInVal)
        return false;
      auto *GEP = dyn_cast<GetElementPtrInst>(SelfInVal);
      if (!GEP || GEP->getNumIndices() != 1 || !GEP->hasAllConstantIndices())
        return false;
      auto *Idx = dyn_cast<ConstantInt>(*GEP->idx_begin());
      if (!Idx || !Idx->isOne())
        return false;

      return true;
    };

    // Verify that the PHI node is used in a block that loops back on itself
    // based on a counter value.
    auto verifyBlockIsLoop = [](PHINode *PN) {
      auto *LoopBB = PN->getParent();
      auto *Branch = dyn_cast<BranchInst>(LoopBB->getTerminator());
      if (!Branch || !Branch->isConditional()) {
        DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                          dbgs() << "Not matched. No conditional branch!\n");
        return false;
      }
      // This could be much more general, but it meets our current needs.
      auto *Condition = dyn_cast<ICmpInst>(Branch->getCondition());
      if (!Condition) {
        DEBUG_WITH_TYPE_P(
            FNFilter, DTRANS_PARTIALPTR,
            dbgs() << "Not matched. Branch condition is not icmp!\n");
        return false;
      }
      ICmpInst::Predicate Pred;
      Instruction *Base;
      const APInt *Count;
      // The condition should be a comparison based on a PHI node.
      if (!match(Condition,
                 m_ICmp(Pred, m_Instruction(Base), m_APInt(Count)))) {
        DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                          dbgs()
                              << "Not matched. icmp not using constant int!\n");
        return false;
      }
      if (Pred != CmpInst::Predicate::ICMP_SGT &&
          Pred != CmpInst::Predicate::ICMP_UGT) {
        DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                          dbgs() << "Not matched. icmp predicate isn't sgt/ugt!\n");
        return false;
      }

      // The loop count can be represented in various ways. The two handled here
      // are:
      //
      //   %Count = phi i64 [ %InitCount, %Block1 ], [ %NextCount, %Block2 ]
      //   ...
      //   %NextCount = add nsw i64 %Count, -1
      //   %Cmp = icmp sgt i64 %Count, 1
      //
      // and
      //
      //   %Count = phi i64 [ %InitCount, %Block1 ], [ %NextCount, %Block2 ]
      //   ...
      //   %NextCount = add nsw i64 %Count, -1
      //   %Cmp = icmp sgt i64 %NextCount, 0
      if (Count->isOneValue()) {
        auto *BasePHI = dyn_cast<PHINode>(Base);
        if (!BasePHI || BasePHI->getParent() != LoopBB) {
          DEBUG_WITH_TYPE_P(
              FNFilter, DTRANS_PARTIALPTR,
              dbgs() << "Not matched. Branch condition isn't PHI!\n");
          return false;
        }
        // The incoming value from the loop block must be a decrement of the
        // count PHI.
        Value *LoopInVal;
        if (BasePHI->getIncomingBlock(0) == LoopBB)
          LoopInVal = BasePHI->getIncomingValue(0);
        else
          LoopInVal = BasePHI->getIncomingValue(1);
        unsigned Bitwidth = LoopInVal->getType()->getScalarSizeInBits();
        if (!(match(LoopInVal, m_Add(m_Specific(BasePHI),
                                     m_SpecificInt(APInt(Bitwidth, -1)))) ||
              match(LoopInVal, m_Add(m_SpecificInt(APInt(Bitwidth, -1)),
                                     m_Specific(BasePHI))))) {
          DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                            dbgs()
                                << "Not matched. PHI decrement not matched!\n");
          return false;
        }
      } else {
        if (!Count->isNullValue()) {
          DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                            dbgs() << "Not matched. icmp not using 0 or 1!\n");
          return false;
        }

        // If the comparison is against zero, we expect the value being
        // compared to be a decrement of the PHI value.
        Instruction *DecBase;
        if (!(match(Base, m_Add(m_Instruction(DecBase), m_SpecificInt(-1))) ||
              match(Base, m_Add(m_SpecificInt(-1), m_Instruction(DecBase))))) {
          DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                            dbgs()
                                << "Not matched. PHI decrement not matched!\n");
          return false;
        }
        auto *BasePHI = dyn_cast<PHINode>(DecBase);
        if (!BasePHI || BasePHI->getParent() != LoopBB) {
          DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                            dbgs()
                                << "Not matched. Decrement input isn't PHI!\n");
          return false;
        }
        // The incoming value from the loop block must be the decrement result.
        Value *LoopInVal;
        if (BasePHI->getIncomingBlock(0) == LoopBB)
          LoopInVal = BasePHI->getIncomingValue(0);
        else
          LoopInVal = BasePHI->getIncomingValue(1);
        if (LoopInVal != Base) {
          DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                            dbgs()
                                << "Not matched. PHI decrement not matched!\n");
          return false;
        }
      }

      return true;
    };

    auto matchPHIUsers = [](PHINode *PN, LoadInst *&LoadUser,
                            StoreInst *&StoreUser,
                            GetElementPtrInst *&GEPUser) {
      // The PHI must have three users.
      if (!PN->hasNUses(3)) {
        DEBUG_WITH_TYPE_P(
            FNFilter, DTRANS_PARTIALPTR,
            dbgs() << "Not matched. PHI doesn't have three users!\n");
        return false;
      }

      LoadUser = nullptr;
      StoreUser = nullptr;
      GEPUser = nullptr;
      for (auto *U : PN->users()) {
        if (!LoadUser)
          LoadUser = dyn_cast<LoadInst>(U);
        if (!StoreUser)
          StoreUser = dyn_cast<StoreInst>(U);
        if (!GEPUser)
          GEPUser = dyn_cast<GetElementPtrInst>(U);
      }
      if (!LoadUser || !StoreUser || !GEPUser) {
        DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                          dbgs() << "Not matched. PHI users don't match!\n");
        return false;
      }

      // The GEP must have a single use which is the PHI.
      if (!GEPUser->hasOneUse() || (*GEPUser->user_begin() != PN)) {
        DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                          dbgs() << "Not matched. "
                                 << "GEP isn't uniquely used by PHI!\n");
        return false;
      }

      // The load user must have a single use. We'll check that elsewhere.
      if (!LoadUser->hasOneUse()) {
        DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                          dbgs() << "Not matched. "
                                 << "Secondary load isn't single use!\n");
        return false;
      }

      // The phi must be the target of the store, not the value stored.
      if (StoreUser->getPointerOperand() != PN) {
        DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                          dbgs() << "Not matched. "
                                 << "Store doesn't write to PHI pointer!\n");
        return false;
      }

      return true;
    };

    auto verifyLoadUsage = [](LoadInst *Load, Value *ExpectedDest) {
      // We've already verified that the load user is only used once.
      // That use must be a store instruction
      auto *Store = dyn_cast<StoreInst>(*Load->user_begin());
      if (!Store) {
        DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                          dbgs() << "Not matched. "
                                 << "Loaded value isn't used by store!\n");
        return false;
      }

      // The loaded value must be the stored value, not the destination
      // of the store.
      if (Store->getValueOperand() != Load) {
        DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                          dbgs()
                              << "Not matched. Loaded value isn't stored!\n");
        return false;
      }

      // Check that the destination of the store is what we expect.
      if (Store->getPointerOperand() != ExpectedDest) {
        DEBUG_WITH_TYPE_P(
            FNFilter, DTRANS_PARTIALPTR,
            dbgs() << "Not matched. Unexpected store destination!\n");
        return false;
      }

      return true;
    };

    auto SetPartialPointerUse = [this](ArrayRef<Value *> Values) {
      for (auto *V : Values) {
        ValueTypeInfo *Info = PTA.getOrCreateValueTypeInfo(V);
        Info->setIsPartialPointerUse();
      }
    };

    // When the idiom is matched there are two load instructions associated with
    // the operation. Keep a set of the matches to avoid the need to evaluate
    // the pattern on the paired load.
    SmallPtrSet<Value *, 16> MatchingLoads;
    for (auto &I : instructions(F)) {
      if (auto *LI = dyn_cast<LoadInst>(&I)) {
        Type *ValTy = LI->getType();
        if (!(ValTy->isIntegerTy(8) || ValTy->isIntegerTy(32)))
          continue;

        if (MatchingLoads.count(LI))
          continue;

        // If we're not loading from a PHI node pointer, then it is not the
        // supported pattern.
        auto *PrimaryPHI = dyn_cast<PHINode>(LI->getPointerOperand());
        if (!PrimaryPHI)
          continue;

        auto *LoopBB = PrimaryPHI->getParent();
        if (!verifyPHI(PrimaryPHI, LoopBB))
          continue;

        DEBUG_WITH_TYPE_P(
            FNFilter, DTRANS_PARTIALPTR,
            dbgs() << "Checking potential partial pointer use idiom: ["
                   << F.getName() << "]" << *LI << "\n");

        // Make sure the block loops back on itself.
        if (!verifyBlockIsLoop(PrimaryPHI))
          continue;

        // Try to match the PHI users as a load, a store, and a GEP.
        LoadInst *LoadUser;
        StoreInst *StoreUser;
        GetElementPtrInst *GEPUser;
        if (!matchPHIUsers(PrimaryPHI, LoadUser, StoreUser, GEPUser))
          continue;

        // The value stored should trace back to our partner value as such:
        //   PartnerVal = bitcast
        //   PartnerPHI = phi [PartnerVal....
        //   StoredVal = load i32, i32* PartnerPHI
        auto *ValStored = StoreUser->getValueOperand();
        auto *PartnerLoad = dyn_cast<LoadInst>(ValStored);
        if (!PartnerLoad) {
          DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                            dbgs()
                                << "Not matched. Can't find partner load!\n");
          continue;
        }
        auto *PartnerPHI = dyn_cast<PHINode>(PartnerLoad->getPointerOperand());
        if (!PartnerPHI || PartnerPHI->getParent() != PrimaryPHI->getParent()) {
          DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                            dbgs() << "Not matched. Can't find partner PHI!\n");
          continue;
        }

        if (!verifyPHI(PartnerPHI, LoopBB)) {
          DEBUG_WITH_TYPE_P(
              FNFilter, DTRANS_PARTIALPTR,
              dbgs() << "Not matched. Partner PHI does match idiom!\n");
          continue;
        }

        // Check that the value loaded from the PHI pointer is stored in the
        // same place that the partner load was loaded from.
        if (!verifyLoadUsage(LoadUser, PartnerPHI))
          continue;

        StoreInst *PartnerStore;
        GetElementPtrInst *PartnerGEP;
        if (!matchPHIUsers(PartnerPHI, PartnerLoad, PartnerStore, PartnerGEP))
          continue;

        DEBUG_WITH_TYPE_P(FNFilter, DTRANS_PARTIALPTR,
                          dbgs() << "Partial pointer use idiom matched: "
                                 << "[" << F.getName() << "]:\n"
                                 << "  Load #1: " << *LI << "\n"
                                 << "  Load #2: " << *PartnerLoad << "\n"
                                 << "  GEP #1 : " << *GEPUser << "\n"
                                 << "  GEP #2 : " << *PartnerGEP << "\n");

        MatchingLoads.insert(LI);
        MatchingLoads.insert(PartnerLoad);
        SetPartialPointerUse({PrimaryPHI, PartnerPHI, GEPUser, PartnerGEP});
      }
    }
  }

  void visitAllocaInst(AllocaInst &I) { analyzeValue(&I); }
  void visitBinaryOperator(BinaryOperator &I) {
    // To support safety analysis on pointer arithmetic, subtract instruction
    // need to be analyzed to see if they carry pointer information.
    if (I.getOpcode() == Instruction::Sub)
      analyzeValue(&I);
  }
  void visitBitCastInst(BitCastInst &I) { analyzeValue(&I); }
  void visitCallBase(CallBase &I) { analyzeValue(&I); }
  void visitExtractValueInst(ExtractValueInst &I) { analyzeValue(&I); }
  void visitFreezeInst(FreezeInst& I) { analyzeValue(&I); }
  void visitGetElementPtrInst(GetElementPtrInst &I) { analyzeValue(&I); }
  void visitIntToPtrInst(IntToPtrInst &I) {
    ValueTypeInfo *ResultInfo = analyzeValue(&I);

    // When the IntToPtrInst is analyzed, the type may come from information
    // about a pointer type that was associated with the integer operand, or it
    // may need to be inferred based on the users of the result. If no types are
    // resolved, then we need to set it as unhandled because there is no
    // information about the pointer type.
    if (ResultInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use).empty())
      ResultInfo->setUnhandled();
  }
  void visitLandingPadInst(LandingPadInst &I) { analyzeValue(&I); }
  void visitLoadInst(LoadInst &I) { analyzeValue(&I); }
  void visitPHINode(PHINode &I) { analyzeValue(&I); }
  void visitPtrToIntInst(PtrToIntInst &I) { analyzeValue(&I); }
  void visitSelectInst(SelectInst &I) { analyzeValue(&I); }

  void visitStoreInst(StoreInst &I) {
    Value *Ptr = I.getPointerOperand();

    if (isCompilerConstant(Ptr)) {
      // If there is something of the form:
      //   store ptr poison, ptr null
      //
      // Create a dummy ValueTypeInfo for the pointer operand so that an object
      // will be available for the DTransSafetyAnalyzer. There is no need to
      // track types for the pointer operand.
      ValueTypeInfo *PtrInfo = PTA.getOrCreateValueTypeInfo(&I, 1);
      PtrInfo->setCompletelyAnalyzed();
    } else {
      // Analyze the pointer operand to ensure that there is a ValueTypeInfo
      // object available for it for the DTransSafetyAnalyzer. This is needed to
      // handle cases where the operand is not a local or global variable. For
      // example:
      //   store i32 0, ptrtoint (i64 256 to i32*)
      ValueTypeInfo *PtrInfo = analyzeValue(Ptr);
      checkForElementZeroAccess(&I, I.getValueOperand()->getType(), PtrInfo,
                                ValueTypeInfo::ValueAnalysisType::VAT_Decl);

      // If the pointer operand is used to store a scalar type, note the type
      // in the usage types for the pointer. TODO: May need to do something
      // for storing pointer type as well.
      if (!PtrInfo->getIsPartialPointerUse()) {
        llvm::Type *ValTy = I.getValueOperand()->getType();
        if (!dtrans::hasPointerType(ValTy))
          PtrInfo->addTypeAlias(
              ValueTypeInfo::VAT_Use,
              TM.getOrCreatePointerType(TM.getOrCreateSimpleType(ValTy)));
      }
    }

    // Storing a constant, such as 'null' or 'poison', is a special case. The
    // pointer may be used to represent any pointer type. For a store of a
    // constant we want to capture a type of the value operand so that
    // the safety analyzer will be able to analyze the instruction without
    // needing to examine the value being stored with a special case for
    // constants, so we will just propagate an appropriate type from what is
    // known about the pointer operand.
    Value *ValOp = I.getValueOperand();
    if (isa<ConstantPointerNull>(ValOp) || isa<PoisonValue>(ValOp)) {
      auto &LocalTM = this->TM;
      auto &LocalPTA = this->PTA;

      // Propagate the known types from the pointer-operand to ValueInfo that
      // represents a use of the constant. This may also, identify that
      // it appears that an element-zero location of an aggregate is being
      // stored, in which case the PointerInfo will be updated to reflect that
      // the pointer operand is being used as an element-pointee.
      auto PropagateDereferencedType =
          [&LocalTM, &LocalPTA](ValueTypeInfo *PointerInfo,
                                ValueTypeInfo *ValueInfo,
                                ValueTypeInfo::ValueAnalysisType Kind) {
            SmallVector<DTransType *, 4> PendingTypes;
            for (auto *Alias : PointerInfo->getPointerTypeAliasSet(Kind)) {
              DTransType *PropAlias = nullptr;
              if (!Alias->isPointerTy())
                continue;

              PropAlias = Alias->getPointerElementType();
              if (!PropAlias->isAggregateType()) {
                ValueInfo->addTypeAlias(Kind, PropAlias);
                continue;
              }

              if (auto ElemZeroPair = LocalPTA.getElementZeroType(PropAlias)) {
                PointerInfo->addElementPointee(
                    ValueTypeInfo::VAT_Use, ElemZeroPair.value().first, 0);
                ValueInfo->addTypeAlias(Kind, ElemZeroPair.value().second);

                // Need to defer updating the PointerInfo until the loop
                // completes because we cannot modify the set while we are
                // iterating it.
                PendingTypes.push_back(LocalTM.getOrCreatePointerType(
                    ElemZeroPair.value().second));
              } else {
                ValueInfo->setUnhandled();
              }
            }

            for (auto *Ty : PendingTypes)
              PointerInfo->addTypeAlias(ValueTypeInfo::VAT_Use, Ty);
          };

      ValueTypeInfo *ValInfo = PTA.getOrCreateValueTypeInfo(&I, 0);
      ValueTypeInfo *PtrInfo = PTA.getValueTypeInfo(&I, 1);
      PropagateDereferencedType(PtrInfo, ValInfo,
                                ValueTypeInfo::ValueAnalysisType::VAT_Decl);
      PropagateDereferencedType(PtrInfo, ValInfo,
                                ValueTypeInfo::ValueAnalysisType::VAT_Use);
      ValInfo->setCompletelyAnalyzed();
    }
  }

  void visitReturnInst(ReturnInst &I) {
    // No additional analysis needs to be done, since the only time a
    // ReturnInst produces a new type is when a type is cast, which will
    // have been examined when inferring the type cast.
    //
    // TODO: When pointers are fully opaque, it may be necessary to
    // perform inference here based on the expected return type.
    return;
  }

  void visitResumeInst(ResumeInst &I) {
    // No type information needs to be collected for ResumeInst. Included here
    // so that visitInstruction does not mark operands as unhandled.
    return;
  }

  void visitCmpInst(CmpInst &I) {
    // No type information needs to be collected for CmpInst. Included here so
    // that visitInstruction does not mark operands as unhandled.
    return;
  }

  // All instructions not handled by other visit functions.
  void visitInstruction(Instruction &I) {
    if (dtrans::hasPointerType(I.getType())) {
      ValueTypeInfo *Info = PTA.getOrCreateValueTypeInfo(&I);
      Info->setUnhandled();
      LLVM_DEBUG(dbgs() << "Unhandled instruction: " << I << "\n");
    }

    for (unsigned OpNum = 0; OpNum < I.getNumOperands(); ++OpNum) {
      Value *Op = I.getOperand(OpNum);
      if (dtrans::hasPointerType(Op->getType())) {
        ValueTypeInfo *Info = PTA.getOrCreateValueTypeInfo(&I, OpNum);
        Info->setUnhandled();
        LLVM_DEBUG(dbgs() << "Operand used in unhandled instruction: " << *Op
                          << "\n"
                          << "User: " << I << "\n");
      }
    }
  }

private:
  // For the dependencies that are to be processed when a value is analyzed,
  // there are two ways the dependency may be processed. In some cases the type
  // can be determined based on the instruction and the operands of the
  // instruction. In other cases, the type is inferred from examining the users
  // of it. This enumeration type is used to determine which path should be used
  // when processing a value taken from the dependency stack.
  enum DepKind { DK_ANALYZE, DK_INFER_FROM_USE };

  // The dependency stack is composed of the Value to be processed and the type
  // of processing to be done on it. For Values that use the infer path, the
  // Value that is trying to be inferred is also kept on the stack. These
  // constants define the indices in the DependencyInfo tuple for accessing the
  // specific components.
  static constexpr unsigned ValueIdx = 0;
  static constexpr unsigned InferValueIdx = 1;
  static constexpr unsigned DepKindIdx = 2;
  using DependencyInfo = std::tuple<Value *, Value *, DepKind>;

  // Type for the stack of dependencies.
  using DependencyStack = SmallVector<DependencyInfo, 16>;
  using DependencyStackImpl = SmallVectorImpl<DependencyInfo>;

  // Cache of load/store instructions that have been analyzed as loading/storing
  // a pointer to itself in the pointer location. This is used during the type
  // inference process because we try to take the types known for a value and
  // treat the pointer operand as a pointer to those types. When a self
  // load/store occurs, it can lead to many additional levels of pointer types
  // due to trying to infer the pointer operand as being a pointer to the type
  // of the value operand.
  //
  //  For example, here the pointer location %141 can be the same value as
  //  %170 which is being stored.
  //
  //   %141 = phi ptr [ %175, %183 ], ...
  //   %170 = ptr @someCall()
  //   store ptr %170, ptr %141
  //   %175 = phi ptr [ %170, %169 ], ...
  //
  // This cache is kept to avoid analyzing a load/store multiple times.
  DenseMap<Instruction *, bool> SelfLoadStoreCache;

  ValueTypeInfo *analyzeValue(Value *V) {
    ValueTypeInfo *Info = PTA.getOrCreateValueTypeInfo(V);
    if (Info->isCompletelyAnalyzed())
      return Info;

    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
      dbgs() << "--\n";
      dbgs() << "Begin analyzeValue for: ";
      printValue(dbgs(), V);
      dbgs() << "\n";
    });

    if (auto *PtrTy = dyn_cast<PointerType>(V->getType())) {
      if (PtrTy->getAddressSpace() != 0) {
        LLVM_DEBUG({
          if (!PTA.getUnsupportedAddressSpaceSeen())
            dbgs() << "Unsupported address space seen: " << *PtrTy << "\n";
        });
        PTA.setUnsupportedAddressSpaceSeen();
      }
    }


    // Check for the special DTrans type metadata, !dtrans-type, used to
    // communicate type information between DTrans passes because of IR
    // produced by a DTrans transformation, that is not easily recoverable from
    // IR analysis. For example, the AOS-to-SOA transformation allocates a large
    // block of memory which is subdivided to hold arrays of different types.
    // TODO: Try to unify this metadata with the other DTrans type metadata.
    // Currently, this is not done because it appears on instructions that are
    // not expected to have the other DTrans type metadata that is produced by
    // the front-end, so will not be processed by the TypeMetadataReader.
    if (auto *I = dyn_cast<Instruction>(V))
      if (auto TyFromMD =
              dtrans::DTransAnnotator::lookupDTransTypeAnnotation(*I)) {
        llvm::Type *Ty = TyFromMD.value().first;
        unsigned Level = TyFromMD.value().second;

        // The format of the metadata used restricts the type to being just a
        // pointer to a scalar or a structure type. This assertion is to catch
        // unsupported uses.
        assert(TM.isSimpleType(Ty) && "Expected simple type");
        DTransType *DTy = TM.getOrCreateSimpleType(Ty);
        assert(DTy && "Failed to create type");
        while (Level--)
          DTy = TM.getOrCreatePointerType(DTy);

        Info->addTypeAlias(ValueTypeInfo::VAT_Use, DTy);
      }

    // Build a stack of unresolved dependent values that must be analyzed
    // before we can complete the analysis of this value.
    DependencyStack DependentVals;
    if (addDependency(V, DependentVals))
      populateDependencyStack(V, DependentVals);

    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE,
                      dumpDependencyStack(V, DependentVals));

    while (!DependentVals.empty()) {
      auto DepInfo = DependentVals.pop_back_val();
      Value *Dep = std::get<ValueIdx>(DepInfo);
      if (std::get<DepKindIdx>(DepInfo) == DK_ANALYZE) {
        // A compiler constant, such as undef or null, cannot help supply the
        // information required to analyze a use because the actual type for
        // these depends on the context in which it is used.
        //
        // For example, the type of %x in the following cannot be resolved based
        // on the 'null' it is used with:
        //   store ptr null, ptr %x
        //   icmp eq ptr null, ptr %x
        //   %x = phi ptr [null, %l1]
        if (isCompilerConstant(Dep))
          continue;

        ValueTypeInfo *DepInfo = PTA.getOrCreateValueTypeInfo(Dep);
        DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_DEP_TRACE, {
          dbgs() << "  Dependent: ";
          printValue(dbgs(), Dep);
          dbgs() << "\n";
          DepInfo->print(dbgs(), /*Combined=*/false, "    ");
        });

        // If we have complete results for this value, don't repeat the
        // analysis.
        if (DepInfo->isCompletelyAnalyzed()) {
          DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_DEP_TRACE, {
            dbgs() << "  Already analyzed: ";
            printValue(dbgs(), Dep);
            dbgs() << "\n";
          });
          continue;
        }
        analyzeValueImpl(Dep, DepInfo);
      } else {
        inferValueImpl(std::get<InferValueIdx>(DepInfo), Dep);
      }
    }

    Info->setCompletelyAnalyzed();
    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
      dbgs() << "End analyzeValue for: ";
      printValue(dbgs(), V);
      dbgs() << "\n";
    });

    return Info;
  }

  void analyzeValueImpl(Value *V, ValueTypeInfo *Info) {
    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
      dbgs() << "  Analyzing: ";
      printValue(dbgs(), V);
      dbgs() << "\n";
    });

    auto *I = dyn_cast<Instruction>(V);
    if (!I) {
      if (auto *Arg = dyn_cast<Argument>(V)) {
        analyzeArgument(Arg, Info);
        return;
      }

      if (auto *GEP = dyn_cast<GEPOperator>(V)) {
        analyzeGetElementPtrOperator(GEP, Info);
        return;
      }

      if (auto *BC = dyn_cast<BitCastOperator>(V)) {
        analyzeBitCastOperator(BC, Info);
        return;
      }

      if (auto *CE = dyn_cast<ConstantExpr>(V))
        if (CE->isCast() && isCompilerConstant(CE->getOperand(0))) {
          // We can ignore a constant converted to a pointer because the
          // conversion is not taking place on something that was previously
          // identified as possibly being a pointer to a structure type of
          // interest. For example:
          //   inttoptr i64 128 to i32*
          return;
        }

      Info->setUnhandled();
      LLVM_DEBUG({
        dbgs() << "analyzeValueImpl not implemented for:";
        printValue(dbgs(), V);
        dbgs() << "\n";
      });

      return;
    }

    switch (I->getOpcode()) {
    default:
      if (I->getType()->isPointerTy()) {
        Info->setUnhandled();
        LLVM_DEBUG({
          dbgs() << "analyzeValueImpl not implemented for:";
          printValue(dbgs(), V);
          dbgs() << "\n";
        });
      }
      break;
    case Instruction::Alloca:
      analyzeAllocaInst(cast<AllocaInst>(I), Info);
      break;
    case Instruction::BitCast:
      analyzeBitCastOperator(cast<BitCastOperator>(I), Info);
      break;
    case Instruction::Call:
    case Instruction::Invoke:
      analyzeCallBase(cast<CallBase>(I), Info);
      break;
    case Instruction::ExtractValue:
      analyzeExtractValueInst(cast<ExtractValueInst>(I), Info);
      break;
    case Instruction::Freeze:
      analyzeFreezeInst(cast<FreezeInst>(I), Info);
      break;
    case Instruction::GetElementPtr:
      analyzeGetElementPtrOperator(cast<GEPOperator>(I), Info);
      break;
    case Instruction::IntToPtr:
      analyzeIntToPtrInst(cast<IntToPtrInst>(I), Info);
      break;
    case Instruction::LandingPad:
      analyzeLandingPadInst(cast<LandingPadInst>(I), Info);
      break;
    case Instruction::Load:
      analyzeLoadInst(cast<LoadInst>(I), Info);
      break;
    case Instruction::PHI:
      analyzePHINode(cast<PHINode>(I), Info);
      break;
    case Instruction::PtrToInt:
      analyzePtrToIntOperator(cast<PtrToIntOperator>(I), Info);
      break;
    case Instruction::Select:
      analyzeSelectInst(cast<SelectInst>(I), Info);
      break;
    case Instruction::Sub:
      analyzeSubInst(cast<BinaryOperator>(I), Info);
      break;
      // TODO: Add other instructions analysis calls.
    }
  }

  void inferValueImpl(Value *ValueToInfer, Value *User) {
    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
      dbgs() << "  Inferring: ";
      printValue(dbgs(), User);
      dbgs() << "  to try to resolve types for: ";
      printValue(dbgs(), ValueToInfer);
      dbgs() << "\n";
    });

    // The partial pointer use pattern is a specific pattern that uses pointers
    // in sets to access an element. Don't infer these because they have a
    // special analysis and we don't want the types used to access the partial
    // elements.
    ValueTypeInfo *PtrInfo = PTA.getOrCreateValueTypeInfo(ValueToInfer);
    if (PtrInfo->getIsPartialPointerUse())
      return;

    if (isa<BitCastInst>(User)) {
      propagateInferenceSet(User, ValueToInfer, DerefType::DT_SameType);
    } else if (auto *Sel = dyn_cast<SelectInst>(User)) {
      if (!isa<Constant>(Sel->getTrueValue()))
        propagateInferenceSet(User, Sel->getTrueValue(),
                              DerefType::DT_SameType);
      if (!isa<Constant>(Sel->getFalseValue()))
        propagateInferenceSet(User, Sel->getFalseValue(),
                              DerefType::DT_SameType);
    } else if (auto *PHI = dyn_cast<PHINode>(User)) {
      propagateInferenceSet(User, ValueToInfer, DerefType::DT_SameType);
      for (Value *InV : PHI->incoming_values())
        if (!isa<Constant>(InV))
          propagateInferenceSet(User, InV, DerefType::DT_SameType);
    } else if (auto *LI = dyn_cast<LoadInst>(User)) {
      inferLoadInst(ValueToInfer, LI);
    } else if (auto *SI = dyn_cast<StoreInst>(User)) {
      inferStoreInst(ValueToInfer, SI);
    } else if (auto *GEPOp = dyn_cast<GEPOperator>(User)) {
      inferGetElementPtr(ValueToInfer, GEPOp);
    } else if (auto *Ret = dyn_cast<ReturnInst>(User)) {
      inferRetInst(ValueToInfer, Ret);
    } else if (auto *Call = dyn_cast<CallBase>(User)) {
      inferCall(ValueToInfer, Call);
    } else if (auto *ICmp = dyn_cast<ICmpInst>(User)) {
      inferICmpInst(ValueToInfer, ICmp);
    } else if (auto *PTI = dyn_cast<PtrToIntInst>(User)) {
      inferPtrToIntInst(ValueToInfer, PTI);
    }

    auto It = PendingValueToInferredTypes.find(ValueToInfer);
    if (It != PendingValueToInferredTypes.end()) {
      ValueTypeInfo *ResultInfo = PTA.getOrCreateValueTypeInfo(ValueToInfer);
      for (auto Ty : It->second)
        ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Use, Ty);
    }
  }

  // Try to determine the type of the GEP pointer operand.
  void inferGetElementPtr(Value *ValueToInfer, GEPOperator *GEP) {
    // GEPs that have a simple source type being indexed into are inferred as
    // being that type.
    //   %x = getelementptr %struct.arc, ptr %ptr, i64 0, i32
    // %ptr is being used as a %struct.arc
    //
    // GEPs that are indexing with a pointer type, need to perform look-ahead
    // for the type.
    //   %x = getelementptr ptr, ptr %ptr, i64 5
    // May be able to infer the type of %ptr by looking at how %x gets used.

    // Should only need to infer for the pointer operand.
    if (GEP->getPointerOperand() != ValueToInfer)
      return;

    llvm::Type *SrcTy = GEP->getSourceElementType();
    if (TM.isSimpleType(SrcTy)) {
      DTransType *DType = TM.getOrCreateSimpleType(SrcTy);
      assert(DType && "Expected simple type");
      addInferredType(ValueToInfer, TM.getOrCreatePointerType(DType));
      return;
    }

    if (GEP->getNumIndices() != 1)
      return;

    propagateInferenceSet(GEP, ValueToInfer, DerefType::DT_SameType);
    return;
  }

  // Try to determine the type of a Value used in a call.
  // The Value may be a parameter to the call, in which case we try to resolve
  // the expected type of the argument. Otherwise, the Value may be the call
  // itself for an indirect call.
  void inferCall(Value *ValueToInfer, CallBase *Call) {
    if (Call->getCalledOperand() == ValueToInfer) {
      // Try to get the type from metadata attached to the call or the function.
      // TODO: If metadata is lost, we may try to determine the type from the
      // parameters.
      DTransType *DType = nullptr;
      Function *Target = dtrans::getCalledFunction(*Call);
      if (Call->isIndirectCall())
        DType = MDReader.getDTransTypeFromMD(Call);
      else if (Target)
        DType = MDReader.getDTransTypeFromMD(Target);

      if (DType)
        addInferredType(ValueToInfer, TM.getOrCreatePointerType(DType));

      return;
    }

    unsigned Idx = 0;
    for (auto &Arg : Call->args()) {
      if (Arg == ValueToInfer) {
        auto P = getArgumentType(Call, Idx);
        if (!P.first) {
          // The argument is not used, or not a type of interest. Just use the
          // existing known types for the parameter to avoid it being marked as
          // unhandled. TODO: see if there is a better way to determine when a
          // type cannot be inferred.
          ValueTypeInfo *ArgInfo =
              PTA.getOrCreateValueTypeInfo(Arg.getUser(), Idx);
          for (auto *DType :
               ArgInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
            if (auto *PtrTy = dyn_cast<DTransPointerType>(DType))
              addInferredType(ValueToInfer, PtrTy);
        } else if (P.second) {
          // Add the type, and continue to the next argument, rather than
          // exiting the loop, in case the ValueToInfer occurs multiple times.
          addInferredType(ValueToInfer, P.second);
        }
      }
      ++Idx;
    }
  }

  // Try to determine a type for the returned value based on the metadata
  // description of the function signature.
  void inferRetInst(Value *ValueToInfer, ReturnInst *Ret) {
    auto P =
        getFunctionReturnType(Ret->getFunction(), GetTLI(*Ret->getFunction()));
    if (P.first && P.second) {
      addInferredType(ValueToInfer, P.second);
    }
  }

  // Try to infer a type of a Value used in a comparison instruction by
  // inferring that it should be the same type as the other operand.
  void inferICmpInst(Value *ValueToInfer, ICmpInst *ICmp) {
    Value *OtherOp = ICmp->getOperand(0);
    if (OtherOp == ValueToInfer)
      OtherOp = ICmp->getOperand(1);

    // Disallow "icmp <pred> ptr %x, %x" since it should not occur, and we cannot
    // get information about one argument by examining the other.
    if (OtherOp == ValueToInfer)
      return;

    // Compiler constants do not provide information about the operand to be
    // inferred, but do not return 'false' because other uses of the operand may
    // provide the type.
    if (isCompilerConstant(OtherOp))
      return;

    ValueTypeInfo *ValInfo = PTA.getValueTypeInfo(OtherOp);
    if (ValInfo)
      for (auto *DType : ValInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
        addInferredType(ValueToInfer, DType);
  }

  // Try to infer the type of the pointer operand in a LoadInst based on either
  // the loaded type (%s = load i64, ptr %x) or by looking at the uses of the
  // loaded value (%p = load ptr, ptr %y).
  void inferLoadInst(Value *ValueToInfer, LoadInst *LI) {
    llvm::Type *LoadType = LI->getType();
    if (TM.isSimpleType(LoadType)) {
      DTransType *DType = TM.getOrCreateSimpleType(LoadType);
      assert(DType && "Expected simple type");
      addInferredType(LI->getPointerOperand(),
                      TM.getOrCreatePointerType(DType));
      return;
    }

    if (isSelfLoadStore(*LI))
      return;

    if (LI->getPointerOperand() == ValueToInfer)
      propagateInferenceSet(LI, ValueToInfer, DerefType::DT_PointerToType);
  }

  // Try to infer a type used in a StoreInst. The ValueToInfer may be either the
  // stored valued, in which case we look at the pointer operand. Or, it may be
  // the pointer operand, in which case we look at the value stored.
  void inferStoreInst(Value *ValueToInfer, StoreInst *SI) {
    // if the stored type is known, and the pointer operand is the one to be
    // inferred, then the answer is easy. e.g. store i32, ptr %y
    Value *ValOp = SI->getValueOperand();
    Value *PtrOp = SI->getPointerOperand();
    llvm::Type *StoreType = ValOp->getType();
    if (TM.isSimpleType(StoreType)) {
      DTransType *DType = TM.getOrCreateSimpleType(StoreType);
      assert(DType && "Expected simple type");
      addInferredType(ValOp, DType);
      addInferredType(PtrOp, TM.getOrCreatePointerType(DType));
      return;
    }

    // Check that the value operand is not the same as the pointer operand
    // because we are going to treat any types identified for the value operand
    // as being pointers to the type for the pointer operand, this can lead to
    // many levels of pointer indirection being tracked.
    // 
    // For example:
    //   %141 = phi ptr [ %175, %183 ], ...
    //   %170 = ptr @someCall()
    //   store ptr %170, ptr %141
    //   %175 = phi ptr [ %170, %169 ], ...
    //
    // If we are treating %141 to be a pointer to whatever type %170 is, and
    // %170 as being the pointer element type of %141, then as we accumulate
    // information, we can end up with additional levels of indirection.
    if (isSelfLoadStore(*SI))
      return;

    if (PtrOp == ValueToInfer) {
      // We cannot infer anything new from a compiler constant used to
      // represent the nullptr, so just treat the pointer operand as being the
      // declared type to avoid it being marked as unhandled from not having any
      // inferred types.
      if (isCompilerConstant(ValOp)) {
        ValueTypeInfo *PtrInfo = PTA.getValueTypeInfo(SI, 1);
        if (PtrInfo)
          for (auto *DType :
               PtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
            addInferredType(PtrOp, DType);

        return;
      }

      // Value being inferred is the pointer operand, analyze the value
      // operand to determine a type for the pointer operand
      ValueTypeInfo *ValInfo = PTA.getValueTypeInfo(SI, 0);
      if (ValInfo) {
        for (auto *DType :
             ValInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use)) {
          addInferredType(ValOp, DType);
          addInferredType(PtrOp, TM.getOrCreatePointerType(DType));
        }
      }

      return;
    }

    // Value being inferred is the value operand, analyze the pointer
    // operand to determine the type being stored.
    ValueTypeInfo *PtrInfo = PTA.getValueTypeInfo(SI, 1);
    if (PtrInfo) {
      for (auto *DType :
           PtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use)) {
        if (auto *PtrTy = dyn_cast<DTransPointerType>(DType)) {
          DTransType *InferredValTy = PtrTy->getPointerElementType();
          if (InferredValTy->isAggregateType()) {
            auto ElemZeroTy = PTA.getElementZeroType(InferredValTy);
            if (ElemZeroTy && ElemZeroTy.value().second->isPointerTy())
              InferredValTy = ElemZeroTy.value().second;
          }
          // The only time we would need to infer the value operand type is for
          // a pointer value. Only add the type inferred if is a pointer value.
          if (InferredValTy->isPointerTy()) {
            addInferredType(ValOp, InferredValTy);
            addInferredType(PtrOp, DType);
          }
        }
      }
    }
  }

  // Return 'true' if we identify that the location being stored to is the same
  // as the value being stored.
  bool isSelfLoadStore(Instruction &I) {
    // Collect PHI/Select instructions that were used to set the Value 'V'.
    std::function<void(Value *, SmallPtrSetImpl<Value *> &)>
        CollectPHIAndSelectSources =
            [&CollectPHIAndSelectSources](Value *V,
                                          SmallPtrSetImpl<Value *> &Visited) {
              if (!Visited.insert(V).second)
                return;

              if (auto *Phi = dyn_cast<PHINode>(V)) {
                for (Value *In : Phi->incoming_values())
                  CollectPHIAndSelectSources(In, Visited);
              } else if (auto *Sel = dyn_cast<SelectInst>(V)) {
                CollectPHIAndSelectSources(Sel->getTrueValue(), Visited);
                CollectPHIAndSelectSources(Sel->getFalseValue(), Visited);
              }
            };

    auto It = SelfLoadStoreCache.find(&I);
    if (It != SelfLoadStoreCache.end())
      return It->second;

    Value *ValOp =
        isa<StoreInst>(&I) ? cast<StoreInst>(&I)->getValueOperand() : &I;
    Value *PtrOp = getLoadStorePointerOperand(&I);
    assert(PtrOp && "Instruction was not load/store");

    SmallPtrSet<Value *, 16> Visited;
    CollectPHIAndSelectSources(PtrOp, Visited);

    for (auto *InV : Visited)
      if (InV == ValOp) {
        SelfLoadStoreCache[&I] = true;
        return true;
      }

    SelfLoadStoreCache[&I] = false;
    return false;
  }

  // Try to infer the type of the pointer operand of a pointer-to-int
  // instruction. This is primarily to capture the pattern where a pointer is
  // converted into a pointer-sized integer and then stored into a memory
  // location which can have its type determined by the analyzer.
  //   %as.i64 = ptrtoint i8* to i64
  //   store i64 %as.i64, i64* %struct_alias
  //
  void inferPtrToIntInst(Value *ValueToInfer, PtrToIntInst *PTI) {
    for (auto *User : PTI->users()) {
      // TODO: Currently, this is limited to just looking for users that are
      // storing the integer value. This may need to be extended in the future
      // to follow the use through 'sub' or 'div' instructions to an inttoptr
      // instruction which can have its pointer type resolved, since the only
      // time DTrans supports integer operations on the pointer are for
      // computing the distance between two pointer elements. Other uses would
      // be complicated to do because they would require following the value
      // forward through all the possible integer operations.
      if (auto *SI = dyn_cast<StoreInst>(User)) {
        Value *PtrOp = SI->getPointerOperand();
        ValueTypeInfo *PtrOpInfo = PTA.getValueTypeInfo(PtrOp);
        if (PtrOpInfo) {
          for (auto *DType :
               PtrOpInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
            if (auto *PtrTy = dyn_cast<DTransPointerType>(DType)) {
              // The original type of a PtrToInt instruction is expected to be a
              // pointer type, so the storage location should be a
              // pointer-to-pointer type. If that is the case, then add the
              // pointer's element type to the inference set. If the stored
              // location is not a pointer-to-pointer, but rather a pointer to an
              // aggregate type, then the store is writing the first field within
              // the aggregate.
              //
              // Case 1: Store a pointer to an allocated structure
              //   %struct.test03 = type { i32, i32 }
              //   %pps = alloca %struct.test03*
              //   %tmp = bitcast %struct.test03** %pps to i64*
              //   %alloc = call i8* @malloc(i64 %size)
              //   %pti = ptrtoint i8* %alloc to i64
              //   store i64 %pti, i64* %tmp
              //
              // Case 2: Store to the first field within a structure
              //   %struct.test04 = type { i32*, i32* }
              //   %ps = alloca %struct.test04
              //   %tmp = bitcast %struct.test04* %ps to i64*
              //   %alloc = call i8* @malloc(i64 %size)
              //   %pti = ptrtoint i8* %alloc to i64
              //   store i64 %pti, i64* %tmp
              //
              DTransType *ElemTy = PtrTy->getPointerElementType();
              if (ElemTy->isPointerTy()) {
                addInferredType(PTI, PtrTy->getPointerElementType());
              } else if (ElemTy->isAggregateType()) {
                auto ElemZeroTy = PTA.getElementZeroType(ElemTy);
                if (ElemZeroTy && ElemZeroTy.value().second->isPointerTy())
                  addInferredType(PTI, ElemZeroTy.value().second);
            }
          }
        }
      }
    }

    // Also, add any known pointer type information about the pointer operand of
    // the PtrToInt instruction. Normally, this will not add any new pointer
    // types for the instruction, but it is necessary to handle cases where an
    // i8* parameter type is being inferred, when it is only used as an i8* type
    // to guarantee the inferred set contains the i8* type to prevent it from
    // being marked as unhandled. For example, the following should propagate
    // the i8* type to the inferred set:
    //   %as1.i64 = ptrtoint i8* %arg1 to i64
    //   %as2.i64 = ptrtoint i8* %arg2 to i64
    //   %length = sub i64 %as2.i64, %as1.i64
    ValueTypeInfo *PTIInfo = PTA.getOrCreateValueTypeInfo(PTI, 0);
    for (auto *DType : PTIInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
      if (auto *PtrTy = dyn_cast<DTransPointerType>(DType))
        addInferredType(PTI, PtrTy);

    propagateInferenceSet(PTI, ValueToInfer, DerefType::DT_SameType);
  }

  // Helper function to copy inferred types associated with one Value to
  // another, with an optional change in the level of indirection.
  void propagateInferenceSet(Value *From, Value *To, DerefType DerefLevel) {
    auto It = PendingValueToInferredTypes.find(From);
    if (It == PendingValueToInferredTypes.end())
      return;

    for (auto Ty : It->second)
      if (DerefLevel == DerefType::DT_SameType)
        addInferredType(To, Ty);
      else if (DerefLevel == DerefType::DT_PointerToType)
        addInferredType(To, TM.getOrCreatePointerType(Ty));
      else if (DerefLevel == DerefType::DT_PointeeType)
        if (Ty->isPointerTy())
          addInferredType(To, Ty->getPointerElementType());
  }

  // Add a type to the inferred set for the Value.
  void addInferredType(Value *V, DTransType *DType) {
    if (PendingValueToInferredTypes[V].insert(DType).second)
      DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
        dbgs() << "    - Inferred: ";
        printValue(dbgs(), V);
        dbgs() << " as " << *DType << "\n";
      });
  }

  // This helper pushes a value on the back of the dependency stack
  // and returns true if this is the first occurrence of the value on the
  // stack or false if it was present before the call. The value is pushed
  // onto the stack in either case. A return value of 'true' indicates that
  // dependents of the Value also need to be added to the stack.
  [[nodiscard]] bool addDependency(Value *DV, DependencyStackImpl &DepStack) {
    // If the dependency has already been completely analyzed, skip adding the
    // item to the stack and return false because all of the values that
    // depend on it have also been analyzed.
    if (auto *Info = PTA.getValueTypeInfo(DV))
      if (Info->isCompletelyAnalyzed())
        return false;

    auto REnd = DepStack.rend();
    auto It =
        std::find_if(DepStack.rbegin(), REnd, [&DV](DependencyInfo &Info) {
          return std::get<ValueIdx>(Info) == DV;
        });
    DepStack.push_back({DV, nullptr, DK_ANALYZE});
    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_DEP_TRACE, {
      dbgs() << "Added dependency: ";
      printValue(dbgs(), DV);
      dbgs() << "\n";
    });

    bool IsNewDependency = It == REnd;

    // Analysis of some values is based on inferring a type by looking at
    // the users of the value. Looking at the users of the value may require
    // looking at the users of those values, or analyzing some operand that is
    // used in the instruction for those values. This can become complex
    // because trying to analyze some other value requires all the
    // dependencies of that value to be analyzed, which may again result in
    // trying to infer the type of a value. This may result in the
    // same value being on the stack multiple times where some intervening
    // values are processed. Care must be taken when building the dependency
    // stack to avoid getting into a recursive cycle when populating it. We
    // want to build a stack of values that need to be analyzed or inferred,
    // such that after processing all the values on the stack that follow the
    // value 'DV', we have enough information to perform type resolution on
    // 'DV'. This is handled by:
    //   1. Allow adding dependences of the value to the stack the first time a
    //      value is placed on the stack. This is controlled by the caller
    //      checking the value of this function.
    //   2. Only adding values to be inferred on the stack the first time a
    //      value is placed on the stack.
    //   3. There is a special case below where dependencies can be added
    //      again to the stack to handle case where a type is being inferred,
    //      and requires a value to be analyzed. In that case, the dependencies
    //      may be added an additional time to ensure they will be processed
    //      before the last infer that required them is popped off the stack.
    //
    if (IsNewDependency && inferNeeded(DV)) {
      DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_DEP_TRACE,
                        dbgs() << "  Dependency requires infer\n");

      // Collect all the users so they can be examined to try to infer the
      // value type. We need to use a SetVector when collecting the Values
      // traversed so that they are processed in the correct order during
      // the calls to infer functions. When visiting the users, Values that
      // may need to be analyzed to help with the infer will also be collected
      // so that we can process those values prior to performing the infer.
      SetVector<std::pair<Value *, Value *>> VisitedUsers;
      DenseMap<Value *, SetVector<Value *>> AnalysisNeeded;
      populateInferStack(DV, VisitedUsers, AnalysisNeeded);

      // Add any users not currently on the stack so they will be examined
      // prior to DV. Also, determine which Values that help with the infer
      // should be placed on the stack to be analyzed before the infer. We
      // allow the items to be inferred to be placed on the stack multiple
      // times because additional information may be determined between the
      // processing of the item. Only the first time on item is placed on the
      // stack will the values that should be analyzed prior to it be added to
      // the stack.
      SmallVector<std::pair<Value *, Value *>, 16> InferDependency;
      for (auto &KV : VisitedUsers) {
        Value *InferInstruction = KV.first;
        Value *ValueToInfer = KV.second;
        // We use rbegin()/rend() here because if the item is on the stack, we
        // are more likely to find the item towards the end than the beginning.
        bool OnStack =
            std::find_if(DepStack.rbegin(), DepStack.rend(),
                         [&InferInstruction](DependencyInfo &Info) {
                           return std::get<ValueIdx>(Info) == InferInstruction;
                         }) != DepStack.rend();

        DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_DEP_TRACE, {
          dbgs() << "Adding infer dependency: ";
          printValue(dbgs(), InferInstruction);
          dbgs() << "\n";
        });
        DepStack.push_back({InferInstruction, ValueToInfer, DK_INFER_FROM_USE});
        if (!OnStack)
          for (auto *ValueToAnalyze : AnalysisNeeded[InferInstruction])
            InferDependency.push_back({ValueToAnalyze, ValueToInfer});
      }

      // Add the values to be analyzed for the infer elements to the stack.
      // Also, we may need to add their dependencies in case that they won't be
      // visited before we get to an item that is to be inferred which required
      // the analysis.
      for (auto &KV : InferDependency) {
        Value *ValueToAnalyze = KV.first;
        Value *ValueToInfer = KV.second;

        bool DepsNeeded = true;
        bool InferFound = false;
        for (auto &Dep : DepStack) {
          if (InferFound && std::get<ValueIdx>(Dep) == ValueToAnalyze) {
            DepsNeeded = false;
            break;
          }
          if (std::get<InferValueIdx>(Dep) == ValueToInfer)
            InferFound = true;
        }

        DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_DEP_TRACE, {
          dbgs() << "Infer requires analysis of: ";
          printValue(dbgs(), ValueToAnalyze);
          dbgs() << "\n";
        });
        DepStack.push_back({ValueToAnalyze, nullptr, DK_ANALYZE});
        if (DepsNeeded)
          populateDependencyStack(ValueToAnalyze, DepStack);
      }
    }

    return IsNewDependency;
  }

  // Return 'true' if the analyzing 'V' should trigger an infer call.
  //
  // There are various instructions where the type cannot be resolved directly,
  // such as a bitcast, inttoptr, or a memory allocation, etc. This function has
  // the rules for when the value needs to be inferred.
  bool inferNeeded(Value *V) {
    if (auto *Arg = dyn_cast<Argument>(V)) {
      Function *F = Arg->getParent();
      if (Arg->users().empty())
        return false;

      // If the metadata did not contain the type, we will try to infer the
      // argument type from its usage, if there was only a single use.
      ValueTypeInfo *FuncInfo = PTA.getValueTypeInfo(F);
      assert(FuncInfo &&
             "Expected VisitModule to create info for all defined functions");
      auto &AliasSet =
          FuncInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl);
      if (AliasSet.empty())
        return true;

      // Also, any i8* argument gets inferred because it may be used as any
      // type.
      DTransType *FnPtrTy = *AliasSet.begin();
      auto *FnTy = cast<DTransFunctionType>(FnPtrTy->getPointerElementType());
      unsigned ArgNo = Arg->getArgNo();
      if (ArgNo >= FnTy->getNumArgs())
        return false;
      auto *ArgTy = FnTy->getArgType(ArgNo);
      if (ArgTy == PTA.getDTransI8PtrType())
        return true;
      return false;
    } else if (auto *BC = dyn_cast<BitCastOperator>(V)) {
      return true;
    } else if (auto *Call = dyn_cast<CallBase>(V)) {
      std::pair<bool, DTransType *> OptRetType = getCallReturnType(Call);
      if (OptRetType.first && OptRetType.second == PTA.getDTransI8PtrType())
        return true;
      return false;
    } else if (auto *ITP = dyn_cast<IntToPtrInst>(V))
      return true;
    return false;
  }

  // Once an Value has been determined that it requires being inferred from the
  // users, this function collects all the users (and users of users, etc) that
  // will be processed. This also collects Values that need to be analyzed to
  // help with the inferring of some Value.
  void
  populateInferStack(Value *ValToInfer,
                     SetVector<std::pair<Value *, Value *>> &Visited,
                     DenseMap<Value *, SetVector<Value *>> &AnalysisNeeded) {

    // Breadth-first traversal to add values to infer stack, and determine any
    // other values that should be analyzed before the infer code runs.
    std::deque<Value *> Worklist;
    Worklist.push_back(ValToInfer);
    while (!Worklist.empty()) {
      Value *CurValToInfer = Worklist.front();
      Worklist.pop_front();

      for (auto *User : CurValToInfer->users()) {
        ValueTypeInfo *Info = PTA.getOrCreateValueTypeInfo(User);
        if (Info->isCompletelyAnalyzed())
          continue;

        if (Visited.insert({User, CurValToInfer})) {
          if (isa<BitCastOperator>(User) || isa<PHINode>(User) ||
              isa<SelectInst>(User) || isa<IntToPtrInst>(User) ||
              isa<PtrToIntInst>(User) || isa<ReturnInst>(User)) {
            Worklist.push_back(User);
            continue;
          }

          if (auto *ICmp = dyn_cast<ICmpInst>(User)) {
            Value *OtherOp = ICmp->getOperand(0);
            if (OtherOp == CurValToInfer)
              OtherOp = ICmp->getOperand(1);
            if (!isCompilerConstant(OtherOp))
              AnalysisNeeded[ICmp].insert(OtherOp);

          } else if (auto *LI = dyn_cast<LoadInst>(User)) {
            // Add the load to be processed. If the type is not simple, then
            // also continue looking ahead to the users of the load.
            llvm::Type *LoadType = LI->getType();
            if (!TM.isSimpleType(LoadType))
              Worklist.push_back(User);
          } else if (auto *SI = dyn_cast<StoreInst>(User)) {
            Value *ValOp = SI->getValueOperand();
            Value *PtrOp = SI->getPointerOperand();
            if (PtrOp != CurValToInfer)
              AnalysisNeeded[SI].insert(PtrOp);
            else
              AnalysisNeeded[SI].insert(ValOp);
          } else if (auto *GEP = dyn_cast<GEPOperator>(User)) {
            // Stop the infer when a simple type is encountered. This prevents
            // the target type that a byte-flattened GEP is used as being
            // back-propagated onto the GEP operand.
            llvm::Type *SrcType = GEP->getSourceElementType();
            if (GEP->getNumIndices() == 1 && !TM.isSimpleType(SrcType))
              Worklist.push_back(User);
          } else if (auto *Call = dyn_cast<CallBase>(User)) {
            if (Call == CurValToInfer ||
                Call->getCalledOperand() == CurValToInfer)
              Worklist.push_back(Call);
          }
        }
      }
    }
  }

  // Build a stack of values that must be analyzed to compute the type of V.
  void populateDependencyStack(Value *V, DependencyStackImpl &DependentVals) {
    if (auto *I = dyn_cast<Instruction>(V)) {
      switch (I->getOpcode()) {
      default:
        break;

      case Instruction::BitCast:
      case Instruction::ExtractValue:
      case Instruction::Freeze:
        if (addDependency(I->getOperand(0), DependentVals))
          populateDependencyStack(I->getOperand(0), DependentVals);
        break;

      case Instruction::GetElementPtr: {
        auto *GEP = cast<GetElementPtrInst>(I);
        Value *BasePtr = GEP->getPointerOperand();
        if (addDependency(BasePtr, DependentVals))
          populateDependencyStack(BasePtr, DependentVals);
        break;
      }
      case Instruction::IntToPtr:
      case Instruction::PtrToInt:
        if (addDependency(I->getOperand(0), DependentVals))
          populateDependencyStack(I->getOperand(0), DependentVals);
        break;
      case Instruction::Load: {
        auto *LI = cast<LoadInst>(I);
        Value *Ptr = LI->getPointerOperand();
        if (addDependency(Ptr, DependentVals))
          populateDependencyStack(Ptr, DependentVals);
        break;
      }
      case Instruction::PHI: {
        // Get the set of unique values, excluding the PHInode to be analyzed,
        // to ensure duplicate incoming values from different paths are not
        // added multiple times. Then, for each new addition to the
        // DependentVals list, add the dependents of those.
        SmallPtrSet<Value *, 4> PhiDeps;
        auto *Phi = cast<PHINode>(V);
        for (Value *InVal : Phi->incoming_values())
          if (InVal != Phi)
            PhiDeps.insert(InVal);

        SmallVector<Value *, 4> DepsAdded;
        for (auto *Dep : PhiDeps)
          if (addDependency(Dep, DependentVals))
            DepsAdded.push_back(Dep);

        for (Value *Dep : DepsAdded)
          populateDependencyStack(Dep, DependentVals);
        break;
      }
      case Instruction::Select: {
        auto *Sel = cast<SelectInst>(V);
        Value *TV = Sel->getTrueValue();
        Value *FV = Sel->getFalseValue();
        bool TrueWasNew = addDependency(TV, DependentVals);
        bool FalseWasNew = addDependency(FV, DependentVals);
        if (TrueWasNew)
          populateDependencyStack(TV, DependentVals);
        if (FalseWasNew)
          populateDependencyStack(FV, DependentVals);
        break;
      }

      case Instruction::Sub:
        if (addDependency(I->getOperand(0), DependentVals))
          populateDependencyStack(I->getOperand(0), DependentVals);
        if (addDependency(I->getOperand(1), DependentVals))
          populateDependencyStack(I->getOperand(1), DependentVals);
        break;
      }
    }
  }

  void dumpDependencyStack(Value *V, DependencyStackImpl &DependentVals) {
    dbgs() << "\n  DependentVals for: ";
    printValue(dbgs(), V, /*ReportFuncName=*/false);
    dbgs() << "\n";
    for (auto &DepInfo : DependentVals) {
      dbgs() << "    ";
      if (std::get<DepKindIdx>(DepInfo) == DK_ANALYZE) {
        dbgs() << "Analyze: ";
      } else {
        dbgs() << "Infer  : ";
        printValue(dbgs(), std::get<InferValueIdx>(DepInfo),
                   /*ReportFuncName=*/false);
        dbgs() << "\n      User : ";
      }
      printValue(dbgs(), std::get<ValueIdx>(DepInfo),
                 /*ReportFuncName=*/false);
      dbgs() << "\n";
    }
    dbgs() << "\n";
  }

  void analyzeArgument(Argument *Arg, ValueTypeInfo *ResultInfo) {
    if (!dtrans::hasPointerType(Arg->getType()))
      return;

    Function *F = Arg->getParent();
    ValueTypeInfo *FuncInfo = PTA.getValueTypeInfo(F);
    assert(FuncInfo &&
           "Expected VisitModule to create info for all defined functions");

    // We expect the declaration type set to have one pointer entry to describe
    // the function. However, we can ignore the Argument, if there are no users.
    // Also, we can try to infer the type based on the usage to see if it is
    // used as a single type.
    auto &AliasSet = FuncInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl);
    if (AliasSet.empty() && !Arg->hasNUsesOrMore(1))
      return;

    if (AliasSet.size() != 1) {
      auto It = PendingValueToInferredTypes.find(Arg);
      if (It != PendingValueToInferredTypes.end()) {
        ValueTypeInfo *ResultInfo = PTA.getOrCreateValueTypeInfo(Arg);
        for (auto Ty : It->second)
          ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Use, Ty);
      }
      auto &UseAliasSet =
          ResultInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use);
      if (UseAliasSet.size() == 1) {
        ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl, *UseAliasSet.begin());
        ResultInfo->setCompletelyAnalyzed();
      } else {
        // It may be possible to allow this to handle non-PAROPT created
        // functions, but for now just handle PAROPT functions that do not get
        // DTrans metadata information, since they will just be called as
        // callback functions.
        if (isPAROPTCreatedFunction(*F)) {
          ResultInfo->setCompletelyAnalyzed();

          // If there is a dominant pointer to structure type, we can treat
          // that as the incoming argument type. This allows for a pointer to
          // a structure to be passed in that is also used as a pointer to
          // the element zero element.
          if (ResultInfo->canAliasToAggregatePointer()) {
            DTransType *DomTy = PTA.getDominantAggregateType(
                *ResultInfo, ValueTypeInfo::VAT_Use);
            if (DomTy)
              ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl, DomTy);
          }

          return;
        }
      
        ResultInfo->setUnhandled();
      }
      return;
    }

    DTransType *FnPtrTy = *AliasSet.begin();
    if (!FnPtrTy->isPointerTy() ||
        !FnPtrTy->getPointerElementType()->isFunctionTy()) {
      ResultInfo->setUnhandled();
      return;
    }

    auto *FnTy = cast<DTransFunctionType>(FnPtrTy->getPointerElementType());
    unsigned ArgNo = Arg->getArgNo();
    if (ArgNo >= FnTy->getNumArgs()) {
      ResultInfo->setUnhandled();
      return;
    }

    auto *ArgTy = FnTy->getArgType(ArgNo);
    ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl, ArgTy);

    // Types that were declared as i8* need to have special treatment because
    // i8* is the equivalent of void*. For these cases, we need to look-ahead to
    // the uses of the argument to see if it may be a pointer to a structure
    // type.
    // For example:
    //   define @qsort(ptr %arg)    ; @qsort(i8* %arg)
    //   %tmp = ptrtoint ptr to i64
    //   call @ar_compare(ptr %arg) ; @ar_compare(%struct.ar* %arg)
    //
    // In this case, to capture the ptrtoint as operating on a pointer to a
    // structure type, we need to look-ahead to where %arg is used, and infer
    // the argument is expected to be a %struct.ar* type.
    if (ArgTy == PTA.getDTransI8PtrType()) {
      auto It = PendingValueToInferredTypes.find(Arg);
      if (It != PendingValueToInferredTypes.end()) {
        ValueTypeInfo *ResultInfo = PTA.getOrCreateValueTypeInfo(Arg);
        for (auto Ty : It->second)
          ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Use, Ty);
      }
    }
  }

  void analyzeAllocaInst(AllocaInst *AI, ValueTypeInfo *ResultInfo) {
    // Types that involve pointers should have metadata annotations attached,
    // use them to set the declared type of the AllocaInst.
    DTransType *DType = MDReader.getDTransTypeFromMD(AI);
    if (DType) {
      // The metadata describes the ValueType of the alloca, but the
      // Value object is a pointer to that type.
      PTA.setDeclaredType(AI, TM.getOrCreatePointerType(DType));
      return;
    }

    // Try to produce the type for a non-metadata annotated type.
    llvm::Type *Ty = AI->getAllocatedType();
    if (TM.isSimpleType(Ty)) {
      DTransType *DTy = TM.getOrCreateSimpleType(Ty);
      assert(DTy && "Expected simple type");

      // Alloca results are pointers to the type allocated
      PTA.setDeclaredType(AI, TM.getOrCreatePointerType(DTy));
      return;
    }

    ResultInfo->setUnhandled();
    LLVM_DEBUG(dbgs() << "Unhandled AllocaInst:" << *AI << "\n");
  }

  void analyzeBitCastOperator(BitCastOperator *BC, ValueTypeInfo *ResultInfo) {
    // Bitcast can be used to cast between any type that has the same underlying
    // size. We need to process conversions that are generating a pointer
    // types, such as:
    //   bitcast %struct.foo* to %struct.bar*
    //   bitcast <2 x i32*> to <2 x i64*>
    if (!dtrans::hasPointerType(BC->getType()))
      return;

    // When pointers are fully opaque, we do not expect to see these
    // instructions, but in order to run analysis on the current IR, we need to
    // treat the BitCast result as just being an alias of the original variable.
    //
    // First set the information on the BitCast as being the same as the
    // information of the source operand. Propagate the original values from
    // the 'VAT_Decl' set, as well, in order to track the type the pointer was
    // originally declared as. This is a special case where we don't try to
    // keep the 'VAT_Decl' type list to match what the LLVM IR type would be.
    // e.g. %y = bitcast %struct.foo* %x to %struct.bar*. We will continue
    // tracking %y as if it were declared as %struct.foo*. This is necessary to
    // help identify the original type to know whether a use of the pointer is
    // going to be safe for that type.
    ValueTypeInfo *SrcInfo = PTA.getOrCreateValueTypeInfo(BC, 0);
    propagate(SrcInfo, ResultInfo, /*Decl=*/true, /*Use=*/true,
              DerefType::DT_SameType);
    if (!SrcInfo->isCompletelyAnalyzed())
      ResultInfo->setPartiallyAnalyzed();

    if (SrcInfo->getUnhandled() || SrcInfo->getDependsOnUnhandled())
      ResultInfo->setDependsOnUnhandled();

    // Next, take the look-ahead value that inferred the target type.
    auto It = PendingValueToInferredTypes.find(BC);
    if (It != PendingValueToInferredTypes.end())
      for (auto Ty : It->second)
        ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Use, Ty);
  }

  // For a call that results in a pointer type, we want to update the
  // ResultInfo with type aliases. We also want to update the ValueTypeInfo
  // of the parameters to reflect the usage type in case a pointer is being
  // passed to a function that is declared as taking a pointer of a different
  // type.
  void analyzeCallBase(CallBase *Call, ValueTypeInfo *ResultInfo) {
    // Check if 'Call' is to an intrinsic function that has special properties.
    auto ProcessIntrinsicCall = [this](CallBase *Call,
                                       ValueTypeInfo *ResultInfo) {
      Function *Target = dtrans::getCalledFunction(*Call);
      if (!Target)
        return false;
      if (Target->isIntrinsic()) {
        switch (Target->getIntrinsicID()) {
        case Intrinsic::ptr_annotation: {
          // The llvm.ptr.annotation.ptr() calls returns the first argument, so
          // we can just the propagate the type information that's been
          // collected for the parameter to the result value.
          ValueTypeInfo *ValInfo = analyzeValue(Call->getArgOperand(0));
          propagate(ValInfo, ResultInfo, /*Decl=*/true, /*Use=*/true,
                    DerefType::DT_SameType);
          return true;
        }
        default:
          return false;
        }
      }

      return false;
    };

    if (ProcessIntrinsicCall(Call, ResultInfo))
      return;

    // Check if the return type should have a pointer type, and the type, if so.
    std::pair<bool, DTransType *> OptRetType = getCallReturnType(Call);
    if (OptRetType.first) {
      if (OptRetType.second) {
        ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl, OptRetType.second);

        // Any call that returns an i8* should be checked for the type it gets
        // used as to support handling IR with non-opaque pointer type casts.
        // For example:
        //   1)  %ty1 = call i8* @creator()
        //   2)  %ty2 = bitcast i8* %ty1 to %struct.type2*
        //   3)  <some instruction using %ty2 as %struct.type2*>
        //
        // TODO: Once opaque pointers are introduced, the bitcast will no longer
        // exist, and instruction 3 will directly use %ty1 as a %struct.type2*
        // value, so this inference should no longer be necessary.
        if (OptRetType.second == PTA.getDTransI8PtrType()) {
          auto It = PendingValueToInferredTypes.find(Call);
          if (It != PendingValueToInferredTypes.end())
            for (auto Ty : It->second)
              ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Use, Ty);
        }
      } else {
        ResultInfo->setUnhandled();
        LLVM_DEBUG(dbgs() << "Unknown return type for call: " << *Call);
      }
    }

    // Perform look-ahead analysis on the Value produced by an allocation
    // function to identify the type the result gets used as.
    //
    // TODO: Extend this for user allocation functions, if needed. Currently,
    // the user alloc functionality relies on pattern matching of bitcast
    // instructions that won't exist once there are opaque pointers and
    // getPointerElementType() so cannot be used yet.
    const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
    dtrans::FreeKind FKind = DTAC.getFreeFnKind(Call, TLI);
    if (FKind != dtrans::FK_NotFree)
      PTA.addFreeCall(Call, FKind);

    dtrans::AllocKind AKind = DTAC.getAllocFnKind(Call, TLI);
    if (AKind != dtrans::AK_NotAlloc) {
      PTA.addAllocationCall(Call, AKind);

      // Track the VAT_Decl type of the allocation as the type the
      // object gets used as.
      auto It = PendingValueToInferredTypes.find(Call);
      if (It != PendingValueToInferredTypes.end())
        for (auto Ty : It->second)
          ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl, Ty);
    } else if (OptRetType.second &&
               OptRetType.second == PTA.getDTransI8PtrType()) {
      // Check if the return type should have a pointer type. If so, infer the
      // type.

      // Any call that returns an i8* should be checked for the type it gets
      // used as to support handling IR with non-opaque pointer type casts.
      // For example:
      //   1)  %val1 = call i8* @creator()
      //   2)  %val2 = bitcast i8* %val1 to %struct.type2*
      //   3)  <some instruction using %val2 as %struct.type2*>
      //
      // Note: Once opaque pointers are introduced, the bitcast will no
      // longer exist, and instruction 3 will directly use %ty1 as a
      // %struct.type2* value, so this inference should no longer be
      // necessary because %val1 will be used directly as a different type.
      auto It = PendingValueToInferredTypes.find(Call);
      if (It != PendingValueToInferredTypes.end())
        for (auto Ty : It->second)
          ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Use, Ty);
    }

    // Set the usage type for the call arguments.
    unsigned NumArg = Call->arg_size();
    for (unsigned AI = 0; AI < NumArg; ++AI) {
      Value *Arg = Call->getArgOperand(AI);
      // We can ignore parameters that are the address of functions, since
      // we do not track safety data on a pointer to a function type.
      if (isa<Function>(Arg))
        continue;

      if (dtrans::hasPointerType(Arg->getType())) {
        ValueTypeInfo *ParamInfo = PTA.getOrCreateValueTypeInfo(Call, AI);
        // Check whether the argument should have a pointer type, and the type,
        // if so.
        std::pair<bool, DTransType *> OptArgType = getArgumentType(Call, AI);
        if (!OptArgType.first)
          continue;

        if (OptArgType.second) {
          ParamInfo->addTypeAlias(ValueTypeInfo::VAT_Use, OptArgType.second);
        } else {
          ParamInfo->setUnhandled();
          LLVM_DEBUG(dbgs() << "Unknown usage type for call argument: " << *Call
                            << " @" << AI << "\n");
        }
      }
    }
  }

  // Determine the expected type for the return value for types of interest for
  // a call.
  //
  // Return value meaning:
  //  <false, nullptr>    - Type is not a pointer type to be tracked.
  //  <true, DTransType*> - Type is a pointer, of the type in the pair.
  //  <true, nullptr>     - Type is a pointer, but could not be resolved.
  std::pair<bool, DTransType *> getCallReturnType(CallBase *Call) {
    if (!dtrans::hasPointerType(Call->getType()))
      return {false, nullptr};

    // Check if there is metadata for information available to use from a called
    // function or an indirect call.
    if (Call->isIndirectCall()) {
      DTransType *DType = MDReader.getDTransTypeFromMD(Call);
      if (DType) {
        auto *FnType = cast<DTransFunctionType>(DType);
        DTransType *DRetTy = FnType->getReturnType();
        return {true, DRetTy};
      }
      return {true, nullptr};
    }
    Function *Target = dtrans::getCalledFunction(*Call);

    // If a target function was not found, we cannot resolve the type.
    // This can also occur as the result of the call instruction being used
    // for an inline-asm statement.
    if (!Target)
      return {true, nullptr};

    return getFunctionReturnType(Target, GetTLI(*Call->getFunction()));
  }

  // Determine the expected type for the return value for types of interest for
  // a Function. (See getCallReturnType for description of return value)
  std::pair<bool, DTransType *>
  getFunctionReturnType(Function *F, const TargetLibraryInfo &TLI) {

    // Check whether the return value will be of interest.
    llvm::Type *RetType = F->getReturnType();
    if (!dtrans::hasPointerType(RetType))
      return {false, nullptr};

    // Handle functions with metadata descriptions.
    DTransType *DType = MDReader.getDTransTypeFromMD(F);
    if (DType) {
      auto *FnType = cast<DTransFunctionType>(DType);
      DTransType *DRetTy = FnType->getReturnType();
      return {true, DRetTy};
    }

    return {true, DTransLibInfo.getFunctionReturnType(F)};
  }

  // Determine the type that a function is expected to use the argument as.
  std::pair<bool, DTransType *> getArgumentType(CallBase *Call, unsigned Idx) {
    if (!dtrans::hasPointerType(Call->getArgOperand(Idx)->getType()))
      return {false, nullptr};

    DTransType *DType = nullptr;
    Function *Target = dtrans::getCalledFunction(*Call);
    if (Call->isIndirectCall())
      DType = MDReader.getDTransTypeFromMD(Call);
    else if (Target)
      DType = MDReader.getDTransTypeFromMD(Target);

    // There is no need to add the type on the call parameter, if the callee
    // is known not to use the argument.
    if (Target && !Target->isDeclaration() && Idx < Target->arg_size()) {
      Argument *TargetArg = Target->getArg(Idx);
      if (TargetArg->user_empty())
        return {false, nullptr};
    }

    if (!DType) {
      // Try to get a FunctionType from the ValueTypeInfo of the call. Give up
      // if there is more than one possibility.
      ValueTypeInfo *CallInfo = PTA.getValueTypeInfo(Call->getCalledOperand());
      if (CallInfo) {
        for (auto *Ty :
             CallInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl)) {
          if (!Ty->isPointerTy())
            continue;
          auto *FnTy =
              dyn_cast_or_null<DTransFunctionType>(Ty->getPointerElementType());
          if (FnTy) {
            if (!DType)
              DType = FnTy;
            else
              return {true, nullptr};
          }
        }
      }
    }

    if (DType) {
      auto *FnType = cast<DTransFunctionType>(DType);
      if (Idx < FnType->getNumArgs())
        return {true, FnType->getArgType(Idx)};

      // Cannot return an expected type for a vararg element, but don't treat it
      // as an error. Safety analysis should check for type compatibility of the
      // vararg elements.
      if (FnType->isVarArg())
        return {false, nullptr};

      return {true, nullptr};
    }

    // If the target could not be resolved, such as because the call is an
    // indirect call that did not have metadata processed above, or the call
    // is an inline-asm block, then we cannot tell the expected argument type.
    if (!Target)
      return {true, nullptr};

    // Ignore any llvm.dbg.* intrinsics, since they don't provide meaning
    // DTransType information.
    if (Target->isIntrinsic() && isa<DbgInfoIntrinsic>(Target))
      return {false, nullptr};

    return {true, DTransLibInfo.getFunctionArgumentType(Target, Idx)};
  }

  void analyzeGetElementPtrOperator(GEPOperator *GEP,
                                    ValueTypeInfo *ResultInfo) {
    analyzeGetElementPtrOperatorIndex(*GEP, ResultInfo);
    analyzeGEPAsBitcastEquivalent(*GEP, ResultInfo);
  }

  // Update \p ResultInfo with the type being indexed into and the type of
  // the Value produced by the GEP if the GEP is indexing an aggregate type. For
  // a GEP with 0 or 1 index, transfer the alias types from the base pointer to
  // the \p ResultInfo.
  void analyzeGetElementPtrOperatorIndex(GEPOperator &GEP,
                                         ValueTypeInfo *ResultInfo) {
    // Get the type of element being indexed within \p DTy using \p IdxList
    // for traversing the types. DTy could be a structure, or sequential type.
    // This will walk the indices to find the type, or return nullptr if the
    // indices are not valid for the input type.
    auto GetGEPIndexedType = [](DTransType *DTy,
                                ArrayRef<Value *> IdxList) -> DTransType * {
      if (IdxList.empty())
        return DTy;

      unsigned CurIdx = 1;
      DTransType *Agg = DTy;
      for (; CurIdx != IdxList.size(); ++CurIdx) {
        if (!Agg || Agg->isPointerTy())
          return nullptr;

        if (auto *DSeqTy = dyn_cast<DTransSequentialType>(Agg)) {
          Agg = DSeqTy->getTypeAtIndex(0);
        } else if (auto *DStTy = dyn_cast<DTransStructType>(Agg)) {
          Value *IndexValue = IdxList[CurIdx];
          uint64_t Idx =
              cast<Constant>(IndexValue)->getUniqueInteger().getZExtValue();
          if (DStTy->getNumFields() <= Idx)
            return nullptr;

          // Try to get the field type of the structure. If there was a conflict
          // while re-constructing the structure types from metadata, then this
          // will result in nullptr, and we cannot tell what type is being used.
          Agg = DStTy->getFieldType(Idx);
          if (!Agg)
            return nullptr;
        }
      }
      return Agg;
    };

    auto ProcessIndexedElement = [&GetGEPIndexedType, this, &GEP, ResultInfo](
                                     DTransType *Ty,
                                     SmallVectorImpl<Value *> &GepOps) {
      // Need to compute indexed type based on GEP indices
      // Case 1: Structure
      //  - 2 operands, type is field member type
      //  - 3 or more operands, type is nested structure or array access.
      //    Iterate on field member type until final type is reached.
      //
      // Case 2: Array
      // - 2 operands, type is array element type.
      //   For example:
      //    [8 x i16] -> i16
      //    [8 x ptr] --> type is kind of pointer stored in array.
      // - 3 or more operands, type is multi-dimensional array.  Iterate
      //   until type is reached.
      //
      // If this function returns 'false', an appropriate type alias could not
      // be found with 'Ty' using the 'GepOps'. The caller should mark the
      // ResultInfo as 'unhandled' if this index lookup was not being done
      // speculatively.
      //
      DTransType *IndexedTy = GetGEPIndexedType(Ty, GepOps);
      if (!IndexedTy)
        return false;

      if (auto *IndexedStTy = dyn_cast<DTransStructType>(IndexedTy)) {
        // The final operand of the GEP of a structure type will always be a
        // constant value when the result of the GEP is the address of a field
        // within a structure. When the operand is not a constant then the
        // indexing is into an array. If the alias type that is being tested
        // matches the type used for the GEP, we should be able to walk the
        // element zero field of the aggregate type identified by
        // GetGEPIndexedType() until reaching the source type of the GEP. If a
        // match of the source type is found, then that type will be added as
        // the result type of the GEP.
        // For example:
        //   Given:
        //     %arg as the DTrans type '%struct.outer*'
        //     %gep = getelementptr [16 x ptr], ptr %arg, i64 0, i64 %idx
        //
        //   Where:
        //     %struct.outer = type { %struct.nodeInfo }
        //     %struct.nodeInfo = type { [16 x ptr], ptr, i64, i64 }
        //
        //   %gep can be resolved as being the type of pointer stored in the
        //   array of %struct.nodeInfo
        //
        // However, other cases caused by union types may not be able to be
        // resolved in this manner because the type used for the GEP does not
        // correspond to a type within the structure. Those will be marked as
        // 'unhandled' for now.
        // For example:
        //   Given:
        //     %18 = getelementptr %struct.varray_head_tag,
        //                         ptr %2, i64 0, i32 4
        //     %20 = getelementptr [1 x ptr], ptr %18, i64 0, i64 %21
        //
        //   Where:
        //      %union.varray_head_tag = type { i64, i64, i32, ptr,
        //                                      %union.varray_data_tag }
        //      %union.varray_data_tag = type { [1 x i64] }
        //
        // The pointer type collection analysis would have %18 as being the type
        // "%union.varray_data_tag*" based on the result of the first GEP. When
        // evaluating the second GEP, the element zero array type will not match
        // the type used in the GEP. This case will return 'false' to
        // conservatively treat the GEP as 'unhandled'
        auto *LastArg =
            dyn_cast<ConstantInt>(GEP.getOperand(GEP.getNumOperands() - 1));
        if (!LastArg) {
          if (IndexedStTy->getNumFields() == 0)
            return false;

          DTransType* FieldTy = IndexedStTy->getFieldType(0);
          if (!FieldTy)
            return false;

          llvm::Type* GEPSrcTy = GEP.getSourceElementType();
          DTransType* ElemTy = FieldTy;
          bool FoundType = false;
          while (ElemTy->isAggregateType()) {
            FieldTy = ElemTy;
            if (ElemTy->isArrayTy())
              ElemTy = ElemTy->getArrayElementType();
            else if (auto* NestedStTy = dyn_cast<DTransStructType>(ElemTy))
              ElemTy = NestedStTy->getFieldType(0);
            else
              llvm_unreachable("Expected Array or Structure type\n");

            if (FieldTy->getLLVMType() == GEPSrcTy) {
              FoundType = true;
              break;
            }
          }
          if (!FoundType)
            return false;

          DTransType* PtrToElemTy = TM.getOrCreatePointerType(ElemTy);
          ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl, PtrToElemTy);
          ResultInfo->addElementPointee(ValueTypeInfo::VAT_Decl, FieldTy, 0);

          return true;
        }

        uint64_t FieldNum = LastArg->getLimitedValue();
        if (FieldNum >= IndexedStTy->getNumFields())
          return false;

        DTransType *FieldTy = IndexedStTy->getFieldType(FieldNum);
        if (!FieldTy)
          return false;

        ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl,
                                 TM.getOrCreatePointerType(FieldTy));

        // Access is to an element of the structure, update element
        // pointee info
        ResultInfo->addElementPointee(ValueTypeInfo::VAT_Decl, IndexedTy,
                                      FieldNum);
        return true;
      }

      if (auto *IndexedSeqTy = dyn_cast<DTransSequentialType>(IndexedTy)) {
        DTransType *ElemTy = IndexedSeqTy->getElementType();
        DTransType *PtrToElemTy = TM.getOrCreatePointerType(ElemTy);
        ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl, PtrToElemTy);

        // If the GEP is accessing an array that is nested within another type,
        // collect the path to the array element.
        auto CollectElementPointeePath =
            [&GetGEPIndexedType](
                DTransType *AggTy, SmallVectorImpl<Value *> &ArrayGepOps,
                SmallVectorImpl<ValueTypeInfo::PointeeLoc::PointeePairType>
                    &ElementOf) {
              if (ArrayGepOps.size() > 1) {
                SmallVector<Value *, 4> Ops(ArrayGepOps.begin(),
                                            ArrayGepOps.end());
                while (Ops.size() > 1) {
                  // Get the index value for field accessed
                  size_t Idx = 0;
                  auto *ConstIdx = dyn_cast<ConstantInt>(Ops.back());
                  if (ConstIdx)
                    Idx = ConstIdx->getLimitedValue();

                  // Determine the parent type of the field.
                  Ops.pop_back();
                  DTransType *IndexedTy = GetGEPIndexedType(AggTy, Ops);
                  ElementOf.push_back({IndexedTy, Idx});
                }
              }
            };

        // Access is to an element of the structure, update element pointee
        // info.
        Value *LastArg = GEP.getOperand(GEP.getNumOperands() - 1);
        auto *ConstIdx = dyn_cast<ConstantInt>(LastArg);
        if (ConstIdx) {
          uint64_t ElemNum = ConstIdx->getLimitedValue();
          if (GepOps.size() > 1) {
            // Capture the GEP chain because an array nested within a structure
            // could trigger a safety condition on the structure type. For
            // example, the address could be passed to a memset call.
            SmallVector<ValueTypeInfo::PointeeLoc::PointeePairType, 4>
                ElementOf;
            CollectElementPointeePath(Ty, GepOps, ElementOf);
            assert(!ElementOf.empty() && "Unexpected empty list for GEP chain");

            // Count the number of trailing zeros in the GEP index list.
            size_t ZeroCount = 0;
            size_t MaxIndex = GepOps.size() - 1;
            for (size_t IndexNum = MaxIndex; IndexNum > 1; --IndexNum) {
              auto *ConstIdx = dyn_cast<ConstantInt>(GepOps[IndexNum]);
              if (ConstIdx && ConstIdx->isZero()) {
                ZeroCount++;
                continue;
              }
              break;
            }
            // Extract 1 or more elements from the ElementOf set to pass into
            // the addElementPointee call.
            SmallVector<ValueTypeInfo::PointeeLoc::PointeePairType, 4>
                ZeroOffsetElementOf(ElementOf.begin(),
                                    ElementOf.begin() + 1 + ZeroCount);
            ResultInfo->addElementPointee(ValueTypeInfo::VAT_Decl, IndexedTy,
                                          ElemNum, ZeroOffsetElementOf);
          } else {
            ResultInfo->addElementPointee(ValueTypeInfo::VAT_Decl, IndexedTy,
                                          ElemNum);
          }
        } else {
          SmallVector<ValueTypeInfo::PointeeLoc::PointeePairType, 4> ElementOf;
          CollectElementPointeePath(Ty, GepOps, ElementOf);
          ResultInfo->addElementPointeeUnknownOffset(ValueTypeInfo::VAT_Decl,
                                                     IndexedTy, ElementOf);
        }
        return true;
      }

      return false;
    };

    // To handle the case where the pointer operand is the constant null, this
    // needs to retrieve the value using the User & Operand number interface.
    // For example:
    //   %91 = getelementptr %struct.listentry, %struct.listentry* null, i64 %90
    ValueTypeInfo *PointerInfo = PTA.getOrCreateValueTypeInfo(&GEP, 0);
    if (!PointerInfo->isCompletelyAnalyzed())
      ResultInfo->setPartiallyAnalyzed();

    if (PointerInfo->getUnhandled() || PointerInfo->getDependsOnUnhandled())
      ResultInfo->setDependsOnUnhandled();

    llvm::Type *SrcTy = GEP.getSourceElementType();
    // If the GEP is of the form:
    //   %y = getelementptr i8, ptr %x, i64 <n>
    // Check if the GEP should be processed as either a byte-flattened GEP or an
    // array of 'i8' elements.
    BFG_Kind ProcessedAsByteFlattenedGEP = BFG_None;
    if (GEP.getNumIndices() == 1 && SrcTy == PTA.getLLVMI8Type()) {
      ProcessedAsByteFlattenedGEP =
          analyzePotentialByteFlattenedGEPAccess(GEP, ResultInfo);
      if (ProcessedAsByteFlattenedGEP != BFG_Identified)
        ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl,
                                 PTA.getDTransI8PtrType());
    }


    // When opaque pointers are used, the known type of pointer may not
    // match the indexed type, so add that type as one of the 'usage' types
    // now. This is done for simple types, such as:
    //   %100 = getelementptr %testB, ptr %known.as.testA, i32 4
    //
    // This is not done for the partial pointer use idiom because it is known in
    // those cases the GEP is indexing the type with a different type in a safe
    // manner for something like:
    //   %NextHalf1 = getelementptr i32, i32* %HalfPtr1, i64 1
    //
    // This is also not done for byte-flattened GEPs because the pointer is just
    // using the i8 for computing the field address, not as a value type.
    //
    // TODO: For non-simple source element types, we may need more metadata to
    // indicate the type that is being indexed to detect when the type is
    // different than expected.
    if (ProcessedAsByteFlattenedGEP == BFG_None &&
        !PointerInfo->getIsPartialPointerUse() &&
        !GEP.getPointerOperandType()->isVectorTy() && TM.isSimpleType(SrcTy)) {
      DTransType *DTransSrcTy = TM.getOrCreateSimpleType(SrcTy);
      assert(DTransSrcTy && "Expected simple type");
      PointerInfo->addTypeAlias(ValueTypeInfo::VAT_Use,
                                TM.getOrCreatePointerType(DTransSrcTy));
    }

    if (GEP.getNumIndices() > 1) {
      // Mark the pointer operand with the type used for indexing.
      //
      // Cases to handle:
      // Case 1: getelementptr <SimpleTy>, ptr %x, i64 0, i32 1
      //   where SimpleTy is a named structure or simple array definition.
      //
      // Case 2: getelementptr <ComplexTy>, ptr %x, i64 0, i32
      //   where ComplexTy is a literal structure or array that involves
      //   pointer types, such as [8 x %struct.test*] or {i32, i32*}
      //
      // In case 1, we know that %x is being used as a pointer to the simple
      // type.
      // In case 2, we need we rely upon the type information collected
      // based on the declaration of %x. If this turns out to not be sufficient,
      // we will need the FE to annotate these cases with metadata.
      DTransType *DTransSrcTy = nullptr;
      SmallVector<Value *, 4> Ops(GEP.idx_begin(), GEP.idx_end() - 1);
      if (TM.isSimpleType(SrcTy)) {
        DTransSrcTy = TM.getOrCreateSimpleType(SrcTy);
        assert(DTransSrcTy && "Expected type to exist");

        // Mark the pointer operand as being used as the indexing type of the
        // GEP.
        PointerInfo->addTypeAlias(ValueTypeInfo::VAT_Use,
                                  TM.getOrCreatePointerType(DTransSrcTy));

        bool Handled = ProcessIndexedElement(DTransSrcTy, Ops);
        if (!Handled) {
          PointerInfo->setUnhandled();
          ResultInfo->setDependsOnUnhandled();
          LLVM_DEBUG(dbgs() << "unable to resolve index type: " << GEP << "\n");
        }

        return;
      }

      // Try to process the alias type for any of the potential types the base
      // pointer represents.
      bool FoundType = false;
      for (auto *Alias :
           PointerInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl)) {
        // Skip over an i8* type since there is no information that can be
        // resolved from it.
        if (Alias == PTA.getDTransI8PtrType())
          continue;

        bool Handled = false;
        if (Alias->isPointerTy())
          Handled = ProcessIndexedElement(Alias->getPointerElementType(), Ops);

        if (!Handled) {
          // We may need to have metadata on the some GEP instructions that
          // defines the indexed type, if we encounter this case.
          PointerInfo->setUnhandled();
          ResultInfo->setDependsOnUnhandled();
          LLVM_DEBUG(dbgs() << "unable to resolve index type: " << GEP << "\n");
          break;
        }
        FoundType = true;
      }
      if (!FoundType && PointerInfo->isCompletelyAnalyzed()) {
        // No type was able to resolved for the GEP using the pointer.
        PointerInfo->setUnhandled();
        ResultInfo->setDependsOnUnhandled();
        LLVM_DEBUG(dbgs() << "unable to resolve index type: " << GEP << "\n");
      }
      return;
    }

    // Zero or Single index operand GEPs of the form, result in the same
    // type as the base pointer.
    // For example, these cases result in the same type identified for %x:
    //   getelementptr ptr, ptr %x, i64 5
    //   getelementptr <ty>, <ptr vector> %x, <vector index type> %idx
    //
    // These may be treating the source operand as an array, and therefore are
    // generating the same type as the pointer operand. However, there is a
    // special case when indexed type is a single value type (ptr, i8, i32,
    // double, etc) where the indexing is just the number of bytes computed
    // based on the type and the GEP operand. Supported byte-flattened cases
    // were checked above. If it's not one of those cases, we still need to
    // check for the possibility that the GEP is addressing into a structure
    // with some other type.
    //
    // Note: we can only handle the case where the indexing amount is a
    // constant. If it is not constant, we assume it is indexing an array. When
    // the safety analyzer runs, hopefully it would detect a type mismatch where
    // the result is used, if it is not an array.
    if (ProcessedAsByteFlattenedGEP == BFG_None) {
      std::function<bool(const DataLayout &, DTransStructType *, uint64_t,
                         DTransStructType **, DTransType **, uint32_t *)>
          GetFieldTypeAndIndex;

      GetFieldTypeAndIndex =
          [&GetFieldTypeAndIndex](
              const DataLayout &DL, DTransStructType *DTransStTy,
              uint64_t OffsetInBytes, DTransStructType **IndexedType,
              DTransType **FieldType, uint32_t *FieldIndex) -> bool {
        // Try to find a field of 'DTransStTy' that starts at 'OffsetInBytes'
        auto *StTy = cast<llvm::StructType>(DTransStTy->getLLVMType());
        auto *SL = DL.getStructLayout(StTy);
        uint64_t ElemIdx = SL->getElementContainingOffset(OffsetInBytes);
        uint64_t ElemOffset = SL->getElementOffset(ElemIdx);
        if (OffsetInBytes == ElemOffset) {
          *IndexedType = DTransStTy;
          *FieldType = DTransStTy->getFieldType(ElemIdx);
          assert(*FieldType && "Fields should be set for structure");
          *FieldIndex = ElemIdx;
          return true;
        }

        // The offset does not start a field in the 'DTransStTy'. Check
        // whether a field with that offset is within is an aggregate type
        // within the current structure.

        // The field number of the llvm::Type should be valid for the
        // DTransType, but make sure.
        if (ElemIdx >= DTransStTy->getNumFields())
          return false;

        // Adjust the offset to where the aggregate field begins.
        uint64_t NewOffset = OffsetInBytes - ElemOffset;
        DTransType *DTransFieldTy = DTransStTy->getFieldType(ElemIdx);
        assert(DTransFieldTy && "Expected fields to be set");

        if (auto *DTransFieldStTy = dyn_cast<DTransStructType>(DTransFieldTy))
          return GetFieldTypeAndIndex(DL, DTransFieldStTy, NewOffset,
                                      IndexedType, FieldType, FieldIndex);

        // If the field is an array, find the array type to check for a
        // structure type being indexed.
        if (DTransFieldTy->isArrayTy()) {
          while (DTransFieldTy->isArrayTy()) {
            llvm::Type *FieldTy = DTransFieldTy->getLLVMType();
            uint64_t FieldSize = DL.getTypeAllocSize(FieldTy);
            if (FieldSize == 0)
              return false;
            NewOffset = NewOffset % FieldSize;
            DTransFieldTy = DTransFieldTy->getArrayElementType();
          }

          // Handle an array of structures.
          if (auto *DTransFieldStTy = dyn_cast<DTransStructType>(DTransFieldTy))
            return GetFieldTypeAndIndex(DL, DTransFieldStTy, NewOffset,
                                        IndexedType, FieldType, FieldIndex);

          // The array was to a pointer type, or other non-structure type.
          *IndexedType = DTransStTy;
          *FieldType = DTransFieldTy;
          *FieldIndex = ElemIdx;
          return true;
        }

        return false;
      };

      bool ProcessedAsFlattenedGEP = false;
      // Check for the case of a GEP that indexes into a structure by using a
      // multiple of size of an atomic type, such as:
      //    %field = getelementptr ptr, ptr %in, i64 5
      if (GEP.getNumIndices() == 1 && !SrcTy->isAggregateType() &&
          PointerInfo->canAliasToAggregatePointer()) {
        auto *ConstOffset = dyn_cast<ConstantInt>(GEP.getOperand(1));
        if (ConstOffset) {
          size_t GEPSize = DL.getTypeAllocSize(SrcTy);
          int64_t OffsetInBytes = ConstOffset->getSExtValue() * GEPSize;

          for (auto *AliasTy :
               PointerInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use)) {
            if (!AliasTy->isPointerTy() ||
                !AliasTy->getPointerElementType()->isStructTy())
              continue;

            auto *DStTy =
                cast<DTransStructType>(AliasTy->getPointerElementType());
            auto *StTy = cast<llvm::StructType>(DStTy->getLLVMType());
            if (StTy->isSized()) {
              uint64_t StructSize = DL.getTypeAllocSize(StTy);
              if (OffsetInBytes > 0 && (uint64_t)OffsetInBytes < StructSize) {
                DTransStructType *IndexedType = nullptr;
                DTransType *FieldType;
                uint32_t FieldNum = 0;
                bool Identified =
                    GetFieldTypeAndIndex(DL, DStTy, OffsetInBytes, &IndexedType,
                                         &FieldType, &FieldNum);
                if (Identified) {
                  ResultInfo->addTypeAlias(
                      ValueTypeInfo::VAT_Decl,
                      TM.getOrCreatePointerType(FieldType));
                  ResultInfo->addElementPointee(ValueTypeInfo::VAT_Decl,
                                                IndexedType, FieldNum);
                  PTA.addFlattenedGEPMapping(&GEP, IndexedType, FieldNum,
                                             GEPSize);
                  ProcessedAsFlattenedGEP = true;
                } else {
                  // The GEP is inside of an aggregate type, but a field was
                  // not identified.
                  ResultInfo->setUnhandled();
                  DEBUG_WITH_TYPE_P(
                      FNFilter, VERBOSE_TRACE,
                      dbgs() << "GEP to unknown location within structure: "
                             << GEP << "\nStructure type: " << *DStTy << "\n");
                }
              }
            }
          }
        }
      }

      if (!ProcessedAsFlattenedGEP)
        propagate(PointerInfo, ResultInfo, /*Decl=*/true, /*Use=*/true,
                  DerefType::DT_SameType);
    }
  }

  // Check whether the GEP is a byte-flattened GEP or an array of 'i8' elements.
  // Returns 'true' if the GEP was processed as a byte-flattened GEP, even if it
  // is just to mark the 'ResultInfo' as being an unknown access. This is
  // necessary so that an access of the form:
  //   %x = getelementptr i8, i8* %pStruct, i64 %idx
  // does not propagate the type of %pStruct to the types collected for %x,
  // since the result should be an element within %pStruct, not a pointer to
  // %pStruct itself.
  //
  BFG_Kind analyzePotentialByteFlattenedGEPAccess(GEPOperator &GEP,
                                                  ValueTypeInfo *ResultInfo) {

    assert(GEP.getNumIndices() == 1 && "Only expecting single index value GEP");

    // The following conditions exclude it from being either a byte-flattened
    // GEP or an 'i8' array:
    // - The type alias list saw the type as a pointer-to-pointer, which is not
    //   for an element zero access.
    // - The type alias list does not contain any aggregate types
    ValueTypeInfo *BasePtrInfo = PTA.getOrCreateValueTypeInfo(&GEP, 0);
    bool HasStructType = false;
    bool HasArrayType = false;
    DTransType *IndexedType = nullptr;
    DTransType *CheckAsElemZeroType = nullptr;
    auto &AliasSet =
        BasePtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl);
    for (auto *AliasTy : AliasSet) {
      if (!AliasTy->isPointerTy())
        continue;

      // If there is a pointer-to-pointer alias, it could be an alias of the
      // element zero type. Save this to check later once we have a potential
      // type being used for the byte-flattened GEP.
      DTransType *ElemType = AliasTy->getPointerElementType();
      if (ElemType->isPointerTy()) {
        if (CheckAsElemZeroType)
          return BFG_None;

        CheckAsElemZeroType = AliasTy;
        continue;
      }

      if (ElemType->isStructTy()) {
        HasStructType = true;
        if (IndexedType)
          return BFG_None;
        IndexedType = AliasTy;
      } else if (ElemType->isArrayTy()) {
        HasArrayType = true;
        if (IndexedType)
          return BFG_None;
        IndexedType = AliasTy;

        // Also, check for array of structures.
        DTransType *BaseType =
            getSequentialObjectBaseType(cast<DTransSequentialType>(ElemType));
        if (BaseType->isStructTy())
          HasStructType = true;
      }
    }

    if (!HasStructType && !HasArrayType)
      return BFG_None;

    if (CheckAsElemZeroType)
      if (!PTA.isElementZeroAccess(IndexedType, CheckAsElemZeroType))
        return BFG_None;

    // We need to figure out which element of the aggregate is being accessed.
    //
    // Get a list of possible constant offsets. Normally, there will just be a
    // single constant offset amount based on the GEP indices. However, it's
    // possible the index is in a variable that can be traced back to a specific
    // set of constants, in which case we need to collect all the constants.
    unsigned BitWidth = DL.getPointerSizeInBits();
    SmallVector<APInt, 3> APOffset;
    APInt CurrOffset(BitWidth, 0);
    if (GEP.accumulateConstantOffset(DL, CurrOffset)) {
      APOffset.push_back(CurrOffset);
    } else {
      // If offset comes from a select instruction with two constant operands
      // then put both values in the list of possible constant offsets.
      // TODO: PHINodes could be added in the future.
      Value *Cond;
      const APInt *SelT, *SelF;
      if (PatternMatch::match(
              GEP.getOperand(1),
              PatternMatch::m_Select(PatternMatch::m_Value(Cond),
                                     PatternMatch::m_APInt(SelT),
                                     PatternMatch::m_APInt(SelF)))) {
        APOffset.push_back(*SelT);
        APOffset.push_back(*SelF);
      }
    }

    // If the offsets are not known, a byte flattened GEP cannot be collected
    // for it.
    if (APOffset.empty()) {
      // The safety analyzer will need to set a safety flag if a structure type
      // is involved. Array types will just be propagated as the same type as
      // the original GEP since it will still be an array type, even though the
      // offset will not be known, so there is no need to set a flag for those.
      if (HasStructType) {
        DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE,
                          dbgs()
                              << "Byte-flattened indices were not recoverable: "
                              << GEP << "\n");
        ResultInfo->setUnknownByteFlattenedGEP();
        return BFG_Unknown;
      }

      return BFG_None;
    }

    // If it's just a char array being accessed, then add the element based on
    // the offsets collected.
    DTransArrayType *AggArType = nullptr;
    if (PTA.isPtrToCharArray(*BasePtrInfo, &AggArType)) {
      for (auto &APOffsetVal : APOffset)
        ResultInfo->addElementPointee(ValueTypeInfo::VAT_Use, AggArType,
                                      APOffsetVal.getLimitedValue());

      return BFG_Identified;
    }

    // If any of the offsets are negative, and it did not match the above check
    // for ptr-to-ptr, then it is not a byte-flattened GEP. However, it is
    // unknown what the pointer is pointing at, so mark it to trigger a safety
    // check. This sort of IR can be triggered from source code that tries to do
    // pointer arithmetic such as:
    //    DOMElementImpl* dummy = 0;
    //    size_t parentOffset = (char *)&(dummy->fParent) - (char *)dummy;
    //    char *retPtr = (char *)p - parentOffset;
    //    return (DOMNode *)retPtr;
    if (std::any_of(APOffset.begin(), APOffset.end(),
                    [](APInt &Offset) { return Offset.isNegative(); })) {

      if (HasStructType) {
        DEBUG_WITH_TYPE_P(
            FNFilter, VERBOSE_TRACE,
            dbgs() << "Byte-flattened indices contained negative index: " << GEP
                   << "\n");
        ResultInfo->setUnknownByteFlattenedGEP();
        return BFG_Unknown;
      }

      return BFG_None;
    }

    // Try all possible offsets to process the address as a byte-flattened GEP.
    // Accumulate the potential alias elements and byte-flattened GEP info
    // into temporary objects until all offsets have been validated for all
    // aliased types.
    ValueTypeInfo LocalInfo(nullptr);
    SmallVector<std::pair<DTransType *, size_t>, 4> PendingByteGEPs;
    bool AllValid = true;
    for (auto *AliasTy :
         BasePtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use)) {
      if (!AllValid)
        break;

      if (!AliasTy->isPointerTy())
        continue;

      DTransType *ElemType = AliasTy->getPointerElementType();
      if (!ElemType->isAggregateType())
        continue;

      for (auto &APOffsetVal : APOffset) {
        if (!analyzePossibleOffsetAggregateAccess(GEP, ElemType,
                                                  APOffsetVal.getLimitedValue(),
                                                  LocalInfo, PendingByteGEPs)) {
          AllValid = false;
          break;
        }
      }
    }

    if (!AllValid) {
      DEBUG_WITH_TYPE_P(
          FNFilter, VERBOSE_TRACE,
          dbgs() << "Byte-flattened indices did not match aliased structure: "
                 << GEP << "\n");
      ResultInfo->setUnknownByteFlattenedGEP();
      return BFG_Unknown;
    }

    // All offsets were valid, merge the element pointees collected into
    // ResultInfo, and save the byte flattened GEP information for use by the
    // safety analyzer.
    propagate(&LocalInfo, ResultInfo, /*Decl=*/true, /*Use=*/true,
              DerefType::DT_SameType);
    for (auto &TyIdxPair : PendingByteGEPs)
      PTA.addByteFlattenedGEPMapping(&GEP, TyIdxPair.first, TyIdxPair.second);

    return BFG_Identified;
  }

  // Return 'true' if 'Offset' is the address of an element of 'AggregateTy'.
  // - update 'Info' to contain an alias of the type accessed and the element
  //   pointee.
  // - update PendingByteGEPs with the type and index accessed.
  bool analyzePossibleOffsetAggregateAccess(
      GEPOperator &GEP, DTransType *AggregateTy, uint64_t Offset,
      ValueTypeInfo &Info,
      SmallVectorImpl<std::pair<DTransType *, size_t>> &PendingByteGEPs) {

    // For analyzing the offset, get the corresponding type within
    // the LLVM type system. Since we do need to resolve the types of any
    // contained pointers when checking the offsets, this allows use of the
    // DataLayout and StructLayout of the LLVM types to simplify the
    // implementation.
    llvm::Type *IRType = AggregateTy->getLLVMType();
    if (!IRType || !IRType->isAggregateType() || !IRType->isSized())
      return false;

    if (auto *StructTy = dyn_cast<StructType>(IRType))
      return analyzePossibleOffsetStructureAccess(
          GEP, cast<DTransStructType>(AggregateTy), StructTy, Offset, Info,
          PendingByteGEPs);

    return analyzePossibleOffsetArrayAccess(
        GEP, cast<DTransArrayType>(AggregateTy), cast<ArrayType>(IRType),
        Offset, Info, PendingByteGEPs);
  }

  // Return 'true' if 'Offset' is the address of an element within 'StructTy'.
  // If so, also update 'Info' and 'PendingByteGEPs'.
  bool analyzePossibleOffsetStructureAccess(
      GEPOperator &GEP, DTransStructType *DTStructTy,
      llvm::StructType *StructTy, uint64_t Offset, ValueTypeInfo &Info,
      SmallVectorImpl<std::pair<DTransType *, size_t>> &PendingByteGEPs) {

    // If the offset is beyond the end of the structure, this isn't a match.
    auto *SL = DL.getStructLayout(StructTy);
    if (Offset >= SL->getSizeInBytes())
      return false;

    // See which element in the structure would contain this offset.
    unsigned IdxAtOffset = SL->getElementContainingOffset(Offset);
    DTransType *FieldType = DTStructTy->getFieldType(IdxAtOffset);
    if (!FieldType)
      return false;

    // If the containing element does not begin at that offset, this isn't a
    // match.
    uint64_t ElementOffset = SL->getElementOffset(IdxAtOffset);
    if (ElementOffset != Offset) {
      // If the element at that offset is an aggregate type, we may be accessing
      // an element within the nested type.
      if (FieldType->isAggregateType()) {
        return analyzePossibleOffsetAggregateAccess(
            GEP, FieldType, Offset - ElementOffset, Info, PendingByteGEPs);
      } else if (dtrans::valueOnlyUsedForMemset(&GEP)) {
        Info.addElementPointeeByOffset(ValueTypeInfo::VAT_Decl, DTStructTy,
                                       Offset);
        return true;
      } else {
        return false;
      }
    }

    // Otherwise, this is a match.
    PendingByteGEPs.push_back({DTStructTy, IdxAtOffset});
    Info.addElementPointee(ValueTypeInfo::VAT_Decl, DTStructTy, IdxAtOffset);
    Info.addTypeAlias(ValueTypeInfo::VAT_Decl,
                      TM.getOrCreatePointerType(FieldType));
    return true;
  }

  // Return 'true' if 'Offset' is the address of an element of 'ArrayTy'.
  // If so, also update 'Info' and 'PendingByteGEPs'.
  bool analyzePossibleOffsetArrayAccess(
      GEPOperator &GEP, DTransArrayType *DTArrayType, llvm::ArrayType *ArrayTy,
      uint64_t Offset, ValueTypeInfo &Info,
      SmallVectorImpl<std::pair<DTransType *, size_t>> &PendingByteGEPs) {

    DTransType *DTransElemTy = DTArrayType->getArrayElementType();
    llvm::Type *ElemTy = ArrayTy->getElementType();
    uint64_t ElementSize = DL.getTypeAllocSize(ElemTy);
    uint64_t NewOffset = Offset % ElementSize;
    if (NewOffset == 0) {
      // The offset is an exact multiple of the element size. This
      // is a match for the element access.
      PendingByteGEPs.push_back({DTArrayType, Offset / ElementSize});
      Info.addElementPointee(ValueTypeInfo::VAT_Decl, DTArrayType,
                             Offset / ElementSize);
      Info.addTypeAlias(ValueTypeInfo::VAT_Decl,
                        TM.getOrCreatePointerType(DTransElemTy));
      return true;
    }

    // Otherwise, we may be accessing a sub-element within a nested aggregate.
    return analyzePossibleOffsetAggregateAccess(GEP, DTransElemTy, NewOffset,
                                                Info, PendingByteGEPs);
  }

  // There's an odd case where LLVM's constant folder will transform
  // a bitcast into a GEP if the first element of the structure at
  // any level of nesting matches the type of the bitcast. Normally
  // this is good, but if the first element is an i8 (or a fixed array
  // of i8, or a nested structure whose first element is an i8, etc.)
  // then this folding can lead to a misleading GEP where the code
  // was actually just trying to obtain a void pointer to the structure.
  //
  // For that reason, if this GEP returns an i8* and all but its first
  // index arguments are zero, we want to include the base structure
  // type in the alias set. The first index argument does not index
  // into the structure but offsets from it (as a dynamic array) so
  // this case applies even if the first index is non-zero.
  //
  // TODO: This is here for compatibility with the existing local pointer
  // analyzer for matching the output of the analyzed types. This may need
  // to be revisited when the compiler actually is using opaque pointers
  // because in that case the rest of the compiler would not be making a
  // distinction between an i8* and any other pointer type. An alternative,
  // may be to infer the type from its uses when that is implemented.
  void analyzeGEPAsBitcastEquivalent(GEPOperator &GEP,
                                     ValueTypeInfo *ResultInfo) {
    // A GEP with a single index will be handled by the byte-flattened GEP
    // processing.
    if (GEP.getNumIndices() == 1)
      return;

    for (unsigned i = 1; i < GEP.getNumIndices(); ++i)
      // The +1 here is because the first operand of a GEP is not an index.
      if (ConstantInt *CI = dyn_cast<ConstantInt>(GEP.getOperand(i + 1)))
        if (!CI->isZero())
          return;

    bool MayBeI8Ptr = false;
    for (auto *AliasTy :
         ResultInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl))
      if (AliasTy == PTA.getDTransI8PtrType()) {
        MayBeI8Ptr = true;
        break;
      }

    if (!MayBeI8Ptr)
      return;

    ValueTypeInfo *PtrInfo =
        PTA.getOrCreateValueTypeInfo(GEP.getPointerOperand());
    for (auto *AliasTy :
         PtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl))
      ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl, AliasTy);

    if (!PtrInfo->isCompletelyAnalyzed())
      ResultInfo->setPartiallyAnalyzed();
  }

  void analyzeIntToPtrInst(IntToPtrInst *ITP, ValueTypeInfo *ResultInfo) {
    // Compiler constants can be ignored because the only way an integer
    // value can have pointer type information is if the Value object
    // originated from a ptrtoint instruction.
    Value *Src = ITP->getOperand(0);
    if (!isa<Constant>(Src)) {
      // Propagate pointer type information that has been identified for the
      // source operand to the result info, if there is any.
      ValueTypeInfo *SrcInfo = PTA.getValueTypeInfo(Src);
      if (SrcInfo) {
        propagate(SrcInfo, ResultInfo, true, true, DerefType::DT_SameType);
        if (SrcInfo->getUnhandled() || SrcInfo->getDependsOnUnhandled())
          ResultInfo->setDependsOnUnhandled();

        if (!SrcInfo->isCompletelyAnalyzed())
          ResultInfo->setPartiallyAnalyzed();
      }
    }

    // If the source operand was evaluated as having been converted from a
    // pointer type, that type will have been propagated as the result type of
    // this inttoptr conversion. Also look at the users to try to infer a type
    // for the pointer.
    // For example:
    //   %i = ptrtoint %struct.foo* to i64
    //   %p = inttoptr i64 %i to %struct.foobar*
    //
    // Or:
    //   %alloc = call %struct.foo* @foo_allocator()
    //   %i = ptrtoint %struct.foo* %alloc to i64
    //   %r2 = ashr i64 %i, 2
    //   %l8 = shl i64 %r2, 2
    //   %a8 = add i64 %l8, 8
    //   %p = inttoptr i64 %a8 to %struct.foo*
    //
    // These cases may result in the DTransSafetyAnalyzer marking the type as
    // unhandled use or bad casting when it processes the IR.
    auto It = PendingValueToInferredTypes.find(ITP);
    if (It != PendingValueToInferredTypes.end())
      for (auto Ty : It->second)
        ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Use, Ty);
  }

  void analyzeExtractValueInst(ExtractValueInst *EV,
    ValueTypeInfo *ResultInfo) {
    if (!isTypeOfInterest(EV->getType()))
      return;

    // Currently, only handle cases with a single index value.
    if (EV->getNumIndices() > 1) {
      ResultInfo->setUnhandled();
      LLVM_DEBUG(
        dbgs() << "Unhandled ExtractValueInst due to number of indices: "
        << *EV << "\n");
      return;
    }

    Value *Src = EV->getAggregateOperand();
    unsigned Idx = EV->getAggregateOperandIndex();
    ValueTypeInfo *SrcInfo = PTA.getOrCreateValueTypeInfo(Src);
    for (auto Alias :
      SrcInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl)) {
      if (!Alias->isAggregateType())
        continue;

      if (auto *DStTy = dyn_cast<DTransStructType>(Alias)) {
        DTransFieldMember &Field = DStTy->getField(Idx);
        for (auto *FieldTTy : Field.getTypes())
          ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl, FieldTTy);
        continue;
      }
      else if (auto *DSeqTy = dyn_cast<DTransSequentialType>(Alias)) {
        ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl,
          DSeqTy->getTypeAtIndex(0));
      }
      else {
        ResultInfo->setUnhandled();
        LLVM_DEBUG(dbgs() << "Unahndled ExtractValue: " << *EV << "\n");
      }
    }
  }

  // The result type of a 'freeze' instruction is the same as the source operand
  // type. If there is pointer information tracked for the source operand,
  // propagate it to 'ResultInfo'.
  void analyzeFreezeInst(FreezeInst *FI, ValueTypeInfo *ResultInfo) {
    ValueTypeInfo *SrcInfo = PTA.getValueTypeInfo(FI, 0);
    if (!SrcInfo)
      return;

    propagate(SrcInfo, ResultInfo, /*Decl=*/true, /*Use=*/true,
              DerefType::DT_SameType);
    if (!SrcInfo->isCompletelyAnalyzed())
      ResultInfo->setPartiallyAnalyzed();

    if (SrcInfo->getUnhandled() || SrcInfo->getDependsOnUnhandled())
      ResultInfo->setDependsOnUnhandled();
  }

  void analyzeLandingPadInst(LandingPadInst *LP, ValueTypeInfo *ResultInfo) {
    ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl, getDTransLandingPadTy());
  }

  void analyzeLoadInst(LoadInst *LI, ValueTypeInfo *ResultInfo) {
    // We cannot just take the value type of the load, and set that as
    // the type of value produced because the pointer may be a
    // pointer-to-pointer of some type.
    //   %val = load i64, ptr %ptr
    // In this case, if %ptr referred to a %struct.foo**, then the type of %val
    // needs to include %struct.foo*. Therefore, we propagate whatever type info
    // was available for the source pointer to the load result, with 1 less
    // level of indirection.
    //
    // Also, need to be able to handle the special case of:
    //   %143 = load i64, ptr undef
    //

    // This lambda function is used for cases where the load is a pointer type,
    // or may be a pointer type loaded as a pointer-sized int.
    //
    // Case 1) %y = load ptr, ptr %x
    // Case 2) %y = load i64, ptr %x
    //   If %x is a pointer to a pointer, then the pointer type should be
    //   propagated to %y.
    //   If %x is a pointer to an aggregate, this could be an element-zero
    //   access. If an element-zero can be resolved to a pointer type after
    //   traversing any nested elements, add that type to %y, and update the
    //   element pointees on the PointerInfo.
    //
    // Case 3) %y = load { ptr, ptr }, { ptr, ptr }* %x
    //   In this case, the call sets 'LoadingAggregateType' to indicate the
    //   structure should not be traversed as a potential zero-element load.
    //
    auto &LocalTM = this->TM;
    auto PropagateDereferencedType =
        [&LocalTM](ValueTypeInfo *PointerInfo, ValueTypeInfo *ResultInfo,
                   ValueTypeInfo::ValueAnalysisType Kind,
                   bool LoadingAggregateType) {
          // This may also identify that it appears that an element-zero
          // location of an aggregate is being loaded. In this case the
          // PointerInfo will be updated to reflect that the pointer operand is
          // being used as an element-pointee.
          SmallVector<DTransType *, 4> PendingTypes;
          for (auto *Alias : PointerInfo->getPointerTypeAliasSet(Kind)) {
            DTransType *PropAlias = nullptr;
            if (!Alias->isPointerTy())
              continue;

            PropAlias = Alias->getPointerElementType();
            // CMPLRLLVM-32994: If the structure is opaque (structure without
            // body), then we can't collect field 0.
            //
            // TODO: We still need to expand this analysis to handle the case
            // when a pointer to an opaque structure is being cast to a
            // pointer to a structure with body. For example:
            //
            // %struct.test01 = type opaque
            // %struct.test02 = type { %struct.test02*, i32 }
            //
            // define internal void @foo(%struct.test01* %arg) {
            // entry:
            //   %tmp0 = bitcast %struct.test01* %arg to %struct.test02**
            //   %tmp1 = load %struct.test02*, %struct.test02** %tmp0
            //   ret void
            // }
            //
            // Function @foo will look as follows in the case of opaque
            // pointers:
            //
            // define internal void @foo(ptr %arg) {
            // entry:
            //   %tmp0 = load ptr, ptr %arg
            //   ret void
            // }
            //
            // Notice that the bitcast instruction is gone and the pointer
            // analyzer will assume that instruction %tmp0 is loading a
            // pointer to %struct.test01 rather than %struct.test02.
            if (!LoadingAggregateType && PropAlias->isAggregateType() &&
                PropAlias->getNumContainedElements() != 0) {
              // If the pointer was a pointer to an aggregate and we are not
              // expecting an aggregate type to be loaded (i.e. an array, such
              // as [2xi16*], or a literal structure, such as {i32, i32}, then
              // this could be an element zero load of a pointer within a nested
              // type. Try to find the type.
              DTransType *PrevNestedType = nullptr;
              DTransType *NestedType = PropAlias;
              while (NestedType && !NestedType->isPointerTy()) {
                PrevNestedType = NestedType;
                if (auto *StTy = dyn_cast<DTransStructType>(NestedType))
                  NestedType = StTy->getFieldType(0);
                else if (auto *ArTy = dyn_cast<DTransArrayType>(NestedType))
                  NestedType = ArTy->getElementType();
                else
                  NestedType = nullptr;
              }

              if (PrevNestedType->isAggregateType())
                PointerInfo->addElementPointee(ValueTypeInfo::VAT_Use,
                                               PrevNestedType, 0);
              if (NestedType) {
                ResultInfo->addTypeAlias(Kind, NestedType);
                PendingTypes.push_back(LocalTM.getOrCreatePointerType(NestedType));
              }
            } else {
              ResultInfo->addTypeAlias(Kind, PropAlias);
            }
          }

          for (auto *Ty : PendingTypes)
            PointerInfo->addTypeAlias(ValueTypeInfo::VAT_Use, Ty);
        };

    llvm::Type *ValTy = LI->getType();

    bool ExpectPtrType = dtrans::hasPointerType(ValTy);
    ValueTypeInfo *PointerInfo = PTA.getOrCreateValueTypeInfo(LI, 0);
    bool LoadingAggregateType = ValTy->isAggregateType();
    if (ExpectPtrType || ValTy == PTA.getLLVMPointerSizedIntType()) {
      PropagateDereferencedType(PointerInfo, ResultInfo,
                                ValueTypeInfo::ValueAnalysisType::VAT_Decl,
                                LoadingAggregateType);
      PropagateDereferencedType(PointerInfo, ResultInfo,
                                ValueTypeInfo::ValueAnalysisType::VAT_Use,
                                LoadingAggregateType);
    }

    // Update the usage type of the pointer argument based on the type
    // of the load instruction if we are loading a known type.
    llvm::Type *LoadType = LI->getType();
    if (!PointerInfo->getIsPartialPointerUse() &&
        !dtrans::hasPointerType(LoadType)) {
      PointerInfo->addTypeAlias(
          ValueTypeInfo::VAT_Use,
          TM.getOrCreatePointerType(TM.getOrCreateSimpleType(LoadType)));
    }

    checkForElementZeroAccess(LI, ValTy, PointerInfo,
                              ValueTypeInfo::ValueAnalysisType::VAT_Decl);

    if (PointerInfo->getUnhandled() || PointerInfo->getDependsOnUnhandled())
      ResultInfo->setDependsOnUnhandled();

    if (!PointerInfo->isCompletelyAnalyzed())
      ResultInfo->setPartiallyAnalyzed();
  }

  // Transfer alias info from one ValueTypeInfo object to another, optionally
  // with the level of pointer indirection being one level higher or lower.
  //
  // When \p Decl is 'true', propagates declared type set.
  // When \p Use is 'true', propagates the usage type set.
  bool propagate(ValueTypeInfo *SrcInfo, ValueTypeInfo *DestInfo, bool Decl,
                 bool Use, DerefType DerefLevel) {
    // Helper routine that will get/create a DTransType to represent with a
    // potential change to level of indirection by at most one level (up or
    // down).
    auto &TM = this->TM;
    auto GetOrCreateTypeWithDerefLevel =
        [&TM](DTransType *Ty, DerefType DerefLevel) -> DTransType * {
      if (DerefLevel == DerefType::DT_SameType)
        return Ty;

      // Create a pointer to the element
      if (DerefLevel == DerefType::DT_PointerToType)
        return TM.getOrCreatePointerType(Ty);

      // Return the underlying type of the pointer.
      auto *PTy = dyn_cast<DTransPointerType>(Ty);
      if (!PTy)
        return nullptr;
      return PTy->getPointerElementType();
    };

    // Helper that propagates either the 'declared' or 'usage' type set.
    auto DoPropagate = [&](ValueTypeInfo::ValueAnalysisType Kind) {
      bool LocalChanged = false;
      for (auto *Alias : SrcInfo->getPointerTypeAliasSet(Kind)) {
        DTransType *PropAlias =
            GetOrCreateTypeWithDerefLevel(Alias, DerefLevel);

        // If all the types could be completely resolved, and there were no
        // bitcasts that changed the level of indirection, we should always be
        // able to create the right type to propagate. Unfortunately, because a
        // bitcast can be used to change the level of indirection, it's possible
        // that trying to propagate the type with a decrease in the level of
        // indirection will not be possible. We do not treat this as an
        // unhandled situation because it's possible that we do not have the
        // type because there was not an actual use in the IR where the value
        // gets used as the unexpected level of indirection.
        if (PropAlias)
          LocalChanged |= DestInfo->addTypeAlias(Kind, PropAlias);
        else
          LLVM_DEBUG({
            dbgs() << "Warning: Could not create element of requested "
                      "deref level when propagating.\nFrom: ";
            SrcInfo->dump();
            dbgs() << "\nTo: ";
            DestInfo->dump();
            dbgs() << "\n";
          });
      }

      // The element pointer set does not need to be transferred when the level
      // of indirection is being changed. For example:
      //   %x = getelementptr %struct.node, ptr %y, i64 0, i32 2
      //   %z = load ptr, ptr %x
      // %x contains a pointer alias type and an element pointee of a structure,
      // but %z is just being updated to be the pointee type from %x.
      if (DerefLevel == DerefType::DT_SameType)
        for (auto PointeePair : SrcInfo->getElementPointeeSet(Kind))
          LocalChanged |= DestInfo->addElementPointee(Kind, PointeePair);

      return LocalChanged;
    };

    bool Changed = false;
    if (Decl)
      Changed |= DoPropagate(ValueTypeInfo::VAT_Decl);
    if (Use)
      Changed |= DoPropagate(ValueTypeInfo::VAT_Use);

    return Changed;
  }

  void checkForElementZeroAccess(Instruction *I, llvm::Type *ValTy,
                                 ValueTypeInfo *PointerInfo,
                                 ValueTypeInfo::ValueAnalysisType Kind) {
    for (auto *Alias : PointerInfo->getPointerTypeAliasSet(Kind)) {
      DTransType *PropAlias = nullptr;
      if (!Alias->isPointerTy())
        continue;

      PropAlias = Alias->getPointerElementType();
      if (PropAlias->isAggregateType() &&
          PropAlias->getNumContainedElements() != 0) {
        DTransType *PrevNestedType = nullptr;
        DTransType *NestedType = PropAlias;
        unsigned int Depth = 0;
        while (NestedType && NestedType->isAggregateType()) {
          PrevNestedType = NestedType;
          ++Depth;
          if (auto *StTy = dyn_cast<DTransStructType>(NestedType))
            NestedType = StTy->getFieldType(0);
          else if (auto *ArTy = dyn_cast<DTransArrayType>(NestedType))
            NestedType = ArTy->getElementType();
          else
            NestedType = nullptr;
        }

        if (NestedType && PrevNestedType->isAggregateType() &&
            (ValTy->isPointerTy() ||
             ValTy == PTA.getLLVMPointerSizedIntType() ||
             NestedType->getLLVMType() == ValTy)) {
          DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE,
                            dbgs() << "Element-zero on: ["
                                   << I->getFunction()->getName() << "]" << *I
                                   << "\n"
                                   << *PropAlias << " Depth=" << Depth << "\n");
          PTA.addElementZeroPointer(I, PropAlias, Depth);
        }
      }
    }
  }

  void analyzePHINode(PHINode *PHI, ValueTypeInfo *ResultInfo) {
    SmallVector<Value *, 4> IncomingVals;
    for (Value *Val : PHI->incoming_values())
      IncomingVals.push_back(Val);

    analyzeSelectOrPhi(IncomingVals, ResultInfo);
  }

  void analyzePtrToIntOperator(PtrToIntOperator *PTI,
                               ValueTypeInfo *ResultInfo) {
    Value *Src = PTI->getPointerOperand();
    if (isCompilerConstant(Src)) {
      // The source pointer could be any type. We could try to infer the type by
      // looking for a conversion of the value back to a pointer, but that is
      // not likely to occur when the source value was a null/undef value. If
      // the value is unused though, it can be ignored.
      if (!PTI->users().empty()) {
        ResultInfo->setUnhandled();
        LLVM_DEBUG(dbgs() << "PtrToInt from constant is not handled: " << *PTI
                          << "\n");
      }
      return;
    }

    // Treat the resulting object to be the same type as the source pointer
    ValueTypeInfo *SrcInfo = PTA.getOrCreateValueTypeInfo(Src);
    propagate(SrcInfo, ResultInfo, true, true, DerefType::DT_SameType);

    if (SrcInfo->getUnhandled() || SrcInfo->getDependsOnUnhandled())
      ResultInfo->setDependsOnUnhandled();

    if (!SrcInfo->isCompletelyAnalyzed())
      ResultInfo->setPartiallyAnalyzed();
  }

  void analyzeSelectInst(SelectInst *Sel, ValueTypeInfo *ResultInfo) {
    SmallVector<Value *, 4> IncomingVals;
    IncomingVals.push_back(Sel->getTrueValue());
    IncomingVals.push_back(Sel->getFalseValue());

    analyzeSelectOrPhi(IncomingVals, ResultInfo);
  }

  void analyzeSelectOrPhi(SmallVectorImpl<Value *> &IncomingVals,
                          ValueTypeInfo *ResultInfo) {
    SmallPtrSet<GlobalObject *, 4> PossibleGlobalWithElidedGEP;
    bool HasNonGlobalObject = false;
    for (auto *ValIn : IncomingVals) {
      // It's possible the operand to the select or phi is 0 or null, ignore
      // these because they do not supply type information useful for
      // determining the type of the result.
      if (isCompilerConstant(ValIn))
        continue;

      // Defer processing of Globals merged with other values because it's
      // possible that the actual use is for the element zero rather than the
      // type of global variable, when the GEP has been elided. In this case,
      // first merge the information from the other inputs, and then we will
      // check if the global should merged using the type of Global, or as an
      // element-zero access type.
      //
      // For example:
      //   @GetPageGeometry.PageSizes = constant [76 x [2 x ptr]]
      //   i10 = phi ptr [ @GetPageGeometry.PageSizes, %bb ], [ %i4, %bb2 ]
      //
      // When %i4 is identified as type [2 x i8*], then we want to conclude that
      // there is an elided element zero GEP on the global variable, rather than
      // treating the result type as also containing [76 x [2 x i8*]]
      if (auto *GO = dyn_cast<GlobalObject>(ValIn))
        if (GO->getValueType()->isAggregateType()) {
          PossibleGlobalWithElidedGEP.insert(GO);
          continue;
        }

      HasNonGlobalObject = true;
      ValueTypeInfo *SrcInfo = PTA.getOrCreateValueTypeInfo(ValIn);
      propagate(SrcInfo, ResultInfo, true, true, DerefType::DT_SameType);

      if (SrcInfo->getUnhandled() || SrcInfo->getDependsOnUnhandled())
        ResultInfo->setDependsOnUnhandled();

      if (!SrcInfo->isCompletelyAnalyzed())
        ResultInfo->setPartiallyAnalyzed();
    }

    if (!PossibleGlobalWithElidedGEP.empty()) {
      // These sets hold the info that needs to be added from the global
      // variables. In order to ensure the updates to ResultInfo are
      // deterministic, we need to check all the Globals against the ResultInfo
      // before we start adding types into the ResultInfo.
      SmallPtrSet<ValueTypeInfo*, 4> InfosToMerge;
      SmallPtrSet<DTransType*, 4> ElementZeroPointeesToAdd;
      for (auto *GO : PossibleGlobalWithElidedGEP) {
        ValueTypeInfo *SrcInfo = PTA.getOrCreateValueTypeInfo(GO);
        bool IsElementZero = false;
        if (HasNonGlobalObject) {
          DTransType *GOType =
              PTA.getDominantAggregateType(*SrcInfo, ValueTypeInfo::VAT_Decl);
          if (GOType) {
            for (auto AliasTy :
                 ResultInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use)) {
              if (AliasTy == GOType)
                continue;

              DTransType* AccessedType = nullptr;
              if (PTA.isElementZeroAccess(GOType, AliasTy, &AccessedType)) {
                IsElementZero = true;
                ElementZeroPointeesToAdd.insert(AccessedType);
              }
            }
          }
        }

        if (!IsElementZero || !HasNonGlobalObject)
          InfosToMerge.insert(SrcInfo);

        // Check for the possibility that the Global object was missing
        // metadata. There is no need to check for "Depends on unhandled" or
        // "Completely analyzed" because Globals are completely processed
        // based on their ValueType and metadata.
        if (SrcInfo->getUnhandled())
          ResultInfo->setDependsOnUnhandled();
      }

      for (auto *Info : InfosToMerge)
        propagate(Info, ResultInfo, true, true, DerefType::DT_SameType);

      for (auto *Ty : ElementZeroPointeesToAdd)
        ResultInfo->addElementPointee(ValueTypeInfo::VAT_Use, Ty, 0);
    }
  }

  // An instruction that is effectively computing "Pointer - Constant", should
  // propagate the information about the source pointer to "ResultInfo".
  // All other cases, including "Constant - Pointer" do not result in the
  // "ResultInfo" being tracked as a pointer type.
  void analyzeSubInst(BinaryOperator *BinOp, ValueTypeInfo *ResultInfo) {
    assert(BinOp->getOpcode() == Instruction::Sub &&
           "Expected Sub instruction");
    if (!isa<ConstantInt>(BinOp->getOperand(1)))
      return;

    ValueTypeInfo *SrcInfo = PTA.getOrCreateValueTypeInfo(BinOp->getOperand(0));
    propagate(SrcInfo, ResultInfo, true, true, DerefType::DT_SameType);
    if (SrcInfo->getUnhandled() || SrcInfo->getDependsOnUnhandled())
      ResultInfo->setDependsOnUnhandled();

    if (!SrcInfo->isCompletelyAnalyzed())
      ResultInfo->setPartiallyAnalyzed();
  }

  // Perform pointer type analysis for constant operator expressions
  void analyzeConstantExpr(ConstantExpr *CE) {
    // This verbose trace is not using the filtering predicate test because
    // constant expressions are at the module level, not function level.
    DEBUG_WITH_TYPE(VERBOSE_TRACE, {
      dbgs() << "--\n";
      dbgs() << "Begin analyzeConstantExpr for: ";
      printValue(dbgs(), CE);
      dbgs() << "\n";
    });

    ValueTypeInfo *Info = PTA.getOrCreateValueTypeInfo(CE);
    if (auto *GEPOp = dyn_cast<GEPOperator>(CE)) {
      analyzeGetElementPtrOperator(GEPOp, Info);
      Info->setCompletelyAnalyzed();
    } else if (auto *BCOp = dyn_cast<BitCastOperator>(CE)) {
      analyzeValue(BCOp);
      Info->setCompletelyAnalyzed();
    } else if (auto *PTI = dyn_cast<PtrToIntOperator>(CE)) {
      analyzePtrToIntOperator(PTI, Info);
      Info->setCompletelyAnalyzed();
    } else {
      Info->setUnhandled();
      LLVM_DEBUG(dbgs() << "Unhandled constant expression: " << *CE << "\n");
    }

    for (auto *U : CE->users())
      if (auto *UCE = dyn_cast<ConstantExpr>(U))
        analyzeConstantExpr(UCE);

    DEBUG_WITH_TYPE(VERBOSE_TRACE, {
      dbgs() << "End analyzeConstantExpr for: ";
      printValue(dbgs(), CE);
      dbgs() << "\n";
    });
  }

  DTransStructType *getDTransLandingPadTy() const {
    return DTransLandingPadTy;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Start of member data
  ////////////////////////////////////////////////////////////////////////////////

  PtrTypeAnalyzerImpl &PTA;
  DTransTypeManager &TM;
  TypeMetadataReader &MDReader;
  DTransLibraryInfo &DTransLibInfo;
  DTransAllocCollector &DTAC;
  const DataLayout &DL;
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;

  // A landing pad instruction generates a literal structure of the form:
  // { i8*, i32 }. Represent this in the DTransType system.
  DTransStructType *DTransLandingPadTy;

  // Keep a mapping of a set of types that are inferred for a Value that is
  // constructed when a type needs to be inferred by doing a look-ahead walk of
  // the users of the value.
  std::map<Value *, SmallPtrSet<DTransType *, 4>> PendingValueToInferredTypes;
};

////////////////////////////////////////////////////////////////////////////////
//
// Implementation of ValueTypeInfo class
//
////////////////////////////////////////////////////////////////////////////////

bool ValueTypeInfo::addTypeAlias(ValueAnalysisType Kind, DTransType *Ty) {
  // Integer and floating point types are not tracked, since those can be
  // readily identified from the IR.
  if (!Ty->isPointerTy() && !Ty->isAggregateType() && !Ty->isVectorTy())
    return false;

  bool Changed = PointerTypeAliases[Kind].insert(Ty).second;
  if (Changed) {
    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
      dbgs() << "    Added alias " << (Kind == VAT_Decl ? "[DECL]" : "[USE]")
             << ": ";
      printValue(dbgs(), V);
      dbgs() << " -- " << *Ty << "\n";
    });

    // Check to see if this is a pointer (at any level of indirection) to
    // an aggregate type. That will make it faster later to tell if a value
    // is interesting or not.
    DTransType *Base = Ty;
    while (Base->isPointerTy())
      Base = Base->getPointerElementType();
    while (Base->isVectorTy())
      Base = Base->getVectorElementType();

    if (Ty->isPointerTy() && Ty->getPointerElementType()->isAggregateType())
      ++DirectAggregatePointerAliasCount[Kind];
    if (Base->isAggregateType())
      ++AggregatePointerAliasCount[Kind];

    if (Kind == VAT_Decl)
      addTypeAlias(VAT_Use, Ty);
  }

  return Changed;
}

bool ValueTypeInfo::addElementPointee(ValueAnalysisType Kind,
                                      DTransType *BaseTy, size_t ElemIdx) {

  PointeeLoc Loc(PointeeLoc::PLK_Field, ElemIdx);
  bool Changed = addElementPointeeImpl(Kind, BaseTy, Loc);
  return Changed;
}

bool ValueTypeInfo::addElementPointee(
    ValueAnalysisType Kind, DTransType *BaseTy, size_t ElemIdx,
    PointeeLoc::ElementOfTypeImpl &ElementOfTypes) {
  PointeeLoc Loc(PointeeLoc::PLK_Field, ElemIdx, ElementOfTypes);
  bool Changed = addElementPointeeImpl(Kind, BaseTy, Loc);
  return Changed;
}

bool ValueTypeInfo::addElementPointeeByOffset(ValueAnalysisType Kind,
                                              DTransType *BaseTy,
                                              size_t ByteOffset) {
  PointeeLoc Loc(PointeeLoc::PLK_ByteOffset, ByteOffset);
  bool Changed = addElementPointeeImpl(Kind, BaseTy, Loc);
  return Changed;
}

bool ValueTypeInfo::addElementPointeeByOffset(
    ValueAnalysisType Kind, DTransType *BaseTy, size_t ByteOffset,
    PointeeLoc::ElementOfTypeImpl &ElementOfTypes) {
  PointeeLoc Loc(PointeeLoc::PLK_ByteOffset, ByteOffset, ElementOfTypes);
  bool Changed = addElementPointeeImpl(Kind, BaseTy, Loc);
  return Changed;
}

bool ValueTypeInfo::addElementPointeeUnknownOffset(ValueAnalysisType Kind,
                                                   DTransType *BaseTy) {
  PointeeLoc Loc(PointeeLoc::PLK_UnknownOffset, 0);
  bool Changed = addElementPointeeImpl(Kind, BaseTy, Loc);
  return Changed;
}

bool ValueTypeInfo::addElementPointeeUnknownOffset(
    ValueAnalysisType Kind, DTransType *BaseTy,
    PointeeLoc::ElementOfTypeImpl &ElementOfTypes) {
  PointeeLoc Loc(PointeeLoc::PLK_UnknownOffset, 0, ElementOfTypes);
  bool Changed = addElementPointeeImpl(Kind, BaseTy, Loc);
  return Changed;
}

bool ValueTypeInfo::addElementPointee(ValueAnalysisType Kind,
                                      const TypeAndPointeeLocPair &Pointee) {
  return addElementPointeeImpl(Kind, Pointee.first, Pointee.second);
}

bool ValueTypeInfo::addElementPointeeImpl(ValueAnalysisType Kind,
                                          DTransType *BaseTy,
                                          const PointeeLoc &Loc) {
  bool Changed = ElementPointees[Kind].insert({BaseTy, Loc}).second;
  if (Changed) {
    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
      dbgs() << "    Added element:" << (Kind == VAT_Decl ? "[DECL]" : "[USE]")
             << ": ";
      printValue(dbgs(), V);
      dbgs() << " -- ";
      BaseTy->print(dbgs(), false);
      dbgs() << " @ ";
      switch (Loc.getKind()) {
      case PointeeLoc::PLK_Field:
        dbgs() << Loc.getElementNum() << "\n";
        break;
      case PointeeLoc::PLK_ByteOffset:
        dbgs() << "ByteOffset=" << Loc.getByteOffset() << ")\n";
        break;
      case PointeeLoc::PLK_UnknownOffset:
        dbgs() << "UnknownOffset\n";
        break;
      }
    });

    if (Kind == VAT_Decl)
      addElementPointeeImpl(VAT_Use, BaseTy, Loc);
  }

  return Changed;
}

void ValueTypeInfo::setPartiallyAnalyzed() {
  assert(AnalysisState != LPIS_CompletelyAnalyzed &&
         "Regression in analysis state!");
  AnalysisState = LPIS_PartiallyAnalyzed;
}

void ValueTypeInfo::setCompletelyAnalyzed() {
  AnalysisState = LPIS_CompletelyAnalyzed;
  DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
    printValue(dbgs(), V);
    dbgs() << " - Marked completely analyzed\n";
  });
}

bool ValueTypeInfo::isPartiallyAnalyzed() {
  return AnalysisState == LPIS_PartiallyAnalyzed;
}

bool ValueTypeInfo::isCompletelyAnalyzed() const {
  return AnalysisState == LPIS_CompletelyAnalyzed;
}

const char *ValueTypeInfo::LPIStateToString(LPIState S) {
  switch (S) {
  case LPIS_NotAnalyzed:
    return "NotAnalyzed";
  case LPIS_PartiallyAnalyzed:
    return "PartiallyAnalyzed";
  case LPIS_CompletelyAnalyzed:
    return "CompletelyAnalyzed";
  }

  llvm_unreachable("Exit from fully covered switch table");
}

void ValueTypeInfo::setUnhandled() {
  DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
    dbgs() << " - Marked as unhandled: ";
    printValue(dbgs(), V);
    dbgs() << "\n";
  });
  Unhandled = true;
}

void ValueTypeInfo::setDependsOnUnhandled() {
  DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
    dbgs() << " - Marked as depends on unhandled: ";
    printValue(dbgs(), V);
    dbgs() << "\n";
  });
  DependsOnUnhandled = true;
}

void ValueTypeInfo::setUnknownByteFlattenedGEP() {
  DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
    dbgs() << " - Marked as unknown byte flattened GEP: ";
    printValue(dbgs(), V);
    dbgs() << "\n";
  });
  UnknownByteFlattenedGEP = true;
}

void ValueTypeInfo::setIsPartialPointerUse() {
  DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
    dbgs() << " - Marked as partial pointer use: ";
    printValue(dbgs(), V);
    dbgs() << "\n";
  });
  IsPartialPointerUse = true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void ValueTypeInfo::dump() const { print(dbgs()); }

void ValueTypeInfo::print(raw_ostream &OS, bool Combined,
                          const char *Prefix) const {
  auto TypeToString = [Prefix](const DTransType *Ty) {
    std::string OutputVal;
    raw_string_ostream OutputStream(OutputVal);

    OutputStream << Prefix << "    " << *Ty;
    OutputStream.flush();
    return OutputVal;
  };

  // Print type alias list
  auto PrintAliasSet = [&OS, Prefix,
                        &TypeToString](const PointerTypeAliasSet &Aliases) {
    OS << Prefix;
    if (Aliases.empty()) {
      OS << "  No aliased types.\n";
      return;
    }

    OS << "  Aliased types:\n";
    dtrans::printCollectionSorted(OS, Aliases.begin(), Aliases.end(), "\n",
                                  TypeToString);
    OS << "\n";
  };

  auto ElementPointeeToString =
      [Prefix](const TypeAndPointeeLocPair &PointeePair) {
        std::string OutputVal;
        raw_string_ostream OutputStream(OutputVal);

        OutputStream << Prefix << "    ";
        if (auto *DTStTy = dyn_cast<DTransStructType>(PointeePair.first))
          if (DTStTy->hasName())
            OutputStream << "%" << DTStTy->getName();
          else
            OutputStream << *PointeePair.first;
        else
          OutputStream << *PointeePair.first;
        OutputStream << " @ ";
        switch (PointeePair.second.getKind()) {
        case PointeeLoc::PLK_Field:
          OutputStream << PointeePair.second.getElementNum();
          break;
        case PointeeLoc::PLK_ByteOffset:
          OutputStream << "not-field ByteOffset: "
                       << PointeePair.second.getByteOffset();
          break;
        case PointeeLoc::PLK_UnknownOffset:
          OutputStream << "UnknownOffset";
          break;
        }

        if (!PointeePair.second.ElementOf.empty()) {
          OutputStream << " ElementOf: ";
          const char *Sep = "";
          for (auto &T : PointeePair.second.ElementOf) {
            OutputStream << Sep;
            T.first->print(OutputStream, false);
            OutputStream << "@" << T.second;
            Sep = ", ";
          }
        }

        OutputStream.flush();
        return OutputVal;
      };

  // Print the element pointee set.
  auto PrintElementPointeeSet = [&OS, Prefix, &ElementPointeeToString](
                                    const ElementPointeeSet &Elements) {
    if (Elements.empty()) {
      OS << Prefix << "  No element pointees.\n";
      return;
    }

    OS << Prefix << "  Element pointees:\n";
    dtrans::printCollectionSorted(OS, Elements.begin(), Elements.end(), "\n",
                                  ElementPointeeToString);
    OS << "\n";
  };

  // Start of the actual printing routine.
  OS << Prefix << "LocalPointerInfo: " << LPIStateToString(AnalysisState)
     << (getUnhandled() ? " <UNHANDLED>" : "")
     << (getDependsOnUnhandled() ? " <DEPENDS ON UNHANDLED>" : "")
     << (getUnknownByteFlattenedGEP() ? " <UNKNOWN BYTE FLATTENED GEP>" : "")
     << (getIsPartialPointerUse() ? " <PARTIAL PTR>" : "") << "\n";

  if (!Combined) {
    OS << Prefix << "Declared Types:\n";
    PrintAliasSet(PointerTypeAliases[VAT_Decl]);
    PrintElementPointeeSet(ElementPointees[VAT_Decl]);
    OS << Prefix << "Usage Types:\n";
    PrintAliasSet(PointerTypeAliases[VAT_Use]);
    PrintElementPointeeSet(ElementPointees[VAT_Use]);
  } else {
    // Merge decl and use sets for printing. Currently, this 'usage' type set
    // should be a superset of the 'declared' type set, but in case this changes
    // in the future, create a set which is the merge of them.
    PointerTypeAliasSet TypeAliases;
    TypeAliases.insert(PointerTypeAliases[VAT_Decl].begin(),
                       PointerTypeAliases[VAT_Decl].end());
    TypeAliases.insert(PointerTypeAliases[VAT_Use].begin(),
                       PointerTypeAliases[VAT_Use].end());

    ElementPointeeSet Elements;
    Elements.insert(ElementPointees[VAT_Decl].begin(),
                    ElementPointees[VAT_Decl].end());
    Elements.insert(ElementPointees[VAT_Use].begin(),
                    ElementPointees[VAT_Use].end());

    PrintAliasSet(TypeAliases);
    PrintElementPointeeSet(Elements);
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

////////////////////////////////////////////////////////////////////////////////
//
// Implementation of PtrTypeAnalyzerImpl class
//
////////////////////////////////////////////////////////////////////////////////
PtrTypeAnalyzerImpl::~PtrTypeAnalyzerImpl() {
  for (auto &LM : LocalMaps)
    for (auto &LPI : LM.second)
      delete LPI.second;
  LocalMaps.clear();

  for (auto &LPI : LocalMapForConstant)
    delete LPI.second;
  LocalMapForConstant.clear();
}

void PtrTypeAnalyzerImpl::addByteFlattenedGEPMapping(GEPOperator *GEP,
                                                     DTransType *Ty,
                                                     size_t Idx) {
  if (ByteFlattenedGEPInfoMap.insert({GEP, {Ty, Idx}}).second)
    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
      dbgs() << "Adding BF-GEP access: ";
      dbgs() << *Ty << " @ " << Idx << " -- ";
      printValue(dbgs(), GEP);
      dbgs() << "\n";
    });
}

std::pair<DTransType *, size_t>
PtrTypeAnalyzerImpl::getByteFlattenedGEPElement(GEPOperator *GEP) const {
  return ByteFlattenedGEPInfoMap.lookup(GEP);
}

void PtrTypeAnalyzerImpl::addFlattenedGEPMapping(GEPOperator *GEP,
                                                 DTransType *Ty, size_t Idx,
                                                 size_t Multiplier) {
  if (FlattenedGEPInfoMap.insert({GEP, {Ty, Idx, Multiplier}}).second)
    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
      dbgs() << "Adding Flattened-GEP access: ";
      dbgs() << *Ty << " @ " << Idx << " -- ";
      if (auto *I = dyn_cast<Instruction>(GEP))
        dbgs() << "[" << I->getFunction()->getName() << "] ";
      printValue(dbgs(), GEP);
      dbgs() << " - Multiplier = " << Multiplier;
      dbgs() << "\n";
      });
}

llvm::Optional<PtrTypeAnalyzer::FlattenedGEPInfoType>
PtrTypeAnalyzerImpl::getFlattenedGEPElement(GEPOperator *GEP) const {
  auto Entry = FlattenedGEPInfoMap.find(GEP);
  if (Entry != FlattenedGEPInfoMap.end())
    return Entry->second;
    
  return None;
}

void PtrTypeAnalyzerImpl::run(Module &M) {
  DTransLibInfo.initialize(M);
  DTransAllocCollector DTAC(MDReader, GetTLI);
  DTAC.populateAllocDeallocTable(M);
  PtrTypeAnalyzerInstVisitor InstAnalyzer(*this, TM, MDReader, DTransLibInfo,
                                          DTAC, M.getContext(), DL, GetTLI);
  InstAnalyzer.visit(M);
}

ValueTypeInfo *PtrTypeAnalyzerImpl::getOrCreateValueTypeInfo(Value *V) {
  assert(!isCompilerConstant(V) && "Should not be compiler constant.");
  Function *F = nullptr;
  if (auto *I = dyn_cast<Instruction>(V))
    F = I->getFunction();
  auto &FuncLocalMap = LocalMaps[F];
  ValueTypeInfo *&Info = FuncLocalMap[V];
  if (!Info)
    Info = new ValueTypeInfo(V);
  return Info;
}

ValueTypeInfo *PtrTypeAnalyzerImpl::getOrCreateValueTypeInfo(const User *U,
                                                             unsigned OpNum) {
  ValueTypeInfo *Info = getValueTypeInfo(U, OpNum);
  if (Info)
    return Info;

  Value *V = U->getOperand(OpNum);
  if (!isCompilerConstant(V))
    return getOrCreateValueTypeInfo(V);

  Info = new ValueTypeInfo(nullptr);
  LocalMapForConstant[{U, OpNum}] = Info;
  return Info;
}

ValueTypeInfo *PtrTypeAnalyzerImpl::getValueTypeInfo(const Value *V) const {
  const Function *F = nullptr;
  if (auto *I = dyn_cast<Instruction>(V))
    F = I->getFunction();
  auto It = LocalMaps.find(F);
  if (It == LocalMaps.end())
    return nullptr;

  const PerFunctionLocalMapType &FuncLocalMap = It->second;
  auto It2 = FuncLocalMap.find(V);
  if (It2 == FuncLocalMap.end())
    return nullptr;

  return It2->second;
}

ValueTypeInfo *PtrTypeAnalyzerImpl::getValueTypeInfo(const User *U,
                                                     unsigned OpNum) const {
  Value *V = U->getOperand(OpNum);
  if (isCompilerConstant(V)) {
    auto It = LocalMapForConstant.find({U, OpNum});
    if (It == LocalMapForConstant.end())
      return nullptr;

    return It->second;
  }

  return getValueTypeInfo(V);
}

void PtrTypeAnalyzerImpl::setDeclaredType(Value *V, DTransType *Ty) {
  ValueTypeInfo *Info = getOrCreateValueTypeInfo(V);
  Info->addTypeAlias(ValueTypeInfo::VAT_Decl, Ty);
  Info->setCompletelyAnalyzed();
}

DTransType *
PtrTypeAnalyzerImpl::getDominantAggregateUsageType(ValueTypeInfo &Info) const {
  return getDominantAggregateType(Info, ValueTypeInfo::VAT_Use);
}

DTransType *PtrTypeAnalyzerImpl::getDominantAggregateType(
    ValueTypeInfo &Info, ValueTypeInfo::ValueAnalysisType Kind) const {
  if (!Info.canAliasToAggregatePointer())
    return nullptr;

  DTransType *DomTy = nullptr;
  bool HaveMultipleAliases = false;
  bool DomTyIsElementZeroAccess = false;
  for (auto *AliasTy : Info.getPointerTypeAliasSet(Kind)) {
    DTransType *BaseTy = AliasTy;
    while (BaseTy->isPointerTy())
      BaseTy = BaseTy->getPointerElementType();

    // Generic pointer forms of 'i8' or pointer sized integers are safe aliases
    // of an aggregate type.
    if (!BaseTy->isAggregateType())
      if (BaseTy == getDTransI8Type() || BaseTy == getDTransPtrSizedIntType())
        continue;

    if (!DomTy) {
      DomTy = AliasTy;
      continue;
    }

    HaveMultipleAliases = true;

    // If this type can be an element zero access of DomTy, DomTy is still
    // dominant.
    if (isElementZeroAccess(DomTy, AliasTy)) {
      DomTyIsElementZeroAccess = true;
      continue;
    }

    // If what we previously thought was the dominant type can be an element
    // zero access of the current alias, the current alias becomes dominant.
    if (isElementZeroAccess(AliasTy, DomTy)) {
      DomTyIsElementZeroAccess = true;
      DomTy = AliasTy;
      continue;
    }

    // Check whether the pending dominant type, DomTy, is a pointer-to-pointer
    // type that may be equivalent to the alias type, AliasTy, based on the
    // element zero types.
    //
    // For example, given the types:
    //   %struct.outer = type { %struct.middle* }
    //   %struct.middle = type { %struct.inner }
    //   %struct.inner = type { i64 }
    //
    // When using an input alias set of:
    //   %struct.outer*, %struct.middle**, and %struct.inner**
    //
    // The types %struct.middle** and %struct.inner** do not dominate each
    // other, but can be used interchangeably.
    //
    // The above checks will choose %struct.outer* as being the dominant type of
    // %struct.middle** and of %struct.inner**, if %struct.outer* is considered
    // as the pending dominant type prior to testing against the other two types
    // because the rule within the isElementZeroAccess that allows for a
    // pointer-to-pointer type to succeed when the element zero type is a
    // pointer type. However, we need to account for these aliases being
    // evaluated in an arbitrary order. If the order of evaluation was chosen to
    // test %struct.middle** and %struct.inner** as the types to evaluate as
    // potential element zero access types, then neither will succeed because
    // they are both pointer-to-pointer types. Here, we try to see whether
    // removing one level of type dereferencing indicates that one of them is an
    // element zero type of the other to determine whether the search for an
    // element zero dominant type should continue. We only need to consider one
    // level of dereferencing because isElementZeroAccess only permits the
    // zeroth element to be an aggregate of one level pointer type.
    if (!(DomTy->isPointerTy() &&
          DomTy->getPointerElementType()->isPointerTy() &&
          AliasTy->isPointerTy() &&
          AliasTy->getPointerElementType()->isPointerTy()))
      return nullptr;

    DTransType *DomDerefTy = DomTy->getPointerElementType();
    DTransType *AliasDerefTy = AliasTy->getPointerElementType();

    // Given that 'DomTy' was '%struct.middle**' and 'AliasTy' was
    // '%struct.inner**', the pointer-to-pointer type in 'AliasTy' is an element
    // zero type of the pointer-to-pointer type in 'DomTy'.
    if (isElementZeroAccess(DomDerefTy, AliasDerefTy)) {
      DomTyIsElementZeroAccess = true;
      continue;
    }

    // Given that 'DomTy' was '%struct.inner**' and 'AliasTy' was
    // '%struct.middle**', the pointer-to-pointer type in 'DomTy' is an element
    // zero type of the pointer-to-pointer in 'AliasTy', update the 'DomTy' to
    // be the pointer-to-pointer of the outer type.
    if (isElementZeroAccess(AliasDerefTy, DomDerefTy)) {
      DomTy = AliasTy;
      DomTyIsElementZeroAccess = true;
      continue;
    }

    // Otherwise, there are conflicting aliases and nothing can be dominant.
    return nullptr;
  }

  // If there was only one potential dominant type, return it.
  // Otherwise, the dominant type must be an element zero accessible type from
  // all the other types.
  if (!HaveMultipleAliases)
    return DomTy;
  if (!DomTyIsElementZeroAccess)
    return nullptr;

  return DomTy;
}

DTransType *PtrTypeAnalyzerImpl::getDominantType(
    ValueTypeInfo &Info, ValueTypeInfo::ValueAnalysisType Kind) const {
  auto IsI8PtrTy = [this](DTransType *Ty) {
    return Ty == getDTransI8PtrType();
  };

  auto IsPtrSizeIntPtrTy = [this](DTransType *Ty) {
    return Ty == getDTransPtrSizedIntPtrType();
  };

  // If there are aggregate types, try to find the dominant one.
  if (Info.canAliasToAggregatePointer(Kind))
    return getDominantAggregateType(Info, Kind);

  // If there are no aggregate types, try to find the best match from pointers
  // to scalar types. By best type, this means that if a type is aliased by a
  // generic form (i8* or pointer sized int pointer), ignore those in
  // preference to a non-generic form. If only generic forms are present, then
  // return the generic form, unless both generic forms are present, in which
  // case we will return the i64* type because it is unknown which is being used
  // as a generic pointer and we want deterministic results.
  // For example, given the following sets of possibilities:
  //   1)  { i8*, i32* }       -> return i32*
  //   2)  { double*, i64* }   -> return double*
  //   3)  { float*, double* } -> return nullptr
  //   4)  { i8*, i64* }       -> i64*
  DTransType *DomTy = nullptr;
  DTransType *GenericTy = nullptr;
  for (auto *AliasTy : Info.getPointerTypeAliasSet(Kind)) {
    if (IsI8PtrTy(AliasTy)) {
      if (!GenericTy)
        GenericTy = AliasTy;
    } else if (IsPtrSizeIntPtrTy(AliasTy)) {
      // Prefer a pointer sized int pointer to an i8* type.
      GenericTy = AliasTy;
    } else {
      if (DomTy)
        return nullptr;

      // Choose AliasTy as the dominant type.
      DomTy = AliasTy;
    }
  }

  if (DomTy)
    return DomTy;
  return GenericTy;
}

bool PtrTypeAnalyzerImpl::isPtrToPtr(ValueTypeInfo &Info) const {
  DTransType *DomTy = getDominantAggregateUsageType(Info);
  if (!DomTy)
    return false;
  if (!DomTy->isPointerTy())
    return false;
  if (!DomTy->getPointerElementType()->isPointerTy())
    return false;
  return true;
}

bool PtrTypeAnalyzerImpl::isPtrToIntOrFloat(ValueTypeInfo &Info) const {
  DTransType *DomTy = getDominantType(Info, ValueTypeInfo::VAT_Use);
  if (!DomTy)
    return false;
  if (!DomTy->isPointerTy())
    return false;
  DTransType *PETy = DomTy->getPointerElementType();
  return PETy && (PETy->isIntegerTy() || PETy->isFloatingPointTy());
}

bool PtrTypeAnalyzerImpl::isPtrToCharArray(ValueTypeInfo &Info,
                                           DTransArrayType **AggArType) const {
  DTransType *DomTy = getDominantAggregateUsageType(Info);
  if (!DomTy) {
    if (Info.pointsToSomeElement())
      for (auto &PointeePair :
           Info.getElementPointeeSet(ValueTypeInfo::VAT_Use))
        if (auto *ArTy = dyn_cast<DTransArrayType>(PointeePair.first))
          if (ArTy->getArrayElementType() == getDTransI8Type()) {
            if (AggArType)
              *AggArType = ArTy;
            return true;
          }
    return false;
  }

  if (!DomTy->isPointerTy())
    return false;
  if (!DomTy->getPointerElementType()->isArrayTy())
    return false;
  if (auto *ArTy = dyn_cast<DTransArrayType>(DomTy->getPointerElementType()))
    if (ArTy->getArrayElementType() == getDTransI8Type()) {
      if (AggArType)
        *AggArType = ArTy;
      return true;
    }

  return false;
}

// This function is called to determine if a bitcast to the specified
// destination type could be used to access element 0 of the source type.
// If the destination type is a pointer type whose element type is the same
// as the type of element zero of the aggregate pointed to by the source
// pointer type, then it would be a valid element zero access.
//
// For example, consider:
//
//   %struct.S1 = type { %struct.S2, i32 }
//   ...
//   %p = bitcast %struct.S1* to %struct.S2*
//
// Because element zero of %struct.S1 has %struct.S2 as a type, this is a
// safe cast that accesses that element. Notice that the pointer to the
// element has a different level of indirection than the declaration of the
// element within the structure. This is expected because the element is
// accessed through a pointer to that element.
//
// Also, consider this example of an unsafe cast.
//
//   %struct.S3 = type { %struct.S4*, i32 }
//   ...
//   %p = bitcast %struct.S3* to %struct.S4*
//
// In this case, element zero of the %struct.S3 type is a pointer, %struct.S4*
// so the correct cast to access that element would be:
//
//   %p = bitcast %struct.S3* to %struct.S4**
//
// Element zero can also be accessed by casting an i8* pointer that is known
// to point to a given structure to a pointer to element zero of that type.
// However, the caller must handle that case by obtaining the necessary
// type alias information and calling this function with the known alias as
// the SrcTy argument.
//
// If the 'AccessedTy' argument is not null, this function will set it to
// nullptr if this is not an element zero access or a pointer to the type of
// the aggregate whose element zero is being accessed. This may be 'SrcTy' or
// it may be a nested type if element zero of the source type is an aggregate
// type whose element zero is being accessed.
bool PtrTypeAnalyzerImpl::isElementZeroAccess(DTransType *SrcTy,
                                              DTransType *DestTy,
                                              DTransType **AccessedTy) const {
  if (AccessedTy)
    *AccessedTy = nullptr;
  if (!SrcTy || !DestTy)
    return false;
  if (!DestTy->isPointerTy() || !SrcTy->isPointerTy())
    return false;

  DTransType *SrcPointeeTy = SrcTy->getPointerElementType();
  DTransType *DestPointeeTy = DestTy->getPointerElementType();
  return isPointeeElementZeroAccess(SrcPointeeTy, DestPointeeTy, AccessedTy);
}

bool PtrTypeAnalyzerImpl::isPointeeElementZeroAccess(
    DTransType *SrcPointeeTy, DTransType *DestPointeeTy,
    DTransType **AccessedTy) const {
  if (AccessedTy)
    *AccessedTy = nullptr;

  auto *CompTy = dyn_cast<DTransCompositeType>(SrcPointeeTy);
  if (!CompTy)
    return false;

  // This avoids problems with opaque structure types.
  if (!CompTy->indexValid(0u))
    return false;

  DTransType *ElementZeroTy = nullptr;
  if (auto *StTy = dyn_cast<DTransStructType>(CompTy))
    ElementZeroTy = StTy->getFieldType(0);
  else
    ElementZeroTy = dyn_cast<DTransSequentialType>(CompTy)->getTypeAtIndex(0u);

  if (!ElementZeroTy)
    return false;

  // If the element zero type matches the destination pointee type,
  // this is an element zero access.
  if (DestPointeeTy->compare(*ElementZeroTy)) {
    if (AccessedTy)
      *AccessedTy = SrcPointeeTy;
    return true;
  }

  // Handle multiple levels of indirection with i8* destinations.
  // If the element zero type is a pointer type and the destination pointee
  // type is a corresponding i8* at the same level of indirection, this is an
  // element zero access.
  if (ElementZeroTy->isPointerTy() && DestPointeeTy->isPointerTy()) {
    auto *TempZeroTy = ElementZeroTy;
    auto *TempDestTy = DestPointeeTy;
    while (TempZeroTy->isPointerTy() && TempDestTy->isPointerTy()) {
      TempZeroTy = TempZeroTy->getPointerElementType();
      TempDestTy = TempDestTy->getPointerElementType();
    }

    if (TempDestTy == getDTransI8Type()) {
      if (AccessedTy)
        *AccessedTy = SrcPointeeTy;
      return true;
    }
  }

  // If element zero is an aggregate type, this cast might be accessing
  // element zero of the nested type.
  if (ElementZeroTy->isAggregateType())
    return isPointeeElementZeroAccess(ElementZeroTy, DestPointeeTy, AccessedTy);

  // If element zero is a pointer to an aggregate type this cast might
  // be creating a pointer to element zero of the type pointed to.
  // For instance, if we have the following types:
  //
  //   %A = type { %B*, ... }
  //   %B = type { %C, ... }
  //   %C = type { ... }
  //
  // The following IR would get the address of A->C.
  //
  //   %ppc = bitcast %A* %pa to %C**
  //
  if (ElementZeroTy->isPointerTy() &&
      ElementZeroTy->getPointerElementType()->isAggregateType()) {
    // In this case, tracking the accessed type is tricky because
    // the check is off by a level of indirection. If it's a match
    // we need to record it as element zero of SrcTy.
    bool Match = isElementZeroAccess(ElementZeroTy, DestPointeeTy);
    if (Match && AccessedTy)
      *AccessedTy = SrcPointeeTy;
    return Match;
  }

  // Otherwise, it must be a bad cast. The caller should handle that.
  return false;
}

Optional<PtrTypeAnalyzerImpl::AggregateElementPair>
PtrTypeAnalyzerImpl::getElementZeroType(DTransType *Ty) {
  if (!Ty->isAggregateType())
    return None;

  DTransType *LastAggregateType = Ty;
  DTransType *NestedType = Ty;
  while (NestedType->isAggregateType()) {
    LastAggregateType = NestedType;
    if (auto *StTy = dyn_cast<DTransStructType>(NestedType)) {
      NestedType = StTy->getFieldType(0);
      if (!NestedType)
        return None;

      continue;
    }

    assert(isa<DTransArrayType>(NestedType) &&
           "Expected aggregate to be structure or array");
    NestedType = cast<DTransArrayType>(NestedType)->getElementType();
  }

  return std::make_pair(LastAggregateType, NestedType);
}

////////////////////////////////////////////////////////////////////////////////
//
// Implementation of PtrTypeAnalyzer class
//
////////////////////////////////////////////////////////////////////////////////

PtrTypeAnalyzer::PtrTypeAnalyzer(
    LLVMContext &Ctx, DTransTypeManager &TM, TypeMetadataReader &MDReader,
    const DataLayout &DL,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI) {
  Impl = std::make_unique<PtrTypeAnalyzerImpl>(Ctx, TM, MDReader, DL, GetTLI);
}

// Declaration needed in source file to enable unique_ptr destructor to see
// implementation class.
PtrTypeAnalyzer::~PtrTypeAnalyzer() = default;

PtrTypeAnalyzer::PtrTypeAnalyzer(PtrTypeAnalyzer &&Other)
    : Impl(std::move(Other.Impl)) {}

PtrTypeAnalyzer &PtrTypeAnalyzer::operator=(PtrTypeAnalyzer &&Other) {
  Impl = std::move(Other.Impl);
  return *this;
}

void PtrTypeAnalyzer::run(Module &M) {
  Impl->run(M);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (PrintPTAResults)
    dumpPTA(M);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
}

ValueTypeInfo *PtrTypeAnalyzer::getValueTypeInfo(const Value *V) const {
  return Impl->getValueTypeInfo(V);
}

ValueTypeInfo *PtrTypeAnalyzer::getValueTypeInfo(const User *U,
                                                 unsigned OpNum) const {
  return Impl->getValueTypeInfo(U, OpNum);
}

bool PtrTypeAnalyzer::isElementZeroAccess(DTransType *SrcTy,
                                          DTransType *DestTy) const {
  return Impl->isElementZeroAccess(SrcTy, DestTy);
}

bool PtrTypeAnalyzer::isPointeeElementZeroAccess(
    DTransType *SrcPointeeTy, DTransType *DestPointeeTy) const {
  return Impl->isPointeeElementZeroAccess(SrcPointeeTy, DestPointeeTy);
}

DTransType *
PtrTypeAnalyzer::getDominantAggregateUsageType(ValueTypeInfo &Info) const {
  return Impl->getDominantAggregateUsageType(Info);
}

DTransType *PtrTypeAnalyzer::getDominantAggregateType(
    ValueTypeInfo &Info, ValueTypeInfo::ValueAnalysisType Kind) const {
  return Impl->getDominantAggregateType(Info, Kind);
}

DTransType *
PtrTypeAnalyzer::getDominantType(ValueTypeInfo &Info,
                                 ValueTypeInfo::ValueAnalysisType Kind) const {
  return Impl->getDominantType(Info, Kind);
}

bool PtrTypeAnalyzer::isPtrToPtr(ValueTypeInfo &Info) const {
  return Impl->isPtrToPtr(Info);
}

bool PtrTypeAnalyzer::isPtrToIntOrFloat(ValueTypeInfo &Info) const {
  return Impl->isPtrToIntOrFloat(Info);
}

std::pair<DTransType *, size_t>
PtrTypeAnalyzer::getByteFlattenedGEPElement(GEPOperator *GEP) const {
  return Impl->getByteFlattenedGEPElement(GEP);
}

llvm::Optional<PtrTypeAnalyzer::FlattenedGEPInfoType>
PtrTypeAnalyzer::getFlattenedGEPElement(GEPOperator *GEP) const {
  return Impl->getFlattenedGEPElement(GEP);
}

dtrans::AllocKind PtrTypeAnalyzer::getAllocationCallKind(CallBase *Call) const {
  return Impl->getAllocationCallKind(Call);
}

dtrans::FreeKind PtrTypeAnalyzer::getFreeCallKind(CallBase *Call) const {
  return Impl->getFreeCallKind(Call);
}

bool PtrTypeAnalyzer::getUnsupportedAddressSpaceSeen() const {
  return Impl->getUnsupportedAddressSpaceSeen();
}

PtrTypeAnalyzer::ElementZeroInfo
PtrTypeAnalyzer::getElementZeroPointer(Instruction *I) const {
  return Impl->getElementZeroPointer(I);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void PtrTypeAnalyzer::dumpPTA(Module &M) {
  PtrTypeAnalyzerAnnotationWriter Annotator(*this, PTAEmitCombinedSets);
  M.print(dbgs(), &Annotator);
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

} // namespace dtransOP
} // end namespace llvm

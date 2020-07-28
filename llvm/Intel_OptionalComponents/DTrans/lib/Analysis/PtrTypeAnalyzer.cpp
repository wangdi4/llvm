//===-----------------------PtrTypeAnalyzer.cpp---------------------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"

#include "Intel_DTrans/Analysis/DTransAllocAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransDebug.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FormattedStream.h"

#define DEBUG_TYPE "dtrans-pta"

// Trace messages for the pointer type analyzer when new information is
// associated with a Value object.
#define VERBOSE_TRACE "dtrans-pta-verbose"

// Trace message about information collected for dependent values.
#define VERBOSE_DEP_TRACE "dtrans-pta-dep-verbose"

namespace llvm {
namespace dtrans {

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

// Check whether the DTransType \p has a pointer. This is similar to the
// utility function dtrans::hasPointerType(llvm::Type* Ty), which operates
// on llvm Types.
static bool hasPointerType(dtrans::DTransType *Ty) {
  if (Ty->isPointerTy())
    return true;

  if (auto *SeqTy = dyn_cast<DTransSequentialType>(Ty))
    return hasPointerType(SeqTy->getElementType());

  if (auto *StTy = dyn_cast<DTransStructType>(Ty)) {
    // Check inside of literal structures because those cannot be referenced by
    // name. However, there is no need to look inside non-literal structures
    // because those will be referenced by their name.
    if (StTy->isLiteralStruct())
      for (auto &Field : StTy->getFields())
        for (auto *ElemTy : Field.getTypes()) {
          bool HasPointer = hasPointerType(ElemTy);
          if (HasPointer)
            return true;
        }
  }

  if (auto *FnTy = dyn_cast<DTransFunctionType>(Ty)) {
    // Check the return type and the parameter types for any possible
    // pointer because metadata descriptions on these will be used to help
    // recovery of opaque pointer types.
    DTransType *RetTy = FnTy->getReturnType();
    if (RetTy && hasPointerType(RetTy))
      return true;

    unsigned NumParams = FnTy->getNumArgs();
    for (unsigned Idx = 0; Idx < NumParams; ++Idx) {
      DTransType *ParmTy = FnTy->getArgType(Idx);
      if (ParmTy && hasPointerType(ParmTy))
        return true;
    }
  }

  return false;
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
  void emitFunctionAnnot(const Function *F, formatted_raw_ostream &OS) {
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
        Analyzer.printDominantAggregateUsageType(OS, *Info);
      } else if (A.getType()->isPointerTy()) {
        OS << ";    <NO PTR INFO AVAILABLE>\n";
      }
    }
  }

  // For pointers and values of interest, print the type information determined
  // for Value \p CV
  void printInfoComment(const Value &CV, formatted_raw_ostream &OS) {
    std::function<void(formatted_raw_ostream &, ConstantExpr *)>
        PrintConstantExpr =
            [&PrintConstantExpr, this](formatted_raw_ostream &OS,
                                       ConstantExpr *CE) -> void {
      OS << "\n;        CE: " << *CE << "\n";
      auto *Info = Analyzer.getValueTypeInfo(CE);
      if (Info) {
        Info->print(OS, CombineUseAndDecl, ";          ");
        OS << ";            ";
        Analyzer.printDominantAggregateUsageType(OS, *Info);
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
      Info->print(OS, CombineUseAndDecl, ";    ");
      OS << ";      ";
      Analyzer.printDominantAggregateUsageType(OS, *Info);
    }
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
      : TM(TM), MDReader(MDReader), DL(DL), GetTLI(GetTLI) {
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

  // Set Ty as the declaration type of value V, and mark the ValueTypeInfo as
  // completely analyzed.
  void setDeclaredType(Value *V, DTransType *Ty);

  llvm::Type *getLLVMI8Type() const { return LLVMI8Type; }
  llvm::Type *getLLVMPointerSizedIntType() const {
    return LLVMPointerSizedIntType;
  }

  dtrans::DTransType *getDTransI8Type() const { return DTransI8Type; }
  dtrans::DTransPointerType *getDTransI8PtrType() const {
    return DTransI8PtrType;
  }

  dtrans::DTransType *getDTransPtrSizedIntType() const {
    return DTransPtrSizedIntType;
  }

  dtrans::DTransType *getDTransPtrSizedIntPtrType() const {
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

private:
  DTransTypeManager &TM;
  TypeMetadataReader &MDReader;
  const DataLayout &DL;
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;

  // We cannot use DenseMap or ValueMap here because we are inserting values
  // during recursive calls to analyzeValue() and with a DenseMap or
  // ValueMap that would cause the LocalPointerInfo to be copied and our
  // local reference to it to be invalidated.
  std::map<const Value *, ValueTypeInfo *> LocalMap;

  // Compiler constants such as 'undef' and 'null' need to have context
  // sensitive information. For example:
  //   %ptr1 = alloca %struct.foo
  //   %ptr2 = alloca %struct.bar
  //   store p0 null, p0 %ptr1
  //   store p0 null, p0 %ptr2
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

  // LLVM Type for 'i8'
  llvm::Type *LLVMI8Type;

  // LLVM integer type that is the same size as a pointer in address space 0.
  llvm::Type *LLVMPointerSizedIntType;

  // Representation of the 'i8' type in DTransType system
  dtrans::DTransType *DTransI8Type;

  // Representation of the 'i8*' type in DTransType system
  dtrans::DTransPointerType *DTransI8PtrType;

  // Representation of an integer type that is the same size as a pointer in the
  // DTransType system.
  dtrans::DTransType *DTransPtrSizedIntType;

  // Representation of a pointer to an integer type that is the same size as a
  // pointer in the DTransType system.
  dtrans::DTransPointerType *DTransPtrSizedIntPtrType;
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

public:
  PtrTypeAnalyzerInstVisitor(
      PtrTypeAnalyzerImpl &PTA, DTransTypeManager &TM,
      TypeMetadataReader &MDReader, LLVMContext &Ctx, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI)
      : PTA(PTA), TM(TM), MDReader(MDReader), DL(DL), GetTLI(GetTLI) {

    // If the metadata contained a description of the system object type for
    // %struct._IO_FILE, use it. Otherwise, default the type to be an i8* since
    // there must not be any accesses of structure elements for it within the
    // user program.
    dtrans::DTransType *StTy = TM.getStructType("struct._IO_FILE");
    if (StTy)
      DTransIOPtrTy = TM.getOrCreatePointerType(StTy);
    else
      DTransIOPtrTy = PTA.getDTransI8PtrType();
  }

  void visitModule(Module &M) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    // Reset the state of the trace filter to the default value in case another
    // call to visitModule is started.
    FNFilter.reset();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    // Get the type for all the Function definitions.
    for (auto &F : M) {
      // TODO: Currently, we do not have information about declarations that are
      // outside the module in the metadata. The metadata descriptions may be
      // extended in the future to handle these.
      if (F.isDeclaration())
        continue;

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

      ValueTypeInfo *Info = PTA.getOrCreateValueTypeInfo(&F);
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
        dtrans::DTransType *Ty = TM.getOrCreateSimpleType(ValTy);
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
    // just be defined as being of type 'p0'
    DTransType *DTy = StringSwitch<DTransType *>(GV.getName())
                          .Case("stdout", getDTransIOPtrTy())
                          .Case("stderr", getDTransIOPtrTy())
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

    if (!F.isDeclaration())
      for (auto &A : F.args())
        if (hasPointerType(A.getType()))
          analyzeValue(&A);
  }

  void visitAllocaInst(AllocaInst &I) { analyzeValue(&I); }
  void visitBitCastInst(BitCastInst &I) { analyzeValue(&I); }
  void visitCallBase(CallBase &I) { analyzeValue(&I); }
  void visitGetElementPtrInst(GetElementPtrInst &I) { analyzeValue(&I); }
  void visitIntToPtrInst(IntToPtrInst &I) { analyzeValue(&I); }
  void visitLoadInst(LoadInst &I) { analyzeValue(&I); }
  void visitPHINode(PHINode &I) { analyzeValue(&I); }
  void visitPtrToIntInst(PtrToIntInst &I) { analyzeValue(&I); }
  void visitSelectInst(SelectInst &I) { analyzeValue(&I); }

  void visitStoreInst(StoreInst &I) {
    // Storing a 'null' constant is a special case, because a Value of 'p0 null'
    // may be used to represent any pointer type. For a store of a 'null'
    // constant we want to capture a type of the value operand so that the
    // safety analyzer will be able to analyze the instruction without needing
    // to examine the value being stored with a special case for 'null'
    // constants.
    if (isa<ConstantPointerNull>(I.getValueOperand())) {
      auto &LocalTM = this->TM;
      auto &LocalPTA = this->PTA;

      // Propagate the known types from the pointer-operand to ValueInfo that
      // represents a use of the 'null' constant. This may also, identify that
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
                    Kind, ElemZeroPair.getValue().first, 0);
                ValueInfo->addTypeAlias(Kind, ElemZeroPair.getValue().second);

                // Need to defer updating the PointerInfo until the loop
                // completes because we cannot modify the set while we are
                // iterating it.
                PendingTypes.push_back(LocalTM.getOrCreatePointerType(
                    ElemZeroPair.getValue().second));
              } else {
                ValueInfo->setUnhandled();
              }
            }

            for (auto *Ty : PendingTypes)
              PointerInfo->addTypeAlias(Kind, Ty);
          };

      ValueTypeInfo *PtrInfo = analyzeValue(I.getPointerOperand());
      ValueTypeInfo *ValInfo = PTA.getOrCreateValueTypeInfo(&I, 0);
      PropagateDereferencedType(PtrInfo, ValInfo,
                                ValueTypeInfo::ValueAnalysisType::VAT_Decl);
      PropagateDereferencedType(PtrInfo, ValInfo,
                                ValueTypeInfo::ValueAnalysisType::VAT_Use);
      ValInfo->setCompletelyAnalyzed();
    }
  }

  // All instructions not handled by other visit functions.
  void visitInstruction(Instruction &I) {
    if (!hasPointerType(I.getType()))
      return;

    ValueTypeInfo *Info = PTA.getOrCreateValueTypeInfo(&I);
    Info->setUnhandled();
    LLVM_DEBUG(dbgs() << "Unhandled instruction: " << I << "\n");
  }

private:
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

    // Build a stack of unresolved dependent values that must be analyzed
    // before we can complete the analysis of this value.
    SmallVector<Value *, 16> DependentVals;
    DependentVals.push_back(V);
    populateDependencyStack(V, DependentVals);
    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE,
                      dumpDependencyStack(V, DependentVals));

    while (!DependentVals.empty()) {
      Value *Dep = DependentVals.back();
      DependentVals.pop_back();

      // A compiler constant, such as undef or null, cannot help supply the
      // information required to analyze a use because the actual type for
      // these depends on the context in which it is used.
      //
      // For example, the type of %x in the following cannot be resolved based
      // on the 'null' it is used with:
      //   store p0 null, p0 %x
      //   icmp eq p0 null, p0 %x
      //   %x = phi p0 [null, %l1]
      if (isCompilerConstant(Dep))
        continue;

      ValueTypeInfo *DepInfo = PTA.getOrCreateValueTypeInfo(Dep);
      DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_DEP_TRACE, {
        dbgs() << "  Dependent: ";
        printValue(dbgs(), Dep);
        dbgs() << "\n";
        DepInfo->print(dbgs(), /*Combined=*/false, "    ");
      });

      // If we have complete results for this value, don't repeat the analysis.
      if (DepInfo->isCompletelyAnalyzed()) {
        DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_DEP_TRACE, {
          dbgs() << "  Already analyzed: ";
          printValue(dbgs(), Dep);
          dbgs() << "\n";
        });
        continue;
      }
      analyzeValueImpl(Dep, DepInfo);
    }

    // Do not mark the value as completely analyzed when the analysis was
    // triggered while trying to infer some value because the types may not be
    // complete until the inference completes.
    if (!isInferInProgress())
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
    case Instruction::GetElementPtr:
      analyzeGetElementPtrOperator(cast<GEPOperator>(I), Info);
      break;
    case Instruction::IntToPtr:
      analyzeIntToPtrInst(cast<IntToPtrInst>(I), Info);
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
      // TODO: Add other instructions analysis calls.
    }
  }

  // Update the ResultInfo usage type alias set for ValueToInfer by looking at
  // all the users of the ValueToInfer to determine its type.
  void inferTypeFromUse(Value *ValueToInfer, ValueTypeInfo *ResultInfo) {
    // Don't start another inference triggered by an analyze routine when
    // one is already underway for the value.
    if (InferInProgress.count(ValueToInfer))
      return;

    // If the type has already been inferred, we will use the previously
    // collected results, rather than examining the uses again.
    auto It = ValueToInferredTypes.find(ValueToInfer);
    if (It == ValueToInferredTypes.end()) {

      // Try to infer types for the value.
      inferTypeFromUseImpl(ValueToInfer);
      It = ValueToInferredTypes.find(ValueToInfer);
    }

    if (It == ValueToInferredTypes.end()) {
      // If there were users, then a type should have been found once all the
      // infer calls complete. However, if there are infer calls still in
      // progress don't treat this as unhandled. The LIT test,
      // ptrtype-icmp-infer01.ll, is an example of a case where inferring one
      // value leads to analysis of another value that starts another inference
      // call.
      //
      // (1) %tmp1008 = bitcast %struct.test01* %arg1001 to
      //                        void (%struct.test01*, i8*)***
      // (2) %tmp1009 = load void(%struct.test01*, i8*)**,
      //                     void(%struct.test01*, i8*)*** %tmp1008
      // (3) %tmp1010 = getelementptr inbounds void(%struct.test01*, i8*)*,
      //                           void(%struct.test01*, i8*)** %tmp1009, i64 3
      // (4) %tmp1011 = load void(%struct.test01*, i8*)*,
      //                     void(%struct.test01*, i8*)** %tmp1010
      // (5) %tmp1012 = bitcast void(%struct.test01*, i8*)* %tmp1011 to i8*
      // (6) %tmp1013 = bitcast void(%struct.test01impl*, i8*)* @helper_test01
      //                        to i8*
      // (7) %tmp1014 = icmp eq i8* %tmp1012, %tmp1013
      //
      // Infer type for (1) requires examining its user (2)
      // Examining (2) requires inferring from its user (3)
      // Examining (3) requires inferring from its user (4)
      // Examining (4) requires inferring from its user (5)
      // Examining (5) requires inferring from its user (7)
      // Examining (6) requires inferring from its user (7)
      // (7) will attempt to analyze the dependent values (5) and (6) for
      // types, which will trigger trying to infer the type for (5). This
      // inference will reach this test while the original inference of (1)
      // is still in progress.
      if (!isInferInProgress() && !ValueToInfer->user_empty()) {
        ResultInfo->setUnhandled();
        LLVM_DEBUG(dbgs() << "Infer did not find any types for:"
                          << *ValueToInfer << "\n");
      }
      return;
    }

    for (auto Ty : It->second)
      ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Use, Ty);
  }

  // Walk all the users of \p ValueToInfer, updating the ValueToInferredTypes
  // mapping with types that ValueToInfer is used as.
  void inferTypeFromUseImpl(Value *ValueToInfer) {
    // Simple types can be directly added to the mapping.
    llvm::Type *Ty = ValueToInfer->getType();
    if (!dtrans::hasPointerType(Ty)) {
      DTransType *DType = TM.getOrCreateSimpleType(Ty);
      assert(DType && "Expected simple type");
      addInferredType(ValueToInfer, DType);
      return;
    }

    if (!InferInProgress.insert(ValueToInfer).second)
      return;

    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE,
                      dbgs()
                          << "    Begin infer for: " << *ValueToInfer << "\n");

    // Try to find the type from looking at all the users. In some cases, the
    // instruction will contain the type, such as 'load i64, p0 %x', or
    // information can be retrieved from the metadata about call parameters. In
    // other cases, this will require further look-ahead to examine the users of
    // the produced value, such as for a bitcast.
    for (auto *User : ValueToInfer->users()) {
      DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
        dbgs() << "      User: ";
        printValue(dbgs(), User);
        dbgs() << "\n";
      });

      // Try to infer a usage type for case like:
      //   getelementptr (%ty2, %ty2* (bitcast %ty1* @V to %ty2*), i64 0, i32 0)
      if (auto *GEPOp = dyn_cast<GEPOperator>(User)) {
        inferGetElementPtr(ValueToInfer, GEPOp);
        continue;
      }

      if (auto *I = dyn_cast<Instruction>(User))
        switch (I->getOpcode()) {
        default: {
          // TODO: This switch table may need to be expanded with other
          // instructions. For now, go conservative on instructions not in this
          // switch.
          ValueTypeInfo *Info = PTA.getOrCreateValueTypeInfo(ValueToInfer);
          Info->setUnhandled();
          break;
        }
        case Instruction::BitCast:
        case Instruction::PHI:
        case Instruction::Select:
          // Look-ahead at the users.
          inferTypeFromUseImpl(User);
          propagateInferenceSet(User, ValueToInfer, DerefType::DT_SameType);
          break;
        case Instruction::Call:
        case Instruction::Invoke:
          inferCall(ValueToInfer, cast<CallBase>(User));
          break;
        case Instruction::GetElementPtr:
          inferGetElementPtr(ValueToInfer, cast<GEPOperator>(User));
          break;
        case Instruction::ICmp:
          inferICmpInst(ValueToInfer, cast<ICmpInst>(User));
          break;
        case Instruction::Load:
          inferLoadInst(ValueToInfer, cast<LoadInst>(User));
          break;
        case Instruction::Ret:
          inferRetInst(ValueToInfer, cast<ReturnInst>(User));
          break;
        case Instruction::PtrToInt:
          // Look ahead at the users of the integer, because if this is
          // converting the pointer to a pointer-sized integer, there may be
          // information gained by looking at the users of it.
          inferPtrToIntInst(ValueToInfer, cast<PtrToIntInst>(User));
          break;
        case Instruction::Store:
          inferStoreInst(ValueToInfer, cast<StoreInst>(User));
          break;
        }
    }

    InferInProgress.erase(ValueToInfer);
    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE,
                      dbgs() << "    End infer for: " << *ValueToInfer << "\n");

    return;
  }

  // Try to determine the type of the GEP pointer operand.
  void inferGetElementPtr(Value *ValueToInfer, GEPOperator *GEP) {
    // GEPs that have a simple source type being indexed into are inferred as
    // being that type.
    //   %x = getelementptr %struct.arc, p0 %ptr, i64 0, i32
    // %ptr is being used as a %struct.arc
    //
    // GEPs that are indexing with a pointer type, need to perform look-ahead
    // for the type.
    //   %x = getelementptr p0, p0 %ptr, i64 5
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

    inferTypeFromUseImpl(GEP);
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
        if (P.first && P.second) {
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

    // Disallow "icmp <pred> p0 %x, %x" since it should not occur, and we cannot
    // get information about one argument by examining the other.
    if (OtherOp == ValueToInfer)
      return;

    // Compiler constants do not provide information about the operand to be
    // inferred, but do not return 'false' because other uses of the operand may
    // provide the type.
    if (isCompilerConstant(OtherOp))
      return;

    ValueTypeInfo *ValInfo = analyzeValue(OtherOp);
    for (auto *DType : ValInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
      addInferredType(ValueToInfer, DType);
  }

  // Try to infer the type of the pointer operand in a LoadInst based on either
  // the loaded type (%s = load i64, p0 %x) or by looking at the uses of the
  // loaded value (%p = load p0, p0 %y).
  void inferLoadInst(Value *ValueToInfer, LoadInst *LI) {
    llvm::Type *LoadType = LI->getType();
    if (TM.isSimpleType(LoadType)) {
      DTransType *DType = TM.getOrCreateSimpleType(LoadType);
      assert(DType && "Expected simple type");
      addInferredType(LI->getPointerOperand(),
                      TM.getOrCreatePointerType(DType));
      return;
    }

    inferTypeFromUseImpl(LI);
    propagateInferenceSet(LI, ValueToInfer, DerefType::DT_PointerToType);
  }

  // Try to infer a type used in a StoreInst. The ValueToInfer may be either the
  // stored valued, in which case we look at the pointer operand. Or, it may be
  // the pointer operand, in which case we look at the value stored.
  void inferStoreInst(Value *ValueToInfer, StoreInst *SI) {
    // if the stored type is known, and the pointer operand is the one to be
    // inferred, then the answer is easy. e.g. store i32, p0 %y
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

    if (PtrOp == ValueToInfer) {
      // We cannot infer anything new from a compiler constant used to
      // represent the nullptr, so just treat the pointer operand as being the
      // declared type to avoid it being marked as unhandled from not having any
      // inferred types.
      if (isCompilerConstant(ValOp)) {
        ValueTypeInfo *PtrInfo = analyzeValue(PtrOp);
        for (auto *DType :
             PtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
          addInferredType(PtrOp, DType);

        return;
      }

      // Value being inferred is the pointer operand, analyze the value
      // operand to determine a type for the pointer operand
      ValueTypeInfo *ValInfo = analyzeValue(ValOp);
      for (auto *DType :
           ValInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use)) {
        addInferredType(ValOp, DType);
        addInferredType(PtrOp, TM.getOrCreatePointerType(DType));
      }
      return;
    }

    // Value being inferred is the value operand, analyze the pointer
    // operand to determine the type being stored.
    ValueTypeInfo *PtrInfo = analyzeValue(PtrOp);
    for (auto *DType : PtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
      if (auto *PtrTy = dyn_cast<DTransPointerType>(DType)) {
        DTransType *InferredValTy = PtrTy->getPointerElementType();
        if (InferredValTy->isAggregateType()) {
          auto ElemZeroTy = PTA.getElementZeroType(InferredValTy);
          if (ElemZeroTy && ElemZeroTy.getValue().second->isPointerTy())
            InferredValTy = ElemZeroTy.getValue().second;
        }
        addInferredType(ValOp, InferredValTy);
        addInferredType(PtrOp, DType);
      }
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
        ValueTypeInfo *PtrOpInfo = analyzeValue(PtrOp);
        for (auto *DType :
             PtrOpInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
          if (auto *PtrTy = dyn_cast<DTransPointerType>(DType))
            addInferredType(PTI, PtrTy->getPointerElementType());
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
    auto It = ValueToInferredTypes.find(From);
    if (It == ValueToInferredTypes.end())
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
    if (ValueToInferredTypes[V].insert(DType).second)
      DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
        dbgs() << "    - Inferred: ";
        printValue(dbgs(), V);
        dbgs() << " as " << *DType << "\n";
      });
  }

  bool isInferInProgress() const { return !InferInProgress.empty(); }

  // Build a stack of values that must be analyzed to compute the type of V.
  void populateDependencyStack(Value *V,
                               SmallVectorImpl<Value *> &DependentVals) {
    // This helper pushes a value on the back of the dependency stack
    // and returns true if this is the first occurrence of the value on the
    // stack or false if it was present before the call. The value is pushed
    // onto the stack in either case. A return value of 'true' indicates that
    // dependents of the Value also need to be added to the stack.
    auto AddDependency = [this](Value *DV, SmallVectorImpl<Value *> &DepStack) {
      // If the dependency has already been completely analyzed, skip adding the
      // item to the stack and return false because all of the values that
      // depend on it have also been analyzed.
      if (auto *Info = PTA.getValueTypeInfo(DV))
        if (Info->isCompletelyAnalyzed())
          return false;

      auto REnd = DepStack.rend();
      auto It = std::find(DepStack.rbegin(), REnd, DV);
      DepStack.push_back(DV);
      return (It == REnd);
    };

    if (auto *I = dyn_cast<Instruction>(V)) {
      switch (I->getOpcode()) {
      default:
        break;
      case Instruction::BitCast:
        if (AddDependency(I->getOperand(0), DependentVals))
          populateDependencyStack(I->getOperand(0), DependentVals);
        break;

      case Instruction::GetElementPtr: {
        auto *GEP = cast<GetElementPtrInst>(I);
        Value *BasePtr = GEP->getPointerOperand();
        if (AddDependency(BasePtr, DependentVals))
          populateDependencyStack(BasePtr, DependentVals);
        break;
      }
      case Instruction::IntToPtr:
      case Instruction::PtrToInt:
        if (AddDependency(I->getOperand(0), DependentVals))
          populateDependencyStack(I->getOperand(0), DependentVals);
        break;
      case Instruction::Load: {
        auto *LI = cast<LoadInst>(I);
        Value *Ptr = LI->getPointerOperand();
        if (AddDependency(Ptr, DependentVals))
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
          if (AddDependency(Dep, DependentVals))
            DepsAdded.push_back(Dep);

        for (Value *Dep : DepsAdded)
          populateDependencyStack(Dep, DependentVals);
        break;
      }
      case Instruction::Select: {
        auto *Sel = cast<SelectInst>(V);
        Value *TV = Sel->getTrueValue();
        Value *FV = Sel->getFalseValue();
        bool TrueWasNew = AddDependency(TV, DependentVals);
        bool FalseWasNew = AddDependency(FV, DependentVals);
        if (TrueWasNew)
          populateDependencyStack(TV, DependentVals);
        if (FalseWasNew)
          populateDependencyStack(FV, DependentVals);
        break;
      }
        // TODO: Add other instructions types the need to be analyzed to switch
        // table.
      }
    }
  }

  void dumpDependencyStack(Value *V, SmallVectorImpl<Value *> &DependentVals) {
    dbgs() << "  DependentVals for:";
    printValue(dbgs(), V, /*ReportFuncName=*/false);
    dbgs() << "\n";
    for (auto *Dep : DependentVals) {
      dbgs() << "    ";
      printValue(dbgs(), Dep, /*ReportFuncName=*/false);
      dbgs() << "\n";
    }
    dbgs() << "\n";
  }

  void analyzeArgument(Argument *Arg, ValueTypeInfo *ResultInfo) {
    if (!hasPointerType(Arg->getType()))
      return;

    Function *F = Arg->getParent();
    ValueTypeInfo *FuncInfo = PTA.getValueTypeInfo(F);
    assert(FuncInfo &&
           "Expected VisitModule to create info for all defined functions");

    // We expect a declaration type set with one pointer entry to describe the
    // function
    auto &AliasSet = FuncInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl);
    if (AliasSet.size() != 1) {
      ResultInfo->setUnhandled();
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
    //   define @qsort(p0 %arg)    ; @qsort(i8* %arg)
    //   %tmp = ptrtoint p0 to i64
    //   call @ar_compare(p0 %arg) ; @ar_compare(%struct.ar* %arg)
    //
    // In this case, to capture the ptrtoint as operating on a pointer to a
    // structure type, we need to look-ahead to where %arg is used, and infer
    // the argument is expected to be a %struct.ar* type.
    if (ArgTy == PTA.getDTransI8PtrType())
      inferTypeFromUse(Arg, ResultInfo);
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
      dtrans::DTransType *DTy = TM.getOrCreateSimpleType(Ty);
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
    ValueTypeInfo *SrcInfo = PTA.getOrCreateValueTypeInfo(BC->getOperand(0));
    propagate(SrcInfo, ResultInfo, /*Decl=*/true, /*Use=*/true,
              DerefType::DT_SameType);
    if (!SrcInfo->isCompletelyAnalyzed())
      ResultInfo->setPartiallyAnalyzed();

    if (SrcInfo->getUnhandled() || SrcInfo->getDependsOnUnhandled())
      ResultInfo->setDependsOnUnhandled();

    // Next, perform look-ahead at the uses of the BitCast result, to infer the
    // target type.
    inferTypeFromUse(BC, ResultInfo);
  }

  // For a call that results in a pointer type, we want to update the
  // ResultInfo with type aliases. We also want to update the ValueTypeInfo
  // of the parameters to reflect the usage type in case a pointer is being
  // passed to a function that is declared as taking a pointer of a different
  // type.
  void analyzeCallBase(CallBase *Call, ValueTypeInfo *ResultInfo) {
    // Check if the return type should have a pointer type, and the type, if so.
    std::pair<bool, DTransType *> OptRetType = getCallReturnType(Call);
    if (OptRetType.first) {
      if (OptRetType.second) {
        ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl, OptRetType.second);

        // An indirect call that returns an i8* should be checked for the
        // actual type it gets used as.
        if (Call->isIndirectCall() &&
            OptRetType.second == PTA.getDTransI8PtrType())
          inferTypeFromUse(Call, ResultInfo);
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
    auto AllocKind = dtrans::getAllocFnKind(Call, TLI);
    if (AllocKind != dtrans::AK_NotAlloc) {
      inferTypeFromUse(Call, ResultInfo);

      // Track the VAT_Decl type of the allocation as the type the
      // object gets used as.
      auto It = ValueToInferredTypes.find(Call);
      if (It != ValueToInferredTypes.end())
        for (auto Ty : It->second)
          ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl, Ty);
    }

    // Set the usage type for the call arguments.
    unsigned NumArg = Call->arg_size();
    for (unsigned AI = 0; AI < NumArg; ++AI) {
      Value *Arg = Call->getArgOperand(AI);
      if (hasPointerType(Arg->getType())) {
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
    if (!hasPointerType(Call->getType()))
      return {false, nullptr};

    // Check if there is metadata for information available to use from a called
    // function or an indirect call.
    if (Call->isIndirectCall()) {
      DTransType *DType = MDReader.getDTransTypeFromMD(Call);
      if (DType) {
        auto *FnType = cast<DTransFunctionType>(DType);
        dtrans::DTransType *DRetTy = FnType->getReturnType();
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

    // Helper lambda for getting the type returned by a library call.
    // TODO: For now, just hard code some common cases that are of interest
    // since there are not many functions that return pointers. In the future,
    // we may need a metadata table.
    auto GetLibCallReturnType =
        [this](LibFunc TheLibFunc) -> dtrans::DTransType * {
      switch (TheLibFunc) {
      default:
        return nullptr;

        // Functions that are known to just return an i8*
      case LibFunc_cxa_allocate_exception:
      case LibFunc_calloc:
      case LibFunc_fgets_unlocked:
      case LibFunc_malloc:
      case LibFunc_realloc:
      case LibFunc_strcpy:
        return PTA.getDTransI8PtrType();

        // Functions that return the system object for FILE*
      case LibFunc_fopen:
        return getDTransIOPtrTy();
      }
    };

    // Check whether the return value will be of interest.
    llvm::Type *RetType = F->getReturnType();
    if (!hasPointerType(RetType))
      return {false, nullptr};

    // Handle functions with metadata descriptions.
    DTransType *DType = MDReader.getDTransTypeFromMD(F);
    if (DType) {
      auto *FnType = cast<DTransFunctionType>(DType);
      dtrans::DTransType *DRetTy = FnType->getReturnType();
      return {true, DRetTy};
    }

    // TODO: Add a list of types returned by intrinsic functions, if there are
    // any of interest.
    if (F->isIntrinsic())
      return {true, nullptr};

    // Check if the call is to a libfunc. If so, use a table to get the
    // type produced by the call.
    LibFunc TheLibFunc;
    if (TLI.getLibFunc(F->getName(), TheLibFunc) && TLI.has(TheLibFunc)) {
      return {true, GetLibCallReturnType(TheLibFunc)};
    }

    // The function returns a type of interest, but we do not have information
    // about the type. In the future, it may be possible to analyze the return
    // instructions within the function body when there is missing metadata to
    // try to resolve the return type.
    return {true, nullptr};
  }

  // Determine the type that a function is expected to use the argument as.
  std::pair<bool, DTransType *> getArgumentType(CallBase *Call, unsigned Idx) {
    // Helper lambda for getting the argument type of an intrinsic function. The
    // majority of intrinsics do not have pointer arguments, and the few that do
    // usually just take an i8*. A simple table here will suffice for now.
    auto GetIntrinsicArgumentType =
        [this](Intrinsic::ID IntrinId,
               unsigned Idx) -> std::pair<bool, DTransType *> {
      switch (IntrinId) {
      default:
        LLVM_DEBUG(
            dbgs()
            << "Warning intrinsic not in table. Unknown pointer type for Idx @ "
            << Idx << "\n");
        return {true, nullptr};

        // We do not need to the treat the argument to these intrinsics as being
        // used as a pointer type.
      case Intrinsic::dbg_addr:
      case Intrinsic::dbg_declare:
      case Intrinsic::dbg_label:
      case Intrinsic::dbg_value:
        return {false, nullptr};

        // Any pointer argument to these intrinsics is being used as i8*
      case Intrinsic::lifetime_end:
      case Intrinsic::lifetime_start:
      case Intrinsic::memcpy:
      case Intrinsic::memmove:
      case Intrinsic::memset:
      case Intrinsic::prefetch:
      case Intrinsic::vacopy:
      case Intrinsic::vaend:
      case Intrinsic::vastart:
        break;

        // TODO:  Add cases where specific pointer argument is not i8*, if
        // needed
      }

      return {true, PTA.getDTransI8PtrType()};
    };

    // Helper lambda for getting the argument type of a library call. Most of
    // these just take i8*.
    auto GetLibCallArgumentType =
        [this](LibFunc TheLibFunc,
               unsigned Idx) -> std::pair<bool, DTransType *> {
      // It should be safe to treat any pointer as an i8*, except in the few
      // rare cases where a pointer to a structure is used. For now, a more
      // conservative approach is taken of requiring the LibFunc to be
      // explicitly listed in the table.
      switch (TheLibFunc) {
      default:
        LLVM_DEBUG(
            dbgs()
            << "Warning LibFunc not in table. Unknown pointer type for Idx @ "
            << Idx << "\n");
        return {true, nullptr};

        // Any pointer argument to these LibFuncs is being used as i8*
      case LibFunc_dunder_isoc99_scanf:
      case LibFunc_dunder_isoc99_sscanf:
      case LibFunc_fopen:
      case LibFunc_free:
      case LibFunc_printf:
      case LibFunc_puts:
      case LibFunc_realloc:
      case LibFunc_sprintf:
      case LibFunc_strcpy:
        break;

        // Handle cases where the argument may be used as something other than
        // i8*. Argument positions not listed will be treated as i8*.
      case LibFunc_fgets_unlocked:
        if (Idx == 2)
          return {true, getDTransIOPtrTy()};
        break;

      case LibFunc_fclose:
      case LibFunc_fflush:
      case LibFunc_ftell:
        if (Idx == 0)
          return {true, getDTransIOPtrTy()};

        break;

      case LibFunc_fread:
      case LibFunc_fwrite:
        if (Idx == 3)
          return {true, getDTransIOPtrTy()};

        break;

      case LibFunc_fprintf:
        if (Idx == 0)
          return {true, getDTransIOPtrTy()};

        break;

      case LibFunc_strtol:
        if (Idx == 1)
          return {true,
                  TM.getOrCreatePointerType(PTA.getDTransI8PtrType())}; // i8**
        break;
      }

      // All cases that exit the switch table means the argument is an i8*.
      return {true, PTA.getDTransI8PtrType()};
    };

    if (!hasPointerType(Call->getArgOperand(Idx)->getType()))
      return {false, nullptr};

    DTransType *DType = nullptr;
    Function *Target = dtrans::getCalledFunction(*Call);
    if (Call->isIndirectCall())
      DType = MDReader.getDTransTypeFromMD(Call);
    else if (Target)
      DType = MDReader.getDTransTypeFromMD(Target);

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

    if (Target->isIntrinsic())
      return GetIntrinsicArgumentType(Target->getIntrinsicID(), Idx);

    const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
    LibFunc TheLibFunc;
    if (TLI.getLibFunc(Target->getName(), TheLibFunc) && TLI.has(TheLibFunc))
      return GetLibCallArgumentType(TheLibFunc, Idx);

    return {true, nullptr};
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

    auto ProcessIndexedElement = [&GetGEPIndexedType, this, &GEP,
                                  ResultInfo](DTransType *Ty,
                                              SmallVector<Value *, 4> &GepOps) {
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
      //    [8 x p0] --> type is kind of pointer stored in array.
      // - 3 or more operands, type is multi-dimensional array.  Iterate
      //   until type is reached.
      //
      // If this function returns 'false', an appropriate type alias could not
      // be found with 'Ty' using the 'GepOps'. The caller should mark the
      // ResultInfo as 'unhandled' if this index lookup was not being done
      // speculatively.
      //
      dtrans::DTransType *IndexedTy = GetGEPIndexedType(Ty, GepOps);
      if (!IndexedTy)
        return false;

      if (auto *IndexedStTy = dyn_cast<DTransStructType>(IndexedTy)) {
        // The final argument of the GEP of a structure field element must
        // always be a constant value
        auto *LastArg =
            cast<ConstantInt>(GEP.getOperand(GEP.getNumOperands() - 1));
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

        // Access is to an element of the structure, update element pointee
        // info.
        Value *LastArg = GEP.getOperand(GEP.getNumOperands() - 1);
        auto *ConstIdx = dyn_cast<ConstantInt>(LastArg);
        if (ConstIdx) {
          uint64_t ElemNum = ConstIdx->getLimitedValue();
          ResultInfo->addElementPointee(ValueTypeInfo::VAT_Decl, IndexedTy,
                                        ElemNum);
        } else {
          // TODO: LocalPointerAnalyzer treats this as an element pointee at
          // index 0 when DTransOutOfBoundsOk = false for some reason, but not
          // when DTransOutOfBounds = true. Mimic this behavior.
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

    // When opaque pointers are used, the known type of pointer may not
    // match the indexed type, so add that type as one of the 'usage' types
    // now. This is done for simple types, such as:
    //   %100 = getelementptr %testB, p0 %known.as.testA, i32 4
    // TODO: For non-simple source element types, we may need more metadata to
    // indicate the type that is being indexed to detect when the type is
    // different than expected.
    if (!GEP.getPointerOperandType()->isVectorTy() && TM.isSimpleType(SrcTy)) {
      DTransType *DTransSrcTy = TM.getOrCreateSimpleType(SrcTy);
      assert(DTransSrcTy && "Expected simple type");
      PointerInfo->addTypeAlias(ValueTypeInfo::VAT_Use,
                                TM.getOrCreatePointerType(DTransSrcTy));
    }

    if (GEP.getNumIndices() > 1) {
      // Mark the pointer operand with the type used for indexing.
      //
      // Cases to handle:
      // Case 1: getelementptr <SimpleTy>, p0 %x, i64 0, i32 1
      //   where SimpleTy is a named structure or simple array definition.
      //
      // Case 2: getelementptr <ComplexTy>, p0 %x, i64 0, i32
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
      for (auto *Alias :
           PointerInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl)) {
        bool Handled = false;
        if (Alias->isPointerTy())
          Handled = ProcessIndexedElement(Alias->getPointerElementType(), Ops);

        if (!Handled) {
          // We may need to have metadata on the some GEP instructions that
          // defines the indexed type, if we encounter this case.
          PointerInfo->setUnhandled();
          ResultInfo->setDependsOnUnhandled();
          LLVM_DEBUG(dbgs() << "unable to resolve index type: " << GEP << "\n");
        }
      }
      return;
    }

    // If the GEP is of the form:
    //   %y = getelementptr i8, p0 %x, i64 <n>
    // Check if the GEP should be processed as either a byte-flattened GEP or an
    // array of 'i8' elements.
    bool ProcessedAsByteFlattenedGEP = false;
    if (SrcTy == PTA.getLLVMI8Type()) {
      ProcessedAsByteFlattenedGEP =
          analyzePotentialByteFlattenedGEPAccess(GEP, ResultInfo);
      ResultInfo->addTypeAlias(ValueTypeInfo::VAT_Decl,
                               PTA.getDTransI8PtrType());
    }

    // Zero or Single index operand GEPs of the form, result in the same
    // type as the base pointer.
    // For example, these cases result in the same type identified for %x:
    //   getelementptr p0, p0 %x, i64 5
    //   getelementptr <ty>, <ptr vector> %x, <vector index type> %idx
    //
    // These are treating the source operand as an array, and therefore are
    // generating the same type as the pointer operand. However, there is a
    // special case when indexed type is an i8, since it may be a
    // byte-flattened GEP that needs to be analyzed.
    if (!ProcessedAsByteFlattenedGEP)
      propagate(PointerInfo, ResultInfo, /*Decl=*/true, /*Use=*/true,
                DerefType::DT_SameType);
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
  bool analyzePotentialByteFlattenedGEPAccess(GEPOperator &GEP,
                                              ValueTypeInfo *ResultInfo) {

    assert(GEP.getNumIndices() == 1 && "Only expecting single index value GEP");

    // The following conditions exclude it from being either a byte-flattened
    // GEP or an 'i8' array:
    // - The type alias list saw the type as a pointer-to-pointer.
    // - The type alias list does not contain any aggregate types
    Value *BasePtr = GEP.getPointerOperand();
    ValueTypeInfo *BasePtrInfo = PTA.getOrCreateValueTypeInfo(BasePtr);
    bool HasStructType = false;
    bool HasArrayType = false;
    for (auto *AliasTy :
         BasePtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use)) {
      if (!AliasTy->isPointerTy())
        continue;

      DTransType *ElemType = AliasTy->getPointerElementType();
      if (ElemType->isPointerTy())
        return false;

      if (ElemType->isStructTy()) {
        HasStructType = true;
      } else if (ElemType->isArrayTy()) {
        HasArrayType = true;

        // Also, check for array of structures.
        DTransType *BaseType =
            getSequentialObjectBaseType(cast<DTransSequentialType>(ElemType));
        if (BaseType->isStructTy())
          HasStructType = true;
      }
    }

    if (!HasStructType && !HasArrayType)
      return false;

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
        return true;
      }

      return false;
    }

    // If it's just a char array being accessed, then add the element based on
    // the offsets collected.
    DTransArrayType *AggArType = nullptr;
    if (PTA.isPtrToCharArray(*BasePtrInfo, &AggArType)) {
      for (auto &APOffsetVal : APOffset)
        ResultInfo->addElementPointee(ValueTypeInfo::VAT_Use, AggArType,
                                      APOffsetVal.getLimitedValue());

      return true;
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
        return true;
      }

      return false;
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
      return true;
    }

    // All offsets were valid, merge the element pointees collected into
    // ResultInfo, and save the byte flattened GEP information for use by the
    // safety analyzer.
    propagate(&LocalInfo, ResultInfo, /*Decl=*/true, /*Use=*/true,
              DerefType::DT_SameType);
    for (auto &TyIdxPair : PendingByteGEPs)
      PTA.addByteFlattenedGEPMapping(&GEP, TyIdxPair.first, TyIdxPair.second);

    return true;
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
      } else if (valueOnlyUsedForMemset(&GEP)) {
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
    // Propagate any pointer type information that has been identified for the
    // source operand to the result info.
    Value *Src = ITP->getOperand(0);
    ValueTypeInfo *SrcInfo = PTA.getOrCreateValueTypeInfo(Src);
    propagate(SrcInfo, ResultInfo, true, true, DerefType::DT_SameType);

    if (SrcInfo->getUnhandled() || SrcInfo->getDependsOnUnhandled())
      ResultInfo->setDependsOnUnhandled();

    if (!SrcInfo->isCompletelyAnalyzed())
      ResultInfo->setPartiallyAnalyzed();

    // Currently, this only supports converting an integer that is the
    // result of a PtrToInt instruction,
    //   %i = ptrtoint %struct.foo* to i64
    //   %p = inttoptr i64 %i to %struct.foo*
    //
    // All other uses will be set to unhandled because it would require
    // analyzing all the integer arithmetic operations that may be modifying the
    // pointer. Special cases that need to analyze patterns of this sort
    // for memory allocation buffers should be processed prior to calling
    // analyzeValue and marked as completely analyzed to avoid being flagged as
    // unhandled here.
    //   For example:
    //     %alloc = call %struct.foo* @foo_allocator()
    //     %i = ptrtoint %struct.foo* %alloc to i64
    //     %r2 = shr i64 %i, 2
    //     %l8 = shl i64 %r2, 2
    //     %a8 = add i64 %l8, 8
    //     %p = inttoptr i64 %a8 to %struct.foo*
    Value *PTI = dyn_cast<PtrToIntInst>(Src);
    if (PTI)
      return;

    ResultInfo->setUnhandled();
    LLVM_DEBUG(dbgs() << "IntToPtr not tracked to source type: " << *ITP
                      << "\n");
  }

  void analyzeLoadInst(LoadInst *LI, ValueTypeInfo *ResultInfo) {
    // We cannot just take the value type of the load, and set that as
    // the type of value produced because the pointer may be a
    // pointer-to-pointer of some type.
    //   %val = load i64, p0 %ptr
    // In this case, if %ptr referred to a %struct.foo**, then the type of %val
    // needs to include %struct.foo*. Therefore, we propagate whatever type info
    // was available for the source pointer to the load result, with 1 less
    // level of indirection.
    //
    // Also, need to be able to handle the special case of:
    //   %143 = load i64, p0 undef
    //

    // This lambda function is used for cases where the load is a pointer type,
    // or may be a pointer type loaded as a pointer-sized int.
    //
    // Case 1) %y = load p0, p0 %x
    // Case 2) %y = load i64, p0 %x
    //   If %x is a pointer to a pointer, then the pointer type should be
    //   propagated to %y.
    //   If %x is a pointer to an aggregate, this could be an element-zero
    //   access. If an element-zero can be resolved to a pointer type after
    //   traversing any nested elements, add that type to %y, and update the
    //   element pointees on the PointerInfo.
    //
    // Case 3) %y = load { p0, p0 }, { p0, p0 }* %x
    //   In this case, the call sets 'LoadingAggregateType' to indicate the
    //   structure should not be traversed as a potential zero-element load.
    //
    auto PropagateDereferencedType =
        [](ValueTypeInfo *PointerInfo, ValueTypeInfo *ResultInfo,
           ValueTypeInfo::ValueAnalysisType Kind, bool LoadingAggregateType) {
          for (auto *Alias : PointerInfo->getPointerTypeAliasSet(Kind)) {
            DTransType *PropAlias = nullptr;
            if (!Alias->isPointerTy())
              continue;

            PropAlias = Alias->getPointerElementType();
            if (!LoadingAggregateType && PropAlias->isAggregateType()) {
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
                PointerInfo->addElementPointee(Kind, PrevNestedType, 0);
              if (NestedType)
                ResultInfo->addTypeAlias(Kind, NestedType);

            } else {
              ResultInfo->addTypeAlias(Kind, PropAlias);
            }
          }
        };

    llvm::Type *ValTy = LI->getType();

#if 0
    // TODO: This commented out code will be needed when opaque pointers are
    // enabled.  This code cannot be enabled until the method 'isOpaque' is
    // available on llvm::PointerType objects. When a pointer type is loaded,
    // it's possible the pointer will be used as a different type than expected,
    // therefore we need to infer the type based on the usage. For example:
    //   %struct.T1 = { i64** }
    //   %pField = getelementptr %struct.T1, p0 %pStruct, i64 0, i32 0
    //   %vP2P = load p0, p0 %pField
    //   %vP   = load p0, p0 %vP2P
    //   %v    = load i32, p0 %vP
    //
    // %pField would be expected to be i64***, but then %vP2P gets used as if it
    // were a i32** type. This code cannot be enabled until the method
    // 'isOpaque' is available on llvm::PointerType objects.
    if (ValTy->isPointerTy() && cast<llvm::PointerType>(ValTy)->isOpaque())
      inferTypeFromUse(LI, ResultInfo);
#endif

    bool ExpectPtrType = hasPointerType(ValTy);
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
    if (!hasPointerType(LoadType)) {
      PointerInfo->addTypeAlias(
          ValueTypeInfo::VAT_Use,
          TM.getOrCreatePointerType(TM.getOrCreateSimpleType(LoadType)));
    }

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
      auto *PTy = dyn_cast<dtrans::DTransPointerType>(Ty);
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
          LLVM_DEBUG(dbgs() << "Warning: Could not create element of requested "
                               "deref level when propagating.\nFrom: "
                            << *SrcInfo << "\nTo: " << *DestInfo << "\n");
      }

      // The element pointer set does not need to be transferred when the level
      // of indirection is being changed. For example:
      //   %x = getelementptr %struct.node, p0 %y, i64 0, i32 2
      //   %z = load p0, p0 %x
      // %x contains a pointer alias type and an element pointee of a structure,
      // but %z is just being updated to be the pointee type from %x.
      if (DerefLevel == DerefType::DT_SameType)
        for (auto PointeePair : SrcInfo->getElementPointeeSet(Kind))
          if (PointeePair.second.getKind() ==
              ValueTypeInfo::PointeeLoc::PLK_Field)
            LocalChanged |= DestInfo->addElementPointee(
                Kind, PointeePair.first, PointeePair.second.getElementNum());
          else
            LocalChanged |= DestInfo->addElementPointeeByOffset(
                Kind, PointeePair.first, PointeePair.second.getByteOffset());

      return LocalChanged;
    };

    bool Changed = false;
    if (Decl)
      Changed |= DoPropagate(ValueTypeInfo::VAT_Decl);
    if (Use)
      Changed |= DoPropagate(ValueTypeInfo::VAT_Use);

    return Changed;
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
      // looking for a conversion of the value back to a pointer, but we do not
      // expect to see a ptrtoint on a null/undef value, so just treat it as
      // unhandled.
      ResultInfo->setUnhandled();
      LLVM_DEBUG(dbgs() << "PtrToInt from constant is not handled: " << *PTI
                        << "\n");
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
    for (auto *ValIn : IncomingVals) {
      // It's possible the operand to the select or phi is 0 or null, ignore
      // these because they do not supply type information useful for
      // determining the type of the result.
      if (isCompilerConstant(ValIn))
        continue;

      ValueTypeInfo *SrcInfo = PTA.getOrCreateValueTypeInfo(ValIn);
      propagate(SrcInfo, ResultInfo, true, true, DerefType::DT_SameType);

      if (SrcInfo->getUnhandled() || SrcInfo->getDependsOnUnhandled())
        ResultInfo->setDependsOnUnhandled();

      if (!SrcInfo->isCompletelyAnalyzed())
        ResultInfo->setPartiallyAnalyzed();
    }
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
      analyzeBitCastOperator(BCOp, Info);
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

  dtrans::DTransPointerType *getDTransIOPtrTy() const { return DTransIOPtrTy; }

  ////////////////////////////////////////////////////////////////////////////////
  // Start of member data
  ////////////////////////////////////////////////////////////////////////////////

  PtrTypeAnalyzerImpl &PTA;
  DTransTypeManager &TM;
  TypeMetadataReader &MDReader;
  const DataLayout &DL;
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;

  // Representation of %struct._IO_FILE*
  dtrans::DTransPointerType *DTransIOPtrTy;

  // Keep a mapping of a set of types that are inferred for a Value that is
  // constructed when a type needs to be inferred by doing a look-ahead walk of
  // the users of the value.
  std::map<Value *, SmallPtrSet<DTransType *, 4>> ValueToInferredTypes;
  SmallPtrSet<Value *, 8> InferInProgress;
};

////////////////////////////////////////////////////////////////////////////////
//
// Implementation of ValueTypeInfo class
//
////////////////////////////////////////////////////////////////////////////////

bool ValueTypeInfo::addTypeAlias(ValueAnalysisType Kind,
                                 dtrans::DTransType *Ty) {
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
    dtrans::DTransType *Base = Ty;
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
                                      dtrans::DTransType *BaseTy,
                                      size_t ElemIdx) {

  PointeeLoc Loc(PointeeLoc::PLK_Field, ElemIdx);
  bool Changed = addElementPointeeImpl(Kind, BaseTy, Loc);
  return Changed;
}

bool ValueTypeInfo::addElementPointeeByOffset(ValueAnalysisType Kind,
                                              dtrans::DTransType *BaseTy,
                                              size_t ByteOffset) {
  PointeeLoc Loc(PointeeLoc::PLK_Offset, ByteOffset);
  bool Changed = addElementPointeeImpl(Kind, BaseTy, Loc);
  return Changed;
}

bool ValueTypeInfo::addElementPointeeImpl(ValueAnalysisType Kind,
                                          dtrans::DTransType *BaseTy,
                                          PointeeLoc &Loc) {
  bool Changed = ElementPointees[Kind].insert({BaseTy, Loc}).second;
  if (Changed) {
    DEBUG_WITH_TYPE_P(FNFilter, VERBOSE_TRACE, {
      dbgs() << "    Added element:" << (Kind == VAT_Decl ? "[DECL]" : "[USE]")
             << ": ";
      printValue(dbgs(), V);
      dbgs() << " -- " << *BaseTy << " @ ";
      if (Loc.getKind() == PointeeLoc::PLK_Field)
        dbgs() << Loc.getElementNum() << "\n";
      else
        dbgs() << "ByteOffset=" << Loc.getByteOffset() << ")\n";
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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void ValueTypeInfo::dump() const { print(dbgs()); }

void ValueTypeInfo::print(raw_ostream &OS, bool Combined,
                          const char *Prefix) const {
  auto TypeToString = [Prefix](const dtrans::DTransType *Ty) {
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

  auto ElementPointeeToString = [Prefix](
                                    const TypeAndPointeeLocPair &PointeePair) {
    std::string OutputVal;
    raw_string_ostream OutputStream(OutputVal);

    OutputStream << Prefix << "    ";
    if (auto *DTStTy = dyn_cast<dtrans::DTransStructType>(PointeePair.first))
      if (DTStTy->hasName())
        OutputStream << "%" << DTStTy->getName();
      else
        OutputStream << *PointeePair.first;
    else
      OutputStream << *PointeePair.first;
    OutputStream << " @ ";
    if (PointeePair.second.getKind() == PointeeLoc::PLK_Field)
      OutputStream << PointeePair.second.getElementNum();
    else
      OutputStream << "not-field ByteOffset: "
                   << PointeePair.second.getByteOffset();
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
     << "\n";

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
  for (auto &LPI : LocalMap)
    delete LPI.second;
  LocalMap.clear();

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

void PtrTypeAnalyzerImpl::run(Module &M) {
  PtrTypeAnalyzerInstVisitor InstAnalyzer(*this, TM, MDReader, M.getContext(),
                                          DL, GetTLI);
  InstAnalyzer.visit(M);
}

ValueTypeInfo *PtrTypeAnalyzerImpl::getOrCreateValueTypeInfo(Value *V) {
  assert(!isCompilerConstant(V) && "Should not be compiler constant.");
  ValueTypeInfo *Info = getValueTypeInfo(V);
  if (Info)
    return Info;

  Info = new ValueTypeInfo(V);
  LocalMap[V] = Info;
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
  auto It = LocalMap.find(V);
  if (It == LocalMap.end())
    return nullptr;

  return It->second;
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

// Check whether the type for the Value object is something that needs to be
// analyzed for potential pointer types. This differs from just checking the
// Type of V with isTypeOfInterest, because this also needs to consider the
// case of a pointer converted into a pointer sized integer.
bool PtrTypeAnalyzer::isPossiblePtrValue(Value *V) const {
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
  if (ValueTy != Impl->getLLVMPointerSizedIntType())
    return false;

  // If it is a pointer-sized integer, we may need to analyze it if
  // it is the result of a load, select or PHI node.
  if (isa<LoadInst>(V) || isa<SelectInst>(V) || isa<PHINode>(V))
    return true;

  // Otherwise, we don't need to analyze it as a pointer.
  return false;
}

void PtrTypeAnalyzerImpl::setDeclaredType(Value *V, dtrans::DTransType *Ty) {
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

    // If this type can be an element zero access of DomTy, DomTy is still
    // dominant.
    if (isElementZeroAccess(DomTy, AliasTy))
      continue;

    // If what we previously thought was the dominant type can be an element
    // zero access of the current alias, the current alias becomes dominant.
    if (isElementZeroAccess(AliasTy, DomTy)) {
      DomTy = AliasTy;
      continue;
    }

    // Otherwise, there are conflicting aliases and nothing can be dominant.
    return nullptr;
  }

  return DomTy;
}

DTransType *PtrTypeAnalyzerImpl::getDominantType(
    ValueTypeInfo &Info, ValueTypeInfo::ValueAnalysisType Kind) const {
  auto IsGenericPtrType = [this](DTransType *Ty) {
    return Ty == getDTransI8PtrType() || Ty == getDTransPtrSizedIntPtrType();
  };

  // If there are aggregate types, try to find the dominant one.
  if (Info.canAliasToAggregatePointer(Kind))
    return getDominantAggregateType(Info, Kind);

  // If there are no aggregate types, try to find the best match from pointers
  // to scalar types. By best type, this means that if a type is aliased by a
  // generic form (i8* or pointer sized int pointer), ignore those in
  // preference to a non-generic form.
  //   For examples, given the following sets of possibilities:
  //   1)  { i8*, i32* }       -> return i32*
  //   2)  { double*, i64* }   -> return double*
  //   3)  { float*, double* } -> return nullptr
  DTransType *DomTy = nullptr;
  bool HaveGenericDomTy = false;
  for (auto *AliasTy : Info.getPointerTypeAliasSet(Kind)) {
    // Allow replacement of a generic type with a non-generic type, but don't
    // allow a non-generic type to be replaced by another non-generic type.
    if (DomTy && !HaveGenericDomTy && !IsGenericPtrType(AliasTy))
      return nullptr;

    DomTy = AliasTy;
    HaveGenericDomTy = IsGenericPtrType(DomTy);
  }

  return DomTy;
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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void PtrTypeAnalyzer::dumpPTA(Module &M) {
  PtrTypeAnalyzerAnnotationWriter Annotator(*this, PTAEmitCombinedSets);
  M.print(dbgs(), &Annotator);
}

void PtrTypeAnalyzer::printDominantAggregateUsageType(raw_ostream &OS,
                                                      ValueTypeInfo &Info) {
  DTransType *DomTy = getDominantAggregateUsageType(Info);
  if (DomTy)
    OS << "DomTy: " << *DomTy << "\n";
  else if (Info.canAliasToAggregatePointer())
    OS << "Ambiguous Dominant Type\n";
  else
    OS << "No Dominant Type\n";
}

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

} // end namespace dtrans
} // end namespace llvm

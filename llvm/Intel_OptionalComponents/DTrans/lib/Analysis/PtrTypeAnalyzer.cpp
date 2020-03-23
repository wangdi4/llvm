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

#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
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
      if (Info)
        Info->print(OS, CombineUseAndDecl, ";    ");
      else if (A.getType()->isPointerTy())
        OS << ";    <NO PTR INFO AVAILABLE>\n";
    }
  }

  // For pointers and values of interest, print the type information determined
  // for Value \p CV
  void printInfoComment(const Value &CV, formatted_raw_ostream &OS) {
    Value *V = const_cast<Value *>(&CV);

    // Check for any constant expressions being used for the instruction, and
    // report types for those, if available.
    if (auto *I = dyn_cast<Instruction>(V))
      for (auto *Op : I->operand_values())
        if (auto *CE = dyn_cast<ConstantExpr>(Op)) {
          OS << "\n;        CE: " << *CE << "\n";
          auto *Info = Analyzer.getValueTypeInfo(CE);
          if (Info)
            Info->print(OS, CombineUseAndDecl, ";          ");
          else if (CE->getType()->isPointerTy())
            OS << ";          <NO PTR INFO AVAILABLE FOR ConstantExpr>\n";
        }

    // Report the information about the value produced by the instruction.
    llvm::Type *ValueTy = V->getType();
    bool ExpectPointerInfo = ValueTy->isPointerTy() ||
                             (ValueTy->isVectorTy() &&
                              ValueTy->getVectorElementType()->isPointerTy()) ||
                             isa<PtrToIntInst>(V);

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
      DTransTypeManager &TM, TypeMetadataReader &MDReader, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI)
      : TM(TM), MDReader(MDReader), DL(DL), GetTLI(GetTLI) {}

  ~PtrTypeAnalyzerImpl();

  void run(Module &M);

  // Get the ValueTypeInfo object for the Value, creating it if necessary.
  ValueTypeInfo *getOrCreateValueTypeInfo(Value *V);

  // Get the ValueTypeInfo object for the specified operand of the Instruction.
  // This method must be used for a Value object that represents 'null'. It may
  // be used for other Value objects, in which case it just redirects to the
  // above overload. Creates a new ValueTypeInfo object if necessary.
  ValueTypeInfo *getOrCreateValueTypeInfo(const Instruction *I, unsigned OpNum);

  // Get the ValueTypeInfo object, if it exists.
  ValueTypeInfo *getValueTypeInfo(const Value *V) const;
  ValueTypeInfo *getValueTypeInfo(const Instruction *I, unsigned OpNum) const;

  // Set Ty as the declaration type of value V, and mark the ValueTypeInfo as
  // completely analyzed.
  void setDeclaredType(Value *V, DTransType *Ty);

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
  std::map<std::pair<const Instruction *, unsigned>, ValueTypeInfo *>
      LocalMapForConstant;
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
public:
  PtrTypeAnalyzerInstVisitor(
      PtrTypeAnalyzerImpl &PTA, DTransTypeManager &TM,
      TypeMetadataReader &MDReader, LLVMContext &Ctx, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI)
      : PTA(PTA), TM(TM), MDReader(MDReader), DL(DL), GetTLI(GetTLI) {

    PointerSizedIntType = llvm::Type::getIntNTy(Ctx, DL.getPointerSizeInBits());
    DTransI8PtrTy = TM.getOrCreatePointerType(
      TM.getOrCreateSimpleType(llvm::Type::getInt8Ty(Ctx)));
  }

  void visitModule(Module &M) {
    // Get the type for all the Function definitions.
    for (auto &F : M) {
      // TODO: Currently, we do not have information about declarations that are
      // outside the module in the metadata. The metadata descriptions may be
      // extended in the future to handle these.
      if (F.isDeclaration())
        continue;

      if (auto *MDNode = F.getMetadata("dtrans_type")) {
        DTransType *DType = MDReader.decodeMDNode(MDNode);
        if (DType) {
          PTA.setDeclaredType(&F, TM.getOrCreatePointerType(DType));
          continue;
        }
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
      if (auto *MDNode = GV.getMetadata("dtrans_type")) {
        dtrans::DTransType *DT = MDReader.decodeMDNode(MDNode);
        if (DT) {
          PTA.setDeclaredType(&GV, TM.getOrCreatePointerType(DT));
          continue;
        }
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
      Info->setUnhandled();
      LLVM_DEBUG(dbgs() << "Unable to set declared type for global variable: "
                        << GV.getName() << "\n");
    }
  }

  void visitFunction(Function &F) {
    for (auto &A : F.args())
      if (isPossiblePtrValue(&A))
        analyzeValue(&A);
  }

  void visitAllocaInst(AllocaInst &I) { analyzeValue(&I); }
  void visitGetElementPtrInst(GetElementPtrInst &I) { analyzeValue(&I); }
  void visitIntToPtrInst(IntToPtrInst &I) { analyzeValue(&I); }
  void visitLoadInst(LoadInst &I) { analyzeValue(&I); }
  void visitPHINode(PHINode &I) { analyzeValue(&I); }
  void visitPtrToIntInst(PtrToIntInst &I) { analyzeValue(&I); }
  void visitSelectInst(SelectInst &I) { analyzeValue(&I); }

private:
  ValueTypeInfo *analyzeValue(Value *V) {
    ValueTypeInfo *Info = PTA.getOrCreateValueTypeInfo(V);
    if (Info->isCompletelyAnalyzed())
      return Info;

    DEBUG_WITH_TYPE(VERBOSE_TRACE, {
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
    DEBUG_WITH_TYPE(VERBOSE_TRACE, dumpDependencyStack(V, DependentVals));

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
      DEBUG_WITH_TYPE(VERBOSE_DEP_TRACE, {
        dbgs() << "  Dependent: ";
        printValue(dbgs(), Dep);
        dbgs() << "\n";
        DepInfo->print(dbgs(), /*Combined=*/false, "    ");
      });

      // If we have complete results for this value, don't repeat the analysis.
      if (DepInfo->isCompletelyAnalyzed()) {
        DEBUG_WITH_TYPE(VERBOSE_DEP_TRACE, {
          dbgs() << "  Already analyzed: ";
          printValue(dbgs(), Dep);
          dbgs() << "\n";
        });
        continue;
      }
      analyzeValueImpl(Dep, DepInfo);
    }

    Info->setCompletelyAnalyzed();

    DEBUG_WITH_TYPE(VERBOSE_TRACE, {
      dbgs() << "End analyzeValue for: ";
      printValue(dbgs(), V);
      dbgs() << "\n";
    });

    return Info;
  }

  void analyzeValueImpl(Value *V, ValueTypeInfo *Info) {
    DEBUG_WITH_TYPE(VERBOSE_TRACE, {
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
      Info->setUnhandled();
      LLVM_DEBUG({
        dbgs() << "analyzeValueImpl not implemented for:";
        printValue(dbgs(), V);
        dbgs() << "\n";
      });
      break;
    case Instruction::Alloca:
      analyzeAllocaInst(cast<AllocaInst>(I), Info);
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
      analyzePtrToIntInst(cast<PtrToIntInst>(I), Info);
      break;
    case Instruction::Select:
      analyzeSelectInst(cast<SelectInst>(I), Info);
      break;
      // TODO: Add other instructions analysis calls.
    }
  }

  void inferTypeFromUse(Value *V, ValueTypeInfo *ResultInfo) {
    // TODO: Implement look-ahead analysis for uses of 'V' to determine the set
    // of potential types.
  }

  // Build a stack of values that must be analyzed to compute the type of V.
  void populateDependencyStack(Value *V,
                               SmallVectorImpl<Value *> &DependentVals) {
    // This helper pushes a value on the back of the dependency stack
    // and returns true if this is the first occurrence of the value on the
    // stack or false if it was present before the call. The value is pushed
    // onto the stack in either case. A return value of 'true' indicates that
    // dependents of the Value also need to be added to the stack.
    auto AddDependency = [](Value *DV, SmallVectorImpl<Value *> &DepStack) {
      auto REnd = DepStack.rend();
      auto It = std::find(DepStack.rbegin(), REnd, DV);
      DepStack.push_back(DV);
      return (It == REnd);
    };

    if (auto *I = dyn_cast<Instruction>(V)) {
      switch (I->getOpcode()) {
      default:
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
    if (!isPossiblePtrValue(Arg))
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
    if (ArgNo > FnTy->getNumArgs()) {
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
    if (ArgTy == getDTransI8Ptr())
      inferTypeFromUse(Arg, ResultInfo);
  }

  void analyzeAllocaInst(AllocaInst *AI, ValueTypeInfo *ResultInfo) {
    // Types that involve pointers should have metadata annotations attached,
    // use them to set the declared type of the AllocaInst.
    if (auto *MDNode = AI->getMetadata("dtrans_type")) {
      dtrans::DTransType *DTy = MDReader.decodeMDNode(MDNode);
      if (DTy) {
        // The metadata describes the ValueType of the alloca, but the
        // Value object is a pointer to that type.
        PTA.setDeclaredType(AI, TM.getOrCreatePointerType(DTy));
        return;
      }
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
      dtrans::DTransType *IndexedTy = GetGEPIndexedType(Ty, GepOps);
      if (!IndexedTy) {
        ResultInfo->setUnhandled();
        LLVM_DEBUG(dbgs() << "unable to resolve index type: " << GEP << "\n");
        return false;
      }

      if (auto *IndexedStTy = dyn_cast<DTransStructType>(IndexedTy)) {
        // The final argument of the GEP of a structure field element must
        // always be a constant value
        auto *LastArg =
            cast<ConstantInt>(GEP.getOperand(GEP.getNumOperands() - 1));
        uint64_t FieldNum = LastArg->getLimitedValue();
        DTransType *FieldTy = IndexedStTy->getFieldType(FieldNum);
        if (!FieldTy) {
          ResultInfo->setUnhandled();
          LLVM_DEBUG(dbgs() << "Unhandled: unknown field type for GEP: " << GEP
                            << "\n");
          return false;
        }

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

      ResultInfo->setUnhandled();
      LLVM_DEBUG(dbgs() << "GEP not handled: " << GEP << "\n");
      return false;
    };

    Value *BasePointer = GEP.getPointerOperand();
    ValueTypeInfo *PointerInfo = PTA.getOrCreateValueTypeInfo(BasePointer);
    if (!PointerInfo->isCompletelyAnalyzed())
      ResultInfo->setPartiallyAnalyzed();

    if (PointerInfo->getUnhandled() || PointerInfo->getDependsOnUnhandled())
      ResultInfo->setDependsOnUnhandled();

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
      llvm::Type *SrcTy = GEP.getSourceElementType();
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

    // Zero or Single index operand GEPs of the form, result in the same
    // type as the base pointer.
    // For example, these cases result in the same type identified for %x:
    //   getelementptr p0, p0 %x, i64 5
    //   getelementptr <ty>, <ptr vector> %x, <vector index type> %idx

    // These are treating the source operand as an array, and therefore are
    // generating the same type as the pointer operand. However, there is a
    // special case when indexed type is an i8, since it may be a
    // byte-flattened GEP that needs to be analyzed.
    propagate(PointerInfo, ResultInfo, /*Decl=*/true, /*Use=*/true, 0);

    // TODO: Check if this is a byte-flattened GEP. We need to have bitcasts
    // analyzed before this can be implemented.
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
    for (unsigned i = 1; i < GEP.getNumIndices(); ++i)
      // The +1 here is because the first operand of a GEP is not an index.
      if (ConstantInt *CI = dyn_cast<ConstantInt>(GEP.getOperand(i + 1)))
        if (!CI->isZero())
          return;

    bool MayBeI8Ptr = false;
    for (auto *AliasTy :
         ResultInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Decl))
      if (AliasTy == getDTransI8Ptr()) {
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
    propagate(SrcInfo, ResultInfo, true, true, 0);

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
    Value *PtrOp = LI->getPointerOperand();
    ValueTypeInfo *PointerInfo = PTA.getOrCreateValueTypeInfo(PtrOp);
    propagate(PointerInfo, ResultInfo, true, true, -1);

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
                 bool Use, int DerefDelta) {
    assert(DerefDelta >= -1 && DerefDelta <= 1 &&
           "Expect at most one level deref difference");

    // Helper routine that will get/create a DTransType to represent with a
    // potential change to level of indirection by at most one level (up or
    // down).
    auto &TM = this->TM;
    auto GetOrCreateTypeWithDerefLevel = [&TM](DTransType *Ty,
                                               int DeltaLevel) -> DTransType * {
      if (DeltaLevel == 0)
        return Ty;

      // Create a pointer to the element
      if (DeltaLevel > 0)
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
            GetOrCreateTypeWithDerefLevel(Alias, DerefDelta);

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
      if (DerefDelta == 0)
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

  void analyzePtrToIntInst(PtrToIntInst *PTI, ValueTypeInfo *ResultInfo) {
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
    propagate(SrcInfo, ResultInfo, true, true, 0);

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
      propagate(SrcInfo, ResultInfo, true, true, 0);

      if (SrcInfo->getUnhandled() || SrcInfo->getDependsOnUnhandled())
        ResultInfo->setDependsOnUnhandled();

      if (!SrcInfo->isCompletelyAnalyzed())
        ResultInfo->setPartiallyAnalyzed();
    }
  }

  // Check whether the type for the Value object is something that needs to be
  // analyzed for potential pointer types.
  bool isPossiblePtrValue(Value *V) {
    llvm::Type *ValueTy = V->getType();

    // If the value is a pointer or the result of a pointer-to-int cast
    // it definitely is a pointer.
    if (ValueTy->isPointerTy() || isa<PtrToIntOperator>(V))
      return true;

    // A vector of pointers should be analyzed to track the pointer type.
    if (ValueTy->isVectorTy() && ValueTy->getVectorElementType()->isPointerTy())
      return true;

    // If the value is not a pointer and is not a pointer-sized integer, it
    // is definitely not a value we will track as a pointer.
    if (ValueTy != getPointerSizedIntType())
      return false;

    // If it is a pointer-sized integer, we may need to analyze it if
    // it is the result of a load, select or PHI node.
    if (isa<LoadInst>(V) || isa<SelectInst>(V) || isa<PHINode>(V))
      return true;

    // Otherwise, we don't need to analyze it as a pointer.
    return false;
  }

  llvm::Type *getPointerSizedIntType() const { return PointerSizedIntType; }
  dtrans::DTransPointerType *getDTransI8Ptr() const { return DTransI8PtrTy; }

  ////////////////////////////////////////////////////////////////////////////////
  // Start of member data
  ////////////////////////////////////////////////////////////////////////////////

  PtrTypeAnalyzerImpl &PTA;
  DTransTypeManager &TM;
  TypeMetadataReader &MDReader;
  const DataLayout &DL;
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;

  // LLVM type for an integer that is the same size as a pointer in address
  // space 0.
  llvm::Type *PointerSizedIntType;

  // Representation of i8* in DTransType system
  dtrans::DTransPointerType *DTransI8PtrTy;
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
    DEBUG_WITH_TYPE(VERBOSE_TRACE, {
      printValue(dbgs(), V);
      dbgs() << " - Added alias " << (Kind == VAT_Decl ? "[DECL]" : "[USE]")
             << ": " << *Ty << "\n";
    });

    // Check to see if this is a pointer (at any level of indirection) to
    // an aggregate type. That will make it faster later to tell if a value
    // is interesting or not.
    dtrans::DTransType *Base = Ty;
    while (Base->isPointerTy())
      Base = Base->getPointerElementType();
    while (Base->isVectorTy())
      Base = Base->getVectorElementType();

    if (Base->isAggregateType())
      AliasesToAggregatePointer = true;

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
    DEBUG_WITH_TYPE(VERBOSE_TRACE, {
      printValue(dbgs(), V);
      dbgs() << " - Added element:" << (Kind == VAT_Decl ? "[DECL]" : "[USE]")
             << ": " << *BaseTy << " @ ";
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
  DEBUG_WITH_TYPE(VERBOSE_TRACE, {
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
     << (getDependsOnUnhandled() ? " <DEPENDS ON UNHANDLED>" : "") << "\n";

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

ValueTypeInfo *
PtrTypeAnalyzerImpl::getOrCreateValueTypeInfo(const Instruction *I,
                                              unsigned OpNum) {
  ValueTypeInfo *Info = getValueTypeInfo(I, OpNum);
  if (Info)
    return Info;

  Value *V = I->getOperand(OpNum);
  if (!isCompilerConstant(V))
    return getOrCreateValueTypeInfo(V);

  Info = new ValueTypeInfo(nullptr);
  LocalMapForConstant[{I, OpNum}] = Info;
  return Info;
}

ValueTypeInfo *PtrTypeAnalyzerImpl::getValueTypeInfo(const Value *V) const {
  auto It = LocalMap.find(V);
  if (It == LocalMap.end())
    return nullptr;

  return It->second;
}

ValueTypeInfo *PtrTypeAnalyzerImpl::getValueTypeInfo(const Instruction *I,
                                                     unsigned OpNum) const {
  Value *V = I->getOperand(OpNum);
  if (isCompilerConstant(V)) {
    auto It = LocalMapForConstant.find({I, OpNum});
    if (It == LocalMapForConstant.end())
      return nullptr;

    return It->second;
  }

  return getValueTypeInfo(V);
}

void PtrTypeAnalyzerImpl::setDeclaredType(Value *V, dtrans::DTransType *Ty) {
  ValueTypeInfo *Info = getOrCreateValueTypeInfo(V);
  Info->addTypeAlias(ValueTypeInfo::VAT_Decl, Ty);
  Info->setCompletelyAnalyzed();
}

////////////////////////////////////////////////////////////////////////////////
//
// Implementation of PtrTypeAnalyzer class
//
////////////////////////////////////////////////////////////////////////////////

PtrTypeAnalyzer::PtrTypeAnalyzer(
    DTransTypeManager &TM, TypeMetadataReader &MDReader, const DataLayout &DL,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI) {
  Impl = std::make_unique<PtrTypeAnalyzerImpl>(TM, MDReader, DL, GetTLI);
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

void PtrTypeAnalyzer::run(Module &M) { Impl->run(M); }

ValueTypeInfo *PtrTypeAnalyzer::getValueTypeInfo(const Value *V) const {
  return Impl->getValueTypeInfo(V);
}

ValueTypeInfo *PtrTypeAnalyzer::getValueTypeInfo(const Instruction *I,
                                                 unsigned OpNum) const {
  return Impl->getValueTypeInfo(I, OpNum);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void PtrTypeAnalyzer::dumpPTA(Module &M) {
  PtrTypeAnalyzerAnnotationWriter Annotator(*this, PTAEmitCombinedSets);
  M.print(dbgs(), &Annotator);
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

} // end namespace dtrans
} // end namespace llvm

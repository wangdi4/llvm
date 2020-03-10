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
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FormattedStream.h"

#define DEBUG_TYPE "dtrans-pta"

// Trace messages for the pointer type analyzer when new information is
// associated with a Value object.
#define VERBOSE_TRACE "dtrans-pta-verbose"

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

    // Info objects may be created for some Values that are not pointers, so
    // check for the empty set before printing to keep the output clean.
    if (Info && !Info->empty()) {
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
      TypeMetadataReader &MDReader, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI)
      : PTA(PTA), TM(TM), MDReader(MDReader), DL(DL), GetTLI(GetTLI) {}

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

private:
  PtrTypeAnalyzerImpl &PTA;
  DTransTypeManager &TM;
  TypeMetadataReader &MDReader;
  const DataLayout &DL;
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;
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
void ValueTypeInfo::dump() { print(dbgs()); }

void ValueTypeInfo::print(raw_ostream &OS, bool Combined, const char *Prefix) {
  auto TypeToString = [Prefix](const dtrans::DTransType *Ty) {
    std::string OutputVal;
    raw_string_ostream OutputStream(OutputVal);

    OutputStream << Prefix << "    " << *Ty;
    OutputStream.flush();
    return OutputVal;
  };

  // Print type alias list
  auto PrintAliasSet = [&OS, Prefix,
                        &TypeToString](PointerTypeAliasSet &Aliases) {
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
  auto PrintElementPointeeSet =
      [&OS, Prefix, &ElementPointeeToString](ElementPointeeSet &Elements) {
        if (Elements.empty()) {
          OS << Prefix << "  No element pointees.\n";
          return;
        }

        OS << Prefix << "  Element pointees:\n";
        dtrans::printCollectionSorted(OS, Elements.begin(), Elements.end(),
                                      "\n", ElementPointeeToString);
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
  PtrTypeAnalyzerInstVisitor InstAnalyzer(*this, TM, MDReader, DL, GetTLI);
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

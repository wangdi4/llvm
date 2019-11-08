//===---------------- DynClone.cpp - DTransDynClonePass -------------------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Dynamic Cloning optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DynClone.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOptBase.h"
#include "Intel_DTrans/Transforms/DTransOptUtils.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <string>
#include <vector>

using namespace llvm;
using namespace PatternMatch;

#define DEBUG_TYPE "dtrans-dynclone"
#define REENCODING "dtrans-dynclone-reencoding"

namespace {

// Shrunken int type width
static cl::opt<int>
    DTransDynCloneShrTyWidth("dtrans-dynclone-shrunken-type-width",
                             cl::init(16), cl::ReallyHidden);
// (Reencoding)
// A delta that allows encoding big constants into shrunken int type. The main
// idea is that candidate field values generally fit into even smaller int type
// besides a number of big constants known in compile time.
static cl::opt<int>
    DTransDynCloneShrTyDelta("dtrans-dynclone-shrunken-type-width-delta",
                             cl::init(1), cl::ReallyHidden);

// Max value for int type with the width equal (DTransDynCloneShrTyWidth -
// DTransDynCloneShrTyDelta). Calculated as max for intX = 2^(X-1)-1.
int64_t getMaxShrIntTyValueWithDelta() {
  if (DTransDynCloneShrTyWidth - DTransDynCloneShrTyDelta > 0)
    return std::pow(2,
                    DTransDynCloneShrTyWidth - DTransDynCloneShrTyDelta - 1) -
           1;
  llvm_unreachable("Unexpected type width for shrinking");
}

// Min value for int type with the width equal (DTransDynCloneShrTyWidth -
// DTransDynCloneShrTyDelta). Calculated as min for intX = 0 - 2^(X-1).
int64_t getMinShrIntTyValueWithDelta() {
  if (DTransDynCloneShrTyWidth - DTransDynCloneShrTyDelta > 0)
    return (0 - std::pow(2, DTransDynCloneShrTyWidth -
                                DTransDynCloneShrTyDelta - 1));
  llvm_unreachable("Unexpected type width for shrinking");
}

class DTransDynCloneWrapper : public ModulePass {
private:
  dtrans::DynClonePass Impl;

public:
  static char ID;

  DTransDynCloneWrapper() : ModulePass(ID) {
    initializeDTransDynCloneWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    DTransAnalysisWrapper &DTAnalysisWrapper =
        getAnalysis<DTransAnalysisWrapper>();
    DTransAnalysisInfo &DTInfo = DTAnalysisWrapper.getDTransInfo(M);

    dtrans::LoopInfoFuncType GetLI = [this](Function &F) -> LoopInfo & {
      return this->getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    };
    auto GetTLI = [this](Function &F) -> const TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    bool Changed = Impl.runImpl(
        M, DTInfo, GetTLI, getAnalysis<WholeProgramWrapperPass>().getResult(),
        GetLI);
    if (Changed)
      DTAnalysisWrapper.setInvalidated();
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<DTransAnalysisWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransDynCloneWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransDynCloneWrapper, "dtrans-dynclone",
                      "DTrans dynamic cloning", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransDynCloneWrapper, "dtrans-dynclone",
                    "DTrans dynamic cloning", false, false)

ModulePass *llvm::createDTransDynCloneWrapperPass() {
  return new DTransDynCloneWrapper();
}

namespace llvm {

namespace dtrans {

// This class is used to reorder fields after shrinking fields to
// eliminate gaps due to shrinking.
class FieldData {
public:
  uint64_t Align = 0;
  uint64_t Size = 0;
  unsigned Index = 0;

  FieldData(uint64_t Align, uint64_t Size, unsigned Index)
      : Align(Align), Size(Size), Index(Index) {}

  // Copy constructor:
  FieldData(const FieldData &FD)
      : Align(FD.Align), Size(FD.Size), Index(FD.Index) {}

  // Sort fields based on below compare function to avoid gaps between fields.
  //    Ascending order based on Alignment, then
  //    Ascending order based on Size, then
  //    Descending order based on Index.
  bool operator<(const FieldData &RHS) const {
    return std::make_tuple(RHS.Align, RHS.Size, -(RHS.Index + 1)) <
           std::make_tuple(Align, Size, -(Index + 1));
  }
};

class DynCloneImpl {

  // Functor that compares Function* using alphabetical ordering of the
  // function's name.
  struct CompareFuncPtr
      : public std::binary_function<Function *, Function *, bool> {
    bool operator()(const Function *lhs, const Function *rhs) const {
      if (lhs == nullptr || rhs == nullptr)
        return lhs < rhs;
      return lhs->getName().compare(rhs->getName()) == -1;
    }
  };

  using DynField = std::pair<llvm::Type *, size_t>;
  using DynFieldList = SmallVector<DynField, 16>;
  // Even though it will be very small set,  using std::set (instead of
  // SmallSet/SetVector) to do set operations like union/intersect.
  using DynFieldSet = std::set<DynField>;
  using FunctionSet = std::set<Function *, CompareFuncPtr>;
  using CallInstSet = SmallPtrSet<CallInst *, 8>;
  using StoreInstSet = SmallPtrSet<StoreInst *, 4>;
  using PHIInstSet = SmallPtrSet<PHINode *, 4>;
  using LoadInstSet = SmallPtrSet<LoadInst *, 8>;
  using InstSet = SmallPtrSet<Instruction *, 32>;

public:
  DynCloneImpl(Module &M, const DataLayout &DL, DTransAnalysisInfo &DTInfo,
               LoopInfoFuncType &GetLI, DynGetTLITy GetTLI)
      : M(M), DL(DL), DTInfo(DTInfo), GetLI(GetLI), GetTLI(GetTLI),
        ShrunkenIntTy(
            Type::getIntNTy(M.getContext(), DTransDynCloneShrTyWidth)),
        ShrunkenIntTyWithDelta(
            Type::getIntNTy(M.getContext(), DTransDynCloneShrTyWidth -
                                                DTransDynCloneShrTyDelta)),
        InitRoutine(nullptr), ShrinkHappenedVar(nullptr),
        DynFieldEncodeFunc(nullptr), DynFieldDecodeFunc(nullptr){};

  bool run(void);

private:
  Module &M;
  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  LoopInfoFuncType &GetLI;
  DynGetTLITy GetTLI;

  // Holds result Type after shrinking 64-bit to 16-bit integer values.
  llvm::Type *ShrunkenIntTy;
  llvm::Type *ShrunkenIntTyWithDelta;

  // List of candidate fields which are represented as <Type of struct,
  // field_index>.
  DynFieldList CandidateFields;

  // This is used to maintain mapping between candidate field and set of
  // other fields that are assigned to the candidate field.
  DenseMap<DynField, DynFieldSet> DependentFieldSet;

  // Mapping between candidate field and the function where unknown values
  // are assigned to the candidate field.
  DenseMap<DynField, Function *> DynFieldFuncMap;

  // Function that has unknown assignments to all qualified candidate fields.
  Function *InitRoutine;

  // It is used to check at runtime if shrinking of structs occurred.
  GlobalVariable *ShrinkHappenedVar;

  // List of cloned routines.
  FunctionSet ClonedFunctionList;

  // List of allocation calls for candidate structs in InitRoutine.
  SmallVector<std::pair<AllocCallInfo *, Type *>, 4> AllocCalls;

  // Map of original and cloned routines.
  DenseMap<Function *, Function *> CloningMap;

  // Map of <Original struct, Vector of new indexes to indicate new
  // locations in shrunken struct>
  DenseMap<StructType *, std::vector<uint32_t>> TransformedIndexes;

  // Map of <Original Struct, Shrunken Struct>
  DenseMap<StructType *, StructType *> TransformedTypeMap;

  // For each candidate type, it collects all routines where the type is
  // accessed. This set is used to decide which routines need to be
  // cloned during transformation.
  DenseMap<llvm::Type *, FunctionSet> TypeAccessedInFunctions;

  // List of CallSites for AddressTaken routines.
  DenseMap<Function *, CallInstSet> FunctionCallInsts;

  // This is used to maintain stored locations where allocation pointers
  // are saved. For each AllocCall, it provides set of simple store
  // instructions which save alloc pointers to global variable.
  DenseMap<AllocCallInfo *, StoreInstSet> AllocSimplePtrStores;

  // List of modified allocation calls
  CallInstSet ModifiedAllocs;

  // Return values of allocation calls of candidate structs are saved in
  // local variables to reuse them later when copying data from old layout
  // to new layout. Mapping between Alloc Instruction and load instruction of
  // the corresponding local variable.
  DenseMap<CallInst *, LoadInst *> AllocCallRets;

  // Size of allocation calls of candidate structs are saved in local
  // variables to reuse them later when copying data from old layout
  // to new layout. Mapping between Alloc Instruction and load
  // instruction of the corresponding local variable.
  DenseMap<CallInst *, LoadInst *> AllocCallSizes;

  // Set of all AOS-2-SOA calls that are marked with
  // "__intel_dtrans_aostosoa_alloc" pointer annotation.
  CallInstSet AOSSOAACalls;

  // Pointers of shrunken struct may be stored in some fields of AOS-2-SOA
  // struct. This represents set of fields of AOS-2-SOA struct where pointers of
  // shrunken struct are stored. This data structure needs to be changed if we
  // allow more than one AOS-2-SOA allocation call.
  DynFieldSet SOAGlobalVarFieldSet;

  // Shrunken struct may have more than one allocation call. This is mapping
  // between fields of AOS-2-SOA struct and alloc call of shrunken struct whose
  // return pointers are only saved in the fields.
  DenseMap<DynField, AllocCallInfo *> SOAFieldAllocCallMap;

  // Mapping between AOSTOSOA allocation call and the corresponding AOSTOSOA
  // global variable. For now, at most one AOSTOSOA allocation call is
  // allowed.
  SmallDenseMap<CallInst *, GlobalVariable *> AOSSOACallGVMap;

  // Set of struct fields that are marked with aostosoa index in IR.
  // These will be collected while walking IR during analysis.
  // Fields, which are marked with AOSTOSOA index, in candidate
  // struct will be shrunken to 2 bytes. These fields will be
  // treated differently from CandidateFields because there is no
  // need to have compile-time/runtime checks on load/stores of these
  // fields in the IR (except a runtime check on size of aostosoa
  // allocation call).
  DynFieldSet AOSTOSOAIndexFields;

  // Set of Load/Store instructions that access more than one
  // struct elements.
  InstSet MultiElemLdStSet;

  // Mapping between MultiElem Ld/St instruction and one of the fields that
  // are accessed by the instruction. This map is used during transformation.
  SmallDenseMap<Instruction *, DynField> MultiElemLdStAOSTOSOAIndexMap;

  SmallPtrSet<CallInst *, 8> RuntimeCheckUnsafeCalls;

  // (Reencoding)
  // Constant values for candidate fields.
  DenseMap<DynField, std::set<int64_t>> DynFieldConstValueMap;
  // Complete set of constants to use in encode/decode functions.
  std::set<int64_t> AllDynFieldConstSet;

  // Encode function for candidate fields.
  Function *DynFieldEncodeFunc;

  // Decode function for candidate fields.
  Function *DynFieldDecodeFunc;

  // Stores that were traced back to compile-time known constants in
  // InitRoutine.
  DenseMap<DynField, std::set<Value *>> DynFieldTracedToConstantValues;

  bool gatherPossibleCandidateFields(void);
  bool prunePossibleCandidateFields(void);
  bool verifyMultiFieldLoadStores(void);
  bool verifyLegalityChecksForInitRoutine(void);
  void createShrunkenTypes(void);
  bool trackPointersOfAllocCalls(void);
  bool verifyCallsInInitRoutine(void);
  void transformInitRoutine(void);
  bool createCallGraphClone(void);
  void transformIR(void);
  bool isAOSTOSOAIndexField(DynField &DField) const;
  bool isShrunkenField(DynField &DField) const;
  bool isCandidateField(DynField &DField) const;
  bool isCandidateStruct(Type *Ty);
  Type *getGEPStructType(GetElementPtrInst *GEP) const;
  DynField getAccessStructField(GEPOperator *GEP) const;
  Type *getCallInfoElemTy(CallInfo *CInfo) const;
  Type *getTypeRelatedToInstruction(Instruction *I) const;
  void createEncodeDecodeFunctions(void);               // (Reencoding)
  void fillupCoderRoutine(Function *F, bool IsEncoder); // (Reencoding)
  void printCandidateFields(raw_ostream &OS) const;
  void printDynField(raw_ostream &OS, const DynField &DField) const;
};

// Collects possible candidate fields for Dynamic cloning.
bool DynCloneImpl::gatherPossibleCandidateFields(void) {

  // Allow only Int64 type fields as candidates for now.
  auto IsCandidateType = [&](Type *Ty) { return Ty->isIntegerTy(64); };

  LLVM_DEBUG(dbgs() << "  Looking for candidate structures.\n");

  for (TypeInfo *TI : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<StructInfo>(TI);
    if (!StInfo)
      continue;

    if (DTInfo.testSafetyData(TI, dtrans::DT_DynClone)) {
      LLVM_DEBUG(dbgs() << "    Rejecting "
                        << getStructName(StInfo->getLLVMType())
                        << " based on safety data.\n");
      continue;
    }
    // Heuristic: Consider only struct that has highest total field
    // frequency for DynClone to avoid performance issues with runtime
    // checks.
    if (DTInfo.getMaxTotalFrequency() != StInfo->getTotalFrequency()) {
      LLVM_DEBUG(dbgs() << "    Rejecting "
                        << getStructName(StInfo->getLLVMType())
                        << " based on heuristic.\n");
      continue;
    }
    StructType *StTy = cast<StructType>(StInfo->getLLVMType());
    for (unsigned I = 0; I < StTy->getNumElements(); ++I) {
      Type *Ty = StTy->getElementType(I);
      if (!IsCandidateType(Ty))
        continue;
      CandidateFields.push_back(std::make_pair(StTy, I));
    }
  }

  LLVM_DEBUG(if (CandidateFields.empty()) dbgs()
             << "    No possible candidates found.\n");

  return !CandidateFields.empty();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DynCloneImpl::printDynField(raw_ostream &OS,
                                 const DynField &DField) const {
  OS << "struct: " << getStructName(DField.first) << " Index: " << DField.second
     << "\n";
}

// Print candidate fields
void DynCloneImpl::printCandidateFields(raw_ostream &OS) const {
  for (auto &CandidatePair : CandidateFields) {
    OS << "    ";
    printDynField(OS, CandidatePair);
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Returns true if \p DField is marked with AOSTOSOA index.
bool DynCloneImpl::isAOSTOSOAIndexField(DynField &DField) const {
  for (auto &CandidatePair : AOSTOSOAIndexFields)
    if (CandidatePair == DField)
      return true;
  return false;
}

// Returns true if \p DField is in CandidateFields.
bool DynCloneImpl::isCandidateField(DynField &DField) const {
  for (auto &CandidatePair : CandidateFields)
    if (CandidatePair == DField)
      return true;
  return false;
}

// Return true if \p DField is either regular candidate
// field or AOSTOSOA index field.
bool DynCloneImpl::isShrunkenField(DynField &DField) const {
  return (isCandidateField(DField) || isAOSTOSOAIndexField(DField));
}

// Returns true if \p Ty is a struct that has candidate fields for
// shrinking.
bool DynCloneImpl::isCandidateStruct(Type *Ty) {
  if (!Ty->isStructTy())
    return false;
  for (auto &CPair : CandidateFields)
    if (CPair.first == Ty)
      return true;
  return false;
}

// Returns Type of struct accessed in GEP if the GEP is in allowed format
// to enable DynClone for the struct. Otherwise, returns nullptr.
Type *DynCloneImpl::getGEPStructType(GetElementPtrInst *GEP) const {
  int32_t NumIndices = GEP->getNumIndices();
  if (NumIndices > 2)
    return nullptr;
  if (NumIndices == 1) {
    auto FPair = DTInfo.getByteFlattenedGEPElement(GEP);
    if (FPair.first)
      return FPair.first;
  }
  auto ElemTy = GEP->getSourceElementType();
  if (!isa<StructType>(ElemTy))
    return nullptr;
  return ElemTy;
}

// If \p GEP is accessing struct field, it returns the field.
// It doesn't handle if NumIndices > 2.
DynCloneImpl::DynField
DynCloneImpl::getAccessStructField(GEPOperator *GEP) const {
  int32_t NumIndices = GEP->getNumIndices();
  if (NumIndices > 2)
    return std::make_pair(nullptr, 0);
  if (NumIndices == 1)
    return DTInfo.getByteFlattenedGEPElement(GEP);
  auto ElemTy = GEP->getSourceElementType();
  if (!isa<StructType>(ElemTy))
    return std::make_pair(nullptr, 0);
  int32_t FieldIndex = cast<ConstantInt>(GEP->getOperand(2))->getLimitedValue();
  return std::make_pair(ElemTy, FieldIndex);
}

// Returns struct type associated with \p CInfo of either Alloc/Memfunc.
Type *DynCloneImpl::getCallInfoElemTy(CallInfo *CInfo) const {
  if (!CInfo)
    return nullptr;
  if (CInfo->getCallInfoKind() != CallInfo::CIK_Alloc &&
      CInfo->getCallInfoKind() != CallInfo::CIK_Memfunc)
    return nullptr;
  auto &CallTypes = CInfo->getPointerTypeInfoRef().getTypes();
  if (CallTypes.size() != 1)
    return nullptr;
  Type *PtrTy = *CallTypes.begin();
  if (!PtrTy->isPointerTy())
    return nullptr;
  Type *ElemTy = PtrTy->getPointerElementType();
  if (!isa<StructType>(ElemTy))
    return nullptr;
  return ElemTy;
}

// Returns Type related to \p I. This is used to get candidate
// struct type if the type is referenced by \p I.
Type *DynCloneImpl::getTypeRelatedToInstruction(Instruction *I) const {
  Type *StTy = nullptr;
  // Treat a struct as accessed if the struct is referenced by
  // any of these instructions.
  // Not checking for MultiElemLoadStore instructions here since
  // those will be covered by GEPs.
  if (auto *GEP = dyn_cast<GetElementPtrInst>(I)) {
    StTy = getGEPStructType(GEP);
  } else if (auto *BinOp = dyn_cast<BinaryOperator>(I)) {
    if (BinOp->getOpcode() == Instruction::Sub)
      StTy = DTInfo.getResolvedPtrSubType(BinOp);
  } else if (auto *LI = dyn_cast<LoadInst>(I)) {
    auto LdElem = DTInfo.getLoadElement(LI);
    StTy = LdElem.first;
  } else if (auto *SI = dyn_cast<StoreInst>(I)) {
    auto StElem = DTInfo.getStoreElement(SI);
    StTy = StElem.first;
  } else if (isa<CallInst>(I)) {
    auto *CInfo = DTInfo.getCallInfo(I);
    StTy = getCallInfoElemTy(CInfo);
  }
  return StTy;
}

// This routine tries to reduce possible candidate fields by analyzing IR
// and also finds InitRoutine, where unknown values are assigned to the
// all candidate fields. A candidate field is eliminated if we can't prove
// that value of the field always fits in 16-bit besides a number of
// compile-time known constants. Analysis does look at memset and store
// instructions.
//
// memset: For now, only zero value is considered as safe. For all other
// values, fields in structure are not qualified for DynClone transformation.
//
// Store Inst:
//   If Store operand is
//     1. Constant Value: If it doesn't fit in shrunken type with delta, it will
//     be added to the set of constants for encoding.
//
//     2. Load Value:
//           a. Same Field: No issues.
//           b. Some other Candidate field: Will add to dependence set and
//              verify it later.
//           c. Non-Candidate field: Not qualified.
//
//     TODO: Support for Args will be added later.
//
//     3. Other (Unknown value): First try to trace it back to compile-time
//     known constant value set. If not - map between field and the function
//     where it is assigned is recorded. All unknown assignments should be in
//     the same routine (i.e InitRoutine).
//
//  Dependence Set Analysis: Disable DynClone transformation conservatively
//  if dependent field of any candidate field is not qualified.
//
//  Init routine detection: All qualified candidates should have unknown
//  assignments in the same routine.
//
bool DynCloneImpl::prunePossibleCandidateFields(void) {

  DynFieldSet InvalidFields;

  // Analyze constant assignments:  str->field = 200;
  // (Reencoding)
  // Collect large constants which doesn't fit into shrunken type with delta.
  auto CheckConstInt = [&](Value *V, DynField &StElem) {
    assert(isa<ConstantInt>(V) && "Expected ConstantInt");
    auto *CInt = cast<ConstantInt>(V);
    int64_t CValue = CInt->getValue().getLimitedValue();
    // If constant does not fit into shrunken type with delta, then add the
    // constant to the encoding constants set.
    if (!ConstantInt::isValueValidForType(ShrunkenIntTyWithDelta, CValue)) {
      DynFieldTracedToConstantValues[StElem].insert(V);
      DynFieldConstValueMap[StElem].insert(CValue);
      LLVM_DEBUG(dbgs() << "    Large constant added to const field value map: "
                        << CValue << " : ";
                 printDynField(dbgs(), StElem));
      DEBUG_WITH_TYPE(
          REENCODING,
          dbgs() << "        (Reencoding) Constant collected for encoding.\n");
    }
  };

  // (Reencoding) Returns 'true' if the value could be traced back to the set of
  // constants. In case of success those constants are stored in
  // DynFieldConstValueMap, the root values are stored into
  // DynFieldTracedToConstantValues map.
  // Currently the following pattern is supported:
  //     V = X + const1     |    V = const1 + X
  //     X = Y * const2     |    X = const2 * Y   |   X = Y << const2
  //     Y = max (Z, const3)
  //     Z = S.a   where S.a has a complete set of constant values calculated by
  //     FSV analysis.
  auto TraceConstantValue = [&](Value *V, DynField &StElem) {
    Value *AddOp = nullptr, *MulOp = nullptr;
    Value *MaxLHS = nullptr, *MaxRHS = nullptr;
    ConstantInt *AddC = nullptr, *MulC = nullptr, *ShlC = nullptr;
    if (!match(V, m_Add(m_Value(AddOp), m_ConstantInt(AddC))) &&
        !match(V, m_Add(m_ConstantInt(AddC), m_Value(AddOp)))) {
      if (llvm::IntegerType *VIntType = dyn_cast<IntegerType>(V->getType())) {
        AddC = ConstantInt::getSigned(VIntType, 0);
        AddOp = V;
      } else
        return false;
    }

    if (!match(AddOp, m_Mul(m_Value(MulOp), m_ConstantInt(MulC))) &&
        !match(AddOp, m_Mul(m_ConstantInt(MulC), m_Value(MulOp))) &&
        !match(AddOp, m_Shl(m_Value(MulOp), m_ConstantInt(ShlC)))) {
      MulOp = AddOp;
    }

    if (SelectInst *SI = dyn_cast<SelectInst>(MulOp)) {
      SelectPatternResult SPR = matchSelectPattern(SI, MaxLHS, MaxRHS);
      if (SPR.Flavor != SPF_SMAX)
        return false;
      MulOp = MaxLHS;
    }

    ConstantInt *MaxC = dyn_cast_or_null<ConstantInt>(MaxRHS);
    LoadInst *Load = dyn_cast<LoadInst>(MulOp);
    if (!Load || (MaxRHS && !MaxC))
      return false;

    std::pair<dtrans::StructInfo *, uint64_t> Res =
        DTInfo.getInfoFromLoad(Load);

    if (!Res.first)
      return false;

    dtrans::FieldInfo &FI = Res.first->getField(Res.second);
    if (!FI.isValueSetComplete())
      return false;

    for (auto *C : FI.values()) {
      ConstantInt *CurrC = cast<ConstantInt>(C);
      int64_t Const = CurrC->getSExtValue();
      if (MaxC)
        Const = std::max(Const, MaxC->getSExtValue());
      if (MulC)
        Const *= MulC->getSExtValue();
      if (ShlC)
        Const <<= ShlC->getSExtValue();
      if (AddC)
        Const += AddC->getSExtValue();
      if (!ConstantInt::isValueValidForType(ShrunkenIntTyWithDelta, Const)) {
        DynFieldConstValueMap[StElem].insert(Const);
        DynFieldTracedToConstantValues[StElem].insert(V);
        DEBUG_WITH_TYPE(REENCODING, dbgs() << "        (Reencoding) "
                                              "Constant collected for "
                                              "encoding: "
                                           << Const << " : ";
                        printDynField(dbgs(), StElem));
      }
    }
    return true;
  };

  // Analyze assignment of Load value:  str1->fielda = str2->fieldb
  // 1. fielda and fieldb are same: No Issue.
  // 2. fieldb is candidate field: Add fieldb as dependent for fielda
  // 3. fieldb is non-candidate: Add fielda to invalid fields.
  auto CheckLoadValue = [&](Value *V, DynField &StElem, Function *F) {
    assert(isa<LoadInst>(V) && "Expected LoadInst");
    LoadInst *LI = cast<LoadInst>(V);

    auto LdElem = DTInfo.getLoadElement(LI);
    if (StElem == LdElem)
      return;
    if (isCandidateField(LdElem)) {
      DependentFieldSet[StElem].insert(LdElem);
      LLVM_DEBUG(dbgs() << "    Adding to dependence-set:\n";
                 dbgs() << "        "; printDynField(dbgs(), StElem);
                 dbgs() << "        "; printDynField(dbgs(), LdElem));
      return;
    }
    auto It = DynFieldFuncMap.find(StElem);
    if (It == DynFieldFuncMap.end()) {
      DynFieldFuncMap.insert({StElem, F});
      LLVM_DEBUG(dbgs() << "    InitRoutine identified for ";
                 printDynField(dbgs(), StElem));
    } else if (It->second != F) {
      InvalidFields.insert(StElem);
      LLVM_DEBUG(dbgs() << "    Invalid...More than one init routine:";
                 printDynField(dbgs(), StElem));
    }
  };

  // Analyze memset here: memset(ptr2str, 0, size)
  //   Any call/ memset with zero value: No issues
  //
  //   memset with non-zero value: If there are any candidate fields in
  //   the struct, make them as invalid fields.
  auto WrittenUnknownValue = [&](Instruction *I) {
    auto *CInfo = DTInfo.getCallInfo(I);
    if (!CInfo || CInfo->getCallInfoKind() != dtrans::CallInfo::CIK_Memfunc)
      return;
    if (cast<MemfuncCallInfo>(CInfo)->getMemfuncCallInfoKind() !=
        MemfuncCallInfo::MK_Memset)
      return;
    MemSetInst &Inst = cast<MemSetInst>(*I);
    if (isValueEqualToSize(Inst.getValue(), 0))
      return;

    auto &CallTypes = CInfo->getPointerTypeInfoRef().getTypes();
    if (CallTypes.size() != 1)
      return;
    Type *PtrTy = *CallTypes.begin();
    if (!PtrTy->isPointerTy())
      return;
    Type *ElemTy = PtrTy->getPointerElementType();
    for (auto &CandidatePair : CandidateFields) {
      if (CandidatePair.first == ElemTy) {
        InvalidFields.insert(CandidatePair);
        LLVM_DEBUG(dbgs() << "    Invalid...written by memset: ";
                   printDynField(dbgs(), CandidatePair));
      }
    }
  };

  // If \p I is accessing a candidate struct, add \p F to the list
  // of routines where the candidate struct is accessed.
  auto UpdateTypeAccessInfo = [&](Instruction *I, Function *F) {
    Type *StTy = getTypeRelatedToInstruction(I);
    if (!StTy)
      return;
    if (isCandidateStruct(StTy))
      TypeAccessedInFunctions[StTy].insert(F);
  };

  // Returns false if \p I is not supported instruction.
  auto IsInstructionSupported = [&](Instruction *I) {
    // TODO: Add more unsupported instructions
    if (isa<InvokeInst>(I))
      return false;
    return true;
  };

  // Returns true if \p CalledValue is a BitCast to convert a function
  // to the FunctionType of \p CI.
  auto IsSimpleFPtrBitCast = [&](Value *CalledValue, CallInst *CI) {
    Type *CalledValueType = CalledValue->getType();
    if (!isa<PointerType>(CalledValueType))
      return false;
    PointerType *PTy = cast<PointerType>(CalledValueType);
    if (PTy->getElementType() != CI->getFunctionType())
      return false;
    if (auto *BC = dyn_cast<BitCastOperator>(CalledValue))
      if (isa<Function>(BC->getOperand(0)))
        return true;
    return false;
  };

  for (Function &F : M) {
    if (F.isDeclaration() || F.isIntrinsic())
      continue;

    LLVM_DEBUG(dbgs() << "  Pruning in routine: " << F.getName() << "\n");

    for (inst_iterator II = inst_begin(F), E = inst_end(F); II != E; ++II) {
      Instruction &Inst = *II;
      if (!IsInstructionSupported(&Inst)) {
        LLVM_DEBUG(dbgs() << "    Unsupported Inst ... Skip DynClone\n");
        return false;
      }
      // Collect AOSTOSOA alloc calls using DTransAnnotator.
      DTransAnnotator::DPA_AnnotKind DPAType;
      DPAType = DTransAnnotator::getDTransPtrAnnotationKind(Inst);
      if (DPAType == DTransAnnotator::DPA_AOSToSOAAllocation) {
        AOSSOAACalls.insert(
            cast<CallInst>(cast<IntrinsicInst>(Inst).getArgOperand(0)));
        continue;
      }
      // Collect AOSTOSOA index fields of candidate structs.
      if (DPAType == DTransAnnotator::DPA_AOSToSOAIndex) {
        auto GEP =
            dyn_cast<GEPOperator>(cast<IntrinsicInst>(Inst).getArgOperand(0));
        if (!GEP)
          continue;
        DynField FPair = getAccessStructField(GEP);
        if (FPair.first && isCandidateStruct(FPair.first))
          AOSTOSOAIndexFields.insert(FPair);
        continue;
      }
      if (DTInfo.isMultiElemLoadStore(&Inst)) {
        MultiElemLdStSet.insert(&Inst);
        continue;
      }

      if (CallInst *CI = dyn_cast<CallInst>(&Inst)) {
        Value *CalledValue = CI->getCalledValue();
        // Calls using aliases are treated as indirect calls.
        if (!isa<Function>(CalledValue) &&
            !IsSimpleFPtrBitCast(CalledValue, CI)) {
          LLVM_DEBUG(dbgs()
                     << "    Indirect Call ... Skip DynClone :" << *CI << "\n");
          return false;
        }
        Function *Callee = cast<Function>(CalledValue->stripPointerCasts());
        // Collecting all uses of routines that are marked with
        // AddressTaken. Use info of AddressTaken functions are not
        // properly updated after IPSCCP. So, we can't get all CallSites
        // of AddressTaken function by walking over uses of the function.
        // FunctionCallInsts will be used later during transformation.
        if (Callee->hasAddressTaken())
          FunctionCallInsts[Callee].insert(cast<CallInst>(&Inst));
      }

      UpdateTypeAccessInfo(&Inst, &F);

      if (auto *StInst = dyn_cast<StoreInst>(&Inst)) {
        auto StElem = DTInfo.getStoreElement(StInst);
        if (!StElem.first)
          continue;
        if (!isCandidateField(StElem))
          continue;

        Value *V = StInst->getValueOperand();
        if (isa<ConstantInt>(V)) {
          CheckConstInt(V, StElem);
        } else if (isa<LoadInst>(V)) {
          CheckLoadValue(V, StElem, &F);
        } else {
          if (TraceConstantValue(V, StElem)) {
            LLVM_DEBUG(
                dbgs()
                    << "    Value traced back to constants in init routine: ";
                printDynField(dbgs(), StElem));
          } else {
            auto It = DynFieldFuncMap.find(StElem);
            if (It == DynFieldFuncMap.end()) {
              DynFieldFuncMap.insert({StElem, &F});
              LLVM_DEBUG(dbgs()
                             << "    unknown value assigned in init routine:";
                         printDynField(dbgs(), StElem));
            } else if (It->second != &F) {
              InvalidFields.insert(StElem);
              LLVM_DEBUG(dbgs() << "    Invalid...More than one init routine: ";
                         printDynField(dbgs(), StElem));
            }
          }
        }
      } else if (isa<CallInst>(Inst)) {
        WrittenUnknownValue(&Inst);
      }
    }
  }

  // Eliminate invalid candidate fields from CandidateFields.
  DynFieldList ValidCandidates;
  for (auto &CandidatePair : CandidateFields)
    if (!InvalidFields.count(CandidatePair))
      ValidCandidates.push_back(CandidatePair);

  std::swap(CandidateFields, ValidCandidates);
  if (CandidateFields.empty()) {
    LLVM_DEBUG(dbgs() << "    No Candidate is qualified ... Skip DynClone\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << "  Candidate fields after Pruning: \n";
             printCandidateFields(dbgs()));

  // Print collected large constant values for candidate fields.
  DEBUG_WITH_TYPE(REENCODING, {
    for (auto CPair : CandidateFields) {
      if (DynFieldConstValueMap[CPair].empty()) {
        dbgs() << "Candidate Field has no large const values.\n";
        printDynField(dbgs(), CPair);
      } else {
        dbgs() << "Candidate Field large const values ";
        printDynField(dbgs(), CPair);
        for (auto C : DynFieldConstValueMap[CPair])
          dbgs() << "\t" << C;
        dbgs() << "\n";
      }
    }
  });

  // Check Dependency sets here. If a field in dependence set of any
  // candidate field is not qualified, remove all fields from
  // CandidateFields conservatively. union/intersect operations are used
  // to do this.
  // (Reencoding) currently only one dependancy set for candidates fields is
  // supported.
  LLVM_DEBUG(dbgs() << "\n  Processing Dependency Sets: \n");
  LLVM_DEBUG(dbgs() << "    Dependency Sets for Candidate fields: \n");
  DynFieldSet FieldEquivSet;
  DynFieldSet AllDepFieldsSet;
  bool FirstField = true;
  for (auto &CPair : CandidateFields) {
    auto It = DependentFieldSet.find(CPair);
    if (It == DependentFieldSet.end()) {
      if (FirstField) {
        FieldEquivSet.insert(CPair);
        FirstField = false;
      }
      LLVM_DEBUG(dbgs() << "      Candidate Field doesn't have Dep Set:";
                 printDynField(dbgs(), CPair));
      continue;
    }
    set_union(AllDepFieldsSet, It->second);
    LLVM_DEBUG(dbgs() << "      Candidate Field:";
               printDynField(dbgs(), It->first); for (auto &SetField
                                                      : It->second) {
                 dbgs() << "        ";
                 printDynField(dbgs(), SetField);
               } dbgs() << "\n";);

    // (Reencoding) We start from adding a first field and its dependents to the
    // field equivalence set. Later if current field or its dependents are in
    // the equivalence set - add current node and its dependents to the
    // equivalence set.
    if (FirstField) {
      FieldEquivSet.insert(CPair);
      set_union(FieldEquivSet, It->second);
      FirstField = false;
    } else {
      bool HasCommonMembers = FieldEquivSet.count(CPair);
      for (auto DF : It->second)
        if (FieldEquivSet.count(DF)) {
          HasCommonMembers = true;
          break;
        }
      if (HasCommonMembers) {
        FieldEquivSet.insert(CPair);
        set_union(FieldEquivSet, It->second);
      }
    }
  }
  set_intersect(AllDepFieldsSet, InvalidFields);

  if (!AllDepFieldsSet.empty()) {
    LLVM_DEBUG(
        dbgs() << "    No Candidate is qualified after Dep...Skip DynClone\n");
    CandidateFields.clear();
    return false;
  }

  // (Reencoding) Bail out candidates that are not in the equivalence set.
  ValidCandidates.clear();
  for (auto &CandidatePair : CandidateFields)
    if (FieldEquivSet.count(CandidatePair))
      ValidCandidates.push_back(CandidatePair);

  std::swap(CandidateFields, ValidCandidates);

  // (Reencoding) Collect all possible constants for candidate fields in one
  // set and use them in the unified encode and decode routines.
  for (auto &CPair : CandidateFields) {
    auto It2 = DynFieldConstValueMap.find(CPair);
    if (It2 != DynFieldConstValueMap.end())
      set_union(AllDynFieldConstSet, It2->second);
  }

  // (Reencoding) Delta is needed to encode those constants that doesn't fit
  // into ShunkenIntType. Limit number of constants to 10 for runtime
  // effectiveness.
  if (AllDynFieldConstSet.size() > 10) {
    DEBUG_WITH_TYPE(
        REENCODING,
        dbgs()
            << "    Too many large constants for encoding...Skip DynClone\n");
    CandidateFields.clear();
    return false;
  }
  LLVM_DEBUG(dbgs() << "  Candidate fields after Pruning: \n";
             printCandidateFields(dbgs()));

  // Verify that all qualified candidate fields have unknown assignments
  // in the same routine. Otherwise, clear CandidateFields.
  assert(!InitRoutine && "Unexpected InitRoutine");
  for (auto &CPair : CandidateFields) {
    LLVM_DEBUG(dbgs() << "    Checking init routine for: ";
               printDynField(dbgs(), CPair));
    auto It = DynFieldFuncMap.find(CPair);
    if (It == DynFieldFuncMap.end()) {
      LLVM_DEBUG(dbgs() << "    Failed ... Field doesn't have Init routine\n");
      InitRoutine = nullptr;
      break;
    }
    if (!InitRoutine) {
      InitRoutine = It->second;
      continue;
    }
    if (InitRoutine != It->second) {
      LLVM_DEBUG(dbgs() << "    Failed ... Init routine doesn't match\n");
      InitRoutine = nullptr;
      break;
    }
  }

  if (!InitRoutine) {
    LLVM_DEBUG(dbgs() << "  No Init Routine identified...Skip DynClone\n");
    CandidateFields.clear();
    return false;
  }

  LLVM_DEBUG(dbgs() << "  Init Routine: " << InitRoutine->getName() << "\n");

  return !CandidateFields.empty();
}

// Analyze all MultiElem Load/Store instructions that are collected.
// Return false if not able to detect all accessed fields by the
// instructions.
// For now, using the below pattern to detect all accessed fields.
// DynClone will be disabled if any other pattern is noticed.
//
//   %GEP1 = getelementptr %struct.a, %struct.a* %p, 0, 2
//   %GEP2 = getelementptr %struct.a, %struct.a* %p, 0, 3
//   %S = select i1, %GEP1, %GEP2
//   %ld = load %S
//
// Another main legality check here is that all accessed fields
// should be marked with AOSTOSOA index. This helps us in two ways.
// 1. During transformation, these loads/stores will be converted to
//    single type (i.e 16 bits).
// 2. During analysis, issue with compile-time/runtime checks for
//    MultiElem load/store instructions can be avoided.
//
bool DynCloneImpl::verifyMultiFieldLoadStores(void) {

  //
  auto AnalyzeMultiElemLdSt = [&](Instruction *I) {
    Value *PtrOp;
    if (auto *LI = dyn_cast<LoadInst>(I))
      PtrOp = LI->getPointerOperand();
    else if (auto *SI = dyn_cast<StoreInst>(I))
      PtrOp = SI->getPointerOperand();
    else
      llvm_unreachable("Unexpected Instruction for MultiElem ld/st");

    auto *Sel = dyn_cast<SelectInst>(PtrOp);
    if (!Sel)
      return false;

    Value *TV = Sel->getTrueValue();
    Value *FV = Sel->getFalseValue();
    GEPOperator *TGEP = dyn_cast<GEPOperator>(TV);
    GEPOperator *FGEP = dyn_cast<GEPOperator>(FV);
    if (!TGEP || !FGEP)
      return false;
    auto TElem = getAccessStructField(TGEP);
    if (!TElem.first)
      return false;
    auto FElem = getAccessStructField(FGEP);
    if (!FElem.first)
      return false;
    // Make sure both are marked with AOSTOSOA index.
    if (!isAOSTOSOAIndexField(FElem) || !isAOSTOSOAIndexField(TElem))
      return false;
    // This map will be used during transformation.
    MultiElemLdStAOSTOSOAIndexMap[I] = TElem;
    return true;
  };

  for (auto *II : MultiElemLdStSet) {
    LLVM_DEBUG(dbgs() << "  Verifying MultiElem load/Store: " << *II << "\n");
    if (!AnalyzeMultiElemLdSt(II))
      return false;
  }
  return true;
}

// Check legality issues for InitRoutine here. DynClone will be disabled
// if any legality check is failed for InitRoutine. Basically, it proves
// that structs with candidate fields are not accessed before
// InitRoutine is called. It does following checks:
//   1. InitRoutine is called only once in "main" routine
//   2. Call to InitRoutine is not in Loop
//   3. No access to a struct with candidate fields before InitRoutine is
//      called.
//
bool DynCloneImpl::verifyLegalityChecksForInitRoutine(void) {

  std::function<bool(BasicBlock * CurrentBB,
                     SmallPtrSetImpl<BasicBlock *> & Visited,
                     Instruction * InitInst)>
      CandidateFieldAccessAfterBB;

  // Recursive lambda function to check legality issues for InitRoutine
  // in CurrentBB and all of the successors until InitInst.
  CandidateFieldAccessAfterBB = [this, &CandidateFieldAccessAfterBB](
                                    BasicBlock *CurrentBB,
                                    SmallPtrSetImpl<BasicBlock *> &Visited,
                                    Instruction *InitInst) -> bool {
    if (!Visited.insert(CurrentBB).second)
      return true;

    // If CurrentBB is the BasicBlock that has call to InitRoutine, check
    // legality issues until the call. Otherwise, check for all instructions
    // in CurrentBB.
    BasicBlock *InitBB = InitInst->getParent();
    BasicBlock::iterator EndIt;
    if (InitBB == CurrentBB)
      EndIt = InitInst->getIterator();
    else
      EndIt = CurrentBB->end();

    BasicBlock::iterator It = CurrentBB->begin();
    for (; It != EndIt; ++It) {
      Instruction &I = *It;
      if (auto *Call = dyn_cast<CallBase>(&I)) {
        // Treat InitRoutine as invalid if there are any indirect calls or
        // calls to user defined routines.
        const Function *Callee = Call->getCalledFunction();
        assert(Callee && "Expected only direct calls");
        // Since WholeProgramSafe is true, just check if Callee is defined
        // to prove it is a user defined routine.
        if (!Callee->isDeclaration()) {
          LLVM_DEBUG(dbgs() << "    InitRoutine failed...User routine called: "
                            << I << "\n");
          return false;
        }
      }
      auto StType = getTypeRelatedToInstruction(&I);
      // DynClone is disabled if a candidate struct is accessed before
      // InitRoutine is called.
      if (!StType)
        continue;
      for (auto &CandidatePair : CandidateFields)
        if (CandidatePair.first == StType) {
          LLVM_DEBUG(dbgs() << "    InitRoutine failed...Struct accessed "
                               "before InitRoutine:"
                            << getStructName(StType) << "\n");
          return false;
        }
    }
    // Stop checking successors of CurrentBB if it is the BasicBlock that
    // has call to InitRoutine.
    if (InitBB == CurrentBB)
      return true;

    bool Result = true;
    for (auto *BB : successors(CurrentBB))
      Result = Result && CandidateFieldAccessAfterBB(BB, Visited, InitInst);

    return Result;
  };

  assert(InitRoutine && "Expected InitRoutine");

  // Check if InitRoutine is called only once from main and it is not in
  // Loop.
  if (!InitRoutine->hasOneUse()) {
    LLVM_DEBUG(dbgs() << "    InitRoutine failed...More than single use \n");
    return false;
  }
  CallInst *CI = cast<CallInst>(InitRoutine->user_back());
  if (InitRoutine->hasAddressTaken()) {
    LLVM_DEBUG(dbgs() << "    InitRoutine failed...AddressTaken \n");
    return false;
  }
  Function *Caller = CI->getCaller();
  if (!isMainFunction(*Caller) || !Caller->use_empty()) {
    LLVM_DEBUG(dbgs() << "    InitRoutine failed...Not called from main \n");
    return false;
  }

  // TODO: Irreducible CFG check needs to be added here.

  BasicBlock *InitBB = CI->getParent();
  LoopInfo &LI = (GetLI)(*Caller);
  if (!LI.empty() && LI.getLoopFor(InitBB)) {
    LLVM_DEBUG(dbgs() << "    InitRoutine failed...call in Loop \n");
    return false;
  }

  // Check legality issues in all BasicBlocks starting from Entry block
  // to the place where InitRoutine is called.
  SmallPtrSet<BasicBlock *, 32> Visited;
  BasicBlock *StartBB = &Caller->getEntryBlock();
  if (!CandidateFieldAccessAfterBB(StartBB, Visited, CI))
    return false;

  SmallPtrSet<Type *, 4> StructAllocFound;
  int32_t BBCount = 0;
  // Collect Alloc calls related to candidate structs and count return
  // stmts.
  for (BasicBlock &BB : *InitRoutine) {
    if (isa<ReturnInst>(BB.getTerminator()))
      BBCount++;
    for (Instruction &II : BB)
      if (isa<CallInst>(&II)) {
        auto *CInfo = DTInfo.getCallInfo(&II);
        Type *StTy = getCallInfoElemTy(CInfo);
        if (!StTy || !isCandidateStruct(StTy) ||
            CInfo->getCallInfoKind() != CallInfo::CIK_Alloc)
          continue;
        AllocCalls.push_back(std::make_pair(cast<AllocCallInfo>(CInfo), StTy));
        StructAllocFound.insert(StTy);
      }
  }

  // Allow only single Return for now.
  if (BBCount != 1) {
    LLVM_DEBUG(dbgs() << "    InitRoutine failed...More than one Return\n");
    return false;
  }
  // We are not allowing memory allocation for candidate structs before
  // init routine is called. DynClone is not triggered for a struct
  // if there is global or local instance of it. So, we expect some
  // memory allocation for every candidate struct in init routine since
  // candidate struct's fields are accessed in the routine.
  // If allocation for a candidate struct is not found in init routine,
  // DynClone is disabled. Check here if each candidate struct has
  // allocation in init routine.
  for (auto &CandidatePair : CandidateFields)
    if (StructAllocFound.count(CandidatePair.first) == 0) {
      LLVM_DEBUG(dbgs() << "    InitRoutine failed...no allocation seen\n");
      return false;
    }
  return true;
}

// Tracks uses of all allocated calls and collects all locations
// where alloc pointers are saved if possible. Otherwise, returns
// false. These locations will be used during transformation.
bool DynCloneImpl::trackPointersOfAllocCalls(void) {

  std::function<bool(PHINode *, unsigned, bool &, StoreInstSet &,
                     StoreInstSet &, InstSet &)>
      TrackPHI;
  std::function<bool(GetElementPtrInst *, unsigned, bool &, StoreInstSet &,
                     StoreInstSet &, InstSet &)>
      TrackGEP;
  std::function<bool(Value *, bool &, StoreInstSet &, StoreInstSet &,
                     InstSet &)>
      TrackUsesOfVal;

  // Returns true if \p Val is a GEP that accesses any field from
  // a GlobalVariable at index zero.
  //  Ex:
  //  getelementptr (%struct.network, %struct.network* @net, i64 0, i32 3)
  //
  auto IsSimpleFieldInGlobalStructVar = [&](Value *Val) {
    auto *GEPO = dyn_cast<GEPOperator>(Val);
    if (!GEPO || GEPO->getNumIndices() != 2 ||
        !isa<GlobalVariable>(GEPO->getOperand(0)) ||
        !cast<GlobalVariable>(GEPO->getOperand(0))
             ->getInitializer()
             ->isNullValue() ||
        !isa<StructType>(GEPO->getSourceElementType()) ||
        !isa<ConstantInt>(GEPO->getOperand(1)) ||
        !cast<ConstantInt>(GEPO->getOperand(1))->isZero() ||
        !isa<ConstantInt>(GEPO->getOperand(2)))
      return false;
    return true;
  };

  // Store instructions, which save alloc pointers, are divided into
  // groups. If alloc pointer is saved in global struct, it is considered
  // as SimplePtrStore. Otherwise, considered as ComplexPtrStore.
  auto ProcessStoreInst = [&](StoreInst *SI, StoreInstSet &SimplePtrStoreSet,
                              StoreInstSet &ComplexPtrStoreSet) {
    if (IsSimpleFieldInGlobalStructVar(SI->getPointerOperand()))
      SimplePtrStoreSet.insert(SI);
    else
      ComplexPtrStoreSet.insert(SI);
  };

  // This function tracks uses of \p GEPI. It allows only three types of uses:
  // GEP/PHI/Store. This function is called recursively in case of GEP/PHI.
  // Store instructions will be added to either \p SimplePtrStoreSet or \p
  // ComplexPtrStoreSet depending on PointerOperand. \p ModifiedMem is set to
  // true if allocated memory is modified. \p GEPI will be added to \p
  // ProcessedInst.
  TrackGEP = [&TrackGEP, &TrackPHI, &ProcessStoreInst](
                 GetElementPtrInst *GEPI, unsigned Depth, bool &ModifiedMem,
                 StoreInstSet &SimplePtrStoreSet,
                 StoreInstSet &ComplexPtrStoreSet, InstSet &ProcessedInst) {
    unsigned NumIndices = GEPI->getNumIndices();
    Type *ElemTy = GEPI->getSourceElementType();
    if (NumIndices > 2 || !isa<StructType>(ElemTy))
      return false;

    ProcessedInst.insert(GEPI);

    // We are handling two cases when a pointer, which is being tracked,
    // is used in GEP.
    // Case 1: Pointer is used in GEP to get address of a field.
    // Case 2: Pointer is used in GEP to get address of Nth element in array.

    if (NumIndices == 2) {
      // Case 1:
      // This is basically address of a field if NumIndices == 2. We are not
      // interested to collet uses of the GEP. Instead, we prove that the
      // address is not escaped by checking all uses of it are used in StoreInst
      // as PointerOperand. Set ModifiedMem flag to indicate allocated memory is
      // modified.
      ModifiedMem = true;
      for (auto *GUSE : GEPI->users()) {
        Instruction *I = cast<Instruction>(GUSE);
        // Ignore PtrAnnotations of DTrans.
        if (DTransAnnotator::isDTransPtrAnnotation(*I) && I->hasNUses(0))
          continue;
        if (!isa<StoreInst>(GUSE))
          return false;
        StoreInst *SI = cast<StoreInst>(GUSE);
        if (SI->getPointerOperand() != GEPI)
          return false;
      }
      return true;
    }

    // Case 2: Treat the result of the GEP as pointer of some element and track
    // uses of this GEP to collect the locations where it is saved. Allowed only
    // GEP/PHI/Store.
    for (auto *GUSE : GEPI->users()) {
      if (auto *GEPUU = dyn_cast<GetElementPtrInst>(GUSE)) {
        if (!TrackGEP(GEPUU, Depth, ModifiedMem, SimplePtrStoreSet,
                      ComplexPtrStoreSet, ProcessedInst))
          return false;
      } else if (auto *PHI = dyn_cast<PHINode>(GUSE)) {
        if (!TrackPHI(PHI, ++Depth, ModifiedMem, SimplePtrStoreSet,
                      ComplexPtrStoreSet, ProcessedInst))
          return false;
      } else if (auto *St = dyn_cast<StoreInst>(GUSE))
        ProcessStoreInst(St, SimplePtrStoreSet, ComplexPtrStoreSet);
      else
        return false;
    }
    return true;
  };

  // This function tracks uses of \p PHI. It allows only 3 types of uses:
  // PHI/GEP/Store. This function is called recursively only in case of GEP.
  // Store instructions will be added to either \p SimplePtrStoreSet or
  // \p ComplexPtrStoreSet depending on PointerOperand. \p ModifiedMem is set
  // to true if allocated memory is modified. \p PHI will added to
  // \p ProcessedInst for further processing later.
  TrackPHI = [&TrackPHI, &TrackGEP, &ProcessStoreInst](
                 PHINode *PHI, unsigned Depth, bool &ModifiedMem,
                 StoreInstSet &SimplePtrStoreSet,
                 StoreInstSet &ComplexPtrStoreSet, InstSet &ProcessedInst) {
    // Limit recursion.
    if (Depth > 3)
      return false;

    ProcessedInst.insert(PHI);
    // Allows only PHI/GEP/Store
    for (auto *PUSE : PHI->users()) {
      if (!isa<Instruction>(PUSE))
        return false;

      if (auto *PHI2 = dyn_cast<PHINode>(PUSE)) {
        if (!TrackPHI(PHI2, ++Depth, ModifiedMem, SimplePtrStoreSet,
                      ComplexPtrStoreSet, ProcessedInst))
          return false;
      } else if (auto *GEPI = dyn_cast<GetElementPtrInst>(PUSE)) {
        if (!TrackGEP(GEPI, Depth, ModifiedMem, SimplePtrStoreSet,
                      ComplexPtrStoreSet, ProcessedInst))
          return false;
      } else if (auto *St = dyn_cast<StoreInst>(PUSE))
        ProcessStoreInst(St, SimplePtrStoreSet, ComplexPtrStoreSet);
      else
        return false;
    }
    return true;
  };

  // This function tracks uses of \p Val. It allows only 5 types of uses:
  // ICMP/BitCast/PHI/GEP/Store. This function is called recursively only
  // in case of BitCast. Store instructions will be added to either
  // \p SimplePtrStoreSet or  \p ComplexPtrStoreSet depending
  // PointerOperand. \p ModifiedMem is set to true if allocated memory
  // is modified. All tracked instructions will be added to ProcessedInst for
  // further processing.
  TrackUsesOfVal = [&TrackUsesOfVal, &TrackPHI, &TrackGEP,
                    &ProcessStoreInst](Value *Val, bool &ModifiedMem,
                                       StoreInstSet &SimplePtrStoreSet,
                                       StoreInstSet &ComplexPtrStoreSet,
                                       InstSet &ProcessedInst) {
    ProcessedInst.insert(cast<Instruction>(Val));
    for (User *U : Val->users()) {
      if (!isa<Instruction>(U))
        return false;
      Instruction *I = cast<Instruction>(U);
      // Ignore uses in ICmp.
      if (I->getOpcode() == Instruction::ICmp)
        continue;

      if (auto *BC = dyn_cast<BitCastInst>(I)) {
        // No need to check for Safe BitCast as safety checks are already
        // passed.
        if (!TrackUsesOfVal(BC, ModifiedMem, SimplePtrStoreSet,
                            ComplexPtrStoreSet, ProcessedInst))
          return false;
      } else if (auto *St = dyn_cast<StoreInst>(I))
        ProcessStoreInst(St, SimplePtrStoreSet, ComplexPtrStoreSet);
      else if (auto *GEPI = dyn_cast<GetElementPtrInst>(I)) {
        if (!TrackGEP(GEPI, 0, ModifiedMem, SimplePtrStoreSet,
                      ComplexPtrStoreSet, ProcessedInst))
          return false;
      } else if (auto *PHI = dyn_cast<PHINode>(I)) {
        if (!TrackPHI(PHI, 1, ModifiedMem, SimplePtrStoreSet,
                      ComplexPtrStoreSet, ProcessedInst))
          return false;
      } else
        return false;
    }
    return true;
  };

  // \p SimplePtrStoreSet has store instructions that store tracking pointers,
  // which point to currently processing memory allocation, to fields of global
  // structure variable. This routine collects if there are any loads from those
  // fields in init routine. It also tries to find if there are any stores to
  // those fields other than ones in \p SimplePtrStoreSet. If there are other
  // stores, that means some other pointer (i.e not allocated by the current
  // allocation call) is saved in the fields.  Returns false if it finds any
  // other store that is not in \p SimplePtrStoreSet.
  auto CollectLoadInstFromSimplePtrStoreLocs =
      [&](StoreInstSet &SimplePtrStoreSet, LoadInstSet &LoadSet) {
        // Collect all locations where pointers are saved.
        DynFieldSet SimpleStoreElems;
        if (SimplePtrStoreSet.size() == 0)
          return true;
        for (auto *SI : SimplePtrStoreSet) {
          auto StElem = DTInfo.getStoreElement(SI);
          if (!StElem.first)
            return false;
          SimpleStoreElems.insert(StElem);
        }

        for (Instruction &I : instructions(InitRoutine)) {
          if (auto *LI = dyn_cast<LoadInst>(&I)) {
            auto LdElem = DTInfo.getLoadElement(LI);
            // Check if LI is accessing locations where tracking pointers are
            // saved.
            if (SimpleStoreElems.count(LdElem))
              LoadSet.insert(LI);
          } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
            // Okay if SI is already processed simple pointer store.
            if (SimplePtrStoreSet.count(SI))
              continue;
            // Return false if SI is storing to location where tracking pointers
            // are saved.
            auto StElem = DTInfo.getStoreElement(SI);
            if (SimpleStoreElems.count(StElem))
              return false;
          }
        }
        return true;
      };

  // This routine is used to get AOSTOSOA global variable for the given \p CI
  // allocation call of AOSTOSOA. Currently, this is implemented using pattern
  // match. This can be improved later. It returns nullptr if doesn't match the
  // pattern.
  //
  // It returns @__soa_.n in the below example.
  //
  // CI:  %57 = call noalias i8* @calloc(i64 %56, i64 96)
  // ...
  // U:   %59 = getelementptr i8, i8* %57, i64 0
  // U1:  %60 = bitcast i8* %59 to i64*
  // SI:  store i64* %60, i64** getelementptr (%__SOA_struct.n, %__SOA_struct.n*
  // @__soa_.n, i64 0, i32 0)
  //
  auto GetAOSSOAAllocType = [&](CallInst *CI) -> GlobalVariable * {
    for (User *U : CI->users()) {
      if (!isa<GetElementPtrInst>(U) || !U->hasOneUse())
        continue;
      User *U1 = *U->user_begin();
      if (!isa<BitCastInst>(U1) || !U1->hasOneUse())
        continue;
      StoreInst *SI = dyn_cast<StoreInst>(*U1->user_begin());
      if (!SI)
        continue;
      if (ConstantExpr *CE = dyn_cast<ConstantExpr>(SI->getPointerOperand())) {
        if (CE->getOpcode() != Instruction::GetElementPtr)
          continue;
        if (GlobalVariable *GV = dyn_cast<GlobalVariable>(CE->getOperand(0)))
          return GV;
      }
    }
    return nullptr;
  };

  // Returns true if \p GEP has no real use except \p SI. Caller knows \p SI is
  // one of the use of \p GEP.
  //   This checks the below pattern where load doesn't have any use.
  //
  // Ex:
  // GEP:  %268 = getelementptr %__SOADT_struct.arc*, %__SOADT_struct.arc**
  // %266, i64 %267
  // U1: %269 = bitcast %__SOADT_struct.arc** %268 to i64* // Single use
  // LI:  %270 = load i64, i64* %269   // No uses
  // SI:  store %__SOADT_struct.arc* %242, %__SOADT_struct.arc** %268
  //
  auto GEPHasNoRealUseExceptStore = [&](GetElementPtrInst *GEP, StoreInst *SI) {
    for (User *U : GEP->users()) {
      if (U == SI)
        continue;
      User *U1 = U;
      if (isa<BitCastInst>(U1)) {
        if (U1->hasNUses(0))
          continue;
        if (!U1->hasOneUse())
          return false;
        U1 = *U1->user_begin();
      }
      LoadInst *LI = dyn_cast<LoadInst>(U1);
      if (!LI || !LI->hasNUses(0))
        return false;
    }
    return true;
  };

  // Analyze pointer operand of \p SI and check if it stores to an element in
  // array field of SOA global variable. \p AInfo represents current processing
  // alloc call of shrunken struct. Load instruction that loads address of array
  // field of SOA global variable is added to \p SOAGVField, which is used later
  // to prove that there are no other pointers from another allocation call are
  // stored in the array field of SOA global variable. \p SOAGV represents AOS
  // global variable.
  //
  //  Ex:
  //  GEPI: %265 = getelementptr %__SOA_struct.n, %__SOA_struct.n* @__soa_.n,
  //  i64 0, i32 8
  //  LI:   %266 = load %__SOADT_struct.a**, %__SOADT_struct.a*** %265
  //  GEP:  %268 = getelementptr %__SOADT_struct.a*, %__SOADT_struct.a**
  //  %266, i64 %267
  //  SI:   store %__SOADT_struct.a* %242, %__SOADT_struct.a** %268
  //
  auto AnalyzeComplexPtr = [&](StoreInst *SI, GlobalVariable *SOAGV,
                               AllocCallInfo *AInfo,
                               LoadInstSet &SOAGVFieldLoads) {
    auto *GEP = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
    if (!GEP)
      return false;
    if (!GEPHasNoRealUseExceptStore(GEP, SI))
      return false;
    // Make sure it is just pointer arithmetic.
    if (GEP->getNumIndices() != 1)
      return false;
    auto *LI = dyn_cast<LoadInst>(GEP->getOperand(0));
    if (!LI)
      return false;
    auto *GEPI = dyn_cast<GetElementPtrInst>(LI->getOperand(0));
    if (!GEPI)
      return false;
    if (GEPI->getOperand(0) != SOAGV)
      return false;
    auto LdElem = DTInfo.getLoadElement(LI);
    if (!LdElem.first)
      return false;
    // Collect array field info of SOA global variable.
    SOAGlobalVarFieldSet.insert(LdElem);

    // Prove that pointers that are allocated from only one allocation call are
    // assigned to any array field of SOA global variable.
    auto *AI = SOAFieldAllocCallMap[LdElem];
    if (!AI)
      SOAFieldAllocCallMap[LdElem] = AInfo;
    else if (AI != AInfo)
      return false;

    // Collect load instructions that load addresses of array fields of AOS
    // global variable.
    SOAGVFieldLoads.insert(LI);
    return true;
  };

  // Analyze all stores in \p ComplexPtrStoreSet and find locations (i.e array
  // fields of SOA global variable) if possible. Collects array fields of AOS
  // global variable and the corresponding allocation call of shrunken struct
  // whose return pointers are saved in the array fields. Prove that no pointers
  // from other allocation calls are saved in those locations. Also, prove that
  // saved pointers are not escaped. \p AInfo represents current processing
  // allocation call of shrunken struct.
  auto AnalyzeComplexPtrStores = [&](StoreInstSet &ComplexPtrStoreSet,
                                     AllocCallInfo *AInfo,
                                     GlobalVariable *SOAGV) {
    if (ComplexPtrStoreSet.size() == 0)
      return true;

    if (!SOAGV)
      return false;

    // Analyze each store and collect locations where tracking pointers are
    // stored.
    LoadInstSet SOAGVFieldLoads;
    for (auto *SI : ComplexPtrStoreSet)
      if (!AnalyzeComplexPtr(SI, SOAGV, AInfo, SOAGVFieldLoads))
        return false;
    // Since it is AOSTOSOA struct, we can assume field addresses of the struct
    // are not escaped. So, there is no way to store elements in array fields of
    // SOA global struct without loading address of array field of SOA struct.
    // Uses of load instructions in SOAGVFieldLoads are processed earlier. Prove
    // that no other pointers are saved to array fields and the saved pointers
    // are not escaped, where tracking pointer are saved, in the routine, by
    // checking no other load of the array fields except the ones in
    // SOAGVFieldLoads.
    for (Instruction &I : instructions(InitRoutine)) {
      if (auto *LI = dyn_cast<LoadInst>(&I)) {
        auto LdElem = DTInfo.getLoadElement(LI);
        if (!LdElem.first)
          continue;
        // Check fields only related to current processing memory allocation.
        if (SOAGlobalVarFieldSet.count(LdElem) &&
            SOAFieldAllocCallMap[LdElem] == AInfo && !SOAGVFieldLoads.count(LI))
          return false;
      }
    }
    return true;
  };

  StoreInstSet SimplePtrStoreSet;
  StoreInstSet ComplexPtrStoreSet;
  InstSet ProcessedInst;
  StoreInstSet NewSimplePtrStoreSet;
  LoadInstSet LoadSet;

  GlobalVariable *SOAGlobVar = nullptr;
  // For now, allow only one AOSTOSOA allocation call.
  if (AOSSOAACalls.size() == 1) {
    // Make sure it is calloc call. We can't allow other calls since we expect
    // zero in uninitialized memory locations.
    CallInst *CI = *AOSSOAACalls.begin();
    auto *ACI = DTInfo.getCallInfo(CI);
    // Get corresponding SOA global variable.
    if (ACI && cast<AllocCallInfo>(ACI)->getAllocKind() == dtrans::AK_Calloc) {
      SOAGlobVar = GetAOSSOAAllocType(CI);
      // This mapping is used during transformation.
      AOSSOACallGVMap[CI] = SOAGlobVar;
    }
  }

  for (auto &AllocPair : AllocCalls) {
    SimplePtrStoreSet.clear();
    ComplexPtrStoreSet.clear();
    ProcessedInst.clear();
    NewSimplePtrStoreSet.clear();
    LoadSet.clear();
    auto *AInfo = AllocPair.first;
    assert(AInfo->getAllocKind() == AK_Calloc && " Unexpected alloc kind");
    CallInst *CI = cast<CallInst>(AInfo->getInstruction());
    LLVM_DEBUG(dbgs() << "Tracking uses of  " << *CI << "\n");
    bool ModifiedMem = false;
    if (!TrackUsesOfVal(CI, ModifiedMem, SimplePtrStoreSet, ComplexPtrStoreSet,
                        ProcessedInst))
      return false;

    // SimplePtrStoreSet: Check if there are any loads from the locations
    // where alloc pointers are saved and track uses of those loads.
    // Make sure pointers of same alloc call are assigned to any location.

    // Collect load instructions that access saved locations and prove that all
    // saved pointers are from same allocation call.
    if (!CollectLoadInstFromSimplePtrStoreLocs(SimplePtrStoreSet, LoadSet))
      return false;

    // Track uses of all Load instructions that access saved locations.
    // Note that NewSimplePtrStoreSet is passed instead of SimplePtrStoreSet
    // to detect if use of any load instruction is saved to fields of global
    // struct. For now, SimplePtrStores are not allowed for load instructions to
    // simplify things.
    for (auto *LI : LoadSet) {
      if (!TrackUsesOfVal(LI, ModifiedMem, NewSimplePtrStoreSet,
                          ComplexPtrStoreSet, ProcessedInst))
        return false;
    }
    if (NewSimplePtrStoreSet.size() != 0) {
      return false;
    }

    // PHI nodes in ProcessedInst: Make sure all operands of PHI nodes are
    // pointing to the same memory allocation by checking all operands of PHI
    // are processed while tracking uses of allocation call.
    for (auto *II : ProcessedInst) {
      if (auto *PN = dyn_cast<PHINode>(II)) {
        for (unsigned I = 0, E = PN->getNumIncomingValues(); I != E; ++I) {
          auto *PI = dyn_cast<Instruction>(PN->getIncomingValue((I)));
          if (!PI)
            return false;
          if (ProcessedInst.count(PI) == 0)
            return false;
        }
      }
    }

    // ComplexPtrStoreSet: Analyze these stores further to prove that
    // they are saved into array fields of a struct, which is transformed
    // by AOS2SOA. Also, prove that there are no loads from those array
    // fields of the struct.
    if (!AnalyzeComplexPtrStores(ComplexPtrStoreSet, AInfo, SOAGlobVar))
      return false;

    // Collect modified allocations to avoid coping data from old layout
    // to new layout if there are no modifications.
    if (ModifiedMem)
      ModifiedAllocs.insert(CI);

    AllocSimplePtrStores[AInfo] = SimplePtrStoreSet;

    LLVM_DEBUG({
      dbgs() << "        SimplePtrStoreInsts: \n";
      for (auto *SI : SimplePtrStoreSet) {
        dbgs() << "            " << *SI << "\n";
      }
      dbgs() << "\n";
      dbgs() << "        ComplexPtrStoreInsts: \n";
      for (auto *SI : ComplexPtrStoreSet) {
        dbgs() << "            " << *SI << "\n";
      }
      dbgs() << "\n";
    });
  }
  return true;
}

// Verifies the legality checks for calls in InitRoutine. Main objects of
// these checks are:
//  1. Make sure user calls don't access candidate structs
//  2. Allocation pointers are stored in SimplePtrStores or ComplexPtrStores.
//     Make sure these pointers are not accessed by getting them from
//     SimplePtrStores or ComplexPtrStores.
//  3. When DynClone is triggered, it copies data from old layout to
//     new layout for all allocations. Make sure these pointers are valid
//     when copying by checking that there are no unsafe calls like "free".
//
//  Checking conditions:
//   1. Return false if InitRoutine has any unexpected intrinsic/lib call.
//   2. Return false if any use call has direct access to candidate structs.
//   3. User call is treated as unsafe call if it has any access to locations
//      of SimplePtrStores or ComplexPtrStores. Returns false if more than
//      one callsite of unsafe function call for now. That means, it still
//      returns true if there is only one unsafe user call in InitRoutine.
//      This unsafe call is handled with runtime check like below.
//
//      Ex:
//      Before:
//          init() {
//          ...
//          calloc();
//          calloc();
//          ...
//          if (some_error)
//            unsafecall();  // This call has access to locations of
//                           // SimplePtrStores or ComplexPtrStores.
//            return -1;
//          }
//
//          return 0;
//          }
//
//      After:
//          init() {
//          dyn.safe = 0; // New local variable is created and set to 0.
//          ...
//          dyn.safe = 1; // Set to 1 in BasicBlock where calloc are located.
//          calloc();
//          calloc();
//          ...
//          if (some_error)
//            dyn.safe = 0; // Reset when unsafe call is executed.
//            unsafecall();  // This call has access to locations of
//                           // SimplePtrStores or ComplexPtrStores.
//            return -1;
//          }
//
//          if (L_Max > 0x000000007fffffff ||
//              L_Min < 0xffffffff80000000 ||
//              dyn.safe == 0)
//              return 0;
//          }
//          else {
//            //... Copy Data here and fix pointers
//            __Shrink__Happened__ = 1;
//            return 0;
//          }
//          }
//
bool DynCloneImpl::verifyCallsInInitRoutine(void) {
  // Returns true if \p ID is safe intrinsic. This list is required
  // for now but not complete. We can add more intrinsics if it is needed.
  auto IsSafeIntrinsic = [&](Intrinsic::ID ID) {
    switch (ID) {
    case Intrinsic::lifetime_start:
    case Intrinsic::lifetime_end:
    case Intrinsic::ptr_annotation:
      return true;

    default:
      return false;
    }
  };

  // Returns true if \p LF is safe libfunc call. This list is required
  // for now but not complete. We can add more libfuncs if it is needed.
  auto IsSafeLibFunc = [&](LibFunc LF) {
    switch (LF) {
    case LibFunc_calloc:
    case LibFunc_dunder_isoc99_sscanf:
    case LibFunc_sscanf:
    case LibFunc_fclose:
    case LibFunc_fgets:
    case LibFunc_fgets_unlocked:
    case LibFunc_fopen:
    case LibFunc_fopen64:
    case LibFunc_puts:
    case LibFunc_printf:
      return true;

    default:
      return false;
    }
  };

  // Returns true if a candidate struct is referenced by any
  // instruction of \p F. It checks if \p F is in set of functions,
  // which have any reference to candidate struct.
  auto HasAnyAccessToCandidateStruct = [&](Function *F) {
    for (auto TPair : TypeAccessedInFunctions) {
      if (!isCandidateStruct(TPair.first))
        continue;
      if (TPair.second.count(F) != 0)
        return true;
    }
    return false;
  };

  // Finds locations of SimplePtrStores for all allocations and
  // add them to \p FSet.
  auto ComputeAllSimplePtrLocs = [&](DynFieldSet &FSet) {
    for (auto &AllocPair : AllocCalls) {
      auto *AInfo = AllocPair.first;
      StoreInstSet &SISet = AllocSimplePtrStores[AInfo];
      for (auto *St : SISet) {
        auto DF = DTInfo.getStoreElement(St);
        FSet.insert(DF);
      }
    }
  };

  // Returns true if \p F may access to any location of SimplePtrStores
  // (\p SimpleLocSet) or ComplexPtrStore (SOAGlobalVarFieldSet).
  // Conservatively returns true if \p F has any call.
  auto HasAnyAccessToPtrStoredLocs = [&](Function *F,
                                         DynFieldSet &SimpleLocSet) {
    for (Instruction &II : instructions(F)) {
      if (isa<DbgInfoIntrinsic>(II))
        continue;
      if (isa<CallInst>(&II))
        return true;
      DynField DField;
      if (auto *LI = dyn_cast<LoadInst>(&II)) {
        DField = DTInfo.getLoadElement(LI);
      } else if (auto *SI = dyn_cast<StoreInst>(&II)) {
        DField = DTInfo.getStoreElement(SI);
      } else {
        continue;
      }
      if (!DField.first)
        continue;
      if (SimpleLocSet.count(DField) || SOAGlobalVarFieldSet.count(DField))
        return true;
    }
    return false;
  };

  BasicBlock *AllocBB = nullptr;
  // Make sure all allocations of candidate structs are in same
  // basicblock.
  for (auto &AllocPair : AllocCalls) {
    BasicBlock *BB = AllocPair.first->getInstruction()->getParent();
    if (AllocBB == nullptr)
      AllocBB = BB;
    else if (AllocBB != BB)
      return false;
  }
  assert(AllocBB && "Expected BasicBlock for Alloc calls");
  for (auto *SOACI : AOSSOAACalls)
    if (AllocBB != SOACI->getParent())
      return false;
  LoopInfo &LI = (GetLI)(*InitRoutine);
  // Make sure allocation call is not in loop.
  if (!LI.empty() && LI.getLoopFor(AllocBB))
    return false;

  auto &TLI = GetTLI(*InitRoutine);
  DenseMap<Function *, CallInstSet> UserCallFuncs;
  // Check if InitRoutine has any unsafe lib or intrinsic
  // calls. Collect all user functions that are called and their
  // callsites.
  for (Instruction &I : instructions(InitRoutine)) {
    if (isa<DbgInfoIntrinsic>(I))
      continue;
    auto *Call = dyn_cast<CallInst>(&I);
    if (!Call)
      continue;
    Function *F = Call->getCalledFunction();
    assert(F && "Expected direct call");
    if (F->isIntrinsic()) {
      if (!IsSafeIntrinsic(F->getIntrinsicID())) {
        LLVM_DEBUG(dbgs() << "Unexpected intrinsic call  " << I << "\n");
        return false;
      }
      continue;
    }
    LibFunc Func;
    if (TLI.getLibFunc(*F, Func)) {
      if (!IsSafeLibFunc(Func) || !TLI.has(Func)) {
        LLVM_DEBUG(dbgs() << "Unexpected Lib call  " << I << "\n");
        return false;
      }
      continue;
    }
    UserCallFuncs[F].insert(Call);
  }

  // Return false if any user call in InitRoutine has access to
  // candidate struct.
  for (auto FPair : UserCallFuncs)
    if (HasAnyAccessToCandidateStruct(FPair.first))
      return false;

  // Compute locations of SimplePtrStores of all allocations.
  DynFieldSet AllSimplePtrStoreLocSet;
  ComputeAllSimplePtrLocs(AllSimplePtrStoreLocSet);

  FunctionSet UnsafeUserFunction;
  // Allocation pointers are saved in either SimplePtrStores or
  // ComplexPtrStores. A user call in InitRoutine is treated as unsafe
  // if it has any access to locations of SimplePtrStores or ComplexPtrStores.
  // Collect unsafe calls here.
  for (auto FPair : UserCallFuncs)
    if (HasAnyAccessToPtrStoredLocs(FPair.first, AllSimplePtrStoreLocSet))
      UnsafeUserFunction.insert(FPair.first);
  auto UnsafeCount = UnsafeUserFunction.size();

  // Everything okay if no unsafe calls.
  if (UnsafeCount == 0)
    return true;

  // Handle this case for now with runtime check only if there is only
  // one unsafe call and only one callsite.
  if (UnsafeCount > 1)
    return false;
  // Check it has only one callsite.
  auto CISet = UserCallFuncs[*UnsafeUserFunction.begin()];
  if (CISet.size() > 1)
    return false;
  CallInst *CI = *CISet.begin();
  // Make sure unsafe calls is not in loop.
  if (!LI.empty() && LI.getLoopFor(CI->getParent()))
    return false;

  // RuntimeCheckUnsafeCalls is used later during transformation.
  RuntimeCheckUnsafeCalls.insert(CI);
  return true;
}

// Create new shrunken structs for all candidate structs and maintain
// mapping between old and new layouts.
//
// New shrunken type is created using the following steps:
//  1. Reduce size of shrunken fields (ex: i64 to i32)
//  2. Reorder fields to eliminate padding created due to shrinking.
//     This step is really not needed since "isPacked" is applied for
//     entire new struct.
//  3. Eliminate padding by applying "isPacked" attribute.
//
void DynCloneImpl::createShrunkenTypes(void) {

  // Return shrunken type for AOSTOSOA index fields. For now, it
  // returns 16 bits always.
  auto GetAOSTOSOAIndexShrunkenType = [&]() {
    return Type::getInt16Ty(M.getContext());
  };

  // Return shrunken type for given struct and field index.
  auto GetShrunkenType = [&](StructType *StTy, unsigned Idx) {
    Type *Ty = StTy->getElementType(Idx);
    // Only i64 is supported for now.
    if (Ty->isIntegerTy(64)) {
      return ShrunkenIntTy;
    }
    llvm_unreachable("Unexpected type for shrinking");
  };

  // For given Struct type and field index, returns type of the field
  // in shrunken record.
  auto GetTypeInShrunkenStruct = [&, GetShrunkenType](StructType *StTy,
                                                      unsigned Idx) {
    DynField Field(std::make_pair(StTy, Idx));
    Type *ShrunkenTy;
    // Get shrunken field type if it is candidate field.
    if (isAOSTOSOAIndexField(Field))
      ShrunkenTy = GetAOSTOSOAIndexShrunkenType();
    else if (isCandidateField(Field))
      ShrunkenTy = GetShrunkenType(StTy, Idx);
    else
      ShrunkenTy = StTy->getElementType(Idx);
    return ShrunkenTy;
  };

  // First, create new struct types without any fields for each
  // candidate struct.
  // No need to fix types of pointer fields that point to their own
  // struct or any other candidate struct since it will be type-casted
  // to original struct types before accessing from memory.
  for (auto &CandidatePair : CandidateFields) {
    StructType *StructT = cast<StructType>(CandidatePair.first);
    if (TransformedTypeMap[StructT])
      continue;
    StructType *NewSt = StructType::create(StructT->getContext(),
                                           "__DYN_" + StructT->getName().str());
    TransformedTypeMap[StructT] = NewSt;
  }

  std::vector<FieldData> Fields; // SmallVector causes memory corruptions
                                 // use std::vector instead

  for (auto &CPair : TransformedTypeMap) {
    Fields.clear();
    StructType *StructT = CPair.first;
    StructType *NewSt = CPair.second;
    // Shrink candidate fields and then apply reordering.
    for (unsigned I = 0; I < StructT->getNumElements(); ++I) {
      Type *ShrunkenTy = GetTypeInShrunkenStruct(StructT, I);
      FieldData FD(DL.getABITypeAlignment(ShrunkenTy),
                   DL.getTypeStoreSize(ShrunkenTy), I);
      Fields.push_back(FD);
    }
    llvm::sort(Fields.begin(), Fields.end());

    // Create new layout of fields and maintain mapping of old and new
    // layouts.
    std::vector<Type *> EltTys(StructT->getNumElements());
    unsigned Index = 0;
    std::vector<unsigned> NewIdxVec(Fields.size());
    for (FieldData &FD : Fields) {
      unsigned NewIdx = FD.Index;
      EltTys[Index] = GetTypeInShrunkenStruct(StructT, NewIdx);
      NewIdxVec[NewIdx] = Index++;
    }
    TransformedIndexes[StructT] = NewIdxVec;

    // Set body of new struct and set isPacked to true.
    NewSt->setBody(EltTys, true);

    LLVM_DEBUG(dbgs() << "After dynamic shrinking " << getStructName(StructT)
                      << " :" << *NewSt << "\n");
  }
}

// Transform InitRoutine:
//
//   1. Generates Runtime checks to detect whether all values assigned
//      to candidate fields fit in shrunken type. "__Shrink__Happened__"
//      new GlobalVariable is created to indicate if sizes of candidate
//      fields are reduced. "__Shrink__Happened__" flag is used by other
//      routines to decide whether to use original data-layout or shrunken
//      data-layout. "__Shrink__Happened__" is set to zero when it is
//      created. This flag is set to 1 if all assigned values of candidate
//      fields fit in shrunken type. Collects min and max values that are
//      assigned to candidate fields and then decides at the end of routine
//      if the min and max values fits in shrunken types.
//
//   2. Copy Contents from old layout to new layout before ReturnInst
//
//      Ex:
//      Before:
//              struct netw { ...} net;
//
//              InitRoutine() {
//                  aosptr = calloc(sizeN * size_elem); // AOSTOSOA alloc call
//                  ...
//                  ptr = calloc(); // Memory allocation for candidate struct
//                  ...
//                  if (some_error) {
//                    unsafecall();
//                    return;
//                  }
//                  ...
//                  net->field1 = ptr;
//                  ...
//                  net->field2 = ptr + index;
//                  ...
//                  stp->candidate1 = value1;
//                  ...
//                  stp->candidate2 = value2;
//                  ...
//                  aosptr->fld5[index] = ptr + j;
//                  ...
//                  return;
//              }
//
//      After:
//              __Shrink__Happened__ = 0;
//
//              InitRoutine() {
//                  dyn.safeflag = 0;  // Initialize
//                  L_i64_Max = 0xffffffff80000000;
//                  L_i64_Min = 0x000000007fffffff;
//                  ...
//                  dyn.safeflag = 1;  // Set before allocation
//                  ptr = calloc();
//                  ...
//                  if (some_error) {
//                    dyn.safeflag = 0; // Reset if unsafecall is executed.
//                    unsafecall();
//                    return;
//                  }
//                  stp->candidate1 = value1;
//                  L_i64_Max = (L_i64_Max < value1) ? value1 : L_i64_Max;
//                  L_i64_Min = (L_i64_Min > value1) ? value1 : L_i64_Min;
//                  ...
//                  stp->candidate2 = value2;
//                  L_i64_Max = (L_i64_Max < value2) ? value2 : L_i64_Max;
//                  L_i64_Min = (L_i64_Min > value2) ? value2 : L_i64_Min;
//                  ...
//                  if !(L_i64_Max > 0x000000007fffffff ||
//                      L_i64_Min < 0xffffffff80000000 ||
//                      sizeN > 0xffff ||
//                      dyn.safeflag == 0) {
//                    OldType* SPtr = ptr;
//                    NewType* DPtr = ptr;
//                    for (i = 0; i < alloc_size_of_calloc; i++) {
//                      t0 = (SPtr + i)->field0;
//                      t1 = (SPtr + i)->field1;
//                      ...
//                      (DPtr + i)->new_field0 = t0;
//                      (DPtr + i)->new_field1 = t1;
//                      ...
//                    }
//                    // Rematerialize saved pointers
//                    if (net->field1) {
//                      net->field1 = ptr + ((((char*)net->field1) -
//                         ((char*)ptr)) / sizeof(OldType)) * sizeof(NewType);
//                    }
//                    if (net->field2) {
//                      net->field2 = ptr + ((((char*)net->field2) -
//                         ((char*)ptr)) / sizeof(OldType)) * sizeof(NewType);
//                    }
//
//                    // Rematerialize pointers that are saved in AOSTOSOA
//                    // global variable.
//                    for (i = 0; i < alloc_size_aostosoa_alloc_call; i++) {
//                      if (aosptr->fld5[i])
//                        aosptr->fld5[i] = ptr + ((((char*)aosptr->fld5[i]) -
//                         ((char*)ptr)) / sizeof(OldType)) * sizeof(NewType);
//                      }
//                    }
//
//                    __Shrink__Happened__ = 1;
//                  }
//                  return;
//              }
//
//
void DynCloneImpl::transformInitRoutine(void) {

  // Returns ConstantInt of max value that fits in shrunken type for
  // the given Ty.
  //    i64  ==>  Max value that fits in int32_t
  auto GetShrunkenMaxValue = [&](Type *Ty) -> Value * {
    if (Ty->isIntegerTy(64)) {
      return ConstantInt::get(Ty, getMaxShrIntTyValueWithDelta());
    }
    llvm_unreachable("Unexpected shrunken type for Max Value");
  };

  // Returns ConstantInt of min value that fits in shrunken type for
  // the given Ty.
  //    i64  ==>  Max value that fits in int32_t
  auto GetShrunkenMinValue = [&](Type *Ty) -> Value * {
    if (Ty->isIntegerTy(64))
      return ConstantInt::get(Ty, getMinShrIntTyValueWithDelta());
    llvm_unreachable("Unexpected shrunken type for Min Value");
  };

  // Generates instructions to find  min / max values.
  auto GenerateMinMaxInsts =
      [&](DynField &StElem, StoreInst *Inst, CmpInst::Predicate Pred,
          SmallDenseMap<Type *, AllocaInst *> &TypeAllocIMap) {
        assert(isa<StoreInst>(Inst) && "Expected StoreInst");
        StructType *StTy = cast<StructType>(StElem.first);
        Type *Ty = StTy->getElementType(StElem.second);

        AllocaInst *AI = TypeAllocIMap[Ty];
        assert(AI && "Expected Local var for Ty");

        Value *LI = new LoadInst(AI, "d.ld", Inst);
        Value *SOp = Inst->getValueOperand();
        ICmpInst *ICmp = new ICmpInst(Inst, Pred, LI, SOp, "d.cmp");
        SelectInst *Sel = SelectInst::Create(ICmp, LI, SOp, "d.sel", Inst);
        StoreInst *SI = new StoreInst(Sel, AI, Inst);
        (void)SI;
        LLVM_DEBUG(dbgs() << "      " << *LI << "\n");
        LLVM_DEBUG(dbgs() << "      " << *ICmp << "\n");
        LLVM_DEBUG(dbgs() << "      " << *Sel << "\n");
        LLVM_DEBUG(dbgs() << "      " << *SI << "\n");
        return;
      };

  // Generate final condition with already loaded value \p LI and "or" with
  // previous condition if available.
  auto GenerateFinalCondWithLIValue =
      [&](LoadInst *LI, Value *V, CmpInst::Predicate Pred, Value *PrevCond,
          ReturnInst *RI) -> Value * {
    ICmpInst *ICmp = new ICmpInst(RI, Pred, LI, V, "d.cmp");
    LLVM_DEBUG(dbgs() << "      " << *ICmp << "\n");
    if (!PrevCond)
      return ICmp;

    Value *FinalCond = BinaryOperator::CreateOr(PrevCond, ICmp, "d.or", RI);
    LLVM_DEBUG(dbgs() << "      " << *FinalCond << "\n");
    return FinalCond;
  };

  // Generate final condition and "or" with previous condition if available.
  auto GenerateFinalCond =
      [&GenerateFinalCondWithLIValue](AllocaInst *AI, Value *V,
                                      CmpInst::Predicate Pred, Value *PrevCond,
                                      ReturnInst *RI) -> Value * {
    LoadInst *LI = new LoadInst(AI, "d.ld", RI);
    LLVM_DEBUG(dbgs() << "      " << *LI << "\n");
    return GenerateFinalCondWithLIValue(LI, V, Pred, PrevCond, RI);
  };

  // Creates GEP instruction like below using \p IRB and returns it.
  //   getelementptr inbounds  %StructTy, %StructTy* %BPtr, %AIdx, FieldIdx
  //
  auto CreateFieldAccessGEP = [&](Type *StructTy, Value *BPtr, PHINode *AIdx,
                                  unsigned FieldIdx,
                                  IRBuilder<> &IRB) -> Value * {
    SmallVector<Value *, 2> Indices;
    Indices.push_back(AIdx);
    Value *Op2 = ConstantInt::get(Type::getInt32Ty(M.getContext()), FieldIdx);
    Indices.push_back(Op2);
    Value *GEP = IRB.CreateInBoundsGEP(StructTy, BPtr, Indices);
    LLVM_DEBUG(dbgs() << "  " << *GEP << "\n");
    return GEP;
  };

  // \p CI is expected to be calloc instruction. Computes Allocation
  // size of \p CI using IRB and returns it.
  //
  // For "calloc(arg1, arg2)", it computes allocation size like
  //
  //    %t1 = mul i64 %arg1, %arg2
  //    %t2 = sdiv %t1, size_of_StTy
  //
  auto ComputeAllocCount = [&](CallInst *CI, Type *StTy,
                               IRBuilder<> &IRB) -> Value * {
    Value *Arg1 = CI->getArgOperand(0);
    Value *Arg2 = CI->getArgOperand(1);
    Value *TSize = IRB.CreateMul(Arg1, Arg2);
    Value *SSize =
        ConstantInt::get(TSize->getType(), DL.getTypeAllocSize(StTy));
    Value *ACount = IRB.CreateSDiv(TSize, SSize);
    LLVM_DEBUG(dbgs() << "  " << *TSize << "\n"
                      << "  " << *ACount << "\n");
    return ACount;
  };

  // Copy data from old layout to new layout for shrunken record by
  // generating loop. \p CI is expected to be calloc instruction.
  // \p Memory is allocated for OrigTy with the calloc instruction.
  // Copy loop is generated before \p InsertBefore.
  // (Reencode) Encode values before storing.
  //
  // Ex:   %ptr = calloc(asize, ssize)
  //       l_size = (asize * ssize) / size_of_OrigTy;
  //       l_ptr = %ptr;
  //
  // Assume NewTy is new shrunken record type for OrigTy.
  //
  // PreLoop:
  //  LoopIdx = 0;
  //  SrcPtr = (OrigTy*) l_ptr;
  //  DstPtr = (NewTy*) l_ptr;
  //  Goto Loop:
  // Loop:
  //   SGEP0 = getelementptr inbounds %OrigTy, %SrcPtr, LoopIdx, 0
  //   L0 = Load %SGEP0
  //   SGEP1 = getelementptr inbounds %OrigTy, %SrcPtr, LoopIdx, 1
  //   L1 = Load %SGEP1
  //   ...
  //
  //   DGEP0 = getelementptr inbounds %NewTy, %DstPtr, LoopIdx, new_idx_of_0
  //   %t1 = call __DYN_encoder(%L0)
  //   Store %t1, %DGEP0
  //   DGEP1 = getelementptr inbounds %NewTy, %DstPtr, LoopIdx, new_idx_of_1
  //   Store %L1, %DGEP1
  //   ...
  //   LoopIdx++
  //   if (LoopIdx < l_size) goto Loop:
  // PostLoop:
  //   __Shrink__Happened__ = 1;
  //
  auto CopyDataFromOrigLayoutToNewLayout = [this, &CreateFieldAccessGEP](
                                               CallInst *CI, Type *OrigTy,
                                               Instruction *InsertBefore) {
    // Get shrunken type.
    Type *NewTy = TransformedTypeMap[cast<StructType>(OrigTy)];
    LLVM_DEBUG(dbgs() << "Copy data for " << getStructName(OrigTy) << "  at "
                      << *CI << "\n");

    // Create blocks for PreLoop, Loop and PostLoop and fix CFG.
    BasicBlock *PreLoopBB = InsertBefore->getParent();
    BasicBlock *PostLoopBB =
        PreLoopBB->splitBasicBlock(InsertBefore, "postloop");
    BasicBlock *LoopBB = BasicBlock::Create(PreLoopBB->getContext(), "copydata",
                                            InitRoutine, PostLoopBB);

    IRBuilder<> PLB(PreLoopBB->getTerminator());

    Value *LCount = AllocCallSizes[CI];
    Value *SrcPtr =
        PLB.CreateBitCast(AllocCallRets[CI], OrigTy->getPointerTo());
    Value *DstPtr = PLB.CreateBitCast(AllocCallRets[CI], NewTy->getPointerTo());

    PLB.CreateBr(LoopBB);
    PreLoopBB->getTerminator()->eraseFromParent();
    LLVM_DEBUG(dbgs() << "  " << *SrcPtr << "\n"
                      << "  " << *DstPtr << "\n");

    // Create loop index.
    IRBuilder<> LB(LoopBB);
    Type *IdxTy = LCount->getType();
    PHINode *LoopIdx = LB.CreatePHI(IdxTy, 2, "lindex");
    LoopIdx->addIncoming(ConstantInt::get(IdxTy, 0U), PreLoopBB);

    SmallVector<LoadInst *, 8> LoadVec;
    StructType *OrigSt = cast<StructType>(OrigTy);
    StructType *NewSt = cast<StructType>(NewTy);
    // Generate Loads for all element of OrigTy at LoopIdx location.
    for (unsigned I = 0; I < OrigSt->getNumElements(); ++I) {
      Value *SrcGEP = CreateFieldAccessGEP(OrigTy, SrcPtr, LoopIdx, I, LB);
      LoadInst *LI = LB.CreateLoad(SrcGEP);
      LoadVec.push_back(LI);
      LLVM_DEBUG(dbgs() << "  " << *LI << "\n");
    }
    // Generate Stores for all element of NewTy at LoopIdx location.
    for (unsigned I = 0; I < OrigSt->getNumElements(); ++I) {
      unsigned NewI = TransformedIndexes[OrigSt][I];
      Value *DstGEP = CreateFieldAccessGEP(NewTy, DstPtr, LoopIdx, NewI, LB);
      Type *NewElemTy = NewSt->getElementType(NewI);
      Value *LI = LoadVec[I];
      if (LI->getType() != NewElemTy) {
        // (Reencoding) We need to encode each stored value for the candidate
        // fields of the new struct.
        DynField DF(std::make_pair(OrigSt, I));
        if (isCandidateField(DF) && DynFieldEncodeFunc)
          LI = LB.CreateCall(DynFieldEncodeFunc, {LI});
        else
          LI = LB.CreateTruncOrBitCast(LI, NewElemTy);
        LLVM_DEBUG(dbgs() << "  " << *LI << "\n");
      }
      StoreInst *SI = LB.CreateStore(LI, DstGEP);
      LLVM_DEBUG(dbgs() << "  " << *SI << "\n");
      (void)SI;
    }
    // Increment LoopIdx.
    Value *NewIdx = LB.CreateAdd(LoopIdx, ConstantInt::get(IdxTy, 1U));
    LoopIdx->addIncoming(NewIdx, LoopBB);
    auto *Br =
        LB.CreateCondBr(LB.CreateICmpULT(NewIdx, LCount), LoopBB, PostLoopBB);
    LLVM_DEBUG(dbgs() << "  " << *NewIdx << "\n");
    LLVM_DEBUG(dbgs() << "  " << *Br << "\n");
    (void)Br;
  };

  // Pointer to shrunken struct is fixed in this routine.
  //    Before:
  //               p = calloc(sizeof(StTy) * N);
  //               ...
  //               net->field = p + some_index;
  //               ...
  //               return;
  //    After:
  //               p = calloc(sizeof(StTy) * N);
  //               local_var = p;
  //               ...
  //               net->field = p + some_index;
  //               ...
  //               if (net->field) {
  //                 Ptr1 = (int64) net->field;
  //                 Ptr2 = (int64) local_var;
  //                 PtrDiff = Ptr1 - Ptr2;
  //                 Index = PtrDiff / sizeof(StTy);
  //                 NewPtr = local_var + Index * sizeof(new layout of StTy); //
  //                 net->field = NewPtr;
  //               }
  //               return;
  //
  // \p Ptr represents address location where alloc pointer is saved.
  //
  auto RematerializePtr = [&](Value *Ptr, LoadInst *RetPtr, Type *StTy,
                              Instruction *InsertBefore) {
    Type *IntPtrTy = Type::getIntNTy(M.getContext(), DL.getPointerSizeInBits());
    IRBuilder<> PIRB(InsertBefore);
    Value *LdPtr = PIRB.CreateLoad(Ptr);
    Value *Cond =
        PIRB.CreateICmpNE(LdPtr, Constant::getNullValue(LdPtr->getType()));
    BasicBlock *CondBB = InsertBefore->getParent();
    BasicBlock *MergeBB = CondBB->splitBasicBlock(InsertBefore);
    BasicBlock *RematBB =
        BasicBlock::Create(CondBB->getContext(), "remat", InitRoutine, MergeBB);
    IRBuilder<> CIR(CondBB->getTerminator());
    auto *Br = CIR.CreateCondBr(Cond, RematBB, MergeBB);
    CondBB->getTerminator()->eraseFromParent();
    (void)Br;
    IRBuilder<> IRB(RematBB);

    // Convert both pointers to IntTy.
    Value *Ptr1 = IRB.CreatePtrToInt(LdPtr, IntPtrTy);
    Value *Ptr2 = IRB.CreatePtrToInt(RetPtr, IntPtrTy);
    // Compute index of the pointer in original layout.
    Value *PtrDiff = IRB.CreateSub(Ptr1, Ptr2);
    Value *SSize = ConstantInt::get(IntPtrTy, DL.getTypeAllocSize(StTy));
    Value *Index = IRB.CreateSDiv(PtrDiff, SSize);
    // Use Index to get position of the pointer in new layout.
    Type *NewTy = TransformedTypeMap[cast<StructType>(StTy)];
    Value *BC = IRB.CreateBitCast(RetPtr, NewTy->getPointerTo());
    Value *NewPtr = IRB.CreateInBoundsGEP(NewTy, BC, makeArrayRef(Index));
    Value *NewBCPtr = IRB.CreateBitCast(NewPtr, StTy->getPointerTo());
    auto *UBI = IRB.CreateBr(MergeBB);
    (void)UBI;

    LLVM_DEBUG(dbgs() << "  " << *LdPtr << "  \n"
                      << *Cond << "  \n"
                      << *Br << "  \n"
                      << *Ptr1 << "  \n"
                      << *Ptr2 << "  \n"
                      << *PtrDiff << "  \n"
                      << *BC << "  \n"
                      << *Index << "  \n"
                      << *NewPtr << "\n"
                      << *NewBCPtr << "\n"
                      << *UBI << "\n");
    return NewBCPtr;
  };

  // Rematerialize pointers of shrunken struct that are saved
  // in array fields of AOSTOSOA global variable by generating loop.
  // \p LCount represents loop count, which is basically array size of
  // AOSTOSOA alloc call.  expected to be calloc instruction.
  // Rematerialize loop is generated before \p InsertBefore.
  // \p LoadVec has mapping of field address loads of array fields of
  // AOSTOSOA global variable and allocation info of shrunken record whose
  // only pointers are saved in the array field.
  // Ex:
  //       %aosptr = calloc(%aoselem, some_size); // AOSTOSOA alloc call
  //       %l_loop_count = %aoselem;
  //       ...
  //       %ptr = calloc(asize, ssize)
  //       %l_ptr = %ptr;
  //
  // Assume NewTy is new shrunken record type for OrigTy.
  //
  // %L1 = Load_address_of_field7;
  // %L2 = Load_address_of_field8;
  // PreLoop:
  //  %LoopIdx = 0;
  //  Goto Loop:
  // Loop:
  //   %GEP1 = getelementptr %struct.a*, %struct.a** %L1, i64 %LoopIdx
  //   %Load1 = load %struct.a*, %struct.a** %GEP1
  //   if (%Load1) {
  //     %Ptr1 = (int64) %Load1;
  //     %Ptr2 = (int64) %l_ptr;
  //     %PtrDiff = %Ptr1 - %Ptr2;
  //     %Index = %PtrDiff / sizeof(OrigTy);
  //     %NewPtr = %l_ptr + %Index * sizeof(NewTy);
  //     store %struct.arc* %NewPtr, %struct.arc** %GEP1
  //   }
  //   %GEP2 = getelementptr %struct.a*, %struct.a** %L2, i64 %LoopIdx
  //   %Load2 = load %struct.a*, %struct.a** %GEP2
  //   if (%Load2) {
  //     %Ptr1 = (int64) %Load2;
  //     %Ptr2 = (int64) %l_ptr;
  //     %PtrDiff = %Ptr1 - %Ptr2;
  //     %Index = %PtrDiff / sizeof(OrigTy);
  //     %NewPtr = %l_ptr + %Index * sizeof(NewTy);
  //     store %struct.arc* %NewPtr, %struct.arc** %GEP2
  //   }
  //   %LoopIdx++
  //   if (%LoopIdx < %l_loop_count) goto Loop:
  // PostLoop:
  //   __Shrink__Happened__ = 1;
  //
  auto RematerializePtrsStoredInAOSTOSOAGlobVar =
      [this, &RematerializePtr](
          Value *LCount,
          SmallVectorImpl<std::pair<LoadInst *, AllocCallInfo *>> &LoadVec,
          Instruction *InsertBefore) {
        BasicBlock *PreLoopBB = InsertBefore->getParent();
        BasicBlock *PostLoopBB =
            PreLoopBB->splitBasicBlock(InsertBefore, "rematpostloop");
        BasicBlock *LoopBB = BasicBlock::Create(
            PreLoopBB->getContext(), "rematptrs", InitRoutine, PostLoopBB);

        IRBuilder<> PLB(PreLoopBB->getTerminator());
        PLB.CreateBr(LoopBB);
        PreLoopBB->getTerminator()->eraseFromParent();

        IRBuilder<> LB(LoopBB);
        Type *IdxTy = LCount->getType();
        PHINode *LoopIdx = LB.CreatePHI(IdxTy, 2, "rematidx");
        LoopIdx->addIncoming(ConstantInt::get(IdxTy, 0U), PreLoopBB);

        Value *NewIdx = LB.CreateAdd(LoopIdx, ConstantInt::get(IdxTy, 1U));
        LoopIdx->addIncoming(NewIdx, LoopBB);
        auto *Br = LB.CreateCondBr(LB.CreateICmpULT(NewIdx, LCount), LoopBB,
                                   PostLoopBB);
        LLVM_DEBUG(dbgs() << "  " << *NewIdx << "\n");
        LLVM_DEBUG(dbgs() << "  " << *Br << "\n");
        (void)Br;

        Instruction *AddInst = cast<Instruction>(NewIdx);
        SmallVector<Value *, 2> Indices;
        for (auto &LPair : LoadVec) {
          IRBuilder<> LBody(AddInst);
          Indices.clear();
          Indices.push_back(LoopIdx);
          Value *GEP = LBody.CreateInBoundsGEP(nullptr, LPair.first, Indices);
          LLVM_DEBUG(dbgs() << "  " << *GEP << "\n");

          auto *AInfo = LPair.second;
          CallInst *CI = cast<CallInst>(AInfo->getInstruction());
          Type *Ty = getCallInfoElemTy(AInfo);
          assert(Ty && "Expected struct type associated with call");
          Value *NewPtr = RematerializePtr(GEP, AllocCallRets[CI], Ty, AddInst);

          IRBuilder<> MergeB(
              cast<Instruction>(NewPtr)->getParent()->getTerminator());
          StoreInst *StoreElem = MergeB.CreateStore(NewPtr, GEP);
          LLVM_DEBUG(dbgs() << "  " << *StoreElem << "\n");
          (void)StoreElem;
        }
      };

  // Creates new local variable in the entry basic block, saves \p Val in
  // the local variable immediately after \p CI and loads it again before \p
  // InsertBefore.
  //     Ex:
  //         AI:   %dyn.alloc = alloca i8*
  //               ...
  //         CI:   %call1 = calloc()
  //         SI:   store i8* %call1, i8** %dyn.alloc
  //         ...
  //         LI:   %dyn.alloc.ld = load i8*, i8** %dyn.alloc
  //         InsertBefore:
  //
  auto UnNormalizeUsingNewlyCreatedLVar = [&](Value *Val, CallInst *CI,
                                              Instruction *InsertBefore) {
    AllocaInst *AI =
        new AllocaInst(Val->getType(), DL.getAllocaAddrSpace(), nullptr,
                       "dyn.alloc", &InitRoutine->getEntryBlock().front());
    StoreInst *SI = new StoreInst(Val, AI, CI->getNextNode());
    LoadInst *LI = new LoadInst(AI, "dyn.alloc.ld", InsertBefore);
    (void)SI;
    LLVM_DEBUG(dbgs() << " Save and Restore: " << *Val << "\n");
    LLVM_DEBUG(dbgs() << "    " << *AI << "\n"
                      << "    " << *SI << "\n"
                      << "    " << *LI << "\n");
    return LI;
  };

  // Creates new safety flag to indicate allocation pointers are valid
  // before returning from InitRoutine. This is needed because the pointers
  // are used to copy data from old layout to new layout.
  auto GenerateAllocSafetyFlagInsts = [&]() {
    LLVM_DEBUG(dbgs() << "   Generated alloc Safety flag instructions:\n");
    Type *FlagType = Type::getInt8Ty(M.getContext());
    // Add instruction to initialize the flag with zero.
    AllocaInst *AI =
        new AllocaInst(FlagType, DL.getAllocaAddrSpace(), nullptr, "dyn.safe",
                       &InitRoutine->getEntryBlock().front());
    StoreInst *SI =
        new StoreInst(ConstantInt::get(FlagType, 0), AI, AI->getNextNode());
    LLVM_DEBUG(dbgs() << "      " << *AI << "\n");
    LLVM_DEBUG(dbgs() << "      " << *SI << "\n");

    // Add instruction to reset the flag before unsafe calls.
    for (auto CIInst : RuntimeCheckUnsafeCalls)
      SI = new StoreInst(ConstantInt::get(FlagType, 0), AI, CIInst);

    // Already proved that all allocations are in same basicblock.
    // Just add an instruction before first calloc to set the flag.
    Instruction *I = AllocCalls.front().first->getInstruction();
    SI = new StoreInst(ConstantInt::get(FlagType, 1), AI, I);
    LLVM_DEBUG(dbgs() << "      " << *SI << "\n");
    (void)SI;
    return AI;
  };

  SmallVector<BasicBlock *, 2> RetBBs;
  SmallVector<StoreInst *, 16> RuntimeCheckStores;

  // Collect all StoreInsts that assign values to candidate fields and
  // BasicBlocks's with ReturnInst..
  for (BasicBlock &BB : *InitRoutine) {
    // Collect BasicBlocks's with ReturnInst.
    if (isa<ReturnInst>(BB.getTerminator()))
      RetBBs.push_back(&BB);

    // Collect StoreInsts.
    for (auto I = BB.begin(), E = BB.end(); I != E; I++) {
      auto *Inst = &*I;
      if (auto *StInst = dyn_cast<StoreInst>(Inst)) {
        // Store instructions in MultiElemLoadStoreInfo are not
        // considered here for checking because DynClone is
        // disabled if any access field of those store instructions
        // is not marked with aostosoa index field.
        auto StElem = DTInfo.getStoreElement(StInst);
        if (!StElem.first)
          continue;
        if (!isCandidateField(StElem))
          continue;
        Value *V = StInst->getValueOperand();
        // Ignore constant values here since they are already verified.
        if (isa<ConstantInt>(V))
          continue;
        RuntimeCheckStores.push_back(StInst);
      }
    }
  }

  assert(RetBBs.size() == 1 && "Expected only one Return");

  // Create GlobalVariable to indicate whether DynClone occurred or not.
  Type *FlagType = Type::getInt8Ty(M.getContext());
  assert(!ShrinkHappenedVar && "Expected nullptr for ShrinkHappenedVar");
  ShrinkHappenedVar = new GlobalVariable(
      M, FlagType, false, GlobalVariable::CommonLinkage,
      ConstantInt::get(FlagType, 0), Twine("__Shrink__Happened__"));

  LLVM_DEBUG(dbgs() << "    ShrinkHappenedVar: " << *ShrinkHappenedVar << "\n");

  // Map of Type and AllocaInst where max value of the type is saved.
  SmallDenseMap<Type *, AllocaInst *> TypeMaxAllocIMap;
  // Map of Type and AllocaInst where min value of the type is saved.
  SmallDenseMap<Type *, AllocaInst *> TypeMinAllocIMap;

  // Create Local variables to save max and min values for each candidate
  // type.
  //  entry:
  //      %d.max = alloca i64
  //      store i64 -2147483648, i64* %d.max
  //      %d.min = alloca i64
  //      store i64 2147483647, i64* %d.min
  //
  LLVM_DEBUG(dbgs() << "    Create and Initialize min and max variables: \n");
  for (auto &CPair : CandidateFields) {
    StructType *StTy = cast<StructType>(CPair.first);
    Type *Ty = StTy->getElementType(CPair.second);
    // Create new Local max and min variables for Ty if not already there.
    if (TypeMaxAllocIMap[Ty]) {
      assert(TypeMinAllocIMap[Ty] &&
             " Expected local min variable already created");
      continue;
    }
    AllocaInst *AI =
        new AllocaInst(Ty, DL.getAllocaAddrSpace(), nullptr, "d.max",
                       &InitRoutine->getEntryBlock().front());
    TypeMaxAllocIMap[Ty] = AI;
    StoreInst *SI =
        new StoreInst(GetShrunkenMinValue(Ty), AI, AI->getNextNode());
    LLVM_DEBUG(dbgs() << "      " << *AI << "\n");
    LLVM_DEBUG(dbgs() << "      " << *SI << "\n");

    AI = new AllocaInst(Ty, DL.getAllocaAddrSpace(), nullptr, "d.min",
                        SI->getNextNode());
    TypeMinAllocIMap[Ty] = AI;
    SI = new StoreInst(GetShrunkenMaxValue(Ty), AI, AI->getNextNode());
    (void)SI;
    LLVM_DEBUG(dbgs() << "      " << *AI << "\n");
    LLVM_DEBUG(dbgs() << "      " << *SI << "\n");
  }

  // Generate instructions related to alloc safety flag.
  AllocaInst *AllocSafetyFlag = GenerateAllocSafetyFlagInsts();

  // For each StoreInst, it generates instructions like
  //   Before:
  //     store i64 %g1, i64* %F1, align 8
  //
  //   After:
  //       %d.ld = load i64, i64* %d.min
  //       %d.cmp = icmp slt i64 %d.ld, %g1
  //       %d.sel = select i1 %d.cmp, i64 %d.ld, i64 %g1
  //       store i64 %d.sel, i64* %d.min
  //       %d.ld1 = load i64, i64* %d.max
  //       %d.cmp2 = icmp sgt i64 %d.ld1, %g1
  //       %d.sel3 = select i1 %d.cmp2, i64 %d.ld1, i64 %g1
  //       store i64 %d.sel3, i64* %d.max
  //       store i64 %g1, i64* %F1, align 8
  //
  for (auto StInst : RuntimeCheckStores) {
    auto StElem = DTInfo.getStoreElement(StInst);
    LLVM_DEBUG(dbgs() << "    Find min and max for " << *StInst << "\n");
    auto It =
        DynFieldTracedToConstantValues[StElem].find(StInst->getValueOperand());
    if (It != DynFieldTracedToConstantValues[StElem].end()) {
      DEBUG_WITH_TYPE(REENCODING, dbgs()
                                      << "      (Reencoding) Skip store: store "
                                         "a value evaluating to constant "
                                      << *(StInst->getValueOperand()) << "\n");
      continue;
    }
    GenerateMinMaxInsts(StElem, StInst, ICmpInst::ICMP_SLT, TypeMinAllocIMap);
    GenerateMinMaxInsts(StElem, StInst, ICmpInst::ICMP_SGT, TypeMaxAllocIMap);
  }

  // Generates instructions to compute final condition to detect
  // whether the assigned values fit in shrunken types like below.
  //
  //   %d.ld16 = load i64, i64* %d.min
  //   %d.cmp17 = icmp slt i64 %d.ld16, -2147483648
  //   %d.ld18 = load i64, i64* %d.max
  //   %d.cmp19 = icmp sgt i64 %d.ld18, 2147483647
  //   %d.or = or i1 %d.cmp17, %d.cmp19
  //
  LLVM_DEBUG(dbgs() << "    Generate Final condition \n");
  BasicBlock *OrigBB = RetBBs.front();
  ReturnInst *RetI = cast<ReturnInst>(OrigBB->getTerminator());
  Value *FinalCond = nullptr;
  // Only calloc is expected to be the allocation call for AOSTOSOA struct.
  // Usually, we get number elements allocated by computing ((1st argument
  // * 2nd argument) / size_of_struct) for calloc. But, 2nd argument of calloc
  // after AOSTOSOA transformation may not be same as size of original struct.
  // So, just use 1st argument as number elements allocated for AOSTOSOA alloc
  // calls.
  for (auto *SOACI : AOSSOAACalls)
    AllocCallSizes[SOACI] =
        UnNormalizeUsingNewlyCreatedLVar(SOACI->getArgOperand(0), SOACI, RetI);

  for (auto &Pair : TypeMinAllocIMap)
    FinalCond = GenerateFinalCond(Pair.second, GetShrunkenMinValue(Pair.first),
                                  ICmpInst::ICMP_SLT, FinalCond, RetI);
  for (auto &Pair : TypeMaxAllocIMap)
    FinalCond = GenerateFinalCond(Pair.second, GetShrunkenMaxValue(Pair.first),
                                  ICmpInst::ICMP_SGT, FinalCond, RetI);

  // Generate runtime check for AOSTOSOA index. AOSTOSOA indexes are always
  // expected to be positive values. Check if AOSTOSOA exceeds 16-bit
  // unsigned max value.
  for (auto *SOACI : AOSSOAACalls)
    FinalCond = GenerateFinalCondWithLIValue(
        AllocCallSizes[SOACI],
        ConstantInt::get(AllocCallSizes[SOACI]->getType(),
                         std::numeric_limits<uint16_t>::max()),
        ICmpInst::ICMP_UGT, FinalCond, RetI);

  // Generate runtime check for alloc safety flag
  FinalCond = GenerateFinalCond(AllocSafetyFlag, ConstantInt::get(FlagType, 0),
                                ICmpInst::ICMP_EQ, FinalCond, RetI);

  // TODO: It is better not to set __Shrink__Happened__ when none of
  // StoreInst of candidate fields is executed at runtime in InitRoutine.
  // Add a another check for FinalCond to do the same.

  assert(FinalCond && "Expected non-null Final condition");

  // Before:
  //   original:
  //     ...
  //     ret void
  //
  // After:
  //   original:
  //   ...
  //   br i1 %d.or, label %1, label %d.set_happened
  //
  //   d.set_happened:
  //     store i8 1, i8* @__Shrink__Happened__
  //     br label %1
  //
  //   <label>:1:
  //     ret void
  //
  // Split OrigBB just before RetI.
  LLVM_DEBUG(dbgs() << "    Set __Shrink__Happened__ to 1 \n");
  BasicBlock *LastBB = OrigBB->splitBasicBlock(RetI);

  BasicBlock *NewBB = BasicBlock::Create(OrigBB->getContext(), "d.set_happened",
                                         OrigBB->getParent(), LastBB);
  OrigBB->getTerminator()->eraseFromParent();
  BranchInst *BI = BranchInst::Create(LastBB, NewBB, FinalCond, OrigBB);
  LLVM_DEBUG(dbgs() << "      " << *BI << "\n");

  StoreInst *SI =
      new StoreInst(ConstantInt::get(ShrinkHappenedVar->getValueType(), 1),
                    ShrinkHappenedVar, NewBB);
  LLVM_DEBUG(dbgs() << "      " << *SI << "\n");
  BranchInst::Create(LastBB, NewBB);
  (void)BI;

  // Save return and size values of alloc calls in local variables to
  // avoid issues with SSA.
  for (auto &AllocPair : AllocCalls) {
    CallInst *CI = cast<CallInst>(AllocPair.first->getInstruction());
    AllocCallRets[CI] = UnNormalizeUsingNewlyCreatedLVar(CI, CI, SI);

    IRBuilder<> IRB(CI);
    Value *ACount = ComputeAllocCount(CI, AllocPair.second, IRB);
    AllocCallSizes[CI] = UnNormalizeUsingNewlyCreatedLVar(ACount, CI, SI);
  }

  // Copy data from old layout to new layout.
  for (auto &AllocPair : AllocCalls) {
    auto *AInfo = AllocPair.first;
    assert(AInfo->getAllocKind() == AK_Calloc && " Unexpected alloc kind");
    CallInst *CI = cast<CallInst>(AInfo->getInstruction());
    CopyDataFromOrigLayoutToNewLayout(CI, AllocPair.second, SI);
  }

  // Rematerialize pointers of AllocCalls that are saved in GlobalVariables.
  for (auto &AllocPair : AllocCalls) {
    auto *AInfo = AllocPair.first;
    StoreInstSet &SISet = AllocSimplePtrStores[AInfo];
    CallInst *CI = cast<CallInst>(AInfo->getInstruction());
    LLVM_DEBUG(dbgs() << "      Rematerialize ptrs of :" << *CI << "\n");
    for (auto *St : SISet) {
      LLVM_DEBUG(dbgs() << "      Before Rematerialize:" << *St << "\n");
      Value *NewPtr = RematerializePtr(St->getPointerOperand(),
                                       AllocCallRets[CI], AllocPair.second, SI);
      IRBuilder<> IRB(cast<Instruction>(NewPtr)->getParent()->getTerminator());
      Instruction *NewSt = IRB.Insert(St->clone());
      NewSt->setOperand(0, NewPtr);
      LLVM_DEBUG(dbgs() << "      After Rematerialize:" << *NewSt << "\n");
    }
  }

  // Rematerialize pointers of AllocCalls that are saved in array fields of
  // AOSTOSOA global variable.
  SmallVector<Value *, 2> Indices;
  // Map between array field address of AOSTOSOA global variable and
  // an allocation of shrunken struct whose only pointers are saved
  // in the array field.
  SmallVector<std::pair<LoadInst *, AllocCallInfo *>, 4> FieldAddrAllocVec;
  IRBuilder<> IRB(SI);
  CallInst *SOACI;
  GlobalVariable *SOAGlobVar;
  if (!SOAGlobalVarFieldSet.empty()) {
    assert(AOSSOAACalls.size() == 1 && "Expected only one AOSTOSOA call");
    SOACI = *AOSSOAACalls.begin();
    SOAGlobVar = AOSSOACallGVMap[SOACI];
    assert(SOAGlobVar && "Expected AOSTOSOA global variable");

    LLVM_DEBUG(dbgs() << "    Field Address loads of AOSTOSOA Glob variable:"
                      << *SOAGlobVar << "\n");
    Type *IdxTy = Type::getIntNTy(M.getContext(), DL.getPointerSizeInBits());
    // Before generating a loop to rematerialize shrunken struct pointers that
    // are saved in array fields of AOSTOSOA struct, generate instructions to
    // load addresses of the array fields. Let us assume 7th and 8th array
    // fields of AOSTOSOA global variable where shrunken struct pointers are
    // saved.
    // Ex:
    //  %L1 = load %struct.a**, %struct.a*** getelementptr
    //                (%struct.n, %struct.n* @n, i64 0, i32 7)
    //  %L2 = load %struct.a**, %struct.a*** getelementptr
    //                (%struct.n, %struct.n* @n, i64 0, i32 8)
    for (auto &FI : SOAGlobalVarFieldSet) {
      Indices.clear();
      Indices.push_back(ConstantInt::get(IdxTy, 0U));
      Value *Op2 =
          ConstantInt::get(Type::getInt32Ty(M.getContext()), FI.second);
      Indices.push_back(Op2);
      Value *GEP = IRB.CreateInBoundsGEP(
          SOAGlobVar->getType()->getElementType(), SOAGlobVar, Indices);
      LoadInst *LI = IRB.CreateLoad(GEP);
      auto *AI = SOAFieldAllocCallMap[FI];
      assert(AI && "Expected AInfo for each array field ");
      // Maintain a map between array field address of AOSTOSOA global variable
      // and an allocation of shrunken struct whose only pointers are saved
      // in the array field.
      FieldAddrAllocVec.push_back(std::make_pair(LI, AI));
      LLVM_DEBUG(dbgs() << "      Load address of "; printDynField(dbgs(), FI);
                 dbgs() << "\n"
                        << "        GEP: " << *GEP << "\n"
                        << "        LI: " << *LI << "\n");
    }
    // Generate loop to rematerialize all shrunken struct pointers that
    // are saved in array fields of AOSTOSOA global variable.
    RematerializePtrsStoredInAOSTOSOAGlobVar(AllocCallSizes[SOACI],
                                             FieldAddrAllocVec, SI);
  }
}

//  Before:
//      main() {
//        ...
//        InitRoutine();
//        ...
//        foo();
//        ...
//        bar();
//        ..
//      }
//
//      foo() {
//        ...
//        baz();
//      }
//      bar() { // Assume no accesses to shrunken structs
//        ...
//      }
//      baz() { // Assume shrunken structs are accessed in this routine.
//        ...
//        baz();
//      }
//
// No user call is allowed before calling InitRoutine. Except InitRoutine,
// all other routines, which have accesses to shrunken structs,  are called
// from main are call-graph cloned. Calls to those routines in main are
// specialized with "__Shrink__Happened__ == 0" check.
//
//  After:
//      main() {
//        ...
//        InitRoutine();
//        ...
//        if (__Shrink__Happened__ == 0) {
//          foo.1();
//        }
//        else {
//          foo();
//        }
//
//        ...
//        bar();  // No specialization
//        ..
//      }
//
//      foo() {
//        ...
//        baz();
//      }
//
//      foo.1() { // foo is cloned as foo.1
//        ...
//        baz.1();  // baz.1 is clone of baz
//      }
//
//      bar() { // bar is not cloned.
//      }
//      baz() {
//        ...
//        baz();
//      }
//      baz.1() {
//        ...
//        baz.1();
//      }
//
// TODO: Main restriction for the above solution is that shrunken structs
// shouldn't be accessed in main routine directly. We could use
// CodeExtractor class to create a new function, which will be cloned
// and controlled under __Shrink__Happened__, for all BasicBlocks that can
// be reached from "InitRoutine()" call.
//
bool DynCloneImpl::createCallGraphClone(void) {

  std::function<void(Function * F, Function * CallerMain,
                     CallInstSet & SpecializedCallsInMain)>
      FindCloningCallChains;

  // We can't set NoInline for all cloned routines due to performance reasons.
  // That means, we need to clone callers of cloned routines also to generate
  // correct code. This recursive lambda function finds all call-chains of
  // cloned routines till "main" is reached. NoInline attribute is set for
  // calls of cloned routines in "main" later.
  FindCloningCallChains =
      [this, &FindCloningCallChains](Function *F, Function *CallerMain,
                                     CallInstSet &SpecializedCallsInMain) {
        LLVM_DEBUG(dbgs() << "    Finding cloning call-chain..." << F->getName()
                          << "\n");
        // Add to the list of cloned routines.
        if (!ClonedFunctionList.insert(F).second) {
          LLVM_DEBUG(dbgs() << "    Already processed ... \n");
          return;
        }

        if (F->hasAddressTaken()) {
          for (CallInst *CI : FunctionCallInsts[F]) {
            LLVM_DEBUG(dbgs() << "    Processing CallSite1: " << *CI << "\n");
            Function *CalledF = CI->getFunction();
            if (CalledF == CallerMain) {
              // Specialize these calls later.
              SpecializedCallsInMain.insert(CI);
              continue;
            }
            FindCloningCallChains(CalledF, CallerMain, SpecializedCallsInMain);
          }
        } else {
          for (auto *U : F->users()) {
            assert(isa<CallInst>(U) && "Expected CallInst");
            CallInst *CI = cast<CallInst>(U);
            LLVM_DEBUG(dbgs() << "    Processing CallSite2: " << *CI << "\n");
            Function *CalledF = CI->getFunction();
            if (CalledF == CallerMain) {
              // Specialize these calls later.
              SpecializedCallsInMain.insert(CI);
              continue;
            }
            FindCloningCallChains(CalledF, CallerMain, SpecializedCallsInMain);
          }
        }
      };

  // Specialize CallInstruction:
  //
  //   Before:
  //      main() {
  //        ...
  //        t = foo();  // F
  //      }
  //
  //   After:
  //      main() {
  //        ...
  //        if (__Shrink__Happened__ == 0) {
  //          t0 = foo.1();  // NewF
  //        }
  //        else {
  //          t1 = foo();   // F
  //        }
  //        t = PHI(t0, t1);
  //      }
  //
  auto SpecializeCallInst = [&](CallInst *CI, Function *F, Function *NewF) {
    // Create ICmp inst
    Instruction *OrigInst = cast<Instruction>(CI);
    LLVM_DEBUG(dbgs() << "    Specializing call: " << *OrigInst << "\n");
    Type *SHVarTy = ShrinkHappenedVar->getValueType();
    Constant *ConstantZero = ConstantInt::get(SHVarTy, 0);
    IRBuilder<> Builder(OrigInst);
    LoadInst *Load = Builder.CreateLoad(ShrinkHappenedVar, "d.gld");
    auto *Cond = Builder.CreateICmpEQ(Load, ConstantZero, "d.gc");
    LLVM_DEBUG(dbgs() << "      Load: " << *Load << "\n");
    LLVM_DEBUG(dbgs() << "      Cmp: " << *Cond << "\n");

    // Fix CFG and create new CallInst
    Instruction *TrueT = nullptr;
    Instruction *FalseT = nullptr;
    SplitBlockAndInsertIfThenElse(Cond, OrigInst, &TrueT, &FalseT);
    BasicBlock *TrueB = TrueT->getParent();
    BasicBlock *FalseB = FalseT->getParent();
    BasicBlock *MergeBlock = OrigInst->getParent();
    TrueB->setName("d.t");
    FalseB->setName("d.f");
    MergeBlock->setName("d.m");
    Instruction *NewInst = OrigInst->clone();
    OrigInst->moveBefore(FalseT);
    NewInst->insertBefore(TrueT);
    CallInst *NewCI = cast<CallInst>(NewInst);
    NewCI->setCalledFunction(NewF);

    LLVM_DEBUG(dbgs() << "      New Call (False): " << *NewInst << "\n");
    LLVM_DEBUG(dbgs() << "    Set NoInline for both orig and new calls\n");
    CI->setIsNoInline();
    NewCI->setIsNoInline();

    if (OrigInst->getType()->isVoidTy() || OrigInst->use_empty())
      return;

    // Unify returns values of original and new call instructions
    Builder.SetInsertPoint(&MergeBlock->front());
    PHINode *Phi = Builder.CreatePHI(OrigInst->getType(), 0, "d.p");
    SmallVector<User *, 16> UsersToUpdate;
    for (User *U : OrigInst->users())
      UsersToUpdate.push_back(U);
    for (User *U : UsersToUpdate)
      U->replaceUsesOfWith(OrigInst, Phi);
    Phi->addIncoming(NewInst, NewInst->getParent());
    Phi->addIncoming(OrigInst, OrigInst->getParent());
    LLVM_DEBUG(dbgs() << "      PHI: " << *Phi << "\n");
  };

  CallInst *CI = cast<CallInst>(InitRoutine->user_back());
  Function *CallerMain = CI->getCaller();

  FunctionSet CandidateAccessRoutines;
  // TODO: Move the below checks before generating any runtime checks.
  for (auto TPair : TypeAccessedInFunctions) {
    if (!isCandidateStruct(TPair.first))
      continue;
    FunctionSet FSet = TPair.second;
    // 1. Candidate struct should be accessed in at least two
    //    routines.
    // 2. One of them should be InitRoutine.
    // 3. Candidate struct shouldn't be accessed in main routine.
    if (FSet.size() < 2 || FSet.count(InitRoutine) == 0 ||
        FSet.count(CallerMain) != 0) {
      LLVM_DEBUG(dbgs() << "    Unexpected AccessSet .. Skip DynClone\n");
      return false;
    }
    CandidateAccessRoutines.insert(FSet.begin(), FSet.end());
  }
  if (CandidateAccessRoutines.empty()) {
    LLVM_DEBUG(dbgs() << "    Empty AccessSet .. Skip DynClone\n");
    return false;
  }

  // List of CallInsts that need to be specialized in main.
  CallInstSet SpecializedCallsInMain;
  // Collect all routines that need to be cloned.
  for (auto *F : CandidateAccessRoutines) {
    // Shouldn't clone InitRoutine
    if (F == InitRoutine)
      continue;
    LLVM_DEBUG(dbgs() << "    Collect call-chains: " << F->getName() << "\n");
    FindCloningCallChains(F, CallerMain, SpecializedCallsInMain);
  }

  // Create cloned routines
  for (auto *F : ClonedFunctionList) {
    ValueToValueMapTy VMap;
    LLVM_DEBUG(dbgs() << "    Cloning ..." << F->getName() << "\n");
    Function *NewF = CloneFunction(F, VMap);
    CloningMap[F] = NewF;
    LLVM_DEBUG(dbgs() << "    New Cloned entry: " << NewF->getName() << "\n");
  }

  for (auto *F : ClonedFunctionList) {
    Function *NewF = CloningMap[F];
    for (inst_iterator II = inst_begin(NewF), E = inst_end(NewF); II != E;
         ++II) {
      Instruction &Inst = *II;
      if (!isa<CallInst>(&Inst))
        continue;
      CallInst *CI = cast<CallInst>(&Inst);
      Value *CalledValue = CI->getCalledValue();
      Function *Callee = cast<Function>(CalledValue->stripPointerCasts());
      if (!ClonedFunctionList.count(Callee))
        continue;
      Function *NewCallee = CloningMap[Callee];
      LLVM_DEBUG(dbgs() << "    Inst Before ..." << Inst << " in "
                        << NewF->getName() << "\n");

      Constant *BitcastNew = NewCallee;
      if (!isa<Function>(CalledValue)) {
        BitcastNew =
            ConstantExpr::getBitCast(NewCallee, CalledValue->getType());
      }
      CI->replaceUsesOfWith(CalledValue, BitcastNew);

      LLVM_DEBUG(dbgs() << "    Inst After ..." << Inst << "\n");
    }
  }
  // Specialize calls in "main".
  for (auto *CI : SpecializedCallsInMain) {
    Function *F = CI->getCalledFunction();
    Function *NewF = CloningMap[F];
    SpecializeCallInst(CI, F, NewF);
  }
  return true;
}

// This routine handles basic IR transformations required to
// access fields of shrunken structs in routines related to new layout.
// GEP/ByteGEP/Load/Store/SDiv/UDiv/MemFunc instructions related to
// shrunken structs are transformed.
void DynCloneImpl::transformIR(void) {

  auto IsMultiElemLdSt = [&](Instruction *I) {
    if (DTInfo.isMultiElemLoadStore(I))
      return true;
    return false;
  };

  auto IsGEP = [&](GetElementPtrInst *GEP) {
    if (GEP->getNumIndices() > 2)
      return false;

    Type *ElementTy = GEP->getSourceElementType();
    return isCandidateStruct(ElementTy);
  };

  auto IsByteGEP = [&](GetElementPtrInst *GEP) {
    if (GEP->getNumIndices() != 1)
      return false;
    auto BPair = DTInfo.getByteFlattenedGEPElement(GEP);
    if (!BPair.first)
      return false;
    return isCandidateStruct(BPair.first);
  };

  auto IsBinOp = [&](BinaryOperator *BinOp) {
    if (BinOp->getOpcode() != Instruction::Sub)
      return false;

    Type *SubTy = DTInfo.getResolvedPtrSubType(BinOp);
    if (!SubTy)
      return false;

    return isCandidateStruct(SubTy);
  };

  auto IsLoad = [&](LoadInst *LI) {
    auto LdElem = DTInfo.getLoadElement(LI);
    if (!LdElem.first || !isShrunkenField(LdElem))
      return false;
    return true;
  };

  auto IsStore = [&](StoreInst *SI) {
    auto StElem = DTInfo.getStoreElement(SI);
    if (!StElem.first || !isShrunkenField(StElem))
      return false;
    return true;
  };

  // The basic idea is to use shrunken struct for all address computations
  // instead of original struct.
  // Note that index of a field in shrunken structure may not be the same
  // as index of the field in original structure since fields will be
  // reordered in shrunken structure after size of some fields are
  // reduced.
  //
  // Before:
  //  %50 = getelementptr %struct.ar, %struct.ar* %48, i64 %49
  //
  // After:
  //  %50 = bitcast %struct.ar* %48 to %__DYN_struct.ar*
  //  %51 = getelementptr %__DYN_struct.ar, %__DYN_struct.ar* %50, i64 %49
  //  %52 = bitcast %__DYN_struct.ar* %51 to %struct.ar*
  //
  //
  // Before:
  //  %37 = getelementptr %struct.ar, %struct.ar* %31, i64 0, i32 0
  //
  // After:
  //  %37 = bitcast %struct.ar* %31 to %__DYN_struct.ar*
  //  %38 = getelementptr %__DYN_struct.ar, %__DYN_struct.ar* %37, i64 0, i32 1
  //  %39 = bitcast i32* %38 to i64*
  //
  auto ProcessGEP = [&](GetElementPtrInst *GEP) {
    unsigned NumIndices = GEP->getNumIndices();
    assert(NumIndices < 3 && "Unexpected indices for GEP");

    LLVM_DEBUG(dbgs() << "GEP Before:" << *GEP << "\n");
    StructType *OldStTy = cast<StructType>(GEP->getSourceElementType());
    Type *NewStTy = TransformedTypeMap[OldStTy];
    assert(NewStTy && "Expected transformed type");
    Type *DstTy = NewStTy->getPointerTo();
    Value *Src = GEP->getPointerOperand();
    CastInst *BC = CastInst::CreateBitOrPointerCast(Src, DstTy, "", GEP);

    SmallVector<Value *, 2> Indices;
    Indices.push_back(GEP->getOperand(1));
    if (NumIndices == 2) {
      Value *Op = GEP->getOperand(2);
      auto *FIdx = cast<ConstantInt>(Op);
      unsigned NewIdx = TransformedIndexes[OldStTy][FIdx->getLimitedValue()];
      Value *NewOp = ConstantInt::get(Op->getType(), NewIdx);
      Indices.push_back(NewOp);
    }
    GetElementPtrInst *NGEP =
        GetElementPtrInst::Create(NewStTy, BC, Indices, "", GEP);
    NGEP->setIsInBounds(GEP->isInBounds());
    Instruction *Res = NGEP;
    if (NGEP->getType() != GEP->getType())
      Res = CastInst::CreateBitOrPointerCast(Res, GEP->getType(), "", GEP);

    GEP->replaceAllUsesWith(Res);
    Res->takeName(GEP);

    LLVM_DEBUG(dbgs() << "GEP After:" << *BC << "\n" << *NGEP << "\n");
    if (Res != NGEP)
      LLVM_DEBUG(dbgs() << *Res << "\n");
  };

  // Fix offset in ByteGEP. No need to change anything else since
  // return type of GEP is i8*.
  //
  // Before:
  //     %F = getelementptr i8, i8* %bp2, i64 40
  //
  // After:
  //     %F = getelementptr i8, i8* %bp2, i64 24
  //
  auto ProcessByteGEP = [&](GetElementPtrInst *GEP) {
    LLVM_DEBUG(dbgs() << "ByteGEP Before:" << *GEP << "\n");
    auto GEPInfo = DTInfo.getByteFlattenedGEPElement(GEP);

    StructType *OldTy = cast<StructType>(GEPInfo.first);
    unsigned NewIdx = TransformedIndexes[OldTy][GEPInfo.second];
    auto *SL = DL.getStructLayout(TransformedTypeMap[OldTy]);
    uint64_t NewOffset = SL->getElementOffset(NewIdx);
    GEP->setOperand(1,
                    ConstantInt::get(GEP->getOperand(1)->getType(), NewOffset));
    LLVM_DEBUG(dbgs() << "ByteGEP After:" << GEP << "\n");
  };

  // Fix size operand in pointers subtract.
  //
  //   Before:
  //       %47 = sdiv exact i64 %46, 40
  //
  //   After:
  //       %47 = sdiv exact i64 %46, 26
  //
  auto ProcessBinOp = [&](BinaryOperator *BinOp) {
    LLVM_DEBUG({
      dbgs() << "SDiv/UDiv  Before: \n";
      for (auto *U : BinOp->users()) {
        dbgs() << "    " << *cast<Instruction>(U) << "\n";
      }
    });

    Type *SubTy = DTInfo.getResolvedPtrSubType(BinOp);
    Type *NewTy = TransformedTypeMap[cast<StructType>(SubTy)];
    updatePtrSubDivUserSizeOperand(BinOp, SubTy, NewTy, DL);

    LLVM_DEBUG({
      dbgs() << "SDiv/UDiv  After: \n";
      for (auto *U : BinOp->users()) {
        dbgs() << "    " << *cast<Instruction>(U) << "\n";
      }
    });
  };

  // This is invoked for only LoadInsts of shrunken fields.
  //   Before:
  //       %40 = load i64, i64* %39, align 8
  //
  //   After (for AOSToSOA field):
  //       %40 = bitcast i32* %39 to i16*
  //       %41 = load i16, i16* %40, align 2
  //       %42 = sext i16 %41 to i32
  //   After (for Candidate field which needs decoding):
  //       %40 = bitcast i64* %39 to i16*
  //       %41 = load i16, i16* %40, align 2
  //       %42 = call __DYN_decoder(i16 %40)
  //
  auto ProcessLoad = [&](LoadInst *LI, DynField &LdElem, bool NeedsDecoding) {
    LLVM_DEBUG(dbgs() << "Load before convert: " << *LI << "\n");
    AAMDNodes AATags;
    LI->getAAMetadata(AATags);

    StructType *OldTy = cast<StructType>(LdElem.first);
    StructType *NewSt = TransformedTypeMap[OldTy];
    unsigned NewIdx = TransformedIndexes[OldTy][LdElem.second];
    Value *SrcOp = LI->getPointerOperand();
    Type *NewTy = NewSt->getElementType(NewIdx);
    Type *PNewTy = NewTy->getPointerTo();
    Value *NewSrcOp = CastInst::CreateBitOrPointerCast(SrcOp, PNewTy, "", LI);
    Instruction *NewLI =
        new LoadInst(NewSrcOp, "", LI->isVolatile(),
                     MaybeAlign(DL.getABITypeAlignment(NewTy)),
                     LI->getOrdering(), LI->getSyncScopeID(), LI);
    // ZExt is used for AOSToSOA index field to avoid unnecessary "mov"
    // instructions (in generated code) since AOSTOSOA transformation
    // uses ZExt for the AOSToSOA index.
    Value *Res = nullptr;
    // (Reencoding) if encoding is not needed, then all values should fit
    // into shrunken bits.
    if (isAOSTOSOAIndexField(LdElem))
      Res = CastInst::CreateZExtOrBitCast(NewLI, LI->getType(), "", LI);
    else if (NeedsDecoding) {
      Res = CallInst::Create(DynFieldDecodeFunc, NewLI, "", LI);
      DEBUG_WITH_TYPE(
          REENCODING,
          dbgs() << "   (Reencoding) Insert a call to decoder function\n");
    } else
      Res = CastInst::CreateSExtOrBitCast(NewLI, LI->getType(), "", LI);

    LI->replaceAllUsesWith(Res);
    Res->takeName(LI);

    if (AATags)
      NewLI->setAAMetadata(AATags);

    LLVM_DEBUG(dbgs() << "Load after convert: " << *NewSrcOp << "\n"
                      << *NewLI << "\n"
                      << *Res << "\n");
  };

  // This is invoked for only StoreInsts of shrunken fields.
  //   Before:
  //       store i64 30, i64* %266, align 8
  //
  //   After (for AOSToSOA fields):
  //       %268 = bitcast i32* %266 to i16*
  //       %267 = trunc i32 30 to i16
  //       store i16 %267, i16* %268, align 2
  //   After (for Candidate fields which needs encoding):
  //       %268 = call __DYN_encoder(i64 30)
  //       %267 = bitcast i64* %266 to i16*
  //       store i16 %268, i16* %267, align 2
  //
  auto ProcessStore = [&](StoreInst *SI, DynField &StElem, bool NeedsEncoding) {
    LLVM_DEBUG(dbgs() << "Store before convert: " << *SI << "\n");
    AAMDNodes AATags;
    SI->getAAMetadata(AATags);

    StructType *OldTy = cast<StructType>(StElem.first);
    StructType *NewSt = TransformedTypeMap[OldTy];
    unsigned NewIdx = TransformedIndexes[OldTy][StElem.second];
    Type *NewTy = NewSt->getElementType(NewIdx);
    Type *PNewTy = NewTy->getPointerTo();
    Value *ValOp = SI->getValueOperand();
    Value *NewVal = nullptr;
    // (Reencoding) if encoding is not needed, then all values should fit
    // into shrunken bits.
    if (NeedsEncoding) {
      NewVal = CallInst::Create(DynFieldEncodeFunc, ValOp, "", SI);
      DEBUG_WITH_TYPE(
          REENCODING,
          dbgs() << "   (Reencoding) Insert a call to encoder function\n");
    } else
      NewVal = CastInst::CreateTruncOrBitCast(ValOp, NewTy, "", SI);

    Value *SrcOp = SI->getPointerOperand();
    Value *NewSrcOp = CastInst::CreateBitOrPointerCast(SrcOp, PNewTy, "", SI);
    Instruction *NewSI =
        new StoreInst(NewVal, NewSrcOp, SI->isVolatile(),
                      MaybeAlign(DL.getABITypeAlignment(NewTy)),
                      SI->getOrdering(), SI->getSyncScopeID(), SI);

    if (AATags)
      NewSI->setAAMetadata(AATags);

    LLVM_DEBUG(dbgs() << "Store after convert: \n"
                      << *NewVal << "\n"
                      << *NewSrcOp << "\n"
                      << *NewSI << "\n");
  };

  // Fix size in MemFuncs like memcpy/realloc etc
  //
  //  Before:
  //      @llvm.memcpy.p0i8.p0i8.i64(i8* %361, i8* %86, i64 40, i1 false)
  //
  //  After:
  //      @llvm.memcpy.p0i8.p0i8.i64(i8* %361, i8* %86, i64 26, i1 false)
  //
  auto ProcessMemFunc = [&](std::pair<CallInfo *, Type *> &MPair) {
    CallInfo *CInfo = MPair.first;
    StructType *OrigTy = cast<StructType>(MPair.second);
    StructType *NewTy = TransformedTypeMap[OrigTy];
    Instruction *I = CInfo->getInstruction();
    LLVM_DEBUG(dbgs() << "MemFunc before convert: " << *I << "\n");
    updateCallSizeOperand(I, CInfo, OrigTy, NewTy, GetTLI(*I->getFunction()));
    LLVM_DEBUG(dbgs() << "MemFunc after convert: " << *I << "\n");
  };

  SmallVector<Instruction *, 4> MultiElemLdStToProcess;
  SmallVector<GetElementPtrInst *, 16> GEPsToProcess;
  SmallVector<BinaryOperator *, 16> BinOpsToProcess;
  DenseMap<LoadInst *, bool> LoadsToProcess;
  DenseMap<StoreInst *, bool> StoresToProcess;
  SmallVector<GetElementPtrInst *, 16> ByteGEPsToProcess;
  SmallVector<std::pair<CallInfo *, Type *>, 4> CallsToProcess;
  SmallVector<Instruction *, 32> InstsToRemove;

  // Cloned routines are used for original layout shrunken struct. That means,
  // there will be no changes to cloned routine. Original routines will be
  // modified to use shrunken layout. Control walks through all original
  // routines, which are cloned, and fixes all accesses to shrunken structs.
  for (auto &CPair : CloningMap) {

    Function *F = CPair.first;
    MultiElemLdStToProcess.clear();
    GEPsToProcess.clear();
    BinOpsToProcess.clear();
    LoadsToProcess.clear();
    StoresToProcess.clear();
    ByteGEPsToProcess.clear();
    CallsToProcess.clear();
    InstsToRemove.clear();

    LLVM_DEBUG(dbgs() << "\n Transforming IR for : " << F->getName() << "\n");

    // Collect all instructions that need to be fixed.
    for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It) {
      Instruction *I = &*It;
      if (IsMultiElemLdSt(I)) {
        MultiElemLdStToProcess.push_back(I);
      } else if (auto *GEP = dyn_cast<GetElementPtrInst>(I)) {
        if (IsGEP(GEP))
          GEPsToProcess.push_back(GEP);
        else if (IsByteGEP(GEP))
          ByteGEPsToProcess.push_back(GEP);
      } else if (auto *BinOp = dyn_cast<BinaryOperator>(I)) {
        if (IsBinOp(BinOp))
          BinOpsToProcess.push_back(BinOp);
      } else if (auto *LI = dyn_cast<LoadInst>(I)) {
        if (IsLoad(LI))
          LoadsToProcess.insert({LI, (DynFieldDecodeFunc != nullptr)});
      } else if (auto *SI = dyn_cast<StoreInst>(I)) {
        if (IsStore(SI))
          StoresToProcess.insert({SI, (DynFieldEncodeFunc != nullptr)});
      } else if (isa<CallInst>(I)) {
        auto *CInfo = DTInfo.getCallInfo(I);
        Type *ElemTy = getCallInfoElemTy(CInfo);
        if (!ElemTy)
          continue;
        if (isCandidateStruct(ElemTy))
          CallsToProcess.push_back(std::make_pair(CInfo, ElemTy));
      }
    }

    // (Reencoding) For each load, check if all its uses land in the store to
    // candidate fields. If so skip enoding/decoding for such load and stores.
    for (auto &LP : LoadsToProcess) {
      // LP.second is false if the load is an AOSToSOA index field load or
      // decoder function is null. In either case encoding is not needed.
      if (LP.second) {
        auto *LI = LP.first;
        auto ItEnd = StoresToProcess.end();
        SmallPtrSet<StoreInst *, 4> StoresWithNoEncoding;
        // Dummy check: if load has no uses, we still need to decode loaded
        // result.
        bool NeedsEncoding = LI->getNumUses() ? false : true;
        for (auto &U : LI->uses()) {
          // If there is at least one use that is not a store to candidate
          // field, then we need encoding/decoding for the load and all its
          // uses.
          if (StoreInst *SI = dyn_cast<StoreInst>(U.getUser())) {
            auto It = StoresToProcess.find(SI);
            if (It == ItEnd) {
              NeedsEncoding = true;
              break;
            } else if (It->second) {
              StoresWithNoEncoding.insert(SI);
            }
          } else {
            NeedsEncoding = true;
            break;
          }
        }
        if (!NeedsEncoding) {
          // Otherwise - mark the load and all corresponding stores as not a
          // candidate for encoding.
          LP.second = false;
          for (auto SI : StoresWithNoEncoding)
            StoresToProcess[SI] = false;
        }
      }
    }

    for (auto &II : MultiElemLdStToProcess) {
      auto It = MultiElemLdStAOSTOSOAIndexMap.find(II);
      assert(It != MultiElemLdStAOSTOSOAIndexMap.end() &&
             " Expected MultiElem instruction in map");

      LLVM_DEBUG(dbgs() << " Processing MultiElem Load/Store:  " << *II
                        << "\n");
      // Since all fields accessed by MultiElem Ld/St will be reduced to the
      // same type, it is okay to use any accessing field for processing Ld/St.
      if (auto *LI = dyn_cast<LoadInst>(II))
        ProcessLoad(LI, It->second, false /*no reencoding*/);
      else if (auto *SI = dyn_cast<StoreInst>(II))
        ProcessStore(SI, It->second, false /*no reencoding*/);
      else
        llvm_unreachable("Unexpected multi field load/store!");
      InstsToRemove.push_back(II);
    }

    for (auto *GEP : GEPsToProcess) {
      ProcessGEP(GEP);
      InstsToRemove.push_back(cast<Instruction>(GEP));
    }

    for (auto *GEP : ByteGEPsToProcess)
      ProcessByteGEP(GEP);

    for (auto *BinOp : BinOpsToProcess)
      ProcessBinOp(BinOp);

    for (auto &LP : LoadsToProcess) {
      auto LdElem = DTInfo.getLoadElement(LP.first);
      auto NeedsDecoding = isAOSTOSOAIndexField(LdElem) ? false : LP.second;
      ProcessLoad(LP.first, LdElem, NeedsDecoding);
      InstsToRemove.push_back(cast<Instruction>(LP.first));
    }

    for (auto &SP : StoresToProcess) {
      auto StElem = DTInfo.getStoreElement(SP.first);
      auto NeedsEncoding = isAOSTOSOAIndexField(StElem) ? false : SP.second;
      ProcessStore(SP.first, StElem, NeedsEncoding);
      InstsToRemove.push_back(cast<Instruction>(SP.first));
    }

    for (auto &CallPair : CallsToProcess)
      ProcessMemFunc(CallPair);

    for (auto *DI : InstsToRemove)
      DI->eraseFromParent();
  }
}

// Create encoder and decoder functions that will translate the original
// 64bit value into/from a smaller ShrunkenIntTy value.
void DynCloneImpl::createEncodeDecodeFunctions(void) {
  if (AllDynFieldConstSet.size() == 0) {
    DynFieldEncodeFunc = nullptr;
    DynFieldDecodeFunc = nullptr;
    return;
  }

  llvm::Type *Int64Ty = Type::getIntNTy(M.getContext(), 64);
  FunctionType *EncoderFT = FunctionType::get(ShrunkenIntTy, {Int64Ty}, false);
  FunctionType *DecoderFT = FunctionType::get(Int64Ty, {ShrunkenIntTy}, false);
  DynFieldEncodeFunc = Function::Create(EncoderFT, GlobalValue::InternalLinkage,
                                        "__DYN_encoder", &M);
  fillupCoderRoutine(DynFieldEncodeFunc, true /*encoder*/);
  DynFieldDecodeFunc = Function::Create(DecoderFT, GlobalValue::InternalLinkage,
                                        "__DYN_decoder", &M);
  fillupCoderRoutine(DynFieldDecodeFunc, false /*decoder*/);
}

// Generate all instructions for encoder/decoder functions.
// i16 __DYN_encoder(i64 arg) {
//   i16 RetVal, Max = max_int_15bit;
//   switch(arg) {
//     case C1: RetVal = Max;
//     case C2: RetVal = Max+1;
//     case C3: RetVal = Max+2;
//     ...
//     default: RetVal = Trunc(arg);
//     };
//   return RetVal;
// }
//
// i64 __DYN_decoder(i16 arg) {
//   i16 Max = max_int_15bit;
//   i64 RetVal;
//   switch(arg) {
//     case Max: RetVal = C1;
//     case Max+1: RetVal = C2;
//     case Max+2: RetVal = C3;
//     ...
//     default: RetVal = SExt(arg);
//     };
//   return RetVal;
// }
//
void DynCloneImpl::fillupCoderRoutine(Function *F, bool IsEncoder) {
  // Indicate this function doesn't use vectors. This prevents the inliner from
  // deleting it from the caller when merging attributes.
  F->addFnAttr("min-legal-vector-width", "0");

  llvm::IntegerType *SrcType = cast<IntegerType>(F->arg_begin()->getType());
  llvm::IntegerType *DstType = cast<IntegerType>(F->getReturnType());
  BasicBlock *BB = BasicBlock::Create(M.getContext(), "entry", F);
  IRBuilder<> IRB(BB);
  BasicBlock *DefaultBB = BasicBlock::Create(M.getContext(), "default", F);
  SwitchInst *SI =
      IRB.CreateSwitch(F->arg_begin(), DefaultBB, AllDynFieldConstSet.size());
  BasicBlock *ReturnBB = BasicBlock::Create(M.getContext(), "return", F);
  IRBuilder<> IRBDefault(DefaultBB);
  Value *Trunc = IRBDefault.CreateSExtOrTrunc(F->arg_begin(), DstType);
  IRBDefault.CreateBr(ReturnBB);
  IRBuilder<> IRBReturn(ReturnBB);
  PHINode *Phi = IRBReturn.CreatePHI(DstType, 0, "phival");
  IRBReturn.CreateRet(Phi);
  Phi->addIncoming(Trunc, DefaultBB);
  int64_t CurrEncodeValue = getMaxShrIntTyValueWithDelta() + 1;
  for (int64_t ConstValue : AllDynFieldConstSet) {
    ConstantInt *CaseValue;
    ConstantInt *CaseReturnValue;
    if (IsEncoder) {
      CaseValue = ConstantInt::get(SrcType, ConstValue);
      CaseReturnValue = ConstantInt::get(DstType, CurrEncodeValue++);
    } else {
      CaseValue = ConstantInt::get(SrcType, CurrEncodeValue++);
      CaseReturnValue = ConstantInt::get(DstType, ConstValue);
    }
    BasicBlock *CaseBB = BasicBlock::Create(M.getContext(), "case", F);
    Phi->addIncoming(CaseReturnValue, CaseBB);
    IRBuilder<> IRBCase(CaseBB);
    IRBCase.CreateBr(ReturnBB);
    SI->addCase(CaseValue, CaseBB);
  }
}

bool DynCloneImpl::run(void) {

  LLVM_DEBUG(dbgs() << "DynCloning Transformation \n");

  if (!gatherPossibleCandidateFields())
    return false;

  LLVM_DEBUG(dbgs() << "    Possible Candidate fields: \n";
             printCandidateFields(dbgs()));

  if (!prunePossibleCandidateFields())
    return false;

  if (!verifyMultiFieldLoadStores()) {
    LLVM_DEBUG(
        dbgs() << "Verification of Multi field Load/Stores Failed... \n");
    return false;
  }

  if (!verifyLegalityChecksForInitRoutine())
    return false;

  if (!trackPointersOfAllocCalls()) {
    LLVM_DEBUG(dbgs() << "Track uses of AllocCalls Failed... \n");
    return false;
  }

  if (!verifyCallsInInitRoutine()) {
    LLVM_DEBUG(dbgs() << " Calls in InitRoutine Failed... \n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "    Verified InitRoutine ... \n");

  createShrunkenTypes();

  createEncodeDecodeFunctions();

  transformInitRoutine();

  LLVM_DEBUG(dbgs() << "    Generated Runtime checks in InitRoutine ... \n");

  LLVM_DEBUG(dbgs() << "  Final Candidate fields: \n";
             printCandidateFields(dbgs()));

  if (!createCallGraphClone())
    return false;

  transformIR();

  // Disable inlining InitRoutine even though it is okay to inline. Usually,
  // it will be inlined due to single call-site heuristic. But, we know it
  // is not hot function and a lot of new code is added to this routine
  // by DynClone. Disabling inline for InitRouine may help other hot functions
  // to be inlined.
  LLVM_DEBUG(dbgs() << "    Disable inlining for InitRoutine: "
                    << InitRoutine->getName() << "\n");
  CallInst *CI = cast<CallInst>(InitRoutine->user_back());
  CI->setIsNoInline();

  return true;
}

bool DynClonePass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                           DynGetTLITy GetTLI, WholeProgramInfo &WPInfo,
                           LoopInfoFuncType &GetLI) {

  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  auto &DL = M.getDataLayout();

  DynCloneImpl DynCloneI(M, DL, DTInfo, GetLI, GetTLI);
  return DynCloneI.run();
}

PreservedAnalyses DynClonePass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  LoopInfoFuncType GetLI = [&FAM](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };
  auto GetTLI = [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };
  if (!runImpl(M, DTransInfo, GetTLI, WPInfo, GetLI))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // namespace dtrans

} // namespace llvm

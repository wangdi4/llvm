//===---------------- Intel_PaddedPtrPropagation.cpp ----------------------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file provides an implementation of the propagation of the padded pointer
// property across program call graph and an interface to mark a Value as
// padded and an interface to query to check if a Value points to padded memory.
//
// Inputs:
// LLVM IR with placed llvm.ptr.annotations marking pointers that have been
// identified by a previous pass as pointers to padded memory. Annotations have
// to be in the form:
// p = llvm.ptr.annotation.tySpec(p, "padded %d bytes", "src_file", src_line).
//
// Annotations are preferably placed by calling the interface function
// void insertPaddedMarkUp(Value *V, int Padding), however they can be placed
// in any way. Although an annotation can be placed for any pointer type,
// currently only pointers to integer and floating point types are supported in
// the propagation algorithm.
//
// The propagation algorithm is implemented in the PaddedPtrPropImpl class with
// the member function transform. PaddedPtrPropImpl::transform does the
// propagation of padded property of pointers within each function and across
// the call graph. When the propagation is completed, parameters, function
// returns and function callsites are marked as padded by the amount of bytes
// evaluated by the propagation algorithm. This is done by placing the
// corresponding annotations before parameter use, before returns and after
// function calls before function result use. Those described above are only
// marked if they meet these conditions:
// 1. It is a pointer of supported type
// 2. It has propagated padding >= 0
// 3. It is not directly used by a sole user which is a padding annotation
//    Example:
//    x1 = padded(x0, 4)
//    x2 = padded(x1, 2)
//    padding for x1 is redundant and is not going to be inserted in case it is
//    followed by another padding annotation
// The propagation algorithm introduces the special padding value -1 which means
// "unknown" padding. It is OK to use negative padding in the propagation
// itself, but it should not be generated in output LLVM IR.
//
// Outputs:
// The LLVR IR with llvm.ptr.annotations which appeared in the IR before this
// pass is run, and the annotations produced by propagation algorithm that are
// described above.
//
// How to query for the padding of a Value:
// After propagation and placing annotations each function has enough
// information to calculate padding for each Value inside the function.
// The padding for a specific value can be queried by calling
// int getPaddingForValue(Value *V). Function getPaddingForValue traverses up
// through def-use chains and calculates padding information for a particular
// Value. In the case where the padding is unknown, 0 is returned as a
// conservative answer.
//
// Further optimizations which use the information in padded memory annotations:
// It is a responsibility of an optimization to mark pointers properly with
// llvm.ptr.annotate intrinsics in case it makes previously set
// annotations invalid. In particular it makes sense when speculative
// optimizations use padding information w/o readjustment of padding data after
// transformation. A good example (in pseudocode) of such a case is:
//
// p = padded(p, 1)
// if (cond)
//    x = *p
//
// It is valid to do a speculative load since padding guarantees that p has at
// least  1 byte of padding, so after the speculative load transformation the
// code could look as follows:
//
// p = padded(p, 1)
// t = *p
// if (cond)
//    x = t
//
// Subsequent speculative load transformations can then incorrectly assume that
// they can again speculatively load from p as follows:
//
// p = padded(p, 1)
// t = *p
// t1 = *(p + 1)
// if (cond)
//    x = t
//
// In order to avoid this, after the first iteration of a speculative load
// transformation, padding information for p should be changed to the following:
//
// p = padded(p, 0)
// t = *p
// if (cond)
//    x = t
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransPaddedMalloc.h"
#include "Intel_DTrans/Transforms/PaddedPointerPropagation.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Type.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace std;

#define DEBUG_TYPE "padded-pointer-prop"

namespace {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Prints function padded pointer information
static cl::opt<bool> PaddedPtrInfoDump("padded-pointer-info", cl::ReallyHidden);
#else
constexpr bool PaddedPtrInfoDump = false;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Helper function returning an annotation string in the form "padded %d bytes"
string getPaddedAnnotationStr(int PaddedBytes) {
  return ("padded " + Twine(PaddedBytes) + " bytes").str();
}

// Helper function checking if a string is in a form "padded %d bytes".
// Return values:
// 	True if it is a padding annotation string, in which case the Padding
//      parameter is set to the actual padding value.
//      False is returned otherwise, and Padding is set to 0
bool parsePaddedAnnotationStr(StringRef S, int &Padding) {
  Padding = 0;
  return S.consume_front("padded ") && !S.consumeInteger(0, Padding) &&
         S.consume_front(" bytes") && S.size() == 0;
}

// Function checking if a Value is a padding annotation
// Return values:
// 	True if it is a value returned by annotation intrinsic and the
// 	annotation string is in the correct form. The Padding parameter is set
// 	to the actual padding value.
//      False is returned otherwise, and Padding is set to 0
bool isPaddedMarkUpAnnotation(Value *V, int &Padding) {
  IntrinsicInst *I = dyn_cast<IntrinsicInst>(V);
  if (!I)
    return false;

  if (I->getIntrinsicID() != Intrinsic::ptr_annotation)
    return false;

  if (auto *O = dyn_cast<ConstantExpr>(I->getArgOperand(1))) {
    if (auto *G = dyn_cast<GlobalVariable>(O->getOperand(0))) {
      if (auto *CA = dyn_cast<ConstantDataArray>(G->getInitializer())) {
        auto S = CA->getAsCString();
        return parsePaddedAnnotationStr(S, Padding);
      }
    }
  }
  return false;
}

// Helper function returning true if a type referred to by T is a pointer to an
// integer or floating-point type.
bool isSupportedPointerType(Type *T) {
  auto *PT = dyn_cast<PointerType>(T);
  if (!PT)
    return false;

  auto *ET = PT->getElementType();
  return ET->isIntegerTy() || ET->isFloatingPointTy();
}

// Set of functions merging padded Values passed as function inputs
// Return values:
// 1. Minimal padding among arguments
// 2. -1 if padding can't be evaluated this time
// Since algorithm takes minimal padding among arguments, it operates only on
// nodes having copy semantics i.e. return, phi, select, etc.
template <typename IType, typename PGetter>
int getMergedValue(IType B, IType E, PGetter &PG) {
  int T = INT_MAX;
  for (auto I = B; I != E; I++) {
    auto X = PG(*I);
    T = (T <= X) ? T : X;
    if (T <= 0)
      return T;
  }
  assert(T != INT_MAX && "Unexpected padding value after merge");
  return T;
}

template <typename RangeTy, typename PGetter>
int getMergedValue(RangeTy &&Range, PGetter &PG) {
  return getMergedValue(begin(Range), end(Range), PG);
}

template <typename PGetter>
int getMergedValue(initializer_list<Value *> &&IList, PGetter &PG) {
  return getMergedValue(begin(IList), end(IList), PG);
}

// A structure to keep padding data related to a function
struct FuncPadInfo final {
  const Function *Func;
  SmallDenseMap<Value *, int> ValuePaddingMap;
  SmallDenseSet<Argument *> PotentiallyPaddedParams;
  SmallDenseSet<Value *> ReturnedValues;

  int ReturnPadding = -1;

  FuncPadInfo(Function *F) {
    Func = F;
    initializeParams(F);
    initializeRets(F);

    if (hasUnknownCallSites()) {
      for (auto *Arg : PotentiallyPaddedParams) {
        ValuePaddingMap[Arg] = 0;
      }
    }
  }

  void initializeParams(Function *F) {
    for (auto &A : F->args()) {
      if (!isSupportedPointerType(A.getType()))
        continue;
      PotentiallyPaddedParams.insert(&A);
    }
  }

  void initializeRets(Function *F) {
    if (!isSupportedPointerType(F->getReturnType()))
      return;

    for (auto &BB : *F) {
      for (auto &I : BB) {
        if (auto Ret = dyn_cast<ReturnInst>(&I)) {
          auto V = Ret->getReturnValue();
          if (V != nullptr)
            ReturnedValues.insert(V);
        }
      }
    }
  }

  bool hasUnknownCallSites() const {
    return !Func->hasLocalLinkage() || Func->hasAddressTaken();
  }

  bool hasPotentiallyPaddedReturn() const { return !ReturnedValues.empty(); }

  bool hasPotentiallyPaddedParams() const {
    return !PotentiallyPaddedParams.empty();
  }

  int getReturnPadding() { return ReturnPadding; }

  void setReturnPadding(int RP) { ReturnPadding = RP; }

  SmallDenseSet<Value *> &getReturnedValues() { return ReturnedValues; }

  SmallDenseSet<Argument *> &getPotentiallyPaddedParams() {
    return PotentiallyPaddedParams;
  }

  int getPaddingForValue(const Value *V) const {
    auto IT = ValuePaddingMap.find(V);
    if (IT == ValuePaddingMap.end())
      return -1;
    return IT->second;
  }

  void setPaddingForValue(Value *V, int Padding) {
    ValuePaddingMap[V] = Padding;
    LLVM_DEBUG({
      dbgs() << "Set padding " << Padding << " for value: ";
      V->print(dbgs());
      dbgs() << "\n";
    });
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const {
    OS << "Function info(" << Func->getName() << "):\n";
    OS << "  HasUnknownCallSites: " << hasUnknownCallSites() << "\n";

    if (hasPotentiallyPaddedReturn()) {
      OS << "  Return Padding: " << ReturnPadding << "\n";
    }

    if (hasPotentiallyPaddedParams()) {
      OS << "  Arguments' Padding:\n";
      for (auto const &A : PotentiallyPaddedParams) {
        OS << "    ";
        A->print(OS);
        OS << " : " << getPaddingForValue(A) << "\n";
      }
    }

    OS << "  Value paddings:\n";
    for (auto const &E : ValuePaddingMap) {
      OS << "    ";
      E.first->print(OS);
      OS << " :: " << E.second << "\n";
    }
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
};

// A functor to calculate the padding of a Value by traversing def-use chains.
// It keeps Values already evaluated for two purposes.
// 1. To cache evaluated padding info for subsequent queries
// 2. To track loops. If a loop has been detected, the stop the traversal.
struct InFunctionPaddingResolver {
  SmallDenseMap<Value *, int> VPMap;
  SmallDenseSet<Value *> LoopTraceSet;

  int operator()(Value *V) {
    if (!isSupportedPointerType(V->getType()))
      return 0;

    auto Inst = dyn_cast<Instruction>(V);
    if (!Inst)
      return 0;

    if (isInLoopTrace(V))
      return 0;

    int Padding;
    if (hasStoredPadding(V, Padding))
      return Padding;

    if (isPaddedMarkUpAnnotation(V, Padding))
      return Padding;

    pushValueToLoopTrace(V);

    switch (Inst->getOpcode()) {
    case Instruction::BitCast: {
      auto BC = cast<BitCastInst>(Inst);
      Padding = (*this)(BC->getOperand(0));
      break;
    }

    case Instruction::GetElementPtr: {
      auto GEP = cast<GetElementPtrInst>(Inst);
      Padding = (*this)(GEP->getPointerOperand());
      break;
    }

    case Instruction::Select: {
      auto SEL = cast<SelectInst>(Inst);
      Padding =
          getMergedValue({SEL->getTrueValue(), SEL->getFalseValue()}, *this);
      break;
    }

    case Instruction::PHI: {
      auto PHI = cast<PHINode>(Inst);
      Padding = getMergedValue(PHI->operands(), *this);
      break;
    }

    default:
      Padding = 0;
    }

    VPMap.insert({V, Padding});
    popValueFromLoopTrace(V);
    return Padding;
  }

  bool isInLoopTrace(Value *V) {
    return LoopTraceSet.find(V) != LoopTraceSet.end();
  }

  bool hasStoredPadding(Value *V, int &Padding) {
    Padding = 0;
    auto II = VPMap.find(V);
    if (II == VPMap.end())
      return false;

    Padding = II->second;
    return true;
  }

  void pushValueToLoopTrace(Value *V) { LoopTraceSet.insert(V); }

  void popValueFromLoopTrace(Value *V) { LoopTraceSet.erase(V); }
};

// A class implementing <struct type - field index> set.
// The set is used to track such pairs for the purpose of initial placement of
// padded annotations. The initial padding annotations are placed based on
// Dtrans single alloc analysis. The placement implemented in 2 phases. The
// first is the collection of the structure fields identified as initialized by
// a single memory allocation routine in to the set. The second is LLVM IR
// traversal which analyzes load instructions and if the load loads from a
// structure field contained in the set then a padding annotation is inserted.
class StructFieldTracker {
  using IndexSetTy = SmallDenseSet<unsigned, 8>;
  using FieldMapTy = SmallDenseMap<StructType *, IndexSetTy *>;

  FieldMapTy FieldMap;

public:
  void insert(StructType *Ty, unsigned index) {
    auto I = FieldMap.find(Ty);
    if (I == FieldMap.end())
      (FieldMap[Ty] = new IndexSetTy())->insert(index);
    else
      FieldMap[Ty]->insert(index);
  }

  bool contains(StructType *Ty, size_t index) {
    auto MapItor = FieldMap.find(Ty);
    if (MapItor == FieldMap.end())
      return false;

    auto SetItor = MapItor->second->find(index);
    if (SetItor == MapItor->second->end())
      return false;

    return true;
  }

  bool empty() { return FieldMap.empty(); }

  ~StructFieldTracker() {
    for (auto II : FieldMap) {
      delete II.second;
    }
  }
};

//===----------------------------------------------------------------------===//
/// Transformation class
//===----------------------------------------------------------------------===//

class PaddedPtrPropImpl {
  using FuncPadInfoMapTy = SmallDenseMap<Function *, FuncPadInfo *>;

  Module &M;
  DTransAnalysisInfo &DTInfo;
  FuncPadInfoMapTy FuncPadInfoMap;
  dtrans::PaddedMallocGlobals PaddedMallocData;
  FuncPadInfo &getFuncPadInfo(Function *F);
  void propagateInFunction(Function *F, SmallDenseSet<Function *> &ImpactedFns);
  bool emit();

  void collectSingleAllocsForType(dtrans::TypeInfo *TyInfo,
                                  StructFieldTracker &Fields);
  bool placeInitialAnnotations();
  bool processGepForInitialAllocations(GEPOperator *GEP,
                                       StructFieldTracker &FieldMap);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(const string &msg, raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

public:
  PaddedPtrPropImpl(Module &M, DTransAnalysisInfo &DTInfo)
      : M(M), DTInfo(DTInfo) {}

  ~PaddedPtrPropImpl() {
    for (auto Entry : FuncPadInfoMap) {
      delete Entry.second;
    }
  }

  bool transform(WholeProgramInfo &WPInfo);
};

// Helper function inserting padding annotation for a Value at the location
// pointed by IRBuilder
void insertPaddedMarkUpInt(IRBuilder<> &Builder, Value *V, int Padding) {
  auto *PType = dyn_cast<PointerType>(V->getType());
  assert(isSupportedPointerType(PType) && "Unsupported type");

  Module *M = Builder.GetInsertBlock()->getParent()->getParent();
  LLVMContext &Ctx = Builder.getContext();
  Type *I32Ty = Type::getInt32Ty(Ctx);

  std::string MarkupStr(getPaddedAnnotationStr(Padding));

  Value *MarkupStrPtr = Builder.CreateGlobalStringPtr(MarkupStr);
  Value *File = Builder.CreateGlobalStringPtr(M->getSourceFileName());
  Constant *LNum = Constant::getIntegerValue(I32Ty, APInt(32, 0, false));

  auto *F = Intrinsic::getDeclaration(M, Intrinsic::ptr_annotation, PType);
  assert(F && "Can't find appropriate ptr_annotation intrinsic");
  auto *A = Builder.CreateCall(F, {V, MarkupStrPtr, File, LNum}, V->getName());
  V->replaceAllUsesWith(A);
  cast<IntrinsicInst>(A)->setArgOperand(0, V);
}

// Padding getter functor for getting padding for the Value from FuncPadInfo
// It is used by getMergedValue for accessing padding data for merging.
struct FPInfoPaddingGetter {
  FuncPadInfo &FPInfo;
  FPInfoPaddingGetter(FuncPadInfo &FPInfo) : FPInfo(FPInfo) {}
  int operator()(Value *V) { return FPInfo.getPaddingForValue(V); }
};

// Propagate padding information inside a function based on annotations inserted
// in the IR by previous passes. The known padding information is propagated to
// function parameters and the padding information of returns of called
// functions
void PaddedPtrPropImpl::propagateInFunction(
    Function *F, SmallDenseSet<Function *> &ImpactedFns) {
  auto &FPInfo = getFuncPadInfo(F);

  // Evaluate padding for function argument if it isn't set yet
  for (auto P : FPInfo.getPotentiallyPaddedParams()) {
    auto Padding = FPInfo.getPaddingForValue(P);
    if (Padding >= 0)
      continue; // Is already set. Skip it.

    // Take minimal padding among all call sites
    int ResPadding = -1;
    int UseCnt = 0;
    for (auto *CI : F->users()) {
      auto *Call = cast<CallBase>(CI);
      auto Caller = Call->getParent()->getParent();
      auto &CFPInfo = getFuncPadInfo(Caller);
      auto *CSArg = Call->getArgOperand(P->getArgNo());
      int PaddingAtCallSite = CFPInfo.getPaddingForValue(CSArg);

      // Initialize the ResultPadding by the padding value from the first call
      // site or get the minimal padding value among the current known minimum
      // and the one obtained from the call site if the first call site is not
      // processed.
      if (UseCnt++ == 0 || ResPadding > PaddingAtCallSite)
        ResPadding = PaddingAtCallSite;
      if (ResPadding <= 0)
        break; // Stop iterating since padding is either 0 - minimal allowed,
               // or -1 - unknown
    }

    if (ResPadding < 0)
      continue; // Unknown padding, skip it.

    // Store freshly calculated padding for particular function parameter
    FPInfo.setPaddingForValue(P, ResPadding);
  }

  // Build initial workset from the consumers of values having known padding
  // If the user has padding already assigned to it, skip it.
  SmallDenseSet<Value *> WorkSet;
  for (auto PMEntry : FPInfo.ValuePaddingMap) {
    assert(PMEntry.second >= 0 && "Negative padding value is not allowed");
    for (auto *U : PMEntry.first->users()) {
      // User already has padding assigned to it
      if (FPInfo.getPaddingForValue(U) >= 0)
        continue;
      WorkSet.insert(U);
    }
  }

  FPInfoPaddingGetter InFunctionPGetter(FPInfo);

  while (!WorkSet.empty()) {
    auto II = WorkSet.begin();
    auto Inst = cast<Instruction>(*II);
    WorkSet.erase(II);

    if (FPInfo.getPaddingForValue(Inst) >= 0)
      continue;

    int Padding = -1;

    switch (Inst->getOpcode()) {
    case Instruction::BitCast: {
      auto BC = cast<BitCastInst>(Inst);

      if (!isSupportedPointerType(BC->getType()))
        continue;

      Padding = FPInfo.getPaddingForValue(BC->getOperand(0));
      break;
    }

    case Instruction::GetElementPtr: {
      auto GEP = cast<GetElementPtrInst>(Inst);

      if (!isSupportedPointerType(GEP->getType()))
        continue;

      Padding = FPInfo.getPaddingForValue(GEP->getPointerOperand());
      break;
    }

    case Instruction::Select: {
      auto SEL = cast<SelectInst>(Inst);

      if (!isSupportedPointerType(SEL->getType()))
        continue;

      Padding = getMergedValue({SEL->getTrueValue(), SEL->getFalseValue()},
                               InFunctionPGetter);

      break;
    }

    case Instruction::PHI: {
      auto PHI = cast<PHINode>(Inst);
      Padding = getMergedValue(PHI->operands(), InFunctionPGetter);
      break;
    }

    case Instruction::Call:
    case Instruction::Invoke: {
      // The propagation reached a call site, that means that the callee
      // has at least one potentially padded argument.
      // Thus the callee must be added to the impacted functions set.
      auto *Call = cast<CallBase>(Inst);
      auto *Callee = Call->getCalledFunction();
      if (!Callee)
        continue;

      ImpactedFns.insert(Callee);
      auto &CFPInfo = getFuncPadInfo(Callee);

      // If the callee has no potentially padded return there is nothing to
      // propagate further in caller
      if (!CFPInfo.hasPotentiallyPaddedReturn())
        continue;

      Padding = CFPInfo.getReturnPadding();
      break;
    }

    case Instruction::Ret: {
      if (FPInfo.ReturnPadding >= 0)
        continue;

      auto RetValues = FPInfo.getReturnedValues();
      FPInfoPaddingGetter CalledFunctionPGetter(FPInfo);
      int P = getMergedValue(RetValues, CalledFunctionPGetter);
      if (P >= 0) {
        FPInfo.ReturnPadding = P;
        // Return padding of the function has been propagated, this requires
        // adding of all the callers to the impacted function set.
        // In addition, set padding for Values returned by this function.
        for (auto *U : F->users()) {
          auto *Call = dyn_cast<CallBase>(U);
          if (!Call)
            continue;

          auto Caller = Call->getParent()->getParent();
          ImpactedFns.insert(Caller);
          auto &CPInfo = getFuncPadInfo(Caller);
          CPInfo.setPaddingForValue(Call, P);
        }
      }
      continue;
    }

    default:
      continue;
    }

    if (Padding < 0)
      continue;

    FPInfo.setPaddingForValue(Inst, Padding);

    for (auto U : Inst->users())
      WorkSet.insert(U);
  }
}

//===----------------------------------------------------------------------===//
///                        PaddedPtrPropImpl implementation
//===----------------------------------------------------------------------===//

FuncPadInfo &PaddedPtrPropImpl::getFuncPadInfo(Function *F) {
  auto I = FuncPadInfoMap.find(F);
  if (I == FuncPadInfoMap.end()) {
    return *(FuncPadInfoMap[F] = new FuncPadInfo(F));
  }
  return *I->second;
}

// Emission of padded annotations for parameters, returns and after function
// calls
bool PaddedPtrPropImpl::emit() {
  bool Modified = false;

  for (auto &FIMEntry : FuncPadInfoMap) {
    auto &FPInfo = *(FIMEntry.second);
    for (auto PVMEntry : FPInfo.ValuePaddingMap) {
      Value *V = PVMEntry.first;
      int Padding = PVMEntry.second;
      if (Padding <= 0)
        continue;

      int UPD;

      if (isa<Argument>(V)) {
        // Do not insert padding for the case when argument padding is
        // overwritten by followed padding annotation.
        if (V->hasOneUse() &&
            isPaddedMarkUpAnnotation(*V->users().begin(), UPD))
          continue;
        insertPaddedMarkUp(V, Padding);
        Modified = true;
        continue;
      }

      if (isa<CallInst>(V) || isa<InvokeInst>(V)) {
        // If it is a padded annotation, do not insert an additional one.
        if (isPaddedMarkUpAnnotation(V, UPD))
          continue;

        // Do not insert padding for the case when returned padding is
        // overwritten by a following padding annotation.
        if (V->hasOneUse() &&
            isPaddedMarkUpAnnotation(*V->users().begin(), UPD))
          continue;

        insertPaddedMarkUp(V, Padding);
        Modified = true;
        continue;
      }
    }

    // Insert padding annotations for returned values.
    for (auto *RetV : FPInfo.getReturnedValues()) {
      int UPD;
      // Do not insert an annotation before ReturnInst if the annotation
      // is already the immediate predecessor of ReturnInst.
      if (isPaddedMarkUpAnnotation(RetV, UPD))
        continue;

      // Do not insert a redundant annotation for the returned values.
      // Let's consider the case when the initial IR looks like the following:
      //
      // p0 = call foo() ; foo returns a padded pointer
      // ret p0
      //
      // Then the ReturnedValues set will contain p0.
      //
      // Since foo() returns a padded pointer, propagation will insert an
      // annotation for foo()'s result in the following manner:
      //
      // p0 = call foo() ; foo returns a padded pointer
      // p1 = padded(p0, 4)
      // ret p1
      //
      // The insertion of annotations for the returned values will attempt to
      // insert the same annotation for p0. This annotation is redundant since
      // one was already inserted.

      if (RetV->hasOneUse() &&
          isPaddedMarkUpAnnotation(*RetV->users().begin(), UPD))
        continue;

      // Do not insert annotations if the padding is unknown or zero.
      int Padding = FPInfo.getPaddingForValue(RetV);
      if (Padding <= 0)
        continue;

      insertPaddedMarkUp(RetV, Padding);
      Modified = true;
    }
  }

  return Modified;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void PaddedPtrPropImpl::dump(const string &Header, raw_ostream &OS) const {
  if (!PaddedPtrInfoDump)
    return;

  SmallVector<const FuncPadInfo *, 32> SortedV;
  for (auto const &Ntr : FuncPadInfoMap) {
    SortedV.append(1, Ntr.second);
  }

  std::sort(SortedV.begin(), SortedV.end(),
            [](const FuncPadInfo *x, const FuncPadInfo *y) {
              return x->Func->getName().compare(y->Func->getName()) < 0;
            });

  OS << "==== " << Header << " ====\n\n";
  for (auto const *FPInfo : SortedV) {
    FPInfo->dump(OS);
    OS << "\n";
  }
  OS << "==== END OF " << Header << " ====\n\n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Gather single allocations assigned to a structure field.
// The function recursively traverses TypeInfo and collects the fields in
// the Fields structure.
void PaddedPtrPropImpl::collectSingleAllocsForType(dtrans::TypeInfo *TyInfo,
                                                   StructFieldTracker &Fields) {
  auto StInfo = dyn_cast<dtrans::StructInfo>(TyInfo);

  // Skip if it isn't a structure
  if (!StInfo)
    return;

  size_t FldId = 0;

  for (auto &FldInfo : StInfo->getFields()) {
    if (isSupportedPointerType(FldInfo.getLLVMType()) &&
        FldInfo.isSingleAllocFunction())
      Fields.insert(cast<StructType>(StInfo->getLLVMType()), FldId);
    else {
      auto *ComponentTI = DTInfo.getTypeInfo(FldInfo.getLLVMType());
      collectSingleAllocsForType(ComponentTI, Fields);
    }
    FldId++;
  }
}

// The function processes a GetElementPointerInst and returns true if it
// corresponds to a structure field which is initialized by a single allocation
// call.
bool PaddedPtrPropImpl::processGepForInitialAllocations(
    GEPOperator *GEP, StructFieldTracker &FieldMap) {

  auto StructField = DTInfo.getStructField(GEP);
  if (!StructField.first)
    return false;

  return FieldMap.contains(StructField.first, StructField.second);
}

// The function places an initial padding annotation based on DTrans single
// alloc analysis results.
bool PaddedPtrPropImpl::placeInitialAnnotations() {
  StructFieldTracker FieldMap;
  for (auto TyInfo : DTInfo.type_info_entries()) {
    collectSingleAllocsForType(TyInfo, FieldMap);
  }

  // There is no a candidate found for the padding annotation
  if (FieldMap.empty())
    return false;

  bool Modified = false;
  for (auto &F : M) {
    for (auto &BB : F) {
      for (auto &I : BB) {
        auto Load = dyn_cast<LoadInst>(&I);
        // Process only loads of the supported types.
        if (!Load)
          continue;

        if (!isSupportedPointerType(Load->getType()))
          continue;

        // Insert the annotations for the loads next to the GEPs referencing the
        // structure fields initialized by a single allocation
        auto *GEP = dyn_cast<GEPOperator>(Load->getPointerOperand());
        if (GEP && processGepForInitialAllocations(GEP, FieldMap)) {
          insertPaddedMarkUp(Load, PaddedMallocData.getPaddedMallocSize(M));
          Modified = true;
        }
      }
    }
  }
  return Modified;
}

// The main routine performing the transformation consisting of the following
// steps
// 1. Initialization of internal data structures
// 2. Building the work list
// 3. Iteration over the work list and propagation of padding info for each
//    function
// 4. In case the propagation succeeded for a function, callers and callees of
//    the function are added to work list under a set of conditions. The
//    processed function is removed from the work list.
// 5. Emission of padded annotations for parameters, returns and after function
//    calls.

bool PaddedPtrPropImpl::transform(WholeProgramInfo &WPInfo) {

  if (!WPInfo.isWholeProgramSafe())
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  PaddedMallocData.buildGlobalsInfo(M);

  LLVM_DEBUG(dbgs() << "\n---- PADDED MALLOC TRANSFORM START ----\n\n");

  bool Modified = placeInitialAnnotations();

  SmallDenseSet<Function *> WorkSet;

  // Find all the functions which have pointer annotations inside and
  // include them into initial work set

  Function *Annotations[] = {
      Intrinsic::getDeclaration(&M, Intrinsic::ptr_annotation,
                                Type::getInt8PtrTy(M.getContext())),
      Intrinsic::getDeclaration(&M, Intrinsic::ptr_annotation,
                                Type::getInt16PtrTy(M.getContext())),
      Intrinsic::getDeclaration(&M, Intrinsic::ptr_annotation,
                                Type::getInt32PtrTy(M.getContext())),
      Intrinsic::getDeclaration(&M, Intrinsic::ptr_annotation,
                                Type::getInt64PtrTy(M.getContext())),
      Intrinsic::getDeclaration(&M, Intrinsic::ptr_annotation,
                                Type::getFloatPtrTy(M.getContext())),
      Intrinsic::getDeclaration(&M, Intrinsic::ptr_annotation,
                                Type::getDoublePtrTy(M.getContext()))};

  for (auto AFunc : Annotations) {
    for (auto U : AFunc->users()) {
      CallInst *Call = cast<CallInst>(U);
      Function *Caller = Call->getParent()->getParent();

      int Padding;
      if (!isPaddedMarkUpAnnotation(U, Padding))
        continue;

      auto &FPInfo = getFuncPadInfo(Caller);
      FPInfo.setPaddingForValue(U, Padding);
      WorkSet.insert(Caller);
    }
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  dump("INITIAL FUNCTION SET", dbgs());
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  while (!WorkSet.empty()) {
    auto FIter = WorkSet.begin();
    auto F = *FIter;
    WorkSet.erase(FIter);

    SmallDenseSet<Function *> Impacted;
    propagateInFunction(F, Impacted);
    WorkSet.insert(Impacted.begin(), Impacted.end());
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  dump("TRANSFORMED FUNCTION SET", dbgs());
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  Modified = emit() || Modified;

  if (!Modified)
    PaddedMallocData.destroyGlobalsInfo(M);

  return Modified;
}

//===----------------------------------------------------------------------===//
///                 Wrapper pass for legacy pass manager
//===----------------------------------------------------------------------===//

struct PaddedPtrPropWrapper final : ModulePass {
  static char ID;
  PaddedPtrPropWrapper();
  bool runOnModule(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

char PaddedPtrPropWrapper::ID = 0;

PaddedPtrPropWrapper::PaddedPtrPropWrapper() : ModulePass(ID) {
  initializePaddedPtrPropWrapperPass(*PassRegistry::getPassRegistry());
}

void PaddedPtrPropWrapper::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<DTransAnalysisWrapper>();
  AU.addPreserved<DTransAnalysisWrapper>();
  AU.addRequired<WholeProgramWrapperPass>();
  AU.addPreserved<WholeProgramWrapperPass>();
}

// Potentially the pass can change the CFG by inserting a new BB after
// InvokeInst. In that case, it preserves none of its analyses.
bool PaddedPtrPropWrapper::runOnModule(Module &M) {
  auto &DTInfo = getAnalysis<DTransAnalysisWrapper>().getDTransInfo(M);
  WholeProgramInfo &WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
  bool Modified = PaddedPtrPropImpl(M, DTInfo).transform(WPInfo);
  return Modified;
}

} // namespace

namespace llvm {

//===----------------------------------------------------------------------===//
/// Interface function to query if value is pointer to padded memory
//===----------------------------------------------------------------------===//

int getPaddingForValue(Value *V) {
  InFunctionPaddingResolver R;
  return R(V);
}

void insertPaddedMarkUp(Value *V, int Padding) {
  if (auto *A = dyn_cast<Argument>(V)) {
    BasicBlock *BB = &A->getParent()->getEntryBlock();
    IRBuilder<> B(BB, BB->getFirstInsertionPt());
    insertPaddedMarkUpInt(B, V, Padding);
  } else if (auto PN = dyn_cast<PHINode>(V)) {
    BasicBlock *BB = PN->getParent();
    IRBuilder<> B(BB, BB->getFirstInsertionPt());
    insertPaddedMarkUpInt(B, V, Padding);
  } else if (auto IVK = dyn_cast<InvokeInst>(V)) {
    auto *IvkBBlock = IVK->getParent();
    auto *NormalDest = IVK->getNormalDest();
    auto *Split = SplitEdge(IvkBBlock, NormalDest);
    IRBuilder<> B(Split, Split->getFirstInsertionPt());
    insertPaddedMarkUpInt(B, V, Padding);
  } else if (auto *I = dyn_cast<Instruction>(V)) {
    BasicBlock::iterator IT(I);
    ++IT;
    IRBuilder<> B(I->getParent(), IT);
    insertPaddedMarkUpInt(B, V, Padding);
  } else
    llvm_unreachable(
        "An attempt to insert padding markup on unsupported value type");
}

void removePaddedMarkUp(IntrinsicInst *I) {
  LLVM_DEBUG({
    int Padding;
    assert(isPaddedMarkUpAnnotation(I, Padding) && "Not padding annotation");
  });
  I->replaceAllUsesWith(I->getArgOperand(0));
  I->eraseFromParent();
}

ModulePass *createPaddedPtrPropWrapperPass() {
  return new PaddedPtrPropWrapper();
}

//===----------------------------------------------------------------------===//
///                  Pass run implementation for new pass manager
//===----------------------------------------------------------------------===//

// Potentially the pass can change CFG by inserting new BB after InvokeInst.
// In that case, it preserves none of its analyses.
PreservedAnalyses PaddedPtrPropPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &DTInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  bool Modified = PaddedPtrPropImpl(M, DTInfo).transform(WPInfo);
  if (Modified) {
    PreservedAnalyses PA;
    PA.preserve<DTransAnalysis>();
    PA.preserve<WholeProgramAnalysis>();
    return PA;
  } else
    return PreservedAnalyses::all();
}

} // namespace llvm

//===----------------------------------------------------------------------===//
///                     Wrapper pass initialization
//===----------------------------------------------------------------------===//

INITIALIZE_PASS_BEGIN(PaddedPtrPropWrapper, DEBUG_TYPE,
                      "Optimize data layout with malloc padding", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(PaddedPtrPropWrapper, DEBUG_TYPE,
                    "Optimize data layout with malloc padding", false, false)

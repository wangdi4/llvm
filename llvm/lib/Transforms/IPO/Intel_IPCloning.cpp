//===------- Intel_IPCloning.cpp - IP Cloning -*------===//
//
// Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file does perform IP Cloning.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_IPCloning.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_AggInline.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_IPCloningAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Intel_CloneUtils.h"
#include <sstream>
#include <string>
using namespace llvm;
using namespace llvm::llvm_cloning_analysis;

#define DEBUG_TYPE "ipcloning"

STATISTIC(NumIPCloned, "Number of functions IPCloned");
STATISTIC(NumIPCallsCloned, "Number of calls to IPCloned functions");

// FuncPtrsClone & SpecializationClone runs before Inlining. GenericClone
// runs after Inlining.
namespace {
enum IPCloneKind {
  NoneClone = 0,
  FuncPtrsClone = 1,
  SpecializationClone = 2,
  GenericClone = 3,
  RecProgressionClone = 4
};
}

// Option to enable AfterInl IP Cloning, which is disabled by default.
// This option mainly for LIT tests to test AfterInl cloning
// without LTO.
static cl::opt<bool> IPCloningAfterInl("ip-cloning-after-inl",
                                   cl::init(false), cl::ReallyHidden);

// Maximum number of clones allowed for any routine.
static cl::opt<unsigned> IPFunctionCloningLimit("ip-function-cloning-limit",
                                   cl::init(3), cl::ReallyHidden);

// Enable Specialization cloning.
static cl::opt<bool> IPSpecializationCloning("ip-specialization-cloning",
                                   cl::init(true), cl::ReallyHidden);

// Maximum size of array allowed as constant argument for specialization clone.
static cl::opt<unsigned> IPSpecCloningArrayLimit("ip-spe-cloning-array-limit",
                                   cl::init(80), cl::ReallyHidden);

// Maximum number of specialization clones allowed at any CallSite.
static cl::opt<unsigned> IPSpeCloningCallLimit(
        "ip-spe-cloning-call-limit", cl::init(4), cl::ReallyHidden);

// Maximum number of CallSites allowed for specialization for any routine.
static cl::opt<unsigned> IPSpeCloningNumCallSitesLimit(
        "ip-spe-cloning-num-callsites-limit", cl::init(7), cl::ReallyHidden);

// Minimum allowed number of argument sets at any Callsite for specialization
// cloning.
static cl::opt<unsigned> IPSpeCloningMinArgSetsLimit(
          "ip-spe-cloning-min-argsets-limit", cl::init(1), cl::ReallyHidden);

// Used to force the enabling of the if-switch heuristics even when they
// would not normally be enabled.
static cl::opt<bool> ForceIFSwitchHeuristic(
          "ip-gen-cloning-force-if-switch-heuristic", cl::init(false),
          cl::ReallyHidden);

// This switch will be enabled (and removed) once the associated loop opt work
// for CMPLRLLVM-8680 is complete.
static cl::opt<bool> EnableMorphologyCloning(
          "ip-gen-cloning-enable-morphology", cl::init(false),
          cl::ReallyHidden);

// Do not qualify a routine for cloning under the "if" heuristic unless we
// see at least this many "if" values that will be made constant.
static cl::opt<unsigned> IPGenCloningMinIFCount(
          "ip-gen-cloning-min-if-count", cl::init(6), cl::ReallyHidden);

// Do not qualify a routine for cloning under the "switch" heuristic unless we
// see at least this many "switch" values that will be made constant.
static cl::opt<unsigned> IPGenCloningMinSwitchCount(
          "ip-gen-cloning-min-switch-count", cl::init(6), cl::ReallyHidden);

// Do not specially qualify a recursive routine for generic cloning unless we
// see at least this many formals that qualify under the "if-switch" heuristic.
static cl::opt<unsigned> IPGenCloningMinRecFormalCount(
          "ip-gen-cloning-min-rec-formal-count", cl::init(2), cl::ReallyHidden);

// Do not specially qualify a recursive routine for generic cloning unless we
// see at least this many callsites to the routine.
static cl::opt<unsigned> IPGenCloningMinRecCallsites(
          "ip-gen-cloning-min-rec-callsites", cl::init(10), cl::ReallyHidden);

// It is a mapping between formals of current function that is being processed
// for cloning and set of possible constant values that can reach from
// call-sites to the formals.
SmallDenseMap<Value *, std::set<Constant *>> FormalConstantValues;

// It is a mapping between actuals of a Callsite that is being processed
// for cloning and set of possible constant values that can reach from
// call-sites to the actuals.
SmallDenseMap<Value *, std::set<Constant *>> ActualConstantValues;

// List of inexact formals for the current function that is being processed
// for cloning. Inexact means that at least one non-constant will reach
// from call-sites to formal.
SmallPtrSet<Value *, 16> InexactFormals;

// Mapping between CallInst and corresponding constant argument set.
DenseMap<CallInst *, unsigned> CallInstArgumentSetIndexMap;

// All constant argument sets for a function that is currently being
// processed. Each constant argument set is mapped with unique index value.
SmallDenseMap<unsigned,
    std::vector<std::pair<unsigned, Constant*>>> FunctionAllArgumentsSets;

// Mapping between newly cloned function and constant argument set index.
SmallDenseMap<unsigned, Function *> ArgSetIndexClonedFunctionMap;

// List of call-sites that need to be processed for cloning
std::vector<CallInst *> CurrCallList;

// List of all cloned functions
std::set<Function *> ClonedFunctionList;

// List of formals of the current function as worthy candidates
// for cloning. These are selected after applying heuristics.
SmallPtrSet<Value *, 16> WorthyFormalsForCloning;

// It is mapping of Callsites of a routine that is currently being processed
// and all possible argument sets at each CallSite.
SmallDenseMap<CallInst *, std::vector<
    std::vector<std::pair<unsigned, Value*>>>> AllCallsArgumentsSets;

// InexactArgsSets means not all possible arguments sets are found at CallSites.
// List of CallSites with InexactArgsSets for a routine that is currently
// being processed.
SmallPtrSet<CallInst *, 8> InexactArgsSetsCallList;

// It is mapping between Special Constants (i.e address of stack location)
// and corresponding Values that need to be propagated to cloned
// function. It basically helps to avoid  processing of IR to find
// propagated values during transformation.
SmallDenseMap<Value *, Value *> SpecialConstPropagatedValueMap;

// It is mapping between Special Constants (i.e address of stack location)
// and GEP Instruction that is used to compute address of arrays. It
// basically helps to get NumIndices during transformation.
SmallDenseMap<Value *,  GetElementPtrInst*> SpecialConstGEPMap;

// It is mapping between Function and LoopInfo. It is used
// to avoid recomputing LoopInfo for a function each time
// when a CallSite of the function is analyzed.
SmallDenseMap<Function *,  LoopInfo*> FunctionLoopInfoMap;

// Returns true if 'Arg' is considered as constant for
// cloning based on FuncPtrsClone.
static bool isConstantArgWorthyForFuncPtrsClone(Value *Arg) {
  Value* FnArg = Arg->stripPointerCasts();
  Function *Fn = dyn_cast<Function>(FnArg);

  if (Fn == nullptr)
    return false;
  // if it is function address, consider only if it has local definition.
  if (Fn->isDeclaration() || Fn->isIntrinsic()
      || !Fn->hasExactDefinition() || !Fn->hasLocalLinkage() ||
      Fn->hasExternalLinkage()) {
    return false;
  }
  return true;
}

// Returns true if 'Arg' is considered as constant for
// cloning based on GenericClone.
static bool isConstantArgWorthyForGenericClone(Value *Arg) {
  Value* FnArg = Arg->stripPointerCasts();
  Function *Fn = dyn_cast<Function>(FnArg);

  // Returns false if it is address of a function
  if (Fn != nullptr)
    return false;

  // For now, allow only INT constants. Later, we may allow
  // isa<ConstantPointerNull>(FnArg), isa<ConstantFP>(FnArg) etc.
  //
  if (!isa<ConstantInt>(FnArg))
    return false;
  return true;
}


// Return true if constant argument 'Arg' is worth considering for cloning
// based on 'CloneType'.
//
static bool isConstantArgWorthy(Value *Arg, IPCloneKind CloneType) {
  bool IsWorthy = false;

  if (CloneType == FuncPtrsClone) {
    IsWorthy = isConstantArgWorthyForFuncPtrsClone(Arg);
  } else if (CloneType == SpecializationClone) {
    IsWorthy = isConstantArgWorthyForSpecializationClone(Arg);
  } else if (CloneType == GenericClone) {
    IsWorthy = isConstantArgWorthyForGenericClone(Arg);
  }
  return IsWorthy;
}

// Return true if actual argument is considered for cloning
static bool isConstantArgForCloning(Value *Arg, IPCloneKind CloneType) {
  if (Constant *C = dyn_cast<Constant>(Arg)) {
    if (isa<UndefValue>(C))
      return false;

    if (isConstantArgWorthy(Arg, CloneType))
      return true;
  }
  return false;
}

// Collect constant value if 'ActualV' is constant actual argument
// and save it in constant list of 'FormalV'. Otherwise, mark
// 'FormalV' as inexact.
static void collectConstantArgument(Value* FormalV, Value* ActualV,
                                    IPCloneKind CloneType) {

  if (!isConstantArgForCloning(ActualV, CloneType)) {
    // Mark inexact formal
    if (!InexactFormals.count(FormalV))
      InexactFormals.insert(FormalV);

    return;
  }
  // Now, we know it is valid constant for cloning.
  Constant *C = cast<Constant>(ActualV);
  auto &ValList = FormalConstantValues[FormalV];

  if (!ValList.count(C))
    ValList.insert(C);

  auto &ActValList = ActualConstantValues[ActualV];

  if (!ActValList.count(C))
    ActValList.insert(C);
}

// Returns maximum possible number of clones based on constant-value-lists
// of formals
static unsigned getMaxClones() {
  unsigned prod = 1;
  unsigned count;
  for (auto I = FormalConstantValues.begin(), E = FormalConstantValues.end();
       I != E; ++I) {
    auto CList = I->second;
    count = CList.size();
    if (InexactFormals.count(I->first))
      count += 1;

    if (count == 0)
      count = 1;

    prod = prod * count;
  }
  return prod;
}

// Returns minimum number of clones needed based on constant-value-lists
// of formals
static unsigned getMinClones() {
  unsigned prod = 1;
  unsigned count;
  for (auto I = FormalConstantValues.begin(), E = FormalConstantValues.end();
       I != E; ++I) {
    auto CList = I->second;
    count = CList.size();
    if (InexactFormals.count(I->first))
      count += 1;

    if (prod < count)
      prod = count;
  }
  return prod;
}

// Sets 'SizeInBytes' to size of array of char and 'NumElems'
// to number of elements in array. 'DL' is used to get size of array.
//
static void GetPointerToArrayDims(Type* PTy, unsigned& SizeInBytes,
                             unsigned& NumElems, const DataLayout &DL) {

  if (!isPointerToCharArray(PTy)) return;
  auto ATy = cast<PointerType>(PTy)->getElementType();

  NumElems = cast<ArrayType>(ATy)->getNumElements();
  SizeInBytes = DL.getTypeSizeInBits(ATy);
}

// Return true if 'V' is address of packed array (i.e int64 value) on
// stack.
//
//  Ex:
//   AInst:        %6 = alloca i64, align 8
//
//   U:            %10 = bitcast i64* %6 to i8*
//
//   Callee:       call void @llvm.lifetime.start(i64 8, i8* %10) #9
//
//   StInst:       store i64 72340172821299457, i64* %6, align 8
//
//   V:            %41 = bitcast i64* %6 to [2 x i8]*
//
//   Callee:       call void @llvm.lifetime.end(i64 8, i8* %10) #9
//
//
static Value* isStartAddressOfPackedArrayOnStack(Value *V) {
  Instruction *I = cast<Instruction>(V);
  Value* AInst = I->getOperand(0);
  if (!isa<AllocaInst>(AInst))
    return nullptr;
  AllocaInst* AllocaI = cast<AllocaInst>(AInst);

  Value* StInst = nullptr;
  for (User *U : AInst->users()) {

    // Ignore if it is the arg that is passed to call.
    if (U == V) continue;

    if (isa<BitCastInst>(U)) {
      for (User *CI : U->users()) {
        IntrinsicInst *Callee = dyn_cast<IntrinsicInst>(CI);
        if (!Callee) return nullptr;
        if (Callee->getIntrinsicID() != Intrinsic::lifetime_start &&
            Callee->getIntrinsicID() != Intrinsic::lifetime_end)
          return nullptr;
      }
      continue;
    }

    if (!isa<StoreInst>(U)) return nullptr;

    // More than one use is noticed
    if (StInst != nullptr) return nullptr;
    StInst = U;
  }
  if (StInst == nullptr) return nullptr;

  Value* ValOp = cast<StoreInst>(StInst)->getValueOperand();
  if (!isa<Constant>(ValOp)) return nullptr;

  if (ValOp->getType() != AllocaI->getAllocatedType()) return nullptr;

  return StInst;
}

// Returns true if 'V' is a Global Variable candidate for specialization
// cloning. 'I' is used to get DataLayout to compute sizes of types.
//
static bool isSpecializationGVCandidate(Value* V, Instruction *I) {
  GlobalVariable* GV;
  GV = dyn_cast<GlobalVariable>(V);
  if (!GV) return false;

  if (!GV->isConstant()) return false;
  if (!GV->hasDefinitiveInitializer()) return false;
  Constant *Init = GV->getInitializer();
  if (!isa<ConstantArray>(Init)) return false;

  if (GV->getLinkage() != GlobalVariable::PrivateLinkage) return false;
  if (GV->hasComdat()) return false;
  if (GV->isThreadLocal()) return false;

  Type *Ty = GV->getValueType();
  if (!Ty->isSized()) return false;
  const DataLayout &DL = I->getModule()->getDataLayout();
  if (DL.getTypeSizeInBits(Ty) > IPSpecCloningArrayLimit) return false;

  // Add more checks like below but not required
  // if (GlobalWasGeneratedByCompiler(GV)) return false;
  // if (!Ty->isArrayTy() ||
  //     !Ty->getArrayElementType()->isArrayTy()) return false;
  return true;
}

// Return true if 'V' is address of stack location where Global array
// is copied completely.
//
// Ex:
//
//  AInst:     %7 = alloca [5 x [2 x i8]], align 1
//
//  MemCpySrc (AUse): %11 = getelementptr inbounds [5 x [2 x i8]],
//           [5 x [2 x i8]]* %7, i64 0, i64 0, i64 0
//
//  Callee:    call void @llvm.lifetime.start(i64 10, i8* %11) #9
//
//  User:      call void @llvm.memcpy.p0i8.p0i8.i64(i8* %11, i8*
//                 getelementptr inbounds ([5 x [2 x i8]],
//                 [5 x [2 x i8]]* @t.CM_THREE, i64 0, i64 0, i64 0), i64 1
//
//  V (GEP):   %43 = getelementptr inbounds [5 x [2 x i8]],
//                   [5 x [2 x i8]]* %7, i64 0, i64 0
//
//  Callee:    call void @llvm.lifetime.start(i64 10, i8* %11) #9
//
//  MemCpyDst: i8* getelementptr inbounds ([5 x [2 x i8]],
//                   [5 x [2 x i8]]* @t.CM_THREE
//
//  GlobAddr:     @t.CM_THREE
//
static Value* isStartAddressOfGLobalArrayCopyOnStack(Value *V) {
  GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(V);
  if (!GEP)
    return nullptr;
  // First, check it is starting array address on stack
  Value* AInst = GEP->getOperand(0);
  if (!isa<AllocaInst>(AInst))
    return nullptr;

  AllocaInst* AllocaI = cast<AllocaInst>(AInst);
  if (!GEP->hasAllZeroIndices())
    return nullptr;

  const Value* AUse = nullptr;
  Type * GEPType = GEP->getSourceElementType();
  if (GEPType != AllocaI->getAllocatedType())
    return nullptr;

  // Get another use of AllocaInst other than the one that
  // is passed to Call
  AUse = nullptr;
  for (const User *U : AInst->users()) {
    // Ignore if it is the arg that is passed to call.
    if (U == V)
      continue;
    // More than one use is noticed
    if (AUse != nullptr)
      return nullptr;
    AUse = U;
  }

  if (AUse == nullptr)
    return nullptr;
  const GetElementPtrInst *MemCpySrc = dyn_cast<GetElementPtrInst>(AUse);
  if (!MemCpySrc)
    return nullptr;
  if (!MemCpySrc->hasAllZeroIndices())
    return nullptr;
  if (GEPType != MemCpySrc->getSourceElementType())
    return nullptr;

  Value* GlobAddr = nullptr;
  for (const User *U : AUse->users()) {
    auto User = dyn_cast<CallInst>(U);
    if (!User)
      return nullptr;
    const IntrinsicInst *Callee = dyn_cast<IntrinsicInst>(U);
    if (!Callee) return nullptr;
    if (Callee->getIntrinsicID() == Intrinsic::lifetime_start ||
        Callee->getIntrinsicID() == Intrinsic::lifetime_end)
       continue;
    if (Callee->getIntrinsicID() != Intrinsic::memcpy)
       return nullptr;

    // Process Memcpy here
    if (User->getArgOperand(0) != AUse)
      return nullptr;
    Value* Dst = User->getArgOperand(1);
    auto *MemCpyDst = dyn_cast<GEPOperator>(Dst);
    if (!MemCpyDst)
      return nullptr;
    if (!MemCpyDst->hasAllZeroIndices())
      return nullptr;
    if (GEPType != MemCpyDst->getSourceElementType())
      return nullptr;
    if (MemCpyDst->getNumIndices() != MemCpySrc->getNumIndices())
      return nullptr;
    Value* MemCpySize = User->getArgOperand(2);

    // Make sure there is only one memcpy
    if (GlobAddr != nullptr) return nullptr;
    GlobAddr = MemCpyDst->getOperand(0);


    if (!isSpecializationGVCandidate(GlobAddr, GEP))
      return nullptr;

    const DataLayout &DL = GEP->getModule()->getDataLayout();
    unsigned ArraySize = DL.getTypeSizeInBits(GEPType) / 8;
    ConstantInt *CI = dyn_cast<ConstantInt>(MemCpySize);
    if (!CI) return nullptr;
    if (!CI->equalsInt(ArraySize)) return nullptr;
  }
  return GlobAddr;
}

// Returns true if 'V' is a special constant for specialization cloning.
// If 'V' special constant, it saves corresponding propagated value in
// 'SpecialConstPropagatedValueMap' to use it during transformation.
// For given 'Arg', which is PHINode, it gets one of the input GEP
// operands and save it in SpecialConstGEPMap to use it during
// transformation.
//
static bool isSpecializationCloningSpecialConst(Value* V, Value* Arg) {
  Value* PropVal = nullptr;

  if (isa<GetElementPtrInst>(V)) {
    PropVal = isStartAddressOfGLobalArrayCopyOnStack(V);
  }
  else if (isa<BitCastInst>(V)) {
    PropVal = isStartAddressOfPackedArrayOnStack(V);
  } else {
    return false;
  }
  if (PropVal == nullptr) return false;

  SpecialConstPropagatedValueMap[V] = PropVal;
  if (!SpecialConstGEPMap[V])
    SpecialConstGEPMap[V] = getAnyGEPAsIncomingValueForPhi(Arg);

  return true;
}

// Collect argument-sets at 'CI' of 'F' for arguments that are passes as PHI
// nodes in 'PhiValues' if possible. It saves argument-sets in
// "AllCallsArgumentsSets" map. 'CI' is added to "InexactArgsSetsCallList"
// if it is not possible to collect all possible argument-sets.
//
static void collectArgsSetsForSpecialization(Function &F, CallInst &CI,
                  SmallPtrSet<Value *, 8>& PhiValues) {

  std::vector<
      std::vector<std::pair<unsigned, Value*>>> CallArgumentsSets;
  std::vector<std::pair<unsigned, Value*>> ConstantArgs;
  CallArgumentsSets.clear();

  auto PHI_I = cast<Instruction>(*PhiValues.begin());
  // Skip CallSite if BasicBlock has too many preds.
  if (cast<PHINode>(PHI_I)->getNumIncomingValues() > IPSpeCloningCallLimit) {
    if (IPCloningTrace)
      errs() << "     More Preds ... Skipped Spe cloning  " << "\n";
    return;
  }

  // Collect argument sets for PHINodes in PhiValues that are passed
  // as arguments at CI.
  BasicBlock *BB = PHI_I->getParent();
  for (BasicBlock *PredBB : predecessors(BB)) {
    unsigned Position = 0;
    bool Inexact = false;
    ConstantArgs.clear();
    auto CAI1 = CI.arg_begin();
    for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end();
       AI != E; ++AI, ++CAI1, ++Position) {

      if (!PhiValues.count(*CAI1)) continue;

      auto PHI = cast<PHINode>(*CAI1);
      Value* C = PHI->getIncomingValueForBlock(PredBB);
      if (isa<Constant>(C) ||
          isSpecializationCloningSpecialConst(C, *CAI1)) {
         ConstantArgs.push_back(std::make_pair(Position, C));
      } else {
        Inexact = true;
        break;
      }
    }

    if (!Inexact) {
      // Eliminate duplicate argument sets here. Don't add ConstantArgs
      // if it is already in the set.
      bool Duplicate = false;
      for (unsigned I = 0; I < CallArgumentsSets.size(); I++) {
        if (CallArgumentsSets[I] == ConstantArgs) {
          Duplicate = true;
          break;
        }
      }
      if (!Duplicate)
        CallArgumentsSets.push_back(ConstantArgs);
    } else {
      if (!InexactArgsSetsCallList.count(&CI))
        InexactArgsSetsCallList.insert(&CI);
    }
  }

  // No need to check for Max limit on CallArgumentsSets.size() since
  // we had already checked on number of preds.

  // Check for minimum limit on size of Argument sets
  if (CallArgumentsSets.size() <= IPSpeCloningMinArgSetsLimit) {
    if (IPCloningTrace)
      errs() << "     Not enough sets... Skipped Spe cloning  " << "\n";
    return;
  }

  // Map CallArgumentsSets to CI here.
  auto &ACallArgs = AllCallsArgumentsSets[&CI];
  std::copy(CallArgumentsSets.begin(), CallArgumentsSets.end(),
                   std::back_inserter(ACallArgs));

  CurrCallList.push_back(&CI);

  // Dump arg sets
  if (IPCloningTrace) {
    errs() << "    Args sets collected \n";
    if (InexactArgsSetsCallList.count(&CI)) {
      errs() << "    Inexact args sets found \n";
    }
    for (unsigned index = 0; index < CallArgumentsSets.size(); index++) {
      errs() << "   Set_" << index << "\n";
      auto CArgs = CallArgumentsSets[index];
      for (auto I = CArgs.begin(), E = CArgs.end(); I != E; I++) {
        errs() <<  "      position: " << I->first << " Value " <<
                  *(I->second) << "\n";
      }
    }
  }
}

// Analyze CallInst 'CI' of 'F' and collect argument sets for
// specialization cloning if possible.
//
static bool analyzeCallForSpecialization(Function &F, CallInst &CI) {
  SmallPtrSet<Value *, 8> PhiValues;

  // Collect PHINodes that are passed as arguments for cloning
  // if possible.
  PhiValues.clear();
  if (!collectPHIsForSpecialization(F, CI, PhiValues))
    return false;

  // Using Loop based heuristics here and remove
  // PHI nodes from PhiValues if not useful in callee.
  // Reuse LoopInfo if it is already available.
  LoopInfo *LI = FunctionLoopInfoMap[&F];
  if (!LI) {
    LI = new LoopInfo(DominatorTree(const_cast<Function &>(F)));
    FunctionLoopInfoMap[&F] = LI;
  }
  if (!applyHeuristicsForSpecialization(F, CI, PhiValues, LI))
    return false;

  // Collect argument sets for specialization.
  collectArgsSetsForSpecialization(F, CI, PhiValues);
  return true;
}

// Analyze all CallSites of 'F' and collect CallSites and argument-sets
// for specialization cloning if possible.
//
static void analyzeCallSitesForSpecializationCloning(Function &F) {
  if (!IPSpecializationCloning) {
    if (IPCloningTrace)
      errs() << "   Specialization cloning disabled \n";
    return;
  }
  FunctionLoopInfoMap.clear();
  for (User *UR : F.users()) {

    if (!isa<CallInst>(UR))
      continue;

    auto CI = cast<CallInst>(UR);
    if (CI->getCalledFunction() != &F)
      continue;

    analyzeCallForSpecialization(F, *CI);
  }
  // All CallSites of 'F' are analyzed. Delete if
  // LoopInfo is computed.
  LoopInfo* LI = FunctionLoopInfoMap[&F];
  if (!LI)
    delete LI;
}

// Look at all CallSites of 'F' and collect all constant values
// of formals. Return true if use of 'F' is noticed as non-call.
static bool analyzeAllCallsOfFunction(Function &F, IPCloneKind CloneType) {
  bool FunctionAddressTaken = false;

  if (CloneType == SpecializationClone) {
    if (IPCloningTrace)
      errs() << " Processing for Spe cloning  " << F.getName() << "\n";
    analyzeCallSitesForSpecializationCloning(F);
    return false;
  }
  for (User *UR : F.users()) {
    // Ignore if use of function is not a call
    if (!isa<CallInst>(UR)) {
      FunctionAddressTaken = true;
      continue;
    }
    auto CI = cast<CallInst>(UR);
    Function *Callee = CI->getCalledFunction();
    if (Callee != &F) {
      FunctionAddressTaken = true;
      continue;
    }

    // Collect constant values for each formal
    CurrCallList.push_back(CI);
    auto CAI = CI->arg_begin();
    for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end();
         AI != E; ++AI, ++CAI) {
      collectConstantArgument(&*AI, *CAI, CloneType);
    }
  }
  return FunctionAddressTaken;
}

// Returns true if it a candidate for function-ptr cloning.
// Returns true if it has at least one formal of function pointer type.
//
static bool IsFunctionPtrCloneCandidate(Function &F) {
  for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end();
       AI != E; ++AI) {
    Type *T = (&*AI)->getType();
    if (isa<PointerType>(T) &&
        isa<FunctionType>(cast<PointerType>(T)->getElementType()))
      return true;
  }
  return false;
}

//
// Special recognition for IR arising from:
//   integer, parameter :: r=9
//   integer :: l(r), u(r), S(r,r), row
//   ...
//   subroutine mysub
//   where(S(row, :) /= 0)
//     l = S(row, :)
//     u = l
//   elsewhere
//     l = 1
//     u = r
//   end where
//
// This will be lowered from the Fortran front end into a series of 5 loops,
// equivalent to:
//   do i = 1, 9
//     t(i) = S(row,i) /= 0
//   end do
//   do i = 1, 9
//     if (.not. t(i)) then
//       l(i) = S(row, i)
//     end if
//   end do
//   do i = 1, 9
//     if (.not. t(i)) then
//       u(i) = l(i)
//     end if
//   end do
//   do i = 1, 9
//     if (t(i)) then
//       l(i) = 1
//     end if
//   end do
//   do i = 1, 9
//     if (t(i)) then
//       u(i) = 9
//     end if
//   end do
//
// The code below will recognize the form of these 5 loops in the IR, allowing
// us to conclude that at the end of this loop sequence, for each i, i = 1, 9,
// either:
//   (1) u(i) .eq. l(i)
// OR
//   (2) l(i) .eq. 1 .and. u(i) .eq. 9
//
// so that we can insert code equivalent to the following after the fifth loop:
//   if (l(8) .eq. u(8)) then
//     call mysubclone(row)
//     return
//   end if
// yielding:
//
//   subroutine mysub(row)
//   integer :: row
//   where(S(row, :) /= 0)
//     l = S(row, :)
//     u = l
//   elsewhere
//     l = 1
//     u = r
//   end where
//   if (l(8) .eq. u(8)) then
//     call mysubclone(row)
//     return
//   end if
//   ! After this point we know l(8) .eq. 1 .and. u(8) .eq. 9
//   ...
//   end subroutine
//
//   subroutine mysubclone(row)
//   integer :: row
//   ...
//   ! After this point we know l(8) .eq. u(8)
//   end subroutine
//
// In the case where l(8) and u(8) are the bounds for the innermost loop
// inside mysub() and mysubclone(), we can replace the bounds for that loop
// with 1, 9 in mysub() and l(8), u(8) in mysubclone().
//

// BEGIN: Special recognition code for extra clone transformation

//
// Return 'true' if 'V' represents an AllocaInst similar to:
//   %x = alloca [9 x i32], align 16
// Here the size of the array, the number of bits in the integer, and the
// alignment do not effect whether we return 'true'.  When we return 'true',
// we set '*ArrayLengthOut' to the number of elements in the array. (In the
// above example, that will be 9.)
//
static bool isRecProAllocaIntArray(Value *V,
                                   int *ArrayLengthOut) {
  auto AI = dyn_cast<AllocaInst>(V);
  if (!AI)
    return false;
  Type *PT = AI->getType();
  if (!PT->isPointerTy())
    return false;
  Type *T = PT->getPointerElementType();
  if (!T->isArrayTy() || !T->getArrayElementType()->isIntegerTy())
    return false;
  *ArrayLengthOut = T->getArrayNumElements();
  return true;
}

//
// Return 'true' if 'BBLatch' is the latch block for a loop with loop header
// 'BBLoopHeader'. If 'TestSimpleOnly', we test only for simple loops like:
//   do i = 1, 9
//     t(i) = S(row,i) /= 0
//   end do
// which have a single store to a single-dimensioned array. Otherwise, we
// also recognize "complex" loops of the form:
//   do i = 1, 9
//     if (.not. t(i)) then
//       l(i) = S(row, i)
//     end if
//   end do
// where the loop has 3 basic blocks: a test block, a "true" block, and a
// "false" block, where the latch block is either the "true" block or the
// "false" block, and the other block has a single store to a single-dimen-
// sioned array. If we return 'true', we set '*IsSimple' to indicate whether
// the loop is simple or complex.
//
static bool isRecProLatchBlock(bool TestSimpleOnly,
                               BasicBlock *BBLoopHeader,
                               BasicBlock *BBLatch,
                               bool *IsSimple) {
  if (!BBLoopHeader || !BBLoopHeader)
    return false;
  if (BBLoopHeader == BBLatch) {
    *IsSimple = true;
    return true;
  }
  if (TestSimpleOnly)
    return false;
  auto BI = dyn_cast<BranchInst>(BBLoopHeader->getTerminator());
  if (!BI || BI->isUnconditional() || BI->getNumSuccessors() != 2)
    return false;
  BasicBlock *BBOther = nullptr;
  if (BI->getSuccessor(0) == BBLatch)
    BBOther = BI->getSuccessor(1);
  else if (BI->getSuccessor(1) == BBLatch)
    BBOther = BI->getSuccessor(0);
  else
    return false;
  if (BBOther->getSingleSuccessor() != BBLatch)
    return false;
  *IsSimple = false;
  return true;
}

//
// Return 'true' if the 'BBPreHeader' is the preheader and 'BBLoopHeader'
// is the loop header for a "simple" or "complex" loop as described in the
// comment immediately above. If 'TestSimpleOnly', we test only for the
// "simple" type of loop. If we return 'true', we expect the loop to have
// constant lower and upper bounds and we set '*LowerBoundOut' and
// '*UpperBoundOut' to those bounds. We set '*PHIOut' to the induction
// variable, '*IsSimpleOut' to whether it is "simple" or not, "*BBLatchOut"
// to the latch block and '*BBExitOut' to the exit block of the loop. (By
// that, we mean the first block encountered after the loop is exited.)
//
static bool isRecProIndexedLoop(BasicBlock *BBPreHeader,
                                BasicBlock *BBLoopHeader,
                                bool TestSimpleOnly,
                                int *LowerBoundOut,
                                int *UpperBoundOut,
                                PHINode **PHIOut,
                                bool *IsSimpleOut,
                                BasicBlock **BBLatchOut,
                                BasicBlock **BBExitOut) {
  if (!BBPreHeader || !BBLoopHeader)
    return false;
  auto PN = dyn_cast<PHINode>(&BBLoopHeader->front());
  if (!PN || PN->getNumIncomingValues() != 2)
    return false;
  unsigned ConstIndex = 0;
  unsigned IncIndex = 1;
  auto CILB = dyn_cast<ConstantInt>(PN->getIncomingValue(0));
  if (!CILB) {
    CILB = dyn_cast<ConstantInt>(PN->getIncomingValue(1));
    if (!CILB)
      return false;
    ConstIndex = 1;
    IncIndex = 0;
  }
  int LowerBound = CILB->getSExtValue();
  if (PN->getIncomingBlock(ConstIndex) != BBPreHeader)
    return false;
  auto BOInc = dyn_cast<BinaryOperator>(PN->getIncomingValue(IncIndex));
  if (!BOInc || BOInc->getOpcode() != Instruction::Add)
    return false;
  if (BOInc->getOperand(0) != PN)
    return false;
  auto CInc = dyn_cast<ConstantInt>(BOInc->getOperand(1));
  if (!CInc || CInc->getSExtValue() != 1)
    return false;
  ICmpInst *CmpI = nullptr;
  for (User *U : BOInc->users()) {
    CmpI = dyn_cast<ICmpInst>(U);
    if (CmpI)
      break;
  }
  if (!CmpI || CmpI->getOperand(0) != BOInc)
    return false;
  if (CmpI->getPredicate() != ICmpInst::ICMP_EQ)
    return false;
  auto CIUB = dyn_cast<ConstantInt>(CmpI->getOperand(1));
  if (!CIUB)
    return false;
  int UpperBound = CIUB->getSExtValue() - 1;
  BasicBlock *BBLatch = PN->getIncomingBlock(IncIndex);
  bool IsSimple = false;
  if (!isRecProLatchBlock(TestSimpleOnly, BBLoopHeader, BBLatch, &IsSimple))
    return false;
  BasicBlock *BBExit = nullptr;
  auto BI = dyn_cast<BranchInst>(BBLatch->getTerminator());
  if (!BI || BI->isUnconditional() || BI->getNumSuccessors() != 2)
    return false;
  if (BI->getSuccessor(0) == BBLoopHeader)
    BBExit = BI->getSuccessor(1);
  else if (BI->getSuccessor(1) == BBLoopHeader)
    BBExit = BI->getSuccessor(0);
  else
    return false;
  *LowerBoundOut = LowerBound;
  *UpperBoundOut = UpperBound;
  *IsSimpleOut = IsSimple;
  *PHIOut = PN;
  *BBLatchOut = BBLatch;
  *BBExitOut = BBExit;
  return true;
}

//
// Return 'true' if the block 'BBStore' has either no StoreInst or a single
// StoreInst. If it has no StoreInst, set '*SIOut' to nullptr, otherwise set
// it to the single StoreInst.
//
static bool isRecProNoOrSingleStoreBlock(BasicBlock *BBStore,
                                         StoreInst **SIOut) {
  StoreInst *SI = nullptr;
  if (!BBStore)
    return false;
  for (Instruction &I : *BBStore) {
    auto CB = dyn_cast<CallBase>(&I);
    if (CB && !dyn_cast<SubscriptInst>(&I))
      return false;
    auto LSI = dyn_cast<StoreInst>(&I);
    if (LSI) {
       if (SI)
         return false;
       SI = LSI;
    }
  }
  *SIOut = SI;
  return true;
}

//
// Return 'true' if 'SI' is a SubscriptInst which indexes a local single-
// dimension 9 element integer array with the induction variable 'PHI'.
// If we return 'true', set '*AIOut' to the AllocaInst that allocates the
// memory for that array.
//
static bool isRecProTempVector(SubscriptInst *SI,
                               PHINode *PHI,
                               AllocaInst **AIOut) {
  auto CI0 = dyn_cast<ConstantInt>(SI->getOperand(0));
  if (!CI0 || CI0->getSExtValue() != 0)
    return false;
  auto CI1 = dyn_cast<ConstantInt>(SI->getOperand(1));
  if (!CI1 || CI1->getSExtValue() != 1)
    return false;
  auto CI2 = dyn_cast<ConstantInt>(SI->getOperand(2));
  if (!CI2 || CI2->getSExtValue() != 4)
    return false;
  auto GEP = dyn_cast<GetElementPtrInst>(SI->getOperand(3));
  if (!GEP || !GEP->hasAllZeroIndices())
    return false;
  int ArraySize = 0;
  if (!isRecProAllocaIntArray(GEP->getPointerOperand(), &ArraySize))
    return false;
  if (ArraySize != 9)
    return false;
  if (SI->getOperand(4) != PHI)
    return false;
  *AIOut = cast<AllocaInst>(GEP->getPointerOperand());
  return true;
}

//
// Return 'true' if 'BBPreHeader' is the preheader and 'BBLoopHeader' is
// the loop header for a "simple" loop, as defined above. If we return
// 'true', set 'AIOut' to the local single-dimension array being written,
// set '*BBLatchOut' to the latch block, and '*BBExitOut' to the exit
// block of the "simple" loop.
//
static bool isRecProSimpleLoop(BasicBlock *BBPreHeader,
                               BasicBlock *BBLoopHeader,
                               AllocaInst **AIOut,
                               BasicBlock **BBLatchOut,
                               BasicBlock **BBExitOut) {
  bool IsSimple = false;
  int LowerBound = 0;
  int UpperBound = 0;
  PHINode *PHI = nullptr;
  if (!BBPreHeader || !BBLoopHeader)
    return false;
  if (!isRecProIndexedLoop(BBPreHeader, BBLoopHeader, true, &LowerBound,
      &UpperBound, &PHI, &IsSimple, BBLatchOut, BBExitOut))
    return false;
  if (!IsSimple || LowerBound != 1 || UpperBound != 9)
    return false;
  StoreInst *SI = nullptr;
  if (!isRecProNoOrSingleStoreBlock(BBLoopHeader, &SI) || !SI)
    return false;
  AllocaInst *AI = nullptr;
  auto SubI = dyn_cast<SubscriptInst>(SI->getPointerOperand());
  if (!SubI || !isRecProTempVector(SubI, PHI, &AI))
    return false;
  *AIOut = AI;
  return true;
}

//
// Return 'true' if 'BBLoopHeader' is the loop header of a "complex" loop
// whose test block tests the value of an element of a single-dimensioned
// array 'AICond' indexed by the induction variable 'PHI'.
//
static bool isRecProComplexCond(BasicBlock *BBLoopHeader,
                                AllocaInst *AICond,
                                PHINode *PHI) {
  if (!BBLoopHeader)
    return false;
  auto BI = dyn_cast<BranchInst>(BBLoopHeader->getTerminator());
  if (!BI || BI->isUnconditional() || BI->getNumSuccessors() != 2)
    return false;
  auto CmpI = dyn_cast<ICmpInst>(BI->getCondition());
  if (!CmpI)
    return false;
  if (CmpI->getPredicate() != ICmpInst::ICMP_EQ)
    return false;
  auto CI0 = dyn_cast<ConstantInt>(CmpI->getOperand(1));
  if (!CI0 || CI0->getSExtValue() != 0)
    return false;
  auto AndI = dyn_cast<BinaryOperator>(CmpI->getOperand(0));
  if (!AndI || AndI->getOpcode() != Instruction::And)
    return false;
  auto CI1 = dyn_cast<ConstantInt>(AndI->getOperand(1));
  if (!CI1 || CI1->getSExtValue() != 1)
    return false;
  auto LI = dyn_cast<LoadInst>(AndI->getOperand(0));
  if (!LI)
    return false;
   AllocaInst *AI = nullptr;
  auto SubI = dyn_cast<SubscriptInst>(LI->getPointerOperand());
  if (!SubI || !isRecProTempVector(SubI, PHI, &AI) || AI != AICond)
    return false;
  return true;
}

//
// Return 'true' if 'BBLoopHeader' is the loop header of a "complex" loop
// which uses the induction varible 'PHI' to index into 'AICond' and assign
// '*AIOut' with the StoreInst '*SIOut'.  If 'IsTrue', the StoreInst is in
// the "true" block of the "complex" loop, otherwise it is in the "false"
// block. Note that '*SIOut' and '*AIOut' are set if we return 'true'.
//
static bool hasRecProComplexTest(BasicBlock *BBLoopHeader,
                                 AllocaInst *AICond,
                                 PHINode *PHI,
                                 bool IsTrue,
                                 StoreInst **SIOut,
                                 AllocaInst **AIOut) {
  if (!BBLoopHeader)
    return false;
  if (!isRecProComplexCond(BBLoopHeader, AICond, PHI))
    return false;
  StoreInst *SI = nullptr;
  if (!isRecProNoOrSingleStoreBlock(BBLoopHeader, &SI))
    return false;
  if (SI)
    return false;
  auto BI = cast<BranchInst>(BBLoopHeader->getTerminator());
  assert(BI->isConditional() && BI->getNumSuccessors() == 2 &&
      "Expected two-way terminator");
  BasicBlock *BBHasnt = IsTrue ? BI->getSuccessor(1) : BI->getSuccessor(0);
  if (!isRecProNoOrSingleStoreBlock(BBHasnt, &SI))
    return false;
  if (SI)
    return false;
  BasicBlock *BBHas = IsTrue ? BI->getSuccessor(0) : BI->getSuccessor(1);
  if (!isRecProNoOrSingleStoreBlock(BBHas, &SI))
    return false;
  if (!SI)
    return false;
  AllocaInst *AI = nullptr;
  auto SubI = dyn_cast<SubscriptInst>(SI->getPointerOperand());
  if (!SubI || !isRecProTempVector(SubI, PHI, &AI))
    return false;
  *SIOut = SI;
  *AIOut = AI;
  return true;
}

//
// Return 'true' if 'BBPreHeader' is the preheader and 'BBLoopHeader'
// is a "complex" loop which tests the value of 'AICond' and assigns
// a constant value '*ConstantValueOut' to '*AIOut'. So, for example, in
//   do i = 1, 9
//     if (t(i)) then
//       l(i) = 1
//     end if
//   end do
// 'AICond' is "t", '*AIOut' is "l" and "*ConstantValueOut" is 1.
// When we return 'true', we set '*AIOut' and '*ConstantValueOut' as well as
// the latch block '*BBLatchOut' and '*BBExitOut'.
//
static bool isRecProTrueBranchComplexLoop(BasicBlock *BBPreHeader,
                                          BasicBlock *BBLoopHeader,
                                          AllocaInst *AICond,
                                          AllocaInst **AIOut,
                                          int *ConstantValueOut,
                                          BasicBlock **BBLatchOut,
                                          BasicBlock **BBExitOut) {
  bool IsSimple = false;
  int LowerBound = 0;
  int UpperBound = 0;
  PHINode *PHI = nullptr;
  if (!BBPreHeader || !BBLoopHeader)
    return false;
  if (!isRecProIndexedLoop(BBPreHeader, BBLoopHeader, false, &LowerBound,
      &UpperBound, &PHI, &IsSimple, BBLatchOut, BBExitOut))
    return false;
  if (IsSimple || LowerBound != 1 || UpperBound != 9)
    return false;
  StoreInst *SI = nullptr;
  AllocaInst *AI = nullptr;
  if (!hasRecProComplexTest(BBLoopHeader, AICond, PHI, true, &SI, &AI))
    return false;
  auto CI = dyn_cast<ConstantInt>(SI->getValueOperand());
  if (!CI)
    return false;
  *AIOut = AI;
  *ConstantValueOut = CI->getSExtValue();
  return true;
}

//
// Return 'true' if 'BBPreHeader' is the preheader and 'BBLoopHeader'
// is a "complex" loop which tests the value of 'AICond' and assigns
// a non-constant value to '*AIOut'. So, for example, in
//   do i = 1, 9
//     if (.not. t(i)) then
//       u(i) = l(i)
//     end if
//   end do
// 'AICond' is "t", '*AIOut' is "u". If 'VLoad' is not nullptr, ensure
// that the value assigned to '*AIOut' is 'VLoad'. When we return 'true',
// we set '*AIOut' as well as the latch block '*BBLatchOut' and '*BBExitOut'.
//
static bool isRecProFalseBranchComplexLoop(BasicBlock *BBPreHeader,
                                           BasicBlock *BBLoopHeader,
                                           AllocaInst *AICond,
                                           AllocaInst *VLoad,
                                           AllocaInst **AIOut,
                                           BasicBlock **BBLatchOut,
                                           BasicBlock **BBExitOut) {
  bool IsSimple = false;
  int LowerBound = 0;
  int UpperBound = 0;
  PHINode *PHI = nullptr;
  if (!BBPreHeader || !BBLoopHeader)
    return false;
  if (!isRecProIndexedLoop(BBPreHeader, BBLoopHeader, false, &LowerBound,
      &UpperBound, &PHI, &IsSimple, BBLatchOut, BBExitOut))
    return false;
  if (IsSimple || LowerBound != 1 || UpperBound != 9)
    return false;
  StoreInst *SI = nullptr;
  AllocaInst *AI = nullptr;
  if (!hasRecProComplexTest(BBLoopHeader, AICond, PHI, false, &SI, &AI))
    return false;
  if (!VLoad) {
    *AIOut = AI;
    return true;
  }
  auto LI = dyn_cast<LoadInst>(SI->getValueOperand());
  if (!LI)
    return false;
  auto SubI = dyn_cast<SubscriptInst>(LI->getPointerOperand());
  if (!SubI)
    return false;
  AllocaInst *AITemp = nullptr;
  if (!isRecProTempVector(SubI, PHI, &AITemp) || AITemp != VLoad)
    return false;
  *AIOut = AI;
  return true;
}

//
// Return 'true' if 'F' starts with a five loop sequence equivalent to:
//   int t(9), l(9), u(9)
//   do i = 1, 9
//     t(i) = ...
//   end do
//   do i = 1, 9
//     if (.not. t(i)) then
//       l(i) = ...
//     end if
//   end do
//   do i = 1, 9
//     if (.not. t(i)) then
//       u(i) = l(i)
//     end if
//   end do
//   do i = 1, 9
//     if (t(i)) then
//       l(i) = constant_l
//     end if
//   end do
//   do i = 1, 9
//     if (t(i)) then
//       u(i) = constant_u
//     end if
//   end do
// If we return 'true', set '*BBLastExitOut' to the block after the sequence,
// set '*LowerValueOut' and '*UpperValueOut' to the AllocaInsts representing
// 'l(9)' and 'u(9)' and '*LowerConstantValueOut' and '*UpperConstantValueOut'
// to the constant values "constant_l" and "constant_u".
//
static bool isRecProSpecialLoopSequence(Function *F,
                                        BasicBlock **BBLastExitOut,
                                        AllocaInst **LowerValueOut,
                                        AllocaInst **UpperValueOut,
                                        int *LowerConstantValueOut,
                                        int *UpperConstantValueOut) {
  AllocaInst *AICond = nullptr;
  BasicBlock *BBLatch = nullptr;
  BasicBlock *BBExit = nullptr;
  AllocaInst *VStoreLower = nullptr;
  AllocaInst *VStoreLowerTemp = nullptr;
  AllocaInst *VStoreUpper = nullptr;
  AllocaInst *VStoreUpperTemp = nullptr;
  int LowerBound = 0;
  int UpperBound = 0;
  BasicBlock *PH = &F->getEntryBlock();
  BasicBlock *LH = PH->getSingleSuccessor();
  if (!isRecProSimpleLoop(PH, LH, &AICond, &BBLatch, &BBExit))
    return false;
  if (!isRecProFalseBranchComplexLoop(BBExit, BBExit->getSingleSuccessor(),
      AICond, nullptr, &VStoreLower, &BBLatch, &BBExit))
    return false;
  if (!isRecProFalseBranchComplexLoop(BBExit, BBExit->getSingleSuccessor(),
      AICond, VStoreLower, &VStoreUpper, &BBLatch, &BBExit))
    return false;
  if (!isRecProTrueBranchComplexLoop(BBLatch, BBExit, AICond, &VStoreLowerTemp,
    &LowerBound, &BBLatch, &BBExit))
    return false;
  if (VStoreLower != VStoreLowerTemp)
    return false;
  if (!isRecProTrueBranchComplexLoop(BBLatch, BBExit, AICond, &VStoreUpperTemp,
    &UpperBound, &BBLatch, &BBExit))
    return false;
  if (VStoreUpper != VStoreUpperTemp)
    return false;
  *BBLastExitOut = BBExit;
  *LowerValueOut = VStoreLower;
  *UpperValueOut = VStoreUpper;
  *LowerConstantValueOut = LowerBound;
  *UpperConstantValueOut = UpperBound;
  return true;
}

// END: Special recognition code for extra clone transformation

//
// Fix the basis call of the recursive progression clone candidate 'OrigF' by
// redirecting it to call the first in the series of recursive progression
// clones, 'NewF'.
//
static void fixRecProgressionBasisCall(Function &OrigF, Function &NewF) {
  auto UI = OrigF.use_begin();
  auto UE = OrigF.use_end();
  for (; UI != UE;) {
    Use &U = *UI;
    ++UI;
    auto CB = dyn_cast<CallBase>(U.getUser());
    if (CB && (CB->getCalledFunction() == &OrigF)) {
      if ((CB->getCaller() != &OrigF) && (CB->getCaller() != &NewF)) {
        U.set(&NewF);
        CB->setCalledFunction(&NewF);
      }
    }
  }
}

// Fix the recursive calls within 'PrevF' to call 'NewF' rather than 'OrigF'.
// After this is done, the recursive progression clone 'foo.1' will look like:
//   static void foo.1(int i) {
//     ..
//     int p = (i + 1) % 4;
//     foo.2(p);
//     ..
//   }
// where 'OrigF' is foo(), 'PrevF' is foo.1(), and 'NewF' is foo.2().
//
static void fixRecProgressionRecCalls(Function &OrigF, Function &PrevF,
                                      Function &NewF) {
  auto UI = OrigF.use_begin();
  auto UE = OrigF.use_end();
  for (; UI != UE;) {
    Use &U = *UI;
    ++UI;
    auto CB = dyn_cast<CallBase>(U.getUser());
    if (CB && (CB->getCalledFunction() == &OrigF)) {
      if (CB->getCaller() == &PrevF) {
        U.set(&NewF);
        CB->setCalledFunction(&NewF);
      }
    }
  }
}

//
// Delete the calls in 'PrevF' to 'OrigF'.
//
// (This is done to ensure that the recursive progression terminates for a
// non-cyclic recursive progression clone candidate.)
//
static void deleteRecProgressionRecCalls(Function &OrigF, Function &PrevF) {
  auto UI = OrigF.use_begin();
  auto UE = OrigF.use_end();
  for (; UI != UE;) {
    Use &U = *UI;
    ++UI;
    auto CB = dyn_cast<CallBase>(U.getUser());
    if (CB && (CB->getCalledFunction() == &OrigF)) {
      if (CB->getCaller() == &PrevF) {
        if (!CB->user_empty())
           CB->replaceAllUsesWith(Constant::getNullValue(CB->getType()));
        CB->eraseFromParent();
      }
    }
  }
}

//
// Create the recursive progression clones for the recursive progression
// clone candidate 'F'.  'ArgPos' is the position of the recursive progression
// argument, whose initial value is 'Start', and is incremented by 'Inc', a
// total of 'Count' times, and then repeats.
//
// If 'IsByRef' is 'true', the recursive progression argument is by reference.
// If 'IsCyclic' is 'true', the recursive progression is cyclic.
//
// For example, in the case of a cyclic recursive progression:
//   static void foo(int i) {
//     ..
//     int p = (i + 1) % 4;
//     foo(p);
//     ..
//   }
//   static void bar() {
//     ..
//     foo(0);
//     ..
//   }
// is replaced by a series of clones:
//   static void foo.0() {
//     ..
//     foo.1();
//     ..
//   }
//   static void foo.1() {
//     ..
//     foo.2();
//     ..
//   }
//   static void foo.2() {
//     ..
//     foo.3();
//     ..
//   }
//   static void foo.3() {
//     ..
//     foo.0();
//     ..
//   }
// with
//   static void bar() {
//     ..
//     foo.0();
//     ..
//   }
//
// while in the case of a non-cyclic recursive progression:
//   static void foo(int j) {
//     ..
//     if (j != 4)
//       foo(j+1);
//     ..
//   }
//   static void bar() {
//     ..
//     foo(0);
//     ..
//   }
// is replaced by a series of clones:
//   static void foo.0() {
//     ..
//     foo.1();
//     ..
//   }
//   static void foo.1() {
//     ..
//     foo.2();
//     ..
//   }
//   static void foo.2() {
//     ..
//     foo.3();
//     ..
//   }
//   static void foo.3() {
//     ..
//     /* The recursive call is deleted here. */
//     ..
//   }
// with
//   static void bar() {
//     ..
//     foo.0();
//     ..
//   }
//
static void createRecProgressionClones(Function &F,
                                       unsigned ArgPos, unsigned Count,
                                       int Start, int Inc, bool IsByRef,
                                       bool IsCyclic) {
  int FormalValue = Start;
  Function *FirstCloneF = nullptr;
  Function *LastCloneF = nullptr;
  assert(Count > 0 && "Expecting at least one RecProgression Clone");
  for (unsigned I = 0; I < Count; ++I) {
    ValueToValueMapTy VMap;
    Function *NewF = CloneFunction(&F, VMap);
    // Mark the first Count - 1 clones as preferred for inlining, the last
    // preferred for not inlining.
    if (!IsCyclic || I < Count - 1)
      NewF->addFnAttr("prefer-inline-rec-pro-clone");
    else
      NewF->addFnAttr("prefer-noinline-rec-pro-clone");
    // In any case, it contains a recursive progression clone, because it is
    // one, and the merge rule function ContainsRecProCloneAttr guarentees
    // that any function this function is inlined into will also contain a
    // recursive progression clone.
    NewF->addFnAttr("contains-rec-pro-clone");
    if (LastCloneF)
      fixRecProgressionRecCalls(F, *LastCloneF, *NewF);
    else
      fixRecProgressionBasisCall(F, *NewF);
    NumIPCloned++;
    Argument *NewFormal = NewF->arg_begin() + ArgPos;
    auto ConstantType = NewFormal->getType();
    if (IsByRef)
      ConstantType = ConstantType->getPointerElementType();
    Value *Rep = ConstantInt::get(ConstantType, FormalValue);
    FormalValue += Inc;
    if (IPCloningTrace) {
      errs() << "        Function: " << NewF->getName() << "\n";
      errs() << "        ArgPos : " << ArgPos << "\n";
      errs() << "        Argument : " << *NewFormal << "\n";
      errs() << "        IsByRef : " << (IsByRef ? "T" : "F") << "\n";
      errs() << "        Replacement:  " << *Rep << "\n";
    }
    if (IsByRef) {
      assert(NewFormal->hasOneUse() && "Expecting single use of ByRef Formal");
      auto LI = cast<LoadInst>(*(NewFormal->user_begin()));
      LI->replaceAllUsesWith(Rep);
    } else
      NewFormal->replaceAllUsesWith(Rep);
    if (!IsCyclic && I == Count - 1) {
      BasicBlock *BBLast = nullptr;
      AllocaInst *AILower = nullptr;
      AllocaInst *AIUpper = nullptr;
      int CVLower = 0;
      int CVUpper = 0;
      if (isRecProSpecialLoopSequence(NewF, &BBLast, &AILower, &AIUpper,
          &CVLower, &CVUpper)) {
        // Add transformation code here.
        if (IPCloningTrace)
          errs() << "Extra RecProClone Candidate: " << NewF->getName() << "\n";
      }
    }
    if (!FirstCloneF)
      FirstCloneF = NewF;
    LastCloneF = NewF;
  }
  if (IsCyclic)
    fixRecProgressionRecCalls(F, *LastCloneF, *FirstCloneF);
  else
    deleteRecProgressionRecCalls(F, *LastCloneF);
}

// Create argument set for CallInst 'CI' of  'F' and save it in
// 'ConstantArgsSet'
//
static void createConstantArgumentsSet(CallInst &CI,  Function &F,
         std::vector<std::pair<unsigned, Constant *>>& ConstantArgsSet) {

  unsigned position = 0;
  auto CAI = CI.arg_begin();
  for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end();
       AI != E; ++AI, ++CAI, position++) {

    // Ignore formals that are not selected by heuristics to reduce
    // code size, compile-time etc
    if (!WorthyFormalsForCloning.count(&*AI))
      continue;


    Value* ActualV = *CAI;
    auto &ValList = ActualConstantValues[ActualV];
    if (ValList.size() == 0)
      continue;
    Constant *C = *ValList.begin();
    ConstantArgsSet.push_back(std::make_pair(position, C));
  }
}

// For given constant argument set 'ConstantArgs', it returns index
// of the constant argument set in "FunctionAllArgumentsSets".
//
static unsigned getConstantArgumentsSetIndex(
        std::vector<std::pair<unsigned, Constant *>>& ConstantArgs) {
  auto I = FunctionAllArgumentsSets.begin();
  auto E = FunctionAllArgumentsSets.end();
  unsigned index = 0;
  for(; I != E; I++) {
    if (I->second == ConstantArgs) {
      return I->first;
    }
    index++;
  }
  auto &CArgs = FunctionAllArgumentsSets[index];
  std::copy(ConstantArgs.begin(), ConstantArgs.end(),
            std::back_inserter(CArgs));
  return index;
}

// Heuristics to enable cloning for 'F'. Currently, it returns true always.
//
static bool isFunctionWorthyForCloning(Function &F) {
  // May need to add some heuristics like size of routine etc
  //
  return true;
}

// Returns true if cloning is skipped for 'F'.
//
static bool skipAnalyzeCallsOfFunction(Function &F) {
  if (F.isDeclaration() || F.isIntrinsic() || !F.hasExactDefinition() ||
      F.use_empty())
    return true;

   // Skip cloning analysis if it is cloned routine.
   if (ClonedFunctionList.count(&F))
     return true;

  // Allow  all routines for now
  if (!F.hasLocalLinkage())
    return true;

  if (!isFunctionWorthyForCloning(F))
    return true;

  return false;
}

// Dump constant values collected for each formal of 'F'
//
static void dumpFormalsConstants(Function &F) {
  unsigned position = 0;
  for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end();
       AI != E; ++AI, position++) {

     auto CList = FormalConstantValues[&*AI];
     errs() <<  "         Formal_" << position << ":";
     if (InexactFormals.count(&*AI))
       errs() << "  (Inexact)  \n";
     else
       errs() << "  (Exact)  \n";

     // Dump list of constants
     for (auto I = CList.begin(), E = CList.end(); I != E; I++) {
       errs() << "                  " << *(*(&*I)) << "\n";
     }
  }
  errs() << "\n\n";
}

// It collects worthy formals for cloning by applying heuristics.
// It returns true if there are any worthy formals.
//
// If 'IFSwitchHeuristic' is true, the if-switch heuristic may be applied.
// If 'IsGenRec' is true, we are testing a recursive function for generic
// cloning. In this case, we may qualify the formals if there are at
// least 'IPGenCloningMinRecFormalCount' formals that qualify under the
// if-switch heuristic, but an additional test on the number of clones and
// callsites will be performed after we return from this function.
// In that case, set '*IsGenRecQualified' to true to indicate that the
// worthy formals are only qualified, if this additional condition is
// fulfilled.
//
static bool findWorthyFormalsForCloning(Function &F, bool AfterInl,
                                        bool IFSwitchHeuristic, bool IsGenRec,
                                        bool *IsGenRecQualified) {

  SmallPtrSet<Value *, 16> PossiblyWorthyFormalsForCloning;
  WorthyFormalsForCloning.clear();
  // Create Loop Info for routine
  LoopInfo LI{DominatorTree(const_cast<Function &>(F))};

  unsigned int f_count = 0;
  unsigned GlobalIFCount = 0;
  unsigned GlobalSwitchCount = 0;
  bool SawPending = false;
  for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end();
       AI != E; ++AI) {

    Value *V = &*AI;
    f_count++;

    // Ignore formal if it doesn't have any constants at call-sites
    auto &ValList = FormalConstantValues[V];
    if (ValList.size() == 0)
      continue;

    if (IPCloningTrace) {
      errs() << " Collecting potential constants for Formal_";
      errs() << (f_count - 1) << "\n";
    }
    if (AfterInl || IsGenRec) {
      unsigned IFCount = 0;
      unsigned SwitchCount = 0;
      if (findPotentialConstsAndApplyHeuristics(F, V, &LI, true,
                                                IFSwitchHeuristic,
                                                &IFCount, &SwitchCount)) {
        if (IFCount + SwitchCount == 0) {
          // Qualified unconditionally under the loop heuristic.
          WorthyFormalsForCloning.insert(V);
          if (IPCloningTrace)
            errs() << "  Selecting FORMAL_" << (f_count - 1) << "\n";
        } else {
          // Qualified under the if-switch heuristic. Mark the formal as
          // pending for now, and qualify it later if the total number of
          // "if" and "switch" values that become constant is great enough.
          SawPending = true;
          GlobalIFCount += IFCount;
          GlobalSwitchCount += SwitchCount;
          PossiblyWorthyFormalsForCloning.insert(V);
          if (IPCloningTrace) {
            errs() << "  Pending FORMAL_" << (f_count - 1) << "\n";
            errs() << "    IFCount " << GlobalIFCount << " <- "
                   << IFCount << "\n";
            errs() << "    SwitchCount " << GlobalSwitchCount << " <- "
                   << SwitchCount << "\n";
          }
        }
      } else {
        if (IPCloningTrace) {
          errs() << "  Skipping FORMAL_" << (f_count - 1);
          errs() << " due to heuristics\n";
        }
      }
    } else {
      // No heuristics for IPCloning before Inlining, unless IsGenRec
      WorthyFormalsForCloning.insert(V);
    }
  }
  if (EnableMorphologyCloning && GlobalIFCount >= IPGenCloningMinIFCount &&
      GlobalSwitchCount >= IPGenCloningMinSwitchCount) {
    // There are enough "if" and "switch" values to qualify the clone under
    // the if-switch heuristic. Convert the pending formals to qualified.
    if (IPCloningTrace)
      errs() << "  Selecting all Pending FORMALs\n";
    for (Value *W : PossiblyWorthyFormalsForCloning)
      WorthyFormalsForCloning.insert(W);
  } else if (IsGenRec && PossiblyWorthyFormalsForCloning.size() >=
      IPGenCloningMinRecFormalCount) {
    if (IPCloningTrace)
      errs() << "  Possibly selecting all Pending FORMALs in "
             << "Recursive Function\n";
    for (Value *W : PossiblyWorthyFormalsForCloning)
      WorthyFormalsForCloning.insert(W);
    *IsGenRecQualified = true;
  } else if (SawPending) {
    if (IPCloningTrace) {
      if (GlobalIFCount < IPGenCloningMinIFCount)
        errs() << "  IFCount (" << GlobalIFCount << ") < Limit ("
               << IPGenCloningMinIFCount << ")\n";
      if (GlobalSwitchCount < IPGenCloningMinSwitchCount)
        errs() << "  SwitchCount (" << GlobalSwitchCount << ") < Limit ("
               << IPGenCloningMinSwitchCount << ")\n";
    }
  }
  // Return false if none of formals is selected.
  if (WorthyFormalsForCloning.size() == 0)
    return false;

  return true;
}

// It analyzes all callsites of 'F' and collect all possible constant
// argument sets. All collected constant argument sets are saved in
// "FunctionAllArgumentsSets". It return false if number of constant
// argument sets exceeds "IPFunctionCloningLimit".
//
static bool collectAllConstantArgumentsSets(Function &F) {

  std::vector<std::pair<unsigned, Constant *>> ConstantArgs;
  for (unsigned i = 0, e = CurrCallList.size(); i != e; ++i) {
    CallInst *CI = CurrCallList[i];
    ConstantArgs.clear();
    createConstantArgumentsSet(*CI, F, ConstantArgs);
    if (ConstantArgs.size() == 0)
      continue;
    unsigned index = getConstantArgumentsSetIndex(ConstantArgs);
    CallInstArgumentSetIndexMap[CI] = index;

    if (FunctionAllArgumentsSets.size() > IPFunctionCloningLimit) {
      if (IPCloningTrace)
        errs() << "     Exceeding number of argument sets limit \n";
      return false;
    }
  }
  if (FunctionAllArgumentsSets.size() == 0) {
    if (IPCloningTrace)
      errs() << "     Zero argument sets found \n";
    return false;
  }
  if (IPCloningTrace) {
    errs() << "    Number of argument sets found: ";
    errs() << FunctionAllArgumentsSets.size() << "\n";
  }

  return true;
}

// Returns true if there is a constant value in 'CArgs' at 'position'.
//
static bool isArgumentConstantAtPosition(
                std::vector<std::pair<unsigned, Constant *>> & CArgs,
                unsigned position) {
  for(auto I = CArgs.begin(), E = CArgs.end(); I != E; I++) {
    if (I->first == position)
      return true;
  }
  return false;
}

// Returns true if it is valid to set callee of callsite 'CI' to 'ClonedFn'.
// This routine makes sure that same constant argument set of 'ClonedFn'
// is passed to 'CI'.  'index' is index of constant argument set for
// 'ClonedFn'.
//
static bool okayEliminateRecursion(Function *ClonedFn, unsigned index,
                                   CallInst &CI) {
  // Get constant argument set for ClonedFn.
  auto &CArgs = FunctionAllArgumentsSets[index];

  unsigned position = 0;
  auto CAI = CI.arg_begin();
  for (Function::arg_iterator AI = ClonedFn->arg_begin(),
       E = ClonedFn->arg_end(); AI != E; ++AI, ++CAI, position++) {

    if (!isArgumentConstantAtPosition(CArgs, position)) {
      // If argument is not constant in CArgs, then actual argument of CI
      // should be non-constant.
     if (isConstantArgForCloning(*CAI, FuncPtrsClone))
        return false;

    } else {
      // If argument is constant in CArgs, then actual argument of CI
      // should pass through formal.
      if ((&*AI) != (*CAI))
        return false;
    }
  }
  return true;
}

// Fix recursion callsites in cloned functions if possible.
//
//  Before cloning:
//     spec_qsort(...) {  <- entry
//        ...
//        spec_qsort(...);  <- call
//        ...
//     }
//
//  After cloning:
//     spec_qsort..0(...) {   <- entry
//        ...
//        spec_qsort(...);    <- call
//        ...
//     }
//
//   Fix recursion if possible:
//     spec_qsort..0(...) {   <- entry
//        ...
//        spec_qsort..0(...); <- call
//        ...
//     }
//
static void eliminateRecursionIfPossible(Function *ClonedFn,
                      Function *OriginalFn, unsigned index) {
  for (inst_iterator II = inst_begin(ClonedFn), E = inst_end(ClonedFn);
     II != E; ++II) {
    if (!isa<CallInst>(&*II))
      continue;
    auto CI = cast<CallInst>(&*II);
    Function *Callee = CI->getCalledFunction();
    if (Callee == OriginalFn &&
        okayEliminateRecursion(ClonedFn, index, *CI)) {
      CI->setCalledFunction(ClonedFn);
      NumIPCallsCloned++;

      if (IPCloningTrace)
        errs() << " Replaced Cloned call:   " << *CI << "\n";
    }
  }
}

// It does actual cloning and fixes recursion calls if possible.
//
static void cloneFunction(void) {
  for (unsigned I = 0, E = CurrCallList.size(); I != E; ++I) {
    ValueToValueMapTy VMap;
    CallInst *CI = CurrCallList[I];

    // Skip callsite if  no constant argument set is collected.
    if (CallInstArgumentSetIndexMap.find(CI) ==
        CallInstArgumentSetIndexMap.end()) {
      continue;
    }
    Function* SrcFn = CI->getCalledFunction();

    // Get cloned function for constant argument set if it is already there
    unsigned index = CallInstArgumentSetIndexMap[CI];
    Function* NewFn = ArgSetIndexClonedFunctionMap[index];

    // Create new clone if it is not there for constant argument set
    if (NewFn == nullptr) {
      NewFn = CloneFunction(SrcFn, VMap);
      ArgSetIndexClonedFunctionMap[index] = NewFn;
      ClonedFunctionList.insert(NewFn);
      NumIPCloned++;
    }

    CI->setCalledFunction(NewFn);
    NumIPCallsCloned++;
    eliminateRecursionIfPossible(NewFn, SrcFn, index);

    if (IPCloningTrace)
      errs() << " Cloned call:   " << *CI << "\n";
  }
}

// Returns true if there is a specialization constant value
// in 'CArgs' at 'Position'.
//
static Value* isSpecializationConstantAtPosition(
   std::vector<std::pair<unsigned, Value *>> & CArgs, unsigned Position) {

  for(auto I = CArgs.begin(), E = CArgs.end(); I != E; I++) {
    if (I->first == Position)
      return I->second;
  }
  return nullptr;
}

// Creates GetElementPtrInst 'BaseAddr' as pointer operand with
// 'NumIndices' number of Indices and  inserts at the beginning
// of 'NewFn'.
//
// %7 = getelementptr inbounds [3 x [2 x i8]],
//                [3 x [2 x i8]]* @t.CM_ONE, i32 0, i32 0
//
static Value* createGEPAtFrontInClonedFunction(Function* NewFn,
                          Value* BaseAddr, unsigned NumIndices) {

   Type *Int32Ty;
   Value* Rep;
   SmallVector<Value*, 4> Indices;

   Instruction *InsertPt = &NewFn->begin()->front();
   Int32Ty = Type::getInt32Ty(NewFn->getContext());
   // Create Indices with zero value.
   for (unsigned I = 0; I < NumIndices; I++)
     Indices.push_back(ConstantInt::get(Int32Ty, 0));

   Rep = GetElementPtrInst::CreateInBounds(BaseAddr, Indices, "", InsertPt);
   if (IPCloningTrace)
     errs() << "     Created New GEP: " << *Rep << "\n";

   return Rep;
}

// It unpacks 'Number' into Initializer with 'Cols' columns and 'Rows'
// rows. Then, it creates new Global Variables and sets Initializer.
// 'NewFn' and 'CallI' are used to get Context and Module for creating
// Types and Global Variable.
//
// Ex:
//  @convolutionalEncode.136.clone.0  = private constant [4 x [2 x i8]]
//     [[2 x i8] c"\01\01", [2 x i8] c"\01\00", [2 x i8] c"\01\01",
//     [2 x i8] c"\01\01"]
//
static GlobalVariable* createGlobalVariableWithInit(Function* NewFn,
                uint64_t Number, Instruction* CallI,
                unsigned Cols, unsigned Rows, unsigned &Counter) {

  auto ArrayTy = ArrayType::get(Type::getInt8Ty(NewFn->getContext()), Rows);
  auto ArrayArrayTy = ArrayType::get(ArrayTy, Cols);
  SmallVector<Constant *, 16> ArrayVec;
  SmallVector<Constant *, 16> ArrayArrayVec;

  // Unpack 'Number" and create INIT like below
  //
  // Convert 0x0101010100010101 to
  // [[2 x i8] c"\01\01", [2 x i8] c"\01\00", [2 x i8] c"\01\01",
  //    [2 x i8] c"\01\01"]
  //
  ArrayArrayVec.clear();
  for (unsigned I = 0; I < Cols; I++) {
    ArrayVec.clear();
    for(unsigned J = 0; J < Rows; J++) {
      ArrayVec.push_back(ConstantInt::get(
              Type::getInt8Ty(NewFn->getContext()), Number & 0xFF));
      // Shift Number by size of Int8Ty
      Number = Number >> 8;
    }
    ArrayArrayVec.push_back(ConstantArray::get(ArrayTy, ArrayVec));
  }

  // Create New Global Variable and set Initializer
  Module *M = CallI->getModule();
  auto *NewGlobal = new GlobalVariable(*M, ArrayArrayTy ,
                /*isConstant=*/true,
                GlobalValue::PrivateLinkage, nullptr,
                NewFn->getName()+".clone."+Twine(Counter));

   NewGlobal->setInitializer(ConstantArray::get(ArrayArrayTy, ArrayArrayVec));
   Counter++;

  if (IPCloningTrace)
    errs() << "     Created New Array:  " << *NewGlobal << "\n";
  return NewGlobal;
}


// For given specialization constant 'V', it gets/creates Value that needs
// to be propagated to 'NewFn'. 'Formal' is used to get type info of
// argument. 'CallI' and 'DL' are used to get Module and size info.
//
static Value* getReplacementValueForArg(Function* NewFn, Value *V,
                                 Value* Formal, Instruction *CallI,
                                 const DataLayout &DL, unsigned& Counter) {

  // Case 0:
  //   It is plain constant. Just returns the same.
  if (isa<Constant>(V)) return V;

  Value* PropValue = nullptr;;
  PropValue = SpecialConstPropagatedValueMap[V];

  // If it is not constant, there are two possible values that need
  // to be propagated.
  // Case 1:
  //        store i64 72340172821299457, i64* %6, align 8
  //
  //  Case 2:
  //   getelementptr inbounds ([5 x [2 x i8]], [5 x [2 x i8]]* @i.CM_THREE
  //

  Value* Rep;
  GetElementPtrInst* GEP = SpecialConstGEPMap[V];
  unsigned NumIndices = GEP->getNumIndices();

  if (!isa<StoreInst>(PropValue)) {
    // Case 2:
    //    Create New GEP Instruction in cloned function
    //
    //    %7 = getelementptr inbounds [5 x [2 x i8]],
    //                [5 x [2 x i8]]* @t.CM_THREE, i32 0, i32 0
    //
    Rep = createGEPAtFrontInClonedFunction(NewFn, PropValue, NumIndices);
    return Rep;
  }

  assert(isa<StoreInst>(PropValue) && "Expects StoreInst");

  // Case 1:
  //     1. Create new global variable with INIT
  //     2. Then create New GEP Instruction in cloned function
  //
  //     @convolutionalEncode.136.clone.0 = private constant [4 x [2 x i8]]
  //         [[2 x i8] c"\01\01", [2 x i8] c"\01\00", [2 x i8] c"\01\01",
  //         [2 x i8] c"\01\01"]
  //
  //     %7 = getelementptr inbounds [4 x [2 x i8]],
  //          [4 x [2 x i8]]* @convolutionalEncode.136.clone.0, i32 0, i32 0
  //

  unsigned SizeInBytes = 0;
  unsigned NumElems = 0;

  // Get Constant value from StoreInst
  Value* Val = cast<StoreInst>(PropValue)->getOperand(0);
  ConstantInt* CI = cast<ConstantInt>(Val);

  GetPointerToArrayDims(Formal->getType(), SizeInBytes, NumElems, DL);
  assert((SizeInBytes > 0) && "Expects pointer to Array Type");

  // Create New GlobalVariable
  auto *NewGlobal = createGlobalVariableWithInit(NewFn,
                            CI->getZExtValue(),
                            CallI,
                            CI->getBitWidth() / SizeInBytes /* cols */,
                            NumElems /* rows */,
                            Counter);

  // Create GEP Inst in cloned function
  Rep = createGEPAtFrontInClonedFunction(NewFn, NewGlobal, NumIndices);
  return Rep;
}

// It propagates all constant arguments to clone function 'NewFn'.
// 'ArgsIndex' is used to get ArgumentSet for 'NewFn'.
// 'CallI' helps to get Module in case if GlobalVariable needs to
// be created.
//
static void propagateArgumentsToClonedFunction(Function* NewFn,
                           unsigned ArgsIndex, CallInst *CallI) {
  unsigned Position = 0;
  unsigned Counter = 0;
  Value* Rep;
  auto &CallArgsSets = AllCallsArgumentsSets[CallI];
  auto CArgs = CallArgsSets[ArgsIndex];
  const DataLayout &DL = CallI->getModule()->getDataLayout();

  for (Function::arg_iterator AI = NewFn->arg_begin(), EI = NewFn->arg_end();
     AI != EI; ++AI, Position++) {

     Value* V = isSpecializationConstantAtPosition(CArgs, Position);
     if (V == nullptr) continue;

     Value* Formal = &*AI;

     Rep = getReplacementValueForArg(NewFn, V, Formal, CallI, DL, Counter);

    if (IPCloningTrace) {
      errs() << "        Formal : " << *AI << "\n";
      errs() << "        Value : " << *V << "\n";
      errs() << "        Replacement:  " << *Rep << "\n";
    }

    Formal->replaceAllUsesWith(Rep);
  }
}

//
// Create a new call instruction for a clone of 'CI' and insert it in
// 'Insert_BB'. Return a CallInst* for that new call instruction.
// NewCall is created for 'ArgsIndex', which is the index of argument-sets
// of CI.
//
static CallInst *createNewCall(CallInst &CI, BasicBlock* Insert_BB,
                               unsigned ArgsIndex) {

  Function* SrcFn = CI.getCalledFunction();

  // Get argument-sets at ArgsIndex fpr CI
  std::vector<std::pair<unsigned, Constant *>> ConstantArgs;
  auto &CallArgsSets = AllCallsArgumentsSets[&CI];
  auto CArgs = CallArgsSets[ArgsIndex];

  // Create ConstantArgs to check if there is already cloned Function
  // created with same ConstantArgs. Reuse it if there is already
  // cloned function for CArgs.
  unsigned Position = 0;
  ConstantArgs.clear();
  for (Function::arg_iterator AI = SrcFn->arg_begin(), EI = SrcFn->arg_end();
     AI != EI; ++AI, Position++) {

     Value* V = isSpecializationConstantAtPosition(CArgs, Position);
     if (V == nullptr) continue;
     // For now, it handles only Constants. We may need to handle special
     // constants like address of stack locations etc in future.
     if (Constant *C = dyn_cast<Constant>(V)) {
       ConstantArgs.push_back(std::make_pair(Position, C));
     }
  }
  unsigned Index = getConstantArgumentsSetIndex(ConstantArgs);
  Function* NewFn = ArgSetIndexClonedFunctionMap[Index];

  ValueToValueMapTy VMap;
  CallInst* New_CI;
  // Create new cloned function for ConstantArgs if it is not already
  // there.
  if (NewFn == nullptr) {
    NewFn = CloneFunction(SrcFn, VMap);
    ArgSetIndexClonedFunctionMap[Index] = NewFn;
    ClonedFunctionList.insert(NewFn);
    propagateArgumentsToClonedFunction(NewFn, ArgsIndex, &CI);
    NumIPCloned++;
  }
  std::vector<Value*> Args(CI.op_begin(), CI.op_end() - 1);
  // NameStr should be "" if return type is void.
  std::string New_Name;
  New_Name = CI.hasName() ? CI.getName().str() + ".clone.spec.cs" : "";
  New_CI = CallInst::Create(NewFn, Args, New_Name, Insert_BB);
  New_CI->setDebugLoc(CI.getDebugLoc());
  New_CI->setCallingConv(CI.getCallingConv());
  New_CI->setAttributes(CI.getAttributes());
  return New_CI;
}

//
//// Produce the cloning specialization tests and calls, based on the
//// information stored in CurrCallList, InexactArgsSetsCallList,
//// CallArgumentsSets, and InexactArgsSetsCallList.
////
//
static void cloneSpecializationFunction(void) {

  std::vector<CmpInst*> NewCondStmts;
    // New conditonal tests used in specialization
  std::vector<BasicBlock*> NewCondStmtBBs;
  std::vector<CallInst*> NewClonedCalls;
  std::vector<BasicBlock*> NewClonedCallBBs;
    // The basic blocks the NewClonedCalls will be in

  // Iterate through the list of CallSites that will be cloned.
  for (unsigned I = 0, E = CurrCallList.size(); I != E; ++I) {
    NewClonedCallBBs.clear();
    NewClonedCalls.clear();
    NewCondStmtBBs.clear();
    NewCondStmts.clear();
    CallInst *CI = CurrCallList[I];
    if (IPCloningTrace)
       errs() << "\n Call-Site (Spec): " << *CI << "\n\n";
    auto &CallArgsSets = AllCallsArgumentsSets[CI];

    if (CallArgsSets.size() == 0)
      continue;

    // No point to specialize, if there is only one arg set for this CallSite
    if (CallArgsSets.size() <= 1) {
      if (IPCloningTrace)
        errs() << "    Giving up: Not enough cases to specialize\n";
      continue;
    }
    // Split the BasicBlock containing the CallSite, so that the newly
    // generated code with tests and calls goes between the split portions.
    BasicBlock *OrigBB = CI->getParent();
    BasicBlock *TailBB = OrigBB->splitBasicBlock(CI);
    unsigned CloneCount = CallArgsSets.size();
    bool IsInexact = InexactArgsSetsCallList.count(CI);
    unsigned NumConds = CloneCount - 1;
    if (IsInexact) ++NumConds;
    // Make the clones for this CallSite
    for (unsigned J = 0; J < CloneCount; J++) {
     if (J < NumConds) {
      // Create a BasicBlock CondBB to hold the condition test
      BasicBlock *CondBB = BasicBlock::Create(CI->getContext(),
        ".clone.spec.cond", OrigBB->getParent(), TailBB);
      // Create the conditional expression
      Value *TAnd = nullptr;
      auto CArgs = CallArgsSets[J];
      for (auto AI = CArgs.begin(), AE = CArgs.end(); AI != AE; AI++) {
        Value *RHS = AI->second;
        Instruction* II = dyn_cast<Instruction>(RHS);
        // If the definition of the right-hand side value is an instruction,
        // rematerialize it.
        Instruction *NewII = nullptr;
        if (II != nullptr)
          NewII = II->clone();
        Value *LCmp = CmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_EQ,
          CI->getArgOperand(AI->first), II != nullptr ? NewII : RHS,
          ".clone.spec.cmp", CondBB);
        if (II != nullptr)
          NewII->insertBefore(cast<Instruction>(LCmp));
        TAnd = TAnd == nullptr ? LCmp
          : BinaryOperator::CreateAnd(TAnd, LCmp, ".clone.spec.and", CondBB);
      }
      Constant *ConstantZero = ConstantInt::get(TAnd->getType(), 0);
     // Cmp is the final comparison for the conditional test
      CmpInst *Cmp = CmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_NE,
        TAnd, ConstantZero, ".clone.spec.cmp", CondBB);
      Cmp->setDebugLoc(CI->getDebugLoc());
      // Set aside Cmp and CondBB for further processing.
      NewCondStmts.push_back(Cmp);
      NewCondStmtBBs.push_back(CondBB);
    }
      // Create a cloned call and the BasicBlock that contains it
      BasicBlock* CallBB = BasicBlock::Create(CI->getContext(),
        ".clone.spec.call", OrigBB->getParent(), TailBB);
      CallInst *NewCI = createNewCall(*CI, CallBB, J);
      NewClonedCalls.push_back(NewCI);
      NewClonedCallBBs.push_back(CallBB);
      // Connect the cloned call's BasicBlock to its successor
      BranchInst *BI = BranchInst::Create(TailBB, CallBB);
      BI->setDebugLoc(CI->getDebugLoc());
    }
    // Generate a fall back case, if needed
    if (IsInexact) {
      // Generate a call for the original function and a BasicBlock to
      // hold it
      BasicBlock* CallBB = BasicBlock::Create(CI->getContext(),
        ".clone.spec.call", OrigBB->getParent(), TailBB);
      CallInst *NewCI = cast<CallInst>(CI->clone());
      CallBB->getInstList().push_back(NewCI);
      NewClonedCalls.push_back(NewCI);
      //NewCondStmtBBs.push_back(CallBB);
      BranchInst *BI = BranchInst::Create(TailBB, CallBB);
      BI->setDebugLoc(CI->getDebugLoc());
      NewClonedCallBBs.push_back(CallBB);
      // Inlining of fallback CallSite causes huge performance regression
      // for conven00 benchmark due to downstream optimizations. Set
      // NoInline attribute for fallback CallSite for now.
      NewCI->setIsNoInline();
    } else {
      // Branch directly to the TailBB without calling the original function
      //NewCondStmtBBs.push_back(TailBB);
    }
    // Complete the BasicBlock to BasicBlock connections
    OrigBB->getInstList().pop_back();
    BranchInst::Create(NewCondStmtBBs[0], OrigBB);
    BasicBlock* F_BB;
    for (unsigned J = 0; J < NumConds; J++) {
      if (J + 1 < NumConds) {
        F_BB = NewCondStmtBBs[J+1];
      } else {
        F_BB = NewClonedCallBBs[J + 1];
      }
      BranchInst *BI = BranchInst::Create(NewClonedCallBBs[J],
        F_BB, NewCondStmts[J], NewCondStmtBBs[J]);
      BI->setDebugLoc(CI->getDebugLoc());
    }
    // If the cloned calls have return values, connect them together with
    // a PHI node.
    if (!CI->getType()->isVoidTy()) {
      unsigned CallCount = NewClonedCalls.size();
      PHINode *RPHI = PHINode::Create(CI->getType(), CallCount,
          ".clone.spec.phi", &TailBB->front());
      for (unsigned J = 0; J < CallCount; J++) {
        RPHI->addIncoming(NewClonedCalls[J], NewClonedCallBBs[J]);
      }
      RPHI->setDebugLoc(CI->getDebugLoc());
      CI->replaceAllUsesWith(RPHI);
    }
    if (IPCloningTrace) {
      for  (unsigned J = 0; J < CloneCount; J++) {
       if (J < NumConds) {
        errs() << "    Cond[" << J << "] = ";
        errs() << *NewCondStmtBBs[J] << "\n";
       }
        errs() << "    ClonedCall[" << J << "] = "
          << *(NewClonedCallBBs[J]) << "\n\n";
      }
      if (IsInexact)
        errs() << "    Fallback Call = "
          << *(NewClonedCallBBs[CloneCount]) << "\n\n";
      else
        errs() << "    No Fallback Call" << "\n\n";
    }
    CI->eraseFromParent();
  }
}

// Clear all maps and sets
//
static void clearAllMaps(void) {
  CallInstArgumentSetIndexMap.clear();
  FunctionAllArgumentsSets.clear();
  ArgSetIndexClonedFunctionMap.clear();
  FormalConstantValues.clear();
  InexactFormals.clear();
  CurrCallList.clear();
  WorthyFormalsForCloning.clear();
  ActualConstantValues.clear();
  InexactArgsSetsCallList.clear();
  SpecialConstPropagatedValueMap.clear();
  AllCallsArgumentsSets.clear();
  SpecialConstGEPMap.clear();
}

//
// Return 'true' if 'F' is a directly recursive routine. (There is a callsite
// in 'F' that calls 'F'.)
//
static bool isDirectlyRecursive(Function *F) {
  for (User *U : F->users()) {
    auto CB = dyn_cast<CallBase>(U);
    if (!CB || CB->getCalledFunction() != F)
      continue;
    if (CB->getCaller() == F)
      return true;
  }
  return false;
}

// Main routine to analyze all calls and clone functions if profitable.
//
static bool analysisCallsCloneFunctions(Module &M, bool AfterInl,
                                        bool IFSwitchHeuristic) {
  bool FunctionAddressTaken;

  if (IPCloningTrace) {
    errs() << " Enter IP cloning";
    if (AfterInl)
      errs() << ": (After inlining)\n";
    else
      errs() << ": (Before inlining)\n";
  }

  ClonedFunctionList.clear();

  for (Function &F : M) {

    if (skipAnalyzeCallsOfFunction(F)) {
      if (IPCloningTrace)
        errs() << " Skipping " << F.getName() << "\n";
      continue;
    }

    clearAllMaps();

    if (IPCloningTrace)
      errs() << " Cloning Analysis for:  " <<  F.getName() << "\n";

    IPCloneKind CloneType;
    if (AfterInl) {
      CloneType = GenericClone;
      if (IPCloningTrace)
        errs() << "    Selected generic cloning  " << "\n";
    } else {
      int Start, Inc;
      unsigned ArgPos, Count;
      bool IsByRef, IsCyclic;
      if (isRecProgressionCloneCandidate(F, true,
          &ArgPos, &Count, &Start, &Inc, &IsByRef, &IsCyclic)) {
        CloneType = RecProgressionClone;
        if (IPCloningTrace)
          errs() << "    Selected RecProgression cloning  " << "\n";
        createRecProgressionClones(F, ArgPos, Count, Start, Inc, IsByRef,
                                   IsCyclic);
        continue;
      }
      // For now, run either FuncPtrsClone or SpecializationClone for any
      // function before inlining. If required, we can run both in future.
      // FuncPtrsClone is selected for a function if it has at least one
      // function-pointer type argument.
      if (IsFunctionPtrCloneCandidate(F)) {
        CloneType = FuncPtrsClone;
        if (IPCloningTrace)
          errs() << "    Selected FuncPtrs cloning  " << "\n";
      } else if (isDirectlyRecursive(&F)) {
        CloneType = GenericClone;
        if (IPCloningTrace)
          errs() << "    Selected generic cloning (recursive) " << "\n";
      } else {
        CloneType = SpecializationClone;
        if (IPCloningTrace)
          errs() << "    Selected Specialization cloning  " << "\n";
      }
    }
    FunctionAddressTaken = analyzeAllCallsOfFunction(F, CloneType);

    // It is okay to enable cloning for address taken routines but
    // disable it for now.
    if (FunctionAddressTaken) {
      if (IPCloningTrace)
        errs() << " Skipping address taken " << F.getName() << "\n";
      continue;
    }

    if (CloneType == SpecializationClone && CurrCallList.size() != 0) {
      if (CurrCallList.size() > IPSpeCloningNumCallSitesLimit) {
        if (IPCloningTrace)
          errs() << " Too many CallSites: Skipping Specialization cloning\n";
        continue;
      }
      // Transformation done here if Specialization cloning is kicked-in.
      cloneSpecializationFunction();
      continue;
    }

    if (FormalConstantValues.size() == 0 || CurrCallList.size() == 0) {
      if (IPCloningTrace)
        errs() << " Skipping non-candidate " << F.getName() << "\n";
      continue;
    }

    if (IPCloningTrace)
      dumpFormalsConstants(F);

    unsigned MaxClones = getMaxClones();
    unsigned MinClones = getMinClones();

    if (IPCloningTrace) {
      errs() << " Max clones:  " << MaxClones << "\n";
      errs() << " Min clones:  " << MinClones << "\n";
    }

    if (MaxClones <= 1 || MinClones > IPFunctionCloningLimit) {
      if (IPCloningTrace)
        errs() << " Skipping not worthy candidate " << F.getName() << "\n";
      continue;
    }

    // For a function that is recursive and for which we are producing a
    // generic clone, potentially relax the rules on the if-switch heuristic.
    bool IsGenRec = CloneType == GenericClone && isDirectlyRecursive(&F);
    bool IsGenRecQualified = false;
    if (!findWorthyFormalsForCloning(F, AfterInl, IFSwitchHeuristic, IsGenRec,
      &IsGenRecQualified)) {
      if (IPCloningTrace)
        errs() << " Skipping due to Heuristics " << F.getName() << "\n";
      continue;
    }

    if (!collectAllConstantArgumentsSets(F)) {
      if (IPCloningTrace)
        errs() << " Skipping not profitable candidate " << F.getName() << "\n";
      continue;
    }

    // If we are relaxing the rules on formals for a generic clone of a
    // recursive function, only clone if there is only one possible clone
    // and at least 'IPGenCloningMinRecCallsites' callsites.
    if (IsGenRecQualified && (FunctionAllArgumentsSets.size() != 1
      || CurrCallList.size() < IPGenCloningMinRecCallsites)) {
      if (IPCloningTrace)
        errs() << " Skipping not profitable recursive candidate "
               << F.getName() << "\n";
      continue;
    }

    cloneFunction();
  }

  if (IPCloningTrace)
    errs() << " Total clones:  " << NumIPCloned << "\n";

  if (NumIPCloned != 0)
    return true;

  return false;
}

static bool runIPCloning(Module &M, bool AfterInl, bool IFSwitchHeuristic) {
  bool Change = false;
  bool IFSwitchHeuristicOn = IFSwitchHeuristic || ForceIFSwitchHeuristic;
  Change = analysisCallsCloneFunctions(M, AfterInl, IFSwitchHeuristicOn);
  clearAllMaps();

  return Change;
}

namespace {

struct IPCloningLegacyPass : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  IPCloningLegacyPass(bool AfterInl = false, bool IFSwitchHeuristic = false)
      : ModulePass(ID), AfterInl(AfterInl),
        IFSwitchHeuristic(IFSwitchHeuristic) {
    initializeIPCloningLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<WholeProgramWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
    AU.addPreserved<InlineAggressiveWrapperPass>();
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    if (IPCloningAfterInl)
      AfterInl = true;
    return runIPCloning(M, AfterInl, IFSwitchHeuristic);
  }

private:
  // This flag helps to decide whether function addresses or other
  // constants need to be considered for cloning.
  bool AfterInl;
  // If 'true' enable cloning on routines with formals that feed a
  // sufficient number of if and switch values that will become constant.
  bool IFSwitchHeuristic;
};
}

char IPCloningLegacyPass::ID = 0;
INITIALIZE_PASS(IPCloningLegacyPass, "ip-cloning", "IP Cloning", false, false)


ModulePass *llvm::createIPCloningLegacyPass(bool AfterInl,
                                            bool IFSwitchHeuristic) {
  return new IPCloningLegacyPass(AfterInl, IFSwitchHeuristic);
}

IPCloningPass::IPCloningPass(bool AfterInl, bool IFSwitchHeuristic)
  : AfterInl(AfterInl), IFSwitchHeuristic(IFSwitchHeuristic) {}

PreservedAnalyses IPCloningPass::run(Module &M, ModuleAnalysisManager &AM) {
  if (!runIPCloning(M, AfterInl, IFSwitchHeuristic))
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses();
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<AndersensAA>();
  PA.preserve<InlineAggAnalysis>();
  return PA;
}

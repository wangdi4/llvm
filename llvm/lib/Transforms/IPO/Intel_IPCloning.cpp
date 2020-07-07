//===------- Intel_IPCloning.cpp - IP Cloning -*------===//
//
// Copyright (C) 2016-2020 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_IPCloningAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Intel_CloneUtils.h"
#include <sstream>
#include <string>
using namespace llvm;
using namespace PatternMatch;
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
  RecProgressionClone = 4,
  ManyRecCallsClone = 5
};
}

// Option to enable AfterInl IP Cloning, which is disabled by default.
// This option mainly for LIT tests to test AfterInl cloning
// without LTO.
static cl::opt<bool> IPCloningAfterInl("ip-cloning-after-inl", cl::init(false),
                                       cl::ReallyHidden);

// Maximum number of clones allowed for any routine.
static cl::opt<unsigned> IPFunctionCloningLimit("ip-function-cloning-limit",
                                                cl::init(3), cl::ReallyHidden);

// Enable Specialization cloning.
static cl::opt<bool> IPSpecializationCloning("ip-specialization-cloning",
                                             cl::init(true), cl::ReallyHidden);

// Maximum size of array allowed as constant argument for specialization clone.
static cl::opt<unsigned> IPSpecCloningArrayLimit("ip-spe-cloning-array-limit",
                                                 cl::init(80),
                                                 cl::ReallyHidden);

// Maximum number of specialization clones allowed at any CallSite.
static cl::opt<unsigned> IPSpeCloningCallLimit("ip-spe-cloning-call-limit",
                                               cl::init(4), cl::ReallyHidden);

// Maximum number of CallSites allowed for specialization for any routine.
static cl::opt<unsigned>
    IPSpeCloningNumCallSitesLimit("ip-spe-cloning-num-callsites-limit",
                                  cl::init(7), cl::ReallyHidden);

// Minimum allowed number of argument sets at any Callsite for specialization
// cloning.
static cl::opt<unsigned>
    IPSpeCloningMinArgSetsLimit("ip-spe-cloning-min-argsets-limit", cl::init(1),
                                cl::ReallyHidden);

// Used to force the enabling of the if-switch heuristics even when they
// would not normally be enabled.
static cl::opt<bool>
    ForceIFSwitchHeuristic("ip-gen-cloning-force-if-switch-heuristic",
                           cl::init(false), cl::ReallyHidden);

// Used to force the enabling of the dtrans-related heuristics even when they
// would not normally be enabled.
static cl::opt<bool> ForceEnableDTrans("ip-gen-cloning-force-enable-dtrans",
                                       cl::init(false), cl::ReallyHidden);

static cl::opt<bool> EnableMorphologyCloning("ip-gen-cloning-enable-morphology",
                                             cl::init(true), cl::ReallyHidden);

// Do not qualify a routine for cloning under the "if" heuristic unless we
// see at least this many "if" values that will be made constant.
static cl::opt<unsigned> IPGenCloningMinIFCount("ip-gen-cloning-min-if-count",
                                                cl::init(6), cl::ReallyHidden);

// Do not qualify a routine for cloning under the "switch" heuristic unless we
// see at least this many "switch" values that will be made constant.
static cl::opt<unsigned>
    IPGenCloningMinSwitchCount("ip-gen-cloning-min-switch-count", cl::init(6),
                               cl::ReallyHidden);

// Do not specially qualify a recursive routine for generic cloning unless we
// see at least this many formals that qualify under the "if-switch" heuristic.
static cl::opt<unsigned>
    IPGenCloningMinRecFormalCount("ip-gen-cloning-min-rec-formal-count",
                                  cl::init(2), cl::ReallyHidden);

// Do not specially qualify a recursive routine for generic cloning unless we
// see at least this many callsites to the routine.
static cl::opt<unsigned>
    IPGenCloningMinRecCallsites("ip-gen-cloning-min-rec-callsites",
                                cl::init(10), cl::ReallyHidden);

// Do not specially qualify a recursive routine for generic cloning unless we
// see at least this many callsites to the routine.
static cl::opt<unsigned> IPManyRecCallsCloningMinRecCallsites(
    "ip-manyreccalls-cloning-min-rec-callsites", cl::init(11),
    cl::ReallyHidden);

static cl::opt<bool> EnableManyRecCallsSplitting("ip-manyreccalls-splitting",
                                                 cl::init(true),
                                                 cl::ReallyHidden);

// Flag to set if we are doing LIT testing of just the splitting. In this
// case, any other ip cloning will be skipped, and the compiler will attempt
// to apply splitting to each Function for which we have IR.
static cl::opt<bool>
    ForceManyRecCallsSplitting("force-ip-manyreccalls-splitting",
                               cl::init(false), cl::ReallyHidden);

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
SmallDenseMap<unsigned, std::vector<std::pair<unsigned, Constant *>>>
    FunctionAllArgumentsSets;

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
SmallDenseMap<CallInst *,
              std::vector<std::vector<std::pair<unsigned, Value *>>>>
    AllCallsArgumentsSets;

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
SmallDenseMap<Value *, GetElementPtrInst *> SpecialConstGEPMap;

// It is mapping between Function and LoopInfo. It is used
// to avoid recomputing LoopInfo for a function each time
// when a CallSite of the function is analyzed.
SmallDenseMap<Function *, LoopInfo *> FunctionLoopInfoMap;

// Returns true if 'Arg' is considered as constant for
// cloning based on FuncPtrsClone.
static bool isConstantArgWorthyForFuncPtrsClone(Value *Arg) {
  Value *FnArg = Arg->stripPointerCasts();
  Function *Fn = dyn_cast<Function>(FnArg);

  if (Fn == nullptr)
    return false;
  // if it is function address, consider only if it has local definition.
  if (Fn->isDeclaration() || Fn->isIntrinsic() || !Fn->hasExactDefinition() ||
      !Fn->hasLocalLinkage() || Fn->hasExternalLinkage()) {
    return false;
  }
  return true;
}

// Returns true if 'Arg' is considered as constant for
// cloning based on GenericClone.
static bool isConstantArgWorthyForGenericClone(Value *Arg) {
  Value *FnArg = Arg->stripPointerCasts();
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
static void collectConstantArgument(Value *FormalV, Value *ActualV,
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
static void GetPointerToArrayDims(Type *PTy, unsigned &SizeInBytes,
                                  unsigned &NumElems, const DataLayout &DL) {

  if (!isPointerToCharArray(PTy))
    return;
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
static Value *isStartAddressOfPackedArrayOnStack(Value *V) {
  Instruction *I = cast<Instruction>(V);
  Value *AInst = I->getOperand(0);
  if (!isa<AllocaInst>(AInst))
    return nullptr;
  AllocaInst *AllocaI = cast<AllocaInst>(AInst);

  Value *StInst = nullptr;
  for (User *U : AInst->users()) {

    // Ignore if it is the arg that is passed to call.
    if (U == V)
      continue;

    if (isa<BitCastInst>(U)) {
      for (User *CI : U->users()) {
        IntrinsicInst *Callee = dyn_cast<IntrinsicInst>(CI);
        if (!Callee)
          return nullptr;
        if (Callee->getIntrinsicID() != Intrinsic::lifetime_start &&
            Callee->getIntrinsicID() != Intrinsic::lifetime_end)
          return nullptr;
      }
      continue;
    }

    if (!isa<StoreInst>(U))
      return nullptr;

    // More than one use is noticed
    if (StInst != nullptr)
      return nullptr;
    StInst = U;
  }
  if (StInst == nullptr)
    return nullptr;

  Value *ValOp = cast<StoreInst>(StInst)->getValueOperand();
  if (!isa<Constant>(ValOp))
    return nullptr;

  if (ValOp->getType() != AllocaI->getAllocatedType())
    return nullptr;

  return StInst;
}

// Returns true if 'V' is a Global Variable candidate for specialization
// cloning. 'I' is used to get DataLayout to compute sizes of types.
//
static bool isSpecializationGVCandidate(Value *V, Instruction *I) {
  GlobalVariable *GV;
  GV = dyn_cast<GlobalVariable>(V);
  if (!GV)
    return false;

  if (!GV->isConstant())
    return false;
  if (!GV->hasDefinitiveInitializer())
    return false;
  Constant *Init = GV->getInitializer();
  if (!isa<ConstantArray>(Init))
    return false;

  if (GV->getLinkage() != GlobalVariable::PrivateLinkage)
    return false;
  if (GV->hasComdat())
    return false;
  if (GV->isThreadLocal())
    return false;

  Type *Ty = GV->getValueType();
  if (!Ty->isSized())
    return false;
  const DataLayout &DL = I->getModule()->getDataLayout();
  if (DL.getTypeSizeInBits(Ty) > IPSpecCloningArrayLimit)
    return false;

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
static Value *isStartAddressOfGLobalArrayCopyOnStack(Value *V) {
  GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(V);
  if (!GEP)
    return nullptr;
  // First, check it is starting array address on stack
  Value *AInst = GEP->getOperand(0);
  if (!isa<AllocaInst>(AInst))
    return nullptr;

  AllocaInst *AllocaI = cast<AllocaInst>(AInst);
  if (!GEP->hasAllZeroIndices())
    return nullptr;

  const Value *AUse = nullptr;
  Type *GEPType = GEP->getSourceElementType();
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

  Value *GlobAddr = nullptr;
  for (const User *U : AUse->users()) {
    auto User = dyn_cast<CallInst>(U);
    if (!User)
      return nullptr;
    const IntrinsicInst *Callee = dyn_cast<IntrinsicInst>(U);
    if (!Callee)
      return nullptr;
    if (Callee->getIntrinsicID() == Intrinsic::lifetime_start ||
        Callee->getIntrinsicID() == Intrinsic::lifetime_end)
      continue;
    if (Callee->getIntrinsicID() != Intrinsic::memcpy)
      return nullptr;

    // Process Memcpy here
    if (User->getArgOperand(0) != AUse)
      return nullptr;
    Value *Dst = User->getArgOperand(1);
    auto *MemCpyDst = dyn_cast<GEPOperator>(Dst);
    if (!MemCpyDst)
      return nullptr;
    if (!MemCpyDst->hasAllZeroIndices())
      return nullptr;
    if (GEPType != MemCpyDst->getSourceElementType())
      return nullptr;
    if (MemCpyDst->getNumIndices() != MemCpySrc->getNumIndices())
      return nullptr;
    Value *MemCpySize = User->getArgOperand(2);

    // Make sure there is only one memcpy
    if (GlobAddr != nullptr)
      return nullptr;
    GlobAddr = MemCpyDst->getOperand(0);

    if (!isSpecializationGVCandidate(GlobAddr, GEP))
      return nullptr;

    const DataLayout &DL = GEP->getModule()->getDataLayout();
    unsigned ArraySize = DL.getTypeSizeInBits(GEPType) / 8;
    ConstantInt *CI = dyn_cast<ConstantInt>(MemCpySize);
    if (!CI)
      return nullptr;
    if (!CI->equalsInt(ArraySize))
      return nullptr;
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
static bool isSpecializationCloningSpecialConst(Value *V, Value *Arg) {
  Value *PropVal = nullptr;

  if (isa<GetElementPtrInst>(V)) {
    PropVal = isStartAddressOfGLobalArrayCopyOnStack(V);
  } else if (isa<BitCastInst>(V)) {
    PropVal = isStartAddressOfPackedArrayOnStack(V);
  } else {
    return false;
  }
  if (PropVal == nullptr)
    return false;

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
static void
collectArgsSetsForSpecialization(Function &F, CallInst &CI,
                                 SmallPtrSet<Value *, 8> &PhiValues) {

  std::vector<std::vector<std::pair<unsigned, Value *>>> CallArgumentsSets;
  std::vector<std::pair<unsigned, Value *>> ConstantArgs;
  CallArgumentsSets.clear();

  auto PHI_I = cast<Instruction>(*PhiValues.begin());
  // Skip CallSite if BasicBlock has too many preds.
  if (cast<PHINode>(PHI_I)->getNumIncomingValues() > IPSpeCloningCallLimit) {
    LLVM_DEBUG(dbgs() << "     More Preds ... Skipped Spe cloning  "
                      << "\n");
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
    for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end(); AI != E;
         ++AI, ++CAI1, ++Position) {

      if (!PhiValues.count(*CAI1))
        continue;

      auto PHI = cast<PHINode>(*CAI1);
      Value *C = PHI->getIncomingValueForBlock(PredBB);
      if (isa<Constant>(C) || isSpecializationCloningSpecialConst(C, *CAI1)) {
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
    LLVM_DEBUG(dbgs() << "     Not enough sets... Skipped Spe cloning  "
                      << "\n");
    return;
  }

  // Map CallArgumentsSets to CI here.
  auto &ACallArgs = AllCallsArgumentsSets[&CI];
  std::copy(CallArgumentsSets.begin(), CallArgumentsSets.end(),
            std::back_inserter(ACallArgs));

  CurrCallList.push_back(&CI);

  // Dump arg sets
  LLVM_DEBUG({
    dbgs() << "    Args sets collected \n";
    if (InexactArgsSetsCallList.count(&CI)) {
      dbgs() << "    Inexact args sets found \n";
    }
    for (unsigned index = 0; index < CallArgumentsSets.size(); index++) {
      dbgs() << "   Set_" << index << "\n";
      auto CArgs = CallArgumentsSets[index];
      for (auto I = CArgs.begin(), E = CArgs.end(); I != E; I++) {
        dbgs() << "      position: " << I->first << " Value " << *(I->second)
               << "\n";
      }
    }
  });
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
    LLVM_DEBUG(dbgs() << "   Specialization cloning disabled \n");
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
  LoopInfo *LI = FunctionLoopInfoMap[&F];
  if (!LI)
    delete LI;
}

// Look at all CallSites of 'F' and collect all constant values
// of formals. Return true if use of 'F' is noticed as non-call.
static bool analyzeAllCallsOfFunction(Function &F, IPCloneKind CloneType) {
  bool FunctionAddressTaken = false;

  if (CloneType == SpecializationClone) {
    LLVM_DEBUG(dbgs() << " Processing for Spe cloning  " << F.getName()
                      << "\n");
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
    for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end(); AI != E;
         ++AI, ++CAI) {
      collectConstantArgument(&*AI, *CAI, CloneType);
    }
  }
  return FunctionAddressTaken;
}

// Returns true if it a candidate for function-ptr cloning.
// Returns true if it has at least one formal of function pointer type.
//
static bool IsFunctionPtrCloneCandidate(Function &F) {
  for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end(); AI != E;
       ++AI) {
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

// BEGIN: Special code for extra clone transformation

//
// Return 'true' if 'AI' represents an AllocaInst similar to:
//   %x = alloca [9 x i32], align 16
// Here the size of the array, the number of bits in the integer, and the
// alignment do not effect whether we return 'true'.  When we return 'true',
// we set '*ArrayLengthOut' to the number of elements in the array. (In the
// above example, that will be 9.)
//
static bool isRecProAllocaIntArray(AllocaInst *AI, int *ArrayLengthOut) {
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
static bool isRecProLatchBlock(bool TestSimpleOnly, BasicBlock *BBLoopHeader,
                               BasicBlock *BBLatch, bool *IsSimple) {
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
                                BasicBlock *BBLoopHeader, bool TestSimpleOnly,
                                int *LowerBoundOut, int *UpperBoundOut,
                                PHINode **PHIOut, bool *IsSimpleOut,
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
// Return 'true' if 'GEPI' is a GEP with 2 zero indices that indexes 'AI'.
//
static bool isRecProGEP(GetElementPtrInst *GEPI, AllocaInst *AI) {
  return GEPI && GEPI->getPointerOperand() == AI && GEPI->hasAllZeroIndices() &&
         GEPI->getNumIndices() == 2;
}

//
// Return 'true' if 'SubI' is a SubscriptInst which indexes 'GEPI' and
// whose Rank is 0, LowerBound is 1, Stride is 4, as is required in the
// RecProVectors.
//
static bool isRecProSub(SubscriptInst *SubI, GetElementPtrInst *GEPI) {
  if (SubI->getRank() != 0)
    return false;
  auto CI1 = dyn_cast<ConstantInt>(SubI->getLowerBound());
  if (!CI1 || CI1->getSExtValue() != 1)
    return false;
  auto CI2 = dyn_cast<ConstantInt>(SubI->getStride());
  if (!CI2 || CI2->getSExtValue() != 4)
    return false;
  auto GEPII = dyn_cast<GetElementPtrInst>(SubI->getPointerOperand());
  if (!GEPII || GEPII != GEPI)
    return false;
  return true;
}

//
// Return 'true' if 'SI' is a SubscriptInst which indexes a local single-
// dimension 9 element integer array with the induction variable 'PHI'.
// If we return 'true', set '*AIOut' to the AllocaInst that allocates the
// memory for that array.
//
static bool isRecProTempVector(SubscriptInst *SI, PHINode *PHI,
                               AllocaInst **AIOut) {
  auto GEPI = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
  if (!GEPI || !isRecProSub(SI, GEPI))
    return false;
  auto AI = dyn_cast<AllocaInst>(GEPI->getPointerOperand());
  if (!AI || !isRecProGEP(GEPI, AI))
    return false;
  int ArraySize = 0;
  if (!isRecProAllocaIntArray(AI, &ArraySize))
    return false;
  if (ArraySize != 9)
    return false;
  if (SI->getIndex() != PHI)
    return false;
  *AIOut = AI;
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
                               BasicBlock *BBLoopHeader, AllocaInst **AIOut,
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
static bool isRecProComplexCond(BasicBlock *BBLoopHeader, AllocaInst *AICond,
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
static bool hasRecProComplexTest(BasicBlock *BBLoopHeader, AllocaInst *AICond,
                                 PHINode *PHI, bool IsTrue, StoreInst **SIOut,
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
static bool
isRecProTrueBranchComplexLoop(BasicBlock *BBPreHeader, BasicBlock *BBLoopHeader,
                              AllocaInst *AICond, AllocaInst **AIOut,
                              StoreInst **SIOut, int *ConstantValueOut,
                              BasicBlock **BBLatchOut, BasicBlock **BBExitOut) {
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
  *SIOut = SI;
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
// 'AICond' is "t", '*AIOut' is "u", and '*SIOut' is the store of "u(i)".
//  If 'VLoad' is not nullptr, ensure that the value assigned to '*AIOut' is
// 'VLoad'. When we return 'true', we set '*AIOut' as well as the latch block
// '*BBLatchOut' and '*BBExitOut'.
//
static bool isRecProFalseBranchComplexLoop(
    BasicBlock *BBPreHeader, BasicBlock *BBLoopHeader, AllocaInst *AICond,
    AllocaInst *VLoad, AllocaInst **AIOut, StoreInst **SIOut,
    BasicBlock **BBLatchOut, BasicBlock **BBExitOut) {
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
    *SIOut = SI;
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
  *SIOut = SI;
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
// set '*BBLastLatchOut' to the last latch block in the sequence, set
// '*LowerValueOut' and '*UpperValueOut' to the AllocaInsts representing
// 'l(9)' and 'u(9)' and '*LowerConstantValueOut' and '*UpperConstantValueOut'
// to the constant values "constant_l" and "constant_u". Also, '*SILower1' and
// '*SILower2' are set to the stores to "l(i)" and '*SIUpper1' and '*SIUpper2'
// are set to the stores to "u(i)".  In both cases, the "lower" store is the
// one that appears first in the five loop sequence, while the "upper" store
// is the one that appears second.
//
static bool isRecProSpecialLoopSequence(
    Function *F, BasicBlock **BBLastExitOut, BasicBlock **BBLastLatchOut,
    AllocaInst **LowerValueOut, AllocaInst **UpperValueOut,
    StoreInst **SILower1, StoreInst **SILower2, StoreInst **SIUpper1,
    StoreInst **SIUpper2, int *LowerConstantValueOut,
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
                                      AICond, nullptr, &VStoreLower, SILower1,
                                      &BBLatch, &BBExit))
    return false;
  if (!isRecProFalseBranchComplexLoop(BBExit, BBExit->getSingleSuccessor(),
                                      AICond, VStoreLower, &VStoreUpper,
                                      SIUpper1, &BBLatch, &BBExit))
    return false;
  if (!isRecProTrueBranchComplexLoop(BBLatch, BBExit, AICond, &VStoreLowerTemp,
                                     SILower2, &LowerBound, &BBLatch, &BBExit))
    return false;
  if (VStoreLower != VStoreLowerTemp)
    return false;
  if (!isRecProTrueBranchComplexLoop(BBLatch, BBExit, AICond, &VStoreUpperTemp,
                                     SIUpper2, &UpperBound, &BBLatch, &BBExit))
    return false;
  if (VStoreUpper != VStoreUpperTemp)
    return false;
  *BBLastLatchOut = BBLatch;
  *BBLastExitOut = BBExit;
  *LowerValueOut = VStoreLower;
  *UpperValueOut = VStoreUpper;
  *LowerConstantValueOut = LowerBound;
  *UpperConstantValueOut = UpperBound;
  return true;
}

//
// Return a GetElementPtrInst which represents the address of 'AI'. If there
// is one in the IR already, return that, otherwise, create one. (Note that
// 'AI' should represent a single dimension integer array.)
//
static GetElementPtrInst *findOrCreateRecProGEP(AllocaInst *AI,
                                                BasicBlock *BB) {
  for (User *U : AI->users()) {
    auto GEPI = dyn_cast<GetElementPtrInst>(U);
    if (GEPI && isRecProGEP(GEPI, AI))
      return GEPI;
  }
  SmallVector<Value *, 2> Indices;
  auto Int64Ty = Type::getInt64Ty(BB->getContext());
  auto CI1 = ConstantInt::get(Int64Ty, 0, true);
  Indices.push_back(CI1);
  auto CI2 = ConstantInt::get(Int64Ty, 0, true);
  Indices.push_back(CI2);
  Type *GEPT = AI->getType()->getElementType();
  return GetElementPtrInst::Create(GEPT, AI, Indices, "", BB);
}

//
// Insert special code of the form:
//   if (l(8) == u(8)) then
//     call FClone(row)
//     return
//   endif
//   l(8) = CVLower
//   u(8) = CVUpper
// into 'F' after 'BBI'. Here 'AILower' is "l()", 'AIUpper' is "u()".
// 'FClone' is the extra clone of the extra clone transformation.
//
static void addSpecialRecProCloneCode(Function *F, Function *FClone,
                                      BasicBlock *BBLastExit,
                                      BasicBlock *BBLastLatch,
                                      AllocaInst *AILower, AllocaInst *AIUpper,
                                      int CVLower, int CVUpper) {
  assert(F && FClone && BBLastExit && BBLastLatch && AILower && AIUpper &&
         "Expect values to be defined in caller.");
  BasicBlock *BBCond = BasicBlock::Create(F->getContext(), "CondBlock", F);
  BBCond->moveAfter(BBLastExit);
  BranchInst *BBBIT = cast<BranchInst>(BBLastLatch->getTerminator());
  for (unsigned I = 0; I < BBBIT->getNumSuccessors(); ++I)
    if (BBBIT->getSuccessor(I) == BBLastExit)
      BBBIT->setSuccessor(I, BBCond);
  BasicBlock *BBCallClone =
      BasicBlock::Create(F->getContext(), "CallCloneBlock", F);
  BasicBlock *BBConstStore =
      BasicBlock::Create(F->getContext(), "ConstStore", F);
  BBConstStore->moveBefore(BBLastExit);
  BBCallClone->moveBefore(BBConstStore);
  BBCond->moveBefore(BBCallClone);
  IRBuilder<> Builder(BBCond);
  GetElementPtrInst *GEPL = findOrCreateRecProGEP(AILower, BBCond);
  auto Int64Ty = Type::getInt64Ty(F->getContext());
  Instruction *SILB = Builder.CreateSubscript(
      0, ConstantInt::get(Int64Ty, 1), ConstantInt::get(Int64Ty, 4), GEPL,
      ConstantInt::get(Int64Ty, 8));
  auto LILBType = SILB->getType()->getPointerElementType();
  LoadInst *LILB8 = Builder.CreateLoad(LILBType, SILB, "LILB8");
  GetElementPtrInst *GEPU = findOrCreateRecProGEP(AIUpper, BBCond);
  Instruction *SIUB = Builder.CreateSubscript(
      0, ConstantInt::get(Int64Ty, 1), ConstantInt::get(Int64Ty, 4), GEPU,
      ConstantInt::get(Int64Ty, 8));
  auto SIUBType = SIUB->getType()->getPointerElementType();
  LoadInst *LIUB8 = Builder.CreateLoad(SIUBType, SIUB, "LIUB8");
  Value *CmpI = Builder.CreateICmpEQ(LILB8, LIUB8, "CMP8S");
  Builder.CreateCondBr(CmpI, BBCallClone, BBConstStore);
  Builder.SetInsertPoint(BBCallClone);
  SmallVector<Value *, 4> Args;
  for (Argument &Arg : F->args())
    Args.push_back(&Arg);
  Builder.CreateCall(FClone, Args);
  Builder.CreateRetVoid();
  Builder.SetInsertPoint(BBConstStore);
  Constant *C1 =
      ConstantInt::get(SILB->getType()->getPointerElementType(), CVLower);
  Builder.CreateStore(C1, SILB);
  Constant *C9 =
      ConstantInt::get(SIUB->getType()->getPointerElementType(), CVUpper);
  Builder.CreateStore(C9, SIUB);
  Builder.CreateBr(BBLastExit);
  LLVM_DEBUG({
    dbgs() << "Inserting special extra clone test:\n";
    BBCond->dump();
    BBCallClone->dump();
    BBConstStore->dump();
  });
}

//
// Return 'true' if 'AI' is only used in SubscriptInsts, which are then
// fed to LoadInsts and StoreInsts. Futhermore, the StoreInsts must be
// exactly those on 'SV'. If we return 'true', we fill 'LV' with the
// LoadInsts which refer to 'AI'.
//
// We use this routine to check that there are no unusual aliases generated
// for 'AI'. If the StoreInsts in 'SV' dominate all LoadInsts for 'AI', we
// can perform forward substitution on the LoadInsts in 'LV'.
//
static bool validateRecProVectorMemOps(AllocaInst *AI,
                                       SmallVectorImpl<StoreInst *> &SV,
                                       SmallVectorImpl<LoadInst *> &LV) {
  for (User *U1 : AI->users()) {
    auto GEPI = dyn_cast<GetElementPtrInst>(U1);
    if (!GEPI || !isRecProGEP(GEPI, AI)) {
      LV.clear();
      return false;
    }
    for (User *U2 : GEPI->users()) {
      auto SubI = dyn_cast<SubscriptInst>(U2);
      if (!SubI || !isRecProSub(SubI, GEPI)) {
        LV.clear();
        return false;
      }
      for (User *U3 : SubI->users()) {
        auto LI = dyn_cast<LoadInst>(U3);
        if (LI)
          LV.push_back(LI);
        else {
          auto SI = dyn_cast<StoreInst>(U3);
          if (SI) {
            unsigned I = 0;
            for (; I < SV.size(); ++I)
              if (SV[I] == SI)
                break;
            if (I == SV.size()) {
              LV.clear();
              return false;
            }
          } else {
            LV.clear();
            return false;
          }
        }
      }
    }
  }
  return true;
}

//
// Return 'true' if 'LI' is a subscripted load of the 'Index'th element
// of some value.
//
static bool hasThisRecProSubscript(LoadInst *LI, unsigned Index) {
  auto SubI = dyn_cast<SubscriptInst>(LI->getPointerOperand());
  if (!SubI)
    return false;
  auto CI = dyn_cast<ConstantInt>(SubI->getIndex());
  return CI && CI->getZExtValue() == Index;
}

//
// Return 'true' if, given that 'S1' and S2' dominate all loads of 'AI'
// in 'F', we can forward substitute 'Bound' for the 8th element of 'AI'.
// If we return 'true', do the forward substitution.
//
static bool tryToMakeRecProSubscriptsConstant(Function *F, AllocaInst *AI,
                                              StoreInst *S1, StoreInst *S2,
                                              int Bound) {
  SmallVector<StoreInst *, 2> SV;
  SmallVector<LoadInst *, 10> LV;
  SV.push_back(S1);
  SV.push_back(S2);
  if (!validateRecProVectorMemOps(AI, SV, LV))
    return false;
  for (unsigned I = 0; I < LV.size(); ++I) {
    auto LI = LV[I];
    if (hasThisRecProSubscript(LI, 8)) {
      auto CCI = ConstantInt::get(LI->getType(), Bound);
      LLVM_DEBUG({
        dbgs() << "Replacing Load in Function " << F->getName() << " with "
               << Bound << "\n";
        LI->dump();
        CCI->dump();
      });
      LI->replaceAllUsesWith(CCI);
    }
  }
  return true;
}

//
// Return 'true' if, given that 'SILower1' and 'SILower2'' dominate all loads
// of 'AILower', and 'SIUpper1' and 'SIUpper2' dominate all loads of 'AIUpper',
// in 'F', we can forward substitute the 8th element of 'AILower' for the 8th
// element of 'AIUpper'. If we return 'true', do the forward substitution.
//
static bool tryToMakeRecProSubscriptsSame(
    Function *F, AllocaInst *AILower, AllocaInst *AIUpper, StoreInst *SILower1,
    StoreInst *SILower2, StoreInst *SIUpper1, StoreInst *SIUpper2) {
  SmallVector<StoreInst *, 2> SV;
  SmallVector<LoadInst *, 10> LV;
  SV.push_back(SILower1);
  SV.push_back(SILower2);
  if (!validateRecProVectorMemOps(AILower, SV, LV))
    return false;
  LoadInst *LILower = nullptr;
  for (unsigned I = 0; I < LV.size(); ++I) {
    auto LI = LV[I];
    if (hasThisRecProSubscript(LI, 8))
      LILower = LI;
  }
  if (!LILower)
    return false;
  SV.clear();
  LV.clear();
  SV.push_back(SIUpper1);
  SV.push_back(SIUpper2);
  if (!validateRecProVectorMemOps(AIUpper, SV, LV))
    return false;
  for (unsigned I = 0; I < LV.size(); ++I) {
    auto LI = LV[I];
    if (hasThisRecProSubscript(LI, 8)) {
      LLVM_DEBUG({
        dbgs() << "Replacing Load in Function " << F->getName()
               << " with alternate bound\n";
        LI->dump();
        LI->getPointerOperand()->dump();
        LILower->dump();
        LILower->getPointerOperand()->dump();
      });
      LI->replaceAllUsesWith(LILower);
    }
  }
  return true;
}

// END: Special code for extra clone transformation

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
// Also, in the non-cyclic case, under certain special circumstances,
// create an extra clone.
static void createRecProgressionClones(Function &F, unsigned ArgPos,
                                       unsigned Count, int Start, int Inc,
                                       bool IsByRef, bool IsCyclic) {
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
    LLVM_DEBUG({
      dbgs() << "        Function: " << NewF->getName() << "\n";
      dbgs() << "        ArgPos : " << ArgPos << "\n";
      dbgs() << "        Argument : " << *NewFormal << "\n";
      dbgs() << "        IsByRef : " << (IsByRef ? "T" : "F") << "\n";
      dbgs() << "        Replacement:  " << *Rep << "\n";
    });
    if (IsByRef) {
      assert(NewFormal->hasOneUse() && "Expecting single use of ByRef Formal");
      auto LI = cast<LoadInst>(*(NewFormal->user_begin()));
      LI->replaceAllUsesWith(Rep);
    } else
      NewFormal->replaceAllUsesWith(Rep);
    if (!FirstCloneF)
      FirstCloneF = NewF;
    LastCloneF = NewF;
  }
  if (IsCyclic)
    fixRecProgressionRecCalls(F, *LastCloneF, *FirstCloneF);
  else {
    BasicBlock *BBLastExit = nullptr;
    BasicBlock *BBLastLatch = nullptr;
    AllocaInst *AILower = nullptr;
    AllocaInst *AIUpper = nullptr;
    StoreInst *SILower1 = nullptr;
    StoreInst *SILower2 = nullptr;
    StoreInst *SIUpper1 = nullptr;
    StoreInst *SIUpper2 = nullptr;
    int CVLower = 0;
    int CVUpper = 0;
    // Test if we should create the extra clone
    Function *ExtraCloneF = nullptr;
    if (isRecProSpecialLoopSequence(LastCloneF, &BBLastExit, &BBLastLatch,
                                    &AILower, &AIUpper, &SILower1, &SILower2,
                                    &SIUpper1, &SIUpper2, &CVLower, &CVUpper)) {
      // Create the extra clone
      ValueToValueMapTy VMapNew;
      ExtraCloneF = CloneFunction(LastCloneF, VMapNew);
      ExtraCloneF->addFnAttr("prefer-inline-rec-pro-clone");
      ExtraCloneF->addFnAttr("contains-rec-pro-clone");
      LLVM_DEBUG(dbgs() << "Extra RecProClone Candidate: "
                        << LastCloneF->getName() << "\n");
    }
    deleteRecProgressionRecCalls(F, *LastCloneF);
    if (ExtraCloneF) {
      int SubCount = 0;
      // Make the inner loop bounds constant for the last normal clone
      if (tryToMakeRecProSubscriptsConstant(LastCloneF, AILower, SILower1,
                                            SILower2, 1))
        ++SubCount;
      if (tryToMakeRecProSubscriptsConstant(LastCloneF, AIUpper, SIUpper1,
                                            SIUpper2, 9))
        ++SubCount;
      // Make the inner loop "trip 1" for the extra clone
      BasicBlock *BBLastExitC = nullptr;
      BasicBlock *BBLastLatchC = nullptr;
      AllocaInst *AILowerC = nullptr;
      AllocaInst *AIUpperC = nullptr;
      StoreInst *SILower1C = nullptr;
      StoreInst *SILower2C = nullptr;
      StoreInst *SIUpper1C = nullptr;
      StoreInst *SIUpper2C = nullptr;
      int CVLowerC = 0;
      int CVUpperC = 0;
      if (isRecProSpecialLoopSequence(ExtraCloneF, &BBLastExitC, &BBLastLatchC,
                                      &AILowerC, &AIUpperC, &SILower1C,
                                      &SILower2C, &SIUpper1C, &SIUpper2C,
                                      &CVLowerC, &CVUpperC)) {
        if (tryToMakeRecProSubscriptsSame(ExtraCloneF, AILowerC, AIUpperC,
                                          SILower1C, SILower2C, SIUpper1C,
                                          SIUpper2C))
          ++SubCount;
      }
      // Add the special test in the last normal clone to switch to the
      // extra clone if appropriate
      addSpecialRecProCloneCode(LastCloneF, ExtraCloneF, BBLastExit,
                                BBLastLatch, AILower, AIUpper, CVLower,
                                CVUpper);
      deleteRecProgressionRecCalls(F, *ExtraCloneF);
      LLVM_DEBUG({
        if (SubCount == 3)
          dbgs() << "All desired subscript bounds substituted\n";
      });
    }
  }
}

// Create argument set for CallInst 'CI' of  'F' and save it in
// 'ConstantArgsSet'
//
static void createConstantArgumentsSet(
    CallInst &CI, Function &F,
    std::vector<std::pair<unsigned, Constant *>> &ConstantArgsSet) {

  unsigned position = 0;
  auto CAI = CI.arg_begin();
  for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end(); AI != E;
       ++AI, ++CAI, position++) {

    // Ignore formals that are not selected by heuristics to reduce
    // code size, compile-time etc
    if (!WorthyFormalsForCloning.count(&*AI))
      continue;

    Value *ActualV = *CAI;
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
    std::vector<std::pair<unsigned, Constant *>> &ConstantArgs) {
  auto I = FunctionAllArgumentsSets.begin();
  auto E = FunctionAllArgumentsSets.end();
  unsigned index = 0;
  for (; I != E; I++) {
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
  for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end(); AI != E;
       ++AI, position++) {

    auto CList = FormalConstantValues[&*AI];
    dbgs() << "         Formal_" << position << ":";
    if (InexactFormals.count(&*AI))
      dbgs() << "  (Inexact)  \n";
    else
      dbgs() << "  (Exact)  \n";

    // Dump list of constants
    for (auto I = CList.begin(), E = CList.end(); I != E; I++) {
      dbgs() << "                  " << *(*(&*I)) << "\n";
    }
  }
  dbgs() << "\n\n";
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
  for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end(); AI != E;
       ++AI) {

    Value *V = &*AI;
    f_count++;

    // Ignore formal if it doesn't have any constants at call-sites
    auto &ValList = FormalConstantValues[V];
    if (ValList.size() == 0)
      continue;

    LLVM_DEBUG({
      dbgs() << " Collecting potential constants for Formal_";
      dbgs() << (f_count - 1) << "\n";
    });
    if (AfterInl || IsGenRec) {
      unsigned IFCount = 0;
      unsigned SwitchCount = 0;
      if (findPotentialConstsAndApplyHeuristics(
              F, V, &LI, true, IFSwitchHeuristic, &IFCount, &SwitchCount)) {
        if (IFCount + SwitchCount == 0) {
          // Qualified unconditionally under the loop heuristic.
          WorthyFormalsForCloning.insert(V);
          LLVM_DEBUG(dbgs() << "  Selecting FORMAL_" << (f_count - 1) << "\n");
        } else {
          // Qualified under the if-switch heuristic. Mark the formal as
          // pending for now, and qualify it later if the total number of
          // "if" and "switch" values that become constant is great enough.
          SawPending = true;
          GlobalIFCount += IFCount;
          GlobalSwitchCount += SwitchCount;
          PossiblyWorthyFormalsForCloning.insert(V);
          LLVM_DEBUG({
            dbgs() << "  Pending FORMAL_" << (f_count - 1) << "\n";
            dbgs() << "    IFCount " << GlobalIFCount << " <- " << IFCount
                   << "\n";
            dbgs() << "    SwitchCount " << GlobalSwitchCount << " <- "
                   << SwitchCount << "\n";
          });
        }
      } else {
        LLVM_DEBUG({
          dbgs() << "  Skipping FORMAL_" << (f_count - 1);
          dbgs() << " due to heuristics\n";
        });
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
    LLVM_DEBUG(dbgs() << "  Selecting all Pending FORMALs\n");
    for (Value *W : PossiblyWorthyFormalsForCloning)
      WorthyFormalsForCloning.insert(W);
  } else if (IsGenRec && PossiblyWorthyFormalsForCloning.size() >=
                             IPGenCloningMinRecFormalCount) {
    LLVM_DEBUG(dbgs() << "  Possibly selecting all Pending FORMALs in "
                      << "Recursive Function\n");
    for (Value *W : PossiblyWorthyFormalsForCloning)
      WorthyFormalsForCloning.insert(W);
    *IsGenRecQualified = true;
  } else if (SawPending) {
    LLVM_DEBUG({
      if (GlobalIFCount < IPGenCloningMinIFCount)
        dbgs() << "  IFCount (" << GlobalIFCount << ") < Limit ("
               << IPGenCloningMinIFCount << ")\n";
      if (GlobalSwitchCount < IPGenCloningMinSwitchCount)
        dbgs() << "  SwitchCount (" << GlobalSwitchCount << ") < Limit ("
               << IPGenCloningMinSwitchCount << ")\n";
    });
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
      LLVM_DEBUG(dbgs() << "     Exceeding number of argument sets limit \n");
      return false;
    }
  }
  if (FunctionAllArgumentsSets.size() == 0) {
    LLVM_DEBUG(dbgs() << "     Zero argument sets found \n");
    return false;
  }
  LLVM_DEBUG({
    dbgs() << "    Number of argument sets found: ";
    dbgs() << FunctionAllArgumentsSets.size() << "\n";
  });

  return true;
}

// Returns true if there is a constant value in 'CArgs' at 'position'.
//
static bool isArgumentConstantAtPosition(
    std::vector<std::pair<unsigned, Constant *>> &CArgs, unsigned position) {
  for (auto I = CArgs.begin(), E = CArgs.end(); I != E; I++) {
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
                              E = ClonedFn->arg_end();
       AI != E; ++AI, ++CAI, position++) {

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
  for (inst_iterator II = inst_begin(ClonedFn), E = inst_end(ClonedFn); II != E;
       ++II) {
    if (!isa<CallInst>(&*II))
      continue;
    auto CI = cast<CallInst>(&*II);
    Function *Callee = CI->getCalledFunction();
    if (Callee == OriginalFn && okayEliminateRecursion(ClonedFn, index, *CI)) {
      CI->setCalledFunction(ClonedFn);
      NumIPCallsCloned++;
      LLVM_DEBUG(dbgs() << " Replaced Cloned call:   " << *CI << "\n");
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
    Function *SrcFn = CI->getCalledFunction();

    // Get cloned function for constant argument set if it is already there
    unsigned index = CallInstArgumentSetIndexMap[CI];
    Function *NewFn = ArgSetIndexClonedFunctionMap[index];

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
    LLVM_DEBUG(dbgs() << " Cloned call:   " << *CI << "\n");
  }
}

// Returns true if there is a specialization constant value
// in 'CArgs' at 'Position'.
//
static Value *isSpecializationConstantAtPosition(
    std::vector<std::pair<unsigned, Value *>> &CArgs, unsigned Position) {

  for (auto I = CArgs.begin(), E = CArgs.end(); I != E; I++) {
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
static Value *createGEPAtFrontInClonedFunction(Function *NewFn, Value *BaseAddr,
                                               unsigned NumIndices) {

  Type *Int32Ty;
  Value *Rep;
  SmallVector<Value *, 4> Indices;

  Instruction *InsertPt = &NewFn->begin()->front();
  Int32Ty = Type::getInt32Ty(NewFn->getContext());
  // Create Indices with zero value.
  for (unsigned I = 0; I < NumIndices; I++)
    Indices.push_back(ConstantInt::get(Int32Ty, 0));

  Rep = GetElementPtrInst::CreateInBounds(BaseAddr, Indices, "", InsertPt);
  LLVM_DEBUG(dbgs() << "     Created New GEP: " << *Rep << "\n");

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
static GlobalVariable *
createGlobalVariableWithInit(Function *NewFn, uint64_t Number,
                             Instruction *CallI, unsigned Cols, unsigned Rows,
                             unsigned &Counter) {

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
    for (unsigned J = 0; J < Rows; J++) {
      ArrayVec.push_back(ConstantInt::get(Type::getInt8Ty(NewFn->getContext()),
                                          Number & 0xFF));
      // Shift Number by size of Int8Ty
      Number = Number >> 8;
    }
    ArrayArrayVec.push_back(ConstantArray::get(ArrayTy, ArrayVec));
  }

  // Create New Global Variable and set Initializer
  Module *M = CallI->getModule();
  auto *NewGlobal = new GlobalVariable(
      *M, ArrayArrayTy,
      /*isConstant=*/true, GlobalValue::PrivateLinkage, nullptr,
      NewFn->getName() + ".clone." + Twine(Counter));

  NewGlobal->setInitializer(ConstantArray::get(ArrayArrayTy, ArrayArrayVec));
  Counter++;
  LLVM_DEBUG(dbgs() << "     Created New Array:  " << *NewGlobal << "\n");
  return NewGlobal;
}

// For given specialization constant 'V', it gets/creates Value that needs
// to be propagated to 'NewFn'. 'Formal' is used to get type info of
// argument. 'CallI' and 'DL' are used to get Module and size info.
//
static Value *getReplacementValueForArg(Function *NewFn, Value *V,
                                        Value *Formal, Instruction *CallI,
                                        const DataLayout &DL,
                                        unsigned &Counter) {

  // Case 0:
  //   It is plain constant. Just returns the same.
  if (isa<Constant>(V))
    return V;

  Value *PropValue = nullptr;
  ;
  PropValue = SpecialConstPropagatedValueMap[V];

  // If it is not constant, there are two possible values that need
  // to be propagated.
  // Case 1:
  //        store i64 72340172821299457, i64* %6, align 8
  //
  //  Case 2:
  //   getelementptr inbounds ([5 x [2 x i8]], [5 x [2 x i8]]* @i.CM_THREE
  //

  Value *Rep;
  GetElementPtrInst *GEP = SpecialConstGEPMap[V];
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
  Value *Val = cast<StoreInst>(PropValue)->getOperand(0);
  ConstantInt *CI = cast<ConstantInt>(Val);

  GetPointerToArrayDims(Formal->getType(), SizeInBytes, NumElems, DL);
  assert((SizeInBytes > 0) && "Expects pointer to Array Type");

  // Create New GlobalVariable
  auto *NewGlobal = createGlobalVariableWithInit(
      NewFn, CI->getZExtValue(), CallI,
      CI->getBitWidth() / SizeInBytes /* cols */, NumElems /* rows */, Counter);

  // Create GEP Inst in cloned function
  Rep = createGEPAtFrontInClonedFunction(NewFn, NewGlobal, NumIndices);
  return Rep;
}

// It propagates all constant arguments to clone function 'NewFn'.
// 'ArgsIndex' is used to get ArgumentSet for 'NewFn'.
// 'CallI' helps to get Module in case if GlobalVariable needs to
// be created.
//
static void propagateArgumentsToClonedFunction(Function *NewFn,
                                               unsigned ArgsIndex,
                                               CallInst *CallI) {
  unsigned Position = 0;
  unsigned Counter = 0;
  Value *Rep;
  auto &CallArgsSets = AllCallsArgumentsSets[CallI];
  auto CArgs = CallArgsSets[ArgsIndex];
  const DataLayout &DL = CallI->getModule()->getDataLayout();

  for (Function::arg_iterator AI = NewFn->arg_begin(), EI = NewFn->arg_end();
       AI != EI; ++AI, Position++) {

    Value *V = isSpecializationConstantAtPosition(CArgs, Position);
    if (V == nullptr)
      continue;

    Value *Formal = &*AI;

    Rep = getReplacementValueForArg(NewFn, V, Formal, CallI, DL, Counter);

    LLVM_DEBUG({
      dbgs() << "        Formal : " << *AI << "\n";
      dbgs() << "        Value : " << *V << "\n";
      dbgs() << "        Replacement:  " << *Rep << "\n";
    });

    Formal->replaceAllUsesWith(Rep);
  }
}

//
// Create a new call instruction for a clone of 'CI' and insert it in
// 'Insert_BB'. Return a CallInst* for that new call instruction.
// NewCall is created for 'ArgsIndex', which is the index of argument-sets
// of CI.
//
static CallInst *createNewCall(CallInst &CI, BasicBlock *Insert_BB,
                               unsigned ArgsIndex) {

  Function *SrcFn = CI.getCalledFunction();

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

    Value *V = isSpecializationConstantAtPosition(CArgs, Position);
    if (V == nullptr)
      continue;
    // For now, it handles only Constants. We may need to handle special
    // constants like address of stack locations etc in future.
    if (Constant *C = dyn_cast<Constant>(V)) {
      ConstantArgs.push_back(std::make_pair(Position, C));
    }
  }
  unsigned Index = getConstantArgumentsSetIndex(ConstantArgs);
  Function *NewFn = ArgSetIndexClonedFunctionMap[Index];

  ValueToValueMapTy VMap;
  CallInst *New_CI;
  // Create new cloned function for ConstantArgs if it is not already
  // there.
  if (NewFn == nullptr) {
    NewFn = CloneFunction(SrcFn, VMap);
    ArgSetIndexClonedFunctionMap[Index] = NewFn;
    ClonedFunctionList.insert(NewFn);
    propagateArgumentsToClonedFunction(NewFn, ArgsIndex, &CI);
    NumIPCloned++;
  }
  std::vector<Value *> Args(CI.op_begin(), CI.op_end() - 1);
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

  std::vector<CmpInst *> NewCondStmts;
  // New conditonal tests used in specialization
  std::vector<BasicBlock *> NewCondStmtBBs;
  std::vector<CallInst *> NewClonedCalls;
  std::vector<BasicBlock *> NewClonedCallBBs;
  // The basic blocks the NewClonedCalls will be in

  // Iterate through the list of CallSites that will be cloned.
  for (unsigned I = 0, E = CurrCallList.size(); I != E; ++I) {
    NewClonedCallBBs.clear();
    NewClonedCalls.clear();
    NewCondStmtBBs.clear();
    NewCondStmts.clear();
    CallInst *CI = CurrCallList[I];
    LLVM_DEBUG(dbgs() << "\n Call-Site (Spec): " << *CI << "\n\n");
    auto &CallArgsSets = AllCallsArgumentsSets[CI];

    if (CallArgsSets.size() == 0)
      continue;

    // No point to specialize, if there is only one arg set for this CallSite
    if (CallArgsSets.size() <= 1) {
      LLVM_DEBUG(dbgs() << "    Giving up: Not enough cases to specialize\n");
      continue;
    }
    // Split the BasicBlock containing the CallSite, so that the newly
    // generated code with tests and calls goes between the split portions.
    BasicBlock *OrigBB = CI->getParent();
    BasicBlock *TailBB = OrigBB->splitBasicBlock(CI);
    unsigned CloneCount = CallArgsSets.size();
    bool IsInexact = InexactArgsSetsCallList.count(CI);
    unsigned NumConds = CloneCount - 1;
    if (IsInexact)
      ++NumConds;
    // Make the clones for this CallSite
    for (unsigned J = 0; J < CloneCount; J++) {
      if (J < NumConds) {
        // Create a BasicBlock CondBB to hold the condition test
        BasicBlock *CondBB = BasicBlock::Create(
            CI->getContext(), ".clone.spec.cond", OrigBB->getParent(), TailBB);
        // Create the conditional expression
        Value *TAnd = nullptr;
        auto CArgs = CallArgsSets[J];
        for (auto AI = CArgs.begin(), AE = CArgs.end(); AI != AE; AI++) {
          Value *RHS = AI->second;
          Instruction *II = dyn_cast<Instruction>(RHS);
          // If the definition of the right-hand side value is an instruction,
          // rematerialize it.
          Instruction *NewII = nullptr;
          if (II != nullptr)
            NewII = II->clone();
          Value *LCmp = CmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_EQ,
                                        CI->getArgOperand(AI->first),
                                        II != nullptr ? NewII : RHS,
                                        ".clone.spec.cmp", CondBB);
          if (II != nullptr)
            NewII->insertBefore(cast<Instruction>(LCmp));
          TAnd = TAnd == nullptr ? LCmp
                                 : BinaryOperator::CreateAnd(
                                       TAnd, LCmp, ".clone.spec.and", CondBB);
        }
        Constant *ConstantZero = ConstantInt::get(TAnd->getType(), 0);
        // Cmp is the final comparison for the conditional test
        CmpInst *Cmp =
            CmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_NE, TAnd,
                            ConstantZero, ".clone.spec.cmp", CondBB);
        Cmp->setDebugLoc(CI->getDebugLoc());
        // Set aside Cmp and CondBB for further processing.
        NewCondStmts.push_back(Cmp);
        NewCondStmtBBs.push_back(CondBB);
      }
      // Create a cloned call and the BasicBlock that contains it
      BasicBlock *CallBB = BasicBlock::Create(
          CI->getContext(), ".clone.spec.call", OrigBB->getParent(), TailBB);
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
      BasicBlock *CallBB = BasicBlock::Create(
          CI->getContext(), ".clone.spec.call", OrigBB->getParent(), TailBB);
      CallInst *NewCI = cast<CallInst>(CI->clone());
      CallBB->getInstList().push_back(NewCI);
      NewClonedCalls.push_back(NewCI);
      // NewCondStmtBBs.push_back(CallBB);
      BranchInst *BI = BranchInst::Create(TailBB, CallBB);
      BI->setDebugLoc(CI->getDebugLoc());
      NewClonedCallBBs.push_back(CallBB);
      // Inlining of fallback CallSite causes huge performance regression
      // for conven00 benchmark due to downstream optimizations. Set
      // NoInline attribute for fallback CallSite for now.
      NewCI->setIsNoInline();
    } else {
      // Branch directly to the TailBB without calling the original function
      // NewCondStmtBBs.push_back(TailBB);
    }
    // Complete the BasicBlock to BasicBlock connections
    OrigBB->getInstList().pop_back();
    BranchInst::Create(NewCondStmtBBs[0], OrigBB);
    BasicBlock *F_BB;
    for (unsigned J = 0; J < NumConds; J++) {
      if (J + 1 < NumConds) {
        F_BB = NewCondStmtBBs[J + 1];
      } else {
        F_BB = NewClonedCallBBs[J + 1];
      }
      BranchInst *BI = BranchInst::Create(NewClonedCallBBs[J], F_BB,
                                          NewCondStmts[J], NewCondStmtBBs[J]);
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
    LLVM_DEBUG({
      for (unsigned J = 0; J < CloneCount; J++) {
        if (J < NumConds) {
          dbgs() << "    Cond[" << J << "] = ";
          dbgs() << *NewCondStmtBBs[J] << "\n";
        }
        dbgs() << "    ClonedCall[" << J << "] = " << *(NewClonedCallBBs[J])
               << "\n\n";
      }
      if (IsInexact)
        dbgs() << "    Fallback Call = " << *(NewClonedCallBBs[CloneCount])
               << "\n\n";
      else
        dbgs() << "    No Fallback Call"
               << "\n\n";
    });
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

// BEGIN: AVX512->AVX2 conversion code

//
// Return 'true' if 'T' is a VectorType or contains a vector type.
//
static bool containsVectorType(Type *T) {
  if (T->isVectorTy())
    return true;
  for (unsigned I = 0; I < T->getNumContainedTypes(); ++I)
    if (containsVectorType(T->getContainedType(I)))
      return true;
  return false;
}

//
// Return 'true' if we can change the target-cpu and target-features
// attributes from AVX512 to AVX2.
//
static bool canChangeCPUAttributes(Module &M) {

  // Attribute string for AVX512. This is encapsulated in the Front Ends
  // and not directly available to the backend, so we must give it here
  // explicitly.

  auto SKLAttributes = StringRef(
      "+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,"
      "+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,"
      "+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,"
      "+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,"
      "+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves");
  LLVM_DEBUG(dbgs() << "Begin test for AVX512->AVX2 conversion\n");
  unsigned FI = llvm::AttributeList::FunctionIndex;
  for (auto &F : M.getFunctionList()) {
    if (!F.hasFnAttribute("target-cpu"))
      continue;
    StringRef TCA = F.getAttribute(FI, "target-cpu").getValueAsString();
    if (TCA != "skylake-avx512") {
      LLVM_DEBUG(dbgs() << "No AVX512->AVX2 conversion: Not skylake-avx512\n");
      return false;
    }
    StringRef TFA = F.getAttribute(FI, "target-features").getValueAsString();
    if (TFA != SKLAttributes) {
      LLVM_DEBUG(dbgs() << "No AVX512->AVX2 conversion: Not skylake-avx512 "
                           "attributes\n");
      return false;
    }
    for (auto &I : instructions(&F)) {
      if (containsVectorType(I.getType())) {
        LLVM_DEBUG(dbgs() << "No AVX512->AVX2 conversion: Vector return "
                             "type\n");
        return false;
      }
      for (Value *Op : I.operands())
        if (containsVectorType(Op->getType())) {
          LLVM_DEBUG(dbgs() << "No AVX512->AVX2 conversion: Vector operand "
                               "type\n");
          return false;
        }
      auto II = dyn_cast<IntrinsicInst>(&I);
      if (II) {
        Function *Callee = II->getCalledFunction();
        if (Callee && Callee->getName().startswith("llvm.x86")) {
          LLVM_DEBUG(dbgs() << "No AVX512->AVX2 conversion: Vector "
                               "intrinsic\n");
          return false;
        }
      }
    }
  }
  LLVM_DEBUG(dbgs() << "AVX512->AVX2 conversion: All tests pass\n");
  return true;
}

static void changeCPUAttributes(Module &M) {

  // Attribute string for AVX2. This is encapsulated in the Front Ends
  // and not directly available to the backend, so we must give it here
  // explicitly.
  auto AVX2Attributes = StringRef(
      "+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,"
      "+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,"
      "+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,"
      "+xsaveopt");

  unsigned FI = llvm::AttributeList::FunctionIndex;
  for (auto &F : M.getFunctionList()) {
    if (!F.hasFnAttribute("target-cpu"))
      continue;
    assert(F.getAttribute(FI, "target-cpu").getValueAsString() ==
               "skylake-avx512" &&
           "Expecting skylake-avx512");
    llvm::AttrBuilder Attrs;
    F.removeFnAttr("target-cpu");
    F.removeFnAttr("target-features");
    Attrs.addAttribute("target-cpu", "core-avx2");
    Attrs.addAttribute("target-features", AVX2Attributes);
    F.addAttributes(FI, Attrs);
  }
  LLVM_DEBUG(dbgs() << "AVX512->AVX2 conversion: Conversion complete\n");
}

// END: AVX512->AVX2 conversion code

// BEGIN: Many Recursive Calls Cloning
//
// This is a specialized type of cloning that will be applied to functions
// which call themselves many times recursively. The actual cloning that is
// done will depend on which arguments feed if-tests and which feed switch-
// tests. The clone created will replace 'best' callsites for which the
// arguments feeding if-tests are constant. The arguments feeding the
// switch-tests are not expected to be constant at the 'best' callsites.
// Specialization tests are introduced for these. The specialization value
// is taken to be 0.

// Short forms of sets and maps used in many recursive calls cloning.
using SmallArgumentSet = SmallPtrSetImpl<Argument *>;
using SmallCallBaseSet = SmallPtrSetImpl<CallBase *>;
using SmallArgConstMap = SmallDenseMap<Argument *, ConstantInt *>;

// Return 'true' if 'F' is a candidate for many recursive calls cloning.
// If it is, add the formal arguments which feed if-tests to 'IfArgs',
// those that feed switch-tests to 'SwitchArgs'. Put in 'BestCBs' the
// callsites on which we intend to clone.
static bool isManyRecCallsCloneCandidate(Function &F, SmallArgumentSet &IfArgs,
                                         SmallArgumentSet &SwitchArgs,
                                         SmallCallBaseSet &BestCBs) {
  // Return the number of recursive calls to 'F'.
  auto NumRecCalls = [](Function &F) -> unsigned {
    unsigned Count = 0;
    for (User *U : F.users()) {
      auto CB = dyn_cast<CallBase>(U);
      if (CB && CB->getCaller() == &F && CB->getCalledFunction() == &F)
        Count++;
    }
    return Count;
  };

  // Insert into 'ConstArgs' the arguments of 'F' for which some call to 'F'
  // provides a constant argument.
  auto FindArgConstCandidates = [](Function &F, SmallArgumentSet &ConstArgs) {
    for (User *U : F.users()) {
      auto CB = dyn_cast<CallBase>(U);
      if (!CB)
        continue;
      unsigned Position = 0;
      for (Value *V : CB->arg_operands()) {
        auto CI = dyn_cast<ConstantInt>(V);
        if (CI)
          ConstArgs.insert(F.getArg(Position));
        Position++;
      }
    }
  };

  // Given 'F' and 'ConstArgs', the set of Arguments of F which have a
  // constant supplied at at least one callsite, compute 'IfArgs', the set
  // of Arguments of F that are used in an ICmpInst and 'SwitchArgs', the
  // set of Arguments that are used in a switch statement without being
  // reassigned in 'F'.
  auto FindArgTestCandidates = [](Function &F, SmallArgumentSet &ConstArgs,
                                  SmallArgumentSet &IfArgs,
                                  SmallArgumentSet &SwitchArgs) {
    for (Argument &Arg : F.args()) {
      if (!ConstArgs.count(&Arg))
        continue;
      for (User *U : Arg.users()) {
        if (isa<ICmpInst>(U))
          IfArgs.insert(&Arg);
        else if (isa<SwitchInst>(U))
          SwitchArgs.insert(&Arg);
      }
    }
  };

  // Add each callsite of 'F' to 'GoodIfCBs' if it has constant values for
  // each Argument in 'IfArgs'.
  auto FindGoodIfCallSites = [](Function &F, SmallArgumentSet &IfArgs,
                                SmallCallBaseSet &GoodIfCBs) {
    for (User *U : F.users()) {
      auto CB = dyn_cast<CallBase>(U);
      if (!CB || CB->getCaller() == &F)
        continue;
      bool IsGood = true;
      for (Argument *Arg : IfArgs) {
        unsigned Position = Arg->getArgNo();
        auto CI = dyn_cast_or_null<ConstantInt>(CB->getArgOperand(Position));
        if (!CI) {
          IsGood = false;
          break;
        }
      }
      if (IsGood)
        GoodIfCBs.insert(CB);
    }
  };

  // Add each callsite of 'F' to 'GoodSwitchCBs' which has a 'SwitchArg'
  // corresponding to an actual argument of the callsite which is a load of
  // a GEP whose pointer operand is an argument. (This is a somewhat "clever"
  // heuristic.)
  auto FindGoodSwitchCallSites = [](Function &F, SmallArgumentSet &SwitchArgs,
                                    SmallCallBaseSet &GoodSwitchCBs) {
    for (User *U : F.users()) {
      auto CB = dyn_cast<CallBase>(U);
      if (!CB || CB->getCaller() == &F)
        continue;
      bool IsGood = true;
      for (Argument *Arg : SwitchArgs) {
        unsigned Position = Arg->getArgNo();
        auto LI = dyn_cast_or_null<LoadInst>(CB->getArgOperand(Position));
        if (!LI) {
          IsGood = false;
          break;
        }
        auto GEPI = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
        if (!GEPI) {
          IsGood = false;
          break;
        }
        auto CA = dyn_cast<Argument>(GEPI->getPointerOperand());
        if (!CA) {
          IsGood = false;
          break;
        }
      }
      if (IsGood)
        GoodSwitchCBs.insert(CB);
    }
  };

  // Create 'BestCBs' which is the interesection of 'GoldIfCBs' and
  // 'GoodSwitchCBs'.
  auto FindBestCallSites = [](SmallCallBaseSet &GoodIfCBs,
                              SmallCallBaseSet &GoodSwitchCBs,
                              SmallCallBaseSet &BestCBs) {
    for (CallBase *CB : GoodIfCBs)
      if (GoodSwitchCBs.find(CB) != GoodSwitchCBs.end())
        BestCBs.insert(CB);
  };

  // Main code for isManyRecCallsCloneCandidate
  SmallPtrSet<Argument *, 16> ConstArgs;
  SmallPtrSet<CallBase *, 16> GoodIfCBs;
  SmallPtrSet<CallBase *, 16> GoodSwitchCBs;
  // Reject candidates that don't have enough recursive calls.
  LLVM_DEBUG(dbgs() << "MRC Cloning: Testing: " << F.getName() << "\n");
  if (NumRecCalls(F) < IPManyRecCallsCloningMinRecCallsites) {
    LLVM_DEBUG(dbgs() << "MRC Cloning: Skipping: Not enough recursive "
                         "callsites\n");
    return false;
  }
  // Reject vargars candidates for simplicity.
  if (F.isVarArg()) {
    LLVM_DEBUG(dbgs() << "MRC Cloning: Skipping: Is VarArgs\n");
    return false;
  }
  // Collect formal Arguments corresponding to actual arguments with constant
  // values that feed if-tests and switch-tests in 'F'.
  FindArgConstCandidates(F, ConstArgs);
  FindArgTestCandidates(F, ConstArgs, IfArgs, SwitchArgs);
  LLVM_DEBUG({
    for (Argument *Arg : IfArgs)
      dbgs() << "MRC Cloning: IF ARG #" << Arg->getArgNo() << "\n";
    for (Argument *Arg : SwitchArgs)
      dbgs() << "MRC Cloning: SWITCH ARG #" << Arg->getArgNo() << "\n";
  });
  // Reject candidates that don't have at least one if-test argument and only
  // one switch-test argument.
  if (IfArgs.size() == 0 || SwitchArgs.size() != 1)
    return false;
  // Find callsites that pass the if-test heuristic.
  FindGoodIfCallSites(F, IfArgs, GoodIfCBs);
  LLVM_DEBUG({
    for (CallBase *CB : GoodIfCBs)
      dbgs() << "MRC Cloning: GOOD IF CB: " << CB->getCaller()->getName() << " "
             << *CB << "\n";
  });
  if (GoodIfCBs.size() == 0) {
    LLVM_DEBUG(dbgs() << "MRC Cloning: Skipping: Not enough good IF "
                         "candidates\n");
    return false;
  }
  // Find callsites that pass the switch-test heuristic.
  FindGoodSwitchCallSites(F, SwitchArgs, GoodSwitchCBs);
  LLVM_DEBUG({
    for (CallBase *CB : GoodSwitchCBs)
      dbgs() << "MRC Cloning: GOOD SWITCH CB: " << CB->getCaller()->getName()
             << " " << *CB << "\n";
  });
  if (GoodSwitchCBs.size() == 0) {
    LLVM_DEBUG(dbgs() << "MRC Cloning: Skipping: Not enough good SWITCH "
                         "candidates\n");
    return false;
  }
  // Get the intersection of the callsites that pass both the if-test heuristic
  // and the switch-test heuristic and consider these the "best" callsites,
  // i.e. the ones that should be transformed.
  FindBestCallSites(GoodIfCBs, GoodSwitchCBs, BestCBs);
  if (BestCBs.size() == 0) {
    LLVM_DEBUG(dbgs() << "MRC Cloning: Skipping: No BEST callsite "
                         "identified\n");
    return false;
  }
  LLVM_DEBUG({
    for (CallBase *CB : BestCBs)
      dbgs() << "MRC Cloning: BEST CB: " << CB->getCaller()->getName() << " "
             << *CB << "\n";
    dbgs() << "MRC Cloning: OK: " << F.getName() << "\n";
  });
  return true;
}

//
// Class which splits a Function 'F' into two Functions 'F1' and 'F2'. 'F1'
// represents the initial computation in 'F', and 'F2' represents the final
// computation in 'F'. Each call to 'F' is split into:
//
//   keep_on = 0;
//   RV = F1(..., &keep_on);
//   if (keep_on)
//     RV = F2(..., RV);
//
// where 'F1' sets 'keep_on' to 1 if 'F2' needs to be executed (i.e. has not
// yet exited through an early return in 'F1'.
//
// Here is the basic technique used:
//   (1) Partition the BasicBlocks of 'F' into two sets, 'Visited' and non-
//       'Visited'. The 'Visited' blocks go into 'F1' and the others into 'F2'.
//   (2) Find the split Instructions. These are the ones which appear in the
//       'Visited' blocks, but have uses in the non-'Visited' blocks. We must
//       find a way to get their values at the beginning of the split block,
//       'BBSplit', to which all of the exits from the 'Visited' blocks will
//       be targeted and which will serve as the new entry block for 'F2'.
//       We currently handle only the types needed for an important special
//       case. The cases handled are:
//       (a) AllocaInsts: If all of their uses are in the non-'Visited' blocks
//           then sink their definition into 'BBSplit'.
//       (b) LoadInsts: Reload the value if we can prove that it has not
//           changed in the 'Visited' blocks.
//       (c) GetElementPtrInsts: Use them as defined in the 'Visited' blocks
//           if we can prove that the pointer operand does not change value
//           during the 'Visited' blocks.
//       (d) PHINodes: If there is a GetElementPtrInst to which all of the
//           PHI values are stored, and the value at that location is not
//           changed after the PHINode values are stored, reload the value
//           using a copy of the GetElementPtrInst.
//       At the end, we expect all split Instructions to be handled except
//       the return value. It will dealt with as explained above.
//  (3) Rematerialize the split Instructions into the 'BBSplit' block.
//  (4) Form 'F1' and 'F2' by creating new empty Functions and moving the
//      'Visited' blocks into 'F1' and the non-'Visited' blocks into F2.
//      Note that an extra Argument will be added to 'F1' and 'F2'. Patch
//      up the code for the extra arguments in 'F1' and 'F2'.
//  (5) Fix up the calls to 'F' so that they look like the sequence above.
//

class Splitter {

public:
  Splitter(Function *F)
      : F(F), F1(nullptr), F2(nullptr), BBSplit(nullptr), BBReturn(nullptr),
        SplitValue(nullptr), InstInsertBefore(nullptr) {}
  // Returns 'true' if it is legal to split 'F'.
  // This implements steps (1)-(2) above.
  bool canSplitFunction();
  // Split 'F' into 'F1' and 'F2'.
  // This implements steps (3)-(5) above.
  void splitFunction();

private:
  // Function to be split
  Function *F;
  // First 'splitend'.
  Function *F1;
  // Second 'splitend'.
  Function *F2;
  // BasicBlocks which will be moved to 'F1'. Those not in this set will be
  // moved to 'F2'.
  SmallPtrSet<BasicBlock *, 10> Visited;
  // The join point through which 'F' will be split. Predecessors of 'BBSplit'
  // will go into 'F1', while successors will go into 'F2'.
  BasicBlock *BBSplit;
  // The unique BasicBlock in 'F' which has a ReturnInst.
  BasicBlock *BBReturn;
  // Instructions which are defined in the 'Visited' blocks, but used in the
  // non-'Visited' blocks. We must resolve each of these in order to split
  // 'F' into 'F1' and 'F2'
  SmallPtrSet<Instruction *, 10> SplitInsts;
  // A list of LoadInsts in the SplitInsts for which a resolution has been
  // determined. These can feed GEPs that also must be resolved, so it is
  // important resolve the LoasInsts before resolving the GEPs on which they
  // depend.
  SmallPtrSet<LoadInst *, 4> RepLIs;
  // This maps PHIs that can be reloaded to the GEPs from which they can
  // be reloaded. The basic idea is that if a PHINode represents a value
  // that is stored back to a particular address given by a GEP, we can
  // get that value by reloading the GEP.
  SmallDenseMap<PHINode *, GetElementPtrInst *> PHIReloadMap;
  // This maps each original GEP in the Visited blocks to one in the
  // 'BBSplit' block with the same value.
  SmallDenseMap<LoadInst *, LoadInst *> LoadReloadMap;
  // A key PHINode value which cannot be reloaded and will be passed from
  // 'F1' to 'F2' through the return value of 'F1'. It represents a partial
  // computation of the return value of 'F' up to the point of the split.
  PHINode *SplitValue;
  // Insert Instructions which rematerialize key values and allow 'F' to be
  // split into 'F1' and 'F2' before this Instruction.
  Instruction *InstInsertBefore;
  // Return 'true' if we can identify a set of blocks into which we can split
  // 'F' into 'F1' and 'F2'.
  bool canSplitBlocks();
  // Find the Instructions which are defined in the 'Visited' blocks but used
  // in the non-'Visited' blocks.
  bool findSplitInsts();
  // Return 'true' if 'AI' can be sunk into 'F2' because all of its uses
  // will appear there.
  bool canSinkAllocaInst(AllocaInst *AI, DominatorTree *DT);
  // Return 'true' if 'LI' can be reloaded from 'F2' because its value did
  // not change during the execution of 'F1'.
  bool canReloadFromGEPI(LoadInst *LI);
  // Return 'true' if 'GEPI' can be reloaded for use in 'F2', because its
  // pointer operand did not change value in 'F1'.
  bool canReplicateGEPI(GetElementPtrInst *GEPI);
  // Return 'true' if 'PHIN' can be reloaded for 'F2', becauase its value
  // if stored back in 'F1' to a consistent location.
  bool canReloadPHI(PHINode *PHIN);
  // If all of the split Instructions can be rematerialized to split 'F' into
  // 'F1' and 'F2', set 'SplitValue' to the value that must be returned from
  // 'F1' and passed into the parameter list of 'F2', and return 'true'.
  bool validateSplitInsts();
  // Move the Uses to non-'Visited' blocks from 'V0' to 'V1'.
  void moveNonVisitedUses(Value *V0, Value *V1);
  // Sink 'AI' so that its definition and uses all will appear in 'F2'.
  void sinkAllocaInst(AllocaInst *AI);
  // Reload 'LI' for 'F2', so that its uses in 'F2' are dominated by the
  // reloaded value.
  void reloadFromGEPI(LoadInst *LI);
  // Replicate 'GEPI' for 'F2', so that the replicated version of 'F2'
  // dominates all of the uses in 'F2'.
  void replicateGEPI(GetElementPtrInst *GEPI);
  // Reload 'PHI' from the place which its value was consistently stored,
  // so that the reloaded value dominates all uses for 'F2'.
  void reloadPHI(PHINode *PHIN);
  // Move the split Instructions to the appropriate places, so that 'F' can
  // be split into 'F1' and 'F2'.
  void reshuffleFunction();
  // Fix up the return values of 'F1' and 'F2' so that the early return
  // value from 'F1' gets passed down to 'F2'.
  void retargetReturns();
  // Return a new Function patterned after 'F' and with an extra argument of
  // Type 'ArgTy' at the end of the argument list. Set '*Arg' to the newly
  // created Argument, '*NewSplitValue' to the 'SplitValue' of the new
  // Function, '*NewBBSplit' to the 'BBSplit' of the new Function.
  Function *makeNewFxnWithExtraArg(Type *ArgTy, Argument **Arg,
                                   PHINode **NewSplitValue,
                                   BasicBlock **NewBBSplit);
  // Split each call to 'F' into a pair of calls to 'F1' and 'F2' with
  // an early return conditional test between 'F1' and 'F2'.
  void splitCallSites();
  // Mark as preferred for inlining callsites which can benefit from inlining
  // if the splitting is performed.
  void markForInlining();
};

bool Splitter::canSplitBlocks() {
  LLVM_DEBUG(dbgs() << "MRCS: START ANALYSIS: " << F->getName() << "\n");
  SetVector<BasicBlock *> Worklist;
  Worklist.insert(&F->getEntryBlock());
  BasicBlock *BBSwitch0 = nullptr;
  BasicBlock *BBReturn0 = nullptr;
  while (!Worklist.empty()) {
    BasicBlock *BB = Worklist.pop_back_val();
    Instruction *BT = BB->getTerminator();
    if (auto RI = dyn_cast<ReturnInst>(BT)) {
      if (!RI->getReturnValue()) {
        LLVM_DEBUG(dbgs() << "MRCS: EXIT: canSplitBlocks: "
                          << "No return value\n");
        return false;
      }
      if (BBReturn0) {
        LLVM_DEBUG(dbgs() << "MRCS: EXIT: canSplitBlocks: "
                          << "Multiple return blocks\n");
        return false;
      }
      BBReturn0 = BB;
    }
    if (isa<SwitchInst>(BT)) {
      BBSwitch0 = BB;
      continue;
    }
    Visited.insert(BB);
    for (auto S : successors(BB))
      if (Visited.insert(S).second)
        Worklist.insert(S);
  }
  if (!BBSwitch0 || !BBReturn0) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canSplitBlocks: "
                      << "No switch or unique return block\n");
    return false;
  }
  PHINode *PHINR = dyn_cast<PHINode>(&BBReturn0->front());
  if (!PHINR) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canSplitBlocks: "
                      << "No PHINode in return blcck\n");
    return false;
  }
  BasicBlock *BBSplit0 = BBSwitch0->getUniquePredecessor();
  if (!BBSplit0) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canSplitBlocks: "
                      << "Switch block without unique predecessor\n");
    return false;
  }
  Visited.erase(BBSwitch0);
  auto BI = dyn_cast<BranchInst>(BBSplit0->getTerminator());
  if (!BI) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canSplitBlocks: "
                      << "Split block terminator is not BranchInst\n");
    return false;
  }
  for (unsigned I = 0, E = BI->getNumSuccessors(); I < E; ++I) {
    BasicBlock *BB = BI->getSuccessor(I);
    if (BB == BBSwitch0)
      continue;
    auto BI = dyn_cast<BranchInst>(BB->getTerminator());
    if (!BI || BI->isConditional() || BI->getSuccessor(0) != BBReturn0) {
      LLVM_DEBUG(dbgs() << "MRCS: EXIT: canSplitBlocks: "
                        << "Unexpected successor of split block\n");
      return false;
    }
    Visited.erase(BB);
  }
  Visited.erase(BBSplit0);
  BBSplit = BBSplit0;
  Visited.erase(BBReturn0);
  BBReturn = BBReturn0;
  LLVM_DEBUG({
    unsigned VisitedCount = 0;
    unsigned NonVisitedCount = 0;
    for (auto &BB : *F)
      if (Visited.count(&BB))
        VisitedCount++;
      else
        NonVisitedCount++;
    dbgs() << "MRCS: Can split blocks: Visited: " << VisitedCount
           << " NonVisted: " << NonVisitedCount << "\n";
  });
  return true;
}

bool Splitter::findSplitInsts() {
  //
  // Check that each Use of an Argument is an Instruction.  This will let us
  // tie each Use to a specific BasicBlock, when we determine the dependences
  // between the two parts into which we want to split 'F'.
  //
  for (Argument &Arg : F->args())
    for (User *U : Arg.users())
      if (!isa<Instruction>(U)) {
        LLVM_DEBUG(dbgs() << "MRCS: EXIT: findSplitInsts: "
                          << "Arg use is not Instruction\n");
        return false;
      }
  for (BasicBlock *BB : Visited)
    for (auto &I : *BB)
      for (User *U : I.users()) {
        auto II = dyn_cast<Instruction>(U);
        if (!II) {
          LLVM_DEBUG(dbgs() << "MRCS: EXIT: findSplitInsts: "
                            << "Visited block Instruction has "
                            << "non-Instruction use\n");
          return false;
        }
        BasicBlock *BBII = II->getParent();
        if (Visited.count(BBII))
          continue;
        if (BBII != BBReturn && BBII != BBSplit)
          SplitInsts.insert(&I);
      }
  LLVM_DEBUG({
    dbgs() << "MRCS: Begin " << SplitInsts.size() << " split insts\n";
    for (Instruction *I : SplitInsts) {
      dbgs() << "MRCS: ";
      I->dump();
    }
    dbgs() << "MRCS: End split insts\n";
  });
  return true;
}

bool Splitter::canSinkAllocaInst(AllocaInst *AI, DominatorTree *DT) {

  auto IsBitCastToLifetime = [this](Instruction *I, BasicBlock *BB) -> bool {
    auto BC = dyn_cast<BitCastInst>(I);
    if (!BC || BC->getParent() != BB)
      return false;
    for (User *U : BC->users()) {
      auto II = dyn_cast<Instruction>(U);
      if (!II)
        return false;
      if (!Visited.count(II->getParent()))
        continue;
      if (II->getParent() != BB)
        return false;
      auto CI = dyn_cast<CallInst>(II);
      if (!CI)
        return false;
      Function *F = CI->getCalledFunction();
      if (!F || F->getIntrinsicID() != Intrinsic::lifetime_start)
        return false;
    }
    return true;
  };

  for (User *U : AI->users()) {
    auto I = cast<Instruction>(U);
    if (IsBitCastToLifetime(I, AI->getParent()))
      continue;
    if (Visited.count(I->getParent())) {
      LLVM_DEBUG(dbgs() << "MRCS: EXIT: canSinkAllocaInst: "
                        << "AllocaInst has Visited use\n");
      return false;
    }
    if (!DT->dominates(DT->getNode(BBSplit), DT->getNode(I->getParent()))) {
      LLVM_DEBUG(dbgs() << "MRCS: EXIT: canSinkAllocaInst: "
                        << "AllocaInst use not dominated by split block\n");
      return false;
    }
  }
  return true;
}

bool Splitter::canReloadFromGEPI(LoadInst *LI) {
  if (RepLIs.count(LI))
    return true;
  Value *V = LI->getPointerOperand();
  BitCastInst *BC = nullptr;
  if (BC = dyn_cast<BitCastInst>(V))
    V = BC->getOperand(0);
  auto GEPI = dyn_cast<GetElementPtrInst>(V);
  if (!GEPI) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReloadFromGEPI: "
                      << "GetElementPtrInst does not feed LoadInst\n");
    return false;
  }
  if (GEPI->getNumOperands() != 3) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReloadFromGEPI: "
                      << "GetElementPtrInst does not have 3 operands\n");
    return false;
  }
  auto Arg = dyn_cast<Argument>(GEPI->getPointerOperand());
  if (!Arg) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReloadFromGEPI: "
                      << "GetElementPtrInst not fed by Argument\n");
    return false;
  }
  if (!match(GEPI->getOperand(1), m_Zero())) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReloadFromGEPI: "
                      << "GetElementPtrInst arg #1 is not zero\n");
    return false;
  }
  ConstantInt *CI = nullptr;
  if (!match(GEPI->getOperand(2), m_ConstantInt(CI))) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReloadFromGEPI: "
                      << "GetElementPtrInst arg #2 is not constant\n");
    return false;
  }
  if (!GEPI->hasOneUse() || BC && !BC->hasOneUse()) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReloadFromGEPI: "
                      << "Instruction feeding load does not have one use\n");
    return false;
  }
  //
  // The Arg should only be used by other GetElementPtrInsts to load other
  // memory locations.  This ensures there are no intervening stores to
  // the location we want to reload.
  for (User *U : Arg->users()) {
    auto I = dyn_cast<Instruction>(U);
    if (!I || !Visited.count(I->getParent()))
      continue;
    auto GEPIX = dyn_cast<GetElementPtrInst>(I);
    if (!GEPIX) {
      LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReloadFromGEPI: "
                        << "Argument use is not GetElementPtrInst\n");
      return false;
    }
    if (GEPIX == GEPI)
      continue;
    for (User *V : GEPIX->users()) {
      auto LIX = dyn_cast<LoadInst>(V);
      if (!LIX || LIX->getPointerOperand() != GEPIX) {
        LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReloadFromGEPI: "
                          << "GetElementPtrInst use not expected LoadInst\n");
        return false;
      }
    }
  }
  RepLIs.insert(LI);
  return true;
}

bool Splitter::canReplicateGEPI(GetElementPtrInst *GEPI) {
  if (GEPI->getNumOperands() != 3) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReplicateGEPI: "
                      << "GetElementPtrInst does not have 3 operands\n");
    return false;
  }
  if (!match(GEPI->getOperand(1), m_Zero())) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReplicateGEPI: "
                      << "GetElementPtrInst arg #1 is not zero\n");
    return false;
  }
  if (!GEPI->hasAllConstantIndices()) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReplicateGEPI: "
                      << "GetElementPtrInst non-consant index\n");
    return false;
  }
  auto LI = dyn_cast<LoadInst>(GEPI->getPointerOperand());
  if (!LI || !canReloadFromGEPI(LI)) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReplicateGEPI: "
                      << "GetElementPtrInst pointer operand is not "
                      << "expected Loadinst\n");
    return false;
  }
  return true;
}

bool Splitter::canReloadPHI(PHINode *PHIN) {
  SmallPtrSet<BasicBlock *, 4> BBTerms;

  //
  // Find the unique GEP which could be used to reload the value 'V' of 'PHIN'
  // in 'BB'.
  //
  auto FindBaseGEP = [](Value *V, BasicBlock *BB) -> GetElementPtrInst * {
    GetElementPtrInst *RVGEPI = nullptr;
    for (auto &I : *BB)
      if (auto SI = dyn_cast<StoreInst>(&I))
        if (SI->getValueOperand() == V) {
          Value *V = SI->getPointerOperand();
          if (auto GEPI = dyn_cast<GetElementPtrInst>(V)) {
            if (RVGEPI)
              return nullptr;
            RVGEPI = GEPI;
          }
        }
    return RVGEPI;
  };

  //
  // Return 'true' if the User 'U' of Value 'V' could be used to write
  // memory at address 'V'. Note: We are currently handling only a few key
  // cases.
  //
  std::function<bool(Value *, User *)> IsBadUser = [&IsBadUser](Value *V,
                                                                User *U) {
    if (isa<LoadInst>(U))
      return false;
    if (auto II = dyn_cast<IntrinsicInst>(U)) {
      if (II->getIntrinsicID() == Intrinsic::memcpy)
        return II->getArgOperand(0) == V;
      return true;
    }
    if (auto GEPI = dyn_cast<GetElementPtrInst>(U)) {
      for (User *V : GEPI->users()) {
        if (auto BC = dyn_cast<BitCastInst>(V)) {
          for (User *UBC : BC->users())
            if (IsBadUser(BC, UBC))
              return true;
        } else if (IsBadUser(GEPI, V)) {
          return true;
        }
      }
      return false;
    }
    return true;
  };

  //
  // Return 'true' if what 'Arg' points to could be modified.
  // Note: We are currently handling only a few key cases.
  //
  auto CouldMod = [&IsBadUser](Argument *Arg) -> bool {
    for (User *U : Arg->users())
      if (IsBadUser(Arg, U))
        return true;
    return false;
  };

  //
  // Check that 'GEPIY' has a reasonable form to be considered as a
  // compatible candidate for PHINode reloading. Basically, we are looking
  // for a GetElementPtrInst which has an Argument as the pointer operator,
  // and the pointer operator is not used anywhere else within the function
  // itself, except within other GetElementPtrInsts. Also, all of its indices
  // should be constant.
  //
  auto CheckNewGEPICandidate = [&CouldMod](GetElementPtrInst *GEPIY) -> bool {
    auto Arg = dyn_cast<Argument>(GEPIY->getPointerOperand());
    if (!Arg)
      return false;
    for (User *U : Arg->users()) {
      auto I = dyn_cast<Instruction>(U);
      if (!I)
        return false;
      if (auto GEPIZ = dyn_cast<GetElementPtrInst>(U)) {
        if (GEPIZ->getPointerOperand() != Arg)
          return false;
      } else if (auto CI = dyn_cast<CallInst>(U)) {
        if (CI->isIndirectCall())
          return false;
        Function *F = CI->getCalledFunction();
        if (!F || F->isVarArg())
          return false;
        for (unsigned I = 0, E = CI->getNumArgOperands(); I < E; ++I)
          if (CI->getArgOperand(I) == Arg && CouldMod(F->getArg(I)))
            return false;
      }
    }
    if (!GEPIY->hasAllConstantIndices())
      return false;
    return true;
  };

  //
  // Return 'true' if 'GEPI' and 'GEPIY' are compatible GetElementPtrInsts.
  // By compatible, we mean that they have the same number of operands, and
  // all of the operands are the same.
  //
  // For example:
  //   %72 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo*
  //       %6, i64 0, i32 6
  // and
  //   %144 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo*
  //       %6, i64 0, i32 6
  // are compatible.
  //
  auto CheckGEPICompatibility = [](GetElementPtrInst *GEPI,
                                   GetElementPtrInst *GEPIY) -> bool {
    if (GEPI->getNumOperands() != GEPIY->getNumOperands())
      return false;
    for (unsigned I = 0, E = GEPIY->getNumOperands(); I < E; ++I)
      if (GEPIY->getOperand(I) != GEPI->getOperand(I))
        return false;
    return true;
  };

  //
  // Check that for each terminating BasicBlock of 'PHIN', that there is a
  // compatible GetElementPtrInst that stores back the value of 'PHIN' in
  // that BasicBlock. If there is, return one of these GetElementPtrInsts,
  // and set 'BBTerms' to the set of terminating BasicBlocks.
  //
  // As an illustration, consider the defining sequence for PHINode %173:
  //   %79 = getelementptr inbounds i16, i16* %78, i64 %67
  //   %81 = phi i16* [ %79, %76 ], [ null, %65 ]
  //   %150 = getelementptr %struct._PixelPacket, %struct._PixelPacket* %149,
  //       i64 0, i32 0
  //   %152 = phi i16* [ %150, %148 ], [ null, %140 ]
  //   %173 = phi i16* [ %152, %169 ], [ %81, %80 ]
  // Here, the terminating blocks are '%76', '%65', '%148', and '%140',
  // because they are where the value of '%173' are defined.
  //
  // Looking at '%76':
  //     %72 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo*
  //         %6, i64 0, i32 6
  //   76:
  //     %77 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo*
  //         %15, i64 0, i32 15, !intel-tbaa !382
  //     %78 = load i16*, i16** %77, align 8, !tbaa !382
  //     %79 = getelementptr inbounds i16, i16* %78, i64 %67
  //     store i16* %79, i16** %72, align 8, !tbaa !308
  //     br label %80
  // we see that the Value %79 is stored to using the GetElementPtrInst %72.
  //
  // Similarly, looking at '%65':
  // 65:
  //     %66 = mul i64 %48, %3
  //     %67 = add i64 %66, %2
  //     %68 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo*
  //         %15, i64 0, i32 14, !intel-tbaa !379
  //     %69 = load %struct._PixelPacket*, %struct._PixelPacket** %68, align 8
  //     %70 = getelementptr inbounds %struct._PixelPacket,
  //         %struct._PixelPacket* %69, i64 %67, !intel-tbaa !410
  //     %71 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo*
  //         %6, i64 0, i32 4
  //     store %struct._PixelPacket* %70, %struct._PixelPacket** %71, align 8
  //     %72 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo*
  //         %6, i64 0, i32 6, !intel-tbaa !308
  //     store i16* null, i16** %72, align 8, !tbaa !308
  //     %73 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo*
  //         %15, i64 0, i32 16, !intel-tbaa !393
  //     %74 = load i32, i32* %73, align 8, !tbaa !393
  //     %75 = icmp eq i32 %74, 0
  //     br i1 %75, label %80, label %76
  // we see that the Value 'null' is also stored to by '%72'.
  //
  // For '%148', we have:
  //     %144 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo*
  //          %6, i64 0, i32 6
  // 148:
  //     %149 = getelementptr inbounds %struct._PixelPacket,
  //         %struct._PixelPacket* %147, i64 %84, !intel-tbaa !410
  //     %150 = getelementptr %struct._PixelPacket, %struct._PixelPacket* %149,
  //         i64 0, i32 0
  //     store i16* %150, i16** %144, align 8, !tbaa !308
  //     br label %151
  //  we have %144 used to store back the Value '%150'. Note that '%144'
  //  loads exactly the same value as '%72', and is therefore considered to
  //  be compatible.
  //
  // Finally, for '%140', we have:
  // 140:
  //     %141 = phi i64 [ %134, %131 ], [ %106, %103 ], [ %113, %112 ]
  //     %142 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo*
  //         %6, i64 0, i32 4
  //     %143 = bitcast %struct._PixelPacket** %142 to i64*
  //     store i64 %141, i64* %143, align 8, !tbaa !376
  //     %144 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo*
  //         %6, i64 0, i32 6, !intel-tbaa !308
  //     store i16* null, i16** %144, align 8, !tbaa !308
  //     %145 = load i32, i32* %86, align 8, !tbaa !393
  //     %146 = icmp eq i32 %145, 0
  //     %147 = inttoptr i64 %141 to %struct._PixelPacket*
  //     br i1 %146, label %151, label %148
  // in which the Value 'null' is also stored 'by' %144'.
  //
  // So, for '%173', we can return either '%72' or '%144', and set 'BBTerms'
  // to the BasicBlocks {'%76', '%65', '%148', '%140'}.
  //
  auto CheckStorebacksForPHI =
      [&](PHINode *PHIN,
          SmallPtrSetImpl<BasicBlock *> &BBTerms) -> GetElementPtrInst * {
    SetVector<PHINode *> Worklist;
    SmallPtrSet<PHINode *, 4> PHIsSeen;
    GetElementPtrInst *GEPI = nullptr;
    Worklist.insert(PHIN);
    PHIsSeen.insert(PHIN);
    while (!Worklist.empty()) {
      PHINode *PHIX = Worklist.pop_back_val();
      for (unsigned I = 0, E = PHIX->getNumIncomingValues(); I < E; ++I) {
        Value *V = PHIX->getIncomingValue(I);
        BasicBlock *BB = PHIX->getIncomingBlock(I);
        auto PHIY = dyn_cast<PHINode>(V);
        if (PHIY) {
          if (PHIsSeen.count(PHIY))
            return nullptr;
          Worklist.insert(PHIY);
          PHIsSeen.insert(PHIY);
          continue;
        }
        BBTerms.insert(BB);
        GetElementPtrInst *GEPIY = FindBaseGEP(V, BB);
        if (!GEPIY)
          return nullptr;
        if (GEPI) {
          if (!CheckGEPICompatibility(GEPI, GEPIY))
            return nullptr;
        } else {
          if (!CheckNewGEPICandidate(GEPIY))
            return nullptr;
          GEPI = GEPIY;
        }
      }
    }
    return GEPI;
  };

  //
  // Return 'true' if 'GEPI' stores a value in 'BB' and no other store
  // in 'BB' will overwrite the store done by 'GEPI'.
  //
  // For example, consider BasicBlock '%140' in th example above:
  //
  // 140:
  //     %141 = phi i64 [ %134, %131 ], [ %106, %103 ], [ %113, %112 ]
  //     %142 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo*
  //         %6, i64 0, i32 4
  //     %143 = bitcast %struct._PixelPacket** %142 to i64*
  //     store i64 %141, i64* %143, align 8, !tbaa !376
  //     %144 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo*
  //         %6, i64 0, i32 6, !intel-tbaa !308
  //     store i16* null, i16** %144, align 8, !tbaa !308
  //     %145 = load i32, i32* %86, align 8, !tbaa !393
  //     %146 = icmp eq i32 %145, 0
  //     %147 = inttoptr i64 %141 to %struct._PixelPacket*
  //     br i1 %146, label %151, label %148
  //
  // The first store in the BasicBlock stores 8 bytes starting at field #4
  // of the %struct._NexusInfo at '%6'. The second store in the BasicBlock
  // stores 8 bytes starting at field #6 of the %struct._NexusInfo at '%6'.
  // If 'GEPI' is '%144', we return 'true', since the second store stores
  // to the 'GEPI's address, while the first store does not overlap.
  //
  auto CheckBBStores = [](GetElementPtrInst *GEPI, BasicBlock *BB,
                          bool IsTerm) -> bool {
    for (auto &I : *BB) {
      if (auto SI = dyn_cast<StoreInst>(&I)) {
        Value *V = SI->getPointerOperand();
        if (auto BC = dyn_cast<BitCastInst>(V)) {
          const DataLayout &DL = BC->getModule()->getDataLayout();
          Type *TySrc = BC->getSrcTy();
          if (!TySrc->isPointerTy())
            return false;
          Type *TySrcE = TySrc->getPointerElementType();
          Type *TyDest = BC->getDestTy();
          if (!TyDest->isPointerTy())
            return false;
          Type *TyDestE = TyDest->getPointerElementType();
          if (DL.getTypeSizeInBits(TySrcE) != DL.getTypeSizeInBits(TyDestE))
            return false;
          V = BC->getOperand(0);
        }
        auto GEPIX = dyn_cast<GetElementPtrInst>(V);
        if (!GEPIX)
          return false;
        if (GEPIX->getPointerOperand()->getType() !=
            GEPI->getPointerOperand()->getType())
          return false;
        if (GEPIX->getNumOperands() < GEPI->getNumOperands())
          return false;
        for (unsigned I = 0, E = GEPI->getNumOperands(); I < E; ++I)
          if (I != E - 1) {
            if (GEPIX->getOperand(I) != GEPI->getOperand(I))
              return false;
          } else {
            if (!IsTerm && GEPIX->getOperand(I) == GEPI->getOperand(I))
              return false;
          }
      }
    }
    return true;
  };

  //
  // Return 'true' if the values of 'PHIN' are always stored back to the
  // address given by 'GEPI', indicating 'PHIN' can be reloaded using 'GEPI'.
  // 'BBTerms' are the terminating BasicBlocks in the 'PHIN' sequence.
  // This will happen if each terminating BasicBlock assigns it value to
  // the address given by 'GEPI' and no non-terminating BasicBlock on
  // any path back to 'PHIN' assigns to the location specified by 'GEPI'.
  //
  auto CheckBBStoresForPHI =
      [CheckBBStores](PHINode *PHIN, GetElementPtrInst *GEPI,
                      SmallPtrSetImpl<BasicBlock *> &BBTerms) -> bool {
    SetVector<BasicBlock *> BBWorklist;
    SmallPtrSet<BasicBlock *, 4> BBVisited;
    BBWorklist.insert(PHIN->getParent());
    while (!BBWorklist.empty()) {
      BasicBlock *BB = BBWorklist.pop_back_val();
      if (BBVisited.count(BB))
        continue;
      BBVisited.insert(BB);
      bool TermBlock = BBTerms.count(BB);
      if (!CheckBBStores(GEPI, BB, TermBlock))
        return false;
      if (!TermBlock)
        for (BasicBlock *PredBB : predecessors(BB))
          BBWorklist.insert(PredBB);
    }
    return true;
  };

  // Main code for Splitter::canReloadPHI
  //
  // Note that in this analysis, we tolerate (even require) one of the
  // SplitInsts that is a PHINode to NOT be reloadable. This will be
  // saved as the SplitValue.
  //
  GetElementPtrInst *GEPI = CheckStorebacksForPHI(PHIN, BBTerms);
  if (!GEPI) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReloadPHI: "
                      << "Could not find storebacks for PHINode\n");
    return false;
  }
  if (!CheckBBStoresForPHI(PHIN, GEPI, BBTerms)) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: canReloadPHI: "
                      << "Improper stores in dominance sequence\n");
    return false;
  }
  PHIReloadMap[PHIN] = GEPI;
  return true;
}

bool Splitter::validateSplitInsts() {
  SplitValue = nullptr;
  DominatorTree DT(*F);
  RepLIs.clear();
  for (auto *I : SplitInsts) {
    if (auto AI = dyn_cast<AllocaInst>(I)) {
      if (!canSinkAllocaInst(AI, &DT))
        return false;
    } else if (auto LI = dyn_cast<LoadInst>(I)) {
      if (!canReloadFromGEPI(LI))
        return false;
    } else if (auto GEPI = dyn_cast<GetElementPtrInst>(I)) {
      if (!canReplicateGEPI(GEPI))
        return false;
    } else if (auto PHIN = dyn_cast<PHINode>(I)) {
      if (!canReloadPHI(PHIN)) {
        if (SplitValue)
          return false;
        LLVM_DEBUG(dbgs() << "MRCS: Ignore previous EXIT. "
                             "Saving PHI as SplitValue\n");
        SplitValue = PHIN;
      }
    } else {
      return false;
    }
  }
  if (!SplitValue) {
    LLVM_DEBUG(dbgs() << "MRCS: EXIT: Did not find SplitValue\n");
    return false;
  }
  LLVM_DEBUG({
    dbgs() << "MRCS: Validated split insts: ";
    SplitValue->dump();
  });
  return true;
}

bool Splitter::canSplitFunction() {

  //
  // Returns 'true' if 'F' has a Use that is not a CallInst
  //
  auto HasNonCallInstUse = [](Function *F) -> bool {
    for (User *U : F->users()) {
      auto CI = dyn_cast<CallInst>(U);
      if (!CI || CI->getCalledFunction() != F) {
        LLVM_DEBUG(dbgs() << "MRCS: EXIT: Non-CallInst Use\n");
        return true;
      }
    }
    return false;
  };

  if (F->isDeclaration())
    return false;
  if (!canSplitBlocks())
    return false;
  if (HasNonCallInstUse(F))
    return false;
  if (!findSplitInsts())
    return false;
  if (!validateSplitInsts())
    return false;
  LLVM_DEBUG(dbgs() << "MRCS: Can split " << F->getName() << "\n");
  return true;
}

void Splitter::moveNonVisitedUses(Value *V0, Value *V1) {
  SmallPtrSet<Use *, 5> UsesToMove;
  for (Use &U : V0->uses()) {
    //
    // Note that V0 should always be a SplitInst, and we checked in
    // findSplitInsts() above that every Instruction in a Visited block has
    // only Uses which are Instructions.
    //
    bool IsVisited = Visited.count(cast<Instruction>(U.getUser())->getParent());
    if (!IsVisited)
      UsesToMove.insert(&U);
  }
  for (Use *U : UsesToMove)
    U->set(V1);
}

void Splitter::sinkAllocaInst(AllocaInst *AI) {
  //
  // Note: All casts in this function were checked in canSinkAllocaInst().
  //
  BasicBlock *BB = AI->getParent();
  SmallVector<Instruction *, 10> MoveSet;
  MoveSet.push_back(AI);
  for (User *U : AI->users()) {
    auto I = cast<Instruction>(U);
    if (I->getParent() == BB) {
      auto BC = cast<BitCastInst>(I);
      MoveSet.push_back(BC);
      for (User *V : BC->users()) {
        auto J = cast<Instruction>(V);
        if (J->getParent() != BB)
          continue;
        auto CI = cast<CallInst>(J);
        MoveSet.push_back(CI);
      }
    }
  }
  for (auto *I : MoveSet) {
    I->removeFromParent();
    I->insertBefore(InstInsertBefore);
  }
}

void Splitter::reloadFromGEPI(LoadInst *LI) {
  if (RepLIs.count(LI))
    return;
  BitCastInst *BCOld = nullptr;
  Value *V = LI->getPointerOperand();
  if (auto BC = dyn_cast<BitCastInst>(V)) {
    BCOld = BC;
    V = BC->getOperand(0);
  }
  auto GEPIOld = cast<GetElementPtrInst>(V);
  Instruction *IL = GEPIOld->clone();
  IL->insertBefore(InstInsertBefore);
  if (BCOld) {
    auto BCNew = CastInst::Create(Instruction::BitCast, IL, BCOld->getDestTy(),
                                  "", InstInsertBefore);
    IL = BCNew;
  }
  LoadInst *LINew = new LoadInst(LI->getType(), IL, "", LI->isVolatile(),
                                 LI->getAlign(), InstInsertBefore);
  RepLIs.insert(LI);
  LoadReloadMap[LI] = LINew;
  moveNonVisitedUses(LI, LINew);
}

void Splitter::replicateGEPI(GetElementPtrInst *GEPI) {
  auto LI = cast<LoadInst>(GEPI->getPointerOperand());
  if (!RepLIs.count(LI))
    reloadFromGEPI(LI);
  LoadInst *GEPIPO = LoadReloadMap[LI];
  SmallVector<Value *, 8> Idxs;
  for (unsigned I = 1, E = GEPI->getNumOperands(); I != E; ++I)
    Idxs.push_back(GEPI->getOperand(I));
  Type *Ty = GEPI->getPointerOperand()->getType()->getPointerElementType();
  auto GEPINew =
      GetElementPtrInst::Create(Ty, GEPIPO, Idxs, "", InstInsertBefore);
  moveNonVisitedUses(GEPI, GEPINew);
}

void Splitter::reloadPHI(PHINode *PHIN) {
  GetElementPtrInst *GEPI = PHIReloadMap[PHIN];
  assert(GEPI != nullptr && "Expecting PHINode in map");
  auto GEPINew = GEPI->clone();
  GEPINew->insertBefore(InstInsertBefore);
  Type *LType = GEPINew->getType()->getPointerElementType();
  const DataLayout &DL = GEPINew->getFunction()->getParent()->getDataLayout();
  Align LAlign = DL.getABITypeAlign(LType);
  LoadInst *LI =
      new LoadInst(LType, GEPINew, "", false, LAlign, InstInsertBefore);
  moveNonVisitedUses(PHIN, LI);
}

void Splitter::reshuffleFunction() {
  RepLIs.clear();
  BasicBlock *BBNew = BBSplit->splitBasicBlock(BBSplit->begin());
  Visited.insert(BBSplit);
  BBSplit = BBNew;
  InstInsertBefore = &BBSplit->front();
  for (auto *I : SplitInsts) {
    if (auto AI = dyn_cast<AllocaInst>(I))
      sinkAllocaInst(AI);
    else if (auto LI = dyn_cast<LoadInst>(I))
      reloadFromGEPI(LI);
    else if (auto GEPI = dyn_cast<GetElementPtrInst>(I))
      replicateGEPI(GEPI);
    else if (auto PHIN = dyn_cast<PHINode>(I)) {
      if (PHIN != SplitValue)
        reloadPHI(PHIN);
    } else
      assert(false && "Unexpected split case");
  }
}

Function *Splitter::makeNewFxnWithExtraArg(Type *ArgTy, Argument **Arg,
                                           PHINode **NewSplitValue,
                                           BasicBlock **NewBBSplit) {
  std::vector<Type *> NewParams;
  FunctionType *FTy = F->getFunctionType();
  for (auto &LArg : F->args())
    NewParams.push_back(LArg.getType());
  NewParams.push_back(ArgTy);
  FunctionType *NewFTy =
      FunctionType::get(FTy->getReturnType(), NewParams, FTy->isVarArg());
  Function *NewF =
      Function::Create(NewFTy, F->getLinkage(), F->getName(), F->getParent());
  NewF->copyAttributesFrom(F);
  NewF->setCallingConv(F->getCallingConv());
  NewF->setComdat(F->getComdat());
  SmallVector<ReturnInst *, 8> Rets;
  auto A = NewF->arg_begin();
  ValueToValueMapTy VMap;
  for (auto I = F->arg_begin(), E = F->arg_end(); I != E; ++I, ++A)
    VMap[&*I] = &*A;
  CloneFunctionInto(NewF, F, VMap, true, Rets);
  Argument *ArgLast = nullptr;
  for (auto &ArgNew : NewF->args())
    ArgLast = &ArgNew;
  *Arg = ArgLast;
  *NewSplitValue = cast<PHINode>(VMap[SplitValue]);
  *NewBBSplit = cast<BasicBlock>(VMap[BBSplit]);
  return NewF;
}

void Splitter::splitCallSites() {
  SmallPtrSet<CallInst *, 10> CallInstList;
  for (User *U : F->users()) {
    auto CI = dyn_cast<CallInst>(U);
    if (CI && CI->getCalledFunction() == F)
      CallInstList.insert(CI);
  }
  LLVMContext &C = F->getContext();
  const DataLayout &DL = F->getParent()->getDataLayout();
  for (CallInst *CI : CallInstList) {
    Type *NewTy1 = llvm::Type::getInt32Ty(C);
    AllocaInst *AI = new AllocaInst(NewTy1, DL.getAllocaAddrSpace(), nullptr,
                                    Align(4), "", CI);
    Constant *CI0 = ConstantInt::get(NewTy1, 0);
    Constant *CI1 = ConstantInt::get(NewTy1, 1);
    new StoreInst(CI0, AI, false, Align(4), CI);
    SmallVector<OperandBundleDef, 1> OpBundles;
    CI->getOperandBundlesAsDefs(OpBundles);
    std::vector<Value *> Args;
    for (Value *V : CI->args())
      Args.push_back(V);
    Args.push_back(AI);
    CallInst *CLI1 = CallInst::Create(F1, Args, OpBundles, "", CI);
    CLI1->setDebugLoc(CI->getDebugLoc());
    LoadInst *LI = new LoadInst(NewTy1, AI, "", false, Align(4), CI);
    CmpInst *IC =
        ICmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_EQ, LI, CI1, "", CI);
    auto NewBBT = SplitBlockAndInsertIfThen(IC, CI, false);
    Args.clear();
    for (Value *V : CI->args())
      Args.push_back(V);
    Args.push_back(CLI1);
    CallInst *CLI2 = CallInst::Create(F2, Args, OpBundles, "", NewBBT);
    CLI2->setDebugLoc(CI->getDebugLoc());
    PHINode *PHIN = PHINode::Create(CLI1->getType(), 2, "",
                                    &NewBBT->getSuccessor(0)->front());
    PHIN->addIncoming(CLI1, CLI1->getParent());
    PHIN->addIncoming(CLI2, CLI2->getParent());
    CI->replaceAllUsesWith(PHIN);
    CI->eraseFromParent();
  }
}

void Splitter::retargetReturns() {
  LLVMContext &C = F->getContext();
  BasicBlock *BBNewReturnIsEE = BasicBlock::Create(C, "", F);
  Visited.insert(BBNewReturnIsEE);
  PHINode *PHIN = cast<PHINode>(&BBReturn->front());
  unsigned CountIsEE = 0;
  unsigned CountNoEE = 0;
  for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; ++I) {
    BasicBlock *BBIn = PHIN->getIncomingBlock(I);
    if (Visited.count(BBIn))
      ++CountIsEE;
    else
      ++CountNoEE;
  }
  PHINode *PHIEE =
      PHINode::Create(PHIN->getType(), CountIsEE, "", BBNewReturnIsEE);
  ReturnInst::Create(C, PHIEE, BBNewReturnIsEE);
  for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; ++I) {
    BasicBlock *BBIn = PHIN->getIncomingBlock(I);
    if (Visited.count(BBIn))
      PHIEE->addIncoming(PHIN->getIncomingValue(I), BBIn);
  }
  PHINode *PHINoEE =
      PHINode::Create(PHIN->getType(), CountNoEE, "", &BBReturn->front());
  for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; ++I) {
    BasicBlock *BBIn = PHIN->getIncomingBlock(I);
    if (!Visited.count(BBIn))
      PHINoEE->addIncoming(PHIN->getIncomingValue(I), BBIn);
  }
  PHIN->replaceAllUsesWith(PHINoEE);
  PHIN->eraseFromParent();
  for (BasicBlock *BB : Visited) {
    if (auto BI = dyn_cast<BranchInst>(BB->getTerminator())) {
      for (unsigned I = 0, E = BI->getNumSuccessors(); I < E; ++I)
        if (BI->getSuccessor(I) == BBReturn)
          BI->setSuccessor(I, BBNewReturnIsEE);
    }
  }
}

void Splitter::markForInlining() {
  for (User *U : F1->users()) {
    auto CB = cast<CallBase>(U);
    Function *Caller = CB->getCaller();
    Function *Callee = CB->getCalledFunction();
    if (Callee == F1 && Caller != F && Caller != Callee && Caller != F2) {
      CB->addAttribute(llvm::AttributeList::FunctionIndex,
                       "prefer-inline-mrc-split");
      LLVM_DEBUG(dbgs() << "MRCS: Inline " << Caller->getName() << " TO "
                        << F1->getName() << "\n");
    }
    unsigned Count = 0;
    for (unsigned I = 0, E = CB->getNumArgOperands(); I < E; ++I) {
      auto CI = dyn_cast<ConstantInt>(CB->getArgOperand(I));
      if (CI && CI->isOne())
        Count++;
    }
    if (Count < 2)
      continue;
    auto LI = dyn_cast<LoadInst>(CB->getArgOperand(0));
    if (!LI)
      continue;
    auto GEPI = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
    if (!GEPI || GEPI->getNumOperands() != 3 || !GEPI->hasAllZeroIndices())
      continue;
    auto Arg = dyn_cast<Argument>(GEPI->getPointerOperand());
    if (!Arg || Arg->getArgNo() != 0)
      continue;
    for (User *V : Caller->users()) {
      auto CBB = dyn_cast<CallBase>(V);
      if (CBB) {
        Function *NCaller = CBB->getCaller();
        Function *NCallee = CBB->getCalledFunction();
        if (NCallee && NCallee == Caller && NCaller != F &&
            NCaller != NCallee) {
          CBB->addAttribute(llvm::AttributeList::FunctionIndex,
                            "prefer-inline-mrc-split");
          LLVM_DEBUG(dbgs() << "MRCS: Inline " << NCaller->getName() << " TO "
                            << NCallee->getName() << "\n");
        }
      }
    }
  }
}

void Splitter::splitFunction() {
  reshuffleFunction();
  retargetReturns();
  LLVMContext &C = F->getContext();
  Type *NewTy1 = llvm::Type::getInt32Ty(C);
  Type *NewPTy1 = llvm::Type::getInt32PtrTy(C);
  Argument *ArgLast1 = nullptr;
  PHINode *SplitValue1 = nullptr;
  BasicBlock *BBSplit1 = nullptr;
  F1 = makeNewFxnWithExtraArg(NewPTy1, &ArgLast1, &SplitValue1, &BBSplit1);
  Argument *ArgLast2 = nullptr;
  Type *NewTy2 = F->getReturnType();
  PHINode *SplitValue2 = nullptr;
  BasicBlock *BBSplit2 = nullptr;
  F2 = makeNewFxnWithExtraArg(NewTy2, &ArgLast2, &SplitValue2, &BBSplit2);
  BasicBlock *BBNewReturnNoEE = BBSplit1->getSinglePredecessor();
  ReturnInst *RI = ReturnInst::Create(C, SplitValue1, BBNewReturnNoEE);
  Constant *CI1 = ConstantInt::get(NewTy1, 1);
  new StoreInst(CI1, ArgLast1, false, Align(4), RI);
  BBNewReturnNoEE->front().eraseFromParent();
  SplitBlock(&F2->getEntryBlock(), &F2->getEntryBlock().front());
  auto BI = cast<BranchInst>(F2->getEntryBlock().getTerminator());
  BI->setSuccessor(0, BBSplit2);
  SplitValue2->replaceAllUsesWith(ArgLast2);
  LLVM_DEBUG(dbgs() << "MRCS: Split " << F->getName() << " into "
                    << F1->getName() << " and " << F2->getName() << "\n");
  splitCallSites();
  markForInlining();
}

// End of code for class Splitter

//
// Create the clones required for the "many recursive calls" cloning. 'F'
// is the Function being cloned. 'IfArgs' are the arguments feeding if-tests
// that will be constant in the clone, 'SwitchArgs' are the arguments that
// will feed switch-tests, but will be tested for explicitly when calls are
// made to the clone. 'BestCBs' are the callsites for which cloning will be
// applied.
//
static void createManyRecCallsClone(Function &F, SmallArgumentSet &IfArgs,
                                    SmallArgumentSet &SwitchArgs,
                                    SmallCallBaseSet &BestCBs) {

  // Change 'CB' to call 'NewF' rather than 'OldF'
  auto SetCallBaseUser = [](CallBase *CB, Function *OldF, Function *NewF) {
    assert(CB && "Expecting non-nullptr CB");
    for (Use &U : OldF->uses()) {
      auto *NCB = dyn_cast<CallBase>(U.getUser());
      if (NCB == CB) {
        U.set(NewF);
        NCB->setCalledFunction(NewF);
        return;
      }
    }
  };

  // Make the control flow structure for an if-test of the form:
  // if (TAnd)
  //   CBClone
  // else
  //   CB
  // where 'TAnd' is in 'BBPred'.
  auto MakeBlocks = [](CallBase *CB, CallBase *CBClone, Value *TAnd,
                       BasicBlock *BBPred) {
    BasicBlock *BBofCB = CB->getParent();
    Instruction *IAfterCB = CB->getNextNonDebugInstruction();
    BasicBlock *BBTail = BBofCB->splitBasicBlock(IAfterCB);
    BasicBlock *BBTrue =
        BasicBlock::Create(CB->getContext(), ".clone.recmanycalls.truepath",
                           CB->getFunction(), BBTail);
    if (!CB->getType()->isVoidTy()) {
      PHINode *PHI = PHINode::Create(CB->getType(), 2, ".clone.recmapcalls.phi",
                                     &BBTail->front());
      CB->replaceAllUsesWith(PHI);
      PHI->addIncoming(CB, BBofCB);
      PHI->addIncoming(CBClone, BBTrue);
    }
    BranchInst::Create(BBTail, BBTrue);
    BranchInst::Create(BBTrue, BBofCB, TAnd, BBPred);
    CBClone->insertBefore(BBTrue->getTerminator());
  };

  // Add conditionals to 'TAnd', placing each in 'BBPred'. Each conditional
  // is of the form 'Argument' == 'ConstantInt' where each Argument comes
  // from the 'SAS' and is mapped to a ConstantInt with the 'ArgConstMap'.
  // The corresponding actual argument value of the 'CBClone' will be set
  // to this ConstantInt, if 'CBClone' is not nullptr. 'CB' is the callsite
  // from which 'CBClone' was cloned. 'NewF' is the clone. We return the
  // updated value of 'TAnd'. The created conditionals are ANDed together
  // with the original value of 'TAnd'.
  auto MakeTAndFromMap = [](Value *TAnd, CallBase *CB, CallBase *CBClone,
                            BasicBlock *BBPred, Function *NewF,
                            SmallArgumentSet &SAS,
                            SmallArgConstMap &ArgConstMap) -> Value * {
    for (Argument *Arg : SAS) {
      Value *V = CB->getArgOperand(Arg->getArgNo());
      Argument *NewArg = NewF->getArg(Arg->getArgNo());
      ConstantInt *CI = ArgConstMap[NewArg];
      if (CBClone)
        CBClone->setArgOperand(Arg->getArgNo(), CI);
      Value *LCmp = CmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_EQ, V, CI,
                                    ".clone.recmanycalls.cmp", BBPred);
      TAnd = !TAnd ? LCmp
                   : BinaryOperator::CreateAnd(
                         TAnd, LCmp, ".clone.recmanycalls.and", BBPred);
    }
    return TAnd;
  };

  // Transform 'CB' into:
  // if (SwitchArg[0] == 0 & ... & SwitchArg[N-1] == 0)
  //   call NewF(...)
  // else
  //   CB
  // where the arguments in the call to NewF are replaced by the ConstantInt
  // values mapped by 'ArgConstMap' from 'IfArgs' and 'SwitchArgs'. Return
  // the new call that was created.
  auto ConditionalizeCallBase2WayEarly =
      [&MakeBlocks,
       &MakeTAndFromMap](CallBase *CB, Function *NewF, SmallArgumentSet &IfArgs,
                         SmallArgumentSet &SwitchArgs,
                         SmallArgConstMap &ArgConstMap) -> CallBase * {
    CallBase *CBClone = cast<CallBase>(CB->clone());
    BasicBlock *BBPred = CB->getParent();
    BBPred->splitBasicBlock(CB);
    BBPred->getTerminator()->eraseFromParent();
    for (Argument *Arg : IfArgs) {
      Value *V = CB->getArgOperand(Arg->getArgNo());
      auto CI = cast<ConstantInt>(V);
      Argument *NewArg = NewF->getArg(Arg->getArgNo());
      ArgConstMap[NewArg] = CI;
    }
    for (Argument *Arg : SwitchArgs) {
      Value *V = CB->getArgOperand(Arg->getArgNo());
      Type *Ty = V->getType();
      auto ITy = cast<IntegerType>(Ty);
      ConstantInt *CI = ConstantInt::get(ITy, 0);
      Argument *NewArg = NewF->getArg(Arg->getArgNo());
      ArgConstMap[NewArg] = CI;
    }
    Value *TAnd = nullptr;
    TAnd = MakeTAndFromMap(TAnd, CB, CBClone, BBPred, NewF, SwitchArgs,
                           ArgConstMap);
    MakeBlocks(CB, CBClone, TAnd, BBPred);
    return CBClone;
  };

  // Return 'true' if all of the actual arguments of CB corresponding to
  // the 'SwitchArgs' have the value of ConstantInt 0.
  auto SwitchArgsMatch = [](CallBase *CB,
                            SmallArgumentSet &SwitchArgs) -> bool {
    for (Argument *Arg : SwitchArgs) {
      Value *V = CB->getArgOperand(Arg->getArgNo());
      auto CI = dyn_cast_or_null<ConstantInt>(V);
      if (!CI || CI->getZExtValue() != 0)
        return false;
    }
    return true;
  };

  // Return 'true' if all of the IfArgs for 'CB' are defined in the
  // 'ArgConstMap' and match the values of the corresponding actual arguments
  // of 'CB'.
  auto IfArgsMatch = [](CallBase *CB, SmallArgumentSet &IfArgs,
                        SmallArgConstMap ArgConstMap) -> bool {
    Function *NewF = CB->getCaller();
    for (Argument *Arg : IfArgs) {
      Value *V = CB->getArgOperand(Arg->getArgNo());
      auto CI = dyn_cast_or_null<ConstantInt>(V);
      Argument *NewArg = NewF->getArg(Arg->getArgNo());
      if (!CI || ArgConstMap[NewArg] != CI) {
        return false;
      }
    }
    return true;
  };

  // Insert into 'ReplaceArgs' all of those members of 'IfArgs' that whose
  // corresponding actual arguments of 'CB' have the values indicated in the
  // 'ArgConstMap'.
  auto FindReplaceArgs = [](CallBase *CB, SmallArgumentSet &IfArgs,
                            SmallArgConstMap ArgConstMap,
                            SmallArgumentSet &ReplaceArgs) {
    Function *NewF = CB->getCaller();
    for (Argument *Arg : IfArgs) {
      Value *V = CB->getArgOperand(Arg->getArgNo());
      auto CI = dyn_cast_or_null<ConstantInt>(V);
      Argument *NewArg = NewF->getArg(Arg->getArgNo());
      if (!CI || ArgConstMap[NewArg] != CI)
        ReplaceArgs.insert(Arg);
    }
  };

  // Transform 'CB' into:
  // if (ReplaceArgs[0] == 0 & ... & ReplaceArgs[N-1])
  //   CBClone(...)
  // else
  //   CB
  // where the 'ArgConstMap' maps the 'ReplaceArgs' into ConstantInt values.
  auto ConditionalizeCallBase2WayLate =
      [&MakeBlocks,
       &MakeTAndFromMap](CallBase *CB, SmallArgConstMap ArgConstMap,
                         SmallArgumentSet &ReplaceArgs) -> CallBase * {
    BasicBlock *BBPred = CB->getParent();
    BBPred->splitBasicBlock(CB);
    BBPred->getTerminator()->eraseFromParent();
    Value *TAnd = nullptr;
    Function *NewF = CB->getCaller();
    CallBase *CBClone = cast<CallBase>(CB->clone());
    TAnd = MakeTAndFromMap(TAnd, CB, CBClone, BBPred, NewF, ReplaceArgs,
                           ArgConstMap);
    MakeBlocks(CB, CBClone, TAnd, BBPred);
    return CBClone;
  };

  // Insert code of the following form into the beginning of 'F', so that
  // the clone 'NewF' is called when the actual arguments of 'F' have the
  // appropriate constant values:
  // if (Args that are expected to be constant in NewF are constant)
  //   call NewF(Args set to those expected constant values)
  auto InsertOriginalToCloneCall =
      [&MakeTAndFromMap](Function *F, Function *NewF, SmallArgumentSet &IfArgs,
                         SmallArgumentSet &SwitchArgs,
                         SmallArgConstMap &ArgConstMap) {
        SmallVector<Value *, 16> Args;
        for (Argument &Arg : F->args())
          Args.push_back(&Arg);
        CallInst *CB = CallInst::Create(NewF->getFunctionType(), NewF, Args,
                                        ".clone.recmanycalls.reccall",
                                        &F->getEntryBlock().front());
        // CMPLRLLVM-10901: Create debug info, calling convention, and
        // attributes for new call.
        if (F->getSubprogram()) {
          DISubprogram *DIS = F->getSubprogram();
          DebugLoc CBDbgLoc = DebugLoc::get(DIS->getScopeLine(), 0, DIS);
          CB->setDebugLoc(CBDbgLoc);
        }
        CB->setCallingConv(F->getCallingConv());
        CB->setAttributes(F->getAttributes());
        BasicBlock *BBofCB = CB->getParent();
        BBofCB->splitBasicBlock(CB);
        BasicBlock *BBPred = BBofCB;
        BBofCB = CB->getParent();
        Instruction *IAfterCB = CB->getNextNonDebugInstruction();
        BasicBlock *BBTail = BBofCB->splitBasicBlock(IAfterCB);
        Value *TAnd = nullptr;
        BBPred->getTerminator()->eraseFromParent();
        TAnd = MakeTAndFromMap(TAnd, CB, nullptr, BBPred, NewF, IfArgs,
                               ArgConstMap);
        TAnd = MakeTAndFromMap(TAnd, CB, nullptr, BBPred, NewF, SwitchArgs,
                               ArgConstMap);
        BranchInst::Create(BBofCB, BBTail, TAnd, BBPred);
        BBofCB->getTerminator()->eraseFromParent();
        if (CB->getType()->isVoidTy())
          ReturnInst::Create(CB->getContext(), CB->getParent());
        else
          ReturnInst::Create(CB->getContext(), CB, CB->getParent());
      };

  // Main code for 'createManyRecCallsClone'. Iterate over the selected
  // "best" callsites of the original Function, creating a clone for each
  // and patching up the callsites to the original and cloned Function
  // as desired.
  for (CallBase *CB : BestCBs) {
    ValueToValueMapTy VMap;
    // Clone the original F to NewF
    Function *NewF = CloneFunction(&F, VMap);
    LLVM_DEBUG(dbgs() << "MRC Cloning: " << F.getName() << " TO "
                      << NewF->getName() << "\n");
    // Surround the best callsite with a test which allows the clone
    // to be called when the right arguments are constant.
    SmallArgConstMap ArgConstMap;
    CB = ConditionalizeCallBase2WayEarly(CB, NewF, IfArgs, SwitchArgs,
                                         ArgConstMap);
    SetCallBaseUser(CB, &F, NewF);
    // Replace the formal arguments in the clone with the appropriate
    // constant values to simplify the the clone.  The actual simpification
    // will be performed by downstream optimizations. The formals will also
    // become dead, and will be eliminated by dead argument elimination.
    for (auto &Entry : ArgConstMap)
      Entry.first->replaceAllUsesWith(Entry.second);
    // Find the calls within the clone that are still to the original function.
    // Determine which need to be replaced by calls to the clone, and whether
    // those calls needs to be conditional.  We transform only those calls
    // which will not be eliminated later by dead code elimination.
    SmallDenseMap<CallBase *, bool> NeedConditionalization;
    for (User *U : F.users()) {
      CallBase *NCB = dyn_cast<CallBase>(U);
      if (!NCB || NCB->getCaller() != NewF)
        continue;
      // If the SwitchArgs don't match, don't transform the call. In the case
      // of a mismatched constant value, it will be dead code eliminated.
      // Otherwise, it may be called with a switch arg value different than
      // the original, and which case the call cannot be legally transformed.
      if (!SwitchArgsMatch(NCB, SwitchArgs))
        continue;
      // If the IFArgs don't match, we must make the call to the clone
      // conditional.
      bool IfMatch = IfArgsMatch(NCB, IfArgs, ArgConstMap);
      NeedConditionalization[NCB] = IfMatch;
    }
    // Convert calls to the original function to calls to the clone,
    // conditionalizing those calls when necessary.
    for (auto &Entry : NeedConditionalization) {
      CallBase *NCB = Entry.first;
      bool HasIfMatch = Entry.second;
      if (!HasIfMatch) {
        SmallPtrSet<Argument *, 16> ReplaceArgs;
        FindReplaceArgs(NCB, IfArgs, ArgConstMap, ReplaceArgs);
        NCB = ConditionalizeCallBase2WayLate(NCB, ArgConstMap, ReplaceArgs);
      }
      SetCallBaseUser(NCB, &F, NewF);
    }
    // Insert code at the beginning of the original function to test if
    // the arguments of the original function have the constant values
    // required by the clone, and if so, call the clone.
    InsertOriginalToCloneCall(&F, NewF, IfArgs, SwitchArgs, ArgConstMap);
    if (EnableManyRecCallsSplitting) {
      Splitter MRCS(NewF);
      if (MRCS.canSplitFunction())
        MRCS.splitFunction();
    }
  }
}

// END: Many Recursive Calls Cloning

// Main routine to analyze all calls and clone functions if profitable.
//
static bool analysisCallsCloneFunctions(Module &M, bool AfterInl,
                                        bool EnableDTrans,
                                        bool IFSwitchHeuristic) {
  bool FunctionAddressTaken;

  // Force RecProCloneSplitting for LIT testing.
  if (ForceManyRecCallsSplitting) {
    bool DidSplit = false;
    for (Function &F : M) {
      Splitter RPCS(&F);
      if (RPCS.canSplitFunction()) {
        RPCS.splitFunction();
        DidSplit = true;
      }
    }
    return DidSplit;
  }

  LLVM_DEBUG({
    dbgs() << " Enter IP cloning";
    if (AfterInl)
      dbgs() << ": (After inlining)\n";
    else
      dbgs() << ": (Before inlining)\n";
  });

  ClonedFunctionList.clear();

  for (Function &F : M) {

    if (skipAnalyzeCallsOfFunction(F)) {
      LLVM_DEBUG(dbgs() << " Skipping " << F.getName() << "\n");
      continue;
    }

    clearAllMaps();

    LLVM_DEBUG(dbgs() << " Cloning Analysis for:  " << F.getName() << "\n");

    IPCloneKind CloneType;
    if (AfterInl) {
      CloneType = GenericClone;
      LLVM_DEBUG(dbgs() << "    Selected generic cloning  "
                        << "\n");
    } else {
      int Start, Inc;
      unsigned ArgPos, Count;
      bool IsByRef, IsCyclic;
      if (EnableDTrans &&
          isRecProgressionCloneCandidate(F, true, &ArgPos, &Count, &Start, &Inc,
                                         &IsByRef, &IsCyclic)) {
        CloneType = RecProgressionClone;
        LLVM_DEBUG(dbgs() << "    Selected RecProgression cloning  "
                          << "\n");
        createRecProgressionClones(F, ArgPos, Count, Start, Inc, IsByRef,
                                   IsCyclic);
        if (!IsCyclic && canChangeCPUAttributes(M))
          changeCPUAttributes(M);
        continue;
      }
      // For now, run either FuncPtrsClone or SpecializationClone for any
      // function before inlining. If required, we can run both in future.
      // FuncPtrsClone is selected for a function if it has at least one
      // function-pointer type argument.
      if (IsFunctionPtrCloneCandidate(F)) {
        CloneType = FuncPtrsClone;
        LLVM_DEBUG(dbgs() << "    Selected FuncPtrs cloning  "
                          << "\n");
      } else if (isDirectlyRecursive(&F)) {
        SmallPtrSet<Argument *, 16> IfArgs;
        SmallPtrSet<Argument *, 16> SwitchArgs;
        SmallPtrSet<CallBase *, 16> BestCBs;
        if (isManyRecCallsCloneCandidate(F, IfArgs, SwitchArgs, BestCBs)) {
          CloneType = ManyRecCallsClone;
          if (EnableManyRecCallsSplitting) {
            LLVM_DEBUG(dbgs() << "    Selected many recursive calls splitting "
                              << "\n");
            Splitter MRCS(&F);
            if (MRCS.canSplitFunction())
              MRCS.splitFunction();
          } else {
            LLVM_DEBUG(dbgs() << "    Selected many recursive calls cloning "
                              << "\n");
            createManyRecCallsClone(F, IfArgs, SwitchArgs, BestCBs);
          }
          continue;
        }
        CloneType = GenericClone;
        LLVM_DEBUG(dbgs() << "    Selected generic cloning (recursive) "
                          << "\n");
      } else {
        CloneType = SpecializationClone;
        LLVM_DEBUG(dbgs() << "    Selected Specialization cloning  "
                          << "\n");
      }
    }

    FunctionAddressTaken = analyzeAllCallsOfFunction(F, CloneType);

    // It is okay to enable cloning for address taken routines but
    // disable it for now.
    if (FunctionAddressTaken) {
      LLVM_DEBUG(dbgs() << " Skipping address taken " << F.getName() << "\n");
      continue;
    }

    if (CloneType == SpecializationClone && CurrCallList.size() != 0) {
      if (CurrCallList.size() > IPSpeCloningNumCallSitesLimit) {
        LLVM_DEBUG(dbgs() << " Too many CallSites: Skipping Specialization "
                             "cloning\n");
        continue;
      }
      // Transformation done here if Specialization cloning is kicked-in.
      cloneSpecializationFunction();
      continue;
    }

    if (FormalConstantValues.size() == 0 || CurrCallList.size() == 0) {
      LLVM_DEBUG(dbgs() << " Skipping non-candidate " << F.getName() << "\n");
      continue;
    }

    LLVM_DEBUG(dumpFormalsConstants(F));

    unsigned MaxClones = getMaxClones();
    unsigned MinClones = getMinClones();

    LLVM_DEBUG({
      dbgs() << " Max clones:  " << MaxClones << "\n";
      dbgs() << " Min clones:  " << MinClones << "\n";
    });

    if (MaxClones <= 1 || MinClones > IPFunctionCloningLimit) {
      LLVM_DEBUG(dbgs() << " Skipping not worthy candidate " << F.getName()
                        << "\n");
      continue;
    }

    // For a function that is recursive and for which we are producing a
    // generic clone, potentially relax the rules on the if-switch heuristic.
    bool IsGenRec = CloneType == GenericClone && isDirectlyRecursive(&F);
    bool IsGenRecQualified = false;
    if (!findWorthyFormalsForCloning(F, AfterInl, IFSwitchHeuristic, IsGenRec,
                                     &IsGenRecQualified)) {
      LLVM_DEBUG(dbgs() << " Skipping due to Heuristics " << F.getName()
                        << "\n");
      continue;
    }

    if (!collectAllConstantArgumentsSets(F)) {
      LLVM_DEBUG(dbgs() << " Skipping not profitable candidate " << F.getName()
                        << "\n");
      continue;
    }

    // If we are relaxing the rules on formals for a generic clone of a
    // recursive function, only clone if there is only one possible clone
    // and at least 'IPGenCloningMinRecCallsites' callsites.
    if (IsGenRecQualified &&
        (FunctionAllArgumentsSets.size() != 1 ||
         CurrCallList.size() < IPGenCloningMinRecCallsites)) {
      LLVM_DEBUG({
        dbgs() << " Skipping not profitable recursive candidate " << F.getName()
               << "\n";
      });
      continue;
    }

    cloneFunction();
  }

  LLVM_DEBUG(dbgs() << " Total clones:  " << NumIPCloned << "\n");

  if (NumIPCloned != 0)
    return true;

  return false;
}

static bool runIPCloning(Module &M, bool AfterInl, bool EnableDTrans) {
  bool Change = false;
  bool IFSwitchHeuristicOn = EnableDTrans || ForceIFSwitchHeuristic;
  bool EnableDTransOn = EnableDTrans || ForceEnableDTrans;
  Change = analysisCallsCloneFunctions(M, AfterInl, EnableDTransOn,
                                       IFSwitchHeuristicOn);
  clearAllMaps();
  return Change;
}

namespace {

struct IPCloningLegacyPass : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  IPCloningLegacyPass(bool AfterInl = false, bool EnableDTrans = false)
      : ModulePass(ID), AfterInl(AfterInl), EnableDTrans(EnableDTrans) {
    initializeIPCloningLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<WholeProgramWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    if (IPCloningAfterInl)
      AfterInl = true;
    return runIPCloning(M, AfterInl, EnableDTrans);
  }

private:
  // This flag helps to decide whether function addresses or other
  // constants need to be considered for cloning.
  bool AfterInl;
  // If 'true' we are doing specialized cloning generally applicable
  // when we are running DTrans.
  bool EnableDTrans;
};
} // namespace

char IPCloningLegacyPass::ID = 0;
INITIALIZE_PASS(IPCloningLegacyPass, "ip-cloning", "IP Cloning", false, false)

ModulePass *llvm::createIPCloningLegacyPass(bool AfterInl, bool EnableDTrans) {
  return new IPCloningLegacyPass(AfterInl, EnableDTrans);
}

IPCloningPass::IPCloningPass(bool AfterInl, bool EnableDTrans)
    : AfterInl(AfterInl), EnableDTrans(EnableDTrans) {}

PreservedAnalyses IPCloningPass::run(Module &M, ModuleAnalysisManager &AM) {
  if (!runIPCloning(M, AfterInl, EnableDTrans))
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses();
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<AndersensAA>();
  return PA;
}

#if INTEL_FEATURE_SW_ADVANCED
//===------- Intel_IPCloning.cpp - IP Cloning -*------===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_IPCloningAnalysis.h"
#include "llvm/Analysis/Intel_OPAnalysisUtils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/AbstractCallSite.h"
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
#include "llvm/Transforms/IPO/Intel_InlineReport.h"
#include "llvm/Transforms/IPO/Intel_MDInlineReport.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/Intel_CloneUtils.h"
#include "llvm/Transforms/Utils/Intel_RegionSplitter.h"
#include <sstream>
#include <string>

#include "Intel_DTrans/Analysis/DTransTypeMetadataBuilder.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"

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
  ManyRecCallsClone = 5,
  ManyLoopSpecializationClone = 6
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

// Used to force off cloning of callback functions called from cloned functions
// even when it would not normally be enabled.
static cl::opt<bool>
    ForceOffCallbackCloning("ip-gen-cloning-force-off-callback-cloning",
                            cl::init(false), cl::ReallyHidden);

// Used to force on cloning of callback functions called from cloned functions
// even when it would not normally be enabled.
static cl::opt<bool>
    ForceOnCallbackCloning("ip-gen-cloning-force-on-callback-cloning",
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

static cl::opt<bool>
    EnablePreferFunctionRegion("ip-manyreccalls-preferfunctionlevelregion",
                               cl::init(false), cl::ReallyHidden);

static cl::opt<bool>
    EnableManyRecCallsPredicateOpt("ip-manyreccalls-predicateopt",
                                   cl::init(true), cl::ReallyHidden);

static cl::opt<bool> EnableClonedFunctionArgsMerge("ip-cloned-func-arg-merge",
                                                   cl::init(true),
                                                   cl::ReallyHidden);

// Minimum number of loops in kernel on which predicate opt is performed
static cl::opt<unsigned>
    PredicateOptMinLoops("ip-manyreccalls-predicateopt-min-loops", cl::init(5),
                         cl::ReallyHidden);

// Maximum descent depth for predicate opt hoisting algorithm
static cl::opt<unsigned>
    PredicateOptMaxDepth("ip-manyreccalls-predicateopt-max-depth", cl::init(6),
                         cl::ReallyHidden);

// Flag to set if we are doing LIT testing of just the splitting. In this
// case, any other ip cloning will be skipped, and the compiler will attempt
// to apply splitting to each Function for which we have IR.
static cl::opt<bool>
    ForceManyRecCallsSplitting("force-ip-manyreccalls-splitting",
                               cl::init(false), cl::ReallyHidden);

// Mininum number of loops for a Function to be a candidate for "many loops
// specialization cloning".
static cl::opt<unsigned> IPSpeCloningMinLoops("ip-spec-cloning-min-loops",
                                              cl::init(30), cl::ReallyHidden);

// While AbstractCallSite solely offers a querying interface, it's essential for
// us to possess the capability to alter a call site and substitute the called
// function. The ModifiableAbstractCallSite introduces the method
// "setCalledOperand," which facilitates the replacement of the called function.
struct ModifiableAbstractCallSite : public AbstractCallSite {
  ModifiableAbstractCallSite(const Use *U) : AbstractCallSite(U) {}
  void setCalledOperand(Function *F) {
    if (isDirectCall())
      getInstruction()->setCalledOperand(F);
    else
      getInstruction()->setArgOperand(getCallArgOperandNoForCallee(), F);
    if (auto CB = dyn_cast<CallBase>(getInstruction())) {
      getInlineReport()->setCalledFunction(CB, F);
      getMDInlineReport()->setCalledFunction(CB, F);
    }
  }
};

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
SmallDenseMap<unsigned, std::vector<std::pair<unsigned, Value *>>>
    FunctionAllArgumentsSets;

// Mapping between newly cloned function and constant argument set index.
SmallDenseMap<unsigned, Function *> ArgSetIndexClonedFunctionMap;

// List of call-sites that need to be processed for cloning
std::vector<ModifiableAbstractCallSite> CurrCallList;

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

// It is a collection of global variables that exhibit potential
// suitability for cloning, when they are passed to a function
// as actual arguments.
MapVector<GlobalVariable *, bool> GlobalArrayConstants;

// This represents a correspondence between CallBase instances and
// AbstractCallSites. This mapping is crucial because while ACS is constructed
// from a function's USE within a CallBase, it may not function correctly if
// another USE within the same CallBase is utilized for ACS construction.
// Nonetheless, there's a requirement to obtain an ACS from any arbitrary USE
// within a CallBase, as seen in situations like traversing the uses of
// GlobalVariables. In such scenarios, this mapping helps to get the accurate
// ACS.
MapVector<CallBase *, std::unique_ptr<AbstractCallSite>> CBToACS;

// The function build CBToACS mapping for a module.
static void collectAbstractCallSites(Module &M) {
  for (auto &F : M.functions()) {
    for (auto &U : F.uses()) {
      auto CB = dyn_cast<CallBase>(U.getUser());
      if (!CB)
        continue;
      auto ACSPtr = std::make_unique<AbstractCallSite>(&U);
      if (!*ACSPtr)
        continue;
      CBToACS[CB] = std::move(ACSPtr);
    }
  }
}

// This function serves as a helper to obtain an AbstractCallSite
// from a given Value. If the function is able to find the corresponding
// AbstractCallSite for a given Value, it returns a pointer to the ACS,
// or otherwise returns a nullptr.
static AbstractCallSite *getAbstractCallSite(Value *V) {
  auto CB = dyn_cast<CallBase>(V);
  if (!CB)
    return nullptr;
  auto IT = CBToACS.find(CB);
  if (IT == CBToACS.end())
    return nullptr;
  return IT->second.get();
}

// This function assesses whether the provided Value corresponds to a constant
// integer array that is initialized to zero and declared as a global variable.
// The global variables that match these criteria are subsequently stored in the
// GlobalArrayConstants map for future reference.
static bool isGlobalConstZeroInitializedArray(Value *V) {
  auto GV = dyn_cast<GlobalVariable>(V);
  if (!GV)
    return false;
  auto IT = GlobalArrayConstants.find(GV);
  if (IT != GlobalArrayConstants.end())
    return IT->second;

  auto checkCriteria = [](GlobalVariable *GV) {
    if (!GV || !GV->hasDefinitiveInitializer() || !GV->isConstant())
      return false;

    auto AT = dyn_cast<ArrayType>(GV->getInitializer()->getType());
    if (!AT)
      return false;
    if (!AT->getElementType()->isIntegerTy())
      return false;

    if (!isa<ConstantAggregateZero>(GV->getInitializer()))
      return false;

    return true;
  };

  return GlobalArrayConstants[GV] = checkCriteria(GV);
}

/// Wrapper functions for creating clones and operating on callsites that
/// also update the classic inlining report.

// Clone the Function 'F' using 'VMap', update the classic inlining report,
// and return the newly cloned Function.
static Function *IPCloneFunction(Function *F, ValueToValueMapTy &VMap) {
  Function *NewF = CloneFunction(F, VMap);
  return NewF;
}

// Set the called Function for 'CB' to 'F', and update the classic inlining
// report.
static void setCalledFunction(CallBase *CB, Function *F) {
  CB->setCalledFunction(F);
  getInlineReport()->setCalledFunction(CB, F);
  getMDInlineReport()->setCalledFunction(CB, F);
}

// Create a 'CallInst' using 'CI' as a model, inserting it before the
// Instruction 'InsertBefore'. Return the new 'CallInst' and indicate to
// the class inlining report that it was cloned from 'CI'.
static CallInst *CallInstCreate(CallInst *CI, FunctionCallee Func,
                                ArrayRef<Value *> Args,
                                const Twine &NameStr,
                                Instruction *InsertBefore) {
  auto NewCI = CallInst::Create(Func, Args, NameStr, InsertBefore);
  getInlineReport()->cloneCallBaseToCallBase(CI, NewCI);
  getMDInlineReport()->cloneCallBaseToCallBase(CI, NewCI);
  return NewCI;
}

// Create a 'CallInst' using 'CI' as a model, inserting it at the end of
// BasicBlock 'InsertAtEnd'. Return the new 'CallInst' and indicate to
// the class inlining report that it was cloned from 'CI'.
static CallInst *CallInstCreate(CallInst *CI, FunctionCallee Func,
                                ArrayRef<Value *> Args,
                                const Twine &NameStr,
                                BasicBlock *InsertAtEnd) {
  auto NewCI = CallInst::Create(Func, Args, NameStr, InsertAtEnd);
  getInlineReport()->cloneCallBaseToCallBase(CI, NewCI);
  getMDInlineReport()->cloneCallBaseToCallBase(CI, NewCI);
  return NewCI;
}

/// End of wrapper functions for creating clones and operating on callsites
/// that also update the classic inlining report.

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
  auto FnArg = Arg->stripPointerCasts();

  // Returns false if it is address of a function
  if (isa<Function>(FnArg))
    return false;

  if (isGlobalConstZeroInitializedArray(FnArg))
    return true;

  // For now, allow only INT constants. Later, we may allow
  // isa<ConstantPointerNull>(FnArg), isa<ConstantFP>(FnArg) etc.
  //
  if (!isa<ConstantInt>(FnArg))
    return false;
  return true;
}

// Return true if constant argument 'ActualV' with corresponding formal
// argument 'FormalV' is worth considering for cloning based on 'CloneType'.
//
static bool isConstantArgWorthy(Argument *FormalV, Value *ActualV,
                                IPCloneKind CloneType) {
  bool IsWorthy = false;

  if (CloneType == FuncPtrsClone) {
    IsWorthy = isConstantArgWorthyForFuncPtrsClone(ActualV);
  } else if (CloneType == SpecializationClone) {
    IsWorthy = isConstantArgWorthyForSpecializationClone(FormalV, ActualV);
  } else if (CloneType == GenericClone) {
    IsWorthy = isConstantArgWorthyForGenericClone(ActualV);
  }
  return IsWorthy;
}

// Return true if actual argument 'ActualV' with corresponding formal
// argument 'FormalV' is considered for cloning.
static bool isConstantArgForCloning(Argument *FormalV, Value *ActualV,
                                    IPCloneKind CloneType) {
  if (Constant *C = dyn_cast<Constant>(ActualV)) {
    if (isa<UndefValue>(C))
      return false;

    if (isConstantArgWorthy(FormalV, ActualV, CloneType))
      return true;
  }
  return false;
}

// Collect constant value if 'ActualV' is constant actual argument
// and save it in constant list of 'FormalV'. Otherwise, mark
// 'FormalV' as inexact.
static void collectConstantArgument(Argument *FormalV, Value *ActualV,
                                    IPCloneKind CloneType) {

  if (!isConstantArgForCloning(FormalV, ActualV, CloneType)) {
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
static void GetPointerToArrayDims(Argument *Arg, unsigned &SizeInBytes,
                                  unsigned &NumElems, const DataLayout &DL) {

  Type *ATy = inferPtrElementType(*Arg);
  if (!ATy || !isCharArray(ATy))
    return;

  NumElems = cast<ArrayType>(ATy)->getNumElements();
  SizeInBytes = DL.getTypeSizeInBits(ATy);
}

// Return a StoreInst (like StInst in the example below) if 'AI' is the address
// of packed array (i.e int64 value) on stack. (Here 'PV' is the previous Value
// encountered when tracing up to 'AI' from the PHINode.)
//
//  Ex:
//   AI:           %6 = alloca i64, align 8
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
// We also handle the opaque pointer case where there are no bitcasts.
// In this case, the alloca may have an array type, while the store
// has a scalar type.
//
static StoreInst *isStartAddressOfPackedArrayOnStack(AllocaInst *AI,
                                                     Value *PV) {

  // Return the integer bit width of 'Ty' if it is an integer type
  // or an array of integer types. If it is not, return 0.
  auto IntegerBitWidth = [](Type *Ty) -> uint64_t {
    uint64_t Result = 1;
    while (auto ATy = dyn_cast<ArrayType>(Ty)) {
      Result *= ATy->getNumElements();
      Ty = ATy->getElementType();
    }
    if (auto ITy = dyn_cast<IntegerType>(Ty))
      return Result * ITy->getBitWidth();
    return 0;
  };

  StoreInst *StInst = nullptr;
  for (User *U : AI->users()) {

    // Ignore if it is the arg that is passed to call.
    if (U == PV)
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

    // For opaque pointers, handle the case where there is no bitcast
    // to the lifetime intrinsics.
    if (auto II = dyn_cast<IntrinsicInst>(U)) {
      if (II->getIntrinsicID() == Intrinsic::lifetime_start ||
          II->getIntrinsicID() == Intrinsic::lifetime_end)
       continue;
    }

    auto SI = dyn_cast<StoreInst>(U);
    if (!SI)
      return nullptr;

    // More than one use is noticed
    if (StInst != nullptr)
      return nullptr;
    StInst = SI;
  }
  if (StInst == nullptr)
    return nullptr;

  Value *ValOp = cast<StoreInst>(StInst)->getValueOperand();
  if (!isa<Constant>(ValOp))
    return nullptr;

  if (ValOp->getType() == AI->getAllocatedType())
    return StInst;

  // Handle the case where the alloca and store value are
  // both integers of the same size.
  uint64_t ValOpIntegerBitWidth = IntegerBitWidth(ValOp->getType());
  if (ValOpIntegerBitWidth == 0)
    return nullptr;
  if (IntegerBitWidth(AI->getAllocatedType()) != ValOpIntegerBitWidth)
    return nullptr;
  return StInst;
}

// Returns 'V' as A GlobalVariable if 'V' is a Global Variable candidate for
// specialization cloning. 'I' is used to get DataLayout to compute sizes of
// types.
//
static GlobalVariable *isSpecializationGVCandidate(Value *V, Instruction *I) {
  auto GV = dyn_cast<GlobalVariable>(V);
  if (!GV)
    return nullptr;

  if (!GV->isConstant())
    return nullptr;
  if (!GV->hasDefinitiveInitializer())
    return nullptr;
  Constant *Init = GV->getInitializer();
  if (!isa<ConstantArray>(Init))
    return nullptr;

  if (GV->getLinkage() != GlobalVariable::PrivateLinkage)
    return nullptr;
  if (GV->hasComdat())
    return nullptr;
  if (GV->isThreadLocal())
    return nullptr;

  Type *Ty = GV->getValueType();
  if (!Ty->isSized())
    return nullptr;
  const DataLayout &DL = I->getModule()->getDataLayout();
  if (DL.getTypeSizeInBits(Ty) > IPSpecCloningArrayLimit)
    return nullptr;

  // Add more checks like below but not required
  // if (GlobalWasGeneratedByCompiler(GV)) return nullptr;
  // if (!Ty->isArrayTy() ||
  //     !Ty->getArrayElementType()->isArrayTy()) return nullptr;
  return GV;
}

// Return a GlobalVariable (like GlobAddr in th example below) if 'GEPI' is
// the address of stack location where Global array is copied completely.
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
//  GEPI:      %43 = getelementptr inbounds [5 x [2 x i8]],
//                   [5 x [2 x i8]]* %7, i64 0, i64 0
//
//  Callee:    call void @llvm.lifetime.start(i64 10, i8* %11) #9
//
//  MemCpyDst: i8* getelementptr inbounds ([5 x [2 x i8]],
//                   [5 x [2 x i8]]* @t.CM_THREE
//
//  GlobAddr:     @t.CM_THREE
//
// We also handle the case where some GEP(X,0,0)s may be absent, which is
// present when we have opaque pointers.
//
static GlobalVariable *
    isStartAddressOfGlobalArrayCopyOnStack(GetElementPtrInst *GEPI) {
  // First, check it is starting array address on stack
  auto AI = dyn_cast<AllocaInst>(GEPI->getPointerOperand());
  if (!AI)
    return nullptr;
  if (!GEPI->hasAllZeroIndices())
    return nullptr;

  Type *GEPType = GEPI->getSourceElementType();
  if (GEPType != AI->getAllocatedType())
    return nullptr;

  // Get another use, AUseGEPI, of AllocaInst other than the one that
  // is passed to Call, if it is present.
  GetElementPtrInst *AUseGEPI = nullptr;
  SmallVector<User *, 16> Worklist;
  for (User *U : AI->users()) {
    // Ignore if it is the arg that is passed to call.
    if (U == GEPI)
      continue;
    if (auto XGEPI = dyn_cast<GetElementPtrInst>(U)) {
      if (!XGEPI->hasAllZeroIndices())
        return nullptr;
      if (GEPType != XGEPI->getSourceElementType())
        return nullptr;
      AUseGEPI = XGEPI;
    } else {
      Worklist.push_back(U);
    }
  }
  // Gather up all of the users of AUseGEPI or of AI, when AUseGEPI is
  // not present.
  if (AUseGEPI) {
    if (!Worklist.empty())
      return nullptr;
    for (User *U : AUseGEPI->users())
      Worklist.push_back(U);
  }
  GlobalVariable *GlobAddr = nullptr;
  while (!Worklist.empty()) {
    User *U = Worklist.pop_back_val();
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
    if (AUseGEPI) {
      if (User->getArgOperand(0) != AUseGEPI)
        return nullptr;
    } else {
      if (User->getArgOperand(0) != AI)
        return nullptr;
    }
    Value *Dst = User->getArgOperand(1);
    if (AUseGEPI) {
      if (auto *MemCpyDst = dyn_cast<GEPOperator>(Dst)) {
        if (!MemCpyDst->hasAllZeroIndices())
          return nullptr;
        if (GEPType != MemCpyDst->getSourceElementType())
          return nullptr;
        if (MemCpyDst->getNumIndices() != AUseGEPI->getNumIndices())
          return nullptr;
        Dst = MemCpyDst->getOperand(0);
      }
    }
    Value *MemCpySize = User->getArgOperand(2);
    // Make sure there is only one memcpy
    if (GlobAddr)
      return nullptr;

    GlobAddr = isSpecializationGVCandidate(Dst, GEPI);
    if (!GlobAddr)
      return nullptr;

    const DataLayout &DL = GEPI->getModule()->getDataLayout();
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
static bool isSpecializationCloningSpecialConst(Value *V, PHINode *Arg) {
  Value *PropVal = nullptr;
  if (auto GEPI = dyn_cast<GetElementPtrInst>(V)) {
    PropVal = isStartAddressOfGlobalArrayCopyOnStack(GEPI);
    if (!PropVal) {
      if (!GEPI->hasAllZeroIndices())
        return false;
      Value *PV = GEPI->getPointerOperand();
      if (auto AI = dyn_cast<AllocaInst>(PV))
        PropVal = isStartAddressOfPackedArrayOnStack(AI, GEPI);
      if (!PropVal)
        return false;
    }
  } else {
    Value *PV = Arg;
    Value *W = V;
    if (auto BCI = dyn_cast<BitCastInst>(V)) {
      PV = BCI;
      W = BCI->getOperand(0);
    }
    if (auto AI = dyn_cast<AllocaInst>(W))
      PropVal = isStartAddressOfPackedArrayOnStack(AI, PV);
    else
      return false;
  }
  if (!PropVal)
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
collectArgsSetsForSpecialization(Function &F, ModifiableAbstractCallSite &ACS,
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

  auto *CI = cast<CallInst>(ACS.getInstruction());

  // Collect argument sets for PHINodes in PhiValues that are passed
  // as arguments at CI.
  BasicBlock *BB = PHI_I->getParent();
  for (BasicBlock *PredBB : predecessors(BB)) {
    bool Inexact = false;
    ConstantArgs.clear();
    for (unsigned I = 0; I < F.arg_size(); I++) {
      auto *CAI1 = ACS.getCallArgOperand(I);

      if (!PhiValues.count(CAI1))
        continue;

      auto PHI = cast<PHINode>(CAI1);
      Value *C = PHI->getIncomingValueForBlock(PredBB);
      if (isa<Constant>(C) || isSpecializationCloningSpecialConst(C, PHI)) {
        ConstantArgs.push_back({I, C});
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
      if (!InexactArgsSetsCallList.count(CI))
        InexactArgsSetsCallList.insert(CI);
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
  auto &ACallArgs = AllCallsArgumentsSets[CI];
  std::copy(CallArgumentsSets.begin(), CallArgumentsSets.end(),
            std::back_inserter(ACallArgs));

  CurrCallList.push_back(ACS);

  // Dump arg sets
  LLVM_DEBUG({
    dbgs() << "    Args sets collected \n";
    if (InexactArgsSetsCallList.count(CI)) {
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
static bool analyzeCallForSpecialization(Function &F,
                                         ModifiableAbstractCallSite &ACS,
                                         LoopInfo **LI) {
  // Current implementations of collectPHIsForSpecialization
  // and applyHeuristicsForSpecialization do not support callbacks
  if (ACS.isCallbackCall())
    return false;

  SmallPtrSet<Value *, 8> PhiValues;
  auto *CI = cast<CallInst>(ACS.getInstruction());
  // Collect PHINodes that are passed as arguments for cloning
  // if possible.
  PhiValues.clear();
  if (!collectPHIsForSpecialization(F, *CI, PhiValues))
    return false;

  // Using Loop based heuristics here and remove
  // PHI nodes from PhiValues if not useful in callee.
  // Reuse LoopInfo if it is already available.
  if (!*LI)
    *LI = new LoopInfo(DominatorTree(const_cast<Function &>(F)));

  if (!applyHeuristicsForSpecialization(F, *CI, PhiValues, *LI))
    return false;

  // Collect argument sets for specialization.
  collectArgsSetsForSpecialization(F, ACS, PhiValues);
  return true;
}

// Analyze all CallSites of 'F' and collect CallSites and argument-sets
// for specialization cloning if possible.
//
static void analyzeCallSitesForSpecializationCloning(Function &F) {
  LoopInfo *LI = nullptr;
  if (!IPSpecializationCloning) {
    LLVM_DEBUG(dbgs() << "   Specialization cloning disabled \n");
    return;
  }
  for (auto &U : F.uses()) {
    ModifiableAbstractCallSite ACS(&U);
    if (!ACS || !ACS.isCallee(&U) || !isa<CallInst>(ACS.getInstruction()) ||
        ACS.getNumArgOperands() < F.arg_size())
      continue;
    analyzeCallForSpecialization(F, ACS, &LI);
  }
  // All CallSites of 'F' are analyzed. Delete if
  // LoopInfo is computed.
  if (LI)
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

  for (Use &U : F.uses()) {
    ModifiableAbstractCallSite ACS(&U);

    // Ignore if use of function is not a call or when the count of formal
    // arguments is less than the count of actual arguments. It's acceptable
    // when the actual arguments are greater than or equal to the formal
    // arguments, but the reverse is considered incorrect. This erroneous
    // situation should be avoided in the code. To prevent compiler errors in
    // such cases, we must filter out these incorrect scenarios.
    if (!ACS || !ACS.isCallee(&U) || !isa<CallInst>(ACS.getInstruction()) ||
        ACS.getNumArgOperands() < F.arg_size()) {
      FunctionAddressTaken = true;
      continue;
    }

    // Collect constant values for each formal
    CurrCallList.push_back(ACS);
    for (auto &A : F.args()) {
      auto *AA = ACS.getCallArgOperand(A);
      if (AA)
        collectConstantArgument(&A, AA, CloneType);
    }
  }
  return FunctionAddressTaken;
}

// Returns true if it a candidate for function-ptr cloning.
// Returns true if it has at least one formal of function pointer type.
//
static bool IsFunctionPtrCloneCandidate(Function &F) {
  for (auto &Arg : F.args())
    for (User *U : Arg.users())
      if (auto CB = dyn_cast<CallBase>(U))
        if (CB->getCalledOperand()->stripPointerCasts() == &Arg)
          return true;
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
  Type *T = AI->getAllocatedType();
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
  if (!BBLoopHeader || !BBLatch)
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
  AllocaInst *AI = nullptr;
  Value *V = SI->getPointerOperand();
  if (auto GEPI = dyn_cast<GetElementPtrInst>(V)) {
    AI = dyn_cast<AllocaInst>(GEPI->getPointerOperand());
    if (!AI || !isRecProGEP(GEPI, AI))
      return false;
  } else {
    AI = dyn_cast<AllocaInst>(V);
    if (!AI)
      return false;
  }
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

  auto FindNextPreHeaderAndLoopHeader = [](BasicBlock *BBLatch,
                                           BasicBlock *BBExit,
                                           BasicBlock **BBPreHeader,
                                           BasicBlock **BBLoopHeader) {
    *BBPreHeader = BBLatch;
    *BBLoopHeader = BBExit;
    if ((*BBLoopHeader)->getSingleSuccessor()) {
      *BBPreHeader = *BBLoopHeader;
      *BBLoopHeader = (*BBLoopHeader)->getSingleSuccessor();
    }
  };

  AllocaInst *AICond = nullptr;
  BasicBlock *BBLatch = nullptr;
  BasicBlock *BBExit = nullptr;
  BasicBlock *BBPreHeader = nullptr;
  BasicBlock *BBLoopHeader = nullptr;
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
  FindNextPreHeaderAndLoopHeader(BBLatch, BBExit, &BBPreHeader, &BBLoopHeader);
  if (!isRecProFalseBranchComplexLoop(BBPreHeader, BBLoopHeader,
                                      AICond, nullptr, &VStoreLower, SILower1,
                                      &BBLatch, &BBExit))
    return false;
  FindNextPreHeaderAndLoopHeader(BBLatch, BBExit, &BBPreHeader, &BBLoopHeader);
  if (!isRecProFalseBranchComplexLoop(BBPreHeader, BBLoopHeader,
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
  Type *GEPT = AI->getAllocatedType();
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
  auto LILBArType = GEPL->getSourceElementType();
  Type *ElTy;
  ElTy = GEPL->getResultElementType();
  Instruction *SILB = Builder.CreateSubscript(
      0, ConstantInt::get(Int64Ty, 1), ConstantInt::get(Int64Ty, 4), GEPL, ElTy,
      ConstantInt::get(Int64Ty, 8));
  // We checked AILower allocates an array type in isRecProAllocaIntArray(),
  // so casting is OK here.
  auto LILBType = cast<ArrayType>(LILBArType)->getElementType();
  LoadInst *LILB8 = Builder.CreateLoad(LILBType, SILB, "LILB8");
  GetElementPtrInst *GEPU = findOrCreateRecProGEP(AIUpper, BBCond);
  auto SIUBArType = GEPU->getSourceElementType();
  ElTy = GEPU->getResultElementType();
  Instruction *SIUB = Builder.CreateSubscript(
      0, ConstantInt::get(Int64Ty, 1), ConstantInt::get(Int64Ty, 4), GEPU, ElTy,
      ConstantInt::get(Int64Ty, 8));
  // We checked AIUpper allocates an array type in isRecProAllocaIntArray(),
  // so casting is OK here.
  auto SIUBType = cast<ArrayType>(SIUBArType)->getElementType();
  LoadInst *LIUB8 = Builder.CreateLoad(SIUBType, SIUB, "LIUB8");
  Value *CmpI = Builder.CreateICmpEQ(LILB8, LIUB8, "CMP8S");
  Builder.CreateCondBr(CmpI, BBCallClone, BBConstStore);
  Builder.SetInsertPoint(BBCallClone);
  SmallVector<Value *, 4> Args;
  for (Argument &Arg : F->args())
    Args.push_back(&Arg);
  CallInst *CI = Builder.CreateCall(FClone, Args);
  // CMPLRLLVM-29047: Create debug info for call, if needed.
  if (DISubprogram *DIS = CI->getCaller()->getSubprogram()) {
    DebugLoc CBDbgLoc = DILocation::get(CI->getContext(), DIS->getScopeLine(),
                                        0, DIS);
    CI->setDebugLoc(CBDbgLoc);
  }
  Builder.CreateRetVoid();
  Builder.SetInsertPoint(BBConstStore);
  Constant *C1 = ConstantInt::get(LILBType, CVLower);
  Builder.CreateStore(C1, SILB);
  Constant *C9 = ConstantInt::get(SIUBType, CVUpper);
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

  auto UpdateLVSV = [](SubscriptInst *SubI,
                       SmallVectorImpl<LoadInst *> &LV,
                       SmallVectorImpl<StoreInst *> &SV) -> bool {
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
    return true;
  };

  for (User *U1 : AI->users()) {
    if (auto GEPI = dyn_cast<GetElementPtrInst>(U1)) {
      if (!isRecProGEP(GEPI, AI)) {
        LV.clear();
        return false;
      }
      for (User *U2 : GEPI->users()) {
        auto SubI = dyn_cast<SubscriptInst>(U2);
        if (!SubI || !isRecProSub(SubI, GEPI)) {
          LV.clear();
          return false;
        }
        if (!UpdateLVSV(SubI, LV, SV))
          return false;
      }
    } else {
      auto SubI = dyn_cast<SubscriptInst>(U1);
      if (!SubI || SubI->getPointerOperand() != AI) {
        LV.clear();
        return false;
      }
      if (!UpdateLVSV(SubI, LV, SV))
        return false;
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
        setCalledFunction(CB, &NewF);
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
        setCalledFunction(CB, &NewF);
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
// If 'IsByRef' is 'true', the recursive progression argument is by reference
// and 'ArgType' is the pointer element type of the recursive progression
// argument, otherwise 'ArgType' is the type of the recursive progression
// argument.  If 'IsCyclic' is 'true', the recursive progression is cyclic.
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
                                       bool IsByRef, Type *ArgType,
                                       bool IsCyclic) {
  int FormalValue = Start;
  Function *FirstCloneF = nullptr;
  Function *LastCloneF = nullptr;
  assert(Count > 0 && "Expecting at least one RecProgression Clone");
  for (unsigned I = 0; I < Count; ++I) {
    ValueToValueMapTy VMap;
    Function *NewF = IPCloneFunction(&F, VMap);
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
    Type *ConstantType = ArgType;
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
      ExtraCloneF = IPCloneFunction(LastCloneF, VMapNew);
      ExtraCloneF->addFnAttr("prefer-inline-rec-pro-clone");
      ExtraCloneF->addFnAttr("contains-rec-pro-clone");
      LLVM_DEBUG(dbgs() << "Extra RecProClone Candidate: "
                        << LastCloneF->getName() << "\n");
    }
    deleteRecProgressionRecCalls(F, *LastCloneF);
    if (ExtraCloneF) {
      int SubCount = 0;
      (void)SubCount;
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
    ModifiableAbstractCallSite &ACS,
    std::vector<std::pair<unsigned, Value *>> &ConstantArgsSet) {
  Function *F = ACS.getCalledFunction();
  for (unsigned APos = 0; APos < F->arg_size(); APos++) {
    // Ignore formals that are not selected by heuristics to reduce
    // code size, compile-time etc
    auto FormalA = F->getArg(APos);
    if (!WorthyFormalsForCloning.count(FormalA))
      continue;

    auto ActualV = ACS.getCallArgOperand(APos);
    auto &ValList = ActualConstantValues[ActualV];
    if (ValList.size() == 0)
      continue;

    Constant *C = *ValList.begin();
    ConstantArgsSet.push_back({APos, C});
  }
}

// For given constant argument set 'ConstantArgs', it returns index
// of the constant argument set in "FunctionAllArgumentsSets".
//
static unsigned getConstantArgumentsSetIndex(
    std::vector<std::pair<unsigned, Value *>> &ConstantArgs) {
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
                                        bool *IsGenRecQualified,
                                        WholeProgramInfo *WPInfo) {

  SmallPtrSet<Value *, 16> PossiblyWorthyFormalsForCloning;
  WorthyFormalsForCloning.clear();
  // Create Loop Info for routine
  LoopInfo LI{DominatorTree(const_cast<Function &>(F))};

  unsigned int f_count = 0;
  (void)f_count;
  unsigned GlobalIFCount = 0;
  unsigned GlobalSwitchCount = 0;
  bool SawPending = false;
  for (auto &Arg : F.args()) {

    f_count++;

    // Ignore formal if it doesn't have any constants at call-sites
    auto &ValList = FormalConstantValues[&Arg];
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
              F, &Arg, &LI, true, IFSwitchHeuristic, &IFCount, &SwitchCount)) {
        if (IFCount + SwitchCount == 0) {
          // Qualified unconditionally under the loop heuristic.
          WorthyFormalsForCloning.insert(&Arg);
          LLVM_DEBUG(dbgs() << "  Selecting FORMAL_" << (f_count - 1) << "\n");
        } else {
          // Qualified under the if-switch heuristic. Mark the formal as
          // pending for now, and qualify it later if the total number of
          // "if" and "switch" values that become constant is great enough.
          SawPending = true;
          GlobalIFCount += IFCount;
          GlobalSwitchCount += SwitchCount;
          PossiblyWorthyFormalsForCloning.insert(&Arg);
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
      WorthyFormalsForCloning.insert(&Arg);
    }
  }

  //
  // Return 'true' if 'WPInfo' determines that this is an AVX2 compilation,
  // but not an advanced AVX2 compilation.
  //
  auto IsNonAdvancedAVX2 = [](WholeProgramInfo *WPInfo) {
    auto AVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasGenericAVX2;
    auto IAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
    return WPInfo && WPInfo->isAdvancedOptEnabled(AVX2) &&
        !WPInfo->isAdvancedOptEnabled(IAVX2);
  };

  if (EnableMorphologyCloning && GlobalIFCount >= IPGenCloningMinIFCount &&
      GlobalSwitchCount >= IPGenCloningMinSwitchCount &&
      !IsNonAdvancedAVX2(WPInfo)) {
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
  std::vector<std::pair<unsigned, Value *>> ConstantArgs;
  for (auto &ACS : CurrCallList) {
    CallInst *CI = cast<CallInst>(ACS.getInstruction());
    ConstantArgs.clear();
    createConstantArgumentsSet(ACS, ConstantArgs);
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
    std::vector<std::pair<unsigned, Value *>> &CArgs, unsigned position) {
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
      if (isConstantArgForCloning(&*AI, *CAI, FuncPtrsClone))
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
      setCalledFunction(CI, ClonedFn);
      NumIPCallsCloned++;
      LLVM_DEBUG(dbgs() << " Replaced Cloned call:   " << *CI << "\n");
    }
  }
}

//
// Class that provides cloning of calls to callback functions within
// a cloning target. The case we are trying to handle has the form:
//    define ... foo(..., arg X, ...) {
//      ...
//      call broker_fxn(..., callback_fxn, ..., arg X, ...)
//      ...
//    }
// Here we are cloning the primary function foo on arg X, which has a
// constant value, and that constant value is passed down to the broker
// function which then passes arg X to the callback function. So, if we
// clone foo for arg X, we also want to clone the callback function for arg X.
// Note that we can handle arbitrary argument sets with this technique.
// Also, an arbitrary number of callback functions for each call to the
// broker function can be considered.
//
// Here are the basic steps in the process. First, before the cloning of
// the primary function, we:
//   (1) createCompleteArgSets(): The standard cloning in cloneFunction()
//       is based on constant values for worthy formals. We use the
//       'CallInstArgumentSetIndexMap' to create an extended version of
//       'FunctionAllArgumentsSets' called CFAAS which contains arg sets
//       for all of the constants on which we will be cloning. This is
//       necessary, because in a key case, we would do no callback cloning
//       unless we consider the full sets of constants over which we clone.
//   (2) createCBVec(): The CBVec is a vector with one element for each
//       clone we are making of the primary function. Each CBVec[I] maps
//       a CallInst to a broker function to a second map. The second map
//       has a key of the form std::pair<unsigned, Function *>, where the
//       the unsigned value is the number of the AbstractCallSite of the
//       broker function, and the Function * refers to the callback function
//       which will be invoked by the broker function. The target of the
//       second map is an arg set for the callback function. Thus, for
//       each clone we are creating of the primary function, we can create
//       clones for the callback functions. We create each CBVec[I] with the
//       following three steps:
//       (a) createCBMap(): Create the initial map by propagating the arg
//           set for this clone through the calls to the broker functions.
//       (b) sortCBMap(): Sort the map so that the args sets are ordered
//           with lower arg indices first.  This is necessary because the
//           map was created by tracing through the values of each of the
//           constant args of the primary function, so the constant args
//           of each callback function are added to the map in arbitrary
//           order.
//       (c) removeConflictsCBMap(): Remove any conflicts in the map. A
//           conflict can happen if the broker function can invoke a
//           paticular callback function more than once with different
//           arguments. Here we reject any cases where the args sets for
//           a particular callback function do not match. We could extend
//           this by taking the intersection of the arg sets, but we expect
//           that to be a rare case.
// During the cloning of the primary function, we call remapCBVec() to
// remap the CallInsts in CBVec from those in the primary function to each
// clone. Once the cloning of the primary function is complete, we create
// the clones of the callback functions by calling cloneCallbackFunctions().
// This is done for each callback function clone by:
//   (1) createCBIMap(): Creating a CBIMap which is a mapping from the
//       CallInsts in each of the clones of the callback functions to an
//       clone index number, similar to 'CallInstArgumentSetIndexMap'.
//   (2) cloneCallbackFunction(): Use the CBIMap to create the specific clone
//       of the callback function.
// Note that this techinique allows us to share callback function clones
// among all of the clones of the primary function.
//

class CallbackCloner {

public:
  CallbackCloner(Function &F) : F(F) {}
  void createCompleteArgSets();
  void createCBVec();
  void remapCBVec(unsigned Index, ValueToValueMapTy &VMap);
  void cloneCallbackFunctions();

private:
  // Types for internal data structures. If the general cloning mechanism
  // is converted to a class, we may want to export these out.
  typedef std::pair<unsigned, Value *> AVTy;
  typedef std::vector<AVTy> AVVecTy;
  typedef std::pair<unsigned, Function *> AFTy;
  typedef MapVector<AFTy, AVVecTy> ACSFMapTy;
  typedef std::map<unsigned, AVVecTy> CFAASTy;
  typedef MapVector<CallInst *, ACSFMapTy> CBMapTy;
  typedef std::vector<CBMapTy> CBVecTy;
  typedef MapVector<CallInst *, unsigned> CBIMapTy;

  // Local objects
  Function &F;     // Primary function being cloned
  CFAASTy CFAAS;   // Complete version of FunctionAllArgumentsSets
  CBVecTy CBVec;   // Vector of maps from primary clone arg sets to
                   //   arg sets of callback functions
  SmallSetVector<Function *, 2> CBCloneSet; // Callback functions eligible to
                                         // be cloned

  // Private functions
  void createCBMap(AVVecTy &AVVec, CBMapTy &CBMap);
  void sortCBMap(CBMapTy &CBMap);
  void removeConflictsCBMap(CBMapTy &CBMap);
  void createCBIMap(Function &F, CBIMapTy &CBIMap);
  void cloneCallbackFunction(Function &F, CBIMapTy &CBIMap);
};

//
// Create CFAAS, the complete version of FunctionAllArgumentsSets.
//
void CallbackCloner::createCompleteArgSets() {
  auto &CIASIMap = CallInstArgumentSetIndexMap;
  for (unsigned I = 0, IE = CurrCallList.size(); I != IE; ++I) {
    CallInst *CI = cast<CallInst>(CurrCallList[I].getInstruction());
    auto CIIt = CIASIMap.find(CI);
    if (CIIt == CIASIMap.end())
      continue;
    unsigned Index = CIIt->second;
    auto CFIt = CFAAS.find(Index);
    if (CFIt == CFAAS.end()) {
     std::vector<std::pair<unsigned, Value *>> ConstantArgs;
     for (unsigned J = 0, JE = CI->arg_size(); J != JE; ++J)
       if (auto C = dyn_cast<Constant>(CI->getArgOperand(J)))
         ConstantArgs.push_back(std::make_pair(J, C));
     auto &CArgs = CFAAS[Index];
     std::copy(ConstantArgs.begin(), ConstantArgs.end(),
         std::back_inserter(CArgs));
   } else {
     auto &CArgs = CFIt->second;
     for (unsigned J = 0, JE = CI->arg_size(); J != JE; ++J)
       if (auto C = dyn_cast<Constant>(CI->getArgOperand(J)))
         for (unsigned K = 0, KE = CArgs.size(); K != KE; ++K)
           if (CArgs[K].first == J) {
             if (CArgs[K].second == C)
               break;
             CArgs.erase(CArgs.begin() + K);
             break;
           }
    }
  }
}

//
// Create a 'CBMap', given the vector of arg sets 'AVVec' for a specific
// clone of the primary function.
//
void CallbackCloner::createCBMap(AVVecTy &AVVec, CBMapTy &CBMap) {
  for (unsigned I = 0, E = AVVec.size(); I != E; ++I) {
    Argument *A = F.getArg(AVVec[I].first);
    Value *VC = AVVec[I].second;
    SmallVector<Value *, 16> Worklist;
    Worklist.push_back(A);
    while (!Worklist.empty()) {
      Value *V = Worklist.pop_back_val();
      for (User *U : V->users()) {
        if (auto CI = dyn_cast<CastInst>(U)) {
          Worklist.push_back(CI);
        } else if (auto CB = dyn_cast<CallInst>(U)) {
          SmallVector<const Use *, 4> CallbackUses;
          AbstractCallSite::getCallbackUses(*CB, CallbackUses);
          unsigned ACDIndex = 0;
          for (const Use *UU : CallbackUses) {
            AbstractCallSite ACS(UU);
            if (!ACS.isCallbackCall())
              continue;
            Function *F = ACS.getCalledFunction();
            for (unsigned J = 0, JE = ACS.getNumArgOperands(); J != JE; ++J) {
              if (ACS.getCallArgOperand(J) == V) {
                ACSFMapTy &ACSFMap = CBMap[CB];
                auto P = std::make_pair(ACDIndex, F);
                CBCloneSet.insert(F);
                ACSFMap[P].push_back(std::make_pair(J, VC));
              }
            }
          }
          ACDIndex++;
        }
      }
    }
  }
}

//
// Sort the 'CBMap' so that the args sets are with lower numbered
// arguments first.
//
void CallbackCloner::sortCBMap(CBMapTy &CBMap) {
  for (auto I = CBMap.begin(), IE = CBMap.end(); I != IE; ++I) {
    ACSFMapTy &AMap = I->second;
    for (auto J = AMap.begin(), JE = AMap.end(); J != JE; ++J) {
      auto &Vec = J->second;
      std::sort(Vec.begin(), Vec.end(), [](AVTy A, AVTy B)
                                          { return A.first < B.first; });
    }
  }
}

//
// Remove conflicts from the 'CBMap', so that there are no conflicting
// arg sets for any callback function.  For example, if we have the
// map entries:
//    (0, callback_function_A) -> ((0, 5), (1, 10))
//    (1, callback_function_A) -> ((0, 7), (1, 10))
// we remove the entries from the 'CBMap'.  But if we have:
//    (0, callback_function_A) -> ((0, 5), (1, 10))
//    (1, callback_function_A) -> ((0, 5), (1, 10))
// we retain them.
//
void CallbackCloner::removeConflictsCBMap(CBMapTy &CBMap) {
  bool SawConflict = false;
  for (auto I = CBMap.begin(), IE = CBMap.end(); I != IE; ++I) {
    ACSFMapTy &AMap = I->second;
    for (auto J = AMap.begin(), JE = AMap.end(); J != JE; ++J) {
      unsigned J1 = J->first.first;
      Function *JF = J->first.second;
      auto &J2Vec = J->second;
      if (J2Vec.empty())
        continue;
      for (auto K = next(J, 1), KE = AMap.end(); K != KE; ++K) {
        unsigned K1 = K->first.first;
        Function *KF = K->first.second;
        auto &K2Vec = K->second;
        if (K2Vec.empty())
          continue;
        if (J1 != K1 && JF == KF && J2Vec != K2Vec) {
          SawConflict = true;
          J2Vec.clear();
          K2Vec.clear();
        }
      }
    }
  }
  if (!SawConflict)
    return;
  for (auto I = CBMap.begin(), IE = CBMap.end(); I != IE; ++I) {
    ACSFMapTy &AMap = I->second;
    AMap.remove_if([&](std::pair<AFTy, AVVecTy> &MapEntry) {
        return (MapEntry.second.empty()); });
  }
  CBMap.remove_if([&](std::pair<CallInst *, ACSFMapTy> &MapEntry) {
      return (MapEntry.second.empty()); });
}

//
// Create the 'CBVec' which for each clone of the primary function maps
// the arg sets of that clone to the args sets of the callback functions
// called by the clone.
//
void CallbackCloner::createCBVec() {
  for (auto I = CFAAS.begin(), E = CFAAS.end(); I != E; ++I) {
    CBMapTy CBMap;
    createCBMap(I->second, CBMap);
    sortCBMap(CBMap);
    removeConflictsCBMap(CBMap);
    CBVec.push_back(CBMap);
  }
}

//
// Remap 'CBVec[Index]' using 'VMap' to use the CallInsts to broker
// functions in the clone rather than those in the primary function.
//
void CallbackCloner::remapCBVec(unsigned Index, ValueToValueMapTy &VMap) {
  auto &CBMap = CBVec[Index];
  CBMapTy CBNewMap;
  for (auto I = CBMap.begin(), E = CBMap.end(), II = I; I != E; I = II) {
    II = next(I, 1);
    CallInst *CB = I->first;
    ACSFMapTy &ACSFMap = I->second;
    if (auto CBNew = dyn_cast<CallInst>(VMap[CB]))
      CBNewMap[CBNew] = ACSFMap;
  }
  CBVec[Index] = CBNewMap;
}

//
// Create a 'CBIMap', similar to 'CallInstArgumentSetIndexMap' for 'F',
// which is a callback function eligible for cloning.
//
void CallbackCloner::createCBIMap(Function &F, CBIMapTy &CBIMap) {
  CFAASTy FAMap;
  for (unsigned J = 0, JE = CBVec.size(); J != JE; ++J) {
    auto &CBMap = CBVec[J];
    for (auto K = CBMap.begin(), KE = CBMap.end(); K != KE; ++K) {
      CallInst *CB = K->first;
      auto &AMap = K->second;
      for (auto L = AMap.begin(), LE = AMap.end(); L != LE; ++L) {
        Function *TF = L->first.second;
        if (TF != &F)
          continue;
        std::vector<AVTy> &AV = L->second;
        bool FoundIndex = false;
        unsigned Index = 0;
        for (auto M = FAMap.begin(), ME = FAMap.end(); M != ME; ++M) {
          if (M->second == AV) {
            FoundIndex = true;
            CBIMap[CB] = Index;
            break;
           }
           ++Index;
        }
        if (!FoundIndex) {
          CBIMap[CB] = Index;
          std::vector<AVTy> &CArgs = FAMap[Index];
          std::copy(AV.begin(), AV.end(), std::back_inserter(CArgs));
        }
      }
    }
  }
}

//
// Use the 'CBIMap' to create a clone of the callback function 'F'.
//
void CallbackCloner::cloneCallbackFunction(Function &F, CBIMapTy &CBIMap) {
  MapVector<unsigned, Function *> FCloneMap;
  for (auto I = CBIMap.begin(), IE = CBIMap.end(); I != IE; ++I) {
    CallInst *CB = I->first;
    unsigned Index = I->second;
    Function *NewF = FCloneMap[Index];
    if (!NewF) {
      ValueToValueMapTy VMap;
      NewF = IPCloneFunction(&F, VMap);
      FCloneMap[Index] = NewF;
      NumIPCloned++;
    }
    SmallVector<const Use *, 4> CallbackUses;
    AbstractCallSite::getCallbackUses(*CB, CallbackUses);
    for (const Use *UU : CallbackUses) {
      AbstractCallSite ACS(UU);
      assert(ACS.isCallbackCall() && "Expecting callback call");
      if (ACS.getCalledFunction() == &F) {
        unsigned I = ACS.getCallArgOperandNoForCallee();
        std::vector<Value *> Args(CB->op_begin(), CB->op_end() - 1);
        if (auto BCO = dyn_cast<BitCastOperator>(Args[I]))
          Args[I] = ConstantExpr::getBitCast(NewF, BCO->getDestTy());
        else
          Args[I] = NewF;
        std::string New_Name = CB->hasName() ? CB->getName().str() +
            ".clone.callback.cs" : "";
        auto NewCI = CallInstCreate(CB, CB->getCalledFunction(), Args,
            New_Name, CB);
        NewCI->setDebugLoc(CB->getDebugLoc());
        NewCI->setCallingConv(CB->getCallingConv());
        NewCI->setAttributes(CB->getAttributes());
        CB->eraseFromParent();
        LLVM_DEBUG(dbgs() << " Cloned callback in "
                          << NewCI->getCaller()->getName()<< ":   "
                          << *NewCI << "\n");
        NumIPCallsCloned++;
      }
    }
  }
}

//
// Create clones of all of the callback functions, as needed.
//
void CallbackCloner::cloneCallbackFunctions() {
  for (auto I = CBCloneSet.begin(), E = CBCloneSet.end(); I != E; ++I) {
    Function *F = *I;
    MapVector<CallInst *, unsigned> CBIMap;
    createCBIMap(*F, CBIMap);
    cloneCallbackFunction(*F, CBIMap);
  }
}

// End of code for class CallbackCloner

// The function operates using a freshly cloned function, and when dealing with
// arguments, it identifies cases where zero-initialized, read-only global
// arrays are employed, subsequently substituting all usages of elements from
// these arrays with zeros.
static void updateGlobalArraysUses(AbstractCallSite &ACS) {
  if (!ACS)
    return;

  Function *F = ACS.getCalledFunction();
  if (!F)
    return;

  SmallSetVector<LoadInst *, 32> LoadsToReplace;
  for (auto &FormalA : F->args()) {
    auto *ActualA = ACS.getCallArgOperand(FormalA);
    if (!ActualA)
      continue;

    auto GV = dyn_cast<GlobalVariable>(ActualA);
    if (!GV)
      continue;

    if (!isGlobalConstZeroInitializedArray(GV))
      continue;

    SmallSetVector<Use *, 32> WL;
    for (auto &U : FormalA.uses())
      WL.insert(&U);

    for (unsigned I = 0; I < WL.size(); I++) {
      auto *CUse = WL[I];
      auto *CUser = CUse->getUser();
      if (auto LI = dyn_cast<LoadInst>(CUser)) {
        if (LI->getPointerOperand() == CUse->get())
          LoadsToReplace.insert(LI);
      } else if (isa<GEPOperator>(CUser)) {
        for (auto &U : CUser->uses())
          WL.insert(&U);
      }
    }
  }

  for (auto *LI : LoadsToReplace) {
    auto Zero = Constant::getNullValue(LI->getType());
    LI->replaceAllUsesWith(Zero);
  }
}

using SameArgSet = SmallVector<unsigned, 8>;
using SameArgSetCollection = SmallVector<SameArgSet, 8>;
// The function gathers formal argument sets from a function.
// These gathered sets specifically include formal arguments that accept
// identical actual arguments, allowing for potential merging.
// In this context, "merging" refers to the ability to replace any formal
// argument within a detected set with another.
//
// Example:
// If a function "foo" has the following body:
// int foo(int a, int* p, int *q) {
//  return a + p[0] + q[1];
// }
//
// And it is called in the manner:
//
// int A[] = {0, 1};
// int B[] = {2, 3};
// int r1 = foo(1, A, A);
// int r2 = foo(0, B, B);
//
// We want function foo to be transformed into:
// int foo(int a, int* p, int *q) {
//  return a + p[0] + p[1];
//                    ^^^^
// }
// So it uses "p" instead of "q" inside the function.
static SameArgSetCollection getSameArgSets(Function *F) {
  LLVM_DEBUG({
    dbgs() << "[IP_CLONING][ARG merge]: Analysing Function(";
    F->printAsOperand(dbgs());
    dbgs() << ")\n";
  });

  SameArgSetCollection Result;
  constexpr size_t NARGS = 64;
  SmallVector<std::bitset<NARGS>> CurrSets;
  bool FirstIter = true;
  for (auto &U : F->uses()) {
    AbstractCallSite ACS(&U);
    if (!ACS)
      return std::move(Result);

    SmallDenseMap<Value *, std::bitset<NARGS>> TmpArgSetMap;
    for (auto &FormalA : F->args()) {
      if (FormalA.getArgNo() >= NARGS)
        continue;
      auto *ActualA = ACS.getCallArgOperand(FormalA);
      if (!ActualA)
        continue;
      TmpArgSetMap[ActualA][FormalA.getArgNo()] = 1;
    }
    if (FirstIter) {
      FirstIter = false;
      for (const auto &[ActualA, ArgSet] : TmpArgSetMap)
        CurrSets.push_back(ArgSet);
      continue;
    }

    for (auto &S : CurrSets) {
      size_t MaxCount = 0;
      std::bitset<NARGS> MaxResultSet;
      for (const auto &[ActualA, ArgSet] : TmpArgSetMap) {
        auto RSet = S & ArgSet;
        size_t Cnt = RSet.count();
        if (Cnt > MaxCount) {
          MaxCount = Cnt;
          MaxResultSet = RSet;
        }
      }
      S = MaxResultSet;
    }
  }

  for (auto &S : CurrSets) {
    if (S.count() < 2)
      continue;
    Result.emplace_back();
    for (size_t I = 0; I < S.size(); I++)
      if (S.test(I))
        Result.back().push_back(I);
  }

  LLVM_DEBUG({
    if (Result.empty())
      dbgs() << "[IP_CLONING][ARG merge]: No mergeable arguments detected.\n";
    else
      dbgs() << "[IP_CLONING][ARG merge]: Detected " << Result.size()
             << " mergeable argument sets.\n";

    for (unsigned I = 0; I < Result.size(); I++) {
      dbgs() << "  [" << I + 1 << "]: ";
      for (auto ArgNo : Result[I]) {
        dbgs() << ArgNo << " ";
      }
      dbgs() << "\n";
    }
  });
  return std::move(Result);
}

// Auxilary function to define if attribute refers to memory
// access type for a function parameter.
static bool isMemAccessAttr(Attribute::AttrKind AKind) {
  return AKind == Attribute::ReadNone || AKind == Attribute::ReadOnly ||
         AKind == Attribute::WriteOnly;
}

static bool isMemAccessAttr(Attribute A) {
  return A.isEnumAttribute() && isMemAccessAttr(A.getKindAsEnum());
}

// The function categorizes a function's parameter attributes into two groups:
// memory access type and all other attributes. The non-memory attributes are
// placed in the initial slot of the resulting tuple, while the memory access
// type is placed in the second slot.
static std::tuple<AttrBuilder, std::optional<Attribute::AttrKind>>
splitAttributes(Function *F, unsigned ArgNo) {
  const auto &Attrs = F->getAttributes();
  auto &Ctx = F->getContext();
  AttrBuilder AB(Ctx);
  std::optional<Attribute::AttrKind> MemAccessTypeAttr;
  for (const auto &A : Attrs.getParamAttrs(ArgNo)) {
    if (isMemAccessAttr(A)) {
      MemAccessTypeAttr = A.getKindAsEnum();
      continue;
    }
    AB.addAttribute(A);
  }

  // If memory access type attributes are absent for a formal function
  // argument, and that argument is a pointer, we designate the result as
  // Attribute::None. This serves as a signal for the subsequent merge logic
  // to recognize this specific situation.
  if (!MemAccessTypeAttr && F->getArg(ArgNo)->getType()->isPointerTy())
    MemAccessTypeAttr = Attribute::None;

  return {std::move(AB), std::move(MemAccessTypeAttr)};
}

// The table of memory access attributes merge rules.
// A / O     | None | ReadNone  | ReadOnly  | WriteOnly
// ----------+------+-----------+-----------+----------
// None      | None | None      | None      | None
// ReadNone  | None | ReadNone  | ReadOnly  | WriteOnly
// ReadOnly  | None | ReadOnly  | ReadOnly  | None
// WriteOnly | None | WriteOnly | None      | WriteOnly

// The function merges memory access attributes and stores result in
// the first parameter.
static void updateMemAccessAttr(Attribute::AttrKind &A, Attribute::AttrKind O) {
  if (A == O || A == Attribute::None || O == Attribute::ReadNone)
    return;
  if (A == Attribute::ReadNone)
    A = O;
  else
    A = Attribute::None;
}

static void updateMemAccessAttr(std::optional<Attribute::AttrKind> &A,
                                const std::optional<Attribute::AttrKind> O) {
  if (A == O || !O)
    return;
  if (!A)
    A = O;
  else
    updateMemAccessAttr(A.value(), O.value());
}

// The function evaluates whether it's possible to merge the attributes of
// the parameters. If this assessment is successful, it provides the merged
// memory access attribute as the second element in the resulting tuple, while
// the result of the validity check is shown as the first element in the tuple.
// To put it simply, the feasibility check assesses whether the attributes,
// excluding memory access attributes, match across all entries in "SAS"
// parameter.
static std::tuple<bool, std::optional<Attribute::AttrKind>>
getMergedAttributeSet(Function *F, const SameArgSet &SAS) {
  if (SAS.empty())
    return {false, std::nullopt};
  unsigned BaseArgNo = SAS.front();
  auto [AB, MemAccessTypeAttr] = splitAttributes(F, BaseArgNo);
  auto ArgTy = F->getArg(BaseArgNo)->getType();
  for (unsigned ArgNo : SAS) {
    if (ArgNo == BaseArgNo)
      continue;
    if (ArgTy != F->getArg(ArgNo)->getType())
      return {false, std::nullopt};
    auto [TAB, TMemAccessTypeAttr] = splitAttributes(F, ArgNo);
    if (AB != TAB)
      return {false, std::nullopt};
    updateMemAccessAttr(MemAccessTypeAttr, TMemAccessTypeAttr);
  }
  return {true, MemAccessTypeAttr};
}

// The function alters the usage of formal function parameters by substituting
// them with one chosen from the recognized set of interchangeable parameters.
static void mergeArgs() {

  for (auto *F : ClonedFunctionList) {
    auto SameArgsSetCollection = getSameArgSets(F);
    for (auto &AS : SameArgsSetCollection) {
      auto [Mergeable, MemAttr] = getMergedAttributeSet(F, AS);
      if (!Mergeable)
        continue;
      // Pick the first arg as base
      unsigned BaseArgNo = AS.front();
      auto *BaseArg = F->getArg(BaseArgNo);
      if (MemAttr) {
        BaseArg->removeAttr(Attribute::ReadNone);
        BaseArg->removeAttr(Attribute::ReadOnly);
        BaseArg->removeAttr(Attribute::WriteOnly);
        if (MemAttr != Attribute::None)
          BaseArg->addAttr(MemAttr.value());
        LLVM_DEBUG({
          dbgs() << "[IP_CLONING][ARG merge]: Set attribute for (";
          BaseArg->printAsOperand(dbgs());
          dbgs() << ") to " << Attribute::getNameFromAttrKind(MemAttr.value());
          dbgs() << "\n";
        });
      }
      // Replace all other args with the base
      for (unsigned ArgNo : AS) {
        if (ArgNo == BaseArgNo)
          continue;
        auto Arg = F->getArg(ArgNo);
        Arg->replaceAllUsesWith(BaseArg);
        LLVM_DEBUG({
          dbgs() << "[IP_CLONING][ARG merge]: Function(";
          F->printAsOperand(dbgs());
          dbgs() << ") - ARG(";
          Arg->printAsOperand(dbgs());
          dbgs() << ") replaced with ";
          BaseArg->printAsOperand(dbgs());
          dbgs() << "\n";
        });
      }
    }
  }
}

// It does actual cloning and fixes recursion calls if possible. If
// 'AttemptCallbackCloning' is 'true', attempt to also clone the callback
// functions which obtain constant arguments through the cloning of the
// primary source function.
//
static void cloneFunction(bool AttemptCallbackCloning) {
  std::unique_ptr<CallbackCloner> CBCloner;
  Function *SrcFn = CurrCallList[0].getCalledFunction();
  if (AttemptCallbackCloning) {
    CBCloner = std::make_unique<CallbackCloner>(*SrcFn);
    CBCloner->createCompleteArgSets();
    CBCloner->createCBVec();
  }

  for (unsigned I = 0, E = CurrCallList.size(); I != E; ++I) {
    auto &ACS = CurrCallList[I];
    CallInst *CI = cast<CallInst>(ACS.getInstruction());

    // Skip callsite if  no constant argument set is collected.
    auto MapIt = CallInstArgumentSetIndexMap.find(CI);
    if (MapIt == CallInstArgumentSetIndexMap.end())
      continue;

    // Get cloned function for constant argument set if it is already there
    unsigned index = MapIt->second;
    Function *NewFn = ArgSetIndexClonedFunctionMap[index];
    // Create new clone if it is not there for constant argument set
    if (NewFn == nullptr) {
      ValueToValueMapTy VMap;
      NewFn = IPCloneFunction(SrcFn, VMap);
      if (CBCloner)
        CBCloner->remapCBVec(index, VMap);
      ArgSetIndexClonedFunctionMap[index] = NewFn;
      ClonedFunctionList.insert(NewFn);
      NumIPCloned++;
    }

    ACS.setCalledOperand(NewFn);
    LLVM_DEBUG(dbgs() << " Cloned call:   " << *CI << "\n");

    NumIPCallsCloned++;
    eliminateRecursionIfPossible(NewFn, SrcFn, index);
    updateGlobalArraysUses(ACS);
  }

  if (CBCloner)
    CBCloner->cloneCallbackFunctions();
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
static GetElementPtrInst *
createGEPAtFrontInClonedFunction(Function *NewFn, GlobalVariable *BaseAddr,
                                 unsigned NumIndices) {

  Type *Int32Ty;
  SmallVector<Value *, 4> Indices;

  Instruction *InsertPt = &NewFn->begin()->front();
  Int32Ty = Type::getInt32Ty(NewFn->getContext());
  // Create Indices with zero value.
  for (unsigned I = 0; I < NumIndices; I++)
    Indices.push_back(ConstantInt::get(Int32Ty, 0));

  GetElementPtrInst *Rep = GetElementPtrInst::CreateInBounds(
      BaseAddr->getValueType(), BaseAddr, Indices, "", InsertPt);
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
                                        Argument *Formal, Instruction *CallI,
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
    auto GV = cast<GlobalVariable>(PropValue);
    Rep = createGEPAtFrontInClonedFunction(NewFn, GV, NumIndices);
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

  GetPointerToArrayDims(Formal, SizeInBytes, NumElems, DL);
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

    Argument *Formal = &*AI;

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
  std::vector<std::pair<unsigned, Value *>> ConstantArgs;
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
    ConstantArgs.push_back(std::make_pair(Position, V));
  }
  unsigned Index = getConstantArgumentsSetIndex(ConstantArgs);
  Function *NewFn = ArgSetIndexClonedFunctionMap[Index];

  ValueToValueMapTy VMap;
  CallInst *New_CI;
  // Create new cloned function for ConstantArgs if it is not already
  // there.
  if (NewFn == nullptr) {
    NewFn = IPCloneFunction(SrcFn, VMap);
    ArgSetIndexClonedFunctionMap[Index] = NewFn;
    ClonedFunctionList.insert(NewFn);
    propagateArgumentsToClonedFunction(NewFn, ArgsIndex, &CI);
    NumIPCloned++;
  }
  std::vector<Value *> Args(CI.op_begin(), CI.op_end() - 1);
  // NameStr should be "" if return type is void.
  std::string New_Name;
  New_Name = CI.hasName() ? CI.getName().str() + ".clone.spec.cs" : "";
  New_CI = CallInstCreate(&CI, NewFn, Args, New_Name, Insert_BB);
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
    CallInst *CI = cast<CallInst>(CurrCallList[I].getInstruction());
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
      NewCI->insertInto(CallBB, CallBB->end());
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
    OrigBB->back().eraseFromParent();
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
  for (auto &F : M.getFunctionList()) {
    if (!F.hasFnAttribute("target-cpu"))
      continue;
    StringRef TCA = F.getFnAttribute("target-cpu").getValueAsString();
    if (TCA != "skylake-avx512") {
      LLVM_DEBUG(dbgs() << "No AVX512->AVX2 conversion: Not skylake-avx512\n");
      return false;
    }
    StringRef TFA = F.getFnAttribute("target-features").getValueAsString();
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

  for (auto &F : M.getFunctionList()) {
    if (!F.hasFnAttribute("target-cpu"))
      continue;
    assert(F.getFnAttribute("target-cpu").getValueAsString() ==
               "skylake-avx512" &&
           "Expecting skylake-avx512");
    llvm::AttrBuilder Attrs(F.getContext());
    F.removeFnAttr("target-cpu");
    F.removeFnAttr("target-features");
    Attrs.addAttribute("target-cpu", "core-avx2");
    Attrs.addAttribute("target-features", AVX2Attributes);
    F.addFnAttrs(Attrs);
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
      for (Value *V : CB->args()) {
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
  SetVector<BasicBlock *> Visited;
  // The join point through which 'F' will be split. Predecessors of 'BBSplit'
  // will go into 'F1', while successors will go into 'F2'.
  BasicBlock *BBSplit;
  // The unique BasicBlock in 'F' which has a ReturnInst.
  BasicBlock *BBReturn;
  // Instructions which are defined in the 'Visited' blocks, but used in the
  // non-'Visited' blocks. We must resolve each of these in order to split
  // 'F' into 'F1' and 'F2'
  SetVector<Instruction *> SplitInsts;
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

  auto NoteCounts = [&](Function &F,
                        SetVector<BasicBlock *> &Visited) {
    LLVM_DEBUG({
      unsigned VisitedCount = 0;
      unsigned NonVisitedCount = 0;
      for (auto &BB : F)
        if (Visited.count(&BB))
          VisitedCount++;
        else
          NonVisitedCount++;
      dbgs() << "MRCS: Can split blocks: Visited: " << VisitedCount
             << " NonVisted: " << NonVisitedCount << "\n";
    });
  };

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
      if (Visited.insert(S))
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
    // Use the switch block if there is no unique precessor.
    Visited.remove(BBSwitch0);
    Visited.remove(BBReturn0);
    BBSplit = BBSwitch0;
    BBReturn = BBReturn0;
    NoteCounts(*F, Visited);
    return true;
  }
  Visited.remove(BBSwitch0);
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
    Visited.remove(BB);
  }
  Visited.remove(BBSplit0);
  BBSplit = BBSplit0;
  Visited.remove(BBReturn0);
  BBReturn = BBReturn0;
  NoteCounts(*F, Visited);
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

  auto IsToLifetime = [this](User *U, BasicBlock *BB) -> bool {
    auto II = dyn_cast<Instruction>(U);
    if (!II)
      return false;
    if (!Visited.count(II->getParent()))
      return true;
    if (II->getParent() != BB)
      return false;
    auto CI = dyn_cast<CallInst>(II);
    if (!CI)
      return false;
    Function *F = CI->getCalledFunction();
    if (!F || F->getIntrinsicID() != Intrinsic::lifetime_start)
      return false;
    return true;
  };

  auto IsBitCastToLifetime = [&IsToLifetime](Instruction *I,
                                             BasicBlock *BB) -> bool {
    if (auto BC = dyn_cast<BitCastInst>(I)) {
      if (BC->getParent() != BB)
        return false;
      for (User *U : BC->users())
        if (!IsToLifetime(U, BB))
          return false;
    } else if (!IsToLifetime(I, BB)) {
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
  if ((BC = dyn_cast<BitCastInst>(V)))
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
  if (!GEPI->hasOneUse() || (BC && !BC->hasOneUse())) {
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
        for (unsigned I = 0, E = CI->arg_size(); I < E; ++I)
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
        GetElementPtrInst *GEPIX = nullptr;
        if (auto BC = dyn_cast<BitCastInst>(V)) {
          V = BC->getOperand(0);
          GEPIX = dyn_cast<GetElementPtrInst>(V);
          if (!GEPIX)
            return false;
          const DataLayout &DL = BC->getModule()->getDataLayout();
          if (!BC->getSrcTy()->isPointerTy())
            return false;
          if (!BC->getDestTy()->isPointerTy())
            return false;
          Type *TySrcE = GEPIX->getResultElementType();
          Type *TyDestE = SI->getValueOperand()->getType();
          if (DL.getTypeSizeInBits(TySrcE) != DL.getTypeSizeInBits(TyDestE))
            return false;
        } else {
          GEPIX = dyn_cast<GetElementPtrInst>(V);
          if (!GEPIX)
            return false;
        }
        if (GEPIX->getSourceElementType() != GEPI->getSourceElementType())
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
      if (auto BC = dyn_cast<BitCastInst>(I)) {
        MoveSet.push_back(BC);
        for (User *V : BC->users()) {
          auto J = cast<Instruction>(V);
          if (J->getParent() != BB)
            continue;
          auto CI = cast<CallInst>(J);
          MoveSet.push_back(CI);
        }
      } else {
        auto J = cast<Instruction>(I);
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
  Type *Ty = GEPI->getSourceElementType();
  auto GEPINew =
      GetElementPtrInst::Create(Ty, GEPIPO, Idxs, "", InstInsertBefore);
  moveNonVisitedUses(GEPI, GEPINew);
}

void Splitter::reloadPHI(PHINode *PHIN) {
  GetElementPtrInst *GEPI = PHIReloadMap[PHIN];
  assert(GEPI != nullptr && "Expecting PHINode in map");
  auto GEPINew = GEPI->clone();
  GEPINew->insertBefore(InstInsertBefore);
  Type *LType = GEPI->getResultElementType();
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
  // This is a special case that does not fit the standard wrapper functions
  // defined above for use with the classic inlining report.
  CloneFunctionInto(NewF, F, VMap, CloneFunctionChangeType::LocalChangesOnly,
                    Rets);
  Argument *ArgLast = nullptr;
  for (auto &ArgNew : NewF->args())
    ArgLast = &ArgNew;
  *Arg = ArgLast;
  *NewSplitValue = cast<PHINode>(VMap[SplitValue]);
  *NewBBSplit = cast<BasicBlock>(VMap[BBSplit]);
  // CMPLRLLVM-37247: Mark this split function so that we can inhibit
  // argument promotion on it.
  NewF->addFnAttr("ip-clone-split-function");
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
    std::vector<Value *> Args;
    for (Value *V : CI->args())
      Args.push_back(V);
    Args.push_back(AI);
    CallInst *CLI1 = CallInstCreate(CI, F1, Args, "", CI);
    CLI1->setDebugLoc(CI->getDebugLoc());
    LoadInst *LI = new LoadInst(NewTy1, AI, "", false, Align(4), CI);
    CmpInst *IC =
        ICmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_EQ, LI, CI1, "", CI);
    auto NewBBT = SplitBlockAndInsertIfThen(IC, CI, false);
    Args.clear();
    for (Value *V : CI->args())
      Args.push_back(V);
    Args.push_back(CLI1);
    CallInst *CLI2 = CallInstCreate(CI, F2, Args, "", NewBBT);
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
      CB->addFnAttr("prefer-inline-mrc-split");
      LLVM_DEBUG(dbgs() << "MRCS: Inline " << Caller->getName() << " TO "
                        << F1->getName() << "\n");
    }
    unsigned Count = 0;
    for (unsigned I = 0, E = CB->arg_size(); I < E; ++I) {
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
          CBB->addFnAttr("prefer-inline-mrc-split");
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
  Type *NewPTy1 = llvm::PointerType::getUnqual(C);
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
// Class which performs predicate optimization across a key path in the
// call graph. The path will terminate with a base function 'BaseF',
// which is called from a wrapper function at the call site 'WrapperCB'.
// The wrapper function is called from a big loop function at the call
// site 'BigLoopCB'. The big loop function must have a series of simple
// loops that enclose 'WrapperCB'. The number of loops is saved in
// 'SimpleLoopDepth'.
//

// Make the control flow structure for an if-test of the form:
// if (MyCond)
//   CBClone
// else
//   CB
// where 'MyCond' is in 'BBPred'.
static void makeBlocks(CallBase *CB, CallBase *CBClone, Value *MyCond,
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
  BranchInst::Create(BBTrue, BBofCB, MyCond, BBPred);
  CBClone->insertBefore(BBTrue->getTerminator());
}

class PredicateOpt {
public:
  PredicateOpt(Function *BaseF,
               std::function<DominatorTree &(Function &)> *GetDT,
               std::function<BlockFrequencyInfo &(Function &)> *GetBFI,
               std::function<BranchProbabilityInfo &(Function &)> *GetBPI,
               std::function<TargetLibraryInfo &(Function &)> *GetTLI)
      : BaseF(BaseF), GetDT(GetDT), GetBFI(GetBFI), GetBPI(GetBPI),
        GetTLI(GetTLI), WrapperCB(nullptr), BigLoopCB(nullptr),
        SimpleLoopDepth(0) {}
  // Find 'WrapperCB' and 'BigLoopCB'.
  bool findSpine();
  // Return 'true' if the desired predicate opt should be attempted.
  bool shouldAttemptPredicateOpt();
  // Attempt the desired predicate opt. Return 'true' if it is performed.
  bool doPredicateOpt();

private:
  // The base function.
  Function *BaseF;
  // DominatorTree getter
  std::function<DominatorTree &(Function &)> *GetDT{};
  // BlockFrequencyInfo getter
  std::function<BlockFrequencyInfo &(Function &)> *GetBFI{};
  // BranchProbabilityInfo getter
  std::function<BranchProbabilityInfo &(Function &)> *GetBPI{};
  // TargetLibraryInfo getter
  std::function<TargetLibraryInfo &(Function &)> *GetTLI{};
  // The call site which calls 'BaseF' in the wrapper function.
  CallBase *WrapperCB = nullptr;
  // The call site that calls the wrapper function and is enclosed
  // within a series of simple loops.
  CallBase *BigLoopCB = nullptr;
  // The number of loops within which 'BigLoopCB' is enclosed.
  unsigned SimpleLoopDepth = 0;
  // A loop info cache to store loop infos for functions being evaluated.
  InliningLoopInfoCache ILIC;
  // The loop that will be multiversioned, with the first version being the
  // optimized version, and the second being the general version.
  Loop *MultiLoop = nullptr;
  // The incoming basic block to the multiloop.
  LoadInst *LIRestrict = nullptr;
  // The type of the structure pointed to by 'LIRestrict'.
  StructType *LIRestrictType = nullptr;
  // Optimized version of the key predicate opt code
  Function *OptF = nullptr;
  // MagickRound(mean_location.x)
  Instruction *MLX = nullptr;
  // MagickRound(mean_location.y)
  Instruction *MLY = nullptr;
  // width/2
  Instruction *W2 = nullptr;
  // height/2
  Instruction *H2 = nullptr;
  // Unique reference to Argument 6 Field 1 of 'BaseF', which represents
  // nexus_info->region.
  GetElementPtrInst *GEPI6F1 = nullptr;
  // Return 'true' if 'F' is a wrapper function.
  bool isWrapper(Function &F);
  // Return the number of simple loops enclosing 'CB'.
  unsigned simpleLoopDepth(CallBase &CB);
  // Find the integer result of the llvm.rint.f64() intrinsic used to compute
  // 'ArgNo' of 'CB'.
  Instruction *findRintResult(CallBase *CB, unsigned ArgNo);
  // Find a user of 'Arg' which is an Lshr with right operand 1.
  Instruction *findLShr1User(Argument *Arg);
  // Find the doubly nested inner loop surrounding 'CB' if there is one,
  // otherwise return 'nullptr'.
  Loop *findMultiLoop(CallBase *CB);
  // Return 'true' if 'L' writes only through 'WrapperF' and through
  // stores to a local variable inside the function containing 'L' which
  // will not alias with actual arguments passed to 'WrapperF'.
  bool validateMultiLoop(Loop *L, Function *WrapperF);
  // Find a unique LoadInst with a 'predicate-opt-restrict' metadata in the
  // entry block of 'BaseF'.
  LoadInst *findUniqueRestrictLoadInst(Function *BaseF);
  // Recursive version of findHoistableFields() with 'Depth".
  bool findHoistableFieldsX(Function *F, Value *V, StructType *STy,
                            unsigned Depth, std::set<unsigned> &HoistYes,
                            std::set<unsigned> &HoistNo);
  // Return 'true' if the local restrict variable 'LIRestrict' in the entry
  // block of 'BaseF' can be hoisted past the entry of the wrapper function.
  bool isRestrictVarHoistablePastWrapperF(Function *BaseF,
                                          LoadInst *LIRestrict);
  // Return 'true' if the subfields in field 1 of the structure pointed to
  // by formal argument 6 of the base function are hoistable past the entry
  // of the wrapper function. If we return 'true' set 'GEPI6F1' to the
  // unique GetElementPtrInst * that references field 1.
  bool isBaseFArg6Field1Hoistable(Function *BaseF, GetElementPtrInst *&GEPI6F1);
  // Find the fields referenced by 'V' of type 'STy' in 'F' that are hoistable
  // and put their indices in 'HoistYes' and 'HoistNo'.
  bool findHoistableFields(Function *F, Value *V, StructType *STy,
                           std::set<unsigned> &HoistYes,
                           std::set<unsigned> &HoistNo);
  // Return a pointer to a newly created hoisted restrict variable that
  // can be used in the condition testing for the optimized code.
  LoadInst *makeHoistedRestrictVar();
  // Clone the multiloop code to create optimized and non-optimized versions.
  void cloneNoOptBB(BasicBlock *BBIn, Function *OptF, Function *NoOptF,
                    BasicBlock *&BBHoist, BasicBlock *&BBOpt,
                    BasicBlock *&BBNoOpt);
  // Make the condition which tests for the optimized code.
  void makeOptTest(LoadInst *CacheInfo, StructType *LIRestrictType,
                   Instruction *MLX, Instruction *MLY, Instruction *W2,
                   Instruction *H2, BasicBlock *HoistBlock,
                   BasicBlock *OptBlock, BasicBlock *NoOptBlock);
  // Return Function with cold code extracted from 'OptBaseF'.
  Function *extractColdCode(Function *OptBaseF);
  // Build metadata for 'OptColdF' which was extracted and replaced by
  // the call 'OptColdCB'.
  bool buildColdCodeMetadata(CallBase *OptColdCB, Function *OptColdF);
  // Return the number of branches simplified, using the values of
  // cache_info fields that are specialized in the optimized version.
  unsigned simplifyCacheInfoBranches(LoadInst *LIRestrict);
  // Return the number of uses of nexus_info->region that were simplified.
  unsigned simplifyNexusInfoRegionUses(GetElementPtrInst *GEPI6F1);
  // Return the number of constant arguments of 'OptBaseF' that were 
  // propagated. 'OptBaseF' has a single call site 'OptBaseCB'.
  unsigned propagateOptBaseFArgConsts(CallBase *OptBaseCB, Function *OptBaseF);
  // Return the number of mixed expressions that were optimized. Mixed
  // expressions include arguments of 'OptBaseF' and fields from the structure
  // pointed to by 'LIRestrict'.
  unsigned simplifyMixedExpressions(Function *OptBaseF, LoadInst *LIRestrict);
  // Perform Partial Dead Store Elimination on argument 3 of 'WrapperF',
  // whose call site inside 'BigLoopF' is 'BigLoopCB'.
  bool doPDSEinWrapperFArg3(CallBase *BigLoopCB, Function *WrapperF);
};

//
// Return 'true' if 'F' is a wrapper function. 'F' must have no more than
// 3 basic blocks and only one call which is not an intrinsic.
//
bool PredicateOpt::isWrapper(Function &F) {
  if (F.size() > 3)
    return false;
  CallBase *SingleCB = nullptr;
  for (auto &I : instructions(F))
    if (auto CB = dyn_cast<CallBase>(&I)) {
      if (isa<IntrinsicInst>(CB))
        continue;
      if (SingleCB)
        return false;
      SingleCB = CB;
    }
  return SingleCB;
}

//
// Return the number of simple loops enclosing 'CB'. A simple loop is
// one which has a latch test with an integer comparison.
//
unsigned PredicateOpt::simpleLoopDepth(CallBase &CB) {

  auto IsSimpleLoop = [](Loop *L) -> bool {
    if (BasicBlock *BB = L->getLoopLatch())
      if (auto BI = dyn_cast<BranchInst>(BB->getTerminator()))
        if (BI->isConditional() && isa<ICmpInst>(BI->getCondition()))
          return true;
    return false;
  };

  unsigned Count = 0;
  LoopInfo *LI = ILIC.getLI(CB.getCaller());
  Loop *L = LI->getLoopFor(CB.getParent());
  if (!L)
    return 0;
  if (IsSimpleLoop(L))
    Count++;
  while (L->getParentLoop()) {
    L = L->getParentLoop();
    if (IsSimpleLoop(L))
      Count++;
  }
  return Count;
}

//
// Return the doubly nested loop enclosing 'CB', otherwise return 'nullptr'.
//
Loop *PredicateOpt::findMultiLoop(CallBase *CB) {
  if (!CB)
    return nullptr;
  LoopInfo *LI = ILIC.getLI(CB->getCaller());
  if (!LI)
    return nullptr;
  Loop *L = LI->getLoopFor(CB->getParent());
  return L ? L->getParentLoop() : nullptr;
}

//
// Show that the 'L' writes only through 'WrapperF' and a single local
// struct variable that will not alias with the actual arguments passed to
// 'WrapperF'.
//
// The single local struct variable has the form:
//   %8 = alloca %struct._ZTS18_MagickPixelPacket._MagickPixelPacket, align 8
// It is referenced by a series of GEPs:
//   %57 = getelementptr inbounds %S, ptr %8, i64 0, i32 5
//   %58 = getelementptr inbounds %S, ptr %8, i64 0, i32 6
//   %59 = getelementptr inbounds %S, ptr %8, i64 0, i32 7
//   %61 = getelementptr inbounds %S, ptr %8, i64 0, i32 8
// where %S = %struct._ZTS18_MagickPixelPacket._MagickPixelPacket.
//
// These GEPs are then used in the following sequence of loads and stores:
//
//   %173 = load float, ptr %57, align 8
//   %174 = fadd fast float %173, %151
//   store float %174, ptr %57, align 8
//   %175 = load float, ptr %58, align 4
//   %176 = fadd fast float %175, %156
//   store float %176, ptr %58, align 4
//   %177 = load float, ptr %59, align 8
//   %178 = fadd fast float %177, %162
//   store float %178, ptr %59, align 8
//   %179 = load i16, ptr %60, align 2
//   %180 = uitofp i16 %179 to float
//   %181 = load float, ptr %61, align 4
//   %182 = fadd fast float %181, %180
//   store float %182, ptr %61, align 4
//
// The only other places %8 is used are lifetime start and end instructions:
//   call void @llvm.lifetime.start.p0(i64 56, ptr nonnull %8)
//   call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %8)
// and being passed to a function:
//   call void @GetMagickPixelPacket(ptr noundef %0, ptr noundef nonnull %8)
// where it is used to store values to the structure fields:
//   %3 = getelementptr inbounds %S, ptr %1, i64 0, i32 0
//   store i32 1, ptr %3, align 8
//   %4 = getelementptr inbounds %S, ptr %1, i64 0, i32 1
//   store i32 13, ptr %4, align 4
//   %5 = getelementptr inbounds %S, ptr %1, i64 0, i32 2
//   store i32 0, ptr %5, align 8
//   %6 = getelementptr inbounds %S, ptr %1, i64 0, i32 3
//   store double 0.000000e+00, ptr %6, align 8
//   %7 = getelementptr inbounds %S, ptr %1, i64 0, i32 4
//   store i64 16, ptr %7, align 8
//   %8 = getelementptr inbounds %S, ptr %1, i64 0, i32 5
//   store float 0.000000e+00, ptr %8, align 8
//   %9 = getelementptr inbounds %S, ptr %1, i64 0, i32 6
//   store float 0.000000e+00, ptr %9, align 4
//   %10 = getelementptr inbounds %S, ptr %1, i64 0, i32 7
//   store float 0.000000e+00, ptr %10, align 8
//   %11 = getelementptr inbounds %S, ptr %1, i64 0, i32 8
//   store float 0.000000e+00, ptr %11, align 4
//   %12 = getelementptr inbounds %S, ptr %1, i64 0, i32 9
//   store float 0.000000e+00, ptr %12, align 8
//   ...
//   store i32 %16, ptr %3, align 8
//   store i32 %18, ptr %4, align 4
//   store i32 %20, ptr %5, align 8
//   store double %24, ptr %6, align 8
//   store i64 %22, ptr %7, align 8
// The key point is that %7 should not be stored some place
// where it can be accessed from the variables we want to hoist.
//
// validateMultiLoop walks through the IR for 'L' and @GetMagickPixelPacket
// and returns 'true' if all of thse conditions are met.
//
bool PredicateOpt::validateMultiLoop(Loop *L, Function *WrapperF) {

  // Put in 'Writes' any instruction in 'BB' that does not call 'WrapperF'
  // and may write to memory. Exclude lifetime starts and ends.
  auto CollectWrites = [](BasicBlock *BB, Function *WrapperF,
                          SmallPtrSetImpl<Instruction *> &Writes) {
    for (auto &I : *BB) {
      if (auto CB = dyn_cast<CallBase>(&I)) {
        if (CB->getCalledFunction() == WrapperF)
          continue;
        if (CB->isLifetimeStartOrEnd())
          continue;
      }
      if (I.mayWriteToMemory())
        Writes.insert(&I);
    }
  };

  // Return 'true' if 'GEPI' is a simple three operand 'GEPI' with pointer
  // operand 'PO', argument 1 equal to 0, and argument 2 equal to a constant.
  auto IsSimpleGEPI = [](GetElementPtrInst *GEPI, Value *PO) -> bool {
    if (GEPI->getNumOperands() != 3)
      return false;
    if (GEPI->getPointerOperand() != PO)
      return false;
    auto CI1 = dyn_cast<ConstantInt>(GEPI->getOperand(1));
    if (!CI1 || !CI1->isZero())
      return false;
    if (!isa<ConstantInt>(GEPI->getOperand(2)))
      return false;
    return true;
  };

  // Return 'true' if 'CB' calls a function where the actual argument 'AI'
  // has values read from or written to its fields.
  auto JustLoadsOrStoresValues = [&IsSimpleGEPI](CallBase *CB,
                                                 AllocaInst *AI) -> bool {
    unsigned Limit = CB->getNumOperands();
    unsigned ArgNo = Limit;
    for (unsigned I = 0; I < Limit; ++I)
      if (CB->getArgOperand(I) == AI) {
        ArgNo = I;
        break;
      }
    if (ArgNo == Limit)
      return false;
    Function *Callee = CB->getCalledFunction();
    if (!Callee || (Callee->arg_size() != CB->arg_size()))
      return false;
    Argument *Arg = Callee->getArg(ArgNo);
    for (User *U : Arg->users()) {
      auto GEPI = dyn_cast<GetElementPtrInst>(U);
      if (!GEPI || !IsSimpleGEPI(GEPI, Arg))
        return false;
      for (User *UU : GEPI->users()) {
        if (auto LI = dyn_cast<LoadInst>(UU)) {
          if (LI->getPointerOperand() != GEPI)
            return false;
        } else if (auto SI = dyn_cast<StoreInst>(UU)) {
          if (SI->getPointerOperand() != GEPI)
            return false;
        } else {
          return false;
        }
      }
    }
    return true;
  };

  // Collect up all of the instructions in 'L' that might write to memory.
  // Exclude the call to 'WrapperF'.
  ArrayRef<BasicBlock *> BlockList;
  BlockList = L->getBlocks();
  SmallPtrSet<Instruction *, 5> Writes;
  for (auto BB : BlockList)
    CollectWrites(BB, WrapperF, Writes);
  // Check that all writes are stores to a single AllocaInst 'AI'.
  AllocaInst *AI = nullptr;
  for (auto I : Writes) {
    auto SI = dyn_cast<StoreInst>(I);
    if (!SI)
      return false;
    auto GEPI = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
    if (!GEPI)
      return false;
    auto LAI = dyn_cast<AllocaInst>(GEPI->getPointerOperand());
    if (AI && (LAI != AI))
      return false;
    AI = LAI;
  }
  if (!AI)
    return false;
  // Check that the users of 'AI' are just simple GEPs which feed loads
  // and stores, lifetime start and end calls, and an actual argument
  // to a function where the fields of 'AI' are loaded or stored.
  for (User *U : AI->users()) {
    if (auto GEPI = dyn_cast<GetElementPtrInst>(U)) {
      if (!IsSimpleGEPI(GEPI, AI))
        return false;
      for (User *UU : GEPI->users())
        if (!isa<LoadInst>(UU) && !isa<StoreInst>(UU))
          return false;
    } else if (auto CB = dyn_cast<CallBase>(U)) {
      if (!CB->isLifetimeStartOrEnd())
        if (!JustLoadsOrStoresValues(CB, AI))
          return false;
    } else {
      return false;
    }
  }
  LLVM_DEBUG({
    dbgs() << "MRC PredicateOpt: Loop Stores To:";
    AI->dump();
  });
  return true;
}

//
// Find a unique LoadInst with a 'predicate-opt-restrict' metadata in the
// entry block of 'BaseF'.
//
LoadInst *PredicateOpt::findUniqueRestrictLoadInst(Function *BaseF) {
  LoadInst *LIR = nullptr;
  for (auto &I : BaseF->getEntryBlock())
    if (auto LI = dyn_cast<LoadInst>(&I))
      if (LI->getMetadata("predicate-opt-restrict")) {
        if (!LIR)
          LIR = LI;
        else
          return nullptr;
      }
  return LIR;
}

//
// Similar to findHoistableFields, but descent depth is limited to
// 'Depth' to ensure termination and save compile time.
//
bool PredicateOpt::findHoistableFieldsX(Function *F, Value *V, StructType *STy,
                                        unsigned Depth,
                                        std::set<unsigned> &HoistYes,
                                        std::set<unsigned> &HoistNo) {

  // Return 'true if 'U' is a GEPI(V,0,0)'.
  auto IsGEPIV00 = [](Value *U, Value *V) -> bool {
    if (auto GEPI0 = dyn_cast<GetElementPtrInst>(U))
      if (GEPI0->getNumOperands() == 3)
        if (GEPI0->getPointerOperand() == V)
          if (auto CI1 = dyn_cast<ConstantInt>(GEPI0->getOperand(1)))
            if (auto CI2 = dyn_cast<ConstantInt>(GEPI0->getOperand(2)))
              if (CI1->isZero() && CI2->isZero())
                return true;
    return false;
  };

  // Return 'true' if 'GEPI' has the form 'GEPI(V,0,FldNo)' or the form
  // 'GEPI(V,Offset)' where 'Offset' is the beginning of a field 'FldNo' in
  // 'STy', and set '*FldNo'.
  auto IsValidGEPI = [](GetElementPtrInst *GEPI, Value *V, StructType *STy,
                        unsigned *FldNo) -> bool {
    if (GEPI->getPointerOperand() != V)
      return false;
    if (GEPI->getNumOperands() == 2) {
      auto CI1 = dyn_cast<ConstantInt>(GEPI->getOperand(1));
      if (!CI1)
        return false;
      unsigned CI1V = CI1->getZExtValue();
      Module *M = GEPI->getModule();
      const StructLayout *SL = M->getDataLayout().getStructLayout(STy);
      unsigned Index = SL->getElementContainingOffset(CI1V);
      if (SL->getElementOffset(Index) != CI1V)
        return false;
      *FldNo = Index;
      return true;
    } else if (GEPI->getNumOperands() == 3) {
      auto CI1 = dyn_cast<ConstantInt>(GEPI->getOperand(1));
      if (!CI1 || !CI1->isZero())
        return false;
      auto CI2 = dyn_cast<ConstantInt>(GEPI->getOperand(2));
      if (!CI2)
        return false;
      *FldNo = CI2->getZExtValue();
      return true;
    } else {
      return false;
    }
  };

  // If 'Arg' is a unique actual argument of 'CB', return its index.
  // If not, return CB->arg_size().
  auto ArgNoFromCallBase = [](CallBase *CB, Value *Arg) -> unsigned {
    unsigned Invalid = CB->arg_size();
    unsigned AArgNo = Invalid;
    for (unsigned I = 0; I < CB->arg_size(); ++I) {
      if (CB->getArgOperand(I) == Arg) {
        if (AArgNo < Invalid) {
          AArgNo = Invalid;
          break;
        }
        AArgNo = I;
      }
    }
    return AArgNo;
  };

  // If 'U' is user of 'V', Check if 'U' is LoadInst *, StoreInst *,
  // or CallBase *, and set 'HoistYes' and 'HoistNo' appropriately.
  // Note that 'HoistNo' is set conservatively for calls. If 'U' is
  // not any of the above types, set '*Done' to 'true'.
  auto CheckUser = [](Value *V, User *U, unsigned FieldNo,
                      std::set<unsigned> &HoistYes, std::set<unsigned> &HoistNo,
                      bool *Done) -> bool {
    *Done = true;
    if (auto LI0 = dyn_cast<LoadInst>(U)) {
      if (LI0->getPointerOperand() == V) {
        HoistYes.insert(FieldNo);
      } else {
        return false;
      }
    } else if (auto SI0 = dyn_cast<StoreInst>(U)) {
      if (SI0->getPointerOperand() == V) {
        HoistNo.insert(FieldNo);
      } else {
        return false;
      }
    } else if (auto CB = dyn_cast<CallBase>(U)) {
      HoistNo.insert(FieldNo);
    } else {
      *Done = false;
    }
    return true;
  };

  // Main code for findHoistableFieldsX().
  // Test if we have hit the limit for the call chain depth.
  if (Depth == 0)
    return false;
  // Check the hoistability of each GEPI user of V.
  for (User *U : V->users()) {
    if (auto GEPI = dyn_cast<GetElementPtrInst>(U)) {
      bool Done = false;
      unsigned FldNo = 0;
      if (!IsValidGEPI(GEPI, V, STy, &FldNo))
        return false;
      for (User *U0 : GEPI->users()) {
        if (!CheckUser(GEPI, U0, FldNo, HoistYes, HoistNo, &Done))
          return false;
        if (Done)
          continue;
        if (IsGEPIV00(U0, GEPI)) {
          for (User *U1 : U0->users())
            if (!CheckUser(U0, U1, FldNo, HoistYes, HoistNo, &Done) || !Done)
              return false;
        } else {
          return false;
        }
      }
    } else if (auto CB = dyn_cast<CallBase>(U)) {
      Function *Callee = CB->getCalledFunction();
      if (!Callee)
        return false;
      unsigned AArgNo = ArgNoFromCallBase(CB, V);
      if (AArgNo < Callee->arg_size()) {
        if (!findHoistableFieldsX(Callee, Callee->getArg(AArgNo), STy,
                                  Depth - 1, HoistYes, HoistNo))
          return false;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
  return true;
}

//
// Return 'true' if 'V' of type 'STy' used in 'F' has dereferenced values
// through GEPs the can be determined to be hoistable. In this case, set
// 'HoistYes' to the indicies of the fields whose values can be hoisted and
// set 'HoistNo' to the indicies of the fields whose values cannot be hoisted.
// Indicies which do not appear in 'HoistYes' or 'HoistNo' are not
// encountered in a depth-first search of the call chain starting with 'F'.
//
bool PredicateOpt::findHoistableFields(Function *F, Value *V, StructType *STy,
                                       std::set<unsigned> &HoistYes,
                                       std::set<unsigned> &HoistNo) {
  // Find which fields can and cannot be hoisted.
  if (!findHoistableFieldsX(F, V, STy, PredicateOptMaxDepth, HoistYes, HoistNo))
    return false;
  // Exclude non-hoistable fields from the list of hoistable fields.
  for (auto &Int : HoistNo)
    if (HoistYes.count(Int))
      HoistYes.erase(Int);
  // Trace output of 'HoistYes' and 'HoistNo'.
  LLVM_DEBUG({
    dbgs() << "MRC PredicateOpt: HoistYes: {";
    bool First = true;
    for (auto &Int : HoistYes) {
      if (!First)
        dbgs() << ",";
      dbgs() << Int;
      First = false;
    }
    dbgs() << "}\n";
    dbgs() << "MRC PredicateOpt: HoistNo: {";
    First = true;
    for (auto &Int : HoistNo) {
      if (!First)
        dbgs() << ",";
      dbgs() << Int;
      First = false;
    }
    dbgs() << "}\n";
  });
  // Fields 0 (storage_class), 1 (colorspace), and 3 (type) must be hoistable.
  return HoistYes.count(0) && HoistYes.count(1) && HoistYes.count(3);
}

//
// Find the "spine" of the application on which we will perform predicate
// optimization, returning 'true' if it is found. In this case, mark the
// function with 'LocalBigLoopCB' as 'prefer-function-level-region'.
//
bool PredicateOpt::findSpine() {
  unsigned LocalSimpleLoopDepth = 0;
  CallBase *LocalWrapperCB = nullptr;
  CallBase *LocalBigLoopCB = nullptr;
  for (User *U0 : BaseF->users())
    if (auto CB0 = dyn_cast<CallBase>(U0)) {
      Function *F0 = CB0->getCaller();
      if (isWrapper(*F0)) {
        for (User *U1 : F0->users())
          if (auto CB1 = dyn_cast<CallBase>(U1)) {
            unsigned LD = simpleLoopDepth(*CB1);
            if (LD > LocalSimpleLoopDepth) {
              LocalSimpleLoopDepth = LD;
              LocalWrapperCB = CB0;
              LocalBigLoopCB = CB1;
            }
          }
      }
    }
  SimpleLoopDepth = LocalSimpleLoopDepth;
  WrapperCB = LocalWrapperCB;
  if (!WrapperCB || (WrapperCB->arg_size() != BaseF->arg_size()))
    return false;
  BigLoopCB = LocalBigLoopCB;
  assert(BigLoopCB && "Expecting spine");
  if (EnablePreferFunctionRegion)
    BigLoopCB->getCaller()->addFnAttr("prefer-function-level-region");
  getInlineReport()->initFunctionClosure(WrapperCB->getCalledFunction());
  getInlineReport()->initFunctionClosure(WrapperCB->getCaller());
  getInlineReport()->initFunctionClosure(BigLoopCB->getCaller());
  return BigLoopCB;
}

//
// First, prove there are no write operations before 'LIRestrict' in 'BaseF'
// and that 'LIRestrict' can be traced back through a GetElementPtrInst to
// an Argument (%0) of 'BaseF':
//
// define internal ptr @GetVirtualPixelsFromNexus(ptr noundef %0,
//     i32 noundef %1, i64 noundef %2, i64 noundef %3, i64 noundef %4,
//     i64 noundef %5, ptr nocapture noundef %6, ptr noundef %7)
//  %9 = alloca i16, align 2
//  %10 = alloca %struct._ZTS12_PixelPacket._PixelPacket, align 2
//  call void @llvm.lifetime.start.p0(i64 2, ptr nonnull %9) #68
//  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %10) #68
//  %11 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %0,
//      i64 0, i32 49
//  %12 = load ptr, ptr %11, align 8, !predicate-opt-data !1150
//
// Second, prove that the corresponding actual argument (%7) is a LoadInst
// that can be traced back through a GetElementPtrInst to and Argument (%0)
// of the wrapper function.
//
// define internal i32 @GetOneCacheViewVirtualPixel(ptr noalias nocapture
//     noundef readonly %0, i64 noundef %1, i64 noundef %2, ptr noalias
//     nocapture noundef writeonly %3, ptr noundef %4)
//   %6 = getelementptr inbounds %struct._ZTS10_CacheView._CacheView, ptr %0,
//     i64 0, i32 0, !intel-tbaa !849
//   %7 = load ptr, ptr %6, align 8, !tbaa !849
// ...
//   %26 = tail call ptr @GetVirtualPixelsFromNexus(ptr noundef %7, i32 noundef
//     %22, i64 noundef %1, i64 noundef %2, i64 noundef 1, i64 noundef 1,
//     ptr noundef %25, ptr noundef %4)
//
bool PredicateOpt::isRestrictVarHoistablePastWrapperF(Function *BaseF,
                                                      LoadInst *LIRestrict) {
  // Prove there are no writes in the entry block of 'BaseF' before
  // 'LIRestrict'.
  BasicBlock &BB = BaseF->getEntryBlock();
  bool FoundLIRestrict = false;
  for (auto &I : BB) {
    if (auto LDI = dyn_cast<LoadInst>(&I)) {
      if (LDI == LIRestrict) {
        FoundLIRestrict = true;
        break;
      }
    } else if (isa<StoreInst>(&I)) {
      return false;
    } else if (auto CBI = dyn_cast<CallBase>(&I)) {
      if (!CBI->isLifetimeStartOrEnd())
        return false;
    }
  }
  if (!FoundLIRestrict)
    return false;
  // Trace from 'LIRestrict' back to a formal argument 'ArgBaseF' of 'BaseF'.
  auto GEPI0 = dyn_cast<GetElementPtrInst>(LIRestrict->getPointerOperand());
  if (!GEPI0)
    return false;
  auto ArgBaseF = dyn_cast<Argument>(GEPI0->getPointerOperand());
  if (!ArgBaseF)
    return false;
  unsigned ArgNo = ArgBaseF->getArgNo();
  // Prove that the corresponding actual argument can be traced back to
  // a formal argument in the wrapper function.
  auto LIWrapper = dyn_cast<LoadInst>(WrapperCB->getArgOperand(ArgNo));
  if (!LIWrapper)
    return false;
  Function *WrapperF = WrapperCB->getCaller();
  if (LIWrapper->getParent() != &WrapperF->getEntryBlock())
    return false;
  Value *V = LIWrapper->getPointerOperand();
  auto GEPI1 = dyn_cast<GetElementPtrInst>(V);
  if (!GEPI1)
    return false;
  if (GEPI1->getPrevNonDebugInstruction())
    return false;
  auto Arg = dyn_cast<Argument>(GEPI1->getPointerOperand());
  if (!Arg)
    return false;
  return isa<Instruction>(BigLoopCB->getArgOperand(Arg->getArgNo()));
}

// Return 'true' if 'GEPI' is a simple three operand GetElementPtrInst
// with pointer operand 'PO', first operand 0, and second operand a
// constant 'Offset'. Set the value of 'Offset' in this case.
bool isSimpleGEPI(GetElementPtrInst *GEPI, Value *PO, unsigned &Offset) {
  if (GEPI->getNumOperands() != 3 || GEPI->getPointerOperand() != PO)
    return false;
  auto CI1 = dyn_cast<ConstantInt>(GEPI->getOperand(1));
  if (!CI1 || !CI1->isZero())
    return false;
  auto CI2 = dyn_cast<ConstantInt>(GEPI->getOperand(2));
  if (!CI2)
    return false;
  Offset = CI2->getZExtValue();
  return true;
}

//
// First, prove that the first field of '%6' is a structure of 4 ia64s
// which, when it is assigned, is assigned to the values of a set of
// arguments (%4, %5, %2, %3) of 'BaseF'.
//
// define internal ptr @GetVirtualPixelsFromNexus(ptr noundef %0, i32 noundef
//     %1, i64 noundef %2, i64 noundef %3, i64 noundef %4, i64 noundef %5,
//     ptr nocapture noundef %6, ptr noundef %7) {
//  %9 = alloca i16, align 2
//  %10 = alloca %struct._ZTS12_PixelPacket._PixelPacket, align 2
//  call void @llvm.lifetime.start.p0(i64 2, ptr nonnull %9) #68
//  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %10) #68
//  %11 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %0, i64 0,
//    i32 49, !intel-tbaa !867
//  %12 = load ptr, ptr %11, align 8, !tbaa !867, !predicate-opt-data !1150
//  %13 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %12,
//     i64 0, i32 3, !intel-tbaa !888
//  %14 = load i32, ptr %13, align 8, !tbaa !888, !alias.scope !1151, !noalias
//     !1154
//  %15 = icmp eq i32 %14, 0
//  br i1 %15, label %598, label %16
//
// 16:                                               ; preds = %8
//  %17 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %0, i64 0,
//    i32 38, !intel-tbaa !990
//  %18 = load ptr, ptr %17, align 8, !tbaa !990
//  %19 = icmp eq ptr %18, null
//  br i1 %19, label %20, label %24
//
// 20:                                               ; preds = %16
//  %21 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %0, i64 0,
//    i32 73, !intel-tbaa !991
//  %22 = load ptr, ptr %21, align 8, !tbaa !991
//  %23 = icmp ne ptr %22, null
//  br label %24
//
// 24:                                               ; preds = %20, %16
//  %25 = phi i1 [ true, %16 ], [ %23, %20 ]
//  %26 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %6,
//    i64 0, i32 1
//  store i64 %4, ptr %26, align 8, !tbaa.struct !631
//  %27 = getelementptr inbounds i8, ptr %26, i64 8
//  store i64 %5, ptr %27, align 8, !tbaa.struct !1161
//  %28 = getelementptr inbounds i8, ptr %26, i64 16
//  store i64 %2, ptr %28, align 8, !tbaa.struct !633
//  %29 = getelementptr inbounds i8, ptr %26, i64 24
//  store i64 %3, ptr %29, align 8, !tbaa.struct !1162
//  %30 = load i32, ptr %13, align 8, !tbaa !888
//  %31 = icmp eq i32 %30, 1
//  br i1 %31, label %35, label %32
// ...
// 598: ; preds = %593, %207, %204, %193, %191, %156, %114, %92, %8
//  %599 = phi ptr [ %166, %593 ], [ null, %8 ], [ null, %156 ],
//  [ %166, %207 ], [ %166, %191 ], [ null, %193 ], [ null, %204 ],
//  [ null, %92 ], [ null, %114 ]
//  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %10) #68
//  call void @llvm.lifetime.end.p0(i64 2, ptr nonnull %9) #68
//  ret ptr %599
//
// Second, prove each actual argument corresponding to the formal arguments
// above (%1, %2, 1, 1) is either a formal argument of the wrapper function or
// an integer constant.
//
// define internal i32 @GetOneCacheViewVirtualPixel(ptr noalias nocapture
//     noundef readonly %0, i64 noundef %1, i64 noundef %2, ptr noalias
//     nocapture noundef writeonly %3, ptr noundef %4) {
//   %6 = getelementptr inbounds %struct._ZTS10_CacheView._CacheView, ptr %0,
//     i64 0, i32 0, !intel-tbaa !849
//   %7 = load ptr, ptr %6, align 8, !tbaa !849
//  ...
//   %26 = tail call ptr @GetVirtualPixelsFromNexus(ptr noundef %7,
//     i32 noundef %22, i64 noundef %1, i64 noundef %2, i64 noundef 1,
//     i64 noundef 1, ptr noundef %25, ptr noundef %4) #72
//
bool PredicateOpt::isBaseFArg6Field1Hoistable(Function *BaseF,
                                              GetElementPtrInst *&GEPI6F1) {

  // Put the basic blocks which are predecessors of 'BBIn', direct or
  // indirect into 'BBOut'.  In the above IR, if block 24 is 'BBIn',
  // on exit from 'FindPredBBs', 'BBOut' should contain basic blocks
  // 20, 16, and 8.
  auto FindPredBBs = [](BasicBlock *BBIn,
                        SmallPtrSetImpl<BasicBlock *> &BBOut) {
    SetVector<BasicBlock *> BBWorklist;
    BBWorklist.insert(BBIn);
    while (!BBWorklist.empty()) {
      BasicBlock *BB = BBWorklist.pop_back_val();
      if (BBOut.insert(BB).second)
        for (BasicBlock *PBB : predecessors(BB))
          BBWorklist.insert(PBB);
    }
  };

  // Return 'true' if 'Ty' is a struct type of four i64s.
  auto Is4i64Struct = [](Type *Ty) -> bool {
    auto STy = dyn_cast<StructType>(Ty);
    if (!STy || STy->getNumElements() != 4)
      return false;
    for (unsigned I = 0; I < 4; ++I) {
      auto ITy = dyn_cast<IntegerType>(STy->getElementType(I));
      if (!ITy || ITy->getBitWidth() != 64)
        return false;
    }
    return true;
  };

  // Return 'true' if 'BB' may have an instruction that writes memory.
  auto MayHaveWrite = [](BasicBlock *BB) -> bool {
    for (auto &I : *BB) {
      if (isa<StoreInst>(&I))
        return true;
      if (auto CB = dyn_cast<CallBase>(&I))
        if (!CB->isLifetimeStartOrEnd())
          return true;
    }
    return false;
  };

  // Starting with 'GEPI6F1', the GEPI that selects the 'region' field:
  //  %26 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo,
  //    ptr %6, i64 0, i32 1
  // Find the basic blocks in which the fields within the 'region' field
  // are loaded. For example, if the field values are stored as:
  //   store i64 %4, ptr %26, align 8, !tbaa.struct !631
  //   %27 = getelementptr inbounds i8, ptr %26, i64 8
  //   store i64 %5, ptr %27, align 8, !tbaa.struct !1161
  //   %28 = getelementptr inbounds i8, ptr %26, i64 16
  //   store i64 %2, ptr %28, align 8, !tbaa.struct !633
  //   %29 = getelementptr inbounds i8, ptr %26, i64 24
  //   store i64 %3, ptr %29, align 8, !tbaa.struct !1162
  // we want the list of basic blocks that contain a LoadInst which is
  // accessed using %26, %27, %28, or %29 or some GEPI based on them.
  auto FindBBTerms = [](GetElementPtrInst *GEPI6F1,
                        SetVector<BasicBlock *> &BBTerms) -> bool {
    BasicBlock *BBP = GEPI6F1->getParent();
    StoreInst *SIUniqueBase = nullptr;
    for (User *U : GEPI6F1->users()) {
      if (auto GEPIT = dyn_cast<GetElementPtrInst>(U)) {
        StoreInst *SIUnique = nullptr;
        for (auto *UU : GEPIT->users()) {
          if (auto LI = dyn_cast<LoadInst>(UU)) {
            BBTerms.insert(LI->getParent());
          } else if (auto SI = dyn_cast<StoreInst>(UU)) {
            if (SIUnique)
              return false;
            if (SI->getParent() != BBP)
              return false;
            SIUnique = SI;
          } else {
            return false;
          }
        }
      } else if (auto SI = dyn_cast<StoreInst>(U)) {
        if (SIUniqueBase)
          return false;
        if (SI->getParent() != BBP)
          return false;
        SIUniqueBase = SI;
      } else {
        return false;
      }
    }
    return true;
  };

  // Let 'BBT' be the basic block containing
  //  %26 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo,
  //    ptr %6, i64 0, i32 1
  // and let 'BBCheck' be the list of basic blocks produced by calling
  // FindBBTerms(). Augment 'BBCheck' by all basic blocks traversed on
  // a path from 'BBT' to some basic block originally in 'BBCheck'.
  // We will need to prove that none of these basic blocks except 'BBT'
  // can assign values to the 'region' field.
  auto FindBBPredCheck = [](BasicBlock *BBT, SetVector<BasicBlock *> &BBCheck) {
    SetVector<BasicBlock *> BBWorklist = BBCheck;
    BBCheck.clear();
    while (!BBWorklist.empty()) {
      BasicBlock *BB = BBWorklist.pop_back_val();
      if (BBCheck.insert(BB))
        if (BB != BBT)
          for (BasicBlock *PBB : predecessors(BB))
            BBWorklist.insert(PBB);
    }
  };

  // Return the unique PHINode feeding the unique ReturnInst in 'F'.
  // For example, if 'F' ends in:
  //   %29 = phi ptr [ null, %8 ], [ null, %12 ], [ null, %10 ], [ %26, %22 ],
  //     [ null, %18 ]
  //   ret ptr %29
  // and this is the only ReturnInst in 'F', return %29.
  auto FindUniqueReturnPHINode = [](Function *F) -> PHINode * {
    PHINode *PHINUnique = nullptr;
    for (BasicBlock &BB : *F) {
      auto RI = dyn_cast_or_null<ReturnInst>(BB.getTerminator());
      if (!RI || !RI->getReturnValue())
        continue;
      auto PHIN = dyn_cast<PHINode>(RI->getReturnValue());
      if (!PHIN)
        continue;
      if (PHINUnique)
        return nullptr;
      PHINUnique = PHIN;
    }
    return PHINUnique;
  };

  // Return 'true' if 'CB' calls a function that allocates memory with
  // malloc and only writes to either errno_location() or the memory
  // that is allocated. For example, consider a function that returns a
  // pointer to 8-byte aligned memory allocated with malloc. This can
  // be implemented by calling malloc with the number of bytes requested
  // + 71 bytes. 64 bytes would be used to hold a pointer to the address
  // returned by malloc, which could then be passed to free() when the
  // memory is deallocated. We would write to errno_location() only if
  // an error occurred.
  auto WritesOnlyToMallocedMemory = [&](CallBase *CB) -> bool {
    Function *Callee = CB->getCalledFunction();
    if (!Callee)
      return false;
    PHINode *PHIN = FindUniqueReturnPHINode(Callee);
    if (!PHIN)
      return false;
    // Look up the PHINode to find 'VUnique', the unique non-null incoming
    // value to the PHINode. For example:
    //   %29 = phi ptr [ null, %8 ], [ null, %12 ], [ null, %10 ], [ %26, %22 ],
    //     [ null, %18 ]
    // 'VUnique' would be '%26'.
    Value *VUnique = nullptr;
    StoreInst *SIMalloc = nullptr;
    CallBase *CBMul = nullptr;
    CallBase *CBMalloc = nullptr;
    for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; ++I) {
      Value *V = PHIN->getIncomingValue(I);
      auto CV = dyn_cast<Constant>(V);
      if (CV && CV->isNullValue())
        continue;
      if (VUnique)
        return false;
      VUnique = V;
      Value *VM = nullptr;
      ConstantInt *CI71A = nullptr;
      ConstantInt *CIM64 = nullptr;
      // Look for a sequence like:
      //   %23 = ptrtoint ptr %20 to i64
      //   %24 = add i64 %23, 71
      //   %25 = and i64 %24, -64
      //   %26 = inttoptr i64 %25 to ptr
      // which implements finding the actual value to be returned by the
      // aligned malloc.
      if (!match(V, m_IntToPtr(m_And(
                        m_Add(m_PtrToInt(m_Value(VM)), m_ConstantInt(CI71A)),
                        m_ConstantInt(CIM64)))))
        return false;
      if (CI71A->getZExtValue() != 71)
        return false;
      if (CIM64->getSExtValue() != -64)
        return false;
      // Make sure the above sequence starts with a call to malloc:
      //   %20 = tail call noalias ptr @malloc(i64 noundef %19)
      auto CB0 = dyn_cast<CallBase>(VM);
      if (!CB0)
        return false;
      Function *CBF = CB0->getCalledFunction();
      if (!CBF)
        return false;
      LibFunc TheLibFunc;
      const TargetLibraryInfo &TLI = (*GetTLI)(*Callee);
      if (!TLI.getLibFunc(CBF->getName(), TheLibFunc) || !TLI.has(TheLibFunc) ||
          TheLibFunc != LibFunc_malloc)
        return false;
      CBMalloc = CB0;
      // Make sure the amount allocated with malloc is %0 * %1 + 71, where
      // %0 and %1 are the arguments to the aligned malloc function.
      //   %3 = tail call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %0,
      //     i64 %1)
      //   %4 = extractvalue { i64, i1 } %3, 0
      //   %19 = add nuw i64 %4, 71
      Value *VMul = nullptr;
      ConstantInt *CI71B = nullptr;
      if (!match(CB0->getOperand(0),
                 m_Add(m_ExtractValue<0>(m_Value(VMul)), m_ConstantInt(CI71B))))
        return false;
      if (CI71B->getZExtValue() != 71)
        return false;
      auto II = dyn_cast<IntrinsicInst>(VMul);
      if (!II || II->getIntrinsicID() != Intrinsic::umul_with_overflow)
        return false;
      if (!isa<Argument>(II->getArgOperand(0)))
        return false;
      if (!isa<Argument>(II->getArgOperand(1)))
        return false;
      // Check that the address of the memory allocated by malloc is
      // stored in the extra 71 bytes that were allocated, just above the
      // returned aligned address:
      //   %27 = getelementptr inbounds ptr, ptr %26, i64 -1
      //   store ptr %20, ptr %27, align 8, !tbaa !1029
      CBMul = II;
      GetElementPtrInst *GEPIUnique = nullptr;
      for (User *U : V->users()) {
        if (U == PHIN)
          continue;
        auto GEPI = dyn_cast<GetElementPtrInst>(U);
        if (!GEPI)
          return false;
        if (GEPIUnique)
          return false;
        GEPIUnique = GEPI;
      }
      if (!GEPIUnique || GEPIUnique->getPointerOperand() != V)
        return false;
      if (GEPIUnique->getNumOperands() != 2)
        return false;
      auto CI = dyn_cast<ConstantInt>(GEPIUnique->getOperand(1));
      if (!CI || !CI->isMinusOne())
        return false;
      if (!GEPIUnique->hasOneUser())
        return false;
      auto SI = dyn_cast<StoreInst>(GEPIUnique->user_back());
      if (!SI || SI->getPointerOperand() != GEPIUnique ||
          SI->getValueOperand() != VM)
        return false;
      SIMalloc = SI;
    }
    // Check that the only other CallBase and StoreInst in the function
    // are the following, which will be called if an error occurs:
    //  %9 = tail call ptr @__errno_location()
    //  store i32 12, ptr %9, align 4
    CallBase *CBErrnoLocation = nullptr;
    for (auto &I : instructions(*Callee)) {
      if (auto CB1 = dyn_cast<CallBase>(&I)) {
        if (CB1 != CBMul && CB1 != CBMalloc) {
          Function *CBF = CB1->getCalledFunction();
          if (!CBF)
            return false;
          LibFunc TheLibFunc;
          const TargetLibraryInfo &TLI = (*GetTLI)(*Callee);
          if (!TLI.getLibFunc(CBF->getName(), TheLibFunc) ||
              !TLI.has(TheLibFunc) ||
              (TheLibFunc != LibFunc_errno_location &&
               TheLibFunc != LibFunc_under_errno))
            return false;
          CBErrnoLocation = CB1;
        }
      } else if (auto SI = dyn_cast<StoreInst>(&I)) {
        if (SI != SIMalloc && SI->getPointerOperand() != CBErrnoLocation)
          return false;
      }
    }
    return true;
  };

  // Return 'true' if 'BB0' matches the following basic block:
  //   %2 = icmp eq ptr 'F->getArg(0)', null
  //   br i1 %2, label 'BB2', label 'BB1'
  // If we return 'true', set 'BB2' and 'BB1'.
  auto MatchRAM0 = [](BasicBlock *BB0, BasicBlock *&BB1,
                      BasicBlock *&BB2) -> bool {
    Function *F = BB0->getParent();
    auto BI = dyn_cast_or_null<BranchInst>(BB0->getTerminator());
    if (!BI || BI->isUnconditional())
      return false;
    auto IC = dyn_cast_or_null<ICmpInst>(BI->getPrevNonDebugInstruction());
    if (!IC || BI->getCondition() != IC || IC->getPrevNonDebugInstruction())
      return false;
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!match(IC, m_ICmp(Pred, m_Specific(F->getArg(0)), m_Zero())))
      return false;
    if (Pred != ICmpInst::ICMP_EQ)
      return false;
    BB1 = BI->getSuccessor(0);
    BB2 = BI->getSuccessor(1);
    return true;
  };

  // Return 'true" if 'BB1' matches the following basic block:
  //   %4 = getelementptr inbounds ptr, ptr %0, i64 -1
  //   %5 = load ptr, ptr %4, align 8
  //   tail call void @free(ptr noundef %5) #71
  //   br label 'BB2'
  auto MatchRAM1 = [&](BasicBlock *BB1, BasicBlock *BB2) -> bool {
    Function *F = BB1->getParent();
    auto BI = dyn_cast_or_null<BranchInst>(BB1->getTerminator());
    if (!BI || BI->isConditional() || BI->getSuccessor(0) != BB2)
      return false;
    auto CB = dyn_cast_or_null<CallBase>(BI->getPrevNonDebugInstruction());
    if (!CB)
      return false;
    Function *CBF = CB->getCalledFunction();
    if (!CBF)
      return false;
    LibFunc TheLibFunc;
    const TargetLibraryInfo &TLI = (*GetTLI)(*F);
    if (!TLI.getLibFunc(CBF->getName(), TheLibFunc) || !TLI.has(TheLibFunc) ||
        TheLibFunc != LibFunc_free)
      return false;
    auto LI = dyn_cast<LoadInst>(CB->getPrevNonDebugInstruction());
    if (!LI || CB->getArgOperand(0) != LI)
      return false;
    auto GEPI = dyn_cast<GetElementPtrInst>(LI->getPrevNonDebugInstruction());
    if (!GEPI || LI->getPointerOperand() != GEPI)
      return false;
    if (GEPI->getNumOperands() != 2)
      return false;
    if (GEPI->getOperand(0) != F->getArg(0))
      return false;
    auto CI = dyn_cast<ConstantInt>(GEPI->getOperand(1));
    if (!CI || !CI->isMinusOne())
      return false;
    if (GEPI->getPrevNonDebugInstruction())
      return false;
    return true;
  };

  // Return 'true' if 'BB2' matches the following basic block:
  //   ret ptr null
  auto MatchRAM2 = [](BasicBlock *BB2) -> bool {
    auto RI = dyn_cast<ReturnInst>(BB2->getTerminator());
    if (!RI || RI->getPrevNonDebugInstruction())
      return false;
    auto CV = dyn_cast<Constant>(RI->getReturnValue());
    if (!CV || !CV->isNullValue())
      return false;
    return true;
  };

  // Return 'true' if 'CB' retrieves a pointer from a field other than the
  // 'region' field and frees it using an aligned free function. For example:
  //   %82 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo,
  //     ptr 'PO', i64 0, i32 3
  //   %83 = load ptr, ptr %82, align 8
  //   %105 = tail call ptr @RelinquishAlignedMemory(ptr noundef nonnull %83)
  // Here field 3 is NOT the 'region' field of the NexusInfo structure.
  // We use pattern matching to recognize the aligned free function.
  auto FreesSafeMallocedMemory = [&](Value *POB, CallBase *CB) -> bool {
    if (CB->arg_size() != 1)
      return false;
    auto LI = dyn_cast<LoadInst>(CB->getArgOperand(0));
    if (!LI)
      return false;
    auto GEPI = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
    unsigned Offset = 0;
    if (!GEPI || !isSimpleGEPI(GEPI, POB, Offset) || Offset == 1)
      return false;
    Function *Callee = CB->getCalledFunction();
    if (!Callee || Callee->size() != 3)
      return false;
    BasicBlock *BB1 = nullptr;
    BasicBlock *BB2 = nullptr;
    if (!MatchRAM0(&Callee->getEntryBlock(), BB2, BB1))
      return false;
    if (!MatchRAM1(BB1, BB2))
      return false;
    if (!MatchRAM2(BB2))
      return false;
    return true;
  };

  // Return 'true' if none of the basic blocks in 'BBCheck' write to the
  // 'region' field selected by 'GEPI6F1' except the basic block containing
  // 'GEPI6F1'.
  auto DoBBPredCheck = [&](GetElementPtrInst *GEPI6F1,
                           SetVector<BasicBlock *> &BBCheck) -> bool {
    Value *POB = GEPI6F1->getPointerOperand();
    BasicBlock *BBT = GEPI6F1->getParent();
    for (BasicBlock *BB : BBCheck) {
      if (BB == BBT)
        continue;
      for (Instruction &I : *BB) {
        if (auto SI = dyn_cast<StoreInst>(&I)) {
          Value *PO = SI->getPointerOperand();
          if (auto GEPI = dyn_cast<GetElementPtrInst>(PO)) {
            unsigned Offset = 0;
            if (!isSimpleGEPI(GEPI, POB, Offset) || Offset == 1)
              return false;
          } else {
            return false;
          }
        } else if (auto CB = dyn_cast<CallBase>(&I)) {
          if (!WritesOnlyToMallocedMemory(CB) &&
              !FreesSafeMallocedMemory(POB, CB))
            return false;
        }
      }
    }
    return true;
  };

  // Find 'GEPIF61' which points to the first first field in the structure
  // pointed to by argument 6 in the base function.
  if (BaseF->arg_size() <= 6)
    return false;
  Argument *Arg6 = BaseF->getArg(6);
  for (User *U : Arg6->users())
    if (auto GEPI = dyn_cast<GetElementPtrInst>(U)) {
      if (GEPI->getNumOperands() != 3)
        continue;
      if (GEPI->getPointerOperand() != Arg6)
        continue;
      auto CI1 = dyn_cast<ConstantInt>(GEPI->getOperand(1));
      if (!CI1 || !CI1->isZero())
        continue;
      auto CI2 = dyn_cast<ConstantInt>(GEPI->getOperand(2));
      if (!CI2 || !CI2->isOne())
        continue;
      if (GEPI6F1)
        return false;
      GEPI6F1 = GEPI;
    }
  // Check that field 1 is a structure of 4 i64s.
  if (!GEPI6F1 || !Is4i64Struct(GEPI6F1->getResultElementType()))
    return false;
  // Check that there are no writes in the basic blocks preceding the
  // block containing 'GEPI6F1', and that any basic blocks that escape
  // are blocks with no successors which also have no writes. In the above
  // IR, block 598 is an escape block.
  SmallPtrSet<BasicBlock *, 5> BBPred;
  SmallPtrSet<BasicBlock *, 5> BBEsc;
  BasicBlock *BBRoot = GEPI6F1->getParent();
  if (BBRoot->getFirstNonPHI() != GEPI6F1)
    return false;
  FindPredBBs(BBRoot, BBPred);
  for (BasicBlock *BB : BBPred) {
    if (BB == BBRoot)
      continue;
    if (MayHaveWrite(BB))
      return false;
    // Check that any basic block that escapes returns and has no writes.
    for (BasicBlock *BBS : successors(BB)) {
      if (BBPred.count(BBS))
        continue;
      if (!isa<ReturnInst>(BBS->getTerminator()))
        return false;
      if (MayHaveWrite(BBS))
        return false;
    }
  }
  // Check that the basic block containing 'GEPI6F1' starts with assignments
  // of various argumemts of 'BaseF' ('ArgsToCheck') to the field pointed to
  // by 'GEPI6F1'.
  if (!isa<PHINode>(GEPI6F1->getPrevNonDebugInstruction()))
    return false;
  SmallPtrSet<Argument *, 4> ArgsToCheck;
  auto SI0 = dyn_cast_or_null<StoreInst>(GEPI6F1->getNextNonDebugInstruction());
  if (!SI0 || SI0->getPointerOperand() != GEPI6F1)
    return false;
  auto Arg = dyn_cast<Argument>(SI0->getValueOperand());
  if (!Arg)
    return false;
  ArgsToCheck.insert(Arg);
  Instruction *II = SI0->getNextNonDebugInstruction();
  unsigned Offset = 8;
  for (unsigned I = 0; I < 3; ++I) {
    auto GEPI = dyn_cast_or_null<GetElementPtrInst>(II);
    if (!GEPI || GEPI->getNumOperands() != 2 ||
        GEPI->getPointerOperand() != GEPI6F1)
      return false;
    auto CI = dyn_cast<ConstantInt>(GEPI->getOperand(1));
    if (!CI || CI->getZExtValue() != Offset)
      return false;
    auto SI = dyn_cast_or_null<StoreInst>(GEPI->getNextNonDebugInstruction());
    if (!SI || SI->getPointerOperand() != GEPI)
      return false;
    auto Arg = dyn_cast<Argument>(SI->getValueOperand());
    if (!Arg)
      return false;
    ArgsToCheck.insert(Arg);
    II = SI->getNextNonDebugInstruction();
    Offset += 8;
  }
  // Ensure that there are no other stores or calls in this basic block.
  for (; II; II = II->getNextNonDebugInstruction())
    if (isa<StoreInst>(II) || isa<CallBase>(II))
      return false;
  // Check that each of the actual arguments in the call to 'BaseF' is
  // either an argument in the wrapper function or a constant integer.
  for (auto Arg : ArgsToCheck) {
    Value *V = WrapperCB->getArgOperand(Arg->getArgNo());
    if (!isa<Argument>(V) && !isa<ConstantInt>(V))
      return false;
  }
  // Check that from the basic block where the 'nexus_info->region' field
  // is assigned to all of the basic blocks where it is used, that none of
  // the fields within this 'nexus_info->region' field is written.
  SetVector<BasicBlock *> BBPredCheck;
  if (!FindBBTerms(GEPI6F1, BBPredCheck))
    return false;
  FindBBPredCheck(GEPI6F1->getParent(), BBPredCheck);
  if (!DoBBPredCheck(GEPI6F1, BBPredCheck))
    return false;
  return true;
}

//
// Looking for the a sequence like:
//    %i119 = call fast double @llvm.rint.f64(double %i112)
//    %i120 = fptosi double %i119 to i64
//    %i143 = add i64 %i134, %i120
//    %i144 = call i32 @GetOneCacheViewVirtualPixel(ptr noundef %i29,
//        i64 noundef %i143, ...)
// and if found, will return %i120.
//
Instruction *PredicateOpt::findRintResult(CallBase *CB, unsigned ArgNo) {
  auto BO = dyn_cast<BinaryOperator>(CB->getArgOperand(ArgNo));
  if (!BO || BO->getOpcode() != Instruction::Add)
    return nullptr;
  if (!isa<PHINode>(BO->getOperand(0)))
    return nullptr;
  auto FPTOSI = dyn_cast<Instruction>(BO->getOperand(1));
  if (!FPTOSI || FPTOSI->getOpcode() != Instruction::FPToSI)
    return nullptr;
  auto II = dyn_cast<IntrinsicInst>(FPTOSI->getOperand(0));
  if (!II || II->getIntrinsicID() != Intrinsic::rint)
    return nullptr;
  return FPTOSI;
}

//
// Looking for a sequence like:
// define internal ptr @MeanShiftImage(ptr noundef %arg, i64 noundef %arg1, ...
//  ...
//   %i48 = lshr i64 %arg1, 1
// and if found, will return %i48.
//
Instruction *PredicateOpt::findLShr1User(Argument *Arg) {
  for (User *U : Arg->users())
    if (auto Inst = dyn_cast<Instruction>(U))
      if (Inst->getOpcode() == Instruction::LShr)
        if (Inst->getOperand(0) == Arg)
          if (auto CI = dyn_cast<ConstantInt>(Inst->getOperand(1)))
            if (CI->isOne())
              return Inst;
  return nullptr;
}

//
// Return 'true' if the desired predicate opt should be attempted.
// NOTE: At this point, we are just checking if the hot path can be identified.
// More code will be added to check additional conditions.
//
bool PredicateOpt::shouldAttemptPredicateOpt() {
  if (!findSpine()) {
    LLVM_DEBUG(dbgs() << "MRC Predicate Opt: Could not find spine\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << "MRC Predicate Opt: Loops: " << SimpleLoopDepth << "\n");
  if (SimpleLoopDepth < PredicateOptMinLoops)
    return false;
  LIRestrict = findUniqueRestrictLoadInst(BaseF);
  LLVM_DEBUG(dbgs() << "MRC Predicate Opt: "
                    << "LIRestrict: " << (LIRestrict ? "T" : "F") << "\n");
  if (!LIRestrict)
    return false;
  Type *Ty = inferPtrElementType(*LIRestrict, /*FunctionOnly=*/true);
  LIRestrictType = dyn_cast_or_null<StructType>(Ty);
  LLVM_DEBUG(dbgs() << "MRC Predicate Opt: "
                    << "LIRestrictType: " << (LIRestrictType ? "T" : "F")
                    << "\n");
  if (!LIRestrictType)
    return false;
  bool IsResHoist = isRestrictVarHoistablePastWrapperF(BaseF, LIRestrict);
  LLVM_DEBUG(dbgs() << "MRC Predicate Opt: "
                    << "RestrictVarHoistablePastWrapperF: "
                    << (IsResHoist ? "T" : "F") << "\n");
  if (!IsResHoist)
    return false;
  bool IsBaseFArg6Field1Hoistable = isBaseFArg6Field1Hoistable(BaseF, GEPI6F1);
  LLVM_DEBUG(dbgs() << "MRC Predicate Opt: "
                    << "BaseFArg6Field1Hoistable: "
                    << (IsBaseFArg6Field1Hoistable ? "T" : "F") << "\n");
  if (!IsBaseFArg6Field1Hoistable)
    return false;
  std::set<unsigned> HoistYes, HoistNo;
  bool IsHoistable =
      findHoistableFields(BaseF, LIRestrict, LIRestrictType, HoistYes, HoistNo);
  LLVM_DEBUG(dbgs() << "MRC Predicate Opt: "
                    << "Hoistable: " << (IsHoistable ? "T" : "F") << "\n");
  if (!IsHoistable)
    return false;
  MultiLoop = findMultiLoop(BigLoopCB);
  LLVM_DEBUG(dbgs() << "MRC Predicate Opt: "
                    << "FindMultiLoop: " << (MultiLoop ? "T" : "F") << "\n");
  if (!MultiLoop)
    return false;
  bool IsValid = validateMultiLoop(MultiLoop, BigLoopCB->getCalledFunction());
  LLVM_DEBUG(dbgs() << "MRC Predicate Opt: "
                    << "ValidateMultiLoop: " << (IsValid ? "T" : "F") << "\n");
  if (!IsValid)
    return false;
  MLX = findRintResult(BigLoopCB, 1);
  LLVM_DEBUG(dbgs() << "MRC Predicate Opt: "
                    << "MLX: " << (MLX ? "T" : "F") << "\n");
  if (!MLX)
    return false;
  MLY = findRintResult(BigLoopCB, 2);
  LLVM_DEBUG(dbgs() << "MRC Predicate Opt: "
                    << "MLY: " << (MLY ? "T" : "F") << "\n");
  if (!MLY)
    return false;
  Function *BigLoopF = BigLoopCB->getCaller();
  W2 = findLShr1User(BigLoopF->getArg(1));
  LLVM_DEBUG(dbgs() << "MRC Predicate Opt: "
                    << "W2: " << (W2 ? "T" : "F") << "\n");
  if (!W2)
    return false;
  H2 = findLShr1User(BigLoopF->getArg(2));
  LLVM_DEBUG(dbgs() << "MRC Predicate Opt: "
                    << "H2: " << (H2 ? "T" : "F") << "\n");
  if (!H2)
    return false;
  return true;
}

//
// If possible, return Function with cold code extracted from 'OptBaseF'.
// Otherwise, return 'nullptr'.
//
Function *PredicateOpt::extractColdCode(Function *OptBaseF) {

  auto FindSplitBasicBlock = [](Function *OptBaseF) -> BasicBlock * {
    BasicBlock *SplitBB = nullptr;
    for (auto &BB : *OptBaseF)
      if (isa<SwitchInst>(BB.getTerminator()))
        if (std::distance(pred_begin(&BB), pred_end(&BB)) >= 3) {
          if (SplitBB)
            return nullptr;
          SplitBB = &BB;
        }
    return SplitBB;
  };

  BasicBlock *SplitBB = FindSplitBasicBlock(OptBaseF);
  if (!SplitBB)
    return nullptr;
  DominatorTree &DT = (*GetDT)(*OptBaseF);
  SmallVector<BasicBlock *, 16> Descendants;
  DT.getDescendants(SplitBB, Descendants);
  CodeExtractor CE(Descendants);
  CodeExtractorAnalysisCache CEAC(*OptBaseF);
  SetVector<Value *> Inputs, Outputs, Sinks;
  CE.findInputsOutputs(Inputs, Outputs, Sinks);
  Function *ColdF = CE.extractCodeRegion(CEAC);
  ColdF->addFnAttr(Attribute::NoInline);
  return ColdF;
}

//
// Return a pointer to a newly created hoisted restrict variable that
// can be used in the condition testing for the optimized code.
//
// The returned value will have the form of %3 below:
//   %i29 = tail call ptr @AcquireVirtualCacheView(...)
//   %0 = getelementptr %struct._ZTS10_CacheView._CacheView, ptr %i29, i64 0,
//     i32 0
//   %1 = load ptr, ptr %0, align 8
//   %2 = getelementptr %struct._ZTS6_Image._Image, ptr %1, i64 0, i32 49
//   %3 = load ptr, ptr %2, align 8
//
LoadInst *PredicateOpt::makeHoistedRestrictVar() {

  // Return a pointer to a GetElementPtrInst which is a copy of
  // 'GEPIIn' by with pointer operand 'PO'. Insert it before
  // instruction 'II'.
  auto MakeGEPIFromGEPI = [](GetElementPtrInst *GEPIIn, Value *PO,
                             Instruction *II) -> GetElementPtrInst * {
    SmallVector<Value *, 2> Indices;
    BasicBlock *BB = II->getParent();
    auto Int32Ty = Type::getInt32Ty(BB->getContext());
    auto Int64Ty = Type::getInt64Ty(BB->getContext());
    auto CIIn1 = cast<ConstantInt>(GEPIIn->getOperand(1));
    unsigned GO1 = CIIn1->getZExtValue();
    auto CI1 = ConstantInt::get(Int64Ty, GO1, true);
    Indices.push_back(CI1);
    auto CIIn2 = cast<ConstantInt>(GEPIIn->getOperand(2));
    unsigned GO2 = CIIn2->getZExtValue();
    auto CI2 = ConstantInt::get(Int32Ty, GO2, true);
    Indices.push_back(CI2);
    Type *GEPITy = GEPIIn->getSourceElementType();
    return GetElementPtrInst::Create(GEPITy, PO, Indices, "", II);
  };

  auto GEPIInner = cast<GetElementPtrInst>(LIRestrict->getPointerOperand());
  auto ArgInner = cast<Argument>(GEPIInner->getPointerOperand());
  unsigned ArgNoInner = ArgInner->getArgNo();
  auto LIOuter = cast<LoadInst>(WrapperCB->getArgOperand(ArgNoInner));
  auto GEPIOuter = cast<GetElementPtrInst>(LIOuter->getPointerOperand());
  auto ArgOuter = cast<Argument>(GEPIOuter->getPointerOperand());
  unsigned ArgNoOuter = ArgOuter->getArgNo();
  auto VInst = cast<Instruction>(BigLoopCB->getArgOperand(ArgNoOuter));
  auto II = VInst->getNextNonDebugInstruction();
  auto NewGEPIOuter = MakeGEPIFromGEPI(GEPIOuter, VInst, II);
  auto NewLIOuter = new LoadInst(LIOuter->getType(), NewGEPIOuter, "", II);
  auto NewGEPIInner = MakeGEPIFromGEPI(GEPIInner, NewLIOuter, II);
  auto NewLIInner = new LoadInst(LIRestrict->getType(), NewGEPIInner, "", II);
  return NewLIInner;
}

//
// With 'BBIn' being the extracted basic block containing the call to the
// 'NoOptF', create a copy of it that calls 'OptF'. Set 'BBNoOpt' to the
// basic block that calls 'NoOptF', 'BBOpt' to the basic block that calls
// 'OptF' and 'BBHoist' to the basic block which will contain the test of
// whether we should execute 'BBOpt' or 'BBNoOpt'.
//
void PredicateOpt::cloneNoOptBB(BasicBlock *BBIn, Function *OptF,
                                Function *NoOptF, BasicBlock *&BBHoist,
                                BasicBlock *&BBOpt, BasicBlock *&BBNoOpt) {
  Function *F = BBIn->getParent();
  Module *M = F->getParent();
  auto BIIn = cast<BranchInst>(BBIn->getTerminator());
  assert(BIIn->getNumSuccessors() == 1 && "Expecting unconditional");
  // Create the basic blocks needed for control flow.
  BasicBlock *BBOut = BIIn->getSuccessor(0);
  BBNoOpt = BBIn;
  BBHoist = BBIn->splitBasicBlockBefore(&BBIn->front());
  BBOpt = BasicBlock::Create(M->getContext(), "optpath", F);
  auto BIOpt = BranchInst::Create(BBOut, BBOpt);
  // Copy instructions from 'BBNoOpt' to 'BBOpt'.
  ValueToValueMapTy VMap;
  for (auto &I : *BBNoOpt) {
    if (&I != BIIn) {
      Instruction *II = I.clone();
      VMap[&I] = II;
      II->insertBefore(BIOpt);
      if (auto CB = dyn_cast<CallBase>(&I)) {
        auto CBNew = cast<CallBase>(II);
        if (CB->getCalledFunction() == NoOptF) {
          setCalledFunction(CBNew, OptF);
          getInlineReport()->addMultiversionedCallSite(CBNew);
          getMDInlineReport()->addMultiversionedCallSite(CBNew);
        }
      }
    }
  }
  // Patch up PHINodes coming into 'BBOut'.
  auto ITerm = BBOut->getFirstNonPHIOrDbgOrLifetime();
  for (auto &II : *BBOut) {
     if (auto PHIN = dyn_cast<PHINode>(&II)) {
       for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; ++I)
         if (PHIN->getIncomingBlock(I) == BBIn) {
           PHIN->addIncoming(VMap[PHIN->getIncomingValue(I)], BBOpt);
           break;
         }
     } else if (&II == ITerm) {
       break;
     }
  } 
}

//
// Make the condition which tests for the optimized code and place it at
// the end of 'HoistBB'. Terminate 'HoistBB' with a conditional BranchInst
// whose true branch is to 'OptBlock' and whose false branch is to 'NoOptBlock'.
//
//   cache_info->storage_class == PseudoClass
//
//   %4 = getelementptr %struct._ZTS6_Image._Image, ptr %3, i64 0, i32 0
//   %5 = load i32, ptr %4, align 4
//   %6 = icmp eq i32 %5, 2
//
//   cache_info->colorspace == CMYKColorspace
//
//   %7 = getelementptr %struct._ZTS6_Image._Image, ptr %3, i64 0, i32 1
//   %8 = load i32, ptr %7, align 4
//   %9 = icmp eq i32 %8, 12
//   %10 = or i1 %6, %9
//   %11 = xor i1 %10, true
//
//   cache_info->type == MemoryCache || cache_info->type == MapCache
//
//   %12 = getelementptr %struct._ZTS6_Image._Image, ptr %3, i64 0, i32 3
//   %13 = load i32, ptr %12, align 4
//   %14 = icmp eq i32 %13, 1
//   %15 = icmp eq i32 %13, 2
//   %16 = or i1 %14, %15
//   %17 = and i1 %11, %16
//   MLX >= W2 && MLY >= H2
//   %18 = icmp sge i64 %i120, %i48
//   %19 = and i1 %17, %18
//   %20 = icmp sge i64 %i122, %i49
//   %21 = and i1 %19, %20
//
//   (MLX + W2 < cache_info->columns) && (MLY + H2 < cache_info->rows)
//
//   %22 = add i64 %i120, %i48
//   %23 = add i64 %i122, %i49
//   %24 = getelementptr %struct._ZTS10_CacheInfo._CacheInfo, ptr %3,
//       i64 0, i32 6
//   %25 = load i64, ptr %24, align 8
//   %26 = getelementptr %struct._ZTS10_CacheInfo._CacheInfo, ptr %3,
//       i64 0, i32 7
//   %27 = load i64, ptr %26, align 8
//   %28 = icmp slt i64 %22, %25
//   %29 = and i1 %21, %28
//   %30 = icmp slt i64 %23, %27
//   %31 = and i1 %29, %30
//   MLY >= H2 && MLX >= W2
//   %32 = icmp sge i64 %i122, %i49
//   %33 = and i1 %31, %32
//   %34 = icmp sge i64 %i120, %i48
//   %35 = and i1 %33, %34
//
//   (MLY <= cache_info->rows - 1 - H2) && (MLX <= cache_info->columns-1-W2)
//
//   %36 = sub i64 %27, 1
//   %37 = sub i64 %36, %i49
//   %38 = icmp sle i64 %i122, %37
//   %39 = and i1 %35, %38
//   %40 = sub i64 %25, 1
//   %41 = sub i64 %40, %i48
//   %42 = icmp sle i64 %i120, %41
//   %43 = and i1 %39, %42
//
// Where %3 is 'CacheInfo' returned by makeHoistedRestrictVar(), %4 is
// %struct._ZTS10_CacheInfo._CacheInfo, %i120 is 'MLX', %i122 is 'MLY',
// %i48 is 'W2' and %i49 is 'H2'.
//
void PredicateOpt::makeOptTest(LoadInst *CacheInfo, StructType *LIRestrictType,
                               Instruction *MLX, Instruction *MLY,
                               Instruction *W2, Instruction *H2,
                               BasicBlock *HoistBlock, BasicBlock *OptBlock,
                               BasicBlock *NoOptBlock) {

  // Make a GetElementPtr and Load sequence where 'PO' is the pointer
  // operand of the GetElementPtrInst, 'Ty' is the source element type of
  // the GetElementPtrInst, 'FieldNo' is the field of the structure
  // accessed by the GetElementPtrInst. Insert the created instructions
  // before 'II'.
  auto MakeGEPILoad = [](Instruction *II, Value *PO, StructType *Ty,
                         unsigned FieldNo) -> LoadInst * {
    SmallVector<Value *, 2> Indices;
    BasicBlock *BB = II->getParent();
    auto Int32Ty = Type::getInt32Ty(BB->getContext());
    auto Int64Ty = Type::getInt64Ty(BB->getContext());
    auto CI1 = ConstantInt::get(Int64Ty, 0, true);
    Indices.push_back(CI1);
    auto CI2 = ConstantInt::get(Int32Ty, FieldNo, true);
    Indices.push_back(CI2);
    auto GEPI = GetElementPtrInst::Create(Ty, PO, Indices, "", II);
    Type *TyFieldNo = Ty->getTypeAtIndex((unsigned)FieldNo);
    return new LoadInst(TyFieldNo, GEPI, "", II);
  };

  // In addition to doing what MakeGEPILoad() does above, add an ICmpInst
  // that compares the result to 'V' and return a pointer to that ICmpInst.
  auto MakeGEPILoadICmp = [&](Instruction *II, Value *PO, StructType *Ty,
                              unsigned FieldNo, unsigned V) -> CmpInst * {
    auto LI = MakeGEPILoad(II, PO, Ty, FieldNo);
    auto CI = ConstantInt::get(LI->getType(), V);
    llvm::CmpInst::Predicate ICMPEQ = ICmpInst::ICMP_EQ;
    auto IC = CmpInst::Create(Instruction::ICmp, ICMPEQ, LI, CI, "", II);
    return IC;
  };

  auto II = HoistBlock->getTerminator();
  CmpInst *IC0 = MakeGEPILoadICmp(II, CacheInfo, LIRestrictType, 0, 2);
  CmpInst *IC1 = MakeGEPILoadICmp(II, CacheInfo, LIRestrictType, 1, 12);
  auto BOr0 = BinaryOperator::CreateOr(IC0, IC1, "", II);
  auto BNot = BinaryOperator::CreateNot(BOr0, "", II);
  auto LI = MakeGEPILoad(II, CacheInfo, LIRestrictType, 3);
  llvm::CmpInst::Predicate ICMPEQ = ICmpInst::ICMP_EQ;
  auto CI1 = ConstantInt::get(LI->getType(), 1);
  auto IC01 = CmpInst::Create(Instruction::ICmp, ICMPEQ, LI, CI1, "", II);
  auto CI2 = ConstantInt::get(LI->getType(), 2);
  auto IC02 = CmpInst::Create(Instruction::ICmp, ICMPEQ, LI, CI2, "", II);
  auto BOr1 = BinaryOperator::CreateOr(IC01, IC02, "", II);
  auto BAnd0 = BinaryOperator::CreateAnd(BNot, BOr1, "", II);
  llvm::CmpInst::Predicate ICMPSGE = ICmpInst::ICMP_SGE;
  auto IC03 = CmpInst::Create(Instruction::ICmp, ICMPSGE, MLX, W2, "", II);
  auto BAnd1 = BinaryOperator::CreateAnd(BAnd0, IC03, "", II);
  auto IC04 = CmpInst::Create(Instruction::ICmp, ICMPSGE, MLY, H2, "", II);
  auto BAnd2 = BinaryOperator::CreateAnd(BAnd1, IC04, "", II);
  auto BAdd0 = BinaryOperator::CreateAdd(MLX, W2, "", II);
  auto BAdd1 = BinaryOperator::CreateAdd(MLY, H2, "", II);
  auto LI0 = MakeGEPILoad(II, CacheInfo, LIRestrictType, 6);
  auto LI1 = MakeGEPILoad(II, CacheInfo, LIRestrictType, 7);
  llvm::CmpInst::Predicate ICMPSLT = ICmpInst::ICMP_SLT;
  auto IC05 = CmpInst::Create(Instruction::ICmp, ICMPSLT, BAdd0, LI0, "", II);
  auto BAnd3 = BinaryOperator::CreateAnd(BAnd2, IC05, "", II);
  auto IC06 = CmpInst::Create(Instruction::ICmp, ICMPSLT, BAdd1, LI1, "", II);
  auto BAnd4 = BinaryOperator::CreateAnd(BAnd3, IC06, "", II);
  auto IC07 = CmpInst::Create(Instruction::ICmp, ICMPSGE, MLY, H2, "", II);
  auto BAnd5 = BinaryOperator::CreateAnd(BAnd4, IC07, "", II);
  auto IC08 = CmpInst::Create(Instruction::ICmp, ICMPSGE, MLX, W2, "", II);
  auto BAnd6 = BinaryOperator::CreateAnd(BAnd5, IC08, "", II);
  auto CI11 = ConstantInt::get(LI1->getType(), 1);
  auto BSub0 = BinaryOperator::CreateSub(LI1, CI11, "", II);
  auto BSub1 = BinaryOperator::CreateSub(BSub0, H2, "", II);
  llvm::CmpInst::Predicate ICMPSLE = ICmpInst::ICMP_SLE;
  auto IC09 = CmpInst::Create(Instruction::ICmp, ICMPSLE, MLY, BSub1, "", II);
  auto BAnd7 = BinaryOperator::CreateAnd(BAnd6, IC09, "", II);
  auto CI10 = ConstantInt::get(LI0->getType(), 1);
  auto BSub2 = BinaryOperator::CreateSub(LI0, CI10, "", II);
  auto BSub3 = BinaryOperator::CreateSub(BSub2, W2, "", II);
  auto IC10 = CmpInst::Create(Instruction::ICmp, ICMPSLE, MLX, BSub3, "", II);
  auto BAnd8 = BinaryOperator::CreateAnd(BAnd7, IC10, "", II);
  BranchInst::Create(OptBlock, NoOptBlock, BAnd8, II);
  II->eraseFromParent();
}

//
// Simplify branches in the optimized base function using predicates
// involving field values of 'LIRestrict' (i.e. cache_info). Return
// the number of branches simplifed.
//
unsigned PredicateOpt::simplifyCacheInfoBranches(LoadInst *LIRestrict) {

  // Match a basic block which has the form:
  //    'GEPI' = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo,
  //        ptr 'PO', i64 0, i32 'Offset'
  //    %198 = load i32, ptr 'GEPI', align 8
  //    %199 = icmp eq i32 %198, 'V'
  //    br i1 %199, label 'BB0', label 'BB1'
  // and return 'true' if a match is found. In this case, set 'BB0' and
  // 'BB1'.
  auto MatchBB = [](GetElementPtrInst *GEPI, Value *PO, unsigned Offset,
                    unsigned V, BasicBlock *&BB0, BasicBlock *&BB1) -> bool {
    BasicBlock *BB = GEPI->getParent();
    if (GEPI != &BB->front())
      return false;
    BasicBlock *BBP = BB->getSinglePredecessor();
    if (!BBP)
      return false;
    unsigned MyOffset = 0;
    if (!isSimpleGEPI(GEPI, PO, MyOffset) || MyOffset != Offset)
      return false;
    auto LI = dyn_cast_or_null<LoadInst>(GEPI->getNextNonDebugInstruction());
    if (!LI || LI->getPointerOperand() != GEPI)
      return false;
    auto IC = dyn_cast_or_null<ICmpInst>(LI->getNextNonDebugInstruction());
    if (!IC || IC->getPredicate() != ICmpInst::ICMP_EQ ||
        IC->getOperand(0) != LI)
      return false;
    auto CI = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CI || CI->getZExtValue() != V)
      return false;
    auto BI = dyn_cast_or_null<BranchInst>(IC->getNextNonDebugInstruction());
    if (!BI || BI->isUnconditional())
      return false;
    if (BI->getNextNonDebugInstruction())
      return false;
    if (BI->getCondition() != IC)
      return false;
    BB0 = BI->getSuccessor(0);
    BB1 = BI->getSuccessor(1);
    return true;
  };

  // Match the pattern below, with %197 as 'GEPI'.
  //   br i1 %195, label %209, label %196
  // 196:                                              ; preds = %193
  //  %197 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo,
  //      ptr %12, i64 0, i32 0
  //  %198 = load i32, ptr %197, align 8
  //  %199 = icmp eq i32 %198, 2
  //  br i1 %199, label %204, label %200
  // 200:                                              ; preds = %196
  //  %201 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo,
  //      ptr %12, i64 0, i32 1
  //  %202 = load i32, ptr %201, align 4
  //  %203 = icmp eq i32 %202, 12
  //  br i1 %203, label %204, label %207
  // 204:                                              ; preds = %200, %196
  // If a match is found, return '1', otherwise return '0'. If we return '1',
  // change:
  //  br i1 %195, label %209, label %196
  // to
  //  br i1 %195, label %209, label %204
  //
  // This simplifies the code of the optimized base function using the
  // predicate:
  //   cache_info->storage_class == PseudoClass ||
  //     cache_info->colorspace == CMYKColorspace
  // where 'storage_class' is field 0. 'colorspace' is field 1,
  // PseudoClass  is 2 and CMYKColorspace is 12.
  // Return the number of branches that were simplified.
  auto SimplifyCacheInfo01 = [&MatchBB](GetElementPtrInst *GEPI) -> unsigned {
    unsigned RVCount = 0;
    BasicBlock *BB0 = GEPI->getParent();
    Value *PO = GEPI->getPointerOperand();
    BasicBlock *BB1 = nullptr;
    BasicBlock *BB2 = nullptr;
    if (!MatchBB(GEPI, PO, 0, 2, BB1, BB2))
      return 0;
    auto GEPI0 = dyn_cast<GetElementPtrInst>(&BB2->front());
    if (!GEPI0)
      return 0;
    BasicBlock *BB3 = nullptr;
    BasicBlock *BB4 = nullptr;
    if (!MatchBB(GEPI0, PO, 1, 12, BB3, BB4))
      return 0;
    if (BB1 != BB3)
      return 0;
    BasicBlock *BBP = BB0->getSinglePredecessor();
    assert(BBP && "Expecting non-nullptr basic block predecessor");
    auto BI = dyn_cast<BranchInst>(BBP->getTerminator());
    if (!BI)
      return 0;
    for (unsigned I = 0; I < BI->getNumSuccessors(); ++I)
      if (BI->getSuccessor(I) == BB0) {
        LLVM_DEBUG({
          dbgs() << "MRC Predicate: CHANGE SUCCESSOR: ";
          BI->dump();
        });
        BI->setSuccessor(I, BB4);
        RVCount++;
      }
    return RVCount;
  };

  // Match a sequence of the form 1:
  //   'LI' = load i32, ptr %13, align 8
  //   'IC' = icmp eq i32 'LI' , 'V'
  //    br i1 'IC', label %209, label %16
  // If 'V' is 1 or 2, change the branch instruction to:
  //    br i1 false, label %209, label %16
  // Or match a sequence of the form 2:
  //    'LI' = load i32, ptr %13, align 8
  //    'IC' = icmp eq i32 'LI', 1
  //    br i1 'IC', label %35, label %32
  //  32:
  //     %33 = icmp ne i32 %30, 2
  //     %34 = or i1 %33, %25
  // In which case, we change the last instruction to:
  //     %34 = or i1 %33, true
  // Return the number of branches that were simplified.
  auto TestIC = [](LoadInst *LI, ICmpInst *IC) -> unsigned {
    auto CI = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CI)
      return 0;
    if (CI->getZExtValue() != 1 && CI->getZExtValue() != 2) {
      auto CF = ConstantInt::getFalse(IC->getContext());
      LLVM_DEBUG({
        dbgs() << "MRC Predicate: TO FALSE: ";
        IC->dump();
      });
      IC->replaceAllUsesWith(CF);
      return 1;
    }
    if (CI->getZExtValue() != 1)
      return 0;
    unsigned RVCount = 0;
    for (User *U3 : IC->users()) {
      auto BI = dyn_cast<BranchInst>(U3);
      if (!BI || BI->getNumSuccessors() != 2)
        continue;
      auto &I = BI->getSuccessor(1)->front();
      if (auto IC0 = dyn_cast<ICmpInst>(&I))
        if (IC0->getPredicate() == ICmpInst::ICMP_NE)
          if (IC0->getOperand(0) == LI)
            if (auto C2 = dyn_cast<ConstantInt>(IC0->getOperand(1)))
              if (C2->getZExtValue() == 2) {
                auto CT = ConstantInt::getTrue(IC->getContext());
                LLVM_DEBUG({
                  dbgs() << "MRC Predicate: TO TRUE: ";
                  IC0->dump();
                });
                IC0->replaceAllUsesWith(CT);
                RVCount++;
              }
    }
    return RVCount;
  };

  // This simplifies the code of the optimized base function using the
  // predicate:
  // (cache_info->type != MemoryCache) && (cache_info->type != MapCache)
  // where 'type' is field 3, 'MemoryCache' is 1, and 'MapCache' is 2.
  // See 'TestIC' for specific simplifications that can be done.
  // Return the number of branches that were simplified.
  auto SimplifyCacheInfo3 = [&TestIC](GetElementPtrInst *GEPI) -> unsigned {
    unsigned RVCount = 0;
    for (User *U0 : GEPI->users())
      if (auto LI = dyn_cast<LoadInst>(U0))
        for (User *U1 : LI->users())
          if (auto IC = dyn_cast<ICmpInst>(U1))
            if (IC->getPredicate() == ICmpInst::ICMP_EQ)
              if (IC->getOperand(0) == LI)
                RVCount += TestIC(LI, IC);
    return RVCount;
  };

  // Return 'true' if the non-debug instruction following 'PI' is the last
  // non-debug instruction in its BasicBlock, whose condition is 'PI', and
  // which has two successors. In this case, set 'BBO' and 'BB1' to the
  // successore.
  auto CheckBI = [](Instruction *PI, BasicBlock *&BB0,
                    BasicBlock *&BB1) -> bool {
    auto BI = dyn_cast_or_null<BranchInst>(PI->getNextNonDebugInstruction());
    if (!BI || BI->getCondition() != PI || BI->getNumSuccessors() != 2)
      return false;
    if (BI->getNextNonDebugInstruction())
      return false;
    BB0 = BI->getSuccessor(0);
    BB1 = BI->getSuccessor(1);
    return true;
  };

  // Return 'true' if 'BBP' has the form:
  // 'BBP':
  //   %37 = add nsw i64 %5, %3
  //   %38 = icmp sgt i64 %2, -1
  //   br i1 %38, label 'BB0', label 'BB1'
  // Set 'BB0' and 'BB1' in this case.
  auto MatchBB67P = [&](BasicBlock *BBP, BasicBlock *&BB0,
                       BasicBlock *&BB1) -> bool {
    Function *F = BBP->getParent();
    auto BO = dyn_cast<BinaryOperator>(&BBP->front());
    if (!BO || !match(BO, m_Add(m_Specific(F->getArg(5)),
                      m_Specific(F->getArg(3)))))
      return false;
    auto IC = dyn_cast_or_null<ICmpInst>(BO->getNextNonDebugInstruction());
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!IC || !match(IC, m_ICmp(Pred, m_Specific(F->getArg(2)),
                      m_SpecificInt(-1))))
      return false;
    if (Pred != ICmpInst::ICMP_SGT)
      return false;
    return CheckBI(IC, BB0, BB1);
  };
 
  // Return 'true' if 'BB' has the form:
  // 'BB':                                             ; preds = 'BBP'
  //   %40 = add nsw i64 %4, %2
  //   %41 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo,
  //     ptr 'LIRestrict', i64 0, i32 6
  //   'LIOut': %42 = load i64, ptr %41, align 8
  //   %43 = icmp sle i64 %40, %42
  //   %44 = icmp sgt i64 %3, -1
  //   %45 = and i1 %43, %44
  //   br i1 %45, label 'BB0', label 'BB1'
  // Set 'LIOut', 'BB0' and 'BB1' in this case.
  auto MatchBB670 = [&](BasicBlock *BB, BasicBlock *BBP, LoadInst *LIRestrict,
                        LoadInst *&LIOut, BasicBlock *&BB0,
                        BasicBlock *&BB1) -> bool {
    if (BB->getSinglePredecessor() != BBP)
      return false;
    Function *F = BB->getParent();
    auto BOAdd = dyn_cast<BinaryOperator>(&BB->front());
    if (!BOAdd || !match(BOAdd, m_Add(m_Specific(F->getArg(4)),
                         m_Specific(F->getArg(2)))))
      return false;
    Instruction *NI0 = BOAdd->getNextNonDebugInstruction();
    auto GEPI = dyn_cast_or_null<GetElementPtrInst>(NI0);
    unsigned Offset = 0;
    if (!GEPI || !isSimpleGEPI(GEPI, LIRestrict, Offset) || Offset != 6)
      return false;
    auto LI = dyn_cast_or_null<LoadInst>(GEPI->getNextNonDebugInstruction());
    if (!LI || LI->getPointerOperand() != GEPI)
      return false;
    LIOut = LI;
    auto IC0 = dyn_cast_or_null<ICmpInst>(LI->getNextNonDebugInstruction());
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!IC0 || !match(IC0, m_ICmp(Pred, m_Specific(BOAdd), m_Specific(LI))))
      return false;
    if (Pred != ICmpInst::ICMP_SLE)
      return false;
    auto IC1 = dyn_cast_or_null<ICmpInst>(IC0->getNextNonDebugInstruction());
    if (!IC1 || !match(IC1, m_ICmp(Pred, m_Specific(F->getArg(3)),
                       m_SpecificInt(-1))))
      return false;
    if (Pred != ICmpInst::ICMP_SGT)
      return false;
    Instruction *NI1 = IC1->getNextNonDebugInstruction();
    auto BOAnd = dyn_cast_or_null<BinaryOperator>(NI1);
    if (!BOAnd || !match(BOAnd, m_And(m_Specific(IC0), m_Specific(IC1))))
      return false;
    return CheckBI(BOAnd, BB0, BB1);
  };

  // Return 'true' if 'BB' has the form:
  // 'BB':                                               ; preds = 'BBP'
  //   %47 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo,
  //     ptr 'LIRestrict', i64 0, i32 7
  //   %48 = load i64, ptr %47, align 8
  //   %49 = icmp sgt i64 'IFirst', %48
  //   br i1 %49, label 'BB0', label 'BB1'
  // Set 'BB0' and 'BB1' in this case.
  auto MatchBB671 = [&](BasicBlock *BB, BasicBlock *BBP, LoadInst *LIRestrict,
                        Instruction *IFirst, BasicBlock *&BB0,
                        BasicBlock *&BB1) -> bool {
    if (BB->getSinglePredecessor() != BBP)
      return false;
    auto GEPI = dyn_cast<GetElementPtrInst>(&BB->front());
    unsigned Offset = 0;
    if (!GEPI || !isSimpleGEPI(GEPI, LIRestrict, Offset) || Offset != 7)
      return false;
    auto LI = dyn_cast_or_null<LoadInst>(GEPI->getNextNonDebugInstruction());
    if (!LI || LI->getPointerOperand() != GEPI)
      return false;
    auto IC = dyn_cast_or_null<ICmpInst>(LI->getNextNonDebugInstruction());
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!IC || !match(IC, m_ICmp(Pred, m_Specific(IFirst), m_Specific(LI))))
      return false;
    if (Pred != ICmpInst::ICMP_SGT)
      return false;
    return CheckBI(IC, BB0, BB1);
  };

  // Return 'true' if 'BB' has the form:
  // 'BB':                                               ; preds = 'BBP'
  //   %51 = icmp eq i64 %5, 1
  //   br i1 %51, label 'BB0', label 'BB1'
  // Set 'BB0' and 'BB1' in this case.
  auto MatchBB672 = [&](BasicBlock *BB, BasicBlock *BBP,
                       BasicBlock *&BB0, BasicBlock *&BB1) -> bool {
    if (BB->getSinglePredecessor() != BBP)
      return false;
    Function *F = BB->getParent();
    auto IC = dyn_cast_or_null<ICmpInst>(&BB->front());
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!IC || !match(IC, m_ICmp(Pred, m_Specific(F->getArg(5)),
                      m_SpecificInt(1))))
      return false;
    if (Pred != ICmpInst::ICMP_EQ)
      return false;
    return CheckBI(IC, BB0, BB1);
  };

  // Return 'true' if 'BB' has the form:
  // 'BB':                                               ; preds = 'BBP'
  //   %51 = icmp eq i64 %5, 1
  //   br i1 %51, label 'BB0', label 'BB1'
  // Set 'BB0' and 'BB1' in this case.
  auto MatchBB673 = [&](BasicBlock *BB, BasicBlock *BBP,
                        BasicBlock *&BB0, BasicBlock *&BB1) -> bool {
    if (BB->getSinglePredecessor() != BBP)
      return false;
    Function *F = BB->getParent();
    auto IC = dyn_cast_or_null<ICmpInst>(&BB->front());
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!IC || !match(IC, m_ICmp(Pred, m_Specific(F->getArg(2)),
                      m_SpecificInt(0))))
      return false;
    if (Pred != ICmpInst::ICMP_EQ)
      return false;
    return CheckBI(IC, BB0, BB1);
  };

  // Return 'true' if 'BB' has the form:
  // 'BB':                                               ; preds = 'BBP'
  //   %55 = icmp eq i64 'LI', %4
  //   br i1 %55, label 'BB0', label 'BB1'
  // Set 'BB0' and 'BB1' in this case.
  auto MatchBB674 = [&](BasicBlock *BB, BasicBlock *BBP, LoadInst *LI,
                        BasicBlock *&BB0, BasicBlock *&BB1) -> bool {
    if (BB->getSinglePredecessor() != BBP)
      return false;
    Function *F = BB->getParent();
    auto IC = dyn_cast_or_null<ICmpInst>(&BB->front());
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!IC || !match(IC, m_ICmp(Pred, m_Specific(LI),
                      m_Specific(F->getArg(4)))))
      return false;
    if (Pred != ICmpInst::ICMP_EQ)
      return false;
    return CheckBI(IC, BB0, BB1);
  };

  // Return 'true' if 'BB' has the form:
  // 'BB':                                               ; preds = 'BBP'
  //   %57 = urem i64 %4, 'LI'
  //   %58 = icmp eq i64 %57, 0
  //   br i1 %58, label 'BB0', label 'BB1'
  // Set 'BB0' and 'BB1' in this case.
  auto MatchBB675 = [&](BasicBlock *BB, BasicBlock *BBP, LoadInst *LI,
                        BasicBlock *&BB0, BasicBlock *&BB1) -> bool {
    if (BB->getSinglePredecessor() != BBP)
      return false;
    Function *F = BBP->getParent();
    auto BO = dyn_cast<BinaryOperator>(&BB->front());
    if (!BO || !match(BO, m_URem(m_Specific(F->getArg(4)), m_Specific(LI))))
      return false;
    auto IC = dyn_cast_or_null<ICmpInst>(BO->getNextNonDebugInstruction());
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!IC || !match(IC, m_ICmp(Pred, m_Specific(BO), m_SpecificInt(0))))
      return false;
    if (Pred != ICmpInst::ICMP_EQ)
      return false;
    return CheckBI(IC, BB0, BB1);
  };

  // Push all instructions which 'II' depends on and which are in the
  // 'Visited' blocks, but not already in 'InstsToSink' onto 'Deps'.
  std::function<bool(Instruction *, SetVector<BasicBlock *> &,
                SetVector<Instruction *> &, SetVector<Instruction *> &)>
      PushInstAndDeps = [&](Instruction *II,
                            SetVector<BasicBlock *> &Visited,
                            SetVector<Instruction *> &Deps,
                            SetVector<Instruction *> &InstsToSink) -> bool {
    for (unsigned I = 0; I < II->getNumOperands(); ++I) {
       auto III = dyn_cast<Instruction>(II->getOperand(I));
       if (!III)
         return false;
       if (Visited.count(III->getParent()) && !Deps.count(III) &&
           !InstsToSink.count(III)) {
         Deps.insert(III);
         PushInstAndDeps(III, Visited, Deps, InstsToSink);
       }
    }
    return true;
  };

  // Sink instructions in the 'Visited' blocks to 'BBIn' which have uses
  // outside the 'Visited' blocks.
  auto SinkInsts = [&](SetVector<BasicBlock *> &Visited,
                       BasicBlock *BBIn) -> bool {
    SetVector<Instruction *> Deps;
    SetVector<Instruction *> InstsToSink;
    for (BasicBlock *BB : Visited)
      for (auto &I : *BB)
        for (User *U : I.users()) {
          auto II = dyn_cast<Instruction>(U);
          if (!II)
            return false;
          if (!Visited.count(II->getParent())) {
            Deps.clear();
            Deps.insert(&I);
            if (!PushInstAndDeps(&I, Visited, Deps, InstsToSink))
              return false;
            for (int J = Deps.size()-1; J >=0; --J)
              InstsToSink.insert(Deps[J]);
            break;
          }
        }
     ValueToValueMapTy VMap;
     Instruction *InstInsertBefore = &BBIn->front();
     for (auto I : InstsToSink) {
       Instruction *NewI = I->clone();
       VMap[I] = NewI;
       NewI->insertBefore(InstInsertBefore);
       for (unsigned J = 0, JE = NewI->getNumOperands(); J != JE; ++J) {
         if (auto OldI = dyn_cast<Instruction>(NewI->getOperand(J))) {
           auto IT = VMap.find(OldI);
           if (IT != VMap.end())
             NewI->setOperand(J, IT->second);
         }
       }
       I->replaceUsesWithIf(NewI, [&Visited](Use &U) {
         if (auto *UI = dyn_cast<Instruction>(U.getUser()))
           return !Visited.count(UI->getParent());
         return false;
       });
     }
     return true;
  };

  // For each predecessor block of 'BBP' replace any successor which is
  // 'BBP' with 'BBIn'.
  auto ReplacePreds = [](BasicBlock *BBP, BasicBlock *BBIn) {
    SmallVector<BasicBlock *, 2> BBList;
    for (BasicBlock *BB : predecessors(BBP))
      BBList.push_back(BB);
    for (BasicBlock *BB : BBList)
      if (auto BI = dyn_cast<BranchInst>(BB->getTerminator()))
        for (unsigned I = 0; I < BI->getNumSuccessors(); ++I)
          if (BI->getSuccessor(I) == BBP)
            BI->setSuccessor(I, BBIn);
  };

  // Return '2' if we can and do simplify the following conditional to
  // 'true' (otherwise return '0'):
  //  (((nexus_info->region.x >= 0) && (x < (ssize_t) cache_info->columns) &&
  //     (nexus_info->region.y >= 0) && (y < (ssize_t) cache_info->rows)) &&
  //    ((nexus_info->region.height == 1UL) || ((nexus_info->region.x == 0) &&
  //     ((nexus_info->region.width == cache_info->columns) ||
  //      ((nexus_info->region.width % cache_info->columns) == 0)))))
  // The actual instructions which are recognized to match this expression
  // are shown in the comments on the lambda functions.
  // Here 'LIRestrict' is the 'cache_info' pointer while 'GEPI' points to
  // 'cache_info->columns'.
  auto SimplifyCacheInfo67 = [&](LoadInst *LIRestrict,
                                 GetElementPtrInst *GEPI) -> unsigned {
    SetVector<BasicBlock *> Visited;
    BasicBlock *BBP = GEPI->getParent()->getSinglePredecessor();
    if (!BBP)
      return 0;
    BasicBlock *BBIn = nullptr;
    BasicBlock *BBOut = nullptr;
    BasicBlock *BBX = nullptr;
    BasicBlock *BBY = nullptr;
    BasicBlock *BB0 = nullptr;
    Visited.insert(BBP);
    if (!MatchBB67P(BBP, BB0, BBOut))
      return 0;
    LoadInst *LI = nullptr;
    BasicBlock *BB1 = nullptr;
    Visited.insert(BB0);
    if (!MatchBB670(BB0, BBP, LIRestrict, LI, BB1, BBX) || BBX != BBOut) 
      return 0;
    BasicBlock *BB2 = nullptr;
    Visited.insert(BB1);
    if (!MatchBB671(BB1, BB0, LIRestrict, &BBP->front(), BBX, BB2) ||
        BBX != BBOut)
      return 0;
    BasicBlock *BB3 = nullptr;
    Visited.insert(BB2);
    if (!MatchBB672(BB2, BB1, BBIn, BB3))
      return 0;
    Visited.insert(BB3);
    BasicBlock *BB4 = nullptr;
    if (!MatchBB673(BB3, BB2, BB4, BBX) || BBX != BBOut)
      return 0;
    BasicBlock *BB5 = nullptr;
    Visited.insert(BB4);
    if (!MatchBB674(BB4, BB3, LI, BBX, BB5) || BBX != BBIn)
      return 0;
    Visited.insert(BB5);
    if (!MatchBB675(BB5, BB4, LI, BBX, BBY) || BBX != BBIn || BBY != BBOut)
      return 0;
    if (!SinkInsts(Visited, BBIn))
      return 0;
    ReplacePreds(BBP, BBIn);
    return 2;
  };

  // Main code for simplifyCacheInfoBranches():
  unsigned RVCount = 0;
  unsigned Offset = 0;
  for (User *U : LIRestrict->users()) {
    auto GEPI = dyn_cast<GetElementPtrInst>(U);
    if (!GEPI)
      continue;
    if (!isSimpleGEPI(GEPI, LIRestrict, Offset))
      continue;
    if (Offset == 0)
      RVCount += SimplifyCacheInfo01(GEPI);
    else if (Offset == 3)
      RVCount += SimplifyCacheInfo3(GEPI);
    else if (Offset == 6)
      RVCount += SimplifyCacheInfo67(LIRestrict, GEPI);
  }
  return RVCount;
}

//
// Simplify the uses of the fields of nexus_info->region field by replacing
// them with the values to whch they are assigned. 'GEPI6F1' is a GEPI which
// accesses the nexus_info->region field.
//
unsigned PredicateOpt::simplifyNexusInfoRegionUses(GetElementPtrInst *GEPI6F1) {

  // Check thet the 'GEPI' has uses, all of which are LoadInsts except
  // one which is a StoreInst. The StoreInst is deduced and eliminated
  // if 'SIIn' is nullptr, otherwise it is assumed to be 'SIIn'. If the
  // check is passed, return 'true' and replace all of the LoadInst uses
  // with the value operand of the StoreInst.
  auto CheckGEPI = [](GetElementPtrInst *GEPI, StoreInst *SIIn) -> bool {
    SmallVector<LoadInst *, 2> Loads;
    StoreInst *Store = SIIn;
    for (User *U : GEPI->users())
      if (auto SI = dyn_cast<StoreInst>(U)) {
        if (!Store && SI->getPointerOperand() == GEPI)
          Store = SI;
        else
          return false;
      } else if (auto LI = dyn_cast<LoadInst>(U)) {
        Loads.push_back(LI);
      } else {
        return false;
      }
    if (!Store)
      return false;
    for (auto LI : Loads) {
      LI->replaceAllUsesWith(Store->getValueOperand());
      LI->eraseFromParent();
    }
    if (!SIIn)
      Store->eraseFromParent();
    GEPI->eraseFromParent();
    return true;
  };

  // Find all of GEPIs which are uses of 'GEPI6F1' and its single StoreInst.
  unsigned Count = 0;
  StoreInst *SIGEPI6F1 = nullptr;
  SmallVector<GetElementPtrInst *, 5> GEPIs;
  for (User *U : GEPI6F1->users()) {
    if (auto SI = dyn_cast<StoreInst>(U)) {
      if (SI->getPointerOperand() == GEPI6F1 && !SIGEPI6F1)
        SIGEPI6F1 = SI;
      else
        return false;
    } else if (auto GEPI = dyn_cast<GetElementPtrInst>(U)) {
      GEPIs.push_back(GEPI);
    } else {
      return false;
    }
  }
  // Return if the StoreInst is not unique.
  if (!SIGEPI6F1)
    return false;
  // Forward substitute field values of nexus_info->region.
  for (auto GEPI : GEPIs) {
    unsigned Offset = 0;
    if (isSimpleGEPI(GEPI, GEPI6F1, Offset)) {
      if (Offset == 0 && CheckGEPI(GEPI, SIGEPI6F1))
        Count++;
      else
        return false;
    } else {
      if (GEPI->getNumOperands() != 2)
        return false;
      if (CheckGEPI(GEPI, nullptr))
        Count++;
      else
        return false;
    }
  }
  // Eliminate 'GEPI6F1' and the value stored to it.
  Count++;
  SIGEPI6F1->eraseFromParent();
  GEPI6F1->eraseFromParent();
  return Count;
}

//
// Simple constant propagation for certain arguments of 'OptBaseF', which has
// a single call site 'OptBaseCB'.
//
unsigned PredicateOpt::propagateOptBaseFArgConsts(CallBase *OptBaseCB,
                                                  Function *OptBaseF) {
  unsigned Count = 0;
  if (auto CI4 = dyn_cast<ConstantInt>(OptBaseCB->getArgOperand(4))) {
    OptBaseF->getArg(4)->replaceAllUsesWith(CI4);
    Count++;
  }
  if (auto CI5 = dyn_cast<ConstantInt>(OptBaseCB->getArgOperand(5))) {
    OptBaseF->getArg(5)->replaceAllUsesWith(CI5);
    Count++;
  }
  return Count;
}

//
// Simplify mixed expressions involving the arguments of the optimized
// base function 'OptBaseF' and fields from the structure pointed to
// by 'LIRestrict'. In particular, let 'x' and 'y' be arguments of the
// optimized base function, and let field 6 pointed to by 'LIRestrict'
// represent cache_info->columns and field 7 pointed to be 'LIRestrict'
// represent cache_info->rows. Then since the guards on the optimized
// base function imply:
//   (x >= 0) && ((ssize_t) (x+columns) <= (ssize_t) cache_info->columns) &&
//   (y >= 0) && ((ssize_t) (y+rows) <= (ssize_t) cache_info->rows))
// evaluates to 'true', ensure that this expression is optimized to 'true'.
//
unsigned PredicateOpt::simplifyMixedExpressions(Function *OptBaseF,
                                                LoadInst *LIRestrict) {

  // If 'V' represents a LoadInst fed by a GetElementPtrInst of the form:
  //   'GEPI' = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo,
  //      ptr 'LIRestrict', i64 0, i32 'Offset'
  //   'V' = load i64, ptr 'GEPI', align 8
  // return 'LI' and set the value of 'Offset'.
  auto LoadInstWithGEPI = [](Value *V, LoadInst *LIRestrict,
                             unsigned &Offset) -> LoadInst * {
    auto LI = dyn_cast<LoadInst>(V);
    if (!LI)
      return nullptr;
    auto GEPI = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
    if (!GEPI || !isSimpleGEPI(GEPI, LIRestrict, Offset))
      return nullptr;
    return LI;
  };

  // If 'PHIN' is a PHINode, fill 'PHINodeTerms' with the non-PHINode
  // terminals that define 'PHIN'. For example, in:
  //   %61 = add i64 %60, %2
  //   %134 = add i64 %133, %2
  //   %139 = add i64 %138, %2
  //   %146 = phi i64 [ %134, %130 ], [ %139, %135 ]
  //   %157 = phi i64 [ %146, %145 ], [ %61, %152 ]
  // If '%157" is 'PHIN', then on return 'PHINodeTerms" will contain
  //   %61, %134, and %139.
  std::function<void(PHINode *, SmallVectorImpl<Value *> &)>
      GetPHINodeTerminals =
          [&](PHINode *PHIN, SmallVectorImpl<Value *> &PHINodeTerms) {
            for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; ++I) {
              Value *V = PHIN->getIncomingValue(I);
              if (auto PHIN0 = dyn_cast<PHINode>(V))
                GetPHINodeTerminals(PHIN0, PHINodeTerms);
              else
                PHINodeTerms.push_back(V);
            }
          };

  // Find a representative LoadInst whose pointer operand is a
  // GetElementPtrInst having the form:
  //   'GEPI' = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo,
  //      ptr 'LIRestrict', i64 0, i32 'Offset'
  //   'V' = load i64, ptr 'GEPI', align 8
  // If 'V' is a LoadInst and has the above form, return 'V'. If 'V' is
  // a PHINode and all of the terminals of the PHINode have the above form
  // with the same 'Offset', return one of the terminals. Otherwise, return
  // 'nullptr'.
  auto RepLoadInstWithGEPI = [&](Value *V, LoadInst *LIRestrict) -> LoadInst * {
    unsigned Offset = 0;
    unsigned CommonOffset = 0;
    if (auto LI = LoadInstWithGEPI(V, LIRestrict, Offset))
      return LI;
    if (auto PHIN = dyn_cast<PHINode>(V)) {
      SmallVector<Value *, 8> PHINodeTerms;
      GetPHINodeTerminals(PHIN, PHINodeTerms);
      LoadInst *LIRep = nullptr;
      for (auto V : PHINodeTerms)
        if (auto LI = LoadInstWithGEPI(V, LIRestrict, Offset))
          if (!LIRep) {
            CommonOffset = Offset;
            LIRep = LI;
          } else if (Offset != CommonOffset) {
            return nullptr;
          }
      return LIRep;
    }
    return nullptr;
  };

  // Let 'A' be an argument of the optimized base function. Let 'LIRestrict'
  // point to the key restrict variable. Replace the users of ICmpInsts
  // with either 'true' or 'false' as follows:
  // Replace the uses of:
  //   icmp 'PredTestOuter' 'A', 'CIValue'
  // with 'BOuter'.
  // Replace the uses of:
  //   'GEPI' = getelementptr 'LIRestrict', 0, 'OffsetTest'
  //   'LI' = load 'GEPI'
  //   'UU' = add 'A', 1
  //   icmp 'PredTestInner' 'UU', 'LI'
  // with 'BOuter'.
  auto ReplaceExp = [&](Argument *A, LoadInst *LIRestrict,
                        ICmpInst::Predicate PredTestInner,
                        ICmpInst::Predicate PredTestOuter, unsigned OffsetTest,
                        int64_t CIValue, bool BOuter, bool BInner) -> unsigned {
    unsigned Count = 0;
    for (User *U : A->users()) {
      ICmpInst::Predicate Pred0 = ICmpInst::BAD_ICMP_PREDICATE;
      ConstantInt *CIO = nullptr;
      if (match(U, m_Add(m_Specific(A), m_One())) ||
          match(U, m_Add(m_One(), m_Specific(A)))) {
        for (User *UU : U->users()) {
          Value *V = nullptr;
          ICmpInst::Predicate Pred1 = ICmpInst::BAD_ICMP_PREDICATE;
          if (match(UU, m_ICmp(Pred1, m_Specific(U), m_Value(V))))
            if (Pred1 == PredTestInner) {
              if (LoadInst *LI = RepLoadInstWithGEPI(V, LIRestrict)) {
                auto GEPI = cast<GetElementPtrInst>(LI->getPointerOperand());
                auto CI = cast<ConstantInt>(GEPI->getOperand(2));
                unsigned Offset = CI->getSExtValue();
                if (Offset == OffsetTest) {
                  auto CII = ConstantInt::getBool(UU->getType(), BInner);
                  UU->replaceAllUsesWith(CII);
                  Count++;
                }
              }
            }
        }
      } else if (match(U, m_ICmp(Pred0, m_Specific(A), m_ConstantInt(CIO)))) {
        if (Pred0 == PredTestOuter && CIO->getSExtValue() == CIValue) {
          auto CII = ConstantInt::getBool(U->getType(), BOuter);
          U->replaceAllUsesWith(CII);
          Count++;
        }
      }
    }
    return Count;
  };

  unsigned Count = 0;
  // Replace the uses of %174 in:
  //   %174 = icmp sgt i64 %2, -1
  // with 'true'.
  // Replace the uses of %178 in:
  //   %56 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo,
  //     ptr %12, i64 0, i32 6
  //   %57 = load i64, ptr %56, align 8
  //   %131 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo,
  //     ptr %12, i64 0, i32 6
  //   %132 = load i64, ptr %131, align 8
  //   %136 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo,
  //     ptr %12, i64 0, i32 6
  //   %137 = load i64, ptr %136, align 8
  //   %147 = phi i64 [ %132, %130 ], [ %137, %135 ]
  //   %161 = phi i64 [ %147, %145 ], [ %57, %152 ]
  //   %177 = add i64 1, %2
  //   %178 = icmp sgt i64 %177, %161
  // with 'false'.
  Count += ReplaceExp(OptBaseF->getArg(2), LIRestrict, ICmpInst::ICMP_SGT,
                      ICmpInst::ICMP_SGT, 6, -1, true, false);
  // Replace the uses of %179 in:
  //   %179 = icmp slt i64 %3, 0
  // with 'false'.
  // Replace the uses of %182 in:
  //   %58 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo,
  //     ptr %12, i64 0, i32 7
  //   %59 = load i64, ptr %58, align 8
  //   %150 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo,
  //     ptr %12, i64 0, i32 7
  //   %151 = load i64, ptr %150, align 8
  //   %160 = phi i64 [ %151, %145 ], [ %59, %152 ]
  //   %181 = add i64 1, %3
  //   %182 = icmp sgt i64 %181, %160
  // with 'false'.
  Count += ReplaceExp(OptBaseF->getArg(3), LIRestrict, ICmpInst::ICMP_SGT,
                      ICmpInst::ICMP_SLT, 7, 0, false, false);
  return Count;
}

//
// Perform Partial Dead Store Elimination of 'WrapperF' argument 3,
// converting:
//   *pixel=cache_view->image->background_color;
//   pixels=GetVirtualPixelsFromNexus(cache_view->image,
//     cache_view->virtual_pixel_method,x,y,1,1,cache_view->nexus_info[id],
//     exception);
//   if (pixels == (const PixelPacket *) NULL)
//     return(MagickFalse);
//   *pixel=(*pixels);
//   return(MagickTrue);
// into:
//   pixels=GetVirtualPixelsFromNexus(cache_view->image,
//     cache_view->virtual_pixel_method,x,y,1,1,cache_view->nexus_info[id],
//     exception);
//   if (pixels == (const PixelPacket *) NULL) {
//     *pixel=cache_view->image->background_color;
//     return(MagickFalse);
//   }
//   *pixel=(*pixels);
//   return(MagickTrue);
//
bool PredicateOpt::doPDSEinWrapperFArg3(CallBase *BigLoopCB,
                                        Function *WrapperF) {

  // Return 'true' if the 'BB' has exactly 'ECount' stores.
  auto StoreCount = [](BasicBlock *BB, unsigned ECount) -> bool {
    unsigned Count = 0;
    for (auto &I : *BB)
      if (isa<StoreInst>(&I))
        Count++;
    return Count == ECount;
  };

  // Check that argument 3 is defined by an AllocaInst in
  // the caller of 'WrapperF', which allocates a StructType.
  auto AI = dyn_cast<AllocaInst>(BigLoopCB->getArgOperand(3));
  if (!AI)
    return false;
  auto STy = dyn_cast<StructType>(AI->getAllocatedType());
  if (!STy)
    return false;
  // Check that argument 3 has 'ECount' uses, one for each
  // field of the StructType.
  unsigned ECount = STy->getNumElements();
  Module *M = WrapperF->getParent();
  Argument *A3 = WrapperF->getArg(3);
  if (!A3->hasNUses(ECount))
    return false;
  // Check that the control flow of 'WrapperF' is a simple if-then.
  DenseMap<unsigned, GetElementPtrInst *> GEPIMap;
  BasicBlock *BB0 = &WrapperF->getEntryBlock();
  auto BI0 = dyn_cast<BranchInst>(BB0->getTerminator());
  if (!BI0 || BI0->isUnconditional() || BI0->getNumSuccessors() != 2)
    return false;
  BasicBlock *BB1 = BI0->getSuccessor(0);
  BasicBlock *BB2 = BI0->getSuccessor(1);
  auto RI = dyn_cast<ReturnInst>(BB1->getTerminator());
  if (!RI)
    return false;
  auto BI2 = dyn_cast<BranchInst>(BB2->getTerminator());
  if (!BI2 || !BI2->isUnconditional() || BI2->getSuccessor(0) != BB1)
    return false;
  // Check that each field of argument 3 is accessed by a GEPI.
  for (User *U : A3->users()) {
    auto GEPI = dyn_cast<GetElementPtrInst>(U);
    unsigned Offset = 0;
    if (!GEPI || !isSimpleGEPI(GEPI, A3, Offset) || Offset >= ECount)
      return false;
    if (GEPI->getParent() != BB0 || GEPIMap.count(Offset))
      return false;
    GEPIMap.insert({Offset, GEPI});
  }
  // Check that each sinkable store and the instructions feeding it
  // which can also be sunk have the form:
  //   %i7 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i6,
  //     i64 0, i32 'Offset'
  //   %i9 = load i16, ptr %i7, align 2
  //   store i16 %i9, ptr %i8, align 2
  // While checking, see if we can also sink:
  //   %i6 = getelementptr inbounds %struct._ZTS6_Image._Image,
  //     ptr %i5, i64 0, i32 12
  SmallVector<StoreInst *, 4> StoresToMove;
  bool ShouldSinkGEPI1 = true;
  GetElementPtrInst *GEPI1 = nullptr;
  for (unsigned I = 0, E = ECount; I < E; ++I) {
    auto GEPI = GEPIMap.lookup(I);
    bool BB0Seen = false;
    bool BB2Seen = false;
    for (User *U : GEPI->users()) {
      auto SI = dyn_cast<StoreInst>(U);
      if (!SI || SI->getPointerOperand() != GEPI)
        return false;
      if (SI->getParent() == BB0) {
        if (BB0Seen)
          return false;
        BB0Seen = true;
        auto LI = dyn_cast<LoadInst>(SI->getValueOperand());
        if (!LI || !LI->hasOneUse() || LI->getParent() != BB0)
          return false;
        auto GEPI0 = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
        if (!GEPI0 || !GEPI0->hasOneUse() || GEPI0->getParent() != BB0)
          return false;
        if (ShouldSinkGEPI1) {
          Value *PO = GEPI0->getPointerOperand();
          auto GEPIT = dyn_cast<GetElementPtrInst>(PO);
          if (!GEPIT || GEPIT->getParent() != BB0)
            ShouldSinkGEPI1 = false;
          if (!GEPI1)
            GEPI1 = GEPIT;
          else if (GEPIT != GEPI1)
            ShouldSinkGEPI1 = false;
        }
        StoresToMove.push_back(SI);
      } else if (SI->getParent() == BB2) {
        if (BB2Seen)
          return false;
        BB2Seen = true;
      } else {
        return false;
      }
    }
    if (!BB0Seen || !BB2Seen)
      return false;
  }
  // Check that the stores we have analyzed above are the only ones
  // in 'WrapperF'.
  if (!StoreCount(BB0, 4))
    return false;
  if (!StoreCount(BB1, 0))
    return false;
  if (!StoreCount(BB2, 4))
    return false;
  // Create a new basic block and sink the appropriate instructions
  // into it.
  BasicBlock *BB3 = BasicBlock::Create(M->getContext(), "sink", WrapperF);
  auto BI3 = BranchInst::Create(BB1, BB3);
  if (ShouldSinkGEPI1) {
    GEPI1->removeFromParent();
    GEPI1->insertBefore(BI3);
  }
  for (auto SI : StoresToMove) {
    auto *LI = cast<LoadInst>(SI->getValueOperand());
    auto *GEPI = cast<GetElementPtrInst>(LI->getPointerOperand());
    GEPI->removeFromParent();
    GEPI->insertBefore(BI3);
    LI->removeFromParent();
    LI->insertBefore(BI3);
    SI->removeFromParent();
    SI->insertBefore(BI3);
  }
  // Patch up the control flow and PHINodes.
  BI0->setSuccessor(0, BB3);
  for (auto &I : *BB1)
    if (auto PHIN = dyn_cast<PHINode>(&I))
      for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; ++I)
        if (PHIN->getIncomingBlock(I) == BB0)
          PHIN->setIncomingBlock(I, BB3);
  return true;
}

bool PredicateOpt::buildColdCodeMetadata(CallBase *OptColdCB,
                                         Function *OptColdF) {

  // Return the DTransType corresponding 'Ty'.
  // NOTE: Only several important types which are needed here
  // are supported. This function can be extended if that is
  // found to be useful.
  auto GetDTransType = [&](dtransOP::DTransTypeManager &TM,
                           dtransOP::DTransTypeBuilder &TB,
                           Type *Ty) -> dtransOP::DTransType * {
    if (auto STy = dyn_cast<StructType>(Ty))
      return TM.getStructType(STy->getName());
    if (auto ITy = dyn_cast<IntegerType>(Ty))
      return TM.getOrCreateAtomicType(ITy);
    if (Ty->isVoidTy())
      return TB.getVoidTy();
    return nullptr;
  };

  Module *M = OptColdF->getParent();
  if (!dtransOP::TypeMetadataReader::getDTransTypesMetadata(*M))
    return true;
  dtransOP::DTransTypeManager TM(M->getContext());
  dtransOP::TypeMetadataReader TR(TM);
  if (!TR.initialize(*M))
    return false;
  dtransOP::DTransTypeBuilder TB(TM);
  Type *RetTy = OptColdF->getReturnType();
  dtransOP::DTransType *RetDTy = GetDTransType(TM, TB, RetTy);
  if (!RetDTy)
    return false;
  SmallVector<dtransOP::DTransType *, 15> ArgTypes;
  for (unsigned I = 0, E = OptColdCB->arg_size(); I < E; ++I) {
    Value *V = OptColdCB->getArgOperand(I);
    Type *VTy = V->getType();
    if (VTy->isPointerTy()) {
      dtransOP::DTransType *DETy = nullptr;
      dtransOP::DTransPointerType *DPTy = nullptr;
      if (Type *PETy = inferPtrElementType(*V)) {
        DETy = GetDTransType(TM, TB, PETy);
        if (!DETy)
          DETy = TB.getIntNTy(1);
      } else {
        DETy = TB.getIntNTy(1);
      }
      DPTy = TM.getOrCreatePointerType(DETy);
      ArgTypes.push_back(DPTy);
    } else {
      dtransOP::DTransType *DTy = GetDTransType(TM, TB, VTy);
      if (!DTy)
        return false;
      ArgTypes.push_back(DTy);
    }
  }
  dtransOP::DTransFunctionType *DFTy =
      TB.getFunctionType(RetDTy, ArgTypes, /*IsVarArg=*/false);
  dtransOP::DTransTypeMetadataBuilder::setDTransFuncMetadata(OptColdF, DFTy);
  return true;
}

//
// Attempt to perform the predicate opt. Return 'true' if it was possible.
// NOTE: Right now we simply emit a trace indicating the key elements
// on the hot path, and the extracted and enclosing functions.
// More code will be added to do the optimization.
//
bool PredicateOpt::doPredicateOpt() {

  auto findUniqueCB = [](Function *Caller, Function *Callee) -> CallBase * {
    CallBase *CBOut = nullptr;
    for (User *U : Callee->users())
      if (auto CB = dyn_cast<CallBase>(U))
        if (CB->getCaller() == Caller) {
          if (CBOut)
            return nullptr;
          CBOut = CB;
        }
    return CBOut;
  };

  Function *WrapperF = WrapperCB->getCaller();
  Function *BigLoopF = BigLoopCB->getCaller();
  (void)BigLoopF;
  LLVM_DEBUG({
    dbgs() << "MRC PredicateOpt: Loop Depth = " << SimpleLoopDepth << "\n";
    dbgs() << "  BaseF: " << BaseF->getName() << "\n";
    dbgs() << "  WrapperF: " << WrapperF->getName() << "\n";
    dbgs() << "  BigLoopF: " << BigLoopF->getName() << "\n";
  });
  Function *FxnOuter = BigLoopCB->getCaller();
  DominatorTree &DT = (*GetDT)(*FxnOuter);
  BlockFrequencyInfo &BFI = (*GetBFI)(*FxnOuter);
  BranchProbabilityInfo &BPI = (*GetBPI)(*FxnOuter);
  bool DidPDSE = doPDSEinWrapperFArg3(BigLoopCB, WrapperF);
  LLVM_DEBUG(dbgs() << "MRC Predicate: DidPDSE : " << (DidPDSE ? "T" : "F")
                    << "\n");
  if (!DidPDSE)
    return false;
  LoadInst *CacheInfo = makeHoistedRestrictVar();
  RegionSplitter MultiLoopSplitter(DT, BFI, BPI);
  Function *NoOptF = MultiLoopSplitter.splitRegion(*MultiLoop);
  NoOptF->removeFnAttr(Attribute::NoInline);
  NoOptF->addFnAttr(Attribute::AlwaysInline);
  CallBase *NoOptCB = cast<CallBase>(NoOptF->user_back());
  getInlineReport()->doOutlining(BigLoopF, NoOptF, NoOptCB);
  getMDInlineReport()->doOutlining(BigLoopF, NoOptF, NoOptCB);
  ValueToValueMapTy VMap;
  OptF = IPCloneFunction(NoOptF, VMap);
  OptF->addFnAttr(Attribute::AlwaysInline);
  BasicBlock *NoOptBB = nullptr;
  BasicBlock *HoistBB = nullptr;
  BasicBlock *OptBB = nullptr;
  cloneNoOptBB(NoOptCB->getParent(), OptF, NoOptF, HoistBB, OptBB, NoOptBB);
  makeOptTest(CacheInfo, LIRestrictType, MLX, MLY, W2, H2, HoistBB, OptBB,
              NoOptBB);
  LLVM_DEBUG(dbgs() << "MRC Predicate: OptF: " << OptF->getName() << "\n");
  LLVM_DEBUG(dbgs() << "MRC Predicate: NoOptF: " << NoOptF->getName() << "\n");
  CallBase *OptWrapperCB = findUniqueCB(OptF, WrapperF);
  assert(OptWrapperCB && "Expecting OptWrapperCB");
  Function *OptWrapperF = IPCloneFunction(WrapperF, VMap);
  setCalledFunction(OptWrapperCB, OptWrapperF);
  LLVM_DEBUG(dbgs() << "MRC Predicate: OptWrapperF : " << OptWrapperF->getName()
                    << "\n");
  CallBase *OptBaseCB = findUniqueCB(OptWrapperF, BaseF);
  Function *OptBaseF = IPCloneFunction(BaseF, VMap);
  LIRestrict = cast<LoadInst>(VMap[LIRestrict]);
  GEPI6F1 = cast<GetElementPtrInst>(VMap[GEPI6F1]);
  setCalledFunction(OptBaseCB, OptBaseF);
  LLVM_DEBUG(dbgs() << "MRC Predicate: OptBaseF : " << OptBaseF->getName()
                    << "\n");
  Function *OptColdF = extractColdCode(OptBaseF);
  if (!OptColdF)
    return false;
  CallBase *OptColdCB = findUniqueCB(OptBaseF, OptColdF);
  assert(OptBaseCB && "Expecting OptBaseCB");
  if (!buildColdCodeMetadata(OptColdCB, OptColdF))
    return false;
  getInlineReport()->doOutlining(OptBaseF, OptColdF, OptColdCB);
  getMDInlineReport()->doOutlining(OptBaseF, OptColdF, OptColdCB);
  LLVM_DEBUG(dbgs() << "MRC Predicate: ColdF : " << OptColdF->getName()
                    << "\n");
  unsigned RVCount0 = simplifyCacheInfoBranches(LIRestrict);
  LLVM_DEBUG(dbgs() << "MRC Predicate: " << RVCount0
                    << " CacheInfo Branches Simplified\n");
  if (RVCount0 < 6)
    return false;
  unsigned RVCount1 = simplifyNexusInfoRegionUses(GEPI6F1);
  LLVM_DEBUG(dbgs() << "MRC Predicate: " << RVCount1
                    << " NexusInfoRegion Uses Simplified\n");
  if (RVCount1 < 5)
    return false;
  unsigned RVCount2 = propagateOptBaseFArgConsts(OptBaseCB, OptBaseF);
  LLVM_DEBUG(dbgs() << "MRC Predicate: " << RVCount2
                    << " OptBaseF Args Propagated\n");
  if (RVCount2 < 2)
    return false;
  unsigned RVCount3 = simplifyMixedExpressions(OptBaseF, LIRestrict);
  LLVM_DEBUG(dbgs() << "MRC Predicate: " << RVCount3
                    << " Mixed Expressions Simplified\n");
  if (RVCount3 < 6)
    return false;
  return true;
}

// End of code for class PredicateOpt

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
        setCalledFunction(NCB, NewF);
        return;
      }
    }
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
      [&MakeTAndFromMap](CallBase *CB, Function *NewF, SmallArgumentSet &IfArgs,
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
    makeBlocks(CB, CBClone, TAnd, BBPred);
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
      [&MakeTAndFromMap](CallBase *CB, SmallArgConstMap ArgConstMap,
                         SmallArgumentSet &ReplaceArgs) -> CallBase * {
    BasicBlock *BBPred = CB->getParent();
    BBPred->splitBasicBlock(CB);
    BBPred->getTerminator()->eraseFromParent();
    Value *TAnd = nullptr;
    Function *NewF = CB->getCaller();
    CallBase *CBClone = cast<CallBase>(CB->clone());
    TAnd = MakeTAndFromMap(TAnd, CB, CBClone, BBPred, NewF, ReplaceArgs,
                           ArgConstMap);
    makeBlocks(CB, CBClone, TAnd, BBPred);
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
          DebugLoc CBDbgLoc =
              DILocation::get(CB->getContext(), DIS->getScopeLine(), 0, DIS);
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
    Function *NewF = IPCloneFunction(&F, VMap);
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

// BEGIN: Many Loops Specialization Cloning

//
// Set of clones made during "many loops specialization cloning". We maintain
// this list so that we don't try to make a clone of the unspecialized
// version of the cloned function.
//
static SmallPtrSet<Function *, 4> ManyLoopClones;

//
// Return 'true' if 'F' is a candidate for "many loops specialization
// cloning". In this variant of cloning, we look for an Argument of F
// which is a pointer to a structure, and one field of the structure
// is used as a loop bound for many loops in the function. Furthermore,
// we expect the loop with that bound to index into one or more arrays,
// and make a clone using the upper bound of the array as a predicted
// value for the loop bound.
//
// If we return 'true', '*IC' is set to a specialization test that can be
// used to test whether the specialized clone should be executed. '*LIOut'
// is set to the LoadInst which loads the value of the structure field, and
// '*TCNumber' is set to the predicted value of the loop trip counts.
//
static bool isManyLoopSpecializationCandidate(Function *F,
                                              Value **IC,
                                              LoadInst **LIOut,
                                              unsigned *TCNumber) {
  //
  // If 'U' is the trip count for a simple single BasicBlock loop whose
  // induction variable starts with 1 and increments by 1, return the
  // predicted value of that trip count using the arrays that are
  // indexed by the loop induction variable. If 'U' is not such a trip
  // count, return 0.
  //
  auto OneTripCount = [](User *U) -> unsigned {
    // Check that this is a single block loop.
    auto IC = dyn_cast<ICmpInst>(U);
    if (!IC || !IC->hasOneUse()) {
      LLVM_DEBUG(dbgs() << "No ICmpInst or more than one use.\n");
      return 0;
    }
    auto BI = dyn_cast<BranchInst>(*IC->user_begin());
    if (!BI) {
      LLVM_DEBUG(dbgs() << "ICmpInst does not feed branch.\n");
      return 0;
    }
    auto BB = BI->getParent();
    unsigned Count = 0;
    for (unsigned I = 0, E = BI->getNumSuccessors(); I < E; ++I)
      if (BI->getSuccessor(I) == BB) {
        Count = 1;
        break;
      }
    if (!Count) {
      LLVM_DEBUG(dbgs() << "Did not find single BasicBlock loop\n");
      return 0;
    }
    // Check that the increment value is 1.
    Value *V0 = IC->getOperand(0) == U ? IC->getOperand(1) :
        IC->getOperand(0);
    Value *V1 = nullptr;
    if (!match(V0, m_Add(m_Value(V1), m_One()))) {
      LLVM_DEBUG(dbgs() << "Did not find loop increment\n");
      return 0;
    }
    auto PHIN = dyn_cast<PHINode>(V1);
    if (!PHIN || PHIN->getNumIncomingValues() != 2) {
      LLVM_DEBUG(dbgs() << "No PHINode or not 2 incoming values\n");
      return 0;
    }
    // Check that the initial value is 1.
    auto CI = dyn_cast<ConstantInt>(PHIN->getIncomingValue(0));
    if (!CI || !CI->isOne()) {
      LLVM_DEBUG(dbgs() << "Simple loop does not start with 1\n");
      return 0;
    }
    if (PHIN->getIncomingValue(1) != V0) {
      LLVM_DEBUG(dbgs() << "Missing loop induction variable\n");
      return 0;
    }
    unsigned TCN = 0;
    bool FirstTime = true;
    // Look for arrays that are indexed by the induction variable.
    for (User *U : PHIN->users()) {
      if (U == V0)
        continue;
      auto SI0 = dyn_cast<SubscriptInst>(U);
      if (!SI0) {
        LLVM_DEBUG(dbgs() << "Induction variable not used in SubscriptInst\n");
        return 0;
      }
      if (SI0->getIndex() != PHIN) {
        LLVM_DEBUG(dbgs() << "Induction variable does not index "
                          << "SubscriptInst\n");
        return 0;
      }
      // If the stride is not a constant, we will tolerate this and skip
      // this case. We don't need this to help all of the loops with
      // SubscriptInst accesses as long as it helps enough of them.
      auto CIS0 = dyn_cast<ConstantInt>(SI0->getStride());
      if (!CIS0 || CIS0->isZero())
        continue;
      // Find the stride of array in the the innermost loop.
      unsigned BaseStride = CIS0->getZExtValue();
      auto SI1 = dyn_cast<SubscriptInst>(SI0->getPointerOperand());
      if (!SI1) {
        LLVM_DEBUG(dbgs() << "Array not at least 2D\n");
        return 0;
      }
      // Find the stride of the array in the next enclosing loop.
      auto CIS1 = dyn_cast<ConstantInt>(SI1->getStride());
      if (!CIS1) {
        LLVM_DEBUG(dbgs() << "Cannot find outer loop stride\n");
        return 0;
      }
      unsigned NextStride = CIS1->getZExtValue();
      // Divide the former into the latter to get the predicted value.
      if (NextStride % BaseStride != 0) {
        LLVM_DEBUG(dbgs() << "Inner loop stride not multiple of "
                             "outer loop stride\n");
        return 0;
      }
      unsigned TCNL = NextStride / BaseStride;
      // Allow only a single, consistent predicted value.
      if (!FirstTime && (TCNL != TCN)) {
        LLVM_DEBUG(dbgs() << "Conflicting stride value\n");
        return 0;
      }
      TCN = TCNL;
      FirstTime = false;
    }
    return TCN;
  };

  //
  // If all of the uses of 'V' represent a consistent trip count value for
  // a simple loop, return the predicted value for that trip count, and set
  // '*TripCountCount' to the number of loops encountered. Otherwise, return 0.
  //
  auto TripCount = [&OneTripCount](Value *V,
                                   unsigned *TripCountCount) -> unsigned {
    unsigned TCN = 0;
    unsigned Count = 0;
    for (User *U : V->users()) {
      unsigned TCNL = OneTripCount(U);
      if (Count != 0 && (TCNL != TCN))
        return 0;
      TCN = TCNL;
      Count++;
    }
    *TripCountCount = Count;
    return TCN;
  };

  //
  // Given 'CI' a call site which we would like to specialize on the Argument
  // with 'ArgNo', a field of which is accessed with 'GEPI' and 'LI', and
  // 'TCNumber' is the predicted field value, return a condition test
  // which tests whether the field value is equal to the predicted value.
  //
  auto CondTest = [](CallInst *CI, int ArgNo, GetElementPtrInst *GEPI,
                     LoadInst *LI, unsigned TCNumber) -> Value * {
    auto SP = CI->getFunction()->getSubprogram();
    auto DIL = SP ? DILocation::get(SP->getContext(),
        CI->getDebugLoc()->getLine(), CI->getDebugLoc()->getColumn(), SP) :
        nullptr;
    IRBuilder <> Builder(CI);
    Value *ArgOut = CI->getArgOperand(ArgNo);
    Type *Ty = GEPI->getSourceElementType();
    Value *GEPINew = Builder.CreateConstGEP2_32(Ty, ArgOut, 0, 1);
    LoadInst *LINew = Builder.CreateLoad(LI->getType(), GEPINew, "");
    if (DIL)
      LINew->setDebugLoc(DIL);
    auto CI2 = ConstantInt::get(LI->getType(), TCNumber);
    Value *IC = Builder.CreateICmp(ICmpInst::ICMP_EQ, LINew, CI2, "");
    return IC;
  };

  // Main code for isManyLoopSpecializationCandidate().

  LLVM_DEBUG(dbgs() << "MLSC: Testing " << F->getName() << ": ");
  // Do not clone the fallback case for a function to which we have already
  // applied many loops specialization cloning.
  if (ManyLoopClones.count(F)) {
    LLVM_DEBUG(dbgs() << "Already an MLSC candidate\n");
    return false;
  }
  // This heuristic requires SubscriptInsts, which will only appear in
  // Fortran Functions.
  if (!F->isFortran()) {
    LLVM_DEBUG(dbgs() << "Not a Fortran Function\n");
    return false;
  }
  // Only clone when this is a unique call site.
  CallInst *CI = uniqueCallSite(*F);
  if (!CI) {
    LLVM_DEBUG(dbgs() << "Not a unique call site\n");
    return false;
  }
  if (F->arg_empty()) {
    LLVM_DEBUG(dbgs() << "No formal args\n");
    return false;
  }
  // Test the arguments of F in turn, choosing the first one that passes
  // the criteria.
  LLVM_DEBUG(dbgs() << "\n");
  for (Argument &Arg : F->args()) {
    unsigned ArgNo = Arg.getArgNo();
    // Test some basic arguments. 'onlyReadsMemory' is the most important one,
    // since we need to test the argument value in the specialization test.
    const DataLayout &DL = F->getParent()->getDataLayout();
    if (!Arg.getType()->isPointerTy()) {
      LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                        << "Arg missing required attributes\n");
      continue;
    }
    Type *Ty = inferPtrElementType(Arg);
    if (!Ty || !Ty->isSized() ||
        Arg.getDereferenceableBytes() < DL.getTypeStoreSize(Ty)) {
      LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                        << "Arg missing required attributes\n");
      continue;
    }
    if (!Arg.onlyReadsMemory()) {
      LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                        << "Arg missing required attributes\n");
      continue;
    }
    if (Arg.use_empty()) {
      LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                        << "Arg has no uses\n");
      continue;
    }
    int ArgUseCount = -1;
    (void)ArgUseCount;
    for (User *U0 : Arg.users()) {
      ArgUseCount++;
      // Look for a GEPI of the form GEPI(PointerOperand, 0, 1). We do this
      // to save compile-time. This could be generalized if it was found a
      // useful thing to do.
      auto GEPI = dyn_cast<GetElementPtrInst>(U0);
      if (!GEPI || GEPI->getPointerOperand() != &Arg || !GEPI->hasOneUse() ||
          GEPI->getNumIndices() != 2) {
        LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                          << "ArgUse(" << ArgUseCount << "): "
                          << "Missing minimal GEPI conditions\n");
        continue;
      }
      auto CIGEPI0 = dyn_cast<ConstantInt>(GEPI->getOperand(1));
      if (!CIGEPI0 || !CIGEPI0->isZero()) {
        LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                          << "ArgUse(" << ArgUseCount << "): "
                          << "First GEPI index not 0\n");
        continue;
      }
      auto CIGEPI1 = dyn_cast<ConstantInt>(GEPI->getOperand(2));
      if (!CIGEPI1 || !CIGEPI1->isOne()) {
        LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                          << "ArgUse(" << ArgUseCount << "): "
                          << "Second GEPI index not 1\n");
        continue;
      }
      auto LI = dyn_cast<LoadInst>(*GEPI->user_begin());
      if (!LI) {
        LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                          << "ArgUse(" << ArgUseCount << "): "
                          << "GEPI does not feed a LoadInst\n");
        continue;
      }
      // Create a vector of LoadInsts that load the potential trip count
      SmallVector<LoadInst *, 4> LoadVec;
      LoadVec.push_back(LI);
      for (User *U00 : LI->users()) {
        auto SI = dyn_cast<StoreInst>(U00);
        if (SI && SI->getValueOperand() == LI) {
          Value *V = SI->getPointerOperand();
          for (User *U01 : V->users())
            if (auto VLI = dyn_cast<LoadInst>(U01))
              LoadVec.push_back(VLI);
        }
      }
      // Look at the LoadInsts to find loops whose trip counts all can
      // be predicted to be the same value.
      int LoadUseCount = -1;
      (void)LoadUseCount;
      unsigned TripCountCount = 0;
      unsigned TCN = 0;
      while (!LoadVec.empty()) {
        LoadInst *LI = LoadVec.pop_back_val();
        for (User *U1 : LI->users()) {
          LoadUseCount++;
          // Right now, a SExtInst or ZExtInst followed by an increment
          // appears in the cases we care about. This code below could also
          // be generalized if it were found to be useful to do so.
          auto CsI = dyn_cast<CastInst>(U1);
          if (!CsI || !CsI->hasOneUse()) {
            LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                              << "ArgUse(" << ArgUseCount << "): "
                              << "LoadUse(" << LoadUseCount << "): "
                              << "Missing CastInst or more than "
                              << "one use\n");
            continue;
          }
          if (!isa<SExtInst>(CsI) && !isa<ZExtInst>(CsI)) {
            LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                              << "ArgUse(" << ArgUseCount << "): "
                              << "LoadUse(" << LoadUseCount << "): "
                              << "Not SExtInst or ZExtInst\n");
            continue;
          }
          Value *TCV = *CsI->user_begin();
          if (!match(TCV, m_Add(m_Specific(CsI), m_One()))) {
            LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                              << "ArgUse(" << ArgUseCount << "): "
                              << "LoadUse(" << LoadUseCount << "): "
                              << "Missing increment\n");
            continue;
          }
          // Expect to see many loops in the function for which we find the
          // same trip count based on this value.
          unsigned LocalTripCountCount = 0;
          unsigned TCNL = TripCount(TCV, &LocalTripCountCount);
          if (TCNL == 0 || TCN != 0 && TCN != TCNL) {
            LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                              << "ArgUse(" << ArgUseCount << "): "
                              << "LoadUse(" << LoadUseCount << "): "
                              << "Could not find matching trip count\n");
            continue;
          }
          LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                            << "ArgUse(" << ArgUseCount << "): "
                            << "LoadUse(" << LoadUseCount << "): "
                            << "Good partial result\n");
          TripCountCount += LocalTripCountCount;
          TCN = TCNL;
        }
      }
      // Check that we have seen enough loops with this trip count.
      if (TripCountCount < IPSpeCloningMinLoops) {
        LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                          << "ArgUse(" << ArgUseCount << "): "
                          << "Not enough loops " << TripCountCount
                          << " < " << IPSpeCloningMinLoops <<"\n");
        continue;
      }
      // We found it! Generate the condition test.
      *IC = CondTest(CI, ArgNo, GEPI, LI, TCN);
      *LIOut = LI;
      *TCNumber = TCN;
      LLVM_DEBUG(dbgs() << "MLSC: Arg(" << ArgNo << "): "
                        << "ArgUse(" << ArgUseCount << "): "
                        << "FOUND MLSC CANDIDATE\n");
      return true;
    }
  }
  return false;
}

//
// Clone 'F' and create a specialization test of the form:
//   if (Arg->Field == TCNumber)
//     foo(...);
//   else
//     foo.clone-index(...);
// Here 'IC' is the specialization test, 'LI' loads the trip count, and
// 'TCNumber' is the predicted trip count value.
//
static void createManyLoopSpecializationClones(Function *F,
                                               Value *IC,
                                               LoadInst *LI,
                                               unsigned TCNumber) {
  // Clone F.
  ValueToValueMapTy VMap;
  Function *NewF = IPCloneFunction(F, VMap);
  // Get the unique call site, and split the BasicBlock it is in so we can
  // put in the specialization test.
  CallInst *CI = uniqueCallSite(*F);
  assert(CI && "Expecting unique callsite for F");
  CallInst *NewCI = cast<CallInst>(CI->clone());
  setCalledFunction(NewCI, NewF);
  Instruction *TrueT = nullptr;
  Instruction *FalseT = nullptr;
  SplitBlockAndInsertIfThenElse(IC, CI, &TrueT, &FalseT);
  CI->moveBefore(TrueT);
  NewCI->insertBefore(FalseT);
  // If the cloned call has a return value, tie that return value and the
  // return value of the original together with a PHINode.
  if (!CI->getType()->isVoidTy()) {
    BasicBlock *FalseBB = FalseT->getParent();
    auto BI = cast<BranchInst>(FalseBB->getTerminator());
    assert(BI->isUnconditional());
    BasicBlock *TailBB = BI->getSuccessor(0);
    PHINode *RPHI =
        PHINode::Create(CI->getType(), 2, ".manyloops.phi", &TailBB->front());
    RPHI->addIncoming(NewCI, FalseBB);
    RPHI->setDebugLoc(CI->getDebugLoc());
    CI->replaceAllUsesWith(RPHI);
    RPHI->addIncoming(CI, CI->getParent());
  }
  // The original Function will be used for the specialized version. Replace
  // the tested expression by the predicted constant value and get rid of
  // the extraneous test in the original Function.
  auto CINew = ConstantInt::get(LI->getType(), TCNumber);
  LI->replaceAllUsesWith(CINew);
  auto GEPI = cast<GetElementPtrInst>(LI->getPointerOperand());
  LI->eraseFromParent();
  GEPI->eraseFromParent();
  // Indicate that the cloned Function is not a candidate for cloning itself.
  ManyLoopClones.insert(NewF);
}

// Main routine to analyze all calls and clone functions if profitable.
//
static bool analysisCallsCloneFunctions(
    Module &M, bool AfterInl, bool EnableDTrans, bool IFSwitchHeuristic,
    WholeProgramInfo *WPInfo, std::function<DominatorTree &(Function &)> *GetDT,
    std::function<BlockFrequencyInfo &(Function &)> *GetBFI,
    std::function<BranchProbabilityInfo &(Function &)> *GetBPI,
    std::function<TargetLibraryInfo &(Function &)> *GetTLI) {

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

  collectAbstractCallSites(M);

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
      // Extra data returned from RecProgressionClone testing
      int Start, Inc;
      unsigned ArgPos, Count;
      bool IsByRef, IsCyclic;
      Type *ArgType = nullptr;
      // Extra data returned from ManyLoopSpecializationClone testing
      Value *IC = nullptr;
      LoadInst *LI = nullptr;
      unsigned TCNumber = 0;
      if (EnableDTrans &&
          isRecProgressionCloneCandidate(F, true, &ArgPos, &Count, &Start, &Inc,
                                         &IsByRef, &ArgType, &IsCyclic)) {
        CloneType = RecProgressionClone;
        LLVM_DEBUG(dbgs() << "    Selected RecProgression cloning  "
                          << "\n");
        createRecProgressionClones(F, ArgPos, Count, Start, Inc, IsByRef,
                                   ArgType, IsCyclic);
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
          if (EnableManyRecCallsPredicateOpt) {
            LLVM_DEBUG(dbgs() << "    Selected many recursive calls "
                              << "predicate opt\n");
            PredicateOpt MRCPO(&F, GetDT, GetBFI, GetBPI, GetTLI);
            if (MRCPO.shouldAttemptPredicateOpt())
              MRCPO.doPredicateOpt();
          } else if (EnableManyRecCallsSplitting) {
            // This is needed to set "prefer-function-level-region" for
            // LoopOpt.
            PredicateOpt MRCPO(&F, GetDT, GetBFI, GetBPI, GetTLI);
            MRCPO.findSpine();
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
      } else if (EnableDTrans &&
          isManyLoopSpecializationCandidate(&F, &IC, &LI, &TCNumber)) {
        CloneType = ManyLoopSpecializationClone;
        LLVM_DEBUG(dbgs() << "    Selected ManyLoopSpecialization cloning  "
                          << "\n");
        createManyLoopSpecializationClones(&F, IC, LI, TCNumber);
        continue;
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
                                     &IsGenRecQualified, WPInfo)) {
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

    assert((!ForceOffCallbackCloning || !ForceOnCallbackCloning) &&
        "Not both ForceOffCallbackCloning and ForceOnCallbackCloning");
    bool AttemptCallbackCloning = !F.isVarArg() &&
        (ForceOnCallbackCloning || (!ForceOffCallbackCloning &&
        IFSwitchHeuristic && vpo::VPOAnalysisUtils::mayHaveOpenmpDirective(F)));
    LLVM_DEBUG({
      if (AttemptCallbackCloning)
        dbgs() << " Attempting callback cloning for " << F.getName() << "\n";
      else
        dbgs() << " Not attempting callback cloning for "
               << F.getName() << "\n";
    });
    cloneFunction(AttemptCallbackCloning);
  }

  if (AfterInl && EnableClonedFunctionArgsMerge)
    mergeArgs();

  LLVM_DEBUG(dbgs() << " Total clones:  " << NumIPCloned << "\n");

  if (NumIPCloned != 0)
    return true;

  return false;
}

static bool
runIPCloning(Module &M, bool AfterInl, bool EnableDTrans,
             WholeProgramInfo *WPInfo,
             std::function<DominatorTree &(Function &)> *GetDT,
             std::function<BlockFrequencyInfo &(Function &)> *GetBFI,
             std::function<BranchProbabilityInfo &(Function &)> *GetBPI,
             std::function<TargetLibraryInfo &(Function &)> *GetTLI) {
  bool Change = false;
  bool IFSwitchHeuristicOn = EnableDTrans || ForceIFSwitchHeuristic;
  bool EnableDTransOn = EnableDTrans || ForceEnableDTrans;
  Change = analysisCallsCloneFunctions(M, AfterInl, EnableDTransOn,
                                       IFSwitchHeuristicOn, WPInfo, GetDT,
                                       GetBFI, GetBPI, GetTLI);
  clearAllMaps();
  return Change;
}

IPCloningPass::IPCloningPass(bool AfterInl, bool EnableDTrans)
    : AfterInl(AfterInl), EnableDTrans(EnableDTrans) {}

PreservedAnalyses IPCloningPass::run(Module &M, ModuleAnalysisManager &AM) {

  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  std::function<DominatorTree &(Function &)> GetDT =
      [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };

  std::function<BlockFrequencyInfo &(Function &)> GetBFI =
      [&FAM](Function &F) -> BlockFrequencyInfo & {
    return FAM.getResult<BlockFrequencyAnalysis>(F);
  };

  std::function<BranchProbabilityInfo &(Function &)> GetBPI =
      [&FAM](Function &F) -> BranchProbabilityInfo & {
    return FAM.getResult<BranchProbabilityAnalysis>(F);
  };

  std::function<TargetLibraryInfo &(Function &)> GetTLI =
      [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };

  if (IPCloningAfterInl)
    AfterInl = true;

  if (!runIPCloning(M, AfterInl, EnableDTrans, &WPInfo, &GetDT, &GetBFI,
                    &GetBPI, &GetTLI))
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses();
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<AndersensAA>();
  return PA;
}
#endif // INTEL_FEATURE_SW_ADVANCED

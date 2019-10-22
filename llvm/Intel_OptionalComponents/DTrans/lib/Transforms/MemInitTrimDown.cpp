//===---------------- MemInitTrimDown.cpp - DTransMemInitTrimDownPass -----===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Initial Memory Allocation Trim Down
// optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/MemInitTrimDown.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"

#include "SOAToAOSClassInfo.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

using namespace llvm;

#define DTRANS_MEMINITTRIMDOWN "dtrans-meminittrimdown"

// This option is used for testing.
cl::opt<bool>
    DTransMemInitRecognizeAll("dtrans-meminit-recognize-all", cl::init(false),
                              cl::Hidden,
                              cl::desc("Recognize All member functions"));

namespace {

class DTransMemInitTrimDownWrapper : public ModulePass {
private:
  dtrans::MemInitTrimDownPass Impl;

public:
  static char ID;

  DTransMemInitTrimDownWrapper() : ModulePass(ID) {
    initializeDTransMemInitTrimDownWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    DTransAnalysisWrapper &DTAnalysisWrapper =
        getAnalysis<DTransAnalysisWrapper>();
    DTransAnalysisInfo &DTInfo = DTAnalysisWrapper.getDTransInfo(M);

    dtrans::MemInitDominatorTreeType GetDT =
        [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };
    auto GetTLI = [this](const Function &F) -> const TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };
    bool Changed =
        Impl.runImpl(M, DTInfo, GetTLI,
                     getAnalysis<WholeProgramWrapperPass>().getResult(), GetDT);
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<DTransAnalysisWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransMemInitTrimDownWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransMemInitTrimDownWrapper, "dtrans-meminittrimdown",
                      "DTrans Mem Init Trim Down", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransMemInitTrimDownWrapper, "dtrans-meminittrimdown",
                    "DTrans Mem Init Trim Down", false, false)

ModulePass *llvm::createDTransMemInitTrimDownWrapperPass() {
  return new DTransMemInitTrimDownWrapper();
}

namespace llvm {

namespace dtrans {

class MemInitClassInfo : public ClassInfo {
public:
  bool checkMemberFunctionCalls();
  bool checkHeuristics();
  bool isCtorCapacityArgPositionPair(std::pair<Function *, int32_t> &);
  bool isClassGetCapacityFunction(Function *);
  void trimDowmMemInit();

  // Iterator for capacity escape points.
  typedef EscapePointTy::const_iterator c_const_iterator;
  inline iterator_range<c_const_iterator> capacity_escape_points() {
    return make_range(CapacityEscapePoints.begin(), CapacityEscapePoints.end());
  }

  // Iterator for capacity values.
  typedef CapacityValuesTy::const_iterator cv_const_iterator;
  inline iterator_range<cv_const_iterator> capacity_values() {
    return make_range(CapacityValues.begin(), CapacityValues.end());
  }

  MemInitClassInfo(const DataLayout &DL, DTransAnalysisInfo &DTInfo,
                   MemGetTLITy GetTLI, MemInitDominatorTreeType GetDT,
                   MemInitCandidateInfo *MICInfo, int32_t FieldIdx,
                   bool RecognizeAll)
      : ClassInfo(DL, DTInfo, GetTLI, GetDT, MICInfo, FieldIdx, RecognizeAll){};

private:
  // GetCapacity function.
  Function *GetCapacityFunction = nullptr;

  // AppendElem function.
  Function *AppendElemFunction = nullptr;

  // Position of argument that the capacity value is passed to Ctor.
  int32_t CapacityArgPos = -1;

  // Value of capacity can be escaped through GetCapacity function call.
  // If return value of GetCapacity function call is passed to some other
  // function call, collect called function and the position of argument
  //  the value is passed for further analysis later.
  EscapePointTy CapacityEscapePoints;

  // Collection of capacity values that are passed to Ctor calls.
  CapacityValuesTy CapacityValues;

  // Set of Ctor calls in member functions of candidate struct.
  SmallPtrSet<CallBase *, 2> CtorCallsInStructMethods;
};

// Returns true if CtorArgPosPair is pair of Ctor and position of capacity that
// is passed as argument to the Ctor.
bool MemInitClassInfo::isCtorCapacityArgPositionPair(
    std::pair<Function *, int32_t> &CtorArgPosPair) {
  return (CtorArgPosPair.first == getCtorFunction() &&
          CtorArgPosPair.second == CapacityArgPos);
}

// Returns true if F is GetCapacityFunction.
bool MemInitClassInfo::isClassGetCapacityFunction(Function *F) {
  return (GetCapacityFunction == F);
}

// Analyze calls of vector class to prove that it is safe
// to trim down capacity value.
//  1. Limit how the member functions are called and their
//     functionalities.
//  2. Make sure Capacity value is modified only when AppendElem
//     is called.
//  3. Make sure capacity value is not escaped. If escaped, collect
//     all escape point for further analysis later.
//  4. Collect capacity values that are passed at callsites of Ctor
//     if possible.
//
bool MemInitClassInfo::checkMemberFunctionCalls() {

  // Collect escape points if value of capacity is escaped
  // through GetCapacity function. Return false if it is
  // unable to collect.
  auto CheckCapacityValueEscaped = [this]() {
    // Safe if GetCapacity function is not defined/used.
    if (!GetCapacityFunction)
      return true;
    // Check uses of capacity value.
    for (auto &U : GetCapacityFunction->uses()) {
      auto *GetCapacityCall = cast<CallBase>(U.getUser());
      for (auto &UU : GetCapacityCall->uses()) {
        auto *Call = dyn_cast<CallBase>(UU.getUser());
        // Only passing it as argument to some other call is allowed.
        if (!Call)
          return false;
        Function *CalledF = Call->getCalledFunction();
        // Can't analyze further if value of capacity is used
        // by indirect call.
        if (!CalledF)
          return false;
        auto CAI = Call->arg_begin();
        auto CAE = Call->arg_end();
        int32_t ArgPos = 0;
        // Find the position of argument that is passed as argument.
        for (; CAI != CAE; CAI++) {
          if (*CAI == GetCapacityCall) {
            DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
              dbgs() << "  Escape Point: " << CalledF->getName() << " Arg_"
                     << ArgPos << "\n";
            });
            CapacityEscapePoints.insert(std::make_pair(CalledF, ArgPos));
            // Not adding break here to continue collect if it is passed
            // as more than argument.
          }
          ArgPos++;
        }
      }
    }
    return true;
  };

  // Append "V" to CapacityValues if it is valid. For now,
  // we allow only constants and return value of other calls.
  auto VerifyCapacityValueAndAppend = [this](Value *V) {
    if (auto *C = dyn_cast<ConstantInt>(V)) {
      // Not valid to have capacity values that are less than
      // MinCapacityLimit.
      if (C->getLimitedValue() < MinCapacityLimit)
        return false;
    } else if (auto *Call = dyn_cast<CallBase>(V)) {
      if (!Call->getCalledFunction())
        return false;
    } else {
      return false;
    }
    CapacityValues.insert(V);
    return true;
  };

  // Try to find position of capacity value that is passed as
  // argument for Ctor calls.
  //
  // Ex:
  //     Derived_Ctor(DThis, d_capacity_v, ..) {
  //        ...
  //        Base_Ctor(BThis, false, d_capacity_v, );
  //     }
  //
  //     Base_Ctor(This, , b_capacity_v) {
  //        This->capacity = b_capacity_v;
  //        ...
  //     }
  // In the example, it tries to find argument position of d_capacity_v
  // for the given "b_capacity_v".
  //
  auto FindCapacityArgPos = [this](Value *ValOp) {
    auto *Arg = dyn_cast<Argument>(ValOp);
    if (!Arg)
      return false;
    int32_t ArgNo = Arg->getArgNo();
    Function *Caller = Arg->getParent();
    if (getCtorFunction() == Caller) {
      CapacityArgPos = ArgNo;
      return true;
    }
    // If Caller is not actual constructor, go one more level up to
    // avoid base class constructor call.
    if (!Caller->hasOneUse())
      return false;
    auto *CallerCall = dyn_cast<CallBase>(*Caller->user_begin());
    if (!CallerCall)
      return false;
    auto *SecondLevArg = dyn_cast<Argument>(CallerCall->getArgOperand(ArgNo));
    if (!SecondLevArg || SecondLevArg->getParent() != getCtorFunction())
      return false;
    CapacityArgPos = SecondLevArg->getArgNo();
    return true;
  };

  // Collect capacity values.
  auto CollectCapacityValues = [this, &VerifyCapacityValueAndAppend,
                                &FindCapacityArgPos]() {
    Value *ValOp = getCapacityInitInst()->getValueOperand();

    // If constant value saved to capacity value, there is no
    // need to find CapacityArgPos.
    if (isa<Constant>(ValOp))
      return VerifyCapacityValueAndAppend(ValOp);

    if (!FindCapacityArgPos(ValOp))
      return false;

    // Collect capacity values using CapacityArgPos and CtorFunction.
    assert(CapacityArgPos != -1 && "Expected valid CapacityArgPos");
    for (auto *CtorCall : CtorCallsInStructMethods) {
      Value *CVal = CtorCall->getArgOperand(CapacityArgPos);
      if (!VerifyCapacityValueAndAppend(CVal))
        return false;
    }
    return true;
  };

  int32_t NumCopyConstructor = 0;
  int32_t NumAppendElem = 0;
  int32_t NumResize = 0;
  int32_t NumDestructors = 0;
  int32_t NumDestructorWrappers = 0;
  int32_t NumGetCapacity = 0;
  int32_t NumGetSize = 0;
  // Verify that candidate vector has at least some minimum set
  // of functionality like AppendElem, Resize etc.
  for (auto *Fn : field_member_functions()) {
    FunctionKind FKind = getFinalFuncKind(Fn);
    switch (FKind) {
    case CopyConstructor:
      NumCopyConstructor++;
      break;
    case AppendElem:
      NumAppendElem++;
      AppendElemFunction = Fn;
      break;
    case Resize:
      // Resize is the only function that changes the value of
      // capacity other than constructor. We already checked that
      // Resize is called from AppendElem before adding new element.
      // Make sure there are no other calls to Resize.
      if (!Fn->hasOneUse()) {
        DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
          dbgs() << "  Failed: Resize has more than one use.\n";
        });
        return false;
      }
      NumResize++;
      break;
    case Destructor:
      NumDestructors++;
      break;
    case DestructorWrapper:
      NumDestructorWrappers++;
      break;
    case GetCapacity:
      if (Fn->hasAddressTaken()) {
        DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                        { dbgs() << "  Failed: GetCapacity address taken\n"; });
        return false;
      }
      NumGetCapacity++;
      GetCapacityFunction = Fn;
      break;
    case GetSize:
      NumGetSize++;
      break;
    // Constructor is already verified. Not doing any checks
    // for SetElem/GetElem.
    default:
      break;
    }
  }
  // Not allowed to have more than one member function for any
  // functionality except SetElem/GetElem.
  // CopyConstructor/GetCapacity/GetSize are not mandatory functionality.
  if (NumCopyConstructor > 1 || NumAppendElem != 1 || NumResize != 1 ||
      NumDestructors != 1 || NumGetCapacity > 1 || NumGetSize > 1 ||
      NumDestructorWrappers > 1) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
      dbgs() << "  Failed: Missing required member functions.\n";
    });
    return false;
  }
  assert(AppendElemFunction && "Expected AppendElem function");
  bool IsCapacityProp = isa<Constant>(getCapacityInitInst()->getValueOperand());
  // Collect constructor calls in struct methods.
  for (auto &U : getCtorFunction()->uses()) {
    auto *Call = cast<CallBase>(U.getUser());
    Function *Caller = Call->getCaller();
    if (!isCandidateStructMethod(Caller)) {
      // Make sure Ctor is called from methods of candidate struct only
      // if capacity value is propagated to callee (i.e constructor).
      if (IsCapacityProp) {
        DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
          dbgs() << "  Failed: Ctor call not in struct methods.\n";
        });
        return false;
      } else {
        DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
          dbgs() << "  Ignored Ctor call: Not in struct methods.\n";
        });
        continue;
      }
    }
    CtorCallsInStructMethods.insert(Call);
  }

  if (!CheckCapacityValueEscaped()) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                    { dbgs() << "  Failed: Capacity value escaped.\n"; });
    return false;
  }

  if (!CollectCapacityValues()) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                    { dbgs() << "  Failed: Capacity value Collection.\n"; });
    return false;
  }
  return true;
}

// Apply some heuristics to trigger the transformation.
//
bool MemInitClassInfo::checkHeuristics(void) {
  // Returns true if Fn is potential a copy-constructor.
  // Type of "this" pointer is checked at callsite.
  auto IsPotentialCandidateCopyCtor = [](Function *Fn) {
    auto *FTy = Fn->getFunctionType();
    if (Fn->arg_size() == 2 && FTy->getParamType(0) == FTy->getParamType(1))
      return true;
    return false;
  };

  // No capacity value should exceed MaxCapacityLimit that is currently
  // set to 4. Better not to trigger the transformation if user uses
  // big capacity numbers.
  for (auto *CVal : CapacityValues) {
    // Later, we will prove that non-constant capacity values are return
    // values of GetCapacityFunction of other vector classes that are
    // selected for the same transformation.
    if (auto *C = dyn_cast<ConstantInt>(CVal)) {
      int32_t IntVal = C->getLimitedValue();
      if (IntVal > MaxCapacityLimit)
        return false;
    }
  }

  // AppendElem is the only function that adds new elements to vector.
  // Just make sure AppendElem is not called too frequently.
  int32_t NumCalled = 0;
  for (auto &U : AppendElemFunction->uses()) {
    Function *Caller = cast<CallBase>(U.getUser())->getCaller();
    if (!isCandidateStructMethod(Caller)) {
      // Ignore AppendElem calls that are not in member functions of
      // candidate struct.
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
        dbgs() << "  Ignored AppendElem call for heuristics.\n";
      });
      continue;
    }
    // If AppendElem is called from Copy-Constructor, probably it
    // is copying elements from one array to another (i.e not adding
    // new elements). Don't consider this as callsite of AppendElem.
    if (IsPotentialCandidateCopyCtor(Caller))
      continue;
    NumCalled++;
  }
  // Allow only one callsite that can potentially add new elements.
  // TODO: This heuristic can be extended by analyzing callsite
  // of AppendElem's caller to conclude AppendElem is not called
  // frequently at runtime.
  if (NumCalled > 1)
    return false;
  return true;
}

// Trim down capacity values either in CapacityInitInst instruction
// (constant) or at Ctor callsite (non-constant).
void MemInitClassInfo::trimDowmMemInit() {
  StoreInst *CapacityIInst = getCapacityInitInst();
  Value *ValOp = CapacityIInst->getValueOperand();

  // Replace capacity value in StoreInst if it is constant.
  // Ex:
  //  Before:
  //    store i32 4, i32* %7
  //
  //  After:
  //    store i32 1, i32* %7
  //
  if (isa<Constant>(ValOp)) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                    { dbgs() << "  Before: " << *CapacityIInst << "\n"; });
    Value *NewVal = ConstantInt::get(ValOp->getType(), NewCapacityValue);
    CapacityIInst->setOperand(0, NewVal);
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                    { dbgs() << "  After: " << *CapacityIInst << "\n"; });

    // Compute new array size.
    int64_t NewSize = NewCapacityValue * getElemTySize();
    // We already proved that memory is allocated for "capacity" number
    // of elements. Since we know the value of "capacity" now, just
    // set new constant array size in allocation statement.
    //
    //  Before:
    //       %1 = tail call i8* malloc(i64 %mul)
    //  After:
    //       %1 = tail call i8* malloc(i64 16)
    //
    assert(getAllocsInCtorSize() != 0 && "Expected allocation in Ctor");
    for (auto &AllocP : allocs_in_ctor()) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                      { dbgs() << "  Before: " << *AllocP.first << "\n"; });
      Value *AllocSize = AllocP.first->getOperand(AllocP.second);
      NewVal = ConstantInt::get(AllocSize->getType(), NewSize);
      AllocP.first->replaceUsesOfWith(AllocSize, NewVal);
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                      { dbgs() << "  After: " << *AllocP.first << "\n"; });
    }
    // Fix size argument of memset.
    //  Before:
    //    call void @llvm.memset(i8* align 8 %22, i8 0, i64 %28, i1 false)
    //  After:
    //    call void @llvm.memset(i8* align 8 %22, i8 0, i64 16, i1 false)
    CallBase *CtorMemsetI = getMemsetInCtor();
    assert(CtorMemsetI && "Expected valid Memset in Ctor");
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                    { dbgs() << "  Before: " << *CtorMemsetI << "\n"; });
    Value *MemsetSize = CtorMemsetI->getOperand(2);
    NewVal = ConstantInt::get(MemsetSize->getType(), NewSize);
    CtorMemsetI->replaceUsesOfWith(MemsetSize, NewVal);
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                    { dbgs() << "  After: " << *CtorMemsetI << "\n"; });
    return;
  }
  assert(isa<Argument>(ValOp) && "Expected Argument");

  for (auto *Call : CtorCallsInStructMethods) {
    Value *CVal = Call->getArgOperand(CapacityArgPos);
    // No Need to do anything if capacity value is passed as non-constant
    // since we already proved that it is return value of GetCapacity of
    // other vector class.
    // Ex:
    //  Before:
    //   @Ctor(%"C"* %20, i32 4, i1 zeroext true, %"M"* %21)
    //
    //  After:
    //   @Ctor(%"C"* %20, i32 1, i1 zeroext true, %"M"* %21)
    //
    if (isa<Constant>(CVal)) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                      { dbgs() << "  Before: " << *Call << "\n"; });
      Value *NewVal = ConstantInt::get(CVal->getType(), NewCapacityValue);
      Call->setArgOperand(CapacityArgPos, NewVal);
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                      { dbgs() << "  After: " << *Call << "\n"; });
    }
  }
}

class MemInitTrimDownImpl {

public:
  MemInitTrimDownImpl(Module &M, const DataLayout &DL,
                      DTransAnalysisInfo &DTInfo, MemGetTLITy GetTLI,
                      MemInitDominatorTreeType GetDT)
      : M(M), DL(DL), DTInfo(DTInfo), GetTLI(GetTLI), GetDT(GetDT){};

  ~MemInitTrimDownImpl() {
    for (auto *CInfo : Candidates)
      delete CInfo;
    for (auto *CInfo : ClassInfoSet)
      delete CInfo;
  }
  bool run(void);

private:
  Module &M;
  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  MemGetTLITy GetTLI;
  MemInitDominatorTreeType GetDT;

  constexpr static int MaxNumCandidates = 1;
  SmallVector<MemInitCandidateInfo *, MaxNumCandidates> Candidates;

  // Collection of ClassInfo. It will used for more analysis and
  // transformation.
  SmallPtrSet<MemInitClassInfo *, 4> ClassInfoSet;

  bool isEscapePointOkay(std::pair<Function *, int32_t> &);
  bool isAnyClassGetCapacityFunction(Function *F);
  bool gatherCandidateInfo(void);
  bool analyzeCandidate(MemInitCandidateInfo *);
  bool verifyFinalSafetyChecks(void);
  void transformMemInit(void);
};

// Transformation for all vector classes in ClassInfoSet.
void MemInitTrimDownImpl::transformMemInit(void) {
  DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                  { dbgs() << "  Applying trim down transformations ...\n"; });
  assert(ClassInfoSet.size() && "Expected atleast one ClassInfo element");
  for (auto *ClassI : ClassInfoSet)
    ClassI->trimDowmMemInit();
}

// Return true if EP is just position of capacity value that is passed to Ctor
// of someother class that is also selected for the transformation.
bool MemInitTrimDownImpl::isEscapePointOkay(
    std::pair<Function *, int32_t> &EP) {
  for (auto *ClassI : ClassInfoSet)
    if (ClassI->isCtorCapacityArgPositionPair(EP))
      return true;
  return false;
}

// Returns true if F is a GetCapacity function of any class in ClassInfoSet.
bool MemInitTrimDownImpl::isAnyClassGetCapacityFunction(Function *F) {
  for (auto *ClassI : ClassInfoSet)
    if (ClassI->isClassGetCapacityFunction(F))
      return true;
  return false;
}

// Verify final safety checks that are across vector classes.
// Entire transformation is disabled if any vector class is
// failed with these checks.
bool MemInitTrimDownImpl::verifyFinalSafetyChecks(void) {

  for (auto *ClassI : ClassInfoSet) {
    // Check all escaped capacity values are consumed by other
    // vector classes that are also selected for the transformation.
    for (auto EP : ClassI->capacity_escape_points()) {
      if (!isEscapePointOkay(EP)) {
        DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                        { dbgs() << "  Failed: Escape point not okay\n"; });
        return false;
      }
    }
    // Make sure all non-constant capacity values that are passed as
    // arguments for Ctor calls are capacity values of other vector
    // classes that selected for the transformation.
    for (auto *CVal : ClassI->capacity_values()) {
      if (isa<Constant>(CVal))
        continue;
      // Already verified that it is direct call.
      auto *CFn = cast<CallBase>(CVal)->getCalledFunction();
      if (!isAnyClassGetCapacityFunction(CFn)) {
        DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                        { dbgs() << "  Failed: Unknown Capacity value\n"; });
        return false;
      }
    }
  }
  return true;
}

// Analyze functionality of each member function of candidate
// field classes to prove that the classes are vector classes.
bool MemInitTrimDownImpl::analyzeCandidate(MemInitCandidateInfo *Cand) {
  for (auto Loc : Cand->candidate_fields()) {
    std::unique_ptr<MemInitClassInfo> ClassI(new MemInitClassInfo(
        DL, DTInfo, GetTLI, GetDT, Cand, Loc, DTransMemInitRecognizeAll));
    if (!ClassI->analyzeClassFunctions())
      return false;
    // Continue checking remaining candidate fields if
    // DTransMemInitRecognizeAll is true.
    if (DTransMemInitRecognizeAll)
      continue;
    if (!ClassI->checkMemberFunctionCalls())
      return false;
    if (!ClassI->checkHeuristics()) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                      { dbgs() << "  Failed: Heuristics\n"; });
      return false;
    }
    ClassInfoSet.insert(ClassI.release());
  }
  // Skip further analysis if DTransMemInitRecognizeAll is true.
  if (DTransMemInitRecognizeAll)
    return false;
  return true;
}

bool MemInitTrimDownImpl::gatherCandidateInfo(void) {

  DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
    dbgs() << "MemInitTrimDown transformation:";
    dbgs() << "\n";
  });
  // TODO: Consider only SOAToAOS candidates for MemInitTrimDown.
  for (TypeInfo *TI : DTInfo.type_info_entries()) {
    std::unique_ptr<MemInitCandidateInfo> CInfo(new MemInitCandidateInfo());

    auto *StInfo = dyn_cast<StructInfo>(TI);
    if (!StInfo)
      continue;

    if (!CInfo->isCandidateType(TI->getLLVMType()))
      continue;

    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
      dbgs() << "  Considering candidate: ";
      TI->getLLVMType()->print(dbgs(), true, true);
      dbgs() << "\n";
    });

    if (DTInfo.testSafetyData(StInfo, dtrans::DT_MemInitTrimDown)) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
        dbgs() << "  Failed: safety test for candidate struct.\n";
      });
      continue;
    }

    // TODO: Check SafetyData for candidate field array structs also.

    if (!CInfo->collectMemberFunctions(M)) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                      { dbgs() << "  Failed: member function collection.\n"; });
      continue;
    }

    if (Candidates.size() >= MaxNumCandidates) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
        dbgs() << "  Failed: Exceeding maximum candidate limit.\n";
      });
      return false;
    }
    Candidates.push_back(CInfo.release());
  }
  if (Candidates.empty()) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                    { dbgs() << "  Failed: No candidates found.\n"; });
    return false;
  }
  DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
    dbgs() << "  Possible candidate structs: \n";
    for (auto *CInfo : Candidates)
      CInfo->printCandidateInfo();
  });
  return true;
}

bool MemInitTrimDownImpl::run(void) {

  if (!gatherCandidateInfo())
    return false;

  // Analyze member functions of candidate classes to prove
  // that functionality match with usual vector class.
  SmallVector<MemInitCandidateInfo *, MaxNumCandidates> ValidCandidates;
  for (auto *Cand : Candidates)
    if (analyzeCandidate(Cand))
      ValidCandidates.push_back(Cand);
  std::swap(Candidates, ValidCandidates);

  if (Candidates.empty()) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
      dbgs() << "  Failed: No candidates after functionality analysis.\n";
    });
    return false;
  }

  if (!verifyFinalSafetyChecks()) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                    { dbgs() << "  Failed: Final safety checks.\n"; });
    return false;
  }
  transformMemInit();
  return false;
}

bool MemInitTrimDownPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                                  MemGetTLITy GetTLI, WholeProgramInfo &WPInfo,
                                  dtrans::MemInitDominatorTreeType &GetDT) {

  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  auto &DL = M.getDataLayout();

  MemInitTrimDownImpl MemInitTrimDownI(M, DL, DTInfo, GetTLI, GetDT);
  return MemInitTrimDownI.run();
}

PreservedAnalyses MemInitTrimDownPass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  MemInitDominatorTreeType GetDT = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function*>(&F)));
  };

  if (!runImpl(M, DTransInfo, GetTLI, WPInfo, GetDT))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<DTransAnalysis>();
  return PA;
}

} // namespace dtrans

} // namespace llvm

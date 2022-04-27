//===- WGLoopBoundaries.cpp - Compute workgroup loop boundaries -*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/WGLoopBoundaries.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/DPCPPStatistic.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataStatsAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/WGBoundDecoder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace DPCPPKernelCompilationUtils;

#define DEBUG_TYPE "dpcpp-kernel-wg-loop-bound"

namespace {

class WGLoopBoundariesImpl {
public:
  explicit WGLoopBoundariesImpl(Module &M, const RuntimeService *RTService)
      : M(M), RTService(RTService),
        DPCPP_STAT_INIT(CreatedEarlyExit,
                        "one if early exit (or late start) was done for the "
                        "kernel. Value is never greater for one, even if "
                        "early-exit done for several dimensions.",
                        KernelStats) {}

  bool run();
  bool runOnFunction(Function &F);

private:
  /// Description of boundary early exit.
  struct TIDDesc {
    Value *Bound;      // actual bound
    unsigned Dim;      // dimension of boundary
    bool IsUpperBound; // true upper bound, false lower bound
    bool ContainsVal;  // true inclusive, false exclusive
    bool IsSigned;     // true bound is signed comparision, false not
    bool IsGID;        // true bound is on global id, false on local id
  };

  /// Description of uniform early exit.
  struct UniformDesc {
    Value *Cond;     // condition (an i1 value)
    bool ExitOnTrue; // if true exit when condition is set.
  };

  using VVec = SmallVector<Value *, 4>;
  using IVec = SmallVector<Instruction *, 4>;
  using VMap = DenseMap<Value *, Value *>;

  /// Module being processed.
  Module &M;
  /// Function being processed.
  Function *F;

  LLVMContext *Ctx;
  const RuntimeService *RTService;

  /// size_t type.
  Type *IndTy;
  /// size_t one constant.
  Constant *ConstOne;
  // size_t zero constant.
  Constant *ConstZero;
  /// Number of WG dimensions.
  unsigned NumDim;
  /// local_id lower bounds per dimension.
  VVec LowerBounds;
  /// local_size lower bounds per dimension.
  IVec LocalSizes;
  /// base_global_ids per dimension.
  IVec BaseGIDs;
  /// loop_size per dimension (upper_bound - lower_bound)
  IVec LoopSizes;
  /// Map get***id calls to their dimension and whether they are global/local.
  DenseMap<Value *, std::pair<unsigned, bool>> TIDs;
  /// Map instruction to whether they are uniform or not.
  DenseMap<Value *, bool> Uni;
  /// Vector boundary descriptions.
  using TIDDescVec = SmallVector<TIDDesc, 4>;
  TIDDescVec TIDDescs;
  /// Vector of uniform early exit descriptions.
  SmallVector<UniformDesc, 4> UniDescs;
  /// Indicate whether there are calls to get***id with non-constant argument.
  bool HasVariableTid;
  /// Contains calls to get***id with variable argument.
  SmallPtrSet<CallInst *, 2> VariableTIDCalls;
  /// The dim's entry holds the get***id of dimension dim.
  SmallVector<SmallVector<CallInst *, 4>, 4> TIDByDim;
  /// Holds instruction marked for removal.
  SmallPtrSet<Instruction *, 8> ToRemove;
  /// Users of atomic/pipe functions.
  FuncSet WIUniqueFuncUsers;
  /// True iff upper bound was set to be inclusive.
  bool RightBoundInc;
  /// True if the pattern is (a - id) < b.
  bool ReverseLowerUpperBound = false;

  /// Statistics
  DPCPPStatistic::ActiveStatsT KernelStats;
  /// Set to 1 if early exit (or late start) was done for this kernel. This
  /// counter is only 0 or 1, even if early-exit was done for several conditions
  /// and/or dimension.
  DPCPPStatistic CreatedEarlyExit;

  /// Collect kernels MaxD WG loop boundaries must be always created for.
  void collectWIUniqueFuncUsers();

  /// In case entry block branch is an early exit branch, remove the branch and
  /// create early exit descriptions.
  /// \returns true iff the entry block branch is an early exit branch.
  bool findAndCollapseEarlyExit();

  /// Handle the case where TidInst has cmp-select boundary.
  /// \param TidInst tid generator to check.
  /// \returns true iff cmp-select boundary was found.
  bool handleCmpSelectBoundary(Instruction *TidInst);

  /// Handle the case where TidInst has min/max boundary.
  /// \param TidInst tid generator to check.
  /// \returns true iff min/max boundary was found.
  bool handleBuiltinBoundMinMax(Instruction *TidInst);

  /// Check if there is cmp-select or min/max early exit pattern and handle it.
  /// \returns true iff early exit pattern found.
  bool findAndHandleTIDMinMaxBound();

  /// Returns true if BB contains instruction with side effect.
  /// \param BB basic block to check.
  bool hasSideEffectInst(BasicBlock *BB);

  /// Returns true if the block lead unconditionally to return instruction with
  /// no side effect instructions.
  /// \param BB basic block to check.
  bool isEarlyExitSucc(BasicBlock *BB);

  /// Check if the branch is an early exit pattern. Fills class members with
  /// early exit description if so.
  /// \param Br branch to analyze.
  /// \param EETrueSide indicate whether early exit occurs if cmp is true.
  /// \returns true iff the branch is early exit instruction.
  bool isEarlyExitBranch(BranchInst *Br, bool EETrueSide);

  /// Returns true if the value is uniform across all work items.
  bool isUniform(Value *V);

  /// Returns true if all operands are uniform.
  /// \param I instruction to check.
  bool isUniformByOps(Instruction *I);

  /// Updates internal data structures with the get***id call.
  /// \param CI get***id call to process.
  /// \param IsGID true iff call is get_global_id.
  void processTIDCall(CallInst *CI, bool IsGID);

  /// Updates data structures with get***id data.
  void collectTIDData();

  /// Root is and/or instruction. If it is recursive and/or of icmp and uniform
  /// conditions into \p Compares, \p UniformConds and returns true.
  /// \param Compares vector of compares to fill.
  /// \param UniformConds vector uniform conditions to fill.
  /// \param Root original and/or to traceback.
  /// \returns true iff this recursive and/or of icmp and uniform conditions.
  bool collectCond(SmallVector<ICmpInst *, 4> &Compares, IVec &UniformConds,
                   Instruction *Root);

  /// Collect tid calls and check uniformity of instructions in the input block.
  /// \param BB basic block to check.
  void collectBlockData(BasicBlock *BB);

  /// Check if the input cmp instruction is supported boundary compare if so
  /// fills description of the boundary compare into EEVec.
  /// \param Cmp compare instruction to check.
  /// \param Bound the early exit boundaries.
  /// \param Tid the get***id call.
  /// \param EETrueSide indicate if early exit occurs if Cmp is true.
  /// \param EEVec vector early exit description to fill.
  /// \returns true iff Cmp is supported boundary compare.
  bool obtainBoundaryEE(ICmpInst *Cmp, Value **Bound, Value *Tid,
                        bool EETrueSide, TIDDescVec &EEVec);

  /// Returns loop boundaries function declaration with the original function
  /// arguments.
  Function *createLoopBoundariesFunctionDecl();

  /// Recover all values in roots and instruction leading to them from F into
  /// BasicBlock BB in NewF. Updates ValueMap on the way.
  /// \param ValueMap maps values from F to their clone in NewF.
  /// \param Roots original roots to recover.
  /// \param BB basic block to put instructions in.
  /// \param NewF new function.
  void recoverInstructions(VMap &ValueMap, VVec &Roots, BasicBlock *BB,
                           Function *NewF);

  /// Traces back the two input values if one is tid dependent and the other is
  /// uniform, assuming the two are compared.
  /// \param V1 first input value.
  /// \param V2 second input value.
  /// \param IsCmpSigned is this is a signed comparision.
  /// \param Loc place to put instructions to correct the bound.
  ///        IMPORTANT - upon a cmp early exit the localtion should be the cmp
  ///        instruction.
  /// \param Bound will hold the boundary values in case of success.
  /// \param Tid will hold the get***id in case of success.
  /// \returns true iff succeeded to trace back bound.
  bool traceBackBound(Value *V1, Value *V2, bool IsCmpSigned, Instruction *Loc,
                      Value **Bound, Value *&Tid);

  /// Serves as easier interface for traceBackBound for tracking compare
  /// instruction operands.
  /// \param Cmp compare instructions to inspect.
  /// \param Bound will hold the boundary values in case of success.
  /// \param Tid will hold the get***id in case of success.
  bool traceBackCmp(ICmpInst *Cmp, Value **Bound, Value *&Tid);

  /// Serves as easier interface for traceBackBound for tracking min/max builtin
  /// operands.
  /// \param CI min/max builtin to inspect.
  /// \param Bound will hold the boundary value in case of success.
  /// \param Tid will hold the get***id in case of success.
  bool traceBackMinMaxCall(CallInst *CI, Value **Bound, Value *&Tid);

  /// Updates the internal data members with cmp-select boundary.
  /// \param Cmp compare for which cmp-select pattern was found.
  /// \param Bound the early exit boundary.
  /// \param Tid the get***id call.
  /// \param IsSameOrder true iff the select and cmp agree on operands order.
  /// \returns true if cmp-select boundary pattern was found.
  bool obtainBoundaryCmpSelect(ICmpInst *Cmp, Value *Bound, Value *Tid,
                               bool IsSameOrder);

  /// Helper function checks that cmp predicate is supported.
  /// \param P predicate to inspect.
  /// \returns true iff compare relational predicate is supported.
  bool isSupportedRelationalComparePredicate(CmpInst::Predicate P);

  /// Helper function checks that cmp predicate is <,<=.
  /// \param P predicate to inspect.
  /// \returns true iff compare predicate is supported <,<=.
  bool isComparePredicateLower(CmpInst::Predicate P);

  /// Helper function checks that cmp predicate is <=,>=.
  /// \param P predicate to inspect.
  /// \returns true iff compare predicate is supported <=,>=.
  bool isComparePredicateInclusive(CmpInst::Predicate P);

  /// Fills LoopSizes, LowerBounds, LocalSize, BaseGIDs with initial values in
  /// case no boundary early exit.
  /// \param BB basic block to put instructions.
  void fillInitialBoundaries(BasicBlock *BB);

  /// Recover boundary values, and uniform early exit conditions and the
  /// instructions leading to them in BasicBlock BB. Update ValueMap on the way.
  /// \param ValueMap maps values from F to their clone in NewF.
  /// \param BB basic block to put instructions in.
  void recoverBoundInstructions(VMap &ValueMap, BasicBlock *BB);

  /// Safely corrects the boundary value in case bound is on local_id, or is
  /// inclusive/exclusive when it shouldn't.
  /// \param TD maps values from F to their clone in NewF.
  /// \param BB basic block to put instructions in.
  Value *correctBound(TIDDesc &TD, BasicBlock *BB, Value *Bound);

  /// Create the loop boundaries function for the current kernel.
  void createWGLoopBoundariesFunction();

  /// Run through all descriptions, and update LoopSizes, LowerBounds according
  /// to the boundary descriptions.
  void obtainEEBoundaries(BasicBlock *BB, VMap &ValueMap);

  /// Returns the uniform early exit condition.
  Value *obtainUniformCond(BasicBlock *BB, VMap &ValueMap);

  /// Replaces tid calls with given value.
  /// \param IsGID true get_global_id, false get_local_id.
  /// \param Dim dimension argument.
  /// \param ToRep Value to replace tid with.
  void replaceTidWithBound(bool IsGID, unsigned Dim, Value *ToRep);

  /// Print data collected by the pass on the given module.
  /// \param OS stream to print the inifo regarding the module into.
  void print(raw_ostream &OS, StringRef FName) const;

  /// Sign extends the bound in case a trunc instruction was called over the
  /// result and the comparison is signed.
  /// \param \IsCmpSigned is the comparison signed.
  /// \param \Loc the location to insert the new instructions.
  /// \param \Bound the current bound - will be updated by the function.
  /// \param \LeftBound the left bound of the early exit.
  /// \param \OriginalTy original type of the bound (nullptr if no trunc
  ///        instruction was performed).
  /// \param \NewTy the type to which we are doing the sext.
  /// \param \Inst the instruction used to compute the right bound.
  /// \returns true if right bound is created and false if failed to create an
  ///          early exit.
  bool createRightBound(bool IsCmpSigned, Instruction *Loc, Value **Bound,
                        Value *LeftBound, Type *OriginalTy, Type *NewTy,
                        Instruction::BinaryOps Inst);

  /// Check if given the left bound and the comparison type, early exit can be
  /// created.
  /// \param ComparisonTy data type of the comparison.
  /// \param LeftBound the left bound of the early exit.
  /// \returns true if left bound fits and false otherwise.
  bool doesLeftBoundFit(Type *ComparisonTy, Value *LeftBound);
};

} // namespace

bool WGLoopBoundariesImpl::run() {
  bool Changed = false;

  auto Kernels = DPCPPKernelMetadataAPI::KernelList(&M);
  if (Kernels.empty())
    return Changed;

  Ctx = &M.getContext();
  NumDim = RTService->getNumJitDimensions();
  IndTy = DPCPPKernelLoopUtils::getIndTy(&M);
  ConstOne = ConstantInt::get(IndTy, 1);
  ConstZero = ConstantInt::get(IndTy, 0);

  // Collect all users of atomic/pipe builtins.
  collectWIUniqueFuncUsers();

  // Get the kernels using the barrier for work group loops.
  for (auto *Kernel : Kernels) {
    auto KIMD = DPCPPKernelMetadataAPI::KernelInternalMetadataAPI(Kernel);
    // No need to check if NoBarrierPath value exists, it is guaranteed that
    // KernelAnalysisPass run before WGLoopBoundariesPass.
    if (KIMD.NoBarrierPath.get()) {
      // Kernel that should not be handled in Barrier pass.
      Changed |= runOnFunction(*Kernel);
    }
  }

  return Changed;
}

void WGLoopBoundariesImpl::collectWIUniqueFuncUsers() {
  // First obtain all the atomic/pipe functions in the module.
  FuncSet WIUniqueFuncs;
  for (Function &F : M) {
    StringRef Name = F.getName();
    if (RTService->isAtomicBuiltin(Name) || isWorkItemPipeBuiltin(Name))
      WIUniqueFuncs.insert(&F);
  }

  // Obtain all the recursive users of the atomic/pipe functions.
  if (!WIUniqueFuncs.empty())
    DPCPPKernelLoopUtils::fillFuncUsersSet(WIUniqueFuncs, WIUniqueFuncUsers);
}

bool WGLoopBoundariesImpl::isUniform(Value *V) {
  auto *I = dyn_cast<Instruction>(V);
  if (!I)
    return true;
  assert(Uni.count(I) && "should not query new instructions");
  return Uni[I];
}

bool WGLoopBoundariesImpl::isUniformByOps(Instruction *I) {
  return llvm::all_of(I->operands(),
                      [this](Value *Op) { return isUniform(Op); });
}

void WGLoopBoundariesImpl::collectBlockData(BasicBlock *BB) {
  // Run over all instructions in the block (excluding the terminator)
  for (auto It = BB->begin(), E = std::prev(BB->end()); It != E; ++It) {
    Instruction *I = &*It;
    if (CallInst *CI = dyn_cast<CallInst>(I)) {
      Function *Callee = CI->getCalledFunction();
      // If the function is defined in the module, it is not uniform.
      // If the function is ID generator, it is not uniform.
      if (!Callee || !Callee->isDeclaration() ||
          VariableTIDCalls.contains(CI) ||
          isWorkGroupDivergent(Callee->getName()) || TIDs.count(CI)) {
        Uni[I] = false;
        LLVM_DEBUG(dbgs() << "Callee " << Callee->getName()
                          << " is not uniform\n");
        continue;
      }

      // Getting this is a regular builtin uniform if all operands are uniform.
      Uni[I] = isUniformByOps(I);
      LLVM_DEBUG(dbgs() << "Callee " << Callee->getName()
                        << " isUniformByOps: " << Uni[I] << "\n");
    } else if (isa<AllocaInst>(I)) {
      Uni[I] = false;
      LLVM_DEBUG(dbgs() << "Alloca " << *I << " is not uniform\n");
    } else {
      Uni[I] = isUniformByOps(I);
      LLVM_DEBUG(dbgs() << "Instruction " << *I << " isUniformByOps: " << Uni[I]
                        << "\n");
    }
  }
}

void WGLoopBoundariesImpl::processTIDCall(CallInst *CI, bool IsGID) {
  assert(CI->getType() == IndTy && "mismatch get***id type");
  auto *DimC = dyn_cast<ConstantInt>(CI->getArgOperand(0));
  if (!DimC) {
    HasVariableTid = true;
    VariableTIDCalls.insert(CI);
    return;
  }
  unsigned Dim = static_cast<unsigned>(DimC->getValue().getZExtValue());
  assert(Dim < MAX_WORK_DIM && "get***id with dim > (MAX_WORK_DIM-1)");
  // All dimension above NumDim are uniform so we don't need to add them.
  if (Dim < NumDim) {
    TIDs[CI] = {Dim, IsGID};
    TIDByDim[Dim].push_back(CI);
  }
}

void WGLoopBoundariesImpl::collectTIDData() {
  // First clear the tids data structures.
  HasVariableTid = false;
  TIDs.clear();
  TIDByDim.clear();
  TIDByDim.resize(NumDim); // allocate vector for each dimension

  auto ProcessTIDCalls = [this](bool IsGID) {
    std::string TIDName = IsGID ? mangledGetGID() : mangledGetLID();
    SmallVector<CallInst *, 4> TIDCalls;
    DPCPPKernelLoopUtils::getAllCallInFunc(TIDName, F, TIDCalls);
    for (auto *CI : TIDCalls) {
      processTIDCall(CI, IsGID);
    }
  };
  // Go over all get_global_id
  ProcessTIDCalls(true);
  // Go over all get_local_id
  ProcessTIDCalls(false);
}

bool WGLoopBoundariesImpl::runOnFunction(Function &F) {
  if (F.hasOptNone())
    return false;

  this->F = &F;

  // Clear used data structures.
  Uni.clear();
  TIDDescs.clear();
  UniDescs.clear();
  ToRemove.clear();

  // Collect information of get***id calls.
  collectTIDData();
  // Collect uniform data from the current basic block.
  collectBlockData(&F.getEntryBlock());

  // Iteratively examines if the entry block branch is early exit branch,
  // min/max with uniform value.
  // If so, collect the early exit description and try to collapse the code
  // successor into entry block.
  bool EarlyExitCollapsed = false;
  bool MinMaxBoundaryRemoved = false;
  do {
    MinMaxBoundaryRemoved = findAndHandleTIDMinMaxBound();
    EarlyExitCollapsed = findAndCollapseEarlyExit();
  } while (MinMaxBoundaryRemoved || EarlyExitCollapsed);

  // Create early exit functions for later use of loop generator.
  createWGLoopBoundariesFunction();

  // Remove all instructions marked for removal.
  for (Instruction *I : ToRemove) {
    assert(I->getNumUses() == 0 && "no users expected");
    I->eraseFromParent();
  }

  DPCPPStatistic::pushFunctionStats(KernelStats, F, DEBUG_TYPE);

  LLVM_DEBUG(print(dbgs(), F.getName()));

  return true;
}

Function *WGLoopBoundariesImpl::createLoopBoundariesFunctionDecl() {
  unsigned NumEntries = WGBoundDecoder::getNumWGBoundArrayEntries(NumDim);
  StringRef FuncName = F->getName();
  std::string EEFuncName = WGBoundDecoder::encodeWGBound(FuncName);
  Type *RetTy = ArrayType::get(IndTy, NumEntries);

  // Check if ArgTypes are already initialized, if not create it.
  SmallVector<Type *, 16> ArgTypes;
  transform(F->args(), std::back_inserter(ArgTypes),
            [](Argument &Arg) { return Arg.getType(); });

  auto *FTy = FunctionType::get(RetTy, ArgTypes, false);
  // Set the linkage type as external, so that it won't be removed before
  // WGLoopCreator pass. We may change the linkage type to private after
  // creating WG loop, and then the function will be removed before CodeGen
  // if inlined.
  auto *CondFunc =
      Function::Create(FTy, GlobalValue::ExternalLinkage, EEFuncName, &M);
  return CondFunc;
}

void WGLoopBoundariesImpl::recoverInstructions(VMap &ValueMap, VVec &Roots,
                                               BasicBlock *BB, Function *NewF) {
  // Mapping the function arguments.
  for (auto It = F->arg_begin(), E = F->arg_end(), NewIt = NewF->arg_begin();
       It != E; ++It, ++NewIt)
    ValueMap[&*It] = &*NewIt;

  // Adding all instructions leading to the boundary to reconstruct set.
  VVec ToAdd = Roots; // Hard copy of Roots.
  BasicBlock *Entry = &F->getEntryBlock();
  while (!ToAdd.empty()) {
    Value *Curr = ToAdd.back();
    ToAdd.pop_back();
    // Value was already mapped, no need to do anything.
    if (ValueMap.count(Curr))
      continue;

    auto *I = dyn_cast<Instruction>(Curr);
    // If the value is not an instruction (global, constant), then it should be
    // mapped to itself.
    if (!I) {
      assert(!isa<Argument>(Curr) && "arguments are supposed to be mapped");
      ValueMap[Curr] = Curr;
      continue;
    }

    assert(I->getParent() == Entry && "Instruction not in the entry block");
    ValueMap[I] = I->clone();
    for (Value *Op : cast<User>(I)->operands())
      ToAdd.push_back(Op);
  }

  // Running according to the order of the original entry block, and connecting
  // each instruction to the operands in the new function. The order of the
  // original function ensures the correctness of the order of the inserted
  // instruction.
  for (auto &I : *Entry) {
    if (!ValueMap.count(&I))
      continue;
    auto *Clone = cast<Instruction>(ValueMap[&I]);
    BB->getInstList().push_back(Clone);
    for (auto Op = Clone->op_begin(), E = Clone->op_end(); Op != E; ++Op) {
      Value *&VMSlot = ValueMap[*Op];
      assert(VMSlot && "all operands should be mapped");
      if (VMSlot)
        *Op = VMSlot;
    }
  }
}

bool WGLoopBoundariesImpl::handleBuiltinBoundMinMax(Instruction *TidInst) {
  // The tid's only user should be min/max builtin.
  if (!TidInst->hasOneUse())
    return false;
  auto *CI = dyn_cast<CallInst>(*(TidInst->user_begin()));
  if (!CI)
    return false;

  // Currently uniformity information is available only for the first block.
  // This can be relaxed when WorkItemAnalysis supports control flow.
  if (CI->getParent() != &(F->getEntryBlock()))
    return false;

  // Check if this is a scalar min/max builtin.
  auto *Callee = CI->getCalledFunction();
  if (!Callee)
    return false;
  StringRef CalleeName = Callee->getName();
  bool IsMinBuiltin;
  bool IsSigned;
  if (!RTService->isScalarMinMaxBuiltin(CalleeName, IsMinBuiltin, IsSigned))
    return false;
  assert(CI->arg_size() == 2 && "bad min,max signature");

  // Track the boundary and the tid call.
  Value *Tid;
  Value *Bound[2] = {nullptr};
  if (!traceBackMinMaxCall(CI, &Bound[0], Tid))
    return false;

  // Get the tid properties from the map.
  assert(TIDs.count(Tid) && Bound[0] && "invalid tid, bound");
  unsigned Dim;
  bool IsGID;
  std::tie(Dim, IsGID) = TIDs[Tid];

  bool IsUpperBound = IsMinBuiltin; // Min creates upper bound, max lower bound.
  bool ContainsVal = true;          // All min/max are inclusive.
  TIDDescs.push_back(
      {Bound[0], Dim, IsUpperBound, ContainsVal, IsSigned, IsGID});
  CI->replaceAllUsesWith(TidInst);
  ToRemove.insert(CI);

  return true;
}

bool WGLoopBoundariesImpl::handleCmpSelectBoundary(Instruction *TidInst) {
  // The TidInst users should be cmp, select with the same operands.
  // First find the select user.
  if (TidInst->getNumUses() != 2)
    return false;
  Value *User1 = *(TidInst->user_begin());
  Value *User2 = *(++(TidInst->user_begin()));
  auto *SI = dyn_cast<SelectInst>(User1);
  if (!SI)
    SI = dyn_cast<SelectInst>(User2);
  if (!SI)
    return false;

  // Currently uniformity information is available only for the first block.
  // This can be relaxed when WorkItemAnalysis supports control flow.
  if (SI->getParent() != &(F->getEntryBlock()))
    return false;

  // The cmp should be the select mask operand.
  // The select should be the only user of the cmp.
  Value *Mask = SI->getCondition();
  ICmpInst *Cmp = dyn_cast<ICmpInst>(Mask);
  if (!Cmp || !Cmp->hasOneUse())
    return false;

  // The compare and the select should have the same operands.
  // This ensures that cmp is user of the TidInst.
  Value *TrueOp = SI->getTrueValue();
  Value *FalseOp = SI->getFalseValue();
  Value *CmpOp0 = Cmp->getOperand(0);
  Value *CmpOp1 = Cmp->getOperand(1);
  if (!(CmpOp0 == TrueOp && CmpOp1 == FalseOp) &&
      !(CmpOp1 == TrueOp && CmpOp0 == FalseOp))
    return false;

  // Track the boundary and tid call.
  Value *Tid;
  Value *Bound[2] = {nullptr};
  if (!traceBackCmp(Cmp, &Bound[0], Tid))
    return false;
  // Update the EEVec with the boundary descriptions.
  if (!obtainBoundaryCmpSelect(Cmp, Bound[0], Tid, CmpOp0 == TrueOp))
    return false;

  // Replace the uses of the select with the original tid call, and mark
  // redundant instructions for removal.
  SI->replaceAllUsesWith(TidInst);
  ToRemove.insert(SI);
  ToRemove.insert(Cmp);

  return true;
}

bool WGLoopBoundariesImpl::obtainBoundaryCmpSelect(ICmpInst *Cmp, Value *Bound,
                                                   Value *Tid,
                                                   bool IsSameOrder) {
  // Patterns like a==b ? a : b are handled trivially by instcombine.
  if (!Cmp->isRelational())
    return false;

  // Get the tid properties from the map.
  assert(TIDs.count(Tid) && Bound && "invalid tid, bound");
  unsigned Dim;
  bool IsGID;
  std::tie(Dim, IsGID) = TIDs[Tid];
  CmpInst::Predicate Pred = Cmp->getPredicate();
  bool ContainsVal = true; // all cmp-select are inclusive.
  assert(isSupportedRelationalComparePredicate(Pred) &&
         "unexpected relational cmp predicate");
  bool IsPredLower = isComparePredicateLower(Pred); // is pred <, <=

  // Note that min/max are always inclusive:
  // (tid > bound ? tid : bound) ~ (tid >= bound ? tid : bound) ~ [bound, ...]
  // Also, if we switch operands order in both cmp and select we get the same
  // bounds:
  // (tid > bound ? tid : bound) ~ (bound > tid ? bound : tid) ~ [bound, ...]
  // These notions reduce the patterns into:
  // tid >,>= bound ? tid : bound ~ [bound, ...]
  // tid >,>= bound ? bound : tid ~ [..., bound]
  // tid <,<= bound ? tid : bound ~ [..., bound]
  // tid <,<= bound ? bound : tid ~ [bound, ...]
  bool IsUpperBound = (!IsPredLower ^ IsSameOrder);

  // Update the boundary descriptions vector with the current compare.
  TIDDescs.push_back(
      {Bound, Dim, IsUpperBound, ContainsVal, Cmp->isSigned(), IsGID});

  return true;
}

bool WGLoopBoundariesImpl::findAndHandleTIDMinMaxBound() {
  // In case there are get***id with variable argument, we can not know who
  // are the users of each dimension.
  if (HasVariableTid)
    return false;

  // In case there is an atomic/pipe call we cannot avoid running two work items
  // that use essentially the same id (due to min(get***id(),uniform) as the
  // atomic/pipe call may have different consequences for the same id.
  if (WIUniqueFuncUsers.count(F))
    return false;

  bool RemovedMinMaxBound = false;
  assert(TIDByDim.size() == NumDim && "num dimension mismatch");
  for (unsigned Dim = 0; Dim < NumDim; ++Dim) {
    // Should have exactly one tid generator for that dimension.
    if (TIDByDim[Dim].size() != 1)
      continue;
    CallInst *CI = TIDByDim[Dim][0];

    // Allow truncation for 64 bit systems.
    Instruction *TidInst = CI;
    if (CI->hasOneUse())
      if (auto *TI = dyn_cast<TruncInst>(*(CI->user_begin())))
        TidInst = TI;

    // Check if it matches min/max patterns.
    if (handleCmpSelectBoundary(TidInst) || handleBuiltinBoundMinMax(TidInst))
      RemovedMinMaxBound = true;
  }

  return RemovedMinMaxBound;
}

bool WGLoopBoundariesImpl::findAndCollapseEarlyExit() {
  // Supported pattern is that entry block ends with conditional branch, and
  // has no side effect instructions.
  BasicBlock *Entry = &F->getEntryBlock();
  auto *Br = dyn_cast<BranchInst>(Entry->getTerminator());
  if (!Br || !Br->isConditional() || hasSideEffectInst(Entry))
    return false;

  // Collect description of early exit if exists.
  BasicBlock *TrueSucc = Br->getSuccessor(0);
  BasicBlock *FalseSucc = Br->getSuccessor(1);
  BasicBlock *EERemove = nullptr;
  BasicBlock *EESucc = nullptr;
  // Checks early exit on true side.
  if (isEarlyExitSucc(TrueSucc)) {
    if (isEarlyExitBranch(Br, true)) {
      EERemove = TrueSucc;
      EESucc = FalseSucc;
    }
  }
  // Checks early exit on false side.
  else if (isEarlyExitSucc(FalseSucc)) {
    if (isEarlyExitBranch(Br, false)) {
      EERemove = FalseSucc;
      EESucc = TrueSucc;
    }
  }

  // An early exit was found, remove the branch at the entry block, and merge
  // it with the non-exit successor if possible.
  if (EERemove) {
    CreatedEarlyExit = 1;
    EERemove->removePredecessor(Entry);
    ToRemove.insert(Br);
    BranchInst::Create(EESucc, Entry);
    // If the successor has the entry as unique predecessor, then we might find
    // the successor is not in a loop and it is safe to scan it for new early
    // exit opportunities.
    if (EESucc->getUniquePredecessor()) {
      // Collect TID info for the code successor block.
      // Since the entry is the only pred, the successor is not part of a loop.
      collectBlockData(EESucc);
      // Try to merge the block into its pred.
      // If the blocks were merged we might have another early exit opportunity.
      if (MergeBlockIntoPredecessor(EESucc))
        return true;
    }
  }

  return false;
}

bool WGLoopBoundariesImpl::collectCond(SmallVector<ICmpInst *, 4> &Compares,
                                       IVec &UniformConds, Instruction *Root) {
  unsigned RootOp = Root->getOpcode();
  assert((RootOp == Instruction::And || RootOp == Instruction::Or ||
          RootOp == Instruction::Select) &&
         "Only 'and', 'or' and 'select' are supported");

  // First candidates are the two operands.
  Value *Cand1 = Root->getOperand(0);
  Value *Cand2 = Root->getOperand(1);
  if (RootOp == Instruction::Select && isa<ConstantInt>(Cand2)) {
    // 'Select' case. Add the non-constant operand.
    Cand2 = Root->getOperand(2);
  }
  SmallVector<Value *, 4> Cands{Cand1, Cand2};

  do {
    auto *Curr = dyn_cast<Instruction>(Cands.back());
    if (!Curr)
      return false;
    Cands.pop_back();
    if (isUniform(Curr)) {
      // Curr is uniform then fill uniformCands.
      LLVM_DEBUG(dbgs() << "Curr is uniform: " << *Curr << "\n");
      UniformConds.push_back(Curr);
    } else if (auto *Cmp = dyn_cast<ICmpInst>(Curr)) {
      Compares.push_back(Cmp);
    } else if (Curr->getOpcode() == RootOp) {
      // It is the same as root, its operands are new candidates.
      Cands.push_back(Curr->getOperand(0));
      Cands.push_back(Curr->getOperand(1));
    } else {
      // Not in the pattern.
      return false;
    }
  } while (!Cands.empty());

  return true;
}

bool WGLoopBoundariesImpl::createRightBound(bool IsCmpSigned, Instruction *Loc,
                                            Value **Bound, Value *LeftBound,
                                            Type *OriginalTy, Type *NewTy,
                                            Instruction::BinaryOps Inst) {
  auto *Cmp = dyn_cast<CmpInst>(Loc);
  if (IsCmpSigned && !Cmp)
    return false;

  if (!IsCmpSigned) {
    Bound[0] = BinaryOperator::Create(Inst, Bound[0], LeftBound,
                                      "right_boundary_align", Loc);
    return true;
  }

  // Incase we perform trunc over the (id+a) expression we want to make sure
  // that we take the left bound with the correct sign (and not Zext(a)).
  // When the comparison is signed we can assume that
  // Sext(id+a) == Sext(id) + Sext(a) - as we are not handling underflow and
  // overflow and as we check the left bound prior to it.
  // Thus before adding a into the right bound, we perform Sext(Trunc(a)).
  if (OriginalTy) {
    auto *CastedLeftBound =
        new TruncInst(LeftBound, OriginalTy, "casted_left_bound", Loc);
    LeftBound = new SExtInst(CastedLeftBound, NewTy, "left_sext_bound", Loc);
  }

  // Checking if empty range is created - right bound is negative.
  Value *CreateEmptyRange = nullptr;

  CmpInst::Predicate CondPred =
      Cmp->isFalseWhenEqual() ? CmpInst::ICMP_SLT : CmpInst::ICMP_SLE;
  assert((Inst == Instruction::Sub || Inst == Instruction::Add) &&
         "invliad Inst");
  if (Inst == Instruction::Sub) {
    CreateEmptyRange =
        new ICmpInst(Loc, CondPred, Bound[0], LeftBound, "right_lt_left");
  } else {
    auto *NegLeft = BinaryOperator::CreateNeg(LeftBound, "left_boundary", Loc);
    CreateEmptyRange =
        new ICmpInst(Loc, CondPred, Bound[0], NegLeft, "right_lt_left");
  }

  // Detect right bound overflow.
  Value *NonNegativeRightBound = BinaryOperator::CreateNot(
      CreateEmptyRange, "non_negative_right_bound", Loc);
  Bound[0] = BinaryOperator::Create(Inst, Bound[0], LeftBound,
                                    "right_boundary_align", Loc);

  // Make upper bound inclusive, because in case of overflow we would like to
  // make it maximum value inclusive.
  if (Cmp->isFalseWhenEqual()) {
    bool IsLT = Cmp->getPredicate() == CmpInst::ICMP_SLT;
    Instruction::BinaryOps BO = IsLT ? Instruction::Sub : Instruction::Add;
    CmpInst::Predicate Pred = IsLT ? CmpInst::ICMP_SGT : CmpInst::ICMP_SLT;
    auto *One = ConstantInt::get(Bound[0]->getType(), 1);
    auto *InclusiveBound = BinaryOperator::Create(
        BO, Bound[0], One, "inclusive_right_boundary", Loc);
    auto *Compare = new ICmpInst(Loc, Pred, InclusiveBound, Bound[0], "");
    *(Bound) = SelectInst::Create(Compare, Bound[0], InclusiveBound,
                                  "inclusive_right_bound", Loc);
    RightBoundInc = true;
  }

  // In case we deduce empty range is created, we set the bounds to [MAX,-1] so
  // that if the bounds should be inversed we would get the whole range
  // inclusive [-1,MAX].
  // Otherwise, no left bound is needed, therefore it is set to -2 (could be any
  // number <-1) to mark that it is not relevant.
  DataLayout DL(&M);
  unsigned SizeInBits = DL.getTypeAllocSizeInBits(Bound[0]->getType());
  APInt MaxSigned = APInt::getSignedMaxValue(SizeInBits);
  Value *Max = ConstantInt::get(Bound[0]->getType(), MaxSigned);
  Value *MinusOne = ConstantInt::get(Bound[0]->getType(), -1);
  Value *Minus = ConstantInt::get(Bound[0]->getType(), -2);
  Bound[0] = SelectInst::Create(CreateEmptyRange, MinusOne, Bound[0],
                                "right_bound", Loc);
  // If left bound is not relevant, set it to a negative number.
  Bound[1] =
      SelectInst::Create(CreateEmptyRange, Max, Minus, "final_left_bound", Loc);

  // Detect right bound overflow.
  // Left and right parts are positive but the total is negative.
  Value *Zero = ConstantInt::get(Bound[0]->getType(), 0);
  Value *RightBoundNeg =
      new ICmpInst(Loc, CmpInst::ICMP_SLT, Bound[0], Zero, "negative_right");
  Value *RightOverflow =
      BinaryOperator::Create(Instruction::And, RightBoundNeg,
                             NonNegativeRightBound, "right_overflow", Loc);
  Bound[0] = SelectInst::Create(RightOverflow, Max, Bound[0],
                                "final_right_bound", Loc);

  return true;

  return true;
}

bool WGLoopBoundariesImpl::doesLeftBoundFit(Type *ComparisonTy,
                                            Value *LeftBound) {
  DataLayout DL(&M);
  // If the left bound is a constant - it is already truncated into the correct
  // size.
  if (isa<ConstantInt>(LeftBound))
    return true;

  // If left bound is a variable, check its type is less or equal the comparison
  // type.
  uint64_t TySize = DL.getTypeAllocSizeInBits(LeftBound->getType());
  if (auto *LeftBoundInst = dyn_cast<Instruction>(LeftBound)) {
    // In case the left bound is an instruction, maybe it is SExt or ZExt, in
    // this case we want to check the original type of the left bound.
    unsigned Op = LeftBoundInst->getOpcode();
    if (Op == Instruction::SExt || Op == Instruction::ZExt)
      TySize =
          DL.getTypeAllocSizeInBits(LeftBoundInst->getOperand(0)->getType());
  }
  if (TySize > DL.getTypeAllocSizeInBits(ComparisonTy))
    return false;

  return true;
}

bool WGLoopBoundariesImpl::traceBackBound(Value *V1, Value *V2,
                                          bool IsCmpSigned, Instruction *Loc,
                                          Value **Bound, Value *&Tid) {
  // The input values should be tid dependent value compared with uniform one.
  // First we find which is uniform and which is tid-dependent and abort
  // otherwise.
  bool IsV1Uniform = isUniform(V1);
  bool IsV2Uniform = isUniform(V2);
  if (IsV1Uniform == IsV2Uniform)
    return false;
  Bound[0] = IsV1Uniform ? V1 : V2;

  Tid = IsV1Uniform ? V2 : V1;
  Type *OriginalTy = nullptr;
  Type *ComparisonTy = V1->getType();
  RightBoundInc = false;

  // The pattern of boundary condition is: comparison between TID and Uniform.
  // But more general pattern is comparison between f(TID) and Uniform.
  // In that case the bound will be f_inverse(Uniform).
  while (auto *TidInst = dyn_cast<Instruction>(Tid)) {
    bool IsFirstOperandUniform = isUniform(TidInst->getOperand(0));
    if (TidInst->getNumOperands() == 2) {
      // If both operands are non-uniform, TID calls will be placed into the
      // boundary function. However they can't be resolved outside of WG loop.
      if (!IsFirstOperandUniform && !isUniform(TidInst->getOperand(1)))
        return false;
    }
    switch (TidInst->getOpcode()) {
    case Instruction::Trunc:
      OriginalTy = Bound[0]->getType();
      // If candidate is trunc instruction, then we can safely extend the bound
      // to have equivalent condition according to sign of comparison.
      Tid = TidInst->getOperand(0);
      Bound[0] = CastInst::CreateIntegerCast(Bound[0], Tid->getType(),
                                             IsCmpSigned, "to_tid_type", Loc);
      if (Bound[1])
        Bound[1] = CastInst::CreateIntegerCast(Bound[1], Tid->getType(),
                                               IsCmpSigned, "to_tid_type", Loc);
      break;
    case Instruction::Add: {
      // At the moment we are looking for particular pattern:
      //
      //   (id + a) < b       // both signed and unsigned comparisons
      //
      // We are trying to replace it with boundaries for id.
      //
      //   -a <= id           // in the unsigned case only
      //   id < (b - a)
      //
      // In addition to that, we should make sure there were no unsigned integer
      // overflow during the boundary computations, otherwise boundaries would
      // be incorrect.
      Tid = IsFirstOperandUniform ? TidInst->getOperand(1)
                                  : TidInst->getOperand(0);
      Value *LeftBound = IsFirstOperandUniform ? TidInst->getOperand(0)
                                               : TidInst->getOperand(1);

      if (IsCmpSigned && !doesLeftBoundFit(ComparisonTy, LeftBound))
        return false;

      assert(Bound[0]->getType() == LeftBound->getType() &&
             "Types of left and right boundaries must match.");
      // Compute right (upper) boundary for the id:
      //   id < (b - a)
      Value *RightBound = Bound[0];
      if (!createRightBound(IsCmpSigned, Loc, Bound, LeftBound, OriginalTy,
                            Tid->getType(), Instruction::Sub))
        return false;
      if (!IsCmpSigned) {
        // Left bound is needed only is unsigned comparison.
        Bound[1] = BinaryOperator::CreateNeg(LeftBound, "left_boundary",
                                             cast<Instruction>(Bound[0]));
        Value *Zero = ConstantInt::get(Bound[1]->getType(), 0);
        auto *Cmp = new ICmpInst(Loc, CmpInst::ICMP_SLT, Bound[1], Zero,
                                 "left_lt_zero");
        Bound[1] = SelectInst::Create(Cmp, Zero, Bound[1],
                                      "non_negative_left_bound", Loc);
        // Unsigned overflow is now allowed, i.e. result value must be
        // non-negative, otherwise it will result in wrong boundary at unsigned
        // comparison.
        // If overflow has happened, right boundary is less than left boundary,
        // so there are no WI to execute. Then we just set left bound to 1 and
        // set right bound to 0.
        assert(dyn_cast<CmpInst>(Loc) && "Expect CMP instruction for tid user");
        CmpInst::Predicate CondPred = dyn_cast<CmpInst>(Loc)->isFalseWhenEqual()
                                          ? CmpInst::ICMP_SLT
                                          : CmpInst::ICMP_SLE;
        auto *RightOverflowCheck =
            new ICmpInst(Loc, CondPred, RightBound, LeftBound, "right_lt_left");
        // If overflow has happened, right boundary should be set to 0.
        Bound[0] = SelectInst::Create(RightOverflowCheck, Zero, Bound[0],
                                      "final_right_bound", Loc);
        // Compute left (lower) boundary for the id, if overflow has happended
        // for right boundary computation, right boundary is set 0. To make
        // left (lower) boundary greater than right (upper) boundary, set left
        // boundary as right boundary value plus one.
        auto *One = ConstantInt::get(Bound[0]->getType(), 1);
        auto *LeftPlusOne = BinaryOperator::Create(
            Instruction::Add, Bound[0], One, "left_after_overflow", Loc);
        Bound[1] = SelectInst::Create(RightOverflowCheck, LeftPlusOne, Bound[1],
                                      "final_left_bound", Loc);
      }
      break;
    }
    case Instruction::Sub: {
      // At the moment we are looking for particular pattern:
      //
      //   (id - a) < b      // both signed and unsigned comparisons
      //
      // We are trying to replace it with boundaries for id.
      //
      //   a <= id           // in the unsigned case only
      //   id < (b + a)
      //
      // In addition to that we should make sure there were no unsigned integer
      // overflow during the boundary computations, otherwise boundaries would
      // be incorrect.
      Tid = IsFirstOperandUniform ? TidInst->getOperand(1)
                                  : TidInst->getOperand(0);
      Value *LeftBound = IsFirstOperandUniform ? TidInst->getOperand(0)
                                               : TidInst->getOperand(1);
      // If the pattern is (a - id) < b, we need to reverse lower/upper
      // bound.
      ReverseLowerUpperBound = IsFirstOperandUniform;
      assert(Bound[0]->getType() == LeftBound->getType() && "Types must match");

      if (IsCmpSigned && !doesLeftBoundFit(ComparisonTy, LeftBound))
        return false;

      Value *RightBound = Bound[0];
      if (!createRightBound(IsCmpSigned, Loc, Bound, LeftBound, OriginalTy,
                            Tid->getType(), Instruction::Add))
        return false;
      if (!IsCmpSigned) {
        // Left bound is needed only if unsigned comparisons.
        // Compute left (lower) boundary for the id: max(0, a).
        // Add additional check for unsigned overflow.
        Bound[1] = LeftBound;
        Value *Zero = ConstantInt::get(Bound[1]->getType(), 0);
        auto *Cmp = new ICmpInst(Loc, CmpInst::ICMP_SLT, Bound[1], Zero,
                                 "left_lt_zero");
        Bound[1] = SelectInst::Create(Cmp, Zero, Bound[1],
                                      "non_negative_left_bound", Loc);
        assert(dyn_cast<CmpInst>(Loc) && "Expect CMP instruction for tid user");
        CmpInst::Predicate CondPred = dyn_cast<CmpInst>(Loc)->isFalseWhenEqual()
                                          ? CmpInst::ICMP_SLT
                                          : CmpInst::ICMP_SLE;
        auto *RightOverflowCheck =
            new ICmpInst(Loc, CondPred, Bound[0], RightBound, "right_lt_left");
        // If overflow has happended for the right boundary computation, set
        // upper bound to maximum possible value.
        // Compute right (upper) boundary for the id:
        //   min(ID_MAX, b + a)
        // Unsigned overflow is now allowed, otherwise it will result in wrong
        // boundary at unsigned comparison.
        Value *Max = ConstantInt::getAllOnesValue(Bound[0]->getType());
        Bound[0] = SelectInst::Create(RightOverflowCheck, Max, Bound[0],
                                      "final_right_bound", Loc);
      }
      break;
    }
    case Instruction::Call:
      // Add uniform information for newly created instructions into Uni for
      // future queries. The return here is the only one which can return true
      // in this function, so we only need to add the uniform information here.
      // Boundaries are uniform over all workitems, so we can just set them to
      // be true.
      Uni[Bound[0]] = true;
      LLVM_DEBUG(dbgs() << "Bound[0] is uniform: " << *Bound[0] << "\n");
      if (Bound[1]) {
        Uni[Bound[1]] = true;
        LLVM_DEBUG(dbgs() << "Bound[1] is uniform: " << *Bound[1] << "\n");
      }
      // Only supported candidate is tid generator itself.
      return TIDs.count(Tid);
    case Instruction::AShr: {
      if (!IsCmpSigned)
        return false;
      Value *ShiftVal = TidInst->getOperand(1);
      auto *ShlInst = dyn_cast<Instruction>(TidInst->getOperand(0));
      if (!ShlInst || ShlInst->getOpcode() != Instruction::Shl)
        return false;
      auto *ShiftLeftVal = dyn_cast<ConstantInt>(ShlInst->getOperand(1));
      auto *ShiftRightVal = dyn_cast<ConstantInt>(ShiftVal);
      if (!ShiftLeftVal || !ShiftRightVal ||
          ShiftLeftVal->getType() != ShiftVal->getType())
        return false;
      if (ShiftLeftVal->getValue() != ShiftRightVal->getValue())
        return false;
      Tid = ShlInst->getOperand(0);
      break;
    }
    default:
      // No other patterns supported.
      return false;
    }
  }
  return false;
}

bool WGLoopBoundariesImpl::traceBackCmp(ICmpInst *Cmp, Value **Bound,
                                        Value *&Tid) {
  Value *Op0 = Cmp->getOperand(0);
  Value *Op1 = Cmp->getOperand(1);
  return traceBackBound(Op0, Op1, Cmp->isSigned(), Cmp, Bound, Tid);
}

bool WGLoopBoundariesImpl::traceBackMinMaxCall(CallInst *CI, Value **Bound,
                                               Value *&Tid) {
  Value *Arg0 = CI->getArgOperand(0);
  Value *Arg1 = CI->getArgOperand(1);
  return traceBackBound(Arg0, Arg1, false, CI, Bound, Tid);
}

void WGLoopBoundariesImpl::replaceTidWithBound(bool IsGID, unsigned Dim,
                                               Value *ToRep) {
  assert(ToRep->getType() == IndTy && "bad type");
  SmallVector<CallInst *, 4> TidCalls;
  DPCPPKernelLoopUtils::getAllCallInFunc(
      IsGID ? mangledGetGID() : mangledGetLID(), F, TidCalls);
  for (auto *TidCall : TidCalls) {
    auto *DimConst = cast<ConstantInt>(TidCall->getOperand(0));
    unsigned DimArg = DimConst->getZExtValue();
    if (Dim != DimArg)
      continue;
    // If ToRep is an instruction, before replacing TidCall With ToRep, we
    // should recursively move all users of TidCall with the same basic block
    // after ToRep, so that ToRep will dominate all its users.
    if (auto *ToRepInst = dyn_cast<Instruction>(ToRep)) {
      SmallVector<Instruction *, 16> WorkList;
      WorkList.push_back(TidCall);
      Instruction *InsertPoint = ToRepInst;
      BasicBlock *CurrBB = TidCall->getParent();
      // As we've run CFG simplication pass before this pass, there should be
      // no case that ToRepInst and TidCall are not in the same basic block.
      assert(CurrBB == ToRepInst->getParent() &&
             "ToRepInst and TiDCall must be in the same basic block");
      while (!WorkList.empty()) {
        Instruction *I = WorkList.pop_back_val();
        for (User *U : I->users()) {
          auto *UI = dyn_cast<Instruction>(U);
          if (!UI || UI->getParent() != CurrBB)
            continue;
          UI->moveAfter(InsertPoint);
          InsertPoint = UI;
          WorkList.push_back(UI);
        }
      }
    }

    // We remove all calls at the end to avoid invalidating internal data
    // structures that keep information about tid calls.
    TidCall->replaceAllUsesWith(ToRep);
    ToRemove.insert(TidCall);
  }
}

bool WGLoopBoundariesImpl::isSupportedRelationalComparePredicate(
    CmpInst::Predicate P) {
  return P == CmpInst::ICMP_ULT || P == CmpInst::ICMP_ULE ||
         P == CmpInst::ICMP_SLT || P == CmpInst::ICMP_SLE ||
         P == CmpInst::ICMP_UGT || P == CmpInst::ICMP_UGE ||
         P == CmpInst::ICMP_SGT || P == CmpInst::ICMP_SGE;
}

bool WGLoopBoundariesImpl::isComparePredicateLower(CmpInst::Predicate P) {
  return P == CmpInst::ICMP_ULT || P == CmpInst::ICMP_ULE ||
         P == CmpInst::ICMP_SLT || P == CmpInst::ICMP_SLE;
}

bool WGLoopBoundariesImpl::isComparePredicateInclusive(CmpInst::Predicate P) {
  return P == CmpInst::ICMP_ULE || P == CmpInst::ICMP_UGE ||
         P == CmpInst::ICMP_SLE || P == CmpInst::ICMP_SGE;
}

bool WGLoopBoundariesImpl::obtainBoundaryEE(ICmpInst *Cmp, Value **Bound,
                                            Value *Tid, bool EETrueSide,
                                            TIDDescVec &EEVec) {
  DataLayout DL(&M);
  unsigned TIDInd = isUniform(Cmp->getOperand(0)) ? 1 : 0;
  assert(isUniform(Cmp->getOperand(1 - TIDInd)) && // tid is compared to uniform
         !isUniform(Cmp->getOperand(TIDInd)) &&    // tid is not uniform
         "exactly one of the operands must be uniform");
  // Get the tid properties from the map.
  assert(TIDs.count(Tid) && Bound && Bound[0] && "invalid tid, bound");
  unsigned Dim;
  bool IsGID;
  std::tie(Dim, IsGID) = TIDs[Tid];

  CmpInst::Predicate Pred = Cmp->getPredicate();
  if (!Cmp->isRelational()) {
    assert((Pred == CmpInst::ICMP_EQ || Pred == CmpInst::ICMP_NE) &&
           "unexpected non-relational cmp predicate");
    if ((Pred == CmpInst::ICMP_EQ) ^ EETrueSide) {
      // Here is support for cases where bound is the only value for the tid,
      // meaning the branch is one of the two options:
      // a. if (tid == bound) { kernelcode }
      // b. if (tid != bound) exit

      // Since bound is the only value for the tid, we can replace all the
      // calls with it.
      replaceTidWithBound(IsGID, Dim, Bound[0]);

      // The bound is the only option for tid, so we will fill it as both
      // upper bound and lower bound, both inclusive. Sign of the comparison
      // is no important, since it is equality. Note that we must still update
      // EEVec with the bounds since the bound might be out of range (not in
      // [0 - local_size]).
      EEVec.push_back({Bound[0], Dim, true, true, false, IsGID});
      EEVec.push_back({Bound[0], Dim, false, true, false, IsGID});
      return true;
    } else {
      // In general we don't support case where single work item does not
      // execute, meaning the branch is one of the two options:
      // a. if (tid != bound) { kernelcode}
      // b. if (tid == bound) exit
      // However, if bound=0 we can treat this as exclusive lower bound since
      // tid is known to be >= 0.
      auto *ConstBound = dyn_cast<Constant>(Bound[0]);
      if (!ConstBound) {
        auto *InstBound = dyn_cast<Instruction>(Bound[0]);
        if (InstBound)
          ConstBound = ConstantFoldInstruction(InstBound, DL);
      }
      if (ConstBound && ConstBound->isNullValue()) {
        EEVec.push_back({Bound[0], Dim, false, false, false, IsGID});
        return true;
      }
    }

    // Could not handle non-relational compare.
    return false;
  }

  // Here is the support for relational compare {<, <=, >, >=}
  assert(isSupportedRelationalComparePredicate(Pred) &&
         "unexpected relational cmp predicate");

  // Collect attributes of the compare instruction.
  bool IsPredLower = isComparePredicateLower(Pred); // is pred <, <=
  bool IsSigned = Cmp->isSigned();
  bool IsInclusive = isComparePredicateInclusive(Pred); // is pred <=, >=
  IsInclusive |= RightBoundInc;

  // When deciding whether the uniform value is an upper bound, and whether it
  // is inclusive we need to take into consideration the index of uniform value
  // and whether we exit if the condition is met.
  // E.g. assuming the compare pattern is:
  //   %cond = icmp ult %tid, %uni (tid is get***id, uni is uniform)
  //   br %cond label %BB1, label %BB2
  // If BB2 is return, uni is an upper bound and exclusive.
  // If BB1 is return, uni is an lower bound and inclusive
  bool IsUpper =
      IsPredLower ^ (TIDInd == 1) ^ ReverseLowerUpperBound ^ EETrueSide;
  // Not handling inverse bound of a form [a,b] as it might result in two
  // intervals.
  if (!IsUpper && !IsSigned && Bound[1])
    return false;
  bool ContainsVal = IsInclusive ^ EETrueSide;

  // Update the boundary descriptions vector with the current compare.
  EEVec.push_back({Bound[0], Dim, IsUpper, ContainsVal, IsSigned, IsGID});

  if (Bound[1]) {
    // Second bound keeps the offset of the left bound which was 0 in the
    // unsigned case. Might also keep a value in the signed case in some
    // special cases. E.g. get_global_id(0)+x<y and y<x result in upper and
    // lower bound.
    EEVec.push_back({Bound[1], Dim, !IsUpper, true, IsSigned, IsGID});
  }

  return true;
}

bool WGLoopBoundariesImpl::isEarlyExitBranch(BranchInst *Br, bool EETrueSide) {
  assert(Br->getParent() == &(F->getEntryBlock()) &&
         "expected entry block branch");
  Value *Cond = Br->getCondition();
  if (isa<ConstantInt>(Cond))
    return false;
  auto *CondInst = dyn_cast<Instruction>(Cond);
  // Generally we can handle this but this is unexpected.
  assert(CondInst &&
         "i1 is expected only as instructions or constant bool values");
  // Sanity for release.
  if (!CondInst)
    return false;

  // Patterns supported are:
  // 1. Completely uniform test (that don't depends on TID)
  // 2. ICmp TID against uniform value
  // 3. Ands of (1,2) for branching into the code (avoiding early exit)
  // 4. Or of (1,2) for branching into early exit
  // 5. Select c0, c1, false (same as 3)
  // 6. Select c0, true, c1 (same as 4)
  SmallVector<ICmpInst *, 4> Compares;
  IVec UniformConds;
  if (isUniform(CondInst)) {
    LLVM_DEBUG(dbgs() << "CondInst is uniform: " << *CondInst << "\n");
    UniformConds.push_back(CondInst);
  } else if (auto *Cmp = dyn_cast<ICmpInst>(CondInst)) {
    Compares.push_back(Cmp);
  } else if (EETrueSide && CondInst->getOpcode() == Instruction::Or) {
    if (!collectCond(Compares, UniformConds, CondInst))
      return false;
  } else if (!EETrueSide && CondInst->getOpcode() == Instruction::And) {
    if (!collectCond(Compares, UniformConds, CondInst))
      return false;
  } else if (CondInst->getOpcode() == Instruction::Select) {
    // After D99674, folding 'select' to 'and/or' is forbidden, and we failed
    // to track the boundary. This causes ~50% regression on specACCELref/126.
    // So, we now manually detect the select pattern, and
    //   1) treat 'select c0, c1, false' as 'and c0, c1';
    //   2) treat 'select c0, true, c1' as 'or c0, c1';
    // This part of code can be removed if 'select' is folded again with
    // 'freeze' instrumented.
    unsigned ConstOpIdx = EETrueSide ? 1 : 2;
    unsigned ExpectedConstVal = EETrueSide ? 1 : 0;
    ConstantInt *C;
    if (!(C = dyn_cast<ConstantInt>(CondInst->getOperand(ConstOpIdx))))
      return false;
    if (C->getZExtValue() != ExpectedConstVal)
      return false;
    if (!collectCond(Compares, UniformConds, CondInst))
      return false;
  } else
    return false;

  // Check that Compares have supported pattern.
  TIDDescVec EEVec;
  for (ICmpInst *Cmp : Compares) {
    // We need to be able to track the original tid call and the bound.
    Value *Tid;
    Value *Bound[2] = {nullptr};
    if (!traceBackCmp(Cmp, &Bound[0], Tid))
      return false;
    // Finally we need to obtain the early exit description(s) into EEVec.
    if (!obtainBoundaryEE(Cmp, &Bound[0], Tid, EETrueSide, EEVec))
      return false;
  }

  // All Compares are valid, so we can add them to the TIDDesc.
  TIDDescs.append(EEVec.begin(), EEVec.end());

  for (Instruction *I : UniformConds) {
    UniDescs.push_back({I, EETrueSide});
  }

  return true;
}

bool WGLoopBoundariesImpl::isEarlyExitSucc(BasicBlock *BB) {
  do {
    auto *TI = BB->getTerminator();
    assert(TI && "no terminator?");
    // Block should have no side effect instructions.
    if (hasSideEffectInst(BB))
      return false;
    // If terminator is not ret instruction, we got to return with no side
    // effect.
    if (isa<ReturnInst>(TI))
      return true;

    // Terminator is not return so for being early exit successor it must be
    // unconditional branch.
    auto *Br = dyn_cast<BranchInst>(TI);
    if (!Br)
      return false;
    if (Br->isConditional())
      return false;
    auto *SuccBB = Br->getSuccessor(0);
    // avoid hang in case of infinite loop.
    if (BB == SuccBB)
      break;
    BB = SuccBB;
  } while (1);
  return false;
}

bool WGLoopBoundariesImpl::hasSideEffectInst(BasicBlock *BB) {
  for (auto &I : *BB) {
    switch (I.getOpcode()) {
    // Store has side effect.
    case Instruction::Store:
      return true;
    // For call ask runtime object.
    case Instruction::Call: {
      Function *Callee = cast<CallInst>(&I)->getCalledFunction();
      if (!Callee)
        return true; // Indirect call may have side effect.
      if (!RTService->hasNoSideEffect(Callee->getName()))
        return true;
      break;
    }
    }
  }
  return false;
}

static Value *getMin(bool IsSigned, Value *A, Value *B, BasicBlock *BB) {
  assert(A->getType()->isIntegerTy() && B->getType()->isIntegerTy() &&
         "expect integer type");
  CmpInst::Predicate Pred = IsSigned ? CmpInst::ICMP_SLT : CmpInst::ICMP_ULT;
  auto *Compare = new ICmpInst(*BB, Pred, A, B, "");
  if (IsSigned) {
    Value *Zero = ConstantInt::get(A->getType(), 0);
    auto *CompareNonNegative =
        new ICmpInst(*BB, CmpInst::ICMP_SLT, B, Zero, "");
    auto *SelectA = BinaryOperator::Create(Instruction::Or, CompareNonNegative,
                                           Compare, "", BB);
    return SelectInst::Create(SelectA, A, B, "", BB);
  }
  return SelectInst::Create(Compare, A, B, "", BB);
}

static Value *getMax(bool IsSigned, Value *A, Value *B, BasicBlock *BB) {
  assert(A->getType()->isIntegerTy() && B->getType()->isIntegerTy() &&
         "expect integer type");
  CmpInst::Predicate Pred = IsSigned ? CmpInst::ICMP_SGT : CmpInst::ICMP_UGT;
  auto *Compare = new ICmpInst(*BB, Pred, A, B, "");
  return SelectInst::Create(Compare, A, B, "", BB);
}

Value *WGLoopBoundariesImpl::correctBound(TIDDesc &TD, BasicBlock *BB,
                                          Value *Bound) {
  Value *NewBound = Bound;
  // Lower bound are expected to be inclusive, upper bound are expected to be
  // exclusive. If this is not the case, add 1.
  if (!TD.ContainsVal ^ TD.IsUpperBound)
    NewBound =
        BinaryOperator::Create(Instruction::Add, NewBound, ConstOne, "", BB);

  // Incase bound is not on GID, add the base global id to the bound.
  if (!TD.IsGID)
    NewBound = BinaryOperator::Create(Instruction::Add, NewBound,
                                      BaseGIDs[TD.Dim], "", BB);

  // Incase the bound is changed, we make sure that the additions did not
  // invalidate the result by crossing the +/- \ maxint\0 border.
  // Thus in case border is crossed, we take the original bound instead. We
  // will avoid using it since it is compared after the original boundaries.
  if (NewBound != Bound)
    NewBound = getMax(TD.IsSigned, Bound, NewBound, BB);

  return NewBound;
}

void WGLoopBoundariesImpl::fillInitialBoundaries(BasicBlock *BB) {
  LowerBounds.clear();
  LocalSizes.clear();
  BaseGIDs.clear();
  LoopSizes.clear();
  StringRef BaseGIDName = nameGetBaseGID();
  for (unsigned Dim = 0; Dim < NumDim; ++Dim) {
    CallInst *LocalSize = DPCPPKernelLoopUtils::getWICall(
        &M, mangledGetLocalSize(), IndTy, Dim, BB);
    CallInst *BaseGID =
        DPCPPKernelLoopUtils::getWICall(&M, BaseGIDName, IndTy, Dim, BB);
    LocalSizes.push_back(LocalSize);
    BaseGIDs.push_back(BaseGID);
    LowerBounds.push_back(BaseGID);
    LoopSizes.push_back(LocalSize);
  }
}

void WGLoopBoundariesImpl::recoverBoundInstructions(VMap &ValueMap,
                                                    BasicBlock *BB) {
  // Collect the boundaries and uniform conditions. Recover the instructions
  // leading to them in BB.
  VVec ToRecover;
  for (const UniformDesc &UD : UniDescs)
    ToRecover.push_back(UD.Cond);
  for (const TIDDesc &TD : TIDDescs)
    ToRecover.push_back(TD.Bound);
  recoverInstructions(ValueMap, ToRecover, BB, BB->getParent());
}

void WGLoopBoundariesImpl::obtainEEBoundaries(BasicBlock *BB, VMap &ValueMap) {
  // Entry i will be true if there is early exit on dimension i.
  SmallVector<bool, MAX_WORK_DIM> HasEE(MAX_WORK_DIM, false);
  // Temporary vector to hold computation of upper bounds, to be used later
  // for loop size computation in case of early exit.
  SmallVector<Value *, MAX_WORK_DIM> UpperBounds(MAX_WORK_DIM, nullptr);

  // Run through all descriptions, and obtain UpperBounds, lowerBounds
  // according to the boundary description.
  for (TIDDesc &TD : TIDDescs) {
    unsigned Dim = TD.Dim;
    HasEE[Dim] = true; // encountered early exit in dimension i.
    assert(ValueMap.count(TD.Bound) && "boundary not in value map");
    // Correct the boundaries if needed.
    Value *Bound = ValueMap[TD.Bound];
    Value *EEVal = correctBound(TD, BB, Bound);
    // Incase no upper bound was set yet, first init the buffer with the
    // trivial one (local_size + base_gid).
    if (!UpperBounds[Dim])
      UpperBounds[Dim] = BinaryOperator::Create(
          Instruction::Add, LocalSizes[Dim], BaseGIDs[Dim], "", BB);
    // Create min/max between the bound and previous upper/lower bound.
    if (TD.IsUpperBound)
      UpperBounds[Dim] = getMin(TD.IsSigned, UpperBounds[Dim], EEVal, BB);
    else
      LowerBounds[Dim] = getMax(TD.IsSigned, LowerBounds[Dim], EEVal, BB);
  }

  // If there is early exit on dimension i, set the loop size as the
  // substraction of the upper and lower bounds. Note that this may be zero or
  // negative and this is taken care when computing the uniform early exit.
  for (unsigned Dim = 0; Dim < NumDim; ++Dim)
    if (HasEE[Dim])
      LoopSizes[Dim] = BinaryOperator::Create(
          Instruction::Sub, UpperBounds[Dim], LowerBounds[Dim], "", BB);
}

Value *WGLoopBoundariesImpl::obtainUniformCond(BasicBlock *BB, VMap &ValueMap) {
  assert((UniDescs.size() || TIDDescs.size()) && "expected early exit");
  // The condition should be true to go into the kernel, so create 'not'
  // incase the early exit is on the true side.
  Value *Ret = ConstantInt::get(*Ctx, APInt(1, 1));
  if (UniDescs.size()) {
    for (const UniformDesc &UD : UniDescs) {
      assert(ValueMap.count(UD.Cond) && "Cond not in value map");
      Value *Cur = ValueMap[UD.Cond];
      assert(Cur->getType() == Ret->getType() && "expect i1 type");
      if (UD.ExitOnTrue)
        Cur = BinaryOperator::CreateNot(Cur, "", BB);
      Ret = BinaryOperator::Create(Instruction::And, Ret, Cur, "", BB);
    }
  }

  // If the loop size is different from local_size (because of an early exit),
  // then we add check that it is positive.
  for (unsigned Dim = 0; Dim < NumDim; ++Dim) {
    if (LoopSizes[Dim] != LocalSizes[Dim]) {
      auto *Cmp =
          new ICmpInst(*BB, CmpInst::ICMP_SLT, ConstZero, LoopSizes[Dim], "");
      Ret = BinaryOperator::Create(Instruction::And, Ret, Cmp, "", BB);
    }
  }

  // Extend Ret to size_t type.
  Ret = new ZExtInst(Ret, IndTy, "zext_cast", BB);
  return Ret;
}

void WGLoopBoundariesImpl::createWGLoopBoundariesFunction() {
  // Get the name for the uniform early exit function.
  Function *BoundFunc = createLoopBoundariesFunctionDecl();
  auto *BB = BasicBlock::Create(*Ctx, "entry", BoundFunc);

  // Fill local size member vector, and set initial lower/upper bounds.
  fillInitialBoundaries(BB);
  Value *UniformCond = ConstOne;
  if (UniDescs.size() || TIDDescs.size()) {
    // In case there are early exits, recover instructions leading to them
    // into the block and update the value map.
    VMap ValueMap;
    recoverBoundInstructions(ValueMap, BB);
    // Update upper/lower bounds according to boundary descriptions.
    obtainEEBoundaries(BB, ValueMap);
    // Update uniform condition according to uniform early exit descriptions.
    UniformCond = obtainUniformCond(BB, ValueMap);
  }

  // Insert boundaries into the array return value.
  Value *RetVal = UndefValue::get(BoundFunc->getReturnType());
  for (unsigned Dim = 0; Dim < NumDim; ++Dim) {
    unsigned LoopSizeInd = WGBoundDecoder::getIndexOfSizeAtDim(Dim);
    RetVal =
        InsertValueInst::Create(RetVal, LoopSizes[Dim], LoopSizeInd, "", BB);
    unsigned LowerInd = WGBoundDecoder::getIndexOfInitGidAtDim(Dim);
    RetVal =
        InsertValueInst::Create(RetVal, LowerBounds[Dim], LowerInd, "", BB);
  }
  // Insert the uniform early exit value to the array return value, and
  // return.
  unsigned UniInd = WGBoundDecoder::getUniformIndex();
  RetVal = InsertValueInst::Create(RetVal, UniformCond, UniInd, "", BB);
  ReturnInst::Create(*Ctx, RetVal, BB);
}

void WGLoopBoundariesImpl::print(raw_ostream &OS, StringRef FName) const {
  auto ToChar = [](bool V) { return V ? 'T' : 'F'; };

  OS << "\nWGLoopBoundaries " << FName << "\n";

  OS.indent(2) << "found " << TIDDescs.size() << " early exit boundaries\n";
  for (const TIDDesc &TD : TIDDescs)
    OS.indent(4) << "Dim=" << TD.Dim << ", "
                 << "Contains=" << ToChar(TD.ContainsVal) << ", "
                 << "IsGID=" << ToChar(TD.IsGID) << ", "
                 << "IsSigned=" << ToChar(TD.IsSigned) << ", "
                 << "IsUpperBound=" << ToChar(TD.IsUpperBound) << ", "
                 << "Bound=\"" << *(TD.Bound) << "\"\n";

  OS.indent(2) << "found " << UniDescs.size()
               << " uniform early exit conditions\n";
  for (const UniformDesc &UD : UniDescs)
    OS.indent(4) << "ExitOnTrue=" << ToChar(UD.ExitOnTrue) << ", "
                 << "Cond=\"" << *UD.Cond << "\"\n";
}

namespace {
/// Legacy WGLoopBoundaries pass.
class WGLoopBoundariesLegacy : public ModulePass {
public:
  static char ID;

  WGLoopBoundariesLegacy() : ModulePass(ID) {
    initializeWGLoopBoundariesLegacyPass(*PassRegistry::getPassRegistry());
  }

  ~WGLoopBoundariesLegacy() {}

  StringRef getPassName() const override { return "WGLoopBoundariesLegacy"; }

  bool runOnModule(Module &M) override {
    BuiltinLibInfo *BLI =
        &getAnalysis<BuiltinLibInfoAnalysisLegacy>().getResult();
    WGLoopBoundariesImpl Impl(M, BLI->getRuntimeService());
    return Impl.run();
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
  }
};

} // namespace

char WGLoopBoundariesLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(WGLoopBoundariesLegacy, DEBUG_TYPE,
                      "Create loop boundaries array function", false, false)
INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
INITIALIZE_PASS_END(WGLoopBoundariesLegacy, DEBUG_TYPE,
                    "Create loop boundaries array function", false, false)

ModulePass *llvm::createWGLoopBoundariesLegacyPass() {
  return new WGLoopBoundariesLegacy();
}

PreservedAnalyses WGLoopBoundariesPass::run(Module &M,
                                            ModuleAnalysisManager &AM) {
  BuiltinLibInfo *BLI = &AM.getResult<BuiltinLibInfoAnalysis>(M);
  WGLoopBoundariesImpl Impl(M, BLI->getRuntimeService());
  return Impl.run() ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

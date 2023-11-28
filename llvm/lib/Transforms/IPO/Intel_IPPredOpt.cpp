#if INTEL_FEATURE_SW_ADVANCED
//===--------------------- Intel_IPPredOpt.cpp ----------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the optimization that hoists condition checks
// across function calls to avoid unnecessary computations.
//
// Ex:
//   Before:
//     if (contains(this)) {
//       if (this->field0) {
//         if (this->field2->vtable->function5 == foo) {
//           do_some_real_thing_1();
//         } else if (this->field2->vtable->function5 == bar) {
//           do_some_real_thing_2();
//         }
//       }
//     }
//
// Let us assume "contains" is not a small function and doesn't have any
// side effects. If value of this->field2->vtable->function5 is neither
// "bar" nor "foo", then there is no need to compute value of "contains()" call.
// We could transform the code like below to avoid unnecessary computations
// if we can prove that there are no side effects with "contains()" call.
//
//   After:
//     if (this->field0 && (this->field2->vtable->function5 == foo ||
//         this->field2->vtable->function5 == bar)) {
//       if (contains(this)) {
//         if (this->field2->vtable->function5 == foo) {
//           do_some_real_thing_1();
//         } else if (this->field2->vtable->function5 == bar) {
//           do_some_real_thing_2();
//         }
//       }
//     }
//
// This is implemented as Module Pass even though the transformation
// is not global since it is required to do analysis across functions.
//

#include "llvm/Transforms/IPO/Intel_IPPredOpt.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/TypeMetadataUtils.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/GeneralUtils.h"

using namespace llvm;

#define DEBUG_TYPE "ippredopt"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// This option is mainly used by LIT tests.
//
static cl::opt<bool> IPPredDumpTargetFunctions(
    "ippred-dump-target-functions", cl::init(false), cl::ReallyHidden,
    cl::desc("Dump target functions for virtual function calls"));

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// This option is mainly used by LIT tests. This option will be removed
// once implementation is complete.
//
static cl::opt<bool>
    IPPredSkipCalleeLegalChecks("ippred-skip-callee-legal-checks",
                                cl::init(false), cl::ReallyHidden,
                                cl::desc("Skip legal checks for callee"));

// This option helps to reduce number of possible targets.
static cl::opt<bool> IPPredSkipIndirectTargetHeuristic(
    "ippred-skip-indirect-target-heuristic", cl::init(true), cl::ReallyHidden,
    cl::desc("Skip indirect target heuristic"));

// This is used to represent control condition.
// Value: Condition in the ICmp instruction.
// bool: Whether BasicBlock is executed when the condition is True or False.
using ControlCond = PointerIntPair<Value *, 1, bool>;
using LoopInfoFuncType = std::function<LoopInfo &(Function &)>;

class IPPredOptImpl;

#ifndef NDEBUG
// Dumper for ControlCond
raw_ostream &operator<<(raw_ostream &OS, const ControlCond &C) {
  OS << "[" << *C.getPointer() << ", " << (C.getInt() ? "true" : "false")
     << "]";
  return OS;
}
#endif

// Application may have many candidates. This class is used to
// handle multiple candidates.
class PredCandidate {
public:
  PredCandidate(BasicBlock *ExitBB, BasicBlock *ThenBB, CallBase *CondCall,
                DominatorTree &DT, PostDominatorTree &PDT, AssumptionCache &AC,
                LoopInfoFuncType &GetLI,
                function_ref<DominatorTree &(Function &)> DTGetter,
                function_ref<PostDominatorTree &(Function &)> PDTGetter)
      : ExitBB(ExitBB), ThenBB(ThenBB), CondCall(CondCall), DT(DT), PDT(PDT),
        AC(AC), GetLI(GetLI), DTGetter(DTGetter), PDTGetter(PDTGetter) {}
  ~PredCandidate() {}

  bool collectExecutedBlocks();
  bool collectControlConds(BasicBlock *BB);
  bool collectControlCondsForBlocks();
  bool isControlCondSame(const ControlCond &C1, const ControlCond &C2);
  bool checkLegalityIssues();
  bool canBeMovedTo(Value *V, Instruction *Loc,
                    SmallPtrSetImpl<Instruction *> &Visited);
  bool isHoistableSimpleLoad(Value *V,
                             SmallVectorImpl<Instruction *> &HoistingInst);
  bool isHoistableFieldVtableLoad(Value *V, Constant *C,
                                  SmallVectorImpl<Instruction *> &HoistingInst,
                                  SmallVector<Instruction *, 2> &NonNullChecks);
  bool checkAllHoistingInstInControlBlocks(
      SmallVectorImpl<Instruction *> &HoistingInst);
  bool checkPointerHasNonNullValue(Value *V);
  bool guaranteedToBeNonNullOnCondCallEntry(Value *V);
  void getValueConstant(ICmpInst *IC, Value **VPtr, Constant **CPtr);
  bool checkCondCallSideEffects(IPPredOptImpl &);
  bool checkSpecialNoSideEffectsCall(CallBase *CB, LoopInfo &LoopI);
  bool checkNoSideEffectsCallWithConstTC(CallBase *CB, LoopInfo &LoopI);
  Value *getTripCountCallBaseInLoop(Instruction *, Value *, LoopInfo &);
  bool getNeededInstsToCompute(Value *V,
                               SmallVectorImpl<Instruction *> &HoistingInst,
                               bool NotForHoisting);
  bool getBBControlConditions(BasicBlock *BB,
                              SmallVectorImpl<ControlCond> &Conditions);
  bool isDTransVectorAccessElemCall(CallBase *CB);
  bool processIndirectCalls(IPPredOptImpl &, SmallPtrSet<CallBase *, 2> &,
                            LoopInfo &);
  bool processDirectCalls(IPPredOptImpl &, SmallPtrSet<CallBase *, 6> &,
                          LoopInfo &);
  bool applyHeuristics();
  bool funcHasNoSideEffects(Function *F);
  void hoistConditions();
  void generateRuntimeChecks();
  void replaceOperandsWithClonedInst(
      Instruction *Cloned,
      SmallDenseMap<Instruction *, Instruction *, 8> &InstCloneInstMap);
  PredCandidate(const PredCandidate &) = delete;
  PredCandidate(PredCandidate &&) = delete;
  PredCandidate &operator=(const PredCandidate &) = delete;
  PredCandidate &operator=(PredCandidate &&) = delete;

private:
  // Type of value in conditional statement.
  //   CT_Temp: Temp value
  //   CT_SimpleLoad: Load instruction with GEP as pointer operand.
  //   CT_VtableFieldLoad: Getting function from Vtable of an object which can
  //                       be loaded from memory.
  enum CondTy : uint8_t {
    CT_Bottom,
    CT_Temp,
    CT_SimpleLoad,
    CT_VtableFieldLoad
  };

  // Maximum executed basic blocks that are controlled under
  // inside condition statements.
  constexpr static int MaxNumberExecutedBlocks = 2;

  // Exit block of outermost conditional statement.
  BasicBlock *ExitBB = nullptr;

  // Starting basic block of candidate's CFG.
  BasicBlock *ThenBB = nullptr;

  // Main control condtion call.
  CallBase *CondCall = nullptr;

  // BB that has hoisted conditions.
  BasicBlock *HoistedCondBB = nullptr;

  DominatorTree &DT;

  PostDominatorTree &PDT;

  AssumptionCache &AC;

  LoopInfoFuncType &GetLI;

  function_ref<DominatorTree &(Function &)> DTGetter;

  function_ref<PostDominatorTree &(Function &)> PDTGetter;

  // Basic blocks that are controlled under inside condition statements.
  SmallSetVector<BasicBlock *, 2> ExecutedBlocks;

  // List of all control basic blocks
  SmallPtrSet<BasicBlock *, 8> ControlBlocks;

  // Map between executed block and corresponding control conditions.
  SmallDenseMap<BasicBlock *, SmallVector<ControlCond, 4>, 2> BBControlCondsMap;

  // Mapping between condition values and their types.
  SmallDenseMap<Value *, CondTy, 4> CondTypeMap;

  // To hoist a condition value, set of instructions that need to be
  // hoisted.
  SmallDenseMap<Value *, SmallVector<Instruction *, 8>, 4> BBHoistCondsMap;

  // Can't prove that some pointers are non-null at compile-time. Generate
  // runtime checks to prove the same.
  SmallDenseMap<Value *, SmallVector<Instruction *, 2>, 2>
      SpecialNonNullCheckCondsMap;

  // Selected possible targets without side-effects of all indirect calls
  // in callee.
  SetVector<Function *> SelectedPossibleTargets;

  // Possible targets with side effects.
  SetVector<Function *> PossibleTargetsWithSE;

  // Instructions needed to compute trip count of a loop that has GetElem
  // function call.
  SmallVector<Instruction *, 8> ElemCallTripCHoistInst;

  // Instructions needed to load virtual function pointer.
  SmallVector<Instruction *, 8> IndirectFPtrHoistInst;

  // Collection of control conditions that are required to load loop
  // trip counter.
  SmallVector<ControlCond, 2> CalleeControlConds;

  // Instructions needed for all pointers that need nullptr checks.
  SmallDenseMap<Value *, SmallVector<Instruction *, 5>, 2> CalleeNullChecks;
};

// Main class to implement the transformation.
class IPPredOptImpl {

public:
  IPPredOptImpl(Module &M, WholeProgramInfo &WPInfo,
                function_ref<DominatorTree &(Function &)> DTGetter,
                function_ref<PostDominatorTree &(Function &)> PDTGetter,
                function_ref<AssumptionCache &(Function &)> ACGetter,
                LoopInfoFuncType &GetLI)
      : M(M), WPInfo(WPInfo), DTGetter(DTGetter), PDTGetter(PDTGetter),
        ACGetter(ACGetter), GetLI(GetLI){};
  ~IPPredOptImpl(){};
  bool run(void);
  bool getVirtualPossibleTargets(CallBase &CB,
                                 SmallVectorImpl<Function *> &TargetFunctions);

private:
  constexpr static int MaxNumCandidates = 1;

  Module &M;
  WholeProgramInfo &WPInfo;
  function_ref<DominatorTree &(Function &)> DTGetter;
  function_ref<PostDominatorTree &(Function &)> PDTGetter;
  function_ref<AssumptionCache &(Function &)> ACGetter;
  LoopInfoFuncType &GetLI;

  DenseMap<Metadata *, SmallSet<std::pair<GlobalVariable *, uint64_t>, 4>>
      TypeIdMap;
  SmallPtrSet<PredCandidate *, MaxNumCandidates> Candidates;

  bool mayBBWriteToMemory(BasicBlock *BB);
  bool checkBBControlAllCode(BasicBlock *BB, BasicBlock *ExitBB);
  void gatherCandidates(Function &F);
  void applyTransformations();
  void buildTypeIdMap();

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dumpTargetFunctions(void);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
};

// Build type identification map for Vtables.
void IPPredOptImpl::buildTypeIdMap() {
  SmallVector<MDNode *, 2> Types;
  for (GlobalVariable &GV : M.globals()) {
    Types.clear();
    GV.getMetadata(LLVMContext::MD_type, Types);
    if (GV.isDeclaration() || Types.empty())
      continue;

    for (MDNode *Type : Types) {
      Metadata *TypeID = Type->getOperand(1).get();

      uint64_t Offset =
          cast<ConstantInt>(
              cast<ConstantAsMetadata>(Type->getOperand(0))->getValue())
              ->getZExtValue();

      TypeIdMap[TypeID].insert(std::make_pair(&GV, Offset));
    }
  }
}

// Get possible target functions using type identification map.
bool IPPredOptImpl::getVirtualPossibleTargets(
    CallBase &CB, SmallVectorImpl<Function *> &TargetFunctions) {
  assert(!CB.getCalledFunction() && "Expected indirect call");

  LLVM_DEBUG(dbgs() << "Collecting possible targets for: " << CB << "\n");
  const Instruction *PrevI = CB.getPrevNode();
  if (!PrevI || !isa<LoadInst>(PrevI)) {
    LLVM_DEBUG(dbgs() << "    No LoadInst Found: " << "\n");
    return false;
  }
  auto *LI = cast<LoadInst>(PrevI);
  PrevI = PrevI->getPrevNode();
  if (PrevI && isa<GetElementPtrInst>(PrevI))
    PrevI = PrevI->getPrevNode();

  if (!PrevI || !isa<IntrinsicInst>(PrevI)) {
    LLVM_DEBUG(dbgs() << "    No Assume call Found: " << "\n");
    return false;
  }
  auto *AI = cast<IntrinsicInst>(PrevI);
  if (AI->getIntrinsicID() != Intrinsic::assume) {
    LLVM_DEBUG(dbgs() << "    No Assume intrinsic Found: " << "\n");
    return false;
  }
  PrevI = PrevI->getPrevNode();
  if (!PrevI || !isa<IntrinsicInst>(PrevI)) {
    LLVM_DEBUG(dbgs() << "    No typetest call Found: " << "\n");
    return false;
  }
  auto *TI = cast<CallInst>(PrevI);
  if (TI->getIntrinsicID() != Intrinsic::type_test) {
    LLVM_DEBUG(dbgs() << "    No typetest intrinsic Found: " << "\n");
    return false;
  }

  auto *TypeId = cast<MetadataAsValue>(TI->getArgOperand(1))->getMetadata();
  const Value *Object = LI->getPointerOperand();
  auto DL = LI->getFunction()->getParent()->getDataLayout();
  APInt ObjectOffset(DL.getTypeSizeInBits(Object->getType()), 0);
  Object->stripAndAccumulateConstantOffsets(DL, ObjectOffset,
                                            /* AllowNonInbounds */ true);

  for (auto &VTableInfo : TypeIdMap[TypeId]) {
    GlobalVariable *VTable = VTableInfo.first;
    uint64_t VTableOffset = VTableInfo.second;

    Function *Caller = CB.getFunction();
    LLVM_DEBUG(dbgs() << "    VTable: " << *VTable << "\n");
    LLVM_DEBUG(dbgs() << "    VTableOffset: " << VTableOffset << "\n");
    LLVM_DEBUG(dbgs() << "    ObjectOffset: " << ObjectOffset << "\n");
    Constant *Ptr = getPointerAtOffset(
        VTable->getInitializer(), VTableOffset + ObjectOffset.getZExtValue(),
        *Caller->getParent());
    if (!Ptr) {
      LLVM_DEBUG(dbgs() << "    Can't find pointer in vtable: " << "\n");
      return false;
    }

    auto TargetFunc = dyn_cast<Function>(Ptr->stripPointerCasts());
    if (!TargetFunc) {
      LLVM_DEBUG(dbgs() << "    vtable entry is not function pointer: "
                        << "\n");
      return false;
    }

    TargetFunctions.push_back(TargetFunc);
    LLVM_DEBUG(dbgs() << "    Adding target function: " << TargetFunc->getName()
                      << "\n");
  }

  llvm::stable_sort(TargetFunctions, [=](auto *A, auto *B) {
    return A->getName() < B->getName();
  });

  return true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void IPPredOptImpl::dumpTargetFunctions(void) {
  for (Function &F : M) {
    if (F.isDeclaration() || F.isIntrinsic())
      continue;
    for (auto &I : instructions(&F)) {
      auto CB = dyn_cast<CallBase>(&I);
      if (!CB || CB->getCalledFunction())
        continue;

      dbgs() << F.getName() << "  --  " << *CB << "\n";
      SmallVector<Function *, 16> TargetFunctions;
      if (!getVirtualPossibleTargets(*CB, TargetFunctions) ||
          TargetFunctions.empty()) {
        dbgs() << " Can't find possible targets \n";
        continue;
      }
      for (auto TF : TargetFunctions)
        dbgs() << "        " << TF->getName() << "\n";
    }
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Returns true if "V" can be moved before "Loc" instruction.
bool PredCandidate::canBeMovedTo(Value *V, Instruction *Loc,
                                 SmallPtrSetImpl<Instruction *> &Visited) {

  // Check if instruction's type is hoistable.
  auto IsHoistableInstType = [](Instruction *I) -> bool {
    return isa<GetElementPtrInst>(I) || isa<BinaryOperator>(I) ||
           isa<CastInst>(I) || isa<SelectInst>(I);
  };

  auto *Inst = dyn_cast<Instruction>(V);
  if (!Inst || DT.dominates(Inst, Loc) || Visited.count(Inst))
    return true;

  if (!IsHoistableInstType(Inst))
    return false;

  if (!isSafeToSpeculativelyExecute(Inst, Loc, &AC, &DT) ||
      Inst->mayReadFromMemory())
    return false;

  Visited.insert(Inst);

  return all_of(Inst->operands(),
                [&](Value *Op) { return canBeMovedTo(Op, Loc, Visited); });
}

// Returns true if C1 and C2 are same. This is used to reduce
// number of conditions to be hoisted.
bool PredCandidate::isControlCondSame(const ControlCond &C1,
                                      const ControlCond &C2) {
  if (C1.getInt() == C2.getInt()) {
    if (C1.getPointer() == C2.getPointer())
      return true;
  } else {
    const auto *Cmp1 = dyn_cast<CmpInst>(C1.getPointer());
    const auto *Cmp2 = dyn_cast<CmpInst>(C2.getPointer());
    if (!Cmp1 || !Cmp2)
      return false;
    if (Cmp1->getPredicate() == Cmp2->getInversePredicate() &&
        Cmp1->getOperand(0) == Cmp2->getOperand(0) &&
        Cmp1->getOperand(1) == Cmp2->getOperand(1))
      return true;
  }
  return false;
}

// Computes all conditions that control "BB" from "ThenBB". Starts from
// "BB" and find controlling conditions using PostDominatorTree info
// till "ThenBB" is reached.
bool PredCandidate::collectControlConds(BasicBlock *BB) {
  if (BB == ThenBB || !DT.dominates(ThenBB, BB))
    return false;

  BasicBlock *CurBlock = BB;
  int NumConditions = 0;
  SmallVector<ControlCond, 4> Conditions;

  do {
    if (!DT.getNode(CurBlock))
      return false;
    // Get immediate dominator of CurBlock.
    BasicBlock *CurrDom = DT.getNode(CurBlock)->getIDom()->getBlock();

    // Makes sure ThenBB dominates CurrDom
    if (!DT.dominates(ThenBB, CurrDom))
      return false;

    const BranchInst *BI = dyn_cast<BranchInst>(CurrDom->getTerminator());
    // For now, allow only conditional branches.
    if (!BI || !BI->isConditional() || !isa<ICmpInst>(BI->getCondition()))
      return false;

    // For now, allow only ICmp with one operand as constant.
    Value *V = nullptr;
    Constant *CompC = nullptr;
    getValueConstant(cast<ICmpInst>(BI->getCondition()), &V, &CompC);
    if (!V || !CompC)
      return false;

    if (PDT.dominates(CurBlock, BI->getSuccessor(0)))
      Conditions.push_back(ControlCond(BI->getCondition(), true));
    else if (PDT.dominates(CurBlock, BI->getSuccessor(1)))
      Conditions.push_back(ControlCond(BI->getCondition(), false));
    else
      return false;

    // Limit to at most 3 control conditions.
    if (++NumConditions > 3)
      return false;

    CurBlock = CurrDom;
    ControlBlocks.insert(CurBlock);

  } while (CurBlock != ThenBB);

  BBControlCondsMap[BB] = Conditions;
  return true;
}

// Compute all conditions that are controlling all executed basic blocks.
bool PredCandidate::collectControlCondsForBlocks() {
  for (auto *BB : ExecutedBlocks)
    if (!collectControlConds(BB))
      return false;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Dump control conditions of all executed blocks.
  for (auto *BB : ExecutedBlocks) {
    LLVM_DEBUG(dbgs() << "Control conditions for " << BB->getName() << ": \n");
    for (const auto &C : BBControlCondsMap[BB])
      LLVM_DEBUG(dbgs() << "        " << C << "\n");
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  return true;
}

// Returns true if all instructions in "HoistingInstis" are defined
// in ControlBlocks.
bool PredCandidate::checkAllHoistingInstInControlBlocks(
    SmallVectorImpl<Instruction *> &HoistingInsts) {
  for (auto *I : HoistingInsts)
    if (!ControlBlocks.count(I->getParent()))
      return false;
  return true;
}

// Returns true if we can prove "V" has non-null value at CondCall.
// Prove that "V" is used to load / store in entry block (guaranteed
// to be executed) of CondCall.
// This helps to avoid generating non-null checks for hoisting conditions.
//
// Ex:
//  Makes sure %arg is used to load / store in entry block.
//
//  contains(..., %arg, ...) {
//    entry:
//       ...
//       %i = getelementptr inbounds %ValueStore, ptr %arg, i64 0, i32 4
//       load ptr, ptr %i
//       ...
//    BB:
//  }
//  ...
//  tail call contains(...,ptr %V, )
//
bool PredCandidate::guaranteedToBeNonNullOnCondCallEntry(Value *V) {
  // Return argument position of "Ptr" if "Ptr" is passed as argument to "CB".
  // Otherwise, return arg_size().
  auto GetArgNoInCall = [](CallBase *CB, Value *Ptr) -> unsigned {
    for (unsigned I = 0; I < CB->arg_size(); ++I)
      if (CB->getArgOperand(I) == Ptr)
        return I;
    return CB->arg_size();
  };

  auto *GEP = dyn_cast<GetElementPtrInst>(V);
  if (!GEP)
    return false;
  Value *Ptr = GEP->getPointerOperand();

  // Get argument position of "Ptr" if passed as argument to CondCall.
  unsigned ArgNo = GetArgNoInCall(CondCall, Ptr);
  Function *CondCallee = CondCall->getCalledFunction();
  if (ArgNo >= CondCall->arg_size() || ArgNo >= CondCallee->arg_size())
    return false;

  Argument *Arg = CondCallee->getArg(ArgNo);
  for (Instruction &I : CondCallee->getEntryBlock()) {
    Value *PtrOp = getLoadStorePointerOperand(&I);
    if (!PtrOp)
      continue;
    auto *LdStAddr = dyn_cast<GetElementPtrInst>(PtrOp);
    if (!LdStAddr)
      continue;
    // Check NumIndices and SourceElementType are same for GEP instruction in
    // Callee and caller.
    if (GEP->getNumIndices() != LdStAddr->getNumIndices() ||
        GEP->getSourceElementType() != LdStAddr->getSourceElementType())
      continue;

    if (LdStAddr->getPointerOperand() == Arg)
      return true;
  }
  return false;
}

// Check if "V" is simple load instruction that can be hoisted.
// All hoistable instructions are collected in HoistingInst
//
// Ex:
//     %i317 = getelementptr %ValueStore, ptr %i28, i64 0, i32 0
// V:  %i318 = load i8, ptr %i317
//     %i319 = icmp eq i8 %i318, 0
bool PredCandidate::isHoistableSimpleLoad(
    Value *V, SmallVectorImpl<Instruction *> &HoistingInst) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  HoistingInst.push_back(LI);

  // If pointer operand of LI already dominates CondCall, no
  // need to do any further checks.
  if (DT.dominates(LI->getPointerOperand(), CondCall))
    return true;

  // Check if pointer operand of LI can be hoisted.
  SmallPtrSet<Instruction *, 32> Visited;
  if (!canBeMovedTo(LI->getPointerOperand(), CondCall, Visited))
    return false;
  // Makes sure pointer operand of LI is non-null value at CondCall.
  if (!guaranteedToBeNonNullOnCondCallEntry(LI->getPointerOperand()))
    return false;

  HoistingInst.push_back(cast<Instruction>(LI->getPointerOperand()));
  return true;
}

// Check if "V" is loading address from VTable and "C" is a function.
// Prove only object load can be hoisted to CondCall. No need to
// prove VTable load can be hoisted as it is implied.
// FieldLI: Runtime check is generated to make sure FieldLI is non-null
// as there is no easy way to prove at compile-time.
// All hoistable instructions are collected in HoistingInst.
//
//    %i321 = getelementptr inbounds %ValueStore, ptr %i28, i64 0, i32 2
//    %i322 = load ptr, ptr %i321   ; FieldLI
//    %i323 = getelementptr IdConstraint, ptr %i322, i64 0, i32 0, i32 0
//    %i324 = load ptr, ptr %i323   ; VTLoad
//    %i325 = tail call i1 @llvm.type.test(ptr %i324, metadata !"E")
//    tail call void @llvm.assume(i1 %i325)
//    %i326 = getelementptr inbounds ptr, ptr %i324, i64 5
// V: %i327 = load ptr, ptr %i326
//    %i328 = icmp eq ptr %i327, @func
//
bool PredCandidate::isHoistableFieldVtableLoad(
    Value *V, Constant *C, SmallVectorImpl<Instruction *> &HoistingInst,
    SmallVector<Instruction *, 2> &NonNullChecks) {

  // Check LI is used by type_test and assume intrinsics.
  auto IsVtableLoad = [](LoadInst *LI, GetElementPtrInst *GEPUse) -> bool {
    Value *UseInTypeTest = nullptr;
    for (auto *U : LI->users()) {
      if (U == GEPUse)
        continue;
      if (UseInTypeTest)
        return false;
      UseInTypeTest = U;
    }
    if (!UseInTypeTest)
      return false;
    auto II = dyn_cast<IntrinsicInst>(UseInTypeTest);
    if (!II || II->getIntrinsicID() != Intrinsic::type_test)
      return false;
    if (!II->hasOneUse())
      return false;
    auto AssumeII = dyn_cast<IntrinsicInst>(II->user_back());
    if (!AssumeII || AssumeII->getIntrinsicID() != Intrinsic::assume)
      return false;
    return true;
  };

  if (!isa<Function>(C))
    return false;
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  HoistingInst.push_back(LI);
  auto *GEP = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
  if (!GEP)
    return false;
  if (GEP->getNumIndices() != 1)
    return false;
  HoistingInst.push_back(GEP);
  auto *VTLoad = dyn_cast<LoadInst>(GEP->getPointerOperand());
  if (!VTLoad)
    return false;
  if (!IsVtableLoad(VTLoad, GEP))
    return false;
  HoistingInst.push_back(VTLoad);

  auto *NormGEP = dyn_cast<GetElementPtrInst>(VTLoad->getPointerOperand());
  if (!NormGEP || !NormGEP->hasAllZeroIndices())
    return false;
  HoistingInst.push_back(NormGEP);
  // No further checks are needed if pointer operand of NormGEP already
  // dominates CondCall.
  if (DT.dominates(NormGEP->getPointerOperand(), CondCall))
    return true;

  // If FieldLI is load from another address, makes sure the load
  // can be hoisted.
  auto *FieldLI = dyn_cast<LoadInst>(NormGEP->getPointerOperand());
  if (!FieldLI)
    return false;
  SmallPtrSet<Instruction *, 32> Visited;
  if (!canBeMovedTo(FieldLI->getPointerOperand(), CondCall, Visited))
    return false;
  if (!guaranteedToBeNonNullOnCondCallEntry(FieldLI->getPointerOperand()))
    return false;

  // Generate runtime checks for FieldLI to prove it is non-null pointer.
  NonNullChecks.push_back(FieldLI);
  NonNullChecks.push_back(cast<Instruction>(FieldLI->getPointerOperand()));

  HoistingInst.push_back(FieldLI);
  HoistingInst.push_back(cast<Instruction>(FieldLI->getPointerOperand()));
  return true;
}

// Check legality issues to hoist conditions.
bool PredCandidate::checkLegalityIssues() {
  // Makes sure all basic blocks between ThenBB and ExitBB are either
  // ExecutedBlocks or ControlBlocks.
  for (BasicBlock &BB :
       make_range(ThenBB->getIterator(), ExitBB->getIterator()))
    if (!ExecutedBlocks.count(&BB) && !ControlBlocks.count(&BB))
      return false;

  // Check if all needed conditions can be hoisted to CondCall.
  for (auto *BB : ControlBlocks) {
    auto *BI = cast<BranchInst>(BB->getTerminator());
    auto *IC = cast<ICmpInst>(BI->getCondition());
    Value *V = nullptr;
    Constant *C = nullptr;
    getValueConstant(IC, &V, &C);
    if (!V || !C)
      return false;

    LLVM_DEBUG(dbgs() << "Check legality for hoisting: " << *V << "\n");
    CondTypeMap[V] = CT_Bottom;
    SmallPtrSet<Instruction *, 32> Visited;
    if (canBeMovedTo(V, CondCall, Visited)) {
      LLVM_DEBUG(dbgs() << "      Hoisting can be done \n");
      CondTypeMap[V] = CT_Temp;
      continue;
    }
    SmallVector<Instruction *, 8> HoistingInst;
    if (isHoistableSimpleLoad(V, HoistingInst)) {
      if (!checkAllHoistingInstInControlBlocks(HoistingInst))
        return false;
      LLVM_DEBUG(dbgs() << "      Simple Load Hoisting  \n");
      CondTypeMap[V] = CT_SimpleLoad;
      BBHoistCondsMap[V] = HoistingInst;
      continue;
    }
    HoistingInst.clear();
    SmallVector<Instruction *, 2> NonNullCheckInst;
    if (isHoistableFieldVtableLoad(V, C, HoistingInst, NonNullCheckInst)) {
      if (!checkAllHoistingInstInControlBlocks(HoistingInst))
        return false;
      LLVM_DEBUG(dbgs() << "      Field Vtable Load Hoisting  \n");
      CondTypeMap[V] = CT_VtableFieldLoad;
      BBHoistCondsMap[V] = HoistingInst;

      // If there are any pointers that can't be proved at compile-time,
      // save the pointers in SpecialNonNullCheckCondsMap.
      if (!NonNullCheckInst.empty())
        SpecialNonNullCheckCondsMap[V] = NonNullCheckInst;
      continue;
    }
    LLVM_DEBUG(dbgs() << "      Hoisting can't be done \n");
    return false;
  }
  return true;
}

// Check Callee takes more time than the execution blocks controlled
// under the CondCall.
bool PredCandidate::applyHeuristics() {
  Function *CondCallee = CondCall->getCalledFunction();
  unsigned CalleeCount = CondCallee->getInstructionCount();

  // Collect all basic blocks in the CFG from EntryBB to ExitBB.
  SmallVector<BasicBlock *> CondBBSet;
  GeneralUtils::collectBBSet(ThenBB, ExitBB, CondBBSet);
  unsigned CondNumInstrs = 0;
  for (auto *BB : CondBBSet)
    CondNumInstrs += std::distance(BB->instructionsWithoutDebug().begin(),
                                   BB->instructionsWithoutDebug().end());

  // Makes sure Callee has many more instructions.
  if (CalleeCount < 3 * CondNumInstrs)
    return false;

  // Makes sure Callee has loops.
  LoopInfo &LoopI = (GetLI)(*CondCallee);
  if (llvm::size(LoopI) < 2)
    return false;
  return true;
}

// Returns true if F doesn't have any side-effects.
bool PredCandidate::funcHasNoSideEffects(Function *F) {
  for (Instruction &Inst : instructions(*F)) {
    if (isa<DbgInfoIntrinsic>(Inst))
      continue;
    if (Inst.mayHaveSideEffects())
      return false;
  }
  return true;
}

// If 'RHS' is PHINode like below, skip incoming zero value and return
// other incoming value (%28). Otherwise, just return RHS.
//
//   %i30 = phi i32 [ %i28, %bb26 ], [ 0, %bb19 ]
//
static Value *skipZeroFromTripCount(Value *RHS) {
  auto *PHI = dyn_cast<PHINode>(RHS);
  Value *TripC = nullptr;
  // Skip all incoming zero values and get actual non-zero trip count.
  if (PHI) {
    for (unsigned I = 0, E = PHI->getNumIncomingValues(); I != E; I++) {
      Value *In = PHI->getIncomingValue(I);
      auto *C = dyn_cast<Constant>(In);
      if (C && C->isZeroValue())
        continue;
      if (TripC)
        return nullptr;
      TripC = In;
    }
  } else {
    TripC = RHS;
  }
  return TripC;
}

// Check if "II" is in a proper loop and loop index is passed as
// argument to II. Return loop trip count that is actually used in
// loop-condition without skipping zero value if all checks are passed.
static Value *getActualTripCountCallBaseInLoop(Instruction *II, Value *LIndex,
                                               LoopInfo &LoopI) {
  Loop *L = LoopI.getLoopFor(II->getParent());
  if (!L)
    return nullptr;
  BasicBlock *Latch = L->getLoopLatch();
  if (!Latch)
    return nullptr;
  PHINode *PN = L->getCanonicalInductionVariable();
  if (!PN)
    return nullptr;

  bool ContinueOnTrue = L->contains(Latch->getTerminator()->getSuccessor(0));
  auto IsValidPredicate = [&](ICmpInst::Predicate Pred) {
    if (ContinueOnTrue)
      return Pred == CmpInst::ICMP_ULT;
    else
      return Pred == CmpInst::ICMP_EQ;
  };

  ICmpInst *Compare = L->getLatchCmpInst();
  if (!Compare || !IsValidPredicate(Compare->getPredicate()) ||
      Compare->hasNUsesOrMore(2))
    return nullptr;
  auto *Increment =
      dyn_cast<BinaryOperator>(PN->getIncomingValueForBlock(Latch));
  if (!Increment || Compare->getOperand(0) != Increment ||
      !Increment->hasNUses(2))
    return nullptr;
  if (PN != LIndex)
    return nullptr;

  // For now, allow only "+1" as loop increment.
  if (Increment->getOpcode() != Instruction::Add)
    return nullptr;
  if (Increment->getOperand(0) != PN)
    return nullptr;
  auto CInc = dyn_cast<ConstantInt>(Increment->getOperand(1));
  if (!CInc || CInc->getSExtValue() != 1)
    return nullptr;
  return Compare->getOperand(1);
}

// Check if "II" is in a proper loop and loop index is passed as
// argument to II. Return loop trip count if all checks are passed.
// Skip constant zero value if there is any.
//
// Ex:
//     for (%i = 0; i% < %size; %i++) {
//       elementAt(%i23, %i)
//     }
Value *PredCandidate::getTripCountCallBaseInLoop(Instruction *II, Value *LIndex,
                                                 LoopInfo &LoopI) {
  Value *ActualTripC = getActualTripCountCallBaseInLoop(II, LIndex, LoopI);
  if (!ActualTripC)
    return nullptr;
  return skipZeroFromTripCount(ActualTripC);
}

// Returns true if Terminator is EH related instructions.
static bool isNoSuccTerminator(Instruction *I) {
  if (isa<UnreachableInst>(I) || isa<ResumeInst>(I))
    return true;
  return false;
}

// Returns RetInst if F has only single RetInst.
static ReturnInst *getSingleRetInst(Function *F) {
  ReturnInst *RI = nullptr;
  for (BasicBlock &BB : *F)
    if (auto *Ret = dyn_cast<ReturnInst>(BB.getTerminator())) {
      if (RI)
        return nullptr;
      else
        RI = Ret;
    }
  return RI;
}

// Returns true if BB doesn't have any side effects.
static bool basicBlockHasNoSideEffects(BasicBlock *BB) {
  for (auto &Inst : *BB) {
    if (Inst.isLifetimeStartOrEnd())
      continue;
    if (isa<DbgInfoIntrinsic>(Inst))
      continue;
    if (Inst.mayHaveSideEffects())
      return false;
  }
  return true;
}

// Check if "CB" doesn't have any side effects even though the callee has
// EH code.
//
//  elementAt(ptr %0, i32 %1) {
//    bb0:
//      %3 = icmp ugt i32 %1, 999
//      br i1 %3, label %4, label %5
//
//    bb1:
//      tail call void @__cxa_rethrow() #7
//      unreachable
//
//    bb2:
//      %6 = zext i32 %1 to i64
//      %7 = getelementptr _ZTS1S.S, ptr %0, i64 %6, i32 2
//      %8 = load i32, ptr %7, align 4, !tbaa !12
//      ret i32 %8
//  }
//
//   contains(%arg) {
//     %i = getelementptr ValueStore, ptr %arg, i64 0, i32 4
//     %i23 = load ptr, ptr %i
//     ...
//     for (%i = 0; i% < 1000; %i++) {
//       elementAt(%i23, %i)
//     }
//   }
//
//   We can prove at compile-time that EH code in elementAt is never
//   executed.
//
bool PredCandidate::checkNoSideEffectsCallWithConstTC(CallBase *CB,
                                                      LoopInfo &LoopI) {
  Function *SpecialDirectCallee = CB->getCalledFunction();
  if (CB->arg_size() < 2 || CB->arg_size() != SpecialDirectCallee->arg_size())
    return false;

  ReturnInst *RI = getSingleRetInst(SpecialDirectCallee);
  if (!RI)
    return false;
  BasicBlock *RetBB = RI->getParent();
  if (!basicBlockHasNoSideEffects(RetBB))
    return false;
  BasicBlock *Pred = RetBB->getSinglePredecessor();
  if (!Pred || Pred != &SpecialDirectCallee->getEntryBlock())
    return false;

  auto *BI = dyn_cast<BranchInst>(Pred->getTerminator());
  if (!BI || !BI->isConditional())
    return false;
  Value *BrCond = BI->getCondition();
  ICmpInst *IC = dyn_cast<ICmpInst>(BrCond);
  if (!IC || BI->getSuccessor(1) != RetBB ||
      IC->getPredicate() != ICmpInst::ICMP_UGT)
    return false;
  if (!basicBlockHasNoSideEffects(Pred))
    return false;

  BasicBlock *FalseBB = BI->getSuccessor(0);
  if (auto *II = dyn_cast<InvokeInst>(FalseBB->getTerminator())) {
    auto *ND = II->getNormalDest();
    auto *UD = II->getUnwindDest();
    // Check no successors for ND and UD.
    if (!isNoSuccTerminator(ND->getTerminator()) ||
        !isNoSuccTerminator(UD->getTerminator()))
      return false;
  } else {
    // Handle Windows's EH case here.
    if (!isa<UnreachableInst>(FalseBB->getTerminator()))
      return false;
  }
  Value *LHS = IC->getOperand(0);
  Value *RHS = IC->getOperand(1);
  auto *A = dyn_cast<Argument>(LHS);
  if (!A)
    return false;
  unsigned ANo = A->getArgNo();
  auto ArgMinVal = dyn_cast<ConstantInt>(RHS);
  if (!ArgMinVal || ArgMinVal->getSExtValue() < 0)
    return false;
  unsigned ArgMin = ArgMinVal->getSExtValue() + 1;

  // Check Call is in a Loop and loop induction variable is passed
  // as ANo of argument to the call.
  Value *TripC = getTripCountCallBaseInLoop(CB, CB->getArgOperand(ANo), LoopI);
  if (!TripC)
    return false;
  auto UB = dyn_cast<ConstantInt>(TripC);
  if (!UB || UB->getSExtValue() < 0)
    return false;
  unsigned LoopIdxMax = UB->getSExtValue() - 1;
  if (LoopIdxMax >= ArgMin)
    return false;
  return true;
}

// Check if "CB" doesn't have any side effects even though the callee has
// EH code.
//
//  elementAt(ptr %arg, i32 %arg1) {
//  bb0:
//    %i = getelementptr BaseRefVectorOf, ptr %arg, i64 0, i32 2
//    %i2 = load i32, ptr %i
//    %i3 = icmp ugt i32 %i2, %arg1
//    br i1 %i3, label %bb11, label %bb4
//
//  bb4:
//    %i5 = tail call ptr @__cxa_allocate_exception(i64 48) #47
//    invoke foo()
//    to label %bb8 unwind label %bb9
//
//  bb8:
//    unreachable
//
//  bb9:
//    resume { ptr, i32 }
//
//  bb11:
//    %i12 = getelementptr BaseRefVectorOf, ptr %arg, i64 0, i32 4
//    %i13 = load ptr, ptr %i12
//    ret ptr %i13
//  }
//
//   contains(%arg) {
//     %i = getelementptr ValueStore, ptr %arg, i64 0, i32 4
//     %i13 = load ptr, ptr %i
//     %i14 = getelementptr BaseRefVectorOf, ptr %i13, i64 0, i32 2
//     %i23 = load ptr, ptr %i
//     %isize = load i32, ptr %i14
//     ...
//     for (%i = 0; i% < %size; %i++) {
//       elementAt(%i23, %i)
//     }
//   }
//
//   We can prove at compile-time that EH code in elementAt is never
//   executed. "%i3" condition in elementAt is always true since upper
//   limit of %arg1 is same as %i2 (which is same as loop trip count in
//   the caller). So, only bb0 and bb11 are verified to prove that "CB"
//   doesn't have any side-effects by skipping all EH code.
//
bool PredCandidate::checkSpecialNoSideEffectsCall(CallBase *CB,
                                                  LoopInfo &LoopI) {
  // If V is argument, returns corresponding operand of "CB" at callsite.
  // Otherwise, just returns V.
  auto GetSourceOperand = [](Value *V, CallBase *CB) -> Value * {
    auto *A = dyn_cast<Argument>(V);
    if (!A || A->getParent() != CB->getCalledFunction())
      return V;
    return CB->getArgOperand(A->getArgNo());
  };

  // This function is used to prove that operands of ICmp instruction in
  // "elementAt" function (in the example) have same value.
  // LHSVec: Sequence of instructions to compute first operand of ICmp.
  // RHSVec: Sequence of instructions to compute upper limit of second
  //         operand of ICmp.
  //
  // Walk though both vectors in reverse order to prove that they are same.
  //
  // LHSVec:
  // %i = getelementptr ValueStore, ptr %arg, i64 0, i32 4
  // %i23 = load ptr, ptr %i
  // %i = getelementptr BaseRefVectorOf, ptr %arg, i64 0, i32 2
  // %i2 = load i32, ptr %i
  //
  // RHSVec:
  // %i = getelementptr ValueStore, ptr %arg, i64 0, i32 4
  // %i13 = load ptr, ptr %i
  // %i14 = getelementptr BaseRefVectorOf, ptr %i13, i64 0, i32 2
  // %i15 = load i32, ptr %i14
  auto ComputeSameResults =
      [&GetSourceOperand](SmallVector<Instruction *, 4> &LHSVec,
                          SmallVector<Instruction *, 4> &RHSVec, CallBase *CB) {
        if (LHSVec.size() != RHSVec.size() || LHSVec.size() == 0)
          return false;

        Value *LHSPrev = nullptr;
        Value *RHSPrev = nullptr;

        for (int32_t I = (int32_t)LHSVec.size() - 1; I >= 0; I--) {
          Value *LHSCurr = LHSVec[I];
          Value *RHSCurr = RHSVec[I];
          Value *LHSCurrOp = nullptr;
          Value *RHSCurrOp = nullptr;

          if (auto *LLI = dyn_cast<LoadInst>(LHSCurr)) {
            auto *RLI = dyn_cast<LoadInst>(RHSCurr);
            if (!RLI || RLI->getType() != LLI->getType())
              return false;
            LHSCurrOp = GetSourceOperand(LLI->getPointerOperand(), CB);
            RHSCurrOp = GetSourceOperand(RLI->getPointerOperand(), CB);
          } else if (auto *LGEP = dyn_cast<GetElementPtrInst>(LHSCurr)) {
            auto *RGEP = dyn_cast<GetElementPtrInst>(RHSCurr);
            if (!RGEP ||
                RGEP->getSourceElementType() != LGEP->getSourceElementType())
              return false;
            if (!std::equal(LGEP->idx_begin(), LGEP->idx_end(),
                            RGEP->idx_begin(), RGEP->idx_end()))
              return false;
            LHSCurrOp = GetSourceOperand(LGEP->getPointerOperand(), CB);
            RHSCurrOp = GetSourceOperand(RGEP->getPointerOperand(), CB);
          } else {
            return false;
          }
          if (LHSPrev && RHSPrev) {
            if (LHSPrev != LHSCurrOp || RHSPrev != RHSCurrOp)
              return false;
          } else {
            if (LHSCurrOp != RHSCurrOp)
              return false;
          }
          LHSPrev = LHSCurr;
          RHSPrev = RHSCurr;
        }
        return true;
      };

  Function *SpecialDirectCallee = CB->getCalledFunction();
  if (CB->arg_size() < 2 || CB->arg_size() != SpecialDirectCallee->arg_size())
    return false;

  BasicBlock *EBB = nullptr;
  SmallPtrSet<BasicBlock *, 4> EHBBs;
  // Try to find first basic block that has EH related code.
  // Currently checking for EH-safety only if EH related blocks
  // are controlled under one condition. If there are more EH blocks,
  // this function returns false when remaining EH related blocks
  // are processed by basicBlockHasNoSideEffects.
  for (BasicBlock &BB : *SpecialDirectCallee) {
    if (auto *II = dyn_cast<InvokeInst>(BB.getTerminator())) {
      EBB = &BB;
      auto *ND = II->getNormalDest();
      auto *UD = II->getUnwindDest();
      // Check no successors for ND and UD.
      if (!isNoSuccTerminator(ND->getTerminator()) ||
          !isNoSuccTerminator(UD->getTerminator()))
        return false;
      ;
      EHBBs.insert(&BB);
      EHBBs.insert(ND);
      EHBBs.insert(UD);
      break;
    } else if (isa<UnreachableInst>(BB.getTerminator())) {
      EBB = &BB;
      EHBBs.insert(&BB);
      break;
    }
  }
  if (!EBB)
    return false;

  for (BasicBlock &BB : *SpecialDirectCallee) {
    // Ignore already processed EH blocks.
    if (EHBBs.count(&BB))
      continue;
    if (!basicBlockHasNoSideEffects(&BB))
      return false;
  }
  BasicBlock *Pred = EBB->getSinglePredecessor();
  if (!Pred)
    return false;
  auto *BI = dyn_cast<BranchInst>(Pred->getTerminator());
  if (!BI || !BI->isConditional())
    return false;
  Value *BrCond = BI->getCondition();
  ICmpInst *IC = dyn_cast<ICmpInst>(BrCond);
  if (!IC || BI->getSuccessor(1) != EBB ||
      IC->getPredicate() != ICmpInst::ICMP_UGT)
    return false;

  SmallVector<Instruction *, 4> CmpLHSInsts;
  SmallVector<Instruction *, 4> CmpRHSInsts;

  Value *LHS = IC->getOperand(0);
  Value *RHS = IC->getOperand(1);
  if (!isa<Argument>(RHS))
    return false;
  if (!isa<LoadInst>(LHS))
    return false;
  if (!getNeededInstsToCompute(LHS, CmpLHSInsts, true))
    return false;
  Instruction *FirstInst = *CmpLHSInsts.rbegin();
  auto *GEP0 = dyn_cast<GetElementPtrInst>(FirstInst);
  if (!GEP0)
    return false;
  auto *Arg0 = dyn_cast<Argument>(GEP0->getPointerOperand());
  if (!Arg0)
    return false;
  Value *LHSArg = CB->getArgOperand(Arg0->getArgNo());
  if (!getNeededInstsToCompute(LHSArg, CmpLHSInsts, true))
    return false;

  // Check Call is in a Loop and loop induction variable is passed
  // as second argument to the call.
  Value *RHSActualTripC =
      getActualTripCountCallBaseInLoop(CB, CB->getArgOperand(1), LoopI);
  if (!RHSActualTripC)
    return false;
  Value *RHSArgTripC = skipZeroFromTripCount(RHSActualTripC);
  if (!RHSArgTripC)
    return false;

  // Get all instructions needed to compute loop trip count.
  if (!getNeededInstsToCompute(RHSArgTripC, CmpRHSInsts, true))
    return false;

  // Prove that ICmp instruction always returns true.
  if (ComputeSameResults(CmpLHSInsts, CmpRHSInsts, CB))
    return true;

  // Ex:
  // unsigned int otherSize = other->size();
  // ...
  // for (...) {
  //   S1* Obj = fValueTuples->elementAt(i);
  //   if (otherSize == Obj->size()) {
  //     for (unsigned int j=0; j<otherSize; j++) {
  //       if (!Obj->foo(j)
  //       ...
  //     }
  //   }
  //
  // foo(S1* Obj, int pos) {
  //   if (pos >= Obj->size())
  //     throw;
  // }
  //
  // To prove that foo is EH-safe at compile time, otherSize as trip
  // count doesn't help. But, the loop is controlled under
  // "if (otherSize == Obj->size())". So, Obj->size() can be used
  // as trip count to prove foo is EH-safe.
  CmpRHSInsts.clear();
  Loop *L = LoopI.getLoopFor(CB->getParent());
  BasicBlock *PHead = L->getLoopPredecessor();
  if (!PHead)
    return false;
  auto *BII = dyn_cast<BranchInst>(PHead->getTerminator());
  if (!BII || !BII->isConditional())
    return false;
  ICmpInst *Cond = dyn_cast<ICmpInst>(BII->getCondition());
  if (!Cond)
    return false;
  if (Cond->getPredicate() != ICmpInst::ICMP_EQ ||
      BII->getSuccessor(0) != CB->getParent())
    return false;
  Value *CmpOp0 = Cond->getOperand(0);
  Value *CmpOp1 = Cond->getOperand(1);
  Value *NewTripC = nullptr;
  if (CmpOp0 == RHSActualTripC)
    NewTripC = CmpOp1;
  else if (CmpOp1 == RHSActualTripC)
    NewTripC = CmpOp0;
  if (!NewTripC)
    return false;
  NewTripC = skipZeroFromTripCount(NewTripC);
  if (!NewTripC)
    return false;
  if (!getNeededInstsToCompute(NewTripC, CmpRHSInsts, true))
    return false;

  // Prove that ICmp instruction always returns true.
  if (!ComputeSameResults(CmpLHSInsts, CmpRHSInsts, CB))
    return false;

  return true;
}

// Collect all needed instructions to compute "V". Only GetElementPtrInst,
// LoadInsts and ZExt/SExt are allowed. Stops when it reaches Argument.
// When NotForHoisting is true, allow CallBase instructions as terminals
// in addition to arguments.
bool PredCandidate::getNeededInstsToCompute(
    Value *V, SmallVectorImpl<Instruction *> &HoistingInsts,
    bool NotForHoisting = false) {
  int NumInsts = 0;

  if (isa<Argument>(V))
    return true;
  if (NotForHoisting && isa<CallBase>(V))
    return true;

  // Limit up to 5 instructions at most.
  while (NumInsts < 6) {
    if (auto *LI = dyn_cast<LoadInst>(V)) {
      HoistingInsts.push_back(LI);
      V = LI->getPointerOperand();
      if (isa<Argument>(V))
        return true;
      if (NotForHoisting && isa<CallBase>(V))
        return true;
    } else if (auto *GEP = dyn_cast<GetElementPtrInst>(V)) {
      // Non-constant indexes are not allowed.
      if (!GEP->hasAllConstantIndices())
        return false;
      HoistingInsts.push_back(GEP);
      V = GEP->getPointerOperand();
      if (isa<Argument>(V))
        return true;
      if (NotForHoisting && isa<CallBase>(V))
        return true;
    } else if (auto *ZExt = dyn_cast<ZExtInst>(V)) {
      HoistingInsts.push_back(ZExt);
      V = ZExt->getOperand(0);
    } else if (auto *SExt = dyn_cast<SExtInst>(V)) {
      HoistingInsts.push_back(SExt);
      V = SExt->getOperand(0);
    } else {
      return false;
    }
    NumInsts++;
  }
  return (HoistingInsts.size() != 0 && NumInsts < 6);
}

bool PredCandidate::getBBControlConditions(
    BasicBlock *BB, SmallVectorImpl<ControlCond> &Conditions) {

  Function *F = BB->getParent();
  DominatorTree &CalleeDT = DTGetter(*F);
  PostDominatorTree &CalleePDT = PDTGetter(*F);

  BasicBlock *EntryBB = &BB->getParent()->getEntryBlock();
  BasicBlock *CurrBB = BB;
  int NumConditions = 0;
  while (CurrBB != EntryBB) {
    if (!CalleeDT.getNode(CurrBB))
      return false;
    BasicBlock *CurrDom = CalleeDT.getNode(CurrBB)->getIDom()->getBlock();
    if (!CurrDom)
      return false;
    const BranchInst *BI = dyn_cast<BranchInst>(CurrDom->getTerminator());

    if (!BI || !BI->isConditional() || !isa<ICmpInst>(BI->getCondition()))
      return false;
    // For now, allow only ICmp with one constant operand.
    Value *V = nullptr;
    Constant *CompC = nullptr;
    getValueConstant(cast<ICmpInst>(BI->getCondition()), &V, &CompC);
    if (!V || !CompC)
      return false;

    if (CalleePDT.dominates(CurrBB, BI->getSuccessor(0)))
      Conditions.push_back(ControlCond(BI->getCondition(), true));
    else if (CalleePDT.dominates(CurrBB, BI->getSuccessor(1)))
      Conditions.push_back(ControlCond(BI->getCondition(), false));
    else
      return false;
    if (++NumConditions > 2)
      return false;
    CurrBB = CurrDom;
  }
  return true;
}

// Returns true if CB is marked with "dtrans-vector-size-field=1" attribute.
bool PredCandidate::isDTransVectorAccessElemCall(CallBase *CB) {
  Function *VCallee = CB->getCalledFunction();
  if (!VCallee)
    return false;
  Attribute SizeFieldAttr = VCallee->getFnAttribute("dtrans-vector-size-field");
  if (!SizeFieldAttr.isValid())
    return false;
  if (VCallee->arg_size() != 2)
    return false;
  return true;
}

// Makes sure direct calls have no side effects.
bool PredCandidate::processDirectCalls(IPPredOptImpl &IPPredObj,
                                       SmallPtrSet<CallBase *, 6> &DirectCalls,
                                       LoopInfo &LoopI) {

  for (auto *CB : DirectCalls) {
    LLVM_DEBUG(dbgs() << "      Checking direct call for no side effects:"
                      << *CB << "\n";);

    Function *Callee = CB->getCalledFunction();
    if (funcHasNoSideEffects(Callee))
      continue;

    // Check if CB is special case when callee has EH code.
    if (!checkSpecialNoSideEffectsCall(CB, LoopI) &&
        !checkNoSideEffectsCallWithConstTC(CB, LoopI))
      return false;
  }
  return true;
}

// For each indirect call, find all possible target functions and no action
// is required if there are no side effects. If there are possible targets
// with side effects, find most probable target, which will be used to generate
// runtime check later, using base class heuristic.
//
bool PredCandidate::processIndirectCalls(
    IPPredOptImpl &IPPredObj, SmallPtrSet<CallBase *, 2> &IndirectCalls,
    LoopInfo &LoopI) {
  // GetCallFirstArgTyMD and GetFunctionFirstParamTyMD are implemented
  // without using MDReader by just using DTrans's metadata that is
  // attached to calls and functions without computing the DTrans's types.

  // Returns DTrans's metadata of first argument of CB if it is found.
  // Otherwise, returns nullptr.
  auto GetCallFirstArgTyMD = [](CallBase *CB) -> MDNode * {
    // Check if type of first argument is pointer.
    if (CB->arg_size() < 1 || !CB->getArgOperand(0)->getType()->isPointerTy())
      return nullptr;

    MDNode *MD = CB->getMetadata("intel_dtrans_type");
    if (!MD)
      return nullptr;

    auto *MDS = dyn_cast<MDString>(MD->getOperand(0));
    if (!MDS)
      return nullptr;
    if (!MDS->getString().equals("F"))
      return nullptr;

    // Get metadata of first argument.
    const unsigned NumArgsPos = 2;
    const unsigned ArgTyStartPos = 4;
    if (MD->getNumOperands() < ArgTyStartPos)
      return nullptr;
    auto *NumArgsMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(NumArgsPos));
    if (!NumArgsMD)
      return nullptr;
    unsigned ArgCount =
        cast<ConstantInt>(NumArgsMD->getValue())->getZExtValue();
    unsigned NumOps = MD->getNumOperands();
    if (NumOps != ArgTyStartPos + ArgCount)
      return nullptr;
    auto *ArgTyMD = dyn_cast<MDNode>(MD->getOperand(ArgTyStartPos));
    if (!ArgTyMD)
      return nullptr;

    return ArgTyMD;
  };

  // Returns DTrans's metadata of first argument of TF if it is found.
  // Otherwise, returns nullptr.
  auto GetFunctionFirstParamTyMD = [](Function *TF) -> MDNode * {
    // Check if type of first argument is pointer.
    if (TF->arg_size() < 1 || !TF->getArg(0)->getType()->isPointerTy())
      return nullptr;

    auto *MDTypeListNode = TF->getMetadata("intel.dtrans.func.type");
    if (!MDTypeListNode)
      return nullptr;
    AttributeList Attrs = TF->getAttributes();
    AttributeSet ParamAttrs = Attrs.getParamAttrs(0);
    Attribute Attr = ParamAttrs.getAttribute("intel_dtrans_func_index");
    if (!Attr.isValid())
      return nullptr;
    StringRef TagName = Attr.getValueAsString();
    uint64_t Index;
    if (TagName.getAsInteger(10, Index))
      return nullptr;
    auto *TypeNode = dyn_cast<MDNode>(MDTypeListNode->getOperand(Index - 1));
    if (!TypeNode)
      return nullptr;

    return TypeNode;
  };

  // Returns object that is used for virtual indirect call.
  //
  // Ex: For given callsite i110, returns %i41.
  //
  //  %i105 = getelementptr %Validator, ptr %i41, i64 0, i32 0, i32 0
  //  %i106 = load ptr, ptr %i105, align 8, !tbaa !1256
  //  %i107 = tail call i1 @llvm.type.test(ptr %i106, metadata !"Validator")
  //  tail call void @llvm.assume(i1 %i107)
  //  %i108 = getelementptr inbounds ptr, ptr %i106, i64 10
  //  %i109 = load ptr, ptr %i108, align 8
  //  %i110 = tail call noundef i32 %i109(ptr %i41)
  //
  auto ProcessVirtualFunctionLoads =
      [](CallBase *CB,
         SmallVector<Instruction *, 8> &FPtrHoistInst) -> Value * {
    Instruction *PrevI = CB->getPrevNonDebugInstruction();
    auto *LI = dyn_cast_or_null<LoadInst>(PrevI);
    if (!LI || PrevI != CB->getCalledOperand())
      return nullptr;
    FPtrHoistInst.push_back(LI);
    Instruction *TempI = nullptr;
    Value *TempIOp = nullptr;
    auto *VGEP =
        dyn_cast_or_null<GetElementPtrInst>(LI->getPrevNonDebugInstruction());
    // Allow VTable load pattern even when GEP (i.e %i108 in the example)
    // is eliminated.
    if (VGEP) {
      if (!VGEP->hasAllConstantIndices())
        return nullptr;
      FPtrHoistInst.push_back(VGEP);
      TempI = VGEP;
      TempIOp = VGEP->getPointerOperand();
    } else {
      TempI = LI;
      TempIOp = LI->getPointerOperand();
    }
    auto *AI =
        dyn_cast_or_null<IntrinsicInst>(TempI->getPrevNonDebugInstruction());
    if (!AI || AI->getIntrinsicID() != Intrinsic::assume)
      return nullptr;
    auto *TI =
        dyn_cast_or_null<IntrinsicInst>(AI->getPrevNonDebugInstruction());
    if (!TI || TI->getIntrinsicID() != Intrinsic::type_test)
      return nullptr;
    auto *VTLI = dyn_cast<LoadInst>(TempIOp);
    if (!VTLI)
      return nullptr;
    FPtrHoistInst.push_back(VTLI);
    auto *GEP = dyn_cast<GetElementPtrInst>(VTLI->getPointerOperand());
    if (!GEP || !GEP->hasAllZeroIndices())
      return nullptr;
    FPtrHoistInst.push_back(GEP);

    return GEP->getPointerOperand();
  };

  // Returns true if FPtr is a direct call with "dtrans-vector-size-field=1"
  // attribute.
  //
  // Ex: Let us assume @bar is marked with "dtrans-vector-size-field=1"
  // attribute.
  //
  //  %i39 = call ptr @bar(ptr %i24, i32 %i38)
  //
  auto IsDTransVectorAccessElemCall = [this](Value *FPtr) {
    auto *VCall = dyn_cast<CallBase>(FPtr);
    if (!VCall)
      return false;
    if (!isDTransVectorAccessElemCall(VCall))
      return false;
    if (isa<Argument>(VCall->getArgOperand(0)))
      return true;
    return false;
  };

  // Try to find the object that is used to call virtual function for given
  // indirect call "CB".
  //
  // Case 1 (bb104): This function detects %i41 is the object that is used
  // for indirect call. Functionality of @bar is known as it is marked
  // with "dtrans-vector-size-field". Returns %i41 as first param of the call
  // is an argument (i.e %arg1) that can be hoisted to the beginning of the
  // routine.
  //
  // Case 2 (bb96): This function detects %i39 is the object that is used
  // for indirect call. Functionality of @bar is known as it is marked
  // with "dtrans-vector-size-field" but first param of the call (i.e %i24)
  // is not argument. Looking at the CFG, BB94 is executed only when %i39
  // and %i41 are same. So, returns %i41 since %i41 (instead of %i39) can
  // be used to generate runtime checks for %i101 call also.
  //
  // define i1 @foo(ptr %arg, ptr %arg1) {
  //   ...
  // bb1:
  //   %i39 = call ptr @bar(ptr %i24, i32 %i38)
  //   %i41 = call ptr @bar(ptr %arg1, i32 %i38)
  //   ...
  // bb94:                                             ; preds = %bb92
  //   %i95 = icmp eq ptr %i39, %i41
  //   br i1 %i95, label %bb96, label %bb104
  //
  // bb96:  ; preds = %bb94 ; Case 2
  //   %i97 = getelementptr %Validator, ptr %i39, i64 0, i32 0, i32 0
  //   %i98 = load ptr, ptr %i97, align 8, !tbaa !1256
  //   %i100 = getelementptr ptr, ptr %i98, i64 10
  //   %i101 = load ptr, ptr %i100, align 8
  //   %i102 = call i32 %i101(ptr %i39)
  //
  // bb104:  ; preds = %bb94 ; Case 1
  //  %i105 = getelementptr %Validator, ptr %i41, i64 0, i32 0, i32 0
  //  %i106 = load ptr, ptr %i105, align 8, !tbaa !1256
  //  %i108 = getelementptr inbounds ptr, ptr %i106, i64 10
  //  %i109 = load ptr, ptr %i108, align 8
  //  %i110 = call i32 %i109(ptr %i41)
  //  ...
  // }
  auto GetValidIndirectCallObj =
      [&IsDTransVectorAccessElemCall, &ProcessVirtualFunctionLoads](
          CallBase *CB,
          SmallVector<Instruction *, 8> &FPtrHoistInst) -> Value * {
    // Check for case 1.
    Value *FPtr = ProcessVirtualFunctionLoads(CB, FPtrHoistInst);
    if (!FPtr)
      return nullptr;
    if (IsDTransVectorAccessElemCall(FPtr)) {
      FPtrHoistInst.push_back(cast<CallBase>(FPtr));
      return FPtr;
    }

    // Check for case 2.
    BasicBlock *CurrBB = CB->getParent();
    BasicBlock *PredBB = CurrBB->getSinglePredecessor();
    if (!PredBB)
      return nullptr;
    auto *BBI = dyn_cast<BranchInst>(PredBB->getTerminator());
    if (!BBI || !BBI->isConditional())
      return nullptr;
    ICmpInst *IC = dyn_cast<ICmpInst>(BBI->getCondition());
    if (!IC || IC->getPredicate() != ICmpInst::ICMP_EQ ||
        BBI->getSuccessor(0) != CurrBB)
      return nullptr;
    Value *NewFPtr = nullptr;
    if (IC->getOperand(0) == FPtr)
      NewFPtr = IC->getOperand(1);
    else if (IC->getOperand(1) == FPtr)
      NewFPtr = IC->getOperand(0);
    if (!NewFPtr)
      return nullptr;
    if (IsDTransVectorAccessElemCall(NewFPtr)) {
      FPtrHoistInst.push_back(cast<CallBase>(NewFPtr));
      return FPtr;
    }
    return nullptr;
  };

  // Find most probable targets using heuristics.
  //   1. No Side effects.
  //   2. No base class virtual function
  //   3. Target function has only one loop.
  auto GetMostProbableTargetFunctions =
      [this, &GetCallFirstArgTyMD, &GetFunctionFirstParamTyMD](
          SmallVectorImpl<Function *> &TargetFunctions, CallBase *CB) -> bool {
    // Get DTrans's metadata for 1st argument of call.
    MDNode *ArgTyMD = GetCallFirstArgTyMD(CB);
    if (!ArgTyMD)
      return false;

    bool FoundTarget = false;
    for (auto *TF : TargetFunctions) {
      if (TF->isDeclaration()) {
        PossibleTargetsWithSE.insert(TF);
        continue;
      }

      // Skip target that has side effects.
      if (!funcHasNoSideEffects(TF)) {
        PossibleTargetsWithSE.insert(TF);
        continue;
      }

      if (!IPPredSkipIndirectTargetHeuristic) {
        // Get DTrans's metadata for 1st argument of TF.
        MDNode *TypeNode = GetFunctionFirstParamTyMD(TF);
        // Skip virtual function in base class
        if (TypeNode == ArgTyMD)
          continue;

        // Check if target has one loop.
        LoopInfo &LoopI = (GetLI)(*TF);
        if (llvm::size(LoopI) != 1)
          continue;
      }

      SelectedPossibleTargets.insert(TF);
      FoundTarget = true;
    }

    // Don't generate runtime checks if number of possible targets with SE is
    // more than or equal to number of possible targets with NSE.
    if (IPPredSkipIndirectTargetHeuristic &&
        SelectedPossibleTargets.size() <= PossibleTargetsWithSE.size())
      return false;

    return FoundTarget;
  };

  Value *IndirectFunctionPtr = nullptr;

  for (auto *CB : IndirectCalls) {
    LLVM_DEBUG(dbgs() << "      Checking indirect call for no side effects:"
                      << *CB << "\n";);
    // Find all possible target functions.
    SmallVector<Function *, 16> TargetFunctions;
    if (!IPPredObj.getVirtualPossibleTargets(*CB, TargetFunctions))
      return false;

    bool NoSideEffects = true;
    for (auto *F : TargetFunctions)
      if (!funcHasNoSideEffects(F)) {
        NoSideEffects = false;
        break;
      }

    // No action is needed if there are no side effects.
    if (NoSideEffects)
      continue;

    // Try to find most probable target functions.
    if (!GetMostProbableTargetFunctions(TargetFunctions, CB)) {
      LLVM_DEBUG(dbgs() << "      No targets selected by heuristics");
      return false;
    }

    // Trying to collect all instructions that are needed to hoist
    // and generate runtime check for computing function pointer.
    SmallVector<Instruction *, 8> FPtrHoistInst;
    Value *FPtr = GetValidIndirectCallObj(CB, FPtrHoistInst);
    if (FPtr) {
      // Handles case where object is returned from a function call
      // that is marked with "dtrans-vector-size-field".
      Function *VCallee = cast<CallBase>(FPtr)->getCalledFunction();
      Attribute SizeFieldAttr =
          VCallee->getFnAttribute("dtrans-vector-size-field");
      if (!SizeFieldAttr.isValid())
        return false;
      StringRef Val = SizeFieldAttr.getValueAsString();
      unsigned SizeField = UINT32_MAX;
      if (Val.getAsInteger(0, SizeField))
        return false;

      auto *GetElemCB = cast<CallBase>(FPtr);
      if (GetElemCB->arg_size() != 2)
        return false;

      if (!isa<Argument>(GetElemCB->getArgOperand(0)))
        continue;
    } else {
      // Handles a simple where object is loaded from  a function argument
      // pointer.
      //   %17 = phi i64 [ 0, %10 ], [ %13, %12 ]
      //   %18 = getelementptr inbounds ptr, ptr %0, i64 %17
      //   %19 = load ptr, ptr %18
      //   %20 = getelementptr %struct._ZTS1S.S, ptr %19, i64 0, i32 0
      //   %21 = load ptr, ptr %20
      //   %22 = tail call i1 @llvm.type.test(ptr %21, metadata !"_ZTS1S")
      //   tail call void @llvm.assume(i1 %22)
      //   %23 = load ptr, ptr %21
      //   %24 = tail call i1 %23(ptr %19)
      //
      FPtrHoistInst.clear();
      FPtr = ProcessVirtualFunctionLoads(CB, FPtrHoistInst);
      if (!FPtr)
        return false;
      auto *FPtrLI = dyn_cast<LoadInst>(FPtr);
      if (!FPtrLI)
        return false;
      auto *FPtrGEP = dyn_cast<GetElementPtrInst>(FPtrLI->getPointerOperand());
      if (!FPtrGEP)
        return false;
      if (FPtrGEP->getNumIndices() != 1)
        return false;
      if (!isa<Argument>(FPtrGEP->getPointerOperand()))
        return false;
      FPtrHoistInst.push_back(FPtrLI);
      FPtrHoistInst.push_back(FPtrGEP);
    }

    if (!IndirectFunctionPtr) {
      IndirectFunctionPtr = FPtr;
      IndirectFPtrHoistInst = FPtrHoistInst;
    } else if (IndirectFunctionPtr != FPtr) {
      return false;
    }
  }
  // For now, don't skip transformation if no possible target is found when
  // IPPredSkipCalleeLegalChecks.
  if (IPPredSkipCalleeLegalChecks && SelectedPossibleTargets.size() == 0)
    return true;

  if (!IndirectFunctionPtr)
    return true;

  if (SelectedPossibleTargets.size() > 8)
    return false;

  LLVM_DEBUG(dbgs() << "      IndirectCallObj:" << *IndirectFunctionPtr
                    << "\n";);
  LLVM_DEBUG({
    for (auto *MPF : SelectedPossibleTargets)
      dbgs() << "      Most probable Target:" << MPF->getName() << "\n";
  });

  auto *GetElemCB = dyn_cast<CallBase>(IndirectFunctionPtr);
  Instruction *II;
  Value *LIndex;
  if (GetElemCB) {
    if (GetElemCB->arg_size() != 2)
      return false;
    II = GetElemCB;
    LIndex = GetElemCB->getArgOperand(1);
  } else {
    // Handles a simple where object is loaded from  a function argument
    // pointer.
    //   %17 = phi i64 [ 0, %10 ], [ %13, %12 ]
    //   %18 = getelementptr inbounds ptr, ptr %0, i64 %17
    //   %19 = load ptr, ptr %18
    II = cast<LoadInst>(IndirectFunctionPtr);
    Instruction *FirstI = *IndirectFPtrHoistInst.rbegin();
    LIndex = cast<GetElementPtrInst>(FirstI)->getOperand(1);
  }

  // Get Loop trip count if the call is in a loop and loop index is passed as
  // second argument to the call.
  Value *TripC = getTripCountCallBaseInLoop(II, LIndex, LoopI);
  if (!TripC)
    return false;
  // Check if TripC is constant and greater than 0.
  auto UB = dyn_cast<ConstantInt>(TripC);
  if (UB) {
    if (UB->getSExtValue() > 0)
      return true;
    return false;
  }

  auto *TripI = dyn_cast<Instruction>(TripC);
  if (!TripI)
    return false;

  // The loop trip count is used to generate a runtime check.
  // Get all conditions that control to compute loop trip count.
  // These conditions will be generated as runtime checks later.
  if (!getBBControlConditions(TripI->getParent(), CalleeControlConds))
    return false;

  // Get all needed instructions to compute loop trip count.
  if (!getNeededInstsToCompute(TripC, ElemCallTripCHoistInst))
    return false;

  // Makes sure all control conditions are nullptr checks and then collects
  // all needed instructions to hoist these conditions.
  for (auto I = CalleeControlConds.rbegin(), E = CalleeControlConds.rend();
       I != E; I++) {
    ControlCond *C = &*I;
    auto *IC = cast<ICmpInst>(C->getPointer());

    Value *V = nullptr;
    Constant *CompC = nullptr;
    getValueConstant(IC, &V, &CompC);
    assert(V && CompC && "Unexpected ICmpInst");
    if (!CompC->isNullValue() || IC->getPredicate() != ICmpInst::ICMP_EQ)
      return false;
    if (CalleeNullChecks.find(V) != CalleeNullChecks.end())
      return false;

    // Collect needed instructions to compute "V".
    SmallVector<Instruction *, 5> HoistNullCheckInsts;
    if (!getNeededInstsToCompute(V, HoistNullCheckInsts))
      return false;

    // Skip transformation if there are no instructions are collected
    // for nullptr checks.
    if (HoistNullCheckInsts.size() == 0)
      return false;
    CalleeNullChecks[V] = HoistNullCheckInsts;
  }

  // Skip transformation if there are no instructions are collected
  // for trip count.
  if (ElemCallTripCHoistInst.size() == 0)
    return false;

  return true;
}

// Checks if CondCall has any side effects. Returns true if CondCall
// doesn't have any side effects or runtime checks can be used to avoid
// side effects.
bool PredCandidate::checkCondCallSideEffects(IPPredOptImpl &IPPredObj) {
  assert(CondCall && "Expected valid CondCall");
  Function *Callee = CondCall->getCalledFunction();
  assert(Callee && "Expected direct call");

  if (funcHasNoSideEffects(Callee))
    return true;

  SmallPtrSet<CallBase *, 6> DirectCalls;
  SmallPtrSet<CallBase *, 2> IndirectCalls;
  LoopInfo &LoopI = (GetLI)(*Callee);

  for (Instruction &Inst : instructions(*Callee)) {
    if (!Inst.mayThrow() && !Inst.mayWriteToMemory())
      continue;

    auto *CB = dyn_cast<CallBase>(&Inst);
    if (!CB)
      return false;
    Function *TargetF = CB->getCalledFunction();
    if (TargetF)
      DirectCalls.insert(CB);
    else
      IndirectCalls.insert(CB);
  }

  if (!processIndirectCalls(IPPredObj, IndirectCalls, LoopI)) {
    LLVM_DEBUG(dbgs() << "      Failed \n";);
    return false;
  }

  if (!processDirectCalls(IPPredObj, DirectCalls, LoopI)) {
    LLVM_DEBUG(dbgs() << "      Failed \n";);
    return false;
  }

  return true;
}

// Get Value and Constant if "IC" is either "ICmp V, C" or "ICmp C, V".
void PredCandidate::getValueConstant(ICmpInst *IC, Value **VPtr,
                                     Constant **CPtr) {
  auto *C1 = dyn_cast<Constant>(IC->getOperand(0));
  auto *C2 = dyn_cast<Constant>(IC->getOperand(1));
  Value *V = nullptr;
  Constant *C = nullptr;
  if (C1 && !C2) {
    V = IC->getOperand(1);
    C = C1;
  } else if (!C1 && C2) {
    V = IC->getOperand(0);
    C = C2;
  }
  if (!V || !C)
    return;
  *VPtr = V;
  *CPtr = C;
}

// If there are any operands of "Cloned" instruction are also hoisted, replace
// them with newly hoisted instructions using "InstCloneInstMap".
void PredCandidate::replaceOperandsWithClonedInst(
    Instruction *Cloned,
    SmallDenseMap<Instruction *, Instruction *, 8> &InstCloneInstMap) {
  for (Value *Op : Cloned->operands()) {
    auto *II = dyn_cast<Instruction>(Op);
    if (!II)
      continue;
    // Ignore if operand is not hoisted.
    if (!InstCloneInstMap.contains(II))
      continue;
    Cloned->replaceUsesOfWith(Op, InstCloneInstMap[II]);
  }
}

// This routine handles all transformations needed to hoist conditions.
//
// Before:
//   bb314:
//     %i315 = call contains();
//     br i1 %i315, label %bb316, label %bb347
//
//   bb316:
//     %Cond1 = some_def;
//     br i1 %Cond1, label %bb347, label %bb320
//
//   bb320:
//     %Cond2 = some_def;
//     br i1 %Cond2, label %bb339, label %bb329
//
//   bb329:
//     %Cond3 = some_def;
//     br il %Cond3 label %bb347, label %bb331
//
//   bb331:
//     ExecutionBB1;
//     br %bb347
//
//   bb339:
//     ExecutionBB2;
//     br %bb347
//
//   bb347:
//
// After:
//   NewBB:
//    %NewC = (!Cond1 && Cond2) || (!Cond1 && !Cond2 && !Cond3)
//    br il %NewC, lable %bb314, %bb347
//
//   bb314:
//     %i315 = call contains();
//     br i1 %i315, label %bb316, label %bb347
//
//   bb316:
//     %Cond1 = some_def;
//     br i1 %Cond1, label %bb347, label %bb320
//
//   bb320:
//     %Cond2 = some_def;
//     br i1 %Cond2, label %bb339, label %bb329
//
//   bb329:
//     %Cond3 = some_def;
//     br il %Cond3 label %bb347, label %bb331
//
//   bb331:
//     ExecutionBB1;
//     br %bb347
//
//   bb339:
//     ExecutionBB2;
//     br %bb347
//
//   bb347:
//
void PredCandidate::hoistConditions() {

  // BBHoistCondsMap provides all necessary instructions that need to be hoisted
  // for given "V" condition value. "IC" is the main ICmpInst that needs to be
  // hoisted. "CondFlag" indicates whether predicate should be inversed.
  auto HoistNecessaryInst = [&](Value *V, ICmpInst *IC, bool CondFlag,
                                IRBuilder<> &B) -> Value * {
    SmallDenseMap<Instruction *, Instruction *, 8> InstCloneInstMap;
    Instruction *Cloned = nullptr;

    // Hoist all necessary instructions and fix operands with cloned
    // instructions.
    for (auto I = BBHoistCondsMap[V].rbegin(), E = BBHoistCondsMap[V].rend();
         I != E; I++) {
      Instruction *CV = *(&*I);
      Cloned = B.Insert(CV->clone());
      InstCloneInstMap[CV] = Cloned;
      replaceOperandsWithClonedInst(Cloned, InstCloneInstMap);
    }
    assert(Cloned && "Expected Cloned instruction");
    assert(isa<ICmpInst>(IC) && "Expected ICmpInst");

    Instruction *NewIC = B.Insert(IC->clone());
    // Replace original value with the hoisted value.
    if (IC->getOperand(0) == V)
      NewIC->setOperand(0, Cloned);
    else
      NewIC->setOperand(1, Cloned);
    if (!CondFlag) {
      auto NewCmpInst = cast<ICmpInst>(NewIC);
      NewCmpInst->setPredicate(
          ICmpInst::getInversePredicate(NewCmpInst->getPredicate()));
    }
    return NewIC;
  };

  // Split block to make CondCall as first instruction in the block before
  // applying any transformations.
  BasicBlock *BB = CondCall->getParent();
  if (CondCall != BB->getFirstNonPHIOrDbgOrLifetime())
    BB->splitBasicBlock(CondCall->getIterator());

  // Create insertion pointer just before CondCall.
  IRBuilder<> B(CondCall);

  bool FirstExeBBCond = true;
  Value *FinalCond = nullptr;
  for (auto *BB : ExecutedBlocks) {
    bool FirstCond = true;
    Value *ResultCond = nullptr;
    // All conditions that control executed block are hoisted. Generate
    // necessary instructions and conditions just before the CondCall.
    // Process conditions of BBControlCondsMap[BB] in reverse order as the
    // control conditions are collected from bottom to top.
    for (auto I = BBControlCondsMap[BB].rbegin(),
              E = BBControlCondsMap[BB].rend();
         I != E; I++) {
      ControlCond *C = &*I;

      auto *IC = cast<ICmpInst>(C->getPointer());
      Value *V = nullptr;
      Constant *CompC = nullptr;
      getValueConstant(IC, &V, &CompC);
      assert(V && CompC && "Unexpected ICmpInst");

      CondTy CTy = CondTypeMap[V];
      assert(CTy != CT_Bottom && "Unexpected Condition type");

      Value *NewC;
      if (CTy == CT_Temp) {
        // No need to hoist temps as it already proved that the temp can be
        // hoisted to the CondCall.
        NewC = V;
      } else {
        assert((CTy == CT_SimpleLoad || CTy == CT_VtableFieldLoad) &&
               "Unexpected Condition type");
        NewC = HoistNecessaryInst(V, IC, C->getInt(), B);
      }

      // Skip generting LogicalAND for first time.
      // Otherwise, generate condtions like "!Cond1 && Cond2"
      if (FirstCond)
        ResultCond = NewC;
      else
        ResultCond = B.CreateLogicalAnd(ResultCond, NewC);
      FirstCond = false;
    }

    // Skip generating LogicalOr for first time.
    // Otherwise, generate conditions like "(!Cond1 && Cond2) || (!Cond1 &&
    // !Cond2 && !Cond3)"
    if (FirstExeBBCond)
      FinalCond = ResultCond;
    else
      FinalCond = B.CreateLogicalOr(FinalCond, ResultCond);
    FirstExeBBCond = false;
  }
  assert(FinalCond && "Expected Final Cond");

  // Fix CFG to control CondCall execution under newly hoisted conditions.
  BasicBlock *OrigCallBB = CondCall->getParent();
  BasicBlock *NewThenBB = OrigCallBB->splitBasicBlock(CondCall->getIterator());
  BranchInst *NewBr = BranchInst::Create(NewThenBB, ExitBB, FinalCond);
  ReplaceInstWithInst(OrigCallBB->getTerminator(), NewBr);

  HoistedCondBB = OrigCallBB;

  // generateRuntimeChecks will handle NullPtrChecks and special conditions.
}

// Generate runtime checks for non-null pointers and virtual call
// possible targets.
//
// Before:
//
//   -------------------
//   |  if (contains()) |
//   |    OriginalCFG   |
//   |                  |
//   --------------------
//          |
//          |
//        ExitBB
//
// After:
//                       ----------------------
//                       | Null-Pointer checks |
//                       | and other runtime   |
//                       | checks              |
//                       -----------------------
//                                 |
//                                 |-----------------|
//                                 |                 |
//                                 |                 |
//                        -------------------        |
//                        |    Hoisted       |       |
//                        |    Conditions    |       |
//                        |    Checks        |       |
//                        --------------------       |
//                                 |                 |
//            ---------------------|                 |
//            |                    |                 |
//            |                    |                 |
//            |                    |<-----------------
//            |           -------------------
//            |           |  if (contains()) |
//            |           |    OriginalCFG   |
//            |           |                  |
//            |           --------------------
//            |                    |
//            |                    |
//            -------------------->|
//                              ExitBB
//
void PredCandidate::generateRuntimeChecks() {

  bool FirstCond = true;
  Value *ResultCond = nullptr;
  SmallPtrSet<Value *, 4> Processed;

  // Use first instruction in HoistedCondBB to generate runtime checks
  // and other nullptr checks.
  Instruction *FI = HoistedCondBB->getFirstNonPHIOrDbg();

  // Create insertion pointer just before CondCall.
  IRBuilder<> B(FI);

  // Create condition to do non-null pointer check for all pointers that
  // are saved in SpecialNonNullCheckCondsMap.
  // For now, walk through control conditions of all executed blocks
  // to get all pointers that require non-null pointer checks.
  for (auto *BB : ExecutedBlocks) {
    for (auto I = BBControlCondsMap[BB].rbegin(),
              E = BBControlCondsMap[BB].rend();
         I != E; I++) {
      ControlCond *C = &*I;

      auto *IC = cast<ICmpInst>(C->getPointer());
      Value *V = nullptr;
      Constant *CompC = nullptr;
      getValueConstant(IC, &V, &CompC);
      assert(V && CompC && "Unexpected ICmpInst");

      // No need to generate null pointer check multiple times.
      if (Processed.count(V))
        continue;

      Processed.insert(V);
      if (!SpecialNonNullCheckCondsMap.contains(V))
        continue;

      assert(CondTypeMap[V] == CT_VtableFieldLoad &&
             "Unexpected Condition type");
      SmallDenseMap<Instruction *, Instruction *, 8> InstCloneInstMap;
      Instruction *Cloned = nullptr;

      for (auto I = SpecialNonNullCheckCondsMap[V].rbegin(),
                E = SpecialNonNullCheckCondsMap[V].rend();
           I != E; I++) {
        Instruction *CV = *(&*I);
        Cloned = B.Insert(CV->clone());
        InstCloneInstMap[CV] = Cloned;
        replaceOperandsWithClonedInst(Cloned, InstCloneInstMap);
      }
      // Generate check for "ICmpNE Ptr, null".
      Value *NewC =
          B.CreateICmpNE(Cloned, Constant::getNullValue(Cloned->getType()));
      if (FirstCond)
        ResultCond = NewC;
      else
        ResultCond = B.CreateLogicalAnd(ResultCond, NewC);
      FirstCond = false;
    }
  }

  BasicBlock *CondCallBB = CondCall->getParent();
  if (ResultCond) {
    BasicBlock *OrigCallBB = FI->getParent();
    BasicBlock *NewThenBB = OrigCallBB->splitBasicBlock(FI->getIterator());
    BranchInst *NewBr = BranchInst::Create(NewThenBB, CondCallBB, ResultCond);
    ReplaceInstWithInst(OrigCallBB->getTerminator(), NewBr);
  }

  // Generate runtime checks that are needed for Callee.
  // These runtime checks are mainly to handle indirect calls.
  //
  //  Ex:
  //   contains(arg1) {
  //   bb:
  //     %i = getelementptr ValueStore, ptr %arg, i64 0, i32 4
  //     %i2 = load ptr, ptr %i
  //     %i3 = icmp eq ptr %i2, null
  //     br i1 %i3, label %bb130, label %bb4
  //
  //   bb4:
  //     %i5 = getelementptr FieldValueMap, ptr %arg1, i64 0, i32 0
  //     %i6 = load ptr, ptr %i5, align 8, !tbaa !892
  //     %i7 = icmp eq ptr %i6, null
  //     br i1 %i7, label %bb11, label %bb8
  //
  //   bb8:
  //     %i9 = getelementptr inbounds %ValueVectorOf, ptr %i6, i64 0, i32 1
  //     %li = load i32, ptr %i9
  //     ...
  //     for (%li = 0; %li < %VecSize; %li++) {
  //       %i40 = getValidatorAt(%arg1, %li); // Marked with attribute
  //                                          //  "dtrans-vector-size-field"
  //       ...
  //       %i104 = getelementptr %Validator, ptr %i39, i64 0, i32 0, i32 0
  //       %i105 = load ptr, ptr %i104
  //       %i107 = getelementptr inbounds ptr, ptr %i105, i64 10
  //       %i108 = load ptr, ptr %i107, align 8
  //       %i109 = tail call noundef i32 %i108(ptr)
  //       ...
  //     }
  //   }
  //
  //   For indirect call %i108, let us assume there are many possible targets
  //   and some of those possible targets have side effects. Using heuristics,
  //   we will pick most probable target that doesn't have side effects. Then,
  //   runtime checks are generated to execute IPPredOpt's transformations
  //   only if %i108 is equal to the most probable target. Also need to
  //   generate runtime checks for needed nullptr checks before accessing
  //   object and vtables.
  //   At callsite, runtime checks will be generated like
  //
  //    %4 = getelementptr inbounds %ValueStore, ptr %i28, i64 0, i32 4
  //    %5 = load ptr, ptr %4
  //    %callee.check = icmp ne ptr %5, null
  //    br i1 %callee.check, label %6, label %UnOptBB
  //
  //  6:
  //    %7 = getelementptr inbounds %FieldValueMap, ptr %i226, i64 0, i32 0
  //    %8 = load ptr, ptr %7
  //    %callee.check1 = icmp ne ptr %8, null
  //    br i1 %callee.check1, label %9, label %UnOptBB
  //
  //  9:
  //    %10 = getelementptr inbounds %ValueVectorOf, ptr %8, i64 0, i32 1
  //    %11 = load i32, ptr %10
  //    %callee.check2 = icmp eq i32 %11, 1
  //    br i1 %callee.check2, label %12, label %UnOptBB
  //
  //  12:
  //    %13 = tail call noundef ptr @getValidatorAt(%i226, 0)
  //    %nunull = icmp ne ptr %13, null
  //    br i1 %nunull, label %14, label %UnOptBB
  //
  //  14:
  //    %15 = getelementptr %Validator, ptr %13, i64 0, i32 0, i32 0
  //    %16 = load ptr, ptr %15
  //    %17 = getelementptr inbounds ptr, ptr %16, i64 10
  //    %18 = load ptr, ptr %17
  //    %callee.check3 = icmp eq ptr %18, @Most_Probable_Func
  //    br i1 %callee.check3, label %OptBB, label %UnOptBB

  // Clone "Inst" in "B" and remap using "VMap".
  auto GenerateClonedInst = [this](Instruction *Inst, ValueToValueMapTy &VMap,
                                   IRBuilder<> &B) -> Value * {
    Value *FinalVal = nullptr;
    auto IT = VMap.find(Inst);
    if (IT != VMap.end()) {
      FinalVal = IT->second;
    } else {
      Instruction *CloneI = B.Insert(Inst->clone());
      VMap[Inst] = CloneI;
      RemapInstruction(CloneI, VMap);
      // For instructions hoisted from callee, use debug info of CondCall
      // if exists.
      if (CondCall->getDebugLoc())
        CloneI->setDebugLoc(CondCall->getDebugLoc());
      FinalVal = CloneI;
    }
    return FinalVal;
  };

  // Clone all instructions "InstVec" in reverse order in "B" and
  // remap using "VMap".
  auto GenerateClonedInstructions =
      [&GenerateClonedInst](SmallVectorImpl<Instruction *> &InstVec,
                            ValueToValueMapTy &VMap,
                            IRBuilder<> &B) -> Value * {
    Value *FinalVal = nullptr;
    for (auto II = InstVec.rbegin(), EE = InstVec.rend(); II != EE; II++)
      FinalVal = GenerateClonedInst(*(&*II), VMap, B);
    return FinalVal;
  };

  // Fix CFG by creating new BrInst using Cond at the end of NewEntryBB.
  auto AdjustCFG = [this](BasicBlock *NewBB, BasicBlock *CondCallBB,
                          BasicBlock *NewEntryBB, Value *Cond) {
    BranchInst *NewBr = BranchInst::Create(NewBB, CondCallBB, Cond);
    if (CondCall->getDebugLoc())
      NewBr->setDebugLoc(CondCall->getDebugLoc());
    ReplaceInstWithInst(NewEntryBB->getTerminator(), NewBr);
  };

  // Clone all instructions "InstVec" in reverse order in newly created BB
  // just before FI->getParent() and create new BrInst using Cond at the
  // end of NewBB.
  auto CloneInstsInNewBBAdjustCFG =
      [this, &GenerateClonedInstructions,
       &AdjustCFG](Instruction *FI, SmallVectorImpl<Instruction *> &InstVec,
                   ValueToValueMapTy &VMap, BasicBlock *CondCallBB,
                   Value *CmpRHS, CmpInst::Predicate Pred) {
        BasicBlock *NewEntryBB = FI->getParent();
        BasicBlock *NewBB = NewEntryBB->splitBasicBlock(FI->getIterator());
        IRBuilder<> B(NewEntryBB, NewEntryBB->getFirstInsertionPt());

        Value *FinalVal = GenerateClonedInstructions(InstVec, VMap, B);
        auto *Cond = B.CreateICmp(Pred, FinalVal, CmpRHS, "callee.check");
        if (CondCall->getDebugLoc())
          cast<Instruction>(Cond)->setDebugLoc(CondCall->getDebugLoc());
        AdjustCFG(NewBB, CondCallBB, NewEntryBB, Cond);
      };

  // Generate runtime conditions to check indirect function pointer
  // is equal to any of target functions in SelectedPossibleTargets.
  //
  // Ex:
  // %18 = load ptr, ptr %17, align 8
  // %callee.check3 = icmp eq ptr %18, @foo
  // %callee.check4 = icmp eq ptr %18, @bar
  // %19 = select i1 %callee.check3, i1 true, i1 %callee.check4
  // %callee.check5 = icmp eq ptr %18, @baz
  // %20 = select i1 %19, i1 true, i1 %callee.check5
  // br i1 %20, label %21, label %54
  auto GenerateIndirectTargetsCheckInNewBBAdjustCFG =
      [this, &GenerateClonedInstructions,
       &AdjustCFG](Instruction *FI, SmallVectorImpl<Instruction *> &InstVec,
                   ValueToValueMapTy &VMap, BasicBlock *CondCallBB,
                   CmpInst::Predicate Pred) {
        BasicBlock *NewEntryBB = FI->getParent();
        BasicBlock *NewBB = NewEntryBB->splitBasicBlock(FI->getIterator());
        IRBuilder<> B(NewEntryBB, NewEntryBB->getFirstInsertionPt());

        Value *FinalVal = GenerateClonedInstructions(InstVec, VMap, B);
        bool FirstCond = true;
        Value *FinalCond = nullptr;
        for (auto *TF : SelectedPossibleTargets) {
          auto *Cond = B.CreateICmp(Pred, FinalVal, TF, "callee.check");
          if (FirstCond)
            FinalCond = Cond;
          else
            FinalCond = B.CreateLogicalOr(FinalCond, Cond);
          if (CondCall->getDebugLoc())
            cast<Instruction>(FinalCond)->setDebugLoc(CondCall->getDebugLoc());
          FirstCond = false;
        }
        AdjustCFG(NewBB, CondCallBB, NewEntryBB, FinalCond);
      };

  if (SelectedPossibleTargets.size() == 0)
    return;

  // Map actual arguments of CondCall and formals of callee.
  ValueToValueMapTy CalleeVMap;
  Function *CondCallee = CondCall->getCalledFunction();
  int ArgI = 0;
  for (Value *Arg : CondCall->args())
    CalleeVMap[CondCallee->getArg(ArgI++)] = Arg;

  // Generate all needed nullptr checks.
  for (auto I = CalleeControlConds.rbegin(), E = CalleeControlConds.rend();
       I != E; I++) {
    ControlCond *C = &*I;
    auto *IC = cast<ICmpInst>(C->getPointer());

    Value *V = nullptr;
    Constant *CompC = nullptr;
    getValueConstant(IC, &V, &CompC);
    CloneInstsInNewBBAdjustCFG(FI, CalleeNullChecks[V], CalleeVMap, CondCallBB,
                               Constant::getNullValue(V->getType()),
                               CmpInst::Predicate::ICMP_NE);
  }

  // Check VecSize (i.e Loop trip count) is equal to 1.
  if (!ElemCallTripCHoistInst.empty()) {
    Value *CmpLHS = ElemCallTripCHoistInst[0];
    CloneInstsInNewBBAdjustCFG(
        FI, ElemCallTripCHoistInst, CalleeVMap, CondCallBB,
        ConstantInt::get(CmpLHS->getType(), 1), CmpInst::Predicate::ICMP_UGE);
  }

  // Generate call to getValidatorAt and fix second argument to access
  // 1st element as we know this is GetElemAccess function with attribute
  // "dtrans-vector-size-field".
  BasicBlock *NewEntryBB = FI->getParent();
  BasicBlock *NewBB = NewEntryBB->splitBasicBlock(FI->getIterator());
  IRBuilder<> Bld(NewEntryBB, NewEntryBB->getFirstInsertionPt());
  Instruction *FirstI = *IndirectFPtrHoistInst.rbegin();
  Value *FinalVal = nullptr;
  if (auto *CallI = dyn_cast<CallBase>(FirstI)) {
    Value *Arg2 = CallI->getArgOperand(1);
    CalleeVMap[Arg2] = ConstantInt::get(Arg2->getType(), 0);
    FinalVal = GenerateClonedInst(CallI, CalleeVMap, Bld);
  } else {
    // Handle simple case here.
    assert(isa<GetElementPtrInst>(FirstI) && "Expected GEP inst");
    auto *GEP = cast<GetElementPtrInst>(FirstI);
    Value *Arg2 = GEP->getOperand(1);
    CalleeVMap[Arg2] = ConstantInt::get(Arg2->getType(), 0);
    FinalVal = GenerateClonedInst(GEP, CalleeVMap, Bld);
  }
  auto *Cond = Bld.CreateICmpNE(
      FinalVal, Constant::getNullValue(FinalVal->getType()), "nunull");
  AdjustCFG(NewBB, CondCallBB, NewEntryBB, Cond);

  // Generate runtime condition to check if indirect pointer is equal to
  // the most probable target.
  GenerateIndirectTargetsCheckInNewBBAdjustCFG(FI, IndirectFPtrHoistInst,
                                               CalleeVMap, CondCallBB,
                                               CmpInst::Predicate::ICMP_EQ);
}

// Collect all executed blocks that are controlled by all conditions.
// Executed blocks are terminated with unconditional branches.
//
//     if (contains(this)) {
//       if (this->field0) {
//         if (this->field2->vtable->function5 == foo) {
//           // Executed block
//         } else if (this->field2->vtable->function5 != bar) {
//           // Executed block
//         }
//       }
//     }
//     ExitBB:
bool PredCandidate::collectExecutedBlocks() {
  assert(ExitBB && "Expected ExitBB");
  for (auto *BB : predecessors(ExitBB)) {
    auto BI = dyn_cast<BranchInst>(BB->getTerminator());
    if (!BI)
      return false;
    if (BI->isConditional())
      continue;
    ExecutedBlocks.insert(BB);
  }
  if (ExecutedBlocks.empty() || ExecutedBlocks.size() > MaxNumberExecutedBlocks)
    return false;
  return true;
}

// Returns true if BB has any instruction with side-effect.
bool IPPredOptImpl::mayBBWriteToMemory(BasicBlock *BB) {
  for (auto &I : *BB)
    if (I.mayWriteToMemory())
      return true;
  return false;
}

// Check terminator of ThenBB is conditional branch and one of its
// successors is ExitBB. Makes sure ThenBB doesn't have any
// mayWriteToMemory instructions.
//
//  ThenBB:    ; single-pred
//    ; No mayWriteToMemory instructions
//    br i1 %i, label %BB2, label %ExitBB
//    ...
//  BB2:
//    ..
//  ExitBB:
//       ...
bool IPPredOptImpl::checkBBControlAllCode(BasicBlock *ThenBB,
                                          BasicBlock *ExitBB) {
  if (!ThenBB->hasNPredecessors(1))
    return false;
  auto *BI = dyn_cast<BranchInst>(ThenBB->getTerminator());
  if (!BI)
    return false;
  if (!BI->isConditional())
    return false;
  if (BI->getSuccessor(0) != ExitBB && BI->getSuccessor(1) != ExitBB)
    return false;
  if (mayBBWriteToMemory(ThenBB))
    return false;
  return true;
}

void IPPredOptImpl::gatherCandidates(Function &F) {

  // Check if terminator of BB is controlled with a return value of a call
  // and return the call if it a valid candidate.
  //
  // BB:
  //   %i = call contains()
  //   br i1 %i, label %TB, label %FB
  //
  // Or
  //
  // BB:
  //   %j = call contains()
  //   %i = icmp eq i32 %j, i32 2
  //   br i1 %i, label %TB, label %FB
  //
  auto GetCondCall = [](BasicBlock &BB) -> CallInst * {
    auto *BBI = dyn_cast<BranchInst>(BB.getTerminator());
    if (!BBI || !BBI->isConditional())
      return nullptr;
    CallInst *CB;
    Value *CmpOp = nullptr;
    CB = dyn_cast<CallInst>(BBI->getCondition());
    if (!CB) {
      ICmpInst *IC = dyn_cast<ICmpInst>(BBI->getCondition());
      if (!IC)
        return nullptr;
      auto *CB1 = dyn_cast<CallInst>(IC->getOperand(0));
      auto *CB2 = dyn_cast<CallInst>(IC->getOperand(1));
      if (CB1 && !CB2) {
        CB = CB1;
        CmpOp = IC->getOperand(1);
      } else if (!CB1 && CB2) {
        CB = CB2;
        CmpOp = IC->getOperand(0);
      }
    }
    if (!CB)
      return nullptr;

    if (CmpOp && !isa<Constant>(CmpOp))
      return nullptr;
    if (!CB->hasOneUse())
      return nullptr;
    if (!CB->hasFnAttr(Attribute::MustProgress))
      return nullptr;
    return CB;
  };

  DominatorTree &DT = DTGetter(F);
  PostDominatorTree &PDT = PDTGetter(F);
  AssumptionCache &AC = ACGetter(F);

  for (auto &BB : F) {
    // Checking for code with this pattern to find candidates.
    //
    // BB:
    //   %i = call contains()
    //   br i1 %i, label %ThenBB, label %ExitBB
    //
    // ThenBB:
    //   br i1 %j, label %ExitBB, label %Cond2BB
    //     ...
    // Cond2BB:
    //   ...
    //   br i1 %k, label %ExecutedBB label %ExitBB
    //
    // ExecutedBB:
    //   ...
    //   br %ExitBB
    //
    // ExitBB:
    //   ...
    CallInst *CI = GetCondCall(BB);
    if (!CI)
      continue;
    Function *Callee = CI->getCalledFunction();
    if (!Callee || Callee->isDeclaration())
      continue;
    BasicBlock *ExitBB = nullptr;
    BasicBlock *ThenBB = nullptr;
    auto *BI = cast<BranchInst>(BB.getTerminator());
    BasicBlock *Succ0 = BI->getSuccessor(0);
    BasicBlock *Succ1 = BI->getSuccessor(1);
    if (Succ0->getSinglePredecessor() == &BB &&
        Succ1->getSinglePredecessor() == nullptr) {
      ThenBB = Succ0;
      ExitBB = Succ1;
    } else if (Succ1->getSinglePredecessor() == &BB &&
               Succ0->getSinglePredecessor() == nullptr) {
      ThenBB = Succ1;
      ExitBB = Succ0;
    }
    if (!ThenBB || !ExitBB)
      continue;
    if (!DT.dominates(&BB, ExitBB))
      continue;
    if (!PDT.dominates(ExitBB, ThenBB))
      continue;
    if (!checkBBControlAllCode(ThenBB, ExitBB))
      continue;

    std::unique_ptr<PredCandidate> CandD(new PredCandidate(
        ExitBB, ThenBB, CI, DT, PDT, AC, GetLI, DTGetter, PDTGetter));

    // Collect ExecutedBB if there are any.
    if (!CandD->collectExecutedBlocks())
      continue;

    if (!CandD->collectControlCondsForBlocks())
      continue;

    if (!CandD->checkLegalityIssues())
      continue;

    if (!CandD->checkCondCallSideEffects(*this)) {
      LLVM_DEBUG(dbgs() << "    Skipped: SideEffects\n");
      continue;
    }

    if (!CandD->applyHeuristics()) {
      LLVM_DEBUG(dbgs() << "    Skipped: Heuristics\n");
      continue;
    }

    Candidates.insert(CandD.release());
  }
}

void IPPredOptImpl::applyTransformations() {
  LLVM_DEBUG(dbgs() << "  IP Pred Opt: Transformations\n");

  auto *Candidate = *Candidates.begin();
  Candidate->hoistConditions();
  Candidate->generateRuntimeChecks();
}

bool IPPredOptImpl::run(void) {

  LLVM_DEBUG(dbgs() << "  IP Pred Opt: Started\n");
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2)) {
    LLVM_DEBUG(dbgs() << "    Failed: Whole Program or target\n");
    return false;
  }

  buildTypeIdMap();
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (IPPredDumpTargetFunctions)
    dumpTargetFunctions();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  for (Function &F : M)
    if (!F.isDeclaration())
      gatherCandidates(F);

  if (Candidates.empty()) {
    LLVM_DEBUG(dbgs() << "    Failed: No Candidate\n");
    return false;
  }
  if (Candidates.size() > MaxNumCandidates) {
    LLVM_DEBUG(dbgs() << "    Failed: Too many candidates found\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "  Found candidate    \n";);

  applyTransformations();
  return true;
}

IPPredOptPass::IPPredOptPass(void) {}

PreservedAnalyses IPPredOptPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  auto DTGetter = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };
  auto PDTGetter = [&FAM](Function &F) -> PostDominatorTree & {
    return FAM.getResult<PostDominatorTreeAnalysis>(F);
  };
  auto ACGetter = [&FAM](Function &F) -> AssumptionCache & {
    return FAM.getResult<AssumptionAnalysis>(F);
  };

  LoopInfoFuncType GetLI = [&FAM](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };

  IPPredOptImpl IPPredOptI(M, WPInfo, DTGetter, PDTGetter, ACGetter, GetLI);
  if (!IPPredOptI.run())
    return PreservedAnalyses::all();
  auto PA = PreservedAnalyses();
  PA.preserve<AndersensAA>();
  PA.preserve<GlobalsAA>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

#endif // INTEL_FEATURE_SW_ADVANCED

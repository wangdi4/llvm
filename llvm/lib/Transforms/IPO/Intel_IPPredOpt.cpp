#if INTEL_FEATURE_SW_ADVANCED
//===--------------------- Intel_IPPredOpt.cpp ----------------------------===//
//
// Copyright (C) 2021-2023 Intel Corporation. All rights reserved.
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

// This is used to represent control condition.
// Value: Condition in the ICmp instruction.
// bool: Whether BasicBlock is executed when the condition is True or False.
using ControlCond = PointerIntPair<Value *, 1, bool>;

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
                DominatorTree &DT, PostDominatorTree &PDT, AssumptionCache &AC)
      : ExitBB(ExitBB), ThenBB(ThenBB), CondCall(CondCall), DT(DT), PDT(PDT),
        AC(AC) {}
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
  bool processIndirectCalls(IPPredOptImpl &, SmallPtrSet<CallBase *, 2> &);
  bool applyHeuristics();
  bool funcHasNoSideEffects(Function *F);
  void hoistConditions();
  void generateRuntimeChecksCloneCFG();
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

  DominatorTree &DT;

  PostDominatorTree &PDT;

  AssumptionCache &AC;

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
};

// Main class to implement the transformation.
class IPPredOptImpl {

public:
  IPPredOptImpl(Module &M, WholeProgramInfo &WPInfo,
                function_ref<DominatorTree &(Function &)> DTGetter,
                function_ref<PostDominatorTree &(Function &)> PDTGetter,
                function_ref<AssumptionCache &(Function &)> ACGetter)
      : M(M), WPInfo(WPInfo), DTGetter(DTGetter), PDTGetter(PDTGetter),
        ACGetter(ACGetter){};
  ~IPPredOptImpl(){};
  bool run(void);
  bool getVirtualPossibleTargets(CallBase &CB,
                                 SetVector<Function *> &TargetFunctions);

private:
  constexpr static int MaxNumCandidates = 1;

  Module &M;
  WholeProgramInfo &WPInfo;
  function_ref<DominatorTree &(Function &)> DTGetter;
  function_ref<PostDominatorTree &(Function &)> PDTGetter;
  function_ref<AssumptionCache &(Function &)> ACGetter;
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
    CallBase &CB, SetVector<Function *> &TargetFunctions) {
  assert(!CB.getCalledFunction() && "Expected indirect call");

  LLVM_DEBUG(dbgs() << "Collecting possible targets for: " << CB << "\n");
  const Instruction *PrevI = CB.getPrevNode();
  if (!PrevI || !isa<LoadInst>(PrevI)) {
    LLVM_DEBUG(dbgs() << "    No LoadInst Found: "
                      << "\n");
    return false;
  }
  auto *LI = cast<LoadInst>(PrevI);
  PrevI = PrevI->getPrevNode();
  if (PrevI && isa<GetElementPtrInst>(PrevI))
    PrevI = PrevI->getPrevNode();

  if (!PrevI || !isa<IntrinsicInst>(PrevI)) {
    LLVM_DEBUG(dbgs() << "    No Assume call Found: "
                      << "\n");
    return false;
  }
  auto *AI = cast<IntrinsicInst>(PrevI);
  if (AI->getIntrinsicID() != Intrinsic::assume) {
    LLVM_DEBUG(dbgs() << "    No Assume intrinsic Found: "
                      << "\n");
    return false;
  }
  PrevI = PrevI->getPrevNode();
  if (!PrevI || !isa<IntrinsicInst>(PrevI)) {
    LLVM_DEBUG(dbgs() << "    No typetest call Found: "
                      << "\n");
    return false;
  }
  auto *TI = cast<CallInst>(PrevI);
  if (TI->getIntrinsicID() != Intrinsic::type_test) {
    LLVM_DEBUG(dbgs() << "    No typetest intrinsic Found: "
                      << "\n");
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
      LLVM_DEBUG(dbgs() << "    Can't find pointer in vtable: "
                        << "\n");
      return false;
    }

    auto TargetFunc = dyn_cast<Function>(Ptr->stripPointerCasts());
    if (!TargetFunc) {
      LLVM_DEBUG(dbgs() << "    vtable entry is not function pointer: "
                        << "\n");
      return false;
    }

    TargetFunctions.insert(TargetFunc);
    LLVM_DEBUG(dbgs() << "    Adding target function: " << TargetFunc->getName()
                      << "\n");
  }

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
      SetVector<Function *> TargetFunctions;
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

bool PredCandidate::applyHeuristics() {
  // TODO:  Add more code here for heuristics.
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

// For each indirect call, find all possible target functions and no action
// is required if there are no side effects. If there are possible targets
// with side effects, find most probable target, which will be used to generate
// runtime check later, using base class heuristic.
//
bool PredCandidate::processIndirectCalls(
    IPPredOptImpl &IPPredObj, SmallPtrSet<CallBase *, 2> &IndirectCalls) {

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
  auto ProcessVirtualFunctionLoads = [](CallBase *CB) -> Value * {
    Instruction *PrevI = CB->getPrevNonDebugInstruction();
    auto *LI = dyn_cast_or_null<LoadInst>(PrevI);
    if (!LI || PrevI != CB->getCalledOperand())
      return nullptr;
    auto *VGEP =
        dyn_cast_or_null<GetElementPtrInst>(LI->getPrevNonDebugInstruction());
    if (!VGEP)
      return nullptr;
    auto *AI =
        dyn_cast_or_null<IntrinsicInst>(VGEP->getPrevNonDebugInstruction());
    if (!AI || AI->getIntrinsicID() != Intrinsic::assume)
      return nullptr;
    auto *TI =
        dyn_cast_or_null<IntrinsicInst>(AI->getPrevNonDebugInstruction());
    if (!TI || TI->getIntrinsicID() != Intrinsic::type_test)
      return nullptr;
    auto *VTLI = dyn_cast<LoadInst>(VGEP->getPointerOperand());
    if (!VTLI)
      return nullptr;
    auto *GEP = dyn_cast<GetElementPtrInst>(VTLI->getPointerOperand());
    if (!GEP || !GEP->hasAllZeroIndices())
      return nullptr;

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
  auto IsDTransVectorAccessElemCall = [](Value *FPtr) {
    auto *VCall = dyn_cast<CallBase>(FPtr);
    if (!VCall)
      return false;
    Function *VCallee = VCall->getCalledFunction();
    if (!VCallee)
      return false;
    Attribute SizeFieldAttr =
        VCallee->getFnAttribute("dtrans-vector-size-field");
    if (!SizeFieldAttr.isValid())
      return false;
    if (VCallee->arg_size() != 2)
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
      [&IsDTransVectorAccessElemCall,
       &ProcessVirtualFunctionLoads](CallBase *CB) -> Value * {
    // Check for case 1.
    Value *FPtr = ProcessVirtualFunctionLoads(CB);
    if (!FPtr)
      return nullptr;
    if (IsDTransVectorAccessElemCall(FPtr))
      return FPtr;

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
    if (IsDTransVectorAccessElemCall(NewFPtr))
      return NewFPtr;
    return nullptr;
  };

  // Finds virtual function call in base class from TargetFunctions.
  // DTrans's metadata is used to check same types.
  auto GetTargetFunctionInBaseType =
      [&GetCallFirstArgTyMD, &GetFunctionFirstParamTyMD](
          SetVector<Function *> &TargetFunctions, CallBase *CB) -> Function * {
    // Get DTrans's metadata for 1st argument of call.
    MDNode *ArgTyMD = GetCallFirstArgTyMD(CB);
    if (!ArgTyMD)
      return nullptr;

    Function *FuncInBaseClass = nullptr;

    for (auto *TF : TargetFunctions) {
      // Get DTrans's metadata for 1st argument of TF.
      MDNode *TypeNode = GetFunctionFirstParamTyMD(TF);
      if (TypeNode != ArgTyMD)
        continue;

      if (FuncInBaseClass)
        return nullptr;
      FuncInBaseClass = TF;
    }
    return FuncInBaseClass;
  };

  SmallPtrSet<Function *, 2> MostProbableTarget;
  SmallPtrSet<Value *, 2> IndirectFunctionPtr;

  for (auto *CB : IndirectCalls) {
    // Find all possible target functions.
    SetVector<Function *> TargetFunctions;
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

    // Try to find most probable target function.
    Function *TF = GetTargetFunctionInBaseType(TargetFunctions, CB);
    if (!TF)
      return false;

    // Don't apply the transformation if most probable target has side effects.
    if (!funcHasNoSideEffects(TF))
      return false;

    // Trying to collect all instructions that are needed to hoist
    // and generate runtime check for computing function pointer.
    Value *FPtr = GetValidIndirectCallObj(CB);
    if (!FPtr)
      return false;
    Function *VCallee = cast<CallBase>(FPtr)->getCalledFunction();
    Attribute SizeFieldAttr =
        VCallee->getFnAttribute("dtrans-vector-size-field");
    if (!SizeFieldAttr.isValid())
      return false;
    StringRef Val = SizeFieldAttr.getValueAsString();
    unsigned SizeField = UINT32_MAX;
    if (Val.getAsInteger(0, SizeField))
      return false;

    // TODO: More code will be added here.

    MostProbableTarget.insert(TF);
    IndirectFunctionPtr.insert(FPtr);
  }
  if (MostProbableTarget.size() == 0)
    return true;

  if (MostProbableTarget.size() > 1 ||
      MostProbableTarget.size() != IndirectFunctionPtr.size())
    return false;

  LLVM_DEBUG(dbgs() << "      IndirectCallObj:"
                    << *(*IndirectFunctionPtr.begin()) << "\n";);
  LLVM_DEBUG(dbgs() << "      Most probable Target:"
                    << (*MostProbableTarget.begin())->getName() << "\n");

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

  SmallPtrSet<CallBase *, 4> DirectCalls;
  SmallPtrSet<CallBase *, 2> IndirectCalls;
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

  if (!processIndirectCalls(IPPredObj, IndirectCalls))
    return false;

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

  if (!IPPredSkipCalleeLegalChecks)
    return;

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

  // TODO: Need to add more code to handle NullPtrChecks and special conditions.
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
//                       | and other checks.   |
//                       |                     |
//                       -----------------------
//                        /                  \
//                       /                    \
//   -------------------                      --------------------
//   |  if (contains()) |                     |  (Cloned Version) |
//   |    OriginalCFG   |                     |  if (contains())  |
//   |                  |                     |    CloneCFG       |
//   --------------------                     ---------------------
//                       \                     /
//                        \                   /
//                         \                 /
//                          \               /
//                              NewExitBB
//                                 |
//                                 |
//                              ExitBB
//
void PredCandidate::generateRuntimeChecksCloneCFG() {

  // Return true if any non-null pointer checks are needed.
  auto NeedRuntimeChecks = [&]() -> bool {
    if (!SpecialNonNullCheckCondsMap.empty())
      return true;
    return false;
  };

  // Find any definitions in BBset which are used in outside basicblocks that
  // are not in BBSet.
  auto FindDefsUsedOutside = [](SmallVectorImpl<BasicBlock *> &BBSet,
                                SmallVectorImpl<Instruction *> &LiveOut) {
    for (auto *Block : BBSet)
      for (auto &Inst : *Block) {
        auto Users = Inst.users();
        if (std::any_of(Users.begin(), Users.end(), [&](User *U) {
              auto *Use = cast<Instruction>(U);
              return std::find(BBSet.begin(), BBSet.end(), Use->getParent()) ==
                     BBSet.end();
            }))
          LiveOut.push_back(&Inst);
      }
  };

  // Create PHI nodes in original ExitBB by joining LiveOut of
  // BBSet and the corresponding cloned LiveOuts.
  auto JoinWithPHINodes = [](ValueToValueMapTy &VMap,
                             SmallVectorImpl<BasicBlock *> &BBSet,
                             SmallVectorImpl<Instruction *> &LiveOut) {
    // NewExitBB is ensured to be the last item in BBSet.
    BasicBlock *NewExitBB = BBSet.back();
    BasicBlock *ClonedExitBB = cast<BasicBlock>(VMap[NewExitBB]);
    // OriginalExitBB is the 'NewTail'.
    BasicBlock *OriginalExitBB = NewExitBB->getSingleSuccessor();

    for (auto *Inst : LiveOut) {
      auto *ClonedInst = cast<Instruction>(VMap[Inst]);
      PHINode *PH =
          PHINode::Create(Inst->getType(), 2, Inst->getName() + ".join.phi",
                          &OriginalExitBB->front());

      for (auto *User : Inst->users())
        if (std::find(BBSet.begin(), BBSet.end(),
                      cast<Instruction>(User)->getParent()) == BBSet.end())
          User->replaceUsesOfWith(Inst, PH);

      PH->addIncoming(Inst, NewExitBB);
      PH->addIncoming(ClonedInst, ClonedExitBB);
    }
  };

  if (!NeedRuntimeChecks())
    return;

  bool FirstCond = true;
  Value *ResultCond = nullptr;
  SmallPtrSet<Value *, 4> Processed;

  // Create insertion pointer just before CondCall.
  IRBuilder<> B(CondCall);

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

  ValueToValueMapTy VMap;
  SmallVector<BasicBlock *, 16> ClonedBBSet;
  BasicBlock *OrigCallBB = CondCall->getParent();
  // Insert the condition just before CondCall.
  BasicBlock *NewEntryBB = OrigCallBB->splitBasicBlock(CondCall->getIterator());

  // Create NewTailBB just before ExitBB.
  BasicBlock *NewTailBB =
      ExitBB->splitBasicBlock(ExitBB->front().getIterator());
  (void)NewTailBB;

  // Collect all basic blocks in the CFG from NewEntryBB to ExitBB.
  SmallVector<BasicBlock *> BBSet;
  GeneralUtils::collectBBSet(NewEntryBB, ExitBB, BBSet);

  // Clone all basicblocks in BBSet.
  Function *F = OrigCallBB->getParent();
  for (BasicBlock *BB : BBSet) {
    BasicBlock *NewBB = CloneBasicBlock(BB, VMap, ".clone", F, nullptr);
    // Add basic block to the mapping.
    VMap[BB] = NewBB;
    ClonedBBSet.push_back(NewBB);
  }
  remapInstructionsInBlocks(ClonedBBSet, VMap);

  // Fix CFG.
  BasicBlock *ClonedEntryBB = ClonedBBSet.front();
  F->splice(ExitBB->getIterator(), F, ClonedEntryBB->getIterator(), F->end());
  BranchInst *NewBr = BranchInst::Create(NewEntryBB, ClonedEntryBB, ResultCond);
  ReplaceInstWithInst(OrigCallBB->getTerminator(), NewBr);

  // Fix liveout values of the CFG.
  SmallVector<Instruction *> LiveOut;
  FindDefsUsedOutside(BBSet, LiveOut);
  JoinWithPHINodes(VMap, BBSet, LiveOut);
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

    std::unique_ptr<PredCandidate> CandD(
        new PredCandidate(ExitBB, ThenBB, CI, DT, PDT, AC));

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

    // TODO: Add more checks here.

    Candidates.insert(CandD.release());
  }
}

void IPPredOptImpl::applyTransformations() {
  LLVM_DEBUG(dbgs() << "  IP Pred Opt: Transformations\n");

  auto *Candidate = *Candidates.begin();
  Candidate->generateRuntimeChecksCloneCFG();
  Candidate->hoistConditions();
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

  IPPredOptImpl IPPredOptI(M, WPInfo, DTGetter, PDTGetter, ACGetter);
  if (!IPPredOptI.run())
    return PreservedAnalyses::all();
  auto PA = PreservedAnalyses();
  PA.preserve<AndersensAA>();
  PA.preserve<GlobalsAA>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

#endif // INTEL_FEATURE_SW_ADVANCED

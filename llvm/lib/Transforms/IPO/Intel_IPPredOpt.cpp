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
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/TypeMetadataUtils.h"
#include "llvm/IR/Dominators.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;

#define DEBUG_TYPE "ippredopt"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// This option is mainly used by LIT tests.
//
static cl::opt<bool> IPPredDumpTargetFunctions(
    "ippred-dump-target-functions", cl::init(false), cl::ReallyHidden,
    cl::desc("Dump target functions for virtual function calls"));

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Application may have many candidates. This class is used to
// handle multiple candidates.
class PredCandidate {
public:
  PredCandidate(BasicBlock *ExitBB) : ExitBB(ExitBB) {}
  ~PredCandidate() {}

  bool collectExecutedBlocks();
  PredCandidate(const PredCandidate &) = delete;
  PredCandidate(PredCandidate &&) = delete;
  PredCandidate &operator=(const PredCandidate &) = delete;
  PredCandidate &operator=(PredCandidate &&) = delete;

private:
  // Maximum executed basic blocks that are controlled under
  // inside condition statements.
  constexpr static int MaxNumberExecutedBlocks = 2;

  // Exit block of outermost conditional statement.
  BasicBlock *ExitBB = nullptr;

  // Basic blocks that are controlled under inside condition statements.
  SmallPtrSet<BasicBlock *, 2> ExecutedBlocks;
};

// Main class to implement the transformation.
class IPPredOptImpl {

public:
  IPPredOptImpl(Module &M, WholeProgramInfo &WPInfo,
                function_ref<DominatorTree &(Function &)> DTGetter,
                function_ref<PostDominatorTree &(Function &)> PDTGetter)
      : M(M), WPInfo(WPInfo), DTGetter(DTGetter), PDTGetter(PDTGetter){};
  ~IPPredOptImpl(){};
  bool run(void);

private:
  constexpr static int MaxNumCandidates = 1;

  Module &M;
  WholeProgramInfo &WPInfo;
  function_ref<DominatorTree &(Function &)> DTGetter;
  function_ref<PostDominatorTree &(Function &)> PDTGetter;
  DenseMap<Metadata *, SmallSet<std::pair<GlobalVariable *, uint64_t>, 4>>
      TypeIdMap;
  SmallPtrSet<PredCandidate *, MaxNumCandidates> Candidates;

  bool mayBBWriteToMemory(BasicBlock *BB);
  bool checkBBControlAllCode(BasicBlock *BB, BasicBlock *ExitBB);
  void gatherCandidates(Function &F);
  void buildTypeIdMap();
  bool getVirtualPossibleTargets(CallBase &CB,
                                 SetVector<Function *> &TargetFunctions);

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

    std::unique_ptr<PredCandidate> CandD(new PredCandidate(ExitBB));

    // Collect ExecutedBB if there are any.
    if (!CandD->collectExecutedBlocks())
      continue;

    // TODO: Add more checks here.

    Candidates.insert(CandD.release());
  }
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
  LLVM_DEBUG(dbgs() << "  Found candidate    \n";);

  // TODO: Add more code here
  return false;
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

  IPPredOptImpl IPPredOptI(M, WPInfo, DTGetter, PDTGetter);
  if (!IPPredOptI.run())
    return PreservedAnalyses::all();
  auto PA = PreservedAnalyses();
  PA.preserve<AndersensAA>();
  PA.preserve<GlobalsAA>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

#endif // INTEL_FEATURE_SW_ADVANCED

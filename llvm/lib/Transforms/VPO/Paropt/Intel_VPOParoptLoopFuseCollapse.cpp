//===-- VPOParoptLoopFuseCollapse.cpp - OpenMP Loop FuseAndCollapse Pass --===//
//
//   Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation of Loop FuseAndCollapse optimization
///
/// \note
/// This pass only concerns OMP 'loop' constructs, so whenever an OMP loop
/// is mentioned below it means OMP generic loop.
///
/// The main goal of this optimization is to increase the scope for
/// ND-Range parallelization (specific ND-Range) aka collapsing. For example:
///
///   Before optimization:
/// \code
///       #pragma omp target
///       #pragma omp loop
///       for (int i = 0; i < M; ++i) {
///         #pragma omp loop
///         for (int j = 0; j < N; ++j)
///           Body1
///         #pragma omp loop
///         for (int j = 0; j < P; ++j)
///           Body2
///       }
/// \endcode
///    After optimization:
/// \code
///       #pragma omp target
///       #pragma omp loop collapse(2)
///       for (int i = 0; i < M; ++i) {
///         for (int j = 0; j < max(N, P); ++j) {
///           if (j < N)
///             Body1
///           if (j < P)
///             Body2
///         }
///       }
/// \endcode
///
/// Original collapsing was only applicable to canonical loop nests of non-OMP
/// loops only, except for a single outermost OMP loop. That approach was
/// missing two optimization opportunities:
///
/// 1. Parallelization of the nests containing OMP loops, e.g.
/// \code
///       #pragma omp target
///       #pragma omp loop
///       for (int i = 0; i < M; ++i)
///         #pragma omp loop
///         for (int j = 0; j < N; ++j)
/// \endcode
///
///    Using ND-Range parallelization for such nest may prove especially
///    beneficial when M is much lower than N.
/// 2. Loop nest is not in a canonical form. This particular pass aims for
///    a nests where the inner loops may be fused hence allowing the
///    subsequent parallelization, e.g.
/// \code
///       #pragma omp target
///       #pragma omp loop
///       for (int i = 0; i < M; ++i) {
///         #pragma omp loop
///         for (int j = 0; j < N; ++j)
///           ...
///         #pragma omp loop
///         for (int j = 0; j < N; ++j)
///           ...
///       }
/// \endcode
///
///
/// The pass consists of 4 main stages:
/// 1. Fusion candidates collection
/// 2. Checking the fusion and collapsing legality
/// 3. Loops fusion:
///    a. Hoisting the LB/UB out of the target region
///    b. Fusion of LLVM loops
///    c. Adjusting the original OMP loop directives
/// 4. Loop collapsing:
///    a. Merging the inner loop directives into the outer one
///    b. Hoisting the inner loops LB/UB and the fused UB out of the target
///    region
///    c. Setting the collapse clause
///
// TODOS:
// 1. Allow fusion of >2 loops
// 2. Incorporate legality checks into the candidates collection to be able
//    to pick other candidates after the checks fail
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/Intel_VPOParoptLoopFuseCollapse.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/DomTreeUpdater.h"
#include "llvm/Analysis/MemorySSA.h"

#include "llvm/Transforms/Utils/CodeMoverUtils.h"

#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"

using namespace llvm;
using namespace llvm::vpo;

#define OPT_SWITCH "vpo-paropt-loop-fuse-collapse"
#define PASS_NAME "VPO Paropt Loop FuseAndCollapse Function Pass"
#define DEBUG_TYPE OPT_SWITCH

cl::opt<bool>
    EnableFuseAndCollapse(OPT_SWITCH, cl::Hidden, cl::init(false),
                          cl::desc("Enable paropt loop fuse&collapse pass"));

static bool fuseAndCollapse(Function &F, WRegionInfo &WI,
                            OptimizationRemarkEmitter &ORE,
                            MemorySSA *MSSA = nullptr) {
  bool Changed = false;

  WI.buildWRGraph();

  if (WI.WRGraphIsEmpty()) {
    LLVM_DEBUG(dbgs() << "\nNo WRegion Candidates for Transformation \n");
    return Changed;
  }

  LLVM_DEBUG(WI.print(dbgs()));
  LLVM_DEBUG(dbgs() << PASS_NAME << " for Function: ");
  LLVM_DEBUG(dbgs().write_escaped(F.getName()) << '\n');

  VPOParoptTransform VP(nullptr, &F, &WI, WI.getDomTree(), WI.getLoopInfo(),
                        WI.getSE(), WI.getTargetTransformInfo(),
                        WI.getAssumptionCache(), WI.getTargetLibraryInfo(),
                        WI.getAliasAnalysis(), MSSA, FuseCollapse,
                        OptReportVerbosity::None, ORE, 2, false);

  Changed |= VP.paroptTransforms();

  return Changed;
}

PreservedAnalyses
VPOParoptLoopFuseCollapsePass::run(Function &F, FunctionAnalysisManager &AM) {

  if (!EnableFuseAndCollapse)
    return PreservedAnalyses::all();

  WRegionInfo &WI = AM.getResult<WRegionInfoAnalysis>(F);
  auto &ORE = AM.getResult<OptimizationRemarkEmitterAnalysis>(F);
  auto &MSSA = AM.getResult<MemorySSAAnalysis>(F);

  PreservedAnalyses PA;

  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
  if (!fuseAndCollapse(F, WI, ORE, &MSSA.getMSSA()))
    PA = PreservedAnalyses::all();
  else
    PA = PreservedAnalyses::none();
  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");

  return PA;
}

// When FE is not able to calculate condition LB < UB in compile time
// it emits it in the IR, so the loop region's CFG looks as following:
//
// PrecondBB:
//   precond = icmp sle lb, ub
//   br precond, RegionEntry, PrecondExit   <- GuardBranch
// RegionEntry:
//   < OMP entry directive >
//   ...
// RegionExit:
//   < OMP exit directive >
//   br PrecondExit
// PrecondExit:
//
// This function returns the guard branch if it exists
static BranchInst *getOMPGuardBranch(const WRegionNode *W) {
  auto *Preheader = W->getEntryBBlock()->getSinglePredecessor();
  assert(Preheader);
  auto *ExitBlock = W->getExitBBlock()->getSingleSuccessor();
  assert(ExitBlock);
  auto *GuardBlock = Preheader->getSinglePredecessor();
  if (GuardBlock) {
    auto *GuardBranch = dyn_cast<BranchInst>(GuardBlock->getTerminator());
    if (GuardBranch && GuardBranch->getNumSuccessors() > 1 &&
        GuardBranch->getSuccessor(0) == Preheader &&
        GuardBranch->getSuccessor(1) == ExitBlock)
      return GuardBranch;
  }
  return nullptr;
}

class FusionCandidate {
  WRegionNode *W;
  Loop *L;

  Value *IV = nullptr;
  Value *LB = nullptr;
  Value *UB = nullptr;

  Type *IVType = nullptr;
  Type *UBType = nullptr;

public:
  FusionCandidate(WRegionNode *W) : W(W), L(W->getWRNLoopInfo().getLoop()) {}

  Loop *getLoop() const { return L; }
  WRegionNode *getRegion() const { return W; }

  Value *getIV() const { return IV; }
  Value *getLB() const { return LB; }
  Value *getUB() const { return UB; }

  Type *getIVType() const { return IVType; }
  Type *getUBType() const { return UBType; }

  BasicBlock *getStartingBlock() const {
    auto *GuardBr = getOMPGuardBranch(W);
    return GuardBr ? GuardBr->getParent()
                   : W->getEntryBBlock()->getSinglePredecessor();
  }

  /// Identify loop LB, UB and IV
  void identifyLoopEssentialValues(DominatorTree *DT);
  /// Check if the loop's UB can be hoisted out of the enclosing target region
  bool canHoistUB(DominatorTree *DT, AAResults *AA) const;
  /// Check if the loop is legal to be ND-Range parallelized
  bool isEligibleForNDRangeParallelization() const;
  /// Check if the loop is legal to be fused with some other loop
  bool isEligibleForFusion(DominatorTree *DT, AAResults *AA) const;
  /// Check if the loop is legal to be fused into \param C
  bool canBeFusedInto(const FusionCandidate &C, DominatorTree *DT,
                      AAResults *AA, MemorySSA *MSSA) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const {
    dbgs() << "OMP fusion candidate:\n\t" << *W->getEntryDirective() << "\n";
  }
#endif

  bool isValid() const {
    bool IsValid = L->isLoopSimplifyForm() && L->getExitBlock();
    IsValid = IsValid && IV && LB && UB;
    return IsValid;
  }
};

// If there exists a StoreInst SI being a single user of V
// and V is a pointer operand of SI, return a pointer to SI,
// nullptr otherwise
static StoreInst *getSingleStoreUserOf(Value *V) {
  if (!V)
    return nullptr;
  StoreInst *Result = nullptr;
  for (auto *U : V->users()) {
    if (auto *SI = dyn_cast<StoreInst>(U)) {
      if (Result || SI->getPointerOperand() != V)
        return nullptr;
      Result = SI;
    }
  }
  return Result;
}

bool FusionCandidate::isEligibleForNDRangeParallelization() const {

  auto *ParentW = W->getParent();
  assert(ParentW && "Parent region not found");

  // For most reduction loops specific nd-range makes the perf worse
  // due to the increased number of teams and the subsequent cross-team
  // reduction costs
  if (!W->getRed().empty())
    return false;

  if (ParentW->canHaveReduction() && !ParentW->getRed().empty())
    return false;

  // TODO: consider removing this
  if (!ParentW->getLpriv().empty() || !W->getLpriv().empty())
    return false;

  if (ParentW->getLoopBind() != WRNLoopBindAbsent ||
      W->getLoopBind() != WRNLoopBindAbsent)
    return false;

  if (ParentW->getLoopOrder() != WRNLoopOrderAbsent ||
      W->getLoopOrder() != WRNLoopOrderAbsent)
    return false;

  return true;
}

bool FusionCandidate::isEligibleForFusion(DominatorTree *DT,
                                          AAResults *AA) const {
  if (!isValid())
    return false;

  if (!isa<WRNGenericLoopNode>(W))
    return false;

  W->populateBBSet();

  // Any unknown side-effects instruction other than stores
  // and any memory-accessing instruction other that load/stores
  // are not allowed
  for (auto *BB : W->blocks()) {
    for (auto &I : *BB) {
      if (I.isLifetimeStartOrEnd() || VPOAnalysisUtils::isOpenMPDirective(&I))
        continue;
      // TODO: allow memory intrinsics as they're typically used for non-POD
      // items
      if (I.mayHaveSideEffects() && !isa<StoreInst>(&I))
        return false;
      if (I.mayReadOrWriteMemory() && !isa<StoreInst>(&I) && !isa<LoadInst>(&I))
        return false;
    }
  }

  return true;
}

bool FusionCandidate::canBeFusedInto(const FusionCandidate &C,
                                     DominatorTree *DT, AAResults *AA,
                                     MemorySSA *MSSA) const {
  const auto *OC = dyn_cast<FusionCandidate>(&C);
  if (!OC)
    return false;
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Checking omp loops for fusion:\n";
             C.dump(); dump(););

  bool AGuarded = getOMPGuardBranch(OC->getRegion());
  bool BGuarded = getOMPGuardBranch(getRegion());

  // Currently we don't support fusion of a mix of guarded and non-guarded loops
  if (AGuarded != BGuarded)
    return false;

  auto *BBInterv = OC->getLoop()->getExitBlock();
  auto *AEntry = OC->getStartingBlock();
  auto *BEntry = getStartingBlock();
  SmallVector<BasicBlock *, 2> IntervBlocks;
  while (BBInterv && BBInterv != BEntry) {
    IntervBlocks.push_back(BBInterv);
    BBInterv = BBInterv->getSingleSuccessor();
  }
  IntervBlocks.push_back(BEntry);

  // There should be no CFG in between the loops
  // TODO: relax this condition
  if (BBInterv != BEntry) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": A.Exit has no single path to B.Entry:\n"
                      << OC->getLoop()->getExitBlock()->getName() << ", "
                      << BEntry->getName() << "\n");
    return false;
  }

  SmallVector<BasicBlock *, 2> PrecedingBlocks;
  assert(OC->getRegion()->getParent() && "Orphan loop region found");
  auto *BBPreceding =
      OC->getRegion()->getParent()->getEntryBBlock()->getSingleSuccessor();
  while (BBPreceding && BBPreceding != AEntry) {
    PrecedingBlocks.push_back(BBInterv);
    BBPreceding = BBPreceding->getSingleSuccessor();
  }
  PrecedingBlocks.push_back(AEntry);

  // Looking for the loads and stores in both candidates
  SmallVector<Instruction *> ALoads;
  SmallVector<Instruction *> AStores;
  SmallVector<Instruction *> BLoads;
  SmallVector<Instruction *> BStores;

  for (BasicBlock *BB : OC->getLoop()->blocks()) {
    for (Instruction &I : *BB) {
      if (auto *LI = dyn_cast<LoadInst>(&I)) {
        if (LI->getPointerOperand() != OC->getIV() &&
            LI->getPointerOperand() != OC->getUB())
          ALoads.push_back(LI);
      } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
        if (SI->getPointerOperand() != OC->getIV() &&
            SI->getPointerOperand() != OC->getUB())
          AStores.push_back(SI);
      }
    }
  }

  for (BasicBlock *BB : getLoop()->blocks()) {
    for (Instruction &I : *BB) {
      if (auto *LI = dyn_cast<LoadInst>(&I)) {
        if (LI->getPointerOperand() != IV && LI->getPointerOperand() != UB)
          BLoads.push_back(LI);
      } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
        if (SI->getPointerOperand() != IV && SI->getPointerOperand() != UB)
          BStores.push_back(SI);
      }
    }
  }

  // Check that none of ToCheck instructions modify memory location
  // accessed by ToMod
  auto CheckAliasModSet = [AA](SmallVector<Instruction *> ToCheck,
                               Instruction *ToMod) {
    return std::none_of(
        ToCheck.begin(), ToCheck.end(), [AA, ToMod](Instruction *I) {
          return AA->canInstructionRangeModRef(
              *I, *I, MemoryLocation::get(ToMod), ModRefInfo::Mod);
        });
  };
  // Check that ToCheck instruction modify none of the memory locations
  // accessed by ToMod instructions
  auto CheckAliasModSetRev = [AA](Instruction *ToCheck,
                                  SmallVector<Instruction *> ToMod) {
    return std::none_of(
        ToMod.begin(), ToMod.end(), [AA, ToCheck](Instruction *I) {
          return AA->canInstructionRangeModRef(
              *ToCheck, *ToCheck, MemoryLocation::get(I), ModRefInfo::Mod);
        });
  };

  auto CheckBBsAliasingWithLoops =
      [&OC, &AStores, &BStores,
       &CheckAliasModSet](SmallVector<BasicBlock *, 2> &BBs,
                          std::function<bool(StoreInst *)> StoresCheck) {
        bool IsEligible = true;
        // No memory location in the preceding code should be modified
        // within the loops
        for (auto *BB : BBs) {
          for (Instruction &I : *BB) {
            if (auto *LI = dyn_cast<LoadInst>(&I)) {
              // Loops should not modify intervening code memlocations
              IsEligible = CheckAliasModSet(AStores, LI) &&
                           CheckAliasModSet(BStores, LI);
            } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
              IsEligible = StoresCheck(SI);
            } else if (I.mayHaveSideEffects() && !I.isLifetimeStartOrEnd()) {
              // Loop A's exit directive does not prevent fusion
              IsEligible = (&I == OC->getRegion()->getExitDirective());
            }
            if (!IsEligible) {
              LLVM_DEBUG(dbgs()
                         << __FUNCTION__
                         << ": Intervening blocks aliasing check failed: "
                         << BB->getName() << "\n");
              return false;
            }
          }
        }
        return true;
      };

  // Example illustrating the following checks
  // (note that order of loads and stores doesn't actually matter, we only
  // concern about their belonging to one of the loops or BBs being checked;
  // and there may be multiple instances of any of them):
  //    BBPred:
  //      load ptr0
  //      store ptr1
  //    ALoopBB: (BBPred dom ALoopBB)
  //      load ptr2
  //      store ptr3
  //    BLoopBB:
  //      load ptr4
  //      store ptr5

  // Check preceding blocks (outer loop header to LoopA's preheader):
  //  - ptr0 doesn't alias with ptr3, ptr5
  //  - ptr1 doesn't alias with ptr3, ptr5
  if (!CheckBBsAliasingWithLoops(
          PrecedingBlocks,
          [&AStores, &BStores, &CheckAliasModSetRev](StoreInst *SI) {
            // Preceding code and the loops should not write to the same memory
            // as it may lead to a data race after collapsing
            // TODO: consider moving this check to
            // isEligibleForNDRangeParallelization
            return CheckAliasModSetRev(SI, AStores) &&
                   CheckAliasModSetRev(SI, BStores);
          }))
    return false;

  // Check intervening blocks (LoopA's exit to LoopB's preheader):
  //  - ptr0 doesn't alias with ptr3, ptr5
  //  - ptr1 doesn't alias with ptr2, ptr3, ptr4, ptr5
  if (!CheckBBsAliasingWithLoops(IntervBlocks,
                                 [&AStores, &BStores, &ALoads, &BLoads,
                                  &CheckAliasModSetRev](StoreInst *SI) {
                                   // Intervening code should not modify loops
                                   // memlocations
                                   return CheckAliasModSetRev(SI, AStores) &&
                                          CheckAliasModSetRev(SI, BStores) &&
                                          CheckAliasModSetRev(SI, ALoads) &&
                                          CheckAliasModSetRev(SI, BLoads);
                                 }))
    return false;

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Intervening code checks ok\n");

  // Check if SI modifies the memlocation accesses by ToMod which belongs to
  // ModFC candidate. This contains additional checks to allow accesses that are
  // safe from OMP PoV
  auto CheckLoopAliasModRef = [&MSSA, &AA](StoreInst *I, LoadInst *ToMod,
                                           const FusionCandidate &ModFC) {
    if (!AA->canInstructionRangeModRef(*I, *I, MemoryLocation::get(ToMod),
                                       ModRefInfo::Mod))
      return true;
    LLVM_DEBUG(dbgs() << *I << " "
                      << "modifies " << *ToMod << "\n");
    bool Result = false;

    MemoryAccess *Clobber = nullptr;
    if (MSSA) {
      auto *LoadAcc = MSSA->getMemoryAccess(ToMod);
      BatchAAResults BAA(*AA);
      Clobber = MSSA->getWalker()->getClobberingMemoryAccess(
          LoadAcc->getDefiningAccess(), MemoryLocation::get(ToMod), BAA);
    }

    // For clobbers there're 2 specific cases we should allow:
    // 1. The stored value depends only on the IV and/or constants
    // 2. The stored value is a map-chain pointer
    // For any other cases we just discard the candidate
    if (auto *Def = dyn_cast_or_null<MemoryDef>(Clobber)) {
      // Checking 1.
      auto *ClobInst = Def->getMemoryInst();
      if (ClobInst == I || ClobInst->getParent() != ToMod->getParent() ||
          !isa<StoreInst>(ClobInst)) {
        Result = false;
      } else {
        Result = true;
        // check IV dep
        std::queue<Value *> Deps;
        Deps.push(ClobInst->getOperand(0));
        while (!Deps.empty()) {
          auto *DepV = Deps.front();
          Deps.pop();
          auto *DepInst = dyn_cast<Instruction>(DepV);
          if (!DepInst)
            continue;
          if (DepInst->getParent() != ToMod->getParent()) {
            Result = false;
            break;
          }
          if (auto *LI = dyn_cast<LoadInst>(DepInst)) {
            if (LI->getPointerOperand() != ModFC.getIV()) {
              Result = false;
              break;
            }
            continue;
          } else if (DepInst->mayReadFromMemory() ||
                     DepInst->mayHaveSideEffects()) {
            Result = false;
            break;
          }
          for (auto &Op : DepInst->operands())
            Deps.push(Op);
        }
      }
    } else {
      // Checking 2.
      auto *PtrA = I->getPointerOperand()->stripInBoundsOffsets();
      auto *PtrB = ToMod->getPointerOperand()->stripInBoundsOffsets();

      if (auto *LA = dyn_cast<LoadInst>(PtrA))
        PtrA = LA->getPointerOperand();
      if (auto *LB = dyn_cast<LoadInst>(PtrB))
        PtrB = LB->getPointerOperand();

      auto *SA = getSingleStoreUserOf(PtrA);
      auto *SB = getSingleStoreUserOf(PtrB);

      if (SA && SB) {
        auto *WTarget = WRegionUtils::getParentRegion(ModFC.getRegion(),
                                                      WRegionNode::WRNTarget);
        assert(WTarget);

        Result = WRegionUtils::wrnSeenAsMap(WTarget, SA->getValueOperand()) &&
                 WRegionUtils::wrnSeenAsMap(WTarget, SB->getValueOperand());
      }
    }
    return Result;
  };

  if (std::any_of(BStores.begin(), BStores.end(),
                  [&ALoads, &CheckLoopAliasModRef, &OC](Instruction *BSt) {
                    return std::any_of(
                        ALoads.begin(), ALoads.end(),
                        [&BSt, &OC, &CheckLoopAliasModRef](Instruction *ALd) {
                          return !CheckLoopAliasModRef(
                              cast<StoreInst>(BSt), cast<LoadInst>(ALd), *OC);
                        });
                  }))
    return false;
  if (std::any_of(AStores.begin(), AStores.end(),
                  [&BLoads, &CheckLoopAliasModRef, this](Instruction *ASt) {
                    return std::any_of(
                        BLoads.begin(), BLoads.end(),
                        [&ASt, this, &CheckLoopAliasModRef](Instruction *BLd) {
                          return !CheckLoopAliasModRef(
                              cast<StoreInst>(ASt), cast<LoadInst>(BLd), *this);
                        });
                  }))
    return false;

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Loop body code checks ok\n");

  return true;
}

void FusionCandidate::identifyLoopEssentialValues(DominatorTree *DT) {
  IV = W->getWRNLoopInfo().getNormIV();
  UB = W->getWRNLoopInfo().getNormUB();

  assert(IV && UB && "Omp loop should have normalized IV and UB");

  if (!L->getLoopPreheader())
    return;

  IVType = W->getWRNLoopInfo().getNormIVElemTy();
  UBType = W->getWRNLoopInfo().getNormUBElemTy();

  // OMP loop should have a dedicated storage for LB
  for (auto *U : IV->users()) {
    auto *UserInst = dyn_cast<StoreInst>(U);
    if (!UserInst)
      continue;
    if (UserInst->getParent() == L->getLoopGuardBranch()->getParent()) {
      if (auto *LBLoad = dyn_cast<LoadInst>(UserInst->getValueOperand())) {
        if (LB) {
          LB = nullptr;
          break;
        }
        LB = LBLoad->getPointerOperand();
      }
    }
  }
}

bool FusionCandidate::canHoistUB(DominatorTree *DT, AAResults *AA) const {
  Value *LBVal = nullptr, *UBVal = nullptr;
  if (!LBVal) {
    auto *LBStore = getSingleStoreUserOf(LB);
    if (LBStore)
      LBVal = LBStore->getValueOperand();
  }
  if (!UBVal) {
    auto *UBStore = getSingleStoreUserOf(UB);
    if (UBStore)
      UBVal = UBStore->getValueOperand();
  }
  auto *WTarget = WRegionUtils::getParentRegion(W, WRegionNode::WRNTarget);
  auto *WParent = W->getParent();
  assert(WTarget);
  if (!LBVal || !UBVal)
    return false;

  assert(cast<ConstantInt>(LBVal)->isNullValue() && "OMP loop LB is non-zero");
  auto *UBInst = dyn_cast<Instruction>(UBVal);
  // Constant UB doesn't need value calculations hoisting
  if (!UBInst)
    return true;

  // Checking whether UBValue depends only on constants or the target
  // region's firstprivate values
  std::queue<Value *> Worklist;
  Worklist.push(UBInst);
  SmallPtrSet<Instruction *, 4> Visited;
  while (!Worklist.empty()) {
    auto *TopVal = Worklist.front();
    Worklist.pop();
    if (isa<Constant>(TopVal))
      continue;
    Instruction *Inst = dyn_cast<Instruction>(TopVal);
    if (!Inst || DT->dominates(Inst->getParent(), WParent->getEntryBBlock()))
      return false;
    if (Visited.count(Inst))
      continue;
    Visited.insert(Inst);

    // if there's a load in the UB def-use chain we only allow it if
    // there's a single store user of the pointer, and
    // there's no CFG and no aliasing accesses in between the user and
    // the load.
    if (auto *LI = dyn_cast<LoadInst>(Inst)) {
      auto *LoadPtr = LI->getPointerOperand();
      if (WRegionUtils::wrnSeenAsFirstprivate(WTarget, LoadPtr)) {
        // Found a fpriv
        continue;
      }
      StoreInst *Store = getSingleStoreUserOf(LoadPtr);
      if (!Store) {
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ": No single store to " << *LoadPtr
                          << " found\n");
        return false;
      }

      if (!DT->dominates(Store, Inst)) {
        LLVM_DEBUG(dbgs() << *Store << " !dom " << *Inst << "\n");
        return false;
      }
      // Checking if the store is in the same block or in the immediate
      // predecessor NOTE: isControlFlowEquivalent would be better, but it
      // needs PDT and we're OK with a simpler check for now
      if (Inst->getParent() != Store->getParent() &&
          Store->getParent() != Inst->getParent()->getSinglePredecessor()) {
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ": CFG check failed\n");
        return false;
      }

      SmallVector<Instruction *> ToCheck;
      // store can't be a terminator
      Instruction *I = Store->getNextNode();
      // collecting instructions between Store and Inst
      while (I && I != Inst) {
        if (I->isTerminator()) {
          // safety is checked above
          I = &Inst->getParent()->front();
        } else {
          ToCheck.push_back(I);
          I = I->getNextNode();
        }
      }

      bool HasIntervAlias = std::any_of(
          ToCheck.begin(), ToCheck.end(), [LoadPtr, AA](Instruction *I) {
            if (I->mayWriteToMemory() && !I->isLifetimeStartOrEnd() &&
                VPOAnalysisUtils::isOpenMPDirective(I)) {
              auto *SI = dyn_cast<StoreInst>(I);
              if (!SI)
                return true;
              if (AA->alias(SI->getPointerOperand(), LoadPtr) !=
                  AliasResult::NoAlias)
                return true;
            }
            return false;
          });

      if (HasIntervAlias) {
        LLVM_DEBUG(dbgs() << __FUNCTION__
                          << ": Intervening aliasing access found\n");
        return false;
      }
      Worklist.push(Store->getValueOperand());
    } else if (!Inst->mayHaveSideEffects()) {
      if (Inst->getNumUses() > 1)
        return false;
      for (auto &Op : Inst->operands())
        Worklist.push(Op);
    } else {
      return false;
    }
  }

  return true;
}

bool VPOParoptTransform::fuseAndCollapseOmpLoops(
    WRegionNode *W, const SmallPtrSet<WRegionNode *, 1> &HaveCollapse) {
  auto *WTarget = WRegionUtils::getParentRegion(W, WRegionNode::WRNTarget);

  if (!WTarget)
    return false;

  if (!isa<WRNGenericLoopNode>(W))
    return false;

  // if the enclosing loop's normalized UB wasn't firstprivatized by the
  // frontend, collapsing is not going to happen anyway
  if (std::any_of(W->getWRNLoopInfo().getNormUBs().begin(),
                  W->getWRNLoopInfo().getNormUBs().end(), [WTarget](Value *UB) {
                    return !WRegionUtils::wrnSeenAsFirstprivate(WTarget, UB);
                  })) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Not all loop's UB are firstprivate, exiting");
    return false;
  }

  // Step 1. Fusion candidates collection

  bool ConsiderCurrent = true;
  if (HaveCollapse.count(W)) {
    // already being collapsed, check if that can be increased
    ConsiderCurrent = true;
  } else {
    // should not be included in other collapsed nest,
    // nor contain other collapsed loop
    ConsiderCurrent =
        !WRegionUtils::containsWRNsWith(W,
                                        [](WRegionNode *WChild) {
                                          return WChild->canHaveCollapse() &&
                                                 WChild->getCollapse() > 1;
                                        }) &&
        !WRegionUtils::hasAncestorWRNWith(W, [](WRegionNode *WAnc) {
          return WAnc->canHaveCollapse() && WAnc->getCollapse() > 1;
        });
    if (!ConsiderCurrent) {
      LLVM_DEBUG(dbgs() << __FUNCTION__
                        << ": Intervening collapse clause found, exiting\n");
    }
  }
  if (!ConsiderCurrent)
    return false;

  bool HasCollapse = W->getCollapse() > 1;
  unsigned LookupLevel = (HasCollapse ? W->getCollapse() : 1);

  using CandidatePtrTy = std::unique_ptr<FusionCandidate>;
  using CandidatesVecTy = SmallVector<CandidatePtrTy, 2>;
  CandidatesVecTy Candidates;

  // OMP collapse clause implies existence of some non-OMP child loops,
  // and we don't want to interfere with that until we decide to
  // to support fusion of non-OMP loops too
  if (!HasCollapse) {
    std::queue<std::pair<WRegionNode *, /* loop level*/ unsigned>> Q;

    for (auto *WC : W->Children)
      Q.push({WC, 1});

    while (!Q.empty()) {
      auto [WC, L] = Q.front();
      Q.pop();
      // Currently only non-simd OMP loops are allowed, with no other constructs
      // in between
      bool IsValidLoop = WC->getIsOmpLoop() && !isa<WRNVecLoopNode>(WC);
      if (!IsValidLoop)
        break;
      if (L == LookupLevel) {
        Candidates.push_back(std::make_unique<FusionCandidate>(WC));
      } else if (L < LookupLevel) {
        for (auto *WCC : WC->Children)
          Q.push({WCC, L + 1});
      }
    }
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Candidates:\n"; for (auto &FC
                                                               : Candidates) {
    dbgs() << __FUNCTION__ << ": \t";
    FC->dump();
    dbgs() << __FUNCTION__ << ": \n";
  } dbgs() << __FUNCTION__
           << ": \n";);

  if (Candidates.empty()) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": No suitable candidates found, exiting\n");
    return false;
  }

  if (Candidates.size() == 1) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Only one candidate found, exiting\n");
    return false;
  }

  // TODO: get rid of this
  if (Candidates.size() > 2) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": >2  candidates found, exiting\n");
    return false;
  }

  std::sort(Candidates.begin(), Candidates.end(),
            [this](const CandidatePtrTy &FC0, const CandidatePtrTy &FC1) {
              return DT->dominates(FC0->getStartingBlock(),
                                   FC1->getStartingBlock());
            });

  std::for_each(
      Candidates.begin(), Candidates.end(),
      [this](CandidatePtrTy &FC) { FC->identifyLoopEssentialValues(DT); });

  // Step 2. Checking the fusion and collapsing legality

  bool CanFuse = std::all_of(Candidates.begin(), Candidates.end(),
                             [this](const CandidatePtrTy &FC) {
                               return FC->isEligibleForFusion(DT, AA);
                             });
  if (!CanFuse) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Per-candidate fusion check failed\n");
    return false;
  }
  bool CanCollapse = std::all_of(
      Candidates.begin(), Candidates.end(), [](const CandidatePtrTy &FC) {
        return FC->isEligibleForNDRangeParallelization();
      });
  if (!CanCollapse) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Per-candidate collapsing check failed\n");
    return false;
  }
  bool CanHoistUB = std::all_of(
      Candidates.begin(), Candidates.end(),
      [this](const CandidatePtrTy &FC) { return FC->canHoistUB(DT, AA); });
  if (!CanHoistUB) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": UB hoisting check failed\n");
    return false;
  }
  for (CandidatesVecTy::const_iterator it = Candidates.begin(),
                                       ie = Candidates.end();
       CanFuse && (ie - it > 1); it++) {
    CanFuse = CanFuse &&
              std::all_of(it + 1, ie, [&it, this](const CandidatePtrTy &FC) {
                return FC->canBeFusedInto(**it, DT, AA, MSSA);
              });
  }
  if (!CanFuse) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Candidate pairs fusion check failed\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__
                    << ": Fusion safety checks passed, proceeding\n");

  return false;
}

//===--- Intel_LoadCoalescing.cpp - Coalescing of consecutive loads -------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Load coalescing of consecutive vector loads.
//
// What it does:
// -------------
// This pass scans for vector loads that are accessing consecutive memory
// locations and attempts to replace them with a wider version.
// For example, if there are two 4-wide loads from A[i:i+3] and A[i+4:i+7], it
// will try to replace them with an 8-wide A[i:i+7], if this is legal.
// If successful, the users of the original 4-wide loads will get their values
// through shuffles. For more detail, please refer the LoadCoalescing.rst file.
//

#include "llvm/Transforms/Vectorize/Intel_LoadCoalescing.h"
#include "Intel_VPlan/IntelVPlanUtils.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Vectorize.h"

#define DEBUG_TYPE "load-coalescing"

const long MAX_SCHEDULER_ATTEMPTS = 60000;

using namespace llvm;
using namespace llvm::vpmemrefanalysis;
using namespace llvm::vpo;

// Turn this ON for testing
static cl::opt<bool>
    SanityChecks("load-coalescing-sanity-checks", cl::init(false), cl::Hidden,
                 cl::desc("Sanity checks to ease debugging !"));

static cl::opt<size_t> MaxVecRegSizeOpt(
    "load-coalescing-max-vec-size", cl::init(0), cl::Hidden,
    cl::desc(
        "Coalesce as many vector loads as possible up to the MAX specified size"
        "Legalizer, which runs as part of CG can take care of breaking it into "
        "legal-sized sub-vectors."));

static cl::opt<size_t>
    MinVecRegSizeOpt("load-coalescing-min-vec-size", cl::init(32), cl::Hidden,
                     cl::desc("Coalesce as many vector loads as possible "
                              "starting from min of size 32"));

// TODO: Set the init value to 4. This parameter can be used to experiment and
// the performance might vary on different platforms
static cl::opt<size_t> MinGroupSizeOpt(
    "load-coalescing-min-group-size", cl::init(2), cl::Hidden,
    cl::desc("Coalesce at least 'MinGroupSizeOpt' vector loads"));

// This parameter controls how far up from the 'bottom' instruction do we end
// the group. Too high a number and it result in complexity explosion.
static cl::opt<size_t> MaxSchedulerDistance(
    "load-coalescing-scheduler-max-distance", cl::init(128), cl::Hidden,
    cl::desc("To avoid complexity explosion, we set a limit to the maximum "
             "group top-bottom distance that the scheduler."));

// This option allows coalescing of scalar loads into vector-loads. Useful for
// experimenting and understanding how sub-components of LoadCoalescing work.
static cl::opt<bool> AllowScalars(
    "load-coalescing-allow-scalars", cl::init(false), cl::Hidden,
    cl::desc("Allow load coalescing of scalar types (for debugging)"));

static cl::opt<int> LoadCoalescingProfitabilityThreshold(
    "load-coalescing-profitability-threshold", cl::init(2), cl::Hidden,
    cl::desc("Set the profitability threshold for LoadCoalescing. This number "
             "is the different between the cost of coalescing, including the "
             "cost of executing the required shuffle/extract - element "
             "instructions V/S the cost of uncoalesced instruction"));

Type *MemInstGroup::getScalarType() const {
  auto Ty = getLoadStoreType(getHead());
  return isa<VectorType>(Ty) ? cast<VectorType>(Ty)->getVectorElementType()
                             : Ty;
}

// Returns true if it is profitable to coalesce group into a single wide-load.
// This is a very simple heuristic. While it is better than the greedy
// strategy it would be better to have a more rubust code-model.
bool MemInstGroup::isCoalescingLoadsProfitable(
    const TargetTransformInfo *TTI) const {

  assert(isa<LoadInst>(getHead()) &&
         "The group should exclusively hold LoadInst's.");
  LoadInst *LI = cast<LoadInst>(getHead());
  VectorType *GroupTy =
      VectorType::get(getScalarType(), getTotalScalarElements());

  size_t ShuffleCost = 0;
  size_t CoalescedLoadScalarOffset = 0;
  unsigned CostBeforeCoalescing = 0;
  for (size_t I = 0, E = size(); I != E; ++I) {
    // Get the cost of shuffles required. Based on whether we are generating an
    // actual shuffle as opposed to an extractelement instruction for getting
    // either a sub-vector or a scalar element, we use the appropriate API to
    // get the cost. For getting the cost of extractelement instruction, we use
    // the getVectorInstrCost function.
    LoadInst *MemberI = cast<LoadInst>(getMember(I));
    Type *GroupMemType = getLoadStoreType(MemberI);
    assert(GroupMemType && "Null-value for Group-member type.");

    if (isa<VectorType>(GroupMemType))
      ShuffleCost +=
          TTI->getShuffleCost(TargetTransformInfo::SK_ExtractSubvector, GroupTy,
                              CoalescedLoadScalarOffset, GroupMemType);
    else
      ShuffleCost += TTI->getVectorInstrCost(
          Instruction::ExtractElement, GroupTy, CoalescedLoadScalarOffset);

    CostBeforeCoalescing += TTI->getMemoryOpCost(
        MemberI->getOpcode(), GroupMemType, MaybeAlign(MemberI->getAlignment()),
        MemberI->getPointerAddressSpace());

    CoalescedLoadScalarOffset += getNumElementsSafe(GroupMemType);
  }

  int GroupLoadCost =
      TTI->getMemoryOpCost(LI->getOpcode(), GroupTy,
                           MaybeAlign(LI->getAlignment()),
                           LI->getPointerAddressSpace());
  int CostAfterCoalescing = GroupLoadCost + ShuffleCost;
  int ProfitabilityThreshold = CostAfterCoalescing - CostBeforeCoalescing;
  bool IsProfitable =
      ProfitabilityThreshold < LoadCoalescingProfitabilityThreshold;
  LLVM_DEBUG(if (!IsProfitable) dbgs()
                 << "LC: G " << getId() << '\n'
                 << " ShuffleCost: " << ShuffleCost << '\n'
                 << " CostAfterCoalescing: " << CostAfterCoalescing << '\n'
                 << " CostBeforeCoalescing: " << CostBeforeCoalescing << '\n';);
  return IsProfitable;
}

void MemInstGroup::append(Instruction *LI, size_t LISize) {
  Type *LIType = LI->getType();
  if (CoalescedLoads.size() == 0) {
    Type *ScalarTy = (isa<VectorType>(LIType)
                          ? cast<VectorType>(LIType)->getVectorElementType()
                          : LIType);
    ScalarSize = DL.getTypeSizeInBits(ScalarTy);
  }

  TotalScalarElements += getNumElementsSafe(LIType);
  CoalescedLoads.insert(LI);
  TotalVectorSize += LISize;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void MemInstGroup::dump() const {
  dbgs().indent(2) << "Group " << Id << "\n";
  dbgs().indent(2) << "ScalarSize: " << ScalarSize
                   << " TotalVectorSize: " << TotalVectorSize << "\n";
  int Cnt = 0;
  for (Instruction *I : CoalescedLoads)
    dbgs() << Cnt++ << ". " << *I << "\n";
  dbgs() << "\n";
}
#endif

bool MemInstGroup::tryInsert(LoadInst *Inst) {
  size_t LoadSize = getVecBits(Inst, DL, AllowScalars);
  bool WillOverflow = willExceedSize(LoadSize, MaxVecRegSize);
  auto InstTy = getLoadStoreType(Inst);
  auto ScalarInstTy = isa<VectorType>(InstTy)
                          ? cast<VectorType>(InstTy)->getVectorElementType()
                          : InstTy;

  bool ScalarTypesMatch = size() > 0 ? (ScalarInstTy == getScalarType()) : true;

  if (WillOverflow || !ScalarTypesMatch) {
    LLVM_DEBUG(if (WillOverflow) dbgs()
                   << "Cannot append " << *Inst << "\n will exceed group-size,"
                   << LoadSize << "--" << MaxVecRegSize << " so bailing out"
                   << '\n';
               else dbgs() << "Cannot append " << *Inst
                           << "\n Types not compatible" << '\n';);

    return false;
  }
  LLVM_DEBUG(dbgs() << "Inserting into group - " << *Inst << '\n';);
  append(Inst, LoadSize);
  return true;
}

GroupDependenceGraph::GroupDependenceGraph(BasicBlock *BB, AAResults *AA)
    : BB(BB), AA(AA) {}

/// Finds all instructions that have a dependence with \p Ins .
/// This includes both def-use and memory dependences.
/// For now, we do not schedule across basic-blocks.
void GroupDependenceGraph::getDefs(Instruction *Ins,
                                   GroupDependenceGraph::DefsTy &Defs) {
  // Check both the Use-Def chain as well as the Memory-dependence
  for (int I = 0, E = Ins->getNumOperands(); I != E; ++I)
    if (Instruction *DefI = dyn_cast<Instruction>(Ins->getOperand(I)))
      if (!isa<PHINode>(DefI) && DefI->getParent() == Ins->getParent() &&
          isInRegion(DefI))
        Defs.insert(DefI);

  if (isMemoryInst(Ins)) {
    for (Instruction *SrcI = Ins->getPrevNode(), *E = TopI->getPrevNode();
         SrcI != E; SrcI = SrcI->getPrevNode()) {
      if (isMemoryDependency(Ins, SrcI)) {
        MemoryLocation DstLoc = getLocation(Ins);
        MemoryLocation SrcLoc = getLocation(SrcI);
        // TODO: Cache the expensive alias check
        if (!(SrcLoc.Ptr && DstLoc.Ptr && !isVolatileOrAtomic(Ins) &&
              !isVolatileOrAtomic(SrcI)) ||
            AA->alias(SrcLoc, DstLoc))
          Defs.insert(SrcI);
      }
    }
  }
}

/// Collect the lexicographic ordering of instructions in \p G .
bool GroupDependenceGraph::collectPos(const MemInstGroup &G) {
  SmallPtrSet<Instruction *, 8> VisitedLoads;

  // Start with the head of G which could be randomly placed in the BB.
  Instruction *RandomI = G.getHead();
  VisitedLoads.insert(RandomI);

  // Keep iterating until we have visited all loads in the group.
  // We are using two iterators, one towards the top and one towards the bottom.
  Instruction *TopVisited = nullptr;
  Instruction *BotVisited = nullptr;
  Instruction *TopIter = RandomI;
  Instruction *BotIter = RandomI;
  // We expect the search to be at most 2 times the value of
  // MaxSchedulerDistance
  size_t SpanSize = 0;
  while (VisitedLoads.size() < G.size()) {
    if (G.contains(TopIter)) {
      VisitedLoads.insert(TopIter);
      TopVisited = TopIter;
    }
    if (G.contains(BotIter)) {
      VisitedLoads.insert(BotIter);
      BotVisited = BotIter;
    }
    // Increment the iterators
    if (TopIter) {
      TopIter = TopIter->getPrevNode();
      ++SpanSize;
    }
    if (BotIter) {
      BotIter = BotIter->getNextNode();
      ++SpanSize;
    }
    // If the Loads span too big of a distance then scheduling will take too
    // long to compile.
    if (SpanSize > MaxSchedulerDistance * 2)
      return false;
  }

  assert((TopVisited != nullptr) && (BotVisited != nullptr) &&
         "Either of TopVisited or BottomVisited cannot be NULL");
  assert(G.contains(TopVisited) && G.contains(BotVisited) && "G loads only");

  // At this point TopVisited and BotVisited should point to the Top and Bot
  // loads of the group.
  // We now traverse from Top to Bot and create the position map.
  size_t Pos = 0;
  PosMap.clear();
  for (Instruction *I = TopVisited, *E = BotVisited->getNextNode(); I != E;
       I = I->getNextNode())
    PosMap[I] = Pos++;

  // Get the top/bot instruction in the group.
  TopI = TopVisited;
  BotI = BotVisited;

  return true;
}

/// \Returns the number of unscheduled successors of \p I .
size_t GroupDependenceGraph::getUnschedSuccsSafe(const Instruction *I) const {
  auto It = NodeData.find(I);
  if (It == NodeData.end())
    return 0;
  return It->second.UnscheduledSuccs;
}

// Build a DAG only for the region of the code that group spans.
bool GroupDependenceGraph::buildDAG(const MemInstGroup &G) {
  // Cleanup
  // TODO: Once we have incremental scheduling, this cleanup should never run.
  NodeData.clear();
  Roots.clear();
  PosMap.clear();

  // Since we may have generated code, we need to update the positions.
  bool LoadsCloseEnough = collectPos(G);
  if (!LoadsCloseEnough) {
    LLVM_DEBUG(dbgs() << "LC: G" << G.getId()
                      << " Loads are too far apart > MaxVecRegSizeOpt.");
    return false;
  }

  if (getPosition(BotI) - getPosition(TopI) > MaxSchedulerDistance)
    LLVM_DEBUG(
        dbgs() << "LC: buildDAG(Group - " << G.getId()
               << "), failed because group-span > MaxSchedulerDistance!\n");

  // Bottom-up traversal from BotI to TopI.
  for (Instruction *I = BotI, *E = TopI->getPrevNode(); I != E;
       I = I->getPrevNode()) {
    // Initialize the NodeData map.
    GroupDependenceGraph::DefsTy Defs;
    getDefs(I, Defs);
    for (Instruction *DefI : Defs) {
      assert(isInRegion(DefI) && "Should have been filtered out in getDefs()");
      // Build an edge DefI->I
      NodeData[DefI].UnscheduledSuccs++;
      NodeData[I].Preds.push_back(DefI);
    }
  }
  // Collect root nodes of the DAG.
  // These are all the instructions that have no unscheduled successors.
  for (Instruction *I = BotI, *E = TopI->getPrevNode(); I != E;
       I = I->getPrevNode()) {
    if (getUnschedSuccsSafe(I) == 0)
      Roots.push_back(I);
  }

  return true;
}

bool GroupDependenceGraph::decrementUnscheduledSuccs(Instruction *Src) {
  unsigned &Unsched = NodeData[Src].UnscheduledSuccs;
  assert(Unsched >= 1 && "Broken scheduler");
  Unsched--;
  return Unsched == 0;
}

bool GroupDependenceGraph::isInRegion(Instruction *I) const {
  assert(!PosMap.empty() && TopI && BotI);
  return PosMap.count(I);
}

Instruction *Scheduler::popReady(Instruction *I) {
  assert(ReadyList.count(I) && "'I' not in ready list.");
  ReadyList.remove(I);
  return I;
}

/// \Returns a ready instructions that is not in \p G .
Instruction *Scheduler::popNonBundleReady(const MemInstGroup &G) {
  for (Instruction *I : ReadyList)
    if (!G.getInstrs().count(I))
      return popReady(I);
  return nullptr;
}

/// Schedules \p ReadyI .
void Scheduler::scheduleReadyInstruction(Instruction *ReadyI) {
  // Schedule ReadyI.
  BottomUpScheduledInstrs.insert(ReadyI);

  // Insert the predecessors of ReadyI into the ready list if ready.
  for (Instruction *DefI : DAG->getDefs(ReadyI)) {
    assert(DAG->isInRegion(DefI) &&
           "Should have been filtered out during DAG creation");
    bool isReady = DAG->decrementUnscheduledSuccs(DefI);
    if (isReady)
      ReadyList.insert(DefI);
  }
}

bool Scheduler::isBundleInReadyList(const MemInstGroup &G) const {
  auto isReady = [&](Instruction *I) { return ReadyList.count(I); };
  return std::all_of(G.getInstrs().begin(), G.getInstrs().end(), isReady);
}

/// Try to schedule the loads in \p G . Returns true on success.
bool Scheduler::trySchedule(const MemInstGroup &G) {
  // Early return as we do not want to spend time scheduling group with a single
  // instruction
  if (G.size() <= 1)
    return false;

  // TODO: Cleanup should be fixed once we have incremental schedule scheduling.
  BottomUpScheduledInstrs.clear();
  ReadyList.clear();

  // 1. We build the localized DAG to include all nodes from the top to the
  // bottom of G.
  bool Success = DAG->buildDAG(G);
  if (!Success) {
    LLVM_DEBUG(dbgs() << "LC: trySchedule(Group " << G.getId()
                      << "), buildDAG() failed!\n");
    return false;
  }

  // 2. 'Dummy' schedule to see if we can schedule the whole bundle at once.
  //     We are not updating the IR yet. We keep the schedule in
  //     'BottomUpScheduledInstrs'
  for (Instruction *RootI : DAG->getRoots())
    ReadyList.insert(RootI);

  int MaxAttempts = MAX_SCHEDULER_ATTEMPTS;
  // Keep trying to schedule until the ready list is empty.
  while (!ReadyList.empty()) {
    // De-prioritize group loads, as long as not all of them are ready.
    bool BundleReady = isBundleInReadyList(G);
    if (!BundleReady) {
      // 'G' is not ready. Try to schedule any other non-G instructions.
      Instruction *ReadyI = popNonBundleReady(G);
      if (!ReadyI) {
        // 'G" is not ready and we could not find any other instrs to schedule.
        LLVM_DEBUG(dbgs() << "LC: trySchedule(Group " << G.getId()
                          << "), popNonBundleReady(G) returned null!\n");
        return false;
      }
      scheduleReadyInstruction(ReadyI);
    } else {
      // All the instructions in the bundle are ready, so schedule them.
      for (Instruction *BI : G.getInstrs()) {
        Instruction *ReadyBI = popReady(BI);
        scheduleReadyInstruction(ReadyBI);
      }
    }
    (void)MaxAttempts;
    assert(MaxAttempts-- != 0 && "Infinite loop");
  }
  LLVM_DEBUG(dbgs() << "LC: trySchedule(Group " << G.getId()
                    << ") Succeeded!\n");
  return true;
}

/// Apply the schedule that we have already saved in 'BottomUpScheduledInstrs'.
void Scheduler::applySchedule() {
  Instruction *BotI = BottomUpScheduledInstrs.front();
  for (Instruction *SI :
       llvm::make_range(std::next(BottomUpScheduledInstrs.begin()),
                        BottomUpScheduledInstrs.end())) {
    SI->moveBefore(BotI);
    BotI = SI;
  }
  if (SanityChecks)
    assert(!verifyFunction(*BB->getParent(), &dbgs()));
}

Scheduler::Scheduler(BasicBlock *BB, AAResults *AA) : BB(BB), AA(AA) {
  DAG = std::make_unique<GroupDependenceGraph>(BB, AA);
}

bool LoadCoalescing::areAdjacentMemoryAccesses(LoadInst *I1, LoadInst *I2) {
  return isConsecutiveAccess(I1, I2, DL, SE, true /*Check-types*/);
}

// Build the largest group possible starting at the 'Seed'. Returns the
// true if we are able to successfully able to form a group and schedule
// instructions along with the next starting point, or the non-grouped
// item in the chain (or nullptr otherwise).
bool LoadCoalescing::buildMaximalGroup(
    const LoadBucketMembers &BucketMembers,
    LoadBucketMembers::const_iterator &BMIter, MemInstGroup &G) {
  bool GroupChanged = false;
  while (BMIter != BucketMembers.end()) {
    auto *SeedInst = BMIter->getInstruction();
    if (G.tryInsert(SeedInst)) {
      GroupChanged |= true;
      ++BMIter;
    } else {
      return GroupChanged;
    }
    // This Instruction marks the end of the chain. We cannot continue adding
    // to the group. We should break and start a new Group
    // if (Tails.count(SeedInst) > 0)
    if (BMIter != BucketMembers.end() &&
        !areAdjacentMemoryAccesses(SeedInst,
                                   BMIter->getInstruction()))
      break;
  }
  return GroupChanged;
}

bool LoadCoalescing::scheduleGroup(const MemInstGroup &G) {
  bool IsSchedulingSuccessful = false;
  if (G.size() >= MinGroupSizeOpt && G.getTotalSize() >= MinVecRegSize) {
    bool IsProfitable = G.isCoalescingLoadsProfitable(TTI);
    LLVM_DEBUG(if (!IsProfitable) dbgs()
               << "LC: Skipping G" << G.getId()
               << " because of coalescing loads not deemed profitable.\n");
    if (IsProfitable) {
      if (!SCH->trySchedule(G)) {
        LLVM_DEBUG(dbgs() << "LC: Group " << G.getId()
                          << " Failed to schedule\n");
      } else
        IsSchedulingSuccessful |= true;
    }
  }
  return IsSchedulingSuccessful;
}

/// Coalesce the loads into a single wider load.
/// Next, generate shuffles to feed the data to their uses.
void LoadCoalescing::codeGen(const MemInstGroup &G) {
  // 1. Schedule the code to avoid def after use.
  SCH->applySchedule();

  // 2. Build the wide load.
  // The loads are guaranteed to be consecutive, therefore we get the pointer
  // from the head of the group.
  LoadInst *Head = cast<LoadInst>(G.getHead());
  unsigned AS = Head->getPointerAddressSpace();
  // Insert the wide load at the head load in the group.
  Builder.SetInsertPoint(Head->getNextNode());
  Value *WidePtr = Builder.CreateBitCast(Head->getPointerOperand(),
                                         G.getWideType()->getPointerTo(AS));
  Instruction *WideLoad = cast<Instruction>(
      Builder.CreateAlignedLoad(WidePtr, Head->getAlignment()));
  LLVM_DEBUG(dbgs() << "LC: Emitted WideLoad: " << *WideLoad << "\n");

  Instruction *InsertPointForShuffle = WideLoad->getNextNode();

  // 3. Generate the shuffles for forwarding the values to their users.
  // Each Load in the group requires its own shuffle.
  size_t Idx = 0;
  for (Instruction *LI : G.getInstrs()) {
    assert(isa<LoadInst>(LI) &&
           "For now we expect to have just load instructions...");
    // Insert the shuffles right after the wide load.
    // Builder.SetInsertPoint(WideLoad->getNextNode());
    Builder.SetInsertPoint(InsertPointForShuffle);

    // Build a shuffle or extract, depending on whether 'LI' is vector or
    // scalar.
    Value *ShuffleOrExtract = nullptr;
    // Vector types need a shuffle for each use of the load.
    if (isa<VectorType>(LI->getType())) {
      SmallVector<Constant *, 8> Mask;
      for (int i = 0, e = getNumElementsSafe(LI->getType()); i != e; ++i) {
        Constant *IdxConstant = Builder.getInt32(Idx++);
        Mask.push_back(IdxConstant);
      }
      Value *ShuffleMask = ConstantVector::get(Mask);
      ShuffleOrExtract = Builder.CreateShuffleVector(
          WideLoad, UndefValue::get(WideLoad->getType()), ShuffleMask);
    }
    // Scalar types need an extractElement for each user.
    else {
      Constant *IdxConstant = Builder.getInt32(Idx++);
      ShuffleOrExtract = Builder.CreateExtractElement(WideLoad, IdxConstant);
    }
    ShuffleOrExtract->setName("LoadCoalescingShuffle_");

    LI->replaceAllUsesWith(ShuffleOrExtract);
    LLVM_DEBUG(dbgs() << "LC: Emitted Shuffle: " << *ShuffleOrExtract << "\n");
    // Delete the now unnecessary instruction.
    LI->eraseFromParent();

    // Update the insert-point
    InsertPointForShuffle = cast<Instruction>(ShuffleOrExtract)->getNextNode();

    if (SanityChecks)
      assert(!verifyFunction(F, &dbgs()));
  }
}

/// This goes through the 'Loads' and builds consecutive chains of loads.
///  A chain looks like this:
///    Head->L1->L2->...->Tail
/// ' ConsecutiveMap' is used to link the loads together.
bool LoadCoalescing::createGroupsAndGenerateCode(BasicBlock *CurrentBB) {

  const BBLoadBucketsTy &Buckets = BBMemRefAnalysis.getLoadBuckets();

  // Each key in LoadListMap can produce one or more groups.
  bool Changed = false;
  for (auto &Bucket : Buckets) {
    if (Bucket.size() == 1)
      // If it is a single element chain, skip.
      continue;
    const auto &BucketMembers = Bucket.getMembers();
    LoadBucketMembers::const_iterator BMIter = BucketMembers.begin();

    while (BMIter != BucketMembers.end()) {
      bool Success;
      // Create a Group Of Instructions that we would want to convert to a
      // widened load.
      MemInstGroup G(DL, MaxVecRegSize);
      Success = buildMaximalGroup(BucketMembers, BMIter, G);
      if (Success) {
        if (scheduleGroup(G)) {
          LLVM_DEBUG(dbgs() << "LC: About to generate code for Group "
                            << G.getId() << "\n");
          codeGen(G);
          Changed |= true;
        }
      } else
        BMIter++;
    }
  }
  return Changed;
}

bool LoadCoalescing::run(BasicBlock &Block) {

  // Initialize the scheduler
  SCH = std::make_unique<Scheduler>(&Block, AA);

  // Collect vector loads based on their pointers.
  BBMemRefAnalysis.populateBasicBlockMemRefBuckets(&Block, AllowScalars);

  return createGroupsAndGenerateCode(&Block);
}


bool LoadCoalescing::run() {
  MaxVecRegSize = (MaxVecRegSizeOpt == 0) ? TTI->getRegisterBitWidth(true)
                                          : MaxVecRegSizeOpt;
  MinVecRegSize = (MinVecRegSizeOpt == 0) ? TTI->getRegisterBitWidth(true)
                                          : MinVecRegSizeOpt;

  bool Changed = false;

  for (BasicBlock &Block : F) {
    // Run LoadCoalescing on a BasicBlock 'Block'.
    Changed |= run(Block);
  }
  return Changed;
}

LoadCoalescing::LoadCoalescing(Function &F, ScalarEvolution &SE,
                               TargetTransformInfo *TTI, AAResults *AA)
    : F(F), DL(F.getParent()->getDataLayout()), SE(SE), TTI(TTI), AA(AA),
      Builder(SE.getContext()), BBMemRefAnalysis(SE, DL) {}

bool LoadCoalescingPass::runImpl(Function *FVal, ScalarEvolution *SEVal,
                                 TargetTransformInfo *TTInfo,
                                 AAResults *AAInfo) {
  F = FVal;
  SE = SEVal;
  TTI = TTInfo;
  AA = AAInfo;
  LoadCoalescing LC(*FVal, *SEVal, TTI, AA);
  return LC.run();
}

PreservedAnalyses LoadCoalescingPass::run(Function &F,
                                          FunctionAnalysisManager &AM) {
  bool Changed =
      runImpl(&F, &AM.getResult<ScalarEvolutionAnalysis>(F),
              &AM.getResult<TargetIRAnalysis>(F), &AM.getResult<AAManager>(F));

  if (Changed) {
    PreservedAnalyses PA;
    PA.preserve<GlobalsAA>();
    PA.preserve<AndersensAA>();
    PA.preserveSet<CFGAnalyses>();
    return PA;
  }
  return PreservedAnalyses::all();
}

LoadCoalescingLegacyPass::LoadCoalescingLegacyPass() : FunctionPass(ID) {
  initializeLoadCoalescingLegacyPassPass(*PassRegistry::getPassRegistry());
}

bool LoadCoalescingLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;
  return Impl.runImpl(&F, &getAnalysis<ScalarEvolutionWrapperPass>().getSE(),
                      &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F),
                      &getAnalysis<AAResultsWrapperPass>().getAAResults());
}

void LoadCoalescingLegacyPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addRequired<AAResultsWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addPreserved<GlobalsAAWrapperPass>();
  AU.addPreserved<AndersensAAWrapperPass>();
  AU.addPreserved<AAResultsWrapperPass>();
}

char LoadCoalescingLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(LoadCoalescingLegacyPass, "load-coalescing",
                      "Load Coalescing", false, false)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_END(LoadCoalescingLegacyPass, "load-coalescing",
                    "Load Coalescing", false, false)

// Public interface to the Load Coalescing pass
Pass *llvm::createLoadCoalescingPass() {
  return new LoadCoalescingLegacyPass();
}

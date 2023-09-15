//===- CSALoopPrep.cpp - ------------------------------*- C++ -*--===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// This pass handles loops marked by the programmer to limit the dynamic
// pipeline depth. The markup appears as the intrinsics
// __builtin_csa_pipeline_limited_loop_entry/exit.
//
// If a loop is marked, this pass creates memory allocation and
// deallocation around the loop(s). It then inserts two intrinsics
// in the loop. The first of these is at the start of the loop
// to acquire a token within the memory pool. The second returns
// the token to the pool. Within the loop, the token value is used
// to index local storage.
//
// During dataflow conversion these pseudoinstructions are removed
// and the loop iterations are limited to the user-specified number.
//
//===----------------------------------------------------------------------===//
//
// \file
//
//===----------------------------------------------------------------------===//

#include "Intel_CSA/Transforms/Scalar/CSALowerParallelIntrinsics.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsCSA.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
using namespace llvm;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "csa-loop-prep"
#define PASS_DESC "CSA: Identify and prepare loops for limiting pipeline depth"

static cl::opt<bool> EnableLoopPrep(
    "csa-loop-locals", cl::Hidden,
    cl::desc("CSA Specific: enable/disable loop pipeline depth limiting pass"),
    cl::init(true));

namespace llvm {
void initializeCSALoopPrepPass(PassRegistry &);
Pass *createCSALoopPrepPass();
} // namespace llvm

namespace {
struct CSALoopPrep : public FunctionPass {
  static char ID;

  LLVMContext *C;
  Module *M;
  const DataLayout *DL;
  DominatorTree *DT;
  PostDominatorTree *PDT;
  LoopInfo *LI;
  bool PreserveLCSSA;

  SmallVector<Instruction *, 32> depthMarkers;
  SmallSet<AllocaInst *, 32> FuncAllocas;
  std::map<Loop *, uint64_t> LoopDepths;
  std::map<Loop *, Instruction *> LoopRegionEntries;
  std::map<Loop *, Instruction *> LoopRegionExits;
  std::map<Loop *, Instruction *> LoopOutermostRegionEntries;
  std::map<Loop *, Instruction *> LoopOutermostRegionExits;
  std::map<Loop *, int> LoopSubloopCount;
  struct LoopAlloca {
    AllocaInst *OuterAllocaInstr;
    AllocaInst *InnerAllocaInstr;
  };
  std::map<Loop *, SmallVector<LoopAlloca, 32>> LoopAllocaInstrs;
  std::map<Loop *, SmallVector<Type *, 32>> LoopAllocaTypes;

  explicit CSALoopPrep() : FunctionPass(ID) {
    initializeCSALoopPrepPass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override { return PASS_DESC; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<PostDominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override;
  bool collectMarkersAndAllocas(Function &F);
  void assignMarkersToLoops(Loop *L);
  void assignMarkerToLoop(Loop *L);
  int collectLocalsInLoops(Loop *L);
  int collectLocalsInLoop(Loop *L);

  void processRegion(SmallVector<Loop *, 32> *RegionLoops,
                     SmallString<16u> WName);

  void processLoops(Loop *L, Value *Pool, SmallString<16u> LName);

  void processLoop(Loop *L, Value *Pool, StructType *TokenPoolAOSType,
                   int PoolElement, SmallString<16u> LName);

  Value *createPool(SmallVector<Loop *, 32> *RegionLoops,
                    StructType *&TokenPoolAOSType, SmallString<16u> PoolName);

  Value *createPool(SmallVector<Type *, 32> &AllocaTypes,
                    Instruction *regionEntry, Instruction *regionExit,
                    uint64_t pipelineDepth, StructType *&TokenPoolAOSType,
                    SmallString<16u> PoolName);

  Value *useSubPool(Loop *L, Value *TokenPool, Value *&TokenPoolStruct,
                    StructType *TokenPoolAOSType, int PoolElement,
                    SmallString<16u> PoolName);

  int collectLoopAllocas(Loop *L);

  uint64_t createFrameType(SmallVector<Type *, 32> &AllocaTypes,
                           StructType *&TokenPoolAOSType);

  uint64_t programmerSpecifiedPipelineDepth(Loop *L);

  bool depthRegionEnclosesLoop(Loop *L, Instruction *regionEntry,
                               Instruction *regionExit);

  bool isLoopMarkedParallel(Loop *L);

  const DebugLoc &getAnyBlockLoc(BasicBlock *);
};
} // namespace

char CSALoopPrep::ID = 0;
INITIALIZE_PASS_BEGIN(CSALoopPrep, "csa-loop-prep", PASS_DESC, false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(CSALoopPrep, "csa-loop-prep", PASS_DESC, false, false)

Pass *llvm::createCSALoopPrepPass() { return new CSALoopPrep(); }

bool CSALoopPrep::runOnFunction(Function &F) {
  bool Changed = false;

  if (skipFunction(F))
    return Changed;

  if (not EnableLoopPrep)
    return Changed;

  C = &F.getContext();
  M = F.getParent();
  DL = &M->getDataLayout();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  PreserveLCSSA = mustPreserveAnalysisID(LCSSAID);

  depthMarkers.clear();
  FuncAllocas.clear();
  LoopDepths.clear();
  LoopRegionEntries.clear();
  LoopRegionExits.clear();
  LoopOutermostRegionEntries.clear();
  LoopOutermostRegionExits.clear();
  LoopSubloopCount.clear();
  LoopAllocaInstrs.clear();
  LoopAllocaTypes.clear();

  if (!collectMarkersAndAllocas(F))
    return Changed;

  LLVM_DEBUG(errs() << "\nFunction " << F.getName()
                    << " before local storage replication:";
             F.dump(); errs() << "\n");

  // Assign depth markers to loops
  for (LoopInfo::iterator Liter = LI->begin(), Lend = LI->end(); Liter != Lend;
       ++Liter) {
    Loop *L = *Liter;
    LLVM_DEBUG(errs() << "Assigning depth markers of outermost loop\n");
    assignMarkersToLoops(L);
  }

  // Collect loop-local variables
  int RegionLSCount = 0;
  for (Loop *L : *LI) {
    LLVM_DEBUG(errs() << "Collecting locals in outermost loop\n");
    int LoopLSCount = collectLocalsInLoops(L);
    LLVM_DEBUG(errs() << "Loop's subloop count = " << LoopSubloopCount[L]
                      << "\n");
    LLVM_DEBUG(errs() << "Loop's self + subloop alloca count = " << LoopLSCount
                      << "\n\n");
    RegionLSCount += LoopLSCount;
  }

  // Check if any outermost loop contains local storage
  if (RegionLSCount == 0)
    return false;

  // Process top-level loops in groups based on region entry/exit
  int RegionNo = 0;
  SmallString<16u> RName;
  SmallVector<Loop *, 32> RegionLoops;
  Instruction *currentRegionEntry = nullptr;
  for (Loop *L : *LI) {
    Instruction *regionEntry = LoopOutermostRegionEntries[L];
    if (currentRegionEntry == nullptr) {
      currentRegionEntry = regionEntry;
      RegionLoops.push_back(L);
    } else {
      if (currentRegionEntry == regionEntry) {
        RegionLoops.push_back(L);
      } else {
        raw_svector_ostream(RName) << "ls.R" << RegionNo;
        processRegion(&RegionLoops, RName);
        ++RegionNo;
        RegionLoops.clear();
        RegionLoops.push_back(L);
        currentRegionEntry = regionEntry;
      }
    }
  }

  if (RegionNo > 0)
    raw_svector_ostream(RName) << "ls.R" << RegionNo;
  else
    raw_svector_ostream(RName) << "ls";

  processRegion(&RegionLoops, RName);

  LLVM_DEBUG(errs() << "\nFunction " << F.getName()
                    << " after local storage replication:";
             F.dump(); errs() << "\n");

  return true;
}

bool CSALoopPrep::collectMarkersAndAllocas(Function &F) {
  depthMarkers.clear();
  FuncAllocas.clear();
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      const IntrinsicInst *const II = dyn_cast<IntrinsicInst>(&I);
      if (II and II->getIntrinsicID() == Intrinsic::csa_pipeline_limited_exit) {
        depthMarkers.push_back(&I);
      }
      if (AllocaInst *AI = dyn_cast<AllocaInst>(&I)) {
        FuncAllocas.insert(AI);
      }
    }
  }

  if (!depthMarkers.empty()) {
    LLVM_DEBUG(errs() << "Found these depth markers:\n";
               for (auto DM = depthMarkers.begin(), DME = depthMarkers.end();
                    DM != DME; ++DM) {
                 Instruction *regionExit = *DM;
                 regionExit->dump();
               });
    LLVM_DEBUG(errs() << "Found these array allocas:\n"; for (auto *const FA
                                                              : FuncAllocas) {
      Instruction *allocaInstr = FA;
      allocaInstr->dump();
    });
  }

  return !depthMarkers.empty();
}

void CSALoopPrep::assignMarkersToLoops(Loop *L) {
  for (Loop *SL : L->getSubLoops()) {
    assignMarkersToLoops(SL);
  }
  assignMarkerToLoop(L);
}

void CSALoopPrep::assignMarkerToLoop(Loop *L) {
  // Check if this loop has its pipeline depth specified
  LLVM_DEBUG(errs() << "\nassignMarkerToLoop on a loop with header name ";
             errs() << L->getHeader()->getName() << "\n");
  LLVM_DEBUG(errs() << "L=" << L << ", L->parent=" << L->getParentLoop()
                    << "\n");

  uint64_t pipelineDepth = programmerSpecifiedPipelineDepth(L);
  LLVM_DEBUG(errs() << "\n**** Loop named " << L->getName()
                    << " has depth = " << pipelineDepth << "\n");
  LoopDepths.emplace(L, pipelineDepth);
}

uint64_t CSALoopPrep::programmerSpecifiedPipelineDepth(Loop *L) {
  LLVM_DEBUG(errs() << "\nChecking a loop for pipeline depth marking\n");
  Instruction *regionEntry;
  Instruction *regionExit;
  uint64_t loopPipelineDepth = 0;
  for (auto DM = depthMarkers.begin(), DME = depthMarkers.end(); DM != DME;
       ++DM) {
    regionExit = *DM;
    regionEntry = nullptr;
    Value *regionId = nullptr;
    ConstantInt *pipelineDepth = nullptr;
    if (match(regionExit, m_Intrinsic<Intrinsic::csa_pipeline_limited_exit>(
                              m_Instruction(regionEntry))) and
        match(regionEntry,
              m_Intrinsic<Intrinsic::csa_pipeline_limited_entry>(
                  m_Value(regionId), m_ConstantInt(pipelineDepth)))) {
      LLVM_DEBUG(
          errs() << "Found pipeline-limited entry/exit pair with region id "
                 << *regionId << "):\n");
      bool appliesToLoop = false;
      if (depthRegionEnclosesLoop(L, regionEntry, regionExit)) {
        appliesToLoop = true;
        Loop *parent = L->getParentLoop();
        while (parent) {
          if (depthRegionEnclosesLoop(parent, regionEntry, regionExit)) {
            appliesToLoop = false;
            break;
          }
          parent = parent->getParentLoop();
        }
      }
      if (appliesToLoop) {
        LLVM_DEBUG(errs() << "This marker applies to loop " << *regionId
                          << "\n");
        // Set outermost region
        if (!LoopOutermostRegionEntries.count(L)) {
          LLVM_DEBUG(errs() << "First Outermost region marker\n");
          LoopOutermostRegionEntries.emplace(L, regionEntry);
          LoopOutermostRegionExits.emplace(L, regionExit);
        } else {
          if (DT->dominates(regionEntry, LoopOutermostRegionEntries[L])) {
            LLVM_DEBUG(
                errs()
                << "Replacing old Outermost region marker with this one\n");
            LoopOutermostRegionEntries[L] = regionEntry;
            LoopOutermostRegionExits[L] = regionExit;
          }
        }
        // Set innermost region
        if (!LoopRegionEntries.count(L)) {
          LLVM_DEBUG(errs() << "First region marker\n");
          LoopRegionEntries.emplace(L, regionEntry);
          LoopRegionExits.emplace(L, regionExit);
          loopPipelineDepth = pipelineDepth->getZExtValue();
        } else {
          if (!DT->dominates(regionEntry, LoopRegionEntries[L])) {
            LLVM_DEBUG(errs() << "Replacing old region marker with this one\n");
            LoopRegionEntries[L] = regionEntry;
            LoopRegionExits[L] = regionExit;
            loopPipelineDepth = pipelineDepth->getZExtValue();
          }
        }
      }
    }
  }
  return loopPipelineDepth;
}

bool CSALoopPrep::depthRegionEnclosesLoop(Loop *L, Instruction *regionEntry,
                                          Instruction *regionExit) {
  if (DT->dominates(regionEntry->getParent(), L->getHeader())) {
    LLVM_DEBUG(errs() << "OK, the region.entry dominates the loop header\n");
    SmallVector<BasicBlock *, 2> exits;
    L->getExitBlocks(exits);
    for (BasicBlock *const exit : exits) {
      if (!PDT->dominates(regionExit->getParent(), exit)) {
        LLVM_DEBUG(
            errs() << "Drat, region.exit does not post-dominate one exit\n");
        return false;
      }
    }
    return true;
  }
  return false;
}

int CSALoopPrep::collectLocalsInLoops(Loop *L) {
  // Process subloops first
  // Gather locals of all subloops
  int SubloopCount = 0;
  for (Loop *SL : L->getSubLoops()) {
    int LSCount;
    if ((LSCount = collectLocalsInLoops(SL))) {
      ++SubloopCount;
      StructType *TokenPoolAOSType;
      createFrameType(LoopAllocaTypes[SL], TokenPoolAOSType);
      uint64_t SubloopDepth = LoopDepths[SL];
      // If the subloop is not pipelined, then storage is replicated to just 1
      if (SubloopDepth == 0)
        SubloopDepth = 1;
      ArrayType *SubloopPoolType =
          ArrayType::get(TokenPoolAOSType, SubloopDepth);
      LLVM_DEBUG(errs() << "\n**** SubloopPoolType:\n";
                 SubloopPoolType->dump());
      LoopAllocaTypes[L].push_back(SubloopPoolType);
    }
  }

  // Make a list of loop-local arrays in this loop.
  // Save the index of the first member that is this loops own locals
  LoopSubloopCount[L] = SubloopCount;
  int LoopLocalCount = collectLocalsInLoop(L);
  return SubloopCount + LoopLocalCount;
}

int CSALoopPrep::collectLocalsInLoop(Loop *L) {
  // Check if this loop has its pipeline depth specified
  LLVM_DEBUG(errs() << "\ncollectLocalsInLoop on a loop with header name ";
             errs() << L->getHeader()->getName() << "\n");
  LLVM_DEBUG(errs() << "L=" << L << ", L->parent=" << L->getParentLoop());

  uint64_t pipelineDepth = LoopDepths[L];

  if (pipelineDepth == 0) {
    LLVM_DEBUG(errs() << "Pipeline depth not specified - done with loop\n\n");
    return 0;
  }
  LLVM_DEBUG(errs() << "programmerSpecifiedPipelineDepth " << pipelineDepth
                    << " found\n");

  // If this is not a parallel loop, we don't care about locals
  if (!isLoopMarkedParallel(L)) {
    LLVM_DEBUG(errs() << "Not marked parallel - done with loop\n\n");
    return 0;
  }

  // Check if any local storage remains in loop
  int LSCount = 0;
  if (!(LSCount = collectLoopAllocas(L))) {
    LLVM_DEBUG(errs() << "No array allocas found - done with loop\n\n");
    return 0;
  }

  LLVM_DEBUG(errs() << "\n**** Loop-local storage in loop named "
                    << L->getName() << "\n");
  // Print
  // 0:
  //  outeralloc
  //  inneralloca
  //  type
  int count = 0;
  for (auto ALoopAlloca : LoopAllocaInstrs[L]) {
    if (ALoopAlloca.OuterAllocaInstr)
      LLVM_DEBUG(errs() << " "; ALoopAlloca.OuterAllocaInstr->dump());
    else
      LLVM_DEBUG(errs() << "   <nullptr>\n");
    if (ALoopAlloca.InnerAllocaInstr)
      LLVM_DEBUG(errs() << " "; ALoopAlloca.InnerAllocaInstr->dump());
    else
      LLVM_DEBUG(errs() << "   <nullptr>\n");
    LLVM_DEBUG(errs() << "   "; LoopAllocaTypes[L][count]->dump();
               errs() << "\n");
    count++;
  }

  return LSCount;
}

// This version checks if the alloca is dominated by the loop's
// depth marker. If not, it is not local to this loop.
// Next, it checks if any of an alloca's users are in the loop.
// If there is no use in the loop, that alloca is not for this loop
// but for a co-worker.
// This version depends on processing loops depth-first.
int CSALoopPrep::collectLoopAllocas(Loop *L) {
  int loopAllocas = 0;

  // Check if any local storage remains associated with loop
  for (auto *const FA : FuncAllocas) {
    AllocaInst *AllocaInstr = FA;
    LLVM_DEBUG(errs() << "Examining this alloca instr:\n"; AllocaInstr->dump());
    auto RegionEntry = LoopRegionEntries[L];

    if (DT->dominates(RegionEntry, AllocaInstr)) {
      LLVM_DEBUG(errs() << "Depth marker dominates this use; possibly private "
                           "to this loop\n");
    } else {
      LLVM_DEBUG(errs() << "This alloca is *not* dominated by region.entry "
                           "marker, therefore not loop-private\n");
      continue;
    }

    bool UsedInLoop = false;
    for (auto *U : AllocaInstr->users()) {
      if (auto *I = dyn_cast<Instruction>(U)) {
        LLVM_DEBUG(errs() << "Here's a user:\n"; I->dump());
        if (L->contains(I)) {
          LLVM_DEBUG(errs() << "This user is in this loop\n");
          UsedInLoop = true;
          break;
        }
      }
    }
    if (UsedInLoop) {
      LoopAlloca AnAlloca = {nullptr, nullptr};
      if (L->contains(AllocaInstr)) {
        LLVM_DEBUG(errs() << "Adding Instr to loop's inner allocas:\n";
                   AllocaInstr->dump());
        AnAlloca.InnerAllocaInstr = AllocaInstr;
      } else {
        LLVM_DEBUG(errs() << "Adding Instr to loop's outer allocas:\n";
                   AllocaInstr->dump());
        AnAlloca.OuterAllocaInstr = AllocaInstr;
      }
      // Add alloca inst to this loop's list of privates
      LoopAllocaInstrs[L].push_back(AnAlloca);
      // Note the depth-wide new array type that will be needed
      LoopAllocaTypes[L].push_back(AllocaInstr->getAllocatedType());
      ++loopAllocas;
      FuncAllocas.erase(AllocaInstr);
    } else {
      LLVM_DEBUG(errs() << "Could not ascribe this alloca to this worker.\n");
    }
  }

  return loopAllocas;
}

uint64_t CSALoopPrep::createFrameType(SmallVector<Type *, 32> &AllocaTypes,
                                      StructType *&TokenPoolAOSType) {
  // TokenPoolAOSType = StructType::create(*C, AllocaTypes);
  TokenPoolAOSType = StructType::get(*C, AllocaTypes);
  LLVM_DEBUG(errs() << "**** Pool AOS Type:\n"; TokenPoolAOSType->dump());
  uint64_t AllocaSize = DL->getTypeAllocSize(TokenPoolAOSType);
  LLVM_DEBUG(errs() << "**** Pool Allocation Size = " << AllocaSize << "\n\n");
  return AllocaSize;
}

void CSALoopPrep::processRegion(SmallVector<Loop *, 32> *RegionLoops,
                                SmallString<16u> RName) {
  // Handle a single-loop region as just a loop
  if (RegionLoops->size() == 1) {
    processLoops((*RegionLoops)[0], 0, RName);
    return;
  }

  StructType *TokenPoolAOSType;
  Value *TokenPool = createPool(RegionLoops, TokenPoolAOSType, RName);
  Value *TokenPoolStruct = nullptr;
  int PoolElement = 0;
  for (Loop *const SL : *RegionLoops) {
    // If the loop uses local storage, then handle it
    if (LoopAllocaTypes[SL].size() > 0) {
      SmallString<16u> WName = RName;
      raw_svector_ostream(WName) << ".W";
      Value *TokenSubPool = useSubPool(SL, TokenPool, TokenPoolStruct,
                                       TokenPoolAOSType, PoolElement, WName);
      raw_svector_ostream(WName) << PoolElement;
      processLoops(SL, TokenSubPool, WName);
      ++PoolElement;
    }
  }
}

void CSALoopPrep::processLoops(Loop *L, Value *TokenPool,
                               SmallString<16u> LName) {
  // If this loop and its subloops have no local storage, we're done
  if (LoopAllocaTypes[L].size() == 0) {
    return;
  }

  StructType *TokenPoolAOSType;

  // If existing pool is not handed to us, we create the pool
  if (!TokenPool) {
    TokenPool = createPool(LoopAllocaTypes[L], LoopOutermostRegionEntries[L],
                           LoopOutermostRegionExits[L], LoopDepths[L],
                           TokenPoolAOSType, LName);
  } else {
    Type *TTAP = TokenPool->getType();
    Type *TTA = TTAP->getPointerElementType();
    Type *TT = TTA->getArrayElementType();
    StructType *STT = dyn_cast<StructType>(TT);
    TokenPoolAOSType = STT;
  }

  // If this loop has its own loop-locals, use token pool for them
  // Loop subloop local count is index of first member of this loop
  int LoopLocals = LoopAllocaTypes[L].size();
  int SubLoopLocals = LoopSubloopCount[L];
  if (LoopLocals > SubLoopLocals) {
    processLoop(L, TokenPool, TokenPoolAOSType, SubLoopLocals, LName);
  }

  // The index of the first member used for subloops is 0
  Value *TokenPoolStruct = nullptr;
  int PoolElement = 0;
  for (Loop *SL : L->getSubLoops()) {
    // If the subloop uses local storage, then handle it
    if (LoopAllocaTypes[SL].size() > 0) {
      SmallString<16u> SLName = LName;
      raw_svector_ostream(SLName) << ".W";
      Value *TokenSubPool = useSubPool(SL, TokenPool, TokenPoolStruct,
                                       TokenPoolAOSType, PoolElement, SLName);
      raw_svector_ostream(SLName) << PoolElement;
      processLoops(SL, TokenSubPool, SLName);
      ++PoolElement;
    }
  }
}

Value *CSALoopPrep::createPool(SmallVector<Loop *, 32> *RegionLoops,
                               StructType *&TokenPoolAOSType,
                               SmallString<16u> PoolName) {
  Loop *L = RegionLoops->front();
  SmallVector<Type *, 32> RegionAllocaTypes;
  for (Loop *const RL : *RegionLoops) {
    uint64_t LoopDepth = LoopDepths[RL];
    SmallVector<Type *, 32> &LoopLSTypes = LoopAllocaTypes[RL];
    if (LoopLSTypes.size() > 0) {
      StructType *RegionPoolAOSType;
      createFrameType(LoopLSTypes, RegionPoolAOSType);
      ArrayType *RegionLoopPoolType =
          ArrayType::get(RegionPoolAOSType, LoopDepth);
      LLVM_DEBUG(errs() << "RegionLoopPoolType:\n"; RegionLoopPoolType->dump());
      RegionAllocaTypes.push_back(RegionLoopPoolType);
    }
  }
  if (!RegionAllocaTypes.empty()) {
    return createPool(RegionAllocaTypes, LoopOutermostRegionEntries[L],
                      LoopOutermostRegionExits[L], 1, TokenPoolAOSType,
                      PoolName);
  }
  return 0;
}

Value *CSALoopPrep::createPool(SmallVector<Type *, 32> &AllocaTypes,
                               Instruction *regionEntry,
                               Instruction *regionExit, uint64_t pipelineDepth,
                               StructType *&TokenPoolAOSType,
                               SmallString<16u> PoolName) {
  // Create token pool and change local storage to use pool pointer
  createFrameType(AllocaTypes, TokenPoolAOSType);
  uint64_t AllocaSize = DL->getTypeAllocSize(TokenPoolAOSType);

  // Create memory pool create/destroy intrinsic calls
  Function *CSAMalloc, *CSAFree;
  CSAMalloc = M->getFunction("CsaMemAlloc");
  CSAFree = M->getFunction("CsaMemFree");

  Instruction *InsertPoint = regionEntry;
  IRBuilder<> IA(InsertPoint);
  Value *PoolSizeArg = IA.getInt32(AllocaSize * pipelineDepth);
  ArrayRef<Value *> Args = {PoolSizeArg};
  CallInst *TokenPool;
  TokenPool = IA.CreateCall(CSAMalloc, Args, PoolName + ".pool");
  LLVM_DEBUG(errs() << "TokenPool create added to preHeader:\n";
             TokenPool->dump());
  InsertPoint = regionExit;
  IRBuilder<>{InsertPoint}.CreateCall(CSAFree, TokenPool);
  LLVM_DEBUG(errs() << "TokenPool destroy added to loop exit:\n";
      InsertPoint->getPrevNonDebugInstruction()->dump());

  return TokenPool;
}

Value *CSALoopPrep::useSubPool(Loop *L, Value *TokenPool,
                               Value *&TokenPoolStruct,
                               StructType *TokenPoolAOSType, int PoolElement,
                               SmallString<16u> PoolName) {
  BasicBlock *LoopPreheader = L->getLoopPreheader();
  if (!LoopPreheader) {
    LoopPreheader = InsertPreheaderForLoop(L, DT, LI, nullptr, PreserveLCSSA);
    if (!LoopPreheader) {
      // assert
      LLVM_DEBUG(
          errs() << "Loop has no preheader and we failed to create one\n");
      return 0;
    }
  }
  Instruction *InsertPt;
  if (TokenPoolStruct == nullptr) {
    InsertPt = dyn_cast<Instruction>(TokenPool)->getNextNode();
    IRBuilder<> IA(InsertPt);
    TokenPoolStruct = IA.CreateBitCast(
        TokenPool, TokenPoolAOSType->getPointerTo(), PoolName + ".all");
  } else {
    InsertPt = dyn_cast<Instruction>(TokenPoolStruct)->getNextNode();
  }
  IRBuilder<> IA(InsertPt);
  SmallString<16u> WName = PoolName;
  raw_svector_ostream(WName) << PoolElement;
  IA.getInt32(PoolElement);
  Value *MemberNumber = IA.getInt32(PoolElement);
  Value *MemberZero = IA.getInt32(0);
  Value *TokenSubPool = IA.CreateGEP(TokenPoolAOSType, TokenPoolStruct,
                                     {MemberZero, MemberNumber}, WName);
  return TokenSubPool;
}

void CSALoopPrep::processLoop(Loop *L, Value *TokenPool,
                              StructType *TokenPoolAOSType, int PoolElement,
                              SmallString<16u> LName) {
  const auto gen_call = [&](Instruction *const IP, Intrinsic::ID Callee,
                            ArrayRef<Value *> Args, const char *Name) {
    CallInst *const Res = IRBuilder<>{IP}.CreateCall(
        Intrinsic::getDeclaration(M, Callee), Args, Name);
    return Res;
  };

  uint64_t AllocaSize = DL->getTypeAllocSize(TokenPoolAOSType);
  uint64_t pipelineDepth = LoopDepths[L];

  // Create a variable to hold the local storage slot address
  Instruction *InsertPt = LoopRegionEntries[L];
  IRBuilder<> IAT0(InsertPt);
  Type *I8PtrTy =
      IAT0.getInt8PtrTy(TokenPool->getType()->getPointerAddressSpace());
  auto *SlotAlloca = IAT0.CreateAlloca(I8PtrTy, nullptr, "ls.slot.alloca");
  LLVM_DEBUG(errs() << "Slot Alloca:\n"; SlotAlloca->dump());

  // Assign slot0 address
  if (TokenPool->getType() != I8PtrTy) {
    TokenPool = IAT0.CreateBitCast(TokenPool, I8PtrTy);
  }
  auto *Slot0Saved =
      IAT0.CreateAlignedStore(TokenPool, SlotAlloca, Align(8), false);
  LLVM_DEBUG(errs() << "Slot0 value assigned:\n"; Slot0Saved->dump());

  // Handle outer allocas
  // Assign firstprivate values to frame0 of pool
  // Then replicate across all frames (TODO)
  InsertPt = Slot0Saved->getNextNode();
  IRBuilder<> IA0(InsertPt);
  uint64_t Count = PoolElement;
  Value *MemberZero = IA0.getInt32(0);
  Value *Frame0StructPtr = IA0.CreateBitCast(
      TokenPool, TokenPoolAOSType->getPointerTo(), LName + ".F0");
  SmallVector<LoopAlloca, 32> &AllocaInstrs = LoopAllocaInstrs[L];
  SmallVector<Instruction *, 32> DeleteList;
  Instruction *TTPt = nullptr;
  for (auto ALoopAlloca : AllocaInstrs) {
    Instruction *Instr = ALoopAlloca.OuterAllocaInstr;
    // Save the location of the first inner alloca
    if (!TTPt and ALoopAlloca.InnerAllocaInstr)
      TTPt = ALoopAlloca.InnerAllocaInstr;
    // There could be inner allocas with no matching outer alloca
    if (!Instr)
      continue;
    // Allocas outside loop are for first iteration
    // They will be followed by an initialization for firstprivate/reduction
    Value *MemberNumber = IA0.getInt32(Count);
    SmallString<16u> MName = LName;
    raw_svector_ostream(MName) << ".F0.M" << Count;
    Value *MemberPtr = IA0.CreateGEP(TokenPoolAOSType, Frame0StructPtr,
                                     {MemberZero, MemberNumber}, MName);
    LLVM_DEBUG(errs() << "\nReplacing:\n"; Instr->dump());
    Instr->replaceAllUsesWith(MemberPtr);
    LLVM_DEBUG(errs() << "Replacement\n"; MemberPtr->dump());
    LLVM_DEBUG(errs() << "Add original alloca instruction to delete list:\n";
               Instr->dump());
    DeleteList.push_back(Instr);
    ++Count;
  }

  // Create token take/return intrinsic calls
  if (!TTPt) {
    TTPt = L->getHeader()->getFirstNonPHI();
  }
  IRBuilder<> IATT(TTPt);
  Value *FrameSize = IATT.getInt32(AllocaSize);
  Value *Depth = IATT.getInt32(pipelineDepth);
  ArrayRef<Value *> Args = {TokenPool, FrameSize, Depth};
  CallInst *const TokenTake = gen_call(
      TTPt, Intrinsic::csa_pipeline_depth_token_take, Args, "ls.token");
  LLVM_DEBUG(errs() << "Token take created in loop:\n"; TokenTake->dump());
  IATT.CreateAlignedStore(TokenTake, SlotAlloca, Align(8), false);
  LLVM_DEBUG(errs() << "Slot-d value assigned:\n";
             TokenTake->getNextNonDebugInstruction()->dump());
  BasicBlock *const LoopEndBlock = L->getLoopLatch();
  if (!LoopEndBlock) {
    assert(LoopEndBlock && "Loop must have single branch back to header");
  }
  Instruction *const LoopEndInstr = LoopEndBlock->getTerminator();
  IRBuilder<> IATR(LoopEndInstr);
  auto *SlotRecovered = IATR.CreateAlignedLoad(SlotAlloca, Align(8));
  LLVM_DEBUG(errs() << "Slot-d value recovered:\n"; SlotRecovered->dump());
  Args = {TokenPool, SlotRecovered};
  gen_call(LoopEndInstr, Intrinsic::csa_pipeline_depth_token_return, Args, "");
  LLVM_DEBUG(errs() << "Token return added to loop terminator:\n";
      LoopEndInstr->getPrevNonDebugInstruction()->dump());

  // Assign alloca'd variables offsets into memory pool
  InsertPt = TokenTake->getNextNode();
  IRBuilder<> IA(InsertPt);
  Count = PoolElement;
  Value *FrameStructPtr = IA.CreateBitCast(
      TokenTake, TokenPoolAOSType->getPointerTo(), LName + "F.all");

  // Now replace allocas that are inside the loop
  // These were generated for privates
  for (auto ALoopAlloca : AllocaInstrs) {
    Instruction *Instr = ALoopAlloca.InnerAllocaInstr;
    // There could be outer allocas with no matching inner alloca
    if (!Instr)
      continue;
    Value *MemberNumber = IA.getInt32(Count);
    SmallString<16u> MName = LName;
    raw_svector_ostream(MName) << ".Fd.M" << Count;
    Value *MemberPtr = IA.CreateGEP(TokenPoolAOSType, FrameStructPtr,
                                    {MemberZero, MemberNumber}, MName);
    LLVM_DEBUG(errs() << "\nReplacing:\n"; Instr->dump());
    Instr->replaceAllUsesWith(MemberPtr);
    LLVM_DEBUG(errs() << "Replacement\n"; MemberPtr->dump());
    LLVM_DEBUG(errs() << "Add original alloca instruction to delete list:\n";
               Instr->dump());
    DeleteList.push_back(Instr);
    ++Count;
  }

  for (auto *Instr : DeleteList) {
    LLVM_DEBUG(errs() << "Erasing original alloca instruction:\n";
               Instr->dump());
    Instr->eraseFromParent();
  }
}

// The following borrowed from CSAInnerLoopPrep.cpp

bool CSALoopPrep::isLoopMarkedParallel(Loop *L) {
  if (auto *LoopID = L->getLoopID()) {
    LLVM_DEBUG(dbgs() << "Loop with metadata: " << L->getHeader()->getName()
                      << "\n");
    for (unsigned Indx = 1; Indx < LoopID->getNumOperands(); ++Indx) {
      if (auto *T = dyn_cast<MDTuple>(LoopID->getOperand(Indx)))
        if (T->getNumOperands() != 0)
          if (auto *S = dyn_cast<MDString>(T->getOperand(0)))
            if (S->getString() == CSALoopTag::Parallel) {
              LLVM_DEBUG(dbgs() << "The loop is marked with Parallel.\n");
              return true;
            }
    }
  }

  for (BasicBlock *bb : L->blocks()) {
    for (Instruction &i : *bb) {
      Instruction *section_exit = &i;
      Instruction *section_entry = nullptr;
      Instruction *region_entry = nullptr;
      Value *regionId = nullptr;
      if (match(section_exit, m_Intrinsic<Intrinsic::csa_parallel_section_exit>(
                                  m_Instruction(section_entry))) and
          match(section_entry,
                m_Intrinsic<Intrinsic::csa_parallel_section_entry>(
                    m_Instruction(region_entry))) and
          match(region_entry, m_Intrinsic<Intrinsic::csa_parallel_region_entry>(
                                  m_Value(regionId))) and
          LI->getLoopFor(section_entry->getParent()) == L and
          LI->getLoopFor(section_exit->getParent()) == L) {
        if (DT->dominates(section_entry->getParent(), L->getHeader()) and
            PDT->dominates(section_exit->getParent(), L->getHeader())) {
          LLVM_DEBUG(errs() << "Found (region id is " << *regionId << "):\n"
                            << "\t" << *region_entry << "\n"
                            << "\t\t" << *section_entry << " (depth "
                            << L->getLoopDepth() << ")\n"
                            << "\t\t" << *section_exit << "\n");
          return true;
        }
      }
    }
  }
  return false;
}

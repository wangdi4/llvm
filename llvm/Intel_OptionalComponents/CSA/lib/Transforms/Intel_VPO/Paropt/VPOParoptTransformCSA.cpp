//===- VPOParoptTransformCSA.cpp - Transformation of W-Region for CSA -*- C++ -*-===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements CSA specific lowering for work regions.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTransform.h"
#include "llvm/IR/DiagnosticPrinter.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-transform-csa"

static cl::opt<unsigned> LoopWorkersDefault(
  "csa-omp-loop-workers-default", cl::init(1), cl::ReallyHidden,
  cl::desc("Defines default number of workers for OpenMP loops with no "
           "num_threads clause."));

static cl::opt<bool> UseExpLoopPrivatizer(
  "csa-omp-exp-loop-privatizer", cl::init(false), cl::ReallyHidden,
  cl::desc("Use experimental privatizer for OpenMP loops."));

namespace {

class CSADiagInfo : public DiagnosticInfoWithLocationBase {
  const Twine &Msg;

public:
  CSADiagInfo(const Function &F, const DiagnosticLocation &Loc,
              const Twine &Msg, DiagnosticSeverity DS = DS_Warning)
    : DiagnosticInfoWithLocationBase(
        static_cast<DiagnosticKind>(DK_FirstPluginKind), DS, F, Loc),
      Msg(Msg)
  {}

  void print(DiagnosticPrinter &DP) const override {
    if (isLocationAvailable())
      DP << getLocationStr() << ": ";
    DP << "CSA - " << Msg;
    if (!isLocationAvailable())
      DP << " (use -g for location info)";
  }
};

} // anonymous namespace

// Helper class which performs CSA specific privatization transformation for
// a work region.
class VPOParoptTransform::CSAPrivatizer {
protected:
  VPOParoptTransform &PT;

  // Work region which is being transformed.
  WRegionNode *W = nullptr;

  // Shortcuts to VPOParoptTransform's DT, LI and SE members.
  DominatorTree *DT = nullptr;
  LoopInfo *LI = nullptr;
  ScalarEvolution *SE = nullptr;

  // Init, fini and last iteration basic blocks. Alloca instructions for all
  // private flavors as well as the intialization for for first private and
  // reduction variables is inserted into InitBB. Destructors for all types
  // of privates are inserted into the FiniBB as well as reduction updates.
  // Copyout code for lastprivate variables is inserted into the LastIterBB.
  BasicBlock *InitBB = nullptr;
  BasicBlock *FiniBB = nullptr;
  BasicBlock *LastIterBB = nullptr;

private:
  // Creates alloca instruction for the given item, inserts it to the InitBB and
  // replaces all uses of the original variable within work region with a
  // private instance.
  virtual void genPrivItem(Item *I, WRegionNode *W, StringRef Suffix) {
    auto *Old = I->getOrig();
    auto *New = PT.genPrivatizationAlloca(W, Old, getInitBB()->getTerminator(),
                                          Suffix);
    I->setNew(New);
    PT.genPrivatizationReplacement(W, Old, New, I);
  }

protected:
  virtual BasicBlock* getInitBB() {
    if (InitBB)
      return InitBB;
    auto *EntryBB = W->getEntryBBlock();
    InitBB = SplitBlock(EntryBB, EntryBB->getTerminator(), DT, LI);
    InitBB->setName("omp.priv.init");
    return InitBB;
  }

  virtual BasicBlock* getFiniBB() {
    if (FiniBB)
      return FiniBB;
    FiniBB = W->getExitBBlock();
    auto *New = SplitBlock(FiniBB, FiniBB->getFirstNonPHI(), DT, LI);
    W->setExitBBlock(New);
    FiniBB->setName(".omp.priv.fini");
    return FiniBB;
  }

  virtual BasicBlock* getLastIterBB() {
    llvm_unreachable("unexpected work region kind");
    return nullptr;
  }

  // Creates privatization code for the given private var.
  void genPrivVar(PrivateItem *I, WRegionNode *W) {
    genPrivItem(I, W, ".priv");
    if (auto *C = I->getConstructor())
      VPOParoptUtils::genConstructorCall(C, I->getNew(), I->getNew());
    if (auto *D = I->getDestructor())
      VPOParoptUtils::genDestructorCall(D, I->getNew(),
          getFiniBB()->getFirstNonPHI());
  }

  // Creates privatization code for the given firstprivate var.
  void genFPrivVar(FirstprivateItem *I, WRegionNode *W) {
    if (auto *LP = I->getInLastprivate())
      if (auto *NewLP = LP->getNew()) {
        I->setNew(NewLP);
        return;
      }
    genPrivItem(I, W, ".fpriv");
    PT.genFprivInit(I, cast<Instruction>(I->getNew())->getNextNode());
    if (auto *D = I->getDestructor())
      VPOParoptUtils::genDestructorCall(D, I->getNew(),
          getFiniBB()->getFirstNonPHI());
  }

  // Creates privatization code for the given lastprivate var, except the
  // copyout code which is created by genLPrivVarCopyout().
  void genLPrivVar(LastprivateItem *I, WRegionNode *W) {
    if (auto *FP = I->getInFirstprivate())
      if (auto *NewFP = FP->getNew()) {
        I->setNew(NewFP);
        return;
      }

    genPrivItem(I, W, ".lpriv");
    if (auto *C = I->getConstructor())
      VPOParoptUtils::genConstructorCall(C, I->getNew(), I->getNew());
    if (auto *D = I->getDestructor())
      VPOParoptUtils::genDestructorCall(D, I->getNew(),
          getFiniBB()->getFirstNonPHI());
  }

  // Creates copyout code for the given lastprivate var.
  void genLPrivVarCopyout(LastprivateItem *I) {
    PT.genLprivFini(I, getLastIterBB()->getTerminator());
  }

  // Creates reduction code for the given var.
  void genRedVar(ReductionItem *I, WRegionNode *W) {
    genPrivItem(I, W, ".red");
    PT.genReductionInit(I, cast<Instruction>(I->getNew())->getNextNode(), DT);
    PT.genReductionFini(I, I->getOrig(), getFiniBB()->getTerminator(), DT);
  }

public:
  CSAPrivatizer(VPOParoptTransform &PT, WRegionNode *W)
    : PT(PT), W(W), DT(PT.DT), LI(PT.LI), SE(PT.SE) {}

  virtual bool run() {
    bool Changed = false;

    assert(W->isBBSetEmpty() &&
           "CSAPrivatizer: BBSET should start empty");

    W->populateBBSet();

    // Generate privatization code for all flavors of private variables.
    if (W->canHavePrivate() && !W->getPriv().empty()) {
      for (auto *I : W->getPriv().items())
        genPrivVar(I, W);
      Changed = true;
    }
    if (W->canHaveFirstprivate() && !W->getFpriv().empty()) {
      for (auto *I : W->getFpriv().items())
        genFPrivVar(I, W);
      Changed = true;
    }
    if (W->canHaveLastprivate() && !W->getLpriv().empty()) {
      for (auto *I : W->getLpriv().items()) {
        genLPrivVar(I, W);
        genLPrivVarCopyout(I);
      }
      Changed = true;
    }
    if (W->canHaveReduction() && !W->getRed().empty()) {
      for (auto *I : W->getRed().items())
        genRedVar(I, W);
      Changed = true;
    }

    // Invalidate BBSet after transformation
    W->resetBBSet();

    // SCEV should be regenerated after privatization.
    if (Changed && W->getIsOmpLoop() && SE)
      SE->forgetLoop(W->getWRNLoopInfo().getLoop());

    return Changed;
  }
};

// CSA privatizer customization for the loop construct.
// TODO: Need to be updated once (and if) we start creating workers in paropt.
class VPOParoptTransform::CSALoopPrivatizer :
    public VPOParoptTransform::CSAPrivatizer {
protected:
  // Work region's loop.
  Loop *L = nullptr;

protected:
  // Last iteration block is inserted into the loop body under if which
  // compares induction variable with the loop's upper bound.
  BasicBlock* getLastIterBB() override {
    if (LastIterBB)
      return LastIterBB;

    // Get loop's induction variable and upper bound.
    auto *IV = WRegionUtils::getOmpCanonicalInductionVariable(L);
    auto *UB = WRegionUtils::getOmpLoopUpperBound(L);

    // Induction variable value will match upper bound on the last
    // iteration of the loop.
    auto *IsLast = new ICmpInst(L->getHeader()->getFirstNonPHI(),
        ICmpInst::ICMP_EQ, IV, UB, "is.last");

    // Insert if-then blocks at the end of the latch block. Copyout code
    // for last private variables will be emitted to this block.
    auto *Term = SplitBlockAndInsertIfThen(IsLast,
        L->getLoopLatch()->getTerminator(), false, nullptr, DT, LI);
    LastIterBB = Term->getParent();
    LastIterBB->getSingleSuccessor()->setName("omp.lpriv.end");
    LastIterBB->setName("omp.lpriv.copyout");
    return LastIterBB;
  }

public:
  CSALoopPrivatizer(VPOParoptTransform &PT, WRegionNode *W)
    : CSAPrivatizer(PT, W), L(W->getWRNLoopInfo().getLoop()) {
    assert(W->getIsOmpLoop() && "loop construct is expected");
    assert(L->isLoopSimplifyForm());
  }
};

// Recursively visit users of value V and check if any def-use chain reaches any
// block in the given set of Blocks. If yes then the def-use chain is added to
// the Defs list and true is returned.
static bool findDefs(const WRegionNode *W, Value *V,
                     const SmallVectorImpl<BasicBlock*> &Blocks,
                     SmallSetVector<Value*, 8u> &Defs) {
  bool IsDef = false;
  for (auto *U : V->users()) {
    if (auto *I = dyn_cast<Instruction>(U)) {
      auto *BB = I->getParent();
      if (!W->contains(BB))
        continue;
      if (find(Blocks, BB) != Blocks.end() || findDefs(W, I, Blocks, Defs))
        IsDef = true;
    }
  }
  if (IsDef)
    Defs.insert(V);
  return IsDef;
}

// Experimental variant of CSA loop privatizer which uses loop preheader as
// initialization block (where alloca instructions for private instances are
// inserted).
class VPOParoptTransform::CSAExpLoopPrivatizer final :
    public VPOParoptTransform::CSALoopPrivatizer {
private:
  // List of blocks that require replacing of original variable references
  // with private instances.
  SmallVector<BasicBlock*, 8u> Blocks;

private:
  BasicBlock* getInitBB() override {
    if (InitBB)
      return InitBB;
    InitBB = L->getLoopPreheader();
    return InitBB;
  }

  BasicBlock* getFiniBB() override {
    if (FiniBB)
      return FiniBB;
    FiniBB = WRegionUtils::getOmpExitBlock(L);
    SplitBlock(FiniBB, FiniBB->getFirstNonPHI(), DT, LI);
    FiniBB->setName(".omp.priv.fini");
    return FiniBB;
  }

  void genPrivItem(Item *I, WRegionNode *W, StringRef Suffix) override {
    auto *Old = I->getOrig();

    // Find all instructions depending on the private variable which need to be
    // replicated in the loop preheader.
    SmallSetVector<Value*, 8u> Defs;
    findDefs(W, Old, Blocks, Defs);

    // Insertion point for alloca.
    auto *InsPt = getInitBB()->getFirstNonPHI();

    // Create alloca for the private variable.
    auto *New = PT.genPrivatizationAlloca(W, Old, InsPt, Suffix);
    I->setNew(New);

    // Create clones of all collected defs at the beginning of the loop
    // preheader.
    ValueToValueMapTy VMap;
    VMap[Old] = New;
    for (const auto *I : reverse(Defs)) {
      if (VMap[I])
        continue;
      auto *C = cast<Instruction>(I)->clone();
      C->insertBefore(InsPt);
      VMap[I] = C;
    }

    // Replace uses of defs within the loop with clones.
    for (auto *V : Defs) {
      SmallVector<Value*, 8u> Uses;
      copy_if(V->users(), std::back_inserter(Uses), [&](const Value *V) {
        if (auto *I = dyn_cast<Instruction>(V))
          return find(Blocks, I->getParent()) != Blocks.end();
        return false;
      });
      for (auto *U : Uses)
        cast<Instruction>(U)->replaceUsesOfWith(V, VMap[V]);

      // Defs with no uses can be safely deleted.
      if (auto *I = dyn_cast<Instruction>(V))
        if (I->user_empty())
          I->eraseFromParent();
    }
  }

public:
  CSAExpLoopPrivatizer(VPOParoptTransform &PT, WRegionNode *W)
    : CSALoopPrivatizer(PT, W) {
    // Initialize blocks list where private variable references will be replaced
    // with private instances. It includes loop preheader and all loop blocks.
    Blocks.push_back(L->getLoopPreheader());
    copy(L->blocks(), std::back_inserter(Blocks));
  }
};

// CSA privatizer customization for the sections construct. Each section
// is treated as an idependent worker, so privatization is done for each
// section.
class VPOParoptTransform::CSASectionsPrivatizer final :
    public VPOParoptTransform::CSAPrivatizer {
private:
  // Last iteration block for sections goes to the end of the last section.
  BasicBlock* getLastIterBB() override {
    if (LastIterBB)
      return LastIterBB;

    auto *LastSecW = W->getChildren().front();
    LastIterBB = LastSecW->getExitBBlock();
    auto *New = SplitBlock(LastIterBB, LastIterBB->getFirstNonPHI(), DT, LI);
    LastSecW->setExitBBlock(New);
    LastIterBB->setName("omp.lpriv.copyout");
    return LastIterBB;
  }

public:
  CSASectionsPrivatizer(VPOParoptTransform &PT, WRegionNode *W)
    : CSAPrivatizer(PT, W) {
    assert(W->getIsSections() && "sections construct is expected");
  }

  bool run() override {
    bool Changed = false;

    assert(W->isBBSetEmpty() &&
           "CSASectionsPrivatizer: BBSET should start empty");

    W->populateBBSet();

    // For sections construct we need to privatizations for each section.
    for (auto SecW : reverse(W->getChildren())) {
      assert(SecW->isBBSetEmpty() &&
             "CSASectionsPrivatizer: BBSET should start empty");
      SecW->populateBBSet();
      if (!W->getPriv().empty()) {
        for (auto *I : W->getPriv().items())
          genPrivVar(I, SecW);
        Changed = true;
      }
      if (!W->getFpriv().empty()) {
        for (auto *I : W->getFpriv().items())
          genFPrivVar(I, SecW);
        Changed = true;
      }
      if (!W->getLpriv().empty()) {
        for (auto *I : W->getLpriv().items())
          genLPrivVar(I, SecW);
        Changed = true;
      }
      if (!W->getRed().empty()) {
        for (auto *I : W->getRed().items())
          genRedVar(I, SecW);
        Changed = true;
      }
      SecW->resetBBSet();
    }

    // Gen copyout code for lastprivate variables.
    if (!W->getLpriv().empty())
      for (auto *I : W->getLpriv().items())
        genLPrivVarCopyout(I);

    // Invalidate BBSet after transformations
    W->resetBBSet();

    return Changed;
  }
};

static WRegionNode* findEnclosingParRegion(WRegionNode *W) {
  while (W && !W->getIsPar())
    W = W->getParent();
  return W;
}

bool VPOParoptTransform::isSupportedOnCSA(WRegionNode *W) {
  switch (W->getWRegionKindID()) {
    case WRegionNode::WRNParallel:
    case WRegionNode::WRNParallelSections:
    case WRegionNode::WRNParallelLoop:
    case WRegionNode::WRNWksLoop:
    case WRegionNode::WRNSections:
    case WRegionNode::WRNAtomic:	// not fully supported, but may work
    case WRegionNode::WRNSection:
    case WRegionNode::WRNMaster:
    case WRegionNode::WRNSingle:
    case WRegionNode::WRNBarrier:
      break;
    default:
      reportCSAWarning(W, "ignoring unsupported \"omp " + W->getName() + "\"");
      return false;
  }

  // num_threads
  if (W->getIsParLoop())
    if (auto *NumThreads = W->getNumThreads())
      if (!dyn_cast<ConstantInt>(NumThreads)) {
        reportCSAWarning(W, "num_threads must be a compile time constant");
        W->setNumThreads(nullptr);
      }

  // proc_bind
  if (W->getIsPar() && W->getProcBind() != WRNProcBindAbsent)
    reportCSAWarning(W, "ignoring unsupported proc_bind clause");

  // schedule
  if (W->canHaveSchedule()) {
    // Check schedule clause of there is one.
    auto &Sched = W->getSchedule();
    if (Sched.getChunkExpr()) {
      // So far we support only static and auto schedule types.
      if (Sched.getKind() != WRNScheduleStatic &&
          Sched.getKind() != WRNScheduleAuto) {
        reportCSAWarning(W, "ignoring unsupported schedule type");
        Sched.setChunkExpr(nullptr);
      }
      // Three options for the chunk size
      //   Sched->getChunk() == 0 => chunk was not specified
      //   Sched->getChunk() > 0  => chunk is a compile time constant
      //   Sched->getChunk() < 0  => chunk is an expression (unsupported)
      else if (Sched.getChunk() < 0) {
        reportCSAWarning(W, "schedule chunk must be a compile time constant");
        Sched.setChunkExpr(nullptr);
      }
    }
  }
  return true;
}

void VPOParoptTransform::reportCSAWarning(WRegionNode *W, const Twine &Msg) {
  DebugLoc Loc;
  if (auto *I = W->getEntryBBlock()->getFirstNonPHI())
    Loc = I->getDebugLoc();
  F->getContext().diagnose(CSADiagInfo(*F, Loc,  Msg));
}

// Insert a pair of CSA parallel region enter and exit intrinsic calls around
// the parallel work region body as follows
//
//   %region = call i32 @llvm.csa.parallel.region.entry(i32 2002);
//   <parallel construct body>
//   call void @llvm.csa.parallel.region.exit(i32 %region);
//
// These calls mark the beginning and end of the parallel region for back-end.
//
Value* VPOParoptTransform::genCSAParallelRegion(WRegionNode *W) {
  assert(isTargetCSA() && "unexpected target");

  // CSA parallel region entry/exit calls will be inserted into the closest
  // enclosing omp parallel region if such region exists.
  if (auto *ParW = findEnclosingParRegion(W))
    W = ParW;

  auto &Region = CSAParallelRegions[W];
  if (Region)
    return Region;

  auto *M = F->getParent();

  // CSA parallel region entry/exit intrinsics
  auto *RegionEntry = Intrinsic::getDeclaration(M,
      Intrinsic::csa_parallel_region_entry);

  auto *RegionExit = Intrinsic::getDeclaration(M,
      Intrinsic::csa_parallel_region_exit);

  // Insert a "entry" call into the work region's entry bblock. The argument
  // is a unique ID which is a work region's unique number plus 2000 (using
  // 2000 as base to avoid conflicts with IDs inserted by CSA builtin parallel
  // loop intrinsics lowering pass which uses IDs starting from 1000).
  IRBuilder<> Builder(W->getEntryBBlock()->getTerminator());
  auto *UniqueID = Builder.getInt32(2000u + W->getNumber());
  Region = Builder.CreateCall(RegionEntry, { UniqueID }, "region");

  // And an "exit" call into the work region's exit bblock.
  Builder.SetInsertPoint(&*W->getExitBBlock()->getFirstInsertionPt());
  Builder.CreateCall(RegionExit, { Region }, {});

  return Region;
}

// Insert a pair of CSA parallel section entry/exit calls before given
// instructions.
void VPOParoptTransform::genCSAParallelSection(Value *Region,
                                               Instruction *EntryPt,
                                               Instruction *ExitPt) {
  assert(isTargetCSA() && "unexpected target");

  auto *M = F->getParent();

  // CSA parallel section entry/exit intrinsics
  auto *SectionEntry = Intrinsic::getDeclaration(M,
      Intrinsic::csa_parallel_section_entry);

  auto *SectionExit = Intrinsic::getDeclaration(M,
      Intrinsic::csa_parallel_section_exit);

  // Insert section entry call into the beginning of the loop header.
  IRBuilder<> Builder(EntryPt);
  auto *Section = Builder.CreateCall(SectionEntry, { Region }, "section");

  // And section exit call before the the loop latch terminator.
  Builder.SetInsertPoint(ExitPt);
  Builder.CreateCall(SectionExit, { Section }, {});
}

// Transform "omp parallel for" construct for CSA by inserting CSA builtins
// which annotate loop as parallel for the back-end passes.
// After transformation the annotated loop looks as follows
//
//   [%spmd = call i32 @llvm.csa.spmdization.entry(<num_threads>, <mode>);]*
//   %region = call i32 @llvm.csa.parallel.region.entry(i32 2002);
//   for (...) {
//     %section = call i32 @llvm.csa.parallel.section.entry(i32 %region);
//     <loop body>
//     call void @llvm.csa.parallel.section.exit(i32 %section);
//   }
//   call void @llvm.csa.parallel.region.exit(i32 %region);
//   [call void @llvm.csa.spmdization.exit(i32 %spmd);]*
//
// *Optional spmdization entry/exit calls are inserted only if parallel for
// has num_threads clause.
//
bool VPOParoptTransform::genCSALoop(WRegionNode *W) {
  assert(isTargetCSA() && "unexpected target");

  auto *Loop = W->getWRNLoopInfo().getLoop();
  assert(Loop->isLoopSimplifyForm());

  // Generate privatization code.
  if (UseExpLoopPrivatizer)
    CSAExpLoopPrivatizer(*this, W).run();
  else
    CSALoopPrivatizer(*this, W).run();

  // Determine the number of workers to use for this loop.
  unsigned NumWorkers = 0;
  if (W->getIsPar())
    if (auto *NumThreads = W->getNumThreads())
      NumWorkers = cast<ConstantInt>(NumThreads)->getZExtValue();
  if (!NumWorkers)
    NumWorkers = LoopWorkersDefault;

  // Annotating loop with spmdization entry/exit intrinsic calls if parallel
  // region has num_threads clause and the number of threads > 1.
  if (NumWorkers > 1u) {
    auto *M = F->getParent();

    auto *Entry = Intrinsic::getDeclaration(M,
        Intrinsic::csa_spmdization_entry);

    auto *Exit = Intrinsic::getDeclaration(M,
        Intrinsic::csa_spmdization_exit);

    // Determine SPMDization mode, it depends on a schedule clause.
    //   No schedule, schedule(auto) or
    //   schedule(static)                 => blocked SPMD (0)
    //   schedule(static, chunksize)
    //     (chunksize == 1)               => cyclic SPMD  (1)
    //     (chunksize > 1)                => hybrid SPMD  (chunksize)
    const auto &Sched = W->getSchedule();
    Value *Mode = ConstantInt::get(Type::getInt32Ty(F->getContext()),
        !Sched.getChunkExpr() || !Sched.getChunk() ? 0 : Sched.getChunk());

    IRBuilder<> Builder(W->getEntryBBlock()->getTerminator());
    auto *SpmdID = Builder.CreateCall(Entry,
                                      { Builder.getInt32(NumWorkers), Mode },
                                      "spmd");

    Builder.SetInsertPoint(&*W->getExitBBlock()->getFirstInsertionPt());
    Builder.CreateCall(Exit, { SpmdID }, {});
  }

  // Insert parallel region entry/exit calls
  auto *Region = genCSAParallelRegion(W);

  // and CSA parallel section entry/exit intrinsics
  genCSAParallelSection(Region,
                        Loop->getHeader()->getFirstNonPHI(),
                        Loop->getLoopLatch()->getTerminator());
  return true;
}

bool VPOParoptTransform::genCSASections(WRegionNode *W) {
  assert(isTargetCSA() && "unexpected target");

  // Generate privatization code for the sections construct.
  CSASectionsPrivatizer(*this, W).run();

  // Insert parallel region entry/exit calls
  auto *Region = genCSAParallelRegion(W);

  // Insert section entry/exit calls in child work regions which all are
  // supposed to be sections.
  for (auto *WSec : W->getChildren()) {
    assert(WSec->getWRegionKindID() == WRegionNode::WRNSection &&
           "Unexpected work region kind");

    genCSAParallelSection(Region,
                          WSec->getEntryBBlock()->getTerminator(),
                          WSec->getExitBBlock()->getFirstNonPHI());

    // Remove directives from section
    VPOUtils::stripDirectives(WSec);
  }
  return true;
}

bool VPOParoptTransform::genCSAParallel(WRegionNode *W) {
  assert(isTargetCSA() && "unexpected target");
  CSAPrivatizer(*this, W).run();
  genCSAParallelRegion(W);
  return true;
}

bool VPOParoptTransform::genCSASingle(WRegionNode *W) {
  assert(isTargetCSA() && "unexpected target");
  return CSAPrivatizer(*this, W).run();
}

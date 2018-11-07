//=- VPOParoptTransformCSA.cpp - W-Region transformations for CSA -*- C++ -*-=//
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
#include "llvm/IR/InstIterator.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-transform-csa"

static cl::opt<unsigned> LoopWorkersDefault(
  "csa-omp-loop-workers-default", cl::init(1), cl::ReallyHidden,
  cl::desc("Defines default number of workers for OpenMP loops with no "
           "num_threads clause."));

static cl::opt<unsigned> LoopSpmdModeDefault(
  "csa-omp-loop-spmd-mode-default", cl::init(0), cl::ReallyHidden,
  cl::desc("Defines default SPMDization mode for OpenMP loops with no "
           "schedule clause"));

static cl::opt<bool> UseExpLoopPrivatizer(
  "csa-omp-exp-loop-privatizer", cl::init(false), cl::ReallyHidden,
  cl::desc("Use experimental privatizer for OpenMP loops."));

static cl::opt<bool> DoLoopSplitting(
  "csa-omp-paropt-loop-splitting", cl::init(false), cl::ReallyHidden,
  cl::desc("Do OpenMP loop splitting in VPO Paropt."));

namespace {

class CSADiagInfo : public DiagnosticInfoWithLocationBase {
  const Twine &Msg;

public:
  CSADiagInfo(const Function &F, const DiagnosticLocation &Loc,
              const Twine &Msg, DiagnosticSeverity DS = DS_Warning)
    : DiagnosticInfoWithLocationBase(
          // TODO (vzakhari 10/8/2018): we may want to introduce a real
          //       DiagnosticKind.
          static_cast<DiagnosticKind>(getNextAvailablePluginDiagnosticKind()),
          DS, F, Loc),
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

  // Lower reduction clauses if set to true.
  bool DoReds = true;

private:
  // Creates alloca instruction for the given item, inserts it to the InitBB and
  // replaces all uses of the original variable within work region with a
  // private instance.
  virtual void genPrivItem(Item *I, WRegionNode *W, StringRef Suffix) {
    auto *Old = I->getOrig();
    auto *New = VPOParoptTransform::genPrivatizationAlloca(Old,
        getInitBB()->getTerminator(), Suffix);
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
  CSAPrivatizer(VPOParoptTransform &PT, WRegionNode *W, bool DoReds = true)
    : PT(PT), W(W), DT(PT.DT), LI(PT.LI), SE(PT.SE), DoReds(DoReds) {}

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
    if (DoReds && W->canHaveReduction() && !W->getRed().empty()) {
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
  CSALoopPrivatizer(VPOParoptTransform &PT, WRegionNode *W, bool DoReds,
                    Loop *LL = nullptr)
    : CSAPrivatizer(PT, W, DoReds), L(LL ? LL : W->getWRNLoopInfo().getLoop()) {
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
      if (is_contained(Blocks, BB) || findDefs(W, I, Blocks, Defs))
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
    auto *New = VPOParoptTransform::genPrivatizationAlloca(Old, InsPt, Suffix);
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
          return is_contained(Blocks, I->getParent());
        return false;
      });
      for (auto *U : Uses)
        cast<Instruction>(U)->replaceUsesOfWith(V, VMap[V]);

      // Defs with no uses can be safely deleted.
      if (auto *I = dyn_cast<Instruction>(V))
        if (I->use_empty())
          I->eraseFromParent();
    }
  }

public:
  CSAExpLoopPrivatizer(VPOParoptTransform &PT, WRegionNode *W, bool DoReds)
    : CSALoopPrivatizer(PT, W, DoReds) {
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

// Insert a pair of CSA parallel region entry/exit calls at specified insertion
// points.
static Value* genParRegionCalls(unsigned ID,
                                Instruction *EntryPt,
                                Instruction *ExitPt,
                                const Twine &Name = "region") {
  auto *M = EntryPt->getModule();
  assert(M == ExitPt->getModule());

  // CSA parallel region entry/exit intrinsics
  auto *RegionEntry = Intrinsic::getDeclaration(M,
      Intrinsic::csa_parallel_region_entry);

  auto *RegionExit = Intrinsic::getDeclaration(M,
      Intrinsic::csa_parallel_region_exit);

  IRBuilder<> Builder(EntryPt);
  auto *UniqueID = Builder.getInt32(ID);
  auto *Region = Builder.CreateCall(RegionEntry, { UniqueID }, Name);

  Builder.SetInsertPoint(ExitPt);
  Builder.CreateCall(RegionExit, { Region }, {});

  return Region;
}

// Insert a pair of CSA parallel section entry/exit calls before given
// instructions.
static void genParSectionCalls(Value *Region,
                               Instruction *EntryPt,
                               Instruction *ExitPt,
                               const Twine &Name = "section") {
  auto *M = EntryPt->getModule();
  assert(M == ExitPt->getModule());

  // CSA parallel section entry/exit intrinsics
  auto *SectionEntry = Intrinsic::getDeclaration(M,
      Intrinsic::csa_parallel_section_entry);

  auto *SectionExit = Intrinsic::getDeclaration(M,
      Intrinsic::csa_parallel_section_exit);

  IRBuilder<> Builder(EntryPt);
  auto *Section = Builder.CreateCall(SectionEntry, { Region }, Name);

  Builder.SetInsertPoint(ExitPt);
  Builder.CreateCall(SectionExit, { Section }, {});
}

// Returns the number of workers to be created for the loop WRN.
static unsigned getNumWorkers(const WRegionNode *W) {
  assert(W->getIsOmpLoop() && "expecting a loop construct");
  unsigned NumWorkers = 0;
  if (W->getIsPar())
    if (auto *NumThreads = W->getNumThreads())
      NumWorkers = cast<ConstantInt>(NumThreads)->getZExtValue();
  if (!NumWorkers)
    NumWorkers = LoopWorkersDefault;
  return NumWorkers;
}

// Returns SPMDization mode for the loop WRN. It depends on a schedule clause.
//   No schedule, schedule(auto) or
//   schedule(static)                 => blocked SPMD (0)
//   schedule(static, chunksize)
//     (chunksize == 1)               => cyclic SPMD  (1)
//     (chunksize > 1)                => hybrid SPMD  (chunksize)
static int getSPMDMode(const WRegionNode *W) {
  assert(W->getIsOmpLoop() && "expecting a loop construct");
  const auto &Sched = W->getSchedule();
  return Sched.getChunkExpr() ? Sched.getChunk() : LoopSpmdModeDefault;
}

template <typename U, typename B>
static void findUsers(Value *V, U &Users, const B &Blocks) {
  copy_if(V->users(), std::back_inserter(Users), [&](Value *V) {
    if (auto *I = dyn_cast<Instruction>(V))
      if (is_contained(Blocks, I->getParent()))
        return true;
    return false;
  });
}

template <typename T>
static void replaceUses(Value *Old, Value *New, const T &Users) {
  for (auto *User : Users)
    cast<Instruction>(User)->replaceUsesOfWith(Old, New);
}

// Unified way of calculating parallel region ID for CSA parallel region entry
// intrinsic call.
static unsigned getParRegionID(WRegionNode *W,
                               unsigned Worker = 0,
                               bool IsHybrid = false) {
  return 2000u + W->getNumber() + (Worker << 16u) + (IsHybrid << 28u);
}

class VPOParoptTransform::CSALoopSplitter {
  VPOParoptTransform &PT;

  // A structure that represents a single worker in a split loop.
  class Worker {
    // Reference to the parent.
    CSALoopSplitter &LS;

    // Worker ID. Goes from zero to LS.getNumWorkers() - 1u.
    unsigned ID = 0;

  public:
    // Work region that is being split.
    WRegionNode *W = nullptr;

    // Worker's loop. This is a clone of the work region's loops.
    Loop *WL = nullptr;

    // Loop around the work region's loop that is created in hybrid mode.
    Loop *HL = nullptr;

    // String that is appended to the worker clones.
    SmallString<8u> Suffix;

    // Worker's basic blocks.
    SmallVector<BasicBlock*, 16u> Blocks;

    BasicBlock *Head = nullptr;
    BasicBlock *Tail = nullptr;

  private:
    void fixupLoopBounds(Value *NewLB, Value *NewUB, BasicBlock *Entry) {
      // Need to update ZTT, induction variable value comming
      // from the preheader block and loop bottom test.
      auto *IV = WRegionUtils::getOmpCanonicalInductionVariable(WL);
      auto *LB = cast<Instruction>(WRegionUtils::getOmpLoopLowerBound(WL));
      assert(LB && is_contained(IV->operands(), LB));
      if (NewLB)
        IV->replaceUsesOfWith(LB, NewLB);
      if (auto *Ztt = WRegionUtils::getOmpLoopZeroTripTest(WL, Entry)) {
        assert(is_contained(Ztt->operands(), LB));
        if (NewUB)
          Ztt->setOperand(Ztt->getOperand(0) == LB, NewUB);
        if (NewLB)
          Ztt->setOperand(Ztt->getOperand(0) != LB, NewLB);
      }
      if (NewUB) {
        auto *Inc = IV->getIncomingValueForBlock(WL->getLoopLatch());
        auto *Cmp = WRegionUtils::getOmpLoopBottomTest(WL);
        assert(Cmp && is_contained(Cmp->operands(), Inc));
        Cmp->setOperand(Cmp->getOperand(0) == Inc, NewUB);
      }
    }

  public:
    Worker(CSALoopSplitter &LS, WRegionNode *W, unsigned ID)
      : LS(LS), ID(ID), W(W) {
      if (ID)
        raw_svector_ostream(Suffix) << ".W" << ID;
    }

    void makeCyclic() {
      // Update loop LB.
      auto *LB = cast<Instruction>(WRegionUtils::getOmpLoopLowerBound(WL));
      auto *NewLB = ConstantInt::get(LB->getType(), ID);
      fixupLoopBounds(NewLB, nullptr, Head);

      // Update stride. For that we need to fixup loop increment instruction
      // which is the IV's value comming from the latch block.
      auto *IV = WRegionUtils::getOmpCanonicalInductionVariable(WL);

      auto *Inc =
          cast<Instruction>(IV->getIncomingValueForBlock(WL->getLoopLatch()));
      assert(Inc && Inc->getOpcode() == Instruction::Add &&
            is_contained(Inc->operands(), IV));

      auto *Stride = Inc->getOperand(Inc->getOperand(0) == IV);
      assert(cast<ConstantInt>(Stride)->isOne() && "unexpected loop stride");

      auto *NewStride = ConstantInt::get(Stride->getType(), LS.Workers.size());
      Inc->replaceUsesOfWith(Stride, NewStride);

      // Remove dead instructions.
      if (LB->use_empty())
        LB->eraseFromParent();
    }

    void makeBlocked() {
      auto *LB = cast<Instruction>(WRegionUtils::getOmpLoopLowerBound(WL));
      auto *UB = WRegionUtils::getOmpLoopUpperBound(WL);

      bool IsSigned = WRegionUtils::getOmpLoopBottomTest(WL)->isSigned();

      auto *Ty = cast<IntegerType>(LB->getType());
      assert(UB->getType() == Ty && "ub/lb type mismatch");

      auto *NWC = ConstantInt::get(Ty, LS.Workers.size(), IsSigned);
      auto *One = ConstantInt::get(Ty, 1u, IsSigned);

      IRBuilder<> Builder(LB);
      auto *Chunk = IsSigned ?
          Builder.CreateSDiv(Builder.CreateAdd(UB, NWC), NWC) :
          Builder.CreateUDiv(Builder.CreateAdd(UB, NWC), NWC);

      Value *NewLB = nullptr;
      if (ID == 0)
        NewLB = ConstantInt::get(Ty, 0, IsSigned);
      else if (ID == 1)
        NewLB = Chunk;
      else
        NewLB = Builder.CreateMul(Chunk, ConstantInt::get(Ty, ID, IsSigned),
                                  "lb" + Suffix);

      auto *Next = (ID == 0) ? Chunk : Builder.CreateAdd(NewLB, Chunk);

      auto *TC = Builder.CreateAdd(UB, One);

      auto *Cmp = IsSigned ? Builder.CreateICmpSLE(Next, TC) :
                             Builder.CreateICmpULE(Next, TC);

      auto *Min = Builder.CreateSelect(Cmp, Next, TC);

      auto *NewUB = Builder.CreateSub(Min, One, "ub" + Suffix);

      fixupLoopBounds(NewLB, NewUB, Head);

      // Remove dead instructions.
      if (LB->use_empty())
        LB->eraseFromParent();
    }

    void makeHybrid(unsigned Chunk) {
      // head:
      //  br label %cyclic.ztt
      //
      // cyclic.ztt:
      //   %clb = (Worker * Chunk)
      //   %ztt = icmp sle %clb, %UB
      //   br %ztt, label %cyclic.ph, label %tail
      //
      // cyclic.ph:
      //  br label %cyclic.loop
      //
      // cyclic.loop:
      //   %blocked.lb = phi [ %clb, %cyclic.ph ], [%cnext, %cyclic.latch ]
      //   %blocked.ub = min(%blocked.lb + Chunk - 1, %UB)
      //   ...
      //
      //  ...
      //
      // cyclic.latch:
      //   %cnext = add nsw i32 %blocked.lb, (NWorker * Chunk)
      //   %cmp = icmp sle %cnext, %UB
      //   br %cmp, label %cyclic.loop, %cyclic.exit
      //
      // cyclic.exit:
      //  br label %tail
      //
      // tail:
      //   ...
      //
      auto *LB = cast<Instruction>(WRegionUtils::getOmpLoopLowerBound(WL));
      auto *UB = WRegionUtils::getOmpLoopUpperBound(WL);

      bool IsSigned = WRegionUtils::getOmpLoopBottomTest(WL)->isSigned();

      auto *Ty = cast<IntegerType>(LB->getType());
      assert(UB->getType() == Ty && "ub/lb type mismatch");

      // Create blocks for the cyclic loop.
      auto *CZtt = SplitBlock(Head, Head->getTerminator(), LS.PT.DT, LS.PT.LI);
      CZtt->setName("cyclic.ztt" + Suffix);
      Blocks.push_back(CZtt);

      auto *CLPH = SplitBlock(CZtt, CZtt->getTerminator(), LS.PT.DT, LS.PT.LI);
      CLPH->setName("cyclic.ph" + Suffix);
      Blocks.push_back(CLPH);

      auto *CLoop = SplitBlock(CLPH, CLPH->getTerminator(), LS.PT.DT, LS.PT.LI);
      CLoop->setName("cyclic.loop" + Suffix);
      Blocks.push_back(CLoop);

      auto *CLatch = Tail;
      Tail = SplitBlock(Tail, &Tail->front(), LS.PT.DT, LS.PT.LI);
      CLatch->setName("cyclic.latch" + Suffix);
      Blocks.push_back(Tail);

      auto *CExit = Tail;
      Tail = SplitBlock(Tail, &Tail->front(), LS.PT.DT, LS.PT.LI);
      CExit->setName("cyclic.exit" + Suffix);
      Blocks.push_back(Tail);

      // Create ZTT code.
      IRBuilder<> Builder(CZtt->getTerminator());
      auto *CLB = ConstantInt::get(Ty, ID * Chunk, IsSigned);
      auto *ZttCond = IsSigned ? Builder.CreateICmpSLE(CLB, UB) :
                                 Builder.CreateICmpULE(CLB, UB);
      Builder.CreateCondBr(ZttCond, CLPH, Tail);

      // Create blocked LB.
      Builder.SetInsertPoint(&CLoop->front());
      auto *BLB = Builder.CreatePHI(Ty, 2, "blocked.lb" + Suffix);
      BLB->addIncoming(CLB, CLPH);

      // And blocked UB.
      auto *BNext = Builder.CreateAdd(BLB,
          ConstantInt::get(Ty, Chunk - 1u, IsSigned));
      auto *BCmp = IsSigned ? Builder.CreateICmpSLE(BNext, UB) :
                              Builder.CreateICmpULE(BNext, UB);
      auto *BUB = Builder.CreateSelect(BCmp, BNext, UB, "blocked.ub" + Suffix);

      // Create latch code.
      Builder.SetInsertPoint(CLatch->getTerminator());
      auto *CStride = ConstantInt::get(Ty, LS.Workers.size() * Chunk, IsSigned);
      auto *CNext = Builder.CreateAdd(BLB, CStride);
      BLB->addIncoming(CNext, CLatch);

      auto *LatchCond = IsSigned ? Builder.CreateICmpSLE(CNext, UB) :
                                   Builder.CreateICmpULE(CNext, UB);
      Builder.CreateCondBr(LatchCond, CLoop, CExit);

      // Fixup inner loop to run from blocked LB to blocked UB.
      fixupLoopBounds(BLB, BUB, CLoop);

      // Mark inner loop as parallel.
      auto *Region = genParRegionCalls(getParRegionID(W, ID + 1u, true),
                                       CLoop->getTerminator(),
                                       CLatch->getFirstNonPHI(),
                                       "inner.region" + Suffix);
      genParSectionCalls(Region,
                         WL->getHeader()->getFirstNonPHI(),
                         WL->getLoopLatch()->getTerminator(),
                         "inner.section" + Suffix);

      // Update loop structure, create outer hubrid loop.
      HL = WRegionUtils::createLoop(WL, WL->getParentLoop(), LS.PT.LI);
      WRegionUtils::updateBBForLoop(CLoop, HL, WL->getParentLoop(), LS.PT.LI);
      WRegionUtils::updateBBForLoop(CLatch, HL, WL->getParentLoop(), LS.PT.LI);
      HL->moveToHeader(CLoop);

      // Remove dead instructions.
      CZtt->getTerminator()->eraseFromParent();
      CLatch->getTerminator()->eraseFromParent();
      if (LB->use_empty())
        cast<Instruction>(LB)->eraseFromParent();
    }

    void addParallelIntrinsicCalls() {
      // Add region entry/exit calls.
      auto *Region = genParRegionCalls(getParRegionID(W, ID + 1u),
                                       Head->getTerminator(),
                                       Tail->getFirstNonPHI(),
                                       "region" + Suffix);

      // And section entry/exit calls.
      auto *L = HL ? HL : WL;
      genParSectionCalls(Region,
                         L->getHeader()->getFirstNonPHI(),
                         L->getLoopLatch()->getTerminator(),
                         "section" + Suffix);
    }
  };
  friend Worker;

  // Privatizer for the worker.
  class WorkerPrivatizer final : public CSALoopPrivatizer {
    const Worker &WI;
    function_ref<BasicBlock*()> InitBBGetter;
    function_ref<BasicBlock*()> FiniBBGetter;

  private:
    BasicBlock* getInitBB() override {
      if (InitBB)
        return InitBB;
      InitBB = InitBBGetter();
      return InitBB;
    }

    BasicBlock* getFiniBB() override {
      if (FiniBB)
        return FiniBB;
      FiniBB = FiniBBGetter();
      return FiniBB;
    }

    void genPrivItem(Item *I, WRegionNode *W, StringRef Suffix) override {
      auto *Old = I->getOrig();
      assert(!isa<Constant>(Old) && "unexpected private item");
      SmallVector<Value*, 8u> Users;
      findUsers(Old, Users, WI.Blocks);

      auto *New = VPOParoptTransform::genPrivatizationAlloca(Old,
          getInitBB()->getTerminator(), Suffix);
      I->setNew(New);
      replaceUses(Old, New, Users);
    }

  public:
    WorkerPrivatizer(VPOParoptTransform &PT, const Worker &WI,
                     function_ref<BasicBlock*()> InitBBGetter,
                     function_ref<BasicBlock*()> FiniBBGetter)
      : CSALoopPrivatizer(PT, WI.W, true, WI.WL), WI(WI),
        InitBBGetter(InitBBGetter), FiniBBGetter(FiniBBGetter)
    {}

    bool run() override {
      bool Changed = CSALoopPrivatizer::run();
      for (auto *I : W->getPriv().items())
        I->setNew(nullptr);
      for (auto *I : W->getFpriv().items())
        I->setNew(nullptr);
      for (auto *I : W->getLpriv().items())
        I->setNew(nullptr);
      for (auto *I : W->getRed().items())
        I->setNew(nullptr);
      return Changed;
    }
  };

private:
  SmallVector<Worker, 8u> Workers;

private:
  // Clones given loop with subloops.
  Loop* cloneLoops(const Loop *L, DenseMap<const Loop*, Loop*> &LMap) {
    if (auto *PL = L->getParentLoop())
      LMap[PL] = PL;
    std::list<const Loop*> Loops;
    Loops.push_back(L);
    do {
      auto *OldL = Loops.front();
      Loops.pop_front();
      auto *NewL = PT.LI->AllocateLoop();
      if (auto *PL = OldL->getParentLoop())
        LMap[PL]->addChildLoop(NewL);
      else
        PT.LI->addTopLevelLoop(NewL);
      LMap[OldL] = NewL;
      copy(OldL->getSubLoops(), std::back_inserter(Loops));
    } while (!Loops.empty());
    return LMap[L];
  }

  void cloneWorkers(WRegionNode *W) {
    // Get the number of workers to use for this loop.
    auto NumWorkers = getNumWorkers(W);

    auto *Entry = W->getEntryBBlock();
    auto *Exit = W->getExitBBlock();

    // Create single entry, single exit subgraph for the work region that will
    // be cloned. Head will be an entry of this subgraph and tail an exit.
    auto *Head = SplitBlock(Entry, Entry->getTerminator(), PT.DT, PT.LI);
    if (!Head->getSingleSuccessor())
      SplitBlock(Head, &Head->front(), PT.DT, PT.LI);
    Head->setName("omp.clone.head");

    auto *Tail = Exit;
    Exit = SplitBlock(Exit, &Exit->front(), PT.DT, PT.LI);
    W->setExitBBlock(Exit);
    if (!Tail->getSinglePredecessor())
      Tail = SplitBlock(Tail, Tail->getTerminator(), PT.DT, PT.LI);
    Tail->setName("omp.clone.tail");

    assert(W->isBBSetEmpty() && "CSALoopSplitter: BBSET should start empty");
    W->populateBBSet();

    // Reserve enough space to avoid any reallocation during cloning.
    Workers.reserve(NumWorkers);

    // Setup the first worker. No cloning is necessary because we will reuse
    // existing blocks for it.
    Workers.emplace_back(*this, W, 0u);
    auto &W0 = Workers.back();
    W0.WL = W->getWRNLoopInfo().getLoop();
    W0.Head = Head;
    W0.Tail = Tail;
    copy_if(W->blocks(), std::back_inserter(W0.Blocks), [=](BasicBlock *B) {
      return B != Entry && B != Exit;
    });

    // Create workers.
    auto *Last = W0.Tail;
    for (unsigned ID = 1u; ID < NumWorkers; ++ID) {
      Workers.emplace_back(*this, W, ID);
      auto &WI = Workers.back();

      // Clone loop structure.
      DenseMap<const Loop*, Loop*> LMap;
      WI.WL = cloneLoops(W0.WL, LMap);

      // Clone blocks.
      ValueToValueMapTy VMap;
      for (auto *B : W0.Blocks) {
        auto *NewB = CloneBasicBlock(B, VMap, WI.Suffix, B->getParent());
        if (auto *BL = PT.LI->getLoopFor(B))
          LMap[BL]->addBasicBlockToLoop(NewB, *PT.LI);
        WI.Blocks.push_back(NewB);
        VMap[B] = NewB;
      }
      remapInstructionsInBlocks(WI.Blocks, VMap);

      WI.Head = cast<BasicBlock>(VMap[W0.Head]);
      WI.Tail = cast<BasicBlock>(VMap[W0.Tail]);

      // Link cloned worker to cfg.
      cast<BranchInst>(Last->getTerminator())->setOperand(0, WI.Head);
      Last = WI.Tail;
    }

    // Connect the last worker to the exit block.
    cast<BranchInst>(Last->getTerminator())->setOperand(0, Exit);

    W->resetBBSet();
  }

public:
  CSALoopSplitter(VPOParoptTransform &PT) : PT(PT) {}

  bool run(WRegionNode *W) {
    assert(W->getIsOmpLoop() && "must be a loop");

    // Create workers.
    cloneWorkers(W);

    // Helper lamdas for the privatizer.
    BasicBlock *InitBB = nullptr;
    auto && getInitBB = [&]() {
      if (InitBB)
        return InitBB;
      auto *EntryBB = W->getEntryBBlock();
      InitBB = SplitBlock(EntryBB, EntryBB->getTerminator(), PT.DT, PT.LI);
      InitBB->setName("omp.priv.init");
      return InitBB;
    };

    BasicBlock *FiniBB = nullptr;
    auto && getFiniBB = [&]() {
      if (FiniBB)
        return FiniBB;
      FiniBB = W->getExitBBlock();
      auto *New = SplitBlock(FiniBB, FiniBB->getFirstNonPHI(), PT.DT, PT.LI);
      W->setExitBBlock(New);
      FiniBB->setName(".omp.priv.fini");
      return FiniBB;
    };

    // Fixup workers.
    auto Mode = getSPMDMode(W);
    for (auto &WI : Workers) {
      switch (Mode) {
        case 0:
          WI.makeBlocked();
          break;
        case 1:
          WI.makeCyclic();
          break;
        default:
          WI.makeHybrid(Mode);
      }

      // Generate privatization code.
      WorkerPrivatizer(PT, WI, getInitBB, getFiniBB).run();

      // Mark loop as parallel.
      WI.addParallelIntrinsicCalls();
    }

    // Make workers independent.
    if (Workers.size() > 1u) {
      auto *Region = genParRegionCalls(getParRegionID(W),
                                       W->getEntryBBlock()->getTerminator(),
                                       W->getExitBBlock()->getFirstNonPHI());
      for (auto &WI : Workers)
        genParSectionCalls(Region,
                           WI.Head->getFirstNonPHI(),
                           WI.Tail->getTerminator());
    }

    // Rebuild dominator tree.
    PT.DT->recalculate(*W->getEntryBBlock()->getParent());
    return true;
  }
};

bool VPOParoptTransform::isSupportedOnCSA(WRegionNode *W) {
  auto && reportWarning = [&](const Twine &Msg) {
    DebugLoc Loc;
    if (const auto *I = W->getEntryBBlock()->getFirstNonPHI())
      Loc = I->getDebugLoc();
    F->getContext().diagnose(CSADiagInfo(*F, Loc, Msg));
  };

  switch (W->getWRegionKindID()) {
    case WRegionNode::WRNTarget:
    case WRegionNode::WRNParallel:
    case WRegionNode::WRNParallelSections:
    case WRegionNode::WRNParallelLoop:
    case WRegionNode::WRNWksLoop:
    case WRegionNode::WRNSections:
    case WRegionNode::WRNAtomic:
    case WRegionNode::WRNSection:
    case WRegionNode::WRNMaster:
    case WRegionNode::WRNSingle:
    case WRegionNode::WRNBarrier:
      break;
    default:
      reportWarning("ignoring unsupported \"omp " + W->getName() + "\"");
      return false;
  }

  // num_threads
  if (W->getIsParLoop())
    if (auto *NumThreads = W->getNumThreads())
      if (!dyn_cast<ConstantInt>(NumThreads)) {
        reportWarning("num_threads must be a compile time constant");
        W->setNumThreads(nullptr);
      }

  // proc_bind
  if (W->getIsPar() && W->getProcBind() != WRNProcBindAbsent)
    reportWarning("ignoring unsupported proc_bind clause");

  // schedule
  if (W->canHaveSchedule()) {
    // Check schedule clause of there is one.
    auto &Sched = W->getSchedule();
    if (Sched.getChunkExpr()) {
      // So far we support only static and auto schedule types.
      if (Sched.getKind() != WRNScheduleStatic &&
          Sched.getKind() != WRNScheduleAuto) {
        reportWarning("ignoring unsupported schedule type");
        Sched.setChunkExpr(nullptr);
      }
      // Three options for the chunk size
      //   Sched->getChunk() == 0 => chunk was not specified
      //   Sched->getChunk() > 0  => chunk is a compile time constant
      //   Sched->getChunk() < 0  => chunk is an expression (unsupported)
      else if (Sched.getChunk() < 0) {
        reportWarning("schedule chunk must be a compile time constant");
        Sched.setChunkExpr(nullptr);
      }
    }
  }
  return true;
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

  if (DoLoopSplitting)
    return CSALoopSplitter(*this).run(W);

  auto *Loop = W->getWRNLoopInfo().getLoop();
  assert(Loop->isLoopSimplifyForm());

  // Determine the number of workers to use for this loop.
  auto NumWorkers = getNumWorkers(W);

  // Generate privatization code.
  if (UseExpLoopPrivatizer)
    CSAExpLoopPrivatizer(*this, W, NumWorkers <= 1u).run();
  else
    CSALoopPrivatizer(*this, W, NumWorkers <= 1u).run();

  // Annotating loop with spmdization entry/exit intrinsic calls if parallel
  // region has num_threads clause and the number of threads > 1.
  if (NumWorkers > 1u) {
    auto *M = F->getParent();

    auto *Entry = Intrinsic::getDeclaration(M,
        Intrinsic::csa_spmdization_entry);

    auto *Exit = Intrinsic::getDeclaration(M,
        Intrinsic::csa_spmdization_exit);

    // Determine SPMDization mode.
    auto Mode = getSPMDMode(W);

    IRBuilder<> Builder(W->getEntryBBlock()->getTerminator());
    auto *SpmdID = Builder.CreateCall(Entry,
                                      { Builder.getInt32(NumWorkers),
                                        Builder.getInt32(Mode) },
                                      "spmd");

    Builder.SetInsertPoint(&*W->getExitBBlock()->getFirstInsertionPt());
    Builder.CreateCall(Exit, { SpmdID }, {});
  }

  // Insert parallel region entry/exit calls
  auto *Region = genParRegionCalls(getParRegionID(W),
                                   W->getEntryBBlock()->getTerminator(),
                                   W->getExitBBlock()->getFirstNonPHI());

  // and CSA parallel section entry/exit intrinsics
  genParSectionCalls(Region,
                     Loop->getHeader()->getFirstNonPHI(),
                     Loop->getLoopLatch()->getTerminator());
  return true;
}

bool VPOParoptTransform::genCSASections(WRegionNode *W) {
  assert(isTargetCSA() && "unexpected target");

  // Generate privatization code for the sections construct.
  CSASectionsPrivatizer(*this, W).run();

  // Insert parallel region entry/exit calls
  auto *Region = genParRegionCalls(getParRegionID(W),
                                   W->getEntryBBlock()->getTerminator(),
                                   W->getExitBBlock()->getFirstNonPHI());

  // Insert section entry/exit calls in child work regions which all are
  // supposed to be sections.
  for (auto *WSec : W->getChildren()) {
    assert(WSec->getWRegionKindID() == WRegionNode::WRNSection &&
           "Unexpected work region kind");

    genParSectionCalls(Region,
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
  return true;
}

bool VPOParoptTransform::genCSASingle(WRegionNode *W) {
  assert(isTargetCSA() && "unexpected target");
  return CSAPrivatizer(*this, W).run();
}

bool VPOParoptTransform::translateCSAOmpRtlCalls() {
  assert(isTargetCSA() && "unexpected target");
  bool Changed = false;
  SmallVector<Instruction*, 8u> DeadInsts;
  for (auto &I : instructions(F))
    if (auto CS = CallSite(&I)) {
      LibFunc LF = NumLibFuncs;
      if (!TLI->getLibFunc(CS, LF))
        continue;

      Value *Val = nullptr;
      switch (LF) {
        case LibFunc_omp_get_thread_num:
        case LibFunc_omp_get_dynamic:
          Val = ConstantInt::get(Type::getInt32Ty(F->getContext()), 0);
          break;
        case LibFunc_omp_get_num_threads:
        case LibFunc_omp_get_max_threads:
        case LibFunc_omp_get_nested:
          Val = ConstantInt::get(Type::getInt32Ty(F->getContext()), 1u);
          break;
        case LibFunc_omp_set_num_threads:
        case LibFunc_omp_set_dynamic:
        case LibFunc_omp_set_nested:
          break;
        default:
          continue;
      }
      Changed = true;
      if (Val)
        CS->replaceAllUsesWith(Val);
      DeadInsts.push_back(CS.getInstruction());
    }
  for (auto *I : DeadInsts)
    I->eraseFromParent();
  return Changed;
}

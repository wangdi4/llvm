//=- VPOParoptTransformCSA.cpp - W-Region transformations for CSA -*- C++ -*-=//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/PatternMatch.h"

using namespace llvm;
using namespace llvm::PatternMatch;
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

static Value *genParDepthRegionEntryCall(unsigned ID, int Depth,
                                         Instruction *EntryPt,
                                         const Twine &Name = "depth.region");

static void genParDepthRegionExitCall(Value *Region, int Depth,
                                      Instruction *ExitPt);

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

  // Shortcuts to VPOParoptTransform's DT, LI, SE and AC members.
  DominatorTree *DT = nullptr;
  LoopInfo *LI = nullptr;
  ScalarEvolution *SE = nullptr;
  AssumptionCache *AC = nullptr;

  // Insertion points for init, fini and last private copyout code. Alloca
  // instructions for all private flavors as well as the intialization for the
  // first private and reduction variables is inserted before InitInsPt
  // instruction. Destructors for all types of privates as well as reduction
  // updates are inserted before the FiniInsPt instruction. And copyout code for
  // lastprivate variables is inserted before the LastInsPt instruction.
  Instruction *InitInsPt = nullptr;
  Instruction *FiniInsPt = nullptr;
  Instruction *LastInsPt = nullptr;

  // Keep track of alloca instruction that were created for private variables.
  // We will try to registerize these allocas.
  SmallSetVector<AllocaInst*, 16u> Allocas;

  // Lower reduction clauses if set to true.
  bool DoReds = true;

protected:
  // Creates alloca instruction for the given item, inserts it to the InitBB and
  // replaces all uses of the original variable within work region with a
  // private instance.
  virtual void genPrivItem(Item *I, WRegionNode *W, const Twine &Suffix) {
    auto *New = genPrivatizationAlloca(I, getInitInsPt(), Suffix);
    addAlloca(New);
    I->setNew(New);
    auto *Rep = getClauseItemReplacementValue(I, getInitInsPt());
    if (Rep != New)
      addAlloca(Rep);
    PT.genPrivatizationReplacement(W, I->getOrig(), Rep);
  }

  virtual Instruction* getInitInsPt() {
    if (InitInsPt)
      return InitInsPt;
    auto *EntryBB = W->getEntryBBlock();
    auto *InitBB = SplitBlock(EntryBB, EntryBB->getTerminator(), DT, LI);
    InitBB->setName("omp.priv.init");
    InitInsPt = InitBB->getTerminator();
    return InitInsPt;
  }

  virtual Instruction* getFiniInsPt() {
    if (FiniInsPt)
      return FiniInsPt;
    auto *FiniBB = W->getExitBBlock();
    auto *New = SplitBlock(FiniBB, FiniBB->getFirstNonPHI(), DT, LI);
    W->setExitBBlock(New);
    FiniBB->setName(".omp.priv.fini");
    FiniInsPt = FiniBB->getTerminator();
    return FiniInsPt;
  }

  virtual Instruction* getLastInsPt() {
    llvm_unreachable("unexpected work region kind");
    return nullptr;
  }

  // Creates privatization code for the given private var.
  void genPrivVar(PrivateItem *I, WRegionNode *W) {
    genPrivItem(I, W, ".priv");
    if (auto *C = I->getConstructor())
      VPOParoptUtils::genConstructorCall(C, I->getNew(), I->getNew());
    if (auto *D = I->getDestructor())
      VPOParoptUtils::genDestructorCall(D, I->getNew(), getFiniInsPt());
  }

  // Creates privatization code for the given firstprivate var.
  void genFPrivVar(FirstprivateItem *I, WRegionNode *W) {
    if (auto *LP = I->getInLastprivate())
      if (auto *NewLP = LP->getNew()) {
        I->setNew(NewLP);
        return;
      }
    genPrivItem(I, W, ".fpriv");
    PT.genFprivInit(I, getInitInsPt());
    if (auto *D = I->getDestructor())
      VPOParoptUtils::genDestructorCall(D, I->getNew(), getFiniInsPt());
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
      VPOParoptUtils::genDestructorCall(D, I->getNew(), getFiniInsPt());
  }

  // Creates copyout code for the given lastprivate var.
  virtual void genLPrivVarCopyout(LastprivateItem *I) {
    PT.genLprivFini(I, getLastInsPt());
  }

  // Creates reduction code for the given var.
  void genRedVar(ReductionItem *I, WRegionNode *W) {
    PT.computeArraySectionTypeOffsetSize(*I, getInitInsPt());
    genPrivItem(I, W, ".red");
    PT.genReductionInit(W, I, getInitInsPt(), DT);
    PT.genReductionFini(W, I, I->getOrig(), getFiniInsPt(), DT);
  }

  // Adds value to the list of candidates for registerization if it is an alloca
  // instruction.
  void addAlloca(Value *V) {
    if (auto *AI = dyn_cast<AllocaInst>(V))
      Allocas.insert(AI);
  }

  // Promote alloca instructions that were created for private variables to
  // registers if that is possible.
  void registerizeAllocas() {
    if (Allocas.empty())
      return;
    SmallVector<AllocaInst*, 16u> PAllocas;
    copy_if(reverse(Allocas), std::back_inserter(PAllocas), isAllocaPromotable);
    PromoteMemToReg(PAllocas, *DT, AC);
  }

  // Remove private references from directive bundle.
  void removePrivateRefsFromBundle() {
    auto && removeRefs = [this](Item *I) {
      auto &DirEntry = W->getEntryBBlock()->front();
      assert(VPOAnalysisUtils::isBeginDirective(&DirEntry));
      if (auto *New = I->getNew())
        DirEntry.replaceUsesOfWith(New, Constant::getNullValue(New->getType()));
    };

    if (W->canHavePrivate())
      for_each(W->getPriv().items(), removeRefs);
    if (W->canHaveFirstprivate())
      for_each(W->getFpriv().items(), removeRefs);
    if (W->canHaveLastprivate())
      for_each(W->getLpriv().items(), removeRefs);
    if (W->canHaveReduction())
      for_each(W->getRed().items(), removeRefs);
  }

public:
  CSAPrivatizer(VPOParoptTransform &PT, WRegionNode *W, bool DoReds = true)
    : PT(PT), W(W), DT(PT.DT), LI(PT.LI), SE(PT.SE), AC(PT.AC), DoReds(DoReds)
  {}

  virtual bool run() {
    bool Changed = false;

    W->populateBBSet();

    // Generate privatization code for all flavors of private variables.
    if (W->canHavePrivate() && !W->getPriv().empty())
      for (auto *I : W->getPriv().items())
        if (!isa<Constant>(I->getOrig())) {
          genPrivVar(I, W);
          Changed = true;
        }
    if (W->canHaveFirstprivate() && !W->getFpriv().empty())
      for (auto *I : W->getFpriv().items())
        if (!isa<Constant>(I->getOrig())) {
          genFPrivVar(I, W);
          Changed = true;
        }
    if (W->canHaveLastprivate() && !W->getLpriv().empty())
      for (auto *I : W->getLpriv().items())
        if (!isa<Constant>(I->getOrig())) {
          genLPrivVar(I, W);
          genLPrivVarCopyout(I);
          Changed = true;
        }
    if (DoReds && W->canHaveReduction() && !W->getRed().empty())
      for (auto *I : W->getRed().items())
        if (!isa<Constant>(I->getOrig())) {
          genRedVar(I, W);
          Changed = true;
        }

    if (Changed) {
      // Remove references to private items from the WRN's bundle.
      removePrivateRefsFromBundle();

      // Promote private variables to registers.
      registerizeAllocas();
    }

    // Invalidate BBSet after transformation
    W->resetBBSetIfChanged(Changed);

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
  Instruction* getLastInsPt() override {
    if (LastInsPt)
      return LastInsPt;

    // Get loop's induction variable and upper bound.
    auto *IV = WRegionUtils::getOmpCanonicalInductionVariable(L);
    auto *UB = WRegionUtils::getOmpLoopUpperBound(L);

    // Induction variable value will match upper bound on the last
    // iteration of the loop.
    auto *IsLast = new ICmpInst(L->getHeader()->getFirstNonPHI(),
        ICmpInst::ICMP_EQ, IV, UB, "is.last");

    // Insert if-then blocks at the end of the latch block. Copyout code
    // for last private variables will be emitted to this block.
    LastInsPt = SplitBlockAndInsertIfThen(IsLast,
        L->getLoopLatch()->getTerminator(), false, nullptr, DT, LI);
    auto *LastIterBB = LastInsPt->getParent();
    LastIterBB->getSingleSuccessor()->setName("omp.lpriv.end");
    LastIterBB->setName("omp.lpriv.copyout");
    return LastInsPt;
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
template <typename DR, typename UR>
static bool findDefs(Value *V, DR && DefBBs, UR && UseBBs,
                     SmallSetVector<Value*, 8u> &Defs) {
  bool IsDef = false;
  for (auto *U : V->users()) {
    if (auto *I = dyn_cast<Instruction>(U)) {
      auto *BB = I->getParent();
      if (!is_contained(DefBBs, BB))
        continue;
      if (is_contained(UseBBs, BB) ||
          findDefs(I, DefBBs, UseBBs, Defs))
        IsDef = true;
    }
  }
  if (IsDef)
    Defs.insert(V);
  return IsDef;
}

template <typename T>
static void replaceUses(Value *Old, Value *New, const T &Users) {
  for (auto *User : Users)
    cast<Instruction>(User)->replaceUsesOfWith(Old, New);
}

// Experimental variant of CSA loop privatizer.
class VPOParoptTransform::CSAExpLoopPrivatizer :
    public VPOParoptTransform::CSALoopPrivatizer {
protected:
  // Insertion point for in-loop allocas which are executed for all iterations
  // except first.
  Instruction *AllocaInsPt = nullptr;

  // And insertion point for instructions at the beginning of the loop body.
  Instruction *InLoopInsPt = nullptr;

  // Maps private variable to its replacement value.
  SmallDenseMap<Value*, Value*, 8u> Orig2Rep;

protected:
  // Returns true if in loop alloca has to be created for the given item. So far
  // this is needed for non scalar privates of all kinds.
  static bool needsInLoopAlloca(const Item *I) {
    Type *ElemType = nullptr;
    Value *NumElems = nullptr;
    unsigned AddrSpace = 0u;
    getItemInfoFromValue(I->getOrig(), ElemType, NumElems, AddrSpace);
    if (I->getIsByRef())
      ElemType = cast<PointerType>(ElemType)->getPointerElementType();
    return NumElems || ElemType->isAggregateType();
  }

  // Creates not-first-iter block if work region needs it. Returns true if it
  // has been created or false otherwise.
  virtual bool createNotFirstIterBlock() {
    // Not-first-iter basic block has to be created only if we have a non-scalar
    // variable in private clauses.
    if (none_of(W->getPriv().items(), needsInLoopAlloca) &&
        none_of(W->getFpriv().items(), needsInLoopAlloca) &&
        none_of(W->getLpriv().items(), needsInLoopAlloca) &&
        none_of(W->getRed().items(), needsInLoopAlloca))
      return false;

    // Get loop's induction variable and lower bound.
    auto *IV = WRegionUtils::getOmpCanonicalInductionVariable(L);
    auto *LB = WRegionUtils::getOmpLoopLowerBound(L);

    // Insertion point for instructions at the beginning of the loop body.
    InLoopInsPt = L->getHeader()->getFirstNonPHI();

    // Induction variable value will match lower bound on the first iteration of
    // the loop.
    auto *IsNotFirst = new ICmpInst(InLoopInsPt, ICmpInst::ICMP_NE, IV, LB,
                                    "is.not.first");

    // Insert if-then blocks at the beginning of the lpop header which is
    // executed for all iterations except first.
    AllocaInsPt = SplitBlockAndInsertIfThen(IsNotFirst, InLoopInsPt, false,
                                            nullptr, DT, LI);
    auto *NotFirstIterBB = AllocaInsPt->getParent();
    NotFirstIterBB->setName("not.first.iter");
    NotFirstIterBB->getSingleSuccessor()->setName("not.first.iter.end");
    return true;
  }

  void genPrivItem(Item *I, WRegionNode *W, const Twine &Suffix) override {
    // Do the "standard" privatization if this item does not require extra
    // alloca inside the loop.
    if (!needsInLoopAlloca(I))
      return CSALoopPrivatizer::genPrivItem(I, W, Suffix);

    auto *New = genPrivatizationAlloca(I, getInitInsPt(), Suffix);
    addAlloca(New);
    I->setNew(New);

    // By-ref item are handled differently, just need to store an in-loop
    // alloca address to the reference.
    if (I->getIsByRef()) {
      auto *Rep = getClauseItemReplacementValue(I, getInitInsPt());
      assert(Rep != New && isa<AllocaInst>(Rep) && "unexpected replacement");
      addAlloca(Rep);
      PT.genPrivatizationReplacement(W, I->getOrig(), Rep);

      auto *NewNF = genPrivatizationAlloca(I, AllocaInsPt,
                                           Suffix + ".not.first");
      new StoreInst(NewNF, Rep, AllocaInsPt);
      return;
    }

    // Replace original value with a private instance.
    PT.genPrivatizationReplacement(W, I->getOrig(), New);

    // Find all instructions depending on the private variable which need to be
    // replicated.
    SmallSetVector<Value*, 8u> Defs;
    findDefs(New, W->blocks(), L->blocks(), Defs);
    if (Defs.empty())
      return;

    // Create reference which will hold the private variable address.
    auto AAS = New->getModule()->getDataLayout().getAllocaAddrSpace();
    auto *Ref = new AllocaInst(New->getType(), AAS, New->getName() + ".ref",
                               getInitInsPt());
    addAlloca(Ref);
    new StoreInst(New, Ref, getInitInsPt());

    // Create in-loop alloca and store new address to the reference.
    auto *NewNF = genPrivatizationAlloca(I, AllocaInsPt,
                                         Suffix + ".not.first");
    new StoreInst(NewNF, Ref, AllocaInsPt);

    // And finally replace all private uses within loop body with the address
    // loaded from the private refence.
    auto *Rep = new LoadInst(Ref, New->getName() + ".in.loop", InLoopInsPt);

    // Save replacement value.
    Orig2Rep[I->getOrig()] = Rep;

    // Clone all dependent defs at the beginning of the loop body.
    ValueToValueMapTy VMap;
    VMap[New] = Rep;
    for (const auto *I : reverse(Defs)) {
      if (VMap[I])
        continue;
      auto *C = cast<Instruction>(I)->clone();
      C->insertBefore(InLoopInsPt);
      VMap[I] = C;
    }

    // Replace uses of defs within the loop with clones.
    for (auto *V : Defs) {
      SmallVector<Value*, 8u> Uses;
      copy_if(V->users(), std::back_inserter(Uses), [this](const Value *U) {
        if (const auto *I = dyn_cast<Instruction>(U))
          return L->contains(I);
        return false;
      });
      replaceUses(V, VMap[V], Uses);

      // Defs with no uses can be safely deleted.
      if (auto *I = dyn_cast<Instruction>(V))
        if (I->use_empty())
          I->eraseFromParent();
    }
  }

  void genLPrivVarCopyout(LastprivateItem *I) override {
    auto *Old = I->getOrig();
    auto *Rep = Orig2Rep.lookup(Old);
    if (!Rep)
      return CSALoopPrivatizer::genLPrivVarCopyout(I);

    auto *New = cast<AllocaInst>(I->getNew());

    const auto &DL = New->getModule()->getDataLayout();
    auto *InsPt = getLastInsPt();

    if (auto *CA = I->getCopyAssign())
      VPOParoptUtils::genCopyAssignCall(CA, Old, Rep, InsPt);
    else if (VPOUtils::canBeRegisterized(New->getAllocatedType(), DL))
      new StoreInst(new LoadInst(Rep, "", InsPt), Old, InsPt);
    else
      VPOUtils::genMemcpy(Old, Rep, DL, New->getAlignment(), InsPt);
  }

public:
  CSAExpLoopPrivatizer(VPOParoptTransform &PT, WRegionNode *W, bool DoReds,
    Loop *LL = nullptr)
    : CSALoopPrivatizer(PT, W, DoReds, LL)
  {}

  bool run() override {
    // Create not-first-iter block if needed before doing privatization.
    createNotFirstIterBlock();
    return CSALoopPrivatizer::run();
  }
};

// Traverses work region's [first|last]private clauses and sets item's new
// value to nullptr.
static void cleanupPrivateItems(WRegionNode *W) {
  if (W->canHavePrivate())
    for (auto *I : W->getPriv().items())
      I->setNew(nullptr);
  if (W->canHaveFirstprivate())
    for (auto *I : W->getFpriv().items())
      I->setNew(nullptr);
  if (W->canHaveLastprivate())
    for (auto *I : W->getLpriv().items())
      I->setNew(nullptr);
  if (W->canHaveReduction())
    for (auto *I : W->getRed().items())
      I->setNew(nullptr);
}

// CSA privatizer customization for the sections construct. Each section
// is treated as an idependent worker, so privatization is done for each
// section.
class VPOParoptTransform::CSASectionsPrivatizer final :
    public VPOParoptTransform::CSAPrivatizer {
private:
  // Last iteration block for sections goes to the end of the last section.
  Instruction* getLastInsPt() override {
    if (LastInsPt)
      return LastInsPt;

    auto *LastSecW = W->getChildren().front();
    auto *LastIterBB = LastSecW->getExitBBlock();
    auto *New = SplitBlock(LastIterBB, LastIterBB->getFirstNonPHI(), DT, LI);
    LastSecW->setExitBBlock(New);
    LastIterBB->setName("omp.lpriv.copyout");
    LastInsPt = LastIterBB->getTerminator();
    return LastInsPt;
  }

public:
  CSASectionsPrivatizer(VPOParoptTransform &PT, WRegionNode *W)
    : CSAPrivatizer(PT, W) {
    assert(W->getIsSections() && "sections construct is expected");
  }

  bool run() override {
    bool Changed = false;

    W->populateBBSet();

    // For sections construct we need to privatizations for each section.
    for (auto SecW : reverse(W->getChildren())) {
      bool SecChanged = false;

      SecW->populateBBSet();
      cleanupPrivateItems(W);
      for (auto *I : W->getPriv().items())
        if (!isa<Constant>(I->getOrig())) {
          genPrivVar(I, SecW);
          SecChanged = true;
        }
      for (auto *I : W->getFpriv().items())
        if (!isa<Constant>(I->getOrig())) {
          genFPrivVar(I, SecW);
          SecChanged = true;
        }
      for (auto *I : W->getLpriv().items())
        if (!isa<Constant>(I->getOrig())) {
          genLPrivVar(I, SecW);
          SecChanged = true;
        }
      for (auto *I : W->getRed().items())
        if (!isa<Constant>(I->getOrig())) {
          genRedVar(I, SecW);
          SecChanged = true;
        }
      SecW->resetBBSetIfChanged(SecChanged);

      Changed |= SecChanged;
    }

    if (Changed) {
      // Gen copyout code for lastprivate variables.
      for (auto *I : W->getLpriv().items())
        if (!isa<Constant>(I->getOrig()))
          genLPrivVarCopyout(I);

      // Remove references to private items from the WRN's bundle.
      removePrivateRefsFromBundle();

      // Promote private variables to registers.
      registerizeAllocas();
    }

    // Invalidate BBSet after transformations
    W->resetBBSetIfChanged(Changed);

    return Changed;
  }
};

// Insert a CSA parallel depth-limited region entry call
// at specified insertion point.
static Value *genParDepthRegionEntryCall(unsigned ID, int Depth,
                                         Instruction *EntryPt,
                                         const Twine &Name) {

  if (Depth > 0) {
    auto *M = EntryPt->getModule();
    auto *DepthLimitedRegionEntry =
        Intrinsic::getDeclaration(M, Intrinsic::csa_pipeline_limited_entry);
    IRBuilder<> Builder(EntryPt);
    auto *UniqueID = Builder.getInt32(2000u + ID);
    auto *F = EntryPt->getParent()->getParent();
    Value *pipelineDepth =
        ConstantInt::get(IntegerType::get(F->getContext(), 32), Depth);
    auto *Region = Builder.CreateCall(DepthLimitedRegionEntry,
                                      {UniqueID, pipelineDepth}, Name);
    return Region;
  }
  return 0;
}

// Insert a CSA parallel depth-limited region exit call
// at specified insertion point.
static void genParDepthRegionExitCall(Value *Region, int Depth,
                                      Instruction *ExitPt) {

  if (Region and Depth > 0) {
    auto *M = ExitPt->getModule();
    auto *DepthLimitedRegionExit =
        Intrinsic::getDeclaration(M, Intrinsic::csa_pipeline_limited_exit);
    IRBuilder<> Builder(ExitPt);
    Builder.SetInsertPoint(ExitPt);
    Builder.CreateCall(DepthLimitedRegionExit, {Region}, {""});
  }
}

static void genParDepthRegionCalls(unsigned ID, int Depth, Instruction *EntryPt,
                                   Instruction *ExitPt, const Twine &Name) {

  Value *DepthRegion =
      genParDepthRegionEntryCall(5000 + ID, Depth, EntryPt, "depth." + Name);
  genParDepthRegionExitCall(DepthRegion, Depth, ExitPt);
}

// Insert a pair of CSA parallel region entry/exit calls at specified insertion
// points.
static Value *genParRegionCalls(unsigned ID, Instruction *EntryPt,
                                Instruction *ExitPt, int Depth = 0,
                                const Twine &Name = "region") {
  auto *M = EntryPt->getModule();
  assert(M == ExitPt->getModule());

  Value *DepthRegion;
  if (Depth)
    DepthRegion =
        genParDepthRegionEntryCall(5000 + ID, Depth, EntryPt, "depth." + Name);

  // CSA parallel region entry/exit intrinsics
  auto *RegionEntry = Intrinsic::getDeclaration(M,
      Intrinsic::csa_parallel_region_entry);

  auto *RegionExit = Intrinsic::getDeclaration(M,
      Intrinsic::csa_parallel_region_exit);

  IRBuilder<> Builder(EntryPt);
  auto *UniqueID = Builder.getInt32(ID);
  auto *Region = Builder.CreateCall(RegionEntry, { UniqueID }, Name);

  Builder.SetInsertPoint(ExitPt);
  Builder.CreateCall(RegionExit, { Region }, {}, "");

  if (Depth)
    genParDepthRegionExitCall(DepthRegion, Depth, ExitPt);

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
  Builder.CreateCall(SectionExit, { Section }, {}, "");
}

// Returns the number of workers to be created for the loop WRN.
static unsigned getNumWorkers(const WRegionNode *W) {
  assert(W->getIsOmpLoop() && "expecting a loop construct");
  unsigned NumWorkers = 0;
  // First check if dataflow(num_workers(n)) has been specified.
  // If num_workers is not specified, check num_threads(n) clause
  if (W->getIsPar()) {
    auto NumSAWorkers = W->getNumWorkers();
    if (NumSAWorkers != 0)
      NumWorkers = NumSAWorkers;
    else if (auto *NumThreads = W->getNumThreads())
      NumWorkers = cast<ConstantInt>(NumThreads)->getZExtValue();
  }
  if (!NumWorkers)
    NumWorkers = LoopWorkersDefault;

  return NumWorkers;
}

// Returns SPMDization mode for the loop WRN.
// Use the dataflow(<schedule>) specification first.
//   No schedule, schedule(auto) or
//   schedule(static)                 => blocked SPMD (0)
//   schedule(static, chunksize)
//     (chunksize == 1)               => cyclic SPMD  (1)
//     (chunksize > 1)                => hybrid SPMD  (chunksize)
static int getSPMDMode(const WRegionNode *W) {
  assert(W->getIsOmpLoop() && "expecting a loop construct");
  const auto &WSched = W->getWorkerSchedule();
  if (WSched.getChunkExpr()) {
    return WSched.getChunk();
  }
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

// Unified way of calculating parallel region ID for CSA parallel region entry
// intrinsic call.
static unsigned getParRegionID(WRegionNode *W,
                               unsigned Worker = 0,
                               bool IsHybrid = false) {
  return 2000u + W->getNumber() + (Worker << 16u) + (IsHybrid << 28u);
}

class VPOParoptTransform::CSALoopSplitter {
  VPOParoptTransform &PT;

  // TODO: makes sense to have OptimizationRemarkEmitterAnalysis as a dependency
  // for paropt transform passes, but until it is done use a local instance of
  // optimization remarks emitter.
  OptimizationRemarkEmitter ORE;

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
    // Check if worker needs ZTT for the loop (outter loop for hybrid mode)
    // based on the min TC esimate if available.
    bool needsZTT() const {
      if (LS.MinTC) {
        auto MinTC = LS.MinTC->getZExtValue();
        if (!LS.SPMDMode) {
          // Blocked mode. All iterations [0, TC) are divided across N workers
          // at runtime. Let TCi be the number of iterations executed by i'th
          // worker.
          //
          //   TC0 = (TC/N) + D, where D is [0, 1)
          //   TCi+1 <= TCi
          //
          // I'th worker does not need ZTT if
          //   i*TC0 < TC
          //
          // or, after expanding TC0
          //   i*((TC/N) + D) < TC
          //
          // In the worst case D = (N-1)/N, therefore
          //   i*((TC/N) + ((N-1)/N)) < TC
          //   i*TC + i*(N-1) < N*TC
          //
          // And finally
          //   (N-i)*TC > i*(N-1)
          //
          // Many thanks to Nikolai Bozhenov for helping with these formulas.
          auto NW = LS.Workers.size();
          return (NW - ID) * MinTC <= ID * (NW - 1u);
        }
        // Cyclic or Hybrid mode.
        return MinTC <= ID * LS.SPMDMode;
      }
      return true;
    }

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

        // Try to eliminate ZTT if this is the outter loop.
        if (LS.SPMDMode <= 1u && !needsZTT() && Ztt->hasOneUse())
          if (auto *BI = dyn_cast<BranchInst>(*Ztt->user_begin()))
            if (auto *LPH = WL->getLoopPreheader()) {
              BranchInst::Create(LPH, BI);
              BI->eraseFromParent();
              assert(Ztt->use_empty());
              Ztt->eraseFromParent();
              Ztt = nullptr;

              LS.ORE.emit(OptimizationRemark(DEBUG_TYPE, "", WL->getStartLoc(),
                                             WL->getHeader())
                          << "CSA Paropt zero trip count check was not "
                             "inserted for Worker #" + std::to_string(ID) +
                             ", as directed by the __builtin_assume.");
            }

        // Fixup loop bounds for ZTT if it is still there.
        if (Ztt) {
          if (NewUB)
            Ztt->setOperand(Ztt->getOperand(0) == LB, NewUB);
          if (NewLB)
            Ztt->setOperand(Ztt->getOperand(0) != LB, NewLB);
        }
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

    void makeHybrid() {
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
      auto *CLB = ConstantInt::get(Ty, ID * LS.SPMDMode, IsSigned);
      if (needsZTT()) {
        auto *Cond = IsSigned ? Builder.CreateICmpSLE(CLB, UB) :
                                Builder.CreateICmpULE(CLB, UB);
        Builder.CreateCondBr(Cond, CLPH, Tail);
      }
      else
        Builder.CreateBr(CLPH);

      // Create blocked LB.
      Builder.SetInsertPoint(&CLoop->front());
      auto *BLB = Builder.CreatePHI(Ty, 2, "blocked.lb" + Suffix);
      BLB->addIncoming(CLB, CLPH);

      // And blocked UB.
      auto *BNext = Builder.CreateAdd(BLB,
          ConstantInt::get(Ty, LS.SPMDMode - 1u, IsSigned));
      auto *BCmp = IsSigned ? Builder.CreateICmpSLE(BNext, UB) :
                              Builder.CreateICmpULE(BNext, UB);
      auto *BUB = Builder.CreateSelect(BCmp, BNext, UB, "blocked.ub" + Suffix);

      // Create latch code.
      Builder.SetInsertPoint(CLatch->getTerminator());
      auto *CStride = ConstantInt::get(Ty, LS.Workers.size() * LS.SPMDMode,
                                       IsSigned);
      auto *CNext = Builder.CreateAdd(BLB, CStride);
      BLB->addIncoming(CNext, CLatch);

      auto *LatchCond = IsSigned ? Builder.CreateICmpSLE(CNext, UB) :
                                   Builder.CreateICmpULE(CNext, UB);
      Builder.CreateCondBr(LatchCond, CLoop, CExit);

      // Fixup inner loop to run from blocked LB to blocked UB.
      fixupLoopBounds(BLB, BUB, CLoop);

      // Mark inner loop as parallel.
      auto *Region = genParRegionCalls(
          getParRegionID(W, ID + 1u, true), CLoop->getTerminator(),
          CLatch->getFirstNonPHI(), /*Depth=*/0, "inner.region" + Suffix);
      genParSectionCalls(Region,
                         WL->getHeader()->getFirstNonPHI(),
                         WL->getLoopLatch()->getTerminator(),
                         "inner.section" + Suffix);

      // Update loop structure, create outer hybrid loop.
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
      auto *Region = genParRegionCalls(
          getParRegionID(W, ID + 1u), Head->getTerminator(),
          Tail->getFirstNonPHI(), /*Depth*/ 0, "region" + Suffix);

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
    function_ref<Instruction*()> InitInsPtGetter;
    function_ref<Instruction*()> FiniInsPtGetter;

  private:
    Instruction* getInitInsPt() override {
      if (InitInsPt)
        return InitInsPt;
      InitInsPt = InitInsPtGetter();
      return InitInsPt;
    }

    Instruction* getFiniInsPt() override {
      if (FiniInsPt)
        return FiniInsPt;
      FiniInsPt = FiniInsPtGetter();
      return FiniInsPt;
    }

    void genPrivItem(Item *I, WRegionNode *W, const Twine &Suffix) override {
      auto *Old = I->getOrig();
      assert(!isa<Constant>(Old) && "unexpected private item");
      SmallVector<Value*, 8u> Users;
      findUsers(Old, Users, WI.Blocks);

      auto *New = genPrivatizationAlloca(I, getInitInsPt(), Suffix + WI.Suffix);
      addAlloca(New);
      I->setNew(New);
      auto *Rep = getClauseItemReplacementValue(I, getInitInsPt());
      if (Rep != New)
        addAlloca(Rep);
      replaceUses(Old, Rep, Users);
    }

  public:
    WorkerPrivatizer(VPOParoptTransform &PT, const Worker &WI,
                     function_ref<Instruction*()> InitInsPtGetter,
                     function_ref<Instruction*()> FiniInsPtGetter)
      : CSALoopPrivatizer(PT, WI.W, true, WI.WL), WI(WI),
        InitInsPtGetter(InitInsPtGetter), FiniInsPtGetter(FiniInsPtGetter)
    {}

    bool run() override {
      cleanupPrivateItems(W);
      return CSALoopPrivatizer::run();
    }
  };

  // Experimental version of the worker privatizer.
  class ExpWorkerPrivatizer final : public CSAExpLoopPrivatizer {
    Worker &WI;
    function_ref<Instruction*()> InitInsPtGetter;
    function_ref<Instruction*()> FiniInsPtGetter;

  private:
    Instruction* getInitInsPt() override {
      if (InitInsPt)
        return InitInsPt;
      InitInsPt = InitInsPtGetter();
      return InitInsPt;
    }

    Instruction* getFiniInsPt() override {
      if (FiniInsPt)
        return FiniInsPt;
      FiniInsPt = FiniInsPtGetter();
      return FiniInsPt;
    }

    bool createNotFirstIterBlock() override {
      if (CSAExpLoopPrivatizer::createNotFirstIterBlock()) {
        // Refresh worker's set of blocks.
        SmallPtrSet<BasicBlock*, 16u> Blocks;
        Blocks.insert(WI.Blocks.begin(), WI.Blocks.end());
        Blocks.insert(L->block_begin(), L->block_end());
        WI.Blocks.assign(Blocks.begin(), Blocks.end());
      }
      return false;
    }

    void genPrivItem(Item *I, WRegionNode *W, const Twine &Suffix) override {
      auto *Old = I->getOrig();

      // Do the "standard" privatization if this item does not require extra
      // alloca inside the loop.
      if (!needsInLoopAlloca(I)) {
        SmallVector<Value*, 8u> Users;
        findUsers(Old, Users, WI.Blocks);

        auto *New = genPrivatizationAlloca(I, getInitInsPt(),
                                           Suffix + WI.Suffix);
        addAlloca(New);
        I->setNew(New);
        auto *Rep = getClauseItemReplacementValue(I, getInitInsPt());
        if (Rep != New)
          addAlloca(Rep);
        replaceUses(Old, Rep, Users);
        return;
      }

      auto *New = genPrivatizationAlloca(I, getInitInsPt(), Suffix + WI.Suffix);
      addAlloca(New);
      I->setNew(New);

      // By-ref item are handled differently, just need to store an in-loop
      // alloca address to the reference.
      if (I->getIsByRef()) {
        auto *Rep = getClauseItemReplacementValue(I, getInitInsPt());
        assert(Rep != New && isa<AllocaInst>(Rep) && "unexpected replacement");
        addAlloca(Rep);
        PT.genPrivatizationReplacement(W, Old, Rep);

        auto *NewNF = genPrivatizationAlloca(I, AllocaInsPt,
                                             Suffix + WI.Suffix + ".not.first");
        new StoreInst(NewNF, Rep, AllocaInsPt);
        return;
      }

      // Replace original value with a private instance.
      SmallVector<Value*, 8u> Users;
      findUsers(Old, Users, WI.Blocks);
      replaceUses(Old, New, Users);

      // Find all instructions depending on the private variable which need to
      // be replicated.
      SmallSetVector<Value*, 8u> Defs;
      findDefs(New, WI.Blocks, L->blocks(), Defs);
      if (Defs.empty())
        return;

      // Create reference which will hold the private variable address.
      auto AAS = New->getModule()->getDataLayout().getAllocaAddrSpace();
      auto *Ref = new AllocaInst(New->getType(), AAS, New->getName() + ".ref",
                                 getInitInsPt());
      addAlloca(Ref);
      new StoreInst(New, Ref, getInitInsPt());

      // Create in-loop alloca and store new address to the reference.
      auto *NewNF = genPrivatizationAlloca(I, AllocaInsPt,
                                           Suffix + WI.Suffix + ".not.first");
      new StoreInst(NewNF, Ref, AllocaInsPt);

      // And finally replace all private uses within loop body with the address
      // loaded from the private refence.
      auto *Rep = new LoadInst(Ref, New->getName() + ".in.loop", InLoopInsPt);

      // Save replacement value.
      Orig2Rep[Old] = Rep;

      // Clone all dependent defs at the beginning of the loop body.
      ValueToValueMapTy VMap;
      VMap[New] = Rep;
      for (const auto *I : reverse(Defs)) {
        if (VMap[I])
          continue;
        auto *C = cast<Instruction>(I)->clone();
        C->insertBefore(InLoopInsPt);
        VMap[I] = C;
      }

      // Replace uses of defs within the loop with clones.
      for (auto *V : Defs) {
        SmallVector<Value*, 8u> Uses;
        copy_if(V->users(), std::back_inserter(Uses), [this](const Value *U) {
          if (const auto *I = dyn_cast<Instruction>(U))
            return L->contains(I);
          return false;
        });
        replaceUses(V, VMap[V], Uses);

        // Defs with no uses can be safely deleted.
        if (auto *I = dyn_cast<Instruction>(V))
          if (I->use_empty())
            I->eraseFromParent();
      }
    }

  public:
    ExpWorkerPrivatizer(VPOParoptTransform &PT, Worker &WI,
                       function_ref<Instruction*()> InitInsPtGetter,
                       function_ref<Instruction*()> FiniInsPtGetter)
      : CSAExpLoopPrivatizer(PT, WI.W, true, WI.WL), WI(WI),
        InitInsPtGetter(InitInsPtGetter), FiniInsPtGetter(FiniInsPtGetter)
    {}

    bool run() override {
      cleanupPrivateItems(W);
      return CSAExpLoopPrivatizer::run();
    }
  };

private:
  SmallVector<Worker, 8u> Workers;

  // SPMD mode
  //   Blocked  0
  //   Cyclic   1
  //   Hybrid   >1  (defines stride for cyclic loop)
  unsigned SPMDMode = 0u;

  // Loop trip count estimate derived from the function's assumption cache.
  Optional<APInt> MinTC;

private:
  void getTripCountEstimate(WRegionNode *W) {
    // Check if we can find the following pattern
    //
    // %Val = ...
    // %Cmp = icmp sgt i32 %Val, Imm1
    // call void @llvm.assume(i1 %Cmp)
    // %UB = add nsw i32 %Val, Imm2
    //
    // If it exists then min TC value would be (Imm1 + Imm2 + 2) taking into
    // account that OpenMP loops are normalized and have 0 as lower bound.
    //
    // It comes from the following computations:
    //   UB = Val + Imm2;   // The pattern we are looking for
    //   UB = TC - 1;	    // True for a normalizer loop
    // =>
    //   TC = Val + Imm2 + 1;
    //
    // From the assumption cache we can find that
    //   Val > Imm1         // Pattern that we are serching assumptions for
    //
    // This leads to
    //   TC > Imm1 + Imm2 + 1;
    // =>
    //   MinTC = Imm1 + Imm2 + 2;

    auto *WL = W->getWRNLoopInfo().getLoop();
    auto *UB = WRegionUtils::getOmpLoopUpperBound(WL);

    APInt TC;

    Value *Val1 = nullptr;
    const APInt *C1 = nullptr;
    if (match(UB, m_c_Add(m_Value(Val1), m_APInt(C1))))
      for (auto &VH : PT.AC->assumptionsFor(Val1)) {
        if (!VH)
          continue;
        auto *AV = cast<CallInst>(VH)->getOperand(0);
        CmpInst::Predicate Pred = CmpInst::BAD_ICMP_PREDICATE;
        Value *Val2 = nullptr;
        const APInt *C2 = nullptr;
        if (match(AV, m_ICmp(Pred, m_Value(Val2), m_APInt(C2)))) {
          if (Pred == CmpInst::ICMP_SGT ||
              Pred == CmpInst::ICMP_UGT) {
            TC = *C1 + *C2 + 2u;
            break;
          }
          else if (Pred == CmpInst::ICMP_SGE ||
                   Pred == CmpInst::ICMP_UGE) {
            TC = *C1 + *C2 + 1u;
            break;
          }
        }
      }

    // Save TC if it has meaningful value.
    if (TC.isStrictlyPositive())
      MinTC = TC;
  }

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

    W->populateBBSet(true);

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
  CSALoopSplitter(VPOParoptTransform &PT) : PT(PT), ORE(PT.F) {}

  bool run(WRegionNode *W) {
    assert(W->getIsOmpLoop() && "must be a loop");

    SPMDMode = getSPMDMode(W);

    // Determine pipelining depth
    auto LoopPipelineDepth = W->getPipelineDepth();

    // Check if we can get TC estimate from the function's assumption cache.
    getTripCountEstimate(W);

    // Create workers.
    cloneWorkers(W);

    // Fixup workers.
    for (auto &WI : Workers)
      switch (SPMDMode) {
        case 0:
          WI.makeBlocked();
          break;
        case 1:
          WI.makeCyclic();
          break;
        default:
          WI.makeHybrid();
      }

    // Rebuild dominator tree.
    PT.DT->recalculate(*W->getEntryBBlock()->getParent());

    // Helper lamdas for the privatizer.
    Instruction *InitInsPt = nullptr;
    auto && getInitInsPt = [&]() {
      if (InitInsPt)
        return InitInsPt;
      auto *EntryBB = W->getEntryBBlock();
      auto *InitBB = SplitBlock(EntryBB, EntryBB->getTerminator(), PT.DT,
                                PT.LI);
      InitBB->setName("omp.priv.init");
      InitInsPt = InitBB->getTerminator();
      return InitInsPt;
    };

    Instruction *FiniInsPt = nullptr;
    auto && getFiniInsPt = [&]() {
      if (FiniInsPt)
        return FiniInsPt;
      auto *FiniBB = W->getExitBBlock();
      auto *New = SplitBlock(FiniBB, FiniBB->getFirstNonPHI(), PT.DT, PT.LI);
      W->setExitBBlock(New);
      FiniBB->setName(".omp.priv.fini");
      FiniInsPt = FiniBB->getTerminator();
      return FiniInsPt;
    };

    for (auto &WI : Workers) {
      // Generate privatization code.
      if (UseExpLoopPrivatizer)
        ExpWorkerPrivatizer(PT, WI, getInitInsPt, getFiniInsPt).run();
      else
        WorkerPrivatizer(PT, WI, getInitInsPt, getFiniInsPt).run();

      // Mark loop as parallel.
      WI.addParallelIntrinsicCalls();
    }

    // Make workers independent.
    if (Workers.size() > 1u) {
      auto *Region = genParRegionCalls(
          getParRegionID(W), W->getEntryBBlock()->getTerminator(),
          W->getExitBBlock()->getFirstNonPHI(), LoopPipelineDepth);
      for (auto &WI : Workers)
        genParSectionCalls(Region,
                           WI.Head->getFirstNonPHI(),
                           WI.Tail->getTerminator());
    } else {
      genParDepthRegionCalls(getParRegionID(W), LoopPipelineDepth,
                             W->getEntryBBlock()->getTerminator(),
                             W->getExitBBlock()->getFirstNonPHI(),
                             "depth.region");
    }
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
    case WRegionNode::WRNSection:
    case WRegionNode::WRNMaster:
    case WRegionNode::WRNSingle:
    case WRegionNode::WRNBarrier:
    case WRegionNode::WRNCritical:
      break;
    case WRegionNode::WRNAtomic:
      // Atomics for variables <= 64-bits are lowered by FE.
      reportWarning("\"omp atomic\" is not supported for variables larger "
                    "64 bits");
      return false;
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
    // Check schedule clause if there is one.
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

  // dataflow(static)
  if (W->canHaveWorkerSchedule()) {
    // Check dataflow(<schedule>) clause of there is one.
    auto &Sched = W->getWorkerSchedule();
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
std::pair<bool, bool> VPOParoptTransform::genCSALoop(WRegionNode *W) {
  assert(isTargetCSA() && "unexpected target");

  // Determine the number of workers to use for this loop.
  auto NumWorkers = getNumWorkers(W);

  // LoopOpt can do parallel lowering for loops with 1 worker so far.
  bool UseLoopOptLowering = UseOmpRegionsInLoopoptFlag && NumWorkers <= 1u;

  if (!UseLoopOptLowering && DoLoopSplitting)
    return { CSALoopSplitter(*this).run(W), true };

  auto *Loop = W->getWRNLoopInfo().getLoop();
  assert(Loop->isLoopSimplifyForm());

  // Generate privatization code.
  bool Changed = false;
  if (UseExpLoopPrivatizer)
    Changed |= CSAExpLoopPrivatizer(*this, W, NumWorkers <= 1u).run();
  else
    Changed |= CSALoopPrivatizer(*this, W, NumWorkers <= 1u).run();

  // We are done if LoopOpt takes care of OpenMP lowering.
  if (UseLoopOptLowering)
    return { Changed, false };

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
    Builder.CreateCall(Exit, { SpmdID }, {}, "");
  }

  // Insert parallel region entry/exit calls
  auto *Region = genParRegionCalls(getParRegionID(W),
                                   W->getEntryBBlock()->getTerminator(),
                                   W->getExitBBlock()->getFirstNonPHI());

  // and CSA parallel section entry/exit intrinsics
  genParSectionCalls(Region,
                     Loop->getHeader()->getFirstNonPHI(),
                     Loop->getLoopLatch()->getTerminator());
  return { true, true };
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

// Generates the following instruction sequence for intializing lock.
//
//   %0 = bitcast %arg to i64*
//   store atomic i64 0, i64* %0 seq_cst
//
static void genInitLock(Value *Arg, Instruction *InsPt) {
  auto AS = cast<PointerType>(Arg->getType())->getAddressSpace();
  const auto &DL = InsPt->getModule()->getDataLayout();
  auto PtrSize = DL.getPointerSizeInBits();
  auto *IntPtrTy = Type::getIntNPtrTy(InsPt->getContext(), PtrSize, AS);

  IRBuilder<> Builder(InsPt);
  auto *Lock = Builder.CreateBitCast(Arg, IntPtrTy);
  auto *Store = Builder.CreateStore(Builder.getIntN(PtrSize, 0), Lock);
  Store->setAlignment(DL.getABIIntegerTypeAlignment(PtrSize));
  Store->setAtomic(AtomicOrdering::SequentiallyConsistent);
}

// Generates the following instruction sequence for setting lock.
//
//   %0 = bitcast %Arg to i64*
//   br label %lock.loop
//
// lock.loop:
//   %1 = cmpxchg i64* %0, i64 0, i64 1 seq_cst seq_cst
//   %2 = extractvalue { i64, i1 } %1, 1
//   br i1 %2, label %loop.exit, label %lock.loop
//
// loop.exit:
//   InsPt
//
static void genSetLock(Value *Arg, Instruction *InsPt,
                       DominatorTree *DT, LoopInfo *LI) {
  auto AS = cast<PointerType>(Arg->getType())->getAddressSpace();
  auto PtrSize = InsPt->getModule()->getDataLayout().getPointerSizeInBits();
  auto *IntPtrTy = Type::getIntNPtrTy(InsPt->getContext(), PtrSize, AS);

  IRBuilder<> Builder(InsPt);
  auto *Lock = Builder.CreateBitCast(Arg, IntPtrTy);

  auto *LoopBB = SplitBlock(InsPt->getParent(), InsPt, DT, LI);
  LoopBB->setName("lock.loop");

  auto *ExitBB = SplitBlock(InsPt->getParent(), InsPt, DT, LI);
  ExitBB->setName("loop.exit");

  Builder.SetInsertPoint(LoopBB->getTerminator());
  auto *Xchg = Builder.CreateAtomicCmpXchg(Lock,
      Builder.getIntN(PtrSize, 0),
      Builder.getIntN(PtrSize, 1),
      AtomicOrdering::SequentiallyConsistent,
      AtomicOrdering::SequentiallyConsistent);

  auto *Res = Builder.CreateExtractValue(Xchg, { 1u });
  Builder.CreateCondBr(Res, ExitBB, LoopBB);
  LoopBB->getTerminator()->eraseFromParent();
}

// Unset lock. Instrustion sequence is the same as for lock initialization.
static void genUnsetLock(Value *Arg, Instruction *InsPt) {
  genInitLock(Arg, InsPt);
}

// Generates the following instruction sequence for testing lock.
//
//    %0 = bitcast %Arg to i64*
//    %1 = cmpxchg i64* %0, i64 0, i64 1 seq_cst seq_cst
//    %2 = extractvalue { i64, i1 } %1, 1
//    %3 = select i1 %2, i32 1, i32 0
//
static Value* genTestLock(Value *Arg, Instruction *InsPt) {
  auto AS = cast<PointerType>(Arg->getType())->getAddressSpace();
  auto PtrSize = InsPt->getModule()->getDataLayout().getPointerSizeInBits();
  auto *IntPtrTy = Type::getIntNPtrTy(InsPt->getContext(), PtrSize, AS);

  IRBuilder<> Builder(InsPt);
  auto *Lock = Builder.CreateBitCast(Arg, IntPtrTy);
  auto *Xchg = Builder.CreateAtomicCmpXchg(Lock,
      Builder.getIntN(PtrSize, 0),
      Builder.getIntN(PtrSize, 1),
      AtomicOrdering::SequentiallyConsistent,
      AtomicOrdering::SequentiallyConsistent);

  auto *Cond = Builder.CreateExtractValue(Xchg, { 1u });
  return Builder.CreateSelect(Cond, Builder.getInt32(1), Builder.getInt32(0));
}

bool VPOParoptTransform::translateCSAOmpRtlCalls() {
  assert(isTargetCSA() && "unexpected target");

  bool Changed = false;
  for (auto BI = F->begin(); BI != F->end(); ++BI) {
    for (auto II = BI->begin(); II != BI->end();) {
      CallSite CS(&*II++);
      if (!CS)
        continue;

      LibFunc LF = NumLibFuncs;
      if (!TLI->getLibFunc(CS, LF))
        continue;

      Value *Val = nullptr;
      switch (LF) {
        case LibFunc_omp_get_thread_num:
        case LibFunc_omp_get_nested:
          Val = ConstantInt::get(Type::getInt32Ty(F->getContext()), 0);
          break;
        case LibFunc_omp_get_num_threads:
        case LibFunc_omp_get_max_threads:
        case LibFunc_omp_get_dynamic:
          Val = ConstantInt::get(Type::getInt32Ty(F->getContext()), 1u);
          break;
        case LibFunc_omp_set_num_threads:
        case LibFunc_omp_set_dynamic:
        case LibFunc_omp_set_nested:
          break;
        case LibFunc_omp_init_lock:
          genInitLock(CS.getArgument(0), CS.getInstruction());
          break;
        case LibFunc_omp_set_lock:
          genSetLock(CS.getArgument(0), CS.getInstruction(), DT, LI);
          break;
        case LibFunc_omp_unset_lock:
          genUnsetLock(CS.getArgument(0), CS.getInstruction());
          break;
        case LibFunc_omp_test_lock:
          Val = genTestLock(CS.getArgument(0), CS.getInstruction());
          break;
        case LibFunc_omp_destroy_lock:
          break;
        default:
          continue;
      }

      Changed = true;
      if (Val)
        CS->replaceAllUsesWith(Val);
      if (auto *Invoke = dyn_cast<InvokeInst>(CS.getInstruction()))
        BranchInst::Create(Invoke->getNormalDest(), CS.getInstruction());
      BI = CS->getParent()->getIterator();
      II = CS->eraseFromParent();
    }
  }
  return Changed;
}

bool VPOParoptTransform::genCSACritical(WRNCriticalNode *W) {
  assert(isTargetCSA() && "unexpected target");

  auto Suffix = W->getUserLockName();
  if (Suffix.empty())
    Suffix = "common";

  SmallString<64u> LockName{"__omp.critical."};
  LockName += Suffix;

  auto *M = F->getParent();
  auto PtrSize = M->getDataLayout().getPointerSizeInBits();
  auto *LockTy = Type::getIntNTy(M->getContext(), PtrSize);

  auto *Lock = M->getGlobalVariable(LockName);
  if (!Lock)
    Lock = new GlobalVariable(*M, LockTy, false, GlobalValue::CommonLinkage,
                              ConstantInt::get(LockTy, 0), LockName);

  genSetLock(Lock, W->getEntryBBlock()->getTerminator(), DT, LI);
  genUnsetLock(Lock, W->getExitBBlock()->getFirstNonPHI());

  return true;
}

//===- HIRIdiomRecognition.cpp - Implements Loop idiom recognition pass ---===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// The pass mostly recognizes the memory operations like memset, memcpy,
// memmove. It replaces the stores and loads with the intrinsic calls in the
// pre-header.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Intrinsics.h"

#include "llvm/Pass.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLoopStatistics.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGrouping.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/ForEach.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#define OPT_SWITCH "hir-idiom"
#define OPT_DESC "HIR Loop Idiom Recognition"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

static cl::list<unsigned>
    TransformNodes(OPT_SWITCH "-nodes", cl::Hidden,
                   cl::desc("List nodes to transform by " OPT_DESC));

STATISTIC(NumMemSet, "Number of memset's formed from loop stores");
STATISTIC(NumMemCpy, "Number of memcpy's formed from loop load+stores");

namespace {

struct MemOpCandidate {
  HLInst *DefInst;

  const RegDDRef *StoreRef;
  RegDDRef *RHS;

  bool IsStoreNegStride;

  bool isMemset() const { return RHS->isTerminalRef(); }
  bool isMemcopy() const { return RHS->isMemRef(); }

  MemOpCandidate() {}

  MemOpCandidate(const RegDDRef *StoreRef) : StoreRef(StoreRef) {
    DefInst = cast<HLInst>(StoreRef->getHLDDNode());
    RHS = DefInst->getRvalDDRef();
  }
};

class HIRIdiomRecognition : public HIRTransformPass {
  const DataLayout *DL;
  Module *M;

  HIRFramework *HIR;
  HIRLoopStatistics *HLS;
  HIRDDAnalysis *DDA;

  // Track removed nodes to ignore obsolete dependencies.
  SmallPtrSet<HLNode *, 8> RemovedNodes;

  // Checks if the candidate is legal from DDG point of view.
  bool isLegalEdge(const RegDDRef *Ref, const DDEdge &E, unsigned Level,
                   bool IsStore);

  template <bool IsOutgoing>
  bool isLegalGraph(const DDGraph &DDG, const HLLoop *Loop, const RegDDRef *Ref,
                    bool IsStore);
  bool isLegalCandidate(const HLLoop *Loop, const MemOpCandidate &Candidate);

  // Analyze and create the transformation candidate for the reference.
  bool analyzeStore(HLLoop *Loop, const RegDDRef *Ref,
                    MemOpCandidate &Candidate);

  // Transform \p Candidates into memset calls
  bool processMemset(HLLoop *Loop, MemOpCandidate &Candidate);

  // Transform \p Candidates into memcpy calls
  bool processMemcpy(HLLoop *Loop, MemOpCandidate &Candidate);

  // Helper method to emit memset call.
  bool genMemset(HLLoop *Loop, MemOpCandidate &Candidate, int64_t StoreSize,
                 bool IsNegStride);

  // Returns size of the /p Ref in bytes.
  unsigned getRefSizeInBytes(const RegDDRef *Ref);

  // Returns true if the /p Ref is a unit-stride reference and outputs the IV
  // direction to the /p IsNegStride;
  bool isUnitStrideRef(const RegDDRef *Ref, const HLLoop *Loop,
                       bool &IsNegStride);

  // Check if the \p Ref could be represented by a repeating i8 type.
  // Update RegDDRef if \p DoBitcast is true.
  static bool isBytewiseValue(RegDDRef *Ref, bool DoBitcast);

  // Creates a ref represented a size of the store across \p Loop iterations.
  static RegDDRef *createSizeDDRef(HLLoop *Loop, int64_t StoreSize);

  // Update \p Ref to represent a leftmost memory location for the loop-variant
  // reference.
  bool makeStartRef(RegDDRef *Ref, HLLoop *Loop, bool IsNegStride);

  // Creates fake ddref for the memset/memcpy calls
  RegDDRef *createFakeDDRef(const RegDDRef *Ref, unsigned Level);

public:
  static char ID;

  HIRIdiomRecognition() : HIRTransformPass(ID) {
    initializeHIRIdiomRecognitionPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  bool runOnLoop(HLLoop *Loop);

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<HIRFramework>();
    AU.addRequiredTransitive<HIRLoopStatistics>();
    AU.addRequiredTransitive<HIRDDAnalysis>();
    AU.setPreservesAll();
  }
};
}

char HIRIdiomRecognition::ID = 0;
INITIALIZE_PASS_BEGIN(HIRIdiomRecognition, OPT_SWITCH, OPT_DESC, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatistics)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
INITIALIZE_PASS_END(HIRIdiomRecognition, OPT_SWITCH, OPT_DESC, false, false)

FunctionPass *llvm::createHIRIdiomRecognitionPass() {
  return new HIRIdiomRecognition();
}

/// If the specified terminal /p Ref can be set by repeating the same byte in
/// memory, return true. This can be done for all i8 values obviously, but is
/// also true for i32 0, i32 -1, i16 0xF0F0, double 0.0 etc. If the value can't
/// be handled with a repeated byte store (e.g. i16 0x1234), return false.
/// Also update the c0 value, Src and Dest types if /p DoBitcast is true.
bool HIRIdiomRecognition::isBytewiseValue(RegDDRef *Ref, bool DoBitcast) {
  if (Ref->getDestType()->isIntegerTy(8)) {
    // Do nothing as the destination type is already desired type.
    return true;
  }

  if (!Ref->isConstant()) {
    return false;
  }

  auto BitcastRef = [DoBitcast](APInt SplatValue, RegDDRef *Ref) -> bool {
    bool GoodToCast = SplatValue.isSplat(8);

    if (GoodToCast && DoBitcast) {
      CanonExpr *CE = Ref->getSingleCanonExpr();
      Type *I8TY = Type::getInt8Ty(Ref->getDDRefUtils().getContext());

      CE->clear();
      CE->setSrcAndDestType(I8TY);

      if (SplatValue.getBitWidth() > 8) {
        SplatValue = SplatValue.trunc(8);
      }

      CE->setConstant(SplatValue.getSExtValue());
    }

    return GoodToCast;
  };

  if (Ref->isNull()) {
    return BitcastRef(APInt(8, 0), Ref);
  }

  ConstantFP *FPC;
  if (Ref->isFPConstant(&FPC)) {
    return BitcastRef(FPC->getValueAPF().bitcastToAPInt(), Ref);
  }

  int64_t Value;
  if (Ref->isIntConstant(&Value)) {
    APInt APValue(Ref->getDestType()->getIntegerBitWidth(), Value);
    return BitcastRef(APValue, Ref);
  }

  // TODO: add vector constant support.

  return false;
}

// Check that it's legal to hoist store/load to the pre-header.
bool HIRIdiomRecognition::isLegalEdge(const RegDDRef *Ref, const DDEdge &E,
                                      unsigned Level, bool IsStore) {
  const DirectionVector &DV = E.getDV();

  if (DV.isIndepFromLevel(Level)) {
    return true;
  }

  // This is a workaround because of not precise DDG for multiple level
  // loopnests. We consider any * outer level DV illegal. Ex.: (* * ?).
  // Only outer "=" DVs are handled for multilevel loopnests. Ex.: (= = ?).
  // Note that we already checked for isIndepFromLevel() before.
  for (unsigned L = 0; L < Level - 1; ++L) {
    if (DV[L] != DVKind::EQ) {
      return false;
    }
  }

  // No stores should be before the idiom DDRef.
  if (IsStore && E.isOUTPUTdep()) {
    DVKind Kind = DV[Level - 1];
    return (Kind == DVKind::EQ || Kind == DVKind::LT) && E.getSrc() == Ref;
  }

  assert(!E.isINPUTdep() && "Input edges are not expected");

  // No loads should be before the idiom DDRef.
  // Legality table:
  //      | Store | Load  |
  // ANTI |   >   |   <   |
  // FLOW |   <   |   >   |
  DVKind Kind;
  if (E.isANTIdep() ^ !IsStore) {
    Kind = DVKind::GT;
  } else {
    Kind = DVKind::LT;
  }

  return DV[Level - 1] == Kind;
}

template <bool IsOutgoing>
bool HIRIdiomRecognition::isLegalGraph(const DDGraph &DDG, const HLLoop *Loop,
                                       const RegDDRef *Ref, bool IsStore) {
  unsigned Level = Loop->getNestingLevel();

  auto Range = IsOutgoing ? DDG.outgoing(Ref) : DDG.incoming(Ref);

  for (DDEdge *E : Range) {
    DDRef *OtherRef = IsOutgoing ? E->getSink() : E->getSrc();

    if (RemovedNodes.count(OtherRef->getHLDDNode())) {
      // No dependence if we removed the node
      continue;
    }

    if (!isLegalEdge(Ref, *E, Level, IsStore)) {
      DEBUG(E->dump());
      return false;
    }
  }

  return true;
}

bool HIRIdiomRecognition::isLegalCandidate(const HLLoop *Loop,
                                           const MemOpCandidate &Candidate) {
  DEBUG(dbgs() << "R: ");
  DDGraph DDG = DDA->getGraph(Loop);

  if (!isLegalGraph<true>(DDG, Loop, Candidate.StoreRef, true)) {
    return false;
  }

  if (!isLegalGraph<false>(DDG, Loop, Candidate.StoreRef, true)) {
    return false;
  }

  if (Candidate.isMemcopy()) {
    if (!isLegalGraph<true>(DDG, Loop, Candidate.RHS, false)) {
      return false;
    }

    if (!isLegalGraph<false>(DDG, Loop, Candidate.RHS, false)) {
      return false;
    }
  }

  DEBUG(dbgs() << "OK\n");

  return true;
}

bool HIRIdiomRecognition::isUnitStrideRef(const RegDDRef *Ref,
                                          const HLLoop *Loop,
                                          bool &IsNegStride) {
  const Type *IVType = Loop->getIVType();
  unsigned Level = Loop->getNestingLevel();
  auto IVSizeInBits = IVType->getPrimitiveSizeInBits();

  for (CanonExpr *CE : llvm::make_range(Ref->canon_begin(), Ref->canon_end())) {
    if (!CE->hasIV(Level)) {
      continue;
    }

    if (CE->getSrcType()->getPrimitiveSizeInBits() < IVSizeInBits ||
        CE->isTrunc()) {
      return false;
    }
  }

  int64_t Stride;
  
  if (!Ref->getConstStrideAtLevel(Loop->getNestingLevel(), &Stride)) {
    return false;
  }

  auto Size = getRefSizeInBytes(Ref);
  IsNegStride = Stride < 0;

  return Size == Stride || Size == -Stride;
}

bool HIRIdiomRecognition::analyzeStore(HLLoop *Loop, const RegDDRef *Ref,
                                       MemOpCandidate &Candidate) {
  Candidate = std::move(MemOpCandidate(Ref));

  bool ForMemset = false;
  bool ForMemcpy = false;

  assert(Ref->isMemRef() && Ref->isLval() &&
         "Only lval memory references are expected here");

  // Check that store stride is constant.
  if (!isUnitStrideRef(Ref, Loop, Candidate.IsStoreNegStride)) {
    return false;
  }

  RegDDRef *RHS = Candidate.RHS;

  if (RHS->isTerminalRef()) {
    const CanonExpr *CE = RHS->getSingleCanonExpr();
    if (CE->isNonLinear()) {
      // Could be legal for memcopy
      ForMemcpy = true;
    } else {
      // Could be legal for memset
      ForMemset = true;
    }
  } else {
    // Could be legal for memcopy
    ForMemcpy = true;
  }

  if (ForMemset) {
    // Check if the RHS could be represented as a i8 value.
    if (!isBytewiseValue(RHS, false)) {
      return false;
    }

    const CanonExpr *CE = RHS->getSingleCanonExpr();
    if (!CE->isInvariantAtLevel(Loop->getNestingLevel())) {
      return false;
    }

    return true;
  }

  if (ForMemcpy) {
    if (!RHS->isMemRef()) {
      DEBUG(dbgs() << "WARNING: Something like a[i] = %0 is found, this pass "
                      "depends on the TempCleanup pass. Check if the input HIR "
                      "is already clean.\n");
      return false;
    }

    if (RHS->isVolatile()) {
      return false;
    }

    bool IsNegStride;
    if (!isUnitStrideRef(RHS, Loop, IsNegStride)) {
      return false;
    }

    if (IsNegStride != Candidate.IsStoreNegStride) {
      return false;
    }

    return true;
  }

  DEBUG(dbgs() << "Unsupported candidate\n");
  return false;
}

bool HIRIdiomRecognition::makeStartRef(RegDDRef *Ref, HLLoop *Loop,
                                       bool IsNegStride) {
  bool BlobUpdateNeeded = false;

  // Update IV to zero or UB.
  for (CanonExpr *CE : llvm::make_range(Ref->canon_begin(), Ref->canon_end())) {
    unsigned Level = Loop->getNestingLevel();

    if (IsNegStride) {
      const CanonExpr *OrigUpperCE = Loop->getUpperCanonExpr();
      // Try to merge upper bound with CE
      if (!CanonExprUtils::mergeable(CE, OrigUpperCE, true) ||
          !CanonExprUtils::replaceIVByCanonExpr(CE, Level, OrigUpperCE, true)) {

        std::unique_ptr<CanonExpr> UpperCE(OrigUpperCE->clone());

        // If merge doesn't work - try to convert UB to blob.
        bool Ret = UpperCE->castStandAloneBlob(CE->getSrcType(), false);

        if (!Ret) {
          return false;
        }

        Ret = CanonExprUtils::replaceIVByCanonExpr(CE, Level, UpperCE.get(),
                                                   true);
        assert(Ret &&
               "Second replaceIVByCanonExpr() should always return true");
        (void)Ret;
      }

      BlobUpdateNeeded = true;
    } else {
      // Positive stride, loop normalized.
      CE->replaceIVByConstant(Level, 0);
    }
  }

  unsigned PreheaderLevel = Loop->getNestingLevel() - 1;

  if (BlobUpdateNeeded) {
    // Make blobs consistent because of upper bound substitution.
    const SmallVector<const RegDDRef *, 1> Aux = {Loop->getUpperDDRef()};
    Ref->makeConsistent(&Aux, PreheaderLevel);
  } else {
    // Update of blob definition level is still required as we move statements
    // to the pre-header.
    Ref->updateDefLevel(PreheaderLevel);
  }

  Ref->setAddressOf(true);

  // Set destination address (i8*)
  Ref->setBaseDestType(Type::getInt8PtrTy(
      HIR->getContext(), Ref->getBaseDestType()->getPointerAddressSpace()));

  return true;
}

RegDDRef *HIRIdiomRecognition::createFakeDDRef(const RegDDRef *Ref,
                                               unsigned Level) {
  // TODO: Reuse Ref for FakeDDRef as we will be removing it's HLNode.
  // If reuse the RemovedNodes should be also updated as the Ref will be
  // attached to the different HLNode.
  RegDDRef *FakeRef = Ref->clone();
  unsigned UndefIndex = InvalidBlobIndex;
  for (CanonExpr *CE :
       llvm::make_range(FakeRef->canon_begin(), FakeRef->canon_end())) {
    if (!CE->hasIV(Level)) {
      continue;
    }

    CE->removeIV(Level);

    CE->getBlobUtils().createUndefBlob(CE->getSrcType(), true, &UndefIndex);
    CE->addBlob(UndefIndex, 1, false);

    break;
  }

  assert(UndefIndex != InvalidBlobIndex && "There should be at least one IV");

  // Fake DDRef will be attached to the pre-header of the i*Level* loop.
  FakeRef->updateDefLevel(Level - 1);

  return FakeRef;
}

RegDDRef *HIRIdiomRecognition::createSizeDDRef(HLLoop *Loop,
                                               int64_t StoreSize) {
  unsigned PreheaderLevel = Loop->getNestingLevel() - 1;

  std::unique_ptr<RegDDRef> Size(Loop->getTripCountDDRef(PreheaderLevel));
  CanonExpr *SizeCE = Size->getSingleCanonExpr();
  if (!SizeCE->multiplyByConstant(StoreSize)) {
    return nullptr;
  }

  // Multiplication could convert self blob to a regular blob.
  const SmallVector<const RegDDRef *, 1> Aux = {Loop->getUpperDDRef()};
  Size->makeConsistent(&Aux, PreheaderLevel);

  return Size.release();
}

bool HIRIdiomRecognition::genMemset(HLLoop *Loop, MemOpCandidate &Candidate,
                                    int64_t StoreSize, bool IsNegStride) {
  HLNodeUtils &HNU = HIR->getHLNodeUtils();
  DDRefUtils &DDU = HIR->getDDRefUtils();

  std::unique_ptr<RegDDRef> Ref(Candidate.StoreRef->clone());
  if (!makeStartRef(Ref.get(), Loop, IsNegStride)) {
    return false;
  }

  RegDDRef *Size = createSizeDDRef(Loop, StoreSize);

  // Remove store
  RemovedNodes.insert(Candidate.DefInst);
  HLNodeUtils::remove(Candidate.DefInst);

  RegDDRef *RHS = Candidate.DefInst->removeRvalDDRef();
  assert(Candidate.RHS == RHS && "DefInst Rval DDRef is a Candidate RHS");

  bool Bitcast = isBytewiseValue(RHS, true);
  assert(Bitcast && "Non-regular candidate, all of them should be filtered");
  (void)Bitcast;

  // The i8 blob could be non linear at the pre-header level.
  RHS->updateDefLevel(Loop->getNestingLevel() - 1);

  RegDDRef *Align = DDU.createConstDDRef(Type::getInt32Ty(HIR->getContext()),
                                         Ref->getAlignment());
  RegDDRef *IsVolatile =
      DDU.createConstDDRef(Type::getInt1Ty(HIR->getContext()), 0);

  Type *Tys[] = {Ref->getDestType(), Size->getDestType()};
  Function *Memset = Intrinsic::getDeclaration(M, Intrinsic::memset, Tys);

  SmallVector<RegDDRef *, 5> Ops = {Ref.release(), RHS, Size, Align,
                                    IsVolatile};

  HLInst *MemsetInst = HNU.createCall(Memset, Ops);
  MemsetInst->addFakeLvalDDRef(
      createFakeDDRef(Candidate.StoreRef, Loop->getNestingLevel()));

  HNU.insertAsLastPreheaderNode(Loop, MemsetInst);

  DEBUG(dbgs() << "G: ");
  DEBUG(MemsetInst->dump());
  DEBUG(dbgs() << "\n");

  NumMemSet++;

  return true;
}

unsigned HIRIdiomRecognition::getRefSizeInBytes(const RegDDRef *Ref) {
  auto SizeInBits = DL->getTypeSizeInBits(Ref->getDestType());
  return (unsigned)SizeInBits >> 3;
}

bool HIRIdiomRecognition::processMemset(HLLoop *Loop,
                                        MemOpCandidate &Candidate) {

  unsigned StoreSize = getRefSizeInBytes(Candidate.RHS);

  if (genMemset(Loop, Candidate, StoreSize, Candidate.IsStoreNegStride)) {
    return true;
  }

  DEBUG(dbgs() << "Failed to generate memset for chain:\n");
  DEBUG(Candidate.StoreRef->dump());
  DEBUG(dbgs() << "\n");

  return false;
}

bool HIRIdiomRecognition::processMemcpy(HLLoop *Loop,
                                        MemOpCandidate &Candidate) {

  HLNodeUtils &HNU = HIR->getHLNodeUtils();
  DDRefUtils &DDU = HIR->getDDRefUtils();

  std::unique_ptr<RegDDRef> StoreRef(Candidate.StoreRef->clone());
  std::unique_ptr<RegDDRef> LoadRef(Candidate.RHS->clone());

  unsigned StoreSize = getRefSizeInBytes(StoreRef.get());

  if (!makeStartRef(StoreRef.get(), Loop, Candidate.IsStoreNegStride)) {
    return false;
  }

  if (!makeStartRef(LoadRef.get(), Loop, Candidate.IsStoreNegStride)) {
    return false;
  }

  RegDDRef *Size = createSizeDDRef(Loop, StoreSize);

  RegDDRef *Align = DDU.createConstDDRef(
      Type::getInt32Ty(HIR->getContext()),
      std::min(StoreRef->getAlignment(), LoadRef->getAlignment()));

  RegDDRef *IsVolatile =
      DDU.createConstDDRef(Type::getInt1Ty(HIR->getContext()), 0);

  Type *Tys[] = {StoreRef->getDestType(), LoadRef->getDestType(),
                 Size->getDestType()};

  Function *Memcpy = Intrinsic::getDeclaration(M, Intrinsic::memcpy, Tys);

  SmallVector<RegDDRef *, 5> Ops = {StoreRef.release(), LoadRef.release(), Size,
                                    Align, IsVolatile};

  HLInst *MemcpyInst = HNU.createCall(Memcpy, Ops);
  MemcpyInst->addFakeLvalDDRef(
      createFakeDDRef(Candidate.StoreRef, Loop->getNestingLevel()));
  MemcpyInst->addFakeRvalDDRef(
      createFakeDDRef(Candidate.RHS, Loop->getNestingLevel()));

  HNU.insertAsLastPreheaderNode(Loop, MemcpyInst);

  HLNodeUtils::remove(Candidate.DefInst);
  RemovedNodes.insert(Candidate.DefInst);

  DEBUG(dbgs() << "G: ");
  DEBUG(MemcpyInst->dump());
  DEBUG(dbgs() << "\n");

  NumMemCpy++;

  return true;
}

bool HIRIdiomRecognition::runOnLoop(HLLoop *Loop) {
  DEBUG(dbgs() << "Processing Loop: <" << Loop->getNumber() << ">\n");

  if (!Loop->isDo() || !Loop->isNormalized() || Loop->isSIMD()) {
    DEBUG(dbgs() << "Skipping - non-DO-Loop / non-Normalized / SIMD\n");
    return false;
  }

  DEBUG(dbgs() << "Loop DD graph:\n");
  DEBUG(DDA->getGraph(Loop).dump());
  DEBUG(dbgs() << "\n");

  // Check for EH context
  // HLS->getTotalLoopStatistics()

  SmallVector<MemOpCandidate, 16> Candidates;

  for (HLNode &Node :
       llvm::make_range(Loop->child_begin(), Loop->child_end())) {
    HLInst *Inst = dyn_cast<HLInst>(&Node);
    if (!Inst) {
      continue;
    }

    RegDDRef *Ref = Inst->getLvalDDRef();

    if (!Ref || !Ref->isMemRef() || Ref->isVolatile() ||
        !isa<StoreInst>(Inst->getLLVMInstruction())) {
      continue;
    }

    MemOpCandidate NewCandidate;

    if (!analyzeStore(Loop, Ref, NewCandidate)) {
      continue;
    }

    Candidates.push_back(NewCandidate);
  }

  bool Changed = false;
  for (MemOpCandidate &Candidate : Candidates) {
    DEBUG(dbgs() << "A: ");
    DEBUG(Candidate.DefInst->dump(true));
    DEBUG(dbgs() << "\n");

    if (!isLegalCandidate(Loop, Candidate)) {
      continue;
    }

    if (Candidate.isMemset()) {
      Changed = processMemset(Loop, Candidate) || Changed;
    } else if (Candidate.isMemcopy()) {
      Changed = processMemcpy(Loop, Candidate) || Changed;
    } else {
      llvm_unreachable("Unknown memopt kind");
    }
  }

  if (Changed) {
    // The transformation could hoist everything to the pre-header, making the
    // loop empty.
    HLNodeUtils::removeEmptyNodes(Loop, false);
  }

  return Changed;
}

bool HIRIdiomRecognition::runOnFunction(Function &F) {
  if (skipFunction(F) || DisablePass) {
    return false;
  }

  DEBUG(dbgs() << OPT_DESC " for Function: " << F.getName() << "\n");

  M = F.getParent();
  DL = &M->getDataLayout();

  HIR = &getAnalysis<HIRFramework>();
  HLS = &getAnalysis<HIRLoopStatistics>();
  DDA = &getAnalysis<HIRDDAnalysis>();

  SmallPtrSet<HLNode *, 8> NodesToInvalidate;

  ForPostEach<HLLoop>::visitRange(
      HIR->hir_begin(), HIR->hir_end(),
      [&NodesToInvalidate, this](HLLoop *Loop) {

        if (!TransformNodes.empty()) {
          if (std::find(TransformNodes.begin(), TransformNodes.end(),
                        Loop->getNumber()) == TransformNodes.end()) {
            DEBUG(dbgs() << "Skipped due to the command line option\n");
            return;
          }
        }

        HLRegion *ParentRegion = Loop->getParentRegion();
        HLLoop *ParentLoop = Loop->getParentLoop();

        if (this->runOnLoop(Loop)) {
          ParentRegion->setGenCode();

          // Check if Loop was removed during
          // transformation
          if (Loop->isAttached()) {
            NodesToInvalidate.insert(Loop);
          }

          if (ParentLoop) {
            NodesToInvalidate.insert(ParentLoop);
          } else {
            NodesToInvalidate.insert(ParentRegion);
          }
        }
      });

  for (HLNode *Node : NodesToInvalidate) {
    if (HLRegion *Region = dyn_cast<HLRegion>(Node)) {
      HIRInvalidationUtils::invalidateNonLoopRegion(Region);
    } else {
      HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(
          cast<HLLoop>(Node));
    }
  }

  DEBUG(dbgs() << "\n");
  return false;
}

void HIRIdiomRecognition::releaseMemory() { RemovedNodes.clear(); }

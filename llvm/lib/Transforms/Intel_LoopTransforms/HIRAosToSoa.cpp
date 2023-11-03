//===- HIRAosToSoa.cpp Implements transformation similar to AOS to SOA -===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// Changes references to array of structs into references to arrays.
//
// e.g.) Original loop:
//
// for (i2 = 0; i2 < N; i2++)
//  for (i3 = 0; i3 < M; i3++)
//   for (i4 = 0; i4 <L; i4++)  {
//      %t0 = uitofp.i16.fp64(s[i2 + %alpha * i3 + i4].0);
//      %t1 = uitofp.i16.fp64(s[i2 + %alpha * i3 + i4].1);
//      %t2 = uitofp.i16.fp64(s[i2 + %alpha * i3 + i4].2);
//   }
//
// To -->
//
// %p = stacksave();
// %add = N + L;
// array0 = alloca f64*  (%add * M);
// array1 = alloca f64*  (%add * M);
// array2 = alloca f64*  (%add * M);
//
// …
// // Copy loop
// for (i2 = 0; i2 <M; i2++)
//   for (i3 = 0; i3 < %add; i3++) {
//         array0[%add * i2 + i3] = uitofp.i16.fp64(s[%alpha * i2 + i3].0);
//         array1[%add * i2 + i3] = uitofp.i16.fp64(s[%alpha * i2 + i3].1);
//         array2[%add * i2 + i3] = uitofp.i16.fp64(s[%alpha * i2 + i3].2);
//   }
// …
// // Main computation loop
// for (i2 = 0; i2 < N; i2++)
//  for (i3 = 0; i3 < M; i3++)
//   for (i4 = 0; i4 < L; i4++)  {
//      %t0 = array0[i2 + %add * i3 + i4];
//      %t1 = array1[i2 + %add * i3 + i4];
//      %t2 = array2[i2 + %add * i3 + i4];
//   }
// ..
// stackrestore(%p);
//
//
// Performance improvement is expected from unit-strided accesses in the main
// loop for vectorization and hoisting out data conversion (e.g. ui to fp) into
// copy loop. In many cases, M*(N + L) < M * N * L.
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRAosToSoaPass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeIterator.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

#define OPT_SWITCH "hir-aos-to-soa"
#define OPT_DESC "HIR AOS to SOA"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

namespace {
const int DefaultNumOfTrailingOffsets = 3;
const int DefaultInnermostLoopNestsDepth = 3;
} // namespace

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

static cl::opt<int> NumberOfTrailingOffsets(
    OPT_SWITCH "-num-trailing-offsets", cl::init(DefaultNumOfTrailingOffsets),
    cl::Hidden, cl::desc("Number of trailing offsets " OPT_DESC " handles"));

static cl::opt<bool>
    AllocConverted(OPT_SWITCH "-alloc-converted", cl::init(false), cl::Hidden,
                   cl::desc(OPT_DESC " allocated converted value"));

namespace {

typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;

class HIRAosToSoa {

private:
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;

  class Analyzer {
  public:
    Analyzer(HLLoop *InnermostLoop, HIRDDAnalysis &DDA,
             unsigned NumOfTrailingOffsets, unsigned InnermostLoopNestsDepth)
        : InnermostLoop(InnermostLoop), DDA(DDA),
          NumOfTrailingOffsets(NumOfTrailingOffsets),
          InnermostLoopNestsDepth(InnermostLoopNestsDepth), IVBlobLevel(0) {}

    /// Legality + Profitablity
    /// Once this function passes true, transformation occurs.
    bool isCandidate();

  private:
    /// Collect loops from Innermost to its ancestors.
    /// Total number of the loops collected should be
    /// InnermostLoopNestsDepth.
    void collectLoopsInNest();

    /// Returns true if any loop in nests have complex loop bound
    bool anyComplexLoopBound(unsigned Level) const;

    /// Are all memrefs in the loopnests are readonly
    bool areAllMemRefsReadOnly(DDGraph DDG,
                               const ArrayRef<RegDDRef *> &MemRefs) const;

    /// See if memrefs are all
    //  - has only one dimension
    //  - has one trailing offset at most
    bool checkMemRefsShape(const SmallVectorImpl<RegDDRef *> &Refs) const;

    /// See ref's subscripts are in a certain form, and all of them are the
    /// same. All IV constant coefficients are either 0 or 1. Only one IV has
    /// BlobCoefficient.
    bool checkSubscripts(unsigned AllocaLevel,
                         const SmallVectorImpl<RegDDRef *> &RefsToCopy);

    static bool hasCommonInvariantCast(const SmallVectorImpl<RegDDRef *> &Refs);

    // Quite specific now. Can be extended further if a need arises.
    static bool isACast(const HLInst *HInst) {
      return isa<UIToFPInst>(HInst->getLLVMInstruction());
    }

    /// Examine in the given canonical expression
    /// if all induction variables'
    /// coefficients are one except for
    /// one induction variable, which has a
    /// blob IV coefficient and a constant coeff 1.
    /// More specifically,
    /// im + i(m+1) + i(m+2) + ... + BlobCoeff *i(n) + i(n+1)...
    /// At least one IV with coeff 1 and exactly one IV with IVBlobCoeff
    /// should be found.
    //  TODO: Due to hasAllIVsInLoopNests(), hasOneIVBlobAndAllOneCoeffs's logic
    //  can be simplified. But left it as-is for potential exention of
    //  logic to do without hasAllIVsInLoopNests().
    static bool hasOneIVBlobAndAllOneCoeffs(const CanonExpr *CE,
                                            unsigned &IVBlobLevel,
                                            unsigned &MinIVLevel);

    /// Check src and dest types of CE and its denominator
    bool isValidIndexCE(const CanonExpr *IndexCE) const;

    // See if CE has all IVs in levels [InnermostLoopLevel - LoopNestsDepth + 1,
    // InnermostLoopLevel]
    bool hasAllIVsInLoopNests(const CanonExpr *CE) const;

  private:
    HLLoop *InnermostLoop;
    HIRDDAnalysis &DDA;

    // Number of distinct trailing offsets we are looking in a candidate loop
    // body e.g.)
    //      s[i2 + %alpha * i3 + i4].0;
    //      s[i2 + %alpha * i3 + i4].1;
    //      s[i2 + %alpha * i3 + i4].2;
    // three trailing offsets .0, .1 and .2.
    unsigned NumOfTrailingOffsets;

    // Depth of the candidate loop nest. For the example in the top of this
    // file, InnermostLoopNestDepth = 3, because the loop nest in our interest
    // is i2, i3 and i4. Before loop at i2, alloca and copy loops will be
    // inserted.
    unsigned InnermostLoopNestsDepth;

  public:
    // Results of analysis to be exposed outside
    unsigned IVBlobLevel;

    // Captures the original computation loop nest
    SmallVector<HLLoop *, 3> LoopNests;

    // Refs to replace in the original loop
    SmallVector<RegDDRef *, 16> RefsToCopy;
  };

  class TransformAosToSoa {
  public:
    TransformAosToSoa(HLLoop *Loop, const SmallVectorImpl<HLLoop *> &LoopNests,
                      SmallVectorImpl<RegDDRef *> &RefsToCopy,
                      unsigned IVBlobLevel)
        : Loop(Loop), LoopNests(LoopNests), RefsToCopy(RefsToCopy),
          IVBlobLevel(IVBlobLevel),
          AllocaLevel(Loop->getNestingLevel() - DefaultInnermostLoopNestsDepth +
                      1),
          Anchor(Loop->getParentLoopAtLevel(AllocaLevel)),
          IVType(Loop->getIVType()), HNU(Loop->getHLNodeUtils()),
          DDRU(Loop->getDDRefUtils()), CEU(Loop->getCanonExprUtils()) {

      // Invalidation before transformation
      HIRInvalidationUtils::invalidateLoopNestBody(Anchor);
      HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(Anchor);
    }

    void rewrite();

  private:
    HLInst *insertCallToStacksave();
    void insertCallToStackrestore(RegDDRef *StackAddrRef);

    /// Create temp arrays for each trailing offset using Alloca.
    /// Product of two function arguments will be the size of each alloca.
    /// e.g.) %t = alloca DestTy MulOprd1 * MulOprd2
    /// The generated allocas are store into TrailingOffsetToAlloca.
    void
    insertAllocas(RegDDRef *MulOprd1, RegDDRef *MulOprd2,
                  SmallDenseMap<unsigned, HLInst *, 4> &TrailingOffsetToAlloca);

    /// This transformation generates 2-level copy loops as shown in the top of
    /// of this file. Outer loop (called CopyOuterLoop) has the same trip count
    /// of the original loop, whose level has IVBlobCoeff in the original
    /// RegDDRefs. Inner loop (called CopyInnerLoop) will have the trip count,
    /// which is the the sum of all trip counts of loops, whose IVs have
    /// coeff 1. In the example in the top of this file, CopyOuterLoop's TC will
    /// be M, and CopyInnerLoop's TC will be (N + L) This function calculates
    /// the trip count of the CopyInnerLoop.
    RegDDRef *calcCopyInnerLoopTripCount(RegDDRef *&AuxRef);

    /// Clone from the existing loop at IVBlobLevel
    /// The resulting loop will be used as a outer loop of the loop nest
    /// copying into "alloca"ed array.
    HLLoop *getCopyOuterLoop();

    /// Insert loops for copying as
    /// CopyOuterLoop
    ///  CopyInnerLoop
    /// Notice that loop body of the inner loop is not populated.
    HLLoop *insertCopyLoops(HLLoop *CopyOuterLoop, RegDDRef *CopyInnerLoopTC,
                            RegDDRef *AuxRef);

    /// Create ZTTs for copy loops.
    void createZttForCopyOuterLoop(HLLoop *CopyOuterLoop) const;

    /// Replace MemRefs having trailing offsets with loads from allocas
    void replaceTrailingOffsetWithAlloca(
        const SmallDenseMap<unsigned, HLInst *, 4> &TrailingOffsetToAlloca,
        unsigned AddBlobIndex, const RegDDRef *AuxRef);

    /// Insert store from original MemRefs having trailing offsets to
    /// alloca'ed arrays.
    void populatedBodyOfCopyLoop(
        HLLoop *CopyInnerLoop,
        const SmallDenseMap<unsigned, HLInst *, 4> &TrailingOffsetToAlloca,
        unsigned AddBlobIndex, const RegDDRef *AuxRef);

  private:
    HLLoop *Loop;
    const SmallVectorImpl<HLLoop *> &LoopNests;
    const SmallVectorImpl<RegDDRef *> &RefsToCopy;
    // Maps blob index of created alloca to the symbase used for creating
    // memrefs based on the alloca.
    SmallDenseMap<unsigned, unsigned> AllocaToMemRefSB;
    unsigned IVBlobLevel;

    unsigned AllocaLevel; // Level where the alloca will be populated.

    HLLoop *Anchor;
    Type *IVType;

    HLNodeUtils &HNU;
    DDRefUtils &DDRU;
    CanonExprUtils &CEU;
  };

private:
  static unsigned getTrailingOffset(const RegDDRef *Ref) {
    ArrayRef<unsigned> TrailingOffsets = Ref->getTrailingStructOffsets(1);
    return TrailingOffsets[0];
  }

  static CanonExpr *getIndexCE(RegDDRef *Ref) {
    return Ref->getDimensionIndex(1);
  }

public:
  HIRAosToSoa(HIRFramework &HIRF, HIRDDAnalysis &DDA) : HIRF(HIRF), DDA(DDA) {}

  bool run();
};

} // namespace

bool HIRAosToSoa::Analyzer::areAllMemRefsReadOnly(
    DDGraph DDG, const ArrayRef<RegDDRef *> &MemRefs) const {

  for (auto *Ref : MemRefs) {
    if (!Ref->isRval())
      return false;
    if (std::distance(DDG.incoming_edges_begin(Ref),
                      DDG.incoming_edges_end(Ref)))
      return false;
    if (std::distance(DDG.outgoing_edges_begin(Ref),
                      DDG.outgoing_edges_end(Ref)))
      return false;
  }

  return true;
}

bool HIRAosToSoa::Analyzer::checkMemRefsShape(
    const SmallVectorImpl<RegDDRef *> &Refs) const {
  std::set<unsigned> DistinctOffsets;

  for (auto *Ref : Refs) {
    if (Ref->getNumDimensions() > 1)
      return false;

    ArrayRef<unsigned> Offsets = Ref->getTrailingStructOffsets(1);
    if (Offsets.size() > 1)
      return false;

    DistinctOffsets.insert(Offsets[0]);
  }

  unsigned Size = DistinctOffsets.size();
  if (Size == 0)
    return false;

  if (Size != NumOfTrailingOffsets)
    return false;

  return true;
}

bool HIRAosToSoa::Analyzer::hasAllIVsInLoopNests(const CanonExpr *CE) const {
  for (unsigned
           I = InnermostLoop->getNestingLevel() - InnermostLoopNestsDepth + 1,
           E = InnermostLoop->getNestingLevel();
       I <= E; I++) {
    if (!CE->hasIV(I))
      return false;
  }

  return true;
}

bool HIRAosToSoa::Analyzer::hasOneIVBlobAndAllOneCoeffs(const CanonExpr *CE,
                                                        unsigned &IVBlobLevel,
                                                        unsigned &MinIVLevel) {
  if (CE->numBlobs() > 0)
    return false;

  unsigned Min = MaxLoopNestLevel + 1;
  bool FoundOneCoeff = false;
  bool FoundIVBlobCoeff = false;
  for (unsigned I = 1; I <= MaxLoopNestLevel; I++) {
    unsigned Index = InvalidBlobIndex;
    int64_t Coeff = 0;
    CE->getIVCoeff(I, &Index, &Coeff);

    if (Coeff == 0) {
      continue;
    }

    if (Coeff != 1) {
      return false;
    }

    if (Index != InvalidBlobIndex) {
      if (FoundIVBlobCoeff) {
        return false;
      }

      IVBlobLevel = I;
      FoundIVBlobCoeff = true;
    } else {
      FoundOneCoeff = true;
    }

    if (I < Min)
      Min = I;
  }

  MinIVLevel = Min;
  return FoundIVBlobCoeff && FoundOneCoeff;
}

bool HIRAosToSoa::Analyzer::isValidIndexCE(const CanonExpr *IndexCE) const {
  Type *IVType = (*LoopNests.begin())->getIVType();

  return IndexCE->getSrcType() == IVType && IndexCE->getDestType() == IVType &&
         IndexCE->getDenominator() == 1;
}

bool HIRAosToSoa::Analyzer::checkSubscripts(
    unsigned AllocaLevel, const SmallVectorImpl<RegDDRef *> &RefsToCopy) {

  for (auto *Ref : RefsToCopy) {
    if (!Ref->isLinearAtLevel(AllocaLevel))
      return false;
  }

  unsigned MinIVLevel = 0;
  const CanonExpr *FirstIndexCE = HIRAosToSoa::getIndexCE(*RefsToCopy.begin());
  if (!isValidIndexCE(FirstIndexCE))
    return false;

  if (!hasAllIVsInLoopNests(FirstIndexCE))
    return false;

  if (!hasOneIVBlobAndAllOneCoeffs(FirstIndexCE, IVBlobLevel, MinIVLevel))
    return false;

  // Outermost Loop's level is the same as AllocaLevel
  if (MinIVLevel < AllocaLevel)
    return false;

  const CanonExpr *FirstBaseCE = (*RefsToCopy.begin())->getBaseCE();
  for (auto *Ref :
       make_range(std::next(RefsToCopy.begin()), RefsToCopy.end())) {

    if (!CanonExprUtils::areEqual(FirstBaseCE, Ref->getBaseCE()))
      return false;

    if (!CanonExprUtils::areEqual(FirstIndexCE, HIRAosToSoa::getIndexCE(Ref)))
      return false;
  }

  return true;
}

bool HIRAosToSoa::Analyzer::hasCommonInvariantCast(
    const SmallVectorImpl<RegDDRef *> &Refs) {
  for (auto *Ref : Refs) {
    if (!isACast(cast<HLInst>(Ref->getHLDDNode())))
      return false;
  }
  return true;
}

void HIRAosToSoa::Analyzer::collectLoopsInNest() {

  unsigned Level = InnermostLoop->getNestingLevel();

  for (HLLoop *Lp = InnermostLoop;
       Lp && Lp->getNestingLevel() > Level - InnermostLoopNestsDepth;
       Lp = Lp->getParentLoop()) {
    LoopNests.push_back(Lp);
  }

  // The assertion condition is already guaranteed by previous
  // checks of the caller and the fact DefaultInnermostLoopNestsDepth is
  // currently set to non-zero.
  // This assertion is helping explicitly stating prerequites
  // for later analysis/transformation.
  assert(InnermostLoopNestsDepth &&
         (LoopNests.size() == InnermostLoopNestsDepth));
}

bool HIRAosToSoa::Analyzer::anyComplexLoopBound(unsigned Level) const {
  Type *IVType = (*LoopNests.begin())->getIVType();
  for (auto *Loop : LoopNests) {
    if (!Loop->isNormalized() || !Loop->isDo() || Loop->getIVType() != IVType ||
        Loop->getUpperCanonExpr()->numBlobs() > 1 ||
        Loop->getUpperCanonExpr()->hasIV() ||
        !Loop->getUpperDDRef()->isLinearAtLevel(Level))
      return true;
  }
  return false;
}

bool HIRAosToSoa::Analyzer::isCandidate() {

  if (!InnermostLoop->isInnermost() ||
      InnermostLoop->getNestingLevel() < InnermostLoopNestsDepth)
    return false;

  collectLoopsInNest();

  unsigned AllocaLevel =
      InnermostLoop->getNestingLevel() - InnermostLoopNestsDepth + 1;

  // Bail out if loop bounds look complicated.
  if (anyComplexLoopBound(AllocaLevel))
    return false;

  // Any non HLInst in the loop body.
  if (std::any_of(InnermostLoop->child_begin(), InnermostLoop->child_end(),
                  [](const HLNode &Node) { return !isa<HLInst>(&Node); }))
    return false;

  MemRefGatherer::VectorTy MemRefs;
  MemRefGatherer::gatherRange(InnermostLoop->child_begin(),
                              InnermostLoop->child_end(), MemRefs);

  std::copy_if(
      MemRefs.begin(), MemRefs.end(), std::back_inserter(RefsToCopy),
      [](const RegDDRef *Ref) { return Ref->hasTrailingStructOffsets(); });

  if (RefsToCopy.size() == 0)
    return false;

  // LoopNests.back()'s level is the level where a new copy
  // loop will be added. Thus, the level, the read refs
  // will be hoisted to be copied.
  DDGraph DDG = DDA.getGraph(LoopNests.back());
  if (!areAllMemRefsReadOnly(DDG, RefsToCopy))
    return false;

  if (!checkMemRefsShape(RefsToCopy))
    return false;

  if (!hasCommonInvariantCast(RefsToCopy))
    return false;

  if (!checkSubscripts(AllocaLevel, RefsToCopy))
    return false;

  return true;
}

void HIRAosToSoa::TransformAosToSoa::populatedBodyOfCopyLoop(
    HLLoop *CopyInnerLoop,
    const SmallDenseMap<unsigned, HLInst *, 4> &TrailingOffsetToAlloca,
    unsigned AddBlobIndex, const RegDDRef *AuxRef) {

  SmallSet<unsigned, 4> CopiedOffset;
  unsigned InnerLevel = CopyInnerLoop->getNestingLevel();

  for (auto *Ref : RefsToCopy) {
    // Make sure only one copy for a trailing offset to an alloca happen.
    unsigned TrailingOffset = getTrailingOffset(Ref);
    if (CopiedOffset.count(TrailingOffset))
      continue;
    CopiedOffset.insert(TrailingOffset);

    RegDDRef *NewRef = Ref;
    HLInst *SrcInst = nullptr;
    if (AllocConverted) {
      // Reuse SrcInst.
      // SrcInst is already in the form of
      //   %t = uitofp(s[i2 + %alpha * i3 + i4].0)
      // Replace its operand to make it
      //   %newLvalTmp = uitofp(s[%alpha * i2 + i3].0);
      SrcInst = cast<HLInst>(NewRef->getHLDDNode());
      RegDDRef *NewLval =
          HNU.createTemp(SrcInst->getLvalDDRef()->getDestType(), "tmp");
      SrcInst->replaceOperandDDRef(SrcInst->getLvalDDRef(), NewLval);
      HNU.insertAsLastChild(CopyInnerLoop, SrcInst);
    } else {
      // Copy should be done without cast, which is mainly a load
      NewRef = Ref->clone();
    }

    CanonExpr *CE = HIRAosToSoa::getIndexCE(NewRef);
    unsigned OrigBlobIndex = CE->getIVBlobCoeff(IVBlobLevel);
    CE->clear();
    CE->setIVCoeff(InnerLevel, InvalidBlobIndex, 1);
    CE->setIVCoeff(InnerLevel - 1, OrigBlobIndex, 1);

    // Create Lval of store, alloca0[%add * i2  + i3]
    HLInst *Alloca = TrailingOffsetToAlloca.find(TrailingOffset)->second;

    unsigned AllocaIndex = Alloca->getLvalDDRef()->getSelfBlobIndex();
    assert(AllocaToMemRefSB.find(AllocaIndex) != AllocaToMemRefSB.end() &&
           "Did not find alloca's symbase");
    RegDDRef *AllocaRef = DDRU.createMemRef(
        cast<AllocaInst>(Alloca->getLLVMInstruction())->getAllocatedType(),
        AllocaIndex, Alloca->getNodeLevel(), AllocaToMemRefSB[AllocaIndex]);

    CanonExpr *NewDimension = CEU.createCanonExpr(IVType);
    NewDimension->setIVCoeff(InnerLevel, InvalidBlobIndex, 1);
    NewDimension->setIVCoeff(InnerLevel - 1, AddBlobIndex, 1);
    AllocaRef->addDimension(NewDimension);

    // Create Store to Alloca
    //   alloca0[%add * i2  + i3] = %newLvalTmp
    RegDDRef *ToStore =
        AllocConverted ? SrcInst->getLvalDDRef()->clone() : NewRef;
    HLInst *StoreInst = HNU.createStore(ToStore, "store", AllocaRef);
    HNU.insertAsLastChild(CopyInnerLoop, StoreInst);
    AllocaRef->makeConsistent(AuxRef);
  }
}

void HIRAosToSoa::TransformAosToSoa::replaceTrailingOffsetWithAlloca(
    const SmallDenseMap<unsigned, HLInst *, 4> &TrailingOffsetToAlloca,
    unsigned AddBlobIndex, const RegDDRef *AuxRef) {

  // Replace uitofp(s[i2 + %alpha * i3 + i4].0) with
  // alloca0[i2 + %add * i3 + i4]

  for (auto *Ref : RefsToCopy) {
    HLDDNode *Src = Ref->getHLDDNode();

    HLInst *Alloca =
        TrailingOffsetToAlloca.find(getTrailingOffset(Ref))->second;

    unsigned AllocaIndex = Alloca->getLvalDDRef()->getSelfBlobIndex();
    assert(AllocaToMemRefSB.find(AllocaIndex) != AllocaToMemRefSB.end() &&
           "Did not find alloca's symbase");
    RegDDRef *AllocaRef = DDRU.createMemRef(
        cast<AllocaInst>(Alloca->getLLVMInstruction())->getAllocatedType(),
        AllocaIndex, Alloca->getNodeLevel(), AllocaToMemRefSB[AllocaIndex]);

    CanonExpr *NewDimension = HIRAosToSoa::getIndexCE(Ref)->clone();
    NewDimension->setIVBlobCoeff(IVBlobLevel, AddBlobIndex);
    AllocaRef->addDimension(NewDimension);

    if (AllocConverted) {
      HLInst *Load =
          HNU.createLoad(AllocaRef, ".read.temp", Src->getLvalDDRef()->clone());

      // By default, replace only unlink the replaced node, not destroying it.
      HNU.replace(Src, Load);
    } else {
      // Reuse src
      HLInst *SrcInst = cast<HLInst>(Src);
      SrcInst->replaceOperandDDRef(Ref, AllocaRef);
    }
    AllocaRef->makeConsistent(AuxRef);
  }
}

HLInst *HIRAosToSoa::TransformAosToSoa::insertCallToStacksave() {
  const DebugLoc &DLoc = Loop->getDebugLoc();
  HLInst *StacksaveCall = HNU.createStacksave(DLoc);
  HLNodeUtils::insertBefore(Anchor, StacksaveCall);

  return StacksaveCall;
}

void HIRAosToSoa::TransformAosToSoa::insertCallToStackrestore(
    RegDDRef *StackAddrRef) {
  auto Int8Ty = Type::getInt8Ty(HNU.getContext());

  StackAddrRef = DDRU.createAddressOfRef(Int8Ty, StackAddrRef->getSelfBlobIndex(),
                                         StackAddrRef->getDefinedAtLevel(),
                                         StackAddrRef->getSymbase(), true);

  StackAddrRef->addDimension(CEU.createCanonExpr(
      Int8Ty, APInt(8, 0)));

  HLInst *StackrestoreCall = HNU.createStackrestore(StackAddrRef);
  HLNodeUtils::insertAfter(Anchor, StackrestoreCall);
}

HLLoop *HIRAosToSoa::TransformAosToSoa::getCopyOuterLoop() {
  // Get the loop at IVBlobLevel.
  // LoopNests contains original loops starting from the innermost loop.

  return (*std::next(LoopNests.begin(),
                     (*LoopNests.begin())->getNestingLevel() - IVBlobLevel))
      ->cloneEmpty();
}

RegDDRef *
HIRAosToSoa::TransformAosToSoa::calcCopyInnerLoopTripCount(RegDDRef *&AuxRef) {

  RegDDRef *CopyInnerLoopTC = nullptr;
  bool FoundFirstCoeffOne = false;
  for (auto *Lp : LoopNests) {
    if (Lp->getNestingLevel() == IVBlobLevel) {

      continue;

    } else if (!FoundFirstCoeffOne) {
      FoundFirstCoeffOne = true;
      CopyInnerLoopTC = Lp->getTripCountDDRef(AllocaLevel + 1);

      continue;
    }

    RegDDRef *AddOperand = Lp->getTripCountDDRef();
    HLInst *AddInst = HNU.createAdd(CopyInnerLoopTC, AddOperand);
    HLNodeUtils::insertBefore(Anchor, AddInst);
    CopyInnerLoopTC->makeConsistent();
    CopyInnerLoopTC = AddInst->getLvalDDRef()->clone();
    AuxRef = AddInst->getLvalDDRef();
  }

  return CopyInnerLoopTC;
}

namespace {
void collectAllocaSymbase(
    const SmallDenseMap<unsigned, HLInst *, 4> &TrailingOffsetToAlloca,
    SmallVectorImpl<unsigned> &AllocaSymbase) {

  assert(!TrailingOffsetToAlloca.empty());

  for (auto &Pair : TrailingOffsetToAlloca) {
    const HLInst *Alloca = Pair.second;
    unsigned BlobSB = Alloca->getLvalDDRef()->getSymbase();
    AllocaSymbase.push_back(BlobSB);
  }
}
} // namespace

void HIRAosToSoa::TransformAosToSoa::insertAllocas(
    RegDDRef *MulOprd1, RegDDRef *MulOprd2,
    SmallDenseMap<unsigned, HLInst *, 4> &TrailingOffsetToAlloca) {

  // Create Muls for array size of alloca
  // Two operands are TCs of CopyInner/Outer Loops.
  HLInst *ArraySizeMulInst = HNU.createMul(MulOprd2, MulOprd1, "array_size");
  HLNodeUtils::insertBefore(Anchor, ArraySizeMulInst);
  MulOprd2->makeConsistent();
  MulOprd1->makeConsistent();

  // Create temp arrays for each trailing offset using Alloca
  Type *DestTy = AllocConverted
                     ? cast<HLInst>((*RefsToCopy.begin())->getHLDDNode())
                           ->getLLVMInstruction()
                           ->getType()
                     : (*RefsToCopy.begin())->getDestType();

  for (auto *Ref : RefsToCopy) {
    unsigned Offset = HIRAosToSoa::getTrailingOffset(Ref);
    if (TrailingOffsetToAlloca.count(Offset))
      continue;

    HLInst *Alloca =
        HNU.createAlloca(DestTy, ArraySizeMulInst->getLvalDDRef()->clone());
    TrailingOffsetToAlloca[Offset] = Alloca;
    HNU.insertBefore(Anchor, Alloca);

    AllocaToMemRefSB[Alloca->getLvalDDRef()->getSelfBlobIndex()] =
        Ref->getDDRefUtils().getNewSymbase();
  }
}

void HIRAosToSoa::TransformAosToSoa::createZttForCopyOuterLoop(
    HLLoop *CopyOuterLoop) const {
  // Skip the original outermost loop because the loop's preheader
  // is already extracted.
  SmallVector<PredicateTuple, 2> ZTTs;
  for (auto *Lp : make_range(LoopNests.begin(), std::prev(LoopNests.end()))) {
    // CopyOuterLoop's ztt is added by mergeZtt() by anyway.
    // Does not need to clone here explicitly.
    if (Lp->getNestingLevel() == IVBlobLevel)
      continue;

    // Now a loop's UB will be used in CopyInnerLoop
    HIRTransformUtils::cloneOrRemoveZttPredicates(Lp, ZTTs, true);
  }

  HIRTransformUtils::mergeZtt(CopyOuterLoop, ZTTs);
}

HLLoop *HIRAosToSoa::TransformAosToSoa::insertCopyLoops(
    HLLoop *CopyOuterLoop, RegDDRef *CopyInnerLoopTC, RegDDRef *AuxRef) {

  // Create Copy InnerLoop
  RegDDRef *CopyInnerLoopUB = CopyInnerLoopTC->clone();
  CopyInnerLoopUB->getSingleCanonExpr()->addConstant(-1, true);
  Type *IVType = Loop->getIVType();
  HLLoop *CopyInnerLoop =
      HNU.createHLLoop(nullptr, DDRU.createConstDDRef(IVType, 0),
                       CopyInnerLoopUB, DDRU.createConstDDRef(IVType, 1));

  HLNodeUtils::insertBefore(Anchor, CopyOuterLoop);
  HLNodeUtils::insertAsFirstChild(CopyOuterLoop, CopyInnerLoop);

  createZttForCopyOuterLoop(CopyOuterLoop);

  CopyInnerLoop->getUpperDDRef()->makeConsistent({AuxRef});

  // TODO: Not handled by makeConsistent?
  // %add - 1 should be Generic Rval
  CopyInnerLoop->getUpperDDRef()->setSymbase(GenericRvalSymbase);

  return CopyInnerLoop;
}

void HIRAosToSoa::TransformAosToSoa::rewrite() {

  Anchor->extractPreheaderAndPostexit();

  // stacksave intrinsic
  HLInst *Stacksave = insertCallToStacksave();

  // Create Add Inst producing trip counts of all unit-strided loops
  // The result is used as the trip count of the innerloop of copy-loop nest
  RegDDRef *AuxRef;
  RegDDRef *CopyInnerLoopTC = calcCopyInnerLoopTripCount(AuxRef);

  HLLoop *CopyOuterLoop = getCopyOuterLoop();

  int64_t ConstUB = 0;
  unsigned AddBlobIndex =
      CopyInnerLoopTC->isIntConstant(&ConstUB)
          ? InvalidBlobIndex
          : CopyInnerLoopTC->getSingleCanonExpr()->getSingleBlobIndex();

  SmallDenseMap<unsigned, HLInst *, 4> TrailingOffsetToAlloca;
  insertAllocas(CopyOuterLoop->getTripCountDDRef(AllocaLevel),
                CopyInnerLoopTC->clone(), TrailingOffsetToAlloca);

  HLLoop *CopyInnerLoop =
      insertCopyLoops(CopyOuterLoop, CopyInnerLoopTC, AuxRef);

  replaceTrailingOffsetWithAlloca(TrailingOffsetToAlloca, AddBlobIndex, AuxRef);
  SmallVector<unsigned, 4> AllocaSymbase;
  collectAllocaSymbase(TrailingOffsetToAlloca, AllocaSymbase);
  for (auto *Lp : LoopNests) {
    Lp->addLiveInTemp(AllocaSymbase);
    Lp->addLiveInTemp(CopyInnerLoopTC);
  }

  populatedBodyOfCopyLoop(CopyInnerLoop, TrailingOffsetToAlloca, AddBlobIndex,
                          AuxRef);

  CopyOuterLoop->clearLiveInTemp();
  CopyOuterLoop->clearLiveOutTemp();
  CopyOuterLoop->addLiveInTemp(CopyOuterLoop->getUpperDDRef());
  CopyOuterLoop->addLiveInTemp(CopyInnerLoop->getUpperDDRef());
  for (auto *ZttRef : make_range(CopyOuterLoop->ztt_ddref_begin(),
                                 CopyOuterLoop->ztt_ddref_end())) {
    CopyOuterLoop->addLiveInTemp(ZttRef);
  }
  CopyOuterLoop->addLiveInTemp(AllocaSymbase);
  CopyOuterLoop->addLiveInTemp(RefsToCopy);

  CopyInnerLoop->addLiveInTemp(CopyInnerLoop->getUpperDDRef());
  CopyInnerLoop->addLiveInTemp(AllocaSymbase);
  CopyInnerLoop->addLiveInTemp(RefsToCopy);

  // stackrestore intrinsic
  insertCallToStackrestore(Stacksave->getOperandDDRef(0));
}

bool HIRAosToSoa::run() {

  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Aos To Soa Disabled \n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR Aos To Soa on Function : "
                    << HIRF.getFunction().getName() << "\n");

  SmallVector<HLLoop *, 16> InnermostLoops;
  (HIRF.getHLNodeUtils()).gatherInnermostLoops(InnermostLoops);

  bool Changed = false;
  for (auto *InnermostLoop : InnermostLoops) {

    Analyzer AN(InnermostLoop, DDA,
                NumberOfTrailingOffsets > 0 ? NumberOfTrailingOffsets
                                            : DefaultNumOfTrailingOffsets,
                DefaultInnermostLoopNestsDepth);

    if (!AN.isCandidate())
      continue;

    TransformAosToSoa(InnermostLoop, AN.LoopNests, AN.RefsToCopy,
                      AN.IVBlobLevel)
        .rewrite();
    Changed = true;
  }

  return Changed;
}

PreservedAnalyses HIRAosToSoaPass::runImpl(llvm::Function &F,
                                           llvm::FunctionAnalysisManager &AM,
                                           HIRFramework &HIRF) {
  ModifiedHIR = HIRAosToSoa(HIRF, AM.getResult<HIRDDAnalysisPass>(F)).run();
  return PreservedAnalyses::all();
}

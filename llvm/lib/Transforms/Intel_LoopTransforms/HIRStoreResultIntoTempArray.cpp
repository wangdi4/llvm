//===-HIRStoreResultIntoTempArray.cpp Implements Store Result Into Temp Array
// class --===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===--------------------------------------------------------------------------------===//
//
// This pass implements two transformations:
//
// The first transformation is loop carried scalar replacement.
//
// We identify multiple expensive insts in one loop. If they have the same
// expression trees in the loop, we can extract the expensive insts and their
// dependent insts to form an independent loopnest and store the result into a
// temp array.
//
// For example,
// We are transforming this-
//
// do k=1,nk-1
//    kp1=k+1
//
//    do j=1,nj-1
//      jp1=mod(j,nj) + 1
//      r1 =  A(j,k) * B(j,k)
//      r2 =  A(jp1,kp1) * B(jp1,kp1)
//      r3 = r3 + (r1 + r2)
//    enddo
//  enddo
//
// To-
//
// do k=1,nk
//    do j=1,nj
//      TMP(j,k) =  A(j,k) * B(j,k)
//    enddo
// enddo
//
// do k=1,nk-1
//    kp1=k+1
//
//    do j=1,nj-1
//      jp1=mod(j,nj) + 1
//      r1 =  TMP(j,k)
//      r2 =  TMP(jp1,kp1)
//      r3 = r3 + (r1 + r2)
//    enddo
// enddo
//
// TODO:
// The second transformation happens among multiple loops in one region. We
// extract expensive insts from the loops if they have the same sequence insts
// and store the result into a temp array.
//
// For example,
// We are transforming this-
//
// do i=1, n
//   a(i) = b(i) ** n + c(i)
// enddo
//
// do i=1, n
//   a(i) = b(i) ** n + c(i+1)
// enddo
//
// do i=1, n
//   a(i) = b(i) ** n + c(i+2)
// enddo
//
// To-
//
// do i=1, n
//   new(i) = b(i) ** n
// enddo
//
// do i=1, n
//   a(i) = new(i) + c(i)
// enddo
//
// do i=1, n
//   a(i) = new(i) + c(i+1)
// enddo
//
// do i=1, n
//   a(i) = new(i)  + c(i+2)
// enddo
//
//===--------------------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRStoreResultIntoTempArray.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define OPT_SWITCH "hir-store-result-into-temp-array"
#define OPT_DESC "HIR Store Result Into Temp Array"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

static cl::opt<unsigned> NumLoopsForBulkTransform(
    "hir-store-result-into-temp-array-num-loops-for-bulk-transform",
    cl::init(2), cl::Hidden,
    cl::desc("Threshold for number of loops for bulk loop carried scalar "
             "replacement"));

namespace {
const unsigned NumLoopnestLevel = 3;

typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;
using LpExpensiveInstsPairsTy =
    SmallVector<std::pair<HLLoop *, SmallVector<HLInst *, 16>>, 4>;

class HIRStoreResultIntoTempArray {
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;

public:
  HIRStoreResultIntoTempArray(HIRFramework &HIRF, HIRDDAnalysis &DDA)
      : HIRF(HIRF), DDA(DDA) {}

  bool run();

private:
  bool isLegalForLoopCarriedScalarReplacement(
      HLLoop *Lp, SmallVectorImpl<HLInst *> &ExpensiveInsts);

  bool isLegalForBulkLoopCarriedScalarReplacement(
      LpExpensiveInstsPairsTy &LpExpensiveInstsPairs,
      SmallVectorImpl<RegDDRef *> &LoopUpperBounds);

  bool
  doLoopCarriedScalarReplacement(HLLoop *Lp,
                                 SmallVectorImpl<HLInst *> &ExpensiveInsts);

  bool doBulkLoopCarriedScalarReplacement(
      LpExpensiveInstsPairsTy &LpExpensiveInstsPairs,
      SmallVectorImpl<RegDDRef *> &LoopUpperBounds);

  bool hasSameExprTree(HLLoop *InnermostLp, DDGraph &DDG,
                       SmallVectorImpl<HLInst *> &ExpensiveInsts);

  void collectInstsInExprTree(DDGraph &DDG, HLInst *HInst,
                              SmallVectorImpl<HLInst *> &InstsInExprTree);

  HLLoop *createExtractedLoop(HLLoop *Lp, RegDDRef *MaxRef, RegDDRef *MinRef,
                              HLInst *ExpensiveInst,
                              SmallVectorImpl<HLInst *> &InstsInExprTree,
                              HLInst *&AllocaInst, unsigned &AllocaMemRefSB,
                              SmallVectorImpl<int64_t> &Offsets);

  HLLoop *createExtractedLoopWithLargestLoopUpperBounds(
      HLLoop *FirstLoop, HLLoop *LoopWithMinRef, RegDDRef *MemRef,
      HLInst *ExpensiveInstForExtractedLoop,
      SmallVectorImpl<RegDDRef *> &LoopUpperBounds,
      SmallVectorImpl<CanonExpr *> &DistsBetweenMemRefs,
      SmallVectorImpl<HLInst *> &InstsInExprTree, HLInst *&AllocaInst,
      unsigned &AllocaMemRefSB, SmallVectorImpl<int64_t> &Offsets);

  HLLoop *getDistsBetweenMinRefAndMaxRef(
      LpExpensiveInstsPairsTy &LpExpensiveInstsPairs,
      SmallVectorImpl<CanonExpr *> &DistsBetweenMemRefs);

  RegDDRef *addDimensionForAllocaMemRef(const HLLoop *ExtractedLoop,
                                        const HLLoop *Lp, RegDDRef *AllocaRef,
                                        const RegDDRef *MemRef,
                                        uint64_t TypeSize,
                                        SmallVectorImpl<int64_t> &Offsets);
};
} // namespace

// Recursively collect the dependent insts of the expensive inst
void HIRStoreResultIntoTempArray::collectInstsInExprTree(
    DDGraph &DDG, HLInst *HInst, SmallVectorImpl<HLInst *> &InstsInExprTree) {
  unsigned NumOperands = HInst->getNumOperands();

  for (unsigned I = 0, E = NumOperands; I < E; ++I) {
    RegDDRef *Ref = HInst->getOperandDDRef(I);

    for (const DDEdge *Edge : DDG.incoming(Ref)) {
      auto *Src = Edge->getSrc();
      HLInst *SrcInst = dyn_cast<HLInst>(Src->getHLDDNode());

      if (std::find(InstsInExprTree.begin(), InstsInExprTree.end(), SrcInst) ==
          InstsInExprTree.end()) {
        InstsInExprTree.push_back(SrcInst);
      }

      collectInstsInExprTree(DDG, SrcInst, InstsInExprTree);
    }
  }

  return;
}

static bool contains(RegDDRef *OpRef, BlobTy SCEVBlob) {
  CanonExpr *CE = OpRef->getSingleCanonExpr();
  BlobUtils &BU = CE->getBlobUtils();

  auto OpBlob = BU.getBlob(CE->getSingleBlobIndex());

  if (BU.contains(SCEVBlob, OpBlob)) {
    return true;
  }

  return false;
}

static bool checkIV(const BlobDDRef *Ref, DDGraph &DDG, unsigned LoopLevel,
                    int64_t &Constant) {
  if (DDG.getTotalNumIncomingFlowEdges(Ref) != 1) {
    return false;
  }

  const DDEdge *Edge = *(DDG.incoming_edges_begin(Ref));
  auto *Src = Edge->getSrc();
  HLInst *SrcInst = cast<HLInst>(Src->getHLDDNode());

  unsigned Opcode = SrcInst->getLLVMInstruction()->getOpcode();

  // check:
  // 1) the opcode is modulo
  // 2) the left operand includes iv
  // 3) the right operand should contain the loop upperbound
  // The upperbound can be zext or sext type, such as zext.i32.i64(%ny) or
  // sext.i32.i64(%nz) for example: %rem = i1 + 1 % %ny + 1
  if (Opcode != Instruction::SRem) {
    LLVM_DEBUG(dbgs() << "Opcode SRem not found.\n");
    return false;
  }

  RegDDRef *OperandRef1 = SrcInst->getOperandDDRef(1);
  RegDDRef *OperandRef2 = SrcInst->getOperandDDRef(2);

  CanonExpr *OperandRefCE2 = OperandRef2->getSingleCanonExpr();

  if (OperandRefCE2->hasIV() || OperandRefCE2->numBlobs() != 1 ||
      (OperandRefCE2->getConstant() != 0 &&
       OperandRefCE2->getConstant() != 1)) {
    return false;
  }

  if (!OperandRef1->hasIV(LoopLevel)) {
    return false;
  }

  CanonExpr *UpperCE =
      SrcInst->getParentLoopAtLevel(LoopLevel)->getUpperCanonExpr();

  if (!(UpperCE->hasBlob() && UpperCE->numBlobs() == 1 &&
        UpperCE->getConstant() < 0)) {
    return false;
  }

  unsigned BlobIndex = UpperCE->getSingleBlobIndex();
  auto &BU = UpperCE->getBlobUtils();
  auto Blob = BU.getBlob(BlobIndex);

  auto ZExt = dyn_cast<SCEVZeroExtendExpr>(Blob);
  auto SExt = dyn_cast<SCEVSignExtendExpr>(Blob);

  if (!(ZExt || SExt)) {
    return false;
  }

  BlobTy SCEVBlob = ZExt ? ZExt->getOperand() : SExt->getOperand();

  if (!contains(OperandRef2, SCEVBlob)) {
    return false;
  }

  Constant = OperandRef2->getSingleCanonExpr()->getConstant();

  return true;
}

static bool compareMemRefs(RegDDRef *Ref1, RegDDRef *Ref2, DDGraph &DDG,
                           HLLoop *InnermostLp) {
  if (!CanonExprUtils::areEqual(Ref1->getBaseCE(), Ref2->getBaseCE(), false)) {
    return false;
  }

  // We check:
  // 1) Ref1 does not have any blobs
  // 2) All its indices are either constant or linear
  // 3) The IV levels should be in increasing order
  // Ref1 should have the format, like %A[i1+1][i2][i3][0]
  // Ref2 might have the format, like %A[i1+2[%rem][%rem6+1]
  unsigned OutermostLoopLevel =
      InnermostLp->getNestingLevel() - NumLoopnestLevel + 1;
  auto *Lp = InnermostLp;
  unsigned LoopLevel = OutermostLoopLevel;

  for (unsigned Dim = Ref1->getNumDimensions(); Dim > 0; --Dim, ++LoopLevel) {
    auto *CE = Ref1->getDimensionIndex(Dim);

    if (CE->hasBlob()) {
      return false;
    }

    if (Dim == 1) {
      if (!CE->isConstant()) {
        return false;
      }
    } else if (Dim <= 4) {
      if (!CE->hasIV(LoopLevel)) {
        return false;
      }

      if (CE->numIVs() != 1 || CE->getDenominator() != 1) {
        return false;
      }

      unsigned Index;
      int64_t Coeff;

      CE->getIVCoeff(LoopLevel, &Index, &Coeff);

      if ((Coeff != 1) || (Index != InvalidBlobIndex)) {
        return false;
      }

      //  CE might have a sext case: LINEAR sext.i32.i64(i2 + 3)
      if (!CE->isSExt() && CE->getSrcType() != CE->getDestType()) {
        return false;
      }
    } else {
      return false;
    }

    if (Dim > 1) {
      CanonExpr *MemRefLowerDimCE1 = Ref1->getDimensionLower(Dim);
      CanonExpr *MemRefLowerDimCE2 = Ref2->getDimensionLower(Dim);

      // Check the dim lower bound for Ref1 and Ref2 are identical
      if (!CanonExprUtils::areEqual(MemRefLowerDimCE1, MemRefLowerDimCE2,
                                    true)) {
        return false;
      }

      CanonExpr *MemRefDimStride1 = Ref1->getDimensionStride(Dim);
      CanonExpr *MemRefDimStride2 = Ref2->getDimensionStride(Dim);

      // Check the dim stride for Ref1 and Ref2 are identical
      if (!CanonExprUtils::areEqual(MemRefDimStride1, MemRefDimStride2, true)) {
        return false;
      }

      Lp = Lp->getParentLoop();
    }
  }

  unsigned Level = OutermostLoopLevel;

  for (auto Iter1 = Ref1->canon_rbegin(), Iter2 = Ref2->canon_rbegin(),
            End = Ref1->canon_rend();
       Iter1 != End; ++Iter1, ++Iter2, ++Level) {
    CanonExpr *CE1 = *Iter1;
    CanonExpr *CE2 = *Iter2;

    if (CE1->hasIV(Level)) {

      if (CE2->hasBlob()) { // CE2 might have the case like: zext.i32.i64(%773)
                            // + 1
        if (CE2->hasIV(Level)) {
          return false;
        }

        if (CE2->getConstant() > 1) {
          return false;
        }

        SmallVector<unsigned, 4> BlobIndices;

        CE2->collectTempBlobIndices(BlobIndices);

        if (BlobIndices.size() != 1) {
          return false;
        }

        unsigned BlobIndex = BlobIndices[0];

        BlobDDRef *BlobRef = Ref2->getBlobDDRef(BlobIndex);

        int64_t Constant = 0;
        if (!checkIV(BlobRef, DDG, Level, Constant)) {
          return false;
        }
      } else if (!CE2->hasIV(Level)) {
        return false;
      } else {
        int64_t Dist = 0;

        // Use relaxed mode for distance computation for such cases like:
        // CE1 =  i64 i2 + 2 and CE2 =  zext.i32.i64(i2 + 3)
        if (!CanonExprUtils::getConstIterationDistance(CE1, CE2, Level, &Dist,
                                                       true)) {
          return false;
        }
      }
      // CE1 has no IV at this level case
    } else {
      if (!CanonExprUtils::areEqual(CE1, CE2)) {
        return false;
      }
    }
  }

  return true;
}

static bool compareRefTypes(RegDDRef *Ref1, RegDDRef *Ref2) {
  if (!((Ref1->isTerminalRef() && Ref2->isTerminalRef()) ||
        (Ref1->isMemRef() && Ref2->isMemRef()))) {
    return false;
  }

  if (Ref1->getNumDimensions() != Ref2->getNumDimensions()) {
    return false;
  }

  return true;
}

// Compare whether the two Refs are corresponding
static bool corresponds(RegDDRef *Ref1, RegDDRef *Ref2, DDGraph &DDG,
                        HLLoop *InnermostLp) {
  if (Ref1->isConstant()) {
    return DDRefUtils::areEqual(Ref1, Ref2);
  }

  if (Ref1->isTerminalRef()) {
    return (Ref1->isSelfBlob() && Ref1->isNonLinear() && Ref2->isSelfBlob() &&
            Ref2->isNonLinear());
  }

  if (!compareRefTypes(Ref1, Ref2)) {
    return false;
  }

  if (!compareMemRefs(Ref1, Ref2, DDG, InnermostLp)) {
    return false;
  }

  return true;
}

static bool compareLvals(HLInst *Inst1, HLInst *Inst2) {
  auto *Lval1 = Inst1->getLvalDDRef();
  auto *Lval2 = Inst2->getLvalDDRef();
  assert(Lval1 && Lval2 && "The insts should have Lvals");

  if (!Lval1->isTerminalRef() || !Lval2->isTerminalRef()) {
    return false;
  }

  if (!Lval1->isNonLinear() || !Lval2->isNonLinear()) {
    return false;
  }

  return true;
}

// Compare the sequence insts in two expression trees
// Here's an example-
//
// %t =
// DO i1
//  %t1 = A[i1];          --
//                         | expression tree 1
//  %t2  = %t1 * %t       --
//
//  %t3 = pow(%t2, 5);    - expensive inst1
//
//  %t4 = A[i1 + 1];      --
//                         | expression tree 2
//  %t5 = %t4 * %t;       --
//
//  %t6 = pow(%t5, 5);    - expensive inst2
// END DO
static bool corresponds(HLInst *Inst1, HLInst *Inst2, DDGraph &DDG,
                        HLLoop *InnermostLp) {
  if (!Inst1->isSameOperationAs(Inst2)) {
    return false;
  }

  auto *RvalIt1 = Inst1->rval_op_ddref_begin();
  auto *RvalIt2 = Inst2->rval_op_ddref_begin();

  for (auto End = Inst1->rval_op_ddref_end(); RvalIt1 != End;
       ++RvalIt1, ++RvalIt2) {
    if (!corresponds(*RvalIt1, *RvalIt2, DDG, InnermostLp)) {
      return false;
    }

    // Give up if ref1 is memref and ref1 and ref2 have any incoming edges
    if ((*RvalIt1)->isMemRef()) {

      if (DDG.incoming_edges_begin(*RvalIt1) !=
              DDG.incoming_edges_end(*RvalIt1) ||
          DDG.incoming_edges_begin(*RvalIt2) !=
              DDG.incoming_edges_end(*RvalIt2)) {
        return false;
      }

      continue;
    }

    unsigned NumIncomingEdge1 = DDG.getTotalNumIncomingFlowEdges(*RvalIt1);
    unsigned NumIncomingEdge2 = DDG.getTotalNumIncomingFlowEdges(*RvalIt2);

    if (NumIncomingEdge1 != NumIncomingEdge2) {
      return false;
    }

    auto EdgeIt1 = DDG.incoming_edges_begin(*RvalIt1);
    auto EdgeIt2 = DDG.incoming_edges_begin(*RvalIt2);

    for (auto EndEdge = DDG.incoming_edges_end(*RvalIt1); EdgeIt1 != EndEdge;
         ++EdgeIt1, ++EdgeIt2) {
      auto *Src1 = (*EdgeIt1)->getSrc();
      auto *Src2 = (*EdgeIt2)->getSrc();

      HLInst *SrcInst1 = cast<HLInst>(Src1->getHLDDNode());
      HLInst *SrcInst2 = cast<HLInst>(Src2->getHLDDNode());

      if (!corresponds(SrcInst1, SrcInst2, DDG, InnermostLp)) {
        return false;
      }
    }
  }

  if (!compareLvals(Inst1, Inst2)) {
    return false;
  }

  return true;
}

// Check whether the expensive insts have the same expression tree in the loop
// Collect all the insts for each expensive inst, then compare the sequences
bool HIRStoreResultIntoTempArray::hasSameExprTree(
    HLLoop *InnermostLp, DDGraph &DDG,
    SmallVectorImpl<HLInst *> &ExpensiveInsts) {
  HLInst *FirstInst = *(ExpensiveInsts.begin());

  for (auto It = std::next(ExpensiveInsts.begin()), End = ExpensiveInsts.end();
       It != End; ++It) {
    HLInst *CurInst = *It;

    if (!corresponds(FirstInst, CurInst, DDG, InnermostLp)) {
      return false;
    }
  }

  return true;
}

static bool isLoopnestValid(const HLLoop *Lp) {
  unsigned OuterLoopLevel = Lp->getNestingLevel() - NumLoopnestLevel + 1;

  do {
    if (!Lp->isDo() || !Lp->isNormalized()) {
      return false;
    }

    const CanonExpr *UpperCE = Lp->getUpperCanonExpr();

    if (!UpperCE->canConvertToStandAloneBlobOrConstant() ||
        UpperCE->getDenominator() != 1) {
      return false;
    }
  } while ((Lp = Lp->getParentLoop()) &&
           Lp->getNestingLevel() >= OuterLoopLevel);

  return true;
}

bool HIRStoreResultIntoTempArray::isLegalForLoopCarriedScalarReplacement(
    HLLoop *Lp, SmallVectorImpl<HLInst *> &ExpensiveInsts) {

  unsigned LoopLevel = Lp->getNestingLevel();

  if (LoopLevel < NumLoopnestLevel) {
    return false;
  }

  if (!isLoopnestValid(Lp)) {
    return false;
  }

  for (auto It = Lp->child_begin(), End = Lp->child_end(); It != End; ++It) {
    HLInst *HInst = dyn_cast<HLInst>(&*It);

    if (!HInst) {
      return false;
    }

    Intrinsic::ID Id;

    if (HInst->isIntrinCall(Id) && Id == Intrinsic::pow) {
      ExpensiveInsts.push_back(HInst);
    }
  }

  if (ExpensiveInsts.size() < 2) {
    return false;
  }

  // Collect all MemRefs in the loop
  MemRefGatherer::VectorTy Refs;
  MemRefGatherer::gatherRange<true, false>(Lp->child_begin(), Lp->child_end(),
                                           Refs);

  if (Refs.empty()) {
    return false;
  }

  DDGraph DDG;

  DDG =
      DDA.getGraph(Lp->getParentLoopAtLevel(LoopLevel - NumLoopnestLevel + 1));

  if (!hasSameExprTree(Lp, DDG, ExpensiveInsts)) {
    return false;
  }

  return true;
}

static RegDDRef *getMemRef(SmallVectorImpl<HLInst *> &InstsInExprTree) {
  for (auto HInst : InstsInExprTree) {
    unsigned NumOperands = HInst->getNumOperands();

    for (unsigned I = 0, E = NumOperands; I < E; ++I) {
      RegDDRef *Ref = HInst->getOperandDDRef(I);

      if (Ref->isMemRef()) {
        return Ref;
      }
    }
  }

  return nullptr;
}

// Record the constant in each dimension of MinRef, for example: the offset of
// // A[i1+2][i2][i3] is {2, 0, 0}
static void recordOffsets(RegDDRef *MinRef, unsigned LoopLevel,
                          SmallVectorImpl<int64_t> &Offsets) {
  for (auto Iter = MinRef->canon_rbegin(), End = MinRef->canon_rend();
       Iter != End; ++Iter) {
    CanonExpr *CE = *Iter;

    if (CE->hasIV(LoopLevel)) {
      int64_t Constant = CE->getConstant() >= 0 ? CE->getConstant() : 0;
      Offsets.push_back(Constant);
      return;
    }
  }

  return;
}

// Create alloca inst based on the multiplication insts of the trip counts
// like %array_size = zext.i32.i64(%ny)  *  sext.i32.i64(%nz);
//      %array_size5 = zext.i32.i64(%nx) + 1  *  %array_size;
//      %TempArray = alloca %array_size5;
static HLInst *createAllocaInst(RegDDRef *MemRef, HLLoop *InnermostLp,
                                Type *DestTy,
                                SmallVectorImpl<HLInst *> &ArraySizeInsts,
                                SmallVectorImpl<RegDDRef *> &TripCountRefs,
                                SmallVectorImpl<int64_t> &Offsets) {
  unsigned Level = InnermostLp->getNestingLevel();
  auto *Lp = InnermostLp;
  auto &HNU = Lp->getHLNodeUtils();
  unsigned OutermostLoopLevel = Level - NumLoopnestLevel + 1;
  RegDDRef *TripCountRef = Lp->getTripCountDDRef();

  // Update the array size by the offset of iv
  recordOffsets(MemRef, Level, Offsets);

  TripCountRefs.push_back(TripCountRef);

  HLInst *ArraySizeMulInst = nullptr;

  unsigned LoopLevel = Level;

  while ((Lp = Lp->getParentLoop()) && (LoopLevel > OutermostLoopLevel)) {
    RegDDRef *ParentTripCountRef = Lp->getTripCountDDRef();
    LoopLevel = Lp->getNestingLevel();

    recordOffsets(MemRef, LoopLevel, Offsets);

    // Create trip count multiplication inst, like %array_size =
    // zext.i32.i64(%ny)  *  sext.i32.i64(%nz);
    ArraySizeMulInst =
        HNU.createMul(ParentTripCountRef, TripCountRef, "array_size");
    ArraySizeInsts.push_back(ArraySizeMulInst);
    TripCountRefs.push_back(ParentTripCountRef);

    TripCountRef = ArraySizeMulInst->getLvalDDRef()->clone();
  }

  std::reverse(Offsets.begin(), Offsets.end());

  HLInst *AllocaInst = HNU.createAlloca(
      DestTy, ArraySizeMulInst->getLvalDDRef()->clone(), "TempArray");

  return AllocaInst;
}

static CanonExpr *getStrideCE(const HLLoop *Lp, uint64_t TypeSize,
                              unsigned Level) {
  auto &HNU = Lp->getHLNodeUtils();
  auto &CEU = HNU.getCanonExprUtils();
  auto IVType = Lp->getIVType();
  CanonExpr *StrideCE = CEU.createCanonExpr(IVType);
  StrideCE->addConstant(TypeSize, true);
  unsigned LoopLevel = Lp->getNestingLevel();

  while (LoopLevel > Level) {
    CanonExpr *TripCountCE = Lp->getTripCountCanonExpr();
    TripCountCE->convertToStandAloneBlobOrConstant();

    unsigned TripCountBlobIndex = TripCountCE->getSingleBlobIndex();

    StrideCE->multiplyByBlob(TripCountBlobIndex);
    --LoopLevel;
    Lp = Lp->getParentLoop();
  }

  return StrideCE;
}

// Add dimensons for the AllocaRef by copying the CEs of the MemRef in the
// expression tree
RegDDRef *HIRStoreResultIntoTempArray::addDimensionForAllocaMemRef(
    const HLLoop *ExtractedLoop, const HLLoop *Lp, RegDDRef *AllocaRef,
    const RegDDRef *MemRef, uint64_t TypeSize,
    SmallVectorImpl<int64_t> &Offsets) {
  unsigned LoopLevel = Lp->getNestingLevel();
  unsigned OutermostLoopLevel = LoopLevel - NumLoopnestLevel + 1;
  DDGraph DDG = DDA.getGraph(Lp->getParentLoopAtLevel(OutermostLoopLevel));

  // A set collects the CE with blob refs that we also handled, like-
  // %t1 = i1 % nx;
  // %t2 = i2 % ny;
  // ... = %A[%t1][%t2];
  SmallSet<const CanonExpr *, 8> CEsChecked;

  unsigned OffsetId = 0;

  for (unsigned I = OutermostLoopLevel; I <= LoopLevel; ++I) {

    for (auto Iter = MemRef->canon_rbegin(), End = MemRef->canon_rend();
         Iter != End; ++Iter) {
      const CanonExpr *CE = *Iter;
      bool FoundBlob = false;
      const BlobDDRef *BlobRef = nullptr;

      // We handle the case if MemRef has blobs, for example,
      // %a = i1 % t;
      // ... = A[%a];
      // We need to check if the source of blob %a has iv
      if (CE->hasBlob()) {
        if (CEsChecked.count(CE)) {
          continue;
        }

        for (auto BI = MemRef->blob_begin(), E = MemRef->blob_end(); BI != E;
             ++BI) {
          BlobRef = *BI;
          int64_t Constant = 0;

          if (!checkIV(BlobRef, DDG, I, Constant)) {
            continue;
          } else {
            FoundBlob = true;
            CEsChecked.insert(CE);
            break;
          }
        }

        if (!FoundBlob) {
          continue;
        }
      }
      // If there is no blob in MemRef, we only check iv in the CE
      else {

        unsigned Index;
        int64_t Coeff;

        CE->getIVCoeff(I, &Index, &Coeff);

        if (!Coeff) {
          continue;
        }
      }

      CanonExpr *CloneCE = CE->clone();

      CloneCE->convertToStandAloneBlobOrConstant();

      CloneCE->addConstant(-Offsets[OffsetId++], true);

      CanonExpr *StrideCE = getStrideCE(ExtractedLoop, TypeSize, I);
      Type *DimTy = AllocaRef->getBaseType();
      Type *DimElemTy = AllocaRef->getBasePtrElementType();
      AllocaRef->addDimension(CloneCE, {}, nullptr, StrideCE, DimTy, DimElemTy);

      break;
    }
  }

  return AllocaRef;
}

static void updateLiveInArraySize(HLLoop *InnermostLp, unsigned SB) {
  HLLoop *Lp = InnermostLp;

  while (Lp) {
    Lp->addLiveInTemp(SB);
    Lp = Lp->getParentLoop();
  }
}

static void updateLiveInAllocaTemp(HLLoop *InnermostLp, unsigned SB) {
  HLLoop *Lp = InnermostLp;
  unsigned LoopLevel = InnermostLp->getNestingLevel();
  unsigned OutermostLoopLevel = LoopLevel - NumLoopnestLevel + 1;

  while (Lp && (Lp->getNestingLevel() >= OutermostLoopLevel)) {
    Lp->addLiveInTemp(SB);
    Lp = Lp->getParentLoop();
  }
}

// We need to update the upperbound by adding the constant in the
// CE with blob, such as %A[i1 + 2][%mod][%mod27 + 1][0]
static bool NeedUpdateUpperBound(DDGraph DDG, RegDDRef *MaxRef,
                                 unsigned LoopLevel, int64_t &Constant) {
  unsigned Dim = MaxRef->getNumDimensions();

  for (unsigned I = 1; I < Dim; ++I) {
    CanonExpr *CE = MaxRef->getDimensionIndex(I);

    if (CE->numBlobs() != 1) {
      continue;
    }

    Constant = CE->getConstant();

    if (Constant <= 0) {
      continue;
    }

    unsigned BlobIndex = CE->getSingleBlobIndex();

    for (auto BI = MaxRef->blob_begin(), E = MaxRef->blob_end(); BI != E;
         ++BI) {
      BlobDDRef *BlobRef = *BI;

      if (BlobRef->getBlobIndex() != BlobIndex) {
        continue;
      }

      int64_t Offset = 0;

      if (checkIV(BlobRef, DDG, LoopLevel, Offset)) {
        return true;
      }
    }
  }

  return false;
}

static void updateUpperBound(DDGraph DDG, RegDDRef *MaxRef, unsigned LoopLevel,
                             HLLoop *Lp) {
  int64_t Constant = 0;

  // We need to update the upperbound by adding the constant in the
  // operand2 of the blob's src instruction, such as %mod = i2 + 3  %
  // %"jacobian_$NY_fetch" + 1;
  for (auto BI = MaxRef->blob_begin(), E = MaxRef->blob_end(); BI != E; ++BI) {
    BlobDDRef *BlobRef = *BI;

    if (checkIV(BlobRef, DDG, LoopLevel, Constant) && Constant > 0) {
      Lp->getUpperCanonExpr()->addConstant(Constant, true);
    }
  }

  int64_t Offset = 0;

  if (NeedUpdateUpperBound(DDG, MaxRef, LoopLevel, Offset) && Offset > 0) {
    Lp->getUpperCanonExpr()->addConstant(Offset, true);
  }

  return;
}

// Create parent loops for the new loop nest and update the upper bound
// For example: MinRef -> %A[i1][i2][i3]
//              MaxRef -> %A[i1+1][%rem1][%rem2]
// the loop i1 needs to increase the upperbound by 1
static HLLoop *createExtractedLoopNest(DDGraph DDG, HLLoop *Lp, HLLoop *NewLoop,
                                       RegDDRef *MaxRef, RegDDRef *MinRef) {
  unsigned Level = Lp->getNestingLevel();
  unsigned LoopLevel = Level;
  unsigned OutermostLoopLevel = Level - NumLoopnestLevel + 1;

  HLLoop *ParentLoop = nullptr;
  CanonExpr *MinCanonExpr = nullptr;
  CanonExpr *MaxCanonExpr = nullptr;
  CanonExpr *MinCE = nullptr;
  CanonExpr *MaxCE = nullptr;
  CanonExpr *MemRefLowerDimCE = nullptr;
  unsigned NumDimension = MinRef->getNumDimensions();
  unsigned Dim = 1;

  while (LoopLevel >= OutermostLoopLevel) {
    bool HasSelfBlob = false;
    bool Subtracted = true;

    do {
      MinCanonExpr = MinRef->getDimensionIndex(Dim);
      MaxCanonExpr = MaxRef->getDimensionIndex(Dim);
      MemRefLowerDimCE = MinRef->getDimensionLower(Dim);
      Dim++;
    } while (MaxCanonExpr->isConstant() && Dim <= NumDimension);

    MinCE = MinCanonExpr->clone();
    MaxCE = MaxCanonExpr->clone();

    if (!MaxCE->hasBlob()) {
      Subtracted = CanonExprUtils::subtract(MaxCE, MinCE, true);
    } else {
      HasSelfBlob = true;
    }

    if (LoopLevel != Level) {
      // Clone the empty parent loop nest for the new loop's parent loops
      Lp = Lp->getParentLoop();
      ParentLoop = Lp->cloneEmpty();

      // We need to update the upperbound by adding the iteration distance
      // between MinRef and MaxRef at corresponding level, for example %A[i1 +
      // 1][i2][i3][0] and %A[i1 + 2][%mod][%mod27][0], we increase i1
      // upperbound by 1
      if (!HasSelfBlob && Subtracted) {
        CanonExprUtils::add(ParentLoop->getUpperCanonExpr(), MaxCE, true);
      } else {
        updateUpperBound(DDG, MaxRef, LoopLevel, ParentLoop);
      }

      HLNodeUtils::insertAsFirstChild(ParentLoop, NewLoop);
      NewLoop = NewLoop->getParentLoop();

    } else {

      if (!HasSelfBlob && Subtracted) {
        CanonExprUtils::add(NewLoop->getUpperCanonExpr(), MaxCE, true);
      } else {
        updateUpperBound(DDG, MaxRef, LoopLevel, NewLoop);
      }
    }

    if (!MemRefLowerDimCE->isZero()) {
      CanonExprUtils::add(NewLoop->getUpperCanonExpr(), MemRefLowerDimCE, true);
    }

    LoopLevel--;
  }

  return ParentLoop;
}

static HLInst *insertCallToStacksave(HLLoop *Lp) {
  const DebugLoc &DLoc = Lp->getDebugLoc();
  auto &HNU = Lp->getHLNodeUtils();

  HLInst *StacksaveCall = HNU.createStacksave(DLoc);
  return StacksaveCall;
}

static HLInst *insertCallToStackrestore(HLLoop *Lp, RegDDRef *StackAddrRef) {
  auto &HNU = Lp->getHLNodeUtils();
  auto &DRU = HNU.getDDRefUtils();
  auto &CEU = HNU.getCanonExprUtils();
  auto Int8Ty = Type::getInt8Ty(HNU.getContext());
  StackAddrRef = DRU.createAddressOfRef(Int8Ty, StackAddrRef->getSelfBlobIndex(),
                                        StackAddrRef->getDefinedAtLevel(),
                                        StackAddrRef->getSymbase(), true);

  StackAddrRef->addDimension(CEU.createCanonExpr(Int8Ty, APInt(8, 0)));

  HLInst *StackrestoreCall = HNU.createStackrestore(StackAddrRef);
  return StackrestoreCall;
}

static void makeConsistent(RegDDRef *AllocaRef, RegDDRef *MemRef, HLLoop *Lp) {

  SmallVector<RegDDRef *, 4> AuxRefs;

  AuxRefs.push_back(MemRef);

  unsigned LoopLevel = Lp->getNestingLevel();
  unsigned OutermostLoopLevel = LoopLevel - NumLoopnestLevel + 1;

  while (Lp && (Lp->getNestingLevel() >= OutermostLoopLevel)) {
    AuxRefs.push_back(Lp->getUpperDDRef());
    Lp = Lp->getParentLoop();
  }

  AllocaRef->makeConsistent(AuxRefs);
}

static void updateLiveInForBlobs(RegDDRef *MemRef, HLLoop *InnermostLp) {
  auto &HNU = InnermostLp->getHLNodeUtils();
  auto &BU = HNU.getBlobUtils();
  SmallVector<unsigned, 4> TempBlobIndices;

  unsigned BlobIndex = MemRef->getSingleCanonExpr()->getSingleBlobIndex();
  BU.collectTempBlobs(BlobIndex, TempBlobIndices);

  for (auto BI : TempBlobIndices) {
    updateLiveInArraySize(InnermostLp, BU.getTempBlobSymbase(BI));
  }
}

// Create a new loop and insert the insts in the expression tree into the new
// loop
HLLoop *HIRStoreResultIntoTempArray::createExtractedLoop(
    HLLoop *Lp, RegDDRef *MaxRef, RegDDRef *MinRef, HLInst *ExpensiveInst,
    SmallVectorImpl<HLInst *> &InstsInExprTree, HLInst *&Alloca,
    unsigned &AllocaMemRefSB, SmallVectorImpl<int64_t> &Offsets) {

  unsigned OutermostLoopLevel = Lp->getNestingLevel() - NumLoopnestLevel + 1;

  HLLoop *OuterAnchorLp = Lp->getParentLoopAtLevel(OutermostLoopLevel);

  // Create the extracted loop
  HLLoop *NewLoop = Lp->cloneEmpty();

  DDGraph DDG = DDA.getGraph(OuterAnchorLp);

  HLLoop *NewOuterLoop =
      createExtractedLoopNest(DDG, Lp, NewLoop, MaxRef, MinRef);

  HLNodeUtils::insertBefore(OuterAnchorLp, NewOuterLoop);

  // stacksave intrinsicc
  HLInst *Stacksave = insertCallToStacksave(NewLoop);

  HLNodeUtils::insertBefore(NewLoop->getParentLoopAtLevel(OutermostLoopLevel),
                            Stacksave);

  // Insert the inst seqences
  for (auto Inst : InstsInExprTree) {
    HLInst *InstClone = Inst->clone();
    HLNodeUtils::insertAsLastChild(NewLoop, InstClone);
    updateLiveInAllocaTemp(NewLoop, InstClone->getLvalDDRef()->getSymbase());
  }

  SmallVector<HLInst *, 8> ArraySizeInsts;
  SmallVector<RegDDRef *, 8> TripCountRefs;

  // Create Alloca inst, like %TempArray = alloca %array_size5
  Alloca = createAllocaInst(MinRef, NewLoop,
                                ExpensiveInst->getLvalDDRef()->getDestType(),
                                ArraySizeInsts, TripCountRefs, Offsets);

  HLLoop *NewOutermostLoop = NewLoop->getParentLoopAtLevel(OutermostLoopLevel);

  // Insert array size insts as last preheader of the loop
  for (auto ArraySizeInst : ArraySizeInsts) {
    HLNodeUtils::insertBefore(NewOutermostLoop, ArraySizeInst);
    updateLiveInAllocaTemp(NewLoop,
                           ArraySizeInst->getLvalDDRef()->getSymbase());
  }

  auto &HNU = NewLoop->getHLNodeUtils();
  auto &DRU = HNU.getDDRefUtils();

  // Update live in of the trip count refs in the array size insts
  for (auto TripCountRef : TripCountRefs) {
    TripCountRef->makeConsistent();
    updateLiveInForBlobs(TripCountRef, NewLoop);
  }

  HNU.insertBefore(NewOutermostLoop, Alloca);

  updateLiveInAllocaTemp(NewLoop, Alloca->getLvalDDRef()->getSymbase());

  // Create a memref using AllocaRef and update the expensive inst's lval
  RegDDRef *AllocaDDRef = DRU.createMemRef(cast<AllocaInst>(Alloca->getLLVMInstruction())->getAllocatedType(),
      Alloca->getLvalDDRef()->getSingleCanonExpr()->getSingleBlobIndex(),
      Alloca->getNodeLevel());

  AllocaMemRefSB = AllocaDDRef->getSymbase();

  RegDDRef *MemRef = getMemRef(InstsInExprTree);

  uint64_t TypeSize = ExpensiveInst->getLvalDDRef()->getDestTypeSizeInBytes();

  RegDDRef *AllocaRef = addDimensionForAllocaMemRef(
      NewLoop, NewLoop, AllocaDDRef->clone(), MemRef, TypeSize, Offsets);

  HLNodeUtils::insertAsLastChild(NewLoop, ExpensiveInst->clone());

  auto LastInst = cast<HLInst>(NewLoop->getLastChild());

  LastInst->setLvalDDRef(AllocaRef);

  makeConsistent(AllocaRef, MemRef, NewLoop);

  updateLiveInAllocaTemp(NewLoop, AllocaRef->getBasePtrSymbase());

  // stackrestore intrinsic
  HLInst *Stackrestore =
      insertCallToStackrestore(Lp, Stacksave->getOperandDDRef(0));

  HLNodeUtils::insertAfter(Lp->getParentLoopAtLevel(OutermostLoopLevel),
                           Stackrestore);
  return NewLoop;
}

// Recursively remove the dependent insts in InstsNeedToKeep vector
static void removeDependentInsts(HLInst *HInst, DDGraph &DDG,
                                 SmallVectorImpl<HLInst *> &InstsInExprTree,
                                 SmallSet<HLInst *, 16> &InstsNeedToKeep) {
  unsigned NumOperands = HInst->getNumOperands();

  for (unsigned I = 0, E = NumOperands; I < E; ++I) {
    RegDDRef *RRef = HInst->getOperandDDRef(I);

    for (const DDEdge *Edge : DDG.incoming(RRef)) {
      auto *Src = Edge->getSrc();
      HLInst *SrcInst = dyn_cast<HLInst>(Src->getHLDDNode());

      auto Iter =
          std::find(InstsInExprTree.begin(), InstsInExprTree.end(), SrcInst);

      if (Iter != InstsInExprTree.end()) {
        InstsNeedToKeep.insert(SrcInst);
        removeDependentInsts(SrcInst, DDG, InstsInExprTree, InstsNeedToKeep);
      }
    }
  }
  return;
}

bool HIRStoreResultIntoTempArray::doLoopCarriedScalarReplacement(
    HLLoop *Lp, SmallVectorImpl<HLInst *> &ExpensiveInsts) {
  DDGraph DDG = DDA.getGraph(Lp);
  auto &HNU = Lp->getHLNodeUtils();
  auto &DRU = HNU.getDDRefUtils();

  SmallVector<HLInst *, 16> InstsInExprTree;

  // Get min and max memrefs in the loop
  // Assume the min memref is in the first expression tree
  // and the max memref is in the last expression tree
  collectInstsInExprTree(DDG, ExpensiveInsts.back(), InstsInExprTree);

  RegDDRef *MaxRef = getMemRef(InstsInExprTree);

  InstsInExprTree.clear();

  collectInstsInExprTree(DDG, ExpensiveInsts[0], InstsInExprTree);

  RegDDRef *MinRef = getMemRef(InstsInExprTree);

  // Start to handle the first ExpensiveInst of which expression tree which
  // includes the MinRef This is also the case that we extract the corresponding
  // expression tree to create the extracted loop
  auto LowerTopSortNum = [](const HLInst *HInst1, const HLInst *HInst2) {
    return HInst1->getTopSortNum() < HInst2->getTopSortNum();
  };

  std::sort(InstsInExprTree.begin(), InstsInExprTree.end(), LowerTopSortNum);

  HLInst *Alloca = nullptr;
  unsigned AllocaMemRefSB;
  SmallVector<int64_t, 4> Offsets;

  HLLoop *ExtractedLoop =
      createExtractedLoop(Lp, MaxRef, MinRef, ExpensiveInsts[0],
                          InstsInExprTree, Alloca, AllocaMemRefSB, Offsets);

  // Create a temporary alloca to store the result
  RegDDRef *AllocaDDRef = DRU.createMemRef(
      cast<AllocaInst>(Alloca->getLLVMInstruction())->getAllocatedType(),
      Alloca->getLvalDDRef()->getSingleCanonExpr()->getSingleBlobIndex(),
      Alloca->getNodeLevel(), AllocaMemRefSB);

  // Handle the transformation of the rest of expression trees in the loop
  for (auto It = ExpensiveInsts.begin(), End = ExpensiveInsts.end(); It != End;
       ++It) {
    HLInst *ExpensiveInst = (*It);
    InstsInExprTree.clear();

    collectInstsInExprTree(DDG, ExpensiveInst, InstsInExprTree);

    std::sort(InstsInExprTree.begin(), InstsInExprTree.end(), LowerTopSortNum);

    // Get the MemRef in the expression tree so that we can copy each
    // dimension's CE to the new AllocaRef
    RegDDRef *MemRef = getMemRef(InstsInExprTree);

    uint64_t TypeSize = ExpensiveInst->getLvalDDRef()->getDestTypeSizeInBytes();

    RegDDRef *AllocaRef = addDimensionForAllocaMemRef(
        ExtractedLoop, Lp, AllocaDDRef->clone(), MemRef, TypeSize, Offsets);

    // Create a load inst to replace the expensive inst, like -
    // transfer from -
    //   %call119.us = @pow(%conv118.us,  2.500000e+00);
    // to -
    //   %call119.us = (%TempArray)[i1 + 2][%rem.us][%rem10.us];
    auto LoadInst = HNU.createLoad(AllocaRef, "Load",
                                   ExpensiveInst->getLvalDDRef()->clone());

    HLNodeUtils::insertAfter(ExpensiveInst, LoadInst);

    updateLiveInAllocaTemp(Lp, AllocaRef->getBasePtrSymbase());

    makeConsistent(AllocaRef, MemRef, Lp);

    for (auto Blob :
         make_range(AllocaRef->blob_begin(), AllocaRef->blob_end())) {
      updateLiveInAllocaTemp(Lp, Blob->getSymbase());
    }

    // Collect the dependent insts for the ExpensiveInst
    SmallSet<HLInst *, 16> InstsNeedToKeep;

    for (auto II = Lp->child_begin(), EE = Lp->child_end(); II != EE; ++II) {

      HLInst *HInst = dyn_cast<HLInst>(&*II);

      if (std::find(InstsInExprTree.begin(), InstsInExprTree.end(), HInst) !=
          InstsInExprTree.end()) {
        continue;
      }

      if (HInst == ExpensiveInst) {
        continue;
      }

      removeDependentInsts(HInst, DDG, InstsInExprTree, InstsNeedToKeep);
    }

    for (auto Inst : InstsInExprTree) {
      if (!InstsNeedToKeep.count(Inst)) {
        HLNodeUtils::remove(Inst);
      }
    }
  }

  for (auto ExpensiveInst : ExpensiveInsts) {
    HLNodeUtils::remove(ExpensiveInst);
  }

  return true;
}

// Check whether two loops have the same lower bounds and strides, and store the
// largest loop upperbounds
bool areLoopBoundsConformed(HLLoop *FirstLoop, HLLoop *CurLoop,
                            SmallVectorImpl<RegDDRef *> &LoopUpperBounds) {
  unsigned FirstLoopLevel = FirstLoop->getNestingLevel();
  unsigned CurLoopLevel = CurLoop->getNestingLevel();

  if (FirstLoopLevel != CurLoopLevel) {
    return false;
  }

  unsigned LoopLevel = FirstLoopLevel;
  unsigned OutermostLoopLevel = FirstLoopLevel - NumLoopnestLevel + 1;

  while (LoopLevel >= OutermostLoopLevel) {
    auto *FirstLoopUpperCanonExpr = FirstLoop->getUpperCanonExpr();
    auto *FirstLoopLowerCanonExpr = FirstLoop->getLowerCanonExpr();
    auto *FirstLoopStrideCanonExpr = FirstLoop->getStrideCanonExpr();

    auto *CurLoopUpperCanonExpr = CurLoop->getUpperCanonExpr();
    auto *CurLoopLowerCanonExpr = CurLoop->getLowerCanonExpr();
    auto *CurLoopStrideCanonExpr = CurLoop->getStrideCanonExpr();

    int64_t Dist = 0;

    bool HaveConstDistance = CanonExprUtils::getConstDistance(
        FirstLoopUpperCanonExpr, CurLoopUpperCanonExpr, &Dist);

    if (!(HaveConstDistance ||
          CanonExprUtils::areEqual(FirstLoopUpperCanonExpr,
                                   CurLoopUpperCanonExpr)) ||
        !CanonExprUtils::areEqual(FirstLoopLowerCanonExpr,
                                  CurLoopLowerCanonExpr) ||
        !CanonExprUtils::areEqual(FirstLoopStrideCanonExpr,
                                  CurLoopStrideCanonExpr)) {
      return false;
    }

    if (Dist >= 0) {
      LoopUpperBounds[2 + LoopLevel - FirstLoopLevel] =
          FirstLoop->getUpperDDRef();
    } else {
      LoopUpperBounds[2 + LoopLevel - FirstLoopLevel] =
          CurLoop->getUpperDDRef();
    }

    FirstLoop = FirstLoop->getParentLoop();
    CurLoop = CurLoop->getParentLoop();
    --LoopLevel;
  }

  return true;
}

static bool compareMemRefs(RegDDRef *Ref1, RegDDRef *Ref2,
                           unsigned InnermostLoopLevel) {
  if (!DDRefUtils::haveEqualBaseAndShape(Ref1, Ref2, false)) {
    return false;
  }

  unsigned Level = InnermostLoopLevel - NumLoopnestLevel + 1;

  for (auto Iter1 = Ref1->canon_rbegin(), Iter2 = Ref2->canon_rbegin(),
            End = Ref1->canon_rend();
       Iter1 != End; ++Iter1, ++Iter2, ++Level) {
    CanonExpr *CE1 = *Iter1;
    CanonExpr *CE2 = *Iter2;

    if (CE1->hasIV(Level)) {
      if (!CE2->hasIV(Level)) {
        return false;
      } else {
        int64_t Dist = 0;
        if (!CanonExprUtils::getConstIterationDistance(CE1, CE2, Level, &Dist,
                                                       true)) {
          return false;
        }
      }
    } else {

      if (!CanonExprUtils::areEqual(CE1, CE2)) {
        return false;
      }
    }
  }

  return true;
}

static bool corresponds(RegDDRef *Ref1, RegDDRef *Ref2,
                        unsigned InnermostLoopLevel) {
  if (Ref1->isConstant()) {
    return DDRefUtils::areEqual(Ref1, Ref2);
  }

  if (Ref1->isTerminalRef()) {
    return (Ref1->isSelfBlob() && Ref1->isNonLinear() && Ref2->isSelfBlob() &&
            Ref2->isNonLinear());
  }

  if (!compareRefTypes(Ref1, Ref2)) {
    return false;
  }

  if (!compareMemRefs(Ref1, Ref2, InnermostLoopLevel)) {
    return false;
  }

  return true;
}

static bool corresponds(HLInst *Inst1, HLInst *Inst2, DDGraph &DDG1,
                        DDGraph &DDG2, unsigned InnermostLoopLevel) {
  if (!Inst1->isSameOperationAs(Inst2)) {
    return false;
  }

  auto *RvalIt1 = Inst1->rval_op_ddref_begin();
  auto *RvalIt2 = Inst2->rval_op_ddref_begin();

  for (auto End = Inst1->rval_op_ddref_end(); RvalIt1 != End;
       ++RvalIt1, ++RvalIt2) {
    if (!corresponds(*RvalIt1, *RvalIt2, InnermostLoopLevel)) {
      return false;
    }

    // Give up if ref1 is memref and ref1 and ref2 have any incoming edges
    if ((*RvalIt1)->isMemRef()) {

      if (DDG1.incoming_edges_begin(*RvalIt1) !=
              DDG1.incoming_edges_end(*RvalIt1) ||
          DDG2.incoming_edges_begin(*RvalIt2) !=
              DDG2.incoming_edges_end(*RvalIt2)) {
        return false;
      }

      continue;
    }

    unsigned NumIncomingEdge1 = DDG1.getTotalNumIncomingFlowEdges(*RvalIt1);
    unsigned NumIncomingEdge2 = DDG2.getTotalNumIncomingFlowEdges(*RvalIt2);

    if (NumIncomingEdge1 != NumIncomingEdge2) {
      return false;
    }

    auto EdgeIt1 = DDG1.incoming_edges_begin(*RvalIt1);
    auto EdgeIt2 = DDG2.incoming_edges_begin(*RvalIt2);

    for (auto EndEdge = DDG1.incoming_edges_end(*RvalIt1); EdgeIt1 != EndEdge;
         ++EdgeIt1, ++EdgeIt2) {
      auto *Src1 = (*EdgeIt1)->getSrc();
      auto *Src2 = (*EdgeIt2)->getSrc();

      HLInst *SrcInst1 = cast<HLInst>(Src1->getHLDDNode());
      HLInst *SrcInst2 = cast<HLInst>(Src2->getHLDDNode());

      if (!corresponds(SrcInst1, SrcInst2, DDG1, DDG2, InnermostLoopLevel)) {
        return false;
      }
    }
  }

  if (!compareLvals(Inst1, Inst2)) {
    return false;
  }

  return true;
}

bool HIRStoreResultIntoTempArray::isLegalForBulkLoopCarriedScalarReplacement(
    LpExpensiveInstsPairsTy &LpExpensiveInstsPairs,
    SmallVectorImpl<RegDDRef *> &LoopUpperBounds) {
  if (LpExpensiveInstsPairs.size() < NumLoopsForBulkTransform) {
    return false;
  }

  auto It1 = LpExpensiveInstsPairs.begin();
  HLLoop *FirstLoop = (*It1).first;

  if (!FirstLoop->getParentRegion()->isFunctionLevel()) {
    return false;
  }

  unsigned InnermostLoopLevel = FirstLoop->getNestingLevel();
  unsigned OutermostLoopLevel = InnermostLoopLevel - NumLoopnestLevel + 1;
  auto *CurNode = FirstLoop->getParentLoopAtLevel(OutermostLoopLevel);
  HLInst *FirstExpensiveInst = (*((*It1).second).begin());
  DDGraph DDG1 = DDA.getGraph(FirstLoop);

  HLLoop *CurLoop = nullptr;
  HLInst *CurExpensiveInst = nullptr;

  for (auto It2 = std::next(LpExpensiveInstsPairs.begin());
       It2 != LpExpensiveInstsPairs.end(); ++It2) {
    CurLoop = (*It2).first;
    CurExpensiveInst = (*((*It2).second).begin());
    auto *CurOutermostLoop = CurLoop->getParentLoopAtLevel(OutermostLoopLevel);

    // Make sure there is no labels or branches among loops
    if (!HLNodeUtils::postDominates(CurOutermostLoop, CurNode) ||
        !HLNodeUtils::dominates(CurNode, CurOutermostLoop)) {
      return false;
    }

    CurNode = CurOutermostLoop;

    if (!areLoopBoundsConformed(FirstLoop, CurLoop, LoopUpperBounds)) {
      return false;
    }

    DDGraph DDG2 = DDA.getGraph(CurLoop);

    if (!corresponds(FirstExpensiveInst, CurExpensiveInst, DDG1, DDG2,
                     InnermostLoopLevel)) {
      return false;
    }
  }

  return true;
}

static HLLoop *
createExtractedLoopNest(HLLoop *Lp, HLLoop *NewLoop, RegDDRef *MemRef,
                        SmallVectorImpl<RegDDRef *> &LoopUpperBounds,
                        SmallVectorImpl<CanonExpr *> &DistsBetweenMemRefs) {
  unsigned InnermostLoopLevel = Lp->getNestingLevel();
  unsigned OutermostLoopLevel = InnermostLoopLevel - NumLoopnestLevel + 1;

  HLLoop *ParentLoop = nullptr;
  CanonExpr *MemCanonExpr = nullptr;
  CanonExpr *MemRefLowerDimCE = nullptr;
  unsigned NumDimension = MemRef->getNumDimensions();
  unsigned Dim = 1;

  for (unsigned Id = NumLoopnestLevel - 1, LoopLevel = InnermostLoopLevel;
       LoopLevel >= OutermostLoopLevel; --LoopLevel, --Id) {
    do {
      MemCanonExpr = MemRef->getDimensionIndex(Dim);
      MemRefLowerDimCE = MemRef->getDimensionLower(Dim);
      Dim++;
    } while (MemCanonExpr->isConstant() && Dim <= NumDimension);

    if (LoopLevel != InnermostLoopLevel) {
      // Clone the empty parent loop nest for the new loop's parent loops
      Lp = Lp->getParentLoop();
      ParentLoop = Lp->cloneEmpty();

      ParentLoop->setUpperDDRef(LoopUpperBounds[Id]->clone());
      CanonExprUtils::add(ParentLoop->getUpperCanonExpr(),
                          DistsBetweenMemRefs[Id], true);

      HLNodeUtils::insertAsFirstChild(ParentLoop, NewLoop);
      NewLoop = NewLoop->getParentLoop();
    } else {
      NewLoop->setUpperDDRef(LoopUpperBounds[Id]->clone());
      CanonExprUtils::add(NewLoop->getUpperCanonExpr(), DistsBetweenMemRefs[Id],
                          true);
    }

    if (!MemRefLowerDimCE->isZero()) {
      CanonExprUtils::add(NewLoop->getUpperCanonExpr(), MemRefLowerDimCE, true);
    }
  }

  return ParentLoop;
}

HLLoop *
HIRStoreResultIntoTempArray::createExtractedLoopWithLargestLoopUpperBounds(
    HLLoop *FirstLoop, HLLoop *LoopWithMinRef, RegDDRef *MemRef,
    HLInst *ExpensiveInstForExtractedLoop,
    SmallVectorImpl<RegDDRef *> &LoopUpperBounds,
    SmallVectorImpl<CanonExpr *> &DistsBetweenMemRefs,
    SmallVectorImpl<HLInst *> &InstsInExprTree, HLInst *&Alloca,
    unsigned &AllocaMemRefSB, SmallVectorImpl<int64_t> &Offsets) {
  unsigned OuterLoopLevel =
      LoopWithMinRef->getNestingLevel() - NumLoopnestLevel + 1;
  HLLoop *OuterAnchorLp = FirstLoop->getParentLoopAtLevel(OuterLoopLevel);

  // Create the extracted loop
  HLLoop *NewLoop = LoopWithMinRef->cloneEmpty();

  HLLoop *NewOuterLoop = createExtractedLoopNest(
      LoopWithMinRef, NewLoop, MemRef, LoopUpperBounds, DistsBetweenMemRefs);

  HLNodeUtils::insertBefore(OuterAnchorLp, NewOuterLoop);

  // Insert the inst seqences
  for (auto Inst : InstsInExprTree) {
    HLInst *InstClone = Inst->clone();
    HLNodeUtils::insertAsLastChild(NewLoop, InstClone);
    updateLiveInAllocaTemp(NewLoop, InstClone->getLvalDDRef()->getSymbase());
  }

  SmallVector<HLInst *, 8> ArraySizeInsts;
  SmallVector<RegDDRef *, 8> TripCountRefs;

  // Create Alloca inst, like %TempArray = alloca %array_size5
  Alloca = createAllocaInst(
      MemRef, NewLoop,
      ExpensiveInstForExtractedLoop->getLvalDDRef()->getDestType(),
      ArraySizeInsts, TripCountRefs, Offsets);

  HLInst *AnchorForArraySizeInsts = nullptr;

  // Insert array size insts as last preheader of the loop
  for (unsigned I = 0; I < ArraySizeInsts.size(); ++I) {
    if (I == 0) {
      HLNodeUtils::insertAsFirstChild(NewLoop->getParentRegion(),
                                      ArraySizeInsts[I]);
    } else {
      HLNodeUtils::insertAfter(AnchorForArraySizeInsts, ArraySizeInsts[I]);
    }
    updateLiveInArraySize(NewLoop,
                          ArraySizeInsts[I]->getLvalDDRef()->getSymbase());

    AnchorForArraySizeInsts = ArraySizeInsts[I];
  }

  auto &HNU = NewLoop->getHLNodeUtils();
  auto &DRU = HNU.getDDRefUtils();

  // Update live in of the trip count refs in the array size insts
  for (auto TripCountRef : TripCountRefs) {
    TripCountRef->makeConsistent();
    updateLiveInForBlobs(TripCountRef, NewLoop);
  }

  HNU.insertAfter(AnchorForArraySizeInsts, Alloca);

  RegDDRef *AllocaLval = Alloca->getLvalDDRef();

  updateLiveInArraySize(NewLoop, AllocaLval->getSymbase());

  updateLiveInForBlobs(AllocaLval, NewLoop);

  // Create a memref using AllocaRef and update the expensive inst's lval
  RegDDRef *AllocaDDRef =
      DRU.createMemRef(cast<AllocaInst>(Alloca->getLLVMInstruction())->getAllocatedType(), AllocaLval->getSingleCanonExpr()->getSingleBlobIndex(),
                       Alloca->getNodeLevel());

  AllocaMemRefSB = AllocaDDRef->getSymbase();

  RegDDRef *AllocaDDRefClone = AllocaDDRef->clone();

  uint64_t TypeSize =
      ExpensiveInstForExtractedLoop->getLvalDDRef()->getDestTypeSizeInBytes();

  RegDDRef *AllocaRef = addDimensionForAllocaMemRef(
      NewLoop, NewLoop, AllocaDDRefClone, MemRef, TypeSize, Offsets);

  HLNodeUtils::insertAsLastChild(NewLoop,
                                 ExpensiveInstForExtractedLoop->clone());

  auto LastInst = cast<HLInst>(NewLoop->getLastChild());

  LastInst->setLvalDDRef(AllocaRef);

  makeConsistent(AllocaRef, MemRef, NewLoop);

  updateLiveInAllocaTemp(NewLoop, AllocaRef->getBasePtrSymbase());

  return NewLoop;
}

// Get the distances between MinRef and MaxRef and return the loop with MinRef
HLLoop *HIRStoreResultIntoTempArray::getDistsBetweenMinRefAndMaxRef(
    LpExpensiveInstsPairsTy &LpExpensiveInstsPairs,
    SmallVectorImpl<CanonExpr *> &DistsBetweenMemRefs) {
  SmallVector<HLInst *, 16> InstsInExprTree;
  SmallVector<std::pair<RegDDRef *, HLLoop *>, 16> MemRefLoopPairs;

  for (auto &LpInstPair : LpExpensiveInstsPairs) {
    HLLoop *Lp = LpInstPair.first;
    SmallVector<HLInst *, 16> ExpensiveInsts = LpInstPair.second;
    DDGraph DDG = DDA.getGraph(Lp);

    for (auto ExpensiveInst : ExpensiveInsts) {
      InstsInExprTree.clear();
      collectInstsInExprTree(DDG, ExpensiveInst, InstsInExprTree);
      RegDDRef *MemRef = getMemRef(InstsInExprTree);
      MemRefLoopPairs.push_back(std::make_pair(MemRef, Lp));
    }
  }

  RegDDRef *MinRef = nullptr;
  HLLoop *LoopWithMinRef = nullptr;
  std::tie(MinRef, LoopWithMinRef) = *(MemRefLoopPairs.begin());

  unsigned NumDimension = MinRef->getNumDimensions();

  for (auto I = std::next(MemRefLoopPairs.begin()), End = MemRefLoopPairs.end();
       I != End; ++I) {
    RegDDRef *CurRef = nullptr;
    HLLoop *LoopWithCurRef = nullptr;
    std::tie(CurRef, LoopWithCurRef) = *(I);

    unsigned Dim = 1;
    CanonExpr *MinCanonExpr = nullptr;
    CanonExpr *CurCanonExpr = nullptr;
    CanonExpr *MinCE = nullptr;
    CanonExpr *CurCE = nullptr;

    for (int Index = NumLoopnestLevel - 1; Index >= 0; --Index) {
      do {
        MinCanonExpr = MinRef->getDimensionIndex(Dim);
        CurCanonExpr = CurRef->getDimensionIndex(Dim);
        Dim++;
      } while (MinCanonExpr->isConstant() && Dim <= NumDimension);

      MinCE = MinCanonExpr->clone();
      CurCE = CurCanonExpr->clone();

      if (CurCE->hasBlob()) {
        continue;
      }

      bool Subtracted = CanonExprUtils::subtract(CurCE, MinCE, true);

      if (!Subtracted) {
        continue;
      }

      if (CurCE->getConstant() < 0) {
        LoopWithMinRef = LoopWithCurRef;
        MinRef = CurRef;
        CurCE->negate();
      }

      if (!DistsBetweenMemRefs[Index]) {
        DistsBetweenMemRefs[Index] = CurCE;
      } else {
        DistsBetweenMemRefs[Index] =
            CurCE->getConstant() > DistsBetweenMemRefs[Index]->getConstant()
                ? CurCE
                : DistsBetweenMemRefs[Index];
      }
    }
  }
  return LoopWithMinRef;
}

bool HIRStoreResultIntoTempArray::doBulkLoopCarriedScalarReplacement(
    LpExpensiveInstsPairsTy &LpExpensiveInstsPairs,
    SmallVectorImpl<RegDDRef *> &LoopUpperBounds) {
  SmallVector<CanonExpr *, NumLoopnestLevel> DistsBetweenMemRefs = {
      nullptr, nullptr, nullptr};

  // Get the largest constant distances for each dimensions between minref and
  // maxref and return the loop with minref. The first expression tree in this
  // loop will be used to create the extracted loop.
  HLLoop *LoopWithMinRef = getDistsBetweenMinRefAndMaxRef(LpExpensiveInstsPairs,
                                                          DistsBetweenMemRefs);

  auto It = LpExpensiveInstsPairs.begin();
  HLLoop *FirstLoop = (*It).first;

  HLInst *ExpensiveInstForExtractedLoop = nullptr;

  for (auto &LpInstPair : LpExpensiveInstsPairs) {
    if (LpInstPair.first == LoopWithMinRef) {
      ExpensiveInstForExtractedLoop = *(LpInstPair.second.begin());
    }
  }
  assert(ExpensiveInstForExtractedLoop && "HLInst is expected");

  DDGraph DDG = DDA.getGraph(LoopWithMinRef);
  auto &HNU = LoopWithMinRef->getHLNodeUtils();
  auto &DRU = HNU.getDDRefUtils();
  SmallVector<HLInst *, 16> InstsInExprTree;

  collectInstsInExprTree(DDG, ExpensiveInstForExtractedLoop, InstsInExprTree);

  RegDDRef *MemRef = getMemRef(InstsInExprTree);

  // Start to handle the first ExpensiveInst of which expression tree which
  // includes the MinRef This is also the case that we extract the corresponding
  // expression tree to create the extracted loop
  auto LowerTopSortNum = [](const HLInst *HInst1, const HLInst *HInst2) {
    return HInst1->getTopSortNum() < HInst2->getTopSortNum();
  };

  std::sort(InstsInExprTree.begin(), InstsInExprTree.end(), LowerTopSortNum);

  HLInst *Alloca = nullptr;
  unsigned AllocaMemRefSB;
  SmallVector<int64_t, 4> Offsets;

  HLLoop *ExtractedLoop = createExtractedLoopWithLargestLoopUpperBounds(
      FirstLoop, LoopWithMinRef, MemRef, ExpensiveInstForExtractedLoop,
      LoopUpperBounds, DistsBetweenMemRefs, InstsInExprTree, Alloca,
      AllocaMemRefSB, Offsets);

  // Create a temporary alloca to store the result
  RegDDRef *AllocaDDRef = DRU.createMemRef(
      cast<AllocaInst>(Alloca->getLLVMInstruction())->getAllocatedType(),
      Alloca->getLvalDDRef()->getSingleCanonExpr()->getSingleBlobIndex(),
      Alloca->getNodeLevel(), AllocaMemRefSB);

  // Handle the transformation of the rest of expression trees in the loop
  for (auto &LpInstPair : LpExpensiveInstsPairs) {
    HLLoop *Lp = LpInstPair.first;
    SmallVector<HLInst *, 16> ExpensiveInsts = LpInstPair.second;
    DDG = DDA.getGraph(Lp);

    for (auto ExpensiveInst : ExpensiveInsts) {

      InstsInExprTree.clear();

      collectInstsInExprTree(DDG, ExpensiveInst, InstsInExprTree);

      std::sort(InstsInExprTree.begin(), InstsInExprTree.end(),
                LowerTopSortNum);

      // Get the MemRef in the expression tree so that we can copy each
      // dimension's CE to the new AllocaRef
      RegDDRef *MemRef = getMemRef(InstsInExprTree);

      uint64_t TypeSize =
          ExpensiveInst->getLvalDDRef()->getDestTypeSizeInBytes();

      RegDDRef *AllocaDDRefClone = AllocaDDRef->clone();

      RegDDRef *AllocaRef = addDimensionForAllocaMemRef(
          ExtractedLoop, Lp, AllocaDDRefClone, MemRef, TypeSize, Offsets);

      // Create a load inst to replace the expensive inst, like -
      // transfer from -
      //   %call119.us = @pow(%conv118.us,  2.500000e+00);
      // to -
      //   %call119.us = (%TempArray)[i1 + 2][%rem.us][%rem10.us];
      auto LoadInst = HNU.createLoad(AllocaRef, "Load",
                                     ExpensiveInst->getLvalDDRef()->clone());

      HLNodeUtils::insertAfter(ExpensiveInst, LoadInst);

      updateLiveInAllocaTemp(Lp, AllocaRef->getBasePtrSymbase());

      makeConsistent(AllocaRef, MemRef, ExtractedLoop);

      for (auto Blob :
           make_range(AllocaRef->blob_begin(), AllocaRef->blob_end())) {
        updateLiveInAllocaTemp(Lp, Blob->getSymbase());
      }

      // Collect the dependent insts for the ExpensiveInst
      SmallSet<HLInst *, 16> InstsNeedToKeep;

      for (auto II = Lp->child_begin(), EE = Lp->child_end(); II != EE; ++II) {

        HLInst *HInst = dyn_cast<HLInst>(&*II);

        if (std::find(InstsInExprTree.begin(), InstsInExprTree.end(), HInst) !=
            InstsInExprTree.end()) {
          continue;
        }

        if (HInst == ExpensiveInst) {
          continue;
        }

        removeDependentInsts(HInst, DDG, InstsInExprTree, InstsNeedToKeep);
      }

      for (auto Inst : InstsInExprTree) {
        if (!InstsNeedToKeep.count(Inst)) {
          HLNodeUtils::remove(Inst);
        }
      }
    }

    for (auto ExpensiveInst : ExpensiveInsts) {
      HLNodeUtils::remove(ExpensiveInst);
    }
  }

  return true;
}

static void setInvalidate(HLLoop *Lp) {
  Lp->addInt32LoopMetadata("intel.loop.distribute.loopnest.enable", 1);

  HIRInvalidationUtils::invalidateBody(Lp);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(
      Lp->getParentLoopAtLevel(Lp->getNestingLevel() - NumLoopnestLevel + 1));
  return;
}

bool HIRStoreResultIntoTempArray::run() {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Store Result Into Temp Array Disabled \n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR Store Result Into Temp Array on Function : "
                    << HIRF.getFunction().getName() << "\n");

  // Gather all inner-most loops as Candidates
  SmallVector<HLLoop *, 64> InnermostLoops;

  HLNodeUtils &HNU = HIRF.getHLNodeUtils();
  HNU.gatherInnermostLoops(InnermostLoops);

  if (InnermostLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << "() has no inner-most loop\n ");
    return false;
  }

  bool Result = false;

  // The vector collects the expensive insts
  SmallVector<HLInst *, 16> ExpensiveInsts;
  LpExpensiveInstsPairsTy LpExpensiveInstsPairs;

  for (auto *Lp : InnermostLoops) {
    ExpensiveInsts.clear();

    if (isLegalForLoopCarriedScalarReplacement(Lp, ExpensiveInsts)) {
      LpExpensiveInstsPairs.push_back(std::make_pair(Lp, ExpensiveInsts));
    }
  }

  SmallVector<RegDDRef *, NumLoopnestLevel> LoopUpperBounds = {nullptr, nullptr,
                                                               nullptr};
  SmallVector<HLLoop *> InvalidateLoops;

  bool Transformed = false;

  if (isLegalForBulkLoopCarriedScalarReplacement(LpExpensiveInstsPairs,
                                                 LoopUpperBounds)) {
    Transformed = doBulkLoopCarriedScalarReplacement(LpExpensiveInstsPairs,
                                                     LoopUpperBounds);

    if (Transformed) {
      (LpExpensiveInstsPairs[0].first)->getParentRegion()->setGenCode();

      for (auto &LpInstPair : LpExpensiveInstsPairs) {
        HLLoop *Lp = LpInstPair.first;
        InvalidateLoops.push_back(Lp);
      }
    }

    Result = Result || Transformed;
  } else {
    for (auto &LpInstPair : LpExpensiveInstsPairs) {
      HLLoop *Lp = LpInstPair.first;
      ExpensiveInsts = LpInstPair.second;
      Transformed = doLoopCarriedScalarReplacement(Lp, ExpensiveInsts);

      if (Transformed) {
        Lp->getParentRegion()->setGenCode();
        InvalidateLoops.push_back(Lp);
      }

      Result = Result || Transformed;
    }
  }

  for (auto *Lp : InvalidateLoops) {
    setInvalidate(Lp);
  }

  return Result;
}

PreservedAnalyses HIRStoreResultIntoTempArrayPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  ModifiedHIR =
      HIRStoreResultIntoTempArray(HIRF, AM.getResult<HIRDDAnalysisPass>(F))
          .run();
  return PreservedAnalyses::all();
}

class HIRStoreResultIntoTempArrayLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRStoreResultIntoTempArrayLegacyPass() : HIRTransformPass(ID) {
    initializeHIRStoreResultIntoTempArrayLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRStoreResultIntoTempArray(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA())
        .run();
  }
};

char HIRStoreResultIntoTempArrayLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRStoreResultIntoTempArrayLegacyPass, OPT_SWITCH,
                      OPT_DESC, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRStoreResultIntoTempArrayLegacyPass, OPT_SWITCH, OPT_DESC,
                    false, false)

FunctionPass *llvm::createHIRStoreResultIntoTempArrayPass() {
  return new HIRStoreResultIntoTempArrayLegacyPass();
}

//===-HIRStoreResultIntoTempArray.cpp Implements Store Result Into Temp Array class --===//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
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

namespace {

typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;

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

  bool
  doLoopCarriedScalarReplacement(HLLoop *Lp,
                                 SmallVectorImpl<HLInst *> &ExpensiveInsts);

  bool hasSameExprTree(unsigned LoopLevel, DDGraph &DDG,
                       SmallVectorImpl<HLInst *> &ExpensiveInsts);

  void collectInstsInExprTree(DDGraph &DDG, HLInst *HInst,
                              SmallVectorImpl<HLInst *> &InstsInExprTree);

  HLLoop *createExtractedLoop(HLLoop *Lp, RegDDRef *MaxRef, RegDDRef *MinRef,
                              HLInst *ExpensiveInst,
                              SmallVectorImpl<HLInst *> &InstsInExprTree,
                              HLInst *&AllocaInst,
                              SmallVectorImpl<int64_t> &Offsets);

  RegDDRef *addDimensionForAllocaMemRef(const HLLoop *ExtractedLoop,
                                        const HLLoop *Lp,
                                        const RegDDRef *AllocaRef,
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
                           unsigned LoopLevel) {
  // We check:
  // 1) Ref1 does not have any blobs
  // 2) All its indices are either constant or linear
  // 3) The IV levels should be in increasing order
  // Ref1 should have the format, like %A[i1+1][i2][i3][0]
  // Ref2 might have the format, like %A[i1+2[%rem][%rem6+1]
  unsigned Dim = 1;
  unsigned Level = 0;

  for (auto Iter = Ref1->canon_rbegin(), End = Ref1->canon_rend(); Iter != End;
       ++Iter, ++Dim) {

    CanonExpr *CE = *Iter;
    if (CE->hasBlob()) {
      return false;
    }

    switch (Dim) {
    case 1:
      if (!CE->hasIV(Dim)) {
        return false;
      }
      break;
    case 2:
      if (!CE->isStandAloneIV(false, &Level) && Level == Dim) {
        return false;
      }
      break;
    case 3:
      if (!CE->isStandAloneIV(false, &Level) && Level == Dim) {
        return false;
      }
      break;
    case 4:
      if (!CE->isConstant()) {
        return false;
      }
      break;
    default:
      return false;
    }
  }

  if (!CanonExprUtils::areEqual(Ref1->getBaseCE(), Ref2->getBaseCE(), false)) {
    return false;
  }

  Level = 1;

  for (auto Iter1 = Ref1->canon_rbegin(), Iter2 = Ref2->canon_rbegin(),
            End = Ref1->canon_rend();
       Iter1 != End; ++Iter1, ++Iter2, ++Level) {
    CanonExpr *CE1 = *Iter1;
    CanonExpr *CE2 = *Iter2;

    if (CE1->hasIV(Level)) {

      if (CE2->hasBlob()) {
        if (CE2->numBlobs() != 1) {
          return false;
        }

        SmallVector<unsigned, 4> BlobIndices;

        CE2->collectTempBlobIndices(BlobIndices);

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
        if (!(CanonExprUtils::getConstIterationDistance(CE1, CE2, Level,
                                                        &Dist) ||
              CanonExprUtils::areEqual(CE1, CE2, true))) {
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

// Compare whether the two Refs are corresponding
static bool corresponds(RegDDRef *Ref1, RegDDRef *Ref2, DDGraph &DDG,
                        unsigned LoopLevel) {
  if (!((Ref1->isTerminalRef() && Ref2->isTerminalRef()) ||
        (Ref1->isMemRef() && Ref2->isMemRef()))) {
    return false;
  }

  if (Ref1->getNumDimensions() != Ref2->getNumDimensions()) {
    return false;
  }

  if (Ref1->isConstant()) {
    return DDRefUtils::areEqual(Ref1, Ref2);
  }

  if (Ref1->isTerminalRef()) {
    return (Ref1->isSelfBlob() && Ref1->isNonLinear() && Ref2->isSelfBlob() &&
            Ref2->isNonLinear());
  }

  if (!compareMemRefs(Ref1, Ref2, DDG, LoopLevel)) {
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
                        unsigned LoopLevel) {
  auto *LLVMInst1 = Inst1->getLLVMInstruction();
  auto *LLVMInst2 = Inst2->getLLVMInstruction();

  if (LLVMInst1->getOpcode() != LLVMInst2->getOpcode()) {
    return false;
  }

  if (auto *FPInst1 = dyn_cast<FPMathOperator>(LLVMInst1)) {
    auto *FPInst2 = dyn_cast<FPMathOperator>(LLVMInst2);

    if (!FPInst2 || (FPInst1->isFast() != FPInst2->isFast())) {
      return false;
    }
  }

  unsigned NumOperands1 = Inst1->getNumOperands();
  unsigned NumOperands2 = Inst2->getNumOperands();

  if (NumOperands1 != NumOperands2) {
    return false;
  }

  auto *RvalIt1 = Inst1->rval_op_ddref_begin();
  auto *RvalIt2 = Inst2->rval_op_ddref_begin();

  for (auto End = Inst1->rval_op_ddref_end(); RvalIt1 != End;
       ++RvalIt1, ++RvalIt2) {
    if (!corresponds(*RvalIt1, *RvalIt2, DDG, LoopLevel)) {
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

      if (!corresponds(SrcInst1, SrcInst2, DDG, LoopLevel)) {
        return false;
      }
    }
  }

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

// Check whether the expensive insts have the same expression tree in the loop
// Collect all the insts for each expensive inst, then compare the sequences
bool HIRStoreResultIntoTempArray::hasSameExprTree(
    unsigned LoopLevel, DDGraph &DDG,
    SmallVectorImpl<HLInst *> &ExpensiveInsts) {
  HLInst *FirstInst = *(ExpensiveInsts.begin());

  for (auto It = std::next(ExpensiveInsts.begin()), End = ExpensiveInsts.end();
       It != End; ++It) {
    HLInst *CurInst = *It;

    if (!corresponds(FirstInst, CurInst, DDG, LoopLevel)) {
      return false;
    }
  }

  return true;
}

static bool isLoopnestValid(const HLLoop *Lp) {
  do {
    if (!Lp->isDo() || !Lp->isNormalized()) {
      return false;
    }

    const CanonExpr *UpperCE = Lp->getUpperCanonExpr();

    if (!UpperCE->canConvertToStandAloneBlob() ||
        UpperCE->getDenominator() != 1) {
      return false;
    }
  } while (Lp = Lp->getParentLoop());

  return true;
}

bool HIRStoreResultIntoTempArray::isLegalForLoopCarriedScalarReplacement(
    HLLoop *Lp, SmallVectorImpl<HLInst *> &ExpensiveInsts) {

  unsigned LoopLevel = Lp->getNestingLevel();

  if (LoopLevel != 3) {
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

  DDG = DDA.getGraph(Lp->getOutermostParentLoop());

  if (!hasSameExprTree(LoopLevel, DDG, ExpensiveInsts)) {
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
static HLInst *createAllocaInst(DDGraph DDG, RegDDRef *MinRef, RegDDRef *MaxRef,
                                HLLoop *Lp, Type *DestTy,
                                SmallVectorImpl<HLInst *> &ArraySizeInsts,
                                SmallVectorImpl<RegDDRef *> &TripCountRefs,
                                SmallVectorImpl<int64_t> &Offsets) {
  auto &HNU = Lp->getHLNodeUtils();
  unsigned LoopLevel = Lp->getNestingLevel();
  RegDDRef *TripCountRef = Lp->getTripCountDDRef();

  // Update the array size by the offset of iv
  recordOffsets(MinRef, LoopLevel, Offsets);

  TripCountRefs.push_back(TripCountRef);

  HLInst *ArraySizeMulInst = nullptr;

  while (Lp = Lp->getParentLoop()) {
    RegDDRef *ParentTripCountRef = Lp->getTripCountDDRef();
    LoopLevel = Lp->getNestingLevel();
    recordOffsets(MinRef, LoopLevel, Offsets);

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
    TripCountCE->convertToStandAloneBlob();

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
    const HLLoop *ExtractedLoop, const HLLoop *Lp, const RegDDRef *AllocaRef,
    const RegDDRef *MemRef, uint64_t TypeSize,
    SmallVectorImpl<int64_t> &Offsets) {
  unsigned LoopLevel = Lp->getNestingLevel();
  DDGraph DDG = DDA.getGraph(Lp->getOutermostParentLoop());
  RegDDRef *AllocaRefClone = AllocaRef->clone();

  // A set collects the CE with blob refs that we also handled, like-
  // %t1 = i1 % nx;
  // %t2 = i2 % ny;
  // ... = %A[%t1][%t2];
  SmallSet<const CanonExpr *, 8> CEsChecked;

  for (unsigned I = 1; I <= LoopLevel; ++I) {

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

      CloneCE->addConstant(-Offsets[I - 1], true);

      CanonExpr *StrideCE = getStrideCE(ExtractedLoop, TypeSize, I);
      Type *DimTy = AllocaRef->getBaseType();

      AllocaRefClone->addDimension(CloneCE, {}, nullptr, StrideCE, DimTy);

      break;
    }
  }

  return AllocaRefClone;
}

static void updateLiveInAllocaTemp(HLLoop *Loop, unsigned SB) {
  HLLoop *Lp = Loop;
  while (Lp) {
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

  HLLoop *ParentLoop = nullptr;
  CanonExpr *MinCanonExpr = nullptr;
  CanonExpr *MaxCanonExpr = nullptr;
  CanonExpr *MinCE = nullptr;
  CanonExpr *MaxCE = nullptr;
  unsigned NumDimension = MinRef->getNumDimensions();
  unsigned Dim = 1;

  while (LoopLevel >= 1) {
    bool HasSelfBlob = false;
    bool Subtracted = true;

    do {
      MinCanonExpr = MinRef->getDimensionIndex(Dim);
      MaxCanonExpr = MaxRef->getDimensionIndex(Dim);
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

    LoopLevel--;
  }

  return ParentLoop;
}

static HLInst *insertCallToStacksave(HLLoop *Lp) {
  const DebugLoc &DLoc = Lp->getDebugLoc();
  auto &HNU = Lp->getHLNodeUtils();

  HLInst *StacksaveCall = HNU.createStacksave(DLoc);
  HLNodeUtils::insertBefore(Lp->getOutermostParentLoop(), StacksaveCall);

  return StacksaveCall;
}

static void insertCallToStackrestore(HLLoop *Lp, RegDDRef *StackAddrRef) {
  auto &HNU = Lp->getHLNodeUtils();
  auto &DRU = HNU.getDDRefUtils();
  auto &CEU = HNU.getCanonExprUtils();

  Type *Ty = StackAddrRef->getDestType();
  StackAddrRef = DRU.createAddressOfRef(StackAddrRef->getSelfBlobIndex(),
                                        StackAddrRef->getDefinedAtLevel(),
                                        StackAddrRef->getSymbase(), true);

  StackAddrRef->addDimension(CEU.createCanonExpr(
      cast<PointerType>(Ty)->getElementType(), APInt(8, 0)));

  HLInst *StackrestoreCall = HNU.createStackrestore(StackAddrRef);
  HLNodeUtils::insertAfter(Lp->getOutermostParentLoop(), StackrestoreCall);
}

static void makeConsistent(RegDDRef *AllocaRef, RegDDRef *MemRef, HLLoop *Lp) {

  SmallVector<RegDDRef *, 4> AuxRefs;

  AuxRefs.push_back(MemRef);

  while (Lp) {
    AuxRefs.push_back(Lp->getUpperDDRef());
    Lp = Lp->getParentLoop();
  }

  AllocaRef->makeConsistent(AuxRefs);
}

// Create a new loop and insert the insts in the expression tree into the new
// loop
HLLoop *HIRStoreResultIntoTempArray::createExtractedLoop(
    HLLoop *Lp, RegDDRef *MaxRef, RegDDRef *MinRef, HLInst *ExpensiveInst,
    SmallVectorImpl<HLInst *> &InstsInExprTree, HLInst *&AllocaInst,
    SmallVectorImpl<int64_t> &Offsets) {

  HLLoop *OuterAnchorLp = Lp->getOutermostParentLoop();

  // Create the extracted loop
  HLLoop *NewLoop = Lp->cloneEmpty();

  DDGraph DDG = DDA.getGraph(OuterAnchorLp);

  HLLoop *NewOuterLoop =
      createExtractedLoopNest(DDG, Lp, NewLoop, MaxRef, MinRef);

  HLNodeUtils::insertBefore(OuterAnchorLp, NewOuterLoop);

  // stacksave intrinsicc
  HLInst *Stacksave = insertCallToStacksave(NewLoop);

  // Insert the inst seqences
  for (auto Inst : InstsInExprTree) {
    HLInst *InstClone = Inst->clone();
    HLNodeUtils::insertAsLastChild(NewLoop, InstClone);
    updateLiveInAllocaTemp(NewLoop, InstClone->getLvalDDRef()->getSymbase());
  }

  SmallVector<HLInst *, 8> ArraySizeInsts;
  SmallVector<RegDDRef *, 8> TripCountRefs;

  // Create Alloca inst, like %TempArray = alloca %array_size5
  AllocaInst = createAllocaInst(DDG, MinRef, MaxRef, NewLoop,
                                ExpensiveInst->getLvalDDRef()->getDestType(),
                                ArraySizeInsts, TripCountRefs, Offsets);

  HLLoop *NewOutermostLoop = NewLoop->getOutermostParentLoop();

  // Insert array size insts as last preheader of the loop
  for (auto ArraySizeInst : ArraySizeInsts) {
    HLNodeUtils::insertBefore(NewOutermostLoop, ArraySizeInst);
    updateLiveInAllocaTemp(NewLoop,
                           ArraySizeInst->getLvalDDRef()->getSymbase());
  }

  auto &HNU = NewLoop->getHLNodeUtils();
  auto &DRU = HNU.getDDRefUtils();
  auto &BU = HNU.getBlobUtils();

  // Update live in of the trip count refs in the array size insts
  for (auto TripCountRef : TripCountRefs) {
    TripCountRef->makeConsistent();
    unsigned BlobIndex =
        TripCountRef->getSingleCanonExpr()->getSingleBlobIndex();
    SmallVector<unsigned, 4> TempBlobIndices;

    BU.collectTempBlobs(BlobIndex, TempBlobIndices);

    for (auto BI : TempBlobIndices) {
      updateLiveInAllocaTemp(NewLoop, BU.getTempBlobSymbase(BI));
    }
  }

  HNU.insertBefore(NewOutermostLoop, AllocaInst);

  updateLiveInAllocaTemp(NewLoop, AllocaInst->getLvalDDRef()->getSymbase());

  // Create a memref using AllocaRef and update the expensive inst's lval
  RegDDRef *AllocaDDRef = DRU.createMemRef(
      AllocaInst->getLvalDDRef()->getSingleCanonExpr()->getSingleBlobIndex());

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
  insertCallToStackrestore(Lp, Stacksave->getOperandDDRef(0));

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

  HLInst *AllocaInst = nullptr;
  SmallVector<int64_t, 4> Offsets;

  HLLoop *ExtractedLoop =
      createExtractedLoop(Lp, MaxRef, MinRef, ExpensiveInsts[0],
                          InstsInExprTree, AllocaInst, Offsets);

  // Create a temporary alloca to store the result
  RegDDRef *AllocaDDRef = DRU.createMemRef(
      AllocaInst->getLvalDDRef()->getSingleCanonExpr()->getSingleBlobIndex());

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

  for (auto *Lp : InnermostLoops) {
    bool Transformed = false;
    ExpensiveInsts.clear();

    if (isLegalForLoopCarriedScalarReplacement(Lp, ExpensiveInsts)) {
      Transformed = doLoopCarriedScalarReplacement(Lp, ExpensiveInsts);
      Result = Result || Transformed;
    }

    if (Transformed) {
      Lp->addInt32LoopMetadata("intel.loop.distribute.loopnest.enable", 1);

      Lp->getParentRegion()->setGenCode();
      HIRInvalidationUtils::invalidateBody(Lp);
      HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(
          Lp->getOutermostParentLoop());
    }
  }

  return Result;
}

PreservedAnalyses
HIRStoreResultIntoTempArrayPass::run(llvm::Function &F,
                                     llvm::FunctionAnalysisManager &AM) {
  HIRStoreResultIntoTempArray(AM.getResult<HIRFrameworkAnalysis>(F),
                              AM.getResult<HIRDDAnalysisPass>(F))
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

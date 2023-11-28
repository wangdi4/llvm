//===---------------- HIRNormalizeCasts.cpp -------------------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===---------------------------------------------------------------------===//
//
// This pass will replace the blobs that are integer casts in a loop to match
// with the same casts used between the loop bounds and the memory references.
// For example:
//
//   BEGIN REGION { }
//         + DO i1 = 0, zext.i32.i64(%cols) + -1, 1   <DO_LOOP>
//                   <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
//         |   %3 = (%arr)[sext.i32.i64(%cols) * i1];
//         |   %4 = (%arr)[zext.i32.i64(%cols) * i1];
//         |   %mul.i6926 = %3  *  %4;
//         |   %result.031.i = %mul.i6926  +  %result.031.i;
//         + END LOOP
//   END REGION
//
// In the example above we have a loop nest where the loop bound is a zero
// extension integer cast to the variable %cols, but the memory reference is a
// signed extension cast to the same variable, and the loop's legal max trip
// count (LEGAL_MAX_TC) is the max signed integer in src type (i32). This pass
// will replace the blob in the loop's upper bound from zero extent to signed
// extent, as well the memory references when it is legal to do so:
//
//   BEGIN REGION { }
//         + DO i1 = 0, sext.i32.i64(%cols) + -1, 1   <DO_LOOP>
//                   <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
//         |   %3 = (%arr)[sext.i32.i64(%cols) * i1];
//         |   %4 = (%arr)[sext.i32.i64(%cols) * i1];
//         |   %mul.i6926 = %3  *  %4;
//         |   %result.031.i = %mul.i6926  +  %result.031.i;
//         + END LOOP
//   END REGION
//
// In the example above, the integer casting in loop's upper bound was
// converted from zext to sext, as well instruction %4.
//
// This fix will help with other analyses that checks if the loop bound is
// used in the memory references.
//
// TODO: Add support for converting zext to sext in the memrefs for the
// HLInst.
//
//===---------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRNormalizeCasts.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#define OPT_SWITCH "hir-normalize-casts"
#define OPT_DESC "HIR normalize casts"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    DisableNormalizeCasts("disable-" OPT_SWITCH, cl::init(false), cl::Hidden,
                          cl::desc("Disable HIR normalize casts"));

STATISTIC(NumLoopsNormalized,
          "Number of loops upperbounds that were normalized");

class NormalizeCasts {
public:
  NormalizeCasts(HIRFramework &HIRF) : HIRF(HIRF) {}
  bool run();

private:
  HIRFramework &HIRF;

  // Traverse the HIR to find loop candidates
  struct LoopsAnalyzer;

  // Loops that will be transformed
  SmallPtrSet<HLLoop *, MaxLoopNestLevel> LoopCandidates;

  // Transform the zext into sext for the loop upperbound found
  void transformLoops();
};

struct NormalizeCasts::LoopsAnalyzer final : public HLNodeVisitorBase {
  NormalizeCasts &Pass;
  // Map a loop with the zext blob found in the upperbound
  SmallVector<std::pair<HLLoop *, BlobTy>, MaxLoopNestLevel> LoopZextInfoVect;

  LoopsAnalyzer(NormalizeCasts &Pass) : Pass(Pass) {}
  void visit(HLLoop *Loop);
  void visit(HLDDNode *DDNode);
  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
  void postVisit(HLLoop *Loop);

  // Find which loop in LoopZextInfoVect matches with the input blob and
  // add it as candidate
  void findAndCollectCandidateLoops(BlobTy SextBlobOP);

  // Find which loop in LoopZextInfoVect matches with the input blob and
  // remove it from the candidates list
  void findAndRemoveCandidateLoops(BlobTy ZextBlobOP);
};

// Check if the upper bound of the input loop has a zext blob, if so then store
// the loop and the blob found in LoopZextInfoVect. For example:
//
//   + DO i1 = 0, zext.i32.i64(%cols) + -1, 1   <DO_LOOP>
//             <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
//
// The loop above shows that the lower bound of the loop is 0 and the
// upperbound is zext.i32.i64(%cols) + -1, and the legal trip count is INT_MAX.
// This loop would be a candidate for transformation.
void NormalizeCasts::LoopsAnalyzer::visit(HLLoop *Loop) {
  if (!Loop->isDo() || !Loop->isNormalized())
    return;

  uint64_t LegalTC = Loop->getLegalMaxTripCount();
  if (LegalTC == 0)
    return;

  // We are looking that the upper bound doesn't have an IV, and the expression
  // is in the form of:
  //
  //   C1 * zext(%n) + C2 + 1 = Legal max trip count
  //
  // where C1 and C2 are constants. This basically means that:
  //
  //   zext(%n) = (Legal max trip count - C2 - 1) / C1
  //
  // In order to secure that zext(%n) is in the bounds of INT_MAX then we need
  // to confirm that (Legal max trip count - C2 - 1) <=u INT_MAX and that
  // 0 < C1.
  auto *UBCE = Loop->getUpperCanonExpr();
  auto &BU = UBCE->getBlobUtils();
  if (UBCE->numBlobs() != 1 || UBCE->hasIV() || UBCE->getDenominator() != 1)
    return;

  // BlobCoeff (C1 from the example above) needs to be larger than 0
  int64_t BlobCoeff = UBCE->getSingleBlobCoeff();
  if (BlobCoeff <= 0)
    return;

  unsigned BlobIdx = UBCE->getSingleBlobIndex();

  BlobTy CurrBlob = BU.getBlob(BlobIdx);
  BlobTy ZextOP = nullptr;
  if (!BU.isZeroExtendBlob(CurrBlob, &ZextOP))
    return;

  // Legal trip cound - upper bound coeff (C2 from the example) - 1 needs to
  // be less or equal that INT_MAX
  Type *CastType = ZextOP->getType();
  uint64_t MaxInt =
      APInt::getSignedMaxValue(CastType->getIntegerBitWidth()).getZExtValue();
  if ((LegalTC - UBCE->getConstant() - 1) > MaxInt)
    return;

  LoopZextInfoVect.push_back(std::make_pair(Loop, ZextOP));
}

// Check which candidate loops in LoopZextInfoVect use the input blob and
// remove them. Any loop that is found in LoopZextInfoVect and will be removed
// from Pass.LoopCandidates too. This will remove all the candidates from the
// current loop to the outermost. For example:
//
//   + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
//   |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
//   |   |   %5 = (%a)[zext.i32.i64(%m) * i1 + sext.i32.i64(%n) * i2];
//   |   |   %res.024 = %5  +  %res.024;
//   |   |   %t2 = (%a)[zext.i32.i64(%n) * i2];
//   |   + END LOOP
//   |
//   |   %res.024.out = %res.024;
//   + END LOOP
//
// Assume that loops i1 and i2 are candidates for normalizing. The load in %t2
// is using %n as zext inside the loop, therefore loops i2 and i1 shouldn't be
// candidates and will be removed from the lists.
//
// This check is done because we currently don't support converting zext into
// sext for the instructions inside the loop.
void NormalizeCasts::LoopsAnalyzer::findAndRemoveCandidateLoops(
    BlobTy ZextBlobOP) {

  auto It = LoopZextInfoVect.begin();
  while (It != LoopZextInfoVect.end()) {
    if (ZextBlobOP == It->second) {
      // If the current loop was inserted as candidate then remove it
      Pass.LoopCandidates.erase(It->first);
      It = LoopZextInfoVect.erase(It);
    } else {
      ++It;
    }
  }
}

// Traverse through the LoopZextInfoVect vector and find which zext's operand
// matches with the input blob. The input blob is the sext blob's operand. If
// the blobs match, then the loop that is mapped with the zext blob will be
// selected as a candidate for transformation.
void NormalizeCasts::LoopsAnalyzer::findAndCollectCandidateLoops(
    BlobTy SextBlobOP) {
  if (LoopZextInfoVect.empty())
    return;

  for (auto const &It :
       make_range(LoopZextInfoVect.begin(), LoopZextInfoVect.end())) {
    if (SextBlobOP == It.second)
      Pass.LoopCandidates.insert(It.first);
  }
}

// Check if the input HLDDNode has a memref that will use one of the zext
// blobs in the loop nest as a sext. For example:
//
//   + DO i1 = 0, zext.i32.i64(%cols) + -1, 1   <DO_LOOP>
//             <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
//   |   %3 = (%arr)[sext.i32.i64(%cols) * i1];
//
// In the example above, intruction %3 uses the blob %cols in the memref for
// %arr as a signed extension, and %cols is used as a zero extension in the
// upperbound. Therefore we are going to update the information in the
// LoopZextInfo for the i1 loop.
void NormalizeCasts::LoopsAnalyzer::visit(HLDDNode *DDNode) {
  if (LoopZextInfoVect.empty() || isa<HLLoop>(DDNode))
    return;

  for (unsigned I = 0, E = DDNode->getNumOperands(); I < E; I++) {
    auto *OpRef = DDNode->getOperandDDRef(I);
    if (!OpRef->isMemRef())
      continue;

    unsigned NumDims = OpRef->getNumDimensions();
    for (unsigned Dim = 1; Dim <= NumDims; Dim++) {
      auto *CE = OpRef->getDimensionIndex(Dim);
      SmallVector<unsigned, 4> BlobIndices;
      CE->collectBlobIndices(BlobIndices);
      if (BlobIndices.empty())
        continue;

      auto &BU = CE->getBlobUtils();
      for (auto Index : BlobIndices) {
        auto *CurrBlob = BU.getBlob(Index);
        BlobTy SExtBlobOP;
        BlobTy ZExtBlobOP;
        // If the blob is a zext then check if it affects any loop candidate
        // and remove it
        if (BU.isZeroExtendBlob(CurrBlob, &ZExtBlobOP))
          findAndRemoveCandidateLoops(ZExtBlobOP);
        else if (BU.isSignExtendBlob(CurrBlob, &SExtBlobOP))
          findAndCollectCandidateLoops(SExtBlobOP);
      }
    }
  }
}

// Remove the current loop from LoopZextInfoVect since it isn't needed anymore
void NormalizeCasts::LoopsAnalyzer::postVisit(HLLoop *Loop) {
  if (LoopZextInfoVect.empty())
    return;

  auto LastEntry = LoopZextInfoVect.rbegin();
  if (LastEntry->first == Loop)
    LoopZextInfoVect.pop_back();
}

// Traverse through each candidate loop and convert the zext blobs in the
// upperbound's canon expression into sext blobs using the information
// collected. For example, the loop
//
//   + DO i1 = 0, zext.i32.i64(%cols) + -1, 1   <DO_LOOP>
//             <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
//
// will be converted into:
//
//   + DO i1 = 0, sext.i32.i64(%cols) + -1, 1   <DO_LOOP>
//             <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
//
void NormalizeCasts::transformLoops() {
  assert(!LoopCandidates.empty() &&
         "Trying to transform loops without information");

  bool Transformed = false;
  for (auto *Loop : LoopCandidates) {
    auto *UBCE = Loop->getUpperCanonExpr();
    auto &BU = UBCE->getBlobUtils();

    unsigned ZextBlobIdx = UBCE->getSingleBlobIndex();
    auto *ZextBlob = cast<SCEVZeroExtendExpr>(BU.getBlob(ZextBlobIdx));
    unsigned SextBlobIdx;
    BU.createSignExtendBlob(ZextBlob->getOperand(), UBCE->getSrcType(), true,
                            &SextBlobIdx);
    UBCE->replaceBlob(ZextBlobIdx, SextBlobIdx);
    Transformed = true;
    NumLoopsNormalized++;
  }

  assert(Transformed && "Casting in loop upperbound not normalized");
  (void)Transformed;
}

bool NormalizeCasts::run() {
  if (DisableNormalizeCasts)
    return false;

  LLVM_DEBUG(dbgs() << "Analyzing function: " << HIRF.getFunction().getName()
                    << "\n");

  LoopsAnalyzer AnalyzeLoops(*this);
  HIRF.getHLNodeUtils().visitAll(AnalyzeLoops);

  if (LoopCandidates.empty()) {
    LLVM_DEBUG(dbgs() << "    No loop found to normalize\n");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "  Loop" << (LoopCandidates.size() > 1 ? "s" : "") << " found:\n";
    for (auto *Loop : LoopCandidates) {
      auto *Region = Loop->getParentRegion();
      dbgs() << "    Loop <" << Loop->getNumber() << "> in Region <"
             << Region->getNumber() << ">\n";
    }
  });

  transformLoops();

  return true;
}

PreservedAnalyses HIRNormalizeCasts::runImpl(Function &F,
                                             FunctionAnalysisManager &AM,
                                             HIRFramework &HIRF) {

  ModifiedHIR = NormalizeCasts(HIRF).run();

  return PreservedAnalyses::all();
}

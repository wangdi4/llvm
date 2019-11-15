//===-----HIRSparseArrayReductionAnalysis.cpp - Sparse Array Reduction-----===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// This file implements Sparse Array Reduction Identification
// Specifically, identify Sparse Array Reduction chain for a loop.
// "Sparse Array" implies Reduction recurrence can be ignored for both
// parallelization and vectorization.
// Handles memory references of the form: A[m[i]] = A[m[i]] + ...
// It selects child stmts directly under the loop and finds the reduction
// groups.

// Sparse Array reductions are of the follow forms:
// a. Single sparse array reduction chain
//    t1 = A[i]
//    t2 = c1 * t1 / c2
//    t3 = m[t2 + x] + ...
//    m[t2 + x] = t3

// Example: 544.nab
// <3>       |   %0 = (@a1)[0][i1];
// <5>       |   %div = 4 * %0  /  3;
// ...
// <59>      |   %mul42 = %sub8  *  %mul41;
// <62>      |   %14 = (@f)[0][%div + %foff];
// <63>      |   %add46 = %14  +  %mul42;
// <64>      |   (@f)[0][%div + %foff] = %add46;

// After temp cleanup instead of <62> and <63> we can get instructions like-
// <63>      |   %add46 = (@f)[0][%div + %foff]  +  %mul42;
// which is also recognized as sparse array reduction

// TODO: Closed form is generated for integer arrays and will not match the
// above pattern. We could improve this to support integer arrays in addition
// to floating point arrays.

// TODO: For long chains in sparse array reduction as those in gromacs,
// special replacement of the arrays are needed before building the pi-groups.
// Ex. tempx below will be scalar expanded.
//   tx12  = faction(j) - t11
//   ...
//   faction(j) = tx12 + t30
//   =>
//   tx12 = 0.0 – t11
//   ...
//   tempx =  tx12 + t30
//   faction(j) += tempx

// TODO: Extend recognition for chains more than 3 statement. Example-
//   jx1 = pos(j3)
//   dx21 = ix2 - jx1
//   fjx1 = faction(j3) - tx11
//   tx21 = dx21 * fs21
//   tx21 = dx21 * fs21
//   fjx1 = fjx1 - tx21
//   tx31 = dx31 * fs31
//   faction(j3) = fjx1 - tx31
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDTests.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSparseArrayReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"

#include <algorithm>
#include <map>
#include <vector>

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-sparse-array-reduction-analysis"
static cl::opt<bool>
    ForceSARA("force-hir-sparse-array-reduction-analysis", cl::init(false),
              cl::Hidden,
              cl::desc("forces sparse array reduction analysis by request"));

AnalysisKey HIRSparseArrayReductionAnalysisPass::Key;
HIRSparseArrayReductionAnalysis
HIRSparseArrayReductionAnalysisPass::run(Function &F,
                                         FunctionAnalysisManager &AM) {
  return HIRSparseArrayReductionAnalysis(AM.getResult<HIRFrameworkAnalysis>(F),
                                         AM.getResult<HIRDDAnalysisPass>(F));
}

FunctionPass *llvm::createHIRSparseArrayReductionAnalysisPass() {
  return new HIRSparseArrayReductionAnalysisWrapperPass();
}

char HIRSparseArrayReductionAnalysisWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRSparseArrayReductionAnalysisWrapperPass,
                      "hir-sparse-array-reduction-analysis",
                      "HIR Sparse Array Reduction Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRSparseArrayReductionAnalysisWrapperPass,
                    "hir-sparse-array-reduction-analysis",
                    "HIR Sparse Array Reduction Analysis", false, true)

void HIRSparseArrayReductionAnalysisWrapperPass::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
  AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
}

// Sample code for calling Sparse Array Reduction
// a. Get sparse array reduction analysis object
//   SARA = &getAnalysis<HIRSparseArrayReductionAnalysis>();
// b. Compute by passing innermost loops
//   SARA->computeSparseArrayReductionChains(outerloops);
// c. In LoopDistribution
// -> Check instructions
//   if (SARA->isSparseArrayReduction(Inst)) {...}
// -> Or walk the chains like below
//   const SparseArrayReductionChainList & SARCL =
//   SARA->getSparseArrayReductionChain(Loop);
//  if (!SARCL.empty()) {
//	  for (auto SRC : SARCL) {
//		  for (auto Inst : SRC) {
//		 	  Inst->print(OS, 2, false);
//		 	  ...
bool HIRSparseArrayReductionAnalysisWrapperPass::runOnFunction(Function &F) {
  HSAR.reset(new HIRSparseArrayReductionAnalysis(
      getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
      getAnalysis<HIRDDAnalysisWrapperPass>().getDDA()));
  return false;
}

HIRSparseArrayReductionAnalysis::HIRSparseArrayReductionAnalysis(
    HIRFramework &HIRF, HIRDDAnalysis &DDA)
    : HIRAnalysis(HIRF), DDA(DDA) {
  if (!ForceSARA) {
    return;
  }

  // Gather the innermost loops as candidates.
  SmallVector<HLLoop *, 32> CandidateLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(CandidateLoops);

  for (auto *Loop : CandidateLoops) {
    identifySparseArrayReductionChains(Loop);
  }
}

void printAChain(formatted_raw_ostream &OS, unsigned Indented,
                 const loopopt::SparseArrayReductionChain &SRC) {
  for (auto *Inst : SRC) {
    Inst->print(OS, Indented, false);
  }
}

void HIRSparseArrayReductionAnalysis::identifySparseArrayReductionChains(
    const HLLoop *Loop) {
  if (!Loop->isDo()) {
    return;
  }

  DDG = DDA.getGraph(Loop);

  // Gather all the memory references and group by equal DDRef.
  MemRefGatherer::VectorTy RefVecUniqueSB;
  MemRefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(),
                              RefVecUniqueSB);
  RefGroupVecTy RefVecGroups;
  DDRefGrouping::groupVec(RefVecGroups, RefVecUniqueSB,
                          std::bind(DDRefUtils::areEqual, std::placeholders::_1,
                                    std::placeholders::_2, false));
  LLVM_DEBUG(formatted_raw_ostream FOS(dbgs());
             DDRefGrouping::dump(RefVecGroups));

  // Process each group and identify reduction chains.
  for (auto &RefVec : RefVecGroups) {
    // Check if the memory references with current symbase satisfies the
    // coefficient, blob etc. requirements. This step does the
    // screening of load/store instructions with dd flow-edge from another
    // load. From the example at the top of the file, symbase
    // (@f)[0][%div + %foff] should be a valid candidate. It also does the
    // pattern matching in associated instructions and decide whether this
    // group forms a reduction chain or not.
    validateAndCreateSparseArrayReduction(Loop, RefVec);
  }
}

void HIRSparseArrayReductionAnalysis::computeSparseArrayReductionChains(
    const HLLoop *Loop) {
  SmallVector<const HLLoop *, 32> CandidateLoops;
  Loop->getHLNodeUtils().gatherInnermostLoops(CandidateLoops, Loop);
  for (auto *Lp : CandidateLoops) {
    auto SARCL = SparseArrayReductionMap.find(Lp);
    if (SARCL != SparseArrayReductionMap.end()) {
      continue;
    }
    identifySparseArrayReductionChains(Lp);
  }
}

const SparseArrayReductionChainList &
HIRSparseArrayReductionAnalysis::getSparseArrayReductionChain(
    const HLLoop *Loop) {
  assert(Loop->isInnermost() &&
         "SparseArrayReduction supports only innermost loop");
  SparseArrayReductionChainList &SARCL = SparseArrayReductionMap[Loop];
  return SARCL;
}

static bool isMatchedLoadPattern(const RegDDRef *RDDRef,
                                 unsigned NestingLevel) {
  auto *SrcInst = cast<HLInst>(RDDRef->getHLDDNode());

  // Properties checked on the load at instruction <3> | %0 = (@a1)[0][i1];
  // a1[i1] should have no incoming edges from the same loop.
  if (!isa<LoadInst>(SrcInst->getLLVMInstruction())) {
      return false;
  }

  const RegDDRef *RRef = SrcInst->getRvalDDRef();
  // TODO: The following check is too conservative in case of inaccurate def
  //       levels. (CMPLRLLVM-10587)
  //
  // DO i1 {
  //   if () {
  //     A = ...
  //   }
  //
  //   ... A[i1] ... (non-linear) (1)
  // }
  //
  // (1) has non-linear base A, however after loop unswitch the def levels
  // in the false loop may be updated to some linear level.
  //
  // const CanonExpr *BaseCE = RRef->getBaseCE();
  // if  (!BaseCE->isInvariantAtLevel(NestingLevel)) {
  //   return false;
  // }

  auto I = RRef->canon_begin();
  auto E = RRef->canon_end();


  if ((*I)->numIVs() != 1) {
    return false;
  }

  for (I++; I != E; I++) {
    if (!(*I)->isInvariantAtLevel(NestingLevel)) {
      return false;
    }
  }
  return true;
}

// This method traverses through ddref edges and finds a load.
// It returns true if a load instruction is found within 'Hop' distance.
// Example: starting with instruction <5>, we will reach instruction <3> within
// two hop <3> | %0 = (@a1)[0][i1]; <5> | %div = 4 * %0 / 3;
bool HIRSparseArrayReductionAnalysis::findLoadInstWithinNHops(
    const HLInst *SrcInst, unsigned NestingLevel, unsigned Hop,
    bool *SingleLoadFound) {
  LLVM_DEBUG(dbgs() << "At hop=" << Hop << ":\t");
  LLVM_DEBUG(formatted_raw_ostream FOS(dbgs()); SrcInst->dump());

  // Traversed more than expected for tracking the load.
  if (Hop == 0) {
    return false;
  }

  // We only allow binary operators and load instructions.
  // It may be possible to allow some unary instructions like casts as well.
  // But we don't want to allow any arbitrary instructions, for ex. calls.
  if (!SrcInst->getLLVMInstruction()->isBinaryOp() &&
      !isa<LoadInst>(SrcInst->getLLVMInstruction())) {
    return false;
  }

  bool SingleIncomingEdge = false;
  unsigned OperandNum = 0;
  unsigned Opcode = SrcInst->getLLVMInstruction()->getOpcode();

  // Check the ddref of rval of the current instruction.
  // Here we get the ddref of 4 * %0 / 3 from instruction <5>
  // Or memory references like (@a1)[0][i1] from instruction <3>.
  for (auto I3 = SrcInst->rval_op_ddref_begin(),
            E3 = SrcInst->rval_op_ddref_end();
       I3 != E3; ++I3, ++OperandNum) {
    const RegDDRef *RRef = *I3;

    // STEP 1: Handle if the the current ref is a memref (from load)
    if (RRef->isMemRef()) {
      if (*SingleLoadFound) {
        // We found load through some other operand before.
        // Found another again, so bail out.
        return false;
      }

      *SingleLoadFound = true;
      if (!isMatchedLoadPattern(RRef, NestingLevel)) {
        // We have found a load which does't match the pattern.
        return false;
      }

      // If there are incoming edges to memref, bail out
      if (DDG.getNumIncomingFlowEdges(RRef) != 0) {
        return false;
      }
    }

    // STEP 2: Check number of incoming flow edges
    unsigned NumIncomingFlowEdges = DDG.getNumIncomingFlowEdges(RRef);
    LLVM_DEBUG(dbgs() << "Number of incoming edge to the rval operand: "
                      << NumIncomingFlowEdges << "\n");

    // No edge to process, continue to the next operand.
    if (NumIncomingFlowEdges == 0) {
      continue;
    }

    // Bail out if the 2nd operand of sub/div has that single incoming edge.
    if ((OperandNum == 1) &&
        (Opcode == Instruction::FSub || Opcode == Instruction::Sub ||
         Opcode == Instruction::UDiv || Opcode == Instruction::SDiv ||
         Opcode == Instruction::FDiv)) {
      return false;
    }

    // There should be at most one incoming flow edge to rval operands.
    // Also if we have seen incoming edge to other operands before, bail out.
    if (NumIncomingFlowEdges > 1 || SingleIncomingEdge) {
      return false;
    }

    SingleIncomingEdge = true;

    // STEP 3: Go through all the only incoming flow edge to rval ref (if any).
    for (auto I2 = DDG.incoming_edges_begin(RRef),
              E2 = DDG.incoming_edges_end(RRef);
         I2 != E2; ++I2) {
      assert((*I2)->isFlow() &&
             "Incoming edges to blob refs should be flow edges only");

      auto *DDRefSrc = (*I2)->getSrc();
      const auto *InstAtNextHop = cast<HLInst>(DDRefSrc->getHLDDNode());

      if (!findLoadInstWithinNHops(InstAtNextHop, NestingLevel, Hop - 1,
                                   SingleLoadFound)) {
        // Returning true is wrong because this may be only the first operand.
        return false;
      }
    }

    // STEP 4: Blob dd refs are also checked, follow if there is
    // any incoming flow edge.
    // Blobs like 4 * %0 from instruction <5> will get processed here.
    for (auto BI = RRef->blob_begin(), BE = RRef->blob_end(); BI != BE;
         BI++) {
      const BlobDDRef *BRRef = *BI;

      for (auto I2 = DDG.incoming_edges_begin(BRRef),
                E2 = DDG.incoming_edges_end(BRRef);
           I2 != E2; ++I2) {
        assert((*I2)->isFlow() &&
               "Incoming edges to blob refs should be flow edges only");

        auto *DDRefSrc = (*I2)->getSrc();
        const auto *InstAtNextHop = cast<HLInst>(DDRefSrc->getHLDDNode());

        if (!findLoadInstWithinNHops(InstAtNextHop, NestingLevel, Hop - 1,
                                     SingleLoadFound)) {
          return false;
        }
      }
    }
  }
  // This is for handling instructions which do not have any incoming flow
  // edges.
  return true;
}

// Instructions like <63> with a reduction operator will be analyzed here.
// TODO: Identify reduction chain with multiple intermediate reduction operation
// <62>      |   %14 = (@f)[0][%div + %foff];
// <63>      |   %add46 = %14  +  %mul42;
// <64>      |   (@f)[0][%div + %foff] = %add46;
bool HIRSparseArrayReductionAnalysis::isReductionStmt(const HLInst *Inst,
                                                      unsigned *ReductionOpCode,
                                                      const RegDDRef *LoadRef) {
  if (!Inst->isReductionOp(ReductionOpCode)) {
    return false;
  }

  unsigned OperandNum = 0;

  // ddrefs of %14 (load) is analyzed here and
  // %mul42 will be ignored.
  for (auto I = Inst->rval_op_ddref_begin(), E = Inst->rval_op_ddref_end();
       I != E; ++I, ++OperandNum) {
    // Based on the reduction opcode either the first (in case of sub/div) or
    // the second operand should have a single incoming edge.
    // Source will be the same as the load's node.
    if ((OperandNum == 1) && (*ReductionOpCode == Instruction::FSub ||
                              *ReductionOpCode == Instruction::Sub ||
                              *ReductionOpCode == Instruction::UDiv ||
                              *ReductionOpCode == Instruction::SDiv ||
                              *ReductionOpCode == Instruction::FDiv)) {
      return false;
    }

    const RegDDRef *RRef = *I;

    // The rval can be a memref (without separate load instruction)
    // After temp cleanup we get instructions like-
    // <63>      |   %add46 = (@f)[0][%div + %foff]  +  %mul42;
    if (RRef == LoadRef) {
      return true;
    }

    // There should be only one incoming flow edge to rval operand %14.
    if (DDG.getNumIncomingFlowEdges(RRef) != 1) {
      continue;
    }

    if (!RRef->isSelfBlob()) {
      continue;
    }

    // Go through the incoming flow edge to this ddref.
    // We are hunting a load with index from another load (thus sparse array
    // reduction).
    auto *Edge = *(DDG.incoming_edges_begin(RRef));
    assert(Edge->isFlow() &&
           "Incoming edges to blob refs should be flow edges only");

    DDRef *DDRefSrc = Edge->getSrc();
    HLInst *SrcInst = cast<HLInst>(DDRefSrc->getHLDDNode());

    // There should be an incoming edge to %14.
    // We want to see if that comes from our previously identified load.
    if (isa<LoadInst>(SrcInst->getLLVMInstruction()) &&
        SrcInst->getRvalDDRef() == LoadRef) {
      return true;
    }
  }
  return false;
}

// Example reduction group:
//   %0 = (@a1)[0][i1];
//   (@f)[0][3 * %0] = %1989;
// From the above example, this function will get the load and store memrefs
// thru the nonlinear-blob %iv.
// We are going to check the legality in terms of dependency, reduction
// operator and existence of load instruction

bool HIRSparseArrayReductionAnalysis::isLegallyValid(
    const RegDDRef *LoadRef, const RegDDRef *StoreRef, const HLLoop *Loop,
    const BlobDDRef *NonLinearBRRef, HLInst **ReductionInst,
    unsigned *ReductionOpCode) {
  // Start processing with stores.
  // Take the ddrefs of rval (%add46).
  auto *ReductionTempRef = StoreRef->getHLDDNode()->getRvalDDRef();
  if (!ReductionTempRef->isSelfBlob()) {
    return false;
  }

  // There should be only one incoming flow edge to rval operand %add46.
  if (DDG.getNumIncomingFlowEdges(ReductionTempRef) != 1) {
    return false;
  }

  // Start walking through the incoming edge to %add46.
  auto *Edge = *(DDG.incoming_edges_begin(ReductionTempRef));
  assert(Edge->isFlow() &&
         "Incoming edges to blob refs should be flow edges only");

  auto *DDRefSrc = Edge->getSrc();
  *ReductionInst = cast<HLInst>(DDRefSrc->getHLDDNode());

  if (!isReductionStmt(*ReductionInst, ReductionOpCode, LoadRef)) {
    return false;
  }

  // There should be only one incoming flow edge to this blob.
  if (DDG.getNumIncomingFlowEdges(NonLinearBRRef) != 1) {
    return false;
  }

  // Check if that non-linear blob (%iv) comes from a load.
  // Thus making the memory access sparse.
  Edge = *(DDG.incoming_edges_begin(NonLinearBRRef));
  assert(Edge->isFlow() &&
         "Incoming edges to blob refs should be flow edges only");

  DDRefSrc = Edge->getSrc();
  auto *InstAtNextHop = cast<HLInst>(DDRefSrc->getHLDDNode());

  // Hunt for load through %iv.
  bool SingleLoadFound = false;
  if (!findLoadInstWithinNHops(InstAtNextHop, Loop->getNestingLevel(),
                               SparserLoadDistance, &SingleLoadFound) ||
      !SingleLoadFound) {
    return false;
  }
  return true;
}

// Returns the non-linear blob index as well as that single blob
// (through second parameter) in the first parameter RegDDRef.
// Returns InvalidBlobIndex if multiple non-linear blob is found
static unsigned getSingleNonLinearBlobIndex(const RegDDRef *StoreRef,
                                            const BlobDDRef **NonLinearBRRef) {
  // Find the index of only non-linear blob.
  unsigned NonLinearBlobIndex = InvalidBlobIndex;
  // Traverse all the blob ddref of StoreRef.
  for (auto BI = StoreRef->blob_begin(), BE = StoreRef->blob_end(); BI != BE;
       ++BI) {
    if ((*BI)->isNonLinear()) {
      if (NonLinearBlobIndex != InvalidBlobIndex) {
        // Multiple non-linear blob.
        return InvalidBlobIndex;
      }
      NonLinearBlobIndex = (*BI)->getBlobIndex();
      *NonLinearBRRef = *BI;
    }
  }
  return NonLinearBlobIndex;
}

// Structural checks on the canon expressions of the store ref
// is done here. From the above example- canon expression is %iv + %foff + ...
// Need to walk the blobs and work with the identified non-linear blob.
static bool isStructurallyValid(const RegDDRef *StoreRef, unsigned LoopLevel,
                                unsigned NonLinearBlobIndex) {
  // This is a profitability check.
  // Do not recognize refs like (%0)[%1].1 as sparse array reductions.
  if (StoreRef->hasTrailingStructOffsets(1)) {
    return false;
  }

  // We do not expect iv in the canon expression.
  auto *FirstCE = StoreRef->getDimensionIndex(1);
  if (FirstCE->hasIV()) {
    return false;
  }

  // We have to explicitly check that the non-linear blob is present in
  // only one CE (last dimension).
  bool IsTopLevelBlob = false;
  auto &BU = FirstCE->getBlobUtils();

  for (auto BI = FirstCE->blob_begin(), BE = FirstCE->blob_end(); BI != BE;
       BI++) {
    // Check top level blob.
    unsigned BlobIndex = FirstCE->getBlobIndex(BI);

    // Check whether the target blob is embedded.
    if (NonLinearBlobIndex == BlobIndex) {
      IsTopLevelBlob = true;
    } else if (BU.contains(BU.getBlob(BlobIndex),
                           BU.getBlob(NonLinearBlobIndex))) {
      return false;
    }
  }

  // Bail out if non-linear blob with constant coefficient not found.
  if (!IsTopLevelBlob) {
    return false;
  }

  // All other CE's should be loop invariant.
  for (unsigned I = 2, NumDims = StoreRef->getNumDimensions(); I <= NumDims;
       ++I) {
    auto *CE = StoreRef->getDimensionIndex(I);
    if (!CE->isInvariantAtLevel(LoopLevel)) {
      return false;
    }
  }
  return true;
}

// Reduction chain like below for a group related to particular symbase will be
// formed here
// <62>      |   %14 = (@f)[0][%div + %foff];
// <63>      |   %add46 = %14  +  %mul42;
// <64>      |   (@f)[0][%div + %foff] = %add46;
// Input to this method is a collection of memory references.
// This method starts with analyzing the collection
// and tries to recognize the reduction chain.

// This method analyzes the patterns associated with a particular symbase.
// And identify whether it meets the preliminary requirements to participate in
// a reduction group.
void HIRSparseArrayReductionAnalysis::validateAndCreateSparseArrayReduction(
    const HLLoop *Loop, const RefGroupTy &RefVec) {
  // Candidate group should have two references (load and store).
  if (RefVec.size() != 2) {
    return;
  }

  // Check if two references are one load and another store.
  if (!((RefVec[0]->isLval() && RefVec[1]->isRval()) ||
        (RefVec[0]->isRval() && RefVec[1]->isLval()))) {
    return;
  }

  // Keep a reference to load and store refs.
  auto *StoreRef = RefVec[0];
  auto *LoadRef = RefVec[1];

  // Swap them if not assigned appropriately.
  if (StoreRef->isRval()) {
    std::swap(LoadRef, StoreRef);
  }

  // Right hand side should have single ddref.
  const HLDDNode *StoreNode = StoreRef->getHLDDNode();
  if (StoreNode->getNumOperands() != 2) {
    return;
  }

  // Each group should have the same base.
  // The ddref should be non-linear
  // and the parent of the ddrefs are within the same loop.
  if (!StoreRef->isNonLinear() || StoreNode->getParent() != Loop) {
    return;
  }

  const BlobDDRef *NonLinearBRRef;
  unsigned NonLinearBlobIndex =
      getSingleNonLinearBlobIndex(StoreRef, &NonLinearBRRef);
  if (NonLinearBlobIndex == InvalidBlobIndex) {
    return;
  }

  // Check if the memrefs are structurally valid
  if (!isStructurallyValid(StoreRef, Loop->getNestingLevel(),
                           NonLinearBlobIndex)) {
    return;
  }

  unsigned ReductionOpCode = 0;
  HLInst *ReductionInst;

  // Check if we get a valid reduction chain
  if (isLegallyValid(LoadRef, StoreRef, Loop, NonLinearBRRef, &ReductionInst,
                     &ReductionOpCode)) {
    // We have identified a valid reduction chain.
    SparseArrayReductionChain ReductionInsts;
    ReductionInsts.push_back(cast<HLInst>(LoadRef->getHLDDNode()));
    ReductionInsts.push_back(ReductionInst);
    ReductionInsts.push_back(cast<HLInst>(StoreRef->getHLDDNode()));

    // Save the newly formed chain.
    setSparseArrayReductionChainList(ReductionInsts, Loop,
                                     StoreRef->getSymbase(), ReductionOpCode);
    LLVM_DEBUG(dbgs() << "Sparse Array Reduction Chain:\n");
    LLVM_DEBUG(formatted_raw_ostream FOS(dbgs());
               printAChain(FOS, 1, ReductionInsts));
    LLVM_DEBUG(dbgs() << "\n");
  }
}

void HIRSparseArrayReductionAnalysis::setSparseArrayReductionChainList(
    SparseArrayReductionChain &ReductionInsts, const HLLoop *Loop,
    unsigned ReductionSymbase, unsigned ReductionOpCode) {
  SparseArrayReductionChainList &SARCL = SparseArrayReductionMap[Loop];
  SARCL.emplace_back(ReductionInsts, ReductionSymbase, ReductionOpCode);
  unsigned SRIIndex = SARCL.size() - 1;

  // We should use []operator instead of insert() to overwrite the previous
  // entry for the instruction. SparseArrayReductionMap and
  // SparseArrayReductionInstMap can go out of sync due to deleted loops. Refer
  // to comment in getSparseArrayReductionInfo().
  for (auto &Inst : ReductionInsts) {
    SparseArrayReductionInstMap[Inst] = SRIIndex;
  }
}

// Check if an instruction is part of a reduction chain.
bool HIRSparseArrayReductionAnalysis::isSparseArrayReduction(
    const HLInst *Inst, bool *IsSingleStmt) const {
  const SparseArrayReductionInfo *SARI = getSparseArrayReductionInfo(Inst);
  if (!SARI) {
    return false;
  }

  if (IsSingleStmt) {
    *IsSingleStmt = (SARI->Chain.size() == 1);
  }

  return true;
}

void HIRSparseArrayReductionAnalysis::print(
    formatted_raw_ostream &OS, const HLLoop *Loop,
    const SparseArrayReductionChainList *SARCL) {
  unsigned Depth = Loop->getNestingLevel() + 1;

  if (SARCL->empty()) {
    Loop->indent(OS, Depth);
    OS << "No Sparse Array Reduction\n";
    return;
  }

  for (auto &SARI : *SARCL) {
    printAChain(OS, Depth, SARI.Chain);
  }
}

void HIRSparseArrayReductionAnalysis::print(formatted_raw_ostream &OS,
                                            const HLLoop *Loop) {
  auto &SARCL = SparseArrayReductionMap[Loop];
  print(OS, Loop, &SARCL);
}

void HIRSparseArrayReductionAnalysisWrapperPass::releaseMemory() {
  HSAR.reset();
}

void HIRSparseArrayReductionAnalysis::markLoopBodyModified(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");
  // No need to clean up info in parent loop.
  auto Iter = SparseArrayReductionMap.find(Loop);
  if (Iter != SparseArrayReductionMap.end()) {
    for (auto &SARI : Iter->second) {
      for (auto Inst : SARI.Chain) {
        SparseArrayReductionInstMap.erase(Inst);
      }
    }
    SparseArrayReductionMap.erase(Iter);
  }
}

const SparseArrayReductionInfo *
HIRSparseArrayReductionAnalysis::getSparseArrayReductionInfo(
    const HLInst *Inst) const {
  auto Iter = SparseArrayReductionInstMap.find(Inst);
  if (Iter == SparseArrayReductionInstMap.end()) {
    return nullptr;
  }

  // Get index of SparseArrayReductionInfo via Inst.
  auto &SRIIndex = Iter->second;
  const HLLoop *Loop = Inst->getLexicalParentLoop();
  // Get SparseArrayReductionChainList via Loop.
  auto Iter2 = SparseArrayReductionMap.find(Loop);

  assert(Iter2 != SparseArrayReductionMap.end() &&
         "sparse array reduction analysis is in an inconsistent state!");

  auto &SARCL = Iter2->second;

  // Return SparseArrayReductionInfo via obtained Index and SARCL.
  return &SARCL[SRIIndex];
}

bool HIRSparseArrayReductionAnalysis::isReductionRef(
    const RegDDRef *Ref, unsigned &ReductionOpCode) {
  auto Node = Ref->getHLDDNode();

  assert(Node && "RegDDRef with null HLDDNode");
  auto *Inst = dyn_cast<HLInst>(Node);

  if (!Inst) {
    return false;
  }

  const SparseArrayReductionInfo *SARI = getSparseArrayReductionInfo(Inst);
  if (!SARI || SARI->Symbase != Ref->getSymbase()) {
    return false;
  }

  ReductionOpCode = SARI->OpCode;
  return true;
}

//===------------------- VPlanDivergenceAnalysis.cpp ----------------------===//
//
//   Copyright (C) 2018 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements VPlan Divergence Analysis. Divergence analysis computes
// information that describes how values change with respect to each vector
// lane for a loop that may be vectorized using a specific VF. Values that are
// uniform are the same for each lane. Values that are or can be different for
// each lane are said to be divergent. Data divergence is discovered in a couple
// of ways:
//
// 1) Values can be predefined to be uniform/divergent, e.g., loop induction
//    variable for vector loop.
//
// 2) Control flow divergence (e.g., conditional branching) causes data
//    divergence because this can result in different execution paths for each
//    vector lane, resulting in multiple definitions reaching a use.
//
// The VPlan Divergence Analysis algorithm identifies uniformity/divergence for
// each VPValue defined within the VPlan based upon both criteria.
//
// This work is based very closely on the DA work done at Saarland University
// for the Region Vectorizer, led by Simon Moll.
// See the following for reference:
//
//   https://github.com/cdl-saarland/vplan-rv
//   https://github.com/cdl-saarland/rv
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanDivergenceAnalysis.h"
#include "IntelVPlan.h"
#include "IntelVPlanBranchDependenceAnalysis.h"
#include "IntelVPlanLoopInfo.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "vplan-divergence-analysis"

using namespace llvm;
using namespace llvm::vpo;

VPlanDivergenceAnalysis::~VPlanDivergenceAnalysis() {
  DivergentValues.clear();
  DivergentBlocks.clear();
  StrideMap.clear();
  AlignmentMap.clear();
}

void VPlanDivergenceAnalysis::print(raw_ostream &OS, VPLoop *VPLp) const {

  LLVM_DEBUG(dbgs() << "\nPrinting Divergence info for " << *VPLp << "\n");
  ReversePostOrderTraversal<VPBlockBase *> RPOT(VPLp->getHeader());
  for (VPBlockBase *Block : make_range(RPOT.begin(), RPOT.end())) {
    if (auto VPBB = dyn_cast<VPBasicBlock>(Block)) {
      LLVM_DEBUG(dbgs() << "Basic Block: " << VPBB->getName() << "\n");
      for (auto &Recipe : VPBB->getRecipes()) {
        if (const VPInstruction *VPInst = dyn_cast<VPInstruction>(&Recipe)) {
          if (const VPValue *V = dyn_cast<VPValue>(VPInst)) {
            if (DivergentValues.find(V) != DivergentValues.end())
              LLVM_DEBUG(OS << "Divergent: "; V->dump(OS));
            else
              LLVM_DEBUG(OS << "Uniform: "; V->dump(OS));
          }
        }
      }
      LLVM_DEBUG(dbgs() << "\n");
    }
  }
}

void VPlanDivergenceAnalysis::markDivergent(const VPValue *V) {
  DivergentValues.insert(V);
}

bool VPlanDivergenceAnalysis::isDivergent(const VPValue *V) const {
  return DivergentValues.find(V) != DivergentValues.end();
}

bool VPlanDivergenceAnalysis::updateDivergenceInfo(const VPInstruction *I) {
  unsigned OpCode = I->getOpcode();
  if (OpCode == Instruction::PHI) {
    // Join in divergence of parent block
    const VPBasicBlock *Parent = I->getParent();
    if (DivergentBlocks.find(Parent) != DivergentBlocks.end())
      return true;
  }

  // Join in incoming value divergence
  for (const auto *Operand : I->operands()) {
    if (isDivergent(Operand))
      return true;
  }

  return false;
}

// The main driver for DA. Start by pushing known divergent instructions to a
// stack and then propagate divergence to all users. Once a user becomes
// divergent, push its users to the stack and propagate divergence to them,
// and so forth.
void VPlanDivergenceAnalysis::compute(VPLoop *CandidateLoop, VPLoopInfo *VPLI,
                                      VPDominatorTree *DomTree,
                                      VPPostDominatorTree *PostDomTree) {

  SmallVector<const VPValue *, 1> Seeds;
  SmallPtrSet<const VPValue *, 1> UniformOverrides;
  SmallVector<const VPInstruction *, 4> Worklist;

  VPlanBranchDependenceAnalysis VPBDA(CandidateLoop->getHeader(), DomTree,
                                      PostDomTree, VPLI);

  // Right now, there is only one vector loop candidate in the VPlan, the outer
  // most loop provided in the region, so DA is computed for it. For this
  // initial implementation, the divergence information that is computed is a
  // plain divergent/uniform property that holds for all VFs. Optimizations for
  // particular VFs will be added later.
  LLVM_DEBUG(dbgs() << "Computing divergence analysis for loop: "
                    << *CandidateLoop << "\n");
  VPBasicBlock *LpHeader = dyn_cast<VPBasicBlock>(CandidateLoop->getHeader());
  LLVM_DEBUG(dbgs() << "Candidate Loop Header: " << *LpHeader << "\n");
  for (const VPRecipeBase &Recipe : LpHeader->getRecipes()) {
    if (const VPInstruction *VPInst = dyn_cast<VPInstruction>(&Recipe)) {
      unsigned OpCode = VPInst->getOpcode();
      if (OpCode != Instruction::PHI) // Is this the same for semi-phi?
        break;
      markDivergent(VPInst);
      Seeds.push_back(VPInst);
    }
  }

  // The loop exit condition of the vector candidate loop is uniform because we
  // are comparing a scalar induction value that increases by VF with a uniform
  // loop upper bound. Adding this condition to UniformOverrides tells DA not to
  // consider this as a potential source of divergence. i.e., we know all lanes
  // will re-enter the loop body for each vector iteration.
  VPBasicBlock *ExitingBlock =
      cast<VPBasicBlock>(CandidateLoop->getExitingBlock());
  // Source of code divergence here away from community - no getTerminator()
  // method for VPBasicBlock since it is not derived from BasicBlock.
  // VPRecipe *ExitingRecipe = ExitingBlock->getTerminator();
  VPValue *ExitCond = ExitingBlock->getCondBit();
  UniformOverrides.insert(ExitCond);

  // Start with the known divergent values (e.g., vector candidate iv), and
  // propagate divergence info accordingly.
  for (auto *DivVal : Seeds) {
    for (const auto *User : DivVal->users()) {
      const auto *UserInst = dyn_cast<const VPInstruction>(User);
      if (!UserInst)
        continue;
      Worklist.push_back(UserInst);
    }
  }

  while (!Worklist.empty()) {
    const VPInstruction *I = Worklist.back();
    Worklist.pop_back();

    if (UniformOverrides.find(I) != UniformOverrides.end())
      continue;

    bool WasDivergent = isDivergent(I);
    if (WasDivergent)
      continue;

    // Branch instructions are not explicitly represented in VPlan, so check
    // to see if the current instruction is the same as the VPCondBit of the
    // parent block. If it is, then this tells us that we have a conditional
    // branch and BranchDependenceAnalysis can be used to tell us which phi
    // instructions are divergent. Note: the condition bit can be something
    // other than a cmp instruction.
    const VPValue *Cond = I->getParent()->getCondBit();
    if (I == Cond) {
      const VPBasicBlock *TermBlock = I->getParent();
      // Since we don't have an explicit branch instruction in VPlan, mark
      // the Condition Bit VPValue as divergent.
      markDivergent(Cond);
      for (const auto *JoinBlock :
           VPBDA.joinBlocks(const_cast<VPBasicBlock *>(TermBlock))) {
        DivergentBlocks.insert(JoinBlock);
        for (const VPRecipeBase &Recipe : JoinBlock->getRecipes()) {
          if (const VPInstruction *VPInst = dyn_cast<VPInstruction>(&Recipe)) {
            unsigned OpCode = VPInst->getOpcode();
            if (OpCode != Instruction::PHI)
              break;
            Worklist.push_back(VPInst);
          }
        }
      }
      continue;
    }

    // Update divergence info for this instruction
    bool UpdateDivergence = updateDivergenceInfo(I);

    if (UpdateDivergence) {
      markDivergent(I);
      for (const auto *User : I->users()) {
        const VPInstruction *UserInst = dyn_cast<VPInstruction>(User);
        if (!UserInst)
          continue;

        // Only compute divergence for instructions that are inside the loop.
        // TODO: is VPBlockBase the right thing to be used in the VPLoop
        // template? Should this actually be VPBasicBlock? Otherwise, unless
        // VPBlockBase is explicitly cast the compiler cannot correctly
        // resolve the type of the contains() argument, which results in a
        // run-time failure. In the community, Loop inherits from
        // LoopBase<BasicBlock, Loop>. We may need to mirror this to avoid
        // problems with other templated functions in Loop/VPLoop, etc. Also,
        // in the community BasicBlock is inherited from Value, but in VPlan
        // VPBasicBlock does not inherit from VPValue.
        if (!CandidateLoop->contains(cast<VPBlockBase>(UserInst->getParent())))
          continue;
        Worklist.push_back(UserInst);
      }
    }
  }

  LLVM_DEBUG(print(dbgs(), CandidateLoop));
}

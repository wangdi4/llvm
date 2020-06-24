#if INTEL_COLLAB
//===--- IntelVPlanDivergenceAnalysis.h - Divergence Analysis -*- C++ ---*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// \file
// The divergence analysis determines which instructions and branches are
// divergent given a set of divergent source instructions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_DIVERGENCE_ANALYSIS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_DIVERGENCE_ANALYSIS_H

#include "IntelVPlanSyncDependenceAnalysis.h"
#include "IntelVPlanVectorShape.h"
#include "llvm/ADT/DenseSet.h"
#include <queue>

namespace llvm {
namespace vpo {

class VPValue;
class VPInstruction;
class VPLoop;
class VPLoopInfo;
#if INTEL_CUSTOMIZATION
class VPVectorShape;
class VPPHINode;
class VPCmpInst;
class VPLoopEntityList;
class VPAllocatePrivate;
class VPInductionInit;
class VPInductionInitStep;
#endif // INTEL_CUSTOMIZATION

class VPDominatorTree;
class VPPostDominatorTree;

/// Generic divergence analysis for reducible CFGs.
///
/// This analysis propagates divergence in a data-parallel context from sources
/// of divergence to all users. It requires reducible CFGs. All assignments
/// should be in SSA form.
class VPlanDivergenceAnalysis {
public:
  /// This instance will analyze the whole function \p F or the loop \p
  /// RegionLoop.
  ///
  /// \param RegionLoop if non-null the analysis is restricted to \p RegionLoop.
  /// Otherwise the whole function is analyzed.
  /// \param IsLCSSAForm whether the analysis may assume that the IR in the
  /// region is in LCSSA form.
  // Note: this compute is a public interface for VPlan because we may want to
  // compute DA on demand after other VPlan transformations.
  void compute(VPlan *Plan, VPLoop *RegionLoop, VPLoopInfo *VPLI,
               VPDominatorTree &DT, VPPostDominatorTree &PDT,
               bool IsLCSSA = true);

  /// Recomputes the shapes of all the instructions in the \p Seeds set and
  /// triggers the recomputation of all dependent instructions.
  /// This function assumes that all the required information like VPlan,
  /// Dominator/Post-Dominator tree, etc. are unchanged from the previous
  /// invocation of the \p compute method.
  void recomputeShapes(SmallPtrSetImpl<VPInstruction *> &Seeds);

  /// Mark \p DivVal as a value that is always divergent.
  void markDivergent(const VPValue &DivVal);

  /// Mark \p UniVal as a value that is non-divergent.
  void markUniform(const VPValue &UniVal);

  /// Whether \p Val will always return a uniform value regardless of its
  /// operands
  bool isAlwaysUniform(const VPValue &Val) const;

  /// Whether \p Val is a divergent value
  bool isDivergent(const VPValue &Val) const;

#if INTEL_CUSTOMIZATION
  /// Return the vector shape for \p V.
  VPVectorShape getVectorShape(const VPValue *V) const;

  /// Updates the vector shape for \p V, if necessary.
  /// Returns true if the shape was updated.
  bool updateVectorShape(const VPValue *V, VPVectorShape Shape);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, const VPLoop *VPLp = nullptr);
#endif // !NDEBUG || LLVM_ENABLE_DUMP
#endif // INTEL_CUSTOMIZATION

  /// Return \p true if the given pointer is unit-strided(1 or -1).
  bool isUnitStridePtr(const VPValue *Ptr) const;

  /// Return \p true if the given pointer is unit-strided(1 or -1).
  /// \p IsNegOneStride is set to true if stride is -1 and false otherwise.
  bool isUnitStridePtr(const VPValue *VPPtr, bool &IsNegOneStride) const;

  void updateDivergence(const VPValue &Val) {
    assert(isa<VPInstruction>(&Val) &&
           "Expected a VPInstruction as input argument.");
    auto NewShape = computeVectorShape(cast<VPInstruction>(&Val));
    updateVectorShape(&Val, NewShape);
  }

  // Clone instructions' vector shapes when we clone VPlan. This function is
  // used when DA recomputation is not allowed.
  void cloneVectorShapes(VPlan *ClonedVPlan,
                         DenseMap<VPValue *, VPValue *> &OrigClonedValuesMap);

  // Disable DA recomputation. It is used when we clone DA during VPlan cloning.
  // When we clone DA after the predicator, we cannot compute/recompute DA.
  void disableDARecomputation() { DARecomputationDisabled = true; }

  /// Returns true if OldShape is not equal to NewShape.
  bool shapesAreDifferent(VPVectorShape OldShape, VPVectorShape NewShape);

private:
  /// Propagate divergence to all instructions in the region.
  /// Divergence is seeded by calls to \p markDivergent.
  void computeImpl();

  /// Push the instruction to the Worklist.
  bool pushToWorklist(const VPInstruction &I);

  /// Pop the instruction from the Worklist.
  const VPInstruction *popFromWorklist();

  // Clear the Worklist.
  void clearWorklist() {
    std::queue<const VPInstruction*> EmptyWL;
    std::swap(Worklist, EmptyWL);
    OnWorklist.clear();
  }

  /// Whether \p BB is part of the region.
  bool inRegion(const VPBasicBlock &BB) const;

  /// Whether \p I is part of the region.
  bool inRegion(const VPInstruction &I) const;

  bool updatePHINode(const VPInstruction &Phi) const;

  /// Computes whether \p Inst is divergent based on the
  /// divergence of its operands.
  ///
  /// \returns Whether \p Inst is divergent.
  ///
  /// This should only be called for non-phi, non-terminator instructions.
  bool updateNormalInstruction(const VPInstruction &Inst) const;

  /// Mark users of live-out users as divergent.
  ///
  /// \param LoopHeader the header of the divergent loop.
  ///
  /// Marks all users of live-out values of the loop headed by \p LoopHeader.
  /// as divergent and puts them on the worklist.
  void taintLoopLiveOuts(const VPBasicBlock &LoopHeader);

  /// Push users of \p Val (in the region) to the worklist.
  void pushUsers(const VPValue &V);

  /// Push all phi nodes in \p Block to the worklist if \p PushAll is true.
  /// If \p PushAll is false, only those phi nodes that have not already been
  /// identified as divergent are pushed.
  void pushPHINodes(const VPBasicBlock &Block, bool PushAll); // INTEL

  /// Mark \p Block as join divergent
  ///
  /// A block is join divergent if two threads may reach it from different
  /// incoming blocks at the same time.
  void markBlockJoinDivergent(const VPBasicBlock &Block) {
    DivergentJoinBlocks.insert(&Block);
  }

  /// Mark \p Block as divergent loop-exit block.
  bool addDivergentLoopExit(const VPBasicBlock &Block) {
    return DivergentLoopExits.insert(&Block).second;
  }

  /// Mark \p Loop as divergent.
  bool addDivergentLoops(const VPLoop &VPLp) {
    return DivergentLoops.insert(&VPLp).second;
  }

  /// Return \p true if \p Loop is divergent.
  bool isDivergentLoop(const VPLoop &VPLp) const {
    return DivergentLoops.find(&VPLp) != DivergentLoops.end();
  }

  /// Return \p true if \p Block is a divergent loop-exit block.
  bool isDivergentLoopExit(const VPBasicBlock &Block) const {
    return DivergentLoopExits.find(&Block) != DivergentLoopExits.end();
  }

  /// Whether \p Block is join divergent
  ///
  /// (see markBlockJoinDivergent).
  bool isJoinDivergent(const VPBasicBlock &Block) const {
    return DivergentJoinBlocks.find(&Block) != DivergentJoinBlocks.end();
  }

  bool addJoinDivergentBlock(const VPBasicBlock &Block) {
    return DivergentJoinBlocks.insert(&Block).second;
  }

  /// Whether \p Val is divergent when read in \p ObservingBlock.
  bool isTemporalDivergent(const VPBasicBlock &ObservingBlock,
                           const VPValue &Val) const;

  /// Get the vector shape of \p Val observed in \p ObserverBlock. This will
  /// be varying if \p Val is defined in divergent loop.
  VPVectorShape getObservedShape(const VPBasicBlock &ObserverBlock,
                                 const VPValue &Val);

  /// Propagate control-induced divergence to users (phi nodes and
  /// instructions).
  //
  // \param JoinBlock is a divergent loop exit or join point of two disjoint
  // paths.
  // \returns Whether \p JoinBlock is a divergent loop exit of \p TermLoop.
  bool propagateJoinDivergence(const VPBasicBlock &JoinBlock,
                               const VPLoop *TermLoop);

  /// Propagate induced value divergence due to control divergence in the
  /// CondBit of \p CondBlock.
  void propagateBranchDivergence(const VPBasicBlock *CondBlock); // INTEL

  /// Propagate divergent caused by a divergent loop exit.
  ///
  /// \param ExitingLoop is a divergent loop.
  void propagateLoopDivergence(const VPLoop &ExitingLoop);

  /// Return the type size in bytes.
  unsigned getTypeSizeInBytes(Type *Ty) const;

#if INTEL_CUSTOMIZATION
  /// Compute vector shape of \p I.
  VPVectorShape computeVectorShape(const VPInstruction *I);

  /// Computes vector shapes for all unary instructions.
  VPVectorShape computeVectorShapeForUnaryInst(const VPInstruction *I);

  /// Computes vector shapes for all binary instructions. E.g., add, sub, etc.
  VPVectorShape computeVectorShapeForBinaryInst(const VPInstruction *I);

  /// Computes vector shapes for cast instructions, sitofp, sext, ptrtoint, etc.
  VPVectorShape computeVectorShapeForCastInst(const VPInstruction *I);

  /// Computes vector shapes for memory address computation instructions
  /// (includes GEP and VPSubscript).
  VPVectorShape computeVectorShapeForMemAddrInst(const VPInstruction *I);

  VPVectorShape computeVectorShapeForSOAGepInst(const VPInstruction *I);

  /// Computes vector shapes for phi nodes.
  VPVectorShape computeVectorShapeForPhiNode(const VPPHINode *Phi);

  /// Computes vector shape for load instructions.
  VPVectorShape computeVectorShapeForLoadInst(const VPInstruction *I);

  /// Computes vector shape for store instructions.
  VPVectorShape computeVectorShapeForStoreInst(const VPInstruction *I);

  /// Computes vector shape for the different types of cmp instructions.
  VPVectorShape computeVectorShapeForCmpInst(const VPCmpInst *I);

  /// Computes vector shape for extract element instructions.
  VPVectorShape computeVectorShapeForInsertExtractInst(const VPInstruction *I);

  /// Computes vector shape for select instructions.
  VPVectorShape computeVectorShapeForSelectInst(const VPInstruction *I);

  /// Computes vector shape for call instructions.
  VPVectorShape computeVectorShapeForCallInst(const VPInstruction *I);

  /// Computes vector shape for AllocatePrivate instructions.
  VPVectorShape
  computeVectorShapeForAllocatePrivateInst(const VPAllocatePrivate *AI);

  /// Computes vector shape for induction-init instruction.
  VPVectorShape computeVectorShapeForInductionInit(const VPInductionInit *Init);

  /// Returns a uniform vector shape.
  VPVectorShape getUniformVectorShape();

  /// Returns a random vector shape.
  VPVectorShape getRandomVectorShape();

  /// Returns a sequential vector shape with the given stride.
  VPVectorShape getSequentialVectorShape(int64_t Stride);

  /// Returns a strided vector shape with the given stride.
  VPVectorShape getStridedVectorShape(int64_t Stride);

  /// Return true if the given variable has VectorShape.
  bool isSOAShape(const VPValue *Val) const;

  /// Returns a SOASequential vector shape with the given stride.
  VPVectorShape getSOASequentialVectorShape(int64_t Stride);

  /// Returns a SOARandom vector shape with the given stride.
  VPVectorShape getSOARandomVectorShape();

  /// Returns in integer value in \p IntVal if \p V is an integer VPConstant.
  bool getConstantIntVal(VPValue *V, int64_t &IntVal);

  /// Returns a VPConstant of \p Val.
  VPConstant* getConstantInt(int64_t Val);

  /// Verify that there are no undefined shapes after divergence analysis.
  /// Also ensure that divergent/uniform properties are consistent with vector
  /// shapes.
  void verifyVectorShapes();

  /// Verify the shape of each instruction in give Block \p VPBB.
  void verifyBasicBlock(const VPBasicBlock *VPBB);

  /// Improve stride information where possible by using information provided
  /// by underlying IR.
  void improveStrideUsingIR();

  VPlan *Plan;

  // If regionLoop != nullptr, analysis is only performed within \p RegionLoop.
  // Otw, analyze the whole function
  VPLoop *RegionLoop;

  // Shape information of divergent values.
  DenseMap<const VPValue *, VPVectorShape> VectorShapes;
#endif // INTEL_CUSTOMIZATION

  VPDominatorTree *DT;
  VPPostDominatorTree *PDT;
  VPLoopInfo *VPLI;

  // Recognized divergent loops
  DenseSet<const VPLoop *> DivergentLoops;

  // The SDA links divergent branches to divergent control-flow joins.
  std::unique_ptr<SyncDependenceAnalysis> SDA;

  // Use simplified code path for LCSSA form.
  bool IsLCSSAForm;

  // Blocks with joining divergent control from different predecessors.
  DenseSet<const VPBasicBlock *> DivergentJoinBlocks;

  // Blocks which are loop-exits and result in divergent Control-flow.
  DenseSet<const VPBasicBlock *> DivergentLoopExits;

  // Internal worklist for divergence propagation.
  std::queue<const VPInstruction *> Worklist;

  // Unique-elements of the Worklist.
  DenseSet<const VPInstruction*> OnWorklist;

  // Keep track of instructions that form CondBits and the actual block(s)
  // containing the CondBit.
  using BlockVectorTy = SmallVector<const VPBasicBlock *, 4>;
  DenseMap<const VPValue *, BlockVectorTy> CondBit2BlockMap;

  // Disable DA recalculation.
  bool DARecomputationDisabled = false;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_DIVERGENCE_ANALYSIS_H
#endif //INTEL_COLLAB

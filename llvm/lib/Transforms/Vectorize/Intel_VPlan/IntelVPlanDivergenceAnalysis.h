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

#include "IntelVPlanLoopInfo.h"
#include "IntelVPlanVectorShape.h"
#include "llvm/ADT/DenseSet.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLDDNode.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#endif

namespace llvm {
namespace vpo {

class VPValue;
class VPInstruction;
class VPBlockBase;
class VPLoop;
class SyncDependenceAnalysis;
#if INTEL_CUSTOMIZATION
class VPVectorShape;
class VPPHINode;
class VPCmpInst;
class VPLoopEntityList;
#endif

using VPDominatorTree = DomTreeBase<VPBlockBase>;
using VPPostDominatorTree = PostDomTreeBase<VPBlockBase>;

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

  /// The loop that defines the analyzed region (if any).
  const VPLoop *getRegionLoop() const { return RegionLoop; }

  /// Mark \p DivVal as a value that is always divergent.
  void markDivergent(const VPValue &DivVal);

  /// Whether any value was marked or analyzed to be divergent.
  bool hasDetectedDivergence() const { return !DivergentValues.empty(); }

  /// Whether \p Val will always return a uniform value regardless of its
  /// operands
  bool isAlwaysUniform(const VPValue &Val) const;

  /// Whether \p Val is a divergent value
  bool isDivergent(const VPValue &Val) const;

#if INTEL_CUSTOMIZATION
  /// Return the vector shape for \p V.
  VPVectorShape* getVectorShape(const VPValue *V) const;

  /// Updates the vector shape for \p V, if necessary.
  /// Returns true if the shape was updated.
  bool updateVectorShape(const VPValue *V, VPVectorShape* Shape);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, const VPLoop *VPLp);
#endif // !NDEBUG || LLVM_ENABLE_DUMP
#endif // INTEL_CUSTOMIZATION

  /// Return \p true if the given pointer is unit-stride.
  bool isUnitStridePtr(const VPValue *Ptr) const;

private:
  /// Whether \p BB is part of the region.
  bool inRegion(const VPBlockBase &BB) const;
  /// Whether \p I is part of the region.
  bool inRegion(const VPInstruction &I) const;

  /// Mark \p UniVal as a value that is always uniform.
  void addUniformOverride(const VPValue &UniVal);

  /// Propagate divergence to all instructions in the region.
  /// Divergence is seeded by calls to \p markDivergent.
  void computeImpl();

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
  /// Marks all users of live-out values of the loop headed by \p LoopHeader
  /// as divergent and puts them on the worklist.
  void taintLoopLiveOuts(const VPBlockBase &LoopHeader);

  /// Push only non-divergent users of \p Val (in the region) to the worklist
  /// unless \p PushAll is true;
  void pushUsers(const VPValue &V, bool PushAll=false); // INTEL

  /// Push all phi nodes in \p Block to the worklist if \p PushAll is true.
  /// If \p PushAll is false, only those phi nodes that have not already been
  /// identified as divergent are pushed.
  void pushPHINodes(const VPBlockBase &Block, bool PushAll); // INTEL

  /// Mark \p Block as join divergent
  ///
  /// A block is join divergent if two threads may reach it from different
  /// incoming blocks at the same time.
  void markBlockJoinDivergent(const VPBlockBase &Block) {
    DivergentJoinBlocks.insert(&Block);
  }

  /// Whether \p Val is divergent when read in \p ObservingBlock.
  bool isTemporalDivergent(const VPBlockBase &ObservingBlock,
                           const VPValue &Val) const;

  /// Whether \p Block is join divergent
  ///
  /// (see markBlockJoinDivergent).
  bool isJoinDivergent(const VPBlockBase &Block) const {
    return DivergentJoinBlocks.find(&Block) != DivergentJoinBlocks.end();
  }

  /// Propagate control-induced divergence to users (phi nodes and
  /// instructions).
  //
  // \param JoinBlock is a divergent loop exit or join point of two disjoint
  // paths.
  // \returns Whether \p JoinBlock is a divergent loop exit of \p TermLoop.
  bool propagateJoinDivergence(const VPBlockBase &JoinBlock,
                               const VPLoop *TermLoop);

  /// Propagate induced value divergence due to control divergence in \p Term.
  void propagateBranchDivergence(const VPValue &Cond); // INTEL

  /// Propagate divergent caused by a divergent loop exit.
  ///
  /// \param ExitingLoop is a divergent loop.
  void propagateLoopDivergence(const VPLoop &ExitingLoop);

  /// Return the type size in bytes.
  unsigned getTypeSizeInBytes(Type *Ty) const;

#if INTEL_CUSTOMIZATION
  /// Initialize shapes before propagation.
  void initializeShapes(SmallVectorImpl<const VPInstruction*> &PhiNodes);

  /// Returns true if OldShape is not equal to NewShape.
  bool shapesAreDifferent(VPVectorShape* OldShape, VPVectorShape* NewShape);

  /// Set any remaining shapes for instructions that stayed uniform after
  /// divergence propagation.
  void setVectorShapesForUniforms(const VPLoop *VPLp);

  /// Compute vector shape of \p I.
  VPVectorShape* computeVectorShape(const VPInstruction *I);

  /// Computes vector shapes for all binary instructions. E.g., add, sub, etc.
  VPVectorShape* computeVectorShapeForBinaryInst(const VPInstruction *I);

  /// Computes vector shapes for cast instructions, sitofp, sext, ptrtoint, etc.
  VPVectorShape* computeVectorShapeForCastInst(const VPInstruction *I);

  /// Computes vector shapes for gep instructions.
  VPVectorShape* computeVectorShapeForGepInst(const VPInstruction *I);

  /// Computes vector shapes for phi nodes.
  VPVectorShape* computeVectorShapeForPhiNode(const VPPHINode *Phi);

  /// Computes vector shape for load instructions.
  VPVectorShape* computeVectorShapeForLoadInst(const VPInstruction *I);

  /// Computes vector shape for store instructions.
  VPVectorShape* computeVectorShapeForStoreInst(const VPInstruction *I);

  /// Computes vector shape for the different types of cmp instructions.
  VPVectorShape* computeVectorShapeForCmpInst(const VPCmpInst *I);

  /// Computes vector shape for extract element instructions.
  VPVectorShape* computeVectorShapeForInsertExtractInst(const VPInstruction *I);

  /// Computes vector shape for select instructions.
  VPVectorShape* computeVectorShapeForSelectInst(const VPInstruction *I);

  /// Computes vector shape for call instructions.
  VPVectorShape* computeVectorShapeForCallInst(const VPInstruction *I);

  /// Returns a uniform vector shape.
  VPVectorShape* getUniformVectorShape();

  /// Returns a random vector shape.
  VPVectorShape* getRandomVectorShape();

  /// Returns a sequential vector shape with the given stride.
  VPVectorShape *getSequentialVectorShape(uint64_t Stride);

  /// Returns a strided vector shape with the given stride.
  VPVectorShape *getStridedVectorShape(uint64_t Stride);

  /// Returns in integer value in \p IntVal if \p V is an integer VPConstant.
  bool getConstantIntVal(VPValue *V, uint64_t &IntVal);

  /// Returns a VPConstant of \p Val.
  VPConstant* getConstantInt(int64_t Val);

  /// Returns true if \p V is a uniform VPValue.
  bool isUniformLoopEntity(const VPValue *V) const;

  /// Push operands of \p I to the worklist. Used when some operands vector
  /// shapes are needed to compute the vector shape of \p I.
  bool pushMissingOperands(const VPInstruction &I);

  /// Verify that there are not undefined shapes after divergence analysis.
  /// Also ensure that divergent/uniform properties are consistent with vector
  /// shapes.
  void verifyVectorShapes(const VPLoop *VPLp);

  VPlan *Plan;

  // If regionLoop != nullptr, analysis is only performed within \p RegionLoop.
  // Otw, analyze the whole function
  VPLoop *RegionLoop;

  // Provides information on uniform, linear, private(vector), etc.
  VPLoopEntityList *RegionLoopEntities;
#endif // INTEL_CUSTOMIZATION

  VPDominatorTree *DT;
  VPLoopInfo *VPLI;

  // Recognized divergent loops
  DenseSet<const VPLoop *> DivergentLoops;

  // The SDA links divergent branches to divergent control-flow joins.
  SyncDependenceAnalysis *SDA;

  // Use simplified code path for LCSSA form.
  bool IsLCSSAForm;

  // Set of known-uniform values.
  DenseSet<const VPValue *> UniformOverrides;

  // Blocks with joining divergent control from different predecessors.
  DenseSet<const VPBlockBase *> DivergentJoinBlocks;

  // Detected/marked divergent values.
  DenseSet<const VPValue *> DivergentValues;

#if INTEL_CUSTOMIZATION
  // Shape information of divergent values.
  DenseMap<const VPValue *, std::unique_ptr<VPVectorShape>> VectorShapes;

  // Undefined shapes are not stored in VectorShapes, but we need a singleton
  // object to compare against other shapes.
  std::unique_ptr<VPVectorShape> UndefShape;
#endif

  // Internal worklist for divergence propagation.
  SmallVector<const VPInstruction *, 8> Worklist;

#if INTEL_CUSTOMIZATION

  /// Mark \p DivVal as a value that is non-divergent.
  void markNonDivergent(const VPValue *DivVal);

  // Internal list of privates/induction/reductions collected from
  // Loop-entities, to help functions like markDivergent and isAlwaysUniform
  // distinguish between regular VPExternalDefs and Private pointers and their
  // aliases which are outside the Loop and also appear as 'VPExternalDefs' in
  // the representation.
  DenseSet<VPValue *> DivergentLoopEntities;

  // Mark all relevant loop-entities as Divergent.
  template <typename EntitiesRange>
  void markEntitiesAsDivergent(const EntitiesRange &Range);
#endif
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_DIVERGENCE_ANALYSIS_H
#endif //INTEL_COLLAB

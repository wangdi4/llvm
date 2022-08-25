//===- IntelVPlan.h - Represent A Vectorizer Plan -------------------------===//
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2021-2022 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the Vectorization Plan base classes:
// 1. Specializations of GraphTraits that allow VPBasicBlock graphs to be treated
//    as proper graphs for generic algorithms;
// 2. VPInstruction and its sub-classes represent instructions contained within
//    VPBasicBlocks;
// 3. The VPlan class holding a candidate for vectorization;
// 4. The VPlanUtils class providing methods for building plans;
// 5. The VPlanPrinter class providing a way to print a plan in dot format.
// These are documented in docs/VectorizationPlan.rst.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLAN_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLAN_H

#include "IntelVPBasicBlock.h"
#include "IntelVPLoopAnalysis.h"
#include "IntelVPlanAlignmentAnalysis.h"
#include "IntelVPlanDivergenceAnalysis.h"
#include "IntelVPlanExternals.h"
#include "IntelVPlanLoopInfo.h"
#include "IntelVPlanScalVecAnalysis.h"
#include "IntelVPlanValue.h"
#include "IntelVPlanValueTracking.h"
#include "VPlanHIR/IntelVPlanInstructionDataHIR.h"
#include "VPlanHIR/IntelVPlanScalarEvolutionHIR.h"
#include "VPlanHIR/IntelVPlanValueTrackingHIR.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/ADT/simple_ilist.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLGoto.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Analysis/Intel_OptReport/Diag.h"
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/GenericDomTreeConstruction.h"
#include "llvm/Support/raw_ostream.h"
#include <atomic>

namespace llvm {
// The (re)use of existing LoopVectorize classes is subject to future VPlan
// refactoring.
// namespace {
class InnerLoopVectorizer;
class LoopVectorizationLegality;
class LoopInfo;
//}

namespace vpo {
class VPBasicBlock;
class VPlan;
} // namespace vpo

template <> struct ilist_traits<vpo::VPBasicBlock> {
private:
  friend class vpo::VPlan;

  vpo::VPlan *Parent;

  using instr_iterator =
      simple_ilist<vpo::VPBasicBlock, ilist_sentinel_tracking<true>>::iterator;

public:
  void addNodeToList(vpo::VPBasicBlock *VPBB);
  void removeNodeFromList(vpo::VPBasicBlock *VPBB);
  void transferNodesFromList(ilist_traits &FromList, instr_iterator First,
                             instr_iterator Last);
  void deleteNode(vpo::VPBasicBlock *VPBB);
};

namespace loopopt {
class RegDDRef;
class HLLoop;
} // namespace loopopt

namespace vpo {
class SyncDependenceAnalysis;
class VPValue;
class VPOCodeGen;
class VPOCodeGenHIR;
class VPOVectorizationLegality;
class VPDominatorTree;
class VPPostDominatorTree;
#if INTEL_CUSTOMIZATION
// To be later declared as a friend
class VPlanCostModelInterface;
namespace VPlanCostModelHeuristics {
class HeuristicSLP;
} // namespace VPlanCostModelHeuristics
#endif // INTEL_CUSTOMIZATION
class VPlanDivergenceAnalysis;
class VPlanBranchDependenceAnalysis;
class VPValueMapper;
class VPlanMasked;
class VPlanScalarPeel;

typedef SmallPtrSet<VPValue *, 8> UniformsTy;

// Class names mapping to minimize the diff:
#define InnerLoopVectorizer VPOCodeGen
#define LoopVectorizationLegality VPOVectorizationLegality

struct TripCountInfo;

// This abstract class is used to create all the necessary analyses that are
// needed for VPlan. They are implemented by the 2 derived classes :
// VPAnalysesFactory and VPAnalysesFactoryHIR.
class VPAnalysesFactoryBase {
private:
  DominatorTree *DT = nullptr;
  AssumptionCache *AC = nullptr;
  const DataLayout *DL = nullptr;

protected:
  virtual ~VPAnalysesFactoryBase() {}

public:
  VPAnalysesFactoryBase(DominatorTree *DT, AssumptionCache *AC,
                        const DataLayout *DL)
      : DT(DT), AC(AC), DL(DL) {}

  virtual std::unique_ptr<VPlanScalarEvolution> createVPSE() = 0;

  virtual std::unique_ptr<VPlanValueTracking>
  createVPVT(VPlanScalarEvolution *VPSE) = 0;

  DominatorTree *getDominatorTree() { return DT; }
  AssumptionCache *getAssumptionCache() { return AC; }
  const DataLayout *getDataLayout() { return DL; }
};

class VPAnalysesFactory final : public VPAnalysesFactoryBase {
private:
  ScalarEvolution &SE;
  const Loop *Lp = nullptr;

public:
  VPAnalysesFactory(ScalarEvolution &SE, Loop *Lp, DominatorTree *DT,
                    AssumptionCache *AC, const DataLayout *DL)
      : VPAnalysesFactoryBase(DT, AC, DL), SE(SE), Lp(Lp) {}

  std::unique_ptr<VPlanScalarEvolution> createVPSE() override {
    auto &Context = Lp->getHeader()->getContext();
    return std::make_unique<VPlanScalarEvolutionLLVM>(SE, Lp, Context,
                                                      getDataLayout());
  }

  std::unique_ptr<VPlanValueTracking>
  createVPVT(VPlanScalarEvolution *VPSE) override {
    auto *VPSELLVM = static_cast<VPlanScalarEvolutionLLVM *>(VPSE);
    return std::make_unique<VPlanValueTrackingLLVM>(
        *VPSELLVM, *getDataLayout(), getAssumptionCache(), getDominatorTree());
  }

  const Loop *getLoop() { return Lp; }
};

class VPAnalysesFactoryHIR final : public VPAnalysesFactoryBase {
private:
  loopopt::HLLoop *HLp = nullptr;

public:
  VPAnalysesFactoryHIR(loopopt::HLLoop *HLp, DominatorTree *DT,
                       AssumptionCache *AC, const DataLayout *DL)
      : VPAnalysesFactoryBase(DT, AC, DL), HLp(HLp) {}

  std::unique_ptr<VPlanScalarEvolution> createVPSE() override {
    return std::make_unique<VPlanScalarEvolutionHIR>(HLp);
  }

  std::unique_ptr<VPlanValueTracking>
  createVPVT(VPlanScalarEvolution *VPSE) override {
    return std::make_unique<VPlanValueTrackingHIR>(
        HLp, *getDataLayout(), getAssumptionCache(), getDominatorTree());
  }

  loopopt::HLLoop *getLoop() { return HLp; }
};

/// Enumeration to uniquely identify various VPlan analyses.
// TODO: Consider introducing an inheritance hierarchy for VPlan analyses i.e. a
// VPAnalysis base class and DA, VLS and SVA inheriting from this base class.
// This enum can be then converted into subclass ID mechanism and virtual
// functionalities like running/clearing analyses will be specialized in the
// derived classes.
enum class VPAnalysisID {
  DA = 0,
  VLS = 1,
  SVA = 2,
  LastVPAnalysis = SVA
};

template <typename ParentTy, typename NodeTy>
ParentTy *getListOwner(ilist_traits<NodeTy> *NodeList) {
  size_t Offset(
      size_t(&((ParentTy *)nullptr->*ParentTy::getSublistAccess(
                                         static_cast<NodeTy *>(nullptr)))));
  using ListTy = ilist<NodeTy, ilist_sentinel_tracking<true>>;
  ListTy *Anchor(static_cast<ListTy *>(NodeList));
  return reinterpret_cast<ParentTy *>(reinterpret_cast<char *>(Anchor) -
                                      Offset);
}

/// In what follows, the term "input IR" refers to code that is fed into the
/// vectorizer whereas the term "output IR" refers to code that is generated by
/// the vectorizer.

/// VPIteration represents a single point in the iteration space of the output
/// (vectorized and/or unrolled) IR loop.
struct VPIteration {
  unsigned Part; ///< in [0..UF)
  unsigned Lane; ///< in [0..VF)
  VPIteration(unsigned Part, unsigned Lane) : Part(Part), Lane(Lane) {}
  VPIteration(unsigned Part) : Part(Part), Lane(ALL_LANES()) {}
  static unsigned ALL_LANES() { return -1; }
};

/// This class is used to enable the VPlan to invoke a method of ILV. This is
/// needed until the method is refactored out of ILV and becomes reusable.
struct VPCallback {
  virtual ~VPCallback() {}
  virtual Value *getOrCreateVectorValues(Value *V, unsigned Part) = 0;
};

/// VPTransformState holds information passed down when "executing" a VPlan,
/// needed for generating the output IR.
struct VPTransformState {
  VPTransformState(unsigned VF, unsigned UF, LoopInfo *LI,
                   class DominatorTree *DT, IRBuilder<> &Builder,
                   InnerLoopVectorizer *ILV, VPCallback &Callback,
                   VPLoopInfo *VPLI)
      : VF(VF), UF(UF), Instance(), LI(LI), DT(DT), Builder(Builder), ILV(ILV),
        Callback(Callback), VPLI(VPLI) {}

  /// The chosen Vectorization and Unroll Factors of the loop being vectorized.
  unsigned VF;
  unsigned UF;

  /// Hold the indices to generate specific scalar instructions. Null indicates
  /// that all instances are to be generated, using either scalar or vector
  /// instructions.
  Optional<VPIteration> Instance;

  struct DataState {
    /// A type for vectorized values in the new loop. Each value from the
    /// original loop, when vectorized, is represented by UF vector values in
    /// the new unrolled loop, where UF is the unroll factor.
    typedef SmallVector<Value *, 2> PerPartValuesTy;

    DenseMap<VPValue *, PerPartValuesTy> PerPartOutput;
  } Data;

  /// Get the generated Value for a given VPValue and a given Part. If such a
  /// Value does not exist by the time this method is called, then this Def
  /// represents Values that are still generated by ILV via ValueMap.
  Value *get(VPValue *Def, unsigned Part) {
    if (Data.PerPartOutput.count(Def))
      return Data.PerPartOutput[Def][Part];
    // Bring the Values from ValueMap.
    return Callback.getOrCreateVectorValues(VPValue2Value[Def], Part);
  }

  /// Set the generated Value for a given VPValue and a given Part.
  void set(VPValue *Def, Value *V, unsigned Part) {
    if (!Data.PerPartOutput.count(Def)) {
      DataState::PerPartValuesTy Entry(UF);
      Data.PerPartOutput[Def] = Entry;
    }
    Data.PerPartOutput[Def][Part] = V;
  }
  /// Hold state information used when constructing the CFG of the output IR,
  /// traversing the VPBasicBlocks and generating corresponding IR BasicBlocks.
  struct CFGState {
    /// The previous VPBasicBlock visited. Initially set to null.
    VPBasicBlock *PrevVPBB = nullptr;
    /// The first VPBasicBlock that will be executed on the vector loop path.
    /// I.e. the first block that can contain vectorized code. The blocks before
    /// it should not contain vector instructions that are used in the vector
    /// loop.
    VPBasicBlock *FirstExecutableVPBB = nullptr;
    /// The previous IR BasicBlock created or used. Initially set to the new
    /// header BasicBlock.
    BasicBlock *PrevBB = nullptr;
    /// The last IR BasicBlock in the output IR. Set to the new latch
    /// BasicBlock, used for placing the newly created BasicBlocks.
    BasicBlock *InsertBefore = nullptr;
    /// A mapping of each VPBasicBlock to the corresponding BasicBlock. In case
    /// of replication, maps the BasicBlock of the last replica created.
    SmallDenseMap<VPBasicBlock *, BasicBlock *> VPBB2IRBB;
    /// A mapping of each VPBasicBlock and the last BasicBlock created for the
    /// same.
    SmallDenseMap<VPBasicBlock *, BasicBlock *> VPBB2IREndBB;
    /// Vector of VPBasicBlocks whose terminator instruction needs to be fixed
    /// up at the end of vector code generation.
    SmallVector<VPBasicBlock *, 8> VPBBsToFix;
  } CFG;

  /// Hold a pointer to LoopInfo to register new basic blocks in the loop.
  class LoopInfo *LI;

  /// Hold a pointer to Dominator Tree to register new basic blocks in the loop.
  class DominatorTree *DT;

  /// Hold a reference to the IRBuilder used to generate output IR code.
  IRBuilder<> &Builder;

  /// Hold a reference to a mapping between VPValues in VPlan and original
  /// Values they correspond to.
  VPValue2ValueTy VPValue2Value;

  /// Hold a pointer to InnerLoopVectorizer to reuse its IR generation methods.
  class InnerLoopVectorizer *ILV;

  VPCallback &Callback;
  VPLoopInfo *VPLI;
};

/// Class to model a single VPlan-level instruction - it may generate a sequence
/// of IR instructions when executed, these instructions would always form a
/// single-def expression as the VPInstruction is also a single def-use vertex.
///
#if INTEL_CUSTOMIZATION
/// For HIR, we classify VPInstructions into 3 sub-types:
///   1) Master VPInstruction: It has underlying HIR data attached and its
///      operands could have been decomposed or not. If so, this VPInstruction
///      is the last one in the UD chain of the group of decomposed
///      VPInstructions. If this VPInstruction or any of its decomposed ones are
///      modified, the HIR will automatically be marked as invalid.
///   2) Decomposed VPInstruction: It's created as a result of decomposing a
///      master VPInstruction. It doesn't have underlying HIR directly attached
///      but it has a pointer to its master VPInstruction holding it. In order
///      to check whether the underlying HIR of a decomposed VPInstruction is
///      valid, its master VPInstruction must be checked.
///   3) New VPInstruction: It's created as a result of a VPlan-to-VPlan
///      transformation, excluding decomposition. It doesn't have underlying HIR
///      or master VPInstruction attached. New VPInstructions also exist in the
///      LLVM-IR path.
///
/// DESIGN PRINCIPLE: access to the underlying IR is forbidden by default. Only
/// the front-end and back-end of VPlan should have access to the underlying IR
/// and be aware of the VPInstruction sub-type (master/decomposed/new)
/// instructions. Some well-delimited VPlan analyses, previous design review
/// approval, may also need to have access to the underlying IR and
/// VPInstruction sub-types to bring analysis information computed on the input
/// IR to VPlan. The remaining VPlan algorithms should process all the
/// VPInstructions ignoring their underlying IR and sub-type. For these
/// reasons, adding new friends to this class must be very well justified.
///
/// DESIGN DECISION: for VPO, we decided to pay the memory and design cost of
/// having LLVM-IR data (Inst) HIR data (MasterData) and their respective
/// interfaces in the same class in favor of minimizing divergence with the
/// community. We know that this is not the best design but creating a
/// VPInstructionHIR sub-class would be complicated because VPInstruction also
/// has sub-classes (VPCmpInst, VPPHINode, etc.) that would need to be
/// replicated under the VPInstructionHIR.
#endif
class VPInstruction : public VPUser,
                      public ilist_node_with_parent<VPInstruction, VPBasicBlock,
                                             ilist_sentinel_tracking<true>> {
  friend class VPBasicBlock;
  friend class VPBranchInst;
  friend class VPBuilder;
  friend class VPlanTTICostModel;
  friend class VPlanCostModelInterface;
  friend class VPlanDivergenceAnalysis;
  friend class VPlanValueTrackingLLVM;
  friend class VPlanVLSAnalysis;
  friend class VPlanVerifier;
  friend class VPOCodeGen;
  friend class VPCloneUtils;
  friend class VPValueMapper;
  friend class VPLoopEntityList;
  friend class VPTransformLibraryCall; // Needed for `copyUnderlyingFrom`
  friend class VPValue;
  // FIXME: This is only needed to support buggy mixed HIR codegen. Once we
  // retire it and use full VPValue-based codegen, underlying IR copying won't
  // be necessary.
  friend class VPlanPredicator;

  // TODO: Integrate SLP natively into VPlan instead.
  friend class VPlanCostModelHeuristics::HeuristicSLP;
  friend class VPlanIdioms;

#if INTEL_CUSTOMIZATION
  friend class HIRSpecifics;
  friend class VPBuilderHIR;
  friend class VPDecomposerHIR;
  friend class VPlanVLSAnalysisHIR;
  // To get underlying HIRData until we have proper VPType.
  friend class VPVLSClientMemrefHIR;
  friend class VPOCodeGenHIR;
#endif // INTEL_CUSTOMIZATION

  /// Central class to capture and differentiate operator-specific attributes
  /// that are attached to an instruction. All operators in LLVM are mutually
  /// exclusive, hence we use tagged unions to explicitly identify which
  /// specific type of operator flags is currently held.
  struct VPOperatorIRFlags {
  private:
    union {
      FastMathFlags FMF;
      std::bitset<2> OverflowFlags;
      unsigned ExactFlag : 1;
      unsigned NonOperator : 1;
    };

  public:
    // Helpful internal enum to tag current type of VPOperatorIRFlags.
    enum class FlagsKind {
      UnknownOperatorFlags = 0,
      VPFastMathFlags = 1,
      VPOverflowingFlags = 2,
      VPExactFlags = 3,
      NumFlagsKind
    };

    enum class OverflowFlagsKind {
      NSWFlag = 0,
      NUWFlag = 1
    };

    // Default initialize the flags based on kind of operator that this
    // VPInstruction corresponds to (using opcode and type).
    VPOperatorIRFlags(unsigned Opcode, Type *InstTy) {
      switch (getOperatorKind(Opcode, InstTy)) {
      case FlagsKind::VPFastMathFlags:
        FMF = FastMathFlags();
        break;
      case FlagsKind::VPOverflowingFlags:
        OverflowFlags = 0;
        break;
      case FlagsKind::VPExactFlags:
        ExactFlag = 0;
        break;
      case FlagsKind::UnknownOperatorFlags:
        NonOperator = 1;
        break;
      default:
        llvm_unreachable("Not valid FlagsKind.");
      }
    }

    // Utility to find type of operator flags based on given opcode. This switch
    // helps us track the mutual exclusivity of operator flags i.e. one type of
    // instruction cannot have multiple types of operator flags.
    FlagsKind getOperatorKind(unsigned Opcode, Type *InstTy = nullptr) const {
      switch (Opcode) {
      case Instruction::FNeg:
      case Instruction::FAdd:
      case Instruction::FSub:
      case Instruction::FMul:
      case Instruction::FDiv:
      case Instruction::FRem:
      case Instruction::FCmp:
        return FlagsKind::VPFastMathFlags;
      case Instruction::PHI:
      case Instruction::Select:
      case Instruction::Call:
      case VPInstruction::TransformLibraryCall:
      case VPInstruction::HIRCopy:
      case VPInstruction::ReductionFinal:
      case VPInstruction::ReductionFinalInscan:
      case VPInstruction::TreeConflict:
      case VPInstruction::RunningInclusiveReduction:
      case VPInstruction::RunningExclusiveReduction: {
        // Conservatively return UnknownOperatorFlags if instruction type info
        // is not provided for opcode.
        if (!InstTy)
          return FlagsKind::UnknownOperatorFlags;

        while (ArrayType *ArrTy = dyn_cast<ArrayType>(InstTy))
          InstTy = ArrTy->getElementType();

        if (InstTy->isFPOrFPVectorTy())
          return FlagsKind::VPFastMathFlags;

        return FlagsKind::UnknownOperatorFlags;
      }
      case Instruction::Add:
      case Instruction::Sub:
      case Instruction::Mul:
      case Instruction::Shl:
        return FlagsKind::VPOverflowingFlags;
      case Instruction::SDiv:
      case Instruction::UDiv:
      case Instruction::AShr:
      case Instruction::LShr:
        return FlagsKind::VPExactFlags;
      default:
        return FlagsKind::UnknownOperatorFlags;
      }
    }

    // NOTE: Both getValueAsXYZ and setValueAsXYZ assert if they are called on
    // unexpected type of instructions (determined by opcode).

    // Getter and setter for VPFastMathFlags type.
    bool hasValueAsFastMathFlags(unsigned Opcode, Type *InstTy) const {
      return getOperatorKind(Opcode, InstTy) == FlagsKind::VPFastMathFlags;
    }
    FastMathFlags getValueAsFastMathFlags(unsigned Opcode, Type *InstTy) const {
      assert(hasValueAsFastMathFlags(Opcode, InstTy) &&
             "VPInstruction cannot have FastMathFlags.");
      return FMF;
    }
    void setValueAsFastMathFlags(unsigned Opcode, Type *InstTy,
                                 FastMathFlags InFMF) {
      assert(hasValueAsFastMathFlags(Opcode, InstTy) &&
             "VPInstruction cannot have FastMathFlags.");
      FMF = InFMF;
    }

    // Getter and setter for VPOverflowingFlags type.
    bool hasValueAsOverflowingFlags(unsigned Opcode) const {
      return getOperatorKind(Opcode) == FlagsKind::VPOverflowingFlags;
    }
    bool getValueAsOverflowingFlags(unsigned Opcode,
                                    OverflowFlagsKind FlagID) const {
      assert(hasValueAsOverflowingFlags(Opcode) &&
             "VPInstruction cannot have overflowing flags (NSW/NUW).");
      return OverflowFlags[static_cast<unsigned>(FlagID)];
    }
    void setValueAsOverflowingFlags(unsigned Opcode, OverflowFlagsKind FlagID,
                                    bool FlagVal) {
      assert(hasValueAsOverflowingFlags(Opcode) &&
             "VPInstruction cannot have overflowing flags (NSW/NUW).");
      OverflowFlags.set(static_cast<unsigned>(FlagID), FlagVal);
    }

    // Getter and setter for VPExactFlag type.
    bool hasValueAsExactFlag(unsigned Opcode) const {
      return getOperatorKind(Opcode) == FlagsKind::VPExactFlags;
    }
    unsigned getValueAsExactFlag(unsigned Opcode) const {
      assert(hasValueAsExactFlag(Opcode) &&
             "VPInstruction cannot have exact flag.");
      return ExactFlag;
    }
    void setValueAsExactFlag(unsigned Opcode, bool InExactFlag) {
      assert(hasValueAsExactFlag(Opcode) &&
             "VPInstruction cannot have exact flag.");
      ExactFlag = InExactFlag;
    }
  };

public:
  /// VPlan opcodes, extending LLVM IR with idiomatics instructions.
  enum {
    Not = Instruction::OtherOpsEnd + 1,
    Abs,
    AllZeroCheck,
    Pred,
    SMax,
    UMax,
    FMax,
    SMin,
    UMin,
    UMinSeq, // Sequentional UMin that differs from vanilla UMin in semantics
             // vs poison value:
             //   UMin(0, poison) = poison
             //   UMinSeq(0, poison) = 0
    FMin,
    InductionInit,
    InductionInitStep,
    InductionFinal,
    ReductionInit,
    ReductionFinal,
    ReductionFinalUdr, // Custom finalization of UDR. Lowered as sequence of
                       // calls to combiner function.
    ReductionFinalInscan, // Reduction finalization (noop for scan).
    AllocatePrivate,
    Subscript,
    Blend,
    HIRCopy, // INTEL
    OrigTripCountCalculation,
    VectorTripCountCalculation,
    ActiveLane,
    ActiveLaneExtract,
    ConstStepVector,
    ScalarPeel,
    ScalarPeelHIR,
    ScalarRemainder,
    ScalarRemainderHIR,
    PeelOrigLiveOut,
    RemOrigLiveOut,
    PeelOrigLiveOutHIR,
    RemOrigLiveOutHIR,
    PushVF,
    PopVF,
    PlanAdapter,
    PlanPeelAdapter,
    VLSLoad,
    VLSStore,
    VLSExtract,
    VLSInsert,
    InvSCEVWrapper,
    PrivateFinalCond,
    PrivateFinalCondMem,
    PrivateFinalUncond,    // No special class implemented.
    PrivateFinalUncondMem, // Temporarily needed to avoid memonly private
                           // finalization during CG.
                           // TODO: Remove when non-explicit remainder loop
                           // support is deprecated.
    PrivateFinalMasked,    // No special class implemented. Represents the last
                        // value of unconditional private in masked mode loop.
                        // Three operands: liveout, execution mask, and
                        // fall-through value that comes out when the execution
                        // mask is 0 (loop is not executed). The last value is
                        // calculated as extract of the item correspnding MSB
                        // set in the mask.
    PrivateFinalMaskedMem, // No special class implemented. Represents the last
                           // value of in-memory unconditional private in
                           // masked mode loop. Two operands: liveout and
                           // execution mask. The last value is calculated as
                           // extract of the item correspnding MSB set in the
                           // mask.
    PrivateFinalArray,
    PrivateFinalArrayMasked,
    PrivateLastValueNonPOD,
    CompressStore, // generate llvm.masked.compressstore intrinsic, for
                   // unit stride stores

    CompressStoreNonu, // generate vcompress intrinsic and masked scatter
                       // mask for scatter: (-1 >> popcnt(exec_mask))

    ExpandLoad, // generate llvm.masked.expandload intrinsic, for unit stride
                // loads

    ExpandLoadNonu, // generate masked gather, and vexpand intrinsic mask for
                    // gather: (-1 >> popcnt(exec_mask))

    CompressExpandIndexInit,  // placeholder for initial value of index
    CompressExpandIndexFinal, // placeholder for the final value of index

    CompressExpandIndex,     // calculate vector of indexes for non-unit stride
                             // compress/expand

    CompressExpandIndexInc, // compress/expand index increment
                            // operands: index, stride, mask
                            // generate: index += stride * pocnt(mask);
    PrivateLastValueNonPODMasked,
    GeneralMemOptConflict,
    ConflictInsn,
    TreeConflict,
    CvtMaskToInt,
    Permute,
    RunningInclusiveReduction, // Represents running inclusive reduction for
                               // inscan reduction vectorization:
                               // E.g., operation for type T:
                               // T4 running-inclusive-reduction(T4 vx, T x) {
                               //   return [vx3 + vx2 + vx1 + vx0 + x,
                               //           vx2 + vx1 + vx0 + x,
                               //           vx1 + vx0 + x,
                               //           vx0 + x];
                               // }
    RunningExclusiveReduction, // Represents running exclusive reduction for
                               // inscan reduction vectorization:
                               // e.g, operation for type T:
                               // T4 running-exclusive-reduction(T4 vx, T x) {
                               //   return [vx2 + vx1 + vx0 + x,
                               //           vx1 + vx0 +x,
                               //           vx0 + x,
                               //           x];
                               // }
    ExtractLastVectorLane,     // Extract a scalar from the lane VF-1.
    TransformLibraryCall,      // Transformed library call whose scalar
                               // signature does not match its vectorized
                               // signature. (e.g. sincos)
    SOAExtractValue,           // Like LLVM extract value, but specialized for
                               // when we know the aggregate is in SOA layout
  };

private:
  typedef unsigned char OpcodeTy;
  OpcodeTy Opcode;

  /// Each VPInstruction belongs to a single VPBasicBlock.
  VPBasicBlock *Parent = nullptr;

  // Debug location for this VPInstruction.
  DebugLoc DbgLoc;
  // Hold operator-related metadata attributes attached to this VPInstruction.
  VPOperatorIRFlags OperatorFlags;

  // Hold the underlying HIR information, if any, attached to this
  // VPInstruction.
  HIRSpecificsData HIRData;

private:
  /// Utility method serving execute(): generates a single instance of the
  /// modeled instruction.
  void generateInstruction(VPTransformState &State, unsigned Part);

  void copyUnderlyingFrom(const VPInstruction &Inst) {
#if INTEL_CUSTOMIZATION
    HIR().cloneFrom(Inst.HIR());
#endif // INTEL_CUSTOMIZATION
    Value *V = Inst.getUnderlyingValue();
    if (V)
      setUnderlyingValue(*V);
    if (!Inst.isUnderlyingIRValid())
      invalidateUnderlyingIR();
  }

  void copyAttributesFrom(const VPInstruction &Inst) {
    DbgLoc = Inst.DbgLoc;
    // Copy other general attributes here when imported.
    OperatorFlags = Inst.OperatorFlags;
  }

protected:
  /// Return the underlying Instruction attached to this VPInstruction. Return
  /// null if there is no Instruction attached. This interface is similar to
  /// getValue() but it hides the cast when we are working with VPInstruction
  /// pointers.
  Instruction *getInstruction() const {
    assert((!getUnderlyingValue() || isa<Instruction>(getUnderlyingValue())) &&
           "Expected Instruction as underlying Value.");
    return cast_or_null<Instruction>(getUnderlyingValue());
  }

#if INTEL_CUSTOMIZATION
  /// Return true if this is a new VPInstruction (i.e., an VPInstruction that is
  /// not coming from the underlying IR.
  bool isNew() const {
    return getUnderlyingValue() == nullptr && !HIR().isSet();
  }
#endif

  virtual VPInstruction *cloneImpl() const {
    VPInstruction *Cloned = new VPInstruction(Opcode, getType(), {});
    for (auto &O : operands()) {
      Cloned->addOperand(O);
    }
    return Cloned;
  }

public:
  VPInstruction(unsigned Opcode, Type *BaseTy, ArrayRef<VPValue *> Operands)
      : VPUser(VPValue::VPInstructionSC, Operands, BaseTy), Opcode(Opcode),
        OperatorFlags(Opcode, BaseTy), HIRData(*this) {
    assert(BaseTy && "BaseTy can't be null!");
  }
  VPInstruction(unsigned Opcode, Type *BaseTy,
                std::initializer_list<VPValue *> Operands)
      : VPUser(VPValue::VPInstructionSC, Operands, BaseTy), Opcode(Opcode),
        OperatorFlags(Opcode, BaseTy), HIRData(*this) {
    assert(BaseTy && "BaseTy can't be null!");
  }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return V->getVPValueID() == VPValue::VPInstructionSC;
  }

  unsigned getOpcode() const { return Opcode; }
  // FIXME: Temporary workaround for TTI problems that make the cost
  // modeling incorrect. The getCMType() returns nullptr in case the underlying
  // instruction is not set and this makes the cost of this instruction
  // undefined (i.e. 0). Non-null return value causes calculation by TTI with
  // incorrect result.
  virtual Type *getCMType() const override;

  // Return true if this VPInstruction represents a cast operation.
  bool isCast() const { return Instruction::isCast(getOpcode()); }

  bool mayHaveSideEffects() const;

  bool isSimpleLoadStore() const {
    if (getOpcode() != Instruction::Load && getOpcode() != Instruction::Store)
      return false;

    // TODO: First-class representation for volatile/atomic property inside
    // VPInstruction.
    if (auto *Underlying = getUnderlyingValue()) {
      if (isa<LoadInst>(Underlying))
        return cast<LoadInst>(Underlying)->isSimple();
      else
        return cast<StoreInst>(Underlying)->isSimple();
    }

    // Load/store without underlying IR or coming from HIR are known to be
    // simple.
    return true;
  }

  const DebugLoc getDebugLocation() const { return DbgLoc; }
  void setDebugLocation(const DebugLoc &Loc) { DbgLoc = Loc; }

  // Utility to populate current VPInstruction's operator flags from
  // llvm::Instruction it was created for.
  void copyOperatorFlagsFrom(const Instruction *Inst) {
    if (auto *OB = dyn_cast<OverflowingBinaryOperator>(Inst)) {
      setHasNoSignedWrap(OB->hasNoSignedWrap());
      setHasNoUnsignedWrap(OB->hasNoUnsignedWrap());
    }
    if (auto *PE = dyn_cast<PossiblyExactOperator>(Inst))
      setIsExact(PE->isExact());
    if (auto *FP = dyn_cast<FPMathOperator>(Inst))
      setFastMathFlags(FP->getFastMathFlags());
    // No other operator flags of interest now for VPlan.
  }

  // Utility to transfer current VPInstruction's operator flags to its
  // corresponding outgoing llvm::Instruction.
  void copyOperatorFlagsTo(Instruction *Inst) {
    if (isa<OverflowingBinaryOperator>(Inst)) {
      Inst->setHasNoSignedWrap(hasNoSignedWrap());
      Inst->setHasNoUnsignedWrap(hasNoUnsignedWrap());
    }
    if (isa<PossiblyExactOperator>(Inst))
      Inst->setIsExact(isExact());
    if (isa<FPMathOperator>(Inst))
      Inst->setFastMathFlags(getFastMathFlags());
  }

  // Getters for operator-specific attributes.
  bool hasFastMathFlags() const {
    return OperatorFlags.hasValueAsFastMathFlags(getOpcode(), getType()) &&
           getFastMathFlags().any();
  }
  FastMathFlags getFastMathFlags() const {
    return OperatorFlags.getValueAsFastMathFlags(getOpcode(), getType());
  }
  bool hasNoSignedWrap() const {
    if (!OperatorFlags.hasValueAsOverflowingFlags(getOpcode()))
      return false;

    return OperatorFlags.getValueAsOverflowingFlags(
        getOpcode(), VPOperatorIRFlags::OverflowFlagsKind::NSWFlag);
  }
  bool hasNoUnsignedWrap() const {
    if (!OperatorFlags.hasValueAsOverflowingFlags(getOpcode()))
      return false;

    return OperatorFlags.getValueAsOverflowingFlags(
        getOpcode(), VPOperatorIRFlags::OverflowFlagsKind::NUWFlag);
  }
  bool isExact() const {
    if (!OperatorFlags.hasValueAsExactFlag(getOpcode()))
      return false;

    return OperatorFlags.getValueAsExactFlag(getOpcode());
  }

  // Setters for operator-specific attributes.
  void setFastMathFlags(FastMathFlags FMF) {
    OperatorFlags.setValueAsFastMathFlags(getOpcode(), getType(), FMF);
  }
  void setHasNoSignedWrap(bool IsNSW) {
    OperatorFlags.setValueAsOverflowingFlags(
        getOpcode(), VPOperatorIRFlags::OverflowFlagsKind::NSWFlag, IsNSW);
  }
  void setHasNoUnsignedWrap(bool IsNUW) {
    OperatorFlags.setValueAsOverflowingFlags(
        getOpcode(), VPOperatorIRFlags::OverflowFlagsKind::NUWFlag, IsNUW);
  }
  void setIsExact(bool IsExact) {
    OperatorFlags.setValueAsExactFlag(getOpcode(), IsExact);
  }

  // ilist should have access to VPInstruction node.
  friend struct ilist_traits<VPInstruction>;
  friend struct ilist_traits<VPBasicBlock>;

  /// \return the VPBasicBlock which this VPInstruction belongs to.
  VPBasicBlock *getParent() { return Parent; }
  const VPBasicBlock *getParent() const { return Parent; }
  void setParent(VPBasicBlock *NewParent) { Parent = NewParent; }

  /// Unlink this instruction from its current basic block and insert it into
  /// the basic block that MovePos lives in, right before MovePos.
  void moveBefore(VPInstruction *MovePos);

  /// Unlink this instruction from its current basic block and insert it into
  /// the basic block that MovePos lives in, right after MovePos.
  void moveAfter(VPInstruction *MovePos);

  /// Unlink this instruction from its current basic block and insert it into
  /// the basic block BB, right before \p I.
  void moveBefore(VPBasicBlock &BB, VPBasicBlock::iterator I);

  /// Generate the instruction.
  /// TODO: We currently execute only per-part unless a specific instance is
  /// provided.
  virtual void execute(VPTransformState &State);
#if INTEL_CUSTOMIZATION
  virtual void executeHIR(VPOCodeGenHIR *CG);
#endif
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &O) const override;
  void printWithoutAnalyses(raw_ostream &O) const;
  static const char *getOpcodeName(unsigned Opcode);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  VPInstruction *clone() const {
    VPInstruction *Cloned = cloneImpl();
    Cloned->copyUnderlyingFrom(*this);
    Cloned->copyAttributesFrom(*this);
    return Cloned;
  }

  bool isUnderlyingIRValid() const {
    return IsUnderlyingValueValid || HIR().isValid();
  }

  void invalidateUnderlyingIR() {
    IsUnderlyingValueValid = false;
    // Temporary hook-up to ignore loop induction related instructions during CG
    // by not invalidating them.
    // TODO: Remove this code after VPInduction support is added to HIR CG.
    const loopopt::HLNode *HNode = HIR().getUnderlyingNode();
    if (HNode && isa<loopopt::HLLoop>(HNode))
      return;
    HIR().invalidate();
    // At this point, we don't have a use-case where invalidation of users of
    // instruction is needed. This is because VPInstructions mostly represent
    // r-value of a HLInst and modifying the r-value should not affect l-value
    // temp refs. For stores this was never an issue since store instructions
    // cannot have users. In case of LLVM-IR invalidating an instruction might
    // mean that the underlying bit value is different. If this is the case then
    // invalidation should be propagated to users as well.
  }

  HIRSpecifics HIR() { return HIRSpecifics(*this); }
  const HIRSpecifics HIR() const { return HIRSpecifics(*this); }

  unsigned getNumSuccessors() const;
};

/// Instruction to set vector factor and unroll factor explicitly.
/// Can uppear in VPlan after cost model. Appears at the beginning of VPlan
/// and is accompanied by VPPopVF in the end of VPlan. The pairing is possible
/// since VPlan is SESE region. This provides possibility to have "inner" VPlans
/// with their own VF inside an outer one.
/// No code generation is expected, CG just updates its internal VF and UF on
/// the fly. The VF-dependent analyses should account this instruction too.
class VPPushVF final : public VPInstruction {
public:
  VPPushVF(LLVMContext *Ctx, unsigned V, unsigned U)
      : VPInstruction(VPInstruction::PushVF, Type::getVoidTy(*Ctx), {}), VF(V),
        UF(U) {}

  unsigned getVF() const { return VF; }
  unsigned getUF() const { return UF; }

  /// Methods for supporting type inquiry through isa, cast, and
  /// dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::PushVF;
  }
  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    void printImpl(raw_ostream &OS) const {
      OS << " VF=" << VF << " UF=" << UF;
    }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
protected:
  VPInstruction *cloneImpl() const override {
    llvm_unreachable("not expected to clone");
  }
  unsigned VF;
  unsigned UF;
};

/// Concrete class for comparison. Represents both integers and floats.
class VPCmpInst : public VPInstruction {
public:
  typedef CmpInst::Predicate Predicate;
  /// Create VPCmpInst with its two operands, a pred and a BaseType.
  /// Operands \p LHS and \p RHS must not have conflicting base types.
  VPCmpInst(VPValue *LHS, VPValue *RHS, Predicate Pred)
      : VPInstruction(inferOpcodeFromPredicate(Pred),
                      CmpInst::makeCmpResultType(LHS->getType()),
                      ArrayRef<VPValue *>({LHS, RHS})),
        Pred(Pred) {}

  /// Return the predicate for this instruction
  Predicate getPredicate() const { return Pred; }

  /// Return the predicate as if the operands were swapped.
  Predicate getSwappedPredicate() const {
    return CmpInst::getSwappedPredicate(Pred);
  }

  /// Methods for supporting type inquiry through isa, cast, and
  /// dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == Instruction::ICmp ||
           VPI->getOpcode() == Instruction::FCmp;
  }
  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

protected:
  virtual VPCmpInst *cloneImpl() const final {
    return new VPCmpInst(getOperand(0), getOperand(1), getPredicate());
  }

private:
  Predicate Pred;

  // Return the opcode that corresponds to predicate
  unsigned inferOpcodeFromPredicate(Predicate Pred) {
    // Infer Opcode from Pred
    if (CmpInst::isIntPredicate(Pred))
      return Instruction::ICmp;
    if (CmpInst::isFPPredicate(Pred))
      return Instruction::FCmp;
    llvm_unreachable("Integer/Float predicate expected");
  }
};

/// Concrete class to represent last value calculation for masked and non-masked
/// private nonPODs in VPlan.
template <unsigned InstOpcode>
class VPPrivateLastValueNonPODTemplInst : public VPInstruction {
public:
  /// Create VPPrivateLastValueNonPODTemplInst with its BaseType, operands and
  /// private object copyassign function pointer.
  VPPrivateLastValueNonPODTemplInst(Type *BaseTy, ArrayRef<VPValue *> Operands,
                                    Function *CopyAssign)
      : VPInstruction(InstOpcode, BaseTy, Operands), CopyAssign(CopyAssign) {}

  /// Return the copyassign function stored by this instruction
  Function *getCopyAssign() const { return CopyAssign; }

  /// Methods for supporting type inquiry through isa, cast, and
  /// dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == InstOpcode;
  }
  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

protected:
  virtual VPInstruction *cloneImpl() const final {
    SmallVector<VPValue *, 3> Ops(operands());
    return new VPPrivateLastValueNonPODTemplInst<InstOpcode>(getType(), Ops,
                                                             getCopyAssign());
  }

private:
  Function *CopyAssign = nullptr;
};

using VPPrivateLastValueNonPODMaskedInst = VPPrivateLastValueNonPODTemplInst<
    VPInstruction::PrivateLastValueNonPODMasked>;
using VPPrivateLastValueNonPODInst =
    VPPrivateLastValueNonPODTemplInst<VPInstruction::PrivateLastValueNonPOD>;

/// Concrete class to represent branch instruction in VPlan.
class VPBranchInst : public VPInstruction {
public:
  /// Construct empty branch instruction with no successors
  /// (e.g. for the latest block in CFG).
  explicit VPBranchInst(Type *BaseTy)
      : VPInstruction(Instruction::Br, BaseTy, {}) {}

  /// Construct branch instruction with one successor.
  explicit VPBranchInst(VPBasicBlock *Succ)
      : VPInstruction(Instruction::Br,
                      Type::getVoidTy(Succ->getType()->getContext()), {Succ}) {
  }

  /// Construct branch instruction with two successors and a condition bit.
  explicit VPBranchInst(VPBasicBlock *IfTrue, VPBasicBlock *IfFalse,
                        VPValue *Cond)
      : VPInstruction(Instruction::Br,
                      Type::getVoidTy(IfTrue->getType()->getContext()),
                      {IfTrue, IfFalse, Cond}) {}

  /// Returns underlying HLGoto node if any or nullptr otherwise.
  const loopopt::HLGoto *getHLGoto() const {
    loopopt::HLNode *Node = HIR().getUnderlyingNode();
    if (!Node)
      return nullptr;
    return cast<loopopt::HLGoto>(Node);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printImpl(raw_ostream &O) const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  static inline bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == Instruction::Br;
  }
  static bool classof(const VPValue *U) {
    return isa<VPInstruction>(U) && classof(cast<VPInstruction>(U));
  }

  /// Iterators and types to access Successors of terminator's
  /// parent VPBasicBlock.
  inline operand_iterator succ_begin() { return op_begin(); }
  inline operand_iterator succ_end() {
    // CondBit is expected to be last operand of VPBranchInst (if available)
    return isConditional() ? std::prev(op_end()) : op_end();
  }
  inline const_operand_iterator succ_begin() const { return op_begin(); }
  inline const_operand_iterator succ_end() const {
    return const_cast<VPBranchInst *>(this)->succ_end();
  }

  /// Successors range.
  const_operand_range successors() const {
    return make_range(succ_begin(), succ_end());
  }

  /// Returns the certain successor.
  VPBasicBlock *getSuccessor(unsigned idx) const {
    assert(idx < getNumSuccessors() && "Successor # out of range for Branch!");
    return cast<VPBasicBlock>(getOperand(idx));
  }

  /// Replaces the certain successor
  /// (the successor with specified index should already exist).
  void setSuccessor(unsigned idx, VPBasicBlock *NewSucc) {
    assert(idx < getNumSuccessors() && "Successor # out of range for Branch!");
    assert(NewSucc && "Successor can't be null");
    setOperand(idx, NewSucc);
  }

  /// Returns successors total count.
  size_t getNumSuccessors() const {
    return std::distance(succ_begin(), succ_end());
  }

  /// Returns true in case if the branch is conditional.
  bool isConditional() const { return getNumOperands() == 3; }

  /// Returns the condition bit selecting the successor.
  VPValue *getCondition() const {
    assert(isConditional() &&
           "Can't return condition for the unconditional branch");
    return getOperand(getNumOperands() - 1);
  }

  /// Set the condition bit.
  void setCondition(VPValue *Cond) {
    assert(isConditional() &&
           "Setting CondBit for unconditional instruction is prohibited");
    assert(Cond && "Condition can't be nullptr");
    setOperand(getNumOperands() - 1, Cond);
  }

  /// Returns LoopID metadata node attached to this terminator instruction. This
  /// is expected to be present only for loop latch terminator.
  MDNode *getLoopIDMetadata() const { return LoopID; }

  /// Set the LoopID metadata node for this terminator instruction.
  void setLoopIDMetadata(MDNode *LpID) { LoopID = LpID; }

protected:
  virtual VPBranchInst *cloneImpl() const final {
    VPBranchInst *Cloned = new VPBranchInst(getType());
    for (unsigned i = 0; i < getNumOperands(); i++)
      Cloned->addOperand(getOperand(i));
    Cloned->setLoopIDMetadata(getLoopIDMetadata());
    return Cloned;
  }

  // Capture the incoming loop's LoopID metadata. It can also be used internally
  // by VPlan framework to attach some metadata related to the loop like TC
  // estimates.
  MDNode *LoopID = nullptr;
};

inline auto successors(const VPBasicBlock *BB) {
  return map_range(
      BB->getTerminator()->successors(),
      [](const VPValue *SuccBB) { return cast<VPBasicBlock>(SuccBB); });
}

/// Concrete class for blending instruction. Was previously represented as
/// VPPHINode with Blend = true.
class VPBlendInst : public VPInstruction {
public:
  explicit VPBlendInst(Type *BaseTy)
      : VPInstruction(VPInstruction::Blend, BaseTy, ArrayRef<VPValue *>()) {}

  /// Return number of incoming values. For blend, this is the number of
  /// operands / 2 because both the incoming value and block-predicate are
  /// added as operands.
  unsigned getNumIncomingValues(void) const { return getNumOperands() / 2; }

  /// Return incoming value at stride 2 Idx.
  VPValue *getIncomingValue(const unsigned Idx) const {
    assert(Idx < getNumIncomingValues() &&
           "Idx outside range of incoming values");
    return getOperand(Idx*2);
  }

  /// Return incoming block-predicate at stride 2 Idx.
  VPValue *getIncomingPredicate(const unsigned Idx) const {
    assert(Idx < getNumIncomingValues() &&
           "Idx outside range of incoming values");
    return getOperand(Idx*2+1);
  }

  /// Add operands of incoming value and block-predicate to the end of the
  /// blend operand list. If \p Plan is provided and \p BlockPred is nullptr,
  /// the "true" will be used as the predicate.
  void addIncoming(VPValue *IncomingVal, VPValue *BlockPred, VPlan *Plan = nullptr);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printImpl(raw_ostream &O) const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::Blend;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

protected:
  /// Create new Blend and copy original incoming values to the newly created
  /// Blend. Caller is responsible to replace these values with what is
  /// needed.
  virtual VPBlendInst *cloneImpl() const override {
    VPBlendInst *Cloned = new VPBlendInst(getType());
    for (unsigned I = 0, E = getNumIncomingValues(); I != E; ++I)
      Cloned->addIncoming(getIncomingValue(I), getIncomingPredicate(I));
    return Cloned;
  }
};

/// Concrete class for PHI instruction.
class VPPHINode : public VPInstruction {
  friend class VPlanCFGMerger;

  SmallVector<VPBasicBlock *, 2> VPBBUsers;
  unsigned MergeId = VPExternalUse::UndefMergeId;

  VPPHINode(unsigned Id, Type *BaseTy)
      : VPInstruction(Instruction::PHI, BaseTy, ArrayRef<VPValue *>()),
        MergeId(Id) {}

public:
  using vpblock_iterator = SmallVectorImpl<VPBasicBlock *>::iterator;
  using const_vpblock_iterator =
      SmallVectorImpl<VPBasicBlock *>::const_iterator;

  explicit VPPHINode(Type *BaseTy)
      : VPInstruction(Instruction::PHI, BaseTy, ArrayRef<VPValue *>()) {}

  vpblock_iterator block_begin(void) { return VPBBUsers.begin(); }

  vpblock_iterator block_end(void) { return VPBBUsers.end(); }

  const_vpblock_iterator block_begin(void) const {
    return VPBBUsers.begin();
  }

  const_vpblock_iterator block_end(void) const {
    return VPBBUsers.end();
  }

  iterator_range<vpblock_iterator> blocks(void) {
    return make_range(block_begin(), block_end());
  }

  iterator_range<const_vpblock_iterator> blocks(void) const {
    return make_range(block_begin(), block_end());
  }

  operand_range incoming_values(void) { return operands(); }
  const_operand_range incoming_values(void) const { return operands(); }

  /// Return number of incoming values.
  unsigned getNumIncomingValues(void) const { return getNumOperands(); }

  /// Return VPValue that corresponds to Idx'th incomming VPBasicBlock.
  VPValue *getIncomingValue(const unsigned Idx) const { return getOperand(Idx); }

  /// Return VPValue that corresponds to incomming VPBB.
  VPValue *getIncomingValue(const VPBasicBlock *VPBB) const {
    auto Idx = getBlockIndexOrNone(VPBB);
    assert(Idx && "Cannot find value for a given BB.");
    return getIncomingValue(Idx.value());
  }

  void setIncomingValue(const unsigned Idx, VPValue *Value) {
    assert(Value && "Value must not be null.");
    setOperand(Idx, Value);
  }

  /// Set the incoming value for a specific basic block.
  void setIncomingValue(VPBasicBlock *VPBB, VPValue *Value) {
    assert(Value && VPBB && "Value and VPBB must not be null.");
    auto Idx = getBlockIndex(VPBB);
    assert(Idx >= 0 && "VPBB should have a valid index.");
    setIncomingValue(Idx, Value);
  }

  /// Add an incoming value to the end of the PHI list
  void addIncoming(VPValue *Value, VPBasicBlock *BB) {
    assert(Value && "Value must not be null.");
    assert(BB && "Block must not be null.");
    addOperand(Value);
    VPBBUsers.push_back(BB);
  }

  /// Return incoming basic block number \p Idx.
  VPBasicBlock *getIncomingBlock(const unsigned Idx) const {
    return VPBBUsers[Idx];
  }

  /// Return incoming basic block corresponding to \p Value.
  VPBasicBlock *getIncomingBlock(const VPValue *Value) const {
    auto It = llvm::find(make_range(op_begin(), op_end()), Value);
    return getIncomingBlock(std::distance(op_begin(), It));
  }

  /// Set incoming basic block as \p Block corresponding to basic block number
  /// \p Idx.
  void setIncomingBlock(const unsigned Idx, VPBasicBlock *BB) {
    assert(BB && "VPPHI node got a null basic block");
    VPBBUsers[Idx] = BB;
  }

  /// Remove a block from the phi node.
  void removeIncomingValue(const VPBasicBlock *BB) {
    unsigned Idx = getBlockIndex(BB);
    assert(Idx <= VPBBUsers.size() && "Index is out of range");
    auto it = VPBBUsers.begin();
    std::advance(it, Idx);
    VPBBUsers.erase(it);
    removeOperand(Idx);
  }

  void clear() {
    int NumOps = getNumIncomingValues();
    for (int Idx = 0; Idx < NumOps; ++Idx)
      removeOperand(NumOps - 1 - Idx);
    VPBBUsers.clear();
  }

  /// Return index for a given \p Block.
  int getBlockIndex(const VPBasicBlock *BB) const {
    auto It = llvm::find(make_range(block_begin(), block_end()), BB);
    if (It != block_end())
      return std::distance(block_begin(), It);
    return -1;
  }

  /// Return Optional index for a given basic block \p Block.
  Optional<unsigned> getBlockIndexOrNone(const VPBasicBlock *BB) const {
    int Idx = getBlockIndex(BB);
    return Idx != -1 ? Optional<unsigned>(Idx) : None;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == Instruction::PHI;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  /// Checks whether the specified PHI node always merges together the same
  /// value, assuming that undefs result in the same value as non-undefs.
  /// Adapted from the LLVM-source PHINode::hasConstantOrUndefValue().
  bool hasConstantOrUndefValue() const {
    VPValue *ConstantValue = nullptr;
    for (int I = 0, E = getNumIncomingValues(); I != E; ++I) {
      VPValue *Incoming = getIncomingValue(I);
      const VPConstant *ConstOp = dyn_cast<VPConstant>(Incoming);
      bool IsUndef = ConstOp && isa<UndefValue>(ConstOp->getConstant());
      if (Incoming != this && !IsUndef) {
        if (ConstantValue && ConstantValue != Incoming)
          return false;
        ConstantValue = Incoming;
      }
    }
    return true;
  }

  unsigned getMergeId() const { return MergeId; }

protected:
  /// Create new PHINode and copy original incoming values to the newly created
  /// PHINode. Caller is responsible to replace these values with what is
  /// needed.
  virtual VPPHINode *cloneImpl() const final {
    VPPHINode *Cloned = new VPPHINode(getType());
    for (unsigned i = 0, e = getNumIncomingValues(); i != e; ++i) {
      Cloned->addIncoming(getIncomingValue(i), getIncomingBlock(i));
    }
    return Cloned;
  }
};

/// Concrete class to represent GEP instruction in VPlan.
class VPGEPInstruction : public VPInstruction {
  friend class VPlanVerifier;

private:
  bool InBounds;
  /// SourceElementType/ResultElementType have the same meaning as for
  /// llvm::GetElementPtrInst and are needed to make opaque pointers work as
  /// that type is used for calculating the offsets.
  Type *SourceElementType;
  // It could be recomputed on demand using the SourceElementType and indices,
  // but it's easier to just store it here (and that follows the
  // llvm::GetElementPtrInst's approach).
  Type *ResultElementType;

public:
  /// Default constructor for VPGEPInstruction. The default value for \p
  /// InBounds is false.
  //
  // Technically, we could have computed ResultElementType from
  // SourceElementType/indices list. However, there are existing inconsistencies
  // (e.g. SOA geps), so delay the auto-deduction it untill those issues are
  // fixed.
  VPGEPInstruction(Type *SourceElementType, Type *ResultElementType,
                   Type *BaseTy, VPValue *Ptr, ArrayRef<VPValue *> IdxList,
                   bool InBounds = false)
      : VPInstruction(Instruction::GetElementPtr, BaseTy, {}),
        InBounds(InBounds), SourceElementType(SourceElementType),
        ResultElementType(ResultElementType) {
    assert(Ptr && "Base pointer operand of GEP cannot be null.");
    // Base pointer should be the first operand of GEP followed by index
    // operands
    assert(!getNumOperands() &&
           "GEP instruction already has operands before base pointer.");
    addOperand(Ptr);
    for (auto Idx : IdxList)
      addOperand(Idx);
    assert(cast<PointerType>(getOperand(0)->getType()->getScalarType())
               ->isOpaqueOrPointeeTypeMatches(SourceElementType) &&
           "SourceElemenType doesn't match non-opaque pointer base!");
    assert(cast<PointerType>(getType()->getScalarType())
               ->isOpaqueOrPointeeTypeMatches(ResultElementType) &&
           "ResultElementType doesn't match non-opaque result ponter!");
  }

  /// Setter and getter functions for InBounds.
  void setIsInBounds(bool IsInBounds) { InBounds = IsInBounds; }
  bool isInBounds() const { return InBounds; }

  /// Get the base pointer operand of given VPGEPInstruction.
  VPValue *getPointerOperand() const { return getOperand(0); }

  Type *getSourceElementType() const { return SourceElementType; }
  Type *getResultElementType() const { return ResultElementType; }

  /// Check if pointer operand is opaque.
  ///
  /// Temporary helper method to reduce boilerplate code. We should delete it
  /// after transition to opaque ptrs is finished.
  bool isOpaque() const {
    return cast<PointerType>(getPointerOperand()->getType()->getScalarType())
        ->isOpaque();
  }

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == Instruction::GetElementPtr;
  }

  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  // Iterators to access indices.
  inline decltype(auto) idx_begin() { return op_begin() + 1; }
  inline decltype(auto) idx_begin() const { return op_begin() + 1; }
  inline decltype(auto) idx_end() { return op_end(); }
  inline decltype(auto) idx_end() const { return op_end(); }

  inline decltype(auto) indices() {
    return make_range(idx_begin(), idx_end());
  }

  inline decltype(auto) indices() const {
    return make_range(idx_begin(), idx_end());
  }

protected:
  virtual VPGEPInstruction *cloneImpl() const final {
    VPGEPInstruction *Cloned =
        new VPGEPInstruction(SourceElementType, ResultElementType, getType(),
                             getOperand(0), {}, isInBounds());
    for (auto *O : make_range(op_begin()+1, op_end())) {
      Cloned->addOperand(O);
    }
    return Cloned;
  }
};

/// Concrete class to represent loopopt:RegDDRefs in the VPlan framework.
///
/// RegDDRefs dimension information is mapped to a DimInfo data structure
/// defined below and the VPSubscriptInst consists of a base pointers and a list
/// of DimInfo elements describing all the dimensions contained in the original
/// RegDDRef. It also stores explicit type information as that is needed due to
/// the opaque pointers for the StructOffsets address computation. (dimensional
/// accesses are computed using explicit strides in bytes).
class VPSubscriptInst final : public VPInstruction {
public:
  struct DimInfo {
    unsigned Rank;
    VPValue *LowerBound;
    VPValue *StrideInBytes;
    VPValue *Index;

    // See loopopt::RegDDRef implementation, we just store the information to be
    // able to generate HIR back in the format the rest of LoopOpt desires.

    // Type associated with each dimension of this array access. For example,
    // incoming HIR contains:
    // %1 = (@arr)[0:0:4096([1024 x i32]*:0)][0:i1:4([1024 x i32]:1024)]
    //
    // then the types will be the following:
    // Dim  --->     DimType
    //  1         [1024 x i32]*
    //  0         [1024 x i32]
    //
    // TODO: Not sure why loopopt really needs it or what will they do once
    // [1024 x i32]* would become simply opaque pointer type.
    Type *DimType;

    // See loopopt::RegDDRef docs as well.
    // For the case above the values would be:
    //  Dim ---->    DimElementType
    //  1         [1024 x i32]
    //  0         i32
    Type *DimElementType;

    // Struct offsets associated with each dimension of this array access. For
    // example, suppose incoming HIR contains:
    //
    //   %1 = @(%Base)[%I1].0.1[%I2].0[%I3]
    //
    // Offsets for the dimension 2 would be {0, 1}, for dimensions 1: {0} and
    // empty for dimension 0;
    ArrayRef<unsigned> StructOffsets;

    DimInfo(unsigned Rank, VPValue *LowerBound, VPValue *StrideInBytes,
            VPValue *Index, Type *DimType, Type *DimElementType,
            ArrayRef<unsigned> StructOffsets = {})
        : Rank(Rank), LowerBound(LowerBound), StrideInBytes(StrideInBytes),
          Index(Index), DimType(DimType), DimElementType(DimElementType),
          StructOffsets(StructOffsets) {}
    DimInfo(const DimInfo &) = default;
  };

private:
  // For internal storage - same as DimInfo but without VPValues stored as
  // operands.
  // Not every dimension is expected to have those, so we don't want to bloat
  // the size of VPSubscriptInst by having each dimension information to keep
  // its own SmallVector. Instead keep all indices here, and have each dimension
  // reference its subrange.
  SmallVector<unsigned, 8> StructOffsetsStorage;
  struct DimInfoWithoutOperands {
    unsigned Rank;
    unsigned short OffsetsBegin;
    unsigned short OffsetsEnd;
    Type *DimType;
    Type *DimElementType;

    DimInfoWithoutOperands(const DimInfo &Dim, unsigned short OffsetsBegin,
                           unsigned short OffsetsEnd)
        : Rank(Dim.Rank), OffsetsBegin(OffsetsBegin), OffsetsEnd(OffsetsEnd),
          DimType(Dim.DimType), DimElementType(Dim.DimElementType) {
      assert((size_t)(OffsetsEnd - OffsetsBegin) == Dim.StructOffsets.size() &&
             "Lost struct offset!");
    }
    DimInfoWithoutOperands(const DimInfoWithoutOperands &) = default;
  };

  bool InBounds = false;

  // TODO: Below SmallVector is currently assuming that dimensions are added
  // in a fixed order i.e. outer-most dimension to inner-most dimension. To
  // support flexibility in this ordering, DenseMaps will be needed. For
  // example, for a 3-dimensional array access, Ranks = < 2, 1, 0 >.
  SmallVector<DimInfoWithoutOperands, 4> Dimensions;

  // To be used in cloneImpl.
  VPSubscriptInst(const VPSubscriptInst &Other)
      : VPInstruction(VPInstruction::Subscript, Other.getType(), {}) {
    for (auto *Op : Other.operands())
      addOperand(Op);
    InBounds = Other.InBounds;
    Dimensions = Other.Dimensions;
    StructOffsetsStorage = Other.StructOffsetsStorage;
  }

public:
  // No need to auto-deduce BaseTy because it will become just opaque pointer
  // type soon enough.
  VPSubscriptInst(Type *BaseTy, VPValue *Base, ArrayRef<DimInfo> Dims)
      : VPInstruction(VPInstruction::Subscript, BaseTy, {Base}) {
    assert(
        is_sorted(Dims, [](const DimInfo &D1,
                           const DimInfo &D2) { return D1.Rank >= D2.Rank; }) &&
        "Ranks are not monotonic!");
    for (auto &Dim : Dims) {
      unsigned short OffsetsBegin = StructOffsetsStorage.size();
      unsigned short OffsetsEnd = OffsetsBegin + Dim.StructOffsets.size();
      StructOffsetsStorage.append(Dim.StructOffsets.begin(),
                                  Dim.StructOffsets.end());
      Dimensions.emplace_back(Dim, OffsetsBegin, OffsetsEnd);
      addOperand(Dim.LowerBound);
      addOperand(Dim.StrideInBytes);
      addOperand(Dim.Index);
    }
  }

  unsigned getNumDimensions() const { return Dimensions.size(); }

  /// Get \p Dim dimension data.
  const DimInfo dim(unsigned Dim) const {
    assert(Dim < getNumDimensions() && "Out of bounds Dim!");

    auto It = op_rbegin();
    std::advance(It, 3 * Dim);

    VPValue *Index = *It++;
    VPValue *StrideInBytes = *It++;
    VPValue *LowerBound = *It++;

    auto DimIt = Dimensions.rbegin();
    std::advance(DimIt, Dim);
    const auto &D = *DimIt;

    return {D.Rank,
            LowerBound,
            StrideInBytes,
            Index,
            D.DimType,
            D.DimElementType,
            ArrayRef<unsigned>(StructOffsetsStorage)
                .slice(D.OffsetsBegin, D.OffsetsEnd - D.OffsetsBegin)};
  }

  /// Setter and getter functions for InBounds.
  void setIsInBounds(bool IsInBounds) { InBounds = IsInBounds; }
  bool isInBounds() const { return InBounds; }

  /// Get the base pointer operand.
  VPValue *getPointerOperand() const { return getOperand(0); }

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::Subscript;
  }

  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

protected:
  virtual VPSubscriptInst *cloneImpl() const override {
    VPSubscriptInst *Cloned = new VPSubscriptInst(*this);
    return Cloned;
  }
};

/// Concrete class to represent load/store instruction in VPlan.
class VPLoadStoreInst : public VPInstruction {
public:
  using MDNodesTy = SmallVector<std::pair<unsigned, MDNode *>, 6>;

  VPLoadStoreInst(unsigned Opcode, Type *Ty, ArrayRef<VPValue *> Operands)
      : VPInstruction(Opcode, Ty, Operands) {
    assert((isLoadOpcode(Opcode) || isStoreOpcode(Opcode)) &&
           "Invalid opcode for load/store instruction.");
  }

  void setAlignment(Align Align) { Alignment = Align; }
  Align getAlignment() const { return Alignment; }

  void setOrdering(AtomicOrdering AO) { Ordering = AO; }
  AtomicOrdering getOrdering() const { return Ordering; }
  bool isAtomic() const { return Ordering != AtomicOrdering::NotAtomic; }

  void setVolatile(bool V) { IsVolatile = V; }
  bool isVolatile() const { return IsVolatile; }

  void setSyncScopeID(SyncScope::ID S) { SSID = S; }
  SyncScope::ID getSyncScopeID() const { return SSID; }

  void setAddressSCEV(VPlanSCEV *Expr) {
    assert(!AddressSCEV && "AddressSCEV has been set already");
    AddressSCEV = Expr;
  }
  VPlanSCEV *getAddressSCEV() const { return AddressSCEV; }

  bool isSimple() const { return !isAtomic() && !isVolatile(); }

  void setMetadata(unsigned KindID, MDNode *MD) {
    auto Iter = find_if(MDs, [KindID](auto &P) -> bool {
      return P.first == KindID;
    });
    if (Iter != MDs.end())
      Iter->second = MD;
    else
      MDs.push_back(std::make_pair(KindID, MD));
  }

  MDNode *getMetadata(unsigned KindID) const {
    auto Iter = find_if(MDs, [KindID](auto &P) -> bool {
      return P.first == KindID;
    });
    return (Iter != MDs.end()) ? Iter->second : nullptr;
  }

  // Get all alias analysis related metadata attached to the incoming
  // instruction.
  void getAAMetadata(AAMDNodes &AANodes) const {
    AANodes.TBAA = getMetadata(LLVMContext::MD_tbaa);
    AANodes.TBAAStruct = getMetadata(LLVMContext::MD_tbaa_struct);
    AANodes.Scope = getMetadata(LLVMContext::MD_alias_scope);
    AANodes.NoAlias = getMetadata(LLVMContext::MD_noalias);
  }

  // Get the HIR memory reference corresponding to the value being loaded
  // or value being stored into.
  const loopopt::RegDDRef *getHIRMemoryRef() const {
    if (!HIR().getUnderlyingNode())
      return nullptr;

    auto *OpHIR = cast<VPBlob>(HIR().getOperandHIR());
    auto *RDDR = cast<loopopt::RegDDRef>(OpHIR->getBlob());
    if (!RDDR->hasGEPInfo()) {
      // This corresponds to a standalone load instruction.
      auto *HInst = cast<loopopt::HLInst>(HIR().getUnderlyingNode());
      assert(isa<LoadInst>(HInst->getLLVMInstruction()) &&
             "Expected standalone load HLInst.");
      RDDR = HInst->getRvalDDRef();
    }
    assert(RDDR && RDDR->hasGEPInfo() &&
           "Invalid RegDDRef attached to load/store instruction.");
    return RDDR;
  }

  // Use underlying IR knowledge to access metadata attached to the incoming
  // instruction.
  void readUnderlyingMetadata(const loopopt::RegDDRef *RDDR = nullptr) {
    assert(MDs.empty() && "Underlying metadata was already read");
    if (auto *IRLoadStore = dyn_cast_or_null<Instruction>(getInstruction())) {
      IRLoadStore->getAllMetadataOtherThanDebugLoc(MDs);
      return;
    }
    if (!RDDR && HIR().getUnderlyingNode()) {
      RDDR = getHIRMemoryRef();
      assert(RDDR && "Value should not be nullptr!");
    }
    if (RDDR)
      RDDR->getAllMetadataOtherThanDebugLoc(MDs);
  }

  static bool isLoadOpcode(unsigned Opcode) {
    return (Opcode == Instruction::Load || Opcode == ExpandLoad ||
            Opcode == ExpandLoadNonu);
  }

  static bool isStoreOpcode(unsigned Opcode) {
    return (Opcode == Instruction::Store || Opcode == CompressStore ||
            Opcode == CompressStoreNonu);
  }

  unsigned getPointerOperandIndex() const {
    if (isLoadOpcode(getOpcode()))
      return 0;
    assert(isStoreOpcode(getOpcode()) && "Unknown LoadStore opcode");
    return 1;
  }

  VPValue *getPointerOperand() const {
    return getOperand(getPointerOperandIndex());
  }
  /// Get type of the pointer operand. Note that it might be either PointerType
  /// or VectorType.
  Type *getPointerOperandType() const { return getPointerOperand()->getType(); }
  /// Get address space of the pointer operand.
  unsigned getPointerAddressSpace() const {
    return cast<PointerType>(getPointerOperandType()->getScalarType())
        ->getAddressSpace();
  }

  Type *getValueType() const {
    if (isLoadOpcode(getOpcode()))
      return getType();
    return getOperand(0)->getType();
  }

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return isLoadOpcode(VPI->getOpcode()) || isStoreOpcode(VPI->getOpcode());
  }

  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printDetails(raw_ostream &O) const {
    unsigned Alignment = getAlignment().value();
    O << "    Align: " << Alignment << "\n";
    O << "    Ordering: " << static_cast<unsigned>(getOrdering())
      << ", Volatile: " << isVolatile()
      << ", SSID: " << static_cast<unsigned>(getSyncScopeID()) << "\n";
    if (!MDs.empty()) {
      O << "    NonDbgMDs -\n";
      for (auto MDPair : MDs) {
        O << "      ";
        MDPair.second->print(O);
        O << "\n";
      }
    }
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  virtual VPLoadStoreInst *cloneImpl() const override {
    SmallVector<VPValue *, 2> Ops;
    for (auto &O : operands())
      Ops.push_back(O);
    VPLoadStoreInst *Cloned = new VPLoadStoreInst(getOpcode(), getType(), Ops);
    Cloned->setAlignment(getAlignment());
    Cloned->setOrdering(getOrdering());
    Cloned->setVolatile(isVolatile());
    Cloned->setSyncScopeID(getSyncScopeID());
    Cloned->MDs = MDs;
    // AddressSCEV cannot be propagated to the Cloned VPlan. VPlanSCEV may refer
    // to VPInstructions, therefore it cannot be used in a different VPlan.
    // AddressSCEVs in the cloned VPlan are recomputed after a new VPSE instance
    // is assigned to the cloned VPlan.
    return Cloned;
  }

private:
  // Captures alignment of underlying access.
  Align Alignment;
  AtomicOrdering Ordering = AtomicOrdering::NotAtomic;
  bool IsVolatile = false;
  SyncScope::ID SSID = SyncScope::SingleThread;
  VPlanSCEV *AddressSCEV = nullptr; //< VPlanSCEV for pointer operand
  MDNodesTy MDs;
};

/// Concrete class to represent copy instruction semantics in VPlan constructed
/// for HIR input.
class VPHIRCopyInst : public VPInstruction {
private:
  // Represents an ID that this copy instruction is tagged with. This is
  // needed to identify all copies that are created during SSA deconstruction of
  // a single VPPHINode.
  int OriginPhiId = -1;

public:
  VPHIRCopyInst(VPValue *CopyFrom)
      : VPInstruction(VPInstruction::HIRCopy, CopyFrom->getType(), {CopyFrom}) {
  }

  /// Setter/getter for OriginPhiId.
  int getOriginPhiId() const { return OriginPhiId; }
  void setOriginPhiId(int Id) { OriginPhiId = Id; }

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::HIRCopy;
  }

  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

protected:
  virtual VPHIRCopyInst *cloneImpl() const override {
    VPHIRCopyInst *Cloned = new VPHIRCopyInst(getOperand(0));
    Cloned->setOriginPhiId(getOriginPhiId());
    return Cloned;
  }
};

/// Concrete class to represent Call instruction in VPlan.
class VPCallInstruction : public VPInstruction {
private:
  /// Data structure to record specific vectorization properties for a call
  /// instruction.
  struct CallVecProperties {
    unsigned VF = 0;
    std::unique_ptr<const VFInfo> MatchedVecVariant;
    unsigned MatchedVecVariantIndex = 0;
    Optional<StringRef> VectorLibraryFn = None;
    Intrinsic::ID VectorIntrinsic = Intrinsic::not_intrinsic;
    unsigned PumpFactor = 1;
    // Specifies if masked version of a vector variant should be used to
    // vectorize the unmasked call (using all-true mask).
    unsigned UseMaskedForUnmasked : 1;
    // NOTE: the order of remarks in this enum is strictly tied to the order of
    // remarks in Diag.cpp (15558-155562)
    enum class SerializationReason {
      UNDEFINED = 0,
      // Remark #15558: Call to function '%s' was serialized due to no suitable
      // vector variants were found.
      NO_VECTOR_VARIANT,
      // Remark #15559: Call to function '%s' was serialized due to no vector
      // variants were found. Consider adding #pragma omp declare simd.
      //
      // This value can't be assigned directly.
      NO_VECTOR_VARIANT_WO_PRAGMA,
      // Remark #15560: Indirect call cannot be vectorized.
      INDIRECT_CALL,
      // Remark #15561: Call to function '%s' was serialized due to operating on
      // scalar operand(s).
      SCALAR_OPERANDS,
      // Remark #15562: Call '%s' cannot be vectorized for current context.
      CURRENT_CONTEXT,
    };
    SerializationReason Reason = SerializationReason::UNDEFINED;
    SerializationReason getSerializationReason () { return Reason; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    void printImpl(raw_ostream &OS) const {
      std::string VecVariantName =
          MatchedVecVariant != nullptr ? MatchedVecVariant->VectorName : "None";
      OS << "  VecVariant: " << VecVariantName << "\n";
      OS << "  VecVariantIndex: " << MatchedVecVariantIndex << "\n";
      OS << "  VecIntrinsic: " << Intrinsic::getBaseName(VectorIntrinsic)
         << "\n";
      OS << "  UseMaskForUnmasked: " << UseMaskedForUnmasked << "\n";
      OS << "  VecLibFn: " << VectorLibraryFn << "\n";
      OS << "  PumpFactor: " << PumpFactor << "\n";
    }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

    CallVecProperties clone() const {
      return CallVecProperties{
          VF,
          MatchedVecVariant ? std::make_unique<const VFInfo>(*MatchedVecVariant)
                            : nullptr,
          MatchedVecVariantIndex,
          VectorLibraryFn,
          VectorIntrinsic,
          PumpFactor,
          UseMaskedForUnmasked,
      };
    }
  } VecProperties;

  enum class CallVecScenarios {
    Undefined,
    // Vectorize call using appropriate vector library function.
    LibraryFunc,
    // Vectorize call using matching SIMD vector variant.
    VectorVariant,
    // Trivially vectorizable call using vector intrinsics.
    TrivialVectorIntrinsic,
    // Specifies if call should be emulated with serialization i.e. appropriate
    // scalar call for each vector lane. Serialization is done today in VPlan
    // for indirect calls and non-vectorizable function calls (functions with no
    // vector library equivalent or matching vector variants).
    Serialization,
    // Specifies if call must not be widened in outgoing vector code based on
    // its -
    // A) underlying attributes. For example, DPC++ kernel uniform calls
    // (function attribute "kernel-uniform-call"), memory fences, prefetch
    // intrinsics with scalar-only semantics.
    // B) uniformity of operands when used in deterministic function calls with
    // no side-effects.
    DoNotWiden,
    // Use associated vector-variants but ignore the mask at the call side - the
    // purpose is to "switch" vectorization dimension and the callee is expected
    // to start with all-ones mask. No pumping is allowed here as well and the
    // inability to match vector-variant isn't expected.
    UnmaskedWiden
  };

  /// Tracks the decision taken on how to vectorize this VPCallInst for given
  /// VF.
  CallVecScenarios VecScenario = CallVecScenarios::Undefined;

  FunctionType *FnTy;
  const CallInst *OrigCall;

protected:
  // Most generic ctor.
  VPCallInstruction(unsigned OpCode, VPValue *Callee, FunctionType *FnTy,
                    ArrayRef<VPValue *> ArgList, const CallInst *OrigCall)
      : VPInstruction(OpCode, FnTy->getReturnType(), ArgList), FnTy(FnTy),
        OrigCall(OrigCall) {
    assert(Callee && "Call instruction does not have Callee");
    // Add called value to end of operand list for def-use chain.
    addOperand(Callee);
    resetVecScenario(0 /*Initial VF*/);

    if (OrigCall) {
      // Check if Call should not be strictly widened i.e. not (re-)vectorized
      // or serialized.
      if (OrigCall->hasFnAttr("kernel-uniform-call"))
        VecScenario = CallVecScenarios::DoNotWiden;

      if (OrigCall->hasFnAttr("unmasked"))
        VecScenario = CallVecScenarios::UnmaskedWiden;
    }
  }

public:
  using CallVecScenariosTy = CallVecScenarios;
  using SerializationReasonTy = CallVecProperties::SerializationReason;

  // When we have no underlying CallInst.
  VPCallInstruction(VPValue *Callee, FunctionType *FnTy,
                    ArrayRef<VPValue *> ArgList)
      : VPCallInstruction(Instruction::Call, Callee, FnTy, ArgList, nullptr) {}

  // If extra information from the underlying CallInst is available.
  VPCallInstruction(VPValue *Callee, ArrayRef<VPValue *> ArgList,
                    const CallInst &OrigCallInst)
      : VPCallInstruction(Instruction::Call, Callee,
                          OrigCallInst.getFunctionType(), ArgList,
                          &OrigCallInst) {}

  /// Helper utility to access underlying CallInst corresponding to this
  /// VPCallInstruction. The utility works for both LLVM-IR and HIR paths.
  /// Returns nullptr if VPCall has no underlying CallInst.
  const CallInst *getUnderlyingCallInst() const {
    if (auto *IRCall = dyn_cast_or_null<CallInst>(getInstruction()))
      return IRCall;
    else if (auto *HIRCall = HIR().getUnderlyingNode()) {
      auto *IRCall = cast<loopopt::HLInst>(HIRCall)->getCallInst();
      assert (IRCall && "Underlying call instruction expected here.");
      return IRCall;
    } else
      return nullptr;
  }

  /// Helper function to access CalledValue (last operand).
  VPValue *getCalledValue() const { return *(op_end() - 1); }

  /// Getter to return called function for this Call instruction. Returns
  /// nullptr for indirect calls.
  Function *getCalledFunction() const {
    if (auto *Func = dyn_cast<VPConstant>(getCalledValue()))
      return cast<Function>(Func->getConstant());

    // Indirect call.
    return nullptr;
  }

  FunctionType *getFunctionType() const { return FnTy; }

  /// Getter for called function's calling convention.
  CallingConv::ID getOrigCallingConv() const {
    if (const CallInst *Call = getUnderlyingCallInst())
      return Call->getCallingConv();
    Function *F = getCalledFunction();
    assert(F && "F is indirect call");
    return F->getCallingConv();
  }

  bool isNoVecVariant() const {
    return VecProperties.Reason == SerializationReasonTy::NO_VECTOR_VARIANT ||
           VecProperties.Reason == SerializationReasonTy::
           NO_VECTOR_VARIANT_WO_PRAGMA;
  }

  /// Getter for original call's tail call attribute.
  bool isOrigTailCall() const {
    if (const CallInst *Call = getUnderlyingCallInst())
      return Call->isTailCall();
    return false;
  }

  // Getter for original call's callsite attributes. If underlying call is
  // absent, then return empty AttributesList.
  AttributeList getOrigCallAttrs() const {
    if (const CallInst *Call = getUnderlyingCallInst())
      return Call->getAttributes();
    return {};
  }

  // Some helpful getters based on underlying call's attributes.
  bool isKernelCallOnce() const {
    return getOrigCallAttrs().hasFnAttr("kernel-call-once");
  }
  bool isOCLVecUniformReturn() const {
    return getOrigCallAttrs().hasFnAttr("opencl-vec-uniform-return");
  }
  bool isKernelUniformCall() const {
    return getOrigCallAttrs().hasFnAttr("kernel-uniform-call");
  }

  /// Return \p true if this call is a lifetime_start/end intrinsic call.
  bool isLifetimeStartOrEndIntrinsic() const {
    return isIntrinsicFromList(
        {Intrinsic::lifetime_start, Intrinsic::lifetime_end});
  }

  /// Return \p true if this call is a intrinsic from the given list \p
  /// IntrinsicsList.
  bool isIntrinsicFromList(ArrayRef<Intrinsic::ID> IntrinsicsList) const {
    if (auto *F = getCalledFunction())
      return F->isIntrinsic() &&
             llvm::find(IntrinsicsList, F->getIntrinsicID()) !=
                 IntrinsicsList.end();
    return false;
  }

  /// Clear decision that was last computed for this call, and reset to initial
  /// state (Undef scenario) for new VF.
  void resetVecScenario(unsigned NewVF) {
    // Record VF for which new vectorization scenario and properties will be
    // recorded.
    VecProperties.VF = NewVF;

    // If call does not have any underlying IR, then it was emitted by
    // intermediate VPlan transforms. Don't try to reset any decisions as the
    // transform has specialized knowledge on how to handle these calls.
    if (getUnderlyingCallInst() == nullptr)
      return;

    // DoNotWiden is used for kernel uniform calls and for uniform calls without
    // side-effects today i.e. the property is not VF-dependent. Hence it need
    // not be reset here.
    if (VecScenario == CallVecScenarios::DoNotWiden)
      return;

    if (VecScenario != CallVecScenarios::UnmaskedWiden) {
      // UnmaskedWiden needs the resets below (mainly the MatchedVecvariant),
      // but the scenario itself is set in stone.
      VecScenario = CallVecScenarios::Undefined;
    }

    VecProperties.MatchedVecVariant.reset();
    VecProperties.MatchedVecVariantIndex = 0;
    VecProperties.VectorLibraryFn = None;
    VecProperties.VectorIntrinsic = Intrinsic::not_intrinsic;
    VecProperties.PumpFactor = 1;
    VecProperties.UseMaskedForUnmasked = 0;
  }

  /// Setter functions for different possible states of VecScenario.

  // Serialization.
  void setShouldBeSerialized() {
    assert(VecScenario == CallVecScenarios::Undefined &&
           "Inconsistent scenario update.");
    if (isKernelCallOnce())
      report_fatal_error(
          "Calls with kernel-call-once attributes cannot be serialized.");
    VecScenario = CallVecScenarios::Serialization;
  }

  void setSerializationReason(SerializationReasonTy Value) {
    VecProperties.Reason = Value;
  }

  // Vectorization using vector library functions (like SVML).
  void setVectorizeWithLibraryFn(StringRef VecLibFn, unsigned PumpFactor = 1) {
    assert(!VecLibFn.empty() && "Invalid VecLibFn.");
    assert(VecScenario == CallVecScenarios::Undefined &&
           "Inconsistent scenario update.");
    assert(PumpFactor == 1 || !isKernelCallOnce() &&
                                  "Pumped vectorization of a kernel "
                                  "called-once function is not allowed.");
    VecScenario = CallVecScenarios::LibraryFunc;
    VecProperties.VectorLibraryFn = VecLibFn;
    VecProperties.PumpFactor = PumpFactor;
  }

  // Unmasked widening - the scenario itself is immutable, but some data is
  // VF-dependent.
  void setUnmaskedVectorVariant(VFInfo VecVariant,
                                unsigned VecVariantIndex) {
    assert(VecScenario == CallVecScenarios::UnmaskedWiden &&
           "Inconsistent scenario update.");
    VecProperties.MatchedVecVariant = std::make_unique<const VFInfo>(VecVariant);
    VecProperties.MatchedVecVariantIndex = VecVariantIndex;
  }

  // Vectorization using SIMD vector variant.
  void setVectorizeWithVectorVariant(VFInfo VecVariant,
                                     unsigned VecVariantIndex,
                                     bool UseMaskedForUnmasked = false,
                                     unsigned PumpFactor = 1) {
    assert(VecScenario == CallVecScenarios::Undefined &&
           "Inconsistent scenario update.");
    assert(PumpFactor == 1 || !isKernelCallOnce() &&
                                  "Pumped vectorization of a kernel "
                                  "called-once function is not allowed.");
    VecScenario = CallVecScenarios::VectorVariant;
    VecProperties.MatchedVecVariant = std::make_unique<const VFInfo>(VecVariant);
    VecProperties.MatchedVecVariantIndex = VecVariantIndex;
    VecProperties.UseMaskedForUnmasked = UseMaskedForUnmasked;
    VecProperties.PumpFactor = PumpFactor;
  }

  // Trivially vectorizable calls using intrinsics.
  void setVectorizeWithIntrinsic(Intrinsic::ID VectorInrinID) {
    assert(VecScenario == CallVecScenarios::Undefined &&
           "Inconsistent scenario update.");
    VecScenario = CallVecScenarios::TrivialVectorIntrinsic;
    VecProperties.VectorIntrinsic = VectorInrinID;
  }

  // Call should not be widened in outgoing IR.
  void setShouldNotBeWidened() {
    assert(VecScenario == CallVecScenarios::Undefined &&
           "Inconsistent scenario update.");
    VecScenario = CallVecScenarios::DoNotWiden;
  }

  /// Get decision about how call will be handled by vectorizer.
  CallVecScenariosTy getVectorizationScenario() const { return VecScenario; }
  /// Get VF for which decision was last recorded.
  unsigned getVFForScenario() const { return VecProperties.VF; }
  /// Getter for vector library function.
  StringRef getVectorLibraryFunc() const {
    assert(VecScenario == CallVecScenarios::LibraryFunc &&
           "Can't get VectorLibraryFn for mismatched scenario.");
    return VecProperties.VectorLibraryFn.value();
  }
  /// Getters for matched vector variant.
  const VFInfo *getVectorVariant() const {
    assert((VecScenario == CallVecScenarios::VectorVariant ||
            VecScenario == CallVecScenarios::UnmaskedWiden) &&
           "Can't get VectorVariant for mismatched scenario.");
    return VecProperties.MatchedVecVariant.get();
  }
  unsigned getVectorVariantIndex() const {
    return VecProperties.MatchedVecVariantIndex;
  }
  bool shouldUseMaskedVariantForUnmasked() const {
    assert(VecScenario == CallVecScenarios::VectorVariant &&
           "Can't get VectorVariant for mismatched scenario.");
    return VecProperties.UseMaskedForUnmasked;
  }
  /// Getter for pump factor.
  unsigned getPumpFactor() const {
    if (VecProperties.PumpFactor > 1) {
      // TODO: Extend to allow vector-variant pumping in future.
      assert(VecScenario == CallVecScenarios::LibraryFunc &&
             "Only vectorized calls can be pumped multi-way.");
      assert(!isKernelCallOnce() &&
             "Calls with kernel-call-once cannot be pumped.");
    }
    return VecProperties.PumpFactor;
  }
  /// Getter for vector intrinsic.
  Intrinsic::ID getVectorIntrinsic() const {
    assert(VecScenario == CallVecScenarios::TrivialVectorIntrinsic &&
           "Can't get VectorIntrinsic for mismatched scenario.");
    return VecProperties.VectorIntrinsic;
  }
  std::string getVectorIntrinName() const {
    Intrinsic::ID VecID = getVectorIntrinsic();
    if (VecID == Intrinsic::not_intrinsic)
      return Intrinsic::getName(VecID).str();

    assert(!getType()->isVoidTy() && "Expected non-void function");
    unsigned VF = getVFForScenario();
    if (VF == 0)
      return Intrinsic::getBaseName(VecID).str();

    SmallVector<Type *, 1> TysForName;
    TysForName.push_back(getWidenedType(getType(), VF));
    Function *F = getCalledFunction();
    assert(F && "Indirect calls not expected here.");
    return Intrinsic::getName(VecID, TysForName, F->getParent());
  }

  /// Getters for SerializationReason
  int getSerialReasonNum() {
    return static_cast<int>(VecProperties.getSerializationReason());
  }

  SerializationReasonTy getSerialReason() {
    return VecProperties.getSerializationReason();
  }

  /// Call argument list size.
  unsigned getNumArgOperands() const {
    assert(getNumOperands() != 0 && "Invalid VPCallInstruction.");
    return getNumOperands() - 1;
  }

  /// Call arguments operand ranges.
  operand_range arg_operands(void) {
    return make_range(op_begin(), op_end() - 1);
  }
  const_operand_range arg_operands(void) const {
    return make_range(op_begin(), op_end() - 1);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printImpl(raw_ostream &O) const;

  void dumpVecProperties(raw_ostream &OS) const {
    OS << "For VF=" << VecProperties.VF << "\n";
    OS << "  Decision: ";
    switch (VecScenario) {
    case CallVecScenarios::LibraryFunc:
      OS << "LibraryFunc";
      break;
    case CallVecScenarios::VectorVariant:
      OS << "VectorVariant";
      break;
    case CallVecScenarios::TrivialVectorIntrinsic:
      OS << "TrivialVectorIntrinsic";
      break;
    case CallVecScenarios::Serialization:
      OS << "Serialize";
      break;
    case CallVecScenarios::DoNotWiden:
      OS << "DoNotWiden";
      break;
    default:
      llvm_unreachable("Unexpected VecScenario.");
    }
    OS << "\n";
    VecProperties.printImpl(OS);
    OS << "\n";
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == Instruction::Call ||
           VPI->getOpcode() == VPInstruction::TransformLibraryCall;
  }

  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  VPValue *getArgOperand(unsigned i) const {
    assert(i < getNumArgOperands() && "Out of bounds!");
    return getOperand(i);
  }

  bool isIntelIndirectCall() const {
    Function *F = getCalledFunction();
    if (F)
      return F->getName().startswith("__intel_indirect_call");
    return false;
  }

  const CallInst *getOriginalCall() const { return OrigCall; }

protected:
  virtual VPCallInstruction *cloneImpl() const final {
    VPCallInstruction *Cloned = new VPCallInstruction(
        getCalledValue(), FnTy, ArrayRef<VPValue *>(op_begin(), op_end() - 1));
    Cloned->OrigCall = getUnderlyingCallInst();
    Cloned->VecScenario = VecScenario;
    Cloned->VecProperties = VecProperties.clone();
    return Cloned;
  }
};

// VPInstruction to initialize vector for induction variable.
// It's initialized depending on the binary operation,
// For +/-   : broadcast(start) + step*{0, 1,..,VL -1}
// For */div : broadcast(start) * pow(step,{0, 1,..,VL -1})
// Other binary operations are not induction-compatible.
class VPInductionInit : public VPInstruction {
public:
  VPInductionInit(VPValue *Start, VPValue *Step, VPValue *StartVal,
                  VPValue *EndVal, unsigned Opc)
      : VPInstruction(VPInstruction::InductionInit, Start->getType(),
                      {Start, Step}),
        BinOpcode(Opc), StartVal(StartVal), EndVal(EndVal) {}

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::InductionInit;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  unsigned getBinOpcode() const { return BinOpcode; }

  VPValue *getStep() const { return getOperand(1); }
  VPValue *getStartValueOperand() const { return getOperand(0); }

  // Replaces start value with the \p newVal
  void replaceStartValue(VPValue *NewVal) {
    assert(NewVal && "Unexpected null start value");
    assert(NewVal->getType() == getStartValueOperand()->getType() &&
           "Inconsistent operand type");
    setOperand(0, NewVal);
  }

  // This is function added to have a consistent interface with
  // VPReductionInit, for an easier templatization of some code.
  bool usesStartValue() const {return true;}

  // Return lower/upper value ranges for the induction.
  VPValue *getStartVal() const { return StartVal; }
  VPValue *getEndVal() const { return EndVal; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printDetails(raw_ostream &O) const {
    O << ", StartVal: ";
    if (auto *StartVal = cast<const VPInductionInit>(this)->getStartVal())
      StartVal->printAsOperand(O);
    else
      O << "?";
    O << ", EndVal: ";
    if (auto *EndVal = cast<const VPInductionInit>(this)->getEndVal())
      EndVal->printAsOperand(O);
    else
      O << "?";
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
protected:
  // Clones VPinductionInit.
  virtual VPInductionInit *cloneImpl() const final {
    return new VPInductionInit(getOperand(0), getOperand(1), StartVal,
                               EndVal, getBinOpcode());
  }

private:
  unsigned BinOpcode;
  VPValue *StartVal;
  VPValue *EndVal;
};

// VPInstruction to initialize vector for induction step.
// It's initialized depending on the binary operation,
// For +/-   : broadcast(step*VL)
// For */div : broadcast(pow(step,VL))
// Other binary operations are not induction-compatible.
class VPInductionInitStep : public VPInstruction {
public:
  VPInductionInitStep(VPValue *Step, Instruction::BinaryOps Opcode)
      : VPInstruction(VPInstruction::InductionInitStep, Step->getType(),
                      {Step}),
        BinOpcode(Opcode) {}

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::InductionInitStep;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }
  Instruction::BinaryOps getBinOpcode() const { return BinOpcode; }

protected:
  // Clones VPInductionInistStep.
  virtual VPInductionInitStep *cloneImpl() const final {
    return new VPInductionInitStep(getOperand(0), getBinOpcode());
  }

private:
  Instruction::BinaryOps BinOpcode = Instruction::BinaryOpsEnd;
};

// VPInstruction for induction last value calculation.
// It's calculated depending on the binary operation,
// For +/-   : lv = start OP step*count
// For */div : lv = start OP pow(step, count)
// Other binary operations are not induction-compatible.
//
// We should choose the optimal way for that - probably, for mul/div we should
// prefer scalar calculations in the loop body or extraction from the final
// vector.
class VPInductionFinal : public VPInstruction {
public:
  /// Constructor to extract last lane (for */div).
  VPInductionFinal(VPValue *InducVec)
      : VPInstruction(VPInstruction::InductionFinal, InducVec->getType(),
                      {InducVec}) {}

  /// Constructor to calculate using close-form (start+step*rounded_tc). The
  /// rounded trip count is known at code generation.
  VPInductionFinal(VPValue *Start, VPValue *Step, Instruction::BinaryOps Opcode)
      : VPInstruction(VPInstruction::InductionFinal, Start->getType(),
                      {Start, Step}),
        BinOpcode(Opcode) {}

  /// Return operand that corresponds to the reducing value.
  VPValue *getInductionOperand() const {
    assert(getNumOperands() == 1 && "Incorrect operand request");
    return getOperand(0);
  }

  /// Return operand that corresponds to the start value.
  VPValue *getInitOperand() const {
    assert(getNumOperands() == 2 && "Incorrect operand request");
    return getOperand(0);
  }

  /// Return operand that corresponds to the step value.
  VPValue *getStepOperand() const {
    assert(getNumOperands() == 2 && "Incorrect operand request");
    return getOperand(1);
  }

  bool isLastValPreIncrement() const { return LastValPreIncrement; }
  void setLastValPreIncrement(bool V) { LastValPreIncrement = V; }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::InductionFinal;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  Instruction::BinaryOps getBinOpcode() const { return BinOpcode; }

  /// Return true if start value is used in induction last value calculation.
  bool usesStartValue() const { return false; }

  // Replaces start value with the \p newVal
  void replaceStartValue(VPValue *NewVal) {
    llvm_unreachable("unsupported replacement");
  }

protected:
  // Clones VPInductionFinal.
  virtual VPInductionFinal *cloneImpl() const final {
    if (getNumOperands() == 1)
      return new VPInductionFinal(getInductionOperand());
    else if (getNumOperands() == 2)
      return new VPInductionFinal(getInitOperand(), getStepOperand(),
                                  getBinOpcode());
    else
      llvm_unreachable("Too many operands.");
  }

private:
  // Tracks if induction's last value is computed before increment.
  bool LastValPreIncrement = false;
  Instruction::BinaryOps BinOpcode = Instruction::BinaryOpsEnd;
};

// VPInstruction for reduction initialization.
// It can be done in two ways and should be aligned with last value
// calculation  The first way is just broadcast(identity), the second one is
// to calculate vector of {start_value, identity,...,identity}. The second way
// is acceptable for some reductions (+,-,*) and allows eliminating one scalar
// operation during last value calculation. Though, that is ineffective for
// multiplication, while for summation the movd/movq x86 instructions
// perfectly fit this way.
class VPReductionInit : public VPInstruction {
public:
  VPReductionInit(VPValue *Identity, bool UseStart, bool IsScalar = false)
      : VPInstruction(VPInstruction::ReductionInit, Identity->getType(),
                      {Identity}),
        UsesStartValue(UseStart), IsScalar(IsScalar) {}

  VPReductionInit(VPValue *Identity, VPValue *StartValue,
                  bool IsScalar = false)
      : VPInstruction(VPInstruction::ReductionInit, Identity->getType(),
                      {Identity, StartValue}),
        UsesStartValue(true), IsScalar(IsScalar) {}

  /// Return operand that corresponds to the indentity value.
  VPValue *getIdentityOperand() const { return getOperand(0);}

  /// Return operand that corresponds to the start value. Can be nullptr for
  /// optimized reduce.
  VPValue *getStartValueOperand() const {
    return getNumOperands() > 1 ? getOperand(1)
                                : (UsesStartValue ? getOperand(0) : nullptr);
  }

  /// Return true if start value is used in reduction initialization. E.g.
  /// min/max reductions use the start value as identity.
  bool usesStartValue() const { return UsesStartValue; }

  bool isScalar() const { return IsScalar; }

  /// Replaces start value with the \p newV
  void replaceStartValue(VPValue *NewVal) {
    assert(usesStartValue() && NewVal && "Can't replace start value");
    assert(NewVal->getType() == getStartValueOperand()->getType() &&
           "Inconsistent operand type");
    unsigned Ind = getNumOperands() - 1;
    // Can't use replaceUsesOfWith() due to two operands can have same value.
    setOperand(Ind, NewVal);
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::ReductionInit;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

protected:
  // Clones VPReductionInit.
  virtual VPReductionInit *cloneImpl() const final {
    if (getNumOperands() == 1)
      return new VPReductionInit(getIdentityOperand(), UsesStartValue,
                                 isScalar());
    else if (getNumOperands() == 2)
      return new VPReductionInit(getIdentityOperand(), getStartValueOperand(),
                                 isScalar());
    else
      llvm_unreachable("Too many operands.");
  }

private:
  bool UsesStartValue;
  bool IsScalar;
};

// VPInstruction for reduction last value calculation.
// It's calculated depending on the binary operation. A general sequence
// is generated:
// v_tmp = shuffle(value,m1) // v_tmp contains the upper half of value
// v_tmp = value OP v_tmp;
// v_tmp2 = shuffle(v_tmp, m2) // v_tmp2 contains the upper half of v_tmp1
// v_tmp2 = v_tmp1 OP v_tmp2;
// ...
// res = (is_minmax || optimized_plus) ? vtmp_N : start OP vtp_N;
// (For minmax and optimized summation (see VPReductionInit) we don't
// need operation in the last step.)
//
// A special way is required for min/max+index reductions. The index
// part of the reduction has a link to the main, min/max, part and code
// generation for it requires two values of the main part, the last vector
// value and the last scalar value. They can be accesses having a link
// to the main instruction, which is passed as an additional argument to
// the index part.
//
class VPReductionFinal : public VPInstruction {
public:
  /// General constructor
  VPReductionFinal(unsigned BinOp, VPValue *ReducVec, VPValue *StartValue,
                   bool Sign)
      : VPInstruction(VPInstruction::ReductionFinal, ReducVec->getType(),
                      {ReducVec, StartValue}),
        BinOpcode(BinOp), Signed(Sign), IsLinearIndex(false) {}

  /// Constructor for optimized summation
  VPReductionFinal(unsigned BinOp, VPValue *ReducVec,
                   unsigned Opcode = VPInstruction::ReductionFinal)
      : VPInstruction(Opcode, ReducVec->getType(), {ReducVec}),
        BinOpcode(BinOp), Signed(false), IsLinearIndex(false) {}

  /// Constructor for index part of min/max+index reduction.
  VPReductionFinal(unsigned BinOp, VPValue *ReducVec, VPValue *ParentExit,
                   VPReductionFinal *ParentFinal, bool Sign)
      : VPInstruction(VPInstruction::ReductionFinal, ReducVec->getType(),
                      {ReducVec, ParentExit, ParentFinal}),
        BinOpcode(BinOp), Signed(Sign), IsLinearIndex(false) {}

  /// Constructor for SelectCmp reduction-final
  VPReductionFinal(unsigned BinOp, VPValue *ReducVec, VPValue *StartValue,
                   VPValue *ChangeValue, bool Sign)
      : VPInstruction(VPInstruction::ReductionFinal, ReducVec->getType(),
                      {ReducVec, StartValue, ChangeValue}),
        BinOpcode(BinOp), Signed(Sign), IsLinearIndex(false) {}

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::ReductionFinal ||
           V->getOpcode() == VPInstruction::ReductionFinalInscan;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  unsigned getBinOpcode() const { return BinOpcode; }

  bool isSigned() const { return Signed; }

  /// Return operand that corresponds to the reducing value.
  VPValue *getReducingOperand() const { return getOperand(0);}

  /// Return operand that corresponds to the start value. Can be nullptr for
  /// optimized reduce.
  VPValue *getStartValueOperand() const {
    return getNumOperands() == 2
      || BinOpcode == Instruction::ICmp
      || BinOpcode == Instruction::FCmp ? getOperand(1) : nullptr;
  }

  /// Return operand that corresponds to the change value for a SelectCmp.
  VPValue *getChangeValueOperand() const {
    return getNumOperands() == 3 ? getOperand(2) : nullptr;
  }

  /// Return operand that corrresponds to min/max parent vector value.
  VPValue *getParentExitValOperand() const {
    return getNumOperands() == 3 ? getOperand(1) : nullptr;
  }

  /// Return operand that corrresponds to min/max parent final value.
  VPValue *getParentFinalValOperand() const {
    return getNumOperands() == 3 ? getOperand(2) : nullptr;
  }

  /// Return true if this instruction is for last value calculation of an index
  /// part of min/max+index idiom.
  bool isMinMaxIndex() const {
    return getParentExitValOperand() != nullptr;
  }

  /// Return true if the instruction uses start value to calculate last value.
  bool usesStartValue() const { return getStartValueOperand() != nullptr; }

  /// Replaces start value with the \p newV
  void replaceStartValue(VPValue *NewVal) {
    assert(usesStartValue() && NewVal && "Can't replace start value");
    assert(NewVal->getType() == getStartValueOperand()->getType() &&
           "Inconsistent operand type");
    replaceUsesOfWith(getStartValueOperand(), NewVal, false);
  }

  /// Return ID of the corresponding reduce intrinsic.
  Intrinsic::ID getVectorReduceIntrinsic() const {
    switch (BinOpcode) {
    case Instruction::Add:
    case Instruction::Sub:
      return Intrinsic::vector_reduce_add;
    case Instruction::FAdd:
    case Instruction::FSub:
      return Intrinsic::vector_reduce_fadd;
    case Instruction::Mul:
      return Intrinsic::vector_reduce_mul;
    case Instruction::FMul:
      return Intrinsic::vector_reduce_fmul;
    case Instruction::And:
      return Intrinsic::vector_reduce_and;
    case Instruction::Or:
      return Intrinsic::vector_reduce_or;
    case Instruction::Xor:
      return Intrinsic::vector_reduce_xor;
    case VPInstruction::UMin:
      return Intrinsic::vector_reduce_umin;
    case VPInstruction::SMin:
      return Intrinsic::vector_reduce_smin;
    case VPInstruction::UMax:
      return Intrinsic::vector_reduce_umax;
    case VPInstruction::SMax:
      return Intrinsic::vector_reduce_smax;
    case VPInstruction::FMax:
      return Intrinsic::vector_reduce_fmax;
    case VPInstruction::FMin:
      return Intrinsic::vector_reduce_fmin;
    default:
      llvm_unreachable("Vector reduction opcode not supported.");
    }
  }

  bool isLinearIndex() const { return IsLinearIndex; }
  void setIsLinearIndex() { IsLinearIndex = true; }

protected:
  // Clones VPReductionFinal.
  virtual VPReductionFinal *cloneImpl() const final {
    if (BinOpcode == Instruction::ICmp || BinOpcode == Instruction::FCmp)
      // SelectCmp
      return new VPReductionFinal(getBinOpcode(), getReducingOperand(),
                                  getStartValueOperand(),
                                  getChangeValueOperand(), isSigned());
    else if (isMinMaxIndex())
      return new VPReductionFinal(
          getBinOpcode(), getReducingOperand(), getParentExitValOperand(),
          cast<VPReductionFinal>(getParentFinalValOperand()), isSigned());
    else if (getStartValueOperand() == nullptr)
      return new VPReductionFinal(getBinOpcode(), getReducingOperand(),
                                  getOpcode());
    else
      return new VPReductionFinal(getBinOpcode(), getReducingOperand(),
                                  getStartValueOperand(), isSigned());
  }

private:
  unsigned BinOpcode;
  bool Signed;
  bool IsLinearIndex;
};

/// Concrete class to represent last value calculation for user-defined
/// reductions in VPlan.
class VPReductionFinalUDR : public VPInstruction {
public:
  /// Create finalization instruction with its BaseType, operands and UDR object
  /// combiner function pointer.
  VPReductionFinalUDR(Type *BaseTy, ArrayRef<VPValue *> Operands,
                      Function *Combiner)
      : VPInstruction(VPInstruction::ReductionFinalUdr, BaseTy, Operands),
        Combiner(Combiner) {
    assert(Combiner && "Unexpected null Combiner for UDR.");
  }

  /// Return the combiner function stored by this instruction
  Function *getCombiner() const { return Combiner; }

  /// Methods for supporting type inquiry through isa, cast, and
  /// dyn_cast
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::ReductionFinalUdr;
  }
  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

protected:
  virtual VPInstruction *cloneImpl() const final {
    SmallVector<VPValue *, 3> Ops(operands());
    return new VPReductionFinalUDR(getType(), Ops, getCombiner());
  }

private:
  Function *Combiner = nullptr;
};

/// Complicated finalization of inscan reduction is not required,
/// in fact, the final value resides in the last vector lane.
/// Returns the input operand's last vector lane.
/// ExtractLastVectorLane cannot be reused for CFG Merger to work correctly.
class VPReductionFinalInscan : public VPReductionFinal {
public:
  VPReductionFinalInscan(unsigned BinOp, VPValue *ReducVec)
    : VPReductionFinal(BinOp, ReducVec, VPInstruction::ReductionFinalInscan) {}

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::ReductionFinalInscan;
  }
};

/// Calculates the value of running inclusive reduction using the binary
/// operation opcode.
class VPRunningInclusiveReduction : public VPInstruction {
public:
  VPRunningInclusiveReduction(
    unsigned BinOp, VPValue *Input, VPValue *CarryOver, VPValue *Identity,
    unsigned OpCode = VPInstruction::RunningInclusiveReduction)
      : VPInstruction(OpCode, Input->getType(), {Input, CarryOver, Identity}),
        BinOpcode(BinOp) {}

  unsigned getBinOpcode() const { return BinOpcode; }
  VPValue *getInputOperand() const { return getOperand(0); }
  VPValue *getCarryOverOperand() const { return getOperand(1); }
  VPValue *getIdentityOperand() const { return getOperand(2); }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::RunningInclusiveReduction;
  }

protected:
  // Clones VPRunningInclusiveReduction
  virtual VPRunningInclusiveReduction *cloneImpl() const override {
    return new VPRunningInclusiveReduction(getBinOpcode(), getInputOperand(),
                                           getCarryOverOperand(),
                                           getIdentityOperand(),
                                           getOpcode());
  }
private:
  unsigned BinOpcode;
};

/// Calculates the value of running exclusive reduction using the binary
/// opcode.
class VPRunningExclusiveReduction : public VPRunningInclusiveReduction {
public:
  VPRunningExclusiveReduction(
    unsigned BinOp, VPValue *Input, VPValue *CarryOver, VPValue *Identity)
      : VPRunningInclusiveReduction(BinOp, Input, CarryOver, Identity,
                                    VPInstruction::RunningExclusiveReduction) {}

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::RunningExclusiveReduction;
  }
protected:
  // Clones VPRunningExclusiveReduction.
  virtual VPRunningExclusiveReduction *cloneImpl() const final {
    return new VPRunningExclusiveReduction(getBinOpcode(), getInputOperand(),
                                           getCarryOverOperand(),
                                           getIdentityOperand());
  }
};

template <unsigned InstOpcode>
class VPCompressExpandInitFinal : public VPInstruction {
public:
  VPCompressExpandInitFinal(VPValue *V)
      : VPInstruction(InstOpcode, V->getType(), {V}) {}

  bool usesStartValue() const {
    return InstOpcode == VPInstruction::CompressExpandIndexInit;
  }

  void replaceStartValue(VPValue *V) {
    assert(usesStartValue() && V && "Can't replace start value");
    setOperand(0, V);
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == InstOpcode;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

protected:
  virtual VPInstruction *cloneImpl() const final {
    return new VPCompressExpandInitFinal<InstOpcode>(getOperand(0));
  }
};

class VPCompressExpandInit
    : public VPCompressExpandInitFinal<VPInstruction::CompressExpandIndexInit> {
public:
  using VPCompressExpandInitFinal::VPCompressExpandInitFinal;
};

class VPCompressExpandFinal : public VPCompressExpandInitFinal<
                                  VPInstruction::CompressExpandIndexFinal> {
public:
  using VPCompressExpandInitFinal::VPCompressExpandInitFinal;
};

/// Concrete class for representing a vector of steps of arithmetic progression.
class VPConstStepVector : public VPInstruction {
public:
  VPConstStepVector(Type *Ty, int Start, int Step, int NumSteps)
      : VPInstruction(VPInstruction::ConstStepVector, Ty, {}), Start(Start),
        Step(Step), NumSteps(NumSteps) {}

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::ConstStepVector;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printImpl(raw_ostream &O) const {
    O << "const-step-vector: { Start:" << Start << ", Step:" << Step
      << ", NumSteps:" << NumSteps << "}";
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  /// Get the start-value of the constant-vector.
  int getStart() const { return Start; }

  /// Get the step-value of the constant-vector.
  int getStep() const { return Step; }

  /// Get the upper-bound of the constant-vector.
  int getNumSteps() const { return NumSteps; }

private:
  int Start;
  int Step;
  int NumSteps;

protected:
  // Clones VPConstStepVector.
  virtual VPConstStepVector *cloneImpl() const final {
    llvm_unreachable("This instruction should not be cloned. It sould be "
                     "regenerated for a different VF.");
    return nullptr;
  }
};

/// Instruction representing trip count of the scalar loop (OrigLoop member).
class VPOrigTripCountCalculation : public VPInstruction {
public:
  /// The interface here assumes the outermost loop being vectorized because
  /// that's the only scenario where we can guarantee empty VPOperand list of
  /// such calculation (inner loops' trip count might depend on defs from the
  /// outer loop). It also happens to be an assumption used in many other places
  /// of the vectorizer.
  ///
  /// As such, introduce an explicit \p VPL parameter that is used purely for the
  /// assert below.
  VPOrigTripCountCalculation(Loop *OrigLoop, const VPLoop *VPL, Type *Ty)
      : VPInstruction(VPInstruction::OrigTripCountCalculation, Ty, {}),
        OrigLoop(OrigLoop), VPL(VPL) {
    // TODO: For inner loop vectorization, that loop's trip count might be
    // dependent on VPInstructions defined in the outer loop, so we need to
    // determine which ones affect the trip count and pass to VPInstruction
    // operands.
    assert(VPL->getParentLoop() == nullptr &&
           "Only outermost loop vectorization is supported so far!");
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::OrigTripCountCalculation;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  Loop *getOrigLoop() { return OrigLoop; }
  const Loop *getOrigLoop() const { return OrigLoop; }

protected:
  virtual VPOrigTripCountCalculation *cloneImpl() const final {
    return new VPOrigTripCountCalculation(OrigLoop, VPL, getType());
  }

private:
  Loop *OrigLoop;
  const VPLoop *VPL;
};

/// Instruction representing the final value of the IV for the vector
/// loop. We increment that IV by VF*UF, so actual value would be the iteration
/// number of the serial loop execution corresponding the lane 0 of the last
/// vector iteration.
/// Initially is constructed with one operand that represents the original
/// upper bound of the loop. Later we can have a second operand added, which
/// represents an adjustment for the peel loop.
class VPVectorTripCountCalculation : public VPInstruction {
public:
  VPVectorTripCountCalculation(VPValue *OrigTripCount, unsigned UF = 1)
      : VPInstruction(VPInstruction::VectorTripCountCalculation,
                      OrigTripCount->getType(), {OrigTripCount}),
        UF(UF) {}

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::VectorTripCountCalculation;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  unsigned getUF() const { return UF; }
  void setUF(unsigned UF) { this->UF = UF; }

protected:
  virtual VPVectorTripCountCalculation *cloneImpl() const final {
    return new VPVectorTripCountCalculation(getOperand(0), UF);
  }

private:
  unsigned UF;
};

// Base-class for the peel and remainder loop instructions.
template <class LoopTy, class LiveInOpTy, unsigned PeelRemOpcode>
class VPPeelRemainderImpl : public VPInstruction {

  /// The original loop.
  LoopTy *Lp;

  /// The live-in operands list.
  SmallVector<LiveInOpTy *, 4> OpLiveInMap;

  /// Flag to indicate whether the scalar loop has to be cloned. (Because we
  /// need two copies of it and this is the second one.)
  bool NeedsCloning = false;

  /// Set of origin opt-report remarks for scalar loop used for annotation
  /// purposes.
  SmallVector<OptReportStatsTracker::RemarkRecord, 4> OriginRemarks;

  /// Set of general opt-report remarks for scalar loop.
  SmallVector<OptReportStatsTracker::RemarkRecord, 4> GeneralRemarks;

  static LLVMContext &getContext(Loop *Lp) {
    return Lp->getHeader()->getContext();
  }

  static LLVMContext &getContext(loopopt::HLLoop *Lp) {
    return Lp->getHLNodeUtils().getContext();
  }

protected:

  using VPInstruction::setOperand;
  using VPInstruction::addOperand;
  using VPInstruction::removeOperand;
  using VPInstruction::removeAllOperands;

  /// Add \p VPVal to the instruction operands list and the \p OrigLoopUse to
  /// the OpLiveInMap.
  void addLiveIn(VPValue *VPVal, LiveInOpTy *OrigLoopUse) {
    assert(isValidLiveIn(VPVal, OrigLoopUse) &&
           "Live-ins can only be a phi/block/DDRef!");
    addOperand(VPVal);
    OpLiveInMap.push_back(OrigLoopUse);
  }

  virtual bool isValidLiveIn(const VPValue *VPVal,
                             const LiveInOpTy *OrigLoopUse) const = 0;

public:
  VPPeelRemainderImpl(LoopTy *Lp, bool ShouldClone)
      : VPInstruction(PeelRemOpcode, Type::getTokenTy(getContext(Lp)),
                      {} /* Operands */),
        Lp(Lp), NeedsCloning(ShouldClone) {}

  /// Get the original loop.
  LoopTy *getLoop() const { return Lp; }

  /// Set the new original loop after cloning.
  void setClonedLoop(LoopTy *L) {
    assert(L && "unexpected null loop");
    Lp = L;
  }

  /// Return true if cloning is required.
  bool isCloningRequired() const { return NeedsCloning; }

  void setCloningRequired() { NeedsCloning = true; }

  /// Get the live-in value corresponding to the \p Idx.
  LiveInOpTy *getLiveIn(unsigned Idx) const {
    assert(Idx <= OpLiveInMap.size() - 1 &&
           "Invalid entry in the live-in map requested.");
    return OpLiveInMap[Idx];
  }

  unsigned getNumLiveIns() const { return OpLiveInMap.size(); }

  /// Get the live-in value corresponding to the \p Idx.
  void setClonedLiveIn(unsigned Idx, LiveInOpTy *U) {
    assert(Idx <= OpLiveInMap.size() - 1 &&
           "Invalid entry in the live-in map requested.");
    OpLiveInMap[Idx] = U;
  }

  /// Add a new origin remark for outgoing scalar loop.
  void addOriginRemark(OptReportStatsTracker::RemarkRecord R) {
    OriginRemarks.push_back(R);
  }

  /// Add a new general remark for outgoing scalar loop.
  void addGeneralRemark(OptReportStatsTracker::RemarkRecord R) {
    GeneralRemarks.push_back(R);
  }

  /// Get all origin remarks for this scalar loop.
  ArrayRef<OptReportStatsTracker::RemarkRecord> getOriginRemarks() const {
    return OriginRemarks;
  }

  /// Get all general remarks for this scalar loop.
  ArrayRef<OptReportStatsTracker::RemarkRecord> getGeneralRemarks() const {
    return GeneralRemarks;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPInstruction *V) {
    return V->getOpcode() == PeelRemOpcode;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  virtual void printImpl(raw_ostream &O) const = 0;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

protected:
  VPInstruction *cloneImpl() const override {
    assert(false && "not expected to clone");
    return nullptr;
  }
};

template <unsigned PeelRemOpcode>
using VPPeelRemainderIRTy = VPPeelRemainderImpl<Loop, Use, PeelRemOpcode>;

// Class to represent scalar IR peel and remainder loop instructions in VPlan.
template <unsigned PeelRemOpcode>
class VPPeelRemainder : public VPPeelRemainderIRTy<PeelRemOpcode> {
  using Base = VPPeelRemainderIRTy<PeelRemOpcode>;

protected:
  bool isValidLiveIn(const VPValue *VPVal,
                     const Use *OrigLoopUse) const override {
    return VPVal->getType()->isLabelTy() ||
           (isa<PHINode>(OrigLoopUse->getUser()) &&
            cast<PHINode>(OrigLoopUse->getUser())->getParent() ==
                Base::getLoop()->getHeader());
  }

public:
  VPPeelRemainder(Loop *Lp, bool ShouldClone = false) : Base(Lp, ShouldClone) {}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  virtual void printImpl(raw_ostream &O) const override {
    O << " " << Base::getLoop()->getName()
      << ", NeedsCloning: " << Base::isCloningRequired() << ", LiveInMap:";
    assert(Base::getNumOperands() == Base::getNumLiveIns() &&
           "Inconsistent live-ins data!");
    for (unsigned I = 0; I < Base::getNumOperands(); ++I) {
      O << "\n       {";
      Base::getLiveIn(I)->get()->printAsOperand(O);
      O << " in {" << *Base::getLiveIn(I)->getUser() << "} -> ";
      Base::getOperand(I)->printAsOperand(O);
      O << " }";
    }
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPInstruction *V) {
    return V->getOpcode() == PeelRemOpcode;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }
};

template <unsigned PeelRemOpcode>
using VPPeelRemainderHIRTy =
    VPPeelRemainderImpl<loopopt::HLLoop, loopopt::DDRef, PeelRemOpcode>;

// Class to represent scalar HIR peel and remainder loop instructions in VPlan.
template <unsigned PeelRemOpcode>
class VPPeelRemainderHIR : public VPPeelRemainderHIRTy<PeelRemOpcode> {
  using Base = VPPeelRemainderHIRTy<PeelRemOpcode>;

protected:
  bool isValidLiveIn(const VPValue *VPVal,
                     const loopopt::DDRef *OrigLoopUse) const override {
    // We allow temps that are live-in to loop or represent unattached refs like
    // %lb.tmp or %ub.tmp.
    return Base::getLoop()->isLiveIn(OrigLoopUse->getSymbase()) ||
           OrigLoopUse->getHLDDNode() == nullptr;
  }

public:
  VPPeelRemainderHIR(loopopt::HLLoop *HLp, bool ShouldClone = false)
      : Base(HLp, ShouldClone) {}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  virtual void printImpl(raw_ostream &O) const override {
    formatted_raw_ostream FO(O);
    // TODO: How to find unique ID for HLLoop?
    FO << " "
       << "<HLLoop>"
       << ", NeedsCloning: " << Base::isCloningRequired();
    FO << ", TempInitMap:";
    assert(Base::getNumOperands() == Base::getNumLiveIns() &&
           "Inconsistent temp init data!");
    for (unsigned I = 0; I < Base::getNumOperands(); ++I) {
      FO << "\n       { Initialize temp ";
      Base::getLiveIn(I)->print(FO);
      FO << " with -> ";
      Base::getOperand(I)->printAsOperand(FO);
      FO << " }";
    }
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPInstruction *V) {
    return V->getOpcode() == PeelRemOpcode;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }
};

/// Class for representing the 'scalar-peel' instruction.
/// This class holds all the information needed for representing the code
/// required for peel-loop generation.
class VPScalarPeel final : public VPPeelRemainder<VPInstruction::ScalarPeel> {

  // Variable that keeps track of index of the TargetLabel.
  int IndexOfTargetLabel = -1;

  // Variable that keeps track of index of the UpperBound.
  int IndexOfUpperBound = -1;

public:
  VPScalarPeel(Loop *Lp, bool ShouldClone) : VPPeelRemainder(Lp, ShouldClone) {}

  bool isValidLiveIn(const VPValue *VPVal,
                     const Use *OrigLoopUse) const override {
    return VPPeelRemainder::isValidLiveIn(VPVal, OrigLoopUse) ||
           OrigLoopUse == findUpperBoundUseInLatch();
  }

  /// Set the original loop upper-bound \p UB and the corresponding use \p
  /// OrigLoopUse.
  void setUpperBound(VPValue *UB) {
    assert(IndexOfUpperBound == -1 && "The Upper-bound has already been set.");
    IndexOfUpperBound = getNumOperands();
    Use *OrigLoopUse = findUpperBoundUseInLatch();
    addLiveIn(UB, OrigLoopUse);
  }

  /// Set the target-label \p TargetLbl and the target-block \p TargetBlock.
  void setTargetLabel(VPValue *TargetLbl, Use *TargetBlock) {
    assert(IndexOfTargetLabel == -1 &&
           "The Target-label has already been set.");
    IndexOfTargetLabel = getNumOperands();
    addLiveIn(TargetLbl, TargetBlock);
  }

  // Get the original loop upper-bound.
  VPValue *getUpperBound() const {
    assert(IndexOfUpperBound >= 0 && "The Upper-bound has not been set.");
    return getOperand(IndexOfUpperBound);
  }

  // Get the use of the original-loop UB.
  Use *getOrigUBUse() const {
    assert(IndexOfUpperBound >= 0 && "The Upper-bound has not been set.");
    return getLiveIn(IndexOfUpperBound);
  }

  // Get the target-label.
  VPValue *getTargetLabel() const {
    assert(IndexOfTargetLabel >= 0 && "The Target-label has not been set.");
    return getOperand(IndexOfTargetLabel);
  }

  // Get the target-block corresponding to the edge.
  Use *getTargetBlock() const {
    assert(IndexOfTargetLabel >= 0 && "The Target-block has not been set.");
    return getLiveIn(IndexOfTargetLabel);
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::ScalarPeel;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

private:
  Use *findUpperBoundUseInLatch() const;
};

/// Class for representing the 'scalar-peel' instruction for HIR input.
class VPScalarPeelHIR final
    : public VPPeelRemainderHIR<VPInstruction::ScalarPeelHIR> {
  // Variable that is used to track index of UpperBound in LiveInMap.
  int IndexOfUB = -1;

public:
  VPScalarPeelHIR(loopopt::HLLoop *HLp, bool ShouldClone)
      : VPPeelRemainderHIR(HLp, ShouldClone) {}

  // Set upper bound for the loop. It creates a new temp and adds it to
  // live-in map.
  void setUpperBound(VPValue *UB) {
    assert(IndexOfUB == -1 && "Upper bound of peel loop already set");
    IndexOfUB = getNumOperands();
    loopopt::RegDDRef *UBTmp = getLoop()->getHLNodeUtils().createTemp(
        getLoop()->getIVType(), "ub.tmp");
    addLiveIn(UB, UBTmp);
  }

  // Helper to get the operand that defines the upper bound of this scalar peel
  // loop.
  VPValue *getUpperBound() const {
    assert(IndexOfUB >= 0 && "Upper bound of peel loop has not been set.");
    return getOperand(IndexOfUB);
  }

  loopopt::DDRef *getUpperBoundTemp() const {
    assert(IndexOfUB >= 0 && "Upper bound of peel loop has not been set.");
    return getLiveIn(IndexOfUB);
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::ScalarPeelHIR;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }
};

/// Class representing the 'scalar-remainder' instruction.
/// This class holds all the information needed for representing the code
/// required for remainder-loop generation.
class VPScalarRemainder final
    : public VPPeelRemainder<VPInstruction::ScalarRemainder> {

public:
  // TODO: Consider storing the loop as header/latch pair with an assert on some
  // canonical form because \p Lp might become stale during CG stage.
  VPScalarRemainder(Loop *Lp, bool ShouldClone)
      : VPPeelRemainder(Lp, ShouldClone) {}

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::ScalarRemainder;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  /// Add the live-in variable \p VPVal and the corresponding use \p
  /// OrigLoopUse.
  void addLiveIn(VPValue *VPVal, Use *OrigLoopUse) {
    VPPeelRemainder::addLiveIn(VPVal, OrigLoopUse);
  }

  /// Get the original use at index \p Idx.
  Use *getOrigUse(unsigned Idx) const { return getLiveIn(Idx); }
};

class VPScalarRemainderHIR final
    : public VPPeelRemainderHIR<VPInstruction::ScalarRemainderHIR> {
  // Variable used to track index of lower bound in LiveInMap.
  int IndexOfLB = -1;

public:
  VPScalarRemainderHIR(loopopt::HLLoop *HLp, bool ShouldClone)
      : VPPeelRemainderHIR(HLp, ShouldClone) {}

  // Add live-in variable VPVal and corresponding temp.
  void addLiveIn(VPValue *VPVal, loopopt::DDRef *Temp) {
    VPPeelRemainderHIR::addLiveIn(VPVal, Temp);
  }

  // Set lower bound temp for the loop.
  void setLowerBoundTemp(VPValue *LB, loopopt::DDRef *Tmp) {
    assert(IndexOfLB == -1 && "Lower bound for remainder loop is already set");
    IndexOfLB = getNumOperands();
    addLiveIn(LB, Tmp);
  }

  loopopt::DDRef *getLowerBoundTemp() const {
    assert(IndexOfLB >= 0 && "Lower bound for remainder loop is not set.");
    return getLiveIn(IndexOfLB);
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::ScalarRemainderHIR;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }
};

/// Class for representing the vp-scev-wrapper instruction.
/// This class uses alignment analysis infrastructure to capture peeling
/// information for the dynamic-peeling scenario. The CG is responsible to
/// converting this VPInstruction to invoke the SCEVExpander and compute the
/// actual pointer address.
class VPInvSCEVWrapper : public VPInstruction {

  // SCEV object.
  VPlanSCEV *Scev;

  // Is the opaque VPlanSCEV a LLVM SCEV?
  const bool IsSCEV;

public:
  VPInvSCEVWrapper(VPlanSCEV *S, Type *Ty, bool IsSCEV = true)
      : VPInstruction(VPInstruction::InvSCEVWrapper, Ty, {}), Scev(S),
        IsSCEV(IsSCEV) {}

  VPlanSCEV *getSCEV() const { return Scev; }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::InvSCEVWrapper;
  }
  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printImpl(raw_ostream &O) const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
protected:
  VPInstruction *cloneImpl() const override {
    llvm_unreachable("not expected to clone");
    return nullptr;
  }
};

/// The VPlanAdapter is a placeholder for a VPlan in CFG of another VPlan.
/// In some scenarios we have inner loops or some other additionally created
/// loops (e.g. peel/remainder) inside a VPlan. We want those loops to be
/// represented also as VPlans and want to have a separate CFG created for them,
/// to be able to process them independently. At the same time we want to place
/// those loops at correct places in the outer VPlan's CFG and keep those places
/// until we finish processing of the inner loops. The VPlanAdapter keeps
/// pointer to underlying VPlan and accepts operands which are treated as
/// incoming values for underlying VPlan. Finally, the VPlanAdapters are
/// replaced by the code from underlying VPlans, with corresponding replacement
/// of incoming values.
class VPlanAdapter : public VPInstruction {
public:
  VPlanAdapter(VPlan &P);
  const VPlan &getVPlan() const { return Plan; }
  VPlan &getVPlan() { return Plan; }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::PlanAdapter ||
           V->getOpcode() == VPInstruction::PlanPeelAdapter;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printImpl(raw_ostream &O) const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

protected:
  VPlan &Plan;

  // Constructor to have descendants
  VPlanAdapter(unsigned Opcode, VPlan &P);

  VPInstruction *cloneImpl() const override {
    llvm_unreachable("not expected to clone");
    return nullptr;
  }
};

/// VPlanPeelAdapter is an adapter for a peel loop. It provides some
/// security/restrictions specific for a peel loop. I.e. peel loop can be either
/// scalar or vectorized masked loop and it always accepts only original
/// incoming values. The only one parameter is accepted by peel loop, it's peel
/// count which is set as the upper bound of the underlying loop
class VPlanPeelAdapter final : public VPlanAdapter {
  // Declare them private, hiding the public base class methods.
  using VPInstruction::addOperand;
  using VPInstruction::removeAllOperands;
  using VPInstruction::removeOperand;
  using VPInstruction::setOperand;

public:
  VPlanPeelAdapter(VPlan &P) : VPlanAdapter(VPInstruction::PlanPeelAdapter, P) {
    assert((isa<VPlanScalarPeel>(P) || isa<VPlanMasked>(P)) &&
           "Unexpected Vplan");
  }

  // Return the upper bound that will be used in the outermost loop of the
  // underlying VPlan.
  const VPValue *getUpperBound() const;

  // Set the upper bound for the outermost loop of the underlying VPlan.
  void setUpperBound(VPValue *TC);

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::PlanPeelAdapter;
  }

private:
  VPValue *getPeelLoop() const;

  // Peel loop specific utility for HIR path where we update orig-live-out-hir
  // created for main loop IV when upper bound of peel loop is set.
  void updateUBInHIROrigLiveOut();
};

// VPInstruction to allocate private memory. This is translated into
// allocation of a private memory in the function entry block. This instruction
// is not supposed to vectorize alloca instructions that appear inside the loop
// for arrays of a variable size.
class VPAllocatePrivate : public VPInstruction {
  // VPLoopEntityList is allowed to set EntityKind.
  friend class VPLoopEntityList;

public:
  VPAllocatePrivate(Type *Ty, Type *AllocatedTy, Align OrigAlignment)
      : VPInstruction(VPInstruction::AllocatePrivate, Ty, {}),
        AllocatedTy(AllocatedTy), IsSOASafe(false), IsSOAProfitable(false),
        OrigAlignment(OrigAlignment), EntityKind(0) {}

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::AllocatePrivate;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  /// Return true if doing SOA-layout transformation for the given memory is
  /// both safe and profitable.
  bool isSOALayout() const { return IsSOASafe && IsSOAProfitable; }

  /// Return true if memory is safe for SOA, i.e. all uses inside the loop
  /// are known and there are no layout-casts.
  bool isSOASafe() const { return IsSOASafe; }

  /// Return true if it's profitable to do SOA transformation, i.e. there
  /// is at least one uniform/unit-stride load/store to that memory (in case of
  /// private array), or the memory is a scalar structure
  bool isSOAProfitable() const { return IsSOAProfitable; }

  /// Set the property of the memory to be SOA-safe.
  void setSOASafe() { IsSOASafe = true; }

  /// Set the memory to be profitable for SOA-layout.
  void setSOAProfitable() { IsSOAProfitable = true; }

  /// Return alignment of original alloca/global that this private memory
  /// corresponds to.
  Align getOrigAlignment() const { return OrigAlignment; }

  Type *getAllocatedType() const { return AllocatedTy; }

  unsigned getEntityKind() const { return EntityKind; }

protected:

  VPAllocatePrivate *cloneImpl() const override {
    auto Ret = new VPAllocatePrivate(
      getType(), getAllocatedType(), getOrigAlignment());
    if (isSOASafe())
      Ret->setSOASafe();
    if (isSOAProfitable())
      Ret->setSOAProfitable();
    Ret->setEntityKind(getEntityKind());
    return Ret;
  }

private:
  /// Set the opcode of the Entity related to this Alloca.
  void setEntityKind(unsigned Kind) { EntityKind = Kind; }

private:
  Type *AllocatedTy;
  bool IsSOASafe;
  bool IsSOAProfitable;
  Align OrigAlignment;
  unsigned EntityKind;
};

/// Return index of some active lane. Currently we use the first one but users
/// must not rely on that behavior.
class VPActiveLane : public VPInstruction {
public:
  VPActiveLane(VPValue *VectorMask)
      : VPInstruction(VPInstruction::ActiveLane, VectorMask->getType(),
                      {VectorMask}) {
    assert(VectorMask->getType()->isIntegerTy(1) &&
           "Mask is expected to have i1 'scalar' type");
    assert((isa<VPConstant>(VectorMask) ||
            any_of(VectorMask->users(),
                   [](const VPUser *U) {
                     return isa<VPInstruction>(U) &&
                            cast<VPInstruction>(U)->getOpcode() ==
                                VPInstruction::Pred;
                   })) &&
           "Mask operand to VPActiveLane instruction is expected to be a "
           "predicate!");
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::ActiveLane;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

protected:
  VPActiveLane *cloneImpl() const override {
    return new VPActiveLane(getOperand(0));
  }
};

/// Expected to be used in the context when divergent value \p V happens to be
/// uniform under some mask. In that case
///
///   %active = VPActiveLane %mask
///   %extract = VPActiveLaneExtract %v, %active
///
/// would allow to get the desired scalar value.
class VPActiveLaneExtract : public VPInstruction {
public:
  VPActiveLaneExtract(VPValue *V, VPActiveLane *Lane)
      : VPInstruction(VPInstruction::ActiveLaneExtract, V->getType(),
                      {V, Lane}) {}

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::ActiveLaneExtract;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

protected:
  VPActiveLaneExtract *cloneImpl() const override {
    return new VPActiveLaneExtract(getOperand(0),
                                   cast<VPActiveLane>(getOperand(1)));
  }
};

/// PrivateFinalC calculates last value of conditional last private.
/// The \p Exit  operand is value to extract from, the \p Index operand
/// is used to calculate the lane to extract last value, \p Orig operand
/// represents original incoming value of private and is returned when
/// no assignment of private was done in the loop.
template <unsigned InstOpcode> class VPPrivateFinalC : public VPInstruction {
public:
  VPPrivateFinalC(VPValue *Exit, VPValue *Index, VPValue *Orig)
      : VPInstruction(InstOpcode, Exit->getType(), {Exit, Index, Orig}) {}
  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == InstOpcode;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  /// Named operands getters.
  VPValue *getExit() const { return getOperand(0); }
  VPValue *getIndex() const { return getOperand(1); }
  VPValue *getOrig() const { return getOperand(2); }
  void setOrig(VPValue *V) { setOperand(2, V); }


protected:
  VPPrivateFinalC *cloneImpl() const override {
    return new VPPrivateFinalC(getExit(), getIndex(), getOrig());
  }
};

/// VPPrivateFinalCond represents last value calculation for [partially]
/// registerized last private.
using VPPrivateFinalCond = VPPrivateFinalC<VPInstruction::PrivateFinalCond>;

/// VPPrivateFinalCondMem represents last value calculation for in-memory
/// lastprivate.
using VPPrivateFinalCondMem = VPPrivateFinalC<VPInstruction::PrivateFinalCondMem>;

/// VPOrigLiveOut represents the outgoing value from the scalar
/// loop described by VPPeelRemainder/VPPeelRemainderHIR, which is its
/// operand. It links an outgoing scalar value from the loop with VPlan.
/// Example.
///
/// The %vp3 describes outgoing value %add0 from the loop VP_REUSE_LOOP.
/// The %vp4 describes outgoing value of induction %indvars.iv from the loop
/// VP_REUSE_LOOP.
///
/// bb8:
///   token %VP_REUSE_LOOP = re-use-loop for.body,
///           # ... VPloopReuse operands/value_map
///   i32 %vp3 = orig-live-out token %VP_REUSE_LOOP,
///                     liveout: %add0 = add nsw i32 a.io, sum.070
///   i64 %vp4 = orig-live-out token %VP_REUSE_LOOP,
///                     liveout: %indvars.iv.next0 = add nuw nsw i64
///                     %indvars.iv0, 1
///   br label %bb7
///
/// bb7: # preds: bb8, bb6
///   i32 [[VP5:%.*]] = phi-merge  [ i32 %vp3, %bb8 ],  [ i32 live-out0, %bb6 ]
///   i64 [[VP6:%.*]] = phi-merge  [ i64 %vp4, %bb8 ],  [ i64 live-out1, %bb6 ]
///   br label %bb9
///
template <class VPReuseLoopTy, class LiveOutTy, unsigned OrigLiveOutOpcode>
class VPOrigLiveOutImpl : public VPInstruction {
  const LiveOutTy *LiveOutVal;
  unsigned MergeId;

public:
  VPOrigLiveOutImpl(Type *BaseTy, VPReuseLoopTy *PeelRemainder,
                    const LiveOutTy *LiveOutVal, unsigned Id)
      : VPInstruction(OrigLiveOutOpcode, BaseTy, {PeelRemainder}),
        LiveOutVal(LiveOutVal), MergeId(Id) {}

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPInstruction *V) {
    return V->getOpcode() == OrigLiveOutOpcode;
  }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printImpl(raw_ostream &O) const {
    formatted_raw_ostream FO(O);
    FO << " ";
    getOperand(0)->printAsOperand(FO);
    FO << ", liveout: ";
    LiveOutVal->print(FO);
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Only CG needs this...
  const LiveOutTy *getLiveOutVal() const { return LiveOutVal; }
  void setClonedLiveOutVal(LiveOutTy *V) { LiveOutVal = V; }

  // Used during CFG merge.
  unsigned getMergeId() const { return MergeId;}

protected:
  VPInstruction *cloneImpl() const override {
    llvm_unreachable("not expected to clone");
    return nullptr;
  }
};

// Specialized types for IR and HIR versions.
using VPPeelOrigLiveOut =
    VPOrigLiveOutImpl<VPScalarPeel, Value, VPInstruction::PeelOrigLiveOut>;
using VPRemainderOrigLiveOut =
    VPOrigLiveOutImpl<VPScalarRemainder, Value, VPInstruction::RemOrigLiveOut>;
using VPPeelOrigLiveOutHIR =
    VPOrigLiveOutImpl<VPScalarPeelHIR, loopopt::DDRef,
                      VPInstruction::PeelOrigLiveOutHIR>;
using VPRemainderOrigLiveOutHIR =
    VPOrigLiveOutImpl<VPScalarRemainderHIR, loopopt::DDRef,
                      VPInstruction::RemOrigLiveOutHIR>;

/// Instruction representing a wide VLS-optimized (Vector Load/Stores) load. It
/// takes place of several adjacent loads and substitutes several
/// non-consecutive accesses with a single wider access.
///
/// The instruction has a "uniform"/"subgroup cooperative" semantics and is
/// expected to be a low-level representation used late in the pipeline before
/// the VPlan CG.
class VPVLSLoad : public VPInstruction {
   // Logical size in Type->getElementType()
  int GroupSize;
  Align Alignment;
   // Number of original loads being optimized. For OptReport purposes.
  int NumOrigLoads;
  // Combined metadata that needs to be assigned to the wide load (e.g. TBAA,
  // alias scopes, etc.). We only preserve fixed metadata kinds, hence this
  // simpler data structure.
  SmallVector<std::pair<unsigned, MDNode *>, 3> Metadata;

public:
  /// \p Ptr should contain the base address for the wide VLSload in its 0th
  /// lane. Currently other lanes are ignored as we only support the VLS groups
  /// without gaps. Its type isn't important, it's the CG's job to insert proper
  /// bitcasts if needed (or move to opaque pointer types in future).
  ///
  /// \p Ty is a post-vectorization wide vector type. Its element type is a
  /// scalar type such that all the individual elements of the groups and offset
  /// between could be represented as a multiple of that type's width.
  ///
  /// \p GroupSize is the size of the group in terms of \p Ty's element type.
  ///
  /// \p Alignment is the alignment of the base pointer (the one in the \p Ptr's
  /// 0th lane)
  ///
  /// \p NumOrigLoads describes how many loads were substituted by this single
  /// VPVLSLoad. This parameter is needed for OptReport purposes.
  ///
  /// Note that this instruction is very low-level (i.e. the VF is implicitly
  /// encoded in the \p Ty) and is expected to be created soon before VPlan CG.
  VPVLSLoad(VPValue *Ptr, Type *Ty, int GroupSize, Align Alignment,
            int NumOrigLoads)
      : VPInstruction(VPInstruction::VLSLoad, Ty, {Ptr}), GroupSize(GroupSize),
        Alignment(Alignment), NumOrigLoads(NumOrigLoads) {}

  VPValue *getPointerOperand() const { return getOperand(0); }
  Type *getValueType() const { return getType(); }

  int getGroupSize() const { return GroupSize; }
  Align getAlignment() const { return Alignment; }
  int getNumOrigLoads() const { return NumOrigLoads; }

  void setMetadata(unsigned Kind, MDNode *MD) {
    assert(none_of(Metadata,
                   [Kind](std::pair<unsigned, MDNode *> Entry) {
                     return Entry.first == Kind;
                   }) &&
           "That kind of metadata has been set already!");
    Metadata.emplace_back(Kind, MD);
  }
  auto getMetadata() const {
    return make_range(Metadata.begin(), Metadata.end());
  }

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::VLSLoad;
  }

  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  VPVLSLoad *cloneImpl() const override {
    return new VPVLSLoad(getOperand(0), getType(), GroupSize, Alignment, NumOrigLoads);
  }
};

/// Instruction representing a wide VLS-optimized (Vector Load/Stores) store. It
/// takes place of several adjacent stores and substitutes several
/// non-consecutive accesses with a single wider access.
///
/// The instruction has a "uniform"/"subgroup cooperative" semantics and is
/// expected to be a low-level representation used late in the pipeline before
/// the VPlan CG.
class VPVLSStore : public VPInstruction {
   // Logical size in Type->getElementType()
  int GroupSize;
  Align Alignment;
   // Number of original stores being optimized. For OptReport purposes.
  int NumOrigStores;
  // Combined metadata that needs to be assigned to the wide load (e.g. TBAA,
  // alias scopes, etc.). We only preserve fixed metadata kinds, hence this
  // simpler data structure.
  SmallVector<std::pair<unsigned, MDNode *>, 3> Metadata;

public:
  /// \p Val is a specially created wide value that can be directly stored as
  /// a wide operation via this VPVLSStore similar to what VPVLSLoad produces.
  /// Its type has similar properties as well - it is a post-vectorization wide
  /// vector type. Its element type is a scalar type such that all the
  /// individual elements of the groups and offset between them could be
  /// represented as a multiple of that type's width.
  ///
  /// \p Ptr should contain the base address for the wide VLSStore in its 0th
  /// lane. Currently other lanes are ignored as we only support the VLS groups
  /// without gaps. Its type isn't important, it's the CG's job to insert proper
  /// bitcasts if needed (or move to opaque pointer types in future).
  ///
  /// \p GroupSize is the size of the group in terms of \p Ty's element type.
  ///
  /// \p Alignment is the alignment of the base pointer (the one in the \p Ptr's
  /// 0th lane)
  ///
  /// \p NumOrigLoads describes how many loads were substituted by this single
  /// VPVLSLoad. This parameter is needed for OptReport purposes.
  ///
  /// Note that this instruction is very low-level (i.e. the VF is implicitly
  /// encoded in the \p Val) and is expected to be created soon before VPlan CG.
  VPVLSStore(VPValue *Val, VPValue *Ptr, int GroupSize, Align Alignment,
             int NumOrigStores)
      : VPInstruction(VPInstruction::VLSStore,
                      Type::getVoidTy(Val->getType()->getContext()),
                      {Val, Ptr}),
        GroupSize(GroupSize), Alignment(Alignment),
        NumOrigStores(NumOrigStores) {}

  VPValue *getValueOperand() const { return getOperand(0); }
  VPValue *getPointerOperand() const { return getOperand(1); }
  Type *getValueType() const { return getValueOperand()->getType(); }

  int getGroupSize() const { return GroupSize; }
  Align getAlignment() const { return Alignment; }
  int getNumOrigStores() const { return NumOrigStores; }

  void setMetadata(unsigned Kind, MDNode *MD) {
    assert(none_of(Metadata,
                   [Kind](std::pair<unsigned, MDNode *> Entry) {
                     return Entry.first == Kind;
                   }) &&
           "That kind of metadata has been set already!");
    Metadata.emplace_back(Kind, MD);
  }
  auto getMetadata() const {
    return make_range(Metadata.begin(), Metadata.end());
  }

    /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::VLSStore;
  }

  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  VPVLSStore *cloneImpl() const override {
    return new VPVLSStore(getValueOperand(), getPointerOperand(), GroupSize,
                          Alignment, NumOrigStores);
  }
};

/// Extract data corresponding to the original non-consecutive load from the
/// wider VPVLSLoad.
class VPVLSExtract : public VPInstruction {
  // Logical size in terms Type->getElementType() elements.
  int GroupSize;
  int Offset;

public:
  /// \p WideVal - the wide value containing data from multiple original loads.
  /// Normally produced by the VPVLSLoad instruction, but that isn't enforced.
  ///
  /// \p Ty - type of the data being extracted. Must have the same element type
  /// that \p WideVal has, but will be of smaller size.
  ///
  /// \p GroupSize - Size of the VLS Group in terms of \p WideVal's element type.
  ///
  /// \p Offset of the data being extracted inside the group, in terms of \p
  /// WideVal's element type.
  VPVLSExtract(VPValue *WideVal, Type *Ty, int GroupSize, int Offset)
      : VPInstruction(VPInstruction::VLSExtract, Ty, {WideVal}),
        GroupSize(GroupSize), Offset(Offset) {
    assert(WideVal->getType()->getScalarType() == Ty->getScalarType() &&
           "Type cast must be explicit in VLS transformation!");
  }

  int getGroupSize() const { return GroupSize; }
  int getOffset() const { return Offset; }

  /// How many GroupTy's scalar elements fit into the type of the value
  /// extracted.
  unsigned getNumGroupEltsPerValue() const;

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::VLSExtract;
  }

  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  VPVLSExtract *cloneImpl() const override {
    return new VPVLSExtract(getOperand(0), getType(), GroupSize, Offset);
  }
};

/// Prepare data to perform a single VLSStore in place of multiple original
/// stores. The behavior is similar to insertelement/shufflevector but
/// interfaces are tuned for VLS purposes.
class VPVLSInsert : public VPInstruction {
  // Logical size in terms Type->getElementType() elements.
  int GroupSize;
  int Offset;

public:
  /// \p WideVal - the wide value containing data for other elements of the
  /// group. In the simplest case it's either an Undef value or the result of
  /// another VPVLSInsert instruction.
  ///
  /// \p Element - data corresponding to an element of the group to be inserted
  /// into \p WideVal. It must have the same element type as \p WideVal.
  ///
  /// \p GroupSize - Size of the VLS Group in terms of \p WideVal's element type.
  ///
  /// \p Offset of the data being inserted inside the group, in terms of \p
  /// WideVal's element type.
  VPVLSInsert(VPValue *WideVal, VPValue *Element, int GroupSize, int Offset)
      : VPInstruction(VPInstruction::VLSInsert, WideVal->getType(),
                      {WideVal, Element}),
        GroupSize(GroupSize), Offset(Offset) {
    assert(WideVal->getType()->getScalarType() ==
               Element->getType()->getScalarType() &&
           "Type cast must be explicit in VLS transformation!");
  }

  int getGroupSize() const { return GroupSize; }
  int getOffset() const { return Offset; }

  /// How many GroupTy's scalar elements fit into the type of the value being
  /// inserted.
  unsigned getNumGroupEltsPerValue() const;

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::VLSInsert;
  }

  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  VPVLSInsert *cloneImpl() const override {
    return new VPVLSInsert(getOperand(0), getOperand(1), GroupSize, Offset);
  }
};

// Represent SESE region inside VPlan.
class VPRegion final : public VPValue {
public:
  VPRegion(LLVMContext *C, const Twine &Name = "")
      : VPValue(VPValue::VPRegionSC, Type::getVoidTy(*C)), Context(C) {
    setName(Name);
  }

  auto getBBs() {
    return map_range(
        BBs, [](std::unique_ptr<VPBasicBlock> &VPBB) { return VPBB.get(); });
  }

  auto getBBs() const {
    return map_range(BBs, [](const std::unique_ptr<VPBasicBlock> &VPBB) {
      return VPBB.get();
    });
  }

  auto getLiveIns() const {
    return map_range(
        LiveIns, [](const std::unique_ptr<VPValue> &LIn) { return LIn.get(); });
  }

  auto getLiveOuts() const {
    return map_range(LiveOuts,
                     [](const std::unique_ptr<VPRegionLiveOut> &LOut) {
                       return LOut.get();
                     });
  }

  int getSize() const { return BBs.size(); }

  void
  addRgnLiveInsOuts(SmallVector<std::unique_ptr<VPValue>> RgnLiveIns,
                    SmallVector<std::unique_ptr<VPRegionLiveOut>> RgnLiveOuts) {
    LiveIns = std::move(RgnLiveIns);
    LiveOuts = std::move(RgnLiveOuts);
  }

  VPBasicBlock *addBB(const Twine &Name = "") {
    BBs.push_back(std::make_unique<VPBasicBlock>(Name, Context));
    return BBs.back().get();
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const {
    for (auto *LIn : getLiveIns())
      OS << "    live-in : " << *LIn << "\n";
    OS << "    Region:\n";
    for (auto *VPBB : getBBs()) {
      OS << "    " << VPBB->getName() << "\n";
      for (const VPInstruction &I : *VPBB)
        OS << "    " << I << "\n";
    }
    for (auto *LOut : getLiveOuts())
      OS << "    live-out : " << *LOut << "\n";
  }

  void dump() const { dump(dbgs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static inline bool classof(const VPValue *VPVal) {
    return VPVal->getVPValueID() == VPValue::VPRegionSC;
  }

  // TODO: Clone the region along with its live-ins and live-outs.
  std::unique_ptr<VPRegion> clone() const {
    llvm_unreachable("Unimplemented code");
  }

private:
  LLVMContext *Context;
  // Keeps Region's live-ins. NOTE: LiveIns must be freed after BBs.
  SmallVector<std::unique_ptr<VPValue>, 2> LiveIns;
  // Keeps Region's basic blocks.
  SmallVector<std::unique_ptr<VPBasicBlock>, 1> BBs;
  // Keeps Region's live-outs. NOTE: LiveOuts must be freed before BBs.
  SmallVector<std::unique_ptr<VPRegionLiveOut>, 1> LiveOuts;
};

// Represents optimized general conflict in VPlan.
class VPGeneralMemOptConflict final : public VPInstruction {
public:
  // For this IR in VPlan/Region, when calling with <%vp.conflict.index,
  // %vp.region, %vp.param1, %vp.param2> parameters, we'll create the following
  // dump fo VPGeneralMemOptConflict:
  // %vp.general.mem.opt.conflict = vp-general-mem-opt-conflict
  // %vp.conflict.index %vp.region %vp.conflict.load %vp.param2 ->
  // VConflictRegion (%live.in1 %live.in2) {
  //  value :
  //  mask :
  //  live-in : %live.in1
  //  live-in : %live.in2
  //  Region:
  //  VConflictBB
  //  ...
  //  live-out :
  // }
  // The first three of VPGeneralMemOptConflict are:
  // i. conflicting index
  // ii. conflict region
  // iii. conflict load
  // The rest operands are related to live-ins of VPRegion. There is an implicit
  // mapping of the operands [2:] of VPGeneralMemOptConflict and the live-ins of
  // the region.
  VPGeneralMemOptConflict(Type *BaseTy, VPValue *VConflictIndex,
                          std::unique_ptr<VPRegion> Rgn,
                          ArrayRef<VPValue *> Params)
      : VPInstruction(VPInstruction::GeneralMemOptConflict, BaseTy, {}),
        Region(std::move(Rgn)), Context(&BaseTy->getContext()) {
    // Add conflicting index as operand.
    addOperand(VConflictIndex);
    // Region has the instructions of VConflict pattern that should be included
    // in the loop that we generate for optimized general conflict.
    addOperand(Region.get());
    assert(Params.size() > 0 && isa<VPInstruction>(Params[0]) &&
           cast<VPInstruction>(Params[0])->getOpcode() == Instruction::Load &&
           "VConflictLoad is expected to be the third operand.");
    for (auto *P : Params)
      // There is an implicit mapping between the operands [2:] of
      // VPGeneralMemOptConflict and the live-ins of the region.
      addOperand(P);
  }

  VPValue *getConflictIndex() const { return getOperand(0); }

  VPRegion *getRegion() const { return Region.get(); }

  VPValue *getConflictLoad() const { return getOperand(2); }

  // Returns VPGeneralMemOptConflict live-ins.
  inline decltype(auto) getLiveIns() const {
    return make_range(op_begin() + 3, op_end());
  }

  // Methods for supporting type inquiry through isa, cast and dyn_cast:
  static inline bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::GeneralMemOptConflict;
  }

  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  VPGeneralMemOptConflict *cloneImpl() const override {
    return new VPGeneralMemOptConflict(
        getType(), getConflictIndex(), Region->clone(),
        ArrayRef<VPValue *>(op_begin() + 2, op_end()));
  }

private:
  std::unique_ptr<VPRegion> Region;
  LLVMContext *Context;
};

// Represents simple tree-conflict idioms which contain a single reduction
// operation in the conflicting region. It tracks 3 operands -
// i. conflicting index
// ii. conflict load
// iii. value used to update reduction (will be region live-in)
//
// If the live-in value is uniform, then this tree-conflict idiom can be
// optimized as Histogram.
class VPTreeConflict final : public VPInstruction {
public:
  VPTreeConflict(VPValue *ConflictIdx, VPValue *ConflictLd,
                 VPValue *RednUpdateOp, unsigned RednOpcode)
      : VPInstruction(VPInstruction::TreeConflict, RednUpdateOp->getType(),
                      {ConflictIdx, ConflictLd, RednUpdateOp}),
        RednOpcode(RednOpcode) {
    assert(RednUpdateOp->getType() == ConflictLd->getType() &&
           "Mismatch in type for tree-conflict operands.");
    assert(isSupportedRednOpcode(RednOpcode) &&
           "Unsupported redution opcode for tree-conflict.");
  }

  VPValue *getConflictIndex() const { return getOperand(0); }

  VPValue *getConflictLoad() const { return getOperand(1); }

  VPValue *getRednUpdateOp() const { return getOperand(2); }

  unsigned getRednOpcode() const { return RednOpcode; }

  static bool isSupportedRednOpcode(unsigned Opcode) {
    switch (Opcode) {
    case Instruction::Add:
    case Instruction::FAdd:
    case Instruction::Sub:
    case Instruction::FSub:
      return true;
    default:
      return false;
    }
  }

  // Methods for supporting type inquiry through isa, cast and dyn_cast:
  static inline bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::TreeConflict;
  }

  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  VPTreeConflict *cloneImpl() const override {
    return new VPTreeConflict(getConflictIndex(), getConflictLoad(),
                              getRednUpdateOp(), getRednOpcode());
  }

private:
  unsigned RednOpcode;
};

class VPConflictInsn final : public VPInstruction {
public:
  VPConflictInsn(Type *BaseTy, VPInstruction *LoadIndex)
      : VPInstruction(VPInstruction::ConflictInsn, BaseTy, {}) {
    assert(LoadIndex->getType()->isIntegerTy() &&
           "Only integers are expected.");
    addOperand(LoadIndex);
  }

  // Utility to obtain the LLVM X86 conflict intrinsic that this VPInstruction
  // will be lowered to. Returns Intrinsic::not_intrinsic if input size is
  // unexpected.
  Intrinsic::ID getConflictIntrinsic(unsigned VF, unsigned TypeSize) const {
    unsigned InputSize = TypeSize * VF;
    if (TypeSize == 32) {
      switch (InputSize) {
      case 128:
        return Intrinsic::x86_avx512_conflict_d_128;
      case 256:
        return Intrinsic::x86_avx512_conflict_d_256;
      case 512:
        return Intrinsic::x86_avx512_conflict_d_512;
      default:
        // Unexpected input size for conflict intrinsic
        return Intrinsic::not_intrinsic;
      }
    } else {
      assert(TypeSize == 64 && "Unexpected type size for load index.");
      switch (InputSize) {
      case 128:
        return Intrinsic::x86_avx512_conflict_q_128;
      case 256:
        return Intrinsic::x86_avx512_conflict_q_256;
      case 512:
        return Intrinsic::x86_avx512_conflict_q_512;
      default:
        // Unexpected input size for conflict intrinsic
        return Intrinsic::not_intrinsic;
      }
    }
  }

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static inline bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::ConflictInsn;
  }

  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  VPConflictInsn *cloneImpl() const override {
    return new VPConflictInsn(getType(), cast<VPInstruction>(getOperand(0)));
  }
};

// VPInstruction that serves as a placeholder for permute intrinsics. So far,
// these instructions are used for tree conflict lowering.
class VPPermute final : public VPInstruction {
// Permute BaseTy elements in PermuteVals using Control.
public:
  VPPermute(Type *BaseTy, VPValue *PermuteVals, VPValue *Control)
      : VPInstruction(VPInstruction::Permute, BaseTy, {PermuteVals, Control}) {
    assert(PermuteVals->getType()->getPrimitiveSizeInBits() ==
           Control->getType()->getPrimitiveSizeInBits() &&
           "Type size of PermuteVals should match that of Control");
  }

  Intrinsic::ID getPermuteIntrinsic(unsigned VF) const {
    Type *Ty = getType();
    if (Ty->isDoubleTy() && VF == 4)
      return Intrinsic::x86_avx512_permvar_df_256;
    if (Ty->isDoubleTy() && VF == 8)
      return Intrinsic::x86_avx512_permvar_df_512;

    if (Ty->isFloatTy() && VF == 4)
      return Intrinsic::x86_avx_vpermilvar_ps;
    if (Ty->isFloatTy() && VF == 8)
      return Intrinsic::x86_avx2_permps;
    if (Ty->isFloatTy() && VF == 16)
      return Intrinsic::x86_avx512_permvar_sf_512;

    if (Ty->isIntegerTy(32) && VF == 4)
      return Intrinsic::x86_avx_vpermilvar_ps;
    if (Ty->isIntegerTy(32) && VF == 8)
      return Intrinsic::x86_avx2_permd;
    if (Ty->isIntegerTy(32) && VF == 16)
      return Intrinsic::x86_avx512_permvar_si_512;

    if (Ty->isIntegerTy(64) && VF == 4)
      return Intrinsic::x86_avx512_permvar_di_256;
    if (Ty->isIntegerTy(64) && VF == 8)
      return Intrinsic::x86_avx512_permvar_di_512;

// TODO: support yet to come for i8/i16 intrinsics, but we've already
//       provisioned for them here.
    if (Ty->isIntegerTy(16) && VF == 8)
      return Intrinsic::x86_avx512_permvar_hi_128;
    if (Ty->isIntegerTy(16) && VF == 16)
      return Intrinsic::x86_avx512_permvar_hi_256;
    if (Ty->isIntegerTy(16) && VF == 32)
      return Intrinsic::x86_avx512_permvar_hi_512;

    if (Ty->isIntegerTy(8) && VF == 16)
      return Intrinsic::x86_avx512_permvar_qi_128;
    if (Ty->isIntegerTy(8) && VF == 32)
      return Intrinsic::x86_avx512_permvar_qi_256;
    if (Ty->isIntegerTy(8) && VF == 64)
      return Intrinsic::x86_avx512_permvar_qi_512;

    return Intrinsic::not_intrinsic;
  }

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static inline bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::Permute;
  }

  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  VPPermute *cloneImpl() const override {
    return new VPPermute(getType(), getOperand(0), getOperand(1));
  }
};

// Represents operation to convert mask to target IntergerTy. It effectively
// represents a bitcast + zext, for example -
//
// i64 %cvt = convert-mask-to-int i1 %mask
//
// is lowered for VF=2 as -
//
// %bc = bitcast <2 x i1> %vec.mask to i2
// %cvt = zext i2 %bc to i64
class VPConvertMaskToInt final : public VPInstruction {
public:
  VPConvertMaskToInt(Type *TargetTy, VPValue *Mask)
      : VPInstruction(VPInstruction::CvtMaskToInt, TargetTy, {Mask}) {
    assert(Mask->getType()->isIntegerTy(1 /*BitWidth*/) &&
           "Mask operand expected to be i1 type.");
  }

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static inline bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::CvtMaskToInt;
  }

  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

  VPConvertMaskToInt *cloneImpl() const override {
    return new VPConvertMaskToInt(getType(), getOperand(0));
  }
};

/// Transformed library call specialization.
/// This represents a library call that has been transformed in some way. By that
/// we mean it has:
///   1. An input signature (the original function call)
///   2. An output signature (the transformed function call)
///
/// Take `sincos` for example:
///
///   void sincos(float, float*, float*)
///   =>
///   {float, float} __svml_sincos(float)
///
class VPTransformLibraryCall final : public VPCallInstruction {
public:
  VPTransformLibraryCall(VPCallInstruction &OrigCall, FunctionType *FnTy,
                         ArrayRef<VPValue *> ArgList)
      : VPCallInstruction(VPInstruction::TransformLibraryCall,
                          OrigCall.getCalledValue(), FnTy, ArgList,
                          OrigCall.getOriginalCall()) {
    assert(OrigCall.getVectorizationScenario() ==
               CallVecScenariosTy::LibraryFunc &&
           "Call being transformed must have library vectorization scenario!");
    assert(FnTy->getNumParams() == ArgList.size() &&
           "Transformed function type and arg list must match!");
    setVectorizeWithLibraryFn(OrigCall.getVectorLibraryFunc(),
                              OrigCall.getPumpFactor());
    copyUnderlyingFrom(OrigCall);
  }

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static inline bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::TransformLibraryCall;
  }

  static inline bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }
};

/// VPlan models a candidate for vectorization, encoding various decisions take
/// to produce efficient output IR, including which branches, basic-blocks and
/// output IR instructions to generate, and their cost.
class VPlan {
  friend class VPlanPrinter;
  friend class VPLiveInOutCreator;

public:

  using VPBasicBlockListTy = ilist<VPBasicBlock, ilist_sentinel_tracking<true>>;
  // VPBasicBlock iterators.
  using iterator = VPBasicBlockListTy::iterator;
  using const_iterator = VPBasicBlockListTy::const_iterator;
  using reverse_iterator = VPBasicBlockListTy::reverse_iterator;
  using const_reverse_iterator = VPBasicBlockListTy::const_reverse_iterator;

protected:
  // Enum to represent the Kind of VPlan.
  enum class VPlanKind {
    ScalarPeel,
    ScalarRemainder,
    Masked,
    NonMasked,
  };

  /// Holds Plan's VPBasicBlocks.
  VPBasicBlockListTy VPBasicBlocks;

  /// Holds the VPLiveInValues.
  SmallVector<std::unique_ptr<VPLiveInValue>, 16> LiveInValues;

  /// Holds the VPLiveOutValues.
  SmallVector<std::unique_ptr<VPLiveOutValue>, 16> LiveOutValues;

  std::unique_ptr<VPlanDivergenceAnalysisBase> VPlanDA;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printLiveIns(raw_ostream &OS) const;
  void printLiveOuts(raw_ostream &OS) const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  VPlan(VPlanKind K, VPExternalValues &E, VPUnlinkedInstructions &UVPI)
      : Kind(K), Externals(E), UnlinkedVPInsts(UVPI) {}

public:

  virtual ~VPlan();

  VPlanKind getVPlanKind() const { return Kind; }

  VPlanDivergenceAnalysisBase *getVPlanDA() const { return VPlanDA.get(); }

  VPExternalValues &getExternals() { return Externals; }
  const VPExternalValues &getExternals() const { return Externals; }
  VPUnlinkedInstructions &getUnlinkedVPInsts() { return UnlinkedVPInsts; }
  const VPUnlinkedInstructions &getUnlinkedVPInsts() const {
    return UnlinkedVPInsts;
  }

  LLVMContext *getLLVMContext(void) const { return Externals.getLLVMContext(); }

  const DataLayout *getDataLayout() const { return Externals.getDataLayout(); }

  const VPLiveInValue *getLiveInValue(unsigned MergeId) const {
    return LiveInValues[MergeId].get();
  }

  auto liveInValues() {
    return map_range(LiveInValues, [](std::unique_ptr<VPLiveInValue> &V) {
      return V.get();});
  }
  auto liveInValues() const {
    return map_range(LiveInValues, [](const std::unique_ptr<VPLiveInValue> &V) {
      return V.get();});
  }

  VPLiveOutValue *getLiveOutValue(unsigned MergeId) const {
    return LiveOutValues[MergeId].get();
  }

  auto liveOutValues() {
    return map_range(LiveOutValues, [](std::unique_ptr<VPLiveOutValue> &V) {
      return V.get();});
  }

  auto liveOutValues() const {
    return map_range(LiveOutValues, [](const std::unique_ptr<VPLiveOutValue> &V) {
      return V.get();});
  }

  size_t getLiveInValuesSize() const { return LiveInValues.size(); }

  size_t getLiveOutValuesSize() const { return LiveOutValues.size(); }

  bool hasExplicitRemainder() const { return ExplicitRemainderUsed; }
  void setExplicitRemainderUsed() { ExplicitRemainderUsed = true; }

  // VPBasicBlock iterator forwarding functions
  iterator begin() { return VPBasicBlocks.begin(); }
  const_iterator begin() const { return VPBasicBlocks.begin(); }
  iterator end() { return VPBasicBlocks.end(); }
  const_iterator end() const { return VPBasicBlocks.end(); }

  reverse_iterator rbegin() { return VPBasicBlocks.rbegin(); }
  const_reverse_iterator rbegin() const { return VPBasicBlocks.rbegin(); }
  reverse_iterator rend() { return VPBasicBlocks.rend(); }
  const_reverse_iterator rend() const { return VPBasicBlocks.rend(); }

  size_t size() const { return VPBasicBlocks.size(); }
  bool empty() const { return VPBasicBlocks.empty(); }
  const VPBasicBlock &front() const { return VPBasicBlocks.front(); }
  VPBasicBlock &front() { return VPBasicBlocks.front(); }
  const VPBasicBlock &back() const { return VPBasicBlocks.back(); }
  VPBasicBlock &back() { return VPBasicBlocks.back(); }

  VPBasicBlock &getEntryBlock() {
    assert(front().getNumPredecessors() == 0 &&
           "Entry block should not have predecesors.");
    return front();
  }
  const VPBasicBlock &getEntryBlock() const {
    assert(front().getNumPredecessors() == 0 &&
           "Entry block should not have predecesors.");
    return front();
  }

  /// Return the last VPBasicBlock in VPlan, i.e. the one with no successors.
  const_iterator getExitBlock() const {
    return find_if(*this, [](const VPBasicBlock &BB) {
      return BB.getNumSuccessors() == 0;
    });
  }
  iterator getExitBlock() {
    return find_if(*this, [](const VPBasicBlock &BB) {
      return BB.getNumSuccessors() == 0;
    });
  }

  const VPBasicBlockListTy &getBasicBlockList() const {
    return VPBasicBlocks;
  }
  VPBasicBlockListTy &getBasicBlockList() { return VPBasicBlocks; }

  void insertAtFront(VPBasicBlock *CurBB) {
    getBasicBlockList().push_front(CurBB);
  }

  void insertBefore(VPBasicBlock *CurBB, VPBasicBlock *MovePos) {
    getBasicBlockList().insert(MovePos->getIterator(), CurBB);
  }

  void insertAfter(VPBasicBlock *CurBB, VPBasicBlock *MovePos) {
    getBasicBlockList().insertAfter(MovePos->getIterator(), CurBB);
  }

  void insertAtBack(VPBasicBlock *CurBB) {
    getBasicBlockList().push_back(CurBB);
  }

  void insertBefore(VPBasicBlock *CurBB, iterator InsertBefore) {
    getBasicBlockList().insert(InsertBefore, CurBB);
  }

  /// Returns a pointer to a member of VPBasicBlock list.
  static VPBasicBlockListTy VPlan::*getSublistAccess(VPBasicBlock *) {
    return &VPlan::VPBasicBlocks;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Print (in text format) VPlan blocks in order based on dominator tree.
  void dump(raw_ostream &OS) const;
  void dump() const;
  void print(raw_ostream &OS, unsigned Indent) const;
  virtual void printSpecifics(raw_ostream &OS) const = 0;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  const std::string &getName() const { return Name; }

  void setName(const Twine &newName) { Name = newName.str(); }

  /// Add unlinked VPInstructions.
  void addUnlinkedVPInst(VPInstruction *I) {
    UnlinkedVPInsts.addUnlinkedVPInst(I);
  }

  /// Create a new VPConstant for \p Const if it doesn't exist or retrieve the
  /// existing one.
  VPConstant *getVPConstant(Constant *Const) {
    return Externals.getVPConstant(Const);
  }

  VPConstant *getVPConstant(const APInt &V) {
    ConstantInt *C = ConstantInt::get(*getLLVMContext(), V);
    return getVPConstant(C);
  }

  VPConstant *getUndef(Type *Ty) {
    return getVPConstant(UndefValue::get(Ty));
  }

  /// Create or retrieve a VPExternalDef for a given Value \p ExtVal.
  VPExternalDef *getVPExternalDef(Value *ExtDef) {
    return Externals.getVPExternalDef(ExtDef);
  }

  /// Create or retrieve a VPExternalDef for a given non-decomposable DDRef \p
  /// DDR.
  VPExternalDef *getVPExternalDefForDDRef(const loopopt::DDRef *DDR) {
    return Externals.getVPExternalDefForDDRef(DDR);
  }

  /// Create or retrieve a VPExternalDef for the blob with index \p BlobIndex in
  /// \p DDR.
  VPExternalDef *getVPExternalDefForBlob(const loopopt::RegDDRef *DDR,
                                         unsigned BlobIndex) {
    return Externals.getVPExternalDefForBlob(DDR, BlobIndex);
  }

  /// Create or retrieve a VPExternalDef for the given canon expression \p CE.
  VPExternalDef *getVPExternalDefForCanonExpr(const loopopt::CanonExpr *CE,
                                              const loopopt::RegDDRef *DDR) {
    return Externals.getVPExternalDefForCanonExpr(CE, DDR);
  }

  /// Retrieve the VPExternalDef for given HIR symbase \p Symbase. If no
  /// external definition exists then a nullptr is returned.
  VPExternalDef *getVPExternalDefForSymbase(unsigned Symbase) {
    return Externals.getVPExternalDefForSymbase(Symbase);
  }

  /// Create or retrieve a VPExternalDef for an HIR IV identified by its \p
  /// IVLevel.
  VPExternalDef *getVPExternalDefForIV(unsigned IVLevel, Type *BaseTy) {
    return Externals.getVPExternalDefForIV(IVLevel, BaseTy);
  }

  VPExternalDef *getVPExternalDefForIfCond(const loopopt::HLIf *If) {
    return Externals.getVPExternalDefForIfCond(If);
  }

  /// Create a new VPMetadataAsValue for \p MDAsValue if it doesn't exist or
  /// retrieve the existing one.
  VPMetadataAsValue *getVPMetadataAsValue(MetadataAsValue *MDAsValue) {
    return Externals.getVPMetadataAsValue(MDAsValue);
  }

  /// Create a new VPMetadataAsValue for Metadata \p MD if it doesn't exist or
  /// retrieve the existing one.
  VPMetadataAsValue *getVPMetadataAsValue(Metadata *MD) {
    return Externals.getVPMetadataAsValue(MD);
  }

  /// Create a new OptReportStatsTracker for VPLoop \p VLp if it doesn't exist
  /// or retrieve the existing one.
  OptReportStatsTracker &getOptRptStatsForLoop(const VPLoop *VLp) const {
    return Externals.getOrCreateOptRptStatsForLoop(VLp);
  }

  /// Clone live-in values from OrigVPlan and add them in LiveInValues.
  void cloneLiveInValues(const VPlan &OrigPlan, VPValueMapper &Mapper);

  /// Clone live-out values from OrigVPlan and add them in LiveOutValues.
  void cloneLiveOutValues(const VPlan &OrigPlan, VPValueMapper &Mapper);

  /// Return true if adding instructions at specified loop depth starting
  /// from topmost loop being vectorized will cause us to exceed maximum
  /// allowed loop nesting level.
  bool exceedsMaxLoopNestingLevel(unsigned InstDepth) {
    // This check is only relevant for HIR path where we have a limit
    // on maximum allowed loop nesting. For LLVM IR case, OrigLoopNestingLevel
    // remains set to 0.
    if (!OrigLoopNestingLevel)
      return false;

    return OrigLoopNestingLevel + InstDepth > loopopt::MaxLoopNestLevel;
  }

  void setOrigLoopNestingLevel(unsigned Level) { OrigLoopNestingLevel = Level; }

  unsigned getOrigLoopNestingLevel() const { return OrigLoopNestingLevel; }

  void setPrintingEnabled(bool V) { PrintingEnabled = V;}
  bool isPrintingEnabled() const { return PrintingEnabled;}

private:
  void addLiveInValue(VPLiveInValue *V) {
    assert(V->getMergeId() == LiveInValues.size() &&
           "Inconsistent livein index");
    LiveInValues.emplace_back(V);
  }

  void setLiveInValue(VPLiveInValue *V, unsigned MergeId) {
    assert((V->getMergeId() == MergeId && !LiveInValues[MergeId]) &&
           "Inconsistent livein index");
    LiveInValues[MergeId].reset(V);
  }

  void allocateLiveInValues(int Count) {
    assert(LiveInValues.size() == 0 && "The list is not empty");
    LiveInValues.resize(Count);
  }

  void addLiveOutValue(VPLiveOutValue *V) {
    assert(V->getMergeId() == LiveOutValues.size() &&
           "Inconsistent liveout index");
    LiveOutValues.emplace_back(V);
  }

  void setLiveOutValue(VPLiveOutValue *V, unsigned MergeId) {
    assert((V->getMergeId() == MergeId && !LiveOutValues[MergeId]) &&
           "Inconsistent liveout index");
    LiveOutValues[MergeId].reset(V);
  }

  void allocateLiveOutValues(int Count) {
    assert(LiveOutValues.size() == 0 && "The list is not empty");
    LiveOutValues.resize(Count);
  }

private:
  VPlanKind Kind;
  VPExternalValues &Externals;
  VPUnlinkedInstructions &UnlinkedVPInsts;

  /// Holds the name of the VPlan, for printing.
  std::string Name;

  /// Flag showing that a new scheme of CG for loops and basic blocks
  /// should be used.
  bool ExplicitRemainderUsed = false;

  /// Set to false when printing is not enabled, e.g. by -filter-print-funcs.
  bool PrintingEnabled = true;

  /// Nesting level of outermost loop being vectorized. VPlan transformations
  /// may generated additional loops and we cannot exceed the maximum
  /// nesting level allowed for HIR. Currently this field is only used in the
  /// HIR path to check such optimizations(TreeConflict lowering for now).
  unsigned OrigLoopNestingLevel = 0;
};

/// Class to represent VPlan for scalar-loops.
// Currently there are no VPlan-VPlan analyses/transforms planned for
// scalar-loops, but this might change in the future.
class VPlanScalar : public VPlan {

public:
  /// Methods for supporting type inquiry through isa, cast, and
  /// dyn_cast:
  static bool classof(const VPlan *V) {
    return V->getVPlanKind() == VPlanKind::ScalarPeel ||
           V->getVPlanKind() == VPlanKind::ScalarRemainder;
  }

  // Set the base-class VPlanDA with input scalar DA-type object.
  void setVPlanDA(std::unique_ptr<VPlanDivergenceAnalysisScalar> VPDA) {
    VPlanDA = std::move(VPDA);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Print Scalar VPlan specific information. Currently, we do not print
  // anything specific for scalar VPlans.
  virtual void printSpecifics(raw_ostream &OS) const override {}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  void setNeedCloneOrigLoop(bool V);
  bool getNeedCloneOrigLoop() const { return NeedCloneOrigLoop; }

  /// Utility to retrieve VPInstruction that represents the scalar loop in this
  /// scalar VPlan.
  VPInstruction *getScalarLoopInst();

protected:
  VPlanScalar(VPlanKind K, VPExternalValues &E, VPUnlinkedInstructions &UVPI)
      : VPlan(K, E, UVPI) {}

  bool NeedCloneOrigLoop = false;
};


/// Class to represent VPlan for Vector-loops. This class holds pointers and
/// data specific to analyses required for vectorization and has nothing
/// specific for masked/non-masked vector loops.
class VPlanVector : public VPlan {

public:
  // When we clone the plan, we can choose if we want to calculate DA from
  // scratch or clone DA or none of them. If the plan is cloned after the
  // predicator, then we just have to clone instructions' vector shapes.
  enum class UpdateDA : uint8_t {
    RecalculateDA, // Compute DA from scratch.
    CloneDA,       // Clone DA of the original plan to the new plan.
    DoNotUpdateDA  // Do not set DA.
  };

  // TODO: Make this pure virtual.
  void computeDA();

  VPlanDivergenceAnalysis *getVPlanDA() const {
    return cast<VPlanDivergenceAnalysis>(VPlanDA.get());
  }

  /// Methods for supporting type inquiry through isa, cast, and
  /// dyn_cast:
  static bool classof(const VPlan *V) {
    return V->getVPlanKind() == VPlanKind::Masked ||
           V->getVPlanKind() == VPlanKind::NonMasked;
  }

  /// Generate the LLVM IR code for this VPlan.
  void execute(struct VPTransformState *State);

#if INTEL_CUSTOMIZATION
  void executeHIR(VPOCodeGenHIR *CG);
#endif // INTEL_CUSTOMIZATION

  VPLoopInfo *getVPLoopInfo() { return VPLInfo.get(); }

  const VPLoopInfo *getVPLoopInfo() const { return VPLInfo.get(); }

  // Return main loop, making the sanity check for that we have
  // the only one top loop in VPLoopInfo. If the \p StrictCheck is true than the
  // check is performed unconditionally. Otherwise it allows multiple top loops
  // when hasExplicitRemainder() is true.
  VPLoop *getMainLoop(bool StrictCheck) {
    assert(((!StrictCheck && hasExplicitRemainder()) ||
            std::distance(VPLInfo->begin(), VPLInfo->end()) == 1) &&
           "Expected single outermost loop!");
    return *VPLInfo->begin();
  }

  const VPLoop *getMainLoop(bool StrictCheck) const {
    return const_cast<VPlanVector *>(this)->getMainLoop(StrictCheck);
  }

  VPlanScalarEvolution *getVPSE() const {
    return VPSE.get();
  }

  VPlanValueTracking *getVPVT() const { return VPVT.get(); }

  void setVPLoopInfo(std::unique_ptr<VPLoopInfo> VPLI) {
    VPLInfo = std::move(VPLI);
  }

  // Set the base-class VPlanDA with input vector DA-type object.
  void setVPlanDA(std::unique_ptr<VPlanDivergenceAnalysis> VPDA) {
    VPlanDA = std::move(VPDA);
  }

  void setVPSE(std::unique_ptr<VPlanScalarEvolution> A);

  void setVPVT(std::unique_ptr<VPlanValueTracking> A) {
    VPVT = std::move(A);
  }

  void setVPlanSVA(std::unique_ptr<VPlanScalVecAnalysisBase> VPSVA) {
    VPlanSVA = std::move(VPSVA);
  }

  VPlanScalVecAnalysisBase *getVPlanSVA() const { return VPlanSVA.get(); }

  // Compute SVA results for this VPlan for given VF.
  void runSVA(unsigned VF);

  // Clear results of SVA.
  void clearSVA();

  void markFullLinearizationForced() { FullLinearizationForced = true; }
  bool isFullLinearizationForced() const { return FullLinearizationForced; }

  void markBackedgeUniformityForced() {
    ForceOuterLoopBackedgeUniformity = true;
  }

  bool isBackedgeUniformityForced() const {
    return ForceOuterLoopBackedgeUniformity;
  }

  void disableActiveLaneInstructions() { DisableActiveLaneInstructions = true; }

  bool areActiveLaneInstructionsDisabled() {
    return DisableActiveLaneInstructions;
  }

  /// Disable SOA-analysis.
  void disableSOAAnalysis() { EnableSOAAnalysis = false; }

  /// Enable SOA-analysis.
  void enableSOAAnalysis() { EnableSOAAnalysis = true; }

  /// Getters for Dominator Tree
  VPDominatorTree *getDT() { return PlanDT.get(); }
  const VPDominatorTree *getDT() const { return PlanDT.get(); }
  /// Getter for Post-Dominator Tree
  VPPostDominatorTree *getPDT() { return PlanPDT.get(); }
  const VPPostDominatorTree *getPDT() const { return PlanPDT.get(); }

  /// Compute the Dominator Tree for this Plan.
  void computeDT();

  /// Compute the Post-Dominator Tree for this Plan.
  void computePDT();

  /// Return \true if SOA-analysis is enabled.
  bool isSOAAnalysisEnabled() const { return EnableSOAAnalysis; }

  /// Return an existing or newly created LoopEntities for the loop \p L.
  VPLoopEntityList *getOrCreateLoopEntities(const VPLoop *L) {
    // Sanity check
    VPBasicBlock *HeaderBB = L->getHeader(); (void)HeaderBB;
    assert(VPLInfo->getLoopFor(HeaderBB) == L &&
           "the loop does not exist in VPlan");

    std::unique_ptr<VPLoopEntityList> &Ptr = LoopEntities[L];
    if (!Ptr) {
      VPLoopEntityList *E =
          new VPLoopEntityList(*this, *(const_cast<VPLoop *>(L)));
      Ptr.reset(E);
    }
    return Ptr.get();
  }

  /// Return LoopEntities list for the loop \p L. The nullptr is returned if
  /// the descriptors were not created for the loop.
  const VPLoopEntityList *getLoopEntities(const VPLoop *L) const {
    auto Iter = LoopEntities.find(L);
    if (Iter == LoopEntities.end())
      return nullptr;
    return Iter->second.get();
  }

  /// Utility to invalidate results of analyses specified by \p Analyses.
  void invalidateAnalyses(ArrayRef<VPAnalysisID> Analyses);

  /// Utility to run/recompute results of analyses specified by \p Analyses.
  // TODO : Implementation is missing.
  void requiredAnalyses(ArrayRef<VPAnalysisID> Analyses);

  /// Add to the given dominator tree the header block and every new basic block
  /// that was created between it and the latch block, inclusive.
  static void updateDominatorTree(class DominatorTree *DT,
                                  BasicBlock *LoopPreHeaderBB,
                                  BasicBlock *LoopLatchBB);

  /// Utility method to clone VPlan. VPAnalysesFactoryBase has methods to
  /// create additional analyses required for cloned VPlan.
  virtual VPlanVector *clone(VPAnalysesFactoryBase &VPAF, UpdateDA UDA) = 0;

  void setPreferredPeeling(unsigned VF,
                           std::unique_ptr<VPlanPeelingVariant> Peeling) {
    PreferredPeelingMap[VF] = std::move(Peeling);
  }

  /// Returns preferred peeling or nullptr.
  VPlanPeelingVariant *getPreferredPeeling(unsigned VF) const {
    auto Iter = PreferredPeelingMap.find(VF);
    if (Iter == PreferredPeelingMap.end())
      return nullptr;
    return Iter->second.get();
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printVectorVPlanData() const;
  /// Print Vector VPlan specific information. Currently, this covers
  /// Loop-entities information.
  virtual void printSpecifics(raw_ostream &OS) const override;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
  /// Dominator Tree for the Plan.
  std::unique_ptr<VPDominatorTree> PlanDT;

  /// Post-Dominator Tree for the Plan.
  std::unique_ptr<VPPostDominatorTree> PlanPDT;

  // We need to force full linearization for certain cases. Currently this
  // happens for cases where while-loop canonicalization or merge loop exits
  // transformation break SSA or for HIR vector code generation which needs
  // to be extended to preserve uniform control flow. This flag is set to true
  // when we need to force full linearization. Full linearization can still
  // kick in when this flag is false such as cases where we use a command
  // line option to do the same.
  bool FullLinearizationForced = false;

  // HIR isn't uplifted for explict vector loop IV - need DA to treat backedge
  // condition as uniform.
  bool ForceOuterLoopBackedgeUniformity = false;

  // HIR CG handles very limited scalar compute and tends to keep most of things
  // on vectors. As such, the stability issue addressed by
  // VPActiveLane/VPActiveLaneExtract doesn't seem to exist for HIR case. Also,
  // implementing the proper CG for them
  //   1) Doesn't seem to be needed right now - we have some time until we'll
  //      implement a better approach to the problem.
  //   2) Might not be easy/possible because the support for the scalar compute
  //      itself is very weak in HIR CG.
  bool DisableActiveLaneInstructions = false;

  /// Enable SOA-analysis flag.
  bool EnableSOAAnalysis = false;

  std::unique_ptr<VPLoopInfo> VPLInfo;
  std::unique_ptr<VPlanScalarEvolution> VPSE;
  std::unique_ptr<VPlanValueTracking> VPVT;
  std::unique_ptr<VPlanScalVecAnalysisBase> VPlanSVA;

  DenseMap<const VPLoop *, std::unique_ptr<VPLoopEntityList>> LoopEntities;

  /// Map: VF -> PreferredPeeling.
  std::map<unsigned, std::unique_ptr<VPlanPeelingVariant>> PreferredPeelingMap;

protected:
  VPlanVector(VPlanKind K, VPExternalValues &E, VPUnlinkedInstructions &UVPI);

  // Helper method used by cloning functionality to populate data in the new
  // VPlan.
  void copyData(VPAnalysesFactoryBase &VPAF, UpdateDA UDA,
                VPlanVector *TargetPlan);
};

/// Class to represent VPlan for scalar-peel loops.
class VPlanScalarPeel : public VPlanScalar {
public:
  VPlanScalarPeel(VPExternalValues &E, VPUnlinkedInstructions &UVPI)
      : VPlanScalar(VPlanKind::ScalarPeel, E, UVPI) {}

  /// Methods for supporting type inquiry through isa, cast, and
  /// dyn_cast:
  static bool classof(const VPlan *V) {
    return V->getVPlanKind() == VPlanKind::ScalarPeel;
  }
};

/// Class to represent VPlan for scalar-remainder loops.
class VPlanScalarRemainder : public VPlanScalar {
public:
  VPlanScalarRemainder(VPExternalValues &E, VPUnlinkedInstructions &UVPI)
      : VPlanScalar(VPlanKind::ScalarRemainder, E, UVPI) {}

  /// Methods for supporting type inquiry through isa, cast, and
  /// dyn_cast:
  static bool classof(const VPlan *V) {
    return V->getVPlanKind() == VPlanKind::ScalarRemainder;
  }
};

/// Class to represent VPlan for masked loop.
// This is a vector-type VPlan as we would want to, in some cases, execute the
// main vector-loop in masked mode. E.g., when we statically know the number of
// iterations and it is less than the VF we decide to vectorize with.
class VPlanMasked : public VPlanVector {

public:
  VPlanMasked(VPExternalValues &E, VPUnlinkedInstructions &UVPI);

  /// Utility method to clone Non-Masked-vplan. VPAnalysesFactoryBase has
  /// methods to create additional analyses required for cloned VPlan.
  VPlanVector *clone(VPAnalysesFactoryBase &VPAF, UpdateDA UDA) override;

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VPlan *V) {
    return V->getVPlanKind() == VPlanKind::Masked;
  }
};

/// Class to represent VPlan for non-masked loop.
class VPlanNonMasked : public VPlanVector {
public:
  VPlanNonMasked(VPExternalValues &E, VPUnlinkedInstructions &UVPI);

  /// Utility method to clone masked-vplan. VPAnalysesFactoryBase has methods to
  /// create additional analyses required for cloned VPlan.
  VPlanVector *clone(VPAnalysesFactoryBase &VPAF, UpdateDA UDA) override;

  /// Utility method to allow a Non-masked VPlan to clone a masked VPlan.
  VPlanMasked *cloneMasked(VPAnalysesFactoryBase &VPAF, UpdateDA UDA);

  /// Methods for supporting type inquiry through isa, cast, and
  /// dyn_cast:
  static bool classof(const VPlan *V) {
    return V->getVPlanKind() == VPlanKind::NonMasked;
  }
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// VPlanPrinter prints a given VPlan to a given output stream. The printing is
/// indented and follows the dot format.
class VPlanPrinter {
  raw_ostream &OS;
  const VPlan &Plan;
  unsigned Depth;
  unsigned TabWidth = 2;
  std::string Indent;

  unsigned BID = 0;

  SmallDenseMap<const VPBasicBlock *, unsigned> BlockID;

  /// Handle indentation.
  void bumpIndent(int b) { Indent = std::string((Depth += b) * TabWidth, ' '); }

  /// Print the information related to the CFG edges going out of a given
  /// \p Block, followed by printing the successor blocks themselves.
  void dumpEdges(const VPBasicBlock *BB);

  /// Print a given \p BasicBlock, including its instructions, followed by
  /// printing its successor blocks.
  void dumpBasicBlock(const VPBasicBlock *BB, bool SkipInstructions);

  unsigned getOrCreateBID(const VPBasicBlock *BB) {
    return BlockID.count(BB) ? BlockID[BB] : BlockID[BB] = BID++;
  }

  const Twine getOrCreateName(const VPBasicBlock *BB);

  const Twine getUID(const VPBasicBlock *BB);

  /// Print the information related to a CFG edge between two VPBasicBlocks.
  void drawEdge(const VPBasicBlock *From, const VPBasicBlock *To, bool Hidden,
                const Twine &Label);

public:
  VPlanPrinter(raw_ostream &O, const VPlan &P) : OS(O), Plan(P) {}
  void dump(bool CFGOnly = false);
};

inline raw_ostream &operator<<(raw_ostream &OS, const VPlan &Plan) {
  VPlanPrinter Printer(OS, Plan);
  Printer.dump();
  return OS;
}

// Set of print functions
inline raw_ostream &operator<<(raw_ostream &OS, const VPInstruction &I) {
  I.print(OS);
  return OS;
}
inline raw_ostream &operator<<(raw_ostream &OS, const VPBasicBlock &BB) {
  BB.print(OS, 2);
  return OS;
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

//===----------------------------------------------------------------------===//
// VPlan Utilities
//===----------------------------------------------------------------------===//
/// The VPlanUtils class provides common interfaces and functions that are
/// required across different VPlan classes e.g. VPBasicBlock.
class VPlanUtils {
private:
  /// Unique ID generator.
  static std::atomic<unsigned> NextOrdinal;

public:
  VPlanUtils() = delete;

  /// Create a unique name for a new VPlan entity such as a VPBasicBlock.
  static std::string createUniqueName(const llvm::Twine &Prefix) {
    std::string S;
    raw_string_ostream RSO(S);
    RSO << Prefix << NextOrdinal++;
    return RSO.str();
  }
};

/// A wrapper class to add VPlan related remarks for opt-report. Currently
/// the implementation is naive with a single method to add a remark for
/// a given loop (can be HLLoop or llvm::Loop).
//  TODO:
/// In the future this will be extended to record all vectorization related
/// remarks emitted by VPlan by mapping the remarks to underlying VPlan data
/// structures that represent a loop. For example:
///
/// VPLoopRegion MainLoop --> {"LOOP WAS VECTORIZED", "vector length: 4"}
/// VPLoopRegion RemainderLoop --> {"remainder loop was not vectorized"}
class VPlanOptReportBuilder {
  OptReportBuilder &ORBuilder;
  // ORBuilder needs the LoopInfo while adding remarks for llvm::Loop. This
  // will be nullptr for HLLoop.
  LoopInfo *LI;

public:
  VPlanOptReportBuilder(OptReportBuilder &ORB, LoopInfo *LI = nullptr)
      : ORBuilder(ORB), LI(LI) {}

  /// Add a vectorization related remark for the HIR loop \p Lp. The remark
  /// message is identified by \p MsgID.
  template <typename... Args>
  void addRemark(loopopt::HLLoop *Lp, OptReportVerbosity::Level Verbosity,
                 unsigned MsgID, Args &&...args) {
    ORBuilder(*Lp).addRemark(Verbosity, MsgID, std::forward<Args>(args)...);
  }

  /// Add a vectorization related remark for the LLVM loop \p Lp. The remark
  /// message is identified by \p MsgID.
  template <typename... Args>
  void addRemark(Loop *Lp, OptReportVerbosity::Level Verbosity, unsigned MsgID,
                 Args &&...args) {
    // For LLVM-IR Loop, LORB needs a valid LoopInfo object
    assert(LI && "LoopInfo for opt-report builder is null.");
    ORBuilder(*Lp, *LI).addRemark(Verbosity, MsgID,
                                  std::forward<Args>(args)...);
  }

  /// Add a origin related remark for the HIR loop \p Lp. The remark
  /// message is identified by \p MsgID.
  template <typename... Args>
  void addOrigin(loopopt::HLLoop *Lp, unsigned MsgID, Args &&...args) {
    ORBuilder(*Lp).addOrigin(MsgID, std::forward<Args>(args)...);
  }

  /// Add a origin related remark for the LLVM loop \p Lp. The remark
  /// message is identified by \p MsgID.
  template <typename... Args>
  void addOrigin(Loop *Lp, unsigned MsgID, Args &&...args) {
    // For LLVM-IR Loop, LORB needs a valid LoopInfo object
    assert(LI && "LoopInfo for opt-report builder is null.");
    ORBuilder(*Lp, *LI).addOrigin(MsgID, std::forward<Args>(args)...);
  }
};

// Several inline functions to hide the #if machinery from the callers.
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
inline void VPLAN_DUMP(bool Cond, StringRef Transformation, const VPlan *Plan) {
  if (!Plan->isPrintingEnabled()) // to not print string headers
    return;
  DEBUG_WITH_TYPE("vplan-dumps",
                  dbgs() << "VPlan after " << Transformation << ":\n";
                  Plan->dump(dbgs()));
  if (!Cond)
    return;
  outs() << "VPlan after " << Transformation << ":\n";
  Plan->dump(outs());
  outs().flush();
}
#else
template <class... Args> inline void VPLAN_DUMP(const Args &...) {}
#endif

// In most cases the whole pair of plain dump/dot digraph can be abstracted...
//
// This is a way to overcome our inability to leverage PassManager
// infrastructure. Ideally, we wouldn't need al that and would use something
// like
//
//   opt -vplan-cfg-import -vplan-print -vplan-predicator -vplan-dot
//
// For now, just have all the pre-set points to print any kind of dump via
// multiple cl::opts.
class VPlanDumpControl {
  // cl::opt is designed to accept StringRefs for name/description, so we need
  // to ensure the actual strings are live thoughout the cl::opt lifetime.
  class OptWrapper {
    std::string NameString;
    std::string DescriptionString;
    cl::opt<bool> Opt;

  public:
    OptWrapper(const Twine &Name, const Twine &Description)
        : NameString(Name.str()), DescriptionString(Description.str()),
          Opt(StringRef(NameString), cl::init(false), cl::Hidden,
              cl::desc(DescriptionString)) {}
    operator bool() const { return Opt; }
  };

public:
  VPlanDumpControl(const char *DumpOptPrefix, const char *DotOptPrefix,
                   const char *DotCFGOnlyOptPrefix, const Twine &ShortName,
                   const Twine &LongName, bool PrintPlainDumpPrefix)
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      : PrintPlainDumpPrefix(PrintPlainDumpPrefix),
        Dump(DumpOptPrefix + ShortName, "Print VPlan after " + LongName),
        Dot(DotOptPrefix + ShortName, "Print VPlan digraph after " + LongName),
        DotCFGOnly(DotCFGOnlyOptPrefix + ShortName,
                   "Print VPlan CFG Only after " + LongName),
        PassDescription(LongName.str()) {
  }
#else
  {
    (void)DumpOptPrefix;
    (void)DotOptPrefix;
    (void)DotCFGOnlyOptPrefix;
    (void)ShortName;
    (void)LongName;
    (void)PrintPlainDumpPrefix;
  }
#endif

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  bool dumpPlain() const { return Dump; }
  bool dumpDot() const { return Dot; }
  bool dumpCFGOnly() const { return DotCFGOnly; }
public:
  const bool PrintPlainDumpPrefix;
  StringRef getPassDescription() const { return PassDescription; }

private:
  OptWrapper Dump;
  OptWrapper Dot;
  OptWrapper DotCFGOnly;

  std::string PassDescription;
#endif
};

struct LoopVPlanDumpControl : public VPlanDumpControl {
  LoopVPlanDumpControl(const Twine &ShortName, const Twine &LongName)
      : VPlanDumpControl("vplan-print-after-", "vplan-dot-after-",
                         "vplan-cfg-only-after-", ShortName, LongName, true) {}
};
struct FuncVecVPlanDumpControl : public VPlanDumpControl {
  FuncVecVPlanDumpControl(const Twine &ShortName, const Twine &LongName,
                          bool PrintPlainDumpPrefix = false)
      : VPlanDumpControl("print-after-vplan-func-vec-",
                         "dot-after-vplan-func-vec-",
                         "cfg-only-after-vplan-func-vec-", ShortName, LongName,
                         PrintPlainDumpPrefix) {}
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
inline void VPLAN_DUMP(const VPlanDumpControl &Control, const VPlan &Plan) {
  if (!Plan.isPrintingEnabled()) // to not print string headers
    return;
  DEBUG_WITH_TYPE("vplan-dumps", dbgs()
                                     << "VPlan after "
                                     << Control.getPassDescription() << ":\n";
                  Plan.dump(dbgs()));

  if (Control.dumpPlain()) {
    if (Control.PrintPlainDumpPrefix)
      outs() << "VPlan after " << Control.getPassDescription() << ":\n";
    Plan.dump(outs());
    outs().flush();
  }
  VPlanPrinter Printer(outs(), Plan);
  if (Control.dumpDot()) {
    Printer.dump();
    outs().flush();
  }
  if (Control.dumpCFGOnly()) {
    Printer.dump(true);
    outs().flush();
  }
}
inline void VPLAN_DUMP(const VPlanDumpControl &Control, const VPlan *Plan) {
  VPLAN_DUMP(Control, *Plan);
}
#endif

using vpinst_iterator = InstIterator<VPlan::VPBasicBlockListTy, VPlan::iterator,
                                     VPBasicBlock::iterator, VPInstruction>;
using vpinst_range = iterator_range<vpinst_iterator>;

inline vpinst_iterator vpinst_begin(VPlan *F) { return vpinst_iterator(*F); }
inline vpinst_iterator vpinst_end(VPlan *F) { return vpinst_iterator(*F, true); }
inline vpinst_range vpinstructions(VPlan *F) {
  return vpinst_range(vpinst_begin(F), vpinst_end(F));
}

using const_vpinst_iterator =
    InstIterator<const VPlan::VPBasicBlockListTy, VPlan::const_iterator,
                 VPBasicBlock::const_iterator, const VPInstruction>;
using const_vpinst_range = iterator_range<const_vpinst_iterator>;

inline const_vpinst_iterator vpinst_begin(const VPlan *F) {
  return const_vpinst_iterator(*F);
}
inline const_vpinst_iterator vpinst_end(const VPlan *F) {
  return const_vpinst_iterator(*F, true);
}
inline const_vpinst_range vpinstructions(const VPlan *F) {
  return const_vpinst_range(vpinst_begin(F), vpinst_end(F));
}

template <class DivergenceAnalysis>
inline decltype(auto) vplan_da_shapes(const VPlan *P,
                                      const DivergenceAnalysis *DA) {
  return map_range(vpinstructions(P), [DA](auto &I) {
    return std::make_pair<const VPValue *, VPVectorShape>(&I, DA->getVectorShape(I));
  });
}

} // namespace vpo

// The following template specializations are implemented to support GraphTraits
// for VPlan.
template <>
struct GraphTraits<vpo::VPlan *> : public GraphTraits<vpo::VPBasicBlock *> {

  using nodes_iterator = df_iterator<NodeRef>;

  static NodeRef getEntryNode(vpo::VPlan *Plan) {
    return &Plan->getEntryBlock();
  }

  static inline nodes_iterator nodes_begin(vpo::VPlan *Plan) {
    return nodes_iterator::begin(&Plan->getEntryBlock());
  }

  static inline nodes_iterator nodes_end(vpo::VPlan *Plan) {
    // df_iterator returns an empty iterator so the node used doesn't matter.
    return nodes_iterator::end(&Plan->getEntryBlock());
  }

  static size_t size(vpo::VPlan *Plan) { return Plan->size(); }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLAN_H

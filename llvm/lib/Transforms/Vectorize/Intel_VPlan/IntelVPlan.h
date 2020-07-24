//===- IntelVPlan.h - Represent A Vectorizer Plan -------------------------===//
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

#if INTEL_CUSTOMIZATION
#include "IntelVPlanValue.h"
#include "IntelVPlanExternals.h"
#else
#include "VPlanValue.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/ADT/simple_ilist.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/GenericDomTreeConstruction.h"
#include "llvm/Support/raw_ostream.h"
#include <atomic>

#if INTEL_CUSTOMIZATION
#include "IntelVPBasicBlock.h"
#include "IntelVPLoopAnalysis.h"
#include "IntelVPlanAlignmentAnalysis.h"
#include "IntelVPlanDivergenceAnalysis.h"
#include "IntelVPlanLoopInfo.h"
#include "IntelVPlanScalVecAnalysis.h"
#include "IntelVPlanValueTracking.h"
#include "VPlanHIR/IntelVPlanInstructionDataHIR.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/Diag.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLGoto.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Support/FormattedStream.h"
#endif // INTEL_CUSTOMIZATION

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

#if INTEL_CUSTOMIZATION
namespace loopopt {
class RegDDRef;
class HLLoop;
class OptReportDiag;
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
class VPlanCostModel; // INTEL: to be later declared as a friend
class VPlanCostModelProprietary; // INTEL: to be later declared as a friend
class VPlanDivergenceAnalysis;
class VPlanBranchDependenceAnalysis;
#endif // INTEL_CUSTOMIZATION
typedef SmallPtrSet<VPValue *, 8> UniformsTy;

// Class names mapping to minimize the diff:
#define InnerLoopVectorizer VPOCodeGen
#define LoopVectorizationLegality VPOVectorizationLegality

struct TripCountInfo;
#endif // INTEL_CUSTOMIZATION

// This class is used to create all the necessairy analyses that are needed for
// VPlan.
class VPAnalysesFactory {
private:
  ScalarEvolution &SE;
  const Loop *Lp = nullptr;
  DominatorTree *DT = nullptr;
  AssumptionCache *AC = nullptr;
  const DataLayout *DL = nullptr;
  bool IsLLVMIR = false;

public:
  VPAnalysesFactory(ScalarEvolution &SE, Loop *Lp, DominatorTree *DT,
                    AssumptionCache *AC, const DataLayout *DL, bool IsLLVMIR)
      : SE(SE), Lp(Lp), DT(DT), AC(AC), DL(DL), IsLLVMIR(IsLLVMIR) {}

  std::unique_ptr<VPlanScalarEvolution> createVPSE() {
    if (IsLLVMIR) {
      return std::make_unique<VPlanScalarEvolutionLLVM>(SE, Lp);
    } else
      llvm_unreachable("Unimplemented for HIR path.");
  }

  std::unique_ptr<VPlanValueTracking> createVPVT(VPlanScalarEvolution *VPSE) {
    if (IsLLVMIR) {
      auto *VPSELLVM = static_cast<VPlanScalarEvolutionLLVM *>(VPSE);
      return std::make_unique<VPlanValueTrackingLLVM>(*VPSELLVM, *DL, AC, DT);
    } else
      llvm_unreachable("Unimplemented for HIR path.");
  }

  bool isLLVMIRPath() { return IsLLVMIR; }

  const Loop *getLoop() { return Lp; }
  DominatorTree *getDominatorTree() { return DT; }
  AssumptionCache *getAssumptionCache() { return AC; }
  const DataLayout *getDataLayout() { return DL; }
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
    /// The previous IR BasicBlock created or used. Initially set to the new
    /// header BasicBlock.
    BasicBlock *PrevBB = nullptr;
    /// The last IR BasicBlock in the output IR. Set to the new latch
    /// BasicBlock, used for placing the newly created BasicBlocks.
    BasicBlock *InsertBefore = nullptr;
    /// A mapping of each VPBasicBlock to the corresponding BasicBlock. In case
    /// of replication, maps the BasicBlock of the last replica created.
    SmallDenseMap<VPBasicBlock *, BasicBlock *> VPBB2IRBB;
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
#if INTEL_CUSTOMIZATION
  VPLoopInfo *VPLI;
#endif
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
#if INTEL_CUSTOMIZATION
  friend class HIRSpecifics;
  friend class VPBasicBlock;
  friend class VPBranchInst;
  friend class VPBuilder;
  friend class VPBuilderHIR;
  friend class VPDecomposerHIR;
  // To get underlying HIRData until we have proper VPType.
  friend class VPVLSClientMemrefHIR;
  friend class VPlanCostModel;
  friend class VPlanCostModelProprietary;
  friend class VPlanDivergenceAnalysis;
  friend class VPlanIdioms;
  friend class VPlanValueTrackingLLVM;
  friend class VPlanVLSAnalysis;
  friend class VPlanVLSAnalysisHIR;
  friend class VPlanVerifier;
  friend class VPOCodeGen;
  friend class VPOCodeGenHIR;
  friend class VPCloneUtils;
  friend class VPValueMapper;
  friend class VPLoopEntityList;
  friend class VPValue;
  // FIXME: This is only needed to support buggy mixed HIR codegen. Once we
  // retire it and use full VPValue-based codegen, underlying IR copying won't
  // be necessary.
  friend class VPlanPredicator;

  /// Hold all the HIR-specific data and interfaces for a VPInstruction.
  class HIRSpecifics {
    friend class VPValue;

  private:
    /// Return true if the underlying HIR data is valid. If it's a decomposed
    /// VPInstruction, the HIR of the attached master VPInstruction is checked.
    bool isValid() const {
      if (isMaster() || isDecomposed())
        return getVPInstData()->isValid();

      // For other VPInstructions without underlying HIR.
      assert(!isSet() && "HIR data must be unset!");
      return false;
    }

    /// Invalidate underlying HIR deta. If decomposed VPInstruction, the HIR of
    /// its master VPInstruction is invalidated.
    void invalidate() {
      if (isMaster() || isDecomposed())
        getVPInstData()->setInvalid();
    }

  public:
    HIRSpecifics() {}
    ~HIRSpecifics() {
      if (isMaster())
        delete getVPInstData();
    }

    // DESIGN PRINCIPLE: IR-independent algorithms don't need to know about
    // HIR-specific master, decomposed and new VPInstructions or underlying HIR
    // information. For that reason, access to the following HIR-specific
    // methods must be restricted. We achieve that goal by making
    // VPInstruction's HIRSpecifics member private.

    // Hold the underlying HIR information related to the LHS operand of this
    // VPInstruction.
    std::unique_ptr<VPOperandHIR> LHSHIROperand;

    // Union used to save needed information based on instruction opcode.
    // 1) For a load/store instruction, save the symbase of the corresponding
    //    scalar memref. Vector memref generated during vector CG is assigned
    //    the same symbase.
    // 2) For convert instructions, save whether the convert represents a
    //    convert of a loop IV that needs to be folded into the containing canon
    //    expression.
    union {
      unsigned Symbase = loopopt::InvalidSymbase;
      bool FoldIVConvert;
    };

    // Temporarily used to store alias analysis related metadata for memory
    // refs. TODO - remove this once metadata representation/propagation fix
    // is in place (CMPLRLLVM-11656).
    AAMDNodes AANodes;

    /// Pointer to access the underlying HIR data attached to this
    /// VPInstruction, if any, depending on its sub-type:
    ///   1) Master VPInstruction: MasterData points to a VPInstDataHIR holding
    ///      the actual HIR data.
    ///   2) Decomposed VPInstruction: MasterData points to master VPInstruction
    ///      holding the actual HIR data.
    ///   3) Other VPInstruction (!Master and !Decomposed): MasterData is null.
    ///      We use a void pointer to represent this case.
    PointerUnion<MasterVPInstData *, VPInstruction *, void *> MasterData =
        (int *)nullptr;

    // Return the VPInstruction data of this VPInstruction if it's a master or
    // decomposed. Return nullptr otherwise.
    MasterVPInstData *getVPInstData() {
      if (isMaster())
        return MasterData.get<MasterVPInstData *>();
      if (isDecomposed())
        return getMaster()->HIR.getVPInstData();
      // New VPInstructions don't have VPInstruction data.
      return nullptr;
    }
    const MasterVPInstData *getVPInstData() const {
      return const_cast<HIRSpecifics *>(this)->getVPInstData();
    }

    void verifyState() const {
      if (MasterData.is<MasterVPInstData *>())
        assert(!MasterData.isNull() &&
               "MasterData can't be null for master VPInstruction!");
      else if (MasterData.is<VPInstruction *>())
        assert(!MasterData.isNull() &&
               "MasterData can't be null for decomposed VPInstruction!");
      else
        assert(MasterData.is<void *>() && MasterData.isNull() &&
               "MasterData must be null for VPInstruction that is not master "
               "or decomposed!");
    }

    /// Return true if this is a master VPInstruction.
    bool isMaster() const {
      verifyState();
      return MasterData.is<MasterVPInstData *>();
    }

    /// Return true if this is a decomposed VPInstruction.
    bool isDecomposed() const {
      verifyState();
      return MasterData.is<VPInstruction *>();
    }

    // Return true if MasterData contains actual HIR data.
    bool isSet() const {
      verifyState();
      return !MasterData.is<void *>();
    }

    /// Return the underlying HIR attached to this master VPInstruction. Return
    /// nullptr if the VPInstruction doesn't have underlying HIR.
    loopopt::HLNode *getUnderlyingNode() {
      MasterVPInstData *MastData = getVPInstData();
      if (!MastData)
        return nullptr;
      return MastData->getNode();
    }
    loopopt::HLNode *getUnderlyingNode() const {
      return const_cast<HIRSpecifics *>(this)->getUnderlyingNode();
    }

    /// Attach \p UnderlyingNode to this VPInstruction and turn it into a master
    /// VPInstruction.
    void setUnderlyingNode(loopopt::HLNode *UnderlyingNode) {
      assert(!isSet() && "MasterData is already set!");
      MasterData = new MasterVPInstData(UnderlyingNode);
    }

    /// Attach \p Def to this VPInstruction as its VPOperandHIR.
    void setOperandDDR(const loopopt::DDRef *Def) {
      assert(!LHSHIROperand && "LHSHIROperand is already set!");
      LHSHIROperand.reset(new VPBlob(Def));
    }

    /// Attach \p IVLevel to this VPInstruction as its VPOperandHIR.
    void setOperandIV(unsigned IVLevel) {
      assert(!LHSHIROperand && "LHSHIROperand is already set!");
      LHSHIROperand.reset(new VPIndVar(IVLevel));
    }

    /// Return the VPOperandHIR with the underlying HIR information of the LHS
    /// operand.
    VPOperandHIR *getOperandHIR() const { return LHSHIROperand.get(); }

    /// Return the master VPInstruction attached to a decomposed VPInstruction.
    VPInstruction *getMaster() {
      assert(isDecomposed() && "Only decomposed VPInstructions have a pointer "
                               "to a master VPInstruction!");
      return MasterData.get<VPInstruction *>();
    }
    VPInstruction *getMaster() const {
      return const_cast<HIRSpecifics *>(this)->getMaster();
    }

    /// Attach \p MasterVPI as master VPInstruction of a decomposed
    /// VPInstruction.
    void setMaster(VPInstruction *MasterVPI) {
      assert(MasterVPI && "Master VPInstruction cannot be set to null!");
      assert(!isMaster() &&
             "A master VPInstruction can't point to a master VPInstruction!");
      assert(!isSet() && "Master VPInstruction is already set!");
      MasterData = MasterVPI;
    }

    /// Mark the underlying HIR data as valid.
    void setValid() {
      assert(isMaster() && "Only a master VPInstruction must set HIR!");
      getVPInstData()->setValid();
    }

    /// Print HIR-specific flags. It's mainly for debugging purposes.
    void printHIRFlags(raw_ostream &OS) const {
      OS << "IsMaster=" << isMaster() << " IsDecomp=" << isDecomposed()
         << " IsNew=" << !isSet() << " HasValidHIR= " << isValid() << "\n";
    }

    void setSymbase(unsigned SB) { Symbase = SB; }
    unsigned getSymbase(void) const { return Symbase; }

    void setFoldIVConvert(bool Fold) { FoldIVConvert = Fold; }
    bool getFoldIVConvert(void) const { return FoldIVConvert; }

    void cloneFrom(const HIRSpecifics &HIR) {
      if (HIR.isMaster()) {
        setUnderlyingNode(HIR.getUnderlyingNode());
        if (HIR.isValid())
          setValid();
      } else if (HIR.isDecomposed())
        setMaster(HIR.getMaster());

      // Copy the operand.
      if (VPOperandHIR *HIROperand = HIR.getOperandHIR()) {
        if (VPBlob *Blob = dyn_cast<VPBlob>(HIROperand))
          setOperandDDR(Blob->getBlob());
        else {
          VPIndVar *IV = cast<VPIndVar>(HIROperand);
          setOperandIV(IV->getIVLevel());
        }
      }
      setSymbase(HIR.getSymbase());
      AANodes = HIR.AANodes;
      setFoldIVConvert(HIR.getFoldIVConvert());

      // Verify correctness of the cloned HIR.
      assert(isMaster() == HIR.isMaster() &&
             "Cloned isMaster() value should be equal to the original one");
      assert(isDecomposed() == HIR.isDecomposed() &&
             "Cloned isDecomposed() value should be equal to the original one");
      assert(isSet() == HIR.isSet() &&
             "Cloned isSet() value should be equal to the original one");
      if (isSet())
        assert(HIR.isValid() == isValid() &&
               "Cloned isValid() value should be equal to the original one");
    }
  };

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
      case VPInstruction::HIRCopy:
      case VPInstruction::ReductionFinal: {
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
#endif // INTEL_CUSTOMIZATION

public:
#if INTEL_CUSTOMIZATION
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
      FMin,
      InductionInit,
      InductionInitStep,
      InductionFinal,
      ReductionInit,
      ReductionFinal,
      AllocatePrivate,
      Subscript,
      Blend,
      HIRCopy,
      OrigTripCountCalculation,
      VectorTripCountCalculation,
  };
#else
  enum { Not = Instruction::OtherOpsEnd + 1 };
#endif

private:
  typedef unsigned char OpcodeTy;
  OpcodeTy Opcode;

  /// Each VPInstruction belongs to a single VPBasicBlock.
  VPBasicBlock *Parent = nullptr;

  // Debug location for this VPInstruction.
  DebugLoc DbgLoc;
  // Hold operator-related metadata attributes attached to this VPInstruction.
  VPOperatorIRFlags OperatorFlags;

  /// Utility method serving execute(): generates a single instance of the
  /// modeled instruction.
  void generateInstruction(VPTransformState &State, unsigned Part);

  void copyUnderlyingFrom(const VPInstruction &Inst) {
#if INTEL_CUSTOMIZATION
    HIR.cloneFrom(Inst.HIR);
#endif // INTEL_CUSTOMIZATION
    Value *V = Inst.getUnderlyingValue();
    if (V)
      setUnderlyingValue(*V);
    if (!Inst.isUnderlyingIRValid())
      invalidateUnderlyingIR();
  }

#if INTEL_CUSTOMIZATION
  void copyAttributesFrom(const VPInstruction &Inst) {
    DbgLoc = Inst.DbgLoc;
    // Copy other general attributes here when imported.
    OperatorFlags = Inst.OperatorFlags;
  }
#endif

#if INTEL_CUSTOMIZATION
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

  /// Return true if this is a new VPInstruction (i.e., an VPInstruction that is
  /// not coming from the underlying IR.
  bool isNew() const { return getUnderlyingValue() == nullptr && !HIR.isSet(); }

  // Hold the underlying HIR information, if any, attached to this
  // VPInstruction. This field is protected to provide access to derived
  // subclasses of VPInstruction.
  HIRSpecifics HIR;

  void setSymbase(unsigned Symbase) {
    assert(Opcode == Instruction::Store ||
           Opcode == Instruction::Load &&
               "setSymbase called for invalid VPInstruction");
    assert(Symbase != loopopt::InvalidSymbase &&
           "Unexpected invalid symbase assignment");
    HIR.setSymbase(Symbase);
  }

  unsigned getSymbase(void) const {
    assert(Opcode == Instruction::Store ||
           Opcode == Instruction::Load &&
               "getSymbase called for invalid VPInstruction");
    assert(HIR.Symbase != loopopt::InvalidSymbase &&
           "Unexpected invalid symbase");
    return HIR.getSymbase();
  }

  void setFoldIVConvert(bool Fold) {
    assert(Fold == false || Opcode == Instruction::SExt ||
           Opcode == Instruction::Trunc ||
           Opcode == Instruction::ZExt &&
               "unexpected call to setFoldIVConvert");
    HIR.setFoldIVConvert(Fold);
  }

  bool getFoldIVConvert(void) const {
    assert(Opcode == Instruction::SExt || Opcode == Instruction::Trunc ||
           Opcode == Instruction::ZExt &&
               "getFoldIVConvert called for invalid VPInstruction");
    return HIR.getFoldIVConvert();
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
        OperatorFlags(Opcode, BaseTy) {
    assert(BaseTy && "BaseTy can't be null!");
    if (Opcode != Instruction::Load && Opcode != Instruction::Store)
      setFoldIVConvert(false);
  }
  VPInstruction(unsigned Opcode, Type *BaseTy,
                std::initializer_list<VPValue *> Operands)
      : VPUser(VPValue::VPInstructionSC, Operands, BaseTy), Opcode(Opcode),
        OperatorFlags(Opcode, BaseTy) {
    assert(BaseTy && "BaseTy can't be null!");
    if (Opcode != Instruction::Load && Opcode != Instruction::Store)
      setFoldIVConvert(false);
  }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return V->getVPValueID() == VPValue::VPInstructionSC;
  }

  unsigned getOpcode() const { return Opcode; }
#if INTEL_CUSTOMIZATION
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
#endif // INTEL_CUSTOMIZATION

  // ilist should have access to VPInstruction node.
  friend struct ilist_traits<VPInstruction>;
  friend struct ilist_traits<VPBasicBlock>;

  /// \return the VPBasicBlock which this VPInstruction belongs to.
  VPBasicBlock *getParent() { return Parent; }
  const VPBasicBlock *getParent() const { return Parent; }
  void setParent(VPBasicBlock *NewParent) { Parent = NewParent; }

  /// Generate the instruction.
  /// TODO: We currently execute only per-part unless a specific instance is
  /// provided.
  virtual void execute(VPTransformState &State);
#if INTEL_CUSTOMIZATION
  virtual void executeHIR(VPOCodeGenHIR *CG);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Dump the VPInstruction.
  void dump(raw_ostream &O) const override { dump(O, nullptr, nullptr); };
  void dump(raw_ostream &O, const VPlanDivergenceAnalysis *DA,
            const VPlanScalVecAnalysis *SVA) const;

  void dump() const override { dump(errs()); }
  void dump(const VPlanDivergenceAnalysis *DA,
            const VPlanScalVecAnalysis *SVA) const {
    dump(dbgs(), DA, SVA);
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
#endif

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Print the VPInstruction.
  virtual void print(raw_ostream &O, const Twine &Indent) const;

  /// Print the VPInstruction.
  void print(raw_ostream &O, const VPlanDivergenceAnalysis *DA = nullptr,
             const VPlanScalVecAnalysis *SVA = nullptr) const;
  static const char *getOpcodeName(unsigned Opcode);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  VPInstruction *clone() const {
    VPInstruction *Cloned = cloneImpl();
    Cloned->copyUnderlyingFrom(*this);
    Cloned->copyAttributesFrom(*this);
    return Cloned;
  }
};

#if INTEL_CUSTOMIZATION
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
    loopopt::HLNode *Node = HIR.getUnderlyingNode();
    if (!Node)
      return nullptr;
    return cast<loopopt::HLGoto>(Node);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &O) const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  static inline bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == Instruction::Br;
  }
  static bool classof(const VPUser *U) {
    return isa<VPInstruction>(U) && classof(cast<VPInstruction>(U));
  }

  /// Iterators and types to access Successors of terminator's
  /// parent VPBasicBlock.
  inline operand_iterator succ_begin() { return op_begin(); }
  inline operand_iterator succ_end() {
    // CondBit is expected to be last operand of VPBranchInst (if available)
    return getCondition() ? std::prev(op_end()) : op_end();
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
    setOperand(idx, NewSucc);
  }

  /// Returns successors total count.
  size_t getNumSuccessors() const {
    return std::distance(succ_begin(), succ_end());
  }

  /// Returns the condition bit selecting the successor.
  VPValue *getCondition() const {
    VPValue *Cond = nullptr;
    if (unsigned Count = getNumOperands()) {
      Cond = getOperand(Count - 1);
      // Condition can't be a basic block.
      if (isa<VPBasicBlock>(Cond))
        Cond = nullptr;
    }
    return Cond;
  }

  /// Set a condition bit or replace the existing one.
  void setCondition(VPValue *Cond) {
    if (getCondition()) {
      if (Cond)
        setOperand(getNumOperands() - 1, Cond);
      else
        removeOperand(getNumOperands() - 1);
    }
    else if (Cond)
      addOperand(Cond);
  }

protected:
  virtual VPBranchInst *cloneImpl() const final {
    VPBranchInst *Cloned = new VPBranchInst(getType());
    for (unsigned i = 0; i < getNumOperands(); i++)
      Cloned->addOperand(getOperand(i));
    return Cloned;
  }
};

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
  void print(raw_ostream &O) const;
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
private:
  SmallVector<VPBasicBlock *, 2> VPBBUsers;

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
    return getIncomingValue(Idx.getValue());
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
  // Trailing struct offsets for a GEP are tracked via a vector of bools called
  // OperandIsStructOffset. This vector will always be consistent with the
  // number of operands stored in a given GEP instruction. An operand's
  // corresponding index entry in OperandIsStructOffset is set to true if it's a
  // trailing struct offset, false otherwise. NOTE: This information is needed
  // only if the GEP is created via HIR-path. For LLVM-IR path the vector will
  // always be false for all entries.
  //
  // Examples:
  // 1. (@a)[0].1[i1] --> gep %a, 0, 1, %vpi1
  //
  //    Vector will look like:
  //    <false, false, true, false>
  //
  //    i.e. operand at index 2 is a struct offset corresponding to the previous
  //    last-found non struct offset operand (index 1).
  //
  // 2. (@a)[0].2.1[5][i1] --> gep %a, 0, 2, 1, 5, %vpi1
  //
  //    Vector will look like:
  //    <false, false, true, true, false, false>
  //

  bool InBounds;
  SmallVector<bool, 4> OperandIsStructOffset;

public:
  /// Default constructor for VPGEPInstruction. The default value for \p
  /// InBounds is false.
  VPGEPInstruction(Type *BaseTy, VPValue *Ptr, ArrayRef<VPValue *> IdxList,
                   bool InBounds = false)
      : VPInstruction(Instruction::GetElementPtr, BaseTy, {}),
        InBounds(InBounds) {
    assert(Ptr && "Base pointer operand of GEP cannot be null.");
    // Base pointer should be the first operand of GEP followed by index
    // operands
    assert(!getNumOperands() &&
           "GEP instruction already has operands before base pointer.");
    VPInstruction::addOperand(Ptr);
    for (auto Idx : IdxList)
      VPInstruction::addOperand(Idx);
    // Track all operands as non struct offsets, since that information is not
    // available at this point.
    OperandIsStructOffset.resize(1 + IdxList.size(), false);
  }

  /// Setter and getter functions for InBounds.
  void setIsInBounds(bool IsInBounds) { InBounds = IsInBounds; }
  bool isInBounds() const { return InBounds; }

  /// Get the base pointer operand of given VPGEPInstruction.
  VPValue *getPointerOperand() const { return VPInstruction::getOperand(0); }

  /// Overloaded method for adding an operand \p Operand. The struct offset
  /// tracker is accordingly updated after operand addition.
  void addOperand(VPValue *Operand, bool IsStructOffset = false) {
    VPInstruction::addOperand(Operand);
    OperandIsStructOffset.push_back(IsStructOffset);
    assert(OperandIsStructOffset.size() == getNumOperands() &&
           "Number of operands and struct offset tracker sizes don't match.");
  }

  /// Overloaded method for setting index \p Idx with operand \p Operand. The
  /// struct offset tracker is accordingly updated after operand is set.
  void setOperand(const unsigned Idx, VPValue *Operand,
                  bool IsStructOffset = false) {
    assert((Idx > 1 || !IsStructOffset) &&
           "Base pointer and first index operand of GEP cannot be a struct "
           "offset.");
    VPInstruction::setOperand(Idx, Operand);
    OperandIsStructOffset[Idx] = IsStructOffset;
  }

  /// Overloaded method for removing an operand at index \p Idx. The struct
  /// offset tracker is accordingly updated after operand removal.
  void removeOperand(const unsigned Idx) {
    VPInstruction::removeOperand(Idx);
    OperandIsStructOffset.erase(OperandIsStructOffset.begin() + Idx);
    assert(OperandIsStructOffset.size() == getNumOperands() &&
           "Number of operands and struct offset tracker sizes don't match.");
  }

  /// Check if a given operand \p Operand of this GEP is a struct offset.
  bool isOperandStructOffset(VPValue *Operand) const {
    auto It = llvm::find(operands(), Operand);
    assert(It != op_end() && "Operand not found in VPGEPInstruction.");
    return OperandIsStructOffset[std::distance(op_begin(), It)];
  }

  /// Check if operand at index \p Idx of this GEP is a struct offset.
  bool isOperandStructOffset(const unsigned Idx) const {
    assert(Idx < OperandIsStructOffset.size() &&
           "Operand index out of bounds.");
    return OperandIsStructOffset[Idx];
  }

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == Instruction::GetElementPtr;
  }

  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

protected:
  virtual VPGEPInstruction *cloneImpl() const final {
    VPGEPInstruction *Cloned =
        new VPGEPInstruction(getType(), getOperand(0), {}, isInBounds());
    for (auto *O : make_range(op_begin()+1, op_end())) {
      Cloned->addOperand(O, isOperandStructOffset(O));
    }
    return Cloned;
  }
};

/// Concrete class to represent a single or multi-dimensional array access
/// implemented using llvm.intel.subscript intrinsic calls in VPlan.
// Consider the following two examples:
//
// 1. Single-dimension access
//
//    If the incoming IR contains:
//    %0 = llvm.intel.subscript...(i8 %Rank, i32 %Lower, i32 %Stride,
//                                 i32* %Base, i32 %Idx)
//      (or)
//    %0 = @(%Base)[%Idx]
//
//    The corresponding VPSubscriptInst in VPlan looks like:
//    i32* %vp0 = subscript i32* %Base, i32 %Lower, i32 %Stride, i32 %Idx
//
//    where,
//     getRank(0) = %Rank
//     getLower(0) = %Lower
//     getStride(0) = %Stride
//     getIndex(0) = %Idx
//
// 2. Combined multi-dimensional access
//
//    If the incoming IR contains:
//    %0 = llvm.intel.subscript...(i8 1, i32 %L1, i32 %S1, i32* %Base, i32 %I1)
//    %1 = llvm.intel.subscript...(i8 0, i32 %L0, i32 %S0, i32* %0, i32 %I0)
//      (or)
//    %1 = @(%Base)[%I1][%I0]
//
//    The corresponding VPSubscriptInst in VPlan for combined access looks like:
//    i32* %vp1 = subscript i32* %Base, i32 %L1, i32 %S1, i32 %I1,
//                                      i32 %L0, i32 %S0, i32 %I0
//
//    where,
//     getRank(1) = 1
//     getLower(1) = %L1
//     getStride(1) = %S1
//     getIndex(1) = %I1
//    and,
//     getRank(0) = 0
//     getLower(0) = %L0
//     getStride(0) = %S0
//     getIndex(0) = %I0
class VPSubscriptInst final : public VPInstruction {
public:
  using DimStructOffsetsMapTy = DenseMap<unsigned, SmallVector<unsigned, 4>>;
  using DimTypeMapTy = DenseMap<unsigned, Type *>;

private:
  bool InBounds = false;
  // TODO: Below SmallVectors are currently assuming that dimensions are added
  // in a fixed order i.e. outer-most dimension to inner-most dimension. To
  // support flexibility in this ordering, DenseMaps will be needed. For
  // example, for a 3-dimensional array access, Ranks = < 2, 1, 0 >.
  SmallVector<unsigned, 4> Ranks;
  // Struct offsets associated with each dimension of this array access. For
  // example, incoming HIR contains:
  // %1 = @(%Base)[%I1].0.1[%I2].0[%I3]
  //
  // then the map below will have following entries:
  // Dim  --->  Offsets
  //  2         {0, 1}
  //  1          {0}
  //  0          {}
  //
  // NOTE: This information is needed only if the subscript instruction
  // is created via HIR-path. For LLVM-IR path the map will always be empty.
  DimStructOffsetsMapTy DimStructOffsets;
  // Type associated with each dimension of this array access. For example,
  // incoming HIR contains:
  // %1 = (@arr)[0:0:4096([1024 x i32]*:0)][0:i1:4([1024 x i32]:1024)]
  //
  // then the map below will have following entries:
  // Dim  --->     Type
  //  1         [1024 x i32]*
  //  0         [1024 x i32]
  //
  // NOTE: This information is needed only if the subscript instruction
  // is created via HIR-path. For LLVM-IR path the map will always be empty.
  DimTypeMapTy DimTypes;

  /// Add a new dimension to represent the array access for this subscript
  /// instruction.
  void addDimension(unsigned Rank, VPValue *Lower, VPValue *Stride,
                    VPValue *Index) {
    if (Ranks.size() > 0) {
      assert(Rank == Ranks.back() - 1 &&
             "Dimension is being added in out-of-order fashion.");
    }
    assert(getNumOperands() > 0 &&
           "Dimension is being added without base pointer.");
    Ranks.push_back(Rank);
    addOperand(Lower);
    addOperand(Stride);
    addOperand(Index);
  }

  unsigned getDimensionArrayIndex(unsigned Dim) const {
    assert(Dim < getNumDimensions() && "Invalid dimension.");
    unsigned Idx = getNumDimensions() - 1 - Dim;
    return Idx;
  }

  /// Explicit copy constructor to copy the operand list, Ranks and struct
  /// offsets map.
  VPSubscriptInst(const VPSubscriptInst &Other)
      : VPInstruction(VPInstruction::Subscript, Other.getType(), {}) {
    for (auto *Op : Other.operands())
      addOperand(Op);
    InBounds = Other.InBounds;
    Ranks = Other.Ranks;
    DimStructOffsets = Other.DimStructOffsets;
    DimTypes = Other.DimTypes;
  }

public:
  /// Constructor to allow clients to create a VPSubscriptInst with a single
  /// dimension only. Type of the instruction will be same as the type of the
  /// base pointer operand.
  VPSubscriptInst(Type *BaseTy, unsigned Rank, VPValue *Lower, VPValue *Stride,
                  VPValue *Base, VPValue *Index)
      : VPInstruction(VPInstruction::Subscript, BaseTy,
                      {Base, Lower, Stride, Index}) {
    Ranks.push_back(Rank);
  }

  /// Constructor to create a VPSubscriptInst that represents a combined
  /// multi-dimensional array access. The fields are expected to be ordered from
  /// highest-dimension to lowest-dimension.
  VPSubscriptInst(Type *BaseTy, unsigned NumDims, ArrayRef<VPValue *> Lowers,
                  ArrayRef<VPValue *> Strides, VPValue *Base,
                  ArrayRef<VPValue *> Indices)
      : VPInstruction(VPInstruction::Subscript, BaseTy, {Base}) {
    assert((Lowers.size() == NumDims && Strides.size() == NumDims &&
            Indices.size() == NumDims) &&
           "Inconsistent parameters for multi-dimensional subscript access.");
    for (unsigned I = 0; I < NumDims; ++I) {
      unsigned Rank = NumDims - 1 - I;
      addDimension(Rank, Lowers[I], Strides[I], Indices[I]);
    }
  }

  /// Constructor to create a VPSubscriptInst that represents a combined
  /// multi-dimensional array access when each dimension has corresponding
  /// struct offsets and the dimension's associated type information is also
  /// available. The fields are expected to be ordered from highest-dimension to
  /// lowest-dimension.
  VPSubscriptInst(Type *BaseTy, unsigned NumDims, ArrayRef<VPValue *> Lowers,
                  ArrayRef<VPValue *> Strides, VPValue *Base,
                  ArrayRef<VPValue *> Indices,
                  DimStructOffsetsMapTy StructOffsets, DimTypeMapTy Types)
      : VPSubscriptInst(BaseTy, NumDims, Lowers, Strides, Base, Indices) {
    DimStructOffsets = std::move(StructOffsets);
    DimTypes = std::move(Types);
  }

  /// Setter and getter functions for InBounds.
  void setIsInBounds(bool IsInBounds) { InBounds = IsInBounds; }
  bool isInBounds() const { return InBounds; }

  /// Get trailing struct offsets for given dimension.
  ArrayRef<unsigned> getStructOffsets(unsigned Dim) const {
    auto Iter = DimStructOffsets.find(Dim);
    if (Iter != DimStructOffsets.end())
      return Iter->second;
    return ArrayRef<unsigned>(None);
  }
  /// Get type associated for given dimension.
  Type *getDimensionType(unsigned Dim) const {
    auto Iter = DimTypes.find(Dim);
    assert(Iter != DimTypes.end() &&
           "Type information not found for dimension.");
    return Iter->second;
  }

  /// Get number of dimensions associated with this array access.
  unsigned getNumDimensions() const { return Ranks.size(); }

  /// Get the rank for given dimension. For multi-dimensional accesses this
  /// should be trivial as Dimension == Rank.
  unsigned getRank(unsigned Dim) const {
    return Ranks[getDimensionArrayIndex(Dim)];
  }
  /// Get the lower bound of index for given dimension.
  VPValue *getLower(unsigned Dim) const {
    unsigned OpIdx = (getDimensionArrayIndex(Dim) * 3) + 1;
    return getOperand(OpIdx);
  }
  /// Get the stride of access for given dimension.
  VPValue *getStride(unsigned Dim) const {
    unsigned OpIdx = (getDimensionArrayIndex(Dim) * 3) + 2;
    return getOperand(OpIdx);
  }
  /// Get the base pointer operand.
  VPValue *getPointerOperand() const { return getOperand(0); }
  /// Get the index operand for given dimension.
  VPValue *getIndex(unsigned Dim) const {
    unsigned OpIdx = (getDimensionArrayIndex(Dim) * 3) + 3;
    return getOperand(OpIdx);
  }

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == VPInstruction::Subscript;
  }

  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

protected:
  virtual VPSubscriptInst *cloneImpl() const {
    VPSubscriptInst *Cloned = new VPSubscriptInst(*this);
    return Cloned;
  }
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
  virtual VPHIRCopyInst *cloneImpl() const {
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
    VectorVariant *MatchedVecVariant = nullptr;
    Optional<StringRef> VectorLibraryFn = None;
    Intrinsic::ID VectorIntrinsic = Intrinsic::not_intrinsic;
    unsigned PumpFactor = 1;
    // Specifies if masked version of a vector variant should be used to
    // vectorize the unmasked call (using all-true mask).
    unsigned UseMaskedForUnmasked : 1;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    void print(raw_ostream &OS) const {
      std::string VecVariantName =
          MatchedVecVariant != nullptr ? MatchedVecVariant->toString() : "None";
      OS << "  VecVariant: " << VecVariantName << "\n";
      OS << "  VecIntrinsic: " << Intrinsic::getName(VectorIntrinsic, {})
         << "\n";
      OS << "  UseMaskForUnmasked: " << UseMaskedForUnmasked << "\n";
      OS << "  VecLibFn: " << VectorLibraryFn << "\n";
      OS << "  PumpFactor: " << PumpFactor << "\n";
    }
    void dump() const { print(outs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
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
    DoNotWiden
  };

  /// Tracks the decision taken on how to vectorize this VPCallInst for given
  /// VF.
  CallVecScenarios VecScenario = CallVecScenarios::Undefined;

public:
  using CallVecScenariosTy = CallVecScenarios;

  VPCallInstruction(VPValue *CalledValue, ArrayRef<VPValue *> ArgList,
                    const CallInst *OrigCall)
      : VPInstruction(Instruction::Call, OrigCall->getType(), ArgList) {
    assert(OrigCall &&
           "VPlan trying to create a new VPCall without underlying Call.");
    assert(CalledValue && "Call instruction does not have CalledValue");
    // Add called value to end of operand list for def-use chain.
    addOperand(CalledValue);
    resetVecScenario(0 /*Initial VF*/);
    // Check if Call should not be strictly widened i.e. not (re-)vectorized or
    // serialized.
    if (OrigCall->hasFnAttr("kernel-uniform-call"))
      VecScenario = CallVecScenarios::DoNotWiden;
  }

  /// Helper utility to access underlying CallInst corresponding to this
  /// VPCallInstruction. The utility works for both LLVM-IR and HIR paths.
  const CallInst *getUnderlyingCallInst() const {
    if (auto *IRCall = dyn_cast_or_null<CallInst>(getInstruction()))
      return IRCall;
    else if (auto *HIRCall = HIR.getUnderlyingNode())
      return cast<loopopt::HLInst>(HIRCall)->getCallInst();
    else
      llvm_unreachable(
          "VPlan created a new VPCallInstruction without underlying IR.");
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

  /// Getter for original call's calling convention.
  CallingConv::ID getOrigCallingConv() const {
    return getUnderlyingCallInst()->getCallingConv();
  }

  // Getter for original call's callsite attributes.
  AttributeList getOrigCallAttrs() const {
    return getUnderlyingCallInst()->getAttributes();
  }

  // Some helpful getters based on underlying call's attributes.
  bool isKernelCallOnce() const {
    return getOrigCallAttrs().hasFnAttribute("kernel-call-once");
  }
  bool isOCLVecUniformReturn() const {
    return getOrigCallAttrs().hasFnAttribute("opencl-vec-uniform-return");
  }
  bool isKernelUniformCall() const {
    return getOrigCallAttrs().hasFnAttribute("kernel-uniform-call");
  }

  /// Clear decision that was last computed for this call, and reset to initial
  /// state (Undef scenario) for new VF.
  void resetVecScenario(unsigned NewVF) {
    // Record VF for which new vectorization scenario and properties will be
    // recorded.
    VecProperties.VF = NewVF;
    // DoNotWiden is used only for kernel uniform calls today i.e. the property
    // is not VF-dependent. Hence it need not be reset here.
    if (VecScenario == CallVecScenarios::DoNotWiden)
      return;

    VecScenario = CallVecScenarios::Undefined;
    VecProperties.MatchedVecVariant = nullptr;
    VecProperties.VectorLibraryFn = None;
    VecProperties.VectorIntrinsic = Intrinsic::not_intrinsic;
    VecProperties.PumpFactor = 1;
    VecProperties.UseMaskedForUnmasked = 0;
  }

  /// Setter functions for different possible states of VecScenario.
  // Scenario 1 : Serialization.
  void setShouldBeSerialized() {
    assert(VecScenario == CallVecScenarios::Undefined &&
           "Inconsistent scenario update.");
    assert(!isKernelCallOnce() &&
           "Calls with kernel-call-once attributes cannot be serialized.");
    VecScenario = CallVecScenarios::Serialization;
  }
  // Scenario 2 : Vectorization using vector library functions (like SVML).
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
  // Scenario 3 : Vectorization using SIMD vector variant.
  void setVectorizeWithVectorVariant(std::unique_ptr<VectorVariant> &VecVariant,
                                     bool UseMaskedForUnmasked = false,
                                     unsigned PumpFactor = 1) {
    assert(VecVariant && "Can't set null vector variant.");
    assert(VecScenario == CallVecScenarios::Undefined &&
           "Inconsistent scenario update.");
    assert(PumpFactor == 1 || !isKernelCallOnce() &&
                                  "Pumped vectorization of a kernel "
                                  "called-once function is not allowed.");
    VecScenario = CallVecScenarios::VectorVariant;
    VecProperties.MatchedVecVariant = VecVariant.release();
    VecProperties.UseMaskedForUnmasked = UseMaskedForUnmasked;
    VecProperties.PumpFactor = PumpFactor;
  }
  // Scenario 4 : Trivially vectorizable calls using intrinsics.
  void setVectorizeWithIntrinsic(Intrinsic::ID VectorInrinID) {
    assert(VecScenario == CallVecScenarios::Undefined &&
           "Inconsistent scenario update.");
    VecScenario = CallVecScenarios::TrivialVectorIntrinsic;
    VecProperties.VectorIntrinsic = VectorInrinID;
  }

  /// Get decision about how call will be handled by vectorizer.
  CallVecScenariosTy getVectorizationScenario() const { return VecScenario; }
  /// Get VF for which decision was last recorded.
  unsigned getVFForScenario() const { return VecProperties.VF; }
  /// Getter for vector library function.
  StringRef getVectorLibraryFunc() const {
    assert(VecScenario == CallVecScenarios::LibraryFunc &&
           "Can't get VectorLibraryFn for mismatched scenario.");
    return VecProperties.VectorLibraryFn.getValue();
  }
  /// Getters for matched vector variant.
  const VectorVariant *getVectorVariant() const {
    assert(VecScenario == CallVecScenarios::VectorVariant &&
           "Can't get VectorVariant for mismatched scenario.");
    return VecProperties.MatchedVecVariant;
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
    SmallVector<Type *, 1> TysForName;
    TysForName.push_back(getWidenedType(getType(), getVFForScenario()));
    return Intrinsic::getName(VecID, TysForName);
  }

  /// Call argument list size.
  unsigned arg_size() const {
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
  void print(raw_ostream &O) const;

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
    VecProperties.print(OS);
    OS << "\n";
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  /// Methods for supporting type inquiry through isa, cast and dyn_cast:
  static bool classof(const VPInstruction *VPI) {
    return VPI->getOpcode() == Instruction::Call;
  }

  static bool classof(const VPValue *V) {
    return isa<VPInstruction>(V) && classof(cast<VPInstruction>(V));
  }

protected:
  virtual VPCallInstruction *cloneImpl() const final {
    VPCallInstruction *Cloned = new VPCallInstruction(
        getCalledValue(), ArrayRef<VPValue *>(op_begin(), op_end() - 1),
        getUnderlyingCallInst() /* Clone gets same underlying Call */);
    Cloned->VecScenario = VecScenario;
    Cloned->VecProperties = VecProperties;
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
  VPInductionInit(VPValue *Start, VPValue *Step, unsigned Opc)
      : VPInstruction(VPInstruction::InductionInit, Start->getType(),
                      {Start, Step}),
        BinOpcode(Opc) {}

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

protected:
  // Clones VPinductionInit.
  virtual VPInductionInit *cloneImpl() const final {
    return new VPInductionInit(getOperand(0), getOperand(1), getBinOpcode());
  }

private:
  unsigned BinOpcode;
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
  VPValue *getStartValueOperand() const {
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

protected:
  // Clones VPInductionFinal.
  virtual VPInductionFinal *cloneImpl() const final {
    if (getNumOperands() == 1)
      return new VPInductionFinal(getInductionOperand());
    else if (getNumOperands() == 2)
      return new VPInductionFinal(getStartValueOperand(), getStepOperand(),
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
//
class VPReductionInit : public VPInstruction {
public:
  VPReductionInit(VPValue *Identity)
      : VPInstruction(VPInstruction::ReductionInit, Identity->getType(),
                      {Identity}) {}

  VPReductionInit(VPValue *Identity, VPValue *StartValue)
      : VPInstruction(VPInstruction::ReductionInit, Identity->getType(),
                      {Identity, StartValue}) {}

  /// Return operand that corresponds to the indentity value.
  VPValue *getIdentityOperand() const { return getOperand(0);}

  /// Return operand that corresponds to the start value. Can be nullptr for
  /// optimized reduce.
  VPValue *getStartValueOperand() const {
    return getNumOperands() > 1 ? getOperand(1) : nullptr;
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
      return new VPReductionInit(getIdentityOperand());
    else if (getNumOperands() == 2)
      return new VPReductionInit(getIdentityOperand(), getStartValueOperand());
    else
      llvm_unreachable("Too many operands.");
  }
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
        BinOpcode(BinOp), Signed(Sign) {}

  /// Constructor for optimized summation
  VPReductionFinal(unsigned BinOp, VPValue *ReducVec)
      : VPInstruction(VPInstruction::ReductionFinal, ReducVec->getType(),
                      {ReducVec}),
        BinOpcode(BinOp), Signed(false) {}

  /// Constructor for index part of min/max+index reduction.
  VPReductionFinal(unsigned BinOp, VPValue *ReducVec, VPValue *ParentExit,
                   VPReductionFinal *ParentFinal, bool Sign)
      : VPInstruction(VPInstruction::ReductionFinal, ReducVec->getType(),
                      {ReducVec, ParentExit, ParentFinal}),
        BinOpcode(BinOp), Signed(Sign) {}

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstruction *V) {
    return V->getOpcode() == VPInstruction::ReductionFinal;
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
    return getNumOperands() == 2 ? getOperand(1) : nullptr;
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

  /// Return ID of the corresponding reduce intrinsic.
  Intrinsic::ID getVectorReduceIntrinsic() const {
    switch (BinOpcode) {
    case Instruction::Add:
    case Instruction::Sub:
      return Intrinsic::experimental_vector_reduce_add;
    case Instruction::FAdd:
    case Instruction::FSub:
      return Intrinsic::experimental_vector_reduce_v2_fadd;
    case Instruction::Mul:
      return Intrinsic::experimental_vector_reduce_mul;
    case Instruction::FMul:
      return Intrinsic::experimental_vector_reduce_v2_fmul;
    case Instruction::And:
      return Intrinsic::experimental_vector_reduce_and;
    case Instruction::Or:
      return Intrinsic::experimental_vector_reduce_or;
    case Instruction::Xor:
      return Intrinsic::experimental_vector_reduce_xor;
    case VPInstruction::UMin:
      return Intrinsic::experimental_vector_reduce_umin;
    case VPInstruction::SMin:
      return Intrinsic::experimental_vector_reduce_smin;
    case VPInstruction::UMax:
      return Intrinsic::experimental_vector_reduce_umax;
    case VPInstruction::SMax:
      return Intrinsic::experimental_vector_reduce_smax;
    case VPInstruction::FMax:
      return Intrinsic::experimental_vector_reduce_fmax;
    case VPInstruction::FMin:
      return Intrinsic::experimental_vector_reduce_fmin;
    default:
      llvm_unreachable("Vector reduction opcode not supported.");
    }
  }

protected:
  // Clones VPReductionFinal.
  virtual VPReductionFinal *cloneImpl() const final {
    if (isMinMaxIndex())
      return new VPReductionFinal(
          getBinOpcode(), getReducingOperand(), getParentExitValOperand(),
          cast<VPReductionFinal>(getParentFinalValOperand()), isSigned());
    else if (getStartValueOperand() == nullptr)
      return new VPReductionFinal(getBinOpcode(), getReducingOperand());
    else
      return new VPReductionFinal(getBinOpcode(), getReducingOperand(),
                                  getStartValueOperand(), isSigned());
  }

private:
  unsigned BinOpcode;
  bool Signed;
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

/// Instruction representing the final value of the explicit IV for the vector
/// loop. We increment that IV by VF*UF, so actual value would be the iteration
/// number of the serial loop execution corresponding the lane 0 of the last
/// vector iteration.
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

// VPInstruction to allocate private memory. This is translated into
// allocation of a private memory in the function entry block. This instruction
// is not supposed to vectorize alloca instructions that appear inside the loop
// for arrays of a variable size.
class VPAllocatePrivate : public VPInstruction {
public:
  VPAllocatePrivate(Type *Ty)
      : VPInstruction(VPInstruction::AllocatePrivate, Ty, {}), IsSOASafe(false),
        IsSOAProfitable(false) {}

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

private:
  bool IsSOASafe;
  bool IsSOAProfitable;
};
#endif // INTEL_CUSTOMIZATION

/// VPlan models a candidate for vectorization, encoding various decisions take
/// to produce efficient output IR, including which branches, basic-blocks and
/// output IR instructions to generate, and their cost.
class VPlan {
  friend class VPlanPrinter;

public:
  using VPBasicBlockListTy = ilist<VPBasicBlock, ilist_sentinel_tracking<true>>;
  // VPBasicBlock iterators.
  using iterator = VPBasicBlockListTy::iterator;
  using const_iterator = VPBasicBlockListTy::const_iterator;
  using reverse_iterator = VPBasicBlockListTy::reverse_iterator;
  using const_reverse_iterator = VPBasicBlockListTy::const_reverse_iterator;

private:
  VPExternalValues &Externals;

  std::unique_ptr<VPLoopInfo> VPLInfo;
  std::unique_ptr<VPlanDivergenceAnalysis> VPlanDA;
  std::unique_ptr<VPlanScalarEvolution> VPSE;
  std::unique_ptr<VPlanValueTracking> VPVT;
  std::unique_ptr<VPlanScalVecAnalysis> VPlanSVA;

  /// Holds Plan's VPBasicBlocks.
  VPBasicBlockListTy VPBasicBlocks;

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

  /// Holds the name of the VPlan, for printing.
  std::string Name;

  /// Map: VF -> PreferredPeeling.
  std::map<unsigned, std::unique_ptr<VPlanPeelingVariant>> PreferredPeelingMap;

  DenseMap<const VPLoop *, std::unique_ptr<VPLoopEntityList>> LoopEntities;

  // Holds the instructions that need to be deleted by VPlan's destructor.
  SmallVector<std::unique_ptr<VPInstruction>, 2> UnlinkedVPInsns;

public:
  VPlan(VPExternalValues &E);

  /// Generate the IR code for this VPlan.
  void execute(struct VPTransformState *State);
#if INTEL_CUSTOMIZATION
  void executeHIR(VPOCodeGenHIR *CG);
#endif // INTEL_CUSTOMIZATION

  VPExternalValues &getExternals() { return Externals; }
  const VPExternalValues &getExternals() const { return Externals; }

  VPLoopInfo *getVPLoopInfo() { return VPLInfo.get(); }

  const VPLoopInfo *getVPLoopInfo() const { return VPLInfo.get(); }

  VPlanScalarEvolution *getVPSE() const { return VPSE.get(); }

  VPlanValueTracking *getVPVT() const { return VPVT.get(); }

  void setVPLoopInfo(std::unique_ptr<VPLoopInfo> VPLI) {
    VPLInfo = std::move(VPLI);
  }

  void setVPlanDA(std::unique_ptr<VPlanDivergenceAnalysis> VPDA) {
    VPlanDA = std::move(VPDA);
  }

  void setVPSE(std::unique_ptr<VPlanScalarEvolution> A) {
    VPSE = std::move(A);
  }

  void setVPVT(std::unique_ptr<VPlanValueTracking> A) {
    VPVT = std::move(A);
  }

  LLVMContext *getLLVMContext(void) const { return Externals.getLLVMContext(); }

  VPlanDivergenceAnalysis *getVPlanDA() const { return VPlanDA.get(); }

  void setVPlanSVA(std::unique_ptr<VPlanScalVecAnalysis> VPSVA) {
    VPlanSVA = std::move(VPSVA);
  }

  VPlanScalVecAnalysis *getVPlanSVA() const { return VPlanSVA.get(); }

  // Compute SVA results for this VPlan.
  void runSVA(unsigned VF, const TargetLibraryInfo *TLI);

  void markFullLinearizationForced() { FullLinearizationForced = true; }
  bool isFullLinearizationForced() const { return FullLinearizationForced; }

  void markBackedgeUniformityForced() {
    ForceOuterLoopBackedgeUniformity = true;
  }
  bool isBackedgeUniformityForced() const {
    return ForceOuterLoopBackedgeUniformity;
  }

  const DataLayout *getDataLayout() const { return Externals.getDataLayout(); }

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

  /// Getters for Dominator Tree
  VPDominatorTree *getDT() { return PlanDT.get(); }
  const VPDominatorTree *getDT() const { return PlanDT.get(); }
  /// Getter for Post-Dominator Tree
  VPPostDominatorTree *getPDT() { return PlanPDT.get(); }

  /// Compute the Dominator Tree for this Plan.
  void computeDT();

  /// Compute the Post-Dominator Tree for this Plan.
  void computePDT();

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

  VPBasicBlock *getEntryBlock() {
    assert(front().getNumPredecessors() == 0 &&
           "Entry block should not have predecesors.");
    return &front();
  }
  const VPBasicBlock *getEntryBlock() const {
    assert(front().getNumPredecessors() == 0 &&
           "Entry block should not have predecesors.");
    return &front();
  }

  const VPBasicBlockListTy &getVPBasicBlockList() const {
    return VPBasicBlocks;
  }
  VPBasicBlockListTy &getVPBasicBlockList() { return VPBasicBlocks; }

  void insertAtFront(VPBasicBlock *CurBB) {
    getVPBasicBlockList().push_front(CurBB);
  }

  void insertBefore(VPBasicBlock *CurBB, VPBasicBlock *MovePos) {
    getVPBasicBlockList().insert(MovePos->getIterator(), CurBB);
  }

  void insertAfter(VPBasicBlock *CurBB, VPBasicBlock *MovePos) {
    getVPBasicBlockList().insertAfter(MovePos->getIterator(), CurBB);
  }

  void insertAtBack(VPBasicBlock *CurBB) {
    getVPBasicBlockList().push_back(CurBB);
  }

  void insertBefore(VPBasicBlock *CurBB, iterator InsertBefore) {
    getVPBasicBlockList().insert(InsertBefore, CurBB);
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
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  const std::string &getName() const { return Name; }

  void setName(const Twine &newName) { Name = newName.str(); }

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

  /// Create a new VPConstant for \p Const if it doesn't exist or retrieve the
  /// existing one.
  VPConstant *getVPConstant(Constant *Const) {
    return Externals.getVPConstant(Const);
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

  /// Create or retrieve a VPExternalUse for a given Value \p ExtVal.
  VPExternalUse *getVPExternalUse(PHINode *ExtDef) {
    return Externals.getVPExternalUse(ExtDef);
  }

  /// Create or retrieve a VPExternalUse for a given non-decomposable DDRef \p
  /// DDR.
  VPExternalUse *getVPExternalUseForDDRef(const loopopt::DDRef *DDR) {
    return Externals.getVPExternalUseForDDRef(DDR);
  }

  /// Create or retrieve a VPExternalUse for an HIR IV identified by its \p
  /// IVLevel.
  VPExternalUse *getVPExternalUseForIV(unsigned IVLevel, Type *BaseTy) {
    return Externals.getVPExternalUseForIV(IVLevel, BaseTy);
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

  // Add a VPInstruction that needs to be erased in UnlinkedVPInsns vector.
  void addUnlinkedVPInst(VPInstruction *I) { UnlinkedVPInsns.emplace_back(I); }

  // Clones VPlan. VPAnalysesFactory has methods to create additional analyses
  // required for cloned VPlan. RecalculateDA indicates whether DA will be
  // calculated from scratch (true) or we will just copy instructions' vector
  // shapes (false).
  std::unique_ptr<VPlan> clone(VPAnalysesFactory &VPAF, bool RecalculateDA);

private:
  /// Add to the given dominator tree the header block and every new basic block
  /// that was created between it and the latch block, inclusive.
  static void updateDominatorTree(class DominatorTree *DT,
                                  BasicBlock *LoopPreHeaderBB,
                                  BasicBlock *LoopLatchBB);
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// VPlanPrinter prints a given VPlan to a given output stream. The printing is
/// indented and follows the dot format.
class VPlanPrinter {
  friend inline raw_ostream &operator<<(raw_ostream &OS, const VPlan &Plan);

private:
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
  void dumpBasicBlock(const VPBasicBlock *BB);

  unsigned getOrCreateBID(const VPBasicBlock *BB) {
    return BlockID.count(BB) ? BlockID[BB] : BlockID[BB] = BID++;
  }

  const Twine getOrCreateName(const VPBasicBlock *BB);

  const Twine getUID(const VPBasicBlock *BB);

  /// Print the information related to a CFG edge between two VPBasicBlocks.
  void drawEdge(const VPBasicBlock *From, const VPBasicBlock *To, bool Hidden,
                const Twine &Label);

  VPlanPrinter(raw_ostream &O, const VPlan &P) : OS(O), Plan(P) {}

  void dump();
};

inline raw_ostream &operator<<(raw_ostream &OS, const VPlan &Plan) {
  VPlanPrinter Printer(OS, Plan);
  Printer.dump();
  return OS;
}

// Set of print functions
inline raw_ostream &operator<<(raw_ostream &OS, const VPInstruction &I) {
  I.dump(OS);
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
  LoopOptReportBuilder &LORBuilder;
  // LORB needs the LoopInfo while adding remarks for llvm::Loop. This will be
  // nullptr for HLLoop.
  LoopInfo *LI;

public:
  VPlanOptReportBuilder(LoopOptReportBuilder &LORB, LoopInfo *LI = nullptr)
      : LORBuilder(LORB), LI(LI) {}

  /// Add a vectorization related remark for the HIR loop \p Lp. The remark
  /// message is identified by \p MsgID.
  template <typename... Args>
  void addRemark(loopopt::HLLoop *Lp, OptReportVerbosity::Level Verbosity,
                 unsigned MsgID, Args &&... args);

  /// Add a vectorization related remark for the LLVM loop \p Lp. The remark
  /// message is identified by \p MsgID.
  template <typename... Args>
  void addRemark(Loop *Lp, OptReportVerbosity::Level Verbosity, unsigned MsgID,
                 Args &&... args);
};

// Several inline functions to hide the #if machinery from the callers.
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
inline void VPLAN_DUMP(bool Cond, const VPlan &Plan) {
  if (!Cond)
    return;
  Plan.dump(outs());
  outs().flush();
}
inline void VPLAN_DUMP(bool Cond, const VPlan *Plan) {
  VPLAN_DUMP(Cond, *Plan);
}

inline void VPLAN_DUMP(bool Cond, StringRef Transformation, const VPlan &Plan) {
  if (!Cond)
    return;
  outs() << "VPlan after " << Transformation << ":\n";
  Plan.dump(outs());
  outs().flush();
}
inline void VPLAN_DUMP(bool Cond, StringRef Transformation, const VPlan *Plan) {
  return VPLAN_DUMP(Cond, Transformation, *Plan);
}

inline void VPLAN_DOT(bool Cond, const VPlan &Plan) {
  if (!Cond)
    return;
  outs() << Plan;
  outs().flush();
}
inline void VPLAN_DOT(bool Cond, const VPlan *Plan) { VPLAN_DOT(Cond, *Plan); }
#else
template <class... Args> inline void VPLAN_DUMP(const Args &...) {}
template <class... Args> inline void VPLAN_DOT(const Args &...) {}
#endif

} // namespace vpo

// The following template specializations are implemented to support GraphTraits
// for VPlan.
template <>
struct GraphTraits<vpo::VPlan *> : public GraphTraits<vpo::VPBasicBlock *> {

  using nodes_iterator = df_iterator<NodeRef>;

  static NodeRef getEntryNode(vpo::VPlan *Plan) {
    return Plan->getEntryBlock();
  }

  static inline nodes_iterator nodes_begin(vpo::VPlan *Plan) {
    return nodes_iterator::begin(Plan->getEntryBlock());
  }

  static inline nodes_iterator nodes_end(vpo::VPlan *Plan) {
    // df_iterator returns an empty iterator so the node used doesn't matter.
    return nodes_iterator::end(Plan->getEntryBlock());
  }

  static size_t size(vpo::VPlan *Plan) { return Plan->size(); }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLAN_H

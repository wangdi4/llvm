#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_INTELVPLAN_H 
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_INTELVPLAN_H

#include "../Intel_VPlan.h"
#include "VPLoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/GenericDomTreeConstruction.h"

namespace llvm {

// Forward declarations
class VPNewInstructionRangeRecipe;

namespace loopopt {
class HLLoop;
}

using namespace loopopt;

namespace vpo {

class VPBranchIfNotAllZeroRecipe : public VPConditionBitRecipeBase {
  friend class VPlanUtilsLoopVectorizer;

public:
  VPBranchIfNotAllZeroRecipe(Value *Cond, VPlan *Plan)
      : VPConditionBitRecipeBase(VPBranchIfNotAllZeroRecipeSC) {
    Instruction *I = dyn_cast<Instruction>(Cond);
    assert(I && "Expected Cond to be an instruction");
    Plan->setInst2Recipe(I, this);
    ConditionBit = Cond;
  }

  ~VPBranchIfNotAllZeroRecipe() {}

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPRecipeBase *V) {
    return V->getVPRecipeID() == VPRecipeBase::VPBranchIfNotAllZeroRecipeSC;
  }

  /// Print the recipe.
  void print(raw_ostream &OS, const Twine &Indent) const override {
    OS << " +\n" << Indent << "\"IfNotAllZero";
    OS << " " << *ConditionBit << "\\l\"";
  }

  void execute(struct VPTransformState &State) override {
    // TODO: vectorizing this recipe should involve generating some type of
    // intrinsic that can do a vector compare, since branch instructions cannot
    // simply be widened.
  }

  StringRef getName() const { return "Branch If Not All Zero Recipe"; }
};

class VPMaskGenerationRecipe : public VPRecipeBase {
  friend class VPlanUtilsLoopVectorizer;

private:
  const Value *IncomingPred;
  const Value *LoopBackedge;

public:
  VPMaskGenerationRecipe(const Value *Pred, const Value *Backedge)
      : VPRecipeBase(VPMaskGenerationRecipeSC), IncomingPred(Pred),
        LoopBackedge(Backedge) {}

  ~VPMaskGenerationRecipe() {}

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPRecipeBase *V) {
    return V->getVPRecipeID() == VPRecipeBase::VPMaskGenerationRecipeSC;
  }

  /// Print the recipe.
  void print(raw_ostream &OS, const Twine &Indent) const override {
    OS << " +\n" << Indent << "\"MaskGeneration";
    OS << " " << *LoopBackedge << "\\l\"";
  }

  void execute(struct VPTransformState &State) override {
    // TODO: vectorizing this recipe should involve generating a mask for the
    // instructions in the loop body. i.e., a phi instruction that has incoming
    // values using IncomingPred and LoopBackedge.
  }
};

class VPNonUniformConditionBitRecipe : public VPConditionBitRecipeBase {
  friend class VPlanUtilsLoopVectorizer;

private:
const VPMaskGenerationRecipe *MaskRecipe;

public:
  VPNonUniformConditionBitRecipe(const VPMaskGenerationRecipe *MR)
      : VPConditionBitRecipeBase(VPNonUniformBranchSC), MaskRecipe(MR) {}

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPRecipeBase *V) {
    return V->getVPRecipeID() == VPRecipeBase::VPNonUniformBranchSC;
  }

  /// The method clones a uniform instruction that calculates condition
  /// for uniform branch.
  void execute(VPTransformState &State) override {}

  /// Print the recipe.
  void print(raw_ostream &OS, const Twine &Indent) const override {
    OS << " +\n" << Indent << "\"Non-uniform branch condition";
    MaskRecipe->print(OS, Indent);
    OS << "\\l\"";
  }

  StringRef getName() const { return "Non-Uniform Cond Bit Recipe"; };
};

class VPLoopRegion : public VPRegionBlock {
  friend class IntelVPlanUtils;

private:
  // Pointer to VPLoopInfo analysis information for this loop region
  VPLoop *VPLp;

protected:
  VPLoopRegion(const unsigned char SC, const std::string &Name, VPLoop *Lp)
      : VPRegionBlock(SC, Name), VPLp(Lp) {}

public:
  VPLoopRegion(const std::string &Name, VPLoop *Lp)
      : VPRegionBlock(VPLoopRegionSC, Name), VPLp(Lp) {}

  const VPLoop *getVPLoop() const { return VPLp; }
  VPLoop *getVPLoop() { return VPLp; }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPBlockBase *B) {
    return B->getVPBlockID() == VPBlockBase::VPLoopRegionSC ||
           B->getVPBlockID() == VPBlockBase::VPLoopRegionHIRSC;
  }
};

/// \brief Specialization of VPLoopRegion that holds HIR-specific loop
/// representation (HLLoop).
///
/// Design Principle: new public member functions are not allowed. This class is
/// meant to be used only by VPlan construction and code generation (and
/// their utilities). For that reason, its interface must be private and be only
/// accessible from well-justified friendship relationships.
class VPLoopRegionHIR : private VPLoopRegion {
  friend class IntelVPlanUtils; 
  friend class VPlanVerifierHIR; 
  
private: 
  // Pointer to the underlying HLLoop.
  HLLoop *HLLp;

  VPLoopRegionHIR(const std::string &Name, VPLoop *VPLp, HLLoop *HLLp)
      : VPLoopRegion(VPLoopRegionHIRSC, Name, VPLp), HLLp(HLLp) {}

  const HLLoop *getHLLoop() const { return HLLp; }


public:
  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPBlockBase *B) {
    return B->getVPBlockID() == VPBlockBase::VPLoopRegionHIRSC;
  }
}; 

class IntelVPlan : public VPlan {

private:
  VPLoopInfo *VPLInfo;

public:
  IntelVPlan() : VPlan(IntelVPlanSC), VPLInfo(nullptr) {}

  ~IntelVPlan() {
    if (VPLInfo)
      delete (VPLInfo);
  }

  void execute(struct VPTransformState *State) override;

  VPLoopInfo *getVPLoopInfo() { return VPLInfo; }
  const VPLoopInfo *getVPLoopInfo() const { return VPLInfo; }

  void setVPLoopInfo(VPLoopInfo *VPLI) { VPLInfo = VPLI; }

  static inline bool classof(const VPlan *V) {
    return V->getVPlanID() == VPlan::IntelVPlanSC;
  }
};

/// A pure virtual class that provides the getScalarCondition() interface
/// function for accessing the scalar condition value.
class VPConditionBitRecipeWithScalar : public VPConditionBitRecipeBase {
protected:
	/// Recipe name.
  std::string Name;

public:
  VPConditionBitRecipeWithScalar(const unsigned char SC)
      : VPConditionBitRecipeBase(SC) {}

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPRecipeBase *V) {
      return V->getVPRecipeID() == VPRecipeBase::VPUniformBranchSC
          || V->getVPRecipeID() == VPRecipeBase::VPLiveInBranchSC;
  }

  /// Return the scalar condition value
  virtual Value *getScalarCondition(void) const = 0;
};

/// A VPUniformConditionBitRecipe is a VPConditionBitRecipe which supports a
/// uniform conditional branch. 
class VPUniformConditionBitRecipe : public VPConditionBitRecipeWithScalar {
public:
  VPUniformConditionBitRecipe(Value *Cond)
      : VPConditionBitRecipeWithScalar(VPUniformBranchSC) {
    ScConditionBit = Cond;
  }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPRecipeBase *V) {
    return V->getVPRecipeID() == VPRecipeBase::VPUniformBranchSC;
  }

  /// The method clones a uniform instruction that calculates condition
  /// for uniform branch.
  void execute(VPTransformState &State) override;

  /// Return the scalar condition value.
  Value *getScalarCondition(void) const override{
    return ScConditionBit;
  }

  /// Set name.
  void setName(std::string Name) { this->Name = Name; }

  /// Print the recipe.
  void print(raw_ostream &OS, const Twine &Indent) const override {
    OS << " +\n" << Indent << "\"" << Name;

    // OS << " Output: ";
    // if (ConditionBit) {
    //  OS << *ConditionBit;
    //} else {
    //  OS << "NULL";
    //}

    // OS << " Input: ";
    OS << ": ";
    if (ScConditionBit) {
      OS << *ScConditionBit;
    } else {
      OS << "NULL";
    }
    OS << "\\l\"";
  }

  StringRef getName() const { return Name; };

private:
  Value *ScConditionBit;
};


/// A VPLiveInConditionBitRecipe is a recipe for a Condition operand of 
/// a uniform conditional branch. The Condition is defined outside the loop.
class VPLiveInConditionBitRecipe : public VPConditionBitRecipeWithScalar {
public:
  VPLiveInConditionBitRecipe(Value *Cond)
      : VPConditionBitRecipeWithScalar(VPLiveInBranchSC) {
    ConditionBit = Cond;
  }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPRecipeBase *V) {
    return V->getVPRecipeID() == VPRecipeBase::VPLiveInBranchSC;
  }

  /// The method clones a uniform instruction that calculates condition
  /// for uniform branch.
  void execute(VPTransformState &State) override {}

  /// Return the scalar condition value.
  /// NOTE: Since it is a live-in CBR, the scalar ConditionBit is re-used.
  Value *getScalarCondition(void) const override {
    return ConditionBit;
  }

  /// Set name.
  void setName(std::string Name) { this->Name = Name; }

  /// Print the recipe.
  void print(raw_ostream &OS, const Twine &Indent) const override {
    OS << " +\n" << Indent << "\"" << Name;

    //OS << " Output: ";
    OS << ": ";
    if (ConditionBit) {
      OS << *ConditionBit;
    } else {
      OS << "NULL";
    }
    OS << "\\l\"";
  }

  StringRef getName() const { return Name; };
};

/// A VPConstantRecipe is a recipe which represents a constant in VPlan.
/// This recipe represents a scalar integer w/o any relation to the source IR.
/// The usage of this recipe is mainly beneficial when we need to argue about
/// new recipes altering the original structure of the code and introducing new
/// commands. e.g. consider the single-exit loop massaging, we need to
/// represent a new \phi with respect to new constant values and compares to 
/// those same values.
class VPConstantRecipe : public VPRecipeBase {
public:
  VPConstantRecipe(int val) : VPRecipeBase(VPConstantSC), val(val) {}

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPRecipeBase *V) {
    return V->getVPRecipeID() == VPRecipeBase::VPConstantSC;
  }

  /// The method clones a uniform instruction that calculates condition
  /// for uniform branch.
  void execute(VPTransformState &State) override {}

  Value *getValue(void) const {
    // TODO after vectorize.
    return nullptr;
  }

  /// Print the recipe.
  void print(raw_ostream &OS, const Twine &Indent) const override {
    OS << " +\n" << Indent << "\"Const " << val << "\\l\"";
  }

  StringRef getName() const { return "Constant: " + val; };

private:
  int val;
};

/// A VPPhiValueRecipe is a recipe which represents a new Phi in VPlan to 
/// facilitate the alteration of VPlan from its original source coded form.
/// Currently the elements of the phi are constants in-order to generate the
/// needed \phi for the single-exit loop massaging. However, this phi can be
/// further enhanced to handle any type of value.
class VPPhiValueRecipe : public VPRecipeBase {
public:
  VPPhiValueRecipe() : VPRecipeBase(VPPhiValueSC), Incoming(), Phi(nullptr) {}

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPRecipeBase *V) {
    return V->getVPRecipeID() == VPRecipeBase::VPLiveInBranchSC;
  }

  /// The method clones a uniform instruction that calculates condition
  /// for uniform branch.
  void execute(VPTransformState &State) override {}

  /// Return the phi value after vectorization.
  Value *getValue(void) const {
    return Phi;
  }

  /// Adds a new element to the resulting \phi.
  void addIncomingValue(VPConstantRecipe IncomingValue,
    VPBlockBase* IncomingBlock) {
    Incoming.push_back(IncomingPair(IncomingValue, IncomingBlock));
  }

  /// Print the recipe.
  void print(raw_ostream &OS, const Twine &Indent) const override {
    OS << " +\n" << Indent << "\"Phi ";

    for (auto item : Incoming) {
      OS << "[";
      item.first.print(OS, Indent);
      OS << ", " << item.second->getName() << "] ";
    }

    OS << "\\l\"";
  }

  StringRef getName() const { return "Phi Recipe"; };

  ~VPPhiValueRecipe() {
    Phi->deleteValue();
  }

private:
  typedef std::pair<VPConstantRecipe , VPBlockBase *> IncomingPair;
  SmallVector<IncomingPair, 4> Incoming;
  Value* Phi;
};


/// A VPCmpBitRecipe is a compare recipe which represents a compare against
/// and exact value, in our case a constant value in order to support the 
/// compares needed for the cascaded ifs in the single-exit loop massaging. 
class VPCmpBitRecipe : public VPConditionBitRecipeWithScalar {
public:
  VPCmpBitRecipe(VPPhiValueRecipe *Phi, VPConstantRecipe ConstantValue)
      : VPConditionBitRecipeWithScalar(VPCmpBitSC), Phi(Phi),
        ConstantValue(ConstantValue) {
    ConditionBit = nullptr;
  }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPRecipeBase *V) {
    return V->getVPRecipeID() == VPRecipeBase::VPCmpBitSC;
  }

  /// The method clones a uniform instruction that calculates condition
  /// for uniform branch.
  void execute(VPTransformState &State) override {
    ConditionBit = nullptr;
  }

  /// Return the scalar condition value.
  /// NOTE: Since it is a live-in CBR, the scalar ConditionBit is re-used.
  Value *getScalarCondition(void) const override {
    return ConditionBit;
  }

  /// Print the recipe.
  void print(raw_ostream &OS, const Twine &Indent) const override {
    OS << " +\n" << Indent << "\"Cmp-Bit Exact Condition: ";
    Phi->print(OS, Indent);
    OS << " == ";
    ConstantValue.print(OS, Indent);
    OS << "\\l\"";
  }

  StringRef getName() const { return Name; };

private:
  VPPhiValueRecipe* Phi;
  VPConstantRecipe ConstantValue;
};

/// ; IntelVPlanUtils class provides interfaces for the construction and
/// manipulation of a VPlan.
class IntelVPlanUtils : public VPlanUtils {

public:
  IntelVPlanUtils(IntelVPlan *Plan) : VPlanUtils(Plan) {}

  IntelVPlan *getVPlan() { return cast<IntelVPlan>(Plan); }

  /// Creates a new recipe that represents an all zeros bypass.
  vpo::VPBranchIfNotAllZeroRecipe *createBranchIfNotAllZeroRecipe(
    Instruction *Cond);

  /// Creates a new recipe that represents generation of an i1 vector to be used
  /// as a mask.
  vpo::VPMaskGenerationRecipe *createMaskGenerationRecipe(
    const Value *Pred, const Value *Backedge);

  /// Creates a new recipe that points to an i1 vector representing a
  /// non-uniform condition.
  vpo::VPNonUniformConditionBitRecipe *createNonUniformConditionBitRecipe(
    const vpo::VPMaskGenerationRecipe *MaskRecipe);

  /// Creates a new VPUniformConditionBitRecipe.
  VPUniformConditionBitRecipe *createUniformConditionBitRecipe(Value *Cond) {
    VPUniformConditionBitRecipe *newRecipe =
        new VPUniformConditionBitRecipe(Cond);
    newRecipe->setName(createUniqueName("UCBR"));
    return newRecipe;
  }

  /// Creates a new VPLiveInConditionBitRecipe.
  VPLiveInConditionBitRecipe *createLiveInConditionBitRecipe(Value *Cond) {
    VPLiveInConditionBitRecipe *newRecipe =
        new VPLiveInConditionBitRecipe(Cond);
    newRecipe->setName(createUniqueName("LiveInCBR"));
    return newRecipe;
  }

  /// Create a new VPVectorizeBooleanRecipe.
  VPVectorizeBooleanRecipe *createVectorizeBooleanRecipe(Value *Cond) {
    VPVectorizeBooleanRecipe *newRecipe =
        new VPVectorizeBooleanRecipe(VPRecipeBase::VPVectorizeBooleanSC, Cond);
    newRecipe->setName(createUniqueName("VBR"));
    return newRecipe;
  }

  /// Create a new VPUniformBooleanRecipe.
  VPUniformBooleanRecipe *createUniformBooleanRecipe(Value *Cond) {
    VPUniformBooleanRecipe *newRecipe =
        new VPUniformBooleanRecipe(VPRecipeBase::VPUniformBooleanSC, Cond);
    newRecipe->setName(createUniqueName("UBR"));
    return newRecipe;
  }

  /// Create a new VPIfTruePredicateRecipe.
  VPIfTruePredicateRecipe *
  createIfTruePredicateRecipe(VPBooleanRecipe *BR,
                              VPPredicateRecipeBase *PredecessorPredicate,
                              BasicBlock *From, BasicBlock *To) {
    VPIfTruePredicateRecipe *newRecipe =
        new VPIfTruePredicateRecipe(BR, PredecessorPredicate, From, To);
    newRecipe->setName(createUniqueName("IfT"));
    return newRecipe;
  }

  /// Create a new VPIfFalsePredicateRecipe.
  VPIfFalsePredicateRecipe *
  createIfFalsePredicateRecipe(VPBooleanRecipe *BR,
                               VPPredicateRecipeBase *PredecessorPredicate,
                               BasicBlock *From, BasicBlock *To) {
    VPIfFalsePredicateRecipe *newRecipe =
        new VPIfFalsePredicateRecipe(BR, PredecessorPredicate, From, To);
    newRecipe->setName(createUniqueName("IfF"));
    return newRecipe;
  }

  VPEdgePredicateRecipe *
  createEdgePredicateRecipe(VPPredicateRecipeBase *PredecessorPredicate,
                            BasicBlock *From, BasicBlock *To) {
    VPEdgePredicateRecipe *newRecipe =
        new VPEdgePredicateRecipe(PredecessorPredicate, From, To);
    newRecipe->setName(createUniqueName("AuxEdgeForMaskSetting"));
    return newRecipe;
  }
  /// Create a new VPBlockPredicateRecipe.
  VPBlockPredicateRecipe *createBlockPredicateRecipe(void) {
    VPBlockPredicateRecipe *newRecipe = new VPBlockPredicateRecipe();
    newRecipe->setName(createUniqueName("BP"));
    return newRecipe;
  }

  /// Returns true if the edge FromBlock->ToBlock is a back-edge.
  bool isBackEdge(const VPBlockBase *FromBlock, const VPBlockBase *ToBlock,
                  const VPLoopInfo *VPLI) {
    assert(FromBlock->getParent() == ToBlock->getParent() &&
           FromBlock->getParent() != nullptr && "Must be in same region");
    const VPLoop *FromLoop = VPLI->getLoopFor(FromBlock);
    const VPLoop *ToLoop = VPLI->getLoopFor(ToBlock);
    if (FromLoop == nullptr || ToLoop == nullptr || FromLoop != ToLoop) {
      return false;
    }
    // A back-edge is latch->header
    return (ToBlock == ToLoop->getHeader() && ToLoop->isLoopLatch(FromBlock));
  }

  /// Create a new and empty VPLoopRegion.
  VPLoopRegion *createLoopRegion(VPLoop *VPL) {
    assert (VPL && "Expected a valid VPLoop.");
    VPLoopRegion *Loop = new VPLoopRegion(createUniqueName("loop"), VPL);
    setReplicator(Loop, false /*IsReplicator*/);
    return Loop;
  }

  /// Create a new and empty VPLoopRegionHIR.
  VPLoopRegion *createLoopRegionHIR(VPLoop *VPL, HLLoop *HLLp) {
    assert (VPL && HLLp && "Expected a valid VPLoop and HLLoop.");
    VPLoopRegion *Loop =
        new VPLoopRegionHIR(createUniqueName("loop"), VPL, HLLp);
    setReplicator(Loop, false /*IsReplicator*/);
    return Loop;
  }

  /// Returns true if Block is a loop latch
  bool blockIsLoopLatch(const VPBlockBase *Block,
                        const VPLoopInfo *VPLInfo) const {

    if (const VPLoop *ParentVPL = VPLInfo->getLoopFor(Block)) {
      return ParentVPL->isLoopLatch(Block);
    }

    return false;
  }

  VPBasicBlock *splitBlock(VPBlockBase *Block, VPLoopInfo *VPLInfo,
                           VPDominatorTree &DomTree,
                           VPPostDominatorTree &PostDomTree);
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_VPLAN_INTELVPLAN_H 


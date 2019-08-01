//===- IntelVPlanValue.h - Represent Values in Vectorizer Plan ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declarations of the entities induced by Vectorization
/// Plans, e.g. the instructions the VPlan intends to generate if executed.
/// VPlan models the following entities:
/// VPValue
///  |-- VPUser
///  |    |-- VPInstruction
/// These are documented in docs/VectorizationPlan.rst.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVALUE_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVALUE_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#if INTEL_CUSTOMIZATION
#include "VPlanHIR/IntelVPlanInstructionDataHIR.h"
#endif

namespace llvm {
#if INTEL_CUSTOMIZATION
namespace vpo {
#endif

// Forward declarations.
class VPUser;

#if INTEL_CUSTOMIZATION
// Forward declaration (need them to friend them within VPInstruction)
// TODO: This needs to be refactored
class VPIfTruePredicateRecipe;
class VPIfFalsePredicateRecipe;
class VPlanPredicator;
class VPlan;
class VPLoop;
class VPExternalUse;
#endif

// This is the base class of the VPlan Def/Use graph, used for modeling the data
// flow into, within and out of the VPlan. VPValues can stand for live-ins
// coming from the input IR, instructions which VPlan will generate if executed
// and live-outs which the VPlan will need to fix accordingly.
class VPValue {
#if INTEL_CUSTOMIZATION
  // The following need access to the underlying IR Value
  // TODO: This needs to be refactored. The VP*PredicateRecipe's will disappear
  //       when they get represented with VPInstructions.
  friend class VPIfTruePredicateRecipe;
  friend class VPIfFalsePredicateRecipe;
  friend class VPlan;
  friend class VPBasicBlock;
  friend class VPlanPredicator;
  friend class VPlanHCFGBuilder;
  friend class VPOCodeGen;
  friend class VPlanDivergenceAnalysis;
  friend class VPVectorShape;
  friend class VPInstruction;
  friend class VPVLSClientMemref;
#endif

private:
  const unsigned char SubclassID; ///< Subclass identifier (for isa/dyn_cast).

#if INTEL_CUSTOMIZATION
  // TODO: This will probably be replaced by a VPType that would additionally
  // keep the number of vector elements in the resulting type as a symbolic
  // expression with VF/UF as parameters to it.

  /// Represents the "base" type of the value, i.e. without VF/UF applied.
  Type *BaseTy;

  // FIXME: To be moved out of the VPValue to some analogue of the
  // llvm::Context (probably the separate HCFG class once we refactor it out of
  // the VPlan).
  std::string Name;
#endif // INTEL_CUSTOMIZATION

  SmallVector<VPUser *, 1> Users;

  // Hold the underlying Val, if any, attached to this VPValue.
  Value *UnderlyingVal;

  // Flag to indicate the validity of underlying Value attached to this VPValue.
  // If the VPValue is modified or updated, then this should be false.
  // Current exceptional use-cases in codegen where underlying Value is still
  // used despite invalidation -
  // 1. Alignment info of load/store
  // 2. IR flags and call attributes
  bool IsUnderlyingValueValid;

  /// Replace all uses of *this with \p NewVal. If the \p Loop is not null then
  /// replacement is restricted by VPInstructions from the \p Loop.
  void replaceAllUsesWithImpl(VPValue *NewVal, VPLoop *L, bool InvalidateIR);

protected:

#if INTEL_CUSTOMIZATION
  VPValue(const unsigned char SC, Type *BaseTy, Value *UV = nullptr)
      : SubclassID(SC), BaseTy(BaseTy), UnderlyingVal(UV),
        IsUnderlyingValueValid(UV ? true : false) {
    assert(BaseTy && "BaseTy can't be null!");
    if (UV && !UV->getName().empty())
      Name = ("vp." + UV->getName()).str();
  }
#else
  VPValue(const unsigned char SC, Value *UV = nullptr)
      : SubclassID(SC), UnderlyingVal(UV) {}
#endif // INTEL_CUSTOMIZATION

  // DESIGN PRINCIPLE: Access to the underlying IR must be strictly limited to
  // the front-end and back-end of VPlan so that the middle-end is as
  // independent as possible of the underlying IR. We grant access to the
  // underlying IR using friendship. In that way, we should be able to use VPlan
  // for multiple underlying IRs (Polly?) by providing a new VPlan front-end,
  // back-end and analysis information for the new IR.

  /// Return the underlying Value attached to this VPValue.
  Value *getUnderlyingValue() const { return UnderlyingVal; }

  // Set \p Val as the underlying Value of this VPValue.
  void setUnderlyingValue(Value &Val) {
    assert(!UnderlyingVal && "Underlying Value is already set.");
    UnderlyingVal = &Val;
    IsUnderlyingValueValid = true;

    if (!Val.getName().empty())
      Name = ("vp." + Val.getName()).str();
  }

  /// Return validity of underlying Value or HIR node.
  bool isUnderlyingIRValid() const;

  /// Invalidate the underlying Value or HIR node.
  void invalidateUnderlyingIR();

public:
  /// An enumeration for keeping track of the concrete subclass of VPValue
  /// that are actually instantiated. Values of this enumeration are kept in
  /// the SubclassID field of the VPValue objects. They are used for concrete
  /// type identification.
#if INTEL_CUSTOMIZATION
  enum {
    VPValueSC,
    VPUserSC,
    VPInstructionSC,
    VPConstantSC,
    VPExternalDefSC,
    VPMetadataAsValueSC,
    VPExternalUseSC,
    VPPrivateMemorySC,
  };
#else
  enum { VPValueSC, VPUserSC, VPInstructionSC };
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
  VPValue(Type *BaseTy, Value *UV = nullptr)
      : SubclassID(VPValueSC), BaseTy(BaseTy), UnderlyingVal(UV) {
    assert(BaseTy && "BaseTy can't be null!");
  }
#endif // INTEL_CUSTOMIZATION
  VPValue(const VPValue &) = delete;
  VPValue &operator=(const VPValue &) = delete;

#if INTEL_CUSTOMIZATION
  virtual ~VPValue() {}
  // FIXME: To be replaced by a proper VPType.
  Type *getType() const { return BaseTy; }

  // FIXME: Remove this when the cost model issues are resolved (see comments
  // for VPInstruction::getCMType())
  virtual Type *getCMType() const { return nullptr; }
  // If \p BaseName starts with "vp.", set it as new name. Otherwise, prepend
  // with "vp." and set the result as new name.
  void setName(const Twine &BaseName) {
    SmallString<256> NameData;
    StringRef NameRef = BaseName.toStringRef(NameData);

    if (NameRef.empty())
      return;

    if (NameRef.startswith("vp."))
      Name = std::string(NameRef);
    else
      Name = ("vp." + NameRef).str();
  }
  StringRef getName() const {
    return Name;
  }
#endif

  /// \return an ID for the concrete type of this object.
  /// This is used to implement the classof checks. This should not be used
  /// for any other purpose, as the values may change as LLVM evolves.
  unsigned getVPValueID() const { return SubclassID; }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#if INTEL_CUSTOMIZATION
  virtual void dump(raw_ostream &OS) const { printAsOperand(OS); }
  virtual void dump() const { dump(errs()); }
  virtual void printAsOperand(raw_ostream &OS) const;
#endif
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  unsigned getNumUsers() const { return Users.size(); }
  void addUser(VPUser &User) { Users.push_back(&User); }
#if INTEL_CUSTOMIZATION
  void removeUser(const VPUser &User) {
    auto It = std::find(user_begin(), user_end(), &User);
    assert(It != user_end() && "User not found!");
    Users.erase(It);
  }

  /// Return the number of users that match \p U.
  int getNumUsersTo(const VPUser *U) const {
    return std::count(Users.begin(), Users.end(), U);
  }

  bool hasExternalUse() const {
    return std::any_of(Users.begin(), Users.end(), [](const VPUser *U) {
             return isa<VPExternalUse>(U);
           });
  }

  /// Replace all uses of *this with \p NewVal. Additionally invalidate the
  /// underlying IR if \p InvalidateIR is set. Note that this is a divergence
  /// from LLVM's RAUW where users are not "modified" after the operation,
  /// however in VPlan we invalidate the underlying value of the VPUser if
  /// applicable.
  void replaceAllUsesWith(VPValue *NewVal, bool InvalidateIR = true) {
    replaceAllUsesWithImpl(NewVal, nullptr, InvalidateIR);
  }

  /// Replace all uses of *this with \p NewVal in the \p Loop. Additionally
  /// invalidate the underlying IR if \p InvalidateIR is set.
  void replaceAllUsesWithInLoop(VPValue *NewVal, VPLoop &Loop,
                                bool InvalidateIR = true) {
    replaceAllUsesWithImpl(NewVal, &Loop, InvalidateIR);
  }
#endif // INTEL_CUSTOMIZATION

  typedef SmallVectorImpl<VPUser *>::iterator user_iterator;
  typedef SmallVectorImpl<VPUser *>::const_iterator const_user_iterator;
  typedef iterator_range<user_iterator> user_range;
  typedef iterator_range<const_user_iterator> const_user_range;

  user_iterator user_begin() { return Users.begin(); }
  const_user_iterator user_begin() const { return Users.begin(); }
  user_iterator user_end() { return Users.end(); }
  const_user_iterator user_end() const { return Users.end(); }
  user_range users() { return user_range(user_begin(), user_end()); }
  const_user_range users() const {
    return const_user_range(user_begin(), user_end());
  }
};

typedef DenseMap<Value *, VPValue *> Value2VPValueTy;
typedef DenseMap<VPValue *, Value *> VPValue2ValueTy;

raw_ostream &operator<<(raw_ostream &OS, const VPValue &V);

/// This class augments VPValue with operands which provide the inverse def-use
/// edges from VPValue's users to their defs.
class VPUser : public VPValue {
private:
  SmallVector<VPValue *, 2> Operands;

protected:
#if INTEL_CUSTOMIZATION
  VPUser(const unsigned char SC, Type *BaseTy) : VPValue(SC, BaseTy) {}
  VPUser(const unsigned char SC, ArrayRef<VPValue *> Operands, Type *BaseTy)
      : VPValue(SC, BaseTy) {
    for (VPValue *Operand : Operands)
      addOperand(Operand);
  }
  VPUser(const unsigned char SC, std::initializer_list<VPValue *> Operands,
         Type *BaseTy)
      : VPValue(SC, BaseTy) {
    for (VPValue *Operand : Operands)
      addOperand(Operand);
  }

  virtual void invalidateIR() {
    // Do nothing for VPUsers without underlying HIR. Unfortunately, this method
    // is also invoked when VPUser ctor is invoked for the construction of a
    // VPInstruction (sub-class), instead of the VPInstruction's counterpart
    // (vtable not ready at that time). However, this shouldn't be a problem
    // because the HIR is invalid by default at construction.
  }
#endif

public:
#ifndef INTEL_CUSTOMIZATION
  VPUser(const unsigned char SC) : VPValue(SC) {}
  VPUser(const unsigned char SC, std::initializer_list<VPValue *> Operands)
      : VPValue(SC) {
    for (VPValue *Operand : Operands)
      addOperand(Operand);
  }

  VPUser() : VPValue(VPValue::VPUserSC) {}
  VPUser(ArrayRef<VPValue *> Operands) : VPValue(VPValue::VPUserSC) {
    for (VPValue *Operand : Operands)
      addOperand(Operand);
  }
  VPUser(std::initializer_list<VPValue *> Operands)
      : VPUser(ArrayRef<VPValue *>(Operands)) {}
#endif // INTEL_CUSTOMIZATION

  VPUser(const VPUser &) = delete;
  VPUser &operator=(const VPUser &) = delete;

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return V->getVPValueID() >= VPUserSC &&
           V->getVPValueID() <= VPInstructionSC;
  }

  void addOperand(VPValue *Operand) {
    assert(Operand && "Operand can't be null!");
    Operands.push_back(Operand);
    Operand->addUser(*this);
  }

  unsigned getNumOperands() const { return Operands.size(); }
  inline VPValue *getOperand(unsigned N) const {
    assert(N < Operands.size() && "Operand index out of bounds");
    return Operands[N];
  }
#if INTEL_CUSTOMIZATION
  void setOperand(const unsigned Idx, VPValue *Operand) {
    assert(Idx < getNumOperands() && "Out of range");
    Operands[Idx]->removeUser(*this);
    Operands[Idx] = Operand;
    Operands[Idx]->addUser(*this);
  }

  void removeOperand(const unsigned Idx) {
    assert(Idx < getNumOperands() && "Out of range");
    Operands[Idx]->removeUser(*this);
    auto It = op_begin();
    std::advance(It, Idx);
    Operands.erase(It);
  }
  /// Return the number of operands that match \p Op.
  int getNumOperandsFrom(const VPValue *Op) const {
    return std::count(op_begin(), op_end(), Op);
  }
  /// Replace all uses of operand \From by \To. Additionally invalidate the
  /// underlying IR if \p InvalidateIR is set.
  void replaceUsesOfWith(VPValue *From, VPValue *To, bool InvalidateIR = true) {
    for (int I = 0, E = getNumOperands(); I != E; ++I)
      if (getOperand(I) == From)
        setOperand(I, To);

    if (InvalidateIR)
      invalidateUnderlyingIR();
  }

  /// Return index of a given \p Operand.
  int getOperandIndex(const VPValue *Operand) const {
    auto It = llvm::find(make_range(op_begin(), op_end()), Operand);
    if (It != op_end())
      return std::distance(op_begin(), It);
    return -1;
  }
#endif // INTEL_CUSTOMIZATION

  typedef SmallVectorImpl<VPValue *>::iterator operand_iterator;
  typedef SmallVectorImpl<VPValue *>::const_iterator const_operand_iterator;
  typedef iterator_range<operand_iterator> operand_range;
  typedef iterator_range<const_operand_iterator> const_operand_range;

  operand_iterator op_begin() { return Operands.begin(); }
  const_operand_iterator op_begin() const { return Operands.begin(); }
  operand_iterator op_end() { return Operands.end(); }
  const_operand_iterator op_end() const { return Operands.end(); }
  operand_range operands() { return operand_range(op_begin(), op_end()); }
  const_operand_range operands() const {
    return const_operand_range(op_begin(), op_end());
  }
};

#if INTEL_CUSTOMIZATION
/// This class augments VPValue with constant operands that encapsulates LLVM
/// Constant information. In the same way as LLVM Constant, VPConstant is
/// immutable (once created they never change) and are fully shared by
/// structural equivalence (e.g. i32 7 == i32 7, but i32 7 != i64 7). This means
/// that two structurally equivalent VPConstants will always have the same
/// address.
// TODO: At this point, to-be-kept-scalar and to-be-widened instances of the
// same input Constant are represented with the same VPConstant because the
// input is the same Constant. Currently, we assume that there is a single VL
// that is applied to everything within VPlan and CG makes the right
// widening/scalarizing decisions. The idea is to progressively model those CG
// decisions in early stages of VPlan and, for that, we will need VPType or
// similar. When a VPConstant has a VPType, the latter would be part of the
// structural equivalence and both to-be-kept-scalar and to-be-widened
// constants will be represented with two different VPConstants.
class VPConstant : public VPValue {
  // VPlan is currently the context where we hold the pool of VPConstants.
  friend class VPlan;
  friend class VPlanDivergenceAnalysis;

protected:
  VPConstant(Constant *Const)
      : VPValue(VPValue::VPConstantSC, Const->getType(), Const) {}

public:
  VPConstant(const VPConstant &) = delete;
  VPConstant &operator=(const VPConstant &) const = delete;

  /// Return the underlying Constant attached to this VPConstant. This interface
  /// is similar to getValue() but hides the cast when we are working with
  /// VPConstant pointers.
  Constant *getConstant() const {
    assert(isa<Constant>(getUnderlyingValue()) &&
           "Expected Constant as underlying Value.");
    return cast<Constant>(getUnderlyingValue());
  }

  /// Return true if underlying Constant is a constant integer.
  bool isConstantInt() const { return isa<ConstantInt>(getUnderlyingValue()); }

  /// Return the zero-extended value of underlying Constant. ZExt value exists
  /// only for constant integers.
  unsigned getZExtValue() const {
    assert(isConstantInt() &&
           "ZExt value cannot be obtained for non-constant integers.");
    return cast<ConstantInt>(getUnderlyingValue())->getZExtValue();
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printAsOperand(raw_ostream &OS) const override {
    getUnderlyingValue()->printAsOperand(OS);
  }
  void dump(raw_ostream &OS) const override { printAsOperand(OS); }
  void dump() const override { dump(errs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return V->getVPValueID() == VPConstantSC;
  }
};

/// This class augments VPValue with definitions that happen outside of the
/// top region represented in VPlan. Similar to VPConstants and Constants,
/// VPExternalDefs are immutable (once created they never change) and are fully
/// shared by structural equivalence (e.g. i32 %param0 == i32 %param0). They
/// must be created through the VPlan::getVPExternalDef interface, to guarantee
/// that only once instance of each external definition is created.
class VPExternalDef : public VPValue, public FoldingSetNode {
  // VPlan is currently the context where the pool of VPExternalDefs is held.
  friend class VPlan;
  friend class VPOCodeGenHIR;
  friend class VPOCodeGen;

private:
  // Hold the DDRef or IV information related to this external definition.
  std::unique_ptr<VPOperandHIR> HIROperand;

  // Construct a VPExternalDef given a Value \p ExtVal.
  VPExternalDef(Value *ExtVal)
      : VPValue(VPValue::VPExternalDefSC, ExtVal->getType(), ExtVal) {
  }

  // Construct a VPExternalDef given an underlying DDRef \p DDR.
  VPExternalDef(const loopopt::DDRef *DDR)
      : VPValue(VPValue::VPExternalDefSC, DDR->getDestType()),
        HIROperand(new VPBlob(DDR)) {}

  // Construct a VPExternalDef given an underlying IV level \p IVLevel.
  VPExternalDef(unsigned IVLevel, Type *BaseTy)
      : VPValue(VPValue::VPExternalDefSC, BaseTy),
        HIROperand(new VPIndVar(IVLevel)) {}

  // DESIGN PRINCIPLE: Access to the underlying IR must be strictly limited to
  // the front-end and back-end of VPlan so that the middle-end is as
  // independent as possible of the underlying IR. We grant access to the
  // underlying IR using friendship.

  /// Return the HIR operand for this VPExternalDef.
  const VPOperandHIR *getOperandHIR() const { return HIROperand.get(); };

public:
  VPExternalDef() = delete;
  VPExternalDef(const VPExternalDef &) = delete;
  VPExternalDef &operator=(const VPExternalDef &) const = delete;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printAsOperand(raw_ostream &OS) const {
    if (getUnderlyingValue())
      getUnderlyingValue()->printAsOperand(OS);
    else {
      getType()->print(OS);
      OS << " ";
      HIROperand->print(OS);
    }
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return V->getVPValueID() == VPExternalDefSC;
  }

  /// Method to support FoldingSet's hashing.
  void Profile(FoldingSetNodeID &ID) const { HIROperand->Profile(ID); }
};

/// Concrete class for an external use.
class VPExternalUse : public VPUser, public FoldingSetNode {
private:
  friend class VPlan;
  friend class VPOCodeGen;

  // Hold the DDRef or IV information related to this external use.
  std::unique_ptr<VPOperandHIR> HIROperand;

  // Construct a VPExternalUse given a Value \p ExtVal.
  VPExternalUse(Value *ExtVal)
      : VPUser(VPValue::VPExternalUseSC, ExtVal->getType()) {
    setUnderlyingValue(*ExtVal);
  }
  // Construct a VPExternalUse given an underlying DDRef \p DDR.
  VPExternalUse(const loopopt::DDRef *DDR)
      : VPUser(VPValue::VPExternalUseSC, DDR->getDestType()),
        HIROperand(new VPBlob(DDR)) {}
  // Construct a VPExternalUse given an underlying IV level \p IVLevel.
  VPExternalUse(unsigned IVLevel, Type *BaseTy)
      : VPUser(VPValue::VPExternalUseSC, BaseTy),
        HIROperand(new VPIndVar(IVLevel)) {}

  // DESIGN PRINCIPLE: Access to the underlying IR must be strictly limited to
  // the front-end and back-end of VPlan so that the middle-end is as
  // independent as possible of the underlying IR. We grant access to the
  // underlying IR using friendship.

  /// Return the HIR operand for this VPExternalDef.
  const VPOperandHIR *getOperandHIR() const { return HIROperand.get(); };

public:
  VPExternalUse() = delete;
  VPExternalUse(const VPExternalUse &) = delete;
  VPExternalUse &operator=(const VPExternalUse &) = delete;

  /// \brief Methods for supporting type inquiry through isa, cast, and
  /// dyn_cast:
  static bool classof(const VPValue *V) {
    return V->getVPValueID() == VPExternalUseSC;
  }

  /// Method to support FoldingSet's hashing.
  void Profile(FoldingSetNodeID &ID) const { HIROperand->Profile(ID); }

  /// Adds operand with an underlying value. The underlying value points to the
  /// value which should be replaced by the new one generated from vector code.
  /// The VPOperands can be replaced during vector transformations but the
  /// underlying values shoud be kept as they point to the values outside of the
  /// loop.
  /// The VPUser::addOperand() should not be used with VPExternalUse. This
  /// method is created to avoid virtual-ness on addOperand().
  void addOperandWithUnderlyingValue(VPValue *Op, Value *Underlying) {
    assert(isConsistent() && "Inconsistent VPExternalUse operands");
    addOperand(Op);
    UnderlyingOperands.push_back(Underlying);
  }

  // TODO: add this call to verification
  bool isConsistent() const {
    return getNumOperands() == UnderlyingOperands.size();
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const override {
    if (getUnderlyingValue())
      cast<Instruction>(getUnderlyingValue())->print(OS);
    for (unsigned I = 0, E = getNumOperands(); I < E; ++I) {
      getOperand(I)->printAsOperand(OS);
      OS << " -> ";
      getUnderlyingOperand(I)->printAsOperand(OS);
      OS << ";\n";
    }
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  const Value *getUnderlyingOperand(unsigned N) const {
    assert(N < UnderlyingOperands.size() && "Operand index out of bounds");
    return UnderlyingOperands[N];
  }

  SmallVector<Value *, 2> UnderlyingOperands;
};

/// This class augments VPValue with Metadata that is used as operand of another
/// VPValue class. It contains a pointer to the underlying MetadataAsValue.
class VPMetadataAsValue : public VPValue {
  // VPlan is currently the context where we hold the pool of
  // VPMetadataAsValues.
  friend class VPlan;

protected:
  VPMetadataAsValue(MetadataAsValue *MDAsValue)
      : VPValue(VPValue::VPMetadataAsValueSC, MDAsValue->getType(), MDAsValue) {
  }

  /// Return the underlying MetadataAsValue.
  MetadataAsValue *getMetadataAsValue() {
    assert(isa<MetadataAsValue>(getUnderlyingValue()) &&
           "Expected MetadataAsValue as underlying Value.");
    return cast<MetadataAsValue>(getUnderlyingValue());
  }

  /// Return the Metadata of the underlying MetadataAsValue.
  Metadata *getMetadata() { return getMetadataAsValue()->getMetadata(); }

public:
  VPMetadataAsValue(const VPMetadataAsValue &) = delete;
  VPMetadataAsValue &operator=(const VPMetadataAsValue &) const = delete;

  // Structural comparators.
  bool operator==(const VPMetadataAsValue &C) const {
    return getUnderlyingValue() == C.getUnderlyingValue();
  };
  bool operator<(const VPMetadataAsValue &C) const {
    return getUnderlyingValue() < C.getUnderlyingValue();
  };

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printAsOperand(raw_ostream &OS) const override {
    getUnderlyingValue()->printAsOperand(OS);
  }
  void dump(raw_ostream &OS) const override { printAsOperand(OS); }
  void dump() const override { dump(errs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return V->getVPValueID() == VPMetadataAsValueSC;
  }
};

} // namespace vpo
#endif // INTEL_CUSTOMIZATION
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVALUE_H

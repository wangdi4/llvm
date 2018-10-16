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
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#if INTEL_CUSTOMIZATION
#include "llvm/IR/Constant.h"
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
#endif

private:
  const unsigned char SubclassID; ///< Subclass identifier (for isa/dyn_cast).

  SmallVector<VPUser *, 1> Users;

protected:
  // Hold the underlying Val, if any, attached to this VPValue.
  Value *UnderlyingVal;

  VPValue(const unsigned char SC, Value *UV = nullptr)
      : SubclassID(SC), UnderlyingVal(UV) {}

  // DESIGN PRINCIPLE: Access to the underlying IR must be strictly limited to
  // the front-end and back-end of VPlan so that the middle-end is as
  // independent as possible of the underlying IR. We grant access to the
  // underlying IR using friendship. In that way, we should be able to use VPlan
  // for multiple underlying IRs (Polly?) by providing a new VPlan front-end,
  // back-end and analysis information for the new IR.

  /// Return the underlying Value attached to this VPValue.
  Value *getUnderlyingValue() { return UnderlyingVal; }

  // Set \p Val as the underlying Value of this VPValue.
  void setUnderlyingValue(Value *Val) {
    assert(!UnderlyingVal && "Underlying Value is already set.");
    UnderlyingVal = Val;
  }

public:
  /// An enumeration for keeping track of the concrete subclass of VPValue
  /// that are actually instantiated. Values of this enumeration are kept in
  /// the SubclassID field of the VPValue objects. They are used for concrete
  /// type identification.
#if INTEL_CUSTOMIZATION
  enum { VPValueSC, VPUserSC, VPInstructionSC, VPConstantSC };
#else
  enum { VPValueSC, VPUserSC, VPInstructionSC };
#endif

  VPValue(Value *UV = nullptr) : VPValue(VPValueSC, UV) {}
  VPValue(const VPValue &) = delete;
  VPValue &operator=(const VPValue &) = delete;
#if INTEL_CUSTOMIZATION
  virtual ~VPValue() {}
  // FIXME: To be replaced by a proper VPType.
  virtual Type *getType() const {
    return nullptr;
  }
#endif

  /// \return an ID for the concrete type of this object.
  /// This is used to implement the classof checks. This should not be used
  /// for any other purpose, as the values may change as LLVM evolves.
  unsigned getVPValueID() const { return SubclassID; }
#if INTEL_CUSTOMIZATION
  virtual void dump(raw_ostream &OS) const { printAsOperand(OS); }
  virtual void dump() const { dump(errs()); }
  virtual
#endif
  void printAsOperand(raw_ostream &OS) const {
    OS << "%vp" << (unsigned short)(unsigned long long)this;
  }

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
  VPUser(const unsigned char SC) : VPValue(SC) {}
#if INTEL_CUSTOMIZATION
  VPUser(const unsigned char SC, ArrayRef<VPValue *> Operands) : VPValue(SC) {
    for (VPValue *Operand : Operands)
      addOperand(Operand);
  }
#endif
  VPUser(const unsigned char SC,
         std::initializer_list<VPValue *> Operands) : VPValue(SC) {
    for (VPValue *Operand : Operands)
      addOperand(Operand);
  }

#if INTEL_CUSTOMIZATION
  virtual void invalidateHIR() {
    // Do nothing for VPUsers without underlying HIR. Unfortunately, this method
    // is also invoked when VPUser ctor is invoked for the construction of a
    // VPInstruction (sub-class), instead of the VPInstruction's counterpart
    // (vtable not ready at that time). However, this shouldn't be a problem
    // because the HIR is invalid by default at construction.
  }
#endif

public:
  VPUser() : VPValue(VPValue::VPUserSC) {}
  VPUser(ArrayRef<VPValue *> Operands) : VPValue(VPValue::VPUserSC) {
    for (VPValue *Operand : Operands)
      addOperand(Operand);
  }
  VPUser(std::initializer_list<VPValue *> Operands)
      : VPUser(ArrayRef<VPValue *>(Operands)) {}
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

protected:
  VPConstant(Constant *Const)
      : VPValue(VPValue::VPConstantSC, Const) {}

  /// Return the underlying Constant attached to this VPConstant. This interface
  /// is similar to getValue() but hides the cast when we are working with
  /// VPConstant pointers.
  Constant *getConstant() {
    assert(isa<Constant>(UnderlyingVal) &&
           "Expected Constant as underlying Value.");
    return cast<Constant>(UnderlyingVal);
  }

public:
  VPConstant(const VPConstant &) = delete;
  VPConstant &operator=(const VPConstant &) const = delete;

  // Structural comparators.
  bool operator==(const VPConstant &C) const {
    return UnderlyingVal == C.UnderlyingVal;
  };
  bool operator<(const VPConstant &C) const {
    return UnderlyingVal < C.UnderlyingVal;
  };

  void printAsOperand(raw_ostream &OS) const override {
    UnderlyingVal->printAsOperand(OS);
  }
  void dump(raw_ostream &OS) const override { printAsOperand(OS); }
  void dump() const override { dump(errs()); }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return V->getVPValueID() == VPConstantSC;
  }
};
} // namespace vpo
#endif // INTEL_CUSTOMIZATION
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVALUE_H

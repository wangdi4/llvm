//===- VPlanValue.h - Represent Values in Vectorizer Plan -----------------===//
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

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VALUE_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VALUE_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#if INTEL_CUSTOMIZATION
#include "llvm/IR/Constant.h"
#endif

namespace llvm {
namespace vpo {

// Forward declarations.
class VPUser;

#if INTEL_CUSTOMIZATION
// Forward declaration (need them to friend them within VPInstruction)
// TODO: This needs to be refactored
class VPIfTruePredicateRecipe;
class VPIfFalsePredicateRecipe;
namespace vpo {
class VPlanPredicator;
class IntelVPlan;
}
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
  friend class IntelVPlan;
  friend class VPlanPredicator;
  friend class VPlanHCFGBuilder;
#endif

private:
  const unsigned char SubclassID; ///< Subclass identifier (for isa/dyn_cast).

  SmallVector<VPUser *, 1> Users;

protected:
  VPValue(const unsigned char SC) : SubclassID(SC) {}

  /// Return the underlying Value attached to this VPInstruction. If there
  /// is no Value attached, it returns null.
  virtual Value *getValue() {
    // FIXME: We are currently creating VPValue objects to wrap some unknown
    // LLVM Values. Make this method pure when VPValue is turned into an
    // abstract class.
    return nullptr;
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

  VPValue() : SubclassID(VPValueSC) {}
  VPValue(const VPValue &) = delete;
  VPValue &operator=(const VPValue &) = delete;
#if INTEL_CUSTOMIZATION
  virtual ~VPValue() {}
#endif

  /// \return an ID for the concrete type of this object.
  /// This is used to implement the classof checks. This should not be used
  /// for any other purpose, as the values may change as LLVM evolves.
  unsigned getVPValueID() const { return SubclassID; }
#if INTEL_CUSTOMIZATION
  virtual
#endif
  void printAsOperand(raw_ostream &OS) const {
    OS << "%vp" << (unsigned short)(unsigned long long)this;
  }

  unsigned getNumUsers() const { return Users.size(); }
  void addUser(VPUser &User) { Users.push_back(&User); }

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

#if INTEL_CUSTOMIZATION
  // Regarding 'addOperand', the equivalent function in LLVM user ('setOperand')
  // is public and we need the same in VPO.
public:
#endif
  void addOperand(VPValue *Operand) {
    Operands.push_back(Operand);
    Operand->addUser(*this);
  }
#if INTEL_CUSTOMIZATION
  // Adding 'private' back in case more members are added after 'addOperand' in
  // the future.
private:
#endif

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

  unsigned getNumOperands() const { return Operands.size(); }
  inline VPValue *getOperand(unsigned N) const {
    assert(N < Operands.size() && "Operand index out of bounds");
    return Operands[N];
  }

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

private:
  Constant *Const;

protected:
  VPConstant(Constant *Const) : VPValue(VPValue::VPConstantSC), Const(Const) {}

  Value *getValue() override { return Const; }
  /// Return the underlying Constant attached to this VPConstant. This interface
  /// is similar to getValue() but allows to avoid the cast when we are working
  /// with VPConstant pointers.
  Constant *getConstant() { return Const; }

public:
  VPConstant(const VPConstant &) = delete;
  VPConstant &operator=(const VPConstant &) const = delete;

  // Structural comparators.
  bool operator==(const VPConstant &C) const { return Const == C.Const; };
  bool operator<(const VPConstant &C) const { return Const < C.Const; };

  void printAsOperand(raw_ostream &OS) const override {
    Const->printAsOperand(OS);
  }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return V->getVPValueID() == VPConstantSC;
  }
};
#endif // INTEL_CUSTOMIZATION
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VALUE_H

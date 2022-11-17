//===- IntelVPlanValue.h - Represent Values in Vectorizer Plan ------------===//
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2021 Intel Corporation
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

#include "VPlanHIR/IntelVPlanInstructionDataHIR.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <climits>

namespace llvm {
namespace vpo {

// Forward declarations.
class VPUser;

// Forward declaration (need them to friend them within VPInstruction)
// TODO: This needs to be refactored
class VPBasicBlock;
class VPlanPredicator;
class VPlan;
class VPLoop;
class VPExternalUse;

// This is the base class of the VPlan Def/Use graph, used for modeling the data
// flow into, within and out of the VPlan. VPValues can stand for live-ins
// coming from the input IR, instructions which VPlan will generate if executed
// and live-outs which the VPlan will need to fix accordingly.
class VPValue {
  // The following need access to the underlying IR Value
  friend class VPlan;
  friend class VPBasicBlock;
  friend class VPlanPredicator;
  friend class VPlanHCFGBuilder;
  friend class VPOCodeGen;
  friend class VPlanDivergenceAnalysis;
  friend class VPVectorShape;
  friend class VPInstruction;
  friend class VPVLSClientMemref;
  friend class VPlanScalarEvolutionLLVM;
  friend class VPLoopEntityList;
  friend class IndirectCallCodeGenerator;
  friend class VPLiveInOutCreator;
  friend class VPlanValueTrackingLLVM;
  friend class VPlanValueTrackingHIR;

private:
  const unsigned char SubclassID; ///< Subclass identifier (for isa/dyn_cast).

  // TODO: This will probably be replaced by a VPType that would additionally
  // keep the number of vector elements in the resulting type as a symbolic
  // expression with VF/UF as parameters to it.

  /// Represents the "base" type of the value, i.e. without VF/UF applied.
  Type *BaseTy;

  // FIXME: To be moved out of the VPValue to some analogue of the
  // llvm::Context (probably the separate HCFG class once we refactor it out of
  // the VPlan).
  std::string Name;

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

protected:

  VPValue(const unsigned char SC, Type *BaseTy, Value *UV = nullptr)
      : SubclassID(SC), BaseTy(BaseTy), UnderlyingVal(UV),
        IsUnderlyingValueValid(UV ? true : false) {
    assert(BaseTy && "BaseTy can't be null!");
    if (UV && !UV->getName().empty())
      Name = (getVPNamePrefix() + UV->getName()).str();
  }

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

    if (!Val.getName().empty() && getName().empty())
      Name = (getVPNamePrefix() + Val.getName()).str();
  }

public:
  /// An enumeration for keeping track of the concrete subclass of VPValue
  /// that are actually instantiated. Values of this enumeration are kept in
  /// the SubclassID field of the VPValue objects. They are used for concrete
  /// type identification.
  enum {
    VPValueSC,
    VPUserSC,
    VPInstructionSC,
    VPConstantSC,
    VPExternalDefSC,
    VPMetadataAsValueSC,
    VPExternalUseSC,
    VPPrivateMemorySC,
    VPBasicBlockSC,
    VPLiveInValueSC,
    VPLiveOutValueSC,
    VPRegionSC,
    VPRegionLiveOutSC,
    VPTemporaryUserSC,
  };

  VPValue(Type *BaseTy, Value *UV = nullptr)
      : SubclassID(VPValueSC), BaseTy(BaseTy), UnderlyingVal(UV) {
    assert(BaseTy && "BaseTy can't be null!");
  }
  VPValue(const VPValue &) = delete;
  VPValue &operator=(const VPValue &) = delete;

  virtual ~VPValue() {
    // TODO: VPExternalUses need to be redesigned. They use operands and thus
    // part of def-use chains but they are not explicitly represented in VPlan
    // CFG. Change the assert below after addressing this issue.
    assert((getNumUsers() == 0 || hasOnlyExternalUses()) &&
           "VPValue being deleted should not have any users.");
  }

  // FIXME: To be replaced by a proper VPType.
  Type *getType() const { return BaseTy; }

  // If \p BaseName starts with "vp.", set it as new name. Otherwise, prepend
  // with "vp." and set the result as new name.
  void setName(const Twine &BaseName) {
    SmallString<256> NameData;
    StringRef NameRef = BaseName.toStringRef(NameData);

    if (NameRef.empty())
      return;

    if (NameRef.startswith(getVPNamePrefix()))
      Name = std::string(NameRef);
    else
      Name = (getVPNamePrefix() + NameRef).str();
  }
  StringRef getName() const {
    return Name;
  }
  /// Return the VPNamePrefix to clients so that proper VPValue-names can be
  /// generated.
  StringRef getVPNamePrefix() const;

  /// Return the original llvm::Value name so that codegen clients can generate
  /// appropriate names in output IR.
  StringRef getOrigName() const {
    StringRef NameRef(Name);

    if (NameRef.startswith(getVPNamePrefix()))
      return NameRef.substr(getVPNamePrefix().size());
    else
      return NameRef;
  }

  /// \return an ID for the concrete type of this object.
  /// This is used to implement the classof checks. This should not be used
  /// for any other purpose, as the values may change as LLVM evolves.
  unsigned getVPValueID() const { return SubclassID; }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  virtual void print(raw_ostream &OS) const { printAsOperand(OS); }
  void dump() const { print(errs()); errs()<< '\n'; }
  virtual void printAsOperand(raw_ostream &OS) const;
  void printAsOperandNoType(raw_ostream &OS) const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  unsigned getNumUsers() const { return Users.size(); }
  void addUser(VPUser &User) { Users.push_back(&User); }
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

  /// Helper utility to identify if all users of this VPValue are VPExternalUse.
  bool hasOnlyExternalUses() const {
    return llvm::all_of(users(),
                        [](const VPUser *U) { return isa<VPExternalUse>(U); });
  }

  /// Replace all uses of this with \p NewVal if a ShouldReplace condition is
  /// true. Additionally invalidate the underlying IR if \p InvalidateIR is set.
  void replaceUsesWithIf(VPValue *NewVal,
                         llvm::function_ref<bool(VPUser *U)> ShouldReplace,
                         bool InvalidateIR = true);

  /// Replace all uses of *this with \p NewVal in the \p VPBB. Additionally
  /// invalidate the underlying IR if \p InvalidateIR is set.
  void replaceAllUsesWithInBlock(VPValue *NewVal, VPBasicBlock &VPBB,
                                 bool InvalidateIR = true);

  /// Replace all uses of *this with \p NewVal in the \p Loop. Additionally
  /// invalidate the underlying IR if \p InvalidateIR is set.
  void replaceAllUsesWithInLoop(VPValue *NewVal, VPLoop &Loop,
                                bool InvalidateIR = true);

  /// Replace all uses of *this with \p NewVal in the \p Region. Region is a
  /// collection of BBs. Additionally invalidate the underlying IR if \p
  /// InvalidateIR is set.
  void replaceAllUsesWithInRegion(VPValue *NewVal,
                                  ArrayRef<VPBasicBlock *> Region,
                                  bool InvalidateIR = true);

  /// Replace all uses of *this with \p NewVal. Additionally invalidate the
  /// underlying IR if \p InvalidateIR is set.
  void replaceAllUsesWith(VPValue *NewVal, bool InvalidateIR = true);

  /// Return validity of underlying Value or HIR node.
  bool isUnderlyingIRValid() const;

  /// Invalidate the underlying Value or HIR node.
  void invalidateUnderlyingIR();

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

public:
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
  void removeAllOperands() {
    int NumOps = getNumOperands();
    for (int Idx = 0; Idx < NumOps; ++Idx)
      removeOperand(NumOps - 1 - Idx);
  }
  /// Return the number of operands that match \p Op.
  int getNumOperandsFrom(const VPValue *Op) const {
    return std::count(op_begin(), op_end(), Op);
  }
  /// Replace all uses of operand \From by \To, invalidating the underlying IR
  /// if \p InvalidateIR is true.
  void replaceUsesOfWith(VPValue *From, VPValue *To, bool InvalidateIR = true) {
    for (int I = 0, E = getNumOperands(); I != E; ++I)
      if (getOperand(I) == From) {
        setOperand(I, To);
        if (InvalidateIR)
          invalidateUnderlyingIR();
      }
  }

  /// Return index of a given \p Operand.
  int getOperandIndex(const VPValue *Operand) const {
    auto It = llvm::find(make_range(op_begin(), op_end()), Operand);
    if (It != op_end())
      return std::distance(op_begin(), It);
    return -1;
  }

  /// This drops all operand uses from this instruction, which is an essential
  /// step in breaking cyclic dependences between references when they are to be
  /// deleted.
  void dropAllReferences() {
    while (getNumOperands())
      removeOperand(0);
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

  using reverse_operand_iterator = SmallVectorImpl<VPValue *>::reverse_iterator;
  using const_reverse_operand_iterator =
      SmallVectorImpl<VPValue *>::const_reverse_iterator;
  reverse_operand_iterator op_rbegin() { return Operands.rbegin(); }
  reverse_operand_iterator op_rend() { return Operands.rend(); }
  const_reverse_operand_iterator op_rbegin() const { return Operands.rbegin(); }
  const_reverse_operand_iterator op_rend() const { return Operands.rend(); }
};

template <unsigned Opcode> class VPProxyUser : public VPUser {
public:
  VPProxyUser(VPValue *Operand)
      : VPUser(Opcode, {Operand}, Operand->getType()) {}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS) const override { OS << *getOperand(0); }
  friend raw_ostream &operator<<(raw_ostream &OS,
                                 const VPProxyUser<Opcode> &VPU) {
    VPU.print(OS);
    return OS;
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  static inline bool classof(const VPValue *V) {
    return V->getVPValueID() == Opcode;
  }

  // VPUser's destructor won't drop references.
  ~VPProxyUser() { dropAllReferences(); }
};

using VPRegionLiveOut = VPProxyUser<VPValue::VPRegionLiveOutSC>;
using VPTemporaryUser = VPProxyUser<VPValue::VPTemporaryUserSC>;

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
  friend class VPExternalValues;

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
  uint64_t getZExtValue() const {
    assert(isConstantInt() &&
           "ZExt value cannot be obtained for non-constant integers.");
    return cast<ConstantInt>(getUnderlyingValue())->getZExtValue();
  }

  int64_t getSExtValue() const {
    assert(isConstantInt() &&
           "SExt value cannot be obtained for non-constant integers.");
    return cast<ConstantInt>(getUnderlyingValue())->getSExtValue();
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printAsOperand(raw_ostream &OS) const override {
    getUnderlyingValue()->printAsOperand(OS);
  }
  void print(raw_ostream &OS) const override { printAsOperand(OS); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return V->getVPValueID() == VPConstantSC;
  }
};

/// VPConstantInt is subset of VPConstant representing Integer constants only.
class VPConstantInt : public VPConstant {
  // VPlan is currently the context where we hold the pool of VPConstants.
  friend class VPExternalValues;

protected:
  VPConstantInt(ConstantInt *ConstInt) : VPConstant(ConstInt) {}

public:
  VPConstantInt(const VPConstantInt &) = delete;
  VPConstantInt &operator=(const VPConstantInt &) const = delete;

  /// Return the underlying Constant attached to this VPConstant. This interface
  /// is similar to getValue() but hides the cast when we are working with
  /// VPConstant pointers.
  ConstantInt *getConstantInt() const {
    assert(isa<ConstantInt>(getUnderlyingValue()) &&
           "Expected Constant as underlying Value.");
    return cast<ConstantInt>(getUnderlyingValue());
  }

  /// Return the APInt value of underlying Constant.
  const APInt &getValue() const {
    return getConstantInt()->getValue();
  }

  /// This function will return true iff every bit in this constant is set to true.
  // Adapted from Constant.h
  bool isMinusOne() const {
    return getConstant()->isAllOnesValue();
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printAsOperand(raw_ostream &OS) const override {
    getUnderlyingValue()->printAsOperand(OS);
  }
  void print(raw_ostream &OS) const override { printAsOperand(OS); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    auto *C = dyn_cast<VPConstant>(V);
    return C && C->isConstantInt();
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
  friend class VPExternalValues;
  friend class VPLiveInOutCreator;
  friend class VPOCodeGenHIR;
  friend class VPOCodeGen;
  friend class HIROperandSpecifics;

private:
  // Hold the DDRef or IV information related to this external definition.
  std::unique_ptr<VPOperandHIR> HIROperand;

  // Construct a VPExternalDef given a Value \p ExtVal.
  VPExternalDef(Value *ExtVal)
      : VPValue(VPValue::VPExternalDefSC, ExtVal->getType(), ExtVal) {}

  // Construct a VPExternalDef given an underlying DDRef \p DDR.
  VPExternalDef(const loopopt::DDRef *DDR)
      : VPValue(VPValue::VPExternalDefSC, DDR->getDestType()),
        HIROperand(new VPBlob(DDR)) {
    setName(getOperandHIR()->getName());
  }

  // Construct a VPExternalDef for blob with index \p BI in \p DDR. \p BType
  // specifies the blob type.
  VPExternalDef(const loopopt::RegDDRef *DDR, unsigned BI, Type *BType)
      : VPValue(VPValue::VPExternalDefSC, BType),
        HIROperand(new VPBlob(DDR, BI)) {
    setName(getOperandHIR()->getName());
  }

  // Construct a VPExternalDef given an underlying CanonExpr \p CE.
  VPExternalDef(const loopopt::CanonExpr *CE, const loopopt::RegDDRef *DDR)
      : VPValue(VPValue::VPExternalDefSC, CE->getDestType()),
        HIROperand(new VPCanonExpr(CE, DDR)) {
    setName(getOperandHIR()->getName());
  }

  // Construct a VPExternalDef given an underlying IV level \p IVLevel.
  VPExternalDef(unsigned IVLevel, Type *BaseTy)
      : VPValue(VPValue::VPExternalDefSC, BaseTy),
        HIROperand(new VPIndVar(IVLevel)) {
    setName(getOperandHIR()->getName());
  }

  VPExternalDef(const loopopt::HLIf *If)
      : VPValue(VPValue::VPExternalDefSC,
                Type::getInt1Ty(If->getHLNodeUtils().getContext())),
        HIROperand(new VPIfCond(If)) {}

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

  const HIROperandSpecifics HIR() const {
    return HIROperandSpecifics(this);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printAsOperand(raw_ostream &OS) const override {
    if (getUnderlyingValue())
      getUnderlyingValue()->printAsOperand(OS);
    else {
      getType()->print(OS);
      OS << " ";
      HIROperand->print(OS);
    }
  }
  void printDetail(raw_ostream &OS) const {
    if (getUnderlyingValue())
      getUnderlyingValue()->printAsOperand(OS);
    else {
      HIROperand->printDetail(OS);
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
/// The object of this class registers the fact that a VPValue is live out and
/// keeps a link to underlying use outside the loop. The link can be undefined
/// in case when VPValue is live-out "partially", e.g. induction that is not
/// live-out in the main loop is live-out in the peel loop.
class VPExternalUse : public VPUser {
private:
  friend class VPExternalValues;
  friend class VPLiveInOutCreator;
  friend class VPOCodeGen;
  friend class VPOCodeGenHIR;
  friend class VPDecomposerHIR;
  friend class HIROperandSpecifics;

  // Hold the DDRef or IV information related to this external use.
  std::unique_ptr<VPOperandHIR> HIROperand;

  // Construct a VPExternalUse given a Value \p ExtVal.
  VPExternalUse(PHINode *ExtVal, unsigned Id)
      : VPUser(VPValue::VPExternalUseSC, ExtVal->getType()), MergeId(Id) {
    setUnderlyingValue(*ExtVal);
  }
  // Construct a VPExternalUse given an underlying DDRef \p DDR.
  VPExternalUse(const loopopt::DDRef *DDR, unsigned Id)
      : VPUser(VPValue::VPExternalUseSC, DDR->getDestType()),
        HIROperand(new VPBlob(DDR)), MergeId(Id) {}
  // Construct a VPExternalUse given an underlying IV level \p IVLevel.
  VPExternalUse(unsigned IVLevel, Type *BaseTy, unsigned Id)
      : VPUser(VPValue::VPExternalUseSC, BaseTy),
        HIROperand(new VPIndVar(IVLevel)), MergeId(Id) {}

  // Construct a VPExternalUse w/o underlying info. This is needed for
  // entities that are non-liveout, to link values between different parts of
  // vectorized loop, peel/main loop/remainder.
  VPExternalUse(unsigned Id, Type *BaseTy)
      : VPUser(VPValue::VPExternalUseSC, BaseTy), MergeId(Id) {}

  // DESIGN PRINCIPLE: Access to the underlying IR must be strictly limited to
  // the front-end and back-end of VPlan so that the middle-end is as
  // independent as possible of the underlying IR. We grant access to the
  // underlying IR using friendship.

  /// Return the HIR operand for this VPExternalDef.
  const VPOperandHIR *getOperandHIR() const { return HIROperand.get(); };

public:
  static constexpr unsigned UndefMergeId =
      std::numeric_limits<unsigned int>::max();

  VPExternalUse() = delete;
  VPExternalUse(const VPExternalUse &) = delete;
  VPExternalUse &operator=(const VPExternalUse &) = delete;

  /// Methods for supporting type inquiry through isa, cast, and dyn_cast:
  static bool classof(const VPValue *V) {
    return V->getVPValueID() == VPExternalUseSC;
  }

  unsigned getMergeId() const { return MergeId; }

  bool hasUnderlying() const {
    return getUnderlyingValue() != nullptr || HIROperand != nullptr;
  }

  const HIROperandSpecifics HIR() const {
    return HIROperandSpecifics(this);
  }

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
  void print(raw_ostream &OS) const override {
    print(OS, nullptr);
  }
  void print(raw_ostream &OS, VPValue* Operand) const {
    OS << "Id: " << getMergeId() << "   ";
    if (getUnderlyingValue()) {
      cast<Instruction>(getUnderlyingValue())->print(OS);
      if (getNumOperands())
        for (unsigned I = 0, E = getNumOperands(); I < E; ++I) {
          OS << " ";
          getOperand(I)->printAsOperand(OS);
          OS << " -> ";
          getUnderlyingOperand(I)->printAsOperand(OS);
          OS << ";\n";
        }
      else {
        assert(Operand && "Expected non-null operand");
        OS << " ";
        Operand->printAsOperand(OS);
        OS << " -> ";
        getUnderlyingOperand(0)->printAsOperand(OS);
        OS << ";\n";
      }
    } else if (HIROperand) {
      if (getNumOperands())
        for (unsigned I = 0, E = getNumOperands(); I < E; ++I) {
          if (I)
            OS << ", ";
          getOperand(I)->printAsOperand(OS);
        }
      else {
        assert(Operand && "Expected non-null operand");
        Operand->printAsOperand(OS);
      }
      OS << " ->";
      HIROperand->printDetail(OS);
    } else {
      OS << "no underlying for ";
      if (getNumOperands())
        getOperand(0)->printAsOperand(OS);
      else {
        assert(Operand && "Expected non-null operand");
        Operand->printAsOperand(OS);
      }
      OS << "\n";
    }
  }

  void printAsOperand(raw_ostream &OS) const override {
    getType()->print(OS);
    OS << " ext-use" << getMergeId() << "(";
    if (getNumOperands() > 0)
      getOperand(0)->printAsOperand(OS);
    OS << ")";
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  const Value *getUnderlyingOperand(unsigned N) const {
    assert(N < UnderlyingOperands.size() && "Operand index out of bounds");
    return UnderlyingOperands[N];
  }

  SmallVector<Value *, 2> UnderlyingOperands;

  // Identifier of the live out value in VPlan. Is provided by VPlan and used
  // to keep track of live-outs during VPlan cloning.
  unsigned MergeId;
};

/// This class augments VPValue with Metadata that is used as operand of another
/// VPValue class. It contains a pointer to the underlying MetadataAsValue.
class VPMetadataAsValue : public VPValue {
  // VPlan is currently the context where we hold the pool of
  // VPMetadataAsValues.
  friend class VPCloneUtils;
  friend class VPExternalValues;
  friend class VPValueMapper;

protected:
  VPMetadataAsValue(MetadataAsValue *MDAsValue)
      : VPValue(VPValue::VPMetadataAsValueSC, MDAsValue->getType(), MDAsValue) {
  }

  /// Return the Metadata of the underlying MetadataAsValue.
  Metadata *getMetadata() { return getMetadataAsValue()->getMetadata(); }

public:
  /// Return the underlying MetadataAsValue.
  MetadataAsValue *getMetadataAsValue() const {
    assert(isa<MetadataAsValue>(getUnderlyingValue()) &&
           "Expected MetadataAsValue as underlying Value.");
    return cast<MetadataAsValue>(getUnderlyingValue());
  }

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
  void print(raw_ostream &OS) const override { printAsOperand(OS); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return V->getVPValueID() == VPMetadataAsValueSC;
  }
};

/// VPLiveInValue is a wrapper for the loop entities incoming values in VPlan.
/// VPLiveInValue replaces the original incoming value in init/final
/// instructions of the loop entities after CFG building. The main purpose of
/// VPLiveInValue is to protect the original incoming value from unintended
/// access.
/// The VPLiveInValue has a MergeId which is its unique index and is used to
/// link it to the original incoming value. The MergeId is not changed during
/// cloning so it can be used identically in different VPlans.
/// The VPLiveInValue-s are created during VPlan construction, after CFG
/// is updated with VPEntity instructions.
/// For example:
/// Before VPLiveInValue creation:
///  %red_init = reduction-init{+} %extdef
///
/// After VPLiveInValue creation:
///  %red_init = reduction-init{+} %live-in1
///
class VPLiveInValue : public VPValue {
public:
  VPLiveInValue(unsigned Id, Type* Ty)
      : VPValue(VPValue::VPLiveInValueSC, Ty), MergeId(Id) {}

  VPLiveInValue(const VPLiveInValue& LI)
      : VPLiveInValue(LI.getMergeId(), LI.getType()) {}

  unsigned getMergeId() const { return MergeId; }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return V->getVPValueID() == VPValue::VPLiveInValueSC;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS) const override {
    OS << "live-in" << getMergeId() << "\n";
  }

  void printAsOperand(raw_ostream &OS) const override {
    getType()->print(OS);
    OS << " live-in" << getMergeId();
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  VPLiveInValue *clone() const {
    VPLiveInValue *Cloned = new VPLiveInValue(*this);
    return Cloned;
  }

private:
  unsigned MergeId;
};

/// VPLiveOutValue is a wrapper for the uses of loop entities outgoing values
/// in VPlan. Those uses are encoded by VPExternalUse.
/// The main purpose of VPLiveOutValue is to provide ability to clone VPlan w/o
/// adding additional operands in VPExternalUse. The VPLiveOutValue becomes a
/// use of liveout instead of VPExternalUse.
/// The VPLiveOutValue contains a link to the VPExternalUse. The linking is done
/// through MergeId which is assigned at creation.
/// The MergeId is not changed during cloning so the live outs of different
/// VPlans can be easily linked with the same external use.
/// The VPLiveOutValue-s are created during VPlan construction, after CFG is
/// updated with VPEntity instructions, and are kept in VPlan.
///
class VPLiveOutValue : public VPUser {
public:
  VPLiveOutValue(unsigned Id, VPValue *Operand)
      : VPUser(VPValue::VPLiveOutValueSC, {Operand}, Operand->getType()), MergeId(Id) {}

  VPLiveOutValue(const VPLiveOutValue& LI)
      : VPLiveOutValue(LI.getMergeId(), LI.getOperand(0)) {}

  unsigned getMergeId() const { return MergeId; }

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPValue *V) {
    return V->getVPValueID() == VPValue::VPLiveOutValueSC;
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS) const override {
    OS << "live-out" << getMergeId() << "(";
    getOperand(0)->printAsOperand(OS);
    OS << ")\n";
  }

  void printAsOperand(raw_ostream &OS) const override {
    getType()->print(OS);
    OS << " live-out" << getMergeId();
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  VPLiveOutValue *clone() const {
    VPLiveOutValue *Cloned = new VPLiveOutValue(MergeId, getOperand(0));
    return Cloned;
  }

private:
  unsigned MergeId;
};
} // namespace vpo

template <> struct GraphTraits<vpo::VPUser *> {
  using NodeRef = vpo::VPUser *;
  using ChildIteratorType = vpo::VPValue::user_iterator;

  static NodeRef getEntryNode(NodeRef N) { return N; }

  static inline ChildIteratorType child_begin(NodeRef N) {
    return N->user_begin();
  }

  static inline ChildIteratorType child_end(NodeRef N) { return N->user_end(); }
};
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVALUE_H

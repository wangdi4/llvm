//===-- IntelVPlanLegalityDescr.h -------------------------------*- C++ -*-===//
//
//   Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// This file defines hierarchy of classes required to capture loop/SIMD
//  entities like reductions/inductions/privates in vectorizer frontend.
//
//  class DescrValue
///          |
///          |
/// class DescrWithAliases
///          |
///          |
/// class PrivDescr
///          |
///          |
/// class PrivDescrNonPOD
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLEGALITYDESCR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLEGALITYDESCR_H

#include "llvm/Analysis/IVDescriptors.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include <type_traits>

namespace llvm {

namespace vpo {
template <typename T, typename = void> struct InstructionTypeDeducer {
  using Instruction = llvm::Instruction;
};

template <typename T>
struct InstructionTypeDeducer<
    T,
    typename std::enable_if<std::is_base_of<loopopt::DDRef, T>::value>::type> {
  using Instruction = loopopt::HLInst;
};

// Based on a provided ARGUMENT deduce type of Instruction - llvm::Instruction
// or loopopt::HLInst. ARGUMENT can be of type llvm::Value or loopopt::DDRef.
// Such mechanism is required to provide common interface for LLVMIR and HIR.
#define INTEL_INTRODUCE_INSTRUCTION(ARGUMENT)                                  \
  using Instruction = typename InstructionTypeDeducer<ARGUMENT>::Instruction;

struct CustomCompareRef {
  bool operator()(const loopopt::DDRef *Ref1,
                  const loopopt::DDRef *Ref2) const {
    return Ref1->getSymbase() == Ref2->getSymbase();
  }
};

struct CustomCompareVal {
  bool operator()(const Value *Val1, const Value *Val2) const {
    return Val1 == Val2;
  }
};

template <typename T, typename = void> struct CustomCompareDeducer {
  using CustomCompare = CustomCompareVal;
};

template <typename T>
struct CustomCompareDeducer<
    T,
    typename std::enable_if<std::is_base_of<loopopt::DDRef, T>::value>::type> {
  using CustomCompare = CustomCompareRef;
};

// Based on a provided ARGUMENT deduce which CustomCompare should be used.
// CustomCompareVal is dummy struct required only to be able to templatize
// CustomCompare for llvm::Value and loopopt::DDRef.
#define INTEL_INTRODUCE_CUSTOMCOMPARE(ARGUMENT)                                \
  using CustomCompare = typename CustomCompareDeducer<ARGUMENT>::CustomCompare;

// Base class for descriptors which have a value and list of instructions that
// update the value inside the loop. Value can be of type llvm::Value or
// loopopt::DDRef.
template <typename Value> class DescrValue {
public:
  // DescrValue classs is base class for DescrWithAliases as well as
  // DescrWithInitValue. To distinguish classes and provide RTTI support
  // following enum is required.
  enum DescrKind { DK_SimpleDescr, DK_WithAliases, DK_WithInitValue };

private:
  INTEL_INTRODUCE_INSTRUCTION(Value)

  /// Pointer to private value.
  Value *Ref;
  DescrKind Kind;
  // TODO: Might be used once filled with some instructons.
  // - For main privates that can be store instructions inside the loop to
  // identify "in-memory" privates - if list is not empty and contains stores
  // it's in-memory.
  // - For aliases that can be either a load that is used inside the loop - the
  // we can put in the list it uses, or we have an operand of the store - we can
  // go through its operands and keep track until loop-phi.
  SmallVector<const Instruction *, 4> UpdateInstructions;

protected:
  DescrValue(Value *RefV, DescrKind K) : Ref(RefV), Kind(K) {}

public:
  DescrValue(Value *RefV) : Ref(RefV), Kind(DescrKind::DK_SimpleDescr) {}

  // Copy constructor
  DescrValue(const DescrValue &Other)
      : Ref(Other.Ref), Kind(Other.Kind),
        UpdateInstructions(Other.UpateInstructions) {}

  // Move constructor
  DescrValue(DescrValue &&Other)
      : Ref(std::exchange(Other.Ref, nullptr)), Kind(Other.Kind),
        UpdateInstructions(std::move(Other.UpdateInstructions)) {}

  // Copy assignment
  DescrValue &operator=(const DescrValue &Other) {
    if (this == &Other)
      return *this;
    return *this = DescrValue(Other);
  }

  // Move assignment
  DescrValue &operator=(DescrValue &&Other) {
    if (this == &Other)
      return *this;
    std::swap(Ref, Other.Ref);
    std::swap(Kind, Other.Kind);
    std::swap(UpdateInstructions, Other.UpdateInstructions);
    return *this;
  }

  virtual ~DescrValue() {}

  /// Return pointer to private value of a type specified in class template
  /// argument.
  Value *getRef() const { return Ref; }
  /// Return list of Update Instructions.
  ArrayRef<const Instruction *> getUpdateInstructions() const {
    return UpdateInstructions;
  }
  /// Add new instruction to list of Update Instructions.
  void addUpdateInstruction(const Instruction *Inst) {
    UpdateInstructions.push_back(Inst);
  }

  virtual bool isValidAlias() const { return UpdateInstructions.size() > 0; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  virtual void print(raw_ostream &OS, unsigned Indent = 0) const {
    OS << "Ref: ";
    Ref->dump();
    OS << "\n";
    OS.indent(Indent + 2) << "UpdateInstructions:\n";
    if (UpdateInstructions.empty())
      OS.indent(Indent + 2) << "none\n";
    else
      for (auto &V : UpdateInstructions) {
        OS.indent(Indent + 2);
        V->dump();
      }
  }
  void dump() const { print(errs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  DescrKind getKind() const { return Kind; }
  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const DescrValue<Value> *Descr) {
    return Descr->getKind() >= DescrKind::DK_SimpleDescr;
  }
};

// Base class for descriptors which may have alias value used within the loop
// of incoming IR. NOTE : Only original descriptors can have aliases and they
// are always of the form &(%a)[0]
template <typename Value> class DescrWithAliases : public DescrValue<Value> {
  using DescrKind = typename DescrValue<Value>::DescrKind;

  INTEL_INTRODUCE_CUSTOMCOMPARE(Value)
  /// Vector of Aliases for particular private value.
  SmallVector<std::unique_ptr<DescrValue<Value>>, 8> Aliases;

protected:
  // Required by descendent in DescrWithInitValue
  DescrWithAliases(Value *RefV, DescrKind K) : DescrValue<Value>(RefV, K) {}

public:
  // Value can be of type llvm::Value or loopopt::DDRef
  DescrWithAliases(Value *RefV)
      : DescrValue<Value>(RefV, DescrKind::DK_WithAliases) {}

  // Copy constructor
  DescrWithAliases(const DescrWithAliases &Other)
      : DescrValue<Value>(Other), Aliases(std::move(Other.Aliases)) {}

  // Move constructor
  DescrWithAliases(DescrWithAliases &&Other)
      : DescrValue<Value>(std::move(Other)), Aliases(std::move(Other.Aliases)) {
  }

  // Copy assignment
  DescrWithAliases &operator=(const DescrWithAliases &Other) {
    if (this == &Other)
      return *this;
    DescrValue<Value>::operator=(Other);
    Aliases = Other.Aliases;
    return *this;
  }

  // Move assignment
  DescrWithAliases &operator=(DescrWithAliases &&Other) {
    if (this == &Other)
      return *this;
    DescrValue<Value>::operator=(std::move(Other));
    std::swap(Aliases, Other.Aliases);
    return *this;
  }

  inline decltype(auto) aliases() const {
    return map_range(Aliases, [](auto &UPtr) { return UPtr.get(); });
  }

  /// Add new alias for private value.
  void addAlias(const Value *RefV, std::unique_ptr<DescrValue<Value>> Descr) {
    if (!findAlias(RefV)) // don't add second time
      Aliases.push_back(std::move(Descr));
  }

  /// Return alias for specific value. If no alias is found return nullptr.
  DescrValue<Value> *findAlias(const Value *RefV) const {
    // When llvm::find_if is used, on Widnows compilation fails with error
    // that iterator cannot be assigned because its copy assignment
    // operator is implicitly deleted.
    // TODO: Fix issue with llvm::find_if on Windows.
    for (auto *Alias : aliases()) {
      if (CustomCompare()(RefV, Alias->getRef()))
        return Alias;
    }

    return nullptr;
  }

  /// Filter out invalid aliases and return the valid one. If no valid alias is
  /// found return nullptr.
  DescrValue<Value> *getValidAlias() const {
    DescrValue<Value> *ValidAlias = nullptr;
    for (auto *Alias : aliases()) {
      if (Alias->isValidAlias())
        ValidAlias = Alias;
    }
    return ValidAlias;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  virtual void print(raw_ostream &OS, unsigned Indent = 0) const override {
    DescrValue<Value>::print(OS);
    if (!Aliases.empty()) {
      for (const auto *AliasIt : aliases()) {
        OS << "\n";
        OS.indent(2) << "Alias";
        AliasIt->print(OS, 2);
      }
      OS << "\n";
    }
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const DescrValue<Value> *Descr) {
    return Descr->getKind() >= DescrKind::DK_WithAliases;
  }
};

// Class used to represent descriptors that may have aliases and initvalue.
template <typename Value>
class DescrWithInitValue : public DescrWithAliases<Value> {
  using DescrKind = typename DescrValue<Value>::DescrKind;

  INTEL_INTRODUCE_CUSTOMCOMPARE(Value)
  // NOTE: InitValue holds only llvm::Values/loopopt::DDRefs for which
  // VPExternalDefs were created for a descriptor/alias.
  // llvm::Values/loopopt::DDRefs with VPConstants are not accounted for. Each
  // descriptor/alias may have multiple updating instructions within the loop.
  const Value *InitValue;

public:
  DescrWithInitValue(Value *RefV)
      : DescrWithAliases<Value>(RefV, DescrKind::DK_WithInitValue),
        InitValue(nullptr) {}

  // Move constructor
  DescrWithInitValue(DescrWithInitValue &&Other) = default;

  void setInitValue(Value *Val) { InitValue = Val; }
  const Value *getInitValue() const { return InitValue; }

  bool isValidAlias() const override {
    return InitValue && DescrValue<Value>::getUpdateInstructions().size() > 0;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, unsigned Indent = 0) const override {
    DescrWithAliases<Value>::print(OS);
    if (InitValue) {
      OS.indent(Indent + 2) << "InitValue: ";
      InitValue->dump();
      OS << "\n";
    }
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  static bool classof(const DescrWithAliases<Value> *Descr) {
    return Descr->getKind() == DescrKind::DK_WithInitValue;
  }

  static bool classof(const DescrValue<Value> *Descr) {
    return Descr->getKind() == DescrKind::DK_WithInitValue;
  }
};

// Specialized class to represent reduction descriptors specified explicitly
// via SIMD reduction clause. The reduction's kind and signed datatype
// information is also stored within this class.
template <typename Value> class RedDescr : public DescrWithInitValue<Value> {
private:
  RecurKind Kind;
  bool IsSigned;

public:
  RedDescr(Value *RegV, RecurKind KindV, bool Signed)
      : DescrWithInitValue<Value>(RegV), Kind(KindV), IsSigned(Signed) {}

  // Move constructor
  RedDescr(RedDescr &&Other) = default;

  void setKind(RecurKind K) { Kind = K; }
  RecurKind getKind() const { return Kind; }

  void setIsSigned(bool V) { IsSigned = V; }
  bool isSigned() const { return IsSigned; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, unsigned Indent = 0) const override {
    DescrWithInitValue<Value>::print(OS);
    OS.indent(Indent + 2) << "RedDescr: ";
    OS << "{Kind: "
       << llvm::Instruction::getOpcodeName(
              RecurrenceDescriptorData::getOpcode(Kind))
       << ", IsSigned: " << ((isSigned()) ? "1" : "0");
    OS << "}\n";
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// Specialized class to represent user-defined reduction (UDR) descriptors
// specified explicitly via SIMD reduction clause.
template <typename Value> class RedDescrUDR : public RedDescr<Value> {
private:
  Function *Combiner = nullptr;
  Function *Initializer = nullptr;
  Function *Ctor = nullptr;
  Function *Dtor = nullptr;

public:
  RedDescrUDR(Value *RegV, Function *Combiner, Function *Initializer,
              Function *Ctor, Function *Dtor)
      : RedDescr<Value>(RegV, RecurKind::Udr, false /*Signed*/),
        Combiner(Combiner), Initializer(Initializer), Ctor(Ctor), Dtor(Dtor) {}

  // Move constructor
  RedDescrUDR(RedDescrUDR &&Other) = default;

  /// Get combiner function for UDR.
  Function *getCombiner() const { return Combiner; }
  /// Get initializer function for UDR.
  Function *getInitializer() const { return Initializer; }
  /// Get constructor function for UDR datatype.
  Function *getCtor() const { return Ctor; }
  /// Get destructor function for UDR datatype.
  Function *getDtor() const { return Dtor; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, unsigned Indent = 0) const override {
    RedDescr<Value>::print(OS);
    OS.indent(Indent + 2) << "RedDescrUDR: ";
    OS << "{Combiner: " << Combiner->getName();
    OS << ", Initializer: " << (Initializer ? Initializer->getName() : "none");
    OS << ", Ctor: " << (Ctor ? Ctor->getName() : "none");
    OS << ", Dtor: " << (Dtor ? Dtor->getName() : "none");
    OS << "}\n";
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// Specialized class to represent private descriptors specified explicitly via
// SIMD private clause.
template <typename Value> class PrivDescr : public DescrWithAliases<Value> {
  using DescrKind = typename DescrValue<Value>::DescrKind;

public:
  enum class PrivateKind {
    NonLast,    // A simple non-live out private
    Last,       // Private was declared as "lastprivate" in the directive
    Conditional // Last private was declared under condition or as
                // "lastprivte:conditional" in the directive
  };

private:
  PrivateKind PrivKind;
  Type *Ty;
  // Special flag for F90_[NONPOD|DV] incomming directive
  bool IsF90;

public:
  // Value can be of type llvm::Value or loopopt::DDRef
  PrivDescr(Value *RegV, Type *Ty, PrivateKind KindV, bool IsF90)
      : DescrWithAliases<Value>(RegV, DescrKind::DK_WithAliases),
        PrivKind(KindV), Ty(Ty), IsF90(IsF90) {}

  // Copy constructor
  PrivDescr(const PrivDescr &Other)
      : DescrWithAliases<Value>(Other), PrivKind(Other.PrivKind), Ty(Other.Ty) {
  }

  // Move constructor
  PrivDescr(PrivDescr &&Other)
      : DescrWithAliases<Value>(std::move(Other)), PrivKind(Other.PrivKind),
        Ty(Other.Ty) {}

  // Copy assignment
  PrivDescr &operator=(const PrivDescr &Other) {
    if (this == &Other)
      return *this;
    DescrWithAliases<Value>::operator=(Other);
    PrivKind = Other.PrivKind;
    Ty = Other.Ty;
  }

  // Move assignment
  PrivDescr &operator=(PrivDescr &&Other) {
    if (this == &Other)
      return *this;
    DescrWithAliases<Value>::operator=(std::move(Other));
    PrivKind = Other.PrivKind;
    Ty = Other.Ty;
  }

  /// Check if private is conditional last private.
  bool isCond() const { return PrivKind == PrivateKind::Conditional; }
  /// Check if private is last or conditional last private.
  bool isLast() const { return PrivKind != PrivateKind::NonLast; }
  /// Check if private if originating from F90_* directive
  bool isF90() const { return IsF90; }
  /// Check if private is for non-POD data type.
  virtual bool isNonPOD() const { return false; }
  /// Get the private Type.
  Type *getType() const { return Ty; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, unsigned Indent = 0) const override {
    DescrWithAliases<Value>::print(OS);
    OS << "PrivDescr: ";
    OS << "{IsCond: " << ((isCond()) ? "1" : "0")
       << ", IsLast: " << ((isLast()) ? "1" : "0");
    OS << ", Type: ";
    Ty->print(OS);
    OS << "}\n";
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// Specialized class to represent non-POD private descriptors specified
// explicitly via SIMD private clause.
template <typename Value> class PrivDescrNonPOD : public PrivDescr<Value> {
  using PrivateKind = typename PrivDescr<Value>::PrivateKind;
  Function *Ctor;
  Function *Dtor;
  Function *CopyAssign;

public:
  // Value can be of type llvm::Value or loopopt::DDRef
  PrivDescrNonPOD(Value *RegV, Type *Ty, PrivateKind KindV, bool IsF90,
                  Function *Ctor, Function *Dtor, Function *CopyAssign)
      : PrivDescr<Value>(RegV, Ty, KindV, IsF90), Ctor(Ctor), Dtor(Dtor),
        CopyAssign(CopyAssign) {
    assert(KindV != PrivateKind::Conditional &&
           "Non POD privates cannot be conditional last privates.");
  }

  // Copy constructor
  PrivDescrNonPOD(const PrivDescrNonPOD &Other)
      : PrivDescr<Value>(Other), Ctor(Other.Ctor), Dtor(Other.Dtor),
        CopyAssign(Other.CopyAssign) {}

  // Move constructor
  PrivDescrNonPOD(PrivDescrNonPOD &&Other)
      : PrivDescr<Value>(std::move(Other)),
        Ctor(std::exchange(Other.Ctor, nullptr)),
        Dtor(std::exchange(Other.Dtor, nullptr)),
        CopyAssign(std::exchange(Other.CopyAssign, nullptr)) {}

  // Copy assign
  PrivDescrNonPOD &operator=(const PrivDescrNonPOD &Other) {
    if (this == &Other)
      return *this;
    PrivDescr<Value>::operator=(Other);
    Ctor = Other.Ctor;
    Dtor = Other.Dtor;
    CopyAssign = Other.CopyAssign;
    return *this;
  }

  // Move assign
  PrivDescrNonPOD &operator=(PrivDescrNonPOD &&Other) {
    if (this == &Other)
      return *this;
    PrivDescr<Value>::operator=(std::move(Other));
    std::swap(Ctor, Other.Ctor);
    std::swap(Dtor, Other.Dtor);
    std::swap(CopyAssign, Other.CopyAssign);
    return *this;
  }

  /// Get constructor function for nonPOD private value.
  Function *getCtor() const { return Ctor; }
  /// Get destructor function for nonPOD private value.
  Function *getDtor() const { return Dtor; }
  /// Get copy assign function for nonPOD private value.
  Function *getCopyAssign() const { return CopyAssign; }
  /// Check if private is for non-POD data type.
  bool isNonPOD() const override { return true; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, unsigned Indent = 0) const override {
    PrivDescr<Value>::print(OS);
    OS << "PrivDescrNonPOD: ";
    OS << "{Ctor: " << Ctor->getName();
    OS << ", Dtor: " << Dtor->getName();
    OS << ", Copy Assign: ";
    // Copy Assign function is not always present
    if (CopyAssign)
      OS << CopyAssign->getName();
    OS << "}\n";
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

} // End namespace vpo

} // End namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLEGALITYDESCR_H

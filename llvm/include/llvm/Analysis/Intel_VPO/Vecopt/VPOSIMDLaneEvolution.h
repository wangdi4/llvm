//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOSIMDLaneEvolution.h -- Defines the SIMD Lane Evolution analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_SLEV_H
#define LLVM_ANALYSIS_VPO_SLEV_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"

#include "llvm/Analysis/Intel_VPO/Vecopt/VPODefUse.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOCFG.h"

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvr.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrVisitor.h"

#include "llvm/ADT/GraphTraits.h"
#include "llvm/Support/DOTGraphTraits.h"

namespace llvm { // LLVM Namespace

namespace vpo {  // VPO Vectorizer Namespace

typedef SmallPtrSet<AVR*, 2> AvrSetTy;

enum SLEVKind {
  BOTTOM,   /// No SLEV information
  CONSTANT, /// Compile-time constant
  UNIFORM,  /// All elements in vector are identical
  STRIDED,  /// Elements are in a compile-time constant stride
  RANDOM,   /// TOP, no evolution between SIMD lanes.
};

class SIMDLaneEvolutionBase;
class SIMDLaneEvolution;
class SIMDLaneEvolutionHIR;

/// \brief This class both holds a SIMD lane evolution state and is the basis
/// for computing it from other SLEVs based on some specific internal structure.
/// The way a specific SLEV depends on others is modelled by the various derived
/// classes, which abstract away the concrete details of the actual instructions
/// the SLEVs represent.
class SLEV {

protected:

  enum SubtypeTy {
    SLEVValueTy,
    SLEVAddTy,
    SLEVAddressTy,
    SLEVCmpTy,
    SLEVDivTy,
    SLEVIdentityTy,
    SLEVLoadTy,
    SLEVMulTy,
    SLEVPredefinedTy,
    SLEVPreserveUniformityTy,
    SLEVSubTy,
    SLEVUseTy,
  };

public:

  typedef SmallPtrSet<SLEV*, 2> SLEVUsersTy;

  /// A class used by SLEVs for representing and operating on the numeric
  /// values SLEV manipulates (representing constant values and strides).
  class Number {
  private:
    bool Defined;
    bool IsInteger;
    APSInt IntN;
    APFloat FloatN;
  public:
    Number() :
        Defined(false), IsInteger(false), IntN(APSInt::getAllOnesValue(32)),
        FloatN(APFloat::getAllOnesValue(32)) {}
    Number(const APSInt& N) : Defined(true),
                              IsInteger(true),
                              IntN(N),
                              FloatN(APFloat::getAllOnesValue(32)) {}
    Number(const APFloat& N) : Defined(true),
                               IsInteger(true),
                               IntN(APSInt::getAllOnesValue(32)),
                               FloatN(N) {}
    Number(const Number& O) : Defined(O.Defined),
                              IsInteger(O.IsInteger), 
                              IntN(O.IntN),
                              FloatN(O.FloatN) {}
    void print(raw_ostream& OS) const;
    bool operator==(const Number &O) const;
    bool operator!=(const Number &O) const { return !(*this == O); }
    Number operator+(const Number &RHS) const;
    Number operator-(const Number &RHS) const;
    Number operator*(const Number &RHS) const;
    Number operator/(const Number &RHS) const;
    APSInt getInteger() {
      assert(Defined && IsInteger && "This is not an integer number");
      return IntN;
    }
    APFloat getFloat() {
      assert(Defined && !IsInteger && "This is not a floating-point number");
      return FloatN;
    }
  };

  static const Number Zero;
  static const Number One;

private:

  const SubtypeTy Subtype;

  /// The computed kind of this SLEV.
  SLEVKind Kind;

  /// The compile-time numeric value related to this SLEV.
  Number NumericValue;

  /// Is SLEV tainted by control-flow divergence.
  bool IsTainted;

  /// SLEVs which would be affected by changes to this one.
  SLEVUsersTy Users;

  /// If set, this SLEV turning non-uniform causes control-flow divergence.
  bool IsBranchCondition;

  /// The AVR this SLEV (optionally) is attached to.
  AVR* Avr;

  void setBranchCondition() { IsBranchCondition = true; }

  void setAVR(AVR* A) { Avr = A; }

  static unsigned long long NextId;
  unsigned long long Id;

  void assignId() { Id = NextId++; }

  friend SIMDLaneEvolutionBase;
  friend SIMDLaneEvolution;
  friend SIMDLaneEvolutionHIR;

protected:

  /// \brief Calculate this SLEV's value in a derived-specific logic.
  virtual void calculate() {}

  void markTainted() { IsTainted = true; }

  void printValue(raw_ostream& OS) const {
    OS << SLEVKINDSTR[Kind];
    if (Kind == SLEVKind::CONSTANT || Kind == SLEVKind::STRIDED) {
      OS << "<";
      NumericValue.print(OS);
      OS << ">";
    }
  }

  virtual bool hasStructure() const { return false; }

  /// \brief A derived-specific print of the complex structure of the SLEV
  /// expression. Implementors should recurse into the printStructure of
  /// their sub-SLEVs. The default implementation (corresponds to having no
  /// structure) is to print the SLEV value.
  virtual void printStructure(raw_ostream& OS) const {
    printValue(OS);
  }

  SLEV(SubtypeTy St) : Subtype(St) {
    assignId();
    IsTainted = false;
    IsBranchCondition = false;
    Avr = nullptr;
    setBOTTOM();
  }

  /// \brief Constructor for SLEV kinds that do not take a numeric value.
  SLEV(SubtypeTy St, SLEVKind SK) : SLEV(St) {
    switch(SK) {
    case BOTTOM:
      break;
    case UNIFORM:
      setUNIFORM();
      break;
    case RANDOM:
      setRANDOM();
      break;
    default:
      llvm_unreachable("SLEV kind requires a numeric value");
    }
  }

  /// \brief Cconstructor for SLEV kinds that take a numeric value.
  SLEV(SubtypeTy St, SLEVKind SK, const Number& N) : SLEV(St) {
    switch(SK) {
    case CONSTANT:
      setCONSTANT(N);
      break;
    case STRIDED:
      setSTRIDED(N);
      break;
    default:
      llvm_unreachable("SLEV kind does not require a numeric value");
    }
  }

public:

  SubtypeTy getSubtype() const { return Subtype; }

  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEV *Node) {
    return (Node->getSubtype() == SLEVValueTy);
  }

  const SLEVUsersTy& getUsers() const { return Users; }

  void addUser(SLEV* User) {
    Users.insert(User);
  }

  AVR* getAVR() const { return Avr; }

  SLEV() : SLEV(SLEVValueTy) {}

  /// \brief Constructor for SLEV kinds that do not take a numeric value.
  SLEV(SLEVKind SK) : SLEV(SLEVValueTy, SK) {}

  /// \brief Constructor for SLEV kinds that take a numeric value.
  SLEV(SLEVKind SK, const Number& N) : SLEV(SLEVValueTy, SK, N) {}

  SLEV(const SLEV& Other) : SLEV(SLEVValueTy) {
    Kind = Other.Kind;
    NumericValue = Other.NumericValue;
    IsTainted = Other.IsTainted;
    Users = Other.Users;
    IsBranchCondition = Other.IsBranchCondition;
    Avr = Other.Avr;
  }

  SLEV getValue() {
    SLEV Copy;
    Copy.copyValue(*this);
    return Copy;
  }

  virtual ~SLEV() {}

  void copyValue(const SLEV& Other) {
    Kind = Other.Kind;
    NumericValue = Other.NumericValue;
  }

  void setBOTTOM() {
    Kind = SLEVKind::BOTTOM;
    NumericValue = Number();
  }

  void setCONSTANT(Number C) {
    Kind = SLEVKind::CONSTANT;
    NumericValue = C;
  }

  void setUNIFORM() {
    Kind = SLEVKind::UNIFORM;
    NumericValue = Number(); // Undefine is universal unit.
  }

  void setSTRIDED(Number C) {
    Kind = SLEVKind::STRIDED;
    NumericValue = C;
  }

  void setRANDOM() {
    Kind = SLEVKind::RANDOM;
    NumericValue = Number();
  }

  bool operator==(const SLEV& O) {
    return Kind == O.Kind && NumericValue == O.NumericValue;
  }

  bool operator!=(const SLEV& O) { return !(*this == O); }

  SLEVKind getKind() const { return Kind; }

  const Number& getConstant() const {
    static Number Undef;
    if (isBOTTOM())
      return Undef;
    assert(isConstant() && "Cannot get constant for non-constant");
    return NumericValue;
  }

  const Number& getStride() const {
    static Number Undef;
    if (isBOTTOM())
      return Undef;
    assert(isStrided() && "Cannot get stride for non-strided");
    if (isUniform())
      return Zero; // The stride of any Uniform is zero.
    return NumericValue;
  }

  /// \brief This is the SLEV lattice least upper bound operation.
  static SLEV join(const SLEV &A, const SLEV &B) {
    const SLEV *Low = A.Kind <= B.Kind ? &A : &B;
    const SLEV *High = A.Kind <= B.Kind ? &B : &A;
    switch (Low->Kind) {
    case BOTTOM:
      return *High;
    case CONSTANT:
      switch (High->Kind) {
      case CONSTANT:
        if (Low->NumericValue != High->NumericValue)
          return SLEV(UNIFORM); // Two different CONSTANTs.
        // Fallthrough
      case UNIFORM:
        return *High; // Two equal CONSTANTS, or CONSTANT V UNIFORM.
      default:
        break;
      }
      break;
    case UNIFORM:
      if (High->Kind == UNIFORM)
        return *High;
      break;
    case STRIDED:
      if (High->Kind == STRIDED && Low->NumericValue == High->NumericValue)
        return *High; // Two equal STRIDED.
    default:
      break;
    }

    // Couldn't find a lesser upper bound than RANDOM.
    return SLEV(RANDOM);
  }

  bool isBOTTOM() const {
    return Kind == SLEVKind::BOTTOM;
  }

  bool isConstant() const {
    return Kind == SLEVKind::CONSTANT;
  }

  bool isUniform() const {
    switch (Kind) {
    case SLEVKind::CONSTANT:
    case SLEVKind::UNIFORM:
      return true;
    default:
      return false;
    }
  }

  bool isSTRIDED() const {
    return Kind == SLEVKind::STRIDED;
  }

  bool isRANDOM() const {
    return Kind == SLEVKind::RANDOM;
  }

  bool isTainted() const {
    return IsTainted;
  }

  /// \brief A Slev is diverging control-flow if it is marked as a branch
  /// condition and is known to be non-UNIFORM (a BOTTOM SLEV isn't, since it
  /// is not known to be anything).
  bool isDivergingControlFlow() const {
    return IsBranchCondition && Kind > UNIFORM;
  }

  /// \brief Convenience method for determining the special case of stride 1.
  bool isConsecutive() const {
    return Kind == SLEVKind::STRIDED && NumericValue == One;
  }

  /// \brief Convenience method for determining if the lanes have a constant
  /// stride, including a stride of zero.
  /// If you're interested if finding if the SLEV's Kind is STRIDED use
  /// isSTRIDED().
  bool isStrided() const {
    switch (Kind) {
    case SLEVKind::CONSTANT:
    case SLEVKind::UNIFORM:
    case SLEVKind::STRIDED:
      return true;
    default:
      return false;
    }
  }

  /// \brief Taint this SLEV's value in a derived-specific logic.
  virtual void taint(const void* IRU, std::set<SLEV*>& affected) {}

  static const char* SLEVKINDSTR[SLEVKind::RANDOM + 1];

  /// \brief Print a SLEV, either by as an expression or evaluated to its value.
  void print(raw_ostream& OS, bool eval) const {
    OS << "{" << Id << "|";
    if (IsTainted)
      OS << "x|";
    if (eval)
      printValue(OS);
    else
      printStructure(OS);
    if (Avr)
      OS << "|AVR-" << Avr->getNumber();
    if (IsBranchCondition)
      OS << "|BC";
    OS << "}";
  }

  /// \brief Utility method for collecting dynamically-allocated SLEVs. Since
  /// SLEVs are not necessarily the (sole) owners of their sub-SLEVs this method
  /// allows collecting them all into a set for single, safe deletion without
  /// getting into sub-SLEV ownership questions.
  /// This method should add to the set the object itself and any SLEV they have
  /// access to unless that SLEV will be deleted as part as the object's dtor.
  virtual void collectGarbage(std::set<SLEV*>& Repo) { Repo.insert(this); }

  /// \brief Print the SLEV structure (if it has one) and its evaluated value.
  void print(raw_ostream& OS) const {
    if (hasStructure()) {
      print(OS, false);
      OS << " = ";
    }
    print(OS, true);
  }

  void dump() const { print(dbgs()); }
};

/// \brief This SLEV calculates to a predefined value (one of the roots of the
/// analysis.
class SLEVPredefined : public SLEV {

private:

  SLEV Predefined;

public:

  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEV *Node) {
    return (Node->getSubtype() == SLEVPredefinedTy);
  }

  SLEVPredefined(const SLEV& S) : SLEV(SLEVPredefinedTy), Predefined(S) {}

  /// \brief Calculate this SLEV's value based on the current values of
  /// its dependecies.
  void calculate() override {
    copyValue(Predefined);
  }

  bool hasStructure() const override { return false; }
};

/// \brief This SLEV is used by unary operations whose semantics take their
/// single argument's SLEV.
class SLEVIdentity : public SLEV {

private:

  SLEV& Source;

public:

  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEV *Node) {
    return (Node->getSubtype() == SLEVIdentityTy);
  }

  SLEVIdentity(SLEV& S) : SLEV(SLEVIdentityTy), Source(S) { S.addUser(this); }

  /// \brief Calculate this SLEV's value based on the current values of
  /// its dependecies.
  void calculate() override {
    copyValue(Source);
  }

  void taint(const void* IRU, std::set<SLEV*>& affected) override {
    Source.taint(IRU, affected);
    if (Source.isTainted())
      markTainted();
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream& OS) const override {
    OS << "(id ";
    Source.print(OS, false);
    OS << ")";
  }

  void collectGarbage(std::set<SLEV*>& Repo) override {
    SLEV::collectGarbage(Repo);
    Source.collectGarbage(Repo);
  }
};

/// \class This SLEV is UNIFORM iff all its dependencies are, otherwise RANDOM.
class SLEVPreserveUniformity : public SLEV {

private:

  SmallPtrSet<SLEV*, 2> Dependencies;

public:

  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEV *Node) {
    return (Node->getSubtype() == SLEVPreserveUniformityTy);
  }

  SLEVPreserveUniformity() : SLEV(SLEVPreserveUniformityTy) {}

  void addDependency(SLEV* S) { 
    Dependencies.insert(S);
    S->addUser(this);
  }

  /// \brief Calculate this SLEV's value based on the current values of
  /// its dependecies.
  void calculate() override {
    for (SLEV* S : Dependencies) {
      if (S->isBOTTOM()) {
        setBOTTOM();
        return;
      }
      if (S->isUniform())
        setUNIFORM();
      else {
        setRANDOM();
        return;
      }
    }
  }

  void taint(const void* IRU, std::set<SLEV*>& affected) override {
    for (SLEV* S : Dependencies) {
      S->taint(IRU, affected);
      if (S->isTainted())
        markTainted();
    }
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream& OS) const override {
    OS << "(all-uniform ";
    bool First = true;
    for (SLEV* S : Dependencies) {
      if (!First)
        OS << " ";
      S->print(OS, false);
      First = false;
    }
    OS << ")";
  }

  void collectGarbage(std::set<SLEV*>& Repo) override {
    SLEV::collectGarbage(Repo);
    for (SLEV* D : Dependencies)
      D->collectGarbage(Repo);
  }
};

class SLEVAddress : public SLEV {

private:

  SLEV* Base;

  unsigned BaseSize;

  SmallVector<SLEV*, 3> Indexes;
  SmallVector<unsigned, 3> IndexSizes;

public:

  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEV *Node) {
    return (Node->getSubtype() == SLEVAddressTy);
  }

  SLEVAddress(SLEV* B, unsigned BS) :
      SLEV(SLEVAddressTy), Base(B), BaseSize(BS) {
    Base->addUser(this);
  }

  void addIndex(SLEV* S, unsigned BS) {
    Indexes.push_back(S);
    IndexSizes.push_back(BS);
    S->addUser(this);
  }

  /// \brief Calculate this SLEV's value based on the current values of
  /// its dependecies.
  void calculate() override {
    if (Base->isBOTTOM())
      return;
    if (!Base->isUniform()) {
      setRANDOM();
      return;
    }
    // TODO accurately.
    unsigned NumIndexes = Indexes.size();
    for (unsigned Position = 0; Position < NumIndexes; ++Position) {
      SLEV* S = Indexes[Position];
      if (S->isBOTTOM()) {
        setBOTTOM();
        return;
      }
      if (S->isUniform())
        continue;
      if (S->isStrided() && (Position == NumIndexes - 1)) {
        copyValue(*S);
        return;
      }
      setRANDOM();
      return;
    }
    setUNIFORM();
  }

  void taint(const void* IRU, std::set<SLEV*>& affected) override {
    Base->taint(IRU, affected);
    if (Base->isTainted())
        markTainted();
    for (SLEV* S : Indexes) {
      S->taint(IRU, affected);
      if (S->isTainted())
        markTainted();
    }
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream& OS) const override {
    OS << "(gep ";
    Base->print(OS, false);
    for (SLEV* S : Indexes) {
      OS << " ";
      S->print(OS, false);
    }
    OS << ")";
  }

  void collectGarbage(std::set<SLEV*>& Repo) override {
    SLEV::collectGarbage(Repo);
    Base->collectGarbage(Repo);
    for (SLEV* I : Indexes)
      I->collectGarbage(Repo);
  }
};

class SLEVLoad : public SLEV {

private:

  SLEV* Address;

public:

  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEV *Node) {
    return (Node->getSubtype() == SLEVLoadTy);
  }

  SLEVLoad(SLEV* A) : SLEV(SLEVLoadTy), Address(A) { A->addUser(this); }

  /// \brief Calculate this SLEV's value based on the current values of
  /// its dependecies.
  void calculate() override {
    if (Address->isBOTTOM())
      return;
    if (Address->isUniform())
      setUNIFORM();
    else
      setRANDOM();
  }

  void taint(const void* IRU, std::set<SLEV*>& affected) override {
    Address->taint(IRU, affected);
    if (Address->isTainted())
      markTainted();
  }

  void collectGarbage(std::set<SLEV*>& Repo) override {
    SLEV::collectGarbage(Repo);
    Address->collectGarbage(Repo);
  }
};

class SLEVAdd : public SLEV {

private:

  static SLEVKind Conversion[RANDOM + 1][RANDOM + 1];

  SLEV &LHS;
  SLEV &RHS;

  void calculate() override {
    SLEVKind CalculatedKind = Conversion[LHS.getKind()][RHS.getKind()];
    switch (CalculatedKind) {
    case BOTTOM:
      break;
    case CONSTANT:
      setCONSTANT(LHS.getConstant() + RHS.getConstant());
      break;
    case UNIFORM:
      setUNIFORM();
      break;
    case STRIDED:
      setSTRIDED(LHS.getStride() + RHS.getStride());
      break;
    case RANDOM:
      setRANDOM();
      break;
    }
  }

public:

  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEV *Node) {
    return (Node->getSubtype() == SLEVAddTy);
  }

  void taint(const void* IRU, std::set<SLEV*>& affected) override {
    LHS.taint(IRU, affected);
    RHS.taint(IRU, affected);
    if (LHS.isTainted() || RHS.isTainted())
      markTainted();
  }

  SLEVAdd(SLEV& L, SLEV& R) : SLEV(SLEVAddTy), LHS(L), RHS(R) {
    L.addUser(this);
    R.addUser(this);
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream& OS) const override {
    OS << "(add ";
    LHS.print(OS, false);
    OS << " ";
    RHS.print(OS, false);
    OS << ")";
  }

  void collectGarbage(std::set<SLEV*>& Repo) override {
    SLEV::collectGarbage(Repo);
    LHS.collectGarbage(Repo);
    RHS.collectGarbage(Repo);
  }
};

class SLEVSub : public SLEV {

private:

  static SLEVKind Conversion[RANDOM + 1][RANDOM + 1];

  SLEV &LHS;
  SLEV &RHS;

  void calculate() override {
    SLEVKind CalculatedKind = Conversion[LHS.getKind()][RHS.getKind()];
    switch (CalculatedKind) {
    case BOTTOM:
      break;
    case CONSTANT:
      setCONSTANT(LHS.getConstant() - RHS.getConstant());
      break;
    case UNIFORM:
      setUNIFORM();
      break;
    case STRIDED:
      setSTRIDED(LHS.getStride() - RHS.getStride());
      break;
    case RANDOM:
      setRANDOM();
      break;
    }
  }

public:

  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEV *Node) {
    return (Node->getSubtype() == SLEVSubTy);
  }

  void taint(const void* IRU, std::set<SLEV*>& affected) override {
    LHS.taint(IRU, affected);
    RHS.taint(IRU, affected);
    if (LHS.isTainted() || RHS.isTainted())
      markTainted();
  }

  SLEVSub(SLEV& L, SLEV& R) : SLEV(SLEVSubTy), LHS(L), RHS(R) {
    L.addUser(this);
    R.addUser(this);
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream& OS) const override {
    OS << "(sub ";
    LHS.print(OS, false);
    OS << " ";
    RHS.print(OS, false);
    OS << ")";
  }

  void collectGarbage(std::set<SLEV*>& Repo) override {
    SLEV::collectGarbage(Repo);
    LHS.collectGarbage(Repo);
    RHS.collectGarbage(Repo);
  }
};

class SLEVMul : public SLEV {

private:

  static SLEVKind Conversion[RANDOM + 1][RANDOM + 1];

  SLEV &LHS;
  SLEV &RHS;

  void calculate() override {
    SLEVKind CalculatedKind = Conversion[LHS.getKind()][RHS.getKind()];
    switch (CalculatedKind) {
    case BOTTOM:
      break;
    case CONSTANT:
      setCONSTANT(LHS.getConstant() * RHS.getConstant());
      break;
    case UNIFORM:
      setUNIFORM();
      break;
    case STRIDED:
      setSTRIDED(LHS.getStride() * RHS.getStride());
      break;
    case RANDOM:
      setRANDOM();
      break;
    }
  }

public:

  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEV *Node) {
    return (Node->getSubtype() == SLEVMulTy);
  }

  void taint(const void* IRU, std::set<SLEV*>& affected) override {
    LHS.taint(IRU, affected);
    RHS.taint(IRU, affected);
    if (LHS.isTainted() || RHS.isTainted())
      markTainted();
  }

  SLEVMul(SLEV& L, SLEV& R) : SLEV(SLEVMulTy), LHS(L), RHS(R) {
    L.addUser(this);
    R.addUser(this);
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream& OS) const override {
    OS << "(mul ";
    LHS.print(OS, false);
    OS << " ";
    RHS.print(OS, false);
    OS << ")";
  }

  void collectGarbage(std::set<SLEV*>& Repo) override {
    SLEV::collectGarbage(Repo);
    LHS.collectGarbage(Repo);
    RHS.collectGarbage(Repo);
  }
};

class SLEVDiv : public SLEV {

private:

  static SLEVKind Conversion[RANDOM + 1][RANDOM + 1];

  SLEV &LHS;
  SLEV &RHS;

  void calculate() override {
    SLEVKind CalculatedKind = Conversion[LHS.getKind()][RHS.getKind()];
    switch (CalculatedKind) {
    case BOTTOM:
      break;
    case CONSTANT:
      setCONSTANT(LHS.getConstant() / RHS.getConstant());
      break;
    case UNIFORM:
      setUNIFORM();
      break;
    case RANDOM:
      setRANDOM();
      break;
    default:
      llvm_unreachable("illegal conversion for SLEV-DIV");
    }
  }

public:

  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEV *Node) {
    return (Node->getSubtype() == SLEVDivTy);
  }

  void taint(const void* IRU, std::set<SLEV*>& affected) override {
    LHS.taint(IRU, affected);
    RHS.taint(IRU, affected);
    if (LHS.isTainted() || RHS.isTainted())
      markTainted();
  }

  SLEVDiv(SLEV& L, SLEV& R) : SLEV(SLEVDivTy), LHS(L), RHS(R) {
    L.addUser(this);
    R.addUser(this);
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream& OS) const override {
    OS << "(div ";
    LHS.print(OS, false);
    OS << " ";
    RHS.print(OS, false);
    OS << ")";
  }

  void collectGarbage(std::set<SLEV*>& Repo) override {
    SLEV::collectGarbage(Repo);
    LHS.collectGarbage(Repo);
    RHS.collectGarbage(Repo);
  }
};

class SLEVCmp : public SLEV {

private:

  static SLEVKind Conversion[RANDOM + 1][RANDOM + 1];

  SLEV &LHS;
  SLEV &RHS;

  void calculate() override {
    // TODO : logic should take into accout the compare operator and any
    // constants involved to derive piecewise-uniformity.
    SLEVKind CalculatedKind = Conversion[LHS.getKind()][RHS.getKind()];
    switch (CalculatedKind) {
    case BOTTOM:
      assert(getKind() == BOTTOM && "Calculated BOTTOM for non-BOTTOM exp");
      break;
    case CONSTANT:
      setCONSTANT(Zero); // TODO: actually compare the constants
      break;
    case UNIFORM:
      setUNIFORM();
      break;
    case RANDOM:
      setRANDOM();
      break;
    default:
      llvm_unreachable("illegal conversion for SLEV-CMP");
    }
  }

public:

  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEV *Node) {
    return (Node->getSubtype() == SLEVCmpTy);
  }

  void taint(const void* IRU, std::set<SLEV*>& affected) override {
    LHS.taint(IRU, affected);
    RHS.taint(IRU, affected);
    if (LHS.isTainted() || RHS.isTainted())
      markTainted();
  }

  SLEVCmp(SLEV& L, SLEV& R) : SLEV(SLEVCmpTy), LHS(L), RHS(R) {
    L.addUser(this);
    R.addUser(this);
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream& OS) const override {
    OS << "(cmp ";
    LHS.print(OS, false);
    OS << " ";
    RHS.print(OS, false);
    OS << ")";
  }

  void collectGarbage(std::set<SLEV*>& Repo) override {
    SLEV::collectGarbage(Repo);
    LHS.collectGarbage(Repo);
    RHS.collectGarbage(Repo);
  }
};

class SLEVUse : public SLEV {

private:

  SmallPtrSet<SLEV*, 2> ReachingSLEVs;

  SmallPtrSet<const void*, 2> IRUses;

  void taint(const void* IRUse, std::set<SLEV*>& affected) override {
    if (IRUses.count(IRUse)) {
      markTainted();
      affected.insert(this);
    }
  }

public:

  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEV *Node) {
    return (Node->getSubtype() == SLEVUseTy);
  }

  /// \brief Construct an empty Blend, to be added incoming SLEVs.
  SLEVUse() : SLEV(SLEVUseTy) {}

  /// \brief Add a reaching SLEV to this SLEV.
  void addReaching(SLEV* S, const void* IRUse) {
    assert(S && "Reaching SLEV is null");
    ReachingSLEVs.insert(S);
    S->addUser(this);
    IRUses.insert(IRUse);
  }

  const SmallPtrSetImpl<SLEV*>& getReachingSLEVs() { return ReachingSLEVs; }

  void calculate() override {
    /// If the SLEV was marked as tainted by control flow divergence then this
    /// SLEV does not behave as "either one of its reaching SLEVs" but rather as
    /// "all its reaching SLEVs" (i.e. each lane selects its reaching SLEV rather
    /// than all lanes selecting one (unknown) reaching SLEV.
    if (isTainted()) {
      setRANDOM();
      return;
    }
    /// This SLEV may behave as either one of its reaching SLEVs. Therefore, the
    /// best SLEV classification we can safely give it is the join of all the
    /// reaching SLEVs.
    SLEV Join(BOTTOM);
    for (auto* ReachingSLEV : ReachingSLEVs)
      Join.copyValue(join(Join, *ReachingSLEV));
    copyValue(Join);
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream& OS) const override {
    OS << "(blend";
    for (auto* ReachingSLEV : ReachingSLEVs) {
      OS << " ";
      ReachingSLEV->print(OS, true);
    }
    OS << ")";
  }

  void collectGarbage(std::set<SLEV*>& Repo) override {
    SLEV::collectGarbage(Repo);
    for (SLEV* R : ReachingSLEVs)
      if (!Repo.count(R)) // break use loops
        R->collectGarbage(Repo);
  }
};

/// \brief This class implements SIMD lane evolution on AVR programs.
///
/// This class depends on its concrete subclasses to produce the
/// IR-independent information it relies on (CFG, Dom/Postdom, Def-Use).
class SIMDLaneEvolutionBase : public FunctionPass {

public:

  /// \brief Utility class representing the Influence Region of a diverging
  /// instruction.
  class InfluenceRegion {
  public:
    typedef SmallPtrSet<AvrBasicBlock*, 32> InfluencedBBsTy;
    AvrBasicBlock* getBasicBlock() const { return BasicBlock; }
    AvrBasicBlock* getPostDominator() const { return PostDominator; }
    const InfluencedBBsTy& getInfluencedBasicBlocks() const {
      return InfluencedBasicBlocks;
    }
    void print(raw_ostream& OS) {
      OS << "<" << BasicBlock->getId() << " => ";
      bool First = true;
      for (AvrBasicBlock* I : InfluencedBasicBlocks) {
        if (!First)
          OS << ", ";
        OS << I->getId();
        First = false;
      }
      OS << " => " << PostDominator->getId() << ">";
    }
  private:
    InfluenceRegion(AvrBasicBlock* BB, AvrBasicBlock* PD) {
      BasicBlock = BB;
      PostDominator = PD;
    }
    InfluenceRegion(const InfluenceRegion& Other) :
        BasicBlock(Other.BasicBlock),
        PostDominator(Other.PostDominator),
        InfluencedBasicBlocks(Other.InfluencedBasicBlocks) {}
    AvrBasicBlock* BasicBlock;
    AvrBasicBlock* PostDominator;
    InfluencedBBsTy InfluencedBasicBlocks;
    friend SIMDLaneEvolutionBase;
  };

private:

  /// \brief Reset all internal data structures.
  void reset();

  /// \brief Calculate the Influence Region of a branch condition AVR.
  InfluenceRegion calculateInfluenceRegion(AVR* Avr);

  /// \brief A pointer to the AVR program to analyze. Provided by derived class.
  AVRGenerateBase* AV = nullptr;

  /// \brief A pointer to the AVR CFG of the AVR program being analyzed.
  /// Provided by derived class.
  AvrCFGBase* CFG = nullptr;

  /// \brief A pointer to the AVR Dominator Tree for the AVR program being
  /// analyzed. Maintained by runOnAvr().
  AvrDominatorTree* DominatorTree = nullptr;

  /// \brief A pointer to the AVR Postdominator Tree for the AVR program being
  /// analyzed. Maintained by runOnAvr().
  AvrDominatorTree* PostDominatorTree = nullptr;

  /// \brief A pointer to the Def Use analysis of the AVR program being analyzed.
  /// Provided by derived class.
  AvrDefUseBase* DefUseBase = nullptr;

protected:

  /// \brief Utility class for printing an opaque underlying IR variable.
  class IRValuePrinterBase {
  public:
    virtual void print(formatted_raw_ostream& OS, const void* Val) const = 0;
  };

  /// \bried Printer for underlying IR variables.
  /// Provided by derived class.
  IRValuePrinterBase* ValuePrinter = nullptr;

  DenseMap<const AVR*, SLEV*> SLEVs;

  std::vector<SLEV*>* FirstCalcQueue = nullptr;

  /// SLEVUse's may be constructed before all their reaching SLEVs are. This
  /// table tracks them for completion at the end of construction.
  typedef std::pair<AVR*, const void*> AVRVarPairTy;
  DenseMap<SLEVUse*, std::set<AVRVarPairTy> > UsesPendingDefs;

  /// Iteratively one set holds the changed from the previous iteration and
  /// the other holds the new changed values from the current iteration.
  typedef SmallPtrSet<SLEV*, 5> AffectedSLEVsTy;
  AffectedSLEVsTy Affected1;
  AffectedSLEVsTy Affected2;
  AffectedSLEVsTy *AffectedNew; // Set of SLEVs affected by current iteration
  AffectedSLEVsTy *AffectedOld; // The other AffectedSLEVsTy

  /// \brief Perform SLEV analysis on a given AVR program.
  void runOnAvr(AVRGenerateBase* A,
                AvrCFGBase* C,
                AvrDefUseBase* DU,
                IRValuePrinterBase* VP);

  const AvrDefUseBase* getDefUse() const { return DefUseBase; }
  
  const AvrCFGBase* getCFG() const { return CFG; }

  /// \brief Predefined SLEVs function as triggers to the whole analysis, so
  /// creating them is normally followed by pushing them into the first-calc
  /// queue. This is a utility method to do both.
  SLEVPredefined* createPredefinedSLEV(const SLEV& S) {
    assert(FirstCalcQueue && "Called with no queue to push SLEV into");
    SLEVPredefined* Slev = new SLEVPredefined(S);
    FirstCalcQueue->push_back(Slev);
    return Slev;
  }

  /// \brief Set the SLEV of an AVR.
  void setSLEV(AVR* Avr, SLEV* Slev);

  /// \brief The main computation function of the analysis: performs the data
  /// flow and reflects its effects over the control-flow.
  void calculate(SLEV* Slev);

  /// \brief Handle the effect of this SLEV's control divergence on the SLEVs
  /// in its Influence Region (See documentation for a detailed explanation).
  void handleControlDivergence(SLEV* Slev);

  void findPartiallyKillingPath(AvrBasicBlock* ConditionBB,
                                AvrBasicBlock* DefBB,
                                AvrCFGBase::PathTy& Result);

  bool isUseTaintedByLeakingIterations(AVR* Condition, AVR* Def, AVR* Use);

  bool isUseTaintedByTwoReachingDefs(AVR* Condition, AVR* Def, AVR* Use);

  /// \brief Mark as tainted SLEVs in the SLEV-tree related to an AVR Use
  /// tainted by the affects of diverging control flow.
  /// \param Use The AVR Use tainted by control flow
  /// \param Vars The set of underlying-IR opaque variables specifying which
  /// SLEVs to taint in Use's SLEV-tree.
  void taint(AVR* Use, const AvrDefUseBase::VarSetTy& Vars);

  typedef std::tuple<bool,
                     const AvrCFGBase::PathTy*,
                     const AvrCFGBase::PathTy*> DistinctPathsTy;

  /// \brief Given two sets of paths, find one path from each set s.t. the two
  /// paths are distinct except for a given set of shareable nodes.
  /// \param Lefts First set of paths.
  /// \param Rights Second set of paths.
  /// \param Except Set of shareable nodes.
  DistinctPathsTy findDistinctPaths(
      const std::set<AvrCFGBase::PathTy>& Lefts,
      const std::set<AvrCFGBase::PathTy>& Rights,
      const SmallPtrSet<const AvrBasicBlock*, 3>& Except) const;

  SIMDLaneEvolutionBase(char &ID);

  /// \brief Utility function for constructing the (given) binary operation
  /// SLEV for AVRExpressions.
  template<typename SLEVT, typename... OTHERS>
  SLEVT* createBinarySLEV(AVRExpression* AExpr, OTHERS... Tail);

  /// \brief Utility function for the common SLEV construction of a uniformity
  /// preserving SLEV off AVRExpressions.
  SLEVPreserveUniformity* createPreservingUniformitySLEV(AVRExpression* AExpr);

  /// \brief A utility function, part of SLEV construction, for adding the SLEV
  /// of some Reaching Def AVR to a SLEVUse (reachable via an underlying-IR
  /// variable). If the required SLEV has not been constructed yet, register it
  /// it as pending (to be filled later).
  void addReaching(SLEVUse* SU, AVR* ReachingDef, const void* IRUse) {
    assert(SU && "Null SU?");
    assert(ReachingDef && "Null Reachingdef?");
    assert(IRUse && "Null IRUse?");
    if (SLEVs.count(ReachingDef)) {
      SU->addReaching(SLEVs[ReachingDef], IRUse);
    }
    else
      UsesPendingDefs[SU].insert(AVRVarPairTy(ReachingDef, IRUse));
  }

public:

  virtual ~SIMDLaneEvolutionBase() { reset(); }

  virtual void construct(AVRExpression* AExpr);
  virtual void construct(AVRValueIR* AValueIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRPhiIR *APhiIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRLabelIR *ALabelIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRCallIR *ACallIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRReturnIR *AReturnIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRSelect *ASelect) { llvm_unreachable("Base"); }
  virtual void construct(AVRCompareIR *ACompareIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRBranchIR *ABranchIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRIfIR *AIfIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRValueHIR* AValueHIR) { llvm_unreachable("Base"); }
  virtual void entering(AVRLoopIR *ALoopIR) { llvm_unreachable("Base"); }
  virtual void exiting(AVRLoopIR *ALoopIR) { llvm_unreachable("Base"); }
  virtual void entering(AVRLoopHIR *ALoopHIR) { llvm_unreachable("Base"); }
  virtual void exiting(AVRLoopHIR *ALoopHIR) { llvm_unreachable("Base"); }

  void print(raw_ostream &OS, const Module* = nullptr) const override;

  static APSInt toAPSInt(int64_t Val) {
    APInt IntVal(64, Val); // int64_t, so bitwidth = 64 bit
    return APSInt(IntVal, false); // int64_t, so isUnsigned = false.
  }
};

class SIMDLaneEvolution : public SIMDLaneEvolutionBase {

private:

  DenseMap<const Value*, SLEV*> Value2SLEV;

  AvrDefUse* DefUse;

  bool isDef(AVRValueIR *AValueIR) {
    return AValueIR->getLLVMValue() == AValueIR->getLLVMInstruction();
  }

  /// \brief During SLEV construction, this is vector candidate loop.
  AVRLoopIR* VectorCandidate;

  /// \brief During SLEV construction, this is the induction variable that
  /// induces a linear stride in the vector candidate loop.
  SmallPtrSet<AVR*, 2> InductionVariableDefs;

  /// \brief During SLEV construction, this is the condition of the vector
  /// candidate loop's latch.
  AVRCompare* LatchCondition;

protected:

  typedef AvrDefUseBase::VarReachingAvrsMapTy ReachingAvrsTy;

  SLEV* constructValueSLEV(const Value* Val,
                           const ReachingAvrsTy& ReachingVars);

  /// \brief Concrete class for printing LLVM-IR underlying variables.
  class IRValuePrinter : public IRValuePrinterBase {
  public:
    void print(formatted_raw_ostream& OS, const void* Val) const override {
      OS << *(const Value*)Val;
    }
  };

public:

  // Pass Identification
  static char ID;

  SIMDLaneEvolution();

  virtual ~SIMDLaneEvolution();

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AvrDefUse>();
    AU.addRequired<AvrCFG>();
    AU.addRequired<AVRGenerate>();
    AU.addRequiredTransitive<LoopInfoWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override;

  void construct(AVRValueIR* AValueIR) override;
  void construct(AVRPhiIR *APhiIR) override;
  void construct(AVRLabelIR *ALabelIR) override;
  void construct(AVRCallIR *ACallIR) override;
  void construct(AVRReturnIR *AReturnIR) override;
  void construct(AVRSelect *ASelect) override;
  void construct(AVRCompareIR *ACompareIR) override;
  void construct(AVRBranchIR *ABranchIR) override;
  void construct(AVRIfIR *AIfIR) override;
  void entering(AVRLoopIR *ALoopIR) override;
  void exiting(AVRLoopIR *ALoopIR) override;
};

/// \brief Utility class keeping a DDRef -> AVRValue mapping only for Defs.
/// TODO: Check if IR2AVRVisitor can be reused instead of this class.
class DDRef2AVR {

public:

  DenseMap<DDRef*, AVRValueHIR*> Map;

  DDRef2AVR() {}

  /// Visit Functions
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }
  void visit(AVR* ANode) {}
  void postVisit(AVR* ANode) {}
  void visit(AVRValueHIR* AValueHIR);
};

class SIMDLaneEvolutionHIR : public SIMDLaneEvolutionBase {

private:

  AvrDefUseHIR* DefUseHIR;

  DDRef2AVR D2A;

protected:

  /// \brief Concrete class for printing HIR underlying variables.
  class IRValuePrinterHIR : public IRValuePrinterBase {
  public:
    void print(formatted_raw_ostream& OS, const void* Val) const override {
      ((const DDRef*)Val)->print(OS, true);
    }
  };

public:

  // Pass Identification
  static char ID;

  SIMDLaneEvolutionHIR();

  virtual ~SIMDLaneEvolutionHIR();

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AvrDefUseHIR>();
    AU.addRequired<AvrCFGHIR>();
    AU.addRequired<AVRGenerateHIR>();
    AU.addRequiredTransitive<HIRParser>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override;

  /// \brief Utility function that constructs a SLEV for a single blob.
  /// @param AValueHIR The AVRValue whose RegDDRef contains the blob.
  /// @return The constructed SLEV
  SLEV* constructSLEV(AVRValueHIR* AValueHIR, unsigned BlobIndex);

  /// \brief Utility function that onstructs a SLEV for a single canon-expr
  /// @param AValueHIR The AVRValue whose RegDDRef contains the canon-expr.
  /// @return The constructed SLEV
  SLEV* constructSLEV(AVRValueHIR* AValueHIR, CanonExpr& CE,
                      unsigned VectorizedDim);

  void construct(AVRValueHIR* AValueHIR) override;
  void entering(AVRLoopHIR *ALoopHIR) override;
  void exiting(AVRLoopHIR *ALoopHIR) override;

  /// \brief Visitor function for SCEV expressions. Used for constructing SLEVs
  /// for nested blobs.
  bool follow(const SCEV *SC) {
    return !isDone();
  }

  /// \brief Visitor function for SCEV expressions. Used for constructing SLEVs
  /// for nested blobs.
  bool isDone() const { return false; }

};

} // End VPO Vectorizer Namespace

/// \brief User-based graph traits for SLEVs.
template <> struct GraphTraits<vpo::SLEV*> {
  typedef vpo::SLEV NodeType;
  typedef vpo::SLEV::SLEVUsersTy::iterator ChildIteratorType;
  typedef standard_df_iterator<vpo::SLEV *> nodes_iterator;

  static NodeType *getEntryNode(vpo::SLEV *N) {
    return N;
  }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->getUsers().begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->getUsers().end();
  }

  static nodes_iterator nodes_begin(vpo::SLEV *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(vpo::SLEV *N) {
    return nodes_iterator(N, false);
  }

#if INTEL_CUSTOMIZATION
  static nodes_iterator nodes_begin(vpo::SLEV **N) {
    return nodes_iterator(*N, true);
  }

  static nodes_iterator nodes_end(vpo::SLEV **N) {
    return nodes_iterator(*N, false);
  }
#endif //INTEL_CUSTOMIZATON
};

} // End LLVM Namespace 

#endif  // LLVM_ANALYSIS_VPO_AVR_SLEV_H

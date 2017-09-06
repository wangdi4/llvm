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
//   TODO: In the same way we have a VPO def-use analysis, we should abstract IV
//   information under a wrapper analysis that gets IV information from the
//   underlying IR and provides an IR-independent AVR interface. This will allow
//   having only one implementation for the IV-related code in SLEV and other
//   AVR analyses/transformations.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_SLEV_ANALYSIS_H
#define LLVM_ANALYSIS_VPO_SLEV_ANALYSIS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvr.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOCFG.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPODefUse.h"

namespace llvm { // LLVM Namespace

namespace vpo { // VPO Vectorizer Namespace

// TODO: non-deterministic behavior?
typedef SmallPtrSet<AVR *, 2> AvrSetTy;

class SIMDLaneEvolutionAnalysisBase;
class SIMDLaneEvolutionAnalysis;
class SIMDLaneEvolutionAnalysisHIR;

/// \brief This class is both a SLEV value and the basis for computing it from
/// other SLEVs based on some specific internal structure.
/// The way a specific SLEVInstruction depends on others is modelled by the
/// various derived classes, which abstract away the concrete details of the
/// actual instructions the SLEVInstructions represent.
class SLEVInstruction : public SLEV {

public:
  typedef SmallPtrSet<SLEVInstruction *, 2> SLEVUsersTy;

protected:
  enum SubtypeTy {
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

  /// \brief Calculate this SLEV's value in a derived-specific logic.
  virtual void calculate() = 0;

  void markTainted() { IsTainted = true; }

  virtual bool hasStructure() const = 0;

  /// \brief A derived-specific print of the complex structure of the SLEV
  /// expression. Implementors should recurse into the printStructure of
  /// their sub-SLEVs. The default implementation (corresponds to having no
  /// structure) is to print the SLEV value.
  virtual void printStructure(raw_ostream &OS) const { printValue(OS); }

  SLEVInstruction(SubtypeTy St) : Subtype(St) {
    assignId();
    IsTainted = false;
    IsBranchCondition = false;
    Avr = nullptr;
  }

private:
  const SubtypeTy Subtype;

  /// SLEVs which would be affected by changes to this one.
  SLEVUsersTy Users;

  /// The AVR this SLEV (optionally) is attached to.
  AVR *Avr;

  void setAVR(AVR *A) { Avr = A; }

  static unsigned long long NextId;
  unsigned long long Id;

  void assignId() { Id = NextId++; }

  /// Is SLEV tainted by control-flow divergence.
  bool IsTainted;

  /// If set, this SLEV turning non-uniform causes control-flow divergence.
  bool IsBranchCondition;

  void setBranchCondition() { IsBranchCondition = true; }

  friend SIMDLaneEvolutionAnalysisBase;
  friend SIMDLaneEvolutionAnalysis;
  friend SIMDLaneEvolutionAnalysisHIR;

public:
  SubtypeTy getSubtype() const { return Subtype; }

  const SLEVUsersTy &getUsers() const { return Users; }

  void addUser(SLEVInstruction *User) { Users.insert(User); }

  AVR *getAVR() const { return Avr; }

  bool isTainted() const { return IsTainted; }

  /// \brief A Slev is diverging control-flow if it is marked as a branch
  /// condition and is known to be non-UNIFORM (a BOTTOM SLEV isn't, since it
  /// is not known to be anything).
  bool isDivergingControlFlow() const {
    return IsBranchCondition && getKind() > UNIFORM;
  }

  /*
  SLEVInstruction(const SLEVInstruction& Other) : SLEV(Other) {
    IsTainted = Other.IsTainted;
    Users = Other.Users;
    IsBranchCondition = Other.IsBranchCondition;
    Avr = Other.Avr;
  }
  */

  /// \brief Taint this SLEV's value in a derived-specific logic.
  virtual void taint(const void *IRU, std::set<SLEVInstruction *> &affected) {}

  /// \brief Utility method for collecting dynamically-allocated SLEVs. Since
  /// SLEVs are not necessarily the (sole) owners of their sub-SLEVs this method
  /// allows collecting them all into a set for single, safe deletion without
  /// getting into sub-SLEV ownership questions.
  /// This method should add to the set the object itself and any SLEV they have
  /// access to unless that SLEV will be deleted as part as the object's dtor.
  virtual void collectGarbage(std::set<SLEVInstruction *> &Repo) {
    Repo.insert(this);
  }

  /// \brief Print a SLEV, either as an expression or evaluated to its value.
  void print(raw_ostream &OS, bool eval) const {
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

  /// \brief Print the SLEV structure (if it has one) and its evaluated value.
  void print(raw_ostream &OS) const {
    if (hasStructure()) {
      print(OS, false);
      OS << " = ";
    }
    print(OS, true);
  }
};

/// \brief This SLEV calculates to a predefined value (one of the roots of the
/// analysis.
class SLEVPredefined : public SLEVInstruction {

private:
  SLEV Predefined;

public:
  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEVInstruction *Node) {
    return (Node->getSubtype() == SLEVPredefinedTy);
  }

  SLEVPredefined(const SLEV &S)
      : SLEVInstruction(SLEVPredefinedTy), Predefined(S) {}

  /// \brief Calculate this SLEV's value based on the current values of
  /// its dependecies.
  void calculate() override { copyValue(Predefined); }

  bool hasStructure() const override { return false; }
};

/// \brief This SLEV is used by unary operations whose semantics take their
/// single argument's SLEV.
class SLEVIdentity : public SLEVInstruction {

private:
  SLEVInstruction &Source;

public:
  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEVInstruction *Node) {
    return (Node->getSubtype() == SLEVIdentityTy);
  }

  SLEVIdentity(SLEVInstruction &S)
      : SLEVInstruction(SLEVIdentityTy), Source(S) {
    S.addUser(this);
  }

  /// \brief Calculate this SLEV's value based on the current values of
  /// its dependecies.
  void calculate() override { copyValue(Source); }

  void taint(const void *IRU, std::set<SLEVInstruction *> &affected) override {
    Source.taint(IRU, affected);
    if (Source.isTainted())
      markTainted();
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream &OS) const override {
    OS << "(id ";
    Source.print(OS, false);
    OS << ")";
  }

  void collectGarbage(std::set<SLEVInstruction *> &Repo) override {
    SLEVInstruction::collectGarbage(Repo);
    Source.collectGarbage(Repo);
  }
};

/// \class This SLEV is UNIFORM iff all its dependencies are, otherwise RANDOM.
class SLEVPreserveUniformity : public SLEVInstruction {

private:
  SmallPtrSet<SLEVInstruction *, 2> Dependencies;

public:
  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEVInstruction *Node) {
    return (Node->getSubtype() == SLEVPreserveUniformityTy);
  }

  SLEVPreserveUniformity() : SLEVInstruction(SLEVPreserveUniformityTy) {}

  void addDependency(SLEVInstruction *S) {
    Dependencies.insert(S);
    S->addUser(this);
  }

  /// \brief Calculate this SLEV's value based on the current values of
  /// its dependecies.
  void calculate() override {
    for (SLEVInstruction *S : Dependencies) {
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

  void taint(const void *IRU, std::set<SLEVInstruction *> &affected) override {
    for (SLEVInstruction *S : Dependencies) {
      S->taint(IRU, affected);
      if (S->isTainted())
        markTainted();
    }
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream &OS) const override {
    OS << "(all-uniform ";
    bool First = true;
    for (SLEVInstruction *S : Dependencies) {
      if (!First)
        OS << " ";
      S->print(OS, false);
      First = false;
    }
    OS << ")";
  }

  void collectGarbage(std::set<SLEVInstruction *> &Repo) override {
    SLEVInstruction::collectGarbage(Repo);
    for (SLEVInstruction *D : Dependencies)
      D->collectGarbage(Repo);
  }
};

class SLEVAddress : public SLEVInstruction {

private:
  SLEVInstruction *Base;

  unsigned BaseSize;

  SmallVector<SLEVInstruction *, 3> Indexes;
  SmallVector<unsigned, 3> IndexSizes;

public:
  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEVInstruction *Node) {
    return (Node->getSubtype() == SLEVAddressTy);
  }

  SLEVAddress(SLEVInstruction *B, unsigned BS)
      : SLEVInstruction(SLEVAddressTy), Base(B), BaseSize(BS) {
    Base->addUser(this);
  }

  void addIndex(SLEVInstruction *S, unsigned BS) {
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
      SLEVInstruction *S = Indexes[Position];
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

  void taint(const void *IRU, std::set<SLEVInstruction *> &affected) override {
    Base->taint(IRU, affected);
    if (Base->isTainted())
      markTainted();
    for (SLEVInstruction *S : Indexes) {
      S->taint(IRU, affected);
      if (S->isTainted())
        markTainted();
    }
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream &OS) const override {
    OS << "(gep ";
    Base->print(OS, false);
    for (SLEVInstruction *S : Indexes) {
      OS << " ";
      S->print(OS, false);
    }
    OS << ")";
  }

  void collectGarbage(std::set<SLEVInstruction *> &Repo) override {
    SLEVInstruction::collectGarbage(Repo);
    Base->collectGarbage(Repo);
    for (SLEVInstruction *I : Indexes)
      I->collectGarbage(Repo);
  }
};

class SLEVLoad : public SLEVInstruction {

private:
  SLEVInstruction *Address;

public:
  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEVInstruction *Node) {
    return (Node->getSubtype() == SLEVLoadTy);
  }

  SLEVLoad(SLEVInstruction *A) : SLEVInstruction(SLEVLoadTy), Address(A) {
    A->addUser(this);
  }

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

  void taint(const void *IRU, std::set<SLEVInstruction *> &affected) override {
    Address->taint(IRU, affected);
    if (Address->isTainted())
      markTainted();
  }

  void collectGarbage(std::set<SLEVInstruction *> &Repo) override {
    SLEVInstruction::collectGarbage(Repo);
    Address->collectGarbage(Repo);
  }
};

class SLEVAdd : public SLEVInstruction {

private:
  static SLEVKind Conversion[RANDOM + 1][RANDOM + 1];

  SLEVInstruction &LHS;
  SLEVInstruction &RHS;

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
  static bool classof(const SLEVInstruction *Node) {
    return (Node->getSubtype() == SLEVAddTy);
  }

  void taint(const void *IRU, std::set<SLEVInstruction *> &affected) override {
    LHS.taint(IRU, affected);
    RHS.taint(IRU, affected);
    if (LHS.isTainted() || RHS.isTainted())
      markTainted();
  }

  SLEVAdd(SLEVInstruction &L, SLEVInstruction &R)
      : SLEVInstruction(SLEVAddTy), LHS(L), RHS(R) {
    L.addUser(this);
    R.addUser(this);
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream &OS) const override {
    OS << "(add ";
    LHS.print(OS, false);
    OS << " ";
    RHS.print(OS, false);
    OS << ")";
  }

  void collectGarbage(std::set<SLEVInstruction *> &Repo) override {
    SLEVInstruction::collectGarbage(Repo);
    LHS.collectGarbage(Repo);
    RHS.collectGarbage(Repo);
  }
};

class SLEVSub : public SLEVInstruction {

private:
  static SLEVKind Conversion[RANDOM + 1][RANDOM + 1];

  SLEVInstruction &LHS;
  SLEVInstruction &RHS;

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
  static bool classof(const SLEVInstruction *Node) {
    return (Node->getSubtype() == SLEVSubTy);
  }

  void taint(const void *IRU, std::set<SLEVInstruction *> &affected) override {
    LHS.taint(IRU, affected);
    RHS.taint(IRU, affected);
    if (LHS.isTainted() || RHS.isTainted())
      markTainted();
  }

  SLEVSub(SLEVInstruction &L, SLEVInstruction &R)
      : SLEVInstruction(SLEVSubTy), LHS(L), RHS(R) {
    L.addUser(this);
    R.addUser(this);
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream &OS) const override {
    OS << "(sub ";
    LHS.print(OS, false);
    OS << " ";
    RHS.print(OS, false);
    OS << ")";
  }

  void collectGarbage(std::set<SLEVInstruction *> &Repo) override {
    SLEVInstruction::collectGarbage(Repo);
    LHS.collectGarbage(Repo);
    RHS.collectGarbage(Repo);
  }
};

class SLEVMul : public SLEVInstruction {

private:
  static SLEVKind Conversion[RANDOM + 1][RANDOM + 1];

  SLEVInstruction &LHS;
  SLEVInstruction &RHS;

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
    case STRIDED: {
      Number LStride = LHS.getStride();
      Number RStride = RHS.getStride();
      assert((LStride != Zero || RStride != Zero) && "Computes to STRIDED?");
      if (LStride == Zero)
        LStride = LHS.getConstant();
      if (RStride == Zero)
        RStride = RHS.getConstant();
      setSTRIDED(LStride * RStride);
    } break;
    case RANDOM:
      setRANDOM();
      break;
    }
  }

public:
  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEVInstruction *Node) {
    return (Node->getSubtype() == SLEVMulTy);
  }

  void taint(const void *IRU, std::set<SLEVInstruction *> &affected) override {
    LHS.taint(IRU, affected);
    RHS.taint(IRU, affected);
    if (LHS.isTainted() || RHS.isTainted())
      markTainted();
  }

  SLEVMul(SLEVInstruction &L, SLEVInstruction &R)
      : SLEVInstruction(SLEVMulTy), LHS(L), RHS(R) {
    L.addUser(this);
    R.addUser(this);
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream &OS) const override {
    OS << "(mul ";
    LHS.print(OS, false);
    OS << " ";
    RHS.print(OS, false);
    OS << ")";
  }

  void collectGarbage(std::set<SLEVInstruction *> &Repo) override {
    SLEVInstruction::collectGarbage(Repo);
    LHS.collectGarbage(Repo);
    RHS.collectGarbage(Repo);
  }
};

class SLEVDiv : public SLEVInstruction {

private:
  static SLEVKind Conversion[RANDOM + 1][RANDOM + 1];

  SLEVInstruction &LHS;
  SLEVInstruction &RHS;

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
  static bool classof(const SLEVInstruction *Node) {
    return (Node->getSubtype() == SLEVDivTy);
  }

  void taint(const void *IRU, std::set<SLEVInstruction *> &affected) override {
    LHS.taint(IRU, affected);
    RHS.taint(IRU, affected);
    if (LHS.isTainted() || RHS.isTainted())
      markTainted();
  }

  SLEVDiv(SLEVInstruction &L, SLEVInstruction &R)
      : SLEVInstruction(SLEVDivTy), LHS(L), RHS(R) {
    L.addUser(this);
    R.addUser(this);
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream &OS) const override {
    OS << "(div ";
    LHS.print(OS, false);
    OS << " ";
    RHS.print(OS, false);
    OS << ")";
  }

  void collectGarbage(std::set<SLEVInstruction *> &Repo) override {
    SLEVInstruction::collectGarbage(Repo);
    LHS.collectGarbage(Repo);
    RHS.collectGarbage(Repo);
  }
};

class SLEVCmp : public SLEVInstruction {

private:
  static SLEVKind Conversion[RANDOM + 1][RANDOM + 1];

  SLEVInstruction &LHS;
  SLEVInstruction &RHS;

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
  static bool classof(const SLEVInstruction *Node) {
    return (Node->getSubtype() == SLEVCmpTy);
  }

  void taint(const void *IRU, std::set<SLEVInstruction *> &affected) override {
    LHS.taint(IRU, affected);
    RHS.taint(IRU, affected);
    if (LHS.isTainted() || RHS.isTainted())
      markTainted();
  }

  SLEVCmp(SLEVInstruction &L, SLEVInstruction &R)
      : SLEVInstruction(SLEVCmpTy), LHS(L), RHS(R) {
    L.addUser(this);
    R.addUser(this);
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream &OS) const override {
    OS << "(cmp ";
    LHS.print(OS, false);
    OS << " ";
    RHS.print(OS, false);
    OS << ")";
  }

  void collectGarbage(std::set<SLEVInstruction *> &Repo) override {
    SLEVInstruction::collectGarbage(Repo);
    LHS.collectGarbage(Repo);
    RHS.collectGarbage(Repo);
  }
};

class SLEVUse : public SLEVInstruction {

private:
  SmallPtrSet<SLEVInstruction *, 2> ReachingSLEVs;

  SmallPtrSet<const void *, 2> IRUses;

  void taint(const void *IRUse,
             std::set<SLEVInstruction *> &affected) override {
    if (IRUses.count(IRUse)) {
      markTainted();
      affected.insert(this);
    }
  }

public:
  /// \brief Method for supporting type inquiry.
  static bool classof(const SLEVInstruction *Node) {
    return (Node->getSubtype() == SLEVUseTy);
  }

  /// \brief Construct an empty Blend, to be added incoming SLEVs.
  SLEVUse() : SLEVInstruction(SLEVUseTy) {}

  /// \brief Add a reaching SLEV to this SLEV.
  void addReaching(SLEVInstruction *S, const void *IRUse) {
    assert(S && "Reaching SLEV is null");
    ReachingSLEVs.insert(S);
    S->addUser(this);
    IRUses.insert(IRUse);
  }

  const SmallPtrSetImpl<SLEVInstruction *> &getReachingSLEVs() {
    return ReachingSLEVs;
  }

  void calculate() override {
    /// If the SLEV was marked as tainted by control flow divergence then this
    /// SLEV does not behave as "either one of its reaching SLEVs" but rather as
    /// "all its reaching SLEVs" (i.e. each lane selects its reaching SLEV
    /// rather
    /// than all lanes selecting one (unknown) reaching SLEV.
    if (isTainted()) {
      setRANDOM();
      return;
    }
    /// This SLEV may behave as either one of its reaching SLEVs. Therefore, the
    /// best SLEV classification we can safely give it is the join of all the
    /// reaching SLEVs.
    SLEV Join(BOTTOM);
    for (SLEVInstruction *ReachingSLEV : ReachingSLEVs)
      Join.copyValue(join(Join, *ReachingSLEV));
    copyValue(Join);
  }

  bool hasStructure() const override { return true; }

  void printStructure(raw_ostream &OS) const override {
    OS << "(blend";
    for (SLEVInstruction *ReachingSLEV : ReachingSLEVs) {
      OS << " ";
      ReachingSLEV->print(OS, true);
    }
    OS << ")";
  }

  void collectGarbage(std::set<SLEVInstruction *> &Repo) override {
    SLEVInstruction::collectGarbage(Repo);
    for (SLEVInstruction *ReachingSLEV : ReachingSLEVs)
      if (!Repo.count(ReachingSLEV)) // break use loops
        ReachingSLEV->collectGarbage(Repo);
  }
};

/// \brief Utility class for printing an opaque underlying IR variable.
class IRValuePrinterBase {
public:
  virtual void print(formatted_raw_ostream &OS, const void *Val) const = 0;
};

/// \brief Concrete class for printing LLVM-IR underlying variables.
class IRValuePrinter : public IRValuePrinterBase {
public:
  void print(formatted_raw_ostream &OS, const void *Val) const override {
    OS << *(const Value *)Val;
  }
};

/// \brief Concrete class for printing HIR underlying variables.
class IRValuePrinterHIR : public IRValuePrinterBase {
public:
  void print(formatted_raw_ostream &OS, const void *Val) const override {
    ((const DDRef *)Val)->print(OS, true);
  }
};

/// \brief This class implements SIMD lane evolution analysis for AVR programs.
///
/// This class depends on its concrete subclasses to produce the
/// IR-independent information it relies on (CFG, Dom/Postdom, Def-Use).
class SIMDLaneEvolutionAnalysisBase {

  friend class SIMDLaneEvolutionAnalysisUtilBase;

public:
  /// \brief Utility class representing the Influence Region of a diverging
  /// instruction.
  class InfluenceRegion {
  public:
    typedef SmallPtrSet<AvrBasicBlock *, 32> InfluencedBBsTy;
    AvrBasicBlock *getBasicBlock() const { return BasicBlock; }
    AvrBasicBlock *getPostDominator() const { return PostDominator; }
    const InfluencedBBsTy &getInfluencedBasicBlocks() const {
      return InfluencedBasicBlocks;
    }
    void print(raw_ostream &OS) {
      OS << "<" << BasicBlock->getId() << " => ";
      bool First = true;
      for (AvrBasicBlock *I : InfluencedBasicBlocks) {
        if (!First)
          OS << ", ";
        OS << I->getId();
        First = false;
      }
      OS << " => " << PostDominator->getId() << ">";
    }

  private:
    InfluenceRegion(AvrBasicBlock *BB, AvrBasicBlock *PD) {
      BasicBlock = BB;
      PostDominator = PD;
    }
    InfluenceRegion(const InfluenceRegion &Other)
        : BasicBlock(Other.BasicBlock), PostDominator(Other.PostDominator),
          InfluencedBasicBlocks(Other.InfluencedBasicBlocks) {}
    AvrBasicBlock *BasicBlock;
    AvrBasicBlock *PostDominator;
    InfluencedBBsTy InfluencedBasicBlocks;
    friend SIMDLaneEvolutionAnalysisBase;
  };

private:
  /// \brief Calculate the Influence Region of a branch condition AVR.
  InfluenceRegion calculateInfluenceRegion(AVR *Avr);

  /// \brief Analysis range - begin
  AvrItr Begin;

  /// \brief Analysis range - end
  AvrItr End;

  /// \brief A pointer to the AVR CFG of the AVR program being analyzed.
  AvrCFGBase *CFG = nullptr;

  /// \brief A pointer to the AVR Dominator Tree for the AVR program being
  /// analyzed. Maintained by runOnAvr().
  AvrDominatorTree *DominatorTree = nullptr;

  /// \brief A pointer to the AVR Postdominator Tree for the AVR program being
  /// analyzed. Maintained by runOnAvr().
  AvrPostDominatorTree *PostDominatorTree = nullptr;

  /// \brief A pointer to the Def Use analysis of the AVR program being
  /// analyzed.
  /// Provided by derived class.
  AvrDefUseBase *DefUseBase = nullptr;

protected:
  MapVector<AVR *, SLEVInstruction *> SLEVs;

  /// \brief During SLEV construction, this is vector candidate loop.
  AVRLoop *VectorCandidate = nullptr;

  std::vector<SLEVInstruction *> *FirstCalcQueue = nullptr;

  /// SLEVUse's may be constructed before all their reaching SLEVs are. This
  /// table tracks them for completion at the end of construction.
  typedef std::pair<AVR *, const void *> AVRVarPairTy;
  DenseMap<SLEVUse *, std::set<AVRVarPairTy>> UsesPendingDefs;

  /// Iteratively one set holds the changed from the previous iteration and
  /// the other holds the new changed values from the current iteration.
  typedef SmallPtrSet<SLEVInstruction *, 5> AffectedSLEVsTy;
  AffectedSLEVsTy Affected1;
  AffectedSLEVsTy Affected2;
  AffectedSLEVsTy *AffectedNew; // Set of SLEVs affected by current iteration
  AffectedSLEVsTy *AffectedOld; // The other AffectedSLEVsTy

  const AvrDefUseBase *getDefUse() const { return DefUseBase; }

  const AvrCFGBase *getCFG() const { return CFG; }

  AvrDominatorTree *getDominatorTree() { return DominatorTree; }

  AvrPostDominatorTree *getPostDominatorTree() { return PostDominatorTree; }

  /// \brief Predefined SLEVs function as triggers to the whole analysis, so
  /// creating them is normally followed by pushing them into the first-calc
  /// queue. This is a utility method to do both.
  SLEVPredefined *createPredefinedSLEV(const SLEV &S) {
    assert(FirstCalcQueue && "Called with no queue to push SLEV into");
    SLEVPredefined *Slev = new SLEVPredefined(S);
    FirstCalcQueue->push_back(Slev);
    return Slev;
  }

  /// \brief Set the SLEV of an AVR.
  void setSLEV(AVR *Avr, SLEVInstruction *Slev);

  /// \brief The main computation function of the analysis: performs the data
  /// flow and reflects its effects over the control-flow.
  void calculate(SLEVInstruction *Slev);

  /// \brief Handle the effect of this SLEV's control divergence on the SLEVs
  /// in its Influence Region (See documentation for a detailed explanation).
  void handleControlDivergence(SLEVInstruction *Slev);

  void findPartiallyKillingPath(AvrBasicBlock *ConditionBB,
                                AvrBasicBlock *DefBB,
                                AvrCFGBase::PathTy &Result);

  bool isUseTaintedByLeakingIterations(AVR *Condition, AVR *Def, AVR *Use);

  bool isUseTaintedByTwoReachingDefs(AVR *Condition, AVR *Def, AVR *Use);

  /// \brief Mark as tainted SLEVs in the SLEV-tree related to an AVR Use
  /// tainted by the affects of diverging control flow.
  /// \param Use The AVR Use tainted by control flow
  /// \param Vars The set of underlying-IR opaque variables specifying which
  /// SLEVs to taint in Use's SLEV-tree.
  void taint(AVR *Use, const AvrDefUseBase::VarSetTy &Vars);

  typedef std::tuple<bool, const AvrCFGBase::PathTy *,
                     const AvrCFGBase::PathTy *>
      DistinctPathsTy;

  /// \brief Given two sets of paths, find one path from each set s.t. the two
  /// paths are distinct except for a given set of shareable nodes.
  /// \param Lefts First set of paths.
  /// \param Rights Second set of paths.
  /// \param Except Set of shareable nodes.
  DistinctPathsTy
  findDistinctPaths(const std::set<AvrCFGBase::PathTy> &Lefts,
                    const std::set<AvrCFGBase::PathTy> &Rights,
                    const SmallPtrSet<const AvrBasicBlock *, 3> &Except) const;

  SIMDLaneEvolutionAnalysisBase(AvrDefUseBase *DUBase);

  /// \brief Utility function for constructing the (given) binary operation
  /// SLEV for AVRExpressions.
  template <typename SLEVT, typename... OTHERS>
  SLEVT *createBinarySLEV(AVRExpression *AExpr, OTHERS... Tail);

  /// \brief Utility function for the common SLEV construction of a uniformity
  /// preserving SLEV off AVRExpressions.
  SLEVPreserveUniformity *createPreservingUniformitySLEV(AVRExpression *AExpr);

  /// \brief A utility function, part of SLEV construction, for adding the SLEV
  /// of some Reaching Def AVR to a SLEVUse (reachable via an underlying-IR
  /// variable). If the required SLEV has not been constructed yet, register it
  /// it as pending (to be filled later).
  void addReaching(SLEVUse *SU, AVR *ReachingDef, const void *IRUse) {
    assert(SU && "Null SU?");
    assert(ReachingDef && "Null Reachingdef?");
    assert(IRUse && "Null IRUse?");
    if (SLEVs.count(ReachingDef)) {
      SU->addReaching(SLEVs[ReachingDef], IRUse);
    } else
      UsesPendingDefs[SU].insert(AVRVarPairTy(ReachingDef, IRUse));
  }

public:
  virtual ~SIMDLaneEvolutionAnalysisBase();

  /// \brief Perform SLEV analysis on a given AVR program.
  void runOnAvr(AvrItr B, AvrItr E, IRValuePrinterBase *VP);

  const AvrCFGBase *getCFG() { return CFG; }

  virtual void construct(AVRExpression *AExpr);
  virtual void construct(AVRValue *AVal);
  virtual void construct(AVRValueIR *AValueIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRPhiIR *APhiIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRLabelIR *ALabelIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRCallIR *ACallIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRReturnIR *AReturnIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRSelect *ASelect) { llvm_unreachable("Base"); }
  virtual void construct(AVRCompareIR *ACompareIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRBranchIR *ABranchIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRIfIR *AIfIR) { llvm_unreachable("Base"); }
  virtual void construct(AVRValueHIR *AValueHIR) { llvm_unreachable("Base"); }
  virtual void entering(AVRLoopIR *ALoopIR) { llvm_unreachable("Base"); }
  virtual void exiting(AVRLoopIR *ALoopIR) { llvm_unreachable("Base"); }
  virtual void entering(AVRLoopHIR *ALoopHIR) { llvm_unreachable("Base"); }
  virtual void exiting(AVRLoopHIR *ALoopHIR) { llvm_unreachable("Base"); }

  void propagateSLEV(AVRValue *AVal);
  
  /// \brief Common implementation for AVRValuesIR and AVRValueHIR.
  SLEVInstruction *constructSLEV(AVRValue *AValue);

  void print(raw_ostream &OS) const;

  static APSInt toAPSInt(const APInt &Val) {
    return APSInt(Val, false); // int64_t, so isUnsigned = false.
  }

  static APSInt toAPSInt(int64_t Val) {
    APInt IntVal(64, Val); // int64_t, so bitwidth = 64 bit
    return toAPSInt(IntVal);
  }
};

/// \brief Base class of a utility for running the analysis. This class creates
/// a (concrete) analysis object, runs the analysis and returns it as read-only.
/// Using these utility classes is the only way to run the analysis.
class SIMDLaneEvolutionAnalysisUtilBase {

private:
  IRValuePrinterBase &ValuePrinterBase;

protected:
  SIMDLaneEvolutionAnalysisUtilBase(IRValuePrinterBase &VPB)
      : ValuePrinterBase(VPB) {}

  virtual SIMDLaneEvolutionAnalysisBase *createAnalysis() = 0;

public:
  void runOnAvr(AvrItr Begin, AvrItr End);
};

/// \brief IR-based SLEV analysis.
class SIMDLaneEvolutionAnalysis : public SIMDLaneEvolutionAnalysisBase {

  friend class SIMDLaneEvolutionAnalysisUtil;
  friend class SIMDLaneEvolution;

private:
  DenseMap<const Value *, SLEVInstruction *> Value2SLEV;

  AvrDefUse *DefUse;

  bool isDef(AVRValueIR *AValueIR) {
    return AValueIR->getLLVMValue() == AValueIR->getLLVMInstruction();
  }

  /// \brief During SLEV construction, this is the induction variable that
  /// induces a linear stride in the vector candidate loop.
  SmallPtrSet<AVR *, 2> InductionVariableDefs;

  /// \brief During SLEV construction, this is the condition of the vector
  /// candidate loop's latch.
  AVRCompare *LatchCondition = nullptr;

  SIMDLaneEvolutionAnalysis(AvrDefUse *DU)
      : SIMDLaneEvolutionAnalysisBase(DU), DefUse(DU){};

protected:
  typedef AvrDefUseBase::VarReachingAvrsMapTy ReachingAvrsTy;

  SLEVInstruction *constructValueSLEV(const Value *Val,
                                      const ReachingAvrsTy &ReachingVars);

public:
  virtual ~SIMDLaneEvolutionAnalysis() {}

  void construct(AVRValueIR *AValueIR) override;
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

/// \brief IR-based utility for running the analysis.
class SIMDLaneEvolutionAnalysisUtil : public SIMDLaneEvolutionAnalysisUtilBase {

private:
  AvrDefUse &DefUse;
  IRValuePrinter ValuePrinter;

protected:
  SIMDLaneEvolutionAnalysisBase *createAnalysis() override {
    return new SIMDLaneEvolutionAnalysis(&DefUse);
  }

public:
  SIMDLaneEvolutionAnalysisUtil(AvrDefUse &DU)
      : SIMDLaneEvolutionAnalysisUtilBase(ValuePrinter), DefUse(DU) {}
};

/// \brief IR-based function-level pass SLEV analysis.
class SIMDLaneEvolution : public FunctionPass {

private:
  SIMDLaneEvolutionAnalysisBase *SLEV = nullptr;

public:
  // Pass Identification
  static char ID;

  SIMDLaneEvolution();

  virtual ~SIMDLaneEvolution();

  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnFunction(Function &F) override;

  void print(raw_ostream &OS, const Module * = nullptr) const override {
    if (SLEV)
      SLEV->print(OS);
    else
      OS << "No SLEV information available\n";
  }
};

/// \brief Utility class keeping a DDRef -> AVRValue mapping only for Defs.
/// TODO: Check if IR2AVRVisitor can be reused instead of this class.
class DDRef2AVR {

public:
  DenseMap<DDRef *, AVRValueHIR *> Map;

  DDRef2AVR() {}

  /// Visit Functions
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }
  void visit(AVR *ANode) {}
  void postVisit(AVR *ANode) {}
  void visit(AVRValueHIR *AValueHIR);
};

/// \brief HIR-based SLEV analysis.
class SIMDLaneEvolutionAnalysisHIR : public SIMDLaneEvolutionAnalysisBase {

  friend class SIMDLaneEvolutionAnalysisUtilHIR;
  friend class SIMDLaneEvolutionHIR;

private:
  SIMDLaneEvolutionAnalysisHIR(AvrDefUseHIR *DU)
      : SIMDLaneEvolutionAnalysisBase(DU), DefUseHIR(DU) {}

  AvrDefUseHIR *DefUseHIR;

  DDRef2AVR D2A;

protected:
public:
  virtual ~SIMDLaneEvolutionAnalysisHIR() {}

  /// \brief Utility function that constructs a SLEV for a simple (decomposed)
  /// @param AValueHIR
  /// @return The constructed SLEV
  SLEVInstruction *constructSLEV(AVRValueHIR *AValueHIR);

  void construct(AVRValueHIR *AValueHIR) override;
  void entering(AVRLoopHIR *ALoopHIR) override;
  void exiting(AVRLoopHIR *ALoopHIR) override;
};

/// \brief HIR-based utility for running the analysis.
class SIMDLaneEvolutionAnalysisUtilHIR
    : public SIMDLaneEvolutionAnalysisUtilBase {

private:
  AvrDefUseHIR &DefUse;
  IRValuePrinterHIR ValuePrinter;

protected:
  SIMDLaneEvolutionAnalysisBase *createAnalysis() override {
    return new SIMDLaneEvolutionAnalysisHIR(&DefUse);
  }

public:
  SIMDLaneEvolutionAnalysisUtilHIR(AvrDefUseHIR &DU)
      : SIMDLaneEvolutionAnalysisUtilBase(ValuePrinter), DefUse(DU) {}
};

/// \brief HIR-based function-level pass SLEV analysis.
class SIMDLaneEvolutionHIR : public FunctionPass {

private:
  SIMDLaneEvolutionAnalysisBase *SLEV = nullptr;

public:
  // Pass Identification
  static char ID;

  SIMDLaneEvolutionHIR();

  virtual ~SIMDLaneEvolutionHIR();

  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnFunction(Function &F) override;

  void print(raw_ostream &OS, const Module * = nullptr) const override {
    if (SLEV)
      SLEV->print(OS);
    else
      OS << "No SLEV information available\n";
  }
};

} // End VPO Vectorizer Namespace

/// \brief User-based graph traits for SLEVs.
template <> struct GraphTraits<vpo::SLEVInstruction *> {
  typedef vpo::SLEVInstruction NodeType;
  typedef vpo::SLEVInstruction *NodeRef;
  typedef vpo::SLEVInstruction::SLEVUsersTy::iterator ChildIteratorType;
  typedef standard_df_iterator2<vpo::SLEVInstruction *> nodes_iterator;

  static NodeType *getEntryNode(vpo::SLEVInstruction *N) { return N; }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->getUsers().begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->getUsers().end();
  }

  static nodes_iterator nodes_begin(vpo::SLEVInstruction *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(vpo::SLEVInstruction *N) {
    return nodes_iterator(N, false);
  }

#if INTEL_CUSTOMIZATION
  static nodes_iterator nodes_begin(vpo::SLEVInstruction **N) {
    return nodes_iterator(*N, true);
  }

  static nodes_iterator nodes_end(vpo::SLEVInstruction **N) {
    return nodes_iterator(*N, false);
  }
#endif // INTEL_CUSTOMIZATON
};

} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_SLEV_ANALYSIS_H

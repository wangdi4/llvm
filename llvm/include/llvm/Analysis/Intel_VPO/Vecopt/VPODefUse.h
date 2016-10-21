//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPODefUse.h -- Defines the AVR-level Def-Use information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_DEF_USE_H
#define LLVM_ANALYSIS_VPO_DEF_USE_H

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/DenseMap.h"

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvr.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrVisitor.h"

namespace llvm { // LLVM Namespace

namespace vpo {  // VPO Vectorizer Namespace

typedef SmallPtrSet<AVR*, 2> AvrSetTy;

/// \brief Utility class to provide the inverse-mapping from the underlying IR
/// values to the AVRs holding them.
class IR2AVRVisitor {

public:

  typedef DenseMap<DDRef*, AVRValueHIR*> DDRef2AVRTy;
  typedef DenseMap<const Value*, AVR*> Value2DefAVRTy;
  typedef DenseMap<const Value*, AvrSetTy> Value2UsesAVRTy;

private:

  /// \brief Maintain the inverse pointer from a DDRef to the AVR node wrapping
  /// it.
  DDRef2AVRTy DDRef2AVR;

  /// \brief Maintain the inverse pointer from a Value to the AVR node wrapping
  /// its Def in the underlying IR.
  Value2DefAVRTy Value2DefAVR;

  /// \brief Maintain the inverse pointer from a Value to the AVR nodes wrapping
  /// its Uses in the underlying IR.
  Value2UsesAVRTy Value2UsesAVR;

  /// \brief Register an AVR node as the Def of some LLVM-IR Instruction.
  void registerDef(const Instruction* Inst, AVR* Avr) {
    Value2DefAVR[Inst] = Avr;
  }

  /// \brief Register an AVR node as a Use of some LLVM-IR instruction.
  void registerUser(const Instruction* Inst, AVR* Avr) {
    Value2UsesAVR[Inst].insert(Avr);
  }

  /// \brief Register an AVR node as a Use of every operand used by some
  /// LLVM-IR Instructions.
  void registerUses(const Instruction* Inst, AVR* Avr) {
    for (Value* Operand : Inst->operands())
      Value2UsesAVR[Operand].insert(Avr);
  }

  void registerDefAndUses(const Instruction* Inst, AVR* Avr) {
    registerDef(Inst, Avr);
    registerUses(Inst, Avr);
  }

public:

  IR2AVRVisitor() {}

  AVRValueHIR* getAVR(DDRef* DDR) const {
    assert(DDRef2AVR.count(DDR) && "DDRef belongs to no AVR");
    return DDRef2AVR.find(DDR)->second;
  }

  AVR* getDefAVR(const Value* Val) const {
    assert(Value2DefAVR.count(Val) && "Value defines no AVR");
    return Value2DefAVR.find(Val)->second;
  }

  const AvrSetTy& getUsingAVRs(const Value* Val) const {
    static AvrSetTy EmptyAvrSet;
    const auto& It = Value2UsesAVR.find(Val);
    if (It == Value2UsesAVR.end())
      return EmptyAvrSet;
    return It->second;
  }

  void print(raw_ostream& OS) const;

  /// Visit Functions
  void visit(AVR* ANode) {}
  void postVisit(AVR* ANode) {}
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }
  void visit(AVRValueHIR* AValueHIR);
  void visit(AVRValueIR* AValueIR);
  void visit(AVRPhiIR *APhiIR);
  void visit(AVRCallIR *ACallIR);
  void visit(AVRReturnIR *AReturnIR);
  void visit(AVRSelectIR *ASelectIR);
  void visit(AVRCompareIR *ACompareIR);
  void visit(AVRBranchIR *ABranchIR);
  void visit(AVRIfIR *AIfIR);
};

/// \brief Def-Use/Use-Def information for a given AVR program.
///
/// This class leverages the underlying IR's DU/UD information rather than
/// implement an AVR level DU/UD analysis.
class AvrDefUseBase : public FunctionPass {

public:

  virtual ~AvrDefUseBase() { reset(); }

  /// \brief A set of opaque underlying-IR 'variables', i.e.: {V1, ..., Vn}.
  typedef SmallPtrSet<const void*, 2> VarSetTy;

  /// \brief A mapping from Use AVRs to sets of underlying-IR 'variables'
  /// through which the AVR relates to some Def AVR, i.e.:
  /// AVR --> {V1, ..., Vn}
  typedef DenseMap<AVR*, VarSetTy> AvrUsedVarsMapTy;

  /// \brief A mapping from Def AVRs to their Use AVRs information, i.e.:
  //           { AVR-U1 --> {V11, ..., V1n1}
  // AVR-D --> {        ...
  //           { AVR-Uk --> {Vk1, ..., Vknk}
  typedef DenseMap<AVR*, AvrUsedVarsMapTy> DefUsesTy;

  /// \brief A mapping from opaque underlying-IR 'variables' to the AVR which
  /// are Reaching Defs of some Use AVR, i.e.:
  /// V --> {AVR-D1, ..., AVR-Dn}
  typedef DenseMap<const void*, AvrSetTy> VarReachingAvrsMapTy;

  /// \brief A mapping from Use AVRs to their Reaching Defs information, i.e.:
  ///           { V1 --> {AVR-D11, ..., AVR-D1n1}
  /// AVR-U --> {
  ///           { Vk --> {AVR-Dk1, ..., AVR-Dknk}
  typedef DenseMap<AVR*, VarReachingAvrsMapTy> ReachingDefsTy;

  const AvrUsedVarsMapTy& getUses(AVR* Def) const {
    static AvrUsedVarsMapTy EmptyUsedVarsMap;
    const auto& It = DefUses.find(Def);
    if (It == DefUses.end())
      return EmptyUsedVarsMap;
    return It->second;
  }

  const VarReachingAvrsMapTy& getReachingDefs(AVR* User) const {
    static VarReachingAvrsMapTy EmptyReachingVarsMap;
    const auto& It = ReachingDefs.find(User);
    if (It == ReachingDefs.end())
      return EmptyReachingVarsMap;
    return It->second;
  }

  const AvrSetTy& getReachingDefs(AVR* User, const void* Var) const {
    static AvrSetTy EmptyAvrSet;
    const VarReachingAvrsMapTy& ReachingVars = getReachingDefs(User);
    const auto& It = ReachingVars.find(Var);
    if (It == ReachingVars.end())
      return EmptyAvrSet;
    return It->second;
  }

  void print(raw_ostream &OS, const Module* = nullptr) const override;

protected:

  AvrDefUseBase(char &ID);

  DefUsesTy DefUses;

  ReachingDefsTy ReachingDefs;

  /// \brief Extract the RHS of an ASSIGN which we consider the actual Def of
  /// the 'variable' defined by given LHS value:
  ///
  /// The general structure is:
  ///
  ///     ASSIGN{EXPR{VALUE{%val/Symbase}}} := EXPR(...)
  ///                                            |
  ///                    +-----------Def---------+
  ///                    V
  ///     ..... = EXPR{VALUE{%val/CE(BLOB(Symbase))}
  AVR* getActualDef(AVRValue* LHSValue) const;

  // TODO: Move the class from VPOSIMDLaneEvolution here instead of this method
  // and reuse it there.
  virtual void printVar(const void* Var,
                        formatted_raw_ostream& FOS,
                        unsigned Depth) const = 0;

  /// \brief Reset all internal data structures.
  void reset();
};

/// \brief Function-level AVR Def-Use/Use-Def analysis based on LLVM-IR.
class AvrDefUse : public AvrDefUseBase {

private:

  IR2AVRVisitor IR2AVR;

  AVRGenerate* AV;

  typedef SmallPtrSet<AVRPhiIR*, 5> VisitedPhisTy;

  AvrUsedVarsMapTy& registerDef(AVR* Def);

  void registerUsers(AVR* Def,
                     AvrUsedVarsMapTy& UVs,
                     const Value* Val,
                     VisitedPhisTy& VisitedPhis);

  template<typename AIRT>
  void registerDefAndUses(AIRT* AIR);

  void printVar(const void* Var,
                formatted_raw_ostream& FOS,
                unsigned Depth) const override {
    std::string Indent((Depth * TabLength), ' ');
    const Value* Val = (const Value*)Var;
    FOS << Indent << *Val;
  }

public:

  // Pass Identification
  static char ID;

  /// \brief An AVRVAlueIR is a Def if its designated Instruction is the one
  /// that defines its designated Value. In LLVM IR, this translates to Val
  /// and Instruct pointing to the same object.
  /// \return whether this AVRValueIR is the Def of its underlying Value.
  static bool isDef(AVRValueIR *AValueIR) {
    return AValueIR->getLLVMValue() == AValueIR->getLLVMInstruction();
  }

  /// \brief An AVRValueIR is a Use if its designated Value is not defined by
  /// its designated Instruction. In LLVM IR, this translates to Val and
  /// Instruct not pointing to the same object.
  static bool isUse(AVRValueIR *AValueIR) {
    return !AValueIR->isConstant() &&
      (AValueIR->getLLVMValue() != AValueIR->getLLVMInstruction());
  }

  const IR2AVRVisitor& getIR2AVR() { return IR2AVR; }

  AvrDefUse();

  virtual ~AvrDefUse();

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AVRGenerate>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override;

  /// Visit Functions
  void visit(AVR* ANode) {}
  void postVisit(AVR* ANode) {}
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }
  void visit(AVRValueIR* AValueIR);
  void visit(AVRCallIR *ACallIR);
  void visit(AVRSelectIR *ASelectIR);
  void visit(AVRCompareIR *ACompareIR);
};

/// \brief Function-level AVR Def-Use/Use-Def analysis based on HIR.
class AvrDefUseHIR : public AvrDefUseBase {

private:

  HIRDDAnalysis* DDA;

  IR2AVRVisitor IR2AVR;

  /// \brief During Def-Use relation construction, this is the current top-level
  // loop.
  AVRLoopHIR* TopLevelLoop;

  /// \brief During Def-Use relation construction, this is the DDG of the
  /// current top-level loop.
  DDGraph DDG;

  void printVar(const void* Var,
                 formatted_raw_ostream& FOS,
                unsigned Depth) const override {
    std::string Indent((Depth * TabLength), ' ');
    FOS << Indent;
    const DDRef* DDR = (const DDRef*)Var;
    DDR->print(FOS, true);
  }

public:

  // Pass Identification
  static char ID;

  AvrDefUseHIR();

  virtual ~AvrDefUseHIR();

  static bool isDef(AVRValueHIR *AValueHIR) {
    DDRef * DDF = AValueHIR->getValue();

    // CHECKME: A BlobDDRef will never be a definition
    if (RegDDRef *RDDF = dyn_cast<RegDDRef>(DDF)) {
      return RDDF->isLval() && RDDF->isTerminalRef();
    }
    return false;
  }

  static bool isUse(AVRValueHIR *AValueHIR) {
    return !AValueHIR->isConstant() && !isDef(AValueHIR);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AVRGenerateHIR>();
    AU.addRequired<HIRDDAnalysis>();
    AU.addRequiredTransitive<HIRParser>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override;

  /// Visit Functions
  void visit(AVR* ANode) {}
  void postVisit(AVR* ANode) {}
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }
  void visit(AVRValueHIR* AValueHIR);
  void visit(AVRLoopHIR* ALoopHIR);
  void postVisit(AVRLoopHIR* ALoopHIR);
  void visit(AVRWrn* AWrn);
  void postVisit(AVRWrn* AWrn);
};

} // End VPO Vectorizer Namespace

} // End LLVM Namespace 

#endif  // LLVM_ANALYSIS_VPO_DEF_USE_H

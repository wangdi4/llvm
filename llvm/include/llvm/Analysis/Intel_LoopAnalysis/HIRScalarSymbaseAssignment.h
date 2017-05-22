//===- HIRScalarSymbaseAssignment.h - Assigns symbase to scalars -*- C++ -*===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis is used to assign symbase to scalars.
// It assigns symbases to liveins(present as phi operands) and livout scalars
// and populates the livein/liveout set for regions as well.
//
// Livein scalars are phi operands coming from outside the region (including
// constants) or instructions defined outisde the region and used inside.
// Please note that globals and function parameters are not marked livein.
//
// Liveout scalars are instructions defined inside the region and used outside.
//
// HIRParser uses its interface to assign symbases to non livein/liveout
// scalars. Non-phi livein scalars are also populated by HIRParser because
// some livein scalars can only be discovered during parsing.
//
// Livein/liveout scalars are required by HIRCG to generate the correct code.
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_SCALARSYMBASEASSIGNMENT_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_SCALARSYMBASEASSIGNMENT_H

#include "llvm/Pass.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRRegionIdentification.h"

namespace llvm {

class Value;
class Function;
class Instruction;
class MDString;
class PHINode;
class ScalarEvolution;

namespace loopopt {

/// Invalid symbase.
const unsigned InvalidSymbase = 0;

/// Symbase for constants.
const unsigned ConstantSymbase = 1;

/// Symbase assigned to non-constant rvals which do not create data
/// dependencies.
const unsigned GenericRvalSymbase = 2;

class HIRSCCFormation;
class HIRLoopFormation;

/// \brief This analysis populates livein/liveout values for regions.
///
/// It is also responsible for assigning symbases to temps.
class HIRScalarSymbaseAssignment : public FunctionPass {
private:
  /// Func - The function we are analyzing.
  Function *Func;

  /// Loop info analysis.
  LoopInfo *LI;

  /// SE - Scalar Evolution analysis for the function.
  ScalarEvolution *SE;

  /// RI - Region identification analysis.
  HIRRegionIdentification *RI;

  /// SCCF - SCC formation analysis.
  HIRSCCFormation *SCCF;

  /// LF - Loop formation analysis.
  HIRLoopFormation *LF;

  /// BaseTemps - Temps used to represent a set of scalar values which are
  /// assigned the same symbase.
  SmallVector<const Value *, 32> BaseTemps;

  /// TempSymbaseMap - Maps temps to their symbase.
  SmallDenseMap<const Value *, unsigned, 64> TempSymbaseMap;

  /// StrSymbaseMap - Used to map MDString (attached to an instruction by
  /// SSADeconstruction pass) to symbase.
  StringMap<unsigned> StrSymbaseMap;

private:
  /// Populates region liveout instruction as loop liveout in its parent loops.
  void populateLoopLiveouts(const Instruction *Inst, unsigned Symbase) const;

  /// \brief Populates liveout Values for the region pointed to by RegIt.
  void populateRegionLiveouts(HIRRegionIdentification::iterator RegIt);

  /// \brief Processes operands of Phi to determine if they are region liveout.
  bool processRegionPhiLivein(HIRRegionIdentification::iterator RegIt,
                              const PHINode *Phi, unsigned Symbase);

  /// Populates loop liveouts based on SCC phi instructions.
  void populateLoopSCCPhiLiveouts(const Instruction *SCCInst, unsigned Symbase);

  /// \brief Populates livein Values from the phi nodes present in the region.
  void populateRegionPhiLiveins(HIRRegionIdentification::iterator RegIt);

  /// \brief Returns index of Symbase in BaseTemps.
  unsigned getIndex(unsigned Symbase) const {
    return Symbase - GenericRvalSymbase - 1;
  }

  /// \brief Inserts Temp into set of base temps and returns its non-zero
  /// symbase.
  unsigned insertBaseTemp(const Value *Temp);

  /// \brief Updates the base temp representing Symbase to the passed in Temp,
  /// if applicable.
  void updateBaseTemp(unsigned Symbase, const Value *Temp,
                      const Value **OldTemp);

  /// \brief Inserts temp-symbase pair into the map. Symbase cannot be
  /// InvalidSymbase or ConstantSymbase.
  void insertTempSymbase(const Value *Temp, unsigned Symbase);

  /// \brief Returns the MDString node attached to Inst, if any, else returns
  /// null.
  MDString *getInstMDString(const Instruction *Inst) const;

  /// \brief Internal to getOrAssignScalarSymbase(). Returns Temp's symbase if
  /// it exists, else assigns a new symbase.
  unsigned getOrAssignTempSymbase(const Value *Temp);

  /// \brief Returns Temp's symbase if it exists, else returns InvalidSymbase.
  unsigned getTempSymbase(const Value *Temp) const;

  /// \brief Assigns a symbase to Temp and returns it.
  unsigned assignTempSymbase(const Value *Temp);

  /// \brief Traces back single operand phis until something else is encountered
  /// (or we leave the current region) and returns that.
  const Value *traceSingleOperandPhis(const Value *Scalar,
                                      const IRRegion &IRReg) const;

  /// \brief Implements getOrAssignScalarSymbase() functionality.
  unsigned getOrAssignScalarSymbaseImpl(const Value *Scalar,
                                        const IRRegion &IRReg, bool Assign,
                                        const Value **OldBaseScalar);

public:
  static char ID; // Pass identification
  HIRScalarSymbaseAssignment();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;

  /// \brief Returns the scalar associated with symbase.
  const Value *getBaseScalar(unsigned Symbase) const;

  /// \brief Returns the max symbase assigned to any scalar.
  unsigned getMaxScalarSymbase() const {
    return BaseTemps.size() + GenericRvalSymbase;
  }

  /// \brief Returns true if this scalar is a constant.
  static bool isConstant(const Value *Scalar);

  /// \brief Returns scalar's symbase if it exists, else assigns a new symbase.
  /// If this scalar has replaced an existing base scalar, the existing scalar
  /// is returned via OldBaseScalar.
  unsigned getOrAssignScalarSymbase(const Value *Scalar, const IRRegion &IRReg,
                                    const Value **OldBaseScalar = nullptr);

  /// \brief Returns scalar's symbase if it exists, else returns 0.
  unsigned getScalarSymbase(const Value *Scalar, const IRRegion &IRReg);
};

} // End namespace loopopt

} // End namespace llvm

#endif

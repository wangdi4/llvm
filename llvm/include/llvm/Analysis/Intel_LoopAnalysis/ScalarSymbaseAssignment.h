//===--- ScalarSymbaseAssignment.h - Assigns symbase to scalars -*- C++ -*-===//
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

#include "llvm/Analysis/Intel_LoopAnalysis/RegionIdentification.h"

namespace llvm {

class Value;
class Function;
class Instruction;
class MDString;
class PHINode;

namespace loopopt {

/// Invalid symbase.
const unsigned INVALID_SYMBASE = 0;

/// Symbase for constants.
const unsigned CONSTANT_SYMBASE = 1;

class SCCFormation;

/// \brief This analysis populates livein/liveout values for regions.
///
/// It is also responsible for assigning symbases to temps.
class ScalarSymbaseAssignment : public FunctionPass {
private:
  /// Func - The function we are analyzing.
  Function *Func;

  /// RI - Region Identification analysis.
  RegionIdentification *RI;

  /// SCCF - SCCFormation analysis.
  SCCFormation *SCCF;

  /// BaseTemps - Temps used to represent a set of scalar values which are
  /// assigned the same symbase.
  SmallVector<const Value *, 32> BaseTemps;

  /// TempSymbaseMap - Maps temps to their symbase.
  SmallDenseMap<const Value *, unsigned, 64> TempSymbaseMap;

  /// StrSymbaseMap - Used to map MDString (attached to an instruction by
  /// SSADeconstruction pass) to symbase.
  StringMap<unsigned> StrSymbaseMap;

  /// ScalarLvalSymbases - Maps symbases to scalar lvals. This is only used for
  /// printing lval DDRefs. To dump HIR correctly it needs to be updated for new
  /// values created by HIR transformations as well.
  SmallDenseMap<unsigned, const Value *, 64> ScalarLvalSymbases;

  /// \brief Populates liveout Values for the region pointed to by RegIt.
  void populateRegionLiveouts(RegionIdentification::iterator RegIt);

  /// \brief Processes operands of Phi to determine if they are region liveout.
  bool processRegionPhiLivein(RegionIdentification::iterator RegIt,
                              const PHINode *Phi, unsigned Symbase);

  /// \brief Populates livein Values from the phi nodes present in the region.
  void populateRegionPhiLiveins(RegionIdentification::iterator RegIt);

  /// \brief Inserts Temp into set of base temps and returns its non-zero
  /// symbase.
  unsigned insertBaseTemp(const Value *Temp);

  /// \brief Inserts temp-symbase pair into the map. Symbase cannot be
  /// INVALID_SYMBASE or CONSTANT_SYMBASE.
  void insertTempSymbase(const Value *Temp, unsigned Symbase);

  /// \brief Returns the MDString node attached to Inst, if any, else returns
  /// null.
  MDString *getInstMDString(const Instruction *Inst) const;

  /// \brief Internal to getOrAssignScalarSymbase(). Returns Temp's symbase if
  /// it exists, else assigns a new symbase.
  unsigned getOrAssignTempSymbase(const Value *Temp);

  /// \brief Returns Temp's symbase if it exists, else returns INVALID_SYMBASE.
  unsigned getTempSymbase(const Value *Temp) const;

  /// \brief Implements getOrAssignScalarSymbase() functionality.
  unsigned getOrAssignScalarSymbaseImpl(const Value *Scalar, bool Assign);

  /// \brief Sets current Function as a generic value to represent loop uppers.
  /// This is a hack to set a generic loop upper symbase which does not
  /// conflict with anything in the region as the loop upper is parsed from
  /// loop's BackEdgeTakenCount which may not have any Value associated with it.
  void setGenericLoopUpperSymbase();

public:
  static char ID; // Pass identification
  ScalarSymbaseAssignment();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;

  /// \brief Registers new lval/symbase pairs created by HIR transformations.
  /// Only used for printing.
  void insertHIRLval(const Value *Lval, unsigned Symbase);

  /// \brief Traces back single operand phis until something else is encountered
  /// and returns that.
  const Value *traceSingleOperandPhis(const Value *Scalar) const;

  /// \brief Returns the scalar associated with symbase.
  const Value *getBaseScalar(unsigned Symbase) const;

  /// \brief Returns the base scalar associated with Scalar, if any, else
  /// returns the same scalar. It is only used for printing.
  const Value *getBaseScalar(const Value *Scalar) const;

  /// \brief Returns the max symbase assigned to any scalar.
  unsigned getMaxScalarSymbase() const;

  /// \brief Returns a generic Val to represent loop uppers.
  const Value *getGenericLoopUpperVal() const;

  /// \brief Returns true if this scalar is a constant.
  bool isConstant(const Value *Scalar) const;

  /// \brief Returns scalar's symbase if it exists, else assigns a new symbase.
  unsigned getOrAssignScalarSymbase(const Value *Scalar);

  /// \brief Returns scalar's symbase if it exists, else returns 0.
  unsigned getScalarSymbase(const Value *Scalar);
};

} // End namespace loopopt

} // End namespace llvm

#endif

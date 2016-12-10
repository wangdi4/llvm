//===- CSAMachineFuctionInfo.h - CSA machine function info -------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares CSA-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_CSA_CSAMACHINEFUNCTIONINFO_H

#include "CSARegisterInfo.h"
#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

/// CSAMachineFunctionInfo - This class is derived from MachineFunction and
/// contains private CSA target-specific information for each MachineFunction.
class CSAMachineFunctionInfo : public MachineFunctionInfo {

  struct Info;
  Info* info;

  virtual void anchor();

  /// Holds for each function where on the stack the Frame Pointer must be
  /// saved. This is used on Prologue and Epilogue to emit FP save/restore
  int FPFrameIndex;

  /// Holds for each function where on the stack the Return Address must be
  /// saved. This is used on Prologue and Epilogue to emit RA save/restore
  int RAFrameIndex;

  /// VarArgsFrameIndex - FrameIndex for start of varargs area.
  int VarArgsFrameIndex;

public:
  explicit CSAMachineFunctionInfo(MachineFunction &MF);
  ~CSAMachineFunctionInfo();
 
  int getFPFrameIndex() const { return FPFrameIndex; }
  void setFPFrameIndex(int Index) { FPFrameIndex = Index; }

  int getRAFrameIndex() const { return RAFrameIndex; }
  void setRAFrameIndex(int Index) { RAFrameIndex = Index; }

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

  // This is a potentially temporary approach for handling LIC allocation.
  // LICs are not considered normally allocatable entities for register
  // allocation - all are reserved.  Instead, each case where a LIC is used
  // is explicitly allocated with allocateLIC.  In assembly generation, LIC
  // declarations are emitted for the current routine for each allocated LIC.

  const TargetRegisterClass* licRCFromGenRC(const TargetRegisterClass* RC);
  const TargetRegisterClass* licFromType(MVT vt);

  // Return an available "physical" LIC matching the LIC "register" class.
  // (e.g. one of CI0, CI1, CI8, CI16, CI32 or CI64)
  // This represents a unique LIC statically allocated for the current routine.
  // Note that a different routine may have a use of the "same" LIC, but
  // it represents a different static physical instance.  (e.g. 2 routines
  // may have CI64_3, but they are distinct entities.)
  unsigned allocateLIC(const TargetRegisterClass* RegClass);
  // True if the lic (register) is allocated
  bool isAllocated(unsigned lic) const;

  // Set the depth for a particular LIC explicitly, rather than the default.
  void setLICDepth(unsigned lic, int amount);

  // Return the depth of the specified LIC.
  // A depth of -1 means the LIC is not allocated
  // A depth of 0 means to use the default (currently, and expected to be, 2)
  // Non-zero means an explicit (normally larger) value.
  int getLICDepth(unsigned lic);
};

} // End llvm namespace

#endif

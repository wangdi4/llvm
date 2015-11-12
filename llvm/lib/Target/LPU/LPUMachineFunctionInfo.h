//===- LPUMachineFuctionInfo.h - LPU machine function info -------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares LPU-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LPU_LPUMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_LPU_LPUMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

/// LPUMachineFunctionInfo - This class is derived from MachineFunction and
/// contains private LPU target-specific information for each MachineFunction.
class LPUMachineFunctionInfo : public MachineFunctionInfo {

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
  explicit LPUMachineFunctionInfo(MachineFunction &MF)
    : FPFrameIndex(-1), RAFrameIndex(-1), VarArgsFrameIndex(-1) {}
 
  int getFPFrameIndex() const { return FPFrameIndex; }
  void setFPFrameIndex(int Index) { FPFrameIndex = Index; }

  int getRAFrameIndex() const { return RAFrameIndex; }
  void setRAFrameIndex(int Index) { RAFrameIndex = Index; }

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

};

} // End llvm namespace

#endif

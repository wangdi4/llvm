//===--- Nios2CallLowering.h - Call lowering ------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file describes how to lower LLVM calls to machine code calls.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NIOS2_NIOS2CALLLOWERING_H
#define LLVM_LIB_TARGET_NIOS2_NIOS2CALLLOWERING_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/CodeGen/GlobalISel/CallLowering.h"
#include <cstdint>
#include <functional>

namespace llvm {

class Nios2TargetLowering;

class Nios2CallLowering: public CallLowering {
public:
  Nios2CallLowering(const Nios2TargetLowering &TLI);

  bool lowerReturn(MachineIRBuilder &MIRBuiler, const Value *Val,
                   unsigned VReg) const override;

  bool lowerFormalArguments(MachineIRBuilder &MIRBuilder, const Function &F,
                            ArrayRef<unsigned> VRegs) const override;

  bool lowerCall(MachineIRBuilder &MIRBuilder, CallingConv::ID CallConv,
                 const MachineOperand &Callee, const ArgInfo &OrigRet,
                 ArrayRef<ArgInfo> OrigArgs) const override;

private:
  typedef std::function<void(MachineIRBuilder &, Type *, unsigned,
                             CCValAssign &)>
      RegHandler;

  typedef std::function<void(MachineIRBuilder &, int, CCValAssign &)>
      MemHandler;

  typedef std::function<void(unsigned, uint64_t)> SplitArgTy;

  void splitToValueTypes(const ArgInfo &OrigArgInfo,
                         SmallVectorImpl<ArgInfo> &SplitArgs,
                         const DataLayout &DL, MachineRegisterInfo &MRI,
                         const SplitArgTy &SplitArg) const;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_NIOS2_NIOS2CALLLOWERING_H

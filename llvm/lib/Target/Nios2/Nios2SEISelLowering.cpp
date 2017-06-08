//===-- Nios2SEISelLowering.cpp - Nios2SE DAG Lowering Interface --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Subclass of Nios2TargetLowering specialized for nios232.
//
//===----------------------------------------------------------------------===//
#include "Nios2MachineFunction.h"
#include "Nios2SEISelLowering.h"

#include "Nios2RegisterInfo.h"
#include "Nios2TargetMachine.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"

using namespace llvm;

#define DEBUG_TYPE "nios2-isel"

static cl::opt<bool>
EnableNios2TailCalls("enable-nios2-tail-calls", cl::Hidden,
                    cl::desc("NIOS2: Enable tail calls."), cl::init(false));

//@Nios2SETargetLowering {
Nios2SETargetLowering::Nios2SETargetLowering(const Nios2TargetMachine &TM,
                                           const Nios2Subtarget &STI)
    : Nios2TargetLowering(TM, STI) {
//@Nios2SETargetLowering body {
  // Set up the register classes
  addRegisterClass(MVT::i32, &Nios2::CPURegsRegClass);

  setOperationAction(ISD::ATOMIC_FENCE,       MVT::Other, Custom);

// must, computeRegisterProperties - Once all of the register classes are 
//  added, this allows us to compute derived properties we expose.
  computeRegisterProperties(Subtarget.getRegisterInfo());
}

SDValue Nios2SETargetLowering::LowerOperation(SDValue Op,
                                             SelectionDAG &DAG) const {

  return Nios2TargetLowering::LowerOperation(Op, DAG);
}

const Nios2TargetLowering *
llvm::createNios2SETargetLowering(const Nios2TargetMachine &TM,
                                 const Nios2Subtarget &STI) {
  return new Nios2SETargetLowering(TM, STI);
}

bool Nios2SETargetLowering::
isEligibleForTailCallOptimization(const Nios2CC &Nios2CCInfo,
                                  unsigned NextStackOffset,
                                  const Nios2FunctionInfo& FI) const {
  if (!EnableNios2TailCalls)
    return false;

  // Return false if either the callee or caller has a byval argument.
  if (Nios2CCInfo.hasByValArg() || FI.hasByvalArg())
    return false;

  // Return true if the callee's argument area is no larger than the
  // caller's.
  return NextStackOffset <= FI.getIncomingArgSize();
}

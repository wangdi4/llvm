//===-- Nios2MachineFunctionInfo.cpp - Private data used for Nios2 ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Nios2MachineFunction.h"
#include "MCTargetDesc/Nios2BaseInfo.h"
#include "Nios2InstrInfo.h"
#include "Nios2Subtarget.h"
#include "llvm/IR/Function.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

bool FixGlobalBaseReg;

Nios2FunctionInfo::~Nios2FunctionInfo() {}

bool Nios2FunctionInfo::globalBaseRegFixed() const {
  return FixGlobalBaseReg;
}

bool Nios2FunctionInfo::globalBaseRegSet() const {
  return GlobalBaseReg;
}

unsigned Nios2FunctionInfo::getGlobalBaseReg() {
  return GlobalBaseReg = Nios2::GP;
}

void Nios2FunctionInfo::createEhDataRegsFI() {
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  const TargetRegisterClass &RC = Nios2::CPURegsRegClass;
  for (int I = 0; I < 2; ++I) {

    EhDataRegFI[I] = MF.getFrameInfo().CreateStackObject(TRI.getSpillSize(RC),
        TRI.getSpillAlignment(RC), false);
  }
}

MachinePointerInfo Nios2FunctionInfo::callPtrInfo(const char *ES) {
  return MachinePointerInfo(MF.getPSVManager().getExternalSymbolCallEntry(ES));
}

MachinePointerInfo Nios2FunctionInfo::callPtrInfo(const GlobalValue *GV) {
  return MachinePointerInfo(MF.getPSVManager().getGlobalValueCallEntry(GV));
}

void Nios2FunctionInfo::anchor() { }

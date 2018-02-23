//===-- CSAMachineFuctionInfo.cpp - CSA machine function info -------------===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "CSAMachineFunctionInfo.h"
#include "CSAInstrInfo.h"
#include "CSARegisterInfo.h"
#include "CSASubtarget.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

void CSAMachineFunctionInfo::anchor() {}

CSAMachineFunctionInfo::CSAMachineFunctionInfo(MachineFunction &MF)
    : MRI(MF.getRegInfo()), TII(MF.getSubtarget<CSASubtarget>().getInstrInfo()),
      FPFrameIndex(-1), RAFrameIndex(-1), VarArgsFrameIndex(-1) {}

CSAMachineFunctionInfo::~CSAMachineFunctionInfo() {}

const TargetRegisterClass *
CSAMachineFunctionInfo::licRCFromGenRC(const TargetRegisterClass *RC) {
  if (RC == &CSA::I0RegClass)
    return &CSA::CI0RegClass;
  else if (RC == &CSA::I1RegClass)
    return &CSA::CI1RegClass;
  else if (RC == &CSA::I8RegClass)
    return &CSA::CI8RegClass;
  else if (RC == &CSA::I16RegClass)
    return &CSA::CI16RegClass;
  else if (RC == &CSA::I32RegClass)
    return &CSA::CI32RegClass;
  else if (RC == &CSA::I64RegClass)
    return &CSA::CI64RegClass;
  else if (RC == &CSA::RI0RegClass)
    return &CSA::CI0RegClass;
  else if (RC == &CSA::RI1RegClass)
    return &CSA::CI1RegClass;
  else if (RC == &CSA::RI8RegClass)
    return &CSA::CI8RegClass;
  else if (RC == &CSA::RI16RegClass)
    return &CSA::CI16RegClass;
  else if (RC == &CSA::RI32RegClass)
    return &CSA::CI32RegClass;
  else if (RC == &CSA::RI64RegClass)
    return &CSA::CI64RegClass;
  return NULL;
}

unsigned CSAMachineFunctionInfo::allocateLIC(const TargetRegisterClass *RC,
                                             const Twine &name) {
  unsigned regno = MRI.createVirtualRegister(RC);
  noteNewLIC(regno, TII->getSizeOfRegisterClass(RC), name);
  return regno;
}

void CSAMachineFunctionInfo::noteNewLIC(unsigned vreg, unsigned size,
                                        const Twine &name) {
  if (name.isTriviallyEmpty()) {
    // Don't set empty names for now.
    // unsigned index = licInfo.size();
    // setLICName(vreg, (Twine("cv") + Twine(size) + "_" + Twine(index)));
  } else {
    setLICName(vreg, name);
  }
}

void CSAMachineFunctionInfo::setLICName(unsigned vreg,
                                        const Twine &name) const {
  // TODO: guarantee uniqueness of names.
  getLICInfo(vreg).name = name.str();
}

CSAMachineFunctionInfo::LICInfo &
CSAMachineFunctionInfo::getLICInfo(unsigned regno) {
  assert(TargetRegisterInfo::isVirtualRegister(regno) &&
         "LICs should be virtual registers");
  auto index = TargetRegisterInfo::virtReg2Index(regno);
  return licInfo[index];
}

int CSAMachineFunctionInfo::getLICSize(unsigned regno) const {
  const TargetRegisterClass *RC = TII->getRegisterClass(regno, MRI);
  return TII->getSizeOfRegisterClass(RC);
}

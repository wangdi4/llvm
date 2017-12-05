//===-- CSAMachineFuctionInfo.cpp - CSA machine function info -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "CSAMachineFunctionInfo.h"
#include "CSAInstrInfo.h"
#include "CSARegisterInfo.h"
#include "CSASubtarget.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

struct CSAMachineFunctionInfo::Info {
  // This is the index into the list of "registers" for the specific class
  // of the next register to be allocated for the class
  std::vector<short> nextRegIndexInClass;
};

void CSAMachineFunctionInfo::anchor() { }

CSAMachineFunctionInfo::CSAMachineFunctionInfo(MachineFunction &MF)
  : MRI(MF.getRegInfo()),
    TII(MF.getSubtarget<CSASubtarget>().getInstrInfo()),
    FPFrameIndex(-1), RAFrameIndex(-1), VarArgsFrameIndex(-1) {
  info = new Info;
  info->nextRegIndexInClass.resize(32 /* register class count*/, 0);
}

CSAMachineFunctionInfo::~CSAMachineFunctionInfo() {
  delete info;
  info = NULL;
}

const TargetRegisterClass* CSAMachineFunctionInfo::licRCFromGenRC(const TargetRegisterClass* RC) {
  if      (RC == &CSA::I0RegClass)  return &CSA::CI0RegClass;
  else if (RC == &CSA::I1RegClass)  return &CSA::CI1RegClass;
  else if (RC == &CSA::I8RegClass)  return &CSA::CI8RegClass;
  else if (RC == &CSA::I16RegClass) return &CSA::CI16RegClass;
  else if (RC == &CSA::I32RegClass) return &CSA::CI32RegClass;
  else if (RC == &CSA::I64RegClass) return &CSA::CI64RegClass;
  else if (RC == &CSA::RI0RegClass)  return &CSA::CI0RegClass;
  else if (RC == &CSA::RI1RegClass)  return &CSA::CI1RegClass;
  else if (RC == &CSA::RI8RegClass)  return &CSA::CI8RegClass;
  else if (RC == &CSA::RI16RegClass) return &CSA::CI16RegClass;
  else if (RC == &CSA::RI32RegClass) return &CSA::CI32RegClass;
  else if (RC == &CSA::RI64RegClass) return &CSA::CI64RegClass;
  return NULL;
}

unsigned CSAMachineFunctionInfo::allocateLIC(const TargetRegisterClass* RC,
    const Twine &name) {
  // In the future, we want to allocate LICs as virtual registers. For now,
  // allocate as a physical register.
#if 0
  unsigned regno = MRI.createVirtualRegister(RC);
#else
  unsigned regindex = info->nextRegIndexInClass[RC->getID()]++;
  unsigned regno = RC->getRegister(regindex);
#endif
  noteNewLIC(regno, TII->getSizeOfRegisterClass(RC), name);
  return regno;
}

void CSAMachineFunctionInfo::noteNewLIC(unsigned vreg, unsigned size,
    const Twine &name) {
  auto &licData = getLICInfo(vreg);
  unsigned index = licInfo.size() + physicalLicInfo.size();
  if (name.isTriviallyEmpty()) {
    licData.name = (Twine("ci") + Twine(size) + "_" + Twine(index)).str();
  } else {
    licData.name = name.str();
  }
}

CSAMachineFunctionInfo::LICInfo &
CSAMachineFunctionInfo::getLICInfo(unsigned regno) {
  bool isVirtual = TargetRegisterInfo::isVirtualRegister(regno);
  auto &licMap = isVirtual ? licInfo : physicalLicInfo;
  auto index = isVirtual ? TargetRegisterInfo::virtReg2Index(regno) : regno;
  return licMap[index];
}

int CSAMachineFunctionInfo::getLICSize(unsigned regno) const {
  const TargetRegisterClass *RC;
  if (TargetRegisterInfo::isVirtualRegister(regno)) {
    RC = MRI.getRegClass(regno);
  } else {
    RC = TII->lookupLICRegClass(regno);
  }
  return TII->getSizeOfRegisterClass(RC);
}

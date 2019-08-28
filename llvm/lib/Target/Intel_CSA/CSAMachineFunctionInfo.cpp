//===-- CSAMachineFuctionInfo.cpp - CSA machine function info -------------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
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
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "CSAUtils.h"

using namespace llvm;

void CSAMachineFunctionInfo::anchor() {}

CSAMachineFunctionInfo::CSAMachineFunctionInfo(MachineFunction &MF)
    : MF(MF), MRI(MF.getRegInfo()), TII(MF.getSubtarget<CSASubtarget>().getInstrInfo()),
      nameCounter(0),
      FPFrameIndex(-1), RAFrameIndex(-1), VarArgsFrameIndex(-1) {
  InMemoryLicSXU = allocateLIC(&CSA::CI0RegClass, "in_ctl");
  DoNotEmitAsm = true;
}

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


bool CSAMachineFunctionInfo::canDeleteLICReg(unsigned reg) const {
  if (getIsGloballyVisible(reg)) return false;
  for (MIOperands MO(*entryMI); MO.isValid(); ++MO) {
    if (!MO->isReg() || !Register::isVirtualRegister(MO->getReg()))
      continue;
    unsigned MOReg = MO->getReg();
    if (MOReg == reg) return false;
  }
  for (MIOperands MO(*returnMI); MO.isValid(); ++MO) {
    if (!MO->isReg() || !Register::isVirtualRegister(MO->getReg()))
      continue;
    unsigned MOReg = MO->getReg();
    if (MOReg == reg) return false;
  }
  return true;
}

unsigned CSAMachineFunctionInfo::allocateLIC(const TargetRegisterClass *RC,
                                             const Twine &name,
                                             const Twine &fname,
                                             bool isDeclared,
                                             bool isGloballyVisible) {
  unsigned regno = MRI.createVirtualRegister(RC);
  noteNewLIC(regno, TII->getSizeOfRegisterClass(RC), name, fname);
  getLICInfo(regno).isDeclared = isDeclared;
  getLICInfo(regno).isGloballyVisible = isGloballyVisible;
  return regno;
}

void CSAMachineFunctionInfo::noteNewLIC(unsigned vreg, unsigned size,
                                        const Twine &name,
                                        const Twine &fname) {
  if (name.isTriviallyEmpty()) {
    // Don't set empty names for now.
    // unsigned index = licInfo.size();
    // setLICName(vreg, (Twine("cv") + Twine(size) + "_" + Twine(index)));
  } else {
    setLICName(vreg, name, fname);
  }
}

// Appending function name as is to the variable name made it too long
// For readability, append _func_<func_index> instead
std::string replaceFuncNameWithNewUniqueName(std::string fname_str, MachineFunction &thisMF) {
  MachineModuleInfo &MMI = thisMF.getMMI();
  const Module *M = MMI.getModule();
  std::string new_fname_str("_func_");
  int index = 0;
  for (const Function &F : *M) {
    if (F.getName() == fname_str)
      break;
    ++index;
  }
  new_fname_str = new_fname_str + std::to_string(index);
  return new_fname_str;
}

void CSAMachineFunctionInfo::setLICName(unsigned vreg,
                                        const Twine &name,
                                        const Twine &fname) const {
  if (Register::isPhysicalRegister(vreg))
    return;
  std::string composed;
  if (!name.isTriviallyEmpty()) {
    composed = name.str();
    for (auto &ch : composed) {
      if ('a' <= ch && ch <= 'z') continue;
      if ('A' <= ch && ch <= 'Z') continue;
      if ('0' <= ch && ch <= '9') continue;
      if (ch == '.' || ch == '_') continue;
      if (ch == '$' || ch == '%') continue;
      ch = '_';
    }
    std::string fname_str;
    if (fname.isTriviallyEmpty())
      fname_str = MF.getFunction().getName();
    else
      fname_str = fname.str();
    fname_str = replaceFuncNameWithNewUniqueName(fname_str,MF);
    if (csa_utils::isAlwaysDataFlowLinkageSet() && (composed.find(fname_str) != 0)) {
      composed = fname_str + "_" + composed;
    }
    auto baseIndex = composed.size();
    while (!namedLICs.insert(composed).second) {
      composed.resize(baseIndex);
      composed += std::to_string(++nameCounter);
    }
    getLICInfo(vreg).name = composed;
  } else {
    getLICInfo(vreg).name.clear();
  }
}

CSAMachineFunctionInfo::LICInfo &
CSAMachineFunctionInfo::getLICInfo(unsigned regno) {
  assert(Register::isVirtualRegister(regno) &&
         "LICs should be virtual registers");
  auto index = Register::virtReg2Index(regno);
  return licInfo[index];
}

unsigned CSAMachineFunctionInfo::getLICSize(unsigned regno) const {
  const TargetRegisterClass *RC = TII->getRegisterClass(regno, MRI);
  return TII->getSizeOfRegisterClass(RC);
}

void CSAMachineFunctionInfo::addLICAttribute(unsigned regno, const StringRef key, const StringRef value) const {
  getLICInfo(regno).attribs[key] = value;
}

StringRef CSAMachineFunctionInfo::getLICAttribute(unsigned reg, StringRef key) const {
  if (getLICInfo(reg).attribs.count(key))
    return getLICInfo(reg).attribs[key];

  return "";
}

unsigned CSAMachineFunctionInfo::getOutMemoryLic() const {
  const MachineInstr *const Return = getReturnMI();
  assert(Return && "Cannot find out ordering lic - no return instruction set");
  const MachineOperand &OrdOp = Return->getOperand(0);
  assert(OrdOp.isUse() && "Output ordering edge isn't a register?");
  return OrdOp.getReg();
}

unsigned CSAMachineFunctionInfo::getInMemoryLic() const {
  if (!MF.getSubtarget<CSASubtarget>().isSequential() &&
      csa_utils::isAlwaysDataFlowLinkageSet()) {
    const MachineInstr *const Entry = getEntryMI();
    assert(Entry && "Cannot find in ordering lic - no entry instruction set");
    const MachineOperand &OrdOp = Entry->getOperand(0);
    assert(OrdOp.isDef() && "Input ordering edge isn't a register?");
    return OrdOp.getReg();
  } else
    return InMemoryLicSXU;
}

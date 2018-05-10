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
#include "CSAUtils.h"

using namespace llvm;

void CSAMachineFunctionInfo::anchor() {}

CSAMachineFunctionInfo::CSAMachineFunctionInfo(MachineFunction &MF)
    : MF(MF), MRI(MF.getRegInfo()), TII(MF.getSubtarget<CSASubtarget>().getInstrInfo()),
      nameCounter(0),
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


bool CSAMachineFunctionInfo::canDeleteLICReg(unsigned reg) const {
  if (getIsGloballyVisible(reg)) return false;
  for (MIOperands MO(*entryMI); MO.isValid(); ++MO) {
    if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg()))
      continue;
    unsigned MOReg = MO->getReg();
    if (MOReg == reg) return false;
  }
  for (MIOperands MO(*returnMI); MO.isValid(); ++MO) {
    if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg()))
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

void CSAMachineFunctionInfo::setLICName(unsigned vreg,
                                        const Twine &name,
                                        const Twine &fname) const {
  if (TargetRegisterInfo::isPhysicalRegister(vreg))
    return;
  std::string composed;  
  Twine new_name;
  if (!name.isTriviallyEmpty()) {
    composed = name.str();
    //std::string composed = new_name.str();
    for (auto &ch : composed) {
      if ('a' <= ch && ch <= 'z') continue;
      if ('A' <= ch && ch <= 'Z') continue;
      if ('0' <= ch && ch <= '9') continue;
      if (ch == '.' || ch == '_') continue;
      if (ch == '$' || ch == '%') continue;
      ch = '_';
    }
    auto baseIndex = composed.size();
    std::string fname_str;
    if (fname.isTriviallyEmpty()) 
      fname_str = MF.getFunction().getName();
    else
      fname_str = fname.str();
    if (csa_utils::isAlwaysDataFlowLinkageSet() && (composed.find(fname_str) != 0)) {
      Twine new_name(Twine(fname_str) + Twine("_") + Twine(composed));
      composed = new_name.str();
    }   
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
  assert(TargetRegisterInfo::isVirtualRegister(regno) &&
         "LICs should be virtual registers");
  auto index = TargetRegisterInfo::virtReg2Index(regno);
  return licInfo[index];
}

int CSAMachineFunctionInfo::getLICSize(unsigned regno) const {
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

//===-- LPUMachineFuctionInfo.cpp - LPU machine function info -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "LPUMachineFunctionInfo.h"
#include "LPURegisterInfo.h"

using namespace llvm;

struct LPUMachineFunctionInfo::Info {
  // Contains the LIC depth, indexed by target physical register number
  // -1 means the LIC is not allocated
  // 0 means default
  // >0 means a specific depth
  std::vector<short> licDepth;
  // This is the index into the list of "registers" for the specific class
  // of the next register to be allocated for the class
  std::vector<short> nextRegIndexInClass;
};

void LPUMachineFunctionInfo::anchor() { }

LPUMachineFunctionInfo::LPUMachineFunctionInfo(MachineFunction &MF)
    : FPFrameIndex(-1), RAFrameIndex(-1), VarArgsFrameIndex(-1) {
  info = new Info;
  info->licDepth.resize(8192 /*LPU::NUM_TARGET_REGS*/, -1);
  info->nextRegIndexInClass.resize(32 /* register class count*/, 0);
}

LPUMachineFunctionInfo::~LPUMachineFunctionInfo() {
  delete info;
  info = NULL;
}

const TargetRegisterClass* LPUMachineFunctionInfo::licRCFromGenRC(const TargetRegisterClass* RC) {
  if      (RC == &LPU::I0RegClass)  return &LPU::CI0RegClass;
  else if (RC == &LPU::I1RegClass)  return &LPU::CI1RegClass;
  else if (RC == &LPU::I8RegClass)  return &LPU::CI8RegClass;
  else if (RC == &LPU::I16RegClass) return &LPU::CI16RegClass;
  else if (RC == &LPU::I32RegClass) return &LPU::CI32RegClass;
  else if (RC == &LPU::I64RegClass) return &LPU::CI64RegClass;
  else if (RC == &LPU::RI0RegClass)  return &LPU::CI0RegClass;
  else if (RC == &LPU::RI1RegClass)  return &LPU::CI1RegClass;
  else if (RC == &LPU::RI8RegClass)  return &LPU::CI8RegClass;
  else if (RC == &LPU::RI16RegClass) return &LPU::CI16RegClass;
  else if (RC == &LPU::RI32RegClass) return &LPU::CI32RegClass;
  else if (RC == &LPU::RI64RegClass) return &LPU::CI64RegClass;
  return NULL;
}

const TargetRegisterClass* LPUMachineFunctionInfo::licFromType(MVT vt) {
  if      (vt==MVT::i1)  return &LPU::CI1RegClass;
  else if (vt==MVT::i8)  return &LPU::CI8RegClass;
  else if (vt==MVT::i16) return &LPU::CI16RegClass;
  else if (vt==MVT::i32) return &LPU::CI32RegClass;
  else if (vt==MVT::i64) return &LPU::CI64RegClass;
  else return NULL;
}

unsigned LPUMachineFunctionInfo::allocateLIC(const TargetRegisterClass* RC) {
  // The register class must be a LIC class!
  // assert(
  //  RC==CI0RegClass || RC==CI1RegClass || RC==CI8RegClass ||
  //    RC==CI16RegClass || RC==CI32RegClass || RC==CI64RegClass);
  // get the index of the next lic in the class
  unsigned index = info->nextRegIndexInClass[RC->getID()]++;

  unsigned lic = RC->getRegister(index);

  info->licDepth[lic] = 0;
  return lic;
}

bool LPUMachineFunctionInfo::isAllocated(unsigned lic) const {
  return info->licDepth[lic] >= 0;
}

// Set the depth for a particular LIC explicitly, rather than the default.
void LPUMachineFunctionInfo::setLICDepth(unsigned lic, int amount) {
  info->licDepth[lic] = amount;
}

// Return the depth of the specified LIC.
int LPUMachineFunctionInfo::getLICDepth(unsigned lic) {
  return info->licDepth[lic];
}

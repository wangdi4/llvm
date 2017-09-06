//===-- CSAMachineFuctionInfo.cpp - CSA machine function info -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "CSAMachineFunctionInfo.h"
#include "CSARegisterInfo.h"

#define GET_REGINFO_ENUM
#include "CSAGenRegisterInfo.inc"

using namespace llvm;

struct CSAMachineFunctionInfo::Info {
  // Contains the LIC depth, indexed by target physical register number
  // -1 means the LIC is not allocated
  // 0 means default
  // >0 means a specific depth
  std::vector<short> licDepth;
  // This is the index into the list of "registers" for the specific class
  // of the next register to be allocated for the class
  std::vector<short> nextRegIndexInClass;
};

void CSAMachineFunctionInfo::anchor() { }

CSAMachineFunctionInfo::CSAMachineFunctionInfo(MachineFunction &MF)
    : FPFrameIndex(-1), RAFrameIndex(-1), VarArgsFrameIndex(-1) {
  info = new Info;
  info->licDepth.resize(CSA::NUM_TARGET_REGS, -1);
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

const TargetRegisterClass* CSAMachineFunctionInfo::licFromType(MVT vt) {
  if      (vt==MVT::i1)  return &CSA::CI1RegClass;
  else if (vt==MVT::i8)  return &CSA::CI8RegClass;
  else if (vt==MVT::i16) return &CSA::CI16RegClass;
  else if (vt==MVT::i32) return &CSA::CI32RegClass;
  else if (vt==MVT::i64) return &CSA::CI64RegClass;
  else return NULL;
}

unsigned CSAMachineFunctionInfo::allocateLIC(const TargetRegisterClass* RC) {
  // The register class must be a LIC class!
  // assert(
  //  RC==CI0RegClass || RC==CI1RegClass || RC==CI8RegClass ||
  //    RC==CI16RegClass || RC==CI32RegClass || RC==CI64RegClass);
  // get the index of the next lic in the class
  unsigned index = info->nextRegIndexInClass[RC->getID()]++;

  unsigned lic = RC->getRegister(index);

  assert(lic < info->licDepth.capacity() && "Invalid CSA register number");
  info->licDepth[lic] = 0;
  return lic;
}

bool CSAMachineFunctionInfo::isAllocated(unsigned lic) const {
  return info->licDepth[lic] >= 0;
}

// Set the depth for a particular LIC explicitly, rather than the default.
void CSAMachineFunctionInfo::setLICDepth(unsigned lic, int amount) {
  info->licDepth[lic] = amount;
}

// Return the depth of the specified LIC.
int CSAMachineFunctionInfo::getLICDepth(unsigned lic) {
  return info->licDepth[lic];
}

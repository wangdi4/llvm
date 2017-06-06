//===---- Nios2ABIInfo.cpp - Information about NIOS2 ABI's ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Nios2ABIInfo.h"
#include "Nios2RegisterInfo.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

static cl::opt<bool>
EnableNios2S32Calls("nios2-s32-calls", cl::Hidden,
                    cl::desc("NIOS2 S32 call: use stack only to pass arguments.\
                    "), cl::init(false));

namespace {
static const MCPhysReg O32IntRegs[4] =
  {Nios2::R4, Nios2::R5, Nios2::R6, Nios2::R7};
  
static const MCPhysReg S32IntRegs = {};
}

const ArrayRef<MCPhysReg> Nios2ABIInfo::GetByValArgRegs() const {
  if (IsO32())
    return makeArrayRef(O32IntRegs);
  if (IsS32())
    return makeArrayRef(S32IntRegs);
  llvm_unreachable("Unhandled ABI");
}

const ArrayRef<MCPhysReg> Nios2ABIInfo::GetVarArgRegs() const {
  if (IsO32())
    return makeArrayRef(O32IntRegs);
  if (IsS32())
    return makeArrayRef(S32IntRegs);
  llvm_unreachable("Unhandled ABI");
}

unsigned Nios2ABIInfo::GetCalleeAllocdArgSizeInBytes(CallingConv::ID CC) const {
  if (IsO32())
    return CC != 0;
  if (IsS32())
    return 0;
  llvm_unreachable("Unhandled ABI");
}

Nios2ABIInfo Nios2ABIInfo::computeTargetABI() {
  Nios2ABIInfo abi(ABI::Unknown);

  if (EnableNios2S32Calls)
    abi = ABI::S32;
  else
    abi = ABI::O32;
  // Assert exactly one ABI was chosen.
  assert(abi.ThisABI != ABI::Unknown);

  return abi;
}

unsigned Nios2ABIInfo::GetStackPtr() const {
  return Nios2::SP;
}

unsigned Nios2ABIInfo::GetFramePtr() const {
  return Nios2::FP;
}

unsigned Nios2ABIInfo::GetNullPtr() const {
  return Nios2::ZERO;
}

unsigned Nios2ABIInfo::GetEhDataReg(unsigned I) const {
  static const unsigned EhDataReg[] = {
    Nios2::R4, Nios2::R5
  };

  return EhDataReg[I];
}

int Nios2ABIInfo::EhDataRegSize() const {
  if (ThisABI == ABI::S32)
    return 0;
  else
    return 2;
}

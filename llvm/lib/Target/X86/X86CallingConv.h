//=== X86CallingConv.h - X86 Custom Calling Convention Routines -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the custom routines for the X86 Calling Convention that
// aren't done by tablegen.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_X86CALLINGCONV_H
#define LLVM_LIB_TARGET_X86_X86CALLINGCONV_H

#if INTEL_CUSTOMIZATION
#include "MCTargetDesc/X86MCTargetDesc.h"
#endif //INTEL_CUSTOMIZATION
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/IR/CallingConv.h"

namespace llvm {

inline bool CC_X86_32_VectorCallIndirect(unsigned &ValNo, MVT &ValVT,
                                         MVT &LocVT,
                                         CCValAssign::LocInfo &LocInfo,
                                         ISD::ArgFlagsTy &ArgFlags,
                                         CCState &State) {
  // Similar to CCPassIndirect, with the addition of inreg.
  LocVT = MVT::i32;
  LocInfo = CCValAssign::Indirect;
  ArgFlags.setInReg();
  return false; // Continue the search, but now for i32.
}


inline bool CC_X86_AnyReg_Error(unsigned &, MVT &, MVT &,
                                CCValAssign::LocInfo &, ISD::ArgFlagsTy &,
                                CCState &) {
  llvm_unreachable("The AnyReg calling convention is only supported by the " \
                   "stackmap and patchpoint intrinsics.");
  // gracefully fallback to X86 C calling convention on Release builds.
  return false;
}

#if INTEL_CUSTOMIZATION
inline bool CC_X86_32_MCUInReg(unsigned &ValNo, MVT &ValVT,
                                         MVT &LocVT,
                                         CCValAssign::LocInfo &LocInfo,
                                         ISD::ArgFlagsTy &ArgFlags,
                                         CCState &State) {
  // This is similar to CCAssignToReg<[EAX, EDX, ECX]>, but makes sure
  // not to split i64 and double between a register and stack
  static const MCPhysReg RegList[] = {X86::EAX, X86::EDX, X86::ECX};
  static const unsigned NumRegs = 3;
  
  SmallVectorImpl<CCValAssign> &PendingMembers = State.getPendingLocs();

  // TODO: Do we need to generalize this to larger cases? That is, can
  // we end up with something that's larger than 2 parts here?
  // I think not, but need to check - unfortunately, it doesn't seem
  // like there's enough information at this point to perform the check...

  // If this is the first part of an i64/double, add it to the pending list
  if (ArgFlags.isSplit()) {
    PendingMembers.push_back(
        CCValAssign::getPending(ValNo, ValVT, LocVT, LocInfo));
    return true;
  }

  // If there are no pending members, this is not the second part of
  // an i64/double, so do the usual inreg stuff
  if (PendingMembers.empty()) {
    if (unsigned Reg = State.AllocateReg(RegList)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return true;
    }
    return false;
  }

  // Ok, so, now what we do depends on how many free registers we have
  // We need to have at least 2, because we need both parts to either
  // be on the stack or in memory.
  unsigned FirstFree = State.getFirstUnallocated(RegList);
  bool UseRegs = FirstFree <= (NumRegs - 2);

  PendingMembers.push_back(CCValAssign::getPending(ValNo, ValVT, LocVT,
    LocInfo));
  for (auto &It : PendingMembers) {
    if (UseRegs)
      It.convertToReg(State.AllocateReg(RegList[FirstFree++]));
    else
      It.convertToMem(State.AllocateStack(4, 4));
    State.addLoc(It);
  }

  PendingMembers.clear();

  return true;
}
#endif //INTEL_CUSTOMIZATION

} // End llvm namespace

#endif


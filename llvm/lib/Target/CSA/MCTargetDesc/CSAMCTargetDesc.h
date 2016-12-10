//===-- LPUMCTargetDesc.h - LPU Target Descriptions -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides LPU specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LPU_MCTARGETDESC_LPUMCTARGETDESC_H
#define LLVM_LIB_TARGET_LPU_MCTARGETDESC_LPUMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

namespace llvm {
class Target;

extern Target TheLPUTarget;

} // End llvm namespace

// Defines symbolic names for LPU registers.
// This defines a mapping from register name to register number.
#define GET_REGINFO_ENUM
#include "LPUGenRegisterInfo.inc"

// Defines symbolic names for the LPU instructions.
#define GET_INSTRINFO_ENUM
#include "LPUGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "LPUGenSubtargetInfo.inc"

#endif

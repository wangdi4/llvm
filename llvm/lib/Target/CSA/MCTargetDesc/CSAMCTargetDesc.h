//===-- CSAMCTargetDesc.h - CSA Target Descriptions -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides CSA specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_MCTARGETDESC_CSAMCTARGETDESC_H
#define LLVM_LIB_TARGET_CSA_MCTARGETDESC_CSAMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

namespace llvm {
class Target;

Target &getTheCSATarget();

} // End llvm namespace

// Defines symbolic names for CSA registers.
// This defines a mapping from register name to register number.
#define GET_REGINFO_ENUM
#include "CSAGenRegisterInfo.inc"

// Defines symbolic names for the CSA instructions.
#define GET_INSTRINFO_ENUM
#include "CSAGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "CSAGenSubtargetInfo.inc"

#endif

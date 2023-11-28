//===-- CSAMCTargetDesc.h - CSA Target Descriptions -------------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
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

} // namespace llvm

// Defines symbolic names for CSA registers.
// This defines a mapping from register name to register number.
#define GET_REGINFO_ENUM
#include "CSAGenRegisterInfo.inc"

// Defines symbolic names for the CSA instructions.
#define GET_INSTRINFO_SCHED_ENUM
#define GET_INSTRINFO_ENUM
#include "CSAGenInstrInfo.inc"

namespace llvm {
namespace CSA {
// Use INVALID_OPCODE as a more meaningful alias for INSTRUCtION_LIST_END
const decltype(INSTRUCTION_LIST_END) INVALID_OPCODE = INSTRUCTION_LIST_END;
} // namespace CSA
} // namespace llvm

#define GET_SUBTARGETINFO_ENUM
#include "CSAGenSubtargetInfo.inc"

#endif

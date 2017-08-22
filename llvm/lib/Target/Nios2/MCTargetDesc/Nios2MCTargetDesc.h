//===-- Nios2MCTargetDesc.h - Nios2 Target Descriptions -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Nios2 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NIOS2_MCTARGETDESC_NIOS2MCTARGETDESC_H
#define LLVM_LIB_TARGET_NIOS2_MCTARGETDESC_NIOS2MCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class StringRef;
class Target;
class Triple;
class raw_ostream;
class raw_pwrite_stream;

Target &getTheNios2Target();


MCCodeEmitter *createNios2MCCodeEmitterEL(const MCInstrInfo &MCII,
                                         const MCRegisterInfo &MRI,
                                         MCContext &Ctx);

MCAsmBackend *createNios2AsmBackendEL32(const Target &T,
                                       const MCRegisterInfo &MRI,
                                       const Triple &TT, StringRef CPU,
                                       const MCTargetOptions &Options);

MCObjectWriter *createNios2ELFObjectWriter(raw_pwrite_stream &OS,
                                          uint8_t OSABI,
                                          bool IsLittleEndian);
} // End llvm namespace

// Defines symbolic names for Nios2 registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "Nios2GenRegisterInfo.inc"

// Defines symbolic names for the Nios2 instructions.
#define GET_INSTRINFO_ENUM
#include "Nios2GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "Nios2GenSubtargetInfo.inc"

#endif

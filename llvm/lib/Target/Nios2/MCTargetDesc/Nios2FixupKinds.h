//===-- Nios2FixupKinds.h - Nios2 Specific Fixup Entries ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NIOS2_MCTARGETDESC_NIOS2FIXUPKINDS_H
#define LLVM_LIB_TARGET_NIOS2_MCTARGETDESC_NIOS2FIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace Nios2 {
  // Although most of the current fixup types reflect a unique relocation
  // one can have multiple fixup types for a given relocation and thus need
  // to be uniquely named.
  //
  // This table *must* be in the save order of
  // MCFixupKindInfo Infos[Nios2::NumTargetFixupKinds]
  // in Nios2AsmBackend.cpp.
  //@Fixups {
  enum Fixups {
    //@ Pure upper 32 bit fixup resulting in - R_NIOS2_32.
    fixup_Nios2_32 = FirstTargetFixupKind,

    // Pure upper 16 bit fixup resulting in - R_NIOS2_HI16.
    fixup_Nios2_HI16,

    // Pure lower 16 bit fixup resulting in - R_NIOS2_LO16.
    fixup_Nios2_LO16,

    // 16 bit fixup for GP offest resulting in - R_NIOS2_GPREL16.
    fixup_Nios2_GPREL16,

    // Symbol fixup resulting in - R_NIOS2_GOT16.
    fixup_Nios2_GOT,

    // PC relative branch fixup resulting in - R_NIOS2_PC16.
    // nios2 PC16, e.g. beq
    fixup_Nios2_PC16,

    // PC relative branch fixup resulting in - R_NIOS2_PC24.
    // nios2 PC24, e.g. jeq, jmp
    fixup_Nios2_PC24,
    
    // resulting in - R_NIOS2_CALL16.
    fixup_Nios2_CALL16,

    // resulting in - R_NIOS2_TLS_GD.
    fixup_Nios2_TLSGD,

    // resulting in - R_NIOS2_TLS_GOTTPREL.
    fixup_Nios2_GOTTPREL,

    // resulting in - R_NIOS2_TLS_TPREL_HI16.
    fixup_Nios2_TP_HI,

    // resulting in - R_NIOS2_TLS_TPREL_LO16.
    fixup_Nios2_TP_LO,

    // resulting in - R_NIOS2_TLS_LDM.
    fixup_Nios2_TLSLDM,

    // resulting in - R_NIOS2_TLS_DTP_HI16.
    fixup_Nios2_DTP_HI,

    // resulting in - R_NIOS2_TLS_DTP_LO16.
    fixup_Nios2_DTP_LO,

    // resulting in - R_NIOS2_GOT_HI16
    fixup_Nios2_GOT_HI16,

    // resulting in - R_NIOS2_GOT_LO16
    fixup_Nios2_GOT_LO16,

    // Marker
    LastTargetFixupKind,
    NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
  };
  //@Fixups }
} // namespace Nios2
} // namespace llvm

#endif // LLVM_NIOS2_NIOS2FIXUPKINDS_H

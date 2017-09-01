//===-- Nios2AsmBackend.cpp - Nios2 Asm Backend  ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Nios2AsmBackend class.
//
//===----------------------------------------------------------------------===//
//

#include "MCTargetDesc/Nios2FixupKinds.h"
#include "MCTargetDesc/Nios2AsmBackend.h"

#include "MCTargetDesc/Nios2MCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Prepare value for the target space for it
static unsigned adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                                 MCContext &Ctx) {

  unsigned Kind = Fixup.getKind();

  // Add/subtract and shift
  switch (Kind) {
  default:
    return 0;
  case FK_GPRel_4:
  case FK_Data_4:
  case Nios2::fixup_Nios2_CALL16:
  case Nios2::fixup_Nios2_LO16:
  case Nios2::fixup_Nios2_GOT_LO16:
    break;
  case Nios2::fixup_Nios2_PC16:
  case Nios2::fixup_Nios2_PC24:
    // So far we are only using this type for branches and jump.
    // For branches we start 1 instruction after the branch
    // so the displacement will be one instruction size less.
    Value -= 4;
    break;
  case Nios2::fixup_Nios2_HI16:
  case Nios2::fixup_Nios2_GOT:
  case Nios2::fixup_Nios2_GOT_HI16:
    // Get the higher 16-bits. Also add 1 if bit 15 is 1.
    Value = ((Value + 0x8000) >> 16) & 0xffff;
    break;
  }

  return Value;
}

// Calculate index for Nios2 specific little endian byte order
static unsigned calculateLEIndex(unsigned i) {
  assert(i <= 3 && "Index out of range!");

  return (1 - i / 2) * 2 + i % 2;
}

/// ApplyFixup - Apply the \p Value for given \p Fixup into the provided
/// data fragment, at the offset specified by the fixup and following the
/// fixup kind as appropriate.
void Nios2AsmBackend::applyFixup(const MCFixup &Fixup, char *Data,
                                 unsigned DataSize, uint64_t Value, bool IsPCRel,
                                 MCContext &Ctx) const {
  MCFixupKind Kind = Fixup.getKind();
  Value = adjustFixupValue(Fixup, Value, Ctx);

  if (!Value)
    return; // Doesn't change encoding.

  // Where do we start in the object
  unsigned Offset = Fixup.getOffset();
  // Number of bytes we need to fixup
  unsigned NumBytes = (getFixupKindInfo(Kind).TargetSize + 7) / 8;
  // Grab current value, if any, from bits.
  uint64_t CurVal = 0;

  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned Idx = calculateLEIndex(i);
    CurVal |= (uint64_t)((uint8_t)Data[Offset + Idx]) << (i*8);
  }

  uint64_t Mask = ((uint64_t)(-1) >>
                    (64 - getFixupKindInfo(Kind).TargetSize));
  CurVal |= Value & Mask;

  // Write out the fixed up bytes back to the code/data bits.
  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned Idx = calculateLEIndex(i);
    Data[Offset + Idx] = (uint8_t)((CurVal >> (i*8)) & 0xff);
  }
}

MCObjectWriter *
Nios2AsmBackend::createObjectWriter(raw_pwrite_stream &OS) const {
  return createNios2ELFObjectWriter(OS,
    MCELFObjectTargetWriter::getOSABI(OSType), IsLittle);
}

Optional<MCFixupKind> Nios2AsmBackend::getFixupKind(StringRef Name) const {
  return StringSwitch<Optional<MCFixupKind>>(Name)
      .Case("R_NIOS2_NONE", (MCFixupKind)Nios2::fixup_Nios2_32)
      .Case("R_NIOS2_32", FK_Data_4)
      .Default(MCAsmBackend::getFixupKind(Name));
}

//@getFixupKindInfo {
const MCFixupKindInfo &Nios2AsmBackend::
getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo Infos[Nios2::NumTargetFixupKinds] = {
    // This table *must* be in same the order of fixup_* kinds in
    // Nios2FixupKinds.h.
    //
    // name                        offset  bits  flags
    { "fixup_Nios2_32",             0,     32,   0 },
    { "fixup_Nios2_HI16",           0,     16,   0 },
    { "fixup_Nios2_LO16",           0,     16,   0 },
    { "fixup_Nios2_GPREL16",        0,     16,   0 },
    { "fixup_Nios2_GOT",            0,     16,   0 },
    { "fixup_Nios2_PC16",           0,     16,  MCFixupKindInfo::FKF_IsPCRel },
    { "fixup_Nios2_PC24",           0,     24,  MCFixupKindInfo::FKF_IsPCRel },
    { "fixup_Nios2_CALL16",         0,     16,   0 },
    { "fixup_Nios2_TLSGD",          0,     16,   0 },
    { "fixup_Nios2_GOTTP",          0,     16,   0 },
    { "fixup_Nios2_TP_HI",          0,     16,   0 },
    { "fixup_Nios2_TP_LO",          0,     16,   0 },
    { "fixup_Nios2_TLSLDM",         0,     16,   0 },
    { "fixup_Nios2_DTP_HI",         0,     16,   0 },
    { "fixup_Nios2_DTP_LO",         0,     16,   0 },
    { "fixup_Nios2_GOT_HI16",       0,     16,   0 },
    { "fixup_Nios2_GOT_LO16",       0,     16,   0 }
  };

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
         "Invalid kind!");
  return Infos[Kind - FirstTargetFixupKind];
}
//@getFixupKindInfo }

/// WriteNopData - Write an (optimal) nop sequence of Count bytes
/// to the given output. If the target cannot generate such a sequence,
/// it should return an error.
///
/// \return - True on success.
bool Nios2AsmBackend::writeNopData(uint64_t Count, MCObjectWriter *OW) const {
  return true;
}

// MCAsmBackend
MCAsmBackend *llvm::createNios2AsmBackendEL32(const Target &T,
                                              const MCRegisterInfo &MRI,
                                              const Triple &TT, StringRef CPU,                                                                   const MCTargetOptions &Options) {

  return new Nios2AsmBackend(T, TT.getOS(), /*IsLittle*/true);
}

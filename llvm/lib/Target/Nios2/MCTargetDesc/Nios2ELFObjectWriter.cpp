//===-- Nios2ELFObjectWriter.cpp - Nios2 ELF Writer -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/Nios2BaseInfo.h"
#include "MCTargetDesc/Nios2FixupKinds.h"
#include "MCTargetDesc/Nios2MCTargetDesc.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include <list>

using namespace llvm;

namespace {
  class Nios2ELFObjectWriter : public MCELFObjectTargetWriter {
  public:
    Nios2ELFObjectWriter(uint8_t OSABI);

    ~Nios2ELFObjectWriter() override;

    unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
    bool needsRelocateWithSymbol(const MCSymbol &Sym,
                                 unsigned Type) const override;
  };
}

Nios2ELFObjectWriter::Nios2ELFObjectWriter(uint8_t OSABI)
  : MCELFObjectTargetWriter(/*_is64Bit=false*/ false, OSABI, ELF::EM_ALTERA_NIOS2,
                            /*HasRelocationAddend*/ false) {}

Nios2ELFObjectWriter::~Nios2ELFObjectWriter() {}

//@GetRelocType {
unsigned Nios2ELFObjectWriter::getRelocType(MCContext &Ctx,
                                           const MCValue &Target,
                                           const MCFixup &Fixup,
                                           bool IsPCRel) const {
  // determine the type of the relocation
  unsigned Type = (unsigned)ELF::R_NIOS2_NONE;
  unsigned Kind = (unsigned)Fixup.getKind();

  switch (Kind) {
  default:
    llvm_unreachable("invalid fixup kind!");
  case FK_Data_4:
    Type = ELF::R_NIOS2_32;
    break;
  case FK_GPRel_4:
    Type = ELF::R_NIOS2_GPREL32;
    break;
  case Nios2::fixup_Nios2_32:
    Type = ELF::R_NIOS2_32;
    break;
  case Nios2::fixup_Nios2_GPREL16:
    Type = ELF::R_NIOS2_GPREL16;
    break;
  case Nios2::fixup_Nios2_CALL16:
    Type = ELF::R_NIOS2_CALL16;
    break;
  case Nios2::fixup_Nios2_GOT:
    Type = ELF::R_NIOS2_GOT16;
    break;
  case Nios2::fixup_Nios2_HI16:
    Type = ELF::R_NIOS2_HI16;
    break;
  case Nios2::fixup_Nios2_LO16:
    Type = ELF::R_NIOS2_LO16;
    break;
  case Nios2::fixup_Nios2_TLSGD:
    Type = ELF::R_NIOS2_TLS_GD;
    break;
  case Nios2::fixup_Nios2_GOTTPREL:
    Type = ELF::R_NIOS2_TLS_GOTTPREL;
    break;
  case Nios2::fixup_Nios2_PC16:
    Type = ELF::R_NIOS2_PC16;
    break;
  case Nios2::fixup_Nios2_PC24:
    Type = ELF::R_NIOS2_PC24;
    break;
  case Nios2::fixup_Nios2_TP_HI:
    Type = ELF::R_NIOS2_TLS_TP_HI16;
    break;
  case Nios2::fixup_Nios2_TP_LO:
    Type = ELF::R_NIOS2_TLS_TP_LO16;
    break;
  case Nios2::fixup_Nios2_TLSLDM:
    Type = ELF::R_NIOS2_TLS_LDM;
    break;
  case Nios2::fixup_Nios2_DTP_HI:
    Type = ELF::R_NIOS2_TLS_DTP_HI16;
    break;
  case Nios2::fixup_Nios2_DTP_LO:
    Type = ELF::R_NIOS2_TLS_DTP_LO16;
    break;
  case Nios2::fixup_Nios2_GOT_HI16:
    Type = ELF::R_NIOS2_GOT_HI16;
    break;
  case Nios2::fixup_Nios2_GOT_LO16:
    Type = ELF::R_NIOS2_GOT_LO16;
    break;
  }

  return Type;
}

bool
Nios2ELFObjectWriter::needsRelocateWithSymbol(const MCSymbol &Sym,
                                             unsigned Type) const {
  // FIXME: This is extremelly conservative. This really needs to use a
  // whitelist with a clear explanation for why each realocation needs to
  // point to the symbol, not to the section.
  switch (Type) {
  default:
    return true;

  case ELF::R_NIOS2_GOT16:
  // For Nios2 pic mode, I think it's OK to return true but I didn't confirm.
  //  llvm_unreachable("Should have been handled already");
    return true;

  // These relocations might be paired with another relocation. The pairing is
  // done by the static linker by matching the symbol. Since we only see one
  // relocation at a time, we have to force them to relocate with a symbol to
  // avoid ending up with a pair where one points to a section and another
  // points to a symbol.
  case ELF::R_NIOS2_HI16:
  case ELF::R_NIOS2_LO16:
  // R_NIOS2_32 should be a relocation record, I don't know why Mips set it to 
  // false.
  case ELF::R_NIOS2_32:
    return true;

  case ELF::R_NIOS2_GPREL16:
    return false;
  }
}

MCObjectWriter *llvm::createNios2ELFObjectWriter(raw_pwrite_stream &OS,
                                                uint8_t OSABI,
                                                bool IsLittleEndian) {
  MCELFObjectTargetWriter *MOTW = new Nios2ELFObjectWriter(OSABI);
  return createELFObjectWriter(MOTW, OS, IsLittleEndian);
}

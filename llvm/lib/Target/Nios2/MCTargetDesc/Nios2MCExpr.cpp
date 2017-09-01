//===-- Nios2MCExpr.cpp - Nios2 specific MC expression classes --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Nios2.h"

#include "Nios2MCExpr.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCObjectStreamer.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/Support/ELF.h"

using namespace llvm;

#define DEBUG_TYPE "nios2mcexpr"

const Nios2MCExpr *Nios2MCExpr::create(Nios2MCExpr::Nios2ExprKind Kind,
                                     const MCExpr *Expr, MCContext &Ctx) {
  return new (Ctx) Nios2MCExpr(Kind, Expr);
}

const Nios2MCExpr *Nios2MCExpr::create(const MCSymbol *Symbol, Nios2MCExpr::Nios2ExprKind Kind,
                         MCContext &Ctx) {
  const MCSymbolRefExpr *MCSym =
      MCSymbolRefExpr::create(Symbol, MCSymbolRefExpr::VK_None, Ctx);
  return new (Ctx) Nios2MCExpr(Kind, MCSym);
}

const Nios2MCExpr *Nios2MCExpr::createGpOff(Nios2MCExpr::Nios2ExprKind Kind,
                                          const MCExpr *Expr, MCContext &Ctx) {
  return create(Kind, create(CEK_None, create(CEK_GPREL, Expr, Ctx), Ctx), Ctx);
}

void Nios2MCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  int64_t AbsVal;

  switch (Kind) {
  case CEK_None:
  case CEK_Special:
    llvm_unreachable("CEK_None and CEK_Special are invalid");
    break;
  case CEK_CALL_HI16:
    OS << "%call_hi";
    break;
  case CEK_CALL_LO16:
    OS << "%call_lo";
    break;
  case CEK_DTP_HI:
    OS << "%dtp_hi";
    break;
  case CEK_DTP_LO:
    OS << "%dtp_lo";
    break;
  case CEK_GOT:
    OS << "%got";
    break;
  case CEK_GOTTPREL:
    OS << "%gottprel";
    break;
  case CEK_GOT_CALL:
    OS << "%call16";
    break;
  case CEK_GOT_DISP:
    OS << "%got_disp";
    break;
  case CEK_GOT_HI16:
    OS << "%got_hi";
    break;
  case CEK_GOT_LO16:
    OS << "%got_lo";
    break;
  case CEK_GPREL:
    OS << "%gp_rel";
    break;
  case CEK_ABS_HI:
    OS << "%hiadj";
    break;
  case CEK_ABS_LO:
    OS << "%lo";
    break;
  case CEK_TLSGD:
    OS << "%tlsgd";
    break;
  case CEK_TLSLDM:
    OS << "%tlsldm";
    break;
  case CEK_TP_HI:
    OS << "%tp_hi";
    break;
  case CEK_TP_LO:
    OS << "%tp_lo";
    break;
  }

  OS << '(';
  if (Expr->evaluateAsAbsolute(AbsVal))
    OS << AbsVal;
  else
    Expr->print(OS, MAI, true);
  OS << ')';
}

bool
Nios2MCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                      const MCAsmLayout *Layout,
                                      const MCFixup *Fixup) const {
  return getSubExpr()->evaluateAsRelocatable(Res, Layout, Fixup);
}

void Nios2MCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}

void Nios2MCExpr::fixELFSymbolsInTLSFixups(MCAssembler &Asm) const {
  switch (getKind()) {
  case CEK_None:
  case CEK_Special:
    llvm_unreachable("CEK_None and CEK_Special are invalid");
    break;
  case CEK_CALL_HI16:
  case CEK_CALL_LO16:
  case CEK_ABS_HI:                                                                                     case CEK_ABS_LO:
  case CEK_DTP_HI:
  case CEK_DTP_LO:
  case CEK_GOT:
  case CEK_GOTTPREL:
  case CEK_GOT_CALL:
  case CEK_GOT_DISP:
  case CEK_GOT_HI16:
  case CEK_GOT_LO16:
  case CEK_GPREL:
  case CEK_TLSGD:
  case CEK_TLSLDM:
  case CEK_TP_HI:
  case CEK_TP_LO:
    break;
  }
}

bool Nios2MCExpr::isGpOff(Nios2ExprKind &Kind) const {
  if (const Nios2MCExpr *S1 = dyn_cast<const Nios2MCExpr>(getSubExpr())) {
    if (const Nios2MCExpr *S2 = dyn_cast<const Nios2MCExpr>(S1->getSubExpr())) {
      if (S1->getKind() == CEK_None && S2->getKind() == CEK_GPREL) {
        Kind = getKind();
        return true;
      }
    }
  }
  return false;
}

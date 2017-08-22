//===-- Nios2MCCodeEmitter.cpp - Convert Nios2 Code to Machine Code ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Nios2MCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//

#include "Nios2MCCodeEmitter.h"
#include "MCTargetDesc/Nios2BaseInfo.h"
#include "MCTargetDesc/Nios2FixupKinds.h"
#include "MCTargetDesc/Nios2MCExpr.h"
#include "MCTargetDesc/Nios2MCTargetDesc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"

#define DEBUG_TYPE "mccodeemitter"

#define GET_INSTRMAP_INFO
#include "Nios2GenInstrInfo.inc"
#undef GET_INSTRMAP_INFO

namespace llvm {
//MCCodeEmitter *llvm::createNios2MCCodeEmitterEB(const MCInstrInfo &MCII,
MCCodeEmitter *createNios2MCCodeEmitterEB(const MCInstrInfo &MCII,
                                         const MCRegisterInfo &MRI,
                                         MCContext &Ctx) {
  return new Nios2MCCodeEmitter(MCII, Ctx, false);
}

//MCCodeEmitter *llvm::createNios2MCCodeEmitterEL(const MCInstrInfo &MCII,
MCCodeEmitter *createNios2MCCodeEmitterEL(const MCInstrInfo &MCII,
                                         const MCRegisterInfo &MRI,
                                         MCContext &Ctx) {
  return new Nios2MCCodeEmitter(MCII, Ctx, true);
}
} // End of namespace llvm

void Nios2MCCodeEmitter::EmitByte(unsigned char C, raw_ostream &OS) const {
  OS << (char)C;
}

void Nios2MCCodeEmitter::EmitInstruction(uint64_t Val, unsigned Size, raw_ostream &OS) const {
  // Output the instruction encoding in little endian byte order.
  for (unsigned i = 0; i < Size; ++i) {
    unsigned Shift = IsLittleEndian ? i * 8 : (Size - 1 - i) * 8;
    EmitByte((Val >> Shift) & 0xff, OS);
  }
}

/// encodeInstruction - Emit the instruction.
/// Size the instruction (currently only 4 bytes)
void Nios2MCCodeEmitter::
encodeInstruction(const MCInst &MI, raw_ostream &OS,
                  SmallVectorImpl<MCFixup> &Fixups,
                  const MCSubtargetInfo &STI) const
{
  uint32_t Binary = getBinaryCodeForInstr(MI, Fixups, STI);

  // Check for unimplemented opcodes.
  // Unfortunately in NIOS2 both NOT and SLL will come in with Binary == 0
  // so we have to special check for them.
  unsigned Opcode = MI.getOpcode();
  if ((Opcode != Nios2::NOP) && (Opcode != Nios2::ROL_R1) && !Binary) //TODO: Nios2 R2 support
    llvm_unreachable("unimplemented opcode in encodeInstruction()");

  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  uint64_t TSFlags = Desc.TSFlags;

  // Pseudo instructions don't get encoded and shouldn't be here
  // in the first place!
  if ((TSFlags & Nios2II::FormMask) == Nios2II::Pseudo)
    llvm_unreachable("Pseudo opcode found in encodeInstruction()");

  // For now all instructions are 4 bytes
  int Size = 4; // FIXME: Have Desc.getSize() return the correct value!

  EmitInstruction(Binary, Size, OS);
}

/// getBranch16TargetOpValue - Return binary encoding of the branch
/// target operand. If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned Nios2MCCodeEmitter::
getBranch16TargetOpValue(const MCInst &MI, unsigned OpNo,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  // If the destination is an immediate, we have nothing to do.
  if (MO.isImm()) return MO.getImm();
  assert(MO.isExpr() && "getBranch16TargetOpValue expects only expressions");

  const MCExpr *Expr = MO.getExpr();
  Fixups.push_back(MCFixup::create(0, Expr,
                                   MCFixupKind(Nios2::fixup_Nios2_PC16)));
  return 0;
}

/// getBranch24TargetOpValue - Return binary encoding of the branch
/// target operand. If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned Nios2MCCodeEmitter::
getBranch24TargetOpValue(const MCInst &MI, unsigned OpNo,
                       SmallVectorImpl<MCFixup> &Fixups,
                       const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  // If the destination is an immediate, we have nothing to do.
  if (MO.isImm()) return MO.getImm();
  assert(MO.isExpr() && "getBranch24TargetOpValue expects only expressions");

  const MCExpr *Expr = MO.getExpr();
  Fixups.push_back(MCFixup::create(0, Expr,
                                   MCFixupKind(Nios2::fixup_Nios2_PC24)));
  return 0;
}

/// getJumpTargetOpValue - Return binary encoding of the jump
/// target operand, such as JSUB. 
/// If the machine operand requires relocation,
/// record the relocation and return zero.
//@getJumpTargetOpValue {
unsigned Nios2MCCodeEmitter::
getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups,
                     const MCSubtargetInfo &STI) const {
  unsigned Opcode = MI.getOpcode();
  const MCOperand &MO = MI.getOperand(OpNo);
  // If the destination is an immediate, we have nothing to do.
  if (MO.isImm()) return MO.getImm();
  assert(MO.isExpr() && "getJumpTargetOpValue expects only expressions");

  const MCExpr *Expr = MO.getExpr();
  if (Opcode == Nios2::CALL_R1 || Opcode == Nios2::JMP_R1) // TODO: Nios2 R2 support
    Fixups.push_back(MCFixup::create(0, Expr,
                                     MCFixupKind(Nios2::fixup_Nios2_PC24)));
  else
    llvm_unreachable("unexpect opcode in getJumpAbsoluteTargetOpValue()");
  return 0;
}

unsigned Nios2MCCodeEmitter::
getExprOpValue(const MCExpr *Expr, SmallVectorImpl<MCFixup> &Fixups,
               const MCSubtargetInfo &STI) const {
  MCExpr::ExprKind Kind = Expr->getKind();
  if (Kind == MCExpr::Constant) {
    return cast<MCConstantExpr>(Expr)->getValue();
  }

  if (Kind == MCExpr::Binary) {
    unsigned Res = getExprOpValue(cast<MCBinaryExpr>(Expr)->getLHS(), Fixups, STI);
    Res += getExprOpValue(cast<MCBinaryExpr>(Expr)->getRHS(), Fixups, STI);
    return Res;
  }

  if (Kind == MCExpr::Target) {
    const Nios2MCExpr *Nios2Expr = cast<Nios2MCExpr>(Expr);

    Nios2::Fixups FixupKind = Nios2::Fixups(0);
    switch (Nios2Expr->getKind()) {
    default: llvm_unreachable("Unsupported fixup kind for target expression!");
    case Nios2MCExpr::CEK_GPREL:
      FixupKind = Nios2::fixup_Nios2_GPREL16;
      break;
    case Nios2MCExpr::CEK_GOT_CALL:
      FixupKind = Nios2::fixup_Nios2_CALL16;
      break;
    case Nios2MCExpr::CEK_GOT:
      FixupKind = Nios2::fixup_Nios2_GOT;
      break;
    case Nios2MCExpr::CEK_ABS_HI:
      FixupKind = Nios2::fixup_Nios2_HI16;
      break;
    case Nios2MCExpr::CEK_ABS_LO:
      FixupKind = Nios2::fixup_Nios2_LO16;
      break;
    case Nios2MCExpr::CEK_TLSGD:
      FixupKind = Nios2::fixup_Nios2_TLSGD;
      break;
    case Nios2MCExpr::CEK_TLSLDM:
      FixupKind = Nios2::fixup_Nios2_TLSLDM;
      break;
    case Nios2MCExpr::CEK_DTP_HI:
      FixupKind = Nios2::fixup_Nios2_DTP_HI;
      break;
    case Nios2MCExpr::CEK_DTP_LO:
      FixupKind = Nios2::fixup_Nios2_DTP_LO;
      break;
    case Nios2MCExpr::CEK_GOTTPREL:
      FixupKind = Nios2::fixup_Nios2_GOTTPREL;
      break;
    case Nios2MCExpr::CEK_TP_HI:
      FixupKind = Nios2::fixup_Nios2_TP_HI;
      break;
    case Nios2MCExpr::CEK_TP_LO:
      FixupKind = Nios2::fixup_Nios2_TP_LO;
      break;
    case Nios2MCExpr::CEK_GOT_HI16:
      FixupKind = Nios2::fixup_Nios2_GOT_HI16;
      break;
    case Nios2MCExpr::CEK_GOT_LO16:
      FixupKind = Nios2::fixup_Nios2_GOT_LO16;
      break;
    } // switch
    Fixups.push_back(MCFixup::create(0, Expr, MCFixupKind(FixupKind)));
    return 0;
  }


  // All of the information is in the fixup.
  return 0;
}

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned Nios2MCCodeEmitter::
getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                  SmallVectorImpl<MCFixup> &Fixups,
                  const MCSubtargetInfo &STI) const {
  if (MO.isReg()) {
    unsigned Reg = MO.getReg();
    unsigned RegNo = Ctx.getRegisterInfo()->getEncodingValue(Reg);
    return RegNo;
  } else if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  } else if (MO.isFPImm()) {
    return static_cast<unsigned>(APFloat(MO.getFPImm())
        .bitcastToAPInt().getHiBits(32).getLimitedValue());
  }
  // MO must be an Expr.
  assert(MO.isExpr());
  return getExprOpValue(MO.getExpr(),Fixups, STI);
}

/// getMemEncoding - Return binary encoding of memory related operand.
/// If the offset operand requires relocation, record the relocation.
unsigned
Nios2MCCodeEmitter::getMemEncoding(const MCInst &MI, unsigned OpNo,
                                  SmallVectorImpl<MCFixup> &Fixups,
                                  const MCSubtargetInfo &STI) const {
  // Base register is encoded in bits 20-16, offset is encoded in bits 15-0.
  assert(MI.getOperand(OpNo).isReg());
  unsigned RegBits = getMachineOpValue(MI, MI.getOperand(OpNo), Fixups, STI) << 16;
  unsigned OffBits = getMachineOpValue(MI, MI.getOperand(OpNo+1), Fixups, STI);

  return (OffBits & 0xFFFF) | RegBits;
}

#include "Nios2GenMCCodeEmitter.inc"

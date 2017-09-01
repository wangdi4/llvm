//===- Nios2Disassembler.cpp - Disassembler for Nios2 -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is part of the Nios2 Disassembler.
//
//===----------------------------------------------------------------------===//

#include "Nios2.h"

#include "Nios2RegisterInfo.h"
#include "Nios2Subtarget.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCFixedLenDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "nios2-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {

/// Nios2DisassemblerBase - a disasembler class for Nios2.
class Nios2DisassemblerBase : public MCDisassembler {
public:
  /// Constructor     - Initializes the disassembler.
  ///
  Nios2DisassemblerBase(const MCSubtargetInfo &STI, MCContext &Ctx,
                       bool bigEndian) :
    MCDisassembler(STI, Ctx),
    IsBigEndian(bigEndian) {}

  virtual ~Nios2DisassemblerBase() {}

protected:
  bool IsBigEndian;
};

/// Nios2Disassembler - a disasembler class for Nios232.
class Nios2Disassembler : public Nios2DisassemblerBase {
public:
  /// Constructor     - Initializes the disassembler.
  ///
  Nios2Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx, bool bigEndian)
      : Nios2DisassemblerBase(STI, Ctx, bigEndian) {
  }

  /// getInstruction - See MCDisassembler.
  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &VStream,
                              raw_ostream &CStream) const override;
};

} // end anonymous namespace

// Decoder tables for GPR register
static const unsigned CPURegsTable[] = {
  Nios2::ZERO, Nios2::AT, Nios2::R2, Nios2::R3,
  Nios2::R4, Nios2::R5, Nios2::EA, Nios2::R6, 
  Nios2::R7, Nios2::R8, Nios2::R9, Nios2::GP, 
  Nios2::FP, Nios2::SP, Nios2::RA, Nios2::R8
};

// Decoder tables for co-processor 0 register
static const unsigned C0RegsTable[] = {
  Nios2::PC, Nios2::R9
};

static DecodeStatus DecodeCPURegsRegisterClass(MCInst &Inst,
                                               unsigned RegNo,
                                               uint64_t Address,
                                               const void *Decoder);
//static DecodeStatus DecodeGPROutRegisterClass(MCInst &Inst,
//                                               unsigned RegNo,
//                                               uint64_t Address,
//                                               const void *Decoder);
//static DecodeStatus DecodeSRRegisterClass(MCInst &Inst,
//                                               unsigned RegNo,
//                                               uint64_t Address,
//                                               const void *Decoder);
//static DecodeStatus DecodeC0RegsRegisterClass(MCInst &Inst,
//                                              unsigned RegNo,
//                                              uint64_t Address,
//                                              const void *Decoder);
static DecodeStatus DecodeBranch16Target(MCInst &Inst,
                                       unsigned Insn,
                                       uint64_t Address,
                                       const void *Decoder);
//static DecodeStatus DecodeBranch24Target(MCInst &Inst,
//                                       unsigned Insn,
//                                       uint64_t Address,
//                                       const void *Decoder);
static DecodeStatus DecodeJumpTarget(MCInst &Inst,
                                     unsigned Insn,
                                     uint64_t Address,
                                     const void *Decoder);
//static DecodeStatus DecodeJumpFR(MCInst &Inst,
//                                 unsigned Insn,
//                                 uint64_t Address,
//                                 const void *Decoder);

//static DecodeStatus DecodeMem(MCInst &Inst,
//                              unsigned Insn,
//                              uint64_t Address,
//                              const void *Decoder);
static DecodeStatus DecodeSimm16(MCInst &Inst,
                                 unsigned Insn,
                                 uint64_t Address,
                                 const void *Decoder);

namespace llvm {
extern Target TheNios2elTarget, TheNios2Target, TheNios264Target,
              TheNios264elTarget;
}

static MCDisassembler *createNios2Disassembler(
                       const Target &T,
                       const MCSubtargetInfo &STI,
                       MCContext &Ctx) {
  return new Nios2Disassembler(STI, Ctx, true);
}

//static MCDisassembler *createNios2elDisassembler(
//                       const Target &T,
//                       const MCSubtargetInfo &STI,
//                       MCContext &Ctx) {
//  return new Nios2Disassembler(STI, Ctx, false);
//}

extern "C" void LLVMInitializeNios2Disassembler() {
  // Register the disassembler.
  TargetRegistry::RegisterMCDisassembler(getTheNios2Target(),
                                         createNios2Disassembler);
}


#include "Nios2GenDisassemblerTables.inc"

/// Read four bytes from the ArrayRef and return 32 bit word sorted
/// according to the given endianess
static DecodeStatus readInstruction32(ArrayRef<uint8_t> Bytes, uint64_t Address,
                                      uint64_t &Size, uint32_t &Insn,
                                      bool IsBigEndian) {
  // We want to read exactly 4 Bytes of data.
  if (Bytes.size() < 4) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  if (IsBigEndian) {
    // Encoded as a big-endian 32-bit word in the stream.
    Insn = (Bytes[3] <<  0) |
           (Bytes[2] <<  8) |
           (Bytes[1] << 16) |
           (Bytes[0] << 24);
  }
  else {
    // Encoded as a small-endian 32-bit word in the stream.
    Insn = (Bytes[0] <<  0) |
           (Bytes[1] <<  8) |
           (Bytes[2] << 16) |
           (Bytes[3] << 24);
  }

  return MCDisassembler::Success;
}

DecodeStatus
Nios2Disassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                              ArrayRef<uint8_t> Bytes,
                                              uint64_t Address,
                                              raw_ostream &VStream,
                                              raw_ostream &CStream) const {
  uint32_t Insn;

  DecodeStatus Result;

  Result = readInstruction32(Bytes, Address, Size, Insn, IsBigEndian);

  if (Result == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Calling the auto-generated decoder function.
  Result = decodeInstruction(DecoderTableNios232, Instr, Insn, Address,
                             this, STI);
  if (Result != MCDisassembler::Fail) {
    Size = 4;
    return Result;
  }

  return MCDisassembler::Fail;
}

static DecodeStatus DecodeCPURegsRegisterClass(MCInst &Inst,
                                               unsigned RegNo,
                                               uint64_t Address,
                                               const void *Decoder) {
  if (RegNo > 15)
    return MCDisassembler::Fail;

  Inst.addOperand(MCOperand::createReg(CPURegsTable[RegNo]));
  return MCDisassembler::Success;
}

//static DecodeStatus DecodeGPROutRegisterClass(MCInst &Inst,
//                                               unsigned RegNo,
//                                               uint64_t Address,
//                                               const void *Decoder) {
//  return DecodeCPURegsRegisterClass(Inst, RegNo, Address, Decoder);
//}

//static DecodeStatus DecodeSRRegisterClass(MCInst &Inst,
//                                               unsigned RegNo,
//                                               uint64_t Address,
//                                               const void *Decoder) {
//  return DecodeCPURegsRegisterClass(Inst, RegNo, Address, Decoder);
//}

//static DecodeStatus DecodeC0RegsRegisterClass(MCInst &Inst,
//                                              unsigned RegNo,
//                                              uint64_t Address,
//                                              const void *Decoder) {
//  if (RegNo > 1)
//    return MCDisassembler::Fail;

//  Inst.addOperand(MCOperand::createReg(C0RegsTable[RegNo]));
//  return MCDisassembler::Success;
//}

//static DecodeStatus DecodeMem(MCInst &Inst,
//                              unsigned Insn,
//                              uint64_t Address,
//                              const void *Decoder) {
//  int Offset = SignExtend32<16>(Insn & 0xffff);
//  int Reg = (int)fieldFromInstruction(Insn, 20, 4);
//  int Base = (int)fieldFromInstruction(Insn, 16, 4);
//
//  if(Inst.getOpcode() == Nios2::R10){
//    Inst.addOperand(MCOperand::createReg(Reg));
//  }

//  Inst.addOperand(MCOperand::createReg(CPURegsTable[Reg]));
//  Inst.addOperand(MCOperand::createReg(CPURegsTable[Base]));
//  Inst.addOperand(MCOperand::createImm(Offset));

//  return MCDisassembler::Success;
//}

static DecodeStatus DecodeBranch16Target(MCInst &Inst,
                                       unsigned Insn,
                                       uint64_t Address,
                                       const void *Decoder) {
  int BranchOffset = fieldFromInstruction(Insn, 0, 16);
  if (BranchOffset > 0x8fff)
  	BranchOffset = -1*(0x10000 - BranchOffset);
  Inst.addOperand(MCOperand::createImm(BranchOffset));
  return MCDisassembler::Success;
}

//static DecodeStatus DecodeBranch24Target(MCInst &Inst,
//                                       unsigned Insn,
//                                       uint64_t Address,
//                                       const void *Decoder) {
//  int BranchOffset = fieldFromInstruction(Insn, 0, 24);
//  if (BranchOffset > 0x8fffff)
//  	BranchOffset = -1*(0x1000000 - BranchOffset);
//  Inst.addOperand(MCOperand::createReg(Nios2::R8));
//  Inst.addOperand(MCOperand::createImm(BranchOffset));
//  return MCDisassembler::Success;
//}

static DecodeStatus DecodeJumpTarget(MCInst &Inst,
                                     unsigned Insn,
                                     uint64_t Address,
                                     const void *Decoder) {

  unsigned JumpOffset = fieldFromInstruction(Insn, 0, 24);
  Inst.addOperand(MCOperand::createImm(JumpOffset));
  return MCDisassembler::Success;
}

//static DecodeStatus DecodeJumpFR(MCInst &Inst,
//                                     unsigned Insn,
//                                     uint64_t Address,
//                                     const void *Decoder) {
//  int Reg_a = (int)fieldFromInstruction(Insn, 20, 4);
//  Inst.addOperand(MCOperand::createReg(CPURegsTable[Reg_a]));
// exapin in http://jonathan2251.github.io/lbd/llvmstructure.html#jr-note
//  if (CPURegsTable[Reg_a] == Nios2::RA)
//    Inst.setOpcode(Nios2::RET_R1); // TODO: nios2 R2 support
//  else
//    Inst.setOpcode(Nios2::JMP_R1); // TODO: nios2 R2 support
//  return MCDisassembler::Success;
//}

static DecodeStatus DecodeSimm16(MCInst &Inst,
                                 unsigned Insn,
                                 uint64_t Address,
                                 const void *Decoder) {
  Inst.addOperand(MCOperand::createImm(SignExtend32<16>(Insn)));
  return MCDisassembler::Success;
}

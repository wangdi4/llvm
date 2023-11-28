//==-- Intel_X86AvoidMemoryRenamingBlocks.cpp - Avoid Memory Renaming Block --==
//
//      Copyright (c) 2022 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//===----------------------------------------------------------------------===//
//
// This pass tries to make make store/load pair MRN-able:
// 1) MRN doesn't support RIP/index register when do addressing, This pass will
// promote the address calculating by a LEA instruction.
//
// 2) Some instructions with load or RMW instructions can't do MRN, this pass
// will expand these instructions as load/op or load/op/store.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/MC/MCInstrDesc.h"

using namespace llvm;

#define DEBUG_TYPE "x86-avoid-MRN-blocks"

static cl::opt<bool> DisableX86AvoidMemoryRenamingBlocks(
    "x86-disable-avoid-MRN-blocks", cl::Hidden,
    cl::desc("X86: Disable Memory Renaming Blocks Fixup."), cl::init(false));

static cl::opt<unsigned> X86AvoidMRNBInspectionLimit(
    "x86-mrnb-inspection-limit",
    cl::desc("X86: Number of instructions forward to "
             "inspect for memory renaming blocks."),
    cl::init(50), cl::Hidden);

namespace {

enum ENTRY_FLAGS { ENTRY_FLAGS_EXPAND = 1 };

struct X86MemoryExpandTableEntry {
  uint32_t KeyOp;
  uint32_t DstOp;
  uint32_t Flags;

  bool operator<(const X86MemoryExpandTableEntry &RHS) const {
    return KeyOp < RHS.KeyOp;
  }
  bool operator==(const X86MemoryExpandTableEntry &RHS) const {
    return KeyOp == RHS.KeyOp;
  }
  friend bool operator<(const X86MemoryExpandTableEntry &TE, unsigned Opcode) {
    return TE.KeyOp < Opcode;
  }
};

static const X86MemoryExpandTableEntry RMWTable[] = {
    {X86::ADC16mi, X86::ADC16ri, 0},
    {X86::ADC16mr, X86::ADC16rr, ENTRY_FLAGS_EXPAND},
    {X86::ADC32mi, X86::ADC32ri, 0},
    {X86::ADC32mr, X86::ADC32rr, ENTRY_FLAGS_EXPAND},
    {X86::ADC64mi32, X86::ADC64ri32, 0},
    {X86::ADC64mr, X86::ADC64rr, ENTRY_FLAGS_EXPAND},
    {X86::ADC8mi, X86::ADC8ri, 0},
    {X86::ADC8mr, X86::ADC8rr, ENTRY_FLAGS_EXPAND},
    {X86::ADD16mi, X86::ADD16ri, 0},
    {X86::ADD16mr, X86::ADD16rr, ENTRY_FLAGS_EXPAND},
    {X86::ADD32mi, X86::ADD32ri, 0},
    {X86::ADD32mr, X86::ADD32rr, ENTRY_FLAGS_EXPAND},
    {X86::ADD64mi32, X86::ADD64ri32, 0},
    {X86::ADD64mr, X86::ADD64rr, ENTRY_FLAGS_EXPAND},
    {X86::ADD8mi, X86::ADD8ri, 0},
    {X86::ADD8mr, X86::ADD8rr, ENTRY_FLAGS_EXPAND},
    {X86::AND16mi, X86::AND16ri, 0},
    {X86::AND16mr, X86::AND16rr, ENTRY_FLAGS_EXPAND},
    {X86::AND32mi, X86::AND32ri, 0},
    {X86::AND32mr, X86::AND32rr, ENTRY_FLAGS_EXPAND},
    {X86::AND64mi32, X86::AND64ri32, 0},
    {X86::AND64mr, X86::AND64rr, ENTRY_FLAGS_EXPAND},
    {X86::AND8mi, X86::AND8ri, 0},
    {X86::AND8mr, X86::AND8rr, ENTRY_FLAGS_EXPAND},
    {X86::BTC16mi8, X86::BTC16ri8, 0},
    {X86::BTC32mi8, X86::BTC32ri8, 0},
    {X86::BTC64mi8, X86::BTC64ri8, 0},
    {X86::BTR16mi8, X86::BTR16ri8, 0},
    {X86::BTR32mi8, X86::BTR32ri8, 0},
    {X86::BTR64mi8, X86::BTR64ri8, 0},
    {X86::BTS16mi8, X86::BTS16ri8, 0},
    {X86::BTS32mi8, X86::BTS32ri8, 0},
    {X86::BTS64mi8, X86::BTS64ri8, 0},
    {X86::DEC16m, X86::DEC16r, 0},
    {X86::DEC32m, X86::DEC32r, 0},
    {X86::DEC64m, X86::DEC64r, 0},
    {X86::DEC8m, X86::DEC8r, 0},
    {X86::INC16m, X86::INC16r, 0},
    {X86::INC32m, X86::INC32r, 0},
    {X86::INC64m, X86::INC64r, 0},
    {X86::INC8m, X86::INC8r, 0},
    {X86::NEG16m, X86::NEG16r, 0},
    {X86::NEG32m, X86::NEG32r, 0},
    {X86::NEG64m, X86::NEG64r, 0},
    {X86::NEG8m, X86::NEG8r, 0},
    {X86::NOT16m, X86::NOT16r, 0},
    {X86::NOT32m, X86::NOT32r, 0},
    {X86::NOT64m, X86::NOT64r, 0},
    {X86::NOT8m, X86::NOT8r, 0},
    {X86::OR16mi, X86::OR16ri, 0},
    {X86::OR16mr, X86::OR16rr, ENTRY_FLAGS_EXPAND},
    {X86::OR32mi, X86::OR32ri, 0},
    {X86::OR32mr, X86::OR32rr, ENTRY_FLAGS_EXPAND},
    {X86::OR64mi32, X86::OR64ri32, 0},
    {X86::OR64mr, X86::OR64rr, ENTRY_FLAGS_EXPAND},
    {X86::OR8mi, X86::OR8ri, 0},
    {X86::OR8mr, X86::OR8rr, ENTRY_FLAGS_EXPAND},
    {X86::RCL16m1, X86::RCL16r1, 0},
    {X86::RCL16mCL, X86::RCL16rCL, ENTRY_FLAGS_EXPAND},
    {X86::RCL16mi, X86::RCL16ri, 0},
    {X86::RCL32m1, X86::RCL32r1, 0},
    {X86::RCL32mCL, X86::RCL32rCL, ENTRY_FLAGS_EXPAND},
    {X86::RCL32mi, X86::RCL32ri, 0},
    {X86::RCL64m1, X86::RCL64r1, 0},
    {X86::RCL64mCL, X86::RCL64rCL, ENTRY_FLAGS_EXPAND},
    {X86::RCL64mi, X86::RCL64ri, 0},
    {X86::RCL8m1, X86::RCL8r1, 0},
    {X86::RCL8mCL, X86::RCL8rCL, ENTRY_FLAGS_EXPAND},
    {X86::RCL8mi, X86::RCL8ri, 0},
    {X86::RCR16m1, X86::RCR16r1, 0},
    {X86::RCR16mCL, X86::RCR16rCL, ENTRY_FLAGS_EXPAND},
    {X86::RCR16mi, X86::RCR16ri, 0},
    {X86::RCR32m1, X86::RCR32r1, 0},
    {X86::RCR32mCL, X86::RCR32rCL, ENTRY_FLAGS_EXPAND},
    {X86::RCR32mi, X86::RCR32ri, 0},
    {X86::RCR64m1, X86::RCR64r1, 0},
    {X86::RCR64mCL, X86::RCR64rCL, ENTRY_FLAGS_EXPAND},
    {X86::RCR64mi, X86::RCR64ri, 0},
    {X86::RCR8m1, X86::RCR8r1, 0},
    {X86::RCR8mCL, X86::RCR8rCL, ENTRY_FLAGS_EXPAND},
    {X86::RCR8mi, X86::RCR8ri, 0},
    {X86::ROL16m1, X86::ROL16r1, 0},
    {X86::ROL16mCL, X86::ROL16rCL, ENTRY_FLAGS_EXPAND},
    {X86::ROL16mi, X86::ROL16ri, 0},
    {X86::ROL32m1, X86::ROL32r1, 0},
    {X86::ROL32mCL, X86::ROL32rCL, ENTRY_FLAGS_EXPAND},
    {X86::ROL32mi, X86::ROL32ri, 0},
    {X86::ROL64m1, X86::ROL64r1, 0},
    {X86::ROL64mCL, X86::ROL64rCL, ENTRY_FLAGS_EXPAND},
    {X86::ROL64mi, X86::ROL64ri, 0},
    {X86::ROL8m1, X86::ROL8r1, 0},
    {X86::ROL8mCL, X86::ROL8rCL, ENTRY_FLAGS_EXPAND},
    {X86::ROL8mi, X86::ROL8ri, 0},
    {X86::ROR16m1, X86::ROR16r1, 0},
    {X86::ROR16mCL, X86::ROR16rCL, ENTRY_FLAGS_EXPAND},
    {X86::ROR16mi, X86::ROR16ri, 0},
    {X86::ROR32m1, X86::ROR32r1, 0},
    {X86::ROR32mCL, X86::ROR32rCL, ENTRY_FLAGS_EXPAND},
    {X86::ROR32mi, X86::ROR32ri, 0},
    {X86::ROR64m1, X86::ROR64r1, 0},
    {X86::ROR64mCL, X86::ROR64rCL, ENTRY_FLAGS_EXPAND},
    {X86::ROR64mi, X86::ROR64ri, 0},
    {X86::ROR8m1, X86::ROR8r1, 0},
    {X86::ROR8mCL, X86::ROR8rCL, ENTRY_FLAGS_EXPAND},
    {X86::ROR8mi, X86::ROR8ri, 0},
    {X86::SAR16m1, X86::SAR16r1, 0},
    {X86::SAR16mCL, X86::SAR16rCL, ENTRY_FLAGS_EXPAND},
    {X86::SAR16mi, X86::SAR16ri, 0},
    {X86::SAR32m1, X86::SAR32r1, 0},
    {X86::SAR32mCL, X86::SAR32rCL, ENTRY_FLAGS_EXPAND},
    {X86::SAR32mi, X86::SAR32ri, 0},
    {X86::SAR64m1, X86::SAR64r1, 0},
    {X86::SAR64mCL, X86::SAR64rCL, ENTRY_FLAGS_EXPAND},
    {X86::SAR64mi, X86::SAR64ri, 0},
    {X86::SAR8m1, X86::SAR8r1, 0},
    {X86::SAR8mCL, X86::SAR8rCL, ENTRY_FLAGS_EXPAND},
    {X86::SAR8mi, X86::SAR8ri, 0},
    {X86::SBB16mi, X86::SBB16ri, 0},
    {X86::SBB16mr, X86::SBB16rr, ENTRY_FLAGS_EXPAND},
    {X86::SBB32mi, X86::SBB32ri, 0},
    {X86::SBB32mr, X86::SBB32rr, ENTRY_FLAGS_EXPAND},
    {X86::SBB64mi32, X86::SBB64ri32, 0},
    {X86::SBB64mr, X86::SBB64rr, ENTRY_FLAGS_EXPAND},
    {X86::SBB8mi, X86::SBB8ri, 0},
    {X86::SBB8mr, X86::SBB8rr, ENTRY_FLAGS_EXPAND},
    {X86::SHL16m1, X86::SHL16r1, 0},
    {X86::SHL16mCL, X86::SHL16rCL, ENTRY_FLAGS_EXPAND},
    {X86::SHL16mi, X86::SHL16ri, 0},
    {X86::SHL32m1, X86::SHL32r1, 0},
    {X86::SHL32mCL, X86::SHL32rCL, ENTRY_FLAGS_EXPAND},
    {X86::SHL32mi, X86::SHL32ri, 0},
    {X86::SHL64m1, X86::SHL64r1, 0},
    {X86::SHL64mCL, X86::SHL64rCL, ENTRY_FLAGS_EXPAND},
    {X86::SHL64mi, X86::SHL64ri, 0},
    {X86::SHL8m1, X86::SHL8r1, 0},
    {X86::SHL8mCL, X86::SHL8rCL, ENTRY_FLAGS_EXPAND},
    {X86::SHL8mi, X86::SHL8ri, 0},
    {X86::SHLD16mrCL, X86::SHLD16rrCL, ENTRY_FLAGS_EXPAND},
    {X86::SHLD16mri8, X86::SHLD16rri8, 0},
    {X86::SHLD32mrCL, X86::SHLD32rrCL, ENTRY_FLAGS_EXPAND},
    {X86::SHLD32mri8, X86::SHLD32rri8, 0},
    {X86::SHLD64mrCL, X86::SHLD64rrCL, ENTRY_FLAGS_EXPAND},
    {X86::SHLD64mri8, X86::SHLD64rri8, 0},
    {X86::SHR16m1, X86::SHR16r1, 0},
    {X86::SHR16mCL, X86::SHR16rCL, ENTRY_FLAGS_EXPAND},
    {X86::SHR16mi, X86::SHR16ri, 0},
    {X86::SHR32m1, X86::SHR32r1, 0},
    {X86::SHR32mCL, X86::SHR32rCL, ENTRY_FLAGS_EXPAND},
    {X86::SHR32mi, X86::SHR32ri, 0},
    {X86::SHR64m1, X86::SHR64r1, 0},
    {X86::SHR64mCL, X86::SHR64rCL, ENTRY_FLAGS_EXPAND},
    {X86::SHR64mi, X86::SHR64ri, 0},
    {X86::SHR8m1, X86::SHR8r1, 0},
    {X86::SHR8mCL, X86::SHR8rCL, ENTRY_FLAGS_EXPAND},
    {X86::SHR8mi, X86::SHR8ri, 0},
    {X86::SHRD16mrCL, X86::SHRD16rrCL, ENTRY_FLAGS_EXPAND},
    {X86::SHRD16mri8, X86::SHRD16rri8, 0},
    {X86::SHRD32mrCL, X86::SHRD32rrCL, ENTRY_FLAGS_EXPAND},
    {X86::SHRD32mri8, X86::SHRD32rri8, 0},
    {X86::SHRD64mrCL, X86::SHRD64rrCL, ENTRY_FLAGS_EXPAND},
    {X86::SHRD64mri8, X86::SHRD64rri8, 0},
    {X86::SUB16mi, X86::SUB16ri, 0},
    {X86::SUB16mr, X86::SUB16rr, ENTRY_FLAGS_EXPAND},
    {X86::SUB32mi, X86::SUB32ri, 0},
    {X86::SUB32mr, X86::SUB32rr, ENTRY_FLAGS_EXPAND},
    {X86::SUB64mi32, X86::SUB64ri32, 0},
    {X86::SUB64mr, X86::SUB64rr, ENTRY_FLAGS_EXPAND},
    {X86::SUB8mi, X86::SUB8ri, 0},
    {X86::SUB8mr, X86::SUB8rr, ENTRY_FLAGS_EXPAND},
    {X86::XOR16mi, X86::XOR16ri, 0},
    {X86::XOR16mr, X86::XOR16rr, ENTRY_FLAGS_EXPAND},
    {X86::XOR32mi, X86::XOR32ri, 0},
    {X86::XOR32mr, X86::XOR32rr, ENTRY_FLAGS_EXPAND},
    {X86::XOR64mi32, X86::XOR64ri32, 0},
    {X86::XOR64mr, X86::XOR64rr, ENTRY_FLAGS_EXPAND},
    {X86::XOR8mi, X86::XOR8ri, 0},
    {X86::XOR8mr, X86::XOR8rr, ENTRY_FLAGS_EXPAND},
};

static const X86MemoryExpandTableEntry StoreTable[] = {
    {X86::MOV16mi, X86::MOV16ri, 0},
    {X86::MOV16mr, X86::MOV16rr, 0},
    {X86::MOV32mi, X86::MOV32ri, 0},
    {X86::MOV32mr, X86::MOV32rr, 0},
    {X86::MOV64mi32, X86::MOV64ri32, 0},
    {X86::MOV64mr, X86::MOV64rr, 0},
    {X86::MOV8mi, X86::MOV8ri, 0},
    {X86::MOV8mr, X86::MOV8rr, 0},
    {X86::MOV8mr_NOREX, X86::MOV8rr_NOREX, 0},
    {X86::SETCCm, X86::SETCCr, 0},
};

static const X86MemoryExpandTableEntry LoadTable0[] = {
    {X86::BT16mi8, X86::BT16ri8, 0},
    {X86::BT32mi8, X86::BT32ri8, 0},
    {X86::BT64mi8, X86::BT64ri8, 0},
    {X86::CALL16m, X86::CALL16r, ENTRY_FLAGS_EXPAND},
    {X86::CALL16m_NT, X86::CALL16r_NT, ENTRY_FLAGS_EXPAND},
    {X86::CALL32m, X86::CALL32r, ENTRY_FLAGS_EXPAND},
    {X86::CALL32m_NT, X86::CALL32r_NT, ENTRY_FLAGS_EXPAND},
    {X86::CALL64m, X86::CALL64r, ENTRY_FLAGS_EXPAND},
    {X86::CALL64m_NT, X86::CALL64r_NT, ENTRY_FLAGS_EXPAND},
    {X86::CMP16mi, X86::CMP16ri, ENTRY_FLAGS_EXPAND},
    {X86::CMP16mr, X86::CMP16rr, ENTRY_FLAGS_EXPAND},
    {X86::CMP32mi, X86::CMP32ri, ENTRY_FLAGS_EXPAND},
    {X86::CMP32mr, X86::CMP32rr, ENTRY_FLAGS_EXPAND},
    {X86::CMP64mi32, X86::CMP64ri32, ENTRY_FLAGS_EXPAND},
    {X86::CMP64mr, X86::CMP64rr, ENTRY_FLAGS_EXPAND},
    {X86::CMP8mi, X86::CMP8ri, ENTRY_FLAGS_EXPAND},
    {X86::CMP8mr, X86::CMP8rr, ENTRY_FLAGS_EXPAND},
    {X86::DIV16m, X86::DIV16r, 0},
    {X86::DIV32m, X86::DIV32r, 0},
    {X86::DIV64m, X86::DIV64r, 0},
    {X86::DIV8m, X86::DIV8r, 0},
    {X86::IDIV16m, X86::IDIV16r, 0},
    {X86::IDIV32m, X86::IDIV32r, 0},
    {X86::IDIV64m, X86::IDIV64r, 0},
    {X86::IDIV8m, X86::IDIV8r, 0},
    {X86::IMUL16m, X86::IMUL16r, 0},
    {X86::IMUL32m, X86::IMUL32r, 0},
    {X86::IMUL64m, X86::IMUL64r, ENTRY_FLAGS_EXPAND},
    {X86::IMUL8m, X86::IMUL8r, 0},
    {X86::JMP16m, X86::JMP16r, ENTRY_FLAGS_EXPAND},
    {X86::JMP16m_NT, X86::JMP16r_NT, ENTRY_FLAGS_EXPAND},
    {X86::JMP32m, X86::JMP32r, ENTRY_FLAGS_EXPAND},
    {X86::JMP32m_NT, X86::JMP32r_NT, ENTRY_FLAGS_EXPAND},
    {X86::JMP64m, X86::JMP64r, ENTRY_FLAGS_EXPAND},
    {X86::JMP64m_NT, X86::JMP64r_NT, ENTRY_FLAGS_EXPAND},
    {X86::MUL16m, X86::MUL16r, 0},
    {X86::MUL32m, X86::MUL32r, 0},
    {X86::MUL64m, X86::MUL64r, ENTRY_FLAGS_EXPAND},
    {X86::MUL8m, X86::MUL8r, 0},
    {X86::PUSH16rmm, X86::PUSH16r, ENTRY_FLAGS_EXPAND},
    {X86::PUSH32rmm, X86::PUSH32r, ENTRY_FLAGS_EXPAND},
    {X86::PUSH64rmm, X86::PUSH64r, ENTRY_FLAGS_EXPAND},
    {X86::TAILJMPm, X86::TAILJMPr, ENTRY_FLAGS_EXPAND},
    {X86::TAILJMPm64, X86::TAILJMPr64, ENTRY_FLAGS_EXPAND},
    {X86::TAILJMPm64_REX, X86::TAILJMPr64_REX, ENTRY_FLAGS_EXPAND},
    {X86::TCRETURNmi, X86::TCRETURNri, ENTRY_FLAGS_EXPAND},
    {X86::TCRETURNmi64, X86::TCRETURNri64, ENTRY_FLAGS_EXPAND},
    {X86::TEST16mi, X86::TEST16ri, ENTRY_FLAGS_EXPAND},
    {X86::TEST16mr, X86::TEST16rr, ENTRY_FLAGS_EXPAND},
    {X86::TEST32mi, X86::TEST32ri, ENTRY_FLAGS_EXPAND},
    {X86::TEST32mr, X86::TEST32rr, ENTRY_FLAGS_EXPAND},
    {X86::TEST64mi32, X86::TEST64ri32, ENTRY_FLAGS_EXPAND},
    {X86::TEST64mr, X86::TEST64rr, ENTRY_FLAGS_EXPAND},
    {X86::TEST8mi, X86::TEST8ri, ENTRY_FLAGS_EXPAND},
    {X86::TEST8mr, X86::TEST8rr, ENTRY_FLAGS_EXPAND},
};

static const X86MemoryExpandTableEntry LoadTable1[] = {

    {X86::BEXTR32rm, X86::BEXTR32rr, 0},
    {X86::BEXTR64rm, X86::BEXTR64rr, 0},
    {X86::BEXTRI32mi, X86::BEXTRI32ri, 0},
    {X86::BEXTRI64mi, X86::BEXTRI64ri, 0},
    {X86::BLCFILL32rm, X86::BLCFILL32rr, 0},
    {X86::BLCFILL64rm, X86::BLCFILL64rr, 0},
    {X86::BLCI32rm, X86::BLCI32rr, 0},
    {X86::BLCI64rm, X86::BLCI64rr, 0},
    {X86::BLCIC32rm, X86::BLCIC32rr, 0},
    {X86::BLCIC64rm, X86::BLCIC64rr, 0},
    {X86::BLCMSK32rm, X86::BLCMSK32rr, 0},
    {X86::BLCMSK64rm, X86::BLCMSK64rr, 0},
    {X86::BLCS32rm, X86::BLCS32rr, 0},
    {X86::BLCS64rm, X86::BLCS64rr, 0},
    {X86::BLSFILL32rm, X86::BLSFILL32rr, 0},
    {X86::BLSFILL64rm, X86::BLSFILL64rr, 0},
    {X86::BLSI32rm, X86::BLSI32rr, 0},
    {X86::BLSI64rm, X86::BLSI64rr, 0},
    {X86::BLSIC32rm, X86::BLSIC32rr, 0},
    {X86::BLSIC64rm, X86::BLSIC64rr, 0},
    {X86::BLSMSK32rm, X86::BLSMSK32rr, 0},
    {X86::BLSMSK64rm, X86::BLSMSK64rr, 0},
    {X86::BLSR32rm, X86::BLSR32rr, 0},
    {X86::BLSR64rm, X86::BLSR64rr, 0},
    {X86::BSF16rm, X86::BSF16rr, 0},
    {X86::BSF32rm, X86::BSF32rr, 0},
    {X86::BSF64rm, X86::BSF64rr, 0},
    {X86::BSR16rm, X86::BSR16rr, 0},
    {X86::BSR32rm, X86::BSR32rr, 0},
    {X86::BSR64rm, X86::BSR64rr, 0},
    {X86::BZHI32rm, X86::BZHI32rr, 0},
    {X86::BZHI64rm, X86::BZHI64rr, 0},
    {X86::CMP16rm, X86::CMP16rr, ENTRY_FLAGS_EXPAND},
    {X86::CMP32rm, X86::CMP32rr, ENTRY_FLAGS_EXPAND},
    {X86::CMP64rm, X86::CMP64rr, ENTRY_FLAGS_EXPAND},
    {X86::CMP8rm, X86::CMP8rr, ENTRY_FLAGS_EXPAND},
    {X86::IMUL16rmi, X86::IMUL16rri, 0},
    {X86::IMUL16rmi8, X86::IMUL16rri8, 0},
    {X86::IMUL32rmi, X86::IMUL32rri, 0},
    {X86::IMUL32rmi8, X86::IMUL32rri8, 0},
    {X86::IMUL64rmi32, X86::IMUL64rri32, 0},
    {X86::IMUL64rmi8, X86::IMUL64rri8, 0},
    {X86::LZCNT16rm, X86::LZCNT16rr, 0},
    {X86::LZCNT32rm, X86::LZCNT32rr, 0},
    {X86::LZCNT64rm, X86::LZCNT64rr, 0},
    {X86::MOV16rm, X86::MOV16rr, 0},
    {X86::MOV32rm, X86::MOV32rr, 0},
    {X86::MOV64rm, X86::MOV64rr, 0},
    {X86::MOV8rm, X86::MOV8rr, 0},
    {X86::MOVSX16rm16, X86::MOVSX16rr16, ENTRY_FLAGS_EXPAND},
    {X86::MOVSX16rm32, X86::MOVSX16rr32, ENTRY_FLAGS_EXPAND},
    {X86::MOVSX16rm8, X86::MOVSX16rr8, ENTRY_FLAGS_EXPAND},
    {X86::MOVSX32rm16, X86::MOVSX32rr16, ENTRY_FLAGS_EXPAND},
    {X86::MOVSX32rm32, X86::MOVSX32rr32, ENTRY_FLAGS_EXPAND},
    {X86::MOVSX32rm8, X86::MOVSX32rr8, ENTRY_FLAGS_EXPAND},
    {X86::MOVSX32rm8_NOREX, X86::MOVSX32rr8_NOREX, ENTRY_FLAGS_EXPAND},
    {X86::MOVSX64rm16, X86::MOVSX64rr16, ENTRY_FLAGS_EXPAND},
    {X86::MOVSX64rm32, X86::MOVSX64rr32, ENTRY_FLAGS_EXPAND},
    {X86::MOVSX64rm8, X86::MOVSX64rr8, ENTRY_FLAGS_EXPAND},
    {X86::MOVZX16rm8, X86::MOVZX16rr8, 0},
    {X86::MOVZX32rm16, X86::MOVZX32rr16, 0},
    {X86::MOVZX32rm8, X86::MOVZX32rr8, 0},
    {X86::MOVZX32rm8_NOREX, X86::MOVZX32rr8_NOREX, 0},
    {X86::MOVZX64rm16, X86::MOVZX64rr16, 0},
    {X86::MOVZX64rm8, X86::MOVZX64rr8, 0},
    {X86::POPCNT16rm, X86::POPCNT16rr, 0},
    {X86::POPCNT32rm, X86::POPCNT32rr, 0},
    {X86::POPCNT64rm, X86::POPCNT64rr, 0},
    {X86::RORX32mi, X86::RORX32ri, 0},
    {X86::RORX64mi, X86::RORX64ri, 0},
    {X86::SARX32rm, X86::SARX32rr, 0},
    {X86::SARX64rm, X86::SARX64rr, 0},
    {X86::SHLX32rm, X86::SHLX32rr, 0},
    {X86::SHLX64rm, X86::SHLX64rr, 0},
    {X86::SHRX32rm, X86::SHRX32rr, 0},
    {X86::SHRX64rm, X86::SHRX64rr, 0},
};

static const X86MemoryExpandTableEntry LoadTable2[] = {
    {X86::ADC16rm, X86::ADC16rr, ENTRY_FLAGS_EXPAND},
    {X86::ADC32rm, X86::ADC32rr, ENTRY_FLAGS_EXPAND},
    {X86::ADC64rm, X86::ADC64rr, ENTRY_FLAGS_EXPAND},
    {X86::ADC8rm, X86::ADC8rr, ENTRY_FLAGS_EXPAND},
    {X86::ADCX32rm, X86::ADCX32rr, ENTRY_FLAGS_EXPAND},
    {X86::ADCX64rm, X86::ADCX64rr, ENTRY_FLAGS_EXPAND},
    {X86::ADD16rm, X86::ADD16rr, 0},
    {X86::ADD32rm, X86::ADD32rr, 0},
    {X86::ADD64rm, X86::ADD64rr, 0},
    {X86::ADD8rm, X86::ADD8rr, 0},
    {X86::ADOX32rm, X86::ADOX32rr, ENTRY_FLAGS_EXPAND},
    {X86::ADOX64rm, X86::ADOX64rr, ENTRY_FLAGS_EXPAND},
    {X86::AND16rm, X86::AND16rr, 0},
    {X86::AND32rm, X86::AND32rr, 0},
    {X86::AND64rm, X86::AND64rr, 0},
    {X86::AND8rm, X86::AND8rr, 0},
    {X86::ANDN32rm, X86::ANDN32rr, 0},
    {X86::ANDN64rm, X86::ANDN64rr, 0},
    {X86::CMOV16rm, X86::CMOV16rr, 0},
    {X86::CMOV32rm, X86::CMOV32rr, 0},
    {X86::CMOV64rm, X86::CMOV64rr, 0},
    {X86::CRC32r32m16, X86::CRC32r32r16, 0},
    {X86::CRC32r32m32, X86::CRC32r32r32, 0},
    {X86::CRC32r32m8, X86::CRC32r32r8, 0},
    {X86::CRC32r64m64, X86::CRC32r64r64, 0},
    {X86::CRC32r64m8, X86::CRC32r64r8, 0},
    {X86::IMUL16rm, X86::IMUL16rr, 0},
    {X86::IMUL32rm, X86::IMUL32rr, 0},
    {X86::IMUL64rm, X86::IMUL64rr, 0},
    {X86::MULX32rm, X86::MULX32rr, 0},
    {X86::MULX64rm, X86::MULX64rr, 0},
    {X86::OR16rm, X86::OR16rr, 0},
    {X86::OR32rm, X86::OR32rr, 0},
    {X86::OR64rm, X86::OR64rr, 0},
    {X86::OR8rm, X86::OR8rr, 0},
    {X86::SBB16rm, X86::SBB16rr, ENTRY_FLAGS_EXPAND},
    {X86::SBB32rm, X86::SBB32rr, ENTRY_FLAGS_EXPAND},
    {X86::SBB64rm, X86::SBB64rr, ENTRY_FLAGS_EXPAND},
    {X86::SBB8rm, X86::SBB8rr, ENTRY_FLAGS_EXPAND},
    {X86::SUB16rm, X86::SUB16rr, 0},
    {X86::SUB32rm, X86::SUB32rr, 0},
    {X86::SUB64rm, X86::SUB64rr, 0},
    {X86::SUB8rm, X86::SUB8rr, 0},
    {X86::XOR16rm, X86::XOR16rr, 0},
    {X86::XOR32rm, X86::XOR32rr, 0},
    {X86::XOR64rm, X86::XOR64rr, 0},
    {X86::XOR8rm, X86::XOR8rr, 0},
};

static const X86MemoryExpandTableEntry *
lookupFoldTableImpl(ArrayRef<X86MemoryExpandTableEntry> Table, unsigned RegOp) {
#ifndef NDEBUG
  // Make sure the tables are sorted.
  static std::atomic<bool> FoldTablesChecked(false);
  if (!FoldTablesChecked.load(std::memory_order_relaxed)) {
    assert(llvm::is_sorted(RMWTable) &&
           std::adjacent_find(std::begin(RMWTable), std::end(RMWTable)) ==
               std::end(RMWTable) &&
           "RMWTable is not sorted and unique!");
    assert(llvm::is_sorted(StoreTable) &&
           std::adjacent_find(std::begin(StoreTable), std::end(StoreTable)) ==
               std::end(StoreTable) &&
           "StoreTable is not sorted and unique!");
    assert(llvm::is_sorted(LoadTable0) &&
           std::adjacent_find(std::begin(LoadTable0), std::end(LoadTable0)) ==
               std::end(LoadTable0) &&
           "LoadTable0 is not sorted and unique!");
    assert(llvm::is_sorted(LoadTable1) &&
           std::adjacent_find(std::begin(LoadTable1), std::end(LoadTable1)) ==
               std::end(LoadTable1) &&
           "LoadTable1 is not sorted and unique!");
    assert(llvm::is_sorted(LoadTable2) &&
           std::adjacent_find(std::begin(LoadTable2), std::end(LoadTable2)) ==
               std::end(LoadTable2) &&
           "LoadTable2 is not sorted and unique!");

    FoldTablesChecked.store(true, std::memory_order_relaxed);
  }
#endif

  const X86MemoryExpandTableEntry *Data = llvm::lower_bound(Table, RegOp);
  if (Data != Table.end() && Data->KeyOp == RegOp)
    return Data;
  return nullptr;
}

static const X86MemoryExpandTableEntry *lookupLoadTable(unsigned RegOp) {
  if (auto Ret = lookupFoldTableImpl(ArrayRef(LoadTable0), RegOp))
    return Ret;

  if (auto Ret = lookupFoldTableImpl(ArrayRef(LoadTable1), RegOp))
    return Ret;

  if (auto Ret = lookupFoldTableImpl(ArrayRef(LoadTable2), RegOp))
    return Ret;

  return nullptr;
}

static const X86MemoryExpandTableEntry *lookupStoreTable(unsigned RegOp) {
  if (auto Ret = lookupFoldTableImpl(ArrayRef(StoreTable), RegOp))
    return Ret;

  return nullptr;
}

static const X86MemoryExpandTableEntry *lookupRMWTable(unsigned RegOp) {
  if (auto Ret = lookupFoldTableImpl(ArrayRef(RMWTable), RegOp))
    return Ret;

  return nullptr;
}

class X86AvoidMRNBPass : public MachineFunctionPass {
public:
  static char ID;
  X86AvoidMRNBPass() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override {
    return "X86 Avoid Memroy Renaming Blocks";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  MachineRegisterInfo *MRI = nullptr;
  const X86InstrInfo *TII = nullptr;
  const X86RegisterInfo *TRI = nullptr;
  const X86Subtarget *ST = nullptr;
  const MachineFunction *MF = nullptr;

  MachineBasicBlock::iterator It;

  void getForwardedLoads(MachineInstr &Store, DenseSet<MachineInstr *> &Loads);
  bool transform(MachineInstr &Store, DenseSet<MachineInstr *> &Loads);
  bool promoteIndexByLEA(MachineInstr &MI);
  bool promoteRIPByLEA(MachineInstr &MI);

  bool expandLoad(MachineInstr &MI);
  bool expandRMW(MachineInstr &MI);

  Register genLEA(MachineInstr &MI, bool SkipDisp);
};

} // end anonymous namespace

char X86AvoidMRNBPass::ID = 0;

INITIALIZE_PASS_BEGIN(X86AvoidMRNBPass, DEBUG_TYPE,
                      "Machine avoid memory renaming block", false, false)
INITIALIZE_PASS_END(X86AvoidMRNBPass, DEBUG_TYPE,
                    "Machine avoid memory renaming block", false, false)

FunctionPass *llvm::createX86AvoidMemoryRenamingBlocks() {
  return new X86AvoidMRNBPass();
}

/// Return true if MO is a virual register or NoRegister.
static bool isVirtualReg(const MachineOperand *MO) {
  if (!MO->isReg())
    return false;
  return Register::isVirtualRegister(MO->getReg());
}

static int getMemOpNo(const MachineInstr &MI) {
  const MCInstrDesc &Descl = MI.getDesc();
  int AddrOffset = X86II::getMemoryOperandNo(Descl.TSFlags);
  assert(AddrOffset != -1 && "Expected Memory Operand");
  AddrOffset += X86II::getOperandBias(Descl);
  return AddrOffset;
}

static MachineMemOperand *getMemoryOperand(MachineInstr &MI, bool isLoad) {
  MachineMemOperand *MMOs = nullptr;
  for (auto MMOIt = MI.memoperands_begin(), MMOEnd = MI.memoperands_end();
       MMOIt != MMOEnd; ++MMOIt) {
    if (isLoad && (*MMOIt)->isLoad()) {
      assert(!MMOs && "Multiple load MMO.");
      MMOs = *MMOIt;
    }

    if (!isLoad && (*MMOIt)->isStore()) {
      assert(!MMOs && "Multiple store MMO.");
      MMOs = *MMOIt;
    }
  }

  return MMOs;
}

static void getAddressingOperands(MachineInstr &MI,
                                  MachineOperand *Operands[5]) {
  int MemOpNo = getMemOpNo(MI);
  Operands[X86::AddrBaseReg] = &(MI.getOperand(MemOpNo + X86::AddrBaseReg));
  Operands[X86::AddrIndexReg] = &(MI.getOperand(MemOpNo + X86::AddrIndexReg));
  Operands[X86::AddrScaleAmt] = &(MI.getOperand(MemOpNo + X86::AddrScaleAmt));
  Operands[X86::AddrSegmentReg] =
      &(MI.getOperand(MemOpNo + X86::AddrSegmentReg));
  Operands[X86::AddrDisp] = &(MI.getOperand(MemOpNo + X86::AddrDisp));
}

// Find the load instructions which mostly have dependency with the store
// instruction.
void X86AvoidMRNBPass::getForwardedLoads(MachineInstr &Store,
                                         DenseSet<MachineInstr *> &Loads) {

  auto HasDependence = [&](MachineInstr &Store, MachineInstr &MI) {
    const MCInstrDesc &Desc = MI.getDesc();
    if (X86II::getMemoryOperandNo(Desc.TSFlags) < 0)
      return false;

    MachineOperand *StoreOperands[5];
    getAddressingOperands(Store, StoreOperands);

    MachineOperand *MIOperands[5];
    getAddressingOperands(MI, MIOperands);

    if (StoreOperands[0]->isIdenticalTo(*MIOperands[0]) &&
        StoreOperands[1]->isIdenticalTo(*MIOperands[1]) &&
        StoreOperands[2]->isIdenticalTo(*MIOperands[2]) &&
        StoreOperands[3]->isIdenticalTo(*MIOperands[3]) &&
        StoreOperands[4]->isIdenticalTo(*MIOperands[4]))
      return true;

    return false;
  };
  LLVM_DEBUG(dbgs() << "==============-----------==============\n";);
  LLVM_DEBUG(dbgs() << "Store:"; Store.dump());
  LLVM_DEBUG(dbgs() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>\n";);
  auto It = ++Store.getIterator();

  MachineBasicBlock *MBB = Store.getParent();
  const unsigned InspectionLimit = X86AvoidMRNBInspectionLimit;
  unsigned InstCount = 0;

  for (auto E = MBB->end(); It != E; ++It) {
    if (It->isMetaInstruction())
      continue;

    if (InstCount++ >= InspectionLimit)
      return;

    if (It->getDesc().isCall())
      return;

    if (!It->mayLoadOrStore())
      continue;

    if (It->mayLoad() && HasDependence(Store, *It)) {
      if (lookupLoadTable(It->getOpcode()) ||
        lookupRMWTable(It->getOpcode())) {
        LLVM_DEBUG(It->dump());
        Loads.insert(&(*It));
      } else
        return;
    }

    if (It->mayStore() && HasDependence(Store, *It))
      return;
  }

  if (InstCount < InspectionLimit) {
    int LimitLeft = InspectionLimit - InstCount;
    for (MachineBasicBlock::succ_iterator SuccB = MBB->succ_begin(),
                                          SuccE = MBB->succ_end();
         SuccB != SuccE; ++SuccB) {
      MachineBasicBlock *MBB = *SuccB;
      int SuccCount = 0;
      for (auto It = MBB->begin(), E = MBB->end(); It != E; ++It) {
        if (It->isMetaInstruction())
          continue;

        if (SuccCount++ >= LimitLeft)
          break;

        if (It->getDesc().isCall())
          break;

        if (!It->mayLoadOrStore())
          continue;

        if (It->mayLoad() && HasDependence(Store, *It)) {
          if (lookupLoadTable(It->getOpcode()) ||
              lookupRMWTable(It->getOpcode())) {
            LLVM_DEBUG(It->dump());
            Loads.insert(&(*It));
          } else
            break;
        }

        if (It->mayStore() && HasDependence(Store, *It))
          break;
      }
    }
  }
}

Register X86AvoidMRNBPass::genLEA(MachineInstr &MI, bool SkipDisp) {
  unsigned LEAOp = ST->is32Bit() ? X86::LEA32r : X86::LEA64r;

  const MCInstrDesc &Desc = MI.getDesc();
  unsigned MemOpNo = getMemOpNo(MI);

  const TargetRegisterClass *TRC =
      TII->getRegClass(Desc, MemOpNo + X86::AddrIndexReg, TRI, *MF);
  assert(TRC && "No TRC?");

  const DebugLoc DL = MI.getDebugLoc();
  const Register DestReg = MRI->createVirtualRegister(TRC);
  MachineBasicBlock *MBB = MI.getParent();

  // Insert LEA instruction with MI's base, scale, index, disp and seg.
  MachineInstrBuilder MIB = BuildMI(*MBB, MI, DL, TII->get(LEAOp), DestReg);
  for (unsigned i = X86::AddrBaseReg, e = X86::AddrNumOperands; i < e; ++i)
    MIB.add(MI.getOperand(MemOpNo + i));
  MachineInstr *Lea = MIB.getInstr();
  if (SkipDisp)
    Lea->getOperand(1 + X86::AddrDisp).ChangeToImmediate(0);

  LLVM_DEBUG(Lea->dump(););

  return DestReg;
}

static unsigned getLdOp(unsigned Size) {
  switch (Size) {
  case 8:
    return X86::MOV8rm;
  case 16:
    return X86::MOV16rm;
  case 32:
    return X86::MOV32rm;
  case 64:
    return X86::MOV64rm;
  default:
    llvm_unreachable("Unexpected bit width.");
    return X86::MOV64rm;
  }
}

static unsigned getStOp(unsigned Size) {
  switch (Size) {
  case 8:
    return X86::MOV8mr;
  case 16:
    return X86::MOV16mr;
  case 32:
    return X86::MOV32mr;
  case 64:
    return X86::MOV64mr;
  default:
    llvm_unreachable("Unexpected bit width.");
    return X86::MOV64mr;
  }
}

// Find a store which can be MRN-able potentially.
static bool isLegalSt(MachineInstr *MI) {
  auto *Entry = lookupStoreTable(MI->getOpcode());
  if (Entry)
    return true;

  Entry = lookupRMWTable(MI->getOpcode());
  if (Entry)
    return true;

  return false;
}

// Expand OPrm to load+OPrr.
bool X86AvoidMRNBPass::expandLoad(MachineInstr &MI) {

  auto Entry = lookupLoadTable(MI.getOpcode());
  if (Entry == nullptr)
    return false;

  if (!(Entry->Flags & ENTRY_FLAGS_EXPAND))
    return false;

  LLVM_DEBUG(dbgs() << "Expand load to:\n";);

  MachineBasicBlock &MBB = *MI.getParent();
  auto *LdMemOp = getMemoryOperand(MI, true);
  auto Dl = MI.getDebugLoc();

  unsigned MemOpNo = getMemOpNo(MI);

  const TargetRegisterClass *RC = TII->getRegClass(
      TII->get(Entry->DstOp), MemOpNo, TRI, *(MBB.getParent()));
  Register LdReg = MRI->createVirtualRegister(RC);

  unsigned LdOp = getLdOp(TRI->getSpillSize(*RC) * 8);

  MachineInstrBuilder MIB = BuildMI(MBB, MI, Dl, TII->get(LdOp), LdReg);
  for (unsigned i = X86::AddrBaseReg, e = X86::AddrNumOperands; i < e; ++i)
    MIB.add(MI.getOperand(MemOpNo + i));
  if (LdMemOp)
    MIB.addMemOperand(LdMemOp);

  LLVM_DEBUG(MIB->dump());

  MIB = BuildMI(MBB, MI, Dl, TII->get(Entry->DstOp));
  MachineInstr *NewMI = MIB.getInstr();
  for (unsigned i = 0, e = MI.getNumOperands(); i < e; ++i) {
    if (MI.getOperand(i).isReg() && MI.getOperand(i).isImplicit())
      continue;

    if (i >= MemOpNo && i < MemOpNo + X86::AddrNumOperands) {
      MIB.addReg(LdReg);
      i += X86::AddrNumOperands - 1;
      continue;
    }
    MIB.add(MI.getOperand(i));
  }
  NewMI->getOperand(MemOpNo).setIsKill(true);

  LLVM_DEBUG(NewMI->dump());
  if (It == MI.getIterator())
    It = ++MI.getIterator();
  MI.eraseFromParent();

  return true;
}

// Expand OPmr to load+OPrr+store.
bool X86AvoidMRNBPass::expandRMW(MachineInstr &MI) {

  const X86MemoryExpandTableEntry *Entry = lookupRMWTable(MI.getOpcode());
  if (Entry == nullptr)
    return false;

  if (!(Entry->Flags & ENTRY_FLAGS_EXPAND))
    return false;

  LLVM_DEBUG(dbgs() << "Expand RMW to:\n";);

  auto Dl = MI.getDebugLoc();
  MachineBasicBlock &MBB = *MI.getParent();

  auto *LdMemOp = getMemoryOperand(MI, true);
  auto *StMemOp = getMemoryOperand(MI, false);

  unsigned MemOpNo = getMemOpNo(MI);

  const TargetRegisterClass *RC =
      TII->getRegClass(TII->get(Entry->DstOp), 0, TRI, *(MBB.getParent()));
  Register LdReg = MRI->createVirtualRegister(RC);
  Register NewReg = MRI->createVirtualRegister(RC);
  unsigned LdOp = getLdOp(TRI->getSpillSize(*RC) * 8);
  unsigned StOp = getStOp(TRI->getSpillSize(*RC) * 8);

  MachineInstrBuilder MIB = BuildMI(MBB, MI, Dl, TII->get(LdOp), LdReg);
  MachineInstr *Load = MIB.getInstr();
  for (unsigned i = X86::AddrBaseReg, e = X86::AddrNumOperands; i < e; ++i) {
    MIB.add(MI.getOperand(MemOpNo + i));
    if (Load->getOperand(i + 1).isReg())
      Load->getOperand(i + 1).setIsKill(false);
  }
  if (LdMemOp)
    MIB.addMemOperand(LdMemOp);

  MIB = BuildMI(MBB, MI, Dl, TII->get(Entry->DstOp), NewReg);
  MachineInstr *NewMI = MIB.getInstr();
  for (unsigned i = 0, e = MI.getNumOperands(); i < e; ++i) {
    if (MI.getOperand(i).isReg() && MI.getOperand(i).isImplicit())
      continue;
    if (i >= MemOpNo && i < MemOpNo + X86::AddrNumOperands) {
      MIB.addReg(LdReg);
      i += X86::AddrNumOperands - 1;
      continue;
    }
    MIB.add(MI.getOperand(i));
  }
  NewMI->getOperand(MemOpNo + 1).setIsKill(true);

  MIB = BuildMI(MBB, MI, Dl, TII->get(StOp));
  MachineInstr *Store = MIB.getInstr();
  for (unsigned i = X86::AddrBaseReg, e = X86::AddrNumOperands; i < e; ++i)
    MIB.add(MI.getOperand(MemOpNo + i));
  if (StMemOp)
    MIB.addReg(NewReg).addMemOperand(StMemOp);
  Store->getOperand(5).setIsKill(true);

  LLVM_DEBUG(Load->dump(); NewMI->dump(); Store->dump());

  if (It == MI.getIterator())
    It = ++MI.getIterator();

  MI.eraseFromParent();

  return true;
}

// Transform op rax, imm(base, index, scale)
// ==>
// LEA Dst, (base, index, scale)
// op rax, Imm(Dst)
bool X86AvoidMRNBPass::promoteIndexByLEA(MachineInstr &MI) {
  int MemOpNo = getMemOpNo(MI);
  auto &IndexOperand = MI.getOperand(MemOpNo + X86::AddrIndexReg);

  if (isVirtualReg(&IndexOperand)) {
    LLVM_DEBUG(dbgs() << "Promote index addressing to:\n";);
    Register NewAddress = genLEA(MI, true);
    if (MI.getOperand(MemOpNo + X86::AddrBaseReg).isReg())
      MI.getOperand(MemOpNo + X86::AddrBaseReg).setReg(NewAddress);
    else
      MI.getOperand(MemOpNo + X86::AddrBaseReg)
          .ChangeToRegister(NewAddress, false);
    MI.getOperand(MemOpNo + X86::AddrBaseReg).setIsKill();
    MI.getOperand(MemOpNo + X86::AddrScaleAmt).ChangeToImmediate(1);
    MI.getOperand(MemOpNo + X86::AddrIndexReg).setReg(X86::NoRegister);
    LLVM_DEBUG(MI.dump(););
    return true;
  }
  return false;
}

// Transform op rax, imm(rip)
// ==>
// LEA Dst, imm(rip)
// op rax, (Dst)
bool X86AvoidMRNBPass::promoteRIPByLEA(MachineInstr &MI) {
  int MemOpNo = getMemOpNo(MI);
  auto &BaseOperand = MI.getOperand(MemOpNo + X86::AddrBaseReg);

  if (BaseOperand.isReg() &&
      (BaseOperand.getReg() == X86::EIP || BaseOperand.getReg() == X86::RIP)) {
    LLVM_DEBUG(dbgs() << "Promote RIP addressing to:\n";);
    Register NewAddress = genLEA(MI, false);
    if (MI.getOperand(MemOpNo + X86::AddrBaseReg).isReg())
      MI.getOperand(MemOpNo + X86::AddrBaseReg).setReg(NewAddress);
    else
      MI.getOperand(MemOpNo + X86::AddrBaseReg)
          .ChangeToRegister(NewAddress, false);
    MI.getOperand(MemOpNo + X86::AddrBaseReg).setIsKill();
    MI.getOperand(MemOpNo + X86::AddrScaleAmt).ChangeToImmediate(1);
    MI.getOperand(MemOpNo + X86::AddrIndexReg).setReg(X86::NoRegister);
    MI.getOperand(MemOpNo + X86::AddrDisp).ChangeToImmediate(0);
    LLVM_DEBUG(MI.dump(););
    return true;
  }
  return false;
}

// Check each store and load, check if they should be fixed to MRN-able.
bool X86AvoidMRNBPass::transform(MachineInstr &Store,
                                 DenseSet<MachineInstr *> &Loads) {

  bool Changed = false;

  LLVM_DEBUG(dbgs() << "==============-----Transform-----==============\n";);
  LLVM_DEBUG(dbgs() << "Store: "; Store.dump(););
  // If the store is also in the Loads, remove the store from Loads,
  // In case, we process the same instruction twice.
  bool SameWithStore = false;
  if (Loads.contains(&Store)) {
    LLVM_DEBUG(dbgs() << "Same with load.\n";);
    Loads.erase(&Store);
    SameWithStore = true;
  }

  Changed |= promoteIndexByLEA(Store);
  Changed |= promoteRIPByLEA(Store);

  if (SameWithStore) {
    Changed |= expandRMW(Store);
    Changed |= expandLoad(Store);
  }

  for (auto &Load : Loads) {
    LLVM_DEBUG(dbgs() << "Load: "; Load->dump(););
    Changed |= promoteIndexByLEA(*Load);
    Changed |= promoteRIPByLEA(*Load);
    Changed |= expandRMW(*Load);
    Changed |= expandLoad(*Load);
  }

  return Changed;
}

bool X86AvoidMRNBPass::runOnMachineFunction(MachineFunction &MF) {
  bool Changed = false;

  if (DisableX86AvoidMemoryRenamingBlocks || skipFunction(MF.getFunction()))
    return false;

  MRI = &MF.getRegInfo();
  assert(MRI->isSSA() && "Expected MIR to be in SSA form");
  TII = MF.getSubtarget<X86Subtarget>().getInstrInfo();
  TRI = MF.getSubtarget<X86Subtarget>().getRegisterInfo();
  ST = &MF.getSubtarget<X86Subtarget>();
  this->MF = &MF;

  if (!ST->hasMRN())
    return false;

  if (!(ST->is32Bit() || ST->isTarget64BitLP64()))
    return false;

  LLVM_DEBUG(dbgs() << "Start X86AvoidMemoryRenamingBlocks\n";);
  for (auto &MBB : MF) {
    for (It = MBB.begin(); It != MBB.end();) {
      MachineInstr &MI = *(It++);
      if (MI.isMetaInstruction())
        continue;

      // Check Assembly?
      if (!MI.mayStore())
        continue;

      const MCInstrDesc &Desc = MI.getDesc();
      if (X86II::getMemoryOperandNo(Desc.TSFlags) < 0)
        continue;

      DenseSet<MachineInstr *> Loads;
      if (isLegalSt(&MI))
        getForwardedLoads(MI, Loads);

      if (Loads.size())
        Changed |= transform(MI, Loads);
    }
  }
  LLVM_DEBUG(dbgs() << "End X86AvoidMemoryRenamingBlocks\n";);

  return Changed;
}


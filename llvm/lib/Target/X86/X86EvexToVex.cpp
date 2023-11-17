//===- X86EvexToVex.cpp ---------------------------------------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Compress EVEX instructions to VEX encoding when possible to reduce code size
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file defines the pass that goes over all AVX-512 instructions which
/// are encoded using the EVEX prefix and if possible replaces them by their
/// corresponding VEX encoding which is usually shorter by 2 bytes.
/// EVEX instructions may be encoded via the VEX prefix when the AVX-512
/// instruction has a corresponding AVX/AVX2 opcode, when vector length 
/// accessed by instruction is less than 512 bits and when it does not use 
//  the xmm or the mask registers or xmm/ymm registers with indexes higher than 15.
/// The pass applies code reduction on the generated code for AVX-512 instrs.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/X86BaseInfo.h"
#include "MCTargetDesc/X86InstComments.h"
#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/Pass.h"
#include <atomic>
#include <cassert>
#include <cstdint>

using namespace llvm;

// Including the generated EVEX2VEX tables.
struct X86EvexToVexCompressTableEntry {
  uint16_t EvexOpcode;
  uint16_t VexOpcode;

  bool operator<(const X86EvexToVexCompressTableEntry &RHS) const {
    return EvexOpcode < RHS.EvexOpcode;
  }

  friend bool operator<(const X86EvexToVexCompressTableEntry &TE,
                        unsigned Opc) {
    return TE.EvexOpcode < Opc;
  }
};
#include "X86GenEVEX2VEXTables.inc"

#define EVEX2VEX_DESC "Compressing EVEX instrs to VEX encoding when possible"
#define EVEX2VEX_NAME "x86-evex-to-vex-compress"

#define DEBUG_TYPE EVEX2VEX_NAME

namespace {

class EvexToVexInstPass : public MachineFunctionPass {

  /// For EVEX instructions that can be encoded using VEX encoding, replace
  /// them by the VEX encoding in order to reduce size.
  bool CompressEvexToVexImpl(MachineInstr &MI) const;

public:
  static char ID;

  EvexToVexInstPass() : MachineFunctionPass(ID) { }

  StringRef getPassName() const override { return EVEX2VEX_DESC; }

  /// Loop over all of the basic blocks, replacing EVEX instructions
  /// by equivalent VEX instructions when possible for reducing code size.
  bool runOnMachineFunction(MachineFunction &MF) override;

  // This pass runs after regalloc and doesn't support VReg operands.
  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().set(
        MachineFunctionProperties::Property::NoVRegs);
  }

private:
  /// Machine instruction info used throughout the class.
  const X86InstrInfo *TII = nullptr;
  const X86Subtarget *ST = nullptr;
};

} // end anonymous namespace

char EvexToVexInstPass::ID = 0;

bool EvexToVexInstPass::runOnMachineFunction(MachineFunction &MF) {
  TII = MF.getSubtarget<X86Subtarget>().getInstrInfo();

  ST = &MF.getSubtarget<X86Subtarget>();
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX256P
  if (!ST->hasAVX3() && !ST->hasNDD() && !ST->hasEGPR())
#else // INTEL_FEATURE_ISA_AVX256P
  if (!ST->hasAVX512() && !ST->hasNDD() && !ST->hasEGPR())
#endif // INTEL_FEATURE_ISA_AVX256P
#endif // INTEL_CUSTOMIZATION
    return false;

  bool Changed = false;

  /// Go over all basic blocks in function and replace
  /// EVEX encoded instrs by VEX encoding when possible.
  for (MachineBasicBlock &MBB : MF) {

    // Traverse the basic block.
    for (MachineInstr &MI : MBB)
      Changed |= CompressEvexToVexImpl(MI);
  }

  return Changed;
}

static bool usesExtendedRegister(const MachineInstr &MI) {
  auto isHiRegIdx = [](unsigned Reg) {
    // Check for XMM register with indexes between 16 - 31.
    if (Reg >= X86::XMM16 && Reg <= X86::XMM31)
      return true;

    // Check for YMM register with indexes between 16 - 31.
    if (Reg >= X86::YMM16 && Reg <= X86::YMM31)
      return true;

#if INTEL_CUSTOMIZATION
    // Check for GPR with indexes between 16 - 31.
    if (X86II::isApxExtendedReg(Reg))
      return true;
#endif // INTEL_CUSTOMIZATION

    return false;
  };

  // Check that operands are not ZMM regs or
  // XMM/YMM regs with hi indexes between 16 - 31.
  for (const MachineOperand &MO : MI.explicit_operands()) {
    if (!MO.isReg())
      continue;

    Register Reg = MO.getReg();

    assert(!(Reg >= X86::ZMM0 && Reg <= X86::ZMM31) &&
           "ZMM instructions should not be in the EVEX->VEX tables");

    if (isHiRegIdx(Reg))
      return true;
  }

  return false;
}

// Do any custom cleanup needed to finalize the conversion.
static bool performCustomAdjustments(MachineInstr &MI, unsigned NewOpc,
                                     const X86Subtarget *ST) {
  (void)NewOpc;
  unsigned Opc = MI.getOpcode();
  switch (Opc) {
  case X86::VALIGNDZ128rri:
  case X86::VALIGNDZ128rmi:
  case X86::VALIGNQZ128rri:
  case X86::VALIGNQZ128rmi: {
    assert((NewOpc == X86::VPALIGNRrri || NewOpc == X86::VPALIGNRrmi) &&
           "Unexpected new opcode!");
    unsigned Scale = (Opc == X86::VALIGNQZ128rri ||
                      Opc == X86::VALIGNQZ128rmi) ? 8 : 4;
    MachineOperand &Imm = MI.getOperand(MI.getNumExplicitOperands()-1);
    Imm.setImm(Imm.getImm() * Scale);
    break;
  }
  case X86::VSHUFF32X4Z256rmi:
  case X86::VSHUFF32X4Z256rri:
  case X86::VSHUFF64X2Z256rmi:
  case X86::VSHUFF64X2Z256rri:
  case X86::VSHUFI32X4Z256rmi:
  case X86::VSHUFI32X4Z256rri:
  case X86::VSHUFI64X2Z256rmi:
  case X86::VSHUFI64X2Z256rri: {
    assert((NewOpc == X86::VPERM2F128rr || NewOpc == X86::VPERM2I128rr ||
            NewOpc == X86::VPERM2F128rm || NewOpc == X86::VPERM2I128rm) &&
           "Unexpected new opcode!");
    MachineOperand &Imm = MI.getOperand(MI.getNumExplicitOperands()-1);
    int64_t ImmVal = Imm.getImm();
    // Set bit 5, move bit 1 to bit 4, copy bit 0.
    Imm.setImm(0x20 | ((ImmVal & 2) << 3) | (ImmVal & 1));
    break;
  }
  case X86::VRNDSCALEPDZ128rri:
  case X86::VRNDSCALEPDZ128rmi:
  case X86::VRNDSCALEPSZ128rri:
  case X86::VRNDSCALEPSZ128rmi:
  case X86::VRNDSCALEPDZ256rri:
  case X86::VRNDSCALEPDZ256rmi:
  case X86::VRNDSCALEPSZ256rri:
  case X86::VRNDSCALEPSZ256rmi:
  case X86::VRNDSCALESDZr:
  case X86::VRNDSCALESDZm:
  case X86::VRNDSCALESSZr:
  case X86::VRNDSCALESSZm:
  case X86::VRNDSCALESDZr_Int:
  case X86::VRNDSCALESDZm_Int:
  case X86::VRNDSCALESSZr_Int:
  case X86::VRNDSCALESSZm_Int:
    const MachineOperand &Imm = MI.getOperand(MI.getNumExplicitOperands()-1);
    int64_t ImmVal = Imm.getImm();
    // Ensure that only bits 3:0 of the immediate are used.
    if ((ImmVal & 0xf) != ImmVal)
      return false;
    break;
  }

  return true;
}


// For EVEX instructions that can be encoded using VEX encoding
// replace them by the VEX encoding in order to reduce size.
bool EvexToVexInstPass::CompressEvexToVexImpl(MachineInstr &MI) const {
  // VEX format.
  // # of bytes: 0,2,3  1      1      0,1   0,1,2,4  0,1
  //  [Prefixes] [VEX]  OPCODE ModR/M [SIB] [DISP]  [IMM]
  //
  // EVEX format.
  //  # of bytes: 4    1      1      1      4       / 1         1
  //  [Prefixes]  EVEX Opcode ModR/M [SIB] [Disp32] / [Disp8*N] [Immediate]

  const MCInstrDesc &Desc = MI.getDesc();

  // Check for EVEX instructions only.
  if ((Desc.TSFlags & X86II::EncodingMask) != X86II::EVEX)
    return false;

#if INTEL_CUSTOMIZATION
  if ((Desc.TSFlags & X86II::OpMapMask) != X86II::T_MAP4) {
#endif // INTEL_CUSTOMIZATION
  // Check for EVEX instructions with mask or broadcast as in these cases
  // the EVEX prefix is needed in order to carry this information
  // thus preventing the transformation to VEX encoding.
  if (Desc.TSFlags & (X86II::EVEX_K | X86II::EVEX_B))
    return false;

  // Check for EVEX instructions with L2 set. These instructions are 512-bits
  // and can't be converted to VEX.
  if (Desc.TSFlags & X86II::EVEX_L2)
    return false;
#if INTEL_CUSTOMIZATION
  }
#endif // INTEL_CUSTOMIZATION

#ifndef NDEBUG
  // Make sure the tables are sorted.
  static std::atomic<bool> TableChecked(false);
  if (!TableChecked.load(std::memory_order_relaxed)) {
    assert(llvm::is_sorted(X86EvexToVex128CompressTable) &&
           "X86EvexToVex128CompressTable is not sorted!");
    assert(llvm::is_sorted(X86EvexToVex256CompressTable) &&
           "X86EvexToVex256CompressTable is not sorted!");
#if INTEL_CUSTOMIZATION
    assert(llvm::is_sorted(ND2NonNDCompressTable) &&
           "ND2NonNDCompressTable is not sorted!");
    assert(llvm::is_sorted(EVEX2LegacyCompressTable) &&
           "EVEX2LegacyCompressTable is not sorted!");
#endif // INTEL_CUSTOMIZATION
    TableChecked.store(true, std::memory_order_relaxed);
  }
#endif

#if INTEL_CUSTOMIZATION
  // TODO: Update the name of the class when upstream
  ArrayRef<X86EvexToVexCompressTableEntry> Table;
  if ((Desc.TSFlags & X86II::OpMapMask) == X86II::T_MAP4 &&
      !(Desc.TSFlags & X86II::EVEX_NF)) {
    if (Desc.TSFlags & X86II::EVEX_B)
      Table = ArrayRef(ND2NonNDCompressTable);
    else
      Table = ArrayRef(EVEX2LegacyCompressTable);
  } else {
    Table = (Desc.TSFlags & X86II::VEX_L)
                ? ArrayRef(X86EvexToVex256CompressTable)
                : ArrayRef(X86EvexToVex128CompressTable);
  }
#endif // INTEL_CUSTOMIZATION

  const auto *I = llvm::lower_bound(Table, MI.getOpcode());
  if (I == Table.end() || I->EvexOpcode != MI.getOpcode())
    return false;

  unsigned NewOpc = I->VexOpcode;

#if INTEL_CUSTOMIZATION
  if ((Desc.TSFlags & X86II::OpMapMask) == X86II::T_MAP4 &&
      (Desc.TSFlags & X86II::EVEX_B)) {
    const MachineOperand &Dst = MI.getOperand(0);
    const MachineOperand &Src = MI.getOperand(1);
    assert(Dst.isReg() && Src.isReg() && "Unexpected MachineInstr!");

    if (Dst.getReg() != Src.getReg()) {
      bool IsCommutable = Desc.isCommutable();
      unsigned NumOperands = Desc.getNumOperands();
      if (!IsCommutable || NumOperands < 3)
        return false;
      const MachineOperand &Src2 = MI.getOperand(2);
      if (!Src2.isReg())
        return false;
      if (Dst.getReg() != Src2.getReg())
        return false;
      TII->commuteInstruction(MI, false, 1, 2);
      // After commnution, the opcode may change
      I = llvm::lower_bound(Table, MI.getOpcode());
      if (I == Table.end() || I->EvexOpcode != MI.getOpcode())
        return false;
      NewOpc = I->VexOpcode;
    }
    assert(MI.getOperand(0).getReg() == MI.getOperand(1).getReg());

    MI.setDesc(TII->get(NewOpc));
    MI.setAsmPrinterFlag(X86::AC_ND_2_NONND);
    MI.tieOperands(0, 1);
    return true;
  } else if ((Desc.TSFlags & X86II::OpMapMask) == X86II::T_MAP4 &&
             !(Desc.TSFlags & X86II::EVEX_B) &&
             !(Desc.TSFlags & X86II::EVEX_NF)) {
    for (unsigned Index = 0, Size = MI.getNumOperands(); Index < Size;
         Index++) {
      const MachineOperand &Op = MI.getOperand(Index);
      if (Op.isReg() && X86II::isApxExtendedReg(Op.getReg()))
        return false;
    }
    MI.setDesc(TII->get(NewOpc));
    MI.setAsmPrinterFlag(X86::AC_EVEX_2_LEGACY);
    return true;
  }
#endif // INTEL_CUSTOMIZATION
  if (usesExtendedRegister(MI))
    return false;

  if (!CheckVEXInstPredicate(MI, ST))
    return false;

  if (!performCustomAdjustments(MI, NewOpc, ST))
    return false;

  MI.setDesc(TII->get(NewOpc));
  MI.setAsmPrinterFlag(X86::AC_EVEX_2_VEX);
  return true;
}

INITIALIZE_PASS(EvexToVexInstPass, EVEX2VEX_NAME, EVEX2VEX_DESC, false, false)

FunctionPass *llvm::createX86EvexToVexInsts() {
  return new EvexToVexInstPass();
}

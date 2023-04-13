//===-- X86TileConfig.cpp - Tile Register Configure----------------------===//
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
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file Pass to config the shape of AMX physical registers
/// AMX register need to be configured before use. In X86PreTileConfig pass
/// the pldtilecfg instruction is inserted, however at that time we don't
/// know the shape of each physical tile registers, because the register
/// allocation is not done yet. This pass runs after egister allocation
/// pass. It collects the shape information of each physical tile register
/// and store the shape in the stack slot that is allocated for load config
/// to tile config register.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86MachineFunctionInfo.h"
#include "X86RegisterInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TileShapeInfo.h"
#include "llvm/CodeGen/VirtRegMap.h"
#include "llvm/InitializePasses.h"

using namespace llvm;

#define DEBUG_TYPE "tileconfig"

namespace {

struct X86TileConfig : public MachineFunctionPass {

  X86TileConfig() : MachineFunctionPass(ID) {}

  /// Return the pass name.
  StringRef getPassName() const override { return "Tile Register Configure"; }

  /// X86TileConfig analysis usage.
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<VirtRegMap>();
    AU.addRequired<LiveIntervals>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  /// Perform register allocation.
  bool runOnMachineFunction(MachineFunction &mf) override;

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().set(
        MachineFunctionProperties::Property::NoPHIs);
  }

  static char ID;
};

} // end anonymous namespace

char X86TileConfig::ID = 0;

INITIALIZE_PASS_BEGIN(X86TileConfig, DEBUG_TYPE, "Tile Register Configure",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(VirtRegMap)
INITIALIZE_PASS_END(X86TileConfig, DEBUG_TYPE, "Tile Register Configure", false,
                    false)

#if INTEL_CUSTOMIZATION
unsigned getAMXRegNum(MachineRegisterInfo *MRI, Register Reg) {
  if (Reg.isVirtual()) {
    unsigned RegClassID = MRI->getRegClass(Reg)->getID();
    if (RegClassID == X86::TILERegClassID)
      return 1;
#if INTEL_FEATURE_ISA_AMX_TRANSPOSE
    if (RegClassID == X86::TILEPAIRRegClassID)
      return 2;
#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE
  } else {
    if (Reg >= X86::TMM0 && Reg <= X86::TMM7)
      return 1;
#if INTEL_FEATURE_ISA_AMX_TRANSPOSE
    if (Reg >= X86::TMM0_TMM1 && Reg <= X86::TMM6_TMM7)
      return 2;
#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE
  }
  return 0;
}

static bool isAMXRegClass(MachineRegisterInfo *MRI, Register Reg) {
  return getAMXRegNum(MRI, Reg) > 0;
}

static void collectVirtRegShapes(MachineRegisterInfo *MRI,
                                 VirtRegMap &VRM,
                                 Register VirtReg,
                                 SmallVector<ShapeT, 8> &Phys2Shapes) {
  unsigned Num = getAMXRegNum(MRI, VirtReg);

  if (Num == 1) {
    unsigned Index = VRM.getPhys(VirtReg) - X86::TMM0;
    if (!Phys2Shapes[Index].isValid()) {
      ShapeT Shape = VRM.getShape(VirtReg);
      Phys2Shapes[Index] = Shape;
      return;
    }
  }
#if INTEL_FEATURE_ISA_AMX_TRANSPOSE
  // Split tile pair shape info to 2 single tile shape info. e.g:
  // Put TMM0_TMM1's Shape to TMM0's shape + TMM1's Shape in Phys2Shapes.
  if (Num == 2) {
    unsigned Index0 = (VRM.getPhys(VirtReg) - X86::TMM0_TMM1) * 2;
    unsigned Index1 = (VRM.getPhys(VirtReg) - X86::TMM0_TMM1) * 2 + 1;

    ShapeT Shape = VRM.getShape(VirtReg);
    assert(Shape.getShapeNum() == 2 && "Unexpected shape number!");

    if (!Phys2Shapes[Index0].isValid()) {
      ShapeT Shape0(Shape.getRow(0), Shape.getCol(0), MRI);
      Phys2Shapes[Index0] = Shape0;
    }

    if (!Phys2Shapes[Index1].isValid()) {
      ShapeT Shape1(Shape.getRow(1), Shape.getCol(1), MRI);
      Phys2Shapes[Index1] = Shape1;
    }
  }
#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE
}
#endif // INTEL_CUSTOMIZATION

bool X86TileConfig::runOnMachineFunction(MachineFunction &MF) {
  const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
  const TargetRegisterInfo *TRI = ST.getRegisterInfo();
  const TargetInstrInfo *TII = ST.getInstrInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  LiveIntervals &LIS = getAnalysis<LiveIntervals>();
  VirtRegMap &VRM = getAnalysis<VirtRegMap>();

  if (VRM.isShapeMapEmpty())
    return false;

  int SS = INT_MAX;
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      if (MI.getOpcode() == X86::PLDTILECFGV) {
        SS = MI.getOperand(0).getIndex();
        break;
      }
    }
    if (SS != INT_MAX)
      break;
  }
  // Didn't find PLDTILECFGV, just return false;
  if (SS == INT_MAX)
    return false;

  // Try to find a point to insert MIs for constant shapes.
  // Here we are leveraging the palette id inserted in PreRA pass.
  unsigned ConstPos = 0;
  MachineInstr *ConstMI = nullptr;
  for (MachineInstr &MI : MF.front()) {
    if (MI.getOpcode() == X86::MOV8mi && SS == MI.getOperand(0).getIndex()) {
      ConstMI = &MI;
      break;
    }
    ++ConstPos;
  }
  assert(ConstMI && "Cannot find an insertion point");

  unsigned AMXRegNum = TRI->getRegClass(X86::TILERegClassID)->getNumRegs();
  SmallVector<ShapeT, 8> Phys2Shapes(AMXRegNum, ShapeT()); // INTEL
  for (unsigned I = 0, E = MRI.getNumVirtRegs(); I != E; ++I) {
    Register VirtReg = Register::index2VirtReg(I);
    if (MRI.reg_nodbg_empty(VirtReg))
      continue;
    if (!isAMXRegClass(&MRI, VirtReg)) // INTEL
      continue;
    if (VRM.getPhys(VirtReg) == VirtRegMap::NO_PHYS_REG)
      continue;
    collectVirtRegShapes(&MRI, VRM, VirtReg, Phys2Shapes); // INTEL
  }

  // Fill in the shape of each tile physical register.
  for (unsigned I = 0; I < AMXRegNum; ++I) {
    ShapeT Shape = Phys2Shapes[I]; // INTEL
    if (!Shape.isValid()) // INTEL
      continue;
    DebugLoc DL;
    bool IsRow = true;
    MachineInstr *NewMI = nullptr;
    for (auto &R : {Shape.getRow()->getReg(), Shape.getCol()->getReg()}) {
      // Here is the data format for the tile config.
      // 0      palette
      // 1      start_row
      // 2-15   reserved, must be zero
      // 16-17  tile0.colsb Tile 0 bytes per row.
      // 18-19  tile1.colsb Tile 1 bytes per row.
      // 20-21  tile2.colsb Tile 2 bytes per row.
      // ... (sequence continues)
      // 30-31  tile7.colsb Tile 7 bytes per row.
      // 32-47  reserved, must be zero
      // 48     tile0.rows Tile 0 rows.
      // 49     tile1.rows Tile 1 rows.
      // 50     tile2.rows Tile 2 rows.
      // ... (sequence continues)
      // 55     tile7.rows Tile 7 rows.
      // 56-63  reserved, must be zero
      int64_t Imm = INT64_MAX;
      int Offset = IsRow ? 48 + I : 16 + I * 2;
      for (auto &DefMI : MRI.def_instructions(R)) {
        MachineBasicBlock &MBB = *DefMI.getParent();
        if (DefMI.isMoveImmediate()) {
          if (Imm != INT64_MAX) {
            // FIXME: We should handle this case in future.
            assert(Imm == DefMI.getOperand(1).getImm() &&
                   "Cannot initialize with different shapes");
            continue;
          }
          Imm = DefMI.getOperand(1).getImm();
          NewMI = addFrameReference(
                      BuildMI(MF.front(), ++ConstMI->getIterator(), DL,
                              TII->get(IsRow ? X86::MOV8mi : X86::MOV16mi)),
                      SS, Offset)
                      .addImm(Imm);
          ConstMI = NewMI;
          LIS.InsertMachineInstrInMaps(*NewMI);
        } else {
          unsigned SubIdx = IsRow ? X86::sub_8bit : X86::sub_16bit;
          unsigned RegSize = TRI->getRegSizeInBits(*MRI.getRegClass(R));
          if ((IsRow && RegSize == 8) || (!IsRow && RegSize == 16))
            SubIdx = 0;
          auto Iter = DefMI.getIterator();
          if (&MBB == &MF.front() &&
              (unsigned)std::distance(MBB.instr_begin(), Iter) < ConstPos)
            Iter = ConstMI->getIterator();
          NewMI = addFrameReference(
                      BuildMI(MBB, ++Iter, DL,
                              TII->get(IsRow ? X86::MOV8mr : X86::MOV16mr)),
                      SS, Offset)
                      .addReg(R, 0, SubIdx);
          SlotIndex SIdx = LIS.InsertMachineInstrInMaps(*NewMI);
          LIS.extendToIndices(LIS.getInterval(R), {SIdx.getRegSlot()});
        }
      }
      IsRow = false;
    }
  }
  return true;
}

FunctionPass *llvm::createX86TileConfigPass() { return new X86TileConfig(); }

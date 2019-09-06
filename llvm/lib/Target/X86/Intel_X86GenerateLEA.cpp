//====-- Intel_X86GenerateLEA.cpp - Generate new LEA instructions ---------------====
//
//      Copyright (c) 2019-2019 Intel Corporation.
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
// This file defines the pass that performs some optimizations with LEA
// instructions in order to improve performance and code size.
// Currently, it does one thing:
// 1) Address calculations in load and store instructions are replaced by
//    new LEA def registers where benifit for DSB or code size.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/X86BaseInfo.h"
#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "Intel_X86MemOpKey.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstdint>
#include <iterator>

using namespace llvm;

#define DEBUG_TYPE "x86-generate-lea-opt"

static cl::opt<bool>
    DisableX86GenLEA("disable-x86-gen-lea-opt", cl::Hidden,
                     cl::desc("X86: Disable generate LEA optimizations."),
                     cl::init(false));

namespace {

class GenerateLEAPass : public MachineFunctionPass {
public:
  GenerateLEAPass() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return "X86 generate LEAs"; }

  /// Loop over all of the basic blocks, replacing address
  /// calculations in load and store instructions.
  bool runOnMachineFunction(MachineFunction &MF) override;

  static char ID;
private:
  using MemOpMap = DenseMap<MemOpKey, SmallVector<MachineInstr *, 16>>;

  /// Collect potential instructions in each set.
  void collectPotentialInst(MachineBasicBlock &MBB, MemOpMap &MemOp2MIs);

  /// Return weight depending on the reduced code size and DSB bandwidth.
  unsigned calculateWeight(const MemOpKey &MemOpKey,
                           const SmallVectorImpl<MachineInstr *> &MIs);

  /// Insert a new LEA instruction before each potential instruction,
  /// and replace the calculation of load and store by new LEA def registers.
  bool insertLEA(const MachineBasicBlock &MBB,
                 const SmallVectorImpl<MachineInstr *> &MIs);

  /// Generate a new LEA instruction before each potential instruction
  /// in the basic block if we can get benifit for:
  /// 1) Reducing code size.
  /// 2) Reducing DSB's pressure.
  bool generateLEAs(MachineBasicBlock &MBB);

  MachineRegisterInfo *MRI;
  const X86InstrInfo *TII;
  const X86Subtarget *ST;

};

} // end anonymous namespace

char GenerateLEAPass::ID = 0;

FunctionPass *llvm::createX86GenerateLEAs() { return new GenerateLEAPass(); }
INITIALIZE_PASS(GenerateLEAPass, DEBUG_TYPE,
                "Generate LEA optimizations", false, false)

/// Return true if MO is a virual register or NoRegister.
static bool isVirtualOrNoReg(const MachineOperand *MO) {
  if (!MO->isReg())
    return false;

  if (MO->getReg() == X86::NoRegister)
    return true;

  if (Register::isVirtualRegister(MO->getReg()))
    return true;

  return false;
}

/// Return the immediate operand index in the instruction.
/// -1 means don't have immediate or don't know if
/// this instruction has immediate operand.
static int getSrcImmIndex(int Opc) {
  switch (Opc) {
  case X86::ADD32mi8:
  case X86::ADD32mi:
  case X86::SUB32mi8:
  case X86::SUB32mi:
  case X86::CMP32mi8:
  case X86::CMP32mi:
  case X86::MOV32mi:
    return 5;
  }
  return -1;
}

/// Return the max displacement and min displacement.
static void getMaxAndMinDisp(const SmallVectorImpl<MachineInstr *> &MIs,
                             int64_t &DispMax, int64_t &DispMin) {
  DispMax = INT64_MIN;
  DispMin = INT64_MAX;

  // Calculate the min and max displacement.
  for (const auto MI : MIs) {
    const MCInstrDesc &Desc = MI->getDesc();
    int OpNo = X86II::getMemoryOperandNo(Desc.TSFlags);
    OpNo += X86II::getOperandBias(Desc);

    const MachineOperand &DispOp = MI->getOperand(OpNo + X86::AddrDisp);
    int64_t DispVal = DispOp.isImm() ? DispOp.getImm() : DispOp.getOffset();

    DispMax = std::max(DispMax, DispVal);
    DispMin = std::min(DispMin, DispVal);
  }
}

/// Collect potential instructions in each set.
void GenerateLEAPass::collectPotentialInst(MachineBasicBlock &MBB,
                                           MemOpMap &MemOp2MIs) {
  // Process all instructions in basic block.
  for (auto &MI : MBB) {

    // Instruction must be load or store.
    if (!MI.mayLoadOrStore())
      continue;

    const MCInstrDesc &Desc = MI.getDesc();
    int MemOpNo = X86II::getMemoryOperandNo(Desc.TSFlags);

    if (MemOpNo < 0)
      continue;

    MemOpNo += X86II::getOperandBias(Desc);

    const MachineOperand *BaseOp = &MI.getOperand(MemOpNo + X86::AddrBaseReg);
    const MachineOperand *IndexOp = &MI.getOperand(MemOpNo + X86::AddrIndexReg);
    const MachineOperand *ScaleOp = &MI.getOperand(MemOpNo + X86::AddrScaleAmt);
    const MachineOperand *SegOp = &MI.getOperand(MemOpNo + X86::AddrSegmentReg);
    const MachineOperand *DispOp = &MI.getOperand(MemOpNo + X86::AddrDisp);

    // Must be virtual registers or NoRegister.
    if (!isVirtualOrNoReg(BaseOp))
      continue;
    if (!isVirtualOrNoReg(IndexOp))
      continue;
    if (!isVirtualOrNoReg(SegOp))
      continue;

    // Don't process JTI.
    if (DispOp->isJTI())
      continue;

    const MemOpKey MemOpKey(BaseOp, ScaleOp, IndexOp, SegOp, DispOp);
    MemOp2MIs[MemOpKey].push_back(&MI);
  }
}

/// Return weight depending on the reduced size and DSB bandwidth.
unsigned GenerateLEAPass::calculateWeight(const MemOpKey &MemOpKey,
                                 const SmallVectorImpl<MachineInstr *> &MIs) {
  int64_t DispMax = INT64_MIN;
  int64_t DispMin = INT64_MAX;
  const bool EnableDSBOpt = ST->hasDSB();

  getMaxAndMinDisp(MIs, DispMax, DispMin);

  unsigned SizeWeight = 0;
  unsigned DSBWeight = 0;

  for (const auto MI : MIs) {
    const MCInstrDesc &Desc = MI->getDesc();
    int OpNo = X86II::getMemoryOperandNo(Desc.TSFlags);
    OpNo += X86II::getOperandBias(Desc);

    const MachineOperand &DispOp = MI->getOperand(OpNo + X86::AddrDisp);
    int64_t DispVal = DispOp.isImm() ? DispOp.getImm() : DispOp.getOffset();

    // Saved 3 bytes for displacement.
    if ((!isInt<8>(DispVal) || DispOp.isGlobal()) &&
        isInt<8>(DispMax - DispMin))
      SizeWeight += 3;

    int ImmOpNo = getSrcImmIndex(MI->getOpcode());
    if (ImmOpNo >= 0) {
      ImmOpNo += OpNo;
      const MachineOperand &ImmOp = MI->getOperand(ImmOpNo);
      if (ImmOp.isImm()) {
        int64_t ImmVal = ImmOp.getImm();

        // Reduced DSB bandwidth:
        // DSB's entry has 32 bits of storage space for address and data bits.
        // And the storage space could be split into two 16-bit fields,
        // if the uop can't fill into one entry, it will take an extra entry.
        // A DSB cache line takes an extra clock cycle to load
        // if it uses extra entry.
        if (isInt<16>(ImmVal) && (!isInt<16>(DispVal) || DispOp.isGlobal()) &&
            isInt<16>(DispMax - DispMin))
          ++DSBWeight;
      }
    }

    // Saved 1 byte for SIB.
    if (MemOpKey.getIndexReg() != X86::NoRegister)
      ++SizeWeight;

    // Saved 1 byte for segment override prefix.
    if (MemOpKey.getSegmentReg() != X86::NoRegister)
      ++SizeWeight;
  }

  if (EnableDSBOpt) {
    return SizeWeight + DSBWeight*2;
  } else {
    return SizeWeight;
  }
}

/// Replace the calculation of load and store by new LEA def registers.
bool GenerateLEAPass::insertLEA(const MachineBasicBlock &MBB,
                                const SmallVectorImpl<MachineInstr *> &MIs) {
  bool Changed = false;
  unsigned LEAOp = X86::LEA64r;
  const TargetRegisterClass *TRC = nullptr;

  if (ST->is32Bit()) {
    LEAOp = X86::LEA32r;
    TRC = &X86::GR32RegClass;
  } else if (ST->isTarget64BitLP64()) {
    LEAOp = X86::LEA64r;
    TRC = &X86::GR64RegClass;
  } else if (ST->isTarget64BitILP32()) {
    LEAOp = X86::LEA64_32r;
    TRC = &X86::GR32RegClass;
  } else {
    return Changed;
  }

  // Insert LEA before each instruction.
  for (auto MI : MIs) {
    const MCInstrDesc &Desc = MI->getDesc();
    int MemOpNo = X86II::getMemoryOperandNo(Desc.TSFlags);
    MemOpNo += X86II::getOperandBias(Desc);

    MachineOperand &BaseOp = MI->getOperand(MemOpNo + X86::AddrBaseReg);
    MachineOperand &ScaleOp = MI->getOperand(MemOpNo + X86::AddrScaleAmt);
    MachineOperand &IndexOp = MI->getOperand(MemOpNo + X86::AddrIndexReg);
    MachineOperand &DispOp = MI->getOperand(MemOpNo + X86::AddrDisp);
    MachineOperand &SegOp = MI->getOperand(MemOpNo + X86::AddrSegmentReg);

    const DebugLoc DL = MI->getDebugLoc();
    const Register DestReg = MRI->createVirtualRegister(TRC);
    MachineBasicBlock *MBB = MI->getParent();

    LLVM_DEBUG(dbgs() << "OptimizeLEAs: Candidate to insert: "; MI->dump(););

#ifndef NDEBUG
    const MachineInstrBuilder &LEA =
#endif
    // Insert LEA instruction with MI's base, scale, index, disp and seg.
    BuildMI(*MBB, *MI, DL, TII->get(LEAOp), DestReg)
        .add(BaseOp)
        .add(ScaleOp)
        .add(IndexOp)
        .add(DispOp)
        .add(SegOp);

    // Change MI's operands.
    BaseOp.setReg(DestReg);
    IndexOp.setReg(X86::NoRegister);
    ScaleOp.ChangeToImmediate(1);
    SegOp.setReg(X86::NoRegister);
    DispOp.ChangeToImmediate(0);

    LLVM_DEBUG(dbgs() << "OptimizeLEAs: After inserting: "; LEA->dump();
               MI->dump(););
    Changed = true;
  }

  return Changed;
}

/// Generate a new LEA instruction before each potential instruction
/// in the basic block if we can get benifit for:
/// 1) Reducing code size.
/// 2) Reducing DSB's pressure.
bool GenerateLEAPass::generateLEAs(MachineBasicBlock &MBB) {
  bool Changed = false;
  MemOpMap MemOp2MIs;

  collectPotentialInst(MBB, MemOp2MIs);

  for (auto I : MemOp2MIs) {
    auto MemOpKey = I.first;
    auto MIs = I.second;

    // The displacement is unsigned when base and index registers are
    // NoRegister. Skip this situation.
    if (MemOpKey.getBaseReg() == X86::NoRegister &&
        MemOpKey.getIndexReg() == X86::NoRegister)
      continue;

    const unsigned Weight = calculateWeight(MemOpKey, MIs);
    // Normally, LEA instruction takes 7 bytes.
    unsigned Threshold = 7;

    // Need 1 byte for SIB.
    if (MemOpKey.getIndexReg() != X86::NoRegister)
      ++Threshold;
    // Need 1 byte for segment override prefix.
    if (MemOpKey.getSegmentReg() != X86::NoRegister)
      ++Threshold;

    if (Weight >= Threshold)
      Changed |= insertLEA(MBB, MIs);
  }
  return Changed;
}

bool GenerateLEAPass::runOnMachineFunction(MachineFunction &MF) {
  bool Changed = false;

  if (DisableX86GenLEA || skipFunction(MF.getFunction()))
    return false;

  MRI = &MF.getRegInfo();
  ST = &MF.getSubtarget<X86Subtarget>();
  TII = ST->getInstrInfo();
  if (MF.getFunction().hasFnAttribute("contains-rec-pro-clone")) {
    // Process all basic blocks.
    for (auto &MBB : MF) {
      // Generate LEA instructions if can get benefit.
      Changed |= generateLEAs(MBB);
    }
  }

  return Changed;
}


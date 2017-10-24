//===-- CSADataflowSimplify.cpp - CSA simplifications to dataflow ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements a few optimizations that work on the dataflow graph of
// the CSA representation.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSAInstrInfo.h"
#include "CSAMachineFunctionInfo.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "csa-df-simplify"

namespace llvm {
  class CSADataflowSimplifyPass : public MachineFunctionPass {
  public:
    static char ID;
    CSADataflowSimplifyPass();

    StringRef getPassName() const override {
      return "CSA Dataflow simplification pass";
    }

    bool runOnMachineFunction(MachineFunction &MF) override;
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

  private:
    MachineFunction *MF;
    CSAMachineFunctionInfo *LMFI;
    const CSAInstrInfo *TII;
    std::vector<MachineInstr *> to_delete;

    /// The following mini pass implements a peephole pass that removes NOT
    /// operands from the control lines of picks and switches.
    bool eliminateNotPicks(MachineInstr *MI);

    /// The following mini pass replaces an ALL that goes into a SWITCH with a
    /// SWITCH that goes into ALL. This helps for memory ordering issues.
    bool invertAllSwitch(MachineInstr *MI);

    /// The following mini pass converts loads and stores that are dependent on
    /// strides into streaming loads and stores.
    bool makeStreamMemOp(MachineInstr *MI);

    MachineInstr *getDefinition(const MachineOperand &MO) const;
    void getUses(const MachineOperand &MO,
        SmallVectorImpl<MachineInstr *> &uses) const;
    MachineInstr *getSingleUse(const MachineOperand &MO) const;
    bool isZero(const MachineOperand &MO) const;
  };
}

char CSADataflowSimplifyPass::ID = 0;

CSADataflowSimplifyPass::CSADataflowSimplifyPass() : MachineFunctionPass(ID) {
}


MachineFunctionPass *llvm::createCSADataflowSimplifyPass() {
  return new CSADataflowSimplifyPass();
}

bool CSADataflowSimplifyPass::runOnMachineFunction(MachineFunction &MF) {
  this->MF = &MF;
  LMFI = MF.getInfo<CSAMachineFunctionInfo>();
  TII = static_cast<const CSAInstrInfo*>(MF.getSubtarget<CSASubtarget>().getInstrInfo());

  // Run several functions one at a time on the entire graph. There is probably
  // a better way of implementing this sort of strategy (like how InstCombiner
  // does its logic), but until we have a need to go a fuller InstCombiner-like
  // route, this logic will do. Note that we can't delete instructions on the
  // fly due to how iteration works, but we do clean them up after every mini
  // pass.
  bool changed = false;
  static auto functions = {
    &CSADataflowSimplifyPass::eliminateNotPicks,
    &CSADataflowSimplifyPass::invertAllSwitch,
    &CSADataflowSimplifyPass::makeStreamMemOp
  };
  for (auto func : functions) {
    for (auto &MBB : MF) {
      for (auto &MI : MBB) {
        changed |= (this->*func)(&MI);
      }
      for (auto MI : to_delete)
        MI->eraseFromParent();
      to_delete.clear();
    }
  }

  this->MF = nullptr;
  return changed;
}

MachineInstr *CSADataflowSimplifyPass::getDefinition(const MachineOperand &MO) const {
  assert(MO.isReg() && "LICs to search for can only be registers");
  for (auto &MBB : *MF) {
    for (auto &MI : MBB) {
      for (auto &def : MI.defs()) {
        if (def.isReg() && def.getReg() == MO.getReg())
          return &MI;
      }
    }
  }

  return nullptr;
}

void CSADataflowSimplifyPass::getUses(const MachineOperand &MO,
    SmallVectorImpl<MachineInstr *> &uses) const {
  assert(MO.isReg() && "LICs to search for can only be registers");
  for (auto &MBB : *MF) {
    for (auto &MI : MBB) {
      for (auto &use : MI.uses()) {
        if (use.isReg() && use.getReg() == MO.getReg())
          uses.push_back(&MI);
      }
    }
  }
}

MachineInstr *CSADataflowSimplifyPass::getSingleUse(
    const MachineOperand &MO) const {
  SmallVector<MachineInstr *, 4> uses;
  getUses(MO, uses);
  return uses.size() == 1 ? uses[0] : nullptr;
}

bool isImm(const MachineOperand &MO, int64_t immValue) {
  return MO.isImm() && MO.getImm() == immValue;
}

bool CSADataflowSimplifyPass::isZero(const MachineOperand &MO) const {
  if (MO.isReg()) {
    const MachineInstr *def = getDefinition(MO);
    if (def->getOpcode() == CSA::MOV64 && isImm(def->getOperand(1), 0))
      return true;
  }
  return isImm(MO, 0);
}

bool CSADataflowSimplifyPass::eliminateNotPicks(MachineInstr *MI) {
  unsigned select_op, low_op, high_op;
  if (TII->isSwitch(MI)) {
    select_op = 2;
    low_op = 0;
    high_op = 1;
  } else if (TII->isPick(MI)) {
    select_op = 1;
    low_op = 2;
    high_op = 3;
  } else {
    return false;
  }

  if (MachineInstr *selector = getDefinition(MI->getOperand(select_op))) {
    if (selector->getOpcode() == CSA::NOT1) {
      // This means the selector is a NOT. Swap the two definitions on the
      // output, and change the selector to be the NOT's inverse.
      int reg_tmp = MI->getOperand(low_op).getReg();
      MI->getOperand(low_op).setReg(MI->getOperand(high_op).getReg());
      MI->getOperand(high_op).setReg(reg_tmp);
      MI->getOperand(select_op).setReg(
          selector->getOperand(1).getReg());
      return true;
    }
  }
  return false;
}

bool CSADataflowSimplifyPass::invertAllSwitch(MachineInstr *MI) {
  if (!TII->isSwitch(MI))
    return false;
  MachineInstr *switched = getDefinition(MI->getOperand(3));
  if (!switched || switched->getOpcode() != CSA::ALL0)
    return false;

  // TODO: should it be the case that we check for ALL0 dependent only on memory
  // ordering issues? Or are we going to be more or less guaranteed that ALL0
  // only happens with these?

  const TargetRegisterClass *class0 =
    MF->getSubtarget().getRegisterInfo()->getRegClass(CSA::CI1RegClassID);
  // TODO: CI0 class is more accurate… but there's only 128 of those
  // Generate switches for each of the inputs, at least those that correspond
  // to actual values.
  unsigned switchOutputRegs[4][2];
  for (int i = 0; i < 4; i++) {
    if (switched->getOperand(1 + i).getReg() == CSA::IGN) {
      switchOutputRegs[i][0] = switchOutputRegs[i][1] = CSA::IGN;
      continue;
    }
    unsigned outL = MI->getOperand(0).getReg() == CSA::IGN ?
      (unsigned)CSA::IGN : LMFI->allocateLIC(class0);
    unsigned outR = MI->getOperand(1).getReg() == CSA::IGN ?
      (unsigned)CSA::IGN : LMFI->allocateLIC(class0);
    MachineInstr *newSwitch = BuildMI(*MI->getParent(), MI, MI->getDebugLoc(),
        TII->get(CSA::SWITCH0))
      .addReg(outL, RegState::Define)
      .addReg(outR, RegState::Define)
      .add(MI->getOperand(2))
      .add(switched->getOperand(1 + i));
    newSwitch->setFlag(MachineInstr::NonSequential);
    switchOutputRegs[i][0] = outL;
    switchOutputRegs[i][1] = outR;
  }

  // Generate the two ALLs as output, unless the output is ignored.
  for (int i = 0; i < 2; i++) {
    if (MI->getOperand(i).getReg() == CSA::IGN)
      continue;
    BuildMI(*MI->getParent(), MI, switched->getDebugLoc(), TII->get(CSA::ALL0))
      .add(MI->getOperand(i))
      .addReg(switchOutputRegs[0][i])
      .addReg(switchOutputRegs[1][i])
      .addReg(switchOutputRegs[2][i])
      .addReg(switchOutputRegs[3][i])
      ->setFlag(MachineInstr::NonSequential);
  }

  to_delete.push_back(MI);
  if (MI == getSingleUse(switched->getOperand(0)))
    to_delete.push_back(switched);
  return false;
}

// This pass takes a load or a store controlled by a sequence operator and
// converts it into a streaming load and store. The requirements for legality
// are as follows:
// 1. The address is calculated as a strided offset, with base and stride
//    known. The stride may be limited to 1 for CSA v1. (TODO: implementation
//    not yet considered).
// 2. The length of the stride must be constant, at least in a SCEV-style
//    sense.
// 3. The input and output memory orders must consume/produce a single memory
//    order for the entire loop and not be used otherwise. This is effectively
//    saying that the input is a repeat guarded by a loop stream and the
//    output is a switch where all but the last value are ignored, but it's
//    possible that earlier optimizations do aggregation on a different level.
//
// The biggest constraint on the valid operations is the second one. For now,
// we accept only sequence operators, since calculating length is easy:
// * SEQOTNE64 0, %lic, 1  => length = %lic
// * SEQOTNE64 %lic, 0, -1 => length = %lic
// * SEQOTLTS64 0, %lic, 1 => length = %lic
// * SEQOTLTU64 0, %lic, 1 => length = %lic
// Note that the pred output here is the %stream we consider.
//
// The source of the address computations is more complicated. The following
// patterns should be okay:
// * LD (STRIDE %stream, %base, %stride) => base = %base, stride = %stride
// * LDX (REPEAT %stream, %base), (SEQOT**64_index 0, %N, %stride)
// TODO: Investigate LDD utility.

bool CSADataflowSimplifyPass::makeStreamMemOp(MachineInstr *MI) {
  const MachineOperand *base, *value;
  unsigned stride;
  const MachineOperand *inOrder, *outOrder, *memOrder;
  MachineInstr *stream;
  switch (MI->getOpcode()) {
  case CSA::LD8: case CSA::LD16: case CSA::LD32: case CSA::LD64:
  case CSA::LD1: case CSA::LD16f: case CSA::LD32f: case CSA::LD64f:
  case CSA::ST8: case CSA::ST16: case CSA::ST32: case CSA::ST64:
  case CSA::ST1: case CSA::ST16f: case CSA::ST32f: case CSA::ST64f:
    {
      // The address here must be a STRIDE.
      bool isLoad = MI->mayLoad();
      MachineInstr *memAddr = getDefinition(MI->getOperand(isLoad ? 2 : 1));
      if (!memAddr || memAddr->getOpcode() != CSA::STRIDE64) {
        return false;
      }

      base = &memAddr->getOperand(2);
      const MachineOperand &strideOp = memAddr->getOperand(3);
      unsigned opcodeSize = 1;
      unsigned opcode = MI->getOpcode();
      if (opcode == CSA::LD16 || opcode == CSA::LD16f)
        opcodeSize = 2;
      else if (opcode == CSA::ST16 || opcode == CSA::ST16f)
        opcodeSize = 2;
      else if (opcode == CSA::LD32 || opcode == CSA::LD32f)
        opcodeSize = 4;
      else if (opcode == CSA::ST32 || opcode == CSA::ST32f)
        opcodeSize = 4;
      else if (opcode == CSA::LD64 || opcode == CSA::LD64f)
        opcodeSize = 8;
      else if (opcode == CSA::ST64 || opcode == CSA::ST64f)
        opcodeSize = 8;
      if (!strideOp.isImm()) {
        DEBUG(dbgs() << "Stride is not an immediate, cannot compute stride\n");
        return false;
      }
      stride = strideOp.getImm() / opcodeSize;

      // The STRIDE's stream parameter defines the stream.
      // TODO: assert that we use the seq predecessor output.
      stream = getDefinition(memAddr->getOperand(1));
      memOrder = &MI->getOperand(3);
      inOrder = &MI->getOperand(4);
      outOrder = &MI->getOperand(isLoad ? 1 : 0);
      value = &MI->getOperand(isLoad ? 0 : 2);
      break;
    }
  case CSA::LD8X: case CSA::LD16X: case CSA::LD32X: case CSA::LD64X:
  case CSA::LD1X: case CSA::LD16fX: case CSA::LD32fX: case CSA::LD64fX:
  case CSA::ST8X: case CSA::ST16X: case CSA::ST32X: case CSA::ST64X:
  case CSA::ST1X: case CSA::ST16fX: case CSA::ST32fX: case CSA::ST64fX:
    {
      bool isLoad = MI->mayLoad();
      // The base address needs to be repeated
      MachineInstr *memBase = getDefinition(MI->getOperand(isLoad ? 2 : 1));
      MachineInstr *memIndex = getDefinition(MI->getOperand(isLoad ? 3 : 2));
      if (!memBase || memBase->getOpcode() != CSA::REPEAT64) {
        return false;
      }

      // The stream controls the base REPEAT--they should be the same
      // instruction.
      stream = getDefinition(memBase->getOperand(1));
      if (stream != memIndex) {
        return false;
      }

      switch (memIndex->getOpcode()) {
      case CSA::SEQOTNE64:
      case CSA::SEQOTLTS64:
      case CSA::SEQOTLTU64:
      case CSA::SEQOTLES64:
      case CSA::SEQOTLEU64:
        break; // These are the valid ones.
      default:
        DEBUG(dbgs() << "Candidate indexed memory store failed to have valid "
            << "stream parameter. It may yet be valid.\n");
        DEBUG(MI->dump());
        DEBUG(dbgs() << "Failed operator: ");
        DEBUG(memIndex->dump());
        return false;
      }

      base = &memBase->getOperand(2);
      const MachineOperand &strideOp = memIndex->getOperand(6);
      if (!strideOp.isImm()) {
        DEBUG(dbgs() << "Candidate instruction has non-constant stride.\n");
        return false;
      }
      stride = strideOp.getImm();
      memOrder = &MI->getOperand(4);
      inOrder = &MI->getOperand(5);
      outOrder = &MI->getOperand(isLoad ? 1 : 0);
      value = &MI->getOperand(isLoad ? 0 : 3);
      break;
    }
  default:
    return false;
  }

  DEBUG(dbgs() << "Identified candidate for streaming memory conversion: ");
  DEBUG(MI->dump());
  DEBUG(dbgs() << "Base: " << *base << "; stride: " << stride <<
      "; controlling stream: ");
  DEBUG(stream->dump());

  // Verify that the memory orders are properly constrained by the stream.
  MachineInstr *inSource = getDefinition(*inOrder);
  if (!inSource || inSource->getOpcode() != CSA::REPEAT1 ||
      getDefinition(inSource->getOperand(1)) != stream) {
    DEBUG(dbgs() << "Conversion failed due to bad in memory order.\n");
    return false;
  }

  MachineInstr *outSink = getSingleUse(*outOrder);
  if (!outSink || !TII->isSwitch(outSink)) {
    DEBUG(dbgs() << "Conversion failed because out memory order is not a switch.\n");
    return false;
  }

  // The output memory order should be a switch that ignores the signal unless
  // it's the last iteration of the stream.
  MachineInstr *sinkControl = getDefinition(outSink->getOperand(2));
  if (!sinkControl) {
    DEBUG(dbgs() << "Cannot found the definition of the output order switch");
    return false;
  }

  // TODO: check that we are using the last output of the stream.
  const MachineOperand *realOutSink;
  if (outSink->getOperand(0).getReg() == CSA::IGN) {
    if (sinkControl != stream) {
      DEBUG(dbgs() << "Output memory order is not controlled by the stream\n");
      return false;
    }
    realOutSink = &outSink->getOperand(1);
  } else if (outSink->getOperand(1).getReg() == CSA::IGN) {
    // The control structure should be a not (stream control)
    if (outSink->getOpcode() != CSA::NOT1 ||
        getDefinition(outSink->getOperand(1)) == stream) {
      DEBUG(dbgs() << "Output memory order is not controlled by the stream\n");
      return false;
    }
    realOutSink = &outSink->getOperand(0);
  } else {
    // The output memory order is not ignored...
    DEBUG(dbgs() << "Output memory order is not controlled by the stream\n");
    return false;
  }

  // Compute the length of the stream from the stream parameter.
  const MachineOperand *length;
  const MachineOperand &seqStart = stream->getOperand(4);
  const MachineOperand &seqEnd = stream->getOperand(5);
  const MachineOperand &seqStep = stream->getOperand(6);
  if (stream->getOpcode() == CSA::SEQOTNE64 && isImm(seqEnd, 0) &&
      isImm(seqStep, -1)) {
    length = &seqStart;
  } else if (stream->getOpcode() == CSA::SEQOTNE64 && isImm(seqStep, 1)) {
    // TODO: This isn't really right. Hopefully this will be fixed by a better
    // pattern-matching language for this stuff...
    if (isZero(seqStart)) {
      length = &seqEnd;
    } else {
      DEBUG(dbgs() << "Stream operand is of unknown form.\n");
      return false;
    }
  } else if ((stream->getOpcode() == CSA::SEQOTLTS64 ||
        stream->getOpcode() == CSA::SEQOTLTU64) && isImm(seqStep, 1)) {
    if (isZero(seqStart)) {
      length = &seqEnd;
    } else {
      DEBUG(dbgs() << "Stream operand is of unknown form.\n");
      return false;
    }
  } else {
    DEBUG(dbgs() << "Stream operand is of unknown form.\n");
    return false;
  }

  DEBUG(dbgs() << "No reason to disqualify the memory operation found, converting\n");

  // Actually build the new instruction now.
  unsigned opcode;
  switch (MI->getOpcode()) {
  case CSA::LD8: case CSA::LD8X: opcode = CSA::SLD8; break;
  case CSA::LD16: case CSA::LD16X: opcode = CSA::SLD16; break;
  case CSA::LD32: case CSA::LD32X: opcode = CSA::SLD32; break;
  case CSA::LD64: case CSA::LD64X: opcode = CSA::SLD64; break;
  case CSA::LD1: case CSA::LD1X: opcode = CSA::SLD1; break;
  case CSA::LD16f: case CSA::LD16fX: opcode = CSA::SLD16f; break;
  case CSA::LD32f: case CSA::LD32fX: opcode = CSA::SLD32f; break;
  case CSA::LD64f: case CSA::LD64fX: opcode = CSA::SLD64f; break;
  case CSA::ST8: case CSA::ST8X: opcode = CSA::SST8; break;
  case CSA::ST16: case CSA::ST16X: opcode = CSA::SST16; break;
  case CSA::ST32: case CSA::ST32X: opcode = CSA::SST32; break;
  case CSA::ST64: case CSA::ST64X: opcode = CSA::SST64; break;
  case CSA::ST1: case CSA::ST1X: opcode = CSA::SST1; break;
  case CSA::ST16f: case CSA::ST16fX: opcode = CSA::SST16f; break;
  case CSA::ST32f: case CSA::ST32fX: opcode = CSA::SST32f; break;
  case CSA::ST64f: case CSA::ST64fX: opcode = CSA::SST64f; break;
  default:
    assert(false && "We should have an opcode mapping for these opcodes");
  }

  auto builder = BuildMI(*MI->getParent(), MI, MI->getDebugLoc(),
      TII->get(opcode));
  if (MI->mayLoad())
    builder.addReg(value->getReg(), RegState::Define); // Value (for load)
  builder.add(*realOutSink); // Output memory order
  builder.add(*base); // Address
  builder.add(*length); // Length
  builder.addImm(stride); // Stride
  if (!MI->mayLoad())
    builder.add(*value); // Value (for store)
  builder.add(*memOrder); // Memory ordering
  builder.add(inSource->getOperand(2)); // Input memory order
  builder->setFlag(MachineInstr::NonSequential); // Don't run on the SXU

  // Delete the old instruction. Also delete the old output switch, since we
  // added a second definition of its input. Dead instruction elimination should
  // handle the rest.
  to_delete.push_back(MI);
  to_delete.push_back(outSink);

  return true;
}

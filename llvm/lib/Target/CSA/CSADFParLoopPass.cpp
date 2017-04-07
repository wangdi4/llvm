//===-- CSADFParLoopPass.cpp - CSA optimization of parallel loops in data flow -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass optimizes post-dataflow code by eliminating unnecessary memory
// order dependencies between iterations of a loop that was annotated as being
// parallel.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSATargetMachine.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MachineCDG.h"

using namespace llvm;

#define DEBUG_TYPE "csa-par-loop"

namespace {

  // Forward declaration
  struct ParallelMemdepSubgraph;

  // Class that describes this pass
  class CSADFParLoopPass : public MachineFunctionPass {

  public:
    static char ID;
    CSADFParLoopPass() : MachineFunctionPass(ID) { }

    StringRef getPassName() const override {
      return "CSA Parallel Loop Pass";
    }

    bool runOnMachineFunction(MachineFunction &MF) override;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      MachineFunctionPass::getAnalysisUsage(AU);
    }

  private:
    const CSAInstrInfo *TII;
    MachineRegisterInfo *MRI;

    // Given an instruction, `MI`, tries to match the pattern for a backedge
    // memory ordering dependency in a parallel loop, starting with the
    // CSA_PARALLEL_MEMDEP psuedo-instruction.  If a match is found, returns
    // true and fills the fields in `sg` with pertanant information about the
    // matching instructions and LICs.
    bool matchParallelMemdep(ParallelMemdepSubgraph& sg, MachineInstr& MI);

    // Given a subgraph that describes a backedge memory ordering dependency,
    // remove the the backedge between the SWITCH and PICK instructions, that
    // handle the, thus breaking the inter-iteration dependency.
    void transformParallelMemdep(ParallelMemdepSubgraph& sg);
  };

  // A `ParallelMemdepSubgraph` struct contains information on a subgraph
  // discovered by pattern-matching on a memory-order backedge.
  struct ParallelMemdepSubgraph {
    MachineInstr* parallel_memdep_instr;  // CSA_PARALLEL_MEMDEP instruction
    MachineInstr* switch_instr;           // SWITCH1 instruction
    MachineInstr* mov_instr;              // MOV1 instruction
    MachineInstr* init_instr;             // INIT1 instruction
    MachineInstr* pick_instr;             // PIC1 instruction
    int loopback_edge;   // 0 for left operand of pick/switch, 1 for right operand.
  };

  // This class is used to iterate over a sequence of instructions, filtering
  // by simple channel, opcode, and operand criteria.  The next matching
  // instruction in the sequence is returned on each call to `next()`.  If
  // `usesOnly` is true, consider only uses of the channel.  If `defsOnly` is
  // true, consider only definitions of the channel.
  template <typename InstrIter, bool usesOnly, bool defsOnly>
  class InstrMatcher {

  protected:
    constexpr InstrMatcher()
      : m_curr(), m_end(), m_channelReg(0), m_matchOpcode(0), m_matchOperandIndex(0) { }

    // Initialize for finding instructions in the range `[start, finish)` with
    // the specified `matchOpcode` that use or define the register specified in
    // `channelReg`. If `matchOperandIndex` is specified, then only matches where
    // the use or def is the `matchOperandIndex`th operand are considered;
    // otherwise any operand may match.
    void init(InstrIter start, InstrIter finish,
              unsigned channelReg, unsigned matchOpcode,
              unsigned matchOperandIndex = UINT_MAX);

  public:
    // Return the next matching instruction in the sequence, or `nullptr` if
    // there are no more matches.
    MachineInstr* next();

  private:
    InstrIter m_curr;
    InstrIter m_end;
    unsigned  m_channelReg;
    unsigned  m_matchOpcode;
    unsigned  m_matchOperandIndex;
  };

  // This class is used to iterate over a the uses of the specified channel,
  // filtering by opcode and operand criteria.  The next matching instruction
  // in the sequence is returned on each call to `next()`.
  struct UsesMatcher : InstrMatcher<MachineRegisterInfo::use_instr_iterator, true, false> {

    UsesMatcher(MachineRegisterInfo* MRI, const MachineOperand& channel,
                unsigned matchOpcode, unsigned matchOperandIndex = UINT_MAX);
  };

  // This class is used to iterate over a the defs of the specified channel,
  // filtering by opcode and operand criteria.  The next matching instruction
  // in the sequence is returned on each call to `next()`.
  struct DefsMatcher : InstrMatcher<MachineRegisterInfo::def_instr_iterator, false, true> {

    DefsMatcher(MachineRegisterInfo* MRI, const MachineOperand& channel,
                unsigned matchOpcode, unsigned matchOperandIndex = UINT_MAX);
  };

} // End unnamed namespace

char CSADFParLoopPass::ID = 0;

MachineFunctionPass *llvm::createCSADFParLoopPass() {
  return new CSADFParLoopPass();
}

template <typename InstrIter, bool usesOnly, bool defsOnly>
void InstrMatcher<InstrIter, usesOnly, defsOnly>::init(InstrIter start, InstrIter finish,
                                                       unsigned channelReg,
                                                       unsigned matchOpcode,
                                                       unsigned matchOperandIndex)
{
  assert(m_curr == m_end);

  m_curr              = start;
  m_end               = finish;
  m_channelReg        = channelReg;
  m_matchOpcode       = matchOpcode;
  m_matchOperandIndex = matchOperandIndex;
}

template <typename InstrIter, bool usesOnly, bool defsOnly>
MachineInstr* InstrMatcher<InstrIter, usesOnly, defsOnly>::next()
{
  // Loop through the sequence of instructions, looking for a matching one.
  while (m_curr != m_end) {
    MachineInstr& instr = *m_curr++;

    if (instr.getOpcode() == m_matchOpcode) {
      // Found an instruction with matching opcode

      DEBUG(errs() << "%%% Found matching opcode: " << instr);

      if (m_matchOperandIndex == UINT_MAX)
        return &instr; // We don't care about which operand matched, so we're done.

      // Check if the matching operand is the one we want.
      MachineOperand& operand = instr.getOperand(m_matchOperandIndex);
      if ((! usesOnly || operand.isUse()) && (! defsOnly || operand.isDef())) {
        if (operand.isReg() && operand.getReg() == m_channelReg)
          return &instr;
      }

      DEBUG(errs() << "%%%     but operand match failed!\n");
    }
  }

  return nullptr;
}

UsesMatcher::UsesMatcher(MachineRegisterInfo* MRI, const MachineOperand& channel,
                         unsigned matchOpcode, unsigned matchOperandIndex)
{
  // Fail if channel does not describe a register
  if (! channel.isReg())
    return;
  unsigned reg = channel.getReg();
  if (! reg)
    return;

  init(MRI->use_instr_begin(reg), MRI->use_instr_end(), reg, matchOpcode, matchOperandIndex);
}

DefsMatcher::DefsMatcher(MachineRegisterInfo* MRI, const MachineOperand& channel,
                         unsigned matchOpcode, unsigned matchOperandIndex)
{
  // Fail if channel does not describe a register
  if (! channel.isReg())
    return;
  unsigned reg = channel.getReg();
  if (! reg)
    return;

  init(MRI->def_instr_begin(reg), MRI->def_instr_end(), reg, matchOpcode, matchOperandIndex);
}

bool CSADFParLoopPass::matchParallelMemdep(ParallelMemdepSubgraph& sg,
                                           MachineInstr& MI) {
  // Match the following pattern, starting from the first instruction:
  //
  //    %LICx_A = CSA_PARALLEL_MEMDEP %LICx_OUT
  //    %LICx_C, %X = SWITCHx %LIC1_B, %LICx_A
  //    %LIC1_D = MOV1 %LIC1_B
  //    %LIC1_D = INIT1 1
  //    %LICx_IN = PICKx %LIC1_D, %LICx_C, %X
  //
  // Notes:
  //  * %LIC1 registers are 1-bit LICs
  //  * %LICx registers are 1-bit today, but will be 0-bit in the future
  //  * SWITCHx and PICKx operations are 1-bit today, but will be 0-bit in the future
  //  * %X is a don't-care operand -- we do not do pattern matching on it.
  //  * The 'INIT1 1' operation can be replaced by an 'INIT1 0' operation, in which
  //    case the order of the outputs of SWITCH and last two inputs of PICK
  //    are reversed.

  // CSA_PARALLEL_MEMDEP should the incoming instruction, else fail.
  if (MI.getOpcode() != CSA::CSA_PARALLEL_MEMDEP)
    return false;
  sg.parallel_memdep_instr = &MI;

  DEBUG(errs() << "%%% Start pattern match on " << *sg.parallel_memdep_instr);

  // Match SWITCH instruction by finding use of %LICx_A
  UsesMatcher switchMatcher(MRI, sg.parallel_memdep_instr->getOperand(0), CSA::SWITCH1);
  while ((sg.switch_instr = switchMatcher.next())) {

    // Match MOV instruction by finding use of %LIC1_B
    UsesMatcher movMatcher(MRI, sg.switch_instr->getOperand(2), CSA::MOV1, 1);
    while ((sg.mov_instr = movMatcher.next())) {

      // Match INIT1 instruction by finding another def of %LIC1_D
      DefsMatcher initMatcher(MRI, sg.mov_instr->getOperand(0), CSA::INIT1, 0);
      while ((sg.init_instr = initMatcher.next())) {
        // Get the INIT value
        MachineOperand& initOperand = sg.init_instr->getOperand(1);
        if (! initOperand.isImm())
          continue;

        int initVal = initOperand.getImm();
        assert(initVal == 0 || initVal == 1);
        sg.loopback_edge = ! initVal;  // 0 for pick/switch left operand, 1 for right operand.

        // Match PICK instruction by finding a use of %LIC1_D
        UsesMatcher pickMatcher(MRI, sg.init_instr->getOperand(0), CSA::PICK1, 1);
        while ((sg.pick_instr = pickMatcher.next())) {
          // Now that we found all of the instructions, validate that the output of the
          // SWITCH (%LICx_C) is the same as the corresponding input to the PICK.
          MachineOperand& switchOut = sg.switch_instr->getOperand(sg.loopback_edge);
          MachineOperand& pickIn    = sg.pick_instr->getOperand(2 + sg.loopback_edge);
          if (! (switchOut.isReg() && pickIn.isReg() && switchOut.getReg() == pickIn.getReg())) {
            DEBUG(errs() << "%%% FAILED: Matching pick & switch operand order to INIT value\n");
            continue; // Mismatch between output of switch and input of pick
          }
          DEBUG(errs() << "%%% SUCCEEDED: Pattern match on CSA_PARALLEL_MEMDEP\n");
          return true;
        }
      }
    }
  }

  return false;
}

void CSADFParLoopPass::transformParallelMemdep(ParallelMemdepSubgraph& sg) {
  // Splice out CSA_PARALLEL_MEMOP instruction.
  sg.switch_instr->getOperand(3).setReg(sg.parallel_memdep_instr->getOperand(1).getReg());

  // Disconnect back edge from switch and pick.
  sg.switch_instr->getOperand(sg.loopback_edge).setReg(CSA::IGN);
  sg.pick_instr->getOperand(2 + sg.loopback_edge).setReg(CSA::IGN);
}

bool CSADFParLoopPass::runOnMachineFunction(MachineFunction &MF) {
  TII = static_cast<const CSAInstrInfo*>(MF.getSubtarget().getInstrInfo());
  MRI = &MF.getRegInfo();

  bool changed = false;

  // For each machine instruction in the function, pattern match for unneeded
  // back edges.
  SmallVector<MachineInstr*, 8> parallelMemdepInstrs;
  for (MachineBasicBlock& BB : MF) {
    for (MachineInstr& MI : BB) {
      ParallelMemdepSubgraph sg;
      if (matchParallelMemdep(sg, MI)) {
        // Found an unneeded back edge.
        parallelMemdepInstrs.push_back(sg.parallel_memdep_instr);
        transformParallelMemdep(sg);
        changed = true;
      }
    }
  }

  // Erase any CSA_PARALLEL_MEMDEP psuedo-ops.  These could not be removed while
  // we were traversing the function blocks, so we saved them up and handled
  // them in batch.
  for (MachineInstr* instr : parallelMemdepInstrs)
    instr->eraseFromParent();

  return changed;
}

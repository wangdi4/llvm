//===- CSADeadInstructionElim.cpp - Remove dead CSA instructions --===//
//
//===----------------------------------------------------------------------===//
//
// This is an extremely simple MachineInstr-level dead-code-elimination pass
// for the CSA.  Much of this module was copied from the stock
// DeadMachineInstructionElim.cpp distributed with llvm.  However, this
// version takes into account the fact that CSA instructions are not in a
// sequential order within a basic block. It also takes into account special
// registers like %ign and %na and the difference between LICs and other
// register types.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;

#define DEBUG_TYPE "csa-dead-instruction"

STATISTIC(NumDeletes,          "Number of dead instructions deleted");

namespace llvm {
namespace CSA { // Register classes
  // Register class for ANYC, a superclass representing a channel of any width.
  // This constant is declared in "CSAGenRegisterInfo.inc", but that file is
  // huge and #including it would slow compilation.
  extern const TargetRegisterClass ANYCRegClass;
}
}

namespace {
  class CSADeadInstructionElim : public MachineFunctionPass {
    // This class defines a machine function pass for the CSA which deletes
    // dead instructions. An instruction is considered dead if all of its
    // outputs are ignored. The definition is transitive, so that if an
    // instruction's output feeds only dead instructions, then that
    // instruction is also dead. However, an instruction is *not* considered
    // dead if:
    //
    // - It has a side effect OR
    // - It may store to memory OR
    // - At least one input is a LIC and at least one output is not a
    //   LIC. Such an instruction is assumed to be needed to synchronize
    //   dataflow with sequential code. (TBD: This rule might need to be
    //   revisited.)

    // This function is the main entry point. It performs dead-instruction
    // elimination on the specified machine function.
    bool runOnMachineFunction(MachineFunction &MF) override;

    const TargetRegisterInfo  *TRI;
    const MachineRegisterInfo *MRI;
    const TargetInstrInfo     *TII;
    BitVector                 LivePhysRegs; // Set of live registers

  public:
    static char ID; // Pass identification, replacement for typeid
    CSADeadInstructionElim() : MachineFunctionPass(ID) { }

  private:
    bool isLIC(unsigned Reg) const;
    bool isDead(const MachineInstr& MI) const;
  };
}

// The declaration for this factory function is in file "CSA.h"
MachineFunctionPass *llvm::createCSADeadInstructionElimPass() {
  return new CSADeadInstructionElim();
}

char CSADeadInstructionElim::ID = 0;

//char &llvm::CSADeadInstructionElimID = CSADeadInstructionElim::ID;

// INITIALIZE_PASS(CSADeadInstructionElim, "CSA-dead-elimination",
//                 "Remove dead CSA instructions", false, false)

bool CSADeadInstructionElim::isLIC(unsigned Reg) const {
  // Return true if `Reg` refers to a LIC.
  // `Reg` is a LIC if it belongs to the ANYC (any channel) register class.
  return CSA::ANYCRegClass.contains(Reg);
}

bool CSADeadInstructionElim::isDead(const MachineInstr& MI) const {
  // Return true if this instruction can be determined to be dead. Note that
  // some instructions may return false in one pass over the dead-instruction
  // algorithm, but then return true in a subsequent pass, as transtitive
  // relationships are traversed.

  // Technically speaking inline asm without side effects and no defs can still
  // be deleted. But when you write inline asm, you expect it to show up in
  // the output, so we let it be.
  if (MI.isInlineAsm())
    return false;

  // Don't delete frame allocation labels.
  // TBD: Does this apply to the CSA?
  if (MI.getOpcode() == TargetOpcode::LOCAL_ESCAPE)
    return false;

  // Don't delete instructions with side effects.
  bool SawStore = false;
  if (!MI.isSafeToMove(nullptr, SawStore) && !MI.isPHI())
    return false;

  // Examine each operand.
  bool hasChannelUses = false;    // True if at least one input is a LIC channel
  bool hasNonChannelDefs = false; // True if at least one output not a LIC

  // Iterate over the operands. If any output operand is live (in used), then
  // instruction is not dead.
  for (const MachineOperand& MO : MI.operands()) {
    if (! MO.isReg())
      continue;  // Ignore non-register operands

    unsigned Reg = MO.getReg();
    if (Reg == CSA::IGN || Reg == CSA::NA)
      continue;  // %ign and %na operands do not cause instruction to be live.

    if (MO.isDef()) {
      if (TargetRegisterInfo::isPhysicalRegister(Reg)) {
        // Don't delete live physreg defs, or any reserved register defs.
        if (LivePhysRegs.test(Reg) || MRI->isReserved(Reg))
          return false;
      } else {
        if (!MRI->use_nodbg_empty(Reg))
          // This def has a non-debug use. Don't delete the instruction!
          return false;
      }
      hasNonChannelDefs = hasNonChannelDefs || ! isLIC(Reg);
    } else /* if MO.isUse() */ {
      hasChannelUses = hasChannelUses || isLIC(Reg);
    }
  }

  // Special case handling to avoid deleting instruction that takes an input
  // from a LIC and outputs to a SXU register. If at least one output is not
  // known to be a LIC channel and at least one input operand is a LIC
  // channel, then this instruction may be synchronizing between data
  // flow and SXU and should not be marked as dead.
  if (hasNonChannelDefs && hasChannelUses)
    return false;

  // If none of the defs has uses, the instruction is dead.
  return true;
}

bool CSADeadInstructionElim::runOnMachineFunction(MachineFunction &MF) {
  // Run dead-instruction elimination on the specified machine function.
  // Return true if any changes were made.

  // Algorithm:
  //   Repeat until no more changes:
  //     Traverse all instructions, marking any physical register input
  //       as live.
  //     Traverse all instructions again, deleting any instruction having
  //       no outputs marked as live
  //   Traverse all instructions, replacing any Def of a non-live physical
  //   register with %ign (final cleanup phase)
  //
  // If N is the number of instructions in MF and D is the number instructions
  // to be deleted, then the above algorithm has a complexity of approximately
  // O(N*D). If L is the number of LICs used as operands, the space bound is
  // approximately O(L) except that the use of a BitVector makes the space
  // proportional to the total number of LICs defined (24K), regardless of the
  // number actually in use.  On the other hand, the BitVector uses only one
  // bit per LIC, so changing to something like an unordered_set might not save
  // much memory in the typical case and would use much more memory in the
  // worst case.
  //
  // If the time or space bounds of the above algorithm prove to be
  // problematic, some alternative algorithms have been proposed:
  //
  // Alternative algorithm 1:
  //   Like the current algorithm, but instead of making a separate pass
  //   marking each live register, determine liveness by traversing the use
  //   list for the operand register.  The register is live if it has a use
  //   other than by the instruction being considered for deletion.  This
  //   eliminates memory for the BitVector and eliminates the liveness-marking
  //   pass.
  //
  // Alternative algorithm 2 (roughly Jim Sukah's original algorithm):
  //   Establish an initial queue, `Qd` of known-dead instructions, e.g. by
  //       iterating over all instructions and adding those whose outputs are
  //       ignored or unused.
  //   While Q is not empty
  //     Pop instruction I off Q
  //     For each physical register C input operand of I:
  //       If I is the only use C, then:
  //         For each instruction I2 that defines C (usually not more than one):
  //           Replace any Def of C in I2 with %ign
  //           If all of I2's outputs are now ignored:
  //             Push I2 onto Q
  //
  // If N is the number of instructions in MF and D is the number instructions
  // to be deleted, then the above algorithm has a complexity of approximately
  // O(N + D) and a space bound of O(D) for Q.

  if (skipFunction(*MF.getFunction()))
    return false;

  bool AnyChanges = false;
  MRI = &MF.getRegInfo();
  TRI = MF.getSubtarget().getRegisterInfo();
  TII = MF.getSubtarget().getInstrInfo();

  bool SubpassChanges;
  do {
    DEBUG(dbgs() << "CSADeadInstructionElim: Begin sub-pass\n");
    SubpassChanges = false;

    // Loop over all instructions in all blocks, from bottom to top, so that
    // it's more likely that chains of dependent but ultimately dead
    // instructions will be cleaned up in each pass.
    // Start out assuming that reserved registers are live out of this block.
    LivePhysRegs = MRI->getReservedRegs();

    // Make initial pass, marking any physreg uses as live.
    for (MachineBasicBlock &MBB : make_range(MF.rbegin(), MF.rend())) {
      for (const MachineInstr& MI : MBB) {
        for (const MachineOperand& MO : MI.operands()) {
          if (MO.isReg() && MO.isUse()) {
            unsigned Reg = MO.getReg();
            if (Reg != CSA::IGN && Reg != CSA::NA &&
                TargetRegisterInfo::isPhysicalRegister(Reg)) {
              for (MCRegAliasIterator AI(Reg, TRI, true); AI.isValid(); ++AI)
                LivePhysRegs.set(*AI);
            }
          }
        }
      }
    } // end for each basic block

    // Now scan the instructions and delete dead ones.
    // "Bring out yer dead!"
    for (MachineBasicBlock &MBB : make_range(MF.rbegin(), MF.rend())) {
      // Note: Cannot use range-based for loop because sometimes iterator is
      // not incremented.
      for (MachineBasicBlock::reverse_iterator MII = MBB.rbegin(),
             MIE = MBB.rend(); MII != MIE; ) {

        MachineInstr& MI = *MII++;

        // If the instruction is dead, delete it!
        if (isDead(MI)) {
          DEBUG(dbgs() << "CSADeadInstructionElim: DELETING: " << MI);
          // It is possible that some DBG_VALUE instructions refer to this
          // instruction.  They get marked as undef and will be deleted
          // in the live debug variable analysis.
          MI.eraseFromParentAndMarkDBGValuesForRemoval();
          AnyChanges = true;
          SubpassChanges = true;
          ++NumDeletes;
          continue;
        }

        // "I'm not dead yet!"
      }
    } // end for each basic block
  } while (SubpassChanges);
  DEBUG(dbgs() << "CSADeadInstructionElim: No more changes\n");

  // Cleanup: Loop over all instructions and replace any output to a dead
  // register with %ign
  for (MachineBasicBlock &MBB : make_range(MF.rbegin(), MF.rend())) {
    for (MachineInstr& MI : MBB) {
      for (MachineOperand& MO : MI.operands()) {
        if (MO.isReg() && MO.isDef()) {
          unsigned Reg = MO.getReg();
          if (Reg != CSA::IGN && Reg != CSA::NA && isLIC(Reg) &&
              ! LivePhysRegs.test(Reg)) {
            // Replace output to dead LIC with output to %ign
            DEBUG(dbgs() << "CSADeadInstructionElim: clean up dead LIC "
                  << MO << " from " << MI << '\n');
            MO.substPhysReg(CSA::IGN, *TRI);
          }
        }
      }
    }
  } // end for each basic block

  LivePhysRegs.clear();
  return AnyChanges;
}

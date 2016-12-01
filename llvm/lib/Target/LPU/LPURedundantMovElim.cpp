//===- LPURedundantMovElim.cpp - Remove redundant MOV instructions       --===//
//
//===----------------------------------------------------------------------===//
//
// This pass is an extremely simple MachineInstr-level pass that
// eliminates unnecessary MOV operations through LICs. 
//
//===----------------------------------------------------------------------===//

#include "LPU.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

#include "LPUInstrInfo.h"

using namespace llvm;

static cl::opt<int>
ElimMovOpt("lpu-elim-mov-opt",
           cl::Hidden,
           cl::desc("LPU Specific: Eliminate redundant MOV instructions"),
           cl::init(0));

// Set to -1 to have no limit.
// Otherwise, the value is the number of MOV instructions we will remove. 
static cl::opt<int>
ElimMovLimit("lpu-elim-mov-limit",
             cl::Hidden,
             cl::desc("LPU Specific: Limit on MOV instructions to disconnect"),
             cl::init(-1));


#define DEBUG_TYPE "lpu-redundant-mov-elim"


namespace {
  class LPURedundantMovElim : public MachineFunctionPass {
    // This class defines a machine function pass for the LPU which
    // deletes unnecessary MOV instructions.
    //
    bool runOnMachineFunction(MachineFunction &MF) override;

    const TargetRegisterInfo  *TRI;
    const MachineRegisterInfo *MRI;
    const LPUInstrInfo        *TII;

  public:
    static char ID; // Pass identification, replacement for typeid
    LPURedundantMovElim() : MachineFunctionPass(ID) { }

  private:

    // Determine if the MOV instruction is necessary.
    //
    // A MOV instruction (on a dataflow unit)
    //     MOV<n> dest, src
    //
    // is unnecessary in the following cases:
    //
    // Case 1:  Normal MOV. All the following conditions hold:
    //
    //  (a) The MOV is a dataflow instruction.
    //  (b) Both "dest" and "src" are LICs (not registers).
    //  (c) "dest" is the unique definition of the LIC.
    //       (i.e., there can't be an INIT statement also defining dest)
    //  (d) "src" has a unique definition as well.
    //  (e) The "effective" bitwidth of the result is the same before
    //      and after removing the MOV.
    //
    //      The effective bitwidth is the number of output bits in
    //      "dest" that could possibly be nonzero.
    //      We expect the same result going through 
    //            src, MOV, and dest.
    //      as we get going through only
    //            src, dest.
    //
    //      (The one exception we allow for is MOV0, since we
    //      technically shouldn't care about the value if there is
    //      MOV0).
    //
    //   If all these conditions hold, we can replace all uses of
    //   "dest" with "src", and eliminate the MOV.
    //
    //   Note that condition (d) could be relaxed, if we extend the
    //   calculations in (f) appropriately.
    //
    //
    // Case 2: TBD: It is conceivable that some MOV instructions can
    //    be eliminated even if there is an "INIT" statement on either
    //    "src" or "dest", if it is a valid transform to move the INIT
    //    statement from "dest" to "src", or vice versa.
    //    For now, we are not handling more complex cases.
    //
    //
    // Note that MOV instructions that involve a register are ignored
    // for now.  These MOV instructions are usually necessary for
    // correct transition between SXU and dataflow code.
    bool isRedundantMov(const MachineInstr& MI) const;

    // Eliminate a MOV instruction, assuming that we have already
    // determined it is not needed.
    void disconnectMovInstr(MachineInstr& MI);




    // Some helper methods we are using in the implementation.
    // Ideally, some of these other methods would be in common files,
    // but we will leave them here for now.

    
    // Returns a MachineInstr* if that instruction is the single def
    // of the Reg.
    //
    // TBD: This method is duplicated in LPUOptDFPass.  We should
    // really put it somewhere common...
    MachineInstr* getSingleDef(unsigned Reg,
                               const MachineRegisterInfo* MRI) const;
  };
}

// The declaration for this factory function is in file "LPU.h"
MachineFunctionPass *llvm::createLPURedundantMovElimPass() {
  return new LPURedundantMovElim();
}

char LPURedundantMovElim::ID = 0;



MachineInstr *
LPURedundantMovElim::getSingleDef(unsigned Reg,
                                  const MachineRegisterInfo *MRI) const {
  MachineInstr *Ret = nullptr;
  for (MachineInstr &DefMI : MRI->def_instructions(Reg)) {
    if (DefMI.isDebugValue())
      continue;
    if (!Ret)
      Ret = &DefMI;
    else if (Ret != &DefMI)
      return nullptr;
  }
  return Ret;
}



// Return true if this instruction can be determined to be an
// unnecessary MOV.
bool LPURedundantMovElim::isRedundantMov(const MachineInstr& MI) const {

  // Check each of the conditions described in comments above:
  
  // (a) 
  if (!MI.getFlag(MachineInstr::NonSequential))
    return false;

  // (b)
  assert(MI.getNumOperands() == 2);
  const MachineOperand* dest = &MI.getOperand(0);
  const MachineOperand* src = &MI.getOperand(1);

  // Check that dest is a physical register (i.e., a LIC).
  if (!dest->isReg() ||
      TargetRegisterInfo::isVirtualRegister(dest->getReg())) {
    return false;
  }
  // Check that src is a physical register (i.e., a LIC).
  if (!src->isReg() ||
      TargetRegisterInfo::isVirtualRegister(src->getReg())) {
    return false;
  }

  unsigned src_reg = src->getReg();
  unsigned dest_reg = dest->getReg();

  // (c)
  //
  // Check for only one definition of destination.  More than one
  // definition usually indicates an initial value on the channel
  // (from an INIT) instruction.  We would have to be more careful in
  // eliminating this MOV instruction (by moving the INIT elsewhere).
  MachineInstr* dest_def_MI = getSingleDef(dest_reg,
                                           this->MRI);
  if (dest_def_MI != &MI) {
    DEBUG(errs() << "RedundantMovElim: ignoring instruction " << MI
          << "; more than one definition of destination.\n");
    return false;
  }


  // (d)
  //
  // Also check for a single definition of the source.
  //
  // TBD: Actually, it would be ok to have multiple definitions, but
  // we'd have to do some of the checks below for each source
  // definition, which we aren't doing for now.
  MachineInstr* src_def_MI = getSingleDef(src->getReg(),
                                          this->MRI);
  if (!src_def_MI) {
    DEBUG(errs() << "RedundantMovElim: ignoring instruction " << MI
          << "; more than one definition of source.\n");
    return false;
  }

  // (e)
  const TargetRegisterClass* src_RC= TII->lookupLICRegClass(src_reg);
  const TargetRegisterClass* dest_RC = TII->lookupLICRegClass(dest_reg);

  if ((!src_RC) || (!dest_RC)) {
    // Error looking up register classes for input/output LICs.
    return false;
  }

  int src_bitwidth = src_RC->getSize();
  int mov_bitwidth = TII->getMOVBitwidth(MI.getOpcode());
  int dest_bitwidth = dest_RC->getSize();

  // Special case: a COPY instruction has an implicit bitwidth equal
  // to the source.
  if (MI.isCopy()) {
    mov_bitwidth = src_bitwidth;
  }

  if ((src_bitwidth < 0) ||
      (mov_bitwidth < 0) ||
      (dest_bitwidth < 0)) {
    DEBUG(errs() << "ERROR: RedundantMovElim: Unknown bitwidth.  src = "
          << src_bitwidth
          << ", mov = " << mov_bitwidth
          << ", dest = " << dest_bitwidth
          << "\n");
    return false;
  }


  // The "effective" bitwidth after the MOV operation, i.e., the
  // minimum number of bits that gets moved through the chain of
  // operations.
  int orig_bitwidth = std::min(src_bitwidth, mov_bitwidth);
  orig_bitwidth = std::min(orig_bitwidth, dest_bitwidth);

  // The bitwidth we would have, if we eliminated the MOV operation.
  int new_bitwidth = std::min(src_bitwidth, dest_bitwidth);

  if (new_bitwidth == orig_bitwidth) {
    // If the old and new bitwidths match, then we don't have any
    // problems eliminating the MOV.
    return true;
  }
  else if (orig_bitwidth == 0) {
    // Special case: we can bypass a MOV0 operation directly, because
    // we assume it doesn't matter what the value going into "dest"
    // is.
    //
    // This logic assumes that no one is relying on the semantics of
    // MOV0 as truncating its input down to a 0.  Technically
    // speaking, it seems wrong to care about the actual value of a
    // MOV0 operation anyway...
    DEBUG(errs() << "WARNING: RedundantMovElim: Bypassing a MOV0 instruction\n");
    return true;
  }

  return false;
}

void LPURedundantMovElim::disconnectMovInstr(MachineInstr& MI) {
  DEBUG(errs() << "TBD: Disconnect MOV instr " << MI << "\n");

  assert(TII->isMOV(&MI) || TII->isMemTokenMOV(&MI) || MI.isCopy());
  MachineOperand* dest = &MI.getOperand(0);
  MachineOperand* src = &MI.getOperand(1);

  // Walk over all uses of the destination LIC, and replace them with
  // the source.
  assert(dest->isReg());
  unsigned dest_reg = dest->getReg();
  assert(src->isReg());
  unsigned src_reg = src->getReg();

  // First, find all instructions that use the dest register.
  // Build a list.
  SmallVector<MachineInstr*, 8> uses_of_dest;
  for (auto def_it = MRI->use_instr_begin(dest_reg);
       def_it != MRI->use_instr_end();
       ++def_it) {
    MachineInstr* use_MI = &(*def_it);
    uses_of_dest.push_back(use_MI);
  }

  // Now, call substitute register on each of those instructions.  The
  // reason we don't do this in one loop above is that we end up
  // invalidating the iterator if we change the instructions.
  unsigned uses_replaced = 0;
  for (auto it = uses_of_dest.begin();
       it != uses_of_dest.end();
       ++it) {
    MachineInstr* use_MI = *it;
    use_MI->substituteRegister(dest_reg,
                               src_reg,
                               0,
                               *this->TRI);
    uses_replaced++;
  }

  assert(uses_replaced == uses_of_dest.size());

  // Disconnect the MOV instruction by clearing the destination.  We
  // don't need to mess with the src, because ignoring the output is
  // already enough to make this instruction dead.
  //
  // TBD: For some reason, I get errors when I set the source to NA.
  dest->substPhysReg(LPU::IGN, *this->TRI);
  src->substPhysReg(LPU::NA, *this->TRI);
}



bool LPURedundantMovElim::runOnMachineFunction(MachineFunction &MF) {
  // Eliminate redundant MOV instructions in the specified machine
  // function.  Return true if any changes were made.

  // Algorithm:
  //
  //   Scan each MOV instruction, and eliminate its destination LIC if
  //   the MOV falls into one of the cases described above.
  //
  //   We eliminate the MOV by replacing all uses of "dest" with
  //   "src", and then disconnecting the MOV (i.e., replacing the
  //   source of the MOV with %na, and the dest with %ign).  This
  //   phase relies on dead instruction elimination later to actually
  //   delete the MOV.
  //
  //   For now, we only need one pass, because each MOV instruction is
  //   eliminated serially.

  // Flag to skip the phase.
  if (!ElimMovOpt)
    return false;

  if (skipFunction(*MF.getFunction()))
    return false;

  bool AnyChanges = false;
  MRI = &MF.getRegInfo();
  TRI = MF.getSubtarget().getRegisterInfo();
  TII = static_cast< const LPUInstrInfo* >(MF.getSubtarget().getInstrInfo());

  int num_removed = 0;

  // Loop the basic blocks, doing dead-instruction elimination on each block
  for (MachineBasicBlock &MBB : make_range(MF.rbegin(), MF.rend())) {
    bool LocalChanges = false;

    for (MachineBasicBlock::reverse_iterator MII = MBB.rbegin(),
           MIE = MBB.rend(); MII != MIE; ) {
      MachineInstr& MI = *MII++;
      if (TII->isMOV(&MI) || TII->isMemTokenMOV(&MI) || MI.isCopy()) {
        if (isRedundantMov(MI)) {
          DEBUG(errs() << "RedundantMovElim: Found instruction to eliminate " << MI << "\n");
          if ((ElimMovLimit < 0) || (num_removed < ElimMovLimit)) {
            disconnectMovInstr(MI);
            LocalChanges = true;
            num_removed++;
          }
        }
      }
    }
    AnyChanges = AnyChanges || LocalChanges;
  } // end for each basic block

  DEBUG(errs() << "Redundant MOV: eliminated " << num_removed
        << " MOV instructions\n");
  return AnyChanges;
}

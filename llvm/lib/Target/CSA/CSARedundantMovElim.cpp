//===- CSARedundantMovElim.cpp - Remove redundant MOV instructions       --===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass is an extremely simple MachineInstr-level pass that
// eliminates unnecessary MOV operations through LICs.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

#include "CSAInstrInfo.h"

using namespace llvm;

static cl::opt<int>
  ElimMovOpt("csa-elim-mov-opt", cl::Hidden,
             cl::desc("CSA Specific: Eliminate redundant MOV instructions"),
             cl::init(1));

// Set to -1 to have no limit.
// Otherwise, the value is the number of MOV instructions we will remove.
static cl::opt<int> ElimMovLimit(
  "csa-elim-mov-limit", cl::Hidden,
  cl::desc("CSA Specific: Limit on MOV instructions to disconnect"),
  cl::init(-1));

static cl::opt<int> SXUMovConstantProp(
  "csa-sxu-mov-constant-prop", cl::Hidden,
  cl::desc(
    "CSA Specific: Constant propagate from a MOV instruction on the SXU"),
  cl::init(0));

#define DEBUG_TYPE "csa-redundant-mov-elim"
#define PASS_NAME "CSA: Redundant move elimination."

namespace {
class CSARedundantMovElim : public MachineFunctionPass {
  // This class defines a machine function pass for the CSA which
  // deletes unnecessary MOV instructions.
  //
  bool runOnMachineFunction(MachineFunction &MF) override;

  const TargetRegisterInfo *TRI;
  const MachineRegisterInfo *MRI;
  const CSAInstrInfo *TII;

public:
  static char ID; // Pass identification, replacement for typeid
  CSARedundantMovElim() : MachineFunctionPass(ID) {
    initializeCSARedundantMovElimPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return PASS_NAME;
  }

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
  bool isRedundantMov(const MachineInstr &MI) const;

  // Eliminate a MOV instruction, assuming that we have already
  // determined it is not needed.
  void disconnectMovInstr(MachineInstr &MI);

  // Returns true if this instruction is a MOV of an immediate into
  // a LIC, not on a sequential unit.
  bool isSXUConstantMov(const MachineInstr &MI) const;

  // Returns true if this operand is the use of a channel with an
  // INIT associated with it.
  bool isInitializedMachineOperand(const MachineOperand &MO) const;

  // Returns true if the operand at op_idx corresponds to one of the
  // memory ordering token operands.
  bool isConstantReplaceableOperand(MachineInstr *MI, unsigned op_idx) const;

  // Returns true if this machine instruction (which should use
  // dest_reg) is a candidate for replacing dest_reg with a
  // constant.
  bool isCandidateForSXUConstantReplacement(MachineInstr *use_MI,
                                            unsigned dest_reg) const;

  // Propagate the constant that is the src of MI to its
  // destinations if possible.
  //
  // This function will disconnect MI if this MOV instruction
  // becomes redundant.
  //
  // Returns true if any part of the graph changed, and/or
  bool sxuConstantPropMovAndDisconnect(MachineInstr &MI);

  // Some helper methods we are using in the implementation.
  // Ideally, some of these other methods would be in common files,
  // but we will leave them here for now.

  // Returns a MachineInstr* if that instruction is the single def
  // of the Reg.
  //
  // TBD: This method is duplicated in CSAOptDFPass.  We should
  // really put it somewhere common...
  MachineInstr *getSingleDef(unsigned Reg,
                             const MachineRegisterInfo *MRI) const;
};
} // namespace

// The declaration for this factory function is in file "CSA.h"
MachineFunctionPass *llvm::createCSARedundantMovElimPass() {
  return new CSARedundantMovElim();
}

char CSARedundantMovElim::ID = 0;

INITIALIZE_PASS(CSARedundantMovElim, DEBUG_TYPE, PASS_NAME, false, false)

MachineInstr *
CSARedundantMovElim::getSingleDef(unsigned Reg,
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

bool CSARedundantMovElim::isInitializedMachineOperand(
  const MachineOperand &MO) const {
  if (MO.isReg() && MO.isUse()) {
    unsigned reg  = MO.getReg();
    int num_inits = 0;
    for (MachineInstr &DefMI : MRI->def_instructions(reg)) {
      if (DefMI.isDebugValue())
        continue;
      if (TII->isInit(&DefMI)) {
        num_inits++;
      }
    }
    return (num_inits > 0);
  } else {
    return false;
  }
}

bool CSARedundantMovElim::isConstantReplaceableOperand(MachineInstr *MI,
                                                       unsigned op_idx) const {
  if (!(TII->isLoad(MI) || TII->isStore(MI)))
    return true;

  // The last operand on ordered memory op should be the memory
  // ordering op.
  return (op_idx != (MI->getNumOperands() - 1));
}

bool CSARedundantMovElim::isCandidateForSXUConstantReplacement(
  MachineInstr *use_MI, unsigned dest_reg) const {

  // Count register inputs of this instruction that we are allowed
  // to replace with a constant.
  int num_reg_inputs           = 0;
  int num_inputs_matching_dest = 0;
  int numOps                   = use_MI->getNumOperands();

  // There is a list of instructions here for which
  // constant-replacement is a bad idea.
  //
  //  PICK can be bad if both the selector and one of the inputs is a
  //  constant.
  //
  //  Are there other cases?
  //
  // TBD(jsukha): This code here is a terrible hack and likely to be
  // broken for arbitrary code.  What is the correct algorithm for
  // determining whether it is safe to replace a given machine operand
  // with a constant?  It seems like we might need to analyze the
  // conditions for of the dataflow operations separately.
  if (!(TII->isPick(use_MI))) {
    for (int op_idx = 0; op_idx < numOps; ++op_idx) {
      MachineOperand &MO = use_MI->getOperand(op_idx);
      if (MO.isReg() && MO.isUse() && (!isInitializedMachineOperand(MO))) {

        num_reg_inputs++;
        if (isConstantReplaceableOperand(use_MI, op_idx)) {
          if (MO.getReg() == dest_reg) {
            LLVM_DEBUG(errs() <<
                       "In instruction " << *use_MI << ": matches index "
                       << op_idx << "\n");
            num_inputs_matching_dest++;
          }
        }
      }
    }
  }

  // If we have at least one matching input, and we have at least one
  // register input that is not matching the destination, we can
  // constant propagate.
  if ((num_inputs_matching_dest > 0) &&
      (num_inputs_matching_dest < num_reg_inputs)) {
    LLVM_DEBUG(errs() <<
               "RedundantMovElim: SXU Constant propagate on instruction "
               << *use_MI << " matching inputs = " << num_inputs_matching_dest
               << ", total reg inputs = " << num_reg_inputs << "\n");
    return true;
  } else {
    LLVM_DEBUG(errs() <<
               "WARNING: skipping constant prop on instruction " << use_MI
               << " because all valid inputs would be replaced\n");
    return false;
  }
}

// Return true if this instruction can be determined to be an
// unnecessary MOV.
bool CSARedundantMovElim::isSXUConstantMov(const MachineInstr &MI) const {

  if (MI.getFlag(MachineInstr::NonSequential))
    return false;

  assert(MI.getNumOperands() == 2);
  const MachineOperand *dest = &MI.getOperand(0);
  const MachineOperand *src  = &MI.getOperand(1);

  // Check that dest is a physical register (i.e., a LIC).
  if (!dest->isReg() || TargetRegisterInfo::isVirtualRegister(dest->getReg())) {
    return false;
  }

  return (src->isImm() || src->isFPImm());
}

bool CSARedundantMovElim::sxuConstantPropMovAndDisconnect(MachineInstr &MI) {
  const MachineOperand *dest = &MI.getOperand(0);
  const MachineOperand *src  = &MI.getOperand(1);
  assert(src->isImm() || src->isFPImm());
  assert(dest->isReg());

  unsigned long long cval;
  unsigned dest_reg                  = dest->getReg();
  const TargetRegisterClass *dest_RC = TII->getRegisterClass(dest_reg, *MRI);
  int mov_bitwidth                   = TII->getLicSize(MI.getOpcode());
  int dest_bitwidth                  = this->TRI->getRegSizeInBits(*dest_RC);
  int final_bitwidth                 = std::min(mov_bitwidth, dest_bitwidth);

  const ConstantFP *fval;
  bool is_int;
  if (src->isImm()) {
    is_int = true;
    cval   = src->getImm();
  } else {
    is_int = false;
    fval   = src->getFPImm();
  }

  unsigned long long upper_bound = (1ULL << final_bitwidth) - 1;

  if ((final_bitwidth == 0) || (cval < upper_bound)) {
    SmallVector<MachineInstr *, 8> uses_of_dest;
    int total_use_count = 0;

    // Check that the constant we are moving will "fit" into the
    // output.  0 bitwidth is a special case; it is legal to propagate
    // that constant since we shouldn't actually care about the
    // value...

    // Algorithm: Walk over all the uses of "dest", put them into a
    // list.
    for (auto def_it = MRI->use_instr_begin(dest_reg);
         def_it != MRI->use_instr_end(); ++def_it) {
      MachineInstr *use_MI = &(*def_it);

      bool is_candidate =
        isCandidateForSXUConstantReplacement(use_MI, dest_reg);
      if (is_candidate) {
        uses_of_dest.push_back(use_MI);
      }
      total_use_count++;
    }

    int total_changes = 0;
    for (auto it = uses_of_dest.begin(); it != uses_of_dest.end(); ++it) {
      MachineInstr *use_MI = *it;
      int num_changes      = 0;
      int K                = use_MI->getNumOperands();

      for (int j = 0; j < K; ++j) {
        MachineOperand &MO = use_MI->getOperand(j);
        if (MO.isReg() && MO.isUse() && MO.getReg() == dest_reg) {
          if (isConstantReplaceableOperand(use_MI, j)) {
            if (is_int) {
              MO.ChangeToImmediate(cval);
            } else {
              MO.ChangeToFPImmediate(fval);
            }
          }
          num_changes++;
        }
      }
      // If we didn't find at least one thing to change, then
      // something is wrong...
      assert(num_changes > 0);
      total_changes += (num_changes > 0);
    }

    assert(total_changes <= total_use_count);
    if (total_changes == total_use_count) {
      LLVM_DEBUG(errs() <<
                 "Propagated all uses of constant. disconnecting " << MI
                 << "\n");
      // Eliminate all remaining uses of dest.
      MachineOperand &dest_to_edit = MI.getOperand(0);
      dest_to_edit.substPhysReg(CSA::IGN, *(this->TRI));
    } else {
      LLVM_DEBUG(errs() <<
                 "Only " << total_changes << " out of " << total_use_count
                 << " converted.\n");
    }
    return (total_changes > 0);
  } else {
    if (is_int) {
      LLVM_DEBUG(errs() << "WARNING: Not propagating an int constant " << cval
                 << " to an output channel of bitwidth " << final_bitwidth
                 << " \n");
    } else {
      LLVM_DEBUG(errs() << "WARNING: Not propagating a FP constant " << fval
                 << " to an output channel of bitwidth " << final_bitwidth
                 << " \n");
    }
    return false;
  }
}

// Return true if this instruction can be determined to be an
// unnecessary MOV.
bool CSARedundantMovElim::isRedundantMov(const MachineInstr &MI) const {

  // Check each of the conditions described in comments above:

  // (a)
  if (!MI.getFlag(MachineInstr::NonSequential))
    return false;

  // (b)
  assert(MI.getNumOperands() == 2);
  const MachineOperand *dest = &MI.getOperand(0);
  const MachineOperand *src  = &MI.getOperand(1);

  // Check that dest is a physical register (i.e., a LIC).
  if (!dest->isReg() || TargetRegisterInfo::isVirtualRegister(dest->getReg())) {
    return false;
  }
  // Check that src is a physical register (i.e., a LIC).
  if (!src->isReg() || TargetRegisterInfo::isVirtualRegister(src->getReg())) {
    return false;
  }

  unsigned src_reg  = src->getReg();
  unsigned dest_reg = dest->getReg();

  const TargetRegisterClass *src_RC  = TII->getRegisterClass(src_reg, *MRI);
  const TargetRegisterClass *dest_RC = TII->getRegisterClass(dest_reg, *MRI);

  if ((!src_RC) || (!dest_RC)) {
    // Error looking up register classes for input/output LICs.
    return false;
  }
  if (dest_RC == &CSA::RI1RegClass)
    return false;

  // (c)
  //
  // Check for only one definition of destination.  More than one
  // definition usually indicates an initial value on the channel
  // (from an INIT) instruction.  We would have to be more careful in
  // eliminating this MOV instruction (by moving the INIT elsewhere).
  MachineInstr *dest_def_MI = getSingleDef(dest_reg, this->MRI);
  if (dest_def_MI != &MI) {
    LLVM_DEBUG(errs() << "RedundantMovElim: ignoring instruction " << MI
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
  MachineInstr *src_def_MI = getSingleDef(src->getReg(), this->MRI);
  if (!src_def_MI) {
    LLVM_DEBUG(errs() << "RedundantMovElim: ignoring instruction " << MI
               << "; more than one definition of source.\n");
    return false;
  }

  // (e)
  int src_bitwidth  = this->TRI->getRegSizeInBits(*src_RC);
  int mov_bitwidth  = TII->getLicSize(MI.getOpcode());
  int dest_bitwidth = this->TRI->getRegSizeInBits(*dest_RC);

  // Special case: a COPY instruction has an implicit bitwidth equal
  // to the source.
  if (MI.isCopy()) {
    mov_bitwidth = src_bitwidth;
  }

  if ((src_bitwidth < 0) || (mov_bitwidth < 0) || (dest_bitwidth < 0)) {
    LLVM_DEBUG(errs() << "ERROR: RedundantMovElim: Unknown bitwidth.  src = "
               << src_bitwidth << ", mov = " << mov_bitwidth
               << ", dest = " << dest_bitwidth << "\n");
    return false;
  }

  // The "effective" bitwidth after the MOV operation, i.e., the
  // minimum number of bits that gets moved through the chain of
  // operations.
  int orig_bitwidth = std::min(src_bitwidth, mov_bitwidth);
  orig_bitwidth     = std::min(orig_bitwidth, dest_bitwidth);

  // The bitwidth we would have, if we eliminated the MOV operation.
  int new_bitwidth = std::min(src_bitwidth, dest_bitwidth);

  if (new_bitwidth == orig_bitwidth) {
    // If the old and new bitwidths match, then we don't have any
    // problems eliminating the MOV.
    return true;
  } else if (orig_bitwidth == 0) {
    // Special case: we can bypass a MOV0 operation directly, because
    // we assume it doesn't matter what the value going into "dest"
    // is.
    //
    // This logic assumes that no one is relying on the semantics of
    // MOV0 as truncating its input down to a 0.  Technically
    // speaking, it seems wrong to care about the actual value of a
    // MOV0 operation anyway...
    LLVM_DEBUG(
      errs() << "WARNING: RedundantMovElim: Bypassing a MOV0 instruction\n");
    return true;
  }

  return false;
}

void CSARedundantMovElim::disconnectMovInstr(MachineInstr &MI) {
  LLVM_DEBUG(errs() << "TBD: Disconnect MOV instr " << MI << "\n");

  assert(TII->isMOV(&MI) || TII->isMemTokenMOV(&MI) || MI.isCopy());
  MachineOperand *dest = &MI.getOperand(0);
  MachineOperand *src  = &MI.getOperand(1);

  // Walk over all uses of the destination LIC, and replace them with
  // the source.
  assert(dest->isReg());
  unsigned dest_reg = dest->getReg();
  assert(src->isReg());
  unsigned src_reg = src->getReg();

  // First, find all instructions that use the dest register.
  // Build a list.
  SmallVector<MachineInstr *, 8> uses_of_dest;
  for (auto def_it = MRI->use_instr_begin(dest_reg);
       def_it != MRI->use_instr_end(); ++def_it) {
    MachineInstr *use_MI = &(*def_it);
    uses_of_dest.push_back(use_MI);
  }

  // Now, call substitute register on each of those instructions.  The
  // reason we don't do this in one loop above is that we end up
  // invalidating the iterator if we change the instructions.
  unsigned uses_replaced = 0;
  for (auto it = uses_of_dest.begin(); it != uses_of_dest.end(); ++it) {
    MachineInstr *use_MI = *it;
    use_MI->substituteRegister(dest_reg, src_reg, 0, *this->TRI);
    uses_replaced++;
  }

  assert(uses_replaced == uses_of_dest.size());

  // Disconnect the MOV instruction by clearing the destination.  We
  // don't need to mess with the src, because ignoring the output is
  // already enough to make this instruction dead.
  //
  // TBD: For some reason, I get errors when I set the source to NA.
  dest->substPhysReg(CSA::IGN, *this->TRI);
  src->substPhysReg(CSA::NA, *this->TRI);
}

bool CSARedundantMovElim::runOnMachineFunction(MachineFunction &MF) {
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

  if (skipFunction(MF.getFunction()))
    return false;

  bool AnyChanges = false;
  MRI             = &MF.getRegInfo();
  TRI             = MF.getSubtarget().getRegisterInfo();
  TII = static_cast<const CSAInstrInfo *>(MF.getSubtarget().getInstrInfo());

  int num_removed = 0;

  // Loop the basic blocks, removing redundant MOVs in each block.
  for (MachineBasicBlock &MBB : make_range(MF.rbegin(), MF.rend())) {
    bool LocalChanges = false;

    for (MachineBasicBlock::reverse_iterator MII = MBB.rbegin(),
                                             MIE = MBB.rend();
         MII != MIE;) {
      MachineInstr &MI = *MII++;
      if (TII->isMOV(&MI) || TII->isMemTokenMOV(&MI) || MI.isCopy()) {
        if (isRedundantMov(MI)) {
          LLVM_DEBUG(errs() <<
                     "RedundantMovElim: Found instruction to eliminate "
                     << MI << "\n");
          if ((ElimMovLimit < 0) || (num_removed < ElimMovLimit)) {
            disconnectMovInstr(MI);
            LocalChanges = true;
            num_removed++;
          }
        } else {
          if (SXUMovConstantProp) {
            if (isSXUConstantMov(MI)) {
              LLVM_DEBUG(errs() <<
                         "RedundantMovElim: Checking constant mov " << MI
                         << "\n");
              // Try to propagate some MOV of constants when possible.
              // This is not always legal to do.
              bool changed = sxuConstantPropMovAndDisconnect(MI);
              LocalChanges = LocalChanges || changed;
            } else {
              LLVM_DEBUG(errs() <<
                         "RedundantMovElim: Ignoring mov " << MI << "\n");
            }
          }
        }
      }
    }
    AnyChanges = AnyChanges || LocalChanges;
  } // end for each basic block

  assert(not AnyChanges);

  LLVM_DEBUG(errs() << "Redundant MOV: eliminated " << num_removed
             << " MOV instructions\n");
  return AnyChanges;
}

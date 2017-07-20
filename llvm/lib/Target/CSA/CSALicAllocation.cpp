//===-- CSALicAllocation.cpp - CSA Frame Information ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains LIC Allocation support.
//
//===----------------------------------------------------------------------===//

#include "CSALicAllocation.h"

#include <map>
#include "InstPrinter/CSAInstPrinter.h"
#include "CSA.h"
#include "CSAInstrInfo.h"
#include "CSALicCopyTree.h"
#include "CSATargetMachine.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

#define DEBUG_TYPE "lic-alloc"

using namespace llvm;



// Count the number of uses of a particular register.
//
// TBD(jsukha): This method is more complicated than it needs to be,
// because it is trying to print some useful debugging error messages.
// Eventually, we may wish to simplify this method down even further.
//
int CSALicAllocation::
count_reg_uses(MachineRegisterInfo* MRI,
               unsigned Reg,
               MachineBasicBlock* BB,
               bool skip_phis,
               bool skip_outside_block)
{
  DEBUG(errs() << "Counting uses of register " << PrintReg(Reg) << " in BB " << BB << "\n");
  int uses_in_block = 0;
  int uses_outside_block = 0;
  int uses_outside_in_PHIs = 0;

  for (MachineInstr &useMI : MRI->use_instructions(Reg)) {
    // Count uses inside the block, and outside.
    bool in_block = (useMI.getParent() == BB);
    bool is_phi = (useMI.isPHI());

    // Walk over the operands in the use instruction.  Why do we need
    // a walk?  Because the instruction might use the register
    // multiple times...
    for (MIOperands MO(useMI); MO.isValid(); ++MO) {
      if (MO->isReg() &&
          MO->isUse() &&
          (MO->getReg() == Reg)) {
        
        if (is_phi && skip_phis) {
          DEBUG(errs() << "Skipping use of " << PrintReg(Reg) << ", a use in a PHI was found\n");
          return 0;
        }
        
        if (in_block) {
          uses_in_block++;
        }
        else {
          if (skip_outside_block) {
            DEBUG(errs() << "Skipping use of " << PrintReg(Reg) <<", a use outside block " << BB << " was found\n");
            return 0;
          }
          // Count the uses outside the block.  We really expect all
          // of them to be PHIs at this point.
          uses_outside_block++;

          // Count uses in PHI instructions, just for fun.
          if (is_phi) {
            uses_outside_in_PHIs++;
          }
        }
      }
    }
  }

  // Print out some warning messages if there are uses outside the
  // expected block which are nto PHIs.
  if (uses_outside_block) {
    DEBUG(errs() << "WARNING: Defs of " << PrintReg(Reg) << ": " << uses_outside_block << ", ");
    DEBUG(errs() << uses_outside_in_PHIs << " in PHI subset \n");
  }

  return uses_in_block + uses_outside_block;
}




int CSALicAllocation::
replace_reg_uses_with_LICs(MachineRegisterInfo* MRI,
                           const TargetRegisterInfo& TRI,
                           unsigned Reg,
                           std::vector<unsigned>& replacement_LICs,
                           bool skip_phis,
                           bool skip_outside_block,
                           MachineBasicBlock* targetBB)
{
  DEBUG(errs() << "Replacing all uses of register " << PrintReg(Reg) << "\n");
  unsigned use_count = 0;
  for (MachineRegisterInfo::reg_iterator I = MRI->reg_begin(Reg), E = MRI->reg_end(); I != E; ) {
    MachineOperand& MO = *I;
    ++I;

    if (MO.isReg() &&
        MO.isUse() &&
        (MO.getReg() == Reg)) {


        MachineInstr* use_ins = MO.getParent();
        bool is_phi = use_ins->isPHI();

        //  Skip PHIs if the flag is set.
        if (skip_phis && is_phi) {
          assert((use_count == 0) && "Replaced use of a virtual register involved in a PHI");
          return use_count;
        }

        bool outside_block = (use_ins->getParent() != targetBB);
        if (skip_outside_block && outside_block) {
          assert(use_ins->getParent());
          assert((use_count == 0) && "Replaced use of a virtual register outside target block");
          return use_count;
        }
        
        unsigned target_LIC;
        if (use_count >= replacement_LICs.size()) {
          assert(replacement_LICs.size() >= 1);
          target_LIC = replacement_LICs[0];
        }
        else {
          target_LIC = replacement_LICs[use_count];
        }
        use_count++;
        DEBUG(errs() << "   Replacing use with " << PrintReg(target_LIC) << "\n");
        MO.substPhysReg(target_LIC, TRI);
    }
  }
  
  DEBUG(errs() << "Replaced " << use_count << " uses of register " << PrintReg(Reg) << "\n");
  return use_count;
}



void
CSALicAllocation::
generate_LIC_copies(CSAMachineFunctionInfo* LMFI,
                    const CSAInstrInfo& TII,
                    unsigned src,
                    const TargetRegisterClass* new_LIC_RC,
                    std::vector<unsigned>* replacement_LICs,
                    int N,
                    MachineInstr* def_ins) {


  const int D = 4;
  
  replacement_LICs->clear();
  if (N >= 2) {
    if (this->ENABLE_COPIES) {
      // Create a copy tree.
      CSALicCopyTree<D> copy_tree(N);

      // After testing the copy tree, we don't really need to
      // double-check this any more.
      //
      // assert(copy_tree.validate());

      int num_extra_lics = copy_tree.leaf_stop();

      DEBUG(errs() << "Copy tree for reg " << PrintReg(src));
      DEBUG(errs() << "  N = " << N);
      DEBUG(errs() << ", requires " << num_extra_lics << " copy LICs\n");
      assert(copy_tree.leaf_stop() - copy_tree.leaf_start() == N);

      // Structure that maps the integer ids for the nodes in the copy
      // tree (from [0, num_extra_lics))
      std::vector<unsigned> reg_map;

      // First, allocate the LICs for all nodes in the tree.
      for (int j = 0; j < copy_tree.leaf_stop(); ++j) {
        if (j == 0) {
          reg_map.push_back(src);
        }
        else {
          unsigned copy_reg = LMFI->allocateLIC(new_LIC_RC);
          reg_map.push_back(copy_reg);
          DEBUG(errs() << " For internal node " << j << ", created reg " << PrintReg(copy_reg) << ", " << copy_reg << "\n");
        }
      }
      
      // Next, generate the map which identifies the leaves of the tree.
      for (int j = copy_tree.leaf_start();
           j < copy_tree.leaf_stop();
           ++j) {
        assert((j >= 0) && (j < (int)reg_map.size()));
        replacement_LICs->push_back(reg_map[j]);
      }

      // Finally, add copy statements to the block.
      const unsigned copy_opcode = TII.getCopyOpcode( new_LIC_RC );

      // Walk over the instructions in the copy tree in reverse order.
      // We use this order because we are going to insert all of them
      // after the definition, and we want them to be in order in the
      // basic block.
      for (int k = copy_tree.num_copy_stmts()-1; k >= 0; k--) {
        const CSALicCopyStmt<D>& cstmt = copy_tree.get_copy_stmt(k);

        unsigned dest_reg[D];
        unsigned src_reg = CSA::IGN;
        
        // Look up the source register from the copy statement.
        assert((cstmt.src >= 0) && "Copy statement has invalid source register index");
        assert(cstmt.src < (int)reg_map.size());
        src_reg = reg_map[cstmt.src];

        // Look up the destination registers from the copy statement.
        // Some of these might be CSA::IGN.
        for (int j = 0; j < D; ++j) {
          int dest_idx = cstmt.out[j];
          if (dest_idx >= 0) {
            assert(dest_idx < (int)reg_map.size());
            dest_reg[j] = reg_map[dest_idx];
          }
          else {
            dest_reg[j] = CSA::IGN;
          }
        }

        // Build a copy instruction.
        MachineBasicBlock* BB = def_ins->getParent();
        MachineInstr* copyInst =
          BuildMI(*BB, def_ins, def_ins->getDebugLoc(),
                  TII.get(copy_opcode),
                  dest_reg[0]).
                  addReg(dest_reg[1], RegState::Define).
                  addReg(dest_reg[2], RegState::Define).
                  addReg(dest_reg[3], RegState::Define).
                  addReg(src_reg);

        // Add it to the basic block, after the definition.  Note the
        // order of insertion is important, because we assume the copy
        // tree has the statements in an order that has all def's
        // before uses.
        copyInst->removeFromParent();
        BB->insertAfter(def_ins, copyInst);
      }
    }
    else {
      // If we aren't generating copies, just point everything at the
      // source LIC.
      for (int j = 0; j < N; ++j) {
        replacement_LICs->push_back(src);
      }
    }
  }
  else if (N == 1) {
    // For exactly one copy, life is easy.  Just push our original root register back. q
    replacement_LICs->push_back(src);
  }
  else {
    assert(N == 0);
    // Someone will eventually generate a statement to eat the result.  But we aren't going to do that here.
  }
}


bool CSALicAllocation::
allocateLicsInBlock(MachineBasicBlock* BB)
{
  DEBUG(errs() << "\nCalling allocateLicsinLoop for block " << BB << "\n");
  
  // Extract fields from the machine function.
  MachineFunction* MF = BB->getParent();
  MachineRegisterInfo *MRI = &(MF->getRegInfo());
  const TargetRegisterInfo &TRI = *MF->getSubtarget().getRegisterInfo();
  CSAMachineFunctionInfo *LMFI = MF->getInfo<CSAMachineFunctionInfo>();
  const CSAInstrInfo &TII = *static_cast<const CSAInstrInfo*>
    (MF->getSubtarget().getInstrInfo());

  bool modified = false;

  // Walk over instructions in basic block BB, changing each def into
  // a LIC, and converting its uses to use the LIC copies.
  for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end();  ++I) {
    MachineInstr *MI = &*I;
    DEBUG(errs() << "Found MachineInstr " << *MI << "\n");

    // If we are skipping PHIs in the processing, don't analyze any
    // virtual register defined by a PHI instruction,
    if (this->SKIP_PHI_FLAG && MI->isPHI()) {
      continue;
    }
    
    // Walk over machine operands in that instruction.
    for (MIOperands MO(*MI); MO.isValid(); ++MO) {
      if (MO->isReg()) {
        DEBUG(errs() << "Reg  " << MO->getReg() << " corresponds to " << PrintReg(MO->getReg()) << " \n");
      }
      if (MO->isReg() &&
          MO->isDef()) {
        unsigned Reg = MO->getReg();

        // TBD(jsukha): 
        // This check is necessary (in part) because instructions
        // like the following can appear. What does it mean?  I don't
        // quite know.
        //
        // ADJCALLSTACKDOWN 0, %SP<imp-def,dead>, %SP<imp-use>
        if (!TargetRegisterInfo::isVirtualRegister(Reg)) {
          continue;
        }

        int num_uses = count_reg_uses(MRI, Reg, BB,
                                      this->SKIP_PHI_FLAG,
                                      this->SKIP_OUTSIDE_BLOCK_FLAG);
        DEBUG(errs() << "Reg " << PrintReg(Reg) << ": found " << num_uses << " uses\n");

        // Look up target register class corresponding to this
        // register.
        const TargetRegisterClass* new_LIC_RC = LMFI->licRCFromGenRC( MRI->getRegClass(Reg) );
        assert(new_LIC_RC && "Can't determine register class for register");

        if (this->do_LIC_replacement()) {
          if (num_uses > 0) {
            // Step 1: Allocate a new LIC for this register.
            unsigned newReg = LMFI->allocateLIC(new_LIC_RC);

            // Step 2: TBD: Allocate copy LICs, and generate LIC copy
            // statements.  For now, replace with all the replacements
            // pointing to the same LIC.
            //
            // If we are doing replacement, use the new defined LIC as
            // the source.  Otherwise, just use the original register.
            std::vector<unsigned> replacement_LICs;
            generate_LIC_copies(LMFI,
                                TII,
                                newReg,
                                new_LIC_RC,
                                &replacement_LICs,
                                num_uses,
                                MI);

            // Step 3: Substitute all the uses of Reg with the
            // appropriate LICs.
            replace_reg_uses_with_LICs(MRI,
                                       TRI,
                                       Reg,
                                       replacement_LICs,
                                       this->SKIP_PHI_FLAG,
                                       this->SKIP_OUTSIDE_BLOCK_FLAG,
                                       BB);

            DEBUG(errs() << "Substituting def of reg " << PrintReg(Reg) << " with " << PrintReg(newReg) << "\n");
            MO->substPhysReg(newReg, TRI);

            modified = true;
          }
          else {
            DEBUG(errs() << "For Reg " << PrintReg(Reg) << ", found no uses that can be replaced\n");
          }
        }
      }
    }
  }
  
  return modified;
}








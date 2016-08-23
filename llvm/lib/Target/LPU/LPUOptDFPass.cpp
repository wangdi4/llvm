//===-- LPUOptDFPass.cpp - LPU optimization of data flow ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass optimizes post-dataflow code.  In particular, it does things
// like insert sequence operations when appropriate.
//
//===----------------------------------------------------------------------===//

#include <map>
#include "LPU.h"
#include "InstPrinter/LPUInstPrinter.h"
#include "LPUInstrInfo.h"
#include "LPUTargetMachine.h"
#include "LPULicAllocation.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

// Define data structures needed for sequence optimizations.
#include "LPUSequenceOpt.h"
// Width of vectors we are using for sequence op calculation.
// TBD(jsukha): As far as I know, this value only affects performance,
// not correctness? 
#define SEQ_VEC_WIDTH 8

using namespace llvm;

static cl::opt<int>
OptDFPass("lpu-opt-df-pass", cl::Hidden,
               cl::desc("LPU Specific: Optimize data flow pass"),
               cl::init(0));

#define DEBUG_TYPE "lpu-opt-df"


// Flag for enabling sequence optimizations.
//
//   0: Disabled optimization
//   1: Analysis to find sequence optimizations only.
//      Prints LLVM debugging output.
//   2: Enable transformations.
//
// Note that this optimization is not run unless lpu-opt-df-pass is
// also set > 0.
static cl::opt<int>
RunSequenceOpt("lpu-seq-opt", cl::Hidden,
               cl::desc("LPU Specific: Enable sequence optimizations"),
               cl::init(1));

// Flag to specify the maximum number of sequence candidates we will
// identify in a given loop.
// TBD(jsukha): We may not care about this limit in the long run?
// No code uses this limit yet...
static cl::opt<int>
SequenceMaxPerLoop("lpu-seq-max",
              cl::Hidden,
              cl::desc("LPU Specific: Max sequence units inserted per loop"),
              cl::init(1024));



namespace {
class LPUOptDFPass : public MachineFunctionPass {
public:
  static char ID;
  LPUOptDFPass() : MachineFunctionPass(ID) { thisMF = nullptr;}

  const char* getPassName() const override {
    return "LPU Convert Control Flow to Data Flow";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    //AU.addRequired<MachineLoopInfo>();
    //AU.addRequired<LiveVariables>();
    //AU.addRequired<MachineDominatorTree>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }




  // Do sequence optimizations.
  void runSequenceOptimizations(int seq_opt_level);

  // Helper methods for sequence
  
  // Debug print methods.
  //
  // Print header
  void seq_debug_print_header(LPUSeqHeader& header);
  //
  // Print loop info. 
  void seq_print_loop_info(SmallVector<LPUSeqLoopInfo, SEQ_VEC_WIDTH>* loops);


  // Returns true if a machine instruction is
  //   <picker> = INIT1 0
  bool seq_is_picker_init_inst(MachineRegisterInfo* MRI,
                               MachineInstr* MI,
                               unsigned* picker_channel);

  // Returns true if a machine instruction is
  //   <picker> = MOV1 <switcher>
  bool seq_is_picker_mov_inst(MachineRegisterInfo* MRI,
                              MachineInstr* MI,
                              unsigned picker_channel,
                              unsigned* switcher_channel);

  // Returns true if we found the sequence of LPU instructions that
  // forms the header of a loop.  If true, fills in "header" 
  // with the info.
  bool seq_identify_header(MachineInstr* MI,
                           LPUSeqHeader* header);

  // Helper method for finding sequence candidates.
  void seq_find_candidates(SmallVector<LPUSeqLoopInfo, SEQ_VEC_WIDTH>* loops);
  
private:
  MachineFunction *thisMF;
};
}

MachineFunctionPass *llvm::createLPUOptDFPass() {
  return new LPUOptDFPass();
}

char LPUOptDFPass::ID = 0;

bool LPUOptDFPass::runOnMachineFunction(MachineFunction &MF) {

  if (OptDFPass == 0) return false;

  thisMF = &MF;

  bool Modified = false;

  // Using SEQ in place of pick/add/cmp/switch pattern.
  runSequenceOptimizations(RunSequenceOpt);

  return Modified;

}



/*****************************************************************************/
// Code for sequence optimizations.
//
// TBD(jsukha): This code should eventually be split out into a
// different file, and possibly a different pass?
//

void LPUOptDFPass::seq_debug_print_header(LPUSeqHeader& header) {
  DEBUG(errs() << "LPUSeqHeader: picker = " << header.pickerChannel);
  DEBUG(errs() << "\nswitcher = " << header.switcherChannel);
  if (header.pickerInit) {
    DEBUG(errs() << " pickerInit: " << *header.pickerInit << "");
  }
  else {
    DEBUG(errs() << " No pickerInit\n");
  }
  if (header.pickerMov1) {
    DEBUG(errs() << " pickerInit: " << *header.pickerMov1 << "");
  }
  else {
    DEBUG(errs() << " No pickerMov1\n");
  }
  if (header.compareInst) {
    DEBUG(errs() << " compareInst: " << *header.compareInst << "");
  }
  else {
    DEBUG(errs() << " No compareInst\n");
  }


}

bool LPUOptDFPass::seq_is_picker_init_inst(MachineRegisterInfo* MRI,
                                           MachineInstr* MI,
                                           unsigned* pickerChannel) {
  // Just insert a dummy value for safety. 
  if (MI->getOpcode() == LPU::INIT1) {
    DEBUG(errs() << "Found an init instruction " << *MI
          << "with " << MI->getNumOperands() << " operands \n");
    if (MI->getNumOperands() == 2) {
      MachineOperand& picker_def = MI->getOperand(0);
      MachineOperand& init_val = MI->getOperand(1);

      if ((init_val.isImm()) &&
          (init_val.getImm() == 0)) {
        DEBUG(errs() << "Found an init 0 \n");
        
        if (picker_def.isReg()) {
          int pickval = picker_def.getReg();
          // TBD(jsukha): I can't figure out how to query the register
          // class of a channel here.  Bcause it is a physical
          // register, I can't seem to use the normal getRegClass methods. 
          // But I'm going to assume that knowing that the
          // opcode was INIT1 was enough...
          if (!TargetRegisterInfo::isVirtualRegister(pickval)) {
            *pickerChannel = pickval;
            return true;
          }
          else {
            DEBUG(errs() << "Found picker in a virtual reg. Skipping...\n");
          }
        }
        else {
          DEBUG(errs() << "Picker def " << picker_def << " is not a reg\n");
        }
      }
    }
  }
  return false;
}


bool LPUOptDFPass::seq_is_picker_mov_inst(MachineRegisterInfo* MRI,
                                          MachineInstr* MI,
                                          unsigned pickerChannel,
                                          unsigned* switcherChannel) {

  if (MI && (MI->getOpcode() == LPU::MOV1)) {
    if (MI->getNumOperands() == 2) {
      MachineOperand& pickerDef = MI->getOperand(0);
      MachineOperand& switcherDef = MI->getOperand(1);
      if (pickerDef.isReg() &&
          (pickerDef.getReg() == pickerChannel)) {
        if (switcherDef.isReg()) {
          int swchannel = switcherDef.getReg();
          if (!TargetRegisterInfo::isVirtualRegister(swchannel)) {
            *switcherChannel = swchannel;
            return true;
          }
        }
      }
    }
  }

  return false;
}

bool LPUOptDFPass::seq_identify_header(MachineInstr* MI,
                                       LPUSeqHeader* header) {
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  const LPUInstrInfo &TII =
    *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  
  // Look for an "<picker> = INIT1 0" instruction.
  unsigned pickerChannel;
  if (seq_is_picker_init_inst(MRI, MI, &pickerChannel)) {

    DEBUG(errs() << "Found picker definition. Register= " <<
          pickerChannel << " = " << PrintReg(pickerChannel) << "\n");

    // Once we have a picker, then walk over and count the defs.  We
    // want to find exactly one (other) def != MI, which is a MOV1
    // instruction.
    int def_count = 0;
    MachineInstr* pickerMov1 = NULL;
    for (auto def_it = MRI->def_instr_begin(pickerChannel);
         def_it != MRI->def_instr_end();
         ++def_it) {
      MachineInstr* defMI = &(*def_it);
      if (defMI != MI) {
        pickerMov1 = defMI;
      }
      def_count++;
    }

    DEBUG(errs() << "Num defs found: " << def_count << "\n");
    unsigned switcherChannel = 0;
    if (!seq_is_picker_mov_inst(MRI, pickerMov1,
                                pickerChannel,
                                &switcherChannel)) {
      // If that last definition is not instruction we want, bail.
      return false;
    }

    DEBUG(errs() << "Found pickerMov1 instruction " << *pickerMov1);
    DEBUG(errs() << " with switcher channel " << switcherChannel << "\n");
    
    // If we make it here, then we have both a picker and a switcher.
    // Next, check if the switcher is defined by a compare.
    int def2_count = 0;
    MachineInstr* compareInst = NULL;
    for (auto def_it = MRI->def_instr_begin(switcherChannel);
         def_it != MRI->def_instr_end();
         ++def_it) {
      def2_count++;
      compareInst = &(*def_it);
    }

    if (! ((def2_count == 1)  && TII.isCmp(compareInst))) {
      DEBUG(errs() << "Stop. Found " << def2_count <<
            " defs, with last instr " << *compareInst << "\n");
      return false;
    }
    DEBUG(errs() << "Found compare instruction " << compareInst << "\n");

    // Finally, if we made it here, success!  Initialize the header.
    header->init(MI,
                 pickerMov1,
                 compareInst,
                 pickerChannel,
                 switcherChannel);
    return true;
  }
  return false;
}


void LPUOptDFPass::
seq_print_loop_info(SmallVector<LPUSeqLoopInfo, SEQ_VEC_WIDTH>* loops) {

  DEBUG(errs() << "************************\n");  
  DEBUG(errs() << "SEQ LOOP INFO:  " << loops->size() << " loops\n");


  for (unsigned i = 0; i < loops->size(); ++i) {
    LPUSeqLoopInfo& current_loop = (*loops)[i];

    DEBUG(errs() << "*****************\n");
    DEBUG(errs() << "Loop " << i << "[ ");
    DEBUG(errs() << current_loop.candidates.size() << " pairs]\n");
    seq_debug_print_header(current_loop.header);

    // Dump the pick/switch pairs that we found.
    for (auto it = current_loop.candidates.begin();
         it != current_loop.candidates.end();
         ++it) {

      LPUSeqCandidate& x = *it;
      DEBUG(errs() << " pick = " << *x.pickInst);
      DEBUG(errs() << " switch = " << *x.switchInst);
    }
    DEBUG(errs() << "*****************\n");
  }
  DEBUG(errs() << "************************\n");  
}

void
LPUOptDFPass::
seq_find_candidates(SmallVector<LPUSeqLoopInfo, SEQ_VEC_WIDTH>* loops) {
  const LPUInstrInfo &TII =
    *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();

  for (MachineFunction::iterator BB = thisMF->begin(), E=thisMF->end();
       BB != E;
       ++BB) {

    LPUSeqHeader tmp_header;
    MachineBasicBlock::iterator iterMI = BB->begin();

    while (iterMI != BB->end()) {
      MachineInstr* MI = iterMI;

      if (seq_identify_header(MI, &tmp_header)) {
        DEBUG(errs() << "Found a sequence header " << "\n");
        seq_debug_print_header(tmp_header);

        // Save the header information into the current loop.
        LPUSeqLoopInfo current_loop;
        current_loop.header = tmp_header;

        // We are going to look for pick and switch instructions, and
        // store them into a map keyed on their loopback inputs.
        DenseMap<unsigned, MachineInstr*> pickMap;
        SmallVector<unsigned, 8> loopback_channels;

        // Walk over uses of the picker channel, storing the picks and
        // corresponding loopback channel.
        for (auto it = MRI->use_instr_begin(current_loop.header.pickerChannel);
             it != MRI->use_instr_end();
             ++it) {
          MachineInstr* MI = &(*it);

          // Look for:
          //   pick[n] top_val, pickerChannel, init_val, loop_back
          if ((TII.isPick(MI)) &&
              (MI->getNumOperands() == 4)) {
            MachineOperand& selectOp = MI->getOperand(1);
            MachineOperand& loopbackOp = MI->getOperand(3);

            if (selectOp.isReg() && loopbackOp.isReg()) {
              unsigned select_reg = selectOp.getReg();
              unsigned loopback_reg = loopbackOp.getReg();

              if ((!TargetRegisterInfo::isVirtualRegister(select_reg)) &&
                  (select_reg == current_loop.header.pickerChannel) &&
                  (!TargetRegisterInfo::isVirtualRegister(loopback_reg))) {
                DEBUG(errs() << "Found a pick candidate " << *MI <<
                      " with loopback reg " << loopback_reg << "\n");

                if (pickMap.find(loopback_reg) != pickMap.end()) {
                  DEBUG(errs() << "WARNING: found an existing pick ins " <<
                        *pickMap[loopback_reg] <<
                        " with same loopback reg...\n");
                }
                else {
                  pickMap[loopback_reg] = MI;
                  loopback_channels.push_back(loopback_reg);
                }
              }
            }
          }
        }

        // Now walk over the uses of the corresponding switcher
        // channel.  Look for a matching pick (based on the loopback
        // channel).
        for (auto it =
               MRI->use_instr_begin(current_loop.header.switcherChannel);
             it != MRI->use_instr_end();
             ++it) {
          MachineInstr* MI = &(*it);

          // Look for:
          //   switch[n] final, loopback, switcherChannel, bottom_val
          if ((TII.isSwitch(MI)) &&
              (MI->getNumOperands() == 4)) {
            MachineOperand& loopbackOp = MI->getOperand(1);
            MachineOperand& selectOp = MI->getOperand(2);

            if (selectOp.isReg() && loopbackOp.isReg()) {
              unsigned select_reg = selectOp.getReg();
              unsigned loopback_reg = loopbackOp.getReg();

              if ((!TargetRegisterInfo::isVirtualRegister(select_reg)) &&
                  (select_reg == current_loop.header.switcherChannel) &&
                  (!TargetRegisterInfo::isVirtualRegister(loopback_reg))) {

                DEBUG(errs() << "Found a possible switch candidate " <<
                      *MI << " with loopback reg " << loopback_reg << "\n");

                if (pickMap.find(loopback_reg) == pickMap.end()) {
                  DEBUG(errs() <<
                        "WARNING: no matching pick for this switch\n");
                }
                else {
                  MachineInstr* matching_pick = pickMap[loopback_reg];

                  // Finally, verify that the number of uses of the
                  // loopback register is exactly 1, i.e., in the
                  // pick.
                  int expected_loopback_uses = 1;
                  int num_loopback_uses = 0;
                  for (auto lb_it = MRI->use_instr_begin(loopback_reg);
                       lb_it != MRI->use_instr_end();
                       ++lb_it) {
                    num_loopback_uses++;
                    MachineInstr* tmpMI = &(*lb_it);
                    if (tmpMI != matching_pick) {
                      DEBUG(errs() <<
                            "WARNING: Found other use of loopback " <<
                            loopback_reg << ": " << *tmpMI << "\n");
                    }
                    if (num_loopback_uses >= expected_loopback_uses) {
                      break;
                    }
                  }

                  if (num_loopback_uses == expected_loopback_uses) {
                    LPUSeqCandidate nc = LPUSeqCandidate(matching_pick,
                                                         MI);
                    current_loop.candidates.push_back(nc);
                  }
                }
              }
            }
          }
        }

        loops->push_back(current_loop);
      }
      else {
        //        DEBUG(errs() << "Skipping header ins " << *MI << "\n");
      }
      
      ++iterMI;
    }
  }
}


void LPUOptDFPass::runSequenceOptimizations(int seq_opt_level) {
  if (seq_opt_level > 0) {

    // Do analysis to identify candidates for sequence optimization. 
    DEBUG(errs() << "Running analysis for sequence optimizations\n");

    // Look for the candidates we might replace with sequences.
    SmallVector<LPUSeqLoopInfo, SEQ_VEC_WIDTH> loops;
    seq_find_candidates(&loops);
    seq_print_loop_info(&loops);
    
    if (seq_opt_level > 1) {
      // Actually do the transforms.
      // 
      // TBD(jsukha): Fill in the code that does the sequence
      // transformations.
      DEBUG(errs() << "TBD: Performing sequence optimizations\n");
    }
  }
  else {
    DEBUG(errs() << "Sequence optimizations disabled\n");
  }
}


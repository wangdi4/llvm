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
#include <set>
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

const TargetRegisterClass* SeqPredRC = &LPU::CI1RegClass;

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



// Test flag, for breaking all memory dependencies for loops that are
// controlled by a sequence.
//
//   0: Disabled 
//   1: Break memory dependencies for any sequence loop if we can find a
//      reasonable subgraph walking back from the switch to the pick.
//      (TBD(jsukha): I think this graph walk is conservative, and
//       guaranteed not to misidentify the memory dependency graph,
//       but I'm not 100% sure.)
// 
//   2. Break memory dependencies whenever we find a sequence.  This
//      setting is likely to be incorrect for arbitrary programs, but
//      may be true for many small parallel kernels we are trying to
//      compile.
//
// Note that this optimization has no effect unless lpu-opt-df-pass is
// also set > 0.
//
// WARNING: Setting this flag may result in incorrect code being
// generated.  Use with extreme caution.
static cl::opt<int>
SeqBreakMemdep("lpu-seq-break-memdep", cl::Hidden,
               cl::desc("LPU Specific: Break memory dependencies for sequenced loops"),
               cl::init(0));

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


  // Return the unique machine instruction that defines or uses a
  // register, if exactly one exists.  Otherwise, returns NULL.
  MachineInstr*
  getSingleDef(unsigned Reg, const MachineRegisterInfo *MRI);
  MachineInstr*
  getSingleUse(unsigned Reg, const MachineRegisterInfo *MRI);



  // Do sequence optimizations.
  void runSequenceOptimizations(int seq_opt_level);

  // Helper methods for sequence
  
  // Debug print methods.
  //
  // Print header
  void seq_debug_print_header(LPUSeqHeader& header);

  // Print the information out of a sequence candidate.
  void seq_debug_print_candidate(LPUSeqCandidate& x);
  
  //
  // Print loop info. 
  void seq_print_loop_info(SmallVector<LPUSeqLoopInfo, SEQ_VEC_WIDTH>* loops);


  // Returns true if a machine instruction is
  //   <picker> = INIT1 0
  bool seq_is_picker_init_inst(MachineRegisterInfo* MRI,
                               MachineInstr* MI,
                               unsigned* picker_channel,
                               bool* picker_sense);

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

  // Returns MI if  MI is a pick instruction that matches the specified
  // loop header.  Otherwise, returns NULL.
  // Also saves MI into pickMap (keyed by loopback register) if a
  // match is found.
  MachineInstr*
  seq_candidate_match_pick(MachineInstr* MI,
                           const LPUSeqHeader& header,
                           const LPUInstrInfo& TII,
                           DenseMap<unsigned, MachineInstr*>& pickMap);
  
  // Returns matching pick instruction if MI is a switch instruction
  // that matches a pick in the specified loop.  Otherwise, returns
  // NULL.
  MachineInstr*
  seq_candidate_match_switch(MachineInstr* MI,
                             const LPUSeqHeader& header,
                             const LPUInstrInfo& TII,
                             DenseMap<unsigned, MachineInstr*>& pickMap);


  // Helper method for finding sequence candidates.
  void seq_find_candidates(SmallVector<LPUSeqLoopInfo, SEQ_VEC_WIDTH>* loops);


  // Check whether this candidate sequence is either a repeat or a
  // reduction.  If yes, then it modifies x.stype.
  //
  // If it is a repeat, save its channel into "repeat_channels".
  // Returns one of: 
  //
  //  SeqType::UNKNOWN
  //  SeqType::REPEAT
  //  SeqType::REDUCTION
  LPUSeqCandidate::SeqType
  seq_classify_repeat_or_reduction(LPUSeqCandidate& x);


  // Check whether this candidate sequence matches a "stride" type.
  // This pass uses the 
  LPUSeqCandidate::SeqType
  seq_classify_stride(LPUSeqCandidate& x,
                      const DenseMap<unsigned, int>& repeat_channels);


  // Check whether this candidate sequence represents a memory dependency
  // chain. 
  LPUSeqCandidate::SeqType
  seq_classify_memdep_graph(LPUSeqCandidate& x);

  // Classify all the candidate sequences in the loops we found.
  void
  seq_classify_candidates(SmallVector<LPUSeqLoopInfo, SEQ_VEC_WIDTH>* loops);




  // The final check to see if we can transform the loop control
  // variable on a loop, and then transform the loop. 
  bool seq_try_transform_loop(LPUSeqLoopInfo& loop,
                              std::set<MachineInstr*>& insSetMarkedForDeletion);
  
  // The hard work of the try_transform method. 
  void seq_do_transform_loop(LPUSeqLoopInfo& loop,
                             LPUSeqCandidate& indvarCandidate,
                             unsigned indvar_opcode,
                             int indvarIdx,
                             int boundIdx,
                             int boundOpIdx,
                             std::set<MachineInstr*>& insSetMarkedForDeletion);


  // Tries to compute a matching sequence opcode for the pair of a
  // comparison instruction and the transform instruction.
  //
  // Returns true if we found a match, and false otherwise.  If true,
  // then *indvar_opcode is the opcode of the new sequence instruction
  // that we will use.
  bool seq_compute_matching_seq_opcode(unsigned compare_opcode,
                                       unsigned transform_opcode,
                                       bool invert_compare, 
                                       unsigned* indvar_opcode);



  // Tries to compute a matching stride opcode for the transform
  // instruction for a stride candidate.
  bool seq_compute_matching_stride_opcode(unsigned transformOpcode,
                                          unsigned* strideOpcode);
  


  // Add a switch instruction after "prev_inst" in basic block BB,
  // which stores the last value to the output channel for the
  // specified sequence candidate (sCandidate).
  //
  // Returns NULL if no switch instruction is necessary (because the
  //  output channel of the candidate is %ign).
  // Otherwise, returns a pointer to the instruction we created. 
  MachineInstr*
  seq_add_output_switch_for_seq_candidate(LPUSeqCandidate& sCandidate,
                                          const LPUSeqHeader& loop_header,
                                          unsigned last_reg,
                                          const LPUInstrInfo &TII,
                                          MachineBasicBlock& BB,
                                          MachineInstr* prev_inst);

  // Look up the stride operation that corresponds to a given sequence
  // candidate.  
  MachineOperand*
  seq_lookup_stride_op(LPUSeqLoopInfo& loop,
                       LPUSeqCandidate& scandidate);

  // Add a repeat instruction for the specified repeat candidate.
  // Returns the added instruction. 
  MachineInstr*
  seq_add_repeat(LPUSeqCandidate& repeat_candidate,
                 const LPUSeqHeader& loop_header,
                 unsigned pred_reg,
                 const LPUInstrInfo &TII,
                 MachineBasicBlock& BB,
                 MachineInstr* prev_inst);

  // Add a stride instruction for the specified stride candidate.
  // Returns the added instruction. 
  MachineInstr*
  seq_add_stride(LPUSeqCandidate& repeat_candidate,
                 const LPUSeqHeader& loop_header,                 
                 unsigned pred_reg,
                 MachineOperand* in_stride_op,
                 unsigned strideOpcode,
                 const LPUInstrInfo &TII,
                 MachineBasicBlock& BB,
                 MachineInstr* prev_inst);


  // Add a repeat/onend pair fora memory dependency chain.
  // Returns the onend instruction.
  MachineInstr*
  seq_add_parloop_memdep(LPUSeqCandidate& memdepCandidate,
                         const LPUSeqHeader& loop_header,
                         unsigned pred_reg,
                         const LPUInstrInfo &TII,
                         MachineBasicBlock& BB,
                         MachineInstr* prev_inst);


  // Replaces either defs (and or uses) or a particular channel with
  // %ign.
  // Returns true if all defs in this instruction are %ign, and at least
  // one replacement has occurred (either of a use or def) has occurred.
  bool
  seq_replace_channel_with_ign(MachineInstr* MI,
                               unsigned channel_to_replace,
                               MachineRegisterInfo* MRI,
                               const TargetRegisterInfo &TRI,
                               bool replace_uses,
                               bool replace_defs);

  void
  seq_mark_loop_ins_for_deletion(LPUSeqLoopInfo& loop,
                                 MachineBasicBlock& BB,
                                 SmallVector<MachineInstr*, SEQ_VEC_WIDTH>& ins_for_deletion);
  
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
  DEBUG(errs() << "LPUSeqHeader: \npicker = " << header.pickerChannel);
  DEBUG(errs() << "\nswitcher = " << header.switcherChannel << "\n");
  if (header.pickerInit) {
    DEBUG(errs() << " pickerInit: " << *header.pickerInit << "");
  }
  else {
    DEBUG(errs() << " No pickerInit\n");
  }
  if (header.pickerMov1) {
    DEBUG(errs() << " pickerMov1: " << *header.pickerMov1 << "");
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

void LPUOptDFPass::
seq_debug_print_candidate(LPUSeqCandidate& x) {

  DEBUG(errs() << " pick = " << *x.pickInst);
  DEBUG(errs() << " switch = " << *x.switchInst);
  if (x.transformInst) {
    DEBUG(errs() << " transform = " << *x.transformInst << "\n");
  }
  switch (x.stype) {
  case LPUSeqCandidate::SeqType::UNKNOWN:
    DEBUG(errs() << "UNKNOWN type" << "\n");
    break;
  case LPUSeqCandidate::SeqType::REPEAT:
    DEBUG(errs() << "REPEAT: top = " << x.top
          << ", bottom = " << x.bottom << "\n");
    break;
  case LPUSeqCandidate::SeqType::REDUCTION:
    DEBUG(errs() << "REDUCTION: top = " << x.top
          << ", bottom = " << x.bottom << "\n");
    break;
  case LPUSeqCandidate::SeqType::STRIDE:
    DEBUG(errs() << "STRIDE: top = " << x.top
          << ", bottom = " << x.bottom << "\n");
    DEBUG(errs() << "stride op = " << *x.stride_op << "\n");
    break;
  case LPUSeqCandidate::SeqType::PARLOOP_MEM_DEP:
    DEBUG(errs() << "PARLOOP_MEM_DEP: top = " << x.top
          << ", bottom = " << x.bottom << "\n");
    break;
  case LPUSeqCandidate::SeqType::INVALID:
    DEBUG(errs() << "INVALID sequence type \n");
    break;
  }
  DEBUG(errs() << "\n");
}


bool LPUOptDFPass::seq_is_picker_init_inst(MachineRegisterInfo* MRI,
                                           MachineInstr* MI,
                                           unsigned* pickerChannel,
                                           bool* pickerSense) {
  if (MI->getOpcode() == LPU::INIT1) {
    DEBUG(errs() << "Found an init instruction " << *MI
          << "with " << MI->getNumOperands() << " operands \n");
    if (MI->getNumOperands() == 2) {
      MachineOperand& picker_def = MI->getOperand(0);
      MachineOperand& init_val = MI->getOperand(1);

      if (init_val.isImm()) {
        int ival = init_val.getImm();
        if ((ival == 0) || (ival == 1)) {
          DEBUG(errs() << "Found an init " << ival << " \n");
          if (picker_def.isReg()) {
            int pickval = picker_def.getReg();
            // TBD(jsukha): I can't figure out how to query the register
            // class of a channel here.  Bcause it is a physical
            // register, I can't seem to use the normal getRegClass methods. 
            // But I'm going to assume that knowing that the
            // opcode was INIT1 was enough...
            if (!TargetRegisterInfo::isVirtualRegister(pickval)) {
              *pickerChannel = pickval;
              *pickerSense = ival;
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
  bool pickerSense = 0;
  if (seq_is_picker_init_inst(MRI, MI,
                              &pickerChannel,
                              &pickerSense)) {
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

    // TBD(jsukha): In theory, we should be able to deal with loops
    // where the switcher control is inverted from the picker control.
    // But I'm ignoring this case for now.
    if (!seq_is_picker_mov_inst(MRI, pickerMov1,
                                pickerChannel,
                                &switcherChannel)) {
      // If that last definition is not instruction we want, bail.
      return false;
    }
    bool switcherSense = pickerSense;

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


    if (compareInst->getNumOperands() != 3) {
      DEBUG(errs() << " Stop. compare inst without 3 operands???" << "\n");
      return false;
    }

    // Finally, if we made it here, success!  Initialize the header.
    header->init(MI,
                 pickerMov1,
                 compareInst,
                 pickerChannel,
                 switcherChannel,
                 pickerSense,
                 switcherSense);
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

    // Print matches to cmp0 and cmp1 uses, if they exist.
    if (current_loop.cmp0Idx() >= 0) {

      DEBUG(errs() << "cmp0 matches candidate: \n");
      seq_debug_print_candidate(current_loop.candidates[current_loop.cmp0Idx()]);
    }
    else {
      DEBUG(errs() << "No cmp0_idx\n");
    }
    if (current_loop.cmp1Idx() >= 0) {
      DEBUG(errs() << "cmp1 matches candidate: \n");
      seq_debug_print_candidate(current_loop.candidates[current_loop.cmp1Idx()]);
    }
    else {
      DEBUG(errs() << "No cmp1_idx\n");
    }

    DEBUG(errs() << "Repeat channels: ");
    for (auto it = current_loop.repeat_channels.begin();
         it != current_loop.repeat_channels.end();
         ++it) {
      unsigned reg = it->getFirst();
      DEBUG(errs() << "(Reg= " << reg
            << ", idx= " << it->getSecond() << ") ");
    }
    DEBUG(errs() << "\n");

    DEBUG(errs() << "\n** All candidates **\n");
    // Dump the pick/switch pairs that we found.
    for (auto it = current_loop.candidates.begin();
         it != current_loop.candidates.end();
         ++it) {
      LPUSeqCandidate& x = *it;
      seq_debug_print_candidate(x);
    }

    DEBUG(errs() << "*****************\n");
  }
  DEBUG(errs() << "************************\n");  
}



// Returns MI if  MI is a pick instruction that matches the specified
// loop header.  Otherwise, returns NULL.
// Also saves MI into pickMap (keyed by loopback register) if a
// match is found.
//
// To be a match, this pick needs to use the same picker channel p as
// header.pickerChannel.
//
// For example, if header.pickerSense == 0, we will match the
// following pick: 
//
//   pick[n] top_val, p, init_val, loop_back
//
// If there is a match, then we save MI into pickMap, keyed on the
// register for loop_back.
MachineInstr*
LPUOptDFPass::
seq_candidate_match_pick(MachineInstr* MI,
                         const LPUSeqHeader& header, 
                         const LPUInstrInfo& TII,
                         DenseMap<unsigned, MachineInstr*>& pickMap) {
  if (TII.isPick(MI)) {
    assert(MI->getNumOperands() == 4);

    int pick_select_idx = LPUSeqHeader::pick_select_op_idx();
    MachineOperand& selectOp = MI->getOperand(pick_select_idx);

    // Figure out which op is the loopback based on the sense of the
    // header.
    int loopback_idx = header.pick_loopback_op_idx();
    MachineOperand& loopbackOp = MI->getOperand(loopback_idx);
            
    if (selectOp.isReg() && loopbackOp.isReg()) {
      unsigned select_reg = selectOp.getReg();
      unsigned loopback_reg = loopbackOp.getReg();
      if ((!TargetRegisterInfo::isVirtualRegister(select_reg)) &&
          (select_reg == header.pickerChannel) &&
          (!TargetRegisterInfo::isVirtualRegister(loopback_reg))) {
        DEBUG(errs() << "Found a pick candidate " << *MI <<
              " with loopback reg " << loopback_reg << "\n");
        if (pickMap.find(loopback_reg) != pickMap.end()) {
          DEBUG(errs() << "WARNING: found an existing pick ins " <<
                        *pickMap[loopback_reg] <<
                " with same loopback reg...\n");
        }
        else {
          // Success! save everything away.
          pickMap[loopback_reg] = MI;          
          return MI;
        }
      }
    }
  }
  return nullptr;
}


// Returns matching pick instruction if MI is a switch instruction
// that matches a pick in the specified loop.  Otherwise, returns
// NULL.
//
// A switch matches another pick if they share the same loopback
// register, and there is no other use of that loopback register.
MachineInstr*
LPUOptDFPass::
seq_candidate_match_switch(MachineInstr* MI,
                           const LPUSeqHeader& header,
                           const LPUInstrInfo& TII,
                           DenseMap<unsigned, MachineInstr*>& pickMap) {
  // Look for:
  //   switch[n] final, loopback, switcherChannel, bottom_val
  if (TII.isSwitch(MI)) {
    
    assert(MI->getNumOperands() == 4);
    int loopback_idx = header.switch_loopback_op_idx();
    MachineOperand& loopbackOp = MI->getOperand(loopback_idx);

    int switch_select_idx = LPUSeqHeader::switch_select_op_idx();
    MachineOperand& selectOp = MI->getOperand(switch_select_idx);

    if (selectOp.isReg() && loopbackOp.isReg()) {
      unsigned select_reg = selectOp.getReg();
      unsigned loopback_reg = loopbackOp.getReg();

      if ((!TargetRegisterInfo::isVirtualRegister(select_reg)) &&
          (select_reg == header.switcherChannel) &&
          (!TargetRegisterInfo::isVirtualRegister(loopback_reg))) {

        DEBUG(errs() << "Found possible switch candidate " <<
              *MI << " with loopback reg " << loopback_reg << "\n");

        if (pickMap.find(loopback_reg) == pickMap.end()) {
          DEBUG(errs() <<
                "WARNING: No match. No matching pick for this switch\n");
        }
        else {
          MachineInstr* matching_pick = pickMap[loopback_reg];

          // Finally, verify that the number of uses of the
          // loopback register is exactly 1, i.e., in the
          // pick.
          MachineRegisterInfo *MRI = &thisMF->getRegInfo();
          matching_pick = getSingleUse(loopback_reg, MRI);

          if (!matching_pick) {
            DEBUG(errs() <<
                  "WARNING: No match.  Found other uses of loopback register "
                  << loopback_reg << "\n" );
          }
          return matching_pick;
        }
      }
    }
  }
  return nullptr;
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

        // Walk over uses of the picker channel, storing the matching picks.
        for (auto it = MRI->use_instr_begin(current_loop.header.pickerChannel);
             it != MRI->use_instr_end();
             ++it) {
          MachineInstr* MI = &(*it);
          seq_candidate_match_pick(MI,
                                   current_loop.header, 
                                   TII,
                                   pickMap);
        }

        // Now walk over the uses of the corresponding switcher
        // channel.  Look for a matching pick (based on the loopback
        // channel).
        for (auto it =
               MRI->use_instr_begin(current_loop.header.switcherChannel);
             it != MRI->use_instr_end();
             ++it) {
          MachineInstr* MI = &(*it);

          // Look for a switch that matches some pick we found
          // earlier.  If we find a match, save the candidate.
          MachineInstr* matching_pick = seq_candidate_match_switch(MI,
                                                                   current_loop.header,
                                                                   TII,
                                                                   pickMap);
          if (matching_pick) {
            LPUSeqCandidate nc = LPUSeqCandidate(matching_pick,
                                                 MI);
            current_loop.candidates.push_back(nc);
          }
        }

        // Initialize other state information about the loop after we
        // have added the relevant information about loop candidates.
        current_loop.init_from_header();
        loops->push_back(current_loop);
      }
      else {
        //        DEBUG(errs() << "No match for header instruction " << *MI << "\n");
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
    
    // Only print after classification now. 
    //    seq_print_loop_info(&loops);

    DEBUG(errs() << "Classifying sequence op types\n");
    seq_classify_candidates(&loops);
    DEBUG(errs() << "After classification: \n");
    seq_print_loop_info(&loops);

    DEBUG(errs() << "Done with sequence classification\n");
    if (seq_opt_level > 1) {
      // Actually do the transforms.

      std::set<MachineInstr*> insSetMarkedForDeletion;
      // 
      // TBD(jsukha): Fill in the code that does the sequence
      // transformations.
      DEBUG(errs() << "TBD: Performing sequence optimizations\n");

      int num_transformed = 0;
      int loop_count = 0;
      for (auto it = loops.begin();
           it != loops.end();
           ++it) {
        LPUSeqLoopInfo& loop = *it;
        bool success = seq_try_transform_loop(loop,
                                              insSetMarkedForDeletion);
        DEBUG(errs() << "Successful transform of loop " << loop_count
              << "? : " << success << "\n");
        if (success)
          num_transformed++;
        loop_count++;

        if (num_transformed > SequenceMaxPerLoop) {
          DEBUG(errs() << "Reached transform loop limit. Stopping\n");
          break;
        }
      }
      DEBUG(errs() << "Done with seq opt. Transformed "
            <<  num_transformed << " loops\n");

      // Finally delete the instructions we found.
      int num_deleted = 0;
      for (auto it = insSetMarkedForDeletion.begin();
           it != insSetMarkedForDeletion.end();
           ++it) {
        num_deleted++;
        MachineInstr* MI = *it;
        assert(MI);
        DEBUG(errs() << "Final deletion: ins = " << *MI << "\n");
        MI->eraseFromParent();
      }
      DEBUG(errs() << "Final deletion: " << num_deleted
            << " instructions.\n");
    }
  }
  else {
    DEBUG(errs() << "Sequence optimizations disabled\n");
  }
}


// Return the MachineInstr* if it is the single def of the Reg.
// This method is a simplfication of the method implemented in
// TwoAddressInstructionPass
MachineInstr *
LPUOptDFPass::getSingleDef(unsigned Reg,
                           const MachineRegisterInfo *MRI) {
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

// Return the MachineInstr* if it is the single use of the Reg.  This
// method is analogous to to the one above for definitions.
MachineInstr *
LPUOptDFPass::getSingleUse(unsigned Reg,
                           const MachineRegisterInfo *MRI) {
  MachineInstr *Ret = nullptr;
  for (MachineInstr &UseMI : MRI->use_instructions(Reg)) {
    if (UseMI.isDebugValue())
      continue;
    if (!Ret)
      Ret = &UseMI;
    else if (Ret != &UseMI)
      return nullptr;
  }
  return Ret;
}



LPUSeqCandidate::SeqType
LPUOptDFPass::
seq_classify_repeat_or_reduction(LPUSeqCandidate& x) {
  assert(x.pickInst && x.switchInst);

  // Example: 
  // %CI64_10 = PICK64 %CI1_0, %CI64_11, %CI64_12
  // %IGN, %CI64_12 = SWITCH64 %CI1_8, %CI64_13

  // Get the source of the switch. In the example above, this register
  // is %CI64_13.
  MachineOperand* bottom_op = x.get_switch_bottom_op();
  MachineOperand* top_op = x.get_pick_top_op();
  
  if (bottom_op->isReg() &&
      (!TargetRegisterInfo::isVirtualRegister(bottom_op->getReg())) &&
      top_op->isReg() &&
      (!TargetRegisterInfo::isVirtualRegister(top_op->getReg()))) {

    unsigned bottom_channel = bottom_op->getReg();
    unsigned top_channel = top_op->getReg();
    MachineRegisterInfo *MRI = &thisMF->getRegInfo();  
    // Look at the instruction that defines bottom_op.
    MachineInstr* def_bottom = getSingleDef(bottom_channel, MRI);

    // First, if the defining instruction is the pick itself, then
    // there is no transform body.  We have a repeat.
    if (def_bottom == x.pickInst) {
      assert(top_channel == bottom_channel);
      x.stype = LPUSeqCandidate::SeqType::REPEAT;
      x.transformInst = NULL;
      x.top = top_channel;
      x.bottom = bottom_channel;
      return LPUSeqCandidate::SeqType::REPEAT;
    } 

    if (!def_bottom)
      return LPUSeqCandidate::SeqType::UNKNOWN;

    // Next, look for reductions or sequence values.
    // To find these transforming bodies, we want
    //  a single add/sub instruction, which
    //  uses top_op's definition as one of its inputs. 

    const LPUInstrInfo &TII =
      *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());

    if ((TII.isAdd(def_bottom)) || (TII.isSub(def_bottom))) {
      // For a reduction, we need no other uses of top_op or
      // bottom_op, except in the add.
      if ((getSingleUse(top_channel, MRI) == def_bottom) &&
          (getSingleUse(bottom_channel, MRI) == x.switchInst)) {
        DEBUG(errs() << "Found reduction transform body " <<
              *def_bottom << "\n");

        x.stype = LPUSeqCandidate::SeqType::REDUCTION;
        x.transformInst = def_bottom;
        x.top = top_channel;
        x.bottom = bottom_channel;
        return LPUSeqCandidate::SeqType::REDUCTION;
      }
    }
  }
  return LPUSeqCandidate::SeqType::UNKNOWN;
}



LPUSeqCandidate::SeqType
LPUOptDFPass::
seq_classify_stride(LPUSeqCandidate& x,
                    const DenseMap<unsigned, int>& repeat_channels) {
  assert(x.pickInst && x.switchInst);
  MachineOperand* bottom_op = x.get_switch_bottom_op();
  MachineOperand* top_op = x.get_pick_top_op();
  
  if (bottom_op->isReg() &&
      (!TargetRegisterInfo::isVirtualRegister(bottom_op->getReg())) &&
      top_op->isReg() &&
      (!TargetRegisterInfo::isVirtualRegister(top_op->getReg()))) {

    unsigned bottom_channel = bottom_op->getReg();
    unsigned top_channel = top_op->getReg();
    MachineRegisterInfo *MRI = &thisMF->getRegInfo();  
    // Look at the instruction that defines bottom_op.
    MachineInstr* def_bottom = getSingleDef(bottom_channel, MRI);

    if (!def_bottom)
      return LPUSeqCandidate::SeqType::UNKNOWN;
    
    const LPUInstrInfo &TII =
      *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());

    if ((TII.isAdd(def_bottom)) || (TII.isSub(def_bottom))) {

      // Bail out if our add instruction doesn't match one of our
      // known striding operations yet.
      unsigned stride_opcode;
      if (!seq_compute_matching_stride_opcode(def_bottom->getOpcode(),
                                              &stride_opcode)) {
        DEBUG(errs() << "WARNING: stride operation for transform "
              << *def_bottom  << " not implemented yet...\n");
        return LPUSeqCandidate::SeqType::INVALID;
      }
      
      if (def_bottom->getNumOperands() == 3) {

        // First, figure out the potential stride, by looking for the
        // top input.
        int stride_idx = 0;
        MachineOperand& add0 = def_bottom->getOperand(1);
        MachineOperand& add1 = def_bottom->getOperand(2);
        
        if (add0.isReg() && (add0.getReg() == top_channel)) {
          stride_idx = 2;
        }
        else if (add1.isReg() && (add1.getReg() == top_channel)) {
          stride_idx = 1;
        }
        else {
          // Neither matches top. We have something weird. 
          DEBUG(errs() << "Add inst " << *def_bottom
                << " doesn't match sequence we expect.\n");
          return LPUSeqCandidate::SeqType::UNKNOWN;
        }

        MachineOperand& stride_op = def_bottom->getOperand(stride_idx);
        if (stride_op.isImm() ||
            (stride_op.isReg() &&
             (repeat_channels.find(stride_op.getReg())
              != repeat_channels.end()))) {
          x.top = top_channel;
          x.bottom = bottom_channel;
          x.stride_op = &stride_op;
          x.stype = LPUSeqCandidate::SeqType::STRIDE;
          x.transformInst = def_bottom;
          return LPUSeqCandidate::SeqType::STRIDE;
        }
      }
      else {
        DEBUG(errs() << "Found weird add/sub " << *def_bottom
              << " does not have 3 operands. Skipping\n");
      }
    }
  }
  return LPUSeqCandidate::SeqType::UNKNOWN;
}



LPUSeqCandidate::SeqType
LPUOptDFPass::
seq_classify_memdep_graph(LPUSeqCandidate& x) {
  assert(x.pickInst && x.switchInst);
  MachineOperand* bottom_op = x.get_switch_bottom_op();
  MachineOperand* top_op = x.get_pick_top_op();


  if ((x.pickInst->getOpcode() == LPU::PICK1) &&
      (x.switchInst->getOpcode() == LPU::SWITCH1) &&
      bottom_op->isReg() &&
      top_op->isReg()) {

    unsigned source_reg = bottom_op->getReg();
    unsigned sink_reg = top_op->getReg();

    // If the knob setting is 2, just ASSUME we have a memory
    // dependency here, instead of trying to walk the graph to verify
    // we have one.  What could possibly go wrong here?
    if (SeqBreakMemdep >= 2) {
      DEBUG(errs() << "ASSUMED we have a memdep candidate.\n");
      DEBUG(errs() << "The flag was set.. it is not my fault if it doesn't work!\n");
      x.stype = LPUSeqCandidate::SeqType::PARLOOP_MEM_DEP;
      x.transformInst = NULL;
      x.top = sink_reg;
      x.bottom = source_reg;
      return x.stype;
    }

    // Otherwise, we are going to attempt to verify that we have a
    // dependency chain of memory operations via a BFS, which tries to
    // walk back from the switch to the pick.
    //
    // We are walking back through the def/use through MOV1, MERGE1,
    // PICK1, SWITCH1, and any OLD* or OST* instructions.
    
    MachineRegisterInfo *MRI = &thisMF->getRegInfo();
    const LPUInstrInfo &TII =
    *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
    
    const int MAX_LEVELS = 10000;
    int num_levels = 0;

    // Two frontiers for the BFS.
    std::set<MachineInstr*> b0;
    std::set<MachineInstr*> b1;

    MachineInstr* first_inst = getSingleDef(source_reg, MRI);
    if (first_inst) {
      b0.insert(first_inst);
    }
    // Pointers to the frontier vectors.
    std::set<MachineInstr*>* p_current = &b0;
    std::set<MachineInstr*>* p_next = &b1;

    while ((p_current->size() > 0) &&
           (num_levels < MAX_LEVELS)) {
      for (std::set<MachineInstr*>::iterator it = p_current->begin();
           it != p_current->end();
           ++it) {
        MachineInstr* MI = *it;
        
        DEBUG(errs() << " MemGraph processing: current ins = "
              <<  *MI << "\n");
        
        if (MI == x.pickInst) {
          if ((p_current->size() == 1) &&
              (p_next->size() == 0)) {
            DEBUG(errs() << "Found memdep candidate\n");
            x.stype = LPUSeqCandidate::SeqType::PARLOOP_MEM_DEP;
            x.transformInst = NULL;
            x.top = sink_reg;
            x.bottom = source_reg;
            return x.stype;
          }
          else {
            // Ignore this pick for now.  The pick can be reached from
            // multiple paths, and we want each one of them to end up
            // here.
            DEBUG(errs() << "Reached pick, but frontier not empty yet. Maybe other paths\n");
          }
        }
        else {
          // Walk backwards from the current instruction, and look for
          unsigned current_op = MI->getOpcode();
          MachineOperand* nextOp[2];
          nextOp[0] = NULL;
          nextOp[1] = NULL;
          int num_ops = 0;
          
          // Handle 3 different types of instructions.  Look up the
          // previous op in the chain.
          if (current_op == LPU::MOV1) {
            assert(MI->getNumOperands() == 2);
            nextOp[0] = &MI->getOperand(1);
            num_ops = 1;
          }
          else if (current_op == LPU::MERGE1) {
            assert(MI->getNumOperands() == 3);
            nextOp[0] = &MI->getOperand(1);
            nextOp[1] = &MI->getOperand(2);
            num_ops = 2;
          }
          else if (current_op == LPU::PICK1) {
            assert(MI->getNumOperands() == 4);
            nextOp[0] = &MI->getOperand(2);
            nextOp[1] = &MI->getOperand(3);
            num_ops = 2;
          }
          else if (current_op == LPU::SWITCH1) {
            assert(MI->getNumOperands() == 4);
            nextOp[0] = &MI->getOperand(3);
            num_ops = 1;
          }
          else if (TII.isOrderedLoad(MI) ||
                   TII.isOrderedStore(MI)) {
            int num_operands = MI->getNumOperands();
            nextOp[0] = &MI->getOperand(num_operands-1);
            num_ops = 1;
          }

          if (num_ops > 0) {
            for (int i = 0; i < num_ops; ++i) {
              if (nextOp[i]->isReg()) {
                unsigned next_reg = nextOp[i]->getReg();
                if ((next_reg != LPU::IGN) &&
                    TargetRegisterInfo::isPhysicalRegister(next_reg)) {
                  MachineInstr* def_inst = getSingleDef(next_reg, MRI);
                  if (def_inst) {
                    p_next->insert(def_inst);
                    continue;
                  }
                }
              }
              DEBUG(errs() << "Unknown op folloing chain, in "
                    << *MI << ". Can't match\n");
              return LPUSeqCandidate::SeqType::UNKNOWN;
            }
          }
          else {
            DEBUG(errs() << "Could not follow chain of memory ops."
                  << *MI << ".  Can't match\n");
            return LPUSeqCandidate::SeqType::UNKNOWN;
          }
        }
      }
      num_levels++;

      p_current->clear();
      // Swap current and next, for next level in BFS.
      std::swap(p_current, p_next);
    }

    DEBUG(errs() << "Falling through. stopping chain after "
          << num_levels << " levels of searching...\n");
  }
  return LPUSeqCandidate::SeqType::UNKNOWN;
}


// Look for a match between bottom and the cmp0/cmp1 channels.
// If we find a match, save the result in the current loop.
//
// Returns true if we found a match, false otherwise.
inline bool update_header_cmp_channels(LPUSeqLoopInfo& current_loop,
                                       int loop_idx,
                                       unsigned bottom) {
  LPUSeqLoopInfo::CmpMatchType ctype;
  // This method in LPUSeqLoopInfo does all the hard work.
  // The remainder of this method is just error reporting.
  ctype = current_loop.match_candidate_with_cmp(bottom, loop_idx);

  switch (ctype) {
  case LPUSeqLoopInfo::CmpMatchType::Match0:
  case LPUSeqLoopInfo::CmpMatchType::Match1:    
    return true;
    
  case LPUSeqLoopInfo::CmpMatchType::Dup0:
    DEBUG(errs() << "WARNING: Finding duplicate seq def for cmp0. Ignoring\n");
    DEBUG(errs() << "Duplicate def of " << bottom <<
          " is at idx " << current_loop.cmp0Idx() << "\n");
    return false;

  case LPUSeqLoopInfo::CmpMatchType::Dup1:
    DEBUG(errs() << "WARNING: Finding duplicate seq def for cmp1. Ignoring\n");
    DEBUG(errs() << "Duplicate def of " << bottom <<
          " is at idx " << current_loop.cmp1Idx() << "\n");
    return false;

  case LPUSeqLoopInfo::CmpMatchType::NoMatch:
    return false;
    
  case LPUSeqLoopInfo::CmpMatchType::OtherError:
  default:
    DEBUG(errs() << "ERROR: encountering bad bottom channel in loop...\n");
    assert(0);
  }
}

// Classify all the candidate sequences in the loops we found.
void
LPUOptDFPass::
seq_classify_candidates(SmallVector<LPUSeqLoopInfo, SEQ_VEC_WIDTH>* loops) {

  if (loops) {
    for (unsigned i = 0; i < loops->size(); ++i) {
      LPUSeqLoopInfo& current_loop = (*loops)[i];

      SmallVector<LPUSeqCandidate, SEQ_VEC_WIDTH> classified;
      SmallVector<LPUSeqCandidate, SEQ_VEC_WIDTH> remaining;
      SmallVector<LPUSeqCandidate, SEQ_VEC_WIDTH> repeat_candidates;

      // First pass: look for repeat / reduction.
      for (auto it = current_loop.candidates.begin();
           it != current_loop.candidates.end();
           ++it) {
        LPUSeqCandidate& x = *it;
        LPUSeqCandidate::SeqType stype;

        stype = seq_classify_repeat_or_reduction(x);

        if (stype == LPUSeqCandidate::SeqType::REPEAT) {
          // Save the channel into our list and map of repeat
          // candidates.    The index in this
          // repeat_candidate vector will be the same as the
          // index in the final vector. 
          int idx = repeat_candidates.size();
          repeat_candidates.push_back(x);
          current_loop.repeat_channels[x.bottom] = idx;

          // Look for a match for this repeat in the comparison
          // channels.
          update_header_cmp_channels(current_loop,
                                     idx,
                                     x.bottom);
        }
        else if (stype == LPUSeqCandidate::SeqType::UNKNOWN) {
          remaining.push_back(x);
        }
        else {
          classified.push_back(x);
        }
      }

      DEBUG(errs() << "Repeat candidates: " << repeat_candidates.size() << "\n");
      DEBUG(errs() << "Classified candidates: " << classified.size() << "\n");
      DEBUG(errs() << "Remaining candidates: " << remaining.size() << "\n");
      DEBUG(errs() << "Current loop candidates: "
            << current_loop.candidates.size() << "\n");

      
      // Move the repeat and classified candidates over into the
      // current loop.  Along the way, look for matches with the
      // operands in the compare.
      current_loop.candidates.clear();

      // Move all the repeats back into candidates.
      current_loop.candidates.insert(current_loop.candidates.end(),
                                     repeat_candidates.begin(),
                                     repeat_candidates.end());
      repeat_candidates.clear();

      // Next, move each of the classified candidates into the
      // candidates.
      for (int i = 0; i < (int)classified.size(); ++i) {
        LPUSeqCandidate& x = classified[i];

        int idx = current_loop.candidates.size();
        current_loop.candidates.push_back(x);
        update_header_cmp_channels(current_loop,
                                   idx, 
                                   x.bottom);


      }
      classified.clear();


      DEBUG(errs() << "After: Classified candidates: " << classified.size() << "\n");
      DEBUG(errs() << "After: Remaining candidates: " << remaining.size() << "\n");
      DEBUG(errs() << "After: Current loop candidates: "
            << current_loop.candidates.size() << "\n");

      // Second pass: look for strides in the remaining candidates.
      for (auto it = remaining.begin();
           it != remaining.end();
           ++it) {
        LPUSeqCandidate& x = *it;
        LPUSeqCandidate::SeqType stype;
        stype = seq_classify_stride(x,
                                    current_loop.repeat_channels);


        // Try to classify memory dependency candidates, if the knob
        // is set.
        if (SeqBreakMemdep > 0) {
          if (stype == LPUSeqCandidate::SeqType::UNKNOWN) {
            stype = seq_classify_memdep_graph(x);
          }
        }

        // For the second pass, classified takes on the role of
        // "remaining", and current_loop will just take in the
        // remaining candidates.
        if ((stype == LPUSeqCandidate::SeqType::UNKNOWN) ||
            (stype == LPUSeqCandidate::SeqType::INVALID)) {
          // Mark the remaining candidates as invalid.
          x.stype = LPUSeqCandidate::SeqType::INVALID;
          classified.push_back(x);
        }
        else {
          int loop_idx = current_loop.candidates.size();
          current_loop.candidates.push_back(x);
          // Again, look for a match with this sequence and the
          // compare instruction.
          update_header_cmp_channels(current_loop,
                                     loop_idx,
                                     x.bottom);
        }
      }

      // TBD(jsukha): Do we keep the invalid candidates around?
      // I guess I'll do it for now...
      current_loop.candidates.insert(current_loop.candidates.end(),
                                     classified.begin(),
                                     classified.end());

      DEBUG(errs() << "Final: Invalid candidates: "
            << classified.size() << "\n");
      DEBUG(errs() << "Final: All loop candidates: "
            << current_loop.candidates.size() << "\n");
      DEBUG(errs() << "Repeat channels found: "
            << current_loop.repeat_channels.size() << "\n");
    }
  }
}


bool 
LPUOptDFPass::
seq_try_transform_loop(LPUSeqLoopInfo& loop,
                       std::set<MachineInstr*>& insSetMarkedForDeletion) {
  
  // Found a valid induction variable. 
  // True if induction variable (i.e., first argument in the compare)
  // is a stride candidate.
  bool found_indvar = false;
  // Index in the loop.candidates vector that identifies a sequence
  // candidate.
  int indvarIdx;
  int boundIdx;
  // Operand index in the comparison instruction for the bound. 
  int boundOpIdx;

  // This comparison sense will be 0 if the original compare is
  //   compare induction_var, stride
  // and 1 if we have
  //   compare stride, induction_var
  //
  bool compare_sense;
  
  // Two cases.  Look for the stride (induction variable) in either
  // cmp0 or cmp1.
  if ((loop.cmp0Idx() >= 0) &&
      loop.candidates[loop.cmp0Idx()].stype == LPUSeqCandidate::SeqType::STRIDE) {
    found_indvar = true;
    indvarIdx = loop.cmp0Idx();
    boundIdx = loop.cmp1Idx();
    
    // Operand index in the comparison instruction for the bound.
    boundOpIdx = 2;
    compare_sense = false;
  }
  else if ((loop.cmp1Idx() >= 0) &&
           loop.candidates[loop.cmp1Idx()].stype == LPUSeqCandidate::SeqType::STRIDE) {
    found_indvar = true;
    indvarIdx = loop.cmp1Idx();
    boundIdx = loop.cmp0Idx();
    boundOpIdx = 1;
    compare_sense = true;
  }

  if (!found_indvar) {
    DEBUG(errs() << "Seq transform failed: invalid induction variable.\n");
    return false;
  }

  // There are two cases where we will need to swap comparison of the
  // sequence from the direction of the original compare.

  //
  // 1. If the comparison sense is inverted (i.e., we have compare
  //    stride, var), then we need to invert the comparison.
  //
  // 2. If the switcher sense is 1 (so it expects 0, 0, 0,
  // ... 1 for its control), then we need to invert the comparison.
  //
  // Both conditions together will cancel each other out.  Thus, the
  // final booleans
  bool invert_compare = compare_sense ^ (loop.header.switcherSense);

  LPUSeqCandidate& indvarCandidate = loop.candidates[indvarIdx];
  
  // Found a valid loop bound.
  // This can either be a repeated channel, or an immediate.
  bool found_bound_channel = 
    ((boundIdx >= 0) &&
     loop.candidates[boundIdx].stype == LPUSeqCandidate::SeqType::REPEAT);
  bool found_bound_imm =
    ((boundIdx == -1) &&
     loop.header.compareInst->getOperand(boundOpIdx).isImm());
  bool found_bound = found_bound_channel || found_bound_imm;
  
  if (!found_bound) {
    DEBUG(errs() << "Seq transform failed: possible non-constant loop.\n");
    DEBUG(errs() << "Boundidx = " << boundIdx << "\n");
    if (boundIdx >= 0) {
      seq_debug_print_candidate(loop.candidates[boundIdx]);

    }
    return false;
  }


  // Compute a matching opcode for the compare and transformation
  // instruction.  
  unsigned indvar_opcode = 0;
  unsigned compare_opcode = loop.header.compareInst->getOpcode();
  unsigned transform_opcode = indvarCandidate.transformInst->getOpcode();

  if (seq_compute_matching_seq_opcode(compare_opcode,
                                      transform_opcode,
                                      invert_compare, 
                                      &indvar_opcode)) {
    DEBUG(errs() << "Can do sequence transform of induction variable!\n");

    seq_do_transform_loop(loop,
                          indvarCandidate,
                          indvar_opcode,
                          indvarIdx,
                          boundIdx,
                          boundOpIdx, 
                          insSetMarkedForDeletion);
    
    return true;
  }

  // If we fall through to here, then there may be some kind of
  // sequence we haven't implemented yet.
  DEBUG(errs() << "WARNING: possible sequence opcode not implemented yet.\n");
  DEBUG(errs() << "  induction variable candidate is: ");
  seq_debug_print_candidate(indvarCandidate);
  
  return false;
}




// TBD(jsukha): FIX ME!
//
// NOTE: This method hard-codes two specific pairs of (compare,
// transform) opcode pairs for the sequence optimization.
//
// These values are hard-coded so that we can get some of our
// initial test examples working.  
//
// But we eventually need to allow for a larger set of matching pairs.
//
bool
LPUOptDFPass::
seq_compute_matching_seq_opcode(unsigned ciOp,
                                unsigned tOp,
                                bool invert_compare,
                                unsigned* indvar_opcode) {
  const LPUInstrInfo &TII =
    *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());

  // Invert the comparison opcode if needed.
  unsigned compareOp = ciOp;
  if (invert_compare) {
    compareOp = TII.commuteCompareOpcode(ciOp);
  }

  // Find a sequence opcode that matches our compare opcode.
  unsigned seqOp = TII.convertCompareOpToSeqOTOp(compareOp);
  if (seqOp != compareOp) {

    // If we have a matching sequence op, then check that the
    // transforming op matches as well.

    switch(tOp) {
    case LPU::ADD8:
      *indvar_opcode = TII.promoteSeqOTOpBitwidth(seqOp, 8);
      return true;      
    case LPU::ADD16:
      *indvar_opcode = TII.promoteSeqOTOpBitwidth(seqOp, 16);      
      return true;      
    case LPU::ADD32:
      *indvar_opcode = TII.promoteSeqOTOpBitwidth(seqOp, 32);            
      return true;      
    case LPU::ADD64:
      *indvar_opcode = TII.promoteSeqOTOpBitwidth(seqOp, 64);                  
      return true;
    }
  }

  return false;
}


bool
LPUOptDFPass::
seq_compute_matching_stride_opcode(unsigned transformOpcode,
                                   unsigned* strideOpcode) {

  switch (transformOpcode) {
  case LPU::ADD64:
    *strideOpcode = LPU::STRIDE64;
    return true;

  case LPU::ADD32:
    *strideOpcode = LPU::STRIDE32;
    return true;
    
  case LPU::ADD16:
    *strideOpcode = LPU::STRIDE16;
    return true;

  case LPU::ADD8:
    *strideOpcode = LPU::STRIDE8;
    return true;
    
  default:
    // No match. return false. 
    return false;
  }
}


// Returns true if all defs in this instruction are %ign, and at least
// one replacement has occurred (either of a use or def) has occurred.
bool LPUOptDFPass::
seq_replace_channel_with_ign(MachineInstr* MI,
                             unsigned channel_to_replace,
                             MachineRegisterInfo* MRI,
                             const TargetRegisterInfo &TRI,
                             bool replace_uses, 
                             bool replace_defs) {
  int num_replacements = 0;
  int num_defs = 0;
  int num_null_defs = 0;
  
  assert(channel_to_replace != LPU::IGN);
  for (MachineOperand &MO : MI->operands()) {
    if (MO.isReg()) {
      unsigned creg = MO.getReg();
      if (MO.isDef()) {
        num_defs++;
        if (replace_defs && (creg == channel_to_replace)) {
          MO.substPhysReg(LPU::IGN, TRI);
          num_replacements++;
          num_null_defs++;
        }
        else if (creg == LPU::IGN) {
          num_null_defs++;
        }
      }
      else if (replace_uses && MO.isUse()) {
        if (creg == channel_to_replace) {
          MO.substPhysReg(LPU::IGN, TRI);
          num_replacements++;
        }
      }
    }
  }

  // This instruction can be deleted if all its definitions are NULL.
  //
  // TBD: Should we care if any replacements happened?
  // What should we do with an instruction that we are passed in which has
  // all its def's as %ign to start with? 
  return ((num_replacements > 0) && (num_null_defs == num_defs));
}


MachineInstr*
LPUOptDFPass::
seq_add_parloop_memdep(LPUSeqCandidate& sc,
                       const LPUSeqHeader& loop_header,                       
                       unsigned pred_reg,
                       const LPUInstrInfo &TII,
                       MachineBasicBlock& BB,
                       MachineInstr* prev_inst) {
  assert(sc.stype == LPUSeqCandidate::SeqType::PARLOOP_MEM_DEP);
  MachineInstr* repinst =
    BuildMI(BB,
            prev_inst,
            sc.pickInst->getDebugLoc(),
            TII.get(LPU::REPEAT8),  // TBD: We technically want repeat1?
            sc.top).
    addReg(pred_reg).
    addOperand(*sc.get_pick_input_op(loop_header));
  repinst->setFlag(MachineInstr::NonSequential);


  MachineOperand* out_s_op = sc.get_switch_output_op(loop_header);
  assert(out_s_op->isReg());
  
  MachineInstr* onend_inst =
    BuildMI(BB,
            repinst,
            sc.switchInst->getDebugLoc(),
            TII.get(LPU::ONEND),
            out_s_op->getReg()).
    addReg(pred_reg).
    addReg(sc.bottom);
  onend_inst->setFlag(MachineInstr::NonSequential);

  return onend_inst;
}

MachineInstr*
LPUOptDFPass::
seq_add_repeat(LPUSeqCandidate& sc,
               const LPUSeqHeader& loop_header,
               unsigned pred_reg,
               const LPUInstrInfo &TII,
               MachineBasicBlock& BB,
               MachineInstr* prev_inst) {

  assert(sc.stype == LPUSeqCandidate::SeqType::REPEAT);

  // TBD(jsukha): FIX ME! Right now, the compiler is only inserting
  // REPEAT64 instructions for all repeat candidates, instead of
  // figuring out the correct size of repeat to insert.
  // (We need to map pick/switch opcode to repeat opcode).
  MachineInstr* repinst =
    BuildMI(BB,
              prev_inst,
              sc.pickInst->getDebugLoc(),
              TII.get(LPU::REPEAT64),
              sc.top).
      addReg(pred_reg).
    addOperand(*sc.get_pick_input_op(loop_header));
  repinst->setFlag(MachineInstr::NonSequential);
  return repinst;
}



MachineInstr*
LPUOptDFPass::
seq_add_stride(LPUSeqCandidate& sc,
               const LPUSeqHeader& loop_header,               
               unsigned pred_reg,
               MachineOperand* in_stride_op,
               unsigned strideOpcode,
               const LPUInstrInfo &TII,
               MachineBasicBlock& BB,
               MachineInstr* prev_inst) {
  assert(sc.stype == LPUSeqCandidate::SeqType::STRIDE);
  assert(in_stride_op);
  MachineInstr* strideInst =
    BuildMI(BB,
            prev_inst,
            sc.pickInst->getDebugLoc(),
            TII.get(strideOpcode),
            sc.top).
    addReg(pred_reg).
    addOperand(*sc.get_pick_input_op(loop_header)).
    addOperand(*in_stride_op);
  strideInst->setFlag(MachineInstr::NonSequential);
  return strideInst;
}


MachineInstr*
LPUOptDFPass::
seq_add_output_switch_for_seq_candidate(LPUSeqCandidate& sCandidate,
                                        const LPUSeqHeader& loop_header,
                                        unsigned last_reg,
                                        const LPUInstrInfo &TII,
                                        MachineBasicBlock& BB,
                                        MachineInstr* prev_inst) {
  MachineInstr* output_switch = NULL;
  MachineOperand* out_s_op = sCandidate.get_switch_output_op(loop_header);

  if (!(out_s_op->isReg() &&
        (out_s_op->getReg() == LPU::IGN))) {
    assert(sCandidate.bottom > 0);

    output_switch =
      BuildMI(BB,
              prev_inst,
              sCandidate.switchInst->getDebugLoc(),
              TII.get(sCandidate.switchInst->getOpcode()),
              LPU::IGN).
      addReg(out_s_op->getReg(), RegState::Define).
      addReg(last_reg).
      addReg(sCandidate.bottom);

    output_switch->setFlag(MachineInstr::NonSequential);
  }
  return output_switch;
}



MachineOperand*
LPUOptDFPass::
seq_lookup_stride_op(LPUSeqLoopInfo& loop,
                     LPUSeqCandidate& scandidate) {
  // For stride, first look in the sequence candidate op.
  //
  // If this stride op is a LIC (instead of an immediate), we 
  // look up the input of the matching repeat instead.
  //
  // Otherwise, it it should be a literal operand.
  MachineOperand* in_s_op = scandidate.stride_op;
  LPUSeqCandidate* stride_repeat = NULL;
  if (in_s_op->isReg()) {
    unsigned bottom_s_reg = in_s_op->getReg();
    if (loop.repeat_channels.find(bottom_s_reg) !=
        loop.repeat_channels.end()) {
      unsigned stride_idx = loop.repeat_channels[bottom_s_reg];
      assert(stride_idx < loop.candidates.size());
      stride_repeat = &loop.candidates[stride_idx];
      in_s_op = stride_repeat->get_pick_input_op(loop.header);
    }
    else {
      // We should have matched the register for this stride op with a
      // repeat earlier.
      DEBUG(errs() << "ERROR: can't find repeat channel for stride...\n");
      return NULL;
    }
  }
  return in_s_op;
}


// Deletes any instructions stored in insMarkedForDeletion.  Also may
// find new instructions to remove, for any unused channels we may
// have created, and push them into insMarkedForDeletion.
//
// NOTE: insMarkedForDeletion is NOT guaranteed to have unique
// elements after this call.
//
// Two cases for any instruction I in insMarkedForDeletion:
//
//   1. For any channel x defined by I, replace all uses of x with
//      %ign, IF I is the ONLY definition of x.
// 
//      In a correct dataflow program, we expect exactly one
//      definition of x.  However, this specification allows us to add
//      an extra definition of x, and then delete the old one.
//
//   2. For any channel x used by I, if I is the only use of x, then
//      replace all definitions of x in all defining instructions D.
//      If a defining instruction D has no other definitions, also
//      mark D for deletion.
//
// 
// TBD(jsukha): WARNING: It is not clear whether this method can be a
// general cleanup /deletion method.
//
// In theory, in step 1, if we have replaced a use of x with %ign in
// instruction K, then (depending on the kind of instruction K is),
// that instruction K may become invalid.  Only some instructions can
// allow some of their uses to be %ign.  Should we continue to mark
// "K" for deletion?
//
// For example, if we change "add %ci64_0, %ci64_1, %ci64_2" into
//            "add $ci64_0, %ign, %ci64_2"
//
// we have technically changed the meaning of the program.  In the
//  worst case, this program may no longer be correct (e.g., if we had
//  an add where the "%ci64_2" is replaced with a literal).
//
void
LPUOptDFPass::
seq_mark_loop_ins_for_deletion(LPUSeqLoopInfo& loop,
                               MachineBasicBlock& BB,
                               SmallVector<MachineInstr*, SEQ_VEC_WIDTH>& insMarkedForDeletion) {
  const TargetMachine &TM = thisMF->getTarget();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();  
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();

  DEBUG(errs() << "TBD: Initial set of instructions to delete: \n");
  for (auto it = insMarkedForDeletion.begin();
       it != insMarkedForDeletion.end();
       ++it) {
    MachineInstr* mi = *it;
    DEBUG(errs() << "    " << *mi);
  }


  SmallVector<MachineInstr*, SEQ_VEC_WIDTH> deleteQ;
  deleteQ.insert(deleteQ.end(),
                 insMarkedForDeletion.begin(),
                 insMarkedForDeletion.end());

  while (deleteQ.size() > 0) {
    MachineInstr* cMI = deleteQ.back();
    deleteQ.pop_back();

    DEBUG(errs() << "Deleting: instruction = " << *cMI << "\n");
    for (MachineOperand& MO : cMI->operands()) {

      DEBUG(errs() << " Deleting: processing operand " << MO << "\n");
      if (MO.isReg()) {
        unsigned current_channel = MO.getReg();
        
        // Skip any channel that is already IGN.
        if (current_channel == LPU::IGN) {
          continue;
        }

        assert(TargetRegisterInfo::isPhysicalRegister(current_channel));

        if (MO.isUse()) {
          MachineInstr* single_use =
            getSingleUse(current_channel, MRI);

          if (single_use) {
            DEBUG(errs() << "Instr " << *cMI
                  << " is the only use of " << MO << "\n");

            // Build a list of the defining instructions.
            SmallVector<MachineInstr*, SEQ_VEC_WIDTH> def_instr;
            for (auto def_it = MRI->def_instr_begin(current_channel);
                 def_it != MRI->def_instr_end();
                 ++def_it) {
              def_instr.push_back(&(*def_it));
            }

            // Replace all the def's of this channel with LPU::IGN.
            // If that instruction has all its def's replaced, then
            // delete that instruction. 
            for (auto def_it = def_instr.begin();
                 def_it != def_instr.end();
                 ++def_it) {
              MachineInstr* def_ins = *def_it;
              assert(def_ins != cMI);

              bool should_delete =
                seq_replace_channel_with_ign(def_ins,
                                             current_channel,
                                             MRI,
                                             TRI,
                                             false,
                                             true);

              if (should_delete) {
                DEBUG(errs() << " Replaced all defs of " << MO
                      << ". Marking for deletion: "
                      << *def_ins << "\n");
                deleteQ.push_back(def_ins);
                insMarkedForDeletion.push_back(def_ins);

                // TBD(jsukha): Dump contents for debugging only.
                // Delete me when we are happy that this method works.
                //
                // for (int i = 0; i < deleteQ.size(); ++i) {
                //   DEBUG(errs() << "DeleteQ " << i << ":"
                //         << deleteQ[i] << " = "
                //         << *(deleteQ[i]) << "\n");
                // }
                // for (int i = 0; i < insMarkedForDeletion.size(); ++i) {
                //   DEBUG(errs() << "insMarkedForDeletion " << i << ":"
                //         << insMarkedForDeletion[i] << " = "
                //         << *(insMarkedForDeletion[i]) << "\n");
                // }
              }
            }
          }
          
          // Substitute the use in this register.
          MO.substPhysReg(LPU::IGN, TRI);
        }
        else if (MO.isDef()) {
          MachineInstr* single_def =
            getSingleDef(current_channel, MRI);
          
          if (single_def) {
            // Walk over all uses of this channel.  These instructions
            // must be deleted, since we are deleting their only def.

            
            // Build a list of the using instructions.
            SmallVector<MachineInstr*, SEQ_VEC_WIDTH> use_instr;
            for (auto use_it = MRI->use_instr_begin(current_channel);
                 use_it != MRI->use_instr_end();
                 ++use_it) {
              use_instr.push_back(&(*use_it));
            }

            
            for (auto use_it = use_instr.begin();
                 use_it != use_instr.end();
                 ++use_it) {
              MachineInstr* use_ins = *use_it;
              assert(use_ins != cMI);

              seq_replace_channel_with_ign(use_ins,
                                           current_channel,
                                           MRI,
                                           TRI,
                                           true,
                                           false);
              // TBD(jsukha): We are only replacing the use of the
              // definition with %ign; we are not checking to see if
              // we should continue to delete the instruction which is
              // now using the "%ign" value.
            }
          } // if (single_def)
          
          MO.substPhysReg(LPU::IGN, TRI);
        }
        else {
          DEBUG(errs() << "ERROR: Found reg operand " << MO
                << " which is neither def nor use...\n");
        }
      }
    }
  }
  
  DEBUG(errs() << "TBD: Final list of instructions to delete: \n");
  for (auto it = insMarkedForDeletion.begin();
       it != insMarkedForDeletion.end();
       ++it) {
    MachineInstr* mi = *it;
    DEBUG(errs() << "    " << *mi);
  }
}


void
LPUOptDFPass::
seq_do_transform_loop(LPUSeqLoopInfo& loop,
                      LPUSeqCandidate& indvarCandidate,
                      unsigned indvar_opcode,
                      int indvarIdx,
                      int boundIdx,
                      int boundOpIdx,
                      std::set<MachineInstr*>& insSetMarkedForDeletion) {

  //  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  LPUMachineFunctionInfo *LMFI = thisMF->getInfo<LPUMachineFunctionInfo>();
  const LPUInstrInfo &TII =
    *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());

  // We need to use the information in "loop" and "indvarCandidate" to
  // replace the loop control with a sequence.  (Step 3 in the
  // description in LPUSequenceOpt.h).
  //
  // Then, once that is done, replacing the remaining dependent
  // sequence candidates, and cleanup (Steps 4 and 5).

  // 1. First convert the loop control variable.
  //
  // This step takes the following set of instructions:
  //
  //      <picker>  = INIT1 0
  //      <picker>  = MOV1 <switcher>
  //      <bottom_i> = add[n] <top_i> <top_s>
  //
  // *    <top_i> = PICK[n] <picker>, <in_i>, <loopBack_i>
  // *    <out_i>, <loopBack_i> = SWITCH[n] <switcher>, <bottom_i>
  // *    <switcher>    = CMP[*] <bottom_i>, <top_b>
  //
  // and replaces it with:
  //
  //      <picker>  = INIT1 0
  //      <picker>  = MOV1 <switcher>
  //      <bottom_i> = add[n] <top_i> <top_s>
  //
  // *    seq[*][n] <top_i>, <pred_i>, %ign, <last_i>, 
  //                <in_i>, <in_b>, <in_s>
  // *    %ign, <out_i> = switch[n] <last_i>, <bottom_i>
  // *    <switcher> = not1 <last_i>
  //
  //
  // 2. Next, we process all of the repeats:
  // 
  //      <top_b> = PICK[n] <picker>, <in_b>, <loopBack_b>
  //      <out_b>, <loopBack_b> = SWITCH[n] <switcher>, <top_b>
  //
  //    Replace with:
  //      <top_b> = REPEAT[n] <pred_i>, <in_b>
  //      %ign, <out_b> = SWITCH[n] <last_i>, <top_b>
  //
  //    Note that this step should catch the repeat for the loop bound
  //    or the stride, if these exist.
  //
  // 3. Then, we process all of the strides:
  //
  //      <top_b> = PICK[n] <picker>, <in_b>, <loopBack_b>
  //      <out_b>, <loopBack_b> = SWITCH[n] <switcher>, <bottom_b>
  //      <bottom_b> = add[n] <top_b> <stride>
  //
  //    Replace with:
  //      <top_b> = stride[n] <pred_i> <stride_op>
  //      <bottom_b> = add[n] <top_b> <stride>
  //      %ign, <out_b> = SWITCH[n] <last_i>, <bottom_b>
  //
  // For each of the steps 1-3, if <out_b> == %ign, then we don't need
  // to add the switch.  Then, this transformation eliminates one use
  // of <bottom_b>.

  // Finally, there is a final cleanup step:
  //
  // 4. For each instruction that is marked for deletion in transform
  //    steps above, convert all the input and output channels into
  //    "%ign" values.
  //    For any converted input, if that input was the only use of the
  //    channel, repeatedly mark the instruction that defined the
  //    channel for deletion.
  //
  // 5. Finally, really delete all the instructions which are marked
  //    for deletion, by removing them from their basic block.

  assert(indvarCandidate.pickInst);
  assert(loop.candidates[indvarIdx].pickInst == indvarCandidate.pickInst);
  MachineBasicBlock* BB = indvarCandidate.pickInst->getParent();

  SmallVector<MachineInstr*, SEQ_VEC_WIDTH> insMarkedForDeletion;

  unsigned top_i = indvarCandidate.top;
  assert(top_i == indvarCandidate.get_pick_top_op()->getReg());

  // <in_i> may or may not be an immediate, but it must come from the
  // pick.
  MachineOperand* in_i_op = indvarCandidate.get_pick_input_op(loop.header);
  
  // Find a matching operand for <in_b>.  It either comes from a
  // corresponding repeat input, or the second argument to the
  // compare.  Note that we can't always pick it straight from the
  // compare unless it is an immediate, because that channel has a
  // value for every loop iteration.
  MachineOperand* in_b_op;
  // This value is NULL if the bound comes from a literal, or non-null
  // if we need a repeat.
  LPUSeqCandidate* bound_repeat = NULL;
  if (boundIdx >= 0) {
    bound_repeat = &loop.candidates[boundIdx];
    assert(bound_repeat->stype == LPUSeqCandidate::SeqType::REPEAT);
    in_b_op = bound_repeat->get_pick_input_op(loop.header);
  }
  else {
    in_b_op = &loop.header.compareInst->getOperand(boundOpIdx);
    assert(in_b_op->isImm());
  }

  assert(indvarCandidate.transformInst);

  MachineOperand* in_s_op = seq_lookup_stride_op(loop,
                                                 indvarCandidate);
  
  int num_dependent_sequences = 0;
  for (unsigned idx = 0; idx < loop.candidates.size(); ++idx) {
    if (idx != (unsigned)indvarIdx) {
      LPUSeqCandidate::SeqType st = loop.candidates[idx].stype;
      // TBD(jsukha): Eventually, this list should include all the
      // sequence types.
      //
      // For now, however, it should only count the ones that we have
      // implemented, because these will use the predicate output of
      // the sequence.
      if ((st == LPUSeqCandidate::SeqType::REPEAT) ||
          (st == LPUSeqCandidate::SeqType::STRIDE) ||
          (st == LPUSeqCandidate::SeqType::PARLOOP_MEM_DEP)) {
        num_dependent_sequences++;
      }
    }
  }
  
  DEBUG(errs() << "For this loop, dependent sequences = " <<
        num_dependent_sequences << "\n");

  // We only need to define a predicate register if we have at least
  // one dependent sequence.
  unsigned pred_reg = LPU::IGN;
  if (num_dependent_sequences > 0) {
    pred_reg = LMFI->allocateLIC(SeqPredRC);
  }
  unsigned first_reg = LPU::IGN;
  unsigned last_reg =  LMFI->allocateLIC(SeqPredRC);

  MachineInstr* seq_inst = BuildMI(*BB,
                                   indvarCandidate.pickInst,
                                   indvarCandidate.pickInst->getDebugLoc(),
                                   TII.get(indvar_opcode),
                                   indvarCandidate.top).
    addReg(pred_reg, RegState::Define).
    addReg(first_reg, RegState::Define).
    addReg(last_reg, RegState::Define).
    addOperand(*in_i_op).
    addOperand(*in_b_op).
    addOperand(*in_s_op);
  seq_inst->setFlag(MachineInstr::NonSequential);


  // If the switcher is expecting 1, 1, 1, ... 1, 0, then
  // loop.header.switcherSense should be 0, and we want a NOT1 to
  // convert from "last" to the "switcher" channel.
  //
  // Otherwise, the switcher is expecting 0, 0, 0, ... 0, 1,
  // switcherSense is 1, and should just use a "MOV1" instead.
  //
  // TBD: We could just generate the last channel directly.  But a
  // later optimization phase can probably eliminate the redundancy
  // fairly easily.
  //
  unsigned switcher_ctrl_opcode = ( loop.header.switcherSense ? LPU::MOV1 : LPU::NOT1 );  
  
  MachineInstr* switcher_def_inst =
    BuildMI(*BB,
            seq_inst,
            indvarCandidate.pickInst->getDebugLoc(),
            TII.get(switcher_ctrl_opcode),
            loop.header.switcherChannel).
    addReg(last_reg);
  switcher_def_inst->setFlag(MachineInstr::NonSequential);

  // If there is a nontrivial output to the switch in the candidate,
  // then insert a switch for the last output.
  //  %ign, <out_i> = switch[n] <last_i>, <bottom_i>
  MachineInstr* output_switch =
    seq_add_output_switch_for_seq_candidate(indvarCandidate,
                                            loop.header,
                                            last_reg,
                                            TII,
                                            *BB,
                                            switcher_def_inst);
  // For the sequence operator, mark the compare, pick and switch for
  // deletion.
  insMarkedForDeletion.push_back(loop.header.compareInst);
  insMarkedForDeletion.push_back(indvarCandidate.pickInst);
  insMarkedForDeletion.push_back(indvarCandidate.switchInst);

  DEBUG(errs() << "Adding a new sequence instruction "
        << *seq_inst << "\n");
  DEBUG(errs() << "Adding a new switcher def inst "
        << *switcher_def_inst << "\n");
  if (output_switch) {
    DEBUG(errs() << "Adding a switch output instruction "
          << *output_switch << "\n");
  }

  // Process all the dependent sequences.
  // The repeat candidates should be first...
  for (unsigned idx = 0; idx < loop.candidates.size(); ++idx) {
    // Skip over the loop induction variable.
    // We should not process it twice.
    if (idx != (unsigned)indvarIdx) {
      LPUSeqCandidate& scandidate = loop.candidates[idx];
      switch (scandidate.stype) {

        // Process repeat/stride in the same case.
        // They look fairly similar...
      case LPUSeqCandidate::SeqType::REPEAT:
        {
          MachineInstr* repinst =
            seq_add_repeat(scandidate,
                           loop.header, 
                           pred_reg,
                           TII,
                           *BB,
                           seq_inst);
          MachineInstr* out_switch =
            seq_add_output_switch_for_seq_candidate(scandidate,
                                                    loop.header,
                                                    last_reg,
                                                    TII,
                                                    *BB,
                                                    repinst);
          DEBUG(errs() << "Adding a repeat/stride: "
                << *repinst << "\n");
          if (out_switch) {
            DEBUG(errs() << "Adding an output switch for the repeat/stride: "
                  << *out_switch << "\n");
          }

          insMarkedForDeletion.push_back(scandidate.pickInst);
          insMarkedForDeletion.push_back(scandidate.switchInst);
          assert(!scandidate.transformInst);
        }
        break;


        // Stride.
        case LPUSeqCandidate::SeqType::STRIDE:
        {
          assert(scandidate.transformInst);
          unsigned transformOpcode
            = scandidate.transformInst->getOpcode();
          unsigned strideOpcode;

          // We should have already matched the opcodes at the time we
          // classified the candidate.  TBD: We could store the match,
          // instead of looking it up here again...
          bool matched =
            seq_compute_matching_stride_opcode(transformOpcode,
                                               &strideOpcode);
          assert(matched);
          
          MachineOperand* my_s_op =
            seq_lookup_stride_op(loop,
                                 scandidate);
          MachineInstr* stride_inst =
            seq_add_stride(scandidate,
                           loop.header, 
                           pred_reg,
                           my_s_op,
                           strideOpcode, 
                           TII,
                           *BB,
                           seq_inst);
          MachineInstr* out_switch =
            seq_add_output_switch_for_seq_candidate(scandidate,
                                                    loop.header,
                                                    last_reg,
                                                    TII,
                                                    *BB,
                                                    stride_inst);
          DEBUG(errs() << "Adding a stride instruction: "
                << *stride_inst << "\n");
          if (out_switch) {
            DEBUG(errs() << "Adding an output switch for the stride: "
                  << *out_switch << "\n");
          }
          
          insMarkedForDeletion.push_back(scandidate.pickInst);
          insMarkedForDeletion.push_back(scandidate.switchInst);
          // We should NOT mark scandidate.transformInst for deletion
          // here.  In cases where the channel at the bottom of the
          // loop is also used, we still need the add instruction.
          // Moreover, marking the switch for deletion is sufficient
          // to get rid of the add, if that switch is the only use of
          // the add's output.
        }
        break;

      case LPUSeqCandidate::SeqType::PARLOOP_MEM_DEP:
        {
          MachineInstr* onend_inst =
            seq_add_parloop_memdep(scandidate,
                                   loop.header,                                   
                                   pred_reg,
                                   TII,
                                   *BB,
                                   seq_inst);
          DEBUG(errs() << "Adding a parloop mem dep repeat-onend: "
                << *onend_inst << "\n");
          insMarkedForDeletion.push_back(scandidate.pickInst);
          insMarkedForDeletion.push_back(scandidate.switchInst);
          assert(!scandidate.transformInst);
        }
        break;
        

      default:
        DEBUG(errs() << "Ignoring sequence candidate in transform: ");
        seq_debug_print_candidate(scandidate);
      }
    }
  }

  // Cleanup: mark instructions for deletion.
  seq_mark_loop_ins_for_deletion(loop,
                                 *BB,
                                 insMarkedForDeletion);

  // Add to the global set of instructions we are deleting.
  // We will delete them all at the end.
  for (auto it = insMarkedForDeletion.begin();
       it != insMarkedForDeletion.end();
       ++it) {
    insSetMarkedForDeletion.insert(*it);
  }
}


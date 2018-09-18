//===-- CSAOptDFPass.cpp - CSA optimization of data flow ------------------===//
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
// This pass optimizes post-dataflow code.  In particular, it does things
// like insert sequence operations when appropriate.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSAInstrInfo.h"
#include "CSAMatcher.h"
#include "CSASeqOpt.h"
#include "CSATargetMachine.h"
#include "InstPrinter/CSAInstPrinter.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include <map>
#include <set>
// Define data structures needed for sequence optimizations.
#include "CSASequenceOpt.h"
// Width of vectors we are using for sequence op calculation.
// TBD(jsukha): As far as I know, this value only affects performance,
// not correctness?
#define SEQ_VEC_WIDTH 8

using namespace llvm;

static cl::opt<int> OptDFPass("csa-opt-df-pass", cl::Hidden,
                              cl::desc("CSA Specific: Optimize data flow pass"),
                              cl::init(1));

#define DEBUG_TYPE "csa-opt-df"
#define PASS_NAME "CSA: (Sequence) Optimizations to Data Flow"

const TargetRegisterClass *const SeqPredRC = &CSA::CI1RegClass;

// Flag for enabling sequence optimizations.
//
//   0:   Disabled optimization
//   >=1: Enabled
//
// Note that this optimization is not run unless csa-opt-df-pass is
// also set > 0.
static cl::opt<int>
  RunSequenceOpt("csa-seq-opt", cl::Hidden,
                 cl::desc("CSA Specific: Enable sequence optimizations"),
                 cl::init(1));

// Flag for choosing type of sequence optimization:
//   0: Everything off.
//   1: Analysis to find sequence optimizations only.
//      Prints LLVM debugging output.
//   2: Default type of sequence transform.
enum SequenceOptMode { off = 0, analysis = 1, standard = 2, scc = 3 };
//
//   Other values might be added if needed.
//
// Technically, this flag subsumes the RunSequenceOpt flag.  However,
// having the default value of the flag be "2" raises more questions
// since users ask about the meaning of other numbers.
// Mostly we expect only compiler developers to use this knob.
static cl::opt<SequenceOptMode> RunSequenceOptType(
  "csa-seq-opt-type", cl::Hidden,
  cl::desc("CSA Specific: Type of sequence optimizations. 0 == off, 1 == "
           "analysis only, 2 == standard, 3 == scc"),
  cl::values(clEnumVal(off, "No sequence optimization"),
             clEnumVal(analysis, "Sequence analysis only, but not transforms"),
             clEnumVal(standard,
                       "Sequence transforms enabled using pattern matching"),
             clEnumValN(scc, "default",
                        "Sequence transforms enabled using scc(default)")),
  cl::init(SequenceOptMode::scc));

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
// Note that this optimization has no effect unless csa-opt-df-pass is
// also set > 0.
//
// WARNING: Setting this flag may result in incorrect code being
// generated.  Use with extreme caution.
static cl::opt<int> SeqBreakMemdep(
  "csa-seq-break-memdep", cl::Hidden,
  cl::desc("CSA Specific: Break memory dependencies for sequenced loops"),
  cl::init(0));

// Enable or disable detection of reductions.
static cl::opt<int> SeqReduction(
  "csa-seq-reduction", cl::Hidden,
  cl::desc("CSA Specific: Enable reduction sequence transformation"),
  cl::init(1));

// Flag to specify the maximum number of sequence candidates we will
// identify in a given loop.
//
// TBD(jsukha): I set this value to be a large but arbitrary value.
static cl::opt<int> SequenceMaxPerLoop(
  "csa-seq-max", cl::Hidden,
  cl::desc("CSA Specific: Max sequence units inserted per loop"),
  cl::init(1024 * 1024));

namespace llvm {

class CSAOptDFPass : public MachineFunctionPass {
public:
  static char ID;
  CSAOptDFPass() : MachineFunctionPass(ID) {
    initializeCSAOptDFPassPass(*PassRegistry::getPassRegistry());
    thisMF = nullptr;
  }

  StringRef getPassName() const override {
    return PASS_NAME;
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfo>();
    AU.addRequired<MachineOptimizationRemarkEmitterPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  // Return the unique machine instruction that defines or uses a
  // register, if exactly one exists.  Otherwise, returns NULL.
  MachineInstr *getSingleDef(unsigned Reg, const MachineRegisterInfo *MRI);
  MachineInstr *getSingleUse(unsigned Reg, const MachineRegisterInfo *MRI);

  // Do sequence optimizations.
  void runSequenceOptimizations(SequenceOptMode seq_opt_mode);

  // Helper methods for sequence

  // Debug print methods.
  //
  // Print header
  void seq_debug_print_header(CSASeqHeader &header);

  // Print the information out of a sequence candidate.
  void seq_debug_print_candidate(CSASeqCandidate &x);

  //
  // Print loop info.
  void seq_print_loop_info(SmallVector<CSASeqLoopInfo, SEQ_VEC_WIDTH> *loops);

  // Returns true if a machine instruction is
  //   <picker> = INIT1 0
  bool seq_is_picker_init_inst(MachineRegisterInfo *MRI, MachineInstr *MI,
                               unsigned *picker_channel, bool *picker_sense);

  // Returns true if a machine instruction is
  //   <picker> = MOV1 <switcher>
  bool seq_is_picker_mov_inst(MachineRegisterInfo *MRI, MachineInstr *MI,
                              unsigned picker_channel,
                              unsigned *switcher_channel);

  // Returns true if we found the sequence of CSA instructions that
  // forms the header of a loop.  If true, fills in "header"
  // with the info.
  bool seq_identify_header(MachineInstr *MI, CSASeqHeader *header);

  // Returns MI if  MI is a pick instruction that matches the specified
  // loop header.  Otherwise, returns NULL.
  // Also saves MI into pickMap (keyed by loopback register) if a
  // match is found.
  MachineInstr *
  seq_candidate_match_pick(MachineInstr *MI, const CSASeqHeader &header,
                           const CSAInstrInfo &TII,
                           DenseMap<unsigned, MachineInstr *> &pickMap);

  // Returns matching pick instruction if MI is a switch instruction
  // that matches a pick in the specified loop.  Otherwise, returns
  // NULL.
  MachineInstr *
  seq_candidate_match_switch(MachineInstr *MI, const CSASeqHeader &header,
                             const CSAInstrInfo &TII,
                             DenseMap<unsigned, MachineInstr *> &pickMap);

  // Helper method for finding sequence candidates.
  void
  seq_find_candidate_loops(SmallVector<CSASeqLoopInfo, SEQ_VEC_WIDTH> *loops);

  // Check whether this candidate sequence is either a repeat or a
  // reduction.  If yes, then it modifies x.stype.
  //
  // If it is a repeat, save its channel into "repeat_channels".
  // Returns one of:
  //
  //  SeqType::UNKNOWN
  //  SeqType::REPEAT
  //  SeqType::REDUCTION
  CSASeqCandidate::SeqType seq_classify_repeat_or_reduction(CSASeqCandidate &x);

  // Check whether this candidate sequence matches a "stride" type.
  CSASeqCandidate::SeqType
  seq_classify_stride(CSASeqCandidate &x,
                      const DenseMap<unsigned, int> &repeat_channels);

  // Check whether this candidate sequence represents a memory dependency
  // chain.
  CSASeqCandidate::SeqType seq_classify_memdep_graph(CSASeqCandidate &x);

  // Classify all the candidate sequences in the loops we found.
  void seq_analyze_loops(SmallVector<CSASeqLoopInfo, SEQ_VEC_WIDTH> *loops);

  // Helper methods for seq_analyze_loops.
  //
  // The three "seq_classify_loop" methods handle together handle the
  // classification of all the sequence candidates for a given loop.
  //
  void seq_classify_loop_repeats(
    CSASeqLoopInfo &current_loop,
    SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &repeats,
    SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &reductions,
    SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &other);
  void seq_classify_loop_reductions_as_strides(
    SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &reductions,
    CSASeqLoopInfo &current_loop,
    SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &remaining);
  void seq_classify_loop_remaining(
    SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &remaining,
    CSASeqLoopInfo &current_loop,
    SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &other);

  // The final check in the analysis phase, which checks whether we
  // can figure out which sequence candidate is an induction variable
  // for the loop.
  // Returns true if we found a valid induction variable and bound.
  bool seq_identify_induction_variable(CSASeqLoopInfo &loop);

  // The actual sequence transformation.
  // This method should only get called once we've passed all our checks for
  // validity in the transform.
  void seq_do_transform_loop(CSASeqLoopInfo &loop);

  // Helper methods for do_transform_loop.

  // Transform the induction variable fo the loop.  Modify the seqInfo
  // structure to save the info about the sequence instruction we
  // created.
  void seq_do_transform_loop_seq(
    CSASeqLoopInfo &loop, MachineBasicBlock *BB, CSASeqInstrInfo *seqInfo,
    SmallVector<MachineInstr *, SEQ_VEC_WIDTH> &insToDisconnect);
  void seq_do_transform_loop_repeat(
    CSASeqCandidate &scandidate, CSASeqLoopInfo &loop, MachineBasicBlock *BB,
    const CSASeqInstrInfo &seqInfo, const CSAInstrInfo &TII,
    SmallVector<MachineInstr *, SEQ_VEC_WIDTH> &insToDisconnect);

  void seq_do_transform_loop_stride(
    CSASeqCandidate &scandidate, CSASeqLoopInfo &loop, MachineBasicBlock *BB,
    const CSASeqInstrInfo &seqInfo, const CSAInstrInfo &TII,
    SmallVector<MachineInstr *, SEQ_VEC_WIDTH> &insToDisconnect);

  void seq_do_transform_loop_parloop_memdep(
    CSASeqCandidate &scandidate, CSASeqLoopInfo &loop, MachineBasicBlock *BB,
    const CSASeqInstrInfo &seqInfo, const CSAInstrInfo &TII,
    SmallVector<MachineInstr *, SEQ_VEC_WIDTH> &insToDisconnect);

  void seq_do_transform_loop_reduction(
    CSASeqCandidate &scandidate, CSASeqLoopInfo &loop, MachineBasicBlock *BB,
    const CSASeqInstrInfo &seqInfo, const CSAInstrInfo &TII,
    SmallVector<MachineInstr *, SEQ_VEC_WIDTH> &insToDisconnect);

  // Add a switch instruction after "prev_inst" in basic block BB,
  // which stores the last value to the output channel for the
  // specified sequence candidate (sCandidate).
  //
  // Returns NULL if no switch instruction is necessary (because the
  //  output channel of the candidate is %ign).
  // Otherwise, returns a pointer to the instruction we created.
  MachineInstr *seq_add_output_switch_for_seq_candidate(
    CSASeqCandidate &sCandidate, const CSASeqHeader &loop_header,
    unsigned last_reg, const CSAInstrInfo &TII, MachineBasicBlock &BB,
    MachineInstr *prev_inst);

  // Look up the stride operation that corresponds to a given sequence
  // candidate.
  MachineOperand *seq_lookup_stride_op(CSASeqLoopInfo &loop,
                                       CSASeqCandidate &scandidate);

  MachineOperand *seq_add_negate_stride_op(MachineOperand *in_stride_op,
                                           unsigned stride_opcode,
                                           const CSAInstrInfo &TII,
                                           CSAMachineFunctionInfo *LMFI,
                                           MachineBasicBlock &BB,
                                           MachineInstr *prev_inst);

  // Add a repeat instruction for the specified repeat candidate.
  // Returns the added instruction.
  MachineInstr *seq_add_repeat(CSASeqCandidate &repeat_candidate,
                               const CSASeqHeader &loop_header,
                               unsigned pred_reg, const CSAInstrInfo &TII,
                               MachineBasicBlock &BB, MachineInstr *prev_inst);

  // Add a stride instruction for the specified stride candidate.
  // Returns the added instruction.
  MachineInstr *seq_add_stride(CSASeqCandidate &repeat_candidate,
                               const CSASeqHeader &loop_header,
                               unsigned pred_reg, MachineOperand *in_stride_op,
                               const CSAInstrInfo &TII, MachineBasicBlock &BB,
                               MachineInstr *prev_inst);

  // Add a repeat/onend pair fora memory dependency chain.
  // Returns the onend instruction.
  MachineInstr *seq_add_parloop_memdep(CSASeqCandidate &memdepCandidate,
                                       const CSASeqHeader &loop_header,
                                       unsigned pred_reg,
                                       const CSAInstrInfo &TII,
                                       MachineBasicBlock &BB,
                                       MachineInstr *prev_inst);

  MachineInstr *seq_add_reduction(CSASeqCandidate &sc,
                                  const CSASeqHeader &loop_header,
                                  unsigned pred_reg, const CSAInstrInfo &TII,
                                  MachineBasicBlock &BB,
                                  MachineInstr *prev_inst,
                                  bool is_fma_reduction);

  // Replace all inputs (uses) of the specified instruction with %na and all
  // outputs (defs) with %ign, thus completely disconnecting the instruction
  // from the graph.  The instruction will be removed during the
  // dead-instruction pass, as will any other instructions that transitively
  // become dead because of this disconnection.
  void disconnect_instruction(MachineInstr *MI);

private:
  MachineFunction *thisMF;
  MachineLoopInfo *MLI;
  const CSAInstrInfo *TII;
  // MachineDominatorTree *DT;
  // CSAMachineFunctionInfo *LMFI;
};
} // namespace llvm

MachineFunctionPass *llvm::createCSAOptDFPass() { return new CSAOptDFPass(); }

char CSAOptDFPass::ID = 0;

INITIALIZE_PASS_BEGIN(CSAOptDFPass, DEBUG_TYPE, PASS_NAME, false, false)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_DEPENDENCY(MachineOptimizationRemarkEmitterPass)
INITIALIZE_PASS_END(CSAOptDFPass, DEBUG_TYPE, PASS_NAME, false, false)


bool CSAOptDFPass::runOnMachineFunction(MachineFunction &MF) {

  if (OptDFPass == 0)
    return false;

  thisMF = &MF;
  MLI    = &getAnalysis<MachineLoopInfo>();
  auto &ORE = getAnalysis<MachineOptimizationRemarkEmitterPass>().getORE();

  TII = static_cast<const CSAInstrInfo *>(MF.getSubtarget().getInstrInfo());

  bool Modified = false;

  if (RunSequenceOpt && RunSequenceOptType == SequenceOptMode::scc) {
    CSASeqOpt seqOpt(thisMF, ORE, DEBUG_TYPE);
    seqOpt.SequenceOPT(false);
    return Modified;
  }
  // Using SEQ in place of pick/add/cmp/switch pattern.
  // Force a consistent setting of RunSequenceOpt and RunSequenceType
  // when sequence transform is off.
  if (RunSequenceOpt == 0) {
    RunSequenceOptType = SequenceOptMode::off;
  }

  runSequenceOptimizations(RunSequenceOptType);

  return Modified;
}

/*****************************************************************************/
// Code for sequence optimizations.
//
// TBD(jsukha): This code should eventually be split out into a
// different file, and possibly a different pass?
//

void CSAOptDFPass::seq_debug_print_header(CSASeqHeader &header) {
  LLVM_DEBUG(errs() << "CSASeqHeader: \npicker = " << header.pickerChannel);
  LLVM_DEBUG(errs() << "\nswitcher = " << header.switcherChannel << "\n");
  if (header.pickerInit) {
    LLVM_DEBUG(errs() << " pickerInit: " << *header.pickerInit << "");
  } else {
    LLVM_DEBUG(errs() << " No pickerInit\n");
  }
  if (header.pickerMov1) {
    LLVM_DEBUG(errs() << " pickerMov1: " << *header.pickerMov1 << "");
  } else {
    LLVM_DEBUG(errs() << " No pickerMov1\n");
  }
  if (header.compareInst) {
    LLVM_DEBUG(errs() << " compareInst: " << *header.compareInst << "");
  } else {
    LLVM_DEBUG(errs() << " No compareInst\n");
  }
}

void CSAOptDFPass::seq_debug_print_candidate(CSASeqCandidate &x) {

  LLVM_DEBUG(errs() << " pick = " << *x.pickInst);
  LLVM_DEBUG(errs() << " switch = " << *x.switchInst);
  if (x.transformInst) {
    LLVM_DEBUG(errs() << " transform = " << *x.transformInst << "\n");
  }
  switch (x.stype) {
  case CSASeqCandidate::SeqType::UNKNOWN:
    LLVM_DEBUG(errs() << "UNKNOWN type"
               << "\n");
    break;
  case CSASeqCandidate::SeqType::REPEAT:
    LLVM_DEBUG(errs() << "REPEAT: top = " << x.top << ", bottom = " << x.bottom
               << "\n");
    break;
  case CSASeqCandidate::SeqType::REDUCTION:
    LLVM_DEBUG(errs() << "REDUCTION: top = " << x.top << ", bottom = " << x.bottom
               << "\n");
    break;
  case CSASeqCandidate::SeqType::STRIDE:
    LLVM_DEBUG(errs() << "STRIDE: top = " << x.top << ", bottom = " << x.bottom
               << "\n");
    LLVM_DEBUG(errs() << "stride op = " << *x.saved_op << "\n");
    break;
  case CSASeqCandidate::SeqType::PARLOOP_MEM_DEP:
    LLVM_DEBUG(errs() << "PARLOOP_MEM_DEP: top = " << x.top
               << ", bottom = " << x.bottom << "\n");
    break;
  case CSASeqCandidate::SeqType::INVALID:
    LLVM_DEBUG(errs() << "INVALID sequence type \n");
    break;
  }
  LLVM_DEBUG(errs() << "\n");
}

bool CSAOptDFPass::seq_is_picker_init_inst(MachineRegisterInfo *MRI,
                                           MachineInstr *MI,
                                           unsigned *pickerChannel,
                                           bool *pickerSense) {
  // Construct a pair of patterns to match.
  // Eventually, we'll be able to OR these together.
  using namespace CSAMatch;
  MIRMATCHER_REGS(PICKER_DEF);
  constexpr auto picker_pat0 = PICKER_DEF = init1(litZero);
  constexpr auto picker_pat1 = PICKER_DEF = init1(litOne);
  // using mirmatch::AnyLiteral;
  // constexpr auto picker_pat =        PICKER_DEF = init1(AnyLiteral);

  // auto pat_match = mirmatch::match(picker_pat, MI);
  auto pat_match = mirmatch::match(picker_pat0, MI);
  int pat_ival   = 0;
  if (!pat_match) {
    pat_match = mirmatch::match(picker_pat1, MI);
    pat_ival  = 1;
  }

  if (MI->getOpcode() == CSA::INIT1) {
    LLVM_DEBUG(errs() << "Found an init instruction " << *MI << "with "
               << MI->getNumOperands() << " operands \n");
    if (MI->getNumOperands() == 2) {
      MachineOperand &picker_def = MI->getOperand(0);
      MachineOperand &init_val   = MI->getOperand(1);

      if (init_val.isImm()) {
        int ival = init_val.getImm();
        if ((ival == 0) || (ival == 1)) {
          LLVM_DEBUG(errs() << "Found an init " << ival << " \n");
          if (picker_def.isReg()) {
            int pickval = picker_def.getReg();
            // TBD(jsukha): I can't figure out how to query the register
            // class of a channel here.  Bcause it is a physical
            // register, I can't seem to use the normal getRegClass methods.
            // But I'm going to assume that knowing that the
            // opcode was INIT1 was enough...
            if (TII->isLIC(picker_def, *MRI)) {
              LLVM_DEBUG(errs() << "Matched %ival = init " << ival << " \n");
              *pickerChannel = pickval;
              *pickerSense   = ival;
              MATCH_ASSERT(pat_match);
              MATCH_ASSERT(pat_ival == ival);
              (void) pat_ival;
              //              MATCH_ASSERT(pat_match.instr(PICKER_DEF =
              //              init1(AnyLiteral)) == MI);
              MATCH_ASSERT(pat_match.reg(PICKER_DEF) == unsigned(pickval));
              return true;
            } else {
              LLVM_DEBUG(errs() <<
                         "Found picker in a virtual reg. Skipping...\n");
            }
          } else {
            LLVM_DEBUG(errs() <<
                       "Picker def " << picker_def << " is not a reg\n");
          }
        } else {
          LLVM_DEBUG(errs() <<
                     "Picker def " << picker_def << " is not a reg\n");
        }
      }
    }
  }
  MATCH_ASSERT(!pat_match);
  return false;
}

bool CSAOptDFPass::seq_is_picker_mov_inst(MachineRegisterInfo *MRI,
                                          MachineInstr *MI,
                                          unsigned pickerChannel,
                                          unsigned *switcherChannel) {

  using namespace CSAMatch;
  MIRMATCHER_REGS(PICKER, SWITCHER);
  constexpr auto pattern = graph(PICKER = mov1(SWITCHER));
  auto pat_match         = mirmatch::match(pattern, MI);

  if (MI && (MI->getOpcode() == CSA::MOV1)) {
    MATCH_ASSERT(pat_match);
    if (MI->getNumOperands() == 2) {
      MachineOperand &pickerDef   = MI->getOperand(0);
      MachineOperand &switcherDef = MI->getOperand(1);
      if (pickerDef.isReg() && (pickerDef.getReg() == pickerChannel)) {
        MATCH_ASSERT(pat_match.reg(PICKER) == pickerChannel);
        if (switcherDef.isReg()) {
          unsigned swchannel = switcherDef.getReg();
          MATCH_ASSERT(pat_match.reg(SWITCHER) == swchannel);
          if (TII->isLIC(switcherDef, *MRI)) {
            *switcherChannel = swchannel;
            return true;
          }
        }
      }
    }
  }

  MATCH_ASSERT(!pat_match || pat_match.reg(PICKER) != pickerChannel);
  return false;
}

// Match any comparison opcode
constexpr mirmatch::OpcodeRange<CSA::CMPEQ16, CSA::CMPUOF64> cmpany{};

bool CSAOptDFPass::seq_identify_header(MachineInstr *MI, CSASeqHeader *header) {
  using namespace CSAMatch;
  using mirmatch::AnyLiteral;
  MIRMATCHER_REGS(PICKER, SWITCHER);
  constexpr auto AnyOpnd = mirmatch::AnyOperand;
  constexpr auto pattern =
    graph(PICKER = init1(AnyLiteral), PICKER = mov1(SWITCHER),
          SWITCHER = cmpany(AnyOpnd, AnyOpnd));

  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  const CSAInstrInfo &TII =
    *static_cast<const CSAInstrInfo *>(thisMF->getSubtarget().getInstrInfo());

  auto pat_match = mirmatch::match(pattern, MI);

  // Look for an "<picker> = INIT1 0" or "<picker> = INIT1 1" instruction.
  unsigned pickerChannel;
  bool pickerSense = 0;
  if (seq_is_picker_init_inst(MRI, MI, &pickerChannel, &pickerSense)) {
    LLVM_DEBUG(errs() << "Found picker definition. Register= " << pickerChannel
               << " = " << printReg(pickerChannel) << "\n");

    // Once we have a picker, then walk over and count the defs.  We
    // want to find exactly one (other) def != MI, which is a MOV1
    // instruction.
    int def_count            = 0;
    MachineInstr *pickerMov1 = NULL;
    for (auto def_it = MRI->def_instr_begin(pickerChannel);
         def_it != MRI->def_instr_end(); ++def_it) {
      MachineInstr *defMI = &(*def_it);
      if (defMI != MI) {
        pickerMov1 = defMI;
      }
      def_count++;
    }

    LLVM_DEBUG(errs() << "Num defs found: " << def_count << "\n");
    unsigned switcherChannel = 0;

    // TBD(jsukha): In theory, we should be able to deal with loops
    // where the switcher control is inverted from the picker control.
    // But I'm ignoring this case for now.
    if (!seq_is_picker_mov_inst(MRI, pickerMov1, pickerChannel,
                                &switcherChannel)) {
      // If that last definition is not instruction we want, bail.
      MATCH_ASSERT(!pat_match);
      return false;
    }
    bool switcherSense = pickerSense;

    LLVM_DEBUG(errs() << "Found pickerMov1 instruction " << *pickerMov1);
    LLVM_DEBUG(errs() << " with switcher channel " << switcherChannel << "\n");

    // If we make it here, then we have both a picker and a switcher.
    // Next, check if the switcher is defined by a compare.
    int def2_count            = 0;
    MachineInstr *compareInst = NULL;
    for (auto def_it = MRI->def_instr_begin(switcherChannel);
         def_it != MRI->def_instr_end(); ++def_it) {
      def2_count++;
      compareInst = &(*def_it);
    }

    if (!((def2_count == 1) && TII.isCmp(compareInst))) {
      LLVM_DEBUG(errs() <<
                 "Stop. Found " << def2_count << " defs, with last instr " <<
                 *compareInst << "\n");
      MATCH_ASSERT(!pat_match);
      return false;
    }
    LLVM_DEBUG(errs() << "Found compare instruction " << compareInst << "\n");

    if (compareInst->getNumOperands() != 3) {
      LLVM_DEBUG(errs() << " Stop. compare inst without 3 operands???"
                 << "\n");
      MATCH_ASSERT(!pat_match);
      return false;
    }

    // Finally, if we made it here, success!  Initialize the header.
    header->init(MI, pickerMov1, compareInst, pickerChannel, switcherChannel,
                 pickerSense, switcherSense);
    LLVM_DEBUG(errs() << "Found loop header\n");
    MATCH_ASSERT(pat_match);
    MATCH_ASSERT(pat_match.instr(PICKER = mov1(SWITCHER)) == pickerMov1);
    MATCH_ASSERT(pat_match.instr(SWITCHER = cmpany(AnyOpnd, AnyOpnd)) ==
                 compareInst);
    MATCH_ASSERT(pat_match.reg(PICKER) == pickerChannel);
    MATCH_ASSERT(pat_match.reg(SWITCHER) == switcherChannel);
    return true;
  }
  MATCH_ASSERT(!pat_match);
  return false;
}

void CSAOptDFPass::seq_print_loop_info(
  SmallVector<CSASeqLoopInfo, SEQ_VEC_WIDTH> *loops) {

  LLVM_DEBUG(errs() << "************************\n");
  LLVM_DEBUG(errs() << "SEQ LOOP INFO:  " << loops->size() << " loops\n");

  for (unsigned i = 0; i < loops->size(); ++i) {
    CSASeqLoopInfo &current_loop = (*loops)[i];

    LLVM_DEBUG(errs() << "*****************\n");
    LLVM_DEBUG(errs() << "Loop " << i << "[ ");
    LLVM_DEBUG(errs() << current_loop.candidates.size() << " pairs]\n");
    seq_debug_print_header(current_loop.header);

    // Print matches to cmp0 and cmp1 uses, if they exist.
    if (current_loop.cmp0Idx() >= 0) {

      LLVM_DEBUG(errs() << "cmp0 matches candidate: \n");
      seq_debug_print_candidate(
        current_loop.candidates[current_loop.cmp0Idx()]);
    } else {
      LLVM_DEBUG(errs() << "No cmp0_idx\n");
    }
    if (current_loop.cmp1Idx() >= 0) {
      LLVM_DEBUG(errs() << "cmp1 matches candidate: \n");
      seq_debug_print_candidate(
        current_loop.candidates[current_loop.cmp1Idx()]);
    } else {
      LLVM_DEBUG(errs() << "No cmp1_idx\n");
    }

    LLVM_DEBUG(errs() << "Repeat channels: ");
    for (auto it = current_loop.repeat_channels.begin();
         it != current_loop.repeat_channels.end(); ++it) {
      unsigned reg = it->getFirst();
      LLVM_DEBUG(errs() <<
                 "(Reg= " << reg << ", idx= " << it->getSecond() << ") ");
      (void) reg;
    }
    LLVM_DEBUG(errs() << "\n");

    LLVM_DEBUG(errs() << "\n** All candidates **\n");
    // Dump the pick/switch pairs that we found.
    for (auto it = current_loop.candidates.begin();
         it != current_loop.candidates.end(); ++it) {
      CSASeqCandidate &x = *it;
      seq_debug_print_candidate(x);
    }

    LLVM_DEBUG(errs() << "*****************\n");
  }
  LLVM_DEBUG(errs() << "************************\n");
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
MachineInstr *CSAOptDFPass::seq_candidate_match_pick(
  MachineInstr *MI, const CSASeqHeader &header, const CSAInstrInfo &TII,
  DenseMap<unsigned, MachineInstr *> &pickMap) {
  if (TII.isPick(MI)) {
    assert(MI->getNumOperands() == 4);

    int pick_select_idx      = CSASeqHeader::pick_select_op_idx();
    MachineOperand &selectOp = MI->getOperand(pick_select_idx);

    // Figure out which op is the loopback based on the sense of the
    // header.
    int loopback_idx           = header.pick_loopback_op_idx();
    MachineOperand &loopbackOp = MI->getOperand(loopback_idx);
    MachineRegisterInfo *MRI   = &thisMF->getRegInfo();

    if (selectOp.isReg() && loopbackOp.isReg()) {
      unsigned select_reg   = selectOp.getReg();
      unsigned loopback_reg = loopbackOp.getReg();
      if (TII.isLIC(selectOp, *MRI) && TII.isLIC(loopbackOp, *MRI) &&
          select_reg == header.pickerChannel) {
        LLVM_DEBUG(errs() << "Found a pick candidate " << *MI
                   << " with loopback reg " << loopback_reg << "\n");
        if (pickMap.find(loopback_reg) != pickMap.end()) {
          LLVM_DEBUG(errs() << "WARNING: found an existing pick ins "
                     << *pickMap[loopback_reg]
                     << " with same loopback reg...\n");
        } else {
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
MachineInstr *CSAOptDFPass::seq_candidate_match_switch(
  MachineInstr *MI, const CSASeqHeader &header, const CSAInstrInfo &TII,
  DenseMap<unsigned, MachineInstr *> &pickMap) {
  // Look for:
  //   switch[n] final, loopback, switcherChannel, bottom_val
  if (TII.isSwitch(MI)) {

    assert(MI->getNumOperands() == 4);
    int loopback_idx           = header.switch_loopback_op_idx();
    MachineOperand &loopbackOp = MI->getOperand(loopback_idx);

    int switch_select_idx    = CSASeqHeader::switch_select_op_idx();
    MachineOperand &selectOp = MI->getOperand(switch_select_idx);

    if (selectOp.isReg() && loopbackOp.isReg()) {
      unsigned select_reg      = selectOp.getReg();
      unsigned loopback_reg    = loopbackOp.getReg();
      MachineRegisterInfo *MRI = &thisMF->getRegInfo();

      if (TII.isLIC(selectOp, *MRI) && TII.isLIC(loopbackOp, *MRI) &&
          select_reg == header.switcherChannel) {

        LLVM_DEBUG(errs() << "Found possible switch candidate " << *MI
                   << " with loopback reg " << loopback_reg << "\n");

        if (pickMap.find(loopback_reg) == pickMap.end()) {
          LLVM_DEBUG(errs() <<
                     "WARNING: No match. No matching pick for this switch\n");
        } else {
          MachineInstr *matching_pick = pickMap[loopback_reg];

          // Finally, verify that the number of uses of the
          // loopback register is exactly 1, i.e., in the
          // pick.
          matching_pick = getSingleUse(loopback_reg, MRI);

          if (!matching_pick) {
            LLVM_DEBUG(
              errs()
              << "WARNING: No match.  Found other uses of loopback register "
              << loopback_reg << "\n");
          }
          return matching_pick;
        }
      }
    }
  }
  return nullptr;
}

void CSAOptDFPass::seq_find_candidate_loops(
  SmallVector<CSASeqLoopInfo, SEQ_VEC_WIDTH> *loops) {
  const CSAInstrInfo &TII =
    *static_cast<const CSAInstrInfo *>(thisMF->getSubtarget().getInstrInfo());
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  int loop_id_counter      = 0;

  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end();
       BB != E; ++BB) {

    CSASeqHeader tmp_header;
    MachineBasicBlock::iterator iterMI = BB->begin();

    while (iterMI != BB->end()) {
      MachineInstr *MI = &*iterMI;

      if (seq_identify_header(MI, &tmp_header)) {
        LLVM_DEBUG(errs() << "Found a sequence header "
                   << "\n");
        seq_debug_print_header(tmp_header);

        // Save the header information into the current loop.
        CSASeqLoopInfo current_loop;
        current_loop.loop_id = loop_id_counter++;
        current_loop.header  = tmp_header;

        // We are going to look for pick and switch instructions, and
        // store them into a map keyed on their loopback inputs.
        DenseMap<unsigned, MachineInstr *> pickMap;

        // Walk over uses of the picker channel, storing the matching picks.
        for (auto it = MRI->use_instr_begin(current_loop.header.pickerChannel);
             it != MRI->use_instr_end(); ++it) {
          MachineInstr *MI = &(*it);
          seq_candidate_match_pick(MI, current_loop.header, TII, pickMap);
        }

        // Now walk over the uses of the corresponding switcher
        // channel.  Look for a matching pick (based on the loopback
        // channel).
        for (auto it =
               MRI->use_instr_begin(current_loop.header.switcherChannel);
             it != MRI->use_instr_end(); ++it) {
          MachineInstr *MI = &(*it);

          // Look for a switch that matches some pick we found
          // earlier.  If we find a match, save the candidate.
          MachineInstr *matching_pick =
            seq_candidate_match_switch(MI, current_loop.header, TII, pickMap);
          if (matching_pick) {
            CSASeqCandidate nc = CSASeqCandidate(matching_pick, MI);
            current_loop.candidates.push_back(nc);
          }
        }

        // Initialize other state information about the loop after we
        // have added the relevant information about loop candidates.
        current_loop.init_from_header();
        loops->push_back(current_loop);
      } else {
        //        DEBUG(errs() << "No match for header instruction " << *MI <<
        //        "\n");
      }

      ++iterMI;
    }
  }
}

void CSAOptDFPass::runSequenceOptimizations(SequenceOptMode seq_opt_mode) {
  if (seq_opt_mode != SequenceOptMode::off) {

    // Do analysis to identify candidates for sequence optimization.
    LLVM_DEBUG(errs() << "Running analysis for sequence optimizations\n");

    // Look for the candidates we might replace with sequences.
    SmallVector<CSASeqLoopInfo, SEQ_VEC_WIDTH> loops;
    seq_find_candidate_loops(&loops);

    // Only print after classification now.
    //    seq_print_loop_info(&loops);

    LLVM_DEBUG(errs() << "Classifying sequence op types\n");
    seq_analyze_loops(&loops);
    LLVM_DEBUG(errs() << "After classification: \n");
    seq_print_loop_info(&loops);

    LLVM_DEBUG(errs() << "Done with sequence classification\n");
    if (seq_opt_mode > SequenceOptMode::analysis) {
      // Actually do the transforms.

      int num_transformed = 0;
      int loop_count      = 0;
      for (auto it = loops.begin(); it != loops.end(); ++it) {
        CSASeqLoopInfo &loop = *it;

        if (loop.sequence_transform_is_valid()) {
          seq_do_transform_loop(loop);
          num_transformed++;
          LLVM_DEBUG(errs() << "Successful transform of loop " << loop_count
                     << ".\n");
        } else {
          LLVM_DEBUG(errs() <<
                     "Failed transform of loop " << loop_count << ".\n");
        }
        loop_count++;

        if (num_transformed > SequenceMaxPerLoop) {
          LLVM_DEBUG(errs() << "Reached transform loop limit. Stopping\n");
          break;
        }
      }
      LLVM_DEBUG(errs() << "Done with seq opt. Transformed " << num_transformed
                 << " loops\n");
    }
  } else {
    LLVM_DEBUG(errs() << "Sequence optimizations disabled\n");
  }
}

// Return the MachineInstr* if it is the single def of the Reg.
// This method is a simplfication of the method implemented in
// TwoAddressInstructionPass
MachineInstr *CSAOptDFPass::getSingleDef(unsigned Reg,
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
MachineInstr *CSAOptDFPass::getSingleUse(unsigned Reg,
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

CSASeqCandidate::SeqType
CSAOptDFPass::seq_classify_repeat_or_reduction(CSASeqCandidate &x) {
  assert(x.pickInst && x.switchInst);

  // Example:
  // %CI64_10 = PICK64 %CI1_0, %CI64_11, %CI64_12
  // %IGN, %CI64_12 = SWITCH64 %CI1_8, %CI64_13

  // Get the source of the switch. In the example above, this register
  // is %CI64_13.
  MachineOperand *bottom_op = x.get_switch_bottom_op();
  MachineOperand *top_op    = x.get_pick_top_op();
  MachineRegisterInfo *MRI  = &thisMF->getRegInfo();

  if (TII->isLIC(*bottom_op, *MRI) && TII->isLIC(*top_op, *MRI)) {

    unsigned bottom_channel = bottom_op->getReg();
    unsigned top_channel    = top_op->getReg();
    // Look at the instruction that defines bottom_op.
    MachineInstr *def_bottom = getSingleDef(bottom_channel, MRI);

    const CSAInstrInfo &TII =
      *static_cast<const CSAInstrInfo *>(thisMF->getSubtarget().getInstrInfo());

    // First, if the defining instruction is the pick itself, then
    // there is no transform body.  We have a repeat.
    if (def_bottom == x.pickInst) {
      assert(top_channel == bottom_channel);
      x.opcode =
        TII.adjustOpcode(x.pickInst->getOpcode(), CSA::Generic::REPEAT);
      assert(x.opcode != CSA::INVALID_OPCODE);
      x.stype         = CSASeqCandidate::SeqType::REPEAT;
      x.transformInst = NULL;
      x.top           = top_channel;
      x.bottom        = bottom_channel;
      return CSASeqCandidate::SeqType::REPEAT;
    }

    if (!def_bottom)
      return CSASeqCandidate::SeqType::UNKNOWN;

    if (SeqReduction) {
      // Next, look for reductions or sequence values.
      // To find these transforming bodies, we want
      //  a single add/sub instruction, which
      //  uses top_op's definition as one of its inputs.

      // For a reduction, we want no other uses of top, except in the
      // the transforming instruction itself.
      if (getSingleUse(top_channel, MRI) == def_bottom) {
        // 3 main cases of interesting reduction operations.
        bool is_commuting_3op_reduction =
          TII.isCommutingReductionTransform(def_bottom);
        bool is_fma = TII.isFMA(def_bottom);
        bool is_sub = TII.isSub(def_bottom);

        if (is_fma || is_commuting_3op_reduction || is_sub) {
          // Figure out whether the last operand of the transform
          // instruction is the output of the pick.
          // To do reductions for "fma" and "sub", it needs to be.
          //
          bool matched_last_use = false;

          // Look up the last two data inputs by navigating from the first
          // input operand. In the case of FMA, where there are three input
          // operands, we look at the last two.
          const MCInstrDesc &desc = def_bottom->getDesc();
          unsigned pair_first_op  = desc.getNumDefs() + (is_fma ? 1 : 0);

          MachineOperand *prev_op = &def_bottom->getOperand(pair_first_op);
          MachineOperand *last_op = &def_bottom->getOperand(pair_first_op + 1);

          if (last_op->isReg() && (last_op->getReg() == top_channel)) {
            matched_last_use = true;
          }

          MachineOperand *input0_op = NULL;
          if (is_fma) {
            if (!matched_last_use) {
              LLVM_DEBUG(errs() << "WARNING: FMA reduction with transform "
                         << *def_bottom
                         << " does not have last input == pick output.\n");
              return CSASeqCandidate::SeqType::UNKNOWN;
            }
            // For FMA, we don't care about setting input0_op.
            // We will look it up from the transform instruction directly later.
          } else if (is_commuting_3op_reduction) {
            // Ops that commute and
            input0_op = (matched_last_use ? prev_op : last_op);
          } else {
            // Should be subtraction.
            assert(is_sub);
            if (!matched_last_use) {
              LLVM_DEBUG(errs() << "WARNING: FMA reduction with transform "
                         << *def_bottom
                         << " does not have last input == pick output.\n");
              return CSASeqCandidate::SeqType::UNKNOWN;
            }
            input0_op = prev_op;
          }

          unsigned reduction_opcode =
            TII.convertTransformToReductionOp(def_bottom->getOpcode());
          if (reduction_opcode == CSA::INVALID_OPCODE) {
            LLVM_DEBUG(errs() << "WARNING: Potential reduction with transform "
                       << *def_bottom << " invalid or not implemented.\n");
            return CSASeqCandidate::SeqType::UNKNOWN;
          }

          LLVM_DEBUG(errs() << "Found reduction transform body " << *def_bottom
                     << "\n");
          x.opcode        = reduction_opcode;
          x.stype         = CSASeqCandidate::SeqType::REDUCTION;
          x.transformInst = def_bottom;
          x.top           = top_channel;
          x.bottom        = bottom_channel;
          x.saved_op      = input0_op;
          return CSASeqCandidate::SeqType::REDUCTION;
        }
      }
    }
  }
  return CSASeqCandidate::SeqType::UNKNOWN;
}

CSASeqCandidate::SeqType CSAOptDFPass::seq_classify_stride(
  CSASeqCandidate &x, const DenseMap<unsigned, int> &repeat_channels) {
  assert(x.pickInst && x.switchInst);
  MachineOperand *bottom_op = x.get_switch_bottom_op();
  MachineOperand *top_op    = x.get_pick_top_op();
  MachineRegisterInfo *MRI  = &thisMF->getRegInfo();

  if (TII->isLIC(*bottom_op, *MRI) && TII->isLIC(*top_op, *MRI)) {

    unsigned bottom_channel = bottom_op->getReg();
    unsigned top_channel    = top_op->getReg();
    // Look at the instruction that defines bottom_op.
    MachineInstr *def_bottom = getSingleDef(bottom_channel, MRI);

    if (!def_bottom)
      return x.stype;

    const CSAInstrInfo &TII =
      *static_cast<const CSAInstrInfo *>(thisMF->getSubtarget().getInstrInfo());

    bool is_add = TII.isAdd(def_bottom);
    bool is_sub = TII.isSub(def_bottom);

    if (is_add || is_sub) {
      if (def_bottom->getNumOperands() == 3) {

        // Bail out if our add instruction doesn't match one of our
        // known striding operations yet.
        // TBD(jsukha): We haven't added code to deal with "sub" yet.
        unsigned stride_opcode;
        int negate_input = is_sub;
        int stride_idx;

        if (is_add) {
          // First, figure out the potential stride, by looking for the
          // top input.
          MachineOperand &add0 = def_bottom->getOperand(1);
          MachineOperand &add1 = def_bottom->getOperand(2);

          if (add0.isReg() && (add0.getReg() == top_channel)) {
            stride_idx = 2;
          } else if (add1.isReg() && (add1.getReg() == top_channel)) {
            stride_idx = 1;
          } else {
            // Neither matches top. We have something weird.
            LLVM_DEBUG(errs() << "Add inst " << *def_bottom
                       << " doesn't match sequence we expect.\n");
            return x.stype;
          }

          stride_opcode =
            TII.adjustOpcode(def_bottom->getOpcode(), CSA::Generic::STRIDE);
          if (stride_opcode == CSA::INVALID_OPCODE) {
            LLVM_DEBUG(errs() << "WARNING: stride operation for add transform "
                       << *def_bottom << " not implemented yet...\n");
            return x.stype;
          }
        } else {
          assert(is_sub);
          MachineOperand &sub0 = def_bottom->getOperand(1);
          // For sub, the first input must be the top, and the second
          // the stride.
          if (sub0.isReg() && (sub0.getReg() == top_channel)) {
            stride_idx = 2;
          } else {
            // Neither matches top. We have something weird.
            LLVM_DEBUG(errs() << "Sub inst " << *def_bottom
                       << " doesn't match sequence we expect.\n");
            return x.stype;
          }
          stride_opcode =
            TII.adjustOpcode(def_bottom->getOpcode(), CSA::Generic::STRIDE);
          if (stride_opcode == CSA::INVALID_OPCODE) {
            LLVM_DEBUG(errs() << "WARNING: stride operation for sub transform "
                       << *def_bottom << " not implemented yet...\n");
            return x.stype;
          }
        }

        MachineOperand &stride_op = def_bottom->getOperand(stride_idx);
        if (stride_op.isImm() ||
            (stride_op.isReg() && (repeat_channels.find(stride_op.getReg()) !=
                                   repeat_channels.end()))) {
          x.top           = top_channel;
          x.bottom        = bottom_channel;
          x.saved_op      = &stride_op;
          x.stype         = CSASeqCandidate::SeqType::STRIDE;
          x.transformInst = def_bottom;
          x.opcode        = stride_opcode;
          x.negate_input  = negate_input;
          return CSASeqCandidate::SeqType::STRIDE;
        }
      } else {
        LLVM_DEBUG(errs() <<
                   "Classify stride found weird add/sub " << *def_bottom <<
                   " does not have 3 operands. Skipping\n");
      }
    }
  }
  return x.stype;
}

CSASeqCandidate::SeqType
CSAOptDFPass::seq_classify_memdep_graph(CSASeqCandidate &x) {
  assert(x.pickInst && x.switchInst);
  MachineOperand *bottom_op = x.get_switch_bottom_op();
  MachineOperand *top_op    = x.get_pick_top_op();

  if ((x.pickInst->getOpcode() == CSA::PICK1) &&
      (x.switchInst->getOpcode() == CSA::SWITCH1) && bottom_op->isReg() &&
      top_op->isReg()) {

    MachineRegisterInfo *MRI = &thisMF->getRegInfo();

    unsigned source_reg = bottom_op->getReg();
    unsigned sink_reg   = top_op->getReg();

    // If the input operand to x.switchInst is generated from a .memdep_sink,
    // then treat this memory dependency as removable.  TBD: Eventually, also
    // remove the .memdep_sink instruction.
    MachineOperand &switchInput = x.switchInst->getOperand(3);
    if (switchInput.isReg()) {
      unsigned switchReg    = switchInput.getReg();
      MachineInstr *srcInst = getSingleDef(switchReg, MRI);
      if (srcInst && CSA::CSA_PARALLEL_MEMDEP == srcInst->getOpcode()) {
        assert(0 && "CSA::CSA_PARALLEL_MEMDEP is no longer in use");
        // TBD: Delete .memdep_sink here
        LLVM_DEBUG(errs() << "Remove back edge from memdep_sink\n");
        x.stype         = CSASeqCandidate::SeqType::PARLOOP_MEM_DEP;
        x.transformInst = NULL;
        x.top           = sink_reg;
        x.bottom        = source_reg;
        return x.stype;
      }
    }

    // If the knob setting is 2, just ASSUME we have identified a memory
    // dependency here, instead of trying to walk the graph to verify
    // we have one.  What could possibly go wrong here?
    // Our goal is to break that dependency.
    if (SeqBreakMemdep >= 2) {
      LLVM_DEBUG(errs() << "ASSUMED we have a memdep candidate.\n");
      LLVM_DEBUG(errs() <<
                 "The flag was set.. it is not my fault if it doesn't work!\n");
      x.stype         = CSASeqCandidate::SeqType::PARLOOP_MEM_DEP;
      x.transformInst = NULL;
      x.top           = sink_reg;
      x.bottom        = source_reg;
      return x.stype;
    }

    // Otherwise, we are going to attempt to verify that we have a
    // dependency chain of memory operations via a BFS, which tries to
    // walk back from the switch to the pick.
    //
    // We are walking back through the def/use through MERGE1, PICK1,
    // SWITCH1, any OLD* or OST* instructions, and the opcode for
    // memory ordering tokens (nominally MOV0).

    const CSAInstrInfo &TII =
      *static_cast<const CSAInstrInfo *>(thisMF->getSubtarget().getInstrInfo());
    const unsigned MemOpMOVOpcode = TII.getMemTokenMOVOpcode();

    const int MAX_LEVELS = 10000;
    int num_levels       = 0;

    // Two frontiers for the BFS.
    std::set<MachineInstr *> b0;
    std::set<MachineInstr *> b1;

    MachineInstr *first_inst = getSingleDef(source_reg, MRI);
    if (first_inst) {
      b0.insert(first_inst);
    }
    // Pointers to the frontier vectors.
    std::set<MachineInstr *> *p_current = &b0;
    std::set<MachineInstr *> *p_next    = &b1;

    while ((p_current->size() > 0) && (num_levels < MAX_LEVELS)) {
      for (std::set<MachineInstr *>::iterator it = p_current->begin();
           it != p_current->end(); ++it) {
        MachineInstr *MI = *it;

        LLVM_DEBUG(errs() <<
                   " MemGraph processing: current ins = " << *MI << "\n");

        if (MI == x.pickInst) {
          if ((p_current->size() == 1) && (p_next->size() == 0)) {
            LLVM_DEBUG(errs() << "Found memdep candidate\n");
            x.stype         = CSASeqCandidate::SeqType::PARLOOP_MEM_DEP;
            x.transformInst = NULL;
            x.top           = sink_reg;
            x.bottom        = source_reg;
            return x.stype;
          } else {
            // Ignore this pick for now.  The pick can be reached from
            // multiple paths, and we want each one of them to end up
            // here.
            LLVM_DEBUG(errs() <<
                       "Reached pick, but frontier not empty yet. Maybe "
                       "other paths\n");
          }
        } else {
          // Walk backwards from the current instruction, and look for
          unsigned current_op = MI->getOpcode();
          MachineOperand *nextOp[2];
          nextOp[0]   = nullptr;
          nextOp[1]   = nullptr;
          int num_ops = 0;

          // Handle 3 different types of instructions.  Look up the
          // previous op in the chain.
          if (current_op == MemOpMOVOpcode) {
            assert(MI->getNumOperands() == 2);
            nextOp[0] = &MI->getOperand(1);
            num_ops   = 1;
          } else if (current_op == CSA::MERGE1) {
            assert(MI->getNumOperands() == 4);
            // Check that the merge selector is an immediate.  This
            // would be consistent with one of our special "merge1"
            // operators we inserted in the memory dependency
            // processing.
            if (MI->getOperand(1).isImm()) {
              nextOp[0] = &MI->getOperand(2);
              nextOp[1] = &MI->getOperand(3);
              num_ops   = 2;
            }
          } else if (current_op == CSA::PICK1) {
            assert(MI->getNumOperands() == 4);
            nextOp[0] = &MI->getOperand(2);
            nextOp[1] = &MI->getOperand(3);
            num_ops   = 2;
          } else if (current_op == CSA::SWITCH1) {
            assert(MI->getNumOperands() == 4);
            nextOp[0] = &MI->getOperand(3);
            num_ops   = 1;
          } else if (TII.isLoad(MI) || TII.isStore(MI)) {
            int num_operands = MI->getNumOperands();
            nextOp[0]        = &MI->getOperand(num_operands - 1);
            num_ops          = 1;
          }

          if (num_ops > 0) {
            for (int i = 0; i < num_ops; ++i) {
              if (nextOp[i]->isReg()) {
                unsigned next_reg = nextOp[i]->getReg();
                if ((next_reg != CSA::IGN) &&
                    TargetRegisterInfo::isPhysicalRegister(next_reg)) {
                  MachineInstr *def_inst = getSingleDef(next_reg, MRI);
                  if (def_inst) {
                    p_next->insert(def_inst);
                    continue;
                  }
                }
              }
              LLVM_DEBUG(errs() << "Unknown op folloing chain, in " << *MI
                         << ". Can't match\n");
              return CSASeqCandidate::SeqType::UNKNOWN;
            }
          } else {
            LLVM_DEBUG(errs() << "Could not follow chain of memory ops." << *MI
                       << ".  Can't match\n");
            return CSASeqCandidate::SeqType::UNKNOWN;
          }
        }
      }
      num_levels++;

      p_current->clear();
      // Swap current and next, for next level in BFS.
      std::swap(p_current, p_next);
    }

    LLVM_DEBUG(errs() << "Falling through. stopping chain after " << num_levels
               << " levels of searching...\n");
  }
  return CSASeqCandidate::SeqType::UNKNOWN;
}

// Look for a match between bottom and the cmp0/cmp1 channels.
// If we find a match, save the result in the current loop.
//
// Returns true if we found a match, false otherwise.
inline bool update_header_cmp_channels(CSASeqLoopInfo &current_loop,
                                       int loop_idx, unsigned bottom) {
  CSASeqLoopInfo::CmpMatchType ctype;
  // This method in CSASeqLoopInfo does all the hard work.
  // The remainder of this method is just error reporting.
  ctype = current_loop.match_candidate_with_cmp(bottom, loop_idx);

  switch (ctype) {
  case CSASeqLoopInfo::CmpMatchType::Match0:
  case CSASeqLoopInfo::CmpMatchType::Match1:
    return true;

  case CSASeqLoopInfo::CmpMatchType::Dup0:
    LLVM_DEBUG(errs() <<
               "WARNING: Finding duplicate seq def for cmp0. Ignoring\n");
    LLVM_DEBUG(errs() << "Duplicate def of " << bottom << " is at idx "
               << current_loop.cmp0Idx() << "\n");
    return false;

  case CSASeqLoopInfo::CmpMatchType::Dup1:
    LLVM_DEBUG(errs() <<
               "WARNING: Finding duplicate seq def for cmp1. Ignoring\n");
    LLVM_DEBUG(errs() << "Duplicate def of " << bottom << " is at idx "
               << current_loop.cmp1Idx() << "\n");
    return false;

  case CSASeqLoopInfo::CmpMatchType::NoMatch:
    return false;

  case CSASeqLoopInfo::CmpMatchType::OtherError:
  default:
    LLVM_DEBUG(errs() <<
               "ERROR: encountering bad bottom channel in loop...\n");
    assert(0);
    return false; // Should not be reached
  }
}

// Partition the sequence candidates in "current_loop.candidates"
// into repeats, reductions, or other.
void CSAOptDFPass::seq_classify_loop_repeats(
  CSASeqLoopInfo &current_loop,
  SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &repeats,
  SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &reductions,
  SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &other) {
  repeats.clear();
  reductions.clear();
  other.clear();

  // First pass: look for repeat / reduction.
  for (auto it = current_loop.candidates.begin();
       it != current_loop.candidates.end(); ++it) {

    CSASeqCandidate &x = *it;
    CSASeqCandidate::SeqType stype;

    stype = seq_classify_repeat_or_reduction(x);

    if (stype == CSASeqCandidate::SeqType::REPEAT) {
      // Save the channel into our list and map of repeat
      // candidates.    The index in this
      // repeat_candidate vector will be the same as the
      // index in the final vector.
      int idx = repeats.size();
      repeats.push_back(x);
      current_loop.repeat_channels[x.bottom] = idx;

      // Look for a match for this repeat in the comparison
      // channels.
      update_header_cmp_channels(current_loop, idx, x.bottom);
    } else if (stype == CSASeqCandidate::SeqType::REDUCTION) {
      reductions.push_back(x);
    } else {
      other.push_back(x);
    }
  }

  LLVM_DEBUG(errs() << "Repeat phase: Loop " << current_loop.loop_id << "\n");
  LLVM_DEBUG(errs() << "  Repeat candidates: " << repeats.size() << "\n");
  LLVM_DEBUG(errs() << "  Reduction candidates: " << reductions.size() << "\n");
  LLVM_DEBUG(errs() << "  Other candidates: " << other.size() << "\n");
  LLVM_DEBUG(errs() << "  Total candidates: " << current_loop.candidates.size()
             << "\n");
  LLVM_DEBUG(errs() << "  Repeat channels found: "
             << current_loop.repeat_channels.size() << "\n");
}

// Classify the candidates in "reductions", (which are ideally
// potential reductions) as either reductions or stride operations.
//
// Insert them into current_loop.candidates if successful.
// Otherwise, insert them into "remaining".
void CSAOptDFPass::seq_classify_loop_reductions_as_strides(
  SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &reductions,
  CSASeqLoopInfo &current_loop,
  SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &remaining) {
  // Move each of the reductions over into the candidates.  Since
  // a stride is sometimes a special case of a reduction, we
  // double-check all the reductions, now that we've identified
  // the repeats.
  for (int i = 0; i < (int)reductions.size(); ++i) {
    CSASeqCandidate &x = reductions[i];
    int idx            = current_loop.candidates.size();
    CSASeqCandidate::SeqType stype;
    stype = seq_classify_stride(x, current_loop.repeat_channels);

    if ((stype == CSASeqCandidate::SeqType::STRIDE) ||
        (stype == CSASeqCandidate::SeqType::REDUCTION)) {
      current_loop.candidates.push_back(x);
      update_header_cmp_channels(current_loop, idx, x.bottom);
    } else {
      remaining.push_back(x);
    }
  }

  LLVM_DEBUG(errs() <<
             "Reduction phase:  Loop " << current_loop.loop_id << "\n");
  LLVM_DEBUG(errs() << "   Repeats, reductions, and strides: "
             << current_loop.candidates.size() << "\n");
  LLVM_DEBUG(errs() << "   Remaining candidates: " << remaining.size() << "\n");
}

// Classify any candidates in "remaining".
// Push them into current_loop.candidates if they are successfully classified,
// or into "other" if they are unknown.
void CSAOptDFPass::seq_classify_loop_remaining(
  SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &remaining,
  CSASeqLoopInfo &current_loop,
  SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> &other) {
  for (CSASeqCandidate &x : remaining) {
    CSASeqCandidate::SeqType stype =
      seq_classify_stride(x, current_loop.repeat_channels);

    // Try to classify memory dependency candidates, if the knob
    // is set.
    if (SeqBreakMemdep > 0) {
      if (stype == CSASeqCandidate::SeqType::UNKNOWN) {
        stype = seq_classify_memdep_graph(x);
      }
    }

    if ((stype == CSASeqCandidate::SeqType::UNKNOWN) ||
        (stype == CSASeqCandidate::SeqType::INVALID)) {
      // Mark the remaining candidates as invalid.
      x.stype = CSASeqCandidate::SeqType::INVALID;
      other.push_back(x);
    } else {
      int loop_idx = current_loop.candidates.size();
      current_loop.candidates.push_back(x);
      // Again, look for a match with this sequence and the
      // compare instruction.
      update_header_cmp_channels(current_loop, loop_idx, x.bottom);
    }
  }
}

// Analyze all the loops that we found, to identify candidates for
// sequence transformation.
void CSAOptDFPass::seq_analyze_loops(
  SmallVector<CSASeqLoopInfo, SEQ_VEC_WIDTH> *loops) {

  if (loops) {
    for (unsigned i = 0; i < loops->size(); ++i) {
      CSASeqLoopInfo &current_loop = (*loops)[i];

      SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> repeat_candidates;
      SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> reductions;
      SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> remaining;

      // Step 1: scan for repeats, and partition the candidates into
      // repeats/reductions/remaining.
      seq_classify_loop_repeats(current_loop, repeat_candidates, reductions,
                                remaining);

      // Move the repeat candidates into the current loop.
      // current loop.
      current_loop.candidates.clear();
      current_loop.candidates.insert(current_loop.candidates.end(),
                                     repeat_candidates.begin(),
                                     repeat_candidates.end());

      // Step 2: Classify the potential reductions as either
      // reduction, stride, or unknown.  Anything successfully
      // classified goes into "current_loop.candidates".  Everything
      // else goes into remaining.
      seq_classify_loop_reductions_as_strides(reductions, current_loop,
                                              remaining);

      // Step 3: classify everything else that is in remaining.
      SmallVector<CSASeqCandidate, SEQ_VEC_WIDTH> other;
      seq_classify_loop_remaining(remaining, current_loop, other);

      // Remember the number of sequences that we can transform.
      current_loop.set_valid_sequence_count(current_loop.candidates.size());

      LLVM_DEBUG(errs() << "Final classification: Loop " << current_loop.loop_id
                 << "\n");
      LLVM_DEBUG(errs() << "   Invalid candidates: " << other.size() << "\n");
      LLVM_DEBUG(errs() <<
                 "   Valid candidates: " << current_loop.candidates.size()
                 << "\n");

      // Print the invalid candidates that we are ignoring.
      LLVM_DEBUG(errs() << "Invalid candidates: \n");
      for (unsigned j = 0; j < other.size(); ++j) {
        seq_debug_print_candidate(other[j]);
      }

      // NOTE: If we wanted to keep track of the invalid candidates
      // in a loop, we could insert it into the list.  But we aren't here.
      //
      //  current_loop.candidates.insert(current_loop.candidates.end(),
      //                                 other.begin(), other.end());

      bool can_transform = seq_identify_induction_variable(current_loop);
      LLVM_DEBUG(errs() << "Loop " << current_loop.loop_id
                 << ": can transform = " << can_transform << "\n");
      (void) can_transform;
    }
  }
}

bool CSAOptDFPass::seq_identify_induction_variable(CSASeqLoopInfo &loop) {
  const CSAInstrInfo &TII =
    *static_cast<const CSAInstrInfo *>(thisMF->getSubtarget().getInstrInfo());

  // Found a valid induction variable.
  bool found_indvar = loop.find_induction_variable();
  if (!found_indvar) {
    LLVM_DEBUG(errs() << "Seq transform failed: invalid induction variable.\n");
    return false;
  }

  bool found_bound = loop.has_valid_bound();
  if (!found_bound) {
    int boundIdx = loop.boundIdx();
    LLVM_DEBUG(errs() << "Seq transform failed: no valid bound (e.g., possible "
               "non-constant loop).\n");
    LLVM_DEBUG(errs() << "Boundidx = " << boundIdx << "\n");
    (void) boundIdx;
    if (loop.boundIdx() >= 0) {
      seq_debug_print_candidate(loop.candidates[loop.boundIdx()]);
    }
    return false;
  }

  bool last_transform_check = loop.sequence_opcode_transform_check(TII);
  if (!last_transform_check) {

    LLVM_DEBUG(errs() << "Failing last seq transform check...\n");
    LLVM_DEBUG(errs() << "Indvar idx is " << loop.indvarIdx() << "\n");
    if (loop.indvarIdx() >= 0) {
      LLVM_DEBUG(errs() << "Induction variable sequence is ...\n");
      seq_debug_print_candidate(loop.candidates[loop.indvarIdx()]);
    }
  }

  return loop.sequence_opcode_transform_check(TII);
}

void CSAOptDFPass::disconnect_instruction(MachineInstr *MI) {
  for (MachineOperand &MO : MI->operands()) {
    if (!MO.isReg())
      continue;

    unsigned replacement = MO.isDef() ? CSA::IGN : CSA::NA;

    const TargetRegisterInfo &TRI = *thisMF->getSubtarget().getRegisterInfo();
    MO.substPhysReg(replacement, TRI);
  }
}

MachineInstr *CSAOptDFPass::seq_add_parloop_memdep(
  CSASeqCandidate &sc, const CSASeqHeader &loop_header, unsigned pred_reg,
  const CSAInstrInfo &TII, MachineBasicBlock &BB, MachineInstr *prev_inst) {
  assert(sc.stype == CSASeqCandidate::SeqType::PARLOOP_MEM_DEP);
  MachineInstr *repinst = BuildMI(BB, prev_inst, sc.pickInst->getDebugLoc(),
                                  TII.get(CSA::REPEAT1), sc.top)
                            .addReg(pred_reg)
                            .add(*sc.get_pick_input_op(loop_header));
  repinst->setFlag(MachineInstr::NonSequential);

  MachineOperand *out_s_op = sc.get_switch_output_op(loop_header);
  assert(out_s_op->isReg());

  MachineInstr *onend_inst = BuildMI(BB, repinst, sc.switchInst->getDebugLoc(),
                                     TII.get(CSA::ONEND), out_s_op->getReg())
                               .addReg(pred_reg)
                               .addReg(sc.bottom);
  onend_inst->setFlag(MachineInstr::NonSequential);

  return onend_inst;
}

MachineInstr *CSAOptDFPass::seq_add_repeat(
  CSASeqCandidate &sc, const CSASeqHeader &loop_header, unsigned pred_reg,
  const CSAInstrInfo &TII, MachineBasicBlock &BB, MachineInstr *prev_inst) {
  assert(sc.stype == CSASeqCandidate::SeqType::REPEAT);
  assert(sc.opcode != CSASeqCandidate::INVALID_OPCODE);

  MachineInstr *repinst = BuildMI(BB, prev_inst, sc.pickInst->getDebugLoc(),
                                  TII.get(sc.opcode), sc.top)
                            .addReg(pred_reg)
                            .add(*sc.get_pick_input_op(loop_header));
  repinst->setFlag(MachineInstr::NonSequential);
  return repinst;
}

MachineInstr *CSAOptDFPass::seq_add_stride(
  CSASeqCandidate &sc, const CSASeqHeader &loop_header, unsigned pred_reg,
  MachineOperand *in_stride_op, const CSAInstrInfo &TII, MachineBasicBlock &BB,
  MachineInstr *prev_inst) {
  assert(sc.stype == CSASeqCandidate::SeqType::STRIDE);
  assert(sc.opcode != CSASeqCandidate::INVALID_OPCODE);

  assert(in_stride_op);
  MachineInstrBuilder MIB = BuildMI(BB, prev_inst, sc.pickInst->getDebugLoc(),
                                    TII.get(sc.opcode), sc.top)
                              .addReg(pred_reg)
                              .add(*sc.get_pick_input_op(loop_header));
  if (in_stride_op->isReg())
    MIB.addReg(in_stride_op->getReg());
  else
    MIB.add(*in_stride_op);
  MachineInstr *strideInst = MIB;

  strideInst->setFlag(MachineInstr::NonSequential);
  return strideInst;
}

MachineInstr *CSAOptDFPass::seq_add_reduction(
  CSASeqCandidate &sc, const CSASeqHeader &loop_header, unsigned pred_reg,
  const CSAInstrInfo &TII, MachineBasicBlock &BB, MachineInstr *prev_inst,
  bool is_fma_reduction) {
  assert(sc.stype == CSASeqCandidate::SeqType::REDUCTION);
  assert(sc.opcode != CSASeqCandidate::INVALID_OPCODE);

  // Output register is the output of the switch.
  MachineOperand *output_op = sc.get_switch_output_op(loop_header);
  assert(output_op->isReg());
  unsigned output_reg = output_op->getReg();

  MachineInstr *red_inst;
  if (is_fma_reduction) {
    // FMAs have two inputs to the sequence reduction.
    // Just look them up directly from the transform instruction.
    MachineOperand *input0_op = sc.get_fma_mul_op(0);
    MachineOperand *input1_op = sc.get_fma_mul_op(1);

    red_inst =
      BuildMI(BB, prev_inst, sc.pickInst->getDebugLoc(), TII.get(sc.opcode),
              output_reg)
        . // result
      addReg(sc.bottom, RegState::Define)
        . // each == bottom
      add(*sc.get_pick_input_op(loop_header))
        . // initial value
      add(*input0_op)
        . // input0
      add(*input1_op)
        .               // input1
      addReg(pred_reg); // control
  } else {
    // Only one input argument for normal sequence/reduction.  We
    // saved the op away earlier (when we had to figure out which one
    // of the two it is, in cases where the op can commute).
    MachineOperand *input0_op = sc.saved_op;
    red_inst =
      BuildMI(BB, prev_inst, sc.pickInst->getDebugLoc(), TII.get(sc.opcode),
              output_reg)
        . // result
      addReg(sc.bottom, RegState::Define)
        . // each == bottom
      add(*sc.get_pick_input_op(loop_header))
        . // initial value
      add(*input0_op)
        .               // input0
      addReg(pred_reg); // control
  }

  red_inst->setFlag(MachineInstr::NonSequential);
  return red_inst;
}

MachineInstr *CSAOptDFPass::seq_add_output_switch_for_seq_candidate(
  CSASeqCandidate &sCandidate, const CSASeqHeader &loop_header,
  unsigned last_reg, const CSAInstrInfo &TII, MachineBasicBlock &BB,
  MachineInstr *prev_inst) {
  MachineInstr *output_switch = NULL;
  MachineOperand *out_s_op    = sCandidate.get_switch_output_op(loop_header);

  if (!(out_s_op->isReg() && (out_s_op->getReg() == CSA::IGN))) {
    assert(sCandidate.bottom > 0);

    output_switch =
      BuildMI(BB, prev_inst, sCandidate.switchInst->getDebugLoc(),
              TII.get(sCandidate.switchInst->getOpcode()), CSA::IGN)
        .addReg(out_s_op->getReg(), RegState::Define)
        .addReg(last_reg)
        .addReg(sCandidate.bottom);

    output_switch->setFlag(MachineInstr::NonSequential);
  }
  return output_switch;
}

MachineOperand *
CSAOptDFPass::seq_lookup_stride_op(CSASeqLoopInfo &loop,
                                   CSASeqCandidate &scandidate) {
  // For stride, first look in the sequence candidate op.
  //
  // If this stride op is a LIC (instead of an immediate), we
  // look up the input of the matching repeat instead.
  //
  // Otherwise, it it should be a literal operand.
  MachineOperand *in_s_op        = scandidate.saved_op;
  CSASeqCandidate *stride_repeat = NULL;
  if (in_s_op->isReg()) {
    unsigned bottom_s_reg = in_s_op->getReg();
    if (loop.repeat_channels.find(bottom_s_reg) != loop.repeat_channels.end()) {
      unsigned stride_idx = loop.repeat_channels[bottom_s_reg];
      assert(stride_idx < loop.candidates.size());
      stride_repeat = &loop.candidates[stride_idx];
      in_s_op       = stride_repeat->get_pick_input_op(loop.header);
    } else {
      // We should have matched the register for this stride op with a
      // repeat earlier.
      LLVM_DEBUG(errs() << "ERROR: can't find repeat channel for stride...\n");
      return NULL;
    }
  }
  return in_s_op;
}

// Creates a "NEG" instruction to negate the specified input stride
// operation.
//
// Returns the output operand of this instruction.
MachineOperand *CSAOptDFPass::seq_add_negate_stride_op(
  MachineOperand *in_stride_op, unsigned stride_opcode, const CSAInstrInfo &TII,
  CSAMachineFunctionInfo *LMFI, MachineBasicBlock &BB,
  MachineInstr *prev_inst) {
  const TargetRegisterClass *myRC = TII.getStrideInputRC(stride_opcode);
  unsigned new_input_reg          = LMFI->allocateLIC(myRC);
  assert(TII.getGenericOpcode(stride_opcode) == CSA::Generic::STRIDE);
  unsigned neg_opcode = TII.adjustOpcode(stride_opcode, CSA::Generic::NEG);
  assert(neg_opcode != CSA::INVALID_OPCODE);

  MachineInstr *neg_inst = BuildMI(BB, prev_inst, prev_inst->getDebugLoc(),
                                   TII.get(neg_opcode), new_input_reg)
                             .add(*in_stride_op);
  neg_inst->setFlag(MachineInstr::NonSequential);
  return &neg_inst->getOperand(0);
}

void CSAOptDFPass::seq_do_transform_loop_seq(
  CSASeqLoopInfo &loop, MachineBasicBlock *BB, CSASeqInstrInfo *seqInfo,
  SmallVector<MachineInstr *, SEQ_VEC_WIDTH> &insToDisconnect) {

  assert(loop.sequence_transform_is_valid());
  int indvarIdx = loop.indvarIdx();
  assert(indvarIdx >= 0);
  CSASeqCandidate &indvarCandidate = loop.candidates[indvarIdx];
  unsigned indvar_opcode           = loop.get_seq_opcode();

  CSAMachineFunctionInfo *LMFI = thisMF->getInfo<CSAMachineFunctionInfo>();
  const CSAInstrInfo &TII =
    *static_cast<const CSAInstrInfo *>(thisMF->getSubtarget().getInstrInfo());

  unsigned top_i = indvarCandidate.top;
  assert(top_i == indvarCandidate.get_pick_top_op()->getReg());
  (void) top_i;

  // <in_i> may or may not be an immediate, but it must come from the
  // pick.
  MachineOperand *in_i_op = indvarCandidate.get_pick_input_op(loop.header);

  // Get the operand we should use for <in_b>.
  MachineOperand *in_b_op = loop.get_input_bound_op();

  assert(indvarCandidate.transformInst);

  MachineOperand *input_s_op = seq_lookup_stride_op(loop, indvarCandidate);
  MachineOperand *in_s_op    = input_s_op;
  if (indvarCandidate.negate_input) {
    // Create a negation of the stride input, if neccesary.
    in_s_op = seq_add_negate_stride_op(input_s_op, indvarCandidate.opcode, TII,
                                       LMFI, *BB, indvarCandidate.pickInst);
  }

  // All but one of the valid sequences in a loop are dependent.
  int num_dependent_sequences = loop.get_valid_sequence_count() - 1;
  assert(num_dependent_sequences >= 0);

  LLVM_DEBUG(errs() << "For loop " << loop.loop_id << ", dependent sequences = "
             << num_dependent_sequences << "\n");

  // We only need to define a predicate register if we have at least
  // one dependent sequence.
  seqInfo->pred_reg = CSA::IGN;
  if (num_dependent_sequences > 0) {
    seqInfo->pred_reg = LMFI->allocateLIC(
      SeqPredRC, Twine("seq_") + Twine(loop.loop_id) + "_pred");
  }
  seqInfo->first_reg = CSA::IGN;
  seqInfo->last_reg =
    LMFI->allocateLIC(SeqPredRC, Twine("seq_") + Twine(loop.loop_id) + "_last");

  seqInfo->seq_inst = BuildMI(*BB, indvarCandidate.pickInst,
                              indvarCandidate.pickInst->getDebugLoc(),
                              TII.get(indvar_opcode), indvarCandidate.top)
                        .addReg(seqInfo->pred_reg, RegState::Define)
                        .addReg(seqInfo->first_reg, RegState::Define)
                        .addReg(seqInfo->last_reg, RegState::Define)
                        .add(*in_i_op)
                        .add(*in_b_op)
                        .add(*in_s_op);
  seqInfo->seq_inst->setFlag(MachineInstr::NonSequential);

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
  unsigned switcher_ctrl_opcode =
    (loop.header.switcherSense ? CSA::MOV1 : CSA::NOT1);

  MachineInstr *switcher_def_inst =
    BuildMI(*BB, seqInfo->seq_inst, indvarCandidate.pickInst->getDebugLoc(),
            TII.get(switcher_ctrl_opcode), loop.header.switcherChannel)
      .addReg(seqInfo->last_reg);
  switcher_def_inst->setFlag(MachineInstr::NonSequential);

  // If there is a nontrivial output to the switch in the candidate,
  // then insert a switch for the last output.
  //  %ign, <out_i> = switch[n] <last_i>, <bottom_i>
  MachineInstr *output_switch = seq_add_output_switch_for_seq_candidate(
    indvarCandidate, loop.header, seqInfo->last_reg, TII, *BB,
    switcher_def_inst);
  // For the sequence operator, mark the compare, pick and switch for
  // deletion.
  insToDisconnect.push_back(loop.header.compareInst);
  insToDisconnect.push_back(indvarCandidate.pickInst);
  insToDisconnect.push_back(indvarCandidate.switchInst);

  LLVM_DEBUG(errs() << "Transform loop_seq: adding a new sequence instruction "
             << *seqInfo->seq_inst << "\n");
  LLVM_DEBUG(errs() <<
             "   Adding a new switcher def inst " << *switcher_def_inst
             << "\n");
  if (output_switch) {
    LLVM_DEBUG(errs() <<
               "   Adding a switch output instruction " << *output_switch
               << "\n");
  }
}

void CSAOptDFPass::seq_do_transform_loop_repeat(
  CSASeqCandidate &scandidate, CSASeqLoopInfo &loop, MachineBasicBlock *BB,
  const CSASeqInstrInfo &seqInfo, const CSAInstrInfo &TII,
  SmallVector<MachineInstr *, SEQ_VEC_WIDTH> &insToDisconnect) {

  assert(!scandidate.transformInst);
  MachineInstr *repinst = seq_add_repeat(
    scandidate, loop.header, seqInfo.pred_reg, TII, *BB, seqInfo.seq_inst);
  MachineInstr *out_switch = seq_add_output_switch_for_seq_candidate(
    scandidate, loop.header, seqInfo.last_reg, TII, *BB, repinst);

  LLVM_DEBUG(errs() << "do_transform_loop_repeat: adding repeat = " << *repinst
             << "\n");
  if (out_switch) {
    LLVM_DEBUG(errs() << "do_transform_loop_repeat: adding output switch = "
               << *out_switch << "\n");
  }

  insToDisconnect.push_back(scandidate.pickInst);
  insToDisconnect.push_back(scandidate.switchInst);
}

void CSAOptDFPass::seq_do_transform_loop_stride(
  CSASeqCandidate &scandidate, CSASeqLoopInfo &loop, MachineBasicBlock *BB,
  const CSASeqInstrInfo &seqInfo, const CSAInstrInfo &TII,
  SmallVector<MachineInstr *, SEQ_VEC_WIDTH> &insToDisconnect) {

  assert(scandidate.transformInst);

  // We should have already matched the opcodes at the time we
  // classified the candidate.
  assert(scandidate.opcode != CSASeqCandidate::INVALID_OPCODE);

  MachineOperand *in_s_op = seq_lookup_stride_op(loop, scandidate);
  MachineOperand *my_s_op = in_s_op;
  if (scandidate.negate_input) {
    CSAMachineFunctionInfo *LMFI = thisMF->getInfo<CSAMachineFunctionInfo>();
    // Create a negation of the stride input, if neccesary.
    my_s_op = seq_add_negate_stride_op(in_s_op, scandidate.opcode, TII, LMFI,
                                       *BB, seqInfo.seq_inst);
  }

  MachineInstr *stride_inst =
    seq_add_stride(scandidate, loop.header, seqInfo.pred_reg, my_s_op, TII, *BB,
                   seqInfo.seq_inst);

  MachineInstr *out_switch = seq_add_output_switch_for_seq_candidate(
    scandidate, loop.header, seqInfo.last_reg, TII, *BB, stride_inst);
  LLVM_DEBUG(errs() << "do_transform_loop_stride: adding stride = " <<
             *stride_inst << "\n");
  if (out_switch) {
    LLVM_DEBUG(errs() << "do_transform_loop_stride: adding output switch = "
               << *out_switch << "\n");
  }

  insToDisconnect.push_back(scandidate.pickInst);
  insToDisconnect.push_back(scandidate.switchInst);
  // We should NOT mark scandidate.transformInst for deletion
  // here.  In cases where the channel at the bottom of the
  // loop is also used, we still need the add instruction.
  // Moreover, marking the switch for deletion is sufficient
  // to get rid of the add, if that switch is the only use of
  // the add's output.
}

void CSAOptDFPass::seq_do_transform_loop_parloop_memdep(
  CSASeqCandidate &scandidate, CSASeqLoopInfo &loop, MachineBasicBlock *BB,
  const CSASeqInstrInfo &seqInfo, const CSAInstrInfo &TII,
  SmallVector<MachineInstr *, SEQ_VEC_WIDTH> &insToDisconnect) {
  assert(!scandidate.transformInst);
  MachineInstr *onend_inst = seq_add_parloop_memdep(
    scandidate, loop.header, seqInfo.pred_reg, TII, *BB, seqInfo.seq_inst);
  LLVM_DEBUG(errs() << "do_transform_parloop_memdep: adding onend = " <<
             *onend_inst << "\n");
  (void) onend_inst;

  insToDisconnect.push_back(scandidate.pickInst);
  insToDisconnect.push_back(scandidate.switchInst);
}

void CSAOptDFPass::seq_do_transform_loop_reduction(
  CSASeqCandidate &scandidate, CSASeqLoopInfo &loop, MachineBasicBlock *BB,
  const CSASeqInstrInfo &seqInfo, const CSAInstrInfo &TII,
  SmallVector<MachineInstr *, SEQ_VEC_WIDTH> &insToDisconnect) {
  assert(scandidate.transformInst);
  bool is_fma = TII.isFMA(scandidate.transformInst);

  MachineInstr *red_inst =
    seq_add_reduction(scandidate, loop.header, seqInfo.pred_reg, TII, *BB,
                      seqInfo.seq_inst, is_fma);

  LLVM_DEBUG(errs() << "do_transform_loop_reduction: adding reduction = "
             << *red_inst << "\n");
  (void) red_inst;

  insToDisconnect.push_back(scandidate.pickInst);
  insToDisconnect.push_back(scandidate.switchInst);
  insToDisconnect.push_back(scandidate.transformInst);
}

void CSAOptDFPass::seq_do_transform_loop(CSASeqLoopInfo &loop) {
  const CSAInstrInfo &TII =
    *static_cast<const CSAInstrInfo *>(thisMF->getSubtarget().getInstrInfo());

  int indvarIdx = loop.indvarIdx();
  assert(indvarIdx >= 0);
  CSASeqCandidate &indvarCandidate = loop.candidates[indvarIdx];
  MachineBasicBlock *BB            = indvarCandidate.pickInst->getParent();

  SmallVector<MachineInstr *, SEQ_VEC_WIDTH> insToDisconnect;

  // Summary information about the sequence instruction.
  CSASeqInstrInfo seqInfo;

  // Transform the induction variable into a sequence instruction.
  seq_do_transform_loop_seq(loop, BB, &seqInfo, insToDisconnect);

  // Now process all the dependent sequences.
  //
  // Dispatch to the corresponding functions for each kind of
  // dependent sequence.
  for (unsigned idx = 0; idx < loop.candidates.size(); ++idx) {
    // Skip over the loop induction variable.
    // We should not process it twice.
    if (idx != (unsigned)indvarIdx) {
      CSASeqCandidate &scandidate = loop.candidates[idx];
      switch (scandidate.stype) {

      case CSASeqCandidate::SeqType::REPEAT:
        seq_do_transform_loop_repeat(scandidate, loop, BB, seqInfo, TII,
                                     insToDisconnect);
        break;

      case CSASeqCandidate::SeqType::STRIDE:
        seq_do_transform_loop_stride(scandidate, loop, BB, seqInfo, TII,
                                     insToDisconnect);
        break;

      case CSASeqCandidate::SeqType::PARLOOP_MEM_DEP:
        seq_do_transform_loop_parloop_memdep(scandidate, loop, BB, seqInfo, TII,
                                             insToDisconnect);
        break;

      case CSASeqCandidate::SeqType::REDUCTION:
        seq_do_transform_loop_reduction(scandidate, loop, BB, seqInfo, TII,
                                        insToDisconnect);
        break;

      default:
        LLVM_DEBUG(
          errs() << "do_transform: Ignoring sequence candidate in transform: ");
        seq_debug_print_candidate(scandidate);
      }
    }
  }

  // Disconnect all the instructions for all candidates at once.
  for (auto it = insToDisconnect.begin(); it != insToDisconnect.end(); ++it) {
    disconnect_instruction(*it);
  }
}

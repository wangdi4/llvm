//===-- CSASequenceOpt.h - CSA structures and methods for sequence opt.----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSASEQUENCEOPT_H
#define LLVM_LIB_TARGET_CSA_CSASEQUENCEOPT_H


#include "CSATargetMachine.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Target/TargetRegisterInfo.h"

/**
 * Sequence optimization.
 *
 *
 * Every candidate for sequence optimization has a control header
 * which controls the execution of the loop.
 *
 * The *sequence header* has the following form (in CSA machine
 * instructions).
 *
 *      <picker>  = INIT1 0
 *      <picker>  = MOV1 <switcher>
 *      <switcher>    = CMP[*] <cmp0>, <cmp1>
 *
 *
 * This header is accompanied by a list of K *sequence candidates*,
 * which are pick/switch pairs of values that represent values flowing
 * around the loop.
 *
 *      <top_0> = PICK[n] <picker>, <in_0>, <loopBack_0>
 *      <out_0>, <loopBack_0> = SWITCH[n] <switcher>, <bottom_0>
 *      [ Transforming statement(s) from <top_0> into <bottom_0> ]
 *
 *      <top_1> = PICK[n] <picker>, <in_1>, <loopBack_1>
 *      <out_1>, <loopBack_1> = SWITCH[n] <switcher>, <bottom_1>
 *      [ Transforming statement(s) from <top_1> into <bottom_1> ]
 *
 *       ...
 *
 *      <top_Km1> = PICK[n] <picker>, <in_Km1>, <loopBack_Km1>
 *      <out_Km1>, <loopBack_Km1> = SWITCH[n] <switcher>, <bottom_Km1>
 *      [ Transforming statement(s) from <top_Km1> into <bottom_Km1> ]
 *
 *
 * For each sequence candidate, the <loopBack_i> channel should have
 * no other uses other than the PICK instruction.
 *
 * We define some of these sequence candidates as *repeat candidates*
 * if <top_i> == <bottom_i> (i.e., there is no transforming statement).
 *
 * For now, we are only considering relatively simple transforming
 * statements (e.g., linear additions or subtractions).
 *
 *
 * Sequence optimization:
 *
 *  1. Find the sequence candidates + header to match a linear loop
 *     control variable.
 *
 *     One of the sequence candidates (e.g., i) should have a transforming
 *     statement:
 *            <bottom_i> = add[n] <top_i> <stride>
 *
 *     Also, <bottom_i> (or possibly <top_i>?) should match one of the
 *     inputs in the comparison statement in the header.  (Most likely
 *     <cmp0>, but it might be <cmp1> in some cases?).
 *
 *
 *  2. Find (up to) 2 repeat candidates, to handle the stride in the
 *     transforming increment and the bound.  More precisely, <stride>
 *     and <cmp1> should match <top_j> for some j, unless they are
 *     literals.
 *
 *
 *  3. Replace the loop control variable and stride/bound with
 *     sequence + repeat.
 *
 *     Let i = sequence candidate for loop control variable.
 *         b = repeat candidate for bound
 *         s = repeat candidate for stride
 *
 *     We can replace these three candidates, plus the comparison in
 *     the header, with the following operations
 *
 *       (a) seq[*][n] <top_i>, <pred_i>, %ign, <last_i>,
 *                     <in_i>, <in_b>, <in_s>
 *
 *       (b) <switcher> = not1 <last_i>
 *       (c)  <bottom_i> = add[n] <top_i>, <top_s>
 *       (d) %ign, <out_i> = switch[n] <last_i>, <bottom_i>
 *
 *       (e) repeat[n] <top_b>, <pred_i>, <in_b>
 *       (f) %ign, <out_b> = switch[n] <last_i>, <top_b>
 *
 *       (g) repeat[n] <top_s>, <pred_i>, <in_s>
 *       (h) %ign, <out_s> = switch[n] <last_i>, <top_s>
 *
 *
 *     Note that many of these instructions are needed only if there
 *     are other instructions which are using the definitions.  For
 *     example:
 *
 *       -  If <out_i>, <out_b>, or <out_s> are not being used after
 *          the loop, then the final switches (in (d), (f), and (h))
 *          are not needed.
 *
 *       -  If there are no uses of <top_b>, then then (e) can be dropped.
 *
 *       -  Similarly, if <bottom_i> is not used then (c) can be dropped.
 *          This may lead to <top_s> being unused, which in turn can
 *          lead to (g) being dropped.
 *
 *
 *   4. Scan the remaining sequence candidates and connect them to the
 *      predicate from the loop control sequence, if they can be
 *      transformed.  These remaining candidates are *dependent
 *      sequences*, and fall into one of several known types:
 *
 *      (a) Repeat:       <bottom_k> == <top_k>
 *
 *          <top_k> = repeat[n] <pred_i>, <in_k>
 *          %ign, <out_k> = switch[n] <last_i>, <top_k>
 *
 *      (b) Sequence:     <bottom_k> = ADD[n] <top_k>, <stride>
 *          In this case, <stride> is either a literal, or the output
 *          of a repeat.
 *          This pattern gets replaced with a stride operation:
 *
 *          <top_k>    = stride[n] <pred_i>, <in_k>, <stride>
 *          <bottom_k> = add[n] <top_k>, <stride>
 *          %ign, <out_k> = switch[n] <last_i>, <bottom_k>
 *
 *      (c) Reduction:    <bottom_i> = ADD[n] <top_i>, <val>
 *          This pattern is quite similar to a sequence, except that the
 *          intermediate values <bottom_i> and <top_i> should have no
 *          other uses.
 *
 *          <out_k> = reduction <pred_i>, <in_k>, <val>
 *
 *      (d) Parallel loop dependency: The transforming statements from
 *          <bottom_k> to <top_k> is a sequence (or subgraph) of
 *          dependencies in memory operations, merge1 instructions,
 *          and mov1 instructions that have a source at <top_k> and
 *          sink at <bottom_k>.  Also, we must have some indication
 *          this particularly loop is parallel, so that we know it is
 *          safe to break the loop-carried dependency.
 *
 *          <top_k> = repeat1 <pred_i>, <in_k>
 *          <out_k> = onend <pred_i>, <bottom_k>
 *
 *      Note that we should identify repeat candidates before looking
 *      for sequence candidates.
 *
 *
 *   5.  Eliminate unused outputs, and simplify.
 *
 *       This step is analogous to the step in 3.  The only additional
 *       complication is that if all sequence candidates in a loop are
 *       successfully converted, then there may no longer be any uses
 *       of the <picker> and <switcher> channels.  In this case, these
 *       instructions in the sequence header may be eliminated (and
 *       transitively, any simplifications that follow).
 *
 *
 *
 *   As a concrete example, here are the relevant instructions from a
 *   simple sequence example:
 *
 *   Sequence header:
 *	%CI1_0<def> = INIT1 0
 *	%CI1_0<def> = MOV1 %CI1_8
 *	%CI1_8<def> = CMPLTS64 %CI64_13, %CI64_2
 *
 *   Loop induction variable:
 *	%CI64_10 = PICK64 %CI1_0, %CI64_11, %CI64_12
 *      %IGN, %CI64_12 = SWITCH64 %CI1_8, %CI64_13
 *	%CI64_13 = ADD64 %CI64_10, %CI64_5
 *
 *   Loop stride:
 *	%CI64_5 = PICK64 %CI1_0, %CI64_6, %CI64_7
 *	%IGN, %CI64_7 = SWITCH64 %CI1_8, %CI64_5
 *
 *   Loop bound:
 *      %CI64_2 = PICK64 %CI1_0, %CI64_3, %CI64_4
 *	%IGN, %CI64_4 = SWITCH64 %CI1_8, %CI64_2
 *
 *   Parallel loop dependency:
 * 	%CI1_5 = PICK1 %CI1_0, %CI1_6, %CI1_7
 *	%CI1_9, %CI1_7 = SWITCH1 %CI1_8, %CI1_10
 *	%CI1_10 = MOV1 %CI1_14
 *	%CI1_14 = OST64fX %CI64_8, %CI64_10, %CI64_14, %CI1_15;
 *	%CI64_16, %CI1_15 = OLD64fX %CI64_8, %CI64_10, %CI1_5
 *
 * TBD(jsukha):
 *   Add the transformed code, based on the description above.
 *
 */


namespace llvm {

  /**
   * Data structure storing a sequence header.
   *
   *      <picker>  = INIT1 0
   *      <picker>  = MOV1 <switcher>
   *      <switcher>    = CMP[*] <cmp0>, <cmp1>
   *
   * Also saves the channel registers so we don't have to look them up
   * from the machine instructions.
   */
  struct CSASeqHeader {
    MachineInstr* pickerInit;
    MachineInstr* pickerMov1;
    MachineInstr* compareInst;

    unsigned pickerChannel;
    unsigned switcherChannel;

    // This value is the "sense" of the pick instructions controlling
    // the loop.  This value should be 0 / false if the loop is
    // controlled by an "INIT 0", and a 1 / true if the loop is
    // controlled by an "INIT 1".
    bool pickerSense;

    // This value is the sense of the channel controlling the switcher
    // for the loop.  If 0, then the final output from the switch is
    // operand 0, and the loopback value is operand 1.
    //
    // Otherwise, for 1, the final output and loopback values are
    // reversed.
    //
    // TBD(jsukha): USUALLY we expect pickerSense and switcherSense to
    // be matching.  But it is concievable that there could be loops
    // where these values are reversed.
    bool switcherSense;

    CSASeqHeader()
      : pickerInit(NULL)
      , pickerMov1(NULL)
      , compareInst(NULL)
    { }

    void init(MachineInstr* pickerInit_,
              MachineInstr* pickerMov1_,
              MachineInstr* compareInst_,
              unsigned pickerChannel_,
              unsigned switcherChannel_,
              bool pickerSense_,
              bool switcherSense_) {
      this->pickerInit = pickerInit_;
      this->pickerMov1 = pickerMov1_;
      this->compareInst = compareInst_;
      this->pickerChannel = pickerChannel_;
      this->switcherChannel = switcherChannel_;
      this->pickerSense = pickerSense_;
      this->switcherSense = switcherSense_;
    }


    static
    int pick_top_op_idx() {
      return 0;
    }
    static
    int pick_select_op_idx() {
      return 1;
    }
    static
    int switch_select_op_idx() {
      return 2;
    }
    static
    int switch_bottom_op_idx() {
      return 3;
    }

    // A pick for a loop looks like:
    //   pick <out>, <ctrl>, <in0>, <in1>
    //
    // If the loop is 0-intialized, <in0> is the initial value and
    // <in1> is the loopback. Otherwise, it is reversed.
    //
    // These two methods return the correct index to the machine
    // operand that we should use.
    int pick_input_op_idx() const {
      return 2 + this->pickerSense;
    }
    int pick_loopback_op_idx() const {
      return 3 - this->pickerSense;
    }

    // The output switch looks like:
    //  switch <out0>, <out1>, <ctrl>, <in>
    //
    // If the loop is 0-initialized, then <out0> is the output.
    // and <out1> is the loopback.  Otherwise, it is reversed.
    int switch_output_op_idx() const {
      return this->switcherSense;
    }
    int switch_loopback_op_idx() const {
      return 1 - this->switcherSense;
    }
  };


  // Data structure storing a particular sequence candidate.
  struct CSASeqCandidate {

    // Enumerate the possible kinds of sequence operations that we
    // currently recognize.
    enum SeqType {
      UNKNOWN = 0,
      REPEAT = 1,
      REDUCTION = 2,
      STRIDE = 3,
      PARLOOP_MEM_DEP = 4,
      INVALID = 5,
    };

    MachineInstr* pickInst;
    MachineInstr* switchInst;

    // For now, we assume the transforming body is a single instruction.
    //
    // TBD(jsukha): In some cases, e.g., for PARLOOP_MEM_DEP, the body
    // could actually be many instructions, but we haven't found a
    // need to save the body yet.
    MachineInstr* transformInst;

    SeqType stype;
    bool    isParallel = false;  // True if this sequence can be pipelined

    // We will save away the channel info for faster processing later.
    unsigned top;
    unsigned bottom;
    MachineOperand* saved_op;


    // The opcode that we would use for this transformed sequence
    // instruction.
    unsigned opcode;
    bool negate_input;

    static const unsigned INVALID_OPCODE = CSA::PHI;

  CSASeqCandidate(MachineInstr* pickI,
                  MachineInstr* switchI)
      : pickInst(pickI)
      , switchInst(switchI)
      , transformInst(NULL)
      , stype(UNKNOWN)
      , top(0)
      , bottom(0)
      , saved_op(NULL)
      , opcode(CSASeqCandidate::INVALID_OPCODE)
      , negate_input(false)
    {
    }

    ~CSASeqCandidate() {
    }

    // Accessor functions for different operands from the pick/switch
    // instructions.
    inline MachineOperand*
    get_pick_top_op() const {
      int idx = CSASeqHeader::pick_top_op_idx();
      return &pickInst->getOperand(idx);
    }

    inline MachineOperand*
    get_pick_input_op(const CSASeqHeader& header) const {
      int idx = header.pick_input_op_idx();
      return &pickInst->getOperand(idx);
    }

    inline MachineOperand*
    get_switch_bottom_op() const {
      int idx = CSASeqHeader::switch_bottom_op_idx();
      return &switchInst->getOperand(idx);
    }

    inline MachineOperand*
    get_switch_output_op(const CSASeqHeader& header) const {
      int idx = header.switch_output_op_idx();
      return &switchInst->getOperand(idx);
    }

    inline MachineOperand*
    get_fma_mul_op(int input_idx) const {
      assert(this->transformInst);
      assert((input_idx >= 0) && (input_idx <= 1));
      return &this->transformInst->getOperand(1 + input_idx);
    }
  };


  // A struct that summarizes key information about a sequence
  // instruction.  This struct is mostly here for packaging purposes,
  // to avoid having too many arguments between functions.
  struct CSASeqInstrInfo {
    // The pointer to the sequence machine instruction we created.
    MachineInstr* seq_inst;

    // The channel numbers for the predicate, first, and last.
    // (Technically we could look these up from seq_inst, but it is
    // more convenient to just save them here).
    unsigned pred_reg;
    unsigned first_reg;
    unsigned last_reg;
  };


  struct CSASeqLoopInfo {
    int loop_id;
    CSASeqHeader header;
    SmallVector<CSASeqCandidate, 12> candidates;


    // A map from repeat channel register to the index in
    // "repeat_candidates" where we found the candidate.
    DenseMap<unsigned, int> repeat_channels;

  private:
    // Tracks the number of sequences in this loop which can be
    // converted.
    int num_valid_sequences;

    // The channel (register) numbers that correspond to the operands
    // in the compare instruction.  These values are "CSA::IGN" if the
    // operand is an immediate.
    unsigned cmp0_channel;
    unsigned cmp1_channel;

    // The index into the candidate array where matches to the uses in
    // the compare are located, if there are any such matches.
    //  -1 indicates no match.
    int cmp0_idx;
    int cmp1_idx;

    // Index into candidates array where we have induction variable.
    int indvar_idx;
    // Index into candidates array where we have bound value.
    int bound_idx;

    // Sense of the compare instruction:
    //  0 if the compare is
    //    compare indvar, bound
    //  1 if the compare is
    //    compare bound, indvar
    int compare_sense;

    // Final flag which indicates we have a safe transformation.
    bool valid_to_transform;

    // The final opcode we should use for this sequence, if we can
    // transform it.
    unsigned seq_opcode;


  public:
    CSASeqLoopInfo()
    : loop_id(-1)
    , num_valid_sequences(0)
    , cmp0_channel(CSA::IGN)
    , cmp1_channel(CSA::IGN)
    , cmp0_idx(-1)
    , cmp1_idx(-1)
    , indvar_idx(-1)
    , bound_idx(-1)
    , compare_sense(0)
    , valid_to_transform(false)
    , seq_opcode(CSASeqCandidate::INVALID_OPCODE)
    {
    }

    ~CSASeqLoopInfo() {}

    // Get and set the number of valid sequences in this loop.
    void set_valid_sequence_count(int val) {
      this->num_valid_sequences = val;
    }
    int get_valid_sequence_count() const {
      return this->num_valid_sequences;
    }

    // Accessor methods for some of the index fields in this data
    // structure.
    int cmp0Idx() const {
      return cmp0_idx;
    }
    int cmp1Idx() const {
      return cmp1_idx;
    }

    // Operand index in the compare instruction where we can find the
    // bound.
    inline int boundOpIdx() const {
      return 2 - this->compare_sense;
    }

    int indvarIdx() const {
      return indvar_idx;
    }
    int boundIdx() const {
      return bound_idx;
    }


    bool sequence_transform_is_valid() const {
      return valid_to_transform;
    }

    unsigned get_seq_opcode() const {
      return seq_opcode;
    }

    // Returns true if we need a sequence instruction whose comparison
    // is inverted from the actual compare instruction we find.
    //
    // There are two cases where we will need to swap comparison of the
    // sequence from the direction of the original compare.
    //
    // 1. If the comparison sense is inverted (i.e., we have compare
    //    stride, var), then we need to invert the comparison.
    //
    // 2. If the switcher sense is 1 (so it expects 0, 0, 0,
    // ... 1 for its control), then we need to invert the comparison.
    //
    // Both conditions together will cancel each other out.  Thus, we
    // xor the booleans together to figure out whether need to invert
    // the compare.
    //    bool invert_compare() const {
    //      return this->compare_sense ^ this->header.switcherSense;
    //    }

    // Returns true if we need a sequence instruction whose operand
    // order is swapped from the compare instruction that we find.
    //
    // If commute_compare() is false, we expect
    //   compare indvar, bound
    // else
    //   compare bound indvar
    //
    bool commute_compare_operands() const {
      return this->compare_sense;
    }

    // 2. If the switcher sense is 1 (so it expects 0, 0, 0, ... 1 for
    // its control), then we need to negate the output of the compare.
    bool negate_compare_output() const {
      return this->header.switcherSense;
    }

    // Helper method: looks up the right opcode for the sequence for
    // an induction variable, based on the opcode for the compare,
    // transforming statement, and whether we need to invert the
    // comparison.
    static
    bool
    compute_matching_seq_opcode(unsigned ciOp,
                                unsigned tOp,
                                bool commute_compare_operands,
                                bool negate_compare,
                                const CSAInstrInfo &TII,
                                unsigned* indvar_opcode) {

      // Transform the comparison opcode if needed.
      unsigned compareOp = ciOp;
      compareOp = TII.commuteNegateCompareOpcode(compareOp,
                                                 commute_compare_operands,
                                                 negate_compare);

      // Find a sequence opcode that matches our compare opcode.
      unsigned seqOp = TII.convertCompareOpToSeqOTOp(compareOp);
      if (seqOp != compareOp) {

        // If we have a matching sequence op, then check that the
        // transforming op matches as well.

        switch(tOp) {
        case CSA::ADD8:
          *indvar_opcode = TII.promoteSeqOTOpBitwidth(seqOp, 8);
          return true;
        case CSA::ADD16:
          *indvar_opcode = TII.promoteSeqOTOpBitwidth(seqOp, 16);
          return true;
        case CSA::ADD32:
          *indvar_opcode = TII.promoteSeqOTOpBitwidth(seqOp, 32);
          return true;
        case CSA::ADD64:
          *indvar_opcode = TII.promoteSeqOTOpBitwidth(seqOp, 64);
          return true;
        }
      }
      return false;
    }




    /*******************************************************************/
    // These methods below should only be called after we have
    // identified a potential header for the loop.
    //
    // But it is safe to call these methods as part of the
    // classification process.


    // Do any initialization of the loop information, once we know we
    // have a valid loop header.   Currently, this method:
    //
    //   1. Initializes repeat channels to empty.
    //   2. Figures out and saves the registers each operand for the
    //      compare instruction if they exist.
    //
    void init_from_header() {
      // Just for paranoia.
      this->repeat_channels.clear();

      // Look up the registers in the compare instruction, if there
      // are any.
      assert(header.compareInst);
      MachineOperand& cmpuse0 = header.compareInst->getOperand(1);
      MachineOperand& cmpuse1 = header.compareInst->getOperand(2);

      if (cmpuse0.isReg()) {
        this->cmp0_channel = cmpuse0.getReg();
      }
      if (cmpuse1.isReg()) {
        this->cmp1_channel = cmpuse1.getReg();
      }
    }

    enum CmpMatchType {
      NoMatch = -1,     // No match
      Match0 = 0,       // Matches input 0 of the compare.
      Match1 = 1,       // Matches input 1 of the compare
      Dup0 = 2,         // Matches input 0, but we already found a match.
      Dup1 = 3,         // Matches input 1, but we already found a match.
      OtherError = 4,   // Some other error.
    };

    // This method attempts to match the "bottom" register with one of
    // the compare inputs, returning one of the codes defined in
    // CmpMatchType.
    //
    // If an input matches, we also save the specified candidate_idx.
    CmpMatchType match_candidate_with_cmp(unsigned bottom,
                                          int candidate_idx) {
      // Errors conditions.
      if ((bottom == CSA::IGN) || (candidate_idx < 0)) {
        return CmpMatchType::OtherError;
      }

      // Try to match with input 0 of the compare.
      if (bottom == this->cmp0_channel) {
        if (this->cmp0_idx >= 0) {
          return CmpMatchType::Dup0;
        }
        else {
          this->cmp0_idx = candidate_idx;
          return CmpMatchType::Match0;
        }
      }

      // Try to match with input 1 of the compare.
      if (bottom == this->cmp1_channel) {
        if (this->cmp1_idx >= 0) {
          return CmpMatchType::Dup1;
        }
        else {
          this->cmp1_idx = candidate_idx;
          return CmpMatchType::Match1;
        }
      }

      // Otherwise, no match.
      return CmpMatchType::NoMatch;
    }


    /*******************************************************************/
    // These methods below should only be called after we have
    // classified the sequence candidates (into REPEAT, STRIDE, etc.)

    // Tries to find the induction variable.  Returns true if found,
    // false otherwise.
    //
    // This method assumes the sequence candidates stored in the
    // "candidates" array have already been classified.
    //
    // This method will initialize indvar_idx, bound_idx, and
    // compare_sense.
    bool find_induction_variable() {
      // Two cases.  Look for the stride (induction variable) in
      // either cmp0 or cmp1.
      if ((this->cmp0Idx() >= 0) &&
          this->candidates[this->cmp0Idx()].stype == CSASeqCandidate::SeqType::STRIDE) {
        this->indvar_idx = this->cmp0Idx();
        this->bound_idx = this->cmp1Idx();
        this->compare_sense = 0;
        return true;
      }
      else if ((this->cmp1Idx() >= 0) &&
               this->candidates[this->cmp1Idx()].stype == CSASeqCandidate::SeqType::STRIDE) {
        this->indvar_idx = this->cmp1Idx();
        this->bound_idx = this->cmp0Idx();
        this->compare_sense = 1;
        return true;
      }
      return false;
    }

    // Returns true if this loop has a valid bound.
    bool has_valid_bound() const {
      // This can either be a repeated channel, or an immediate.
      bool found_bound_channel =
        ((this->bound_idx >= 0) &&
         this->candidates[this->bound_idx].stype == CSASeqCandidate::SeqType::REPEAT);

      int boundOp_idx = this->boundOpIdx();
      bool found_bound_imm =
        ((this->bound_idx == -1) &&
         this->header.compareInst->getOperand(boundOp_idx).isImm());
      return found_bound_channel || found_bound_imm;
    }

    // Look up the machine operand that we should use for the initial
    // value of a bound.
    MachineOperand* get_input_bound_op() const {
      // Find a matching operand for <in_b>.  It either comes from a
      // corresponding repeat input, or the bound argument of the
      // compare.  Note that we can't always pick it straight from the
      // compare unless it is an immediate, because that channel has a
      // value for every loop iteration.
      MachineOperand* in_b_op;
      if (this->bound_idx >= 0) {
        // If we have a bound_idx, the bound corresponds to a repeat
        // statement.
        const CSASeqCandidate* bound_repeat = &candidates[this->bound_idx];
        assert(bound_repeat);
        assert(bound_repeat->stype == CSASeqCandidate::SeqType::REPEAT);
        in_b_op = bound_repeat->get_pick_input_op(header);
      }
      else {
        // Otherwise, the bound should be a literal.
        in_b_op = &header.compareInst->getOperand(this->boundOpIdx());
        assert(in_b_op->isImm());
      }
      return in_b_op;
    }


    /*******************************************************************/
    // These methods below should only be called after we have found a
    // valid induction variable candidate for this loop.

    // Returns true we can do the sequence transform.
    // If yes, then store the opcode that we need for the sequence instruction.
    bool sequence_opcode_transform_check(const CSAInstrInfo &TII) {
      if (this->indvar_idx >= 0) {
        CSASeqCandidate& indvarCandidate = this->candidates[this->indvar_idx];
        unsigned compare_opcode = this->header.compareInst->getOpcode();
        unsigned transform_opcode = indvarCandidate.transformInst->getOpcode();

        this->valid_to_transform =
          CSASeqLoopInfo::compute_matching_seq_opcode(compare_opcode,
                                                      transform_opcode,
                                                      this->commute_compare_operands(),
                                                      this->negate_compare_output(),
                                                      TII,
                                                      &this->seq_opcode);
        return this->valid_to_transform;
      }

      this->valid_to_transform = false;
      return false;
    }

  };

} // namespace llvm


#endif // LLVM_LIB_TARGET_CSA_CSASEQUENCEOPT_H

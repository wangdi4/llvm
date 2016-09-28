//===-- LPUSequenceOpt.h - LPU structures and methods for sequence opt.----===//
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

#ifndef LLVM_LIB_TARGET_LPU_LPUSEQUENCEOPT_H
#define LLVM_LIB_TARGET_LPU_LPUSEQUENCEOPT_H


#include "LPUTargetMachine.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Target/TargetRegisterInfo.h"

/**
 * Sequence optimization.
 *
 * 
 * Every candidate for sequence optimization has a control header
 * which controls the execution of the loop.
 *
 * The *sequence header* has the following form (in LPU machine
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
  struct LPUSeqHeader {
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

    LPUSeqHeader()
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
  struct LPUSeqCandidate {

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

    // The transforming body is either a single instruction, or a list
    // of instructions.
    //
    // (For now, the list is only used for PARLOOP_MEM_DEP.
    //  All the other cases expect 0 or 1 instructions). 
    MachineInstr* transformInst;
    SmallVectorImpl< MachineInstr* >* transformBody;
    SeqType stype;

    // We will save away the channel info for faster processing later.
    unsigned top;
    unsigned bottom;
    MachineOperand* stride_op;

  LPUSeqCandidate(MachineInstr* pickI,
                  MachineInstr* switchI)
      : pickInst(pickI)
      , switchInst(switchI)
      , transformInst(NULL)
      , transformBody(NULL)
      , stype(UNKNOWN)
      , top(0)
      , bottom(0)
      , stride_op(NULL)
    {
    }

    ~LPUSeqCandidate() {
      if (transformBody) {
        delete transformBody;
      }
    }

    // Accessor functions for different operands from the pick/switch
    // instructions.
    inline MachineOperand*
    get_pick_top_op() const {
      int idx = LPUSeqHeader::pick_top_op_idx();
      return &pickInst->getOperand(idx);
    }

    inline MachineOperand*
    get_pick_input_op(const LPUSeqHeader& header) const {
      int idx = header.pick_input_op_idx();
      return &pickInst->getOperand(idx);
    }

    inline MachineOperand*
    get_switch_bottom_op() const {
      int idx = LPUSeqHeader::switch_bottom_op_idx();
      return &switchInst->getOperand(idx);
    }

    inline MachineOperand*
    get_switch_output_op(const LPUSeqHeader& header) const {
      int idx = header.switch_output_op_idx();      
      return &switchInst->getOperand(idx);
    }


    
  };


  struct LPUSeqLoopInfo {
    LPUSeqHeader header;
    SmallVector<LPUSeqCandidate, 12> candidates;

    // A map from repeat channel register to the index in
    // "repeat_candidates" where we found the candidate.
    DenseMap<unsigned, int> repeat_channels;

    // The index into the candidate array where matches to the uses in
    // the compare are located, if there are any such matches.
    //  -1 indicates no match. 
    int cmp0_idx;
    int cmp1_idx;

    LPUSeqLoopInfo()
    : cmp0_idx(-1)
    , cmp1_idx(-1)
    {
    }

    ~LPUSeqLoopInfo() {}
  };

} // namespace llvm


#endif // LLVM_LIB_TARGET_LPU_LPUSEQUENCEOPT_H

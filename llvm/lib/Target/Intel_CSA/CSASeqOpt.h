//==--- CSASeqOpt.h - Sequence operator optimization --==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
#include "CSALoopInfo.h"
#include "CSAMachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineOptimizationRemarkEmitter.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

namespace llvm {
class CSALoopInfo;
class MachineInstr;
class MachineFunction;
class CSAInstrInfo;
class MachineRegisterInfo;
class CSASeqOpt {
public:
  /// Analyze a pick and switch instruction to see if they form a header and
  /// exit combo. Return the register that indicates the backedge, as well as
  /// the value for the pick/switch control instruction that reads from the
  /// outside of the loop instead of the backedge.
  bool analyzePickSwitchPair(MachineInstr *pickInstr,
                             MachineInstr *switchInstr,
                             unsigned &backedgeReg,
                             unsigned &loopSense,
                             MachineInstr *cmpInstr = nullptr);
  bool isIntegerOpcode(unsigned opcode);
  MachineInstr *repeatOpndInSameLoop(MachineOperand &opnd, MachineInstr *lpCmp);
  MachineInstr *getSeqOTDef(MachineOperand &opnd, bool *isNegated = nullptr);
  void SequenceOPT(bool);
  void SequenceIndv(MachineInstr *cmpInst, MachineInstr *switchInst,
                    MachineInstr *addInst, MachineInstr *lhdrPhiInst);
  MachineOperand CalculateTripCnt(MachineOperand &initOpnd,
                                  MachineOperand &bndOpnd, MachineInstr *pos);
  MachineOperand tripCntForSeq(MachineInstr *seqInstr, MachineInstr *pos);
  MachineOperand getTripCntForSeq(MachineInstr *seqInstr, MachineInstr *pos);
  void MultiSequence(MachineInstr *switchInst, MachineInstr *addInst,
                     MachineInstr *lhdrPickInst);
  void SequenceApp(MachineInstr *switchInst, MachineInstr *addInst,
                   MachineInstr *lhdrPhiInst);
  void SequenceSwitchOut(MachineInstr *switchInst, MachineInstr *addInst,
                         MachineInstr *lhdrPickInst, MachineInstr *seqIndv,
                         unsigned seqReg, unsigned backedgeReg);

  CSASeqOpt(MachineFunction *F, MachineOptimizationRemarkEmitter &ORE,
            CSALoopInfoPass &LI, const char *PassName);
  /// Set lic depth for a lic out of a sequence instr
  /// \parameter lic - channel that need new depth
  /// \parameter depth - new depth for the channel
  void SetSeqLicDepth(unsigned lic, unsigned depth);
  /// Given a sequence instr corresponding to an INDV, find the ideal
  /// lic depth for its output
  /// \parameter seqIndv - the sequence induction variable
  unsigned GetSeqIndvLicDepth(MachineInstr *seqIndv);
  /// find the const src def if it happens to be a constant
  /// supplied through repeats/filters/movs
  /// \parameter opnd - the src operand started from
  MachineOperand* GetConstSrc(MachineOperand &opnd);

private:
  MachineFunction *thisMF;

  /// \brief Optimization remark emitter for users of CSASeqOpt.
  MachineOptimizationRemarkEmitter &ORE;

  CSALoopInfoPass &LI;

  /// \brief Pass name to be used for emitting optimization remarks
  /// on behalf of the passes using CSASeqOpt.
  const char *PassName;
  const CSAInstrInfo *TII;
  MachineRegisterInfo *MRI;
  CSAMachineFunctionInfo *LMFI;
  const TargetRegisterInfo *TRI;
  DenseMap<MachineInstr *, MachineOperand *> seq2tripcnt;
  DenseMap<unsigned, unsigned> reg2neg;

  /// Generate repeats, strides, and sequences for a specific loop.
  void optimizeDFLoop(const CSALoopInfo &Loop);

  /// Form PICK/SWITCH operands into repeat operands when necessary.
  MachineInstr *FormRepeat(MachineInstr *PickInst, unsigned PickBackedgeIdx,
                           MachineInstr *SwitchInst, unsigned LoopPredicate);

  /// Turn an affine variable inside the loop into a STRIDE operator.
  MachineInstr *CreateStride(MachineInstr *PickInst, unsigned PickBackedgeIdx,
    MachineInstr *SwitchInst, unsigned LoopPredicate,
    MachineInstr *AddInst);

  /// Convert a STRIDE operator into a SEQOT operator.
  MachineInstr *StrideToSeq(MachineInstr *cmpInst, MachineInstr *switchInst,
                            MachineInstr *addInst, MachineInstr *strideInst,
                            unsigned SwitchBackedgeIdx, unsigned LoopPredicate);

  /// Turn a reduction pattern into a RED operator.
  MachineInstr *CreateReduc(MachineInstr *PickInst, unsigned PickBackedgeIdx,
                            MachineInstr *SwitchInst, unsigned LoopPredicate,
                            MachineInstr *ReducModInst);

  MachineOperand *getInvariantOperand(MachineOperand &Op);

  unsigned negateRegister(unsigned Register);

  /// This is a mapping of the loop control edges to LIC predicate groups.
  /// Don't use this mapping directly, go through getLoopPredicate instead.
  DenseMap<unsigned, std::shared_ptr<CSALicGroup>> loopPredicateGroups;

  /// Get a LIC group that is appropriate for the predicate output of a
  /// sequence optimization.
  std::shared_ptr<CSALicGroup> getLoopPredicate(MachineInstr *lhdrPhi);

  /// Collect a list of REPEAT and STRIDE operations that were created by this
  /// pass.
  std::vector<MachineInstr *> NewDrivenOps;

  /// Add backedge annotations to the control inputs of REPEAT/STRIDE operations
  /// created by this pass.
  void annotateBackedges();

  /// Apply loop carry collapse to the loops. Please refer to
  /// https://jira.devtools.intel.com/browse/CMPLRLLVM-8598
  /// for details.
  void doLoopCarryCollapse();

  // The value of this field is assigned to a newly created, collapsed, loop.
  unsigned NextLoopId;

  // This class handles the loop carry collapse optimization.
  class LccCanvas {
  public:
    LccCanvas(CSASeqOpt *Parent, CSALoopInfoPass::riterator &Loop,
              std::vector<CSALoopInfo> &NewLoops)
      : Parent(Parent), TII(Parent->TII), MRI(Parent->MRI),
        LMFI(Parent->LMFI), LI(Parent->LI),
        OutermostLoop(Loop), NewLoops(NewLoops) {}

    void startLCC();

  private:
    CSASeqOpt *Parent;
    const CSAInstrInfo *TII;
    MachineRegisterInfo *MRI;
    CSAMachineFunctionInfo *LMFI;
    CSALoopInfoPass &LI;

    // This is the loop that starts this round of loop carry collapse.
    const CSALoopInfoPass::riterator &OutermostLoop;

    std::vector<CSALoopInfo> &NewLoops;

    // Merge2Op is a section where a merge instruction's output
    // is one of the inputs to an eligible operation.
    // Op2Merge is a section where an eligible operation's output
    // is one of the inputs to a merge instruction.
    // Merge is a section where the merge instruction's other input is not
    // dependent on the loop-carried value.
    enum SectionType { If, Loop, MergeOp, OpMerge, Merge, Logical };

    class Section {
    public:
      SectionType Type;
      // Canonical is true for SectionType
      //   'If' when Operand 0 of the switch instructionn is the fallthrough.
      //   'Loop' when the backedge index is 1.
      //   'MergeOp' when the identity of the op is operand 2.
      //   'OpMerge' when the output the op feeds into operand 2 of merge.
      //   'Merge' when non-dependent op is the second merge input.
      //   'Logical' when the op is an or1.
      // Canonical is false otherwise.
      bool Canonical;
      // HeadInstr is a switch instruction for 'If'
      //                pick               for 'Loop'
      //                merge              for 'MergeOp', 'OpMerge', and 'Merge'
      //                or1/and1           for 'Logical'
      MachineInstr *HeadInstr;
      // TailInstr is a pick   instruction for 'If'
      //                switch             for 'Loop'
      //                op                 for 'MergeOp', 'OpMerge', and 'Merge'
      //                or1/and1           for 'Logical'
      MachineInstr *TailInstr;
      // This is the predicate that drives the original if/loop/merge.
      unsigned Predicate;
      // This is the LIC that goes into the operator from a switch.
      // Only effective for 'MergeOp' and 'OpMerge'.
      unsigned SwitchToOp;
      // This is the operand index of the LIC that goes from the
      // head instruction of the outer construct to the operator.
      // Only effective for 'MergeOp' and 'OpMerge'.
      unsigned OpIdx;

      Section(SectionType Type, bool Canonical, MachineInstr *HeadInstr,
              MachineInstr *TailInstr, unsigned Predicate,
              unsigned SwitchToOp = 0, unsigned OpIdx = 0)
        : Type(Type), Canonical(Canonical), HeadInstr(HeadInstr),
          TailInstr(TailInstr), Predicate(Predicate),
          SwitchToOp(SwitchToOp), OpIdx(OpIdx) {}
    };

    enum Operation {
      TransformOutermostIfLoop,
      TransformOutermostLoop,
      TransformInnerIf,
      TransformInnerLoop
    };

    class Region {
    public:
      Region(std::vector<Section> Sections,
             std::vector<Operation> Operations,
             unsigned W, unsigned X,
             unsigned Y, unsigned Z )
        : Sections(Sections), Operations(Operations),
          W(W), X(X), Y(Y), Z(Z) {}
      Region() : W(0), X(0), Y(0), Z(0) {};
      std::vector<Section> Sections;
      std::vector<Operation> Operations;
      // Please refer to the graphs in the slide deck
      // These are the input and output LICs of the loops and ifs that
      // remain after loop carry collapse. The namings here comply with
      // those used in the graphs in the slide deck
      // https://sharepoint.amr.ith.intel.com/sites/KNPPath/SPATIAL_ACCELERATORS_WG_DOCLIB/CSA%20Compiler%20Transforms/Loop%20Carry%20Collapse.pptx
      unsigned W, X, Y, Z;
    };

    Region CurrentRegion;

    std::vector<Region> Regions;

    bool isLoopLccEligible(const CSALoopInfo& Loop);
    bool isPairLccEligible(MachineInstr *Pick, MachineInstr *Switch,
                           unsigned BackedgeIndex);

    // Collects the loop consists of LoopHead and LoopTail and
    // an enclosing if for collapse.
    // Returns true if successful, false otherwise.
    bool addOutermostIfLoop(MachineInstr *LoopHead, MachineInstr *Looptail,
                            unsigned BackedgeIndex);

    // Collects the loop consists of LoopHead and LoopTail for collapse.
    // Returns true if successful, false otherwise.
    bool addOutermostLoop(MachineInstr *LoopHead, MachineInstr *Looptail,
                          unsigned BackedgeIndex);

    // Collects an if immediately enclosed by the innermost section
    // collected so far.
    // Returns true if successful, false otherwise.
    bool addInnerIf();

    // Collects Loop if it is immediately enclosed by the innermost section
    // collected so far.
    // Returns true if successful, false otherwise.
    bool addInnerLoop(const CSALoopInfo& Loop);

    // Returns true if Operation is an operation with identity;
    // false otherhwise.
    bool isOperationWithIdentity(MachineInstr *Operation);

    // Returns true if Operation is stateless (i.e. has no side-effect);
    // false otherhwise.
    bool isOperationStateless(MachineInstr *Operation);

    // Returns true if Merge has the identity of Operation as one of its
    // inputs; false otherwise.
    // Output parameter IdIdx is the index to the identity input if
    // the function returns true.
    bool getIdIdx(MachineInstr *Merge, MachineInstr *Operation,
                  unsigned& IdIdx);

    // Returns true if the two regions share the same predicate;
    // false otherwise.
    bool sharingPredicate(Region &Region1, Region &Region2);

    // Fixes up certain types of sections involving merges to complete merge-if
    // conversion and other related transforms.
    void fixupInnerSections(Region &);
    void fixupInnerMergeOp(Section &);
    void fixupInnerOpMerge(Section &);
    void fixupInnerMerge(Section &);
    void fixupInnerLogical(Section &);

    MachineInstr *generatePredicate(Region &Region);

    // The following transform functions emit the code for
    // generating the predicate stream for the given sections.
    MachineInstr *transformOutermostIfLoop(Section &IfSection, Section &LoopSection);
    MachineInstr *transformOutermostLoop(Section &Section);
    MachineInstr *transformInnerIf(MachineInstr *Instr, Section &Section);
    MachineInstr *transformInnerLoop(MachineInstr *Instr, Section &Section);
    MachineInstr *transformLoopPredicate(MachineInstr *LoopTail, bool Canonical);

    // The pick and switch instructions created for the new
    // loop are in the output parameters.
    void reExpandIfLoop(MachineInstr *Instr, Region &Region,
                        MachineInstr *&Pick, MachineInstr *&Switch);

    auto assignLicGroup(unsigned Reg, ScaledNumber<uint64_t> Frequency,
                        unsigned LoopId = 0);

    void finalizeCurrentRegion();
    void finalize();
  };
};
} // namespace llvm

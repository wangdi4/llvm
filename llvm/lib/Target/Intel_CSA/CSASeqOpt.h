//==--- CSASeqOpt.h - Sequence operator optimization --==//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
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
};
} // namespace llvm

//==--- CSASeqOpt.h - Sequence operator optimization --==//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
#include "CSAMachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineOptimizationRemarkEmitter.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

namespace llvm {
class CSASSANode;
class MachineInstr;
class MachineFunction;
class CSAInstrInfo;
class MachineRegisterInfo;
class CSASeqOpt {
public:
  MachineInstr *lpInitForPickSwitchPair(MachineInstr *pickInstr,
                                        MachineInstr *switchInstr,
                                        unsigned &backedgeReg,
                                        MachineInstr *cmpInstr = nullptr);
  bool isIntegerOpcode(unsigned opcode);
  MachineInstr *repeatOpndInSameLoop(MachineOperand &opnd, MachineInstr *lpCmp);
  void FoldRptInit(MachineInstr *rptInstr);
  MachineInstr *getSeqOTDef(MachineOperand &opnd);
  void PrepRepeat();
  void SequenceOPT(bool);
  void SequenceIndv(CSASSANode *cmpNode, CSASSANode *switchNode,
                    CSASSANode *addNode, CSASSANode *lhdrPhiNode);
  MachineOperand CalculateTripCnt(MachineOperand &initOpnd,
                                  MachineOperand &bndOpnd, MachineInstr *pos);
  MachineOperand tripCntForSeq(MachineInstr *seqInstr, MachineInstr *pos);
  MachineOperand getTripCntForSeq(MachineInstr *seqInstr, MachineInstr *pos);
  void MultiSequence(CSASSANode *switchNode, CSASSANode *addNode,
                     CSASSANode *lhdrPickNode);
  void SequenceApp(CSASSANode *switchNode, CSASSANode *addNode,
                   CSASSANode *lhdrPhiNode);
  void SequenceReduction(CSASSANode *switchNode, CSASSANode *addNode,
                         CSASSANode *lhdrPhiNode);
  void SequenceSwitchOut(CSASSANode *switchNode, CSASSANode *addNode,
                         CSASSANode *lhdrPickNode, MachineInstr *seqIndv,
                         unsigned seqReg, unsigned backedgeReg);
  void SequenceRepeat(CSASSANode *switchNode, CSASSANode *lhdrPhiNode);

  CSASeqOpt(MachineFunction *F, MachineOptimizationRemarkEmitter &ORE,
            const char *PassName);
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

  /// \brief Pass name to be used for emitting optimization remarks
  /// on behalf of the passes using CSASeqOpt.
  const char *PassName;
  const CSAInstrInfo *TII;
  MachineRegisterInfo *MRI;
  CSAMachineFunctionInfo *LMFI;
  const TargetRegisterInfo *TRI;
  DenseMap<MachineInstr *, MachineOperand *> seq2tripcnt;
  DenseMap<unsigned, unsigned> reg2neg;

  /// This is a mapping of the loop control edges to LIC predicate groups.
  /// Don't use this mapping directly, go through getLoopPredicate instead.
  DenseMap<unsigned, std::shared_ptr<CSALicGroup>> loopPredicateGroups;

  /// Get a LIC group that is appropriate for the predicate output of a
  /// sequence optimization.
  std::shared_ptr<CSALicGroup> getLoopPredicate(CSASSANode *lhdrPhiNode);

  /// Collect a list of REPEAT and STRIDE operations that were created by this
  /// pass.
  std::vector<MachineInstr *> NewDrivenOps;

  /// Add backedge annotations to the control inputs of REPEAT/STRIDE operations
  /// created by this pass.
  void annotateBackedges();
};
} // namespace llvm

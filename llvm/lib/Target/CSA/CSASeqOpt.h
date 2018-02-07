//==--- CSASeqOpt.h - Sequence operator optimization --==//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
#include "CSAMachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
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
  CSASeqOpt(MachineFunction *F);

private:
  MachineFunction *thisMF;
  const CSAInstrInfo *TII;
  MachineRegisterInfo *MRI;
  CSAMachineFunctionInfo *LMFI;
  const TargetRegisterInfo *TRI;
  DenseMap<MachineInstr *, MachineOperand *> seq2tripcnt;
  DenseMap<unsigned, unsigned> reg2neg;
};
} // namespace llvm

namespace llvm 
{
  class CSASSANode;
  class MachineInstr;
  class MachineFunction;
  class CSAInstrInfo;
  class MachineRegisterInfo;
  class CSASeqOpt 
  {
    public:
    MachineInstr* lpInitForPickSwitchPair(MachineInstr* pickInstr, MachineInstr* switchInstr, unsigned& backedgeReg, MachineInstr* cmpInstr=nullptr);
    bool isIntegerOpcode(unsigned opcode);
    MachineInstr* repeatOpndInSameLoop(MachineOperand& opnd, MachineInstr* lpCmp);
    void FoldRptInit(MachineInstr* rptInstr);
    void PrepRepeat();
    void SequenceOPT();
    void SequenceIndv(CSASSANode* cmpNode, CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPhiNode); 
    MachineOperand CalculateTripCnt(MachineOperand& initOpnd, MachineOperand& bndOpnd);
    MachineOperand tripCntForSeq(MachineInstr*seqInstr);
    void MultiSequence(CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPickNode);
    void SequenceApp(CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPhiNode);
    void SequenceReduction(CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPhiNode);
    void SequenceSwitchOutLast(CSASSANode* cmpNode, MachineInstr* initInstr, MachineInstr* seqIndv);
    void SequenceSwitchOutLast(MachineInstr* switchInstr, MachineInstr* seqIndv);
    void SequenceSwitchOut(CSASSANode* switchNode, 
                           CSASSANode* addNode, 
                           CSASSANode* lhdrPickNode, 
                           MachineInstr* seqIndv,
                           unsigned seqReg,
                           unsigned backedgeReg);
    void SequenceFlipSwitchDsts(CSASSANode* cmpNode);
    void SequenceRepeat(CSASSANode* switchNode, CSASSANode* lhdrPhiNode);
    CSASeqOpt(MachineFunction *F);
    
    private:
    MachineFunction *thisMF;
    const CSAInstrInfo* TII;
    MachineRegisterInfo* MRI;
    CSAMachineFunctionInfo *LMFI;
    const TargetRegisterInfo* TRI;
  };
}

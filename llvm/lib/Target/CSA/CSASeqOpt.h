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
    bool repeatOpndInSameLoop(MachineOperand& opnd, MachineInstr* lpCmp);
    void SequenceOPT();
    void SequenceIndv(CSASSANode* cmpNode, CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPhiNode);
    MachineOperand CalculateTripCnt(MachineOperand& initOpnd, MachineOperand& bndOpnd);
    MachineOperand tripCntForSeq(MachineInstr*seqInstr);
    void MultiSequence(CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPickNode);
    void SequenceApp(CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPhiNode);
    void SequenceReduction(CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPhiNode);
    void SequenceRepeat(CSASSANode* switchNode, CSASSANode* lhdrPhiNode);
    CSASeqOpt(MachineFunction *F);
    
    private:
    MachineFunction *thisMF;
    const CSAInstrInfo* TII;
    MachineRegisterInfo* MRI;
    CSAMachineFunctionInfo *LMFI;
  };
}

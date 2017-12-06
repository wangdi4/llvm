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
    bool isIntegerReg(unsigned Reg);
    void SequenceOPT();
    void SequenceIndv(CSASSANode* cmpNode, CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPhiNode);
    void SequenceAddress(CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPhiNode);
    void SequenceRepeat(CSASSANode* switchNode, CSASSANode* lhdrPhiNode);
    CSASeqOpt(MachineFunction *F);
    
    private:
    MachineFunction *thisMF;
    const CSAInstrInfo* TII;
    MachineRegisterInfo* MRI;
    CSAMachineFunctionInfo *LMFI;
  };
}

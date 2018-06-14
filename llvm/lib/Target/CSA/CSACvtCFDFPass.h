//===-- CSACvtCFDFPass.h - CSA convert control flow to data flow ----------===//
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
// This file "reexpresses" the code containing traditional control flow
// into a basically data flow representation suitable for the CSA.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSAInstrInfo.h"
#include "CSAMachineFunctionInfo.h"
#include "MachineCDG.h"
#include "llvm/ADT/IntEqClasses.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/CodeGen/MachineBlockFrequencyInfo.h"
#include "llvm/CodeGen/MachineBranchProbabilityInfo.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

namespace llvm {
class CSACvtCFDFPass : public MachineFunctionPass {
public:
  struct CmpFcn {
    CmpFcn(const DenseMap<MachineBasicBlock *, unsigned> &m) : mbb2rpo(m){};
    DenseMap<MachineBasicBlock *, unsigned> mbb2rpo;
    bool operator()(MachineBasicBlock *A, MachineBasicBlock *B) {
      return mbb2rpo[A] < mbb2rpo[B];
    }
  };
  static char ID;
  CSACvtCFDFPass();

  StringRef getPassName() const override {
    return "CSA: Convert Control Flow to Data Flow";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
  MachineInstr *insertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgpBB);
  MachineInstr *getOrInsertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgBB);
  MachineInstr *insertPICKForReg(MachineBasicBlock *ctrlBB, unsigned Reg,
                                 MachineBasicBlock *inBB, MachineInstr *phi,
                                 unsigned pickReg = 0);
  void assignPICKSrcForReg(unsigned &pickFalseReg, unsigned &pickTrueReg,
                           unsigned Reg, MachineBasicBlock *ctrlBB,
                           MachineBasicBlock *inBB, MachineInstr *phi);
  // generate a PICK for SSA value dst at fork of ctrlBB with source input Reg
  // from inBB, and output in pickReg
  MachineInstr *PatchOrInsertPickAtFork(MachineBasicBlock *ctrlBB, unsigned dst,
                                        unsigned Reg, MachineBasicBlock *inBB,
                                        MachineInstr *phi,
                                        unsigned pickReg = 0);
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfo>();
    AU.addRequired<ControlDependenceGraph>();
    AU.addRequired<MachineBlockFrequencyInfo>();
    AU.addRequired<MachineBranchProbabilityInfo>();
    AU.addRequired<MachineDominatorTree>();
    AU.addRequired<MachinePostDominatorTree>();
    AU.addRequired<AAResultsWrapperPass>();
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
  void insertSWITCHForConstant(MachineInstr *MI, MachineBasicBlock *mbb);
  void insertSWITCHForOperand(MachineOperand &MO, MachineBasicBlock *mbb,
                              MachineInstr *phiIn = nullptr);
  void insertSWITCHForIf();
  void renameOnLoopEntry();
  void renameAcrossLoopForRepeat(MachineLoop *);
  void repeatOperandInLoop(MachineLoop *mloop, unsigned pickCtrlReg,
                           unsigned backedgePred, bool pickCtrlInverted,
                           SmallVector<MachineOperand *, 4> *in   = nullptr,
                           SmallVector<MachineOperand *, 4> *back = nullptr);
  void repeatOperandInLoopUsePred(MachineLoop *mloop, MachineInstr *initInst,
                                  unsigned backedgePred, unsigned exitPred);
  MachineBasicBlock *
  getDominatingExitingBB(SmallVectorImpl<MachineBasicBlock *> &exitingBlks,
                         MachineInstr *UseMI, unsigned Reg);
  void insertSWITCHForLoopExit();
  void insertSWITCHForLoopExit(
    MachineLoop *L,
    DenseMap<MachineBasicBlock *, std::set<unsigned> *> &LCSwitch);
  unsigned SwitchOutExitingBlk(MachineBasicBlock *exitingBlk, unsigned Reg,
                               MachineLoop *mloop);
  void SwitchDefAcrossExits(unsigned Reg, MachineBasicBlock *mbb,
                            MachineLoop *mloop, MachineOperand &UseMO);
  void SwitchDefAcrossLoops(unsigned Reg, MachineBasicBlock *mbb,
                            MachineLoop *mloop);
  void replacePhiWithPICK();
  void replaceLoopHdrPhi();
  void replaceLoopHdrPhi(MachineLoop *L);
  void replaceCanonicalLoopHdrPhi(MachineBasicBlock *lhdr);
  void replaceCanonicalLoopHdrPhiPipelined(MachineBasicBlock *mbb,
                                           unsigned numTokensSpecified = 1);
  MachineOperand *createUseTree(MachineBasicBlock *mbb,
                                MachineBasicBlock::iterator before,
                                unsigned opcode,
                                const SmallVector<MachineOperand *, 4> vals,
                                SmallVector<MachineInstr *, 4> *created = nullptr,
                                unsigned unusedReg = CSA::IGN);
  void generateCompletePickTreeForPhi(MachineBasicBlock *);
  void CombineDuplicatePickTreeInput();
  void PatchCFGLeaksFromPcikTree(unsigned phiDst);
  unsigned findLoopExitCondition(MachineLoop* mloop);
  unsigned generateLandSeq(SmallVectorImpl<unsigned> &landOpnds,
                           MachineBasicBlock *mbb, MachineInstr *MI = nullptr);
  unsigned generateOrSeq(SmallVectorImpl<unsigned> &orOpnds,
                         MachineBasicBlock *mbb, MachineInstr *ploc = nullptr);
  void generateDynamicPickTreeForFooter(MachineBasicBlock *);
  void generateDynamicPickTreeForHeader(MachineBasicBlock *);
  bool parentsLinearInCDG(MachineBasicBlock *mbb);
  bool needDynamicPreds();
  bool needDynamicPreds(MachineLoop *L);
  unsigned getInnerLoopPipeliningDegree(MachineLoop *L);
  void generateDynamicPreds();
  void generateDynamicPreds(MachineLoop *L);
  unsigned getEdgePred(MachineBasicBlock *fromBB,
                       ControlDependenceNode::EdgeType childType);
  void setEdgePred(MachineBasicBlock *fromBB,
                   ControlDependenceNode::EdgeType childType, unsigned ch);
  unsigned getBBPred(MachineBasicBlock *inBB);
  void setBBPred(MachineBasicBlock *inBB, unsigned ch);
  MachineInstr *getOrInsertPredMerge(MachineBasicBlock *mbb, MachineInstr *loc,
                                     unsigned e1, unsigned e2);
  MachineInstr *InsertPredProp(MachineBasicBlock *mbb, unsigned bbPred = 0);
  unsigned computeEdgePred(MachineBasicBlock *fromBB, MachineBasicBlock *toBB,
                           std::list<MachineBasicBlock *> &path);
  unsigned mergeIncomingEdgePreds(MachineBasicBlock *inBB,
                                  std::list<MachineBasicBlock *> &path);
  unsigned computeBBPred(MachineBasicBlock *inBB,
                         std::list<MachineBasicBlock *> &path);
  void TraceCtrl(MachineBasicBlock *inBB, MachineBasicBlock *mbb, unsigned Reg,
                 unsigned dst, unsigned src, MachineInstr *MI);
  void TraceThroughPhi(MachineInstr *iphi, MachineBasicBlock *mbb,
                       unsigned dst);
  void TraceLeak(MachineBasicBlock *ctrlBB, MachineBasicBlock *mbb,
                 SmallVectorImpl<unsigned> &landOpnds);
  void CombineDuplicatePhiInputs(
    SmallVectorImpl<std::pair<unsigned, unsigned> *> &pred2values,
    MachineInstr *iPhi);
  void LowerXPhi(SmallVectorImpl<std::pair<unsigned, unsigned> *> &pred2values,
                 MachineInstr *MI);
  bool CheckPhiInputBB(MachineBasicBlock *inBB, MachineBasicBlock *mbb);
  void replaceIfFooterPhiSeq();
  void assignLicForDF();
  void createFIEntryDefs();
  void removeBranch();
  void linearizeCFG();
  unsigned findSwitchingDstForReg(unsigned Reg, MachineBasicBlock *mbb);
  void handleAllConstantInputs();
  bool hasAllConstantInputs(MachineInstr *);
  void releaseMemory() override;
  bool replaceUndefWithIgn();

private:
  MachineFunction *thisMF;
  const CSAInstrInfo *TII;
  MachineRegisterInfo *MRI;
  const TargetRegisterInfo *TRI;
  CSAMachineFunctionInfo *LMFI;
  MachineDominatorTree *DT;
  MachinePostDominatorTree *PDT;
  ControlDependenceGraph *CDG;
  MachineLoopInfo *MLI;
  AliasAnalysis *AA;
  AliasSetTracker *AS;
  MachineBasicBlock *entryBB;
  DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *>
    bb2switch; // switch for Reg added in bb
  DenseMap<MachineBasicBlock *, unsigned> bb2predcpy;
  DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *>
    bb2pick; // switch for Reg added in bb
  DenseMap<MachineBasicBlock *, SmallVectorImpl<unsigned> *> edgepreds;
  DenseMap<MachineBasicBlock *, unsigned> bbpreds;
  DenseMap<MachineBasicBlock *, MachineInstr *> bb2predmerge;
  DenseMap<MachineBasicBlock *, unsigned> bb2rpo;
  DenseMap<MachineInstr *, MachineBasicBlock *> multiInputsPick;
  std::set<MachineBasicBlock *> dcgBBs;

  /// Assign a name to the LIC.
  /// The generated name will be a concatenation of the 5 pieces in order of
  /// the argument, although passing in 0 for baseReg or nullptr for the
  /// containingBlock will cause those to be treated as the empty string.
  void nameLIC(unsigned vreg, const Twine &prefix,
      unsigned baseReg = 0, const Twine &infix = "",
      const MachineBasicBlock *containingBlock = nullptr,
      const Twine &suffix = "");

  /// Propagate block frequency and branch probability information to generated
  /// LIC groups.
  void assignLicFrequencies(MachineBlockFrequencyInfo &MBFI);

  // The following data structures represent conceptual mappings of individual
  // LICs, basic blocks, and edges in the CFG all to equivalence groups. The
  // LLVM API that is used for equivalence groups does not permit mapping
  // different key kinds to the same equivalence group collection, so we instead
  // rely on an indirect mapping here. LICs are mapped to equivalence groups
  // according to the basic set, and we maintain a separate map of basic blocks
  // to a representative LIC, and another map of edges to a representative LIC.

  /// A pair of representative LIC numbers for the edge table mapping.
  typedef std::pair<unsigned, unsigned> EdgeRegs;

  /// A magic value that indicates that the basic block or edge does not have
  /// an assigned representative LIC.
  static const unsigned UNMAPPED_REG = ~0U;

  /// The mapping of LICs to equivalence classes. The integers that are used as
  /// keys in this array is the vreg index of the register (so only virtual
  /// registers can be assigned to equivalence classes).
  IntEqClasses licGrouping;

  /// The mapping of basic blocks to representative LICs. The basic block
  /// number is an index into this vector; the result is a virtual register
  /// index that can be used as an index for licGrouping.
  SmallVector<unsigned, 8> basicBlockRegs;

  /// The mapping of CFG edges to representative LICs. The basic block number
  /// of the source of the edge is used as index into this vector. If the edge
  /// is the first edge of a basic block, the first element in the pair is the
  /// virtual register index for licGrouping; if it is the second edge, then the
  /// second element is the virtual register index.
  SmallVector<EdgeRegs, 8> switchOuts;

  /// Attempt to find LIC groups for all LICs in the function. If
  /// buildBBMapping is true, attempt to propagate information about basic
  /// block mapping as well.
  void findLICGroups(bool buildBBMapping);
};
} // namespace llvm

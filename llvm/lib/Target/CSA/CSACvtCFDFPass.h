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
#include "CSALoopInfo.h"
#include "CSAMachineFunctionInfo.h"
#include "MachineCDG.h"
#include "llvm/ADT/IntEqClasses.h"
#include "llvm/ADT/PostOrderIterator.h"
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
  static char ID;
  CSACvtCFDFPass();

  StringRef getPassName() const override {
    return "CSA: Convert Control Flow to Data Flow";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
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
  void repeatOperandInLoop(MachineLoop *mloop, unsigned pickCtrlReg,
                           unsigned backedgePred, bool pickCtrlInverted);
  void repeatOperandInLoopUsePred(MachineLoop *mloop, MachineInstr *initInst,
                                  unsigned backedgePred, unsigned exitPred);
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
  void CombineDuplicatePhiInputs(
    SmallVectorImpl<std::pair<unsigned, unsigned> *> &pred2values,
    MachineInstr *iPhi);
  void LowerXPhi(SmallVectorImpl<std::pair<unsigned, unsigned> *> &pred2values,
                 MachineInstr *MI);
  bool CheckPhiInputBB(MachineBasicBlock *inBB, MachineBasicBlock *mbb);
  void assignLicForDF();
  void createFIEntryDefs();
  void removeBranch();
  void linearizeCFG();
  unsigned findSwitchingDstForReg(unsigned Reg, MachineBasicBlock *mbb);
  void handleAllConstantInputs();

  /// Return true if the instruction is a MOV instruction whose inputs are all
  /// constant.
  bool hasAllConstantInputs(MachineInstr *);
  void releaseMemory() override;
  bool replaceUndefWithIgn();

private:
  /// \defgroup Switch generation
  /// This set of functions implements the generation of switch instructions
  /// from branch instructions.
  /// @{

  /// Insert all of the necessary switches for a function.
  void switchNormalRegisters();

  /// Insert switches for all of the branches that the register is alive across.
  ///
  /// There are two modes to this function, which can be controlled by the
  /// StrictLive parameter. The default, when it is false, is to only consider
  /// control-dependent regions as the basis for liveness. This means that a
  /// variable does not need to be switched into an if-statement if it is not
  /// used on either leg, even if it is, strictly speaking, live across the if.
  /// The other mode uses the strict definition of liveness that considers only
  /// basic blocks, and would insert switch statements for that scenario.
  void switchRegister(unsigned Reg, bool StrictLive = false);

  /// Switch the register across the branch at the end of MBB, reusing a switch
  /// that was generated earlier if present.
  MachineInstr *getOrInsertSWITCHForReg(unsigned Reg, MachineBasicBlock *MBB);

  /// Insert, without trying to reuse other switches, a switch for a register
  /// across the branch at the end of MBB.
  MachineInstr *insertSWITCHForReg(unsigned Reg, MachineBasicBlock *MBB);

  /// Get the index of the output of a switch that would correspond to the edge
  /// from the parent to the child. The parent must have two successors (as
  /// otherwise, it would not have a switch).
  unsigned getSwitchIndexForEdge(MachineBasicBlock *Parent,
      MachineBasicBlock *Child);

  /// A map of basic block => (register => switch) instructions that were
  /// generated.
  DenseMap<MachineBasicBlock *,
    std::unique_ptr<DenseMap<unsigned, MachineInstr *>>> GenSwitches;
  /// @}

  /// \defgroup Structured phi generation
  /// This set of functions implements the generation of picks for structured
  /// code. The general overview of how this code works is documented elsewhere.
  ///
  /// These functions build up multiInputsPick for a single PHI statement at
  /// the end. This PHI is generally referred to as FinalPhi, its containing
  /// basic block FinalPhiMBB, and its output FinalDestReg when used as
  /// parameters in this name. When traversing PHIs, if one the parameters is
  /// itself a PHI, we trace them recursively. As parameters, this PHI is
  /// referred to as Phi, the operand of which we are looking at is called
  /// PhiSrcReg and its corresponding basic block the PhiSrcMBB.
  /// @{

  /// Replace phis with PICKs for all non-loop-related phi instructions.
  void replaceIfFooterPhiSeq();

  /// Converts all of the PHI instructions in the MachineBasicBlock into PICK
  /// instructions, for structured phi generation.
  void generateCompletePickTreeForPhi(MachineBasicBlock *);

  /// Trace the phi instruction to build the pick tree for the computation of
  /// FinalDestReg. The phi instruction is not necessarily the one generating
  /// the final register.
  void TraceThroughPhi(MachineInstr *Phi, MachineBasicBlock *FinalPhiMBB,
                       unsigned FinalDestReg);

  /// Recurse through the control-dependent predecessors of the source block
  /// until we get to the dominator of the final phi, and build an inverse tree
  /// of picks along the way.
  void TraceCtrl(MachineBasicBlock *SourceMBB,
                 MachineBasicBlock *FinalPhiMBB,
                 unsigned PickOperandReg,
                 unsigned FinalDestReg,
                 unsigned PhiSrcReg,
                 MachineInstr *Phi);

  /// Create a PICK for the FinalDestReg that uses the switched control value
  /// from ForkingBB.
  ///
  /// This will insert an instruction (or replace the %ign with a real value
  /// for an existing instruction) of the form
  /// PickReg = PICK ForkingBB.SwitchCtl, PhiSrcReg, %ign
  ///
  /// If PickReg is 0, generate a new register for the result of the PICK. This
  /// probably means that you intend to use it immediately in a subsequent
  /// PICK.
  MachineInstr *PatchOrInsertPickAtFork(MachineBasicBlock *ForkingBB,
                                        unsigned FinalDestReg,
                                        unsigned PhiSrcReg,
                                        MachineBasicBlock *PhiSrcBB,
                                        MachineInstr *Phi,
                                        unsigned PickReg = 0);

  /// Create a partial PICK instruction (the other operand is set to %ign) for
  /// the register Reg when coming from ctrlBB to inBB. If ResultReg is 0,
  /// create a new register to save the result.
  MachineInstr *insertPICKForReg(MachineBasicBlock *ctrlBB, unsigned Reg,
                                 MachineBasicBlock *inBB, MachineInstr *phi,
                                 unsigned ResultReg = 0);

  /// Assign Reg to PickFalseReg or PickTrueReg, and %ign to the other edge,
  /// based on whether or not the ctrlBB->inBB edge is the true or false edge.
  /// If ctrlBB == inBB, use the direction of the control dependence from ctrlBB
  /// to phi's parent block.
  void assignPICKSrcForReg(unsigned &pickFalseReg, unsigned &pickTrueReg,
                           unsigned Reg, MachineBasicBlock *ctrlBB,
                           MachineBasicBlock *inBB, MachineInstr *phi);

  /// Find instances where the same value is used in multiple places in the
  /// PICK tree that replaces the current phi, and insert switches to send the
  /// value to the appropriate target.
  void CombineDuplicatePickTreeInput();

  /// Handle the PICK tree for cases where the basic block might not run
  /// because it does not postdominate its immediate dominator. These show up
  /// as %ign values in the PICK tree.
  void PatchCFGLeaksFromPickTree(unsigned phiDst, MachineBasicBlock *phiHome);

  /// Build a list of operands that indicate the control dependence triggers
  /// from ctrlBB to the dominator of mbb.
  void TraceLeak(MachineBasicBlock *ctrlBB, MachineBasicBlock *mbb,
                 SmallVectorImpl<unsigned> &landOpnds);

  /// This maps generated PICKs to the basic block whose switch they mirror.
  /// Structured phi conversion handles one PHI at a time, but that PHI may
  /// require multiple PICKs to be generated as a pick tree.
  /// Since operands are traced one at a time, some of the PICKs in this map
  /// may have some of their inputs mapped to %IGN, as we have yet to trace the
  /// operands to join them.
  DenseMap<MachineInstr *, MachineBasicBlock *> multiInputsPick;
  /// @}

  /// Given a dataflow loop, pipeline the loop using inner-loop pipelining
  /// that supports at most the given number of concurrent iterations.
  void pipelineLoop(MachineBasicBlock *header, CSALoopInfo &DFLoop,
                    unsigned numTokens);

  /// Map all loops to CSALoopInfo. This does not enter any of the pick or
  /// switch information.
  void prefillLoopInfo(MachineLoop *Loop);

private:
  MachineFunction *thisMF;
  /// An iterator over basic blocks in RPO.
  ReversePostOrderTraversal<MachineFunction *> *RPOT;
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
  DenseMap<MachineBasicBlock *, unsigned> bb2predcpy;
  DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *>
    bb2pick; // switch for Reg added in bb
  DenseMap<MachineBasicBlock *, SmallVectorImpl<unsigned> *> edgepreds;
  DenseMap<MachineBasicBlock *, unsigned> bbpreds;
  DenseMap<MachineBasicBlock *, MachineInstr *> bb2predmerge;
  DenseMap<MachineBasicBlock *, unsigned> bb2rpo;
  DenseMap<MachineLoop *, CSALoopInfo> loopInfo;
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

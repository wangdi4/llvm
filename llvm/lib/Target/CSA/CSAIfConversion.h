//===-- CSAIfConversion.h - CSA if conversion structures ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the machine instruction level if-conversion pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAIFCONVERSION_H
#define LLVM_LIB_TARGET_CSA_CSAIFCONVERSION_H

#include "llvm/ADT/SmallSet.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineBranchProbabilityInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetSchedule.h"

namespace llvm {
  class CSAIfConversion {
  public:
    enum IfcvtKind {
      ICNotClassfied,  // BB data valid, but not classified.
      ICSimpleFalse,   // Same as ICSimple, but on the false path.
      ICSimple,        // BB is entry of an one split, no rejoin sub-CFG.
      ICTriangleFRev,  // Same as ICTriangleFalse, but false path rev condition.
      ICTriangleRev,   // Same as ICTriangle, but true path rev condition.
      ICTriangleFalse, // Same as ICTriangle, but on the false path.
      ICTriangle,      // BB is entry of a triangle sub-CFG.
      ICDiamond        // BB is entry of a diamond sub-CFG.
    };

    /// BBInfo - One per MachineBasicBlock, this is used to cache the result
    /// if-conversion feasibility analysis. This includes results from
    /// TargetInstrInfo::AnalyzeBranch() (i.e. TBB, FBB, and Cond), and its
    /// classification, and common tail block of its successors (if it's a
    /// diamond shape), its size, whether it's predicable, and whether any
    /// instruction can clobber the 'would-be' predicate.
    ///
    /// IsDone          - True if BB is not to be considered for ifcvt.
    /// IsBeingAnalyzed - True if BB is currently being analyzed.
    /// IsAnalyzed      - True if BB has been analyzed (info is still valid).
    /// IsEnqueued      - True if BB has been enqueued to be ifcvt'ed.
    /// IsBrAnalyzable  - True if AnalyzeBranch() returns false.
    /// HasFallThrough  - True if BB may fallthrough to the following BB.
    /// IsUnpredicable  - True if BB is known to be unpredicable.
    /// ClobbersPred    - True if BB could modify predicates (e.g. has
    ///                   cmp, call, etc.)
    /// NonPredSize     - Number of non-predicated instructions.
    /// ExtraCost       - Extra cost for multi-cycle instructions.
    /// ExtraCost2      - Some instructions are slower when predicated
    /// BB              - Corresponding MachineBasicBlock.
    /// TrueBB / FalseBB- See AnalyzeBranch().
    /// BrCond          - Conditions for end of block conditional branches.
    /// Predicate       - Predicate used in the BB.
    struct BBInfo {
      bool IsDone          : 1;
      bool IsBeingAnalyzed : 1;
      bool IsAnalyzed      : 1;
      bool IsEnqueued      : 1;
      bool IsBrAnalyzable  : 1;
      bool HasFallThrough  : 1;
      bool IsUnpredicable  : 1;
      bool CannotBeCopied  : 1;
      bool ClobbersPred    : 1;
      unsigned NonPredSize;
      unsigned ExtraCost;
      unsigned ExtraCost2;
      MachineBasicBlock *BB;
      MachineBasicBlock *TrueBB;
      MachineBasicBlock *FalseBB;
      SmallVector<MachineOperand, 4> BrCond;
      SmallVector<MachineOperand, 4> Predicate;
      BBInfo() : IsDone(false), IsBeingAnalyzed(false),
                 IsAnalyzed(false), IsEnqueued(false), IsBrAnalyzable(false),
                 HasFallThrough(false), IsUnpredicable(false),
                 CannotBeCopied(false), ClobbersPred(false), NonPredSize(0),
                 ExtraCost(0), ExtraCost2(0), BB(nullptr), TrueBB(nullptr),
                 FalseBB(nullptr) {}
    };

    /// IfcvtToken - Record information about pending if-conversions to attempt:
    /// BBI             - Corresponding BBInfo.
    /// Kind            - Type of block. See IfcvtKind.
    /// NeedSubsumption - True if the to-be-predicated BB has already been
    ///                   predicated.
    /// NumDups      - Number of instructions that would be duplicated due
    ///                   to this if-conversion. (For diamonds, the number of
    ///                   identical instructions at the beginnings of both
    ///                   paths).
    /// NumDups2     - For diamonds, the number of identical instructions
    ///                   at the ends of both paths.
    struct IfcvtToken {
      BBInfo &BBI;
      IfcvtKind Kind;
      bool NeedSubsumption;
      unsigned NumDups;
      unsigned NumDups2;
      IfcvtToken(BBInfo &b, IfcvtKind k, bool s, unsigned d, unsigned d2 = 0)
        : BBI(b), Kind(k), NeedSubsumption(s), NumDups(d), NumDups2(d2) {}
    };

    /// BBAnalysis - Results of if-conversion feasibility analysis indexed by
    /// basic block number.
    std::vector<BBInfo> BBAnalysis;
    TargetSchedModel SchedModel;

    const TargetLoweringBase *TLI;
    const TargetInstrInfo *TII;
    const TargetRegisterInfo *TRI;
    const MachineBlockFrequencyInfo *MBFI;
    const MachineBranchProbabilityInfo *MBPI;
    MachineRegisterInfo *MRI;

    LivePhysRegs Redefs;
    LivePhysRegs DontKill;

    bool PreRegAlloc;
    bool MadeChange;
    int FnNum;
  public:
    static char ID;
    CSAIfConversion() {
      //initializeCSAIfConversionPass(*PassRegistry::getPassRegistry());
    }

    /*void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<MachineBlockFrequencyInfo>();
      AU.addRequired<MachineBranchProbabilityInfo>();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

    bool runOnMachineFunction(MachineFunction &MF) override;*/

    BBInfo &AnalyzeBlock(MachineBasicBlock *BB,
                         std::vector<IfcvtToken*> &Tokens);

    bool reverseBranchCondition(BBInfo &BBI);
    bool ValidSimple(BBInfo &TrueBBI, unsigned &Dups,
                     const BranchProbability &Prediction) const;
    bool ValidTriangle(BBInfo &TrueBBI, BBInfo &FalseBBI,
                       bool FalseBranch, unsigned &Dups,
                       const BranchProbability &Prediction) const;
    bool ValidDiamond(BBInfo &TrueBBI, BBInfo &FalseBBI,
                      unsigned &Dups1, unsigned &Dups2) const;
    void ScanInstructions(BBInfo &BBI);
    bool FeasibilityAnalysis(BBInfo &BBI, SmallVectorImpl<MachineOperand> &Cond,
                             bool isTriangle = false, bool RevBranch = false);
    void AnalyzeBlocks(MachineFunction &MF, std::vector<IfcvtToken*> &Tokens);
    void InvalidatePreds(MachineBasicBlock *BB);
    void RemoveExtraEdges(BBInfo &BBI);
    
    // if conversion routines
    bool IfConvertSimple(BBInfo &BBI, IfcvtKind Kind) { return false; }
    bool IfConvertTriangle(BBInfo &BBI, IfcvtKind Kind);
    bool IfConvertDiamond(BBInfo &BBI, IfcvtKind Kind, 
                          unsigned NumDups1, unsigned NumDups2){ return false; }

    void PredicateBlock(BBInfo &BBI,
                        MachineBasicBlock::iterator E,
                        SmallVectorImpl<MachineOperand> &Cond,
                        SmallSet<unsigned, 4> *LaterRedefs = nullptr);
    void CopyAndPredicateBlock(BBInfo &ToBBI, BBInfo &FromBBI,
                               SmallVectorImpl<MachineOperand> &Cond,
                               bool IgnoreBr = false);
    void MergeBlocks(BBInfo &ToBBI, BBInfo &FromBBI, bool AddEdges = true);

    bool MeetIfcvtSizeLimit(MachineBasicBlock &BB,
                            unsigned Cycle, unsigned Extra,
                            const BranchProbability &Prediction) const {
      return Cycle > 0 && TII->isProfitableToIfCvt(BB, Cycle, Extra,
                                                   Prediction);
    }

    bool MeetIfcvtSizeLimit(MachineBasicBlock &TBB,
                            unsigned TCycle, unsigned TExtra,
                            MachineBasicBlock &FBB,
                            unsigned FCycle, unsigned FExtra,
                            const BranchProbability &Prediction) const {
      return TCycle > 0 && FCycle > 0 &&
        TII->isProfitableToIfCvt(TBB, TCycle, TExtra, FBB, FCycle, FExtra,
                                 Prediction);
    }

    // blockAlwaysFallThrough - Block ends without a terminator.
    bool blockAlwaysFallThrough(BBInfo &BBI) const {
      return BBI.IsBrAnalyzable && BBI.TrueBB == nullptr;
    }

    // IfcvtTokenCmp - Used to sort if-conversion candidates.
    static bool IfcvtTokenCmp(IfcvtToken *C1, IfcvtToken *C2) {
      int Incr1 = (C1->Kind == ICDiamond)
        ? -(int)(C1->NumDups + C1->NumDups2) : (int)C1->NumDups;
      int Incr2 = (C2->Kind == ICDiamond)
        ? -(int)(C2->NumDups + C2->NumDups2) : (int)C2->NumDups;
      if (Incr1 > Incr2)
        return true;
      else if (Incr1 == Incr2) {
        // Favors subsumption.
        if (C1->NeedSubsumption == false && C2->NeedSubsumption == true)
          return true;
        else if (C1->NeedSubsumption == C2->NeedSubsumption) {
          // Favors diamond over triangle, etc.
          if ((unsigned)C1->Kind < (unsigned)C2->Kind)
            return true;
          else if (C1->Kind == C2->Kind)
            return C1->BBI.BB->getNumber() < C2->BBI.BB->getNumber();
        }
      }
      return false;
    }
  };

}

#endif

//===----------- HIRCreation.h - Creates HIR nodes ------------*-- C++ --*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This analysis is used to create HIR nodes out of identified HIR regions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRCREATION_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRCREATION_H

#include "llvm/Pass.h"
#include "llvm/IR/Intel_LoopIR/HLNode.h"

namespace llvm {

class Function;
class DominatorTree;
struct PostDominatorTree;

namespace loopopt {

class RegionIdentification;
class HLRegion;

/// \brief This analysis creates and populates HIR regions with HLNodes using
/// the information provided by RegionIdentification pass. The overall sequence
/// of building the HIR is as follows-
///
/// 1) RegionIdentification - identifies regions in IR.
/// 2) HIRCreation - populates HIR regions with a sequence of HLNodes (without
///    HIR loops).
/// 3) LoopFormation - Forms HIR loops within HIR regions.
/// 4) HIRParser - Creates DDRefs and parses SCEVs into CanonExprs.
/// 5) TODO: Add more steps.
///
class HIRCreation : public FunctionPass {
public:
  /// Iterators to iterate over regions
  typedef HLContainerTy::iterator iterator;
  typedef HLContainerTy::const_iterator const_iterator;
  typedef HLContainerTy::reverse_iterator reverse_iterator;
  typedef HLContainerTy::const_reverse_iterator const_reverse_iterator;

private:
  /// Regions - HLRegions formed out of incoming LLVM IR.
  HLContainerTy Regions;

  /// Func - The function we are analyzing.
  Function *Func;

  /// DT - The dominator tree.
  DominatorTree *DT;

  /// PDT - The post-dominator tree.
  PostDominatorTree *PDT;

  /// Labels - TODO: Insert HLLabels to be used by later passes.
  /// SmallVector< pair<BasicBlock*, HLLabel*>, 32> Labels;

  /// Gotos - TODO: Insert HLGotos to be used by later passes.
  /// SmallVector< pair<BasicBlock*, HLGoto*>, 32> Gotos;

  /// Ifs - TODO: Insert HLIfs to be used by later passes.
  /// SmallVector< pair<HLIf*, BasicBlock*>, 32> Ifs;

  /// \brief Populates Region with HLNodes.
  void populateRegion(HLRegion *Region, BasicBlock *EntryBB);

public:
  static char ID; // Pass identification
  HIRCreation();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;

  void create(RegionIdentification *RI);

  /// Region iterator methods
  iterator begin() { return Regions.begin(); }
  const_iterator begin() const { return Regions.begin(); }
  iterator end() { return Regions.end(); }
  const_iterator end() const { return Regions.end(); }

  reverse_iterator rbegin() { return Regions.rbegin(); }
  const_reverse_iterator rbegin() const { return Regions.rbegin(); }
  reverse_iterator rend() { return Regions.rend(); }
  const_reverse_iterator rend() const { return Regions.rend(); }
};

} // End namespace loopopt

} // End namespace llvm

#endif

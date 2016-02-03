//===------- RegionIdentification.h - Identifies HIR regions ---*- C++ --*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis is used to identify sections(regions) of LLVM IR on which loop
// transformations can be applied.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_REGIONIDENTIFICATION_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_REGIONIDENTIFICATION_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/DenseMap.h"

#include "llvm/Pass.h"

namespace llvm {

class Value;
class Function;
class Instruction;
class PHINode;
class Loop;
class LoopInfo;
class DominatorTree;
struct PostDominatorTree;
class ScalarEvolution;
class GetElementPtrInst;
class GEPOperator;
class SCEV;

namespace loopopt {

class IRRegion;

/// \brief This analysis is the first step in creating HIR. We start by
/// identiyfing regions as a set of basic blocks in the incoming IR. This
/// information is then used by HIRCreation pass to create and populate
/// HIR regions.
class RegionIdentification : public FunctionPass {
public:
  typedef SmallVector<IRRegion *, 16> IRRegionsTy;

  /// Iterators to iterate over regions
  typedef IRRegionsTy::iterator iterator;
  typedef IRRegionsTy::const_iterator const_iterator;
  typedef IRRegionsTy::reverse_iterator reverse_iterator;
  typedef IRRegionsTy::const_reverse_iterator const_reverse_iterator;

private:
  /// IRRegions - Vector of IRRegion.
  IRRegionsTy IRRegions;

  /// LI - The loop information for the function we are currently analyzing.
  LoopInfo *LI;

  /// DT - The dominator tree.
  DominatorTree *DT;

  /// PDT - The post-dominator tree.
  PostDominatorTree *PDT;

  /// SE - Scalar Evolution analysis for the function.
  ScalarEvolution *SE;

  /// \brief Returns the base(earliest) GEP operator in case of multiple GEPs
  /// associated with a load/store.
  /// This is used for temporarily suppressing struct GEPs until we can handle
  /// them in HIR.
  const GEPOperator *getBaseGEPOp(const GEPOperator *GEPOp) const;

  /// \brief Returns true if Lp appears to be generable without looking at the
  /// sub loops.
  bool isSelfGenerable(const Loop &Lp, unsigned LoopnestDepth) const;

  /// \brief Returns true if Lp is a SIMD loop.
  bool isSIMDLoop(const Loop &Lp) const;

  /// \brief Creates a Region out of Lp's basic blocks.
  void createRegion(const Loop &Lp);

  /// \brief Returns true if we can form a region around this loop. Returns the
  /// max loopnest depth in LoopnestDepth.
  bool formRegionForLoop(const Loop &Lp, unsigned *LoopnestDepth);

  /// \brief Identifies regions in the incoming LLVM IR.
  void formRegions();

  /// \brief Returns true if Inst contains a type not supported by HIR.
  bool containsUnsupportedTy(const Instruction *Inst) const;

  /// \brief Returns IV definition PHINode of the loop.
  const PHINode *findIVDefInHeader(const Loop &Lp,
                                   const Instruction *Inst) const;

public:
  static char ID; // Pass identification
  RegionIdentification();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;

  /// IRRegion iterator methods
  iterator begin() { return IRRegions.begin(); }
  const_iterator begin() const { return IRRegions.begin(); }
  iterator end() { return IRRegions.end(); }
  const_iterator end() const { return IRRegions.end(); }

  reverse_iterator rbegin() { return IRRegions.rbegin(); }
  const_reverse_iterator rbegin() const { return IRRegions.rbegin(); }
  reverse_iterator rend() { return IRRegions.rend(); }
  const_reverse_iterator rend() const { return IRRegions.rend(); }

  unsigned getNumRegions() const { return IRRegions.size(); }

  /// \brief Returns true if this type is supported. Currently returns false for
  /// structure and function types.
  bool isSupported(Type *Ty) const;

  // NOTE: Following functions were moved here so they can be shared between
  // HIRParser and SSADeconstruction. Is there a better way?

  /// \brief Returns the primary element type of PtrTy. Primary element type is
  /// the underlying type which this type is pointing to. For example, the
  /// primary element type of both i32* && [100 x [100 x i32]]* is i32.
  Type *getPrimaryElementType(Type *PtrTy) const;

  /// \brief Returns true if Phi occurs in the header of a loop.
  bool isHeaderPhi(const PHINode *Phi) const;
};

} // End namespace loopopt

} // End namespace llvm

#endif

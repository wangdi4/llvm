// INTEL CONFIDENTIAL
//
// Copyright 2014-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __PRELEGALIZEBOOLS_PASS__H__
#define __PRELEGALIZEBOOLS_PASS__H__

#include "llvm/ADT/SetVector.h"
#include "llvm/Pass.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"

#include <list>
#include <set>
#include <map>

namespace intel {

  //         Boolean vectors are illegal for CPU and in the current LLVM CG any illegal type is
  //         always legalized to a closest legal type.
  //         For example <8 x i1> is legalized to <8 x i16> so when a boolean vector is produced
  //         by an icmp gt <8 x i32> instruction (i.e. VPCMPGTD) its <8 x i32> result is converted
  //         to <8 x i16>.  This result further can be passed to an instruction which accepts <8 x i32>
  //         vector of booleans so it is to be converted back (PTEST is an example).
  //         Machine Code Optimizations optimize out such unnecessary conversions inside a BB but
  //         if boolean vector crosses a BB boundary these conversions are stay intact.
  //
  //         This pass does the following transformation to mitigate this issue.
  //
  //         Replace
  //          BB0:
  //            ...
  //            %orig.i1v0 = icmp <op> <N x i32> ...
  //            ...
  //          BB2:
  //            %phi0 = phi <N x i1> [ %orig.i1v0, %BB0 ], [ %orig.i1v1, %BB1 ]
  //            ...
  //            %0 = SELECT <N x i1> cond, <N x i1> %phi, %...
  //            ...
  //            %X = LOGOP <N x i1> %X-1, %X-n
  //            %1 = SEXT <N x i1> to <N x i32>
  //          BB1:
  //            ...
  //            %orig.i1v1 = icmp <op> <N x i32> ...
  //            ...
  //
  //         With
  //          BB0:
  //            ...
  //            %orig.i1v0 = icmp <op> <N x i32>
  //            SEXT %orig.i1v0 to <N x i32>
  //          BB2:
  //            %phi = phi <N x i32> [ %orig.i32v0, %BB0 ], [ %orig.i32v1, %BB1 ]
  //            ...
  //            %0 = SELECT <N x i1> cond, <N x i32> %phi, %...
  //            ...
  //            %X = LOGOP <N x i32> %..., %...
  //            ...
  //          BB1:
  //            ...
  //            %orig.i1v1 = icmp <op> <N x i32>
  //            SEXT %orig.i1v1 to <N x i32>
  //            ...

  /// @brief Function pass to mitigate legalization issue of boolean vectors.
  //         GVN pass should be ran after this pass to clean up redundant instructions.
  //         InstCombine pass should be ran to combine "trunc -> zext" (and similar)
  //         sequences.
  //
  //         Related CQ ticket: CSSD100019855
  class PreLegalizeBools : public llvm::ModulePass
  {
  public:
    static char ID; // Pass identification, replacement for typeid

    // Constructor
    PreLegalizeBools();

    bool runOnModule(llvm::Module &M);

    bool runOnFunction(llvm::Function &F);

  private:
    /// \brief Produces an extended to pNewTy value of pVal
    llvm::Value * makeSExtValue(llvm::Value * pVal, llvm::Type * pNewTy);
    /// \brief Replace an old instruction with a truncation of a new instruction.
    /// \param pOldInst An old boolean instruction.
    /// \param pNewInst A new wide instruction.
    /// \param pInsertBefore Insert a new Trunc instructions before this one.
    void replace(llvm::Instruction * pOldInst, llvm::Instruction * pNewInst,
                 llvm::Instruction * pInsertBefore);

    /// \brief Check for the pattern strarting from SExt instruction
    bool testSExt(llvm::Instruction * pInst);

    /// \brief Replace calls to i1 versions of __ocl_allOne, __ocl_allZero w\ i32 versions
    bool replaceAllOneZeroCalls(llvm::Function &F);
    /// \brief Bottom-up propagation of sign extended boolean vectors
    bool propagateSExt(llvm::Function &F);

    /// \brief List of detected patterns
    llvm::SetVector<llvm::Instruction *> m_workSet;
    /// \brief Cache to minimize redundant instructions
    std::map<llvm::Type *, std::map<llvm::Value *, llvm::Value *> > m_sextCache;
    /// \brief Set of Trunc instructions created by this pass
    std::set<llvm::Instruction *> m_edgeTruncs;
  };
}
#endif

#if INTEL_COLLAB // -*- C++ -*-
//=======-- VPOUtils.h - Class definitions for VPO utilites -*- C++ -*-=======//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the VPOUtils class and provides a set of common utilities
/// that are shared across the Vectorizer and Parallelizer.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORM_VPO_UTILS_VPOUTILS_H
#define LLVM_TRANSFORM_VPO_UTILS_VPOUTILS_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Analysis/Intel_Directives.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionNode.h"
#include <unordered_map>

// Used for Parallel Section Transformations
#include <stack>
#include "llvm/IR/IRBuilder.h"

namespace llvm {

class Value;
class Module;
class Function;
class Type;
class BasicBlock;
class Loop;
class LoopInfo;
class DominatorTree;
class StringRef;
class CallInst;
class IntrinsicInst;
class Constant;
class LLVMContext;

namespace vpo {

/// Data structure Used for Parallel Section Transformations
typedef struct ParSectNode {
  BasicBlock *EntryBB;
  BasicBlock *ExitBB;
  int DirBeginID;
  SmallVector<ParSectNode *, 8> Children;
} ParSectNode;

/// This class contains a set of utility functions used by VPO passes.
class VPOUtils {
private:
  /// Add PHI instructions to `merged bblock` for any value that is live
  /// outside \p BBSet. Here `merged bblock` is the successor of ExitBB in \p
  /// BBSet, where \p BBSet and `ClonedBBSet` (stored in `VMap`) merge.
  static void addPHINodes(ValueToValueMapTy &VMap,
                          SmallVectorImpl<BasicBlock *> &BBSet,
                          SmallVectorImpl<Instruction *> &LiveOut);

public:
  /// \name Constructor and Destructor
  /// @{
  VPOUtils() {}
  ~VPOUtils() {}
  /// @}

  /// Restructure the CFG on demand such that each
  /// directive for Cilk, OpenMP, Offload, Vectorization is put into a
  /// standalone basic block. This is a pre-required process for WRegion
  /// construction for each function.
  ///
  /// Note that since WRegion construction requires
  /// DominatorTreeWrapperPass and LoopInfoWrapperPass to be executed prior
  /// to it, when calling CFGRestructuring, we need to update DominatorTree
  /// and LoopInfo whenever a basic block splitting happens.
  static void CFGRestructuring(Function &F, DominatorTree *DT = nullptr,
                               LoopInfo *LI = nullptr);

  /// Remove calls to directive intrinsics from WRegionNode \p WRN.
  /// By default, the util removes the directive intrinsic calls from the
  /// Entry and Exit BBlocks of \p WRN. This can be extended to handle needs
  /// of specific WRegionNode kinds.
  /// \returns \b true if <em>one or more</em> directive intrinsics were
  /// stripped from \em each of the entry as well as exit BasicBlocks of
  /// \p WRN; \b false otherwise.
  static bool stripDirectives(WRegionNode *WRN);

  /// Remove calls to directive intrinsics from BasicBlock \p BB.
  /// \returns \b true if <em>one or more</em> directive intrinsics were
  /// stripped from \p BB; \b false otherwise.
  static bool stripDirectives(BasicBlock &BB);

  /// Remove calls to directive intrinsics from function \p F.
  /// \returns \b true if <em>one or more</em> directive intrinsics were
  /// stripped from \p F; \b false otherwise.
  static bool stripDirectives(Function &F);

  /// Remove OpenMP PRIVATE clauses if present.
  /// \returns \b true if removal happened.
  static bool stripPrivateClauses(WRegionNode *WRN);
  static bool stripPrivateClauses(BasicBlock &BB);

  /// Remove `@llvm.dbg.declare`, `@llvm.dbg.value` calls from \p F.
  /// This is a temporary workaround needed because CodeExtractor does not
  /// update these calls present in the region to be extracted. So they have
  /// invalid metadata when present in the extracted function, causing
  /// verification failure with `-fiopenmp -O0 -g`.
  static void stripDebugInfoInstrinsics(Function &F);

  /// Copy data from the \p Source to the \p Destination.
  static void genCopyFromSrcToDst(Type *AllocaTy, const DataLayout &DL,
                                  IRBuilder<> &Builder, AllocaInst *NewPrivInst,
                                  Value *Source, Value *Destination,
                                  BasicBlock *InsertBB);

  /// Generate the alias_scope and no_alias metadata for the incoming BBs.
  static void genAliasSet(ArrayRef<BasicBlock *> BBs, AliasAnalysis *AA,
                          const DataLayout *DL);

  /// Generate a memcpy call with the destination argument \p D and the source
  /// argument \p S at the end of basic block \p BB.
  /// \code
  ///   call void @llvm.memcpy.p0i8.p0i8.i32(i8* bitcast (i32* @a to i8*),
  ///                                        i8* %2,
  ///                                        i32 4,
  ///                                        i32 4,
  ///                                        i1 false)
  /// \endcode
  static CallInst *genMemcpy(Value *D, Value *S, const DataLayout &DL,
                             unsigned Align, BasicBlock *BB);

  /// Return true if the type of AI instruction is not single vlaue type.
  static bool isNotLegalSingleValueType(AllocaInst *AI);

  /// \name MultiVersioning Transformation
  /// @{

  /// Given a single-entry and single-exit region represented by BBSet,
  /// generate the code: `if(Cond) BBSet; else ClonedBBSet;`. Where
  /// ClonedBBSet is the cloned region of BBSet.
  ///
  /// Client needs to recompute Loop information if needed and may need to
  /// recompute WRegion information as well depending on what the cloned
  /// region will look like, e.g., becoming serial code or remaining as a
  /// parallel region.
  static void singleRegionMultiVersioning(BasicBlock *EntryBB,
                                          BasicBlock *ExitBB,
                                          SmallVectorImpl<BasicBlock *> &BBSet,
                                          Value *Cond,
                                          DominatorTree *DT = nullptr);

  /// Clone all the basic blocks in \p BBSet to \p ClonedBBSet and remap
  /// all the values in the \p ClonedBBSet. The function does not maintain DT
  /// and LI information. So the client needs to do that. The cloned
  /// basic blocks are attached to the end of function \p F's basic block
  /// list. Function \p F can be the same as where BBSet resides, or
  /// different. \p CodeInfo (optional) helps to collect additional
  /// information about the cloned region, such as if it contains call,
  /// dynamic or static alloca instructions. \p VMap bookkeeps both basic
  /// block mapping and instruction mapping.
  static void cloneBBSet(SmallVectorImpl<BasicBlock *> &BBSet,
                         SmallVectorImpl<BasicBlock *> &ClonedBBSet,
                         ValueToValueMapTy &VMap, const Twine &NameSuffix,
                         Function *F, ClonedCodeInfo *CodeInfo = nullptr);

  /// Collect all the instructions in \p BBSet which have uses (live-out)
  /// outside the \p BBSet and store them in \p LiveOut.
  static void
  findDefsUsedOutsideOfRegion(SmallVectorImpl<BasicBlock *> &BBSet,
                              SmallVectorImpl<Instruction *> &LiveOut);

  /// @}

  /// \name Functions for Parallel Section Transformation
  /// @{

  /// Transform OpenMP parallel sections to parallel do loop, and work-sharing
  /// sections to work-sharing do loop in function \p F. Use a post-order
  /// travesal on the Section Tree (built based on the Dominator Tree) to do
  /// the transformation so that children can be deleted after
  /// transformation at each node. The core transforamtion function is
  /// doParSectTrans.
  static bool parSectTransformer(Function *F, DominatorTree *DT = nullptr);
  static void parSectTransRecursive(Function *F, ParSectNode *Node,
                                    int &Counter, DominatorTree *DT = nullptr);
  static void doParSectTrans(Function *F, ParSectNode *Node, int Counter,
                             DominatorTree *DT = nullptr);

  // Generate an empty loop with lower bound \p LB, upper bound \p UB, stride
  // \p stride, right after the InsertBlock pointed by \p Builder.
  static Value *genNewLoop(Value *LB, Value *UB, Value *Stride,
                           IRBuilder<> &Builder, int Counter,
                           Value *&NormalizedUB, DominatorTree *DT = nullptr);

  // Generate a switch statement for the parallel section or work-sharing
  // section represented by \p Node. Each section in the parallel section or
  // work-sharing section corresponds to a case statement in the switch.
  static void genParSectSwitch(Value *SwitchCond, ParSectNode *Node,
                               IRBuilder<> &Builder, int Counter,
                               DominatorTree *DT = nullptr);

  // Generate a tree data structure called Section Tree to represent the
  // nesting relationship of OMP_PARALLEL_SECTIONS (or OMP_SECTIONS) and
  // OMP_SECTION in a function. It is used for the transformations of OpenMP
  // parallel sections and work-sharing sections.
  static ParSectNode *buildParSectTree(Function *F,
                                       DominatorTree *DT = nullptr);
  static void buildParSectTreeRecursive(BasicBlock *BB,
                                        std::stack<ParSectNode *> &SectionStack,
                                        DominatorTree *DT = nullptr);

  static void printParSectTree(ParSectNode *Node);

  static void
  gatherImplicitSectionRecursive(BasicBlock *BB,
                                 std::stack<ParSectNode *> &ImpSectStack,
                                 DominatorTree *DT = nullptr);

  static void insertSectionRecursive(Function *F, ParSectNode *Node,
                                     int &Counter, DominatorTree *DT = nullptr);

  /// @}

  /// \name Functions for vector code generation
  /// @{

  /// Return a call to the `llvm.masked.gather` intrinsic. A null Mask
  /// defaults to an unmasked gather. A null PassThru value uses undef value
  /// for pass through value.
  static CallInst *createMaskedGatherCall(Value *VecPtr, IRBuilder<> &Builder,
                                          unsigned Alignment = 0,
                                          Value *Mask = nullptr,
                                          Value *PassThru = nullptr);

  /// Return a call to the `llvm.masked.load intrinsic`. It uses the
  /// interface provided by IRBuilder. A null PassThru value uses undef value
  /// for pass through value.
  static CallInst *createMaskedLoadCall(Value *VecPtr, IRBuilder<> &Builder,
                                        unsigned Alignment, Value *Mask,
                                        Value *PassThru = nullptr);

  /// Return a call to the `llvm.masked.scatter` intrinsic. A null Mask
  /// defaults to an unmasked scatter.
  static CallInst *createMaskedScatterCall(Value *VecPtr, Value *VecData,
                                           IRBuilder<> &Builder,
                                           unsigned Alignment = 0,
                                           Value *Mask = nullptr);

  /// Return a call to the `llvm.masked.store` intrinsic. It uses the
  /// interface provided by IRBuilder.
  static CallInst *createMaskedStoreCall(Value *VecPtr, Value *VecData,
                                         IRBuilder<> &Builder,
                                         unsigned Alignment, Value *Mask);

  /// @}
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VPO_UTILS_VPOUTILS_H
#endif // INTEL_COLLAB

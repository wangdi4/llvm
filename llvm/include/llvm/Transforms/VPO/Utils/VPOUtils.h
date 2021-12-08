#if INTEL_COLLAB // -*- C++ -*-
//===------ VPOUtils.h - Class definitions for VPO utilites -*- C++ -*-----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
#include "llvm/Analysis/Directives.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include <unordered_map>

// Used for Parallel Section Transformations
#include <stack>
#include "llvm/IR/IRBuilder.h"

namespace llvm {

class AAResults;
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
class TargetLibraryInfo;

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

  // Restore the clause operands by undoing the renaming done in the prepare
  // pass.
  static bool restoreOperands(Function &F);

  // Remove the branch from entry directive to end directive generated to
  // prevent deletion of end directive in case it's dead code.
  static bool removeBranchesFromBeginToEndDirective(
      Function &F, const TargetLibraryInfo *TLI, DominatorTree *DT);

  /// If \p ValWithCasts is a CastInst, or a chain of CastInsts, the function
  /// recursively gets its operand until it encounters a non-CastInst. All
  /// CastInsts seen in this process are added to \p SeenCastInsts.
  /// \returns The base Value obtained after traversing all Casts.
  static Value *stripCasts(Value *ValWithCasts,
                           SmallVectorImpl<Instruction *> &SeenCastInsts);

  /// Return \b true if \p V is a pointer cast, address space cast, or
  /// zero-offset GEP; \b false otherwise.
  static bool isPointerCastOrZeroOffsetGEP(Value *V);

  /// If \p F has an attribute "may-have-openmp-directive", and it is not
  /// already `false`, then set it to `false`, and return \b true; return
  /// \b false otherwise.
  static bool unsetMayHaveOpenmpDirectiveAttribute(Function &F);

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
  /// If \p IDs is not empty, then the method will only remove
  /// calls to directive intrinsics with the specified directive ids.
  static bool stripDirectives(BasicBlock &BB, ArrayRef<int> IDs = None);

  /// Remove calls to directive intrinsics from function \p F.
  /// \returns \b true if <em>one or more</em> directive intrinsics were
  /// stripped from \p F; \b false otherwise.
  /// If \p IDs is not empty, then the method will only remove
  /// calls to directive intrinsics with the specified directive ids.
  static bool stripDirectives(Function &F, ArrayRef<int> IDs = None);

  /// Remove `@llvm.dbg.declare`, `@llvm.dbg.value` calls from \p F.
  /// This is a temporary workaround needed because CodeExtractor does not
  /// update these calls present in the region to be extracted. So they have
  /// invalid metadata when present in the extracted function, causing
  /// verification failure with `-fiopenmp -O0 -g`.
  static void stripDebugInfoInstrinsics(Function &F);
#if INTEL_CUSTOMIZATION

  /// Generate the alias_scope and no_alias metadata for the incoming BBs.
  static void genAliasSet(ArrayRef<BasicBlock *> BBs, AAResults *AA,
                          const DataLayout *DL);
#endif  // INTEL_CUSTOMIZATION

  /// Generate a memcpy call with the destination argument \p D, the source
  /// argument \p S and size argument \p Size. \p Align specifies
  /// maximum guanranteed alignment of \p D and \p S. The new call is inserted
  /// at the end of basic block \p BB.
  /// \code
  ///   call void @llvm.memcpy.p0i8.p0i8.i32(i8* bitcast (i32* @a to i8*),
  ///                                        i8* %2,
  ///                                        i32 4,
  ///                                        i32 4,
  ///                                        i1 false)
  /// \endcode
  static CallInst *genMemcpy(Value *D, Value *S, uint64_t Size,
                             unsigned Align, BasicBlock *BB);

  /// Generate a memcpy call with the destination argument \p D, the source
  /// argument \p S and size argument \p Size. \p Align specifies
  /// maximum guanranteed alignment of \p D and \p S. The new call is inserted
  /// using \p MemcpyBuilder IRBuilder.
  static CallInst *genMemcpy(Value *D, Value *S, uint64_t Size,
                             unsigned Align, IRBuilder<> &MemcpyBuilder);

  /// Generate a memset call with the pointer argument \p P, the value
  /// argument \p V and size argument \p Size. \p Align specifies
  /// maximum guanranteed alignment of \p P. The new call is inserted
  /// using \p MemsetBuilder IRBuilder.
  static CallInst *genMemset(Value *P, Value *V, uint64_t Size,
                             unsigned Align, IRBuilder<> &MemsetBuilder);

  /// Return true if the type can be registerized.
  static bool canBeRegisterized(Type *ScalarTy, const DataLayout &DL);

  /// \name MultiVersioning Transformation
  /// @{

  /// Given a single-entry and single-exit region represented by BBSet,
  /// generate the code: `if(Cond) BBSet; else ClonedBBSet;`. Where
  /// ClonedBBSet is the cloned region of BBSet.
  /// The output parameter \p VMap is a value map between the original
  /// and the cloned instructions and blocks. \p DT and \p LI are updated
  /// accordingly, if they are not null.
  static void singleRegionMultiVersioning(BasicBlock *EntryBB,
                                          BasicBlock *ExitBB,
                                          SmallVectorImpl<BasicBlock *> &BBSet,
                                          ValueToValueMapTy &VMap,
                                          Value *Cond,
                                          DominatorTree *DT = nullptr,
                                          LoopInfo *LI = nullptr);

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
  static bool parSectTransformer(Function *F, DominatorTree *DT, LoopInfo *LI);
  static void parSectTransRecursive(Function *F, ParSectNode *Node,
                                    int &Counter, DominatorTree *DT,
                                    LoopInfo *LI);
  static void doParSectTrans(Function *F, ParSectNode *Node, int Counter,
                             DominatorTree *DT, LoopInfo *LI);

  // Generate an empty loop with lower bound \p LB, upper bound \p UB, stride
  // \p stride, right after the InsertBlock pointed by \p Builder.
  static Value *genNewLoop(Value *LB, Value *UB, Value *Stride,
                           IRBuilder<> &Builder, int Counter,
                           Value *&NormalizedUB, DominatorTree *DT,
                           LoopInfo *LI);

  // Generate a switch statement for the parallel section or work-sharing
  // section represented by \p Node. Each section in the parallel section or
  // work-sharing section corresponds to a case statement in the switch.
  static void genParSectSwitch(Value *SwitchCond, Type *SwitchCondTy,
                               ParSectNode *Node, IRBuilder<> &Builder,
                               int Counter, DominatorTree *DT, LoopInfo *LI);

  // Generate a tree data structure called Section Tree to represent the
  // nesting relationship of OMP_PARALLEL_SECTIONS (or OMP_SECTIONS) and
  // OMP_SECTION in a function. It is used for the transformations of OpenMP
  // parallel sections and work-sharing sections.
  static ParSectNode *buildParSectTree(Function *F,
                                       DominatorTree *DT = nullptr);
  static void buildParSectTreeIterative(BasicBlock *BB,
                                        std::stack<ParSectNode *> &SectionStack,
                                        DominatorTree *DT = nullptr);

  static void printParSectTree(ParSectNode *Node);

  static void
  gatherImplicitSectionIterative(BasicBlock *BB,
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

  /// Creates a clone of \p CI, and adds \p OpBundlesToAdd the new
  /// CallInst. \returns the created CallInst, if it created one, \p CI
  /// otherwise (when \p OpBundlesToAdd is empty).
  static CallInst *addOperandBundlesInCall(
      CallInst *CI,
      ArrayRef<std::pair<StringRef, ArrayRef<Value *>>> OpBundlesToAdd);

  /// Creates a clone of \p CI without any operand bundles whose tags match an
  /// entry in \p OpBundlesToRemove. Replaces all uses of the original \p CI
  /// with the new Instruction created.
  /// \returns the created CallInst, if it created one, \p CI otherwise (when \p
  /// OpBundlesToRemove is empty, or has no matching bundle on \p CI).
  static CallInst *
  removeOperandBundlesFromCall(CallInst *CI,
                               ArrayRef<StringRef> OpBundlesToRemove);

  /// Creates a clone of \p CI without any operand bundles that represent
  /// OpenMP clauses with clause id matching any of the ids in \p ClauseIds.
  /// Replaces all uses of the original \p CI with the new Instruction created.
  /// \returns the created CallInst, if it created one, \p CI otherwise (when
  /// \p ClauseIds is empty, or has no matching clauses on \p CI).
  ///
  /// Note that this method is different from removeOperandBundlesFromCall(),
  /// because it will remove a clause regardless of the modifiers attached
  /// to it. removeOperandBundlesFromCall() looks for exact string match
  /// of the bundle tag.
  static CallInst *removeOpenMPClausesFromCall(CallInst *CI,
                                               ArrayRef<int> ClauseIds);

  /// "Privatizes" an Instruction \p I by adding it to a supported entry
  /// directive clause.
  /// If the Instruction is already used in a directive, nothing is done.
  /// \p BlockPos: The first dominating entry directive over this block is
  /// chosen.
  /// \p SimdOnly: Only search for SIMD directives.
  /// Return false if no directive was found.
  /// Intended to be used outside of paropt when creating new values inside
  /// a region.
  static bool addPrivateToEnclosingRegion(AllocaInst *I, BasicBlock *BlockPos,
                                          DominatorTree &DT, bool SimdOnly);

  /// Returns the next enclosing OpenMP begin directive, or nullptr if none.
  static IntrinsicInst *enclosingBeginDirective(Instruction *I,
                                                DominatorTree *DT);
  /// @}
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VPO_UTILS_VPOUTILS_H
#endif // INTEL_COLLAB

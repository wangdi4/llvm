//===-- Scalar.h - Scalar Transformations -----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This header file defines prototypes for accessor functions that expose passes
// in the Scalar transformations library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_H
#define LLVM_TRANSFORMS_SCALAR_H

#include <functional>

namespace llvm {

class Function;
class FunctionPass;
class ModulePass;
class Pass;
class GetElementPtrInst;
class PassInfo;
class TargetLowering;
class TargetMachine;

//===----------------------------------------------------------------------===//
//
// ConstantPropagation - A worklist driven constant propagation pass
//
FunctionPass *createConstantPropagationPass();

//===----------------------------------------------------------------------===//
//
// AlignmentFromAssumptions - Use assume intrinsics to set load/store
// alignments.
//
FunctionPass *createAlignmentFromAssumptionsPass();

//===----------------------------------------------------------------------===//
//
// SCCP - Sparse conditional constant propagation.
//
FunctionPass *createSCCPPass();

//===----------------------------------------------------------------------===//
//
// DeadInstElimination - This pass quickly removes trivially dead instructions
// without modifying the CFG of the function.  It is a FunctionPass.
//
Pass *createDeadInstEliminationPass();

//===----------------------------------------------------------------------===//
//
// DeadCodeElimination - This pass is more powerful than DeadInstElimination,
// because it is worklist driven that can potentially revisit instructions when
// their other instructions become dead, to eliminate chains of dead
// computations.
//
FunctionPass *createDeadCodeEliminationPass();

//===----------------------------------------------------------------------===//
//
// DeadStoreElimination - This pass deletes stores that are post-dominated by
// must-aliased stores and are not loaded used between the stores.
//
FunctionPass *createDeadStoreEliminationPass();


//===----------------------------------------------------------------------===//
//
// CallSiteSplitting - This pass split call-site based on its known argument
// values.
FunctionPass *createCallSiteSplittingPass();

//===----------------------------------------------------------------------===//
//
// AggressiveDCE - This pass uses the SSA based Aggressive DCE algorithm.  This
// algorithm assumes instructions are dead until proven otherwise, which makes
// it more successful are removing non-obviously dead instructions.
//
FunctionPass *createAggressiveDCEPass();
FunctionPass *createUnskippableAggressiveDCEPass(); // INTEL

//===----------------------------------------------------------------------===//
//
// GuardWidening - An optimization over the @llvm.experimental.guard intrinsic
// that (optimistically) combines multiple guards into one to have fewer checks
// at runtime.
//
FunctionPass *createGuardWideningPass();


//===----------------------------------------------------------------------===//
//
// LoopGuardWidening - Analogous to the GuardWidening pass, but restricted to a
// single loop at a time for use within a LoopPassManager.  Desired effect is
// to widen guards into preheader or a single guard within loop if that's not
// possible.
//
Pass *createLoopGuardWideningPass();


//===----------------------------------------------------------------------===//
//
// BitTrackingDCE - This pass uses a bit-tracking DCE algorithm in order to
// remove computations of dead bits.
//
FunctionPass *createBitTrackingDCEPass();

//===----------------------------------------------------------------------===//
//
// SROA - Replace aggregates or pieces of aggregates with scalar SSA values.
//
FunctionPass *createSROAPass();

//===----------------------------------------------------------------------===//
//
// InductiveRangeCheckElimination - Transform loops to elide range checks on
// linear functions of the induction variable.
//
Pass *createInductiveRangeCheckEliminationPass();

//===----------------------------------------------------------------------===//
//
// InductionVariableSimplify - Transform induction variables in a program to all
// use a single canonical induction variable per loop.
//
Pass *createIndVarSimplifyPass();

//===----------------------------------------------------------------------===//
//
// LICM - This pass is a loop invariant code motion and memory promotion pass.
//
Pass *createLICMPass();
Pass *createLICMPass(unsigned LicmMssaOptCap,
                     unsigned LicmMssaNoAccForPromotionCap);

//===----------------------------------------------------------------------===//
//
// LoopSink - This pass sinks invariants from preheader to loop body where
// frequency is lower than loop preheader.
//
Pass *createLoopSinkPass();

//===----------------------------------------------------------------------===//
//
// LoopPredication - This pass does loop predication on guards.
//
Pass *createLoopPredicationPass();

//===----------------------------------------------------------------------===//
//
// LoopInterchange - This pass interchanges loops to provide a more
// cache-friendly memory access patterns.
//
Pass *createLoopInterchangePass();

//===----------------------------------------------------------------------===//
//
// LoopStrengthReduce - This pass is strength reduces GEP instructions that use
// a loop's canonical induction variable as one of their indices.
//
Pass *createLoopStrengthReducePass();

//===----------------------------------------------------------------------===//
//
// LoopUnswitch - This pass is a simple loop unswitching pass.
//
Pass *createLoopUnswitchPass(bool OptimizeForSize = false,
                             bool hasBranchDivergence = false);

//===----------------------------------------------------------------------===//
//
// LoopInstSimplify - This pass simplifies instructions in a loop's body.
//
Pass *createLoopInstSimplifyPass();

//===----------------------------------------------------------------------===//
//
// LoopUnroll - This pass is a simple loop unrolling pass.
//
Pass *createLoopUnrollPass(int OptLevel = 2, bool OnlyWhenForced = false,
                           bool ForgetAllSCEV = false, int Threshold = -1,
                           int Count = -1, int AllowPartial = -1,
                           int Runtime = -1, int UpperBound = -1,
                           int AllowPeeling = -1);
// Create an unrolling pass for full unrolling that uses exact trip count only.
Pass *createSimpleLoopUnrollPass(int OptLevel = 2, bool OnlyWhenForced = false,
                                 bool ForgetAllSCEV = false);

//===----------------------------------------------------------------------===//
//
// LoopUnrollAndJam - This pass is a simple loop unroll and jam pass.
//
Pass *createLoopUnrollAndJamPass(int OptLevel = 2);

//===----------------------------------------------------------------------===//
//
// LoopReroll - This pass is a simple loop rerolling pass.
//
Pass *createLoopRerollPass();

//===----------------------------------------------------------------------===//
//
// LoopRotate - This pass is a simple loop rotating pass.
//
Pass *createLoopRotatePass(int MaxHeaderSize = -1);

//===----------------------------------------------------------------------===//
//
// LoopIdiom - This pass recognizes and replaces idioms in loops.
//
Pass *createLoopIdiomPass();

//===----------------------------------------------------------------------===//
//
// LoopVersioningLICM - This pass is a loop versioning pass for LICM.
//
Pass *createLoopVersioningLICMPass();

//===----------------------------------------------------------------------===//
//
// DemoteRegisterToMemoryPass - This pass is used to demote registers to memory
// references. In basically undoes the PromoteMemoryToRegister pass to make cfg
// hacking easier.
//
FunctionPass *createDemoteRegisterToMemoryPass();
extern char &DemoteRegisterToMemoryID;

//===----------------------------------------------------------------------===//
//
// Reassociate - This pass reassociates commutative expressions in an order that
// is designed to promote better constant propagation, GCSE, LICM, PRE...
//
// For example:  4 + (x + 5)  ->  x + (4 + 5)
//
FunctionPass *createReassociatePass();

#if INTEL_CUSTOMIZATION
//===----------------------------------------------------------------------===//
//
// SmartReassociate - Targeted reassociation that increases reuse across
// statements containing Adds and Subs.
//
// For example:  X = A - B - C  -> X = A - (B + C)
//               Y = A + B + C     X = A + (B + C)
//
FunctionPass *createAddSubReassociatePass();

//===----------------------------------------------------------------------===//
//
// JumpThreading - Thread control through mult-pred/multi-succ blocks where some
// preds always go to some succ. Thresholds other than minus one override the
// internal BB duplication default threshold.
//

// AllowCFGSimps is an Intel-specific argument that specifies whether the jump
// threading pass may perform simple CFG simplifications other than jump
// threading. CFGSimplification does a more thorough job of exploring the
// possible CFG simplifications, so when running jump threading before
// CFGSimplification, we want it to do jump threading and nothing else.
//
FunctionPass *createJumpThreadingPass(int Threshold = -1,
                                      bool AllowCFGSimps = true);
// NonLTOGlobalOptimizerPass is a pass which pormotes the non escaped block
// scope global variables into the registers.
FunctionPass *createNonLTOGlobalOptimizerPass();

// RemoveRegionDirectivesLegacyPass is a pass which removes region directives.
FunctionPass *createRemoveRegionDirectivesLegacyPass();

// StdContainerOptPass is a pass which generates the std container
// metadata based on the analysis of std container intrinisc.
FunctionPass *createStdContainerOptPass();

// TbaaMDPropagationLegacyPass is a pass which recovers the tbaa information
// for the return pointer dereferences after functions have been inlined.
FunctionPass *createTbaaMDPropagationLegacyPass();

// TransformFPGARegPass is a pass which transforms __fpga_reg builtin
// from llvm.annotation.*/llvm.ptr.annotation.* intrinsic representation
// to fpga.reg.* intrinsic representation
ModulePass *createTransformFPGARegPass();

// CleanupFakeLoadsPass is a pass which removes intel.fakeload intrinsics
// (which are needed by TbaaMDPropagationPass) after all inlining is finished.
FunctionPass *createCleanupFakeLoadsPass();

// IndirectCallConv - Converts indirect calls to direct calls using
// points-to info and/or DTrans Field Single Value Info if possible
FunctionPass *createIndirectCallConvLegacyPass(bool UseAndersen = false,
                                               bool UseDTrans = false);

// LoopOptMarker - Indicates loopopt based throttling to subsequent passes.
FunctionPass *createLoopOptMarkerLegacyPass();

// LoopCarriedCSE - This pass groups two Phi Nodes in a binary operation by a
// new Phi Node if their latch values have the same binary operation.
FunctionPass *createLoopCarriedCSEPass();

#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
// VPOParoptTpv - Supports the thread private legacy mode.
ModulePass *createVPOParoptTpvPass();
#endif // INTEL_COLLAB


//===----------------------------------------------------------------------===//
//
// CFGSimplification - Merge basic blocks, eliminate unreachable blocks,
// simplify terminator instructions, convert switches to lookup tables, etc.
//
FunctionPass *createCFGSimplificationPass(
    unsigned Threshold = 1, bool ForwardSwitchCond = false,
    bool ConvertSwitch = false, bool KeepLoops = true, bool SinkCommon = false,
    std::function<bool(const Function &)> Ftor = nullptr);

//===----------------------------------------------------------------------===//
//
// FlattenCFG - flatten CFG, reduce number of conditional branches by using
// parallel-and and parallel-or mode, etc...
//
FunctionPass *createFlattenCFGPass();

//===----------------------------------------------------------------------===//
//
// CFG Structurization - Remove irreducible control flow
//
///
/// When \p SkipUniformRegions is true the structizer will not structurize
/// regions that only contain uniform branches.
Pass *createStructurizeCFGPass(bool SkipUniformRegions = false);

//===----------------------------------------------------------------------===//
//
// TailCallElimination - This pass eliminates call instructions to the current
// function which occur immediately before return instructions.
#if INTEL_CUSTOMIZATION
// When skipRecProgression is TRUE, we skip functions with a recognized
// recursive progression through one of the arguments.
FunctionPass *createTailCallEliminationPass(bool skipRecProgression = false);
#endif // INTEL_CUSTOMIZATION

//===----------------------------------------------------------------------===//
//
// EarlyCSE - This pass performs a simple and fast CSE pass over the dominator
// tree.
//
FunctionPass *createEarlyCSEPass(bool UseMemorySSA = false);

//===----------------------------------------------------------------------===//
//
// GVNHoist - This pass performs a simple and fast GVN pass over the dominator
// tree to hoist common expressions from sibling branches.
//
FunctionPass *createGVNHoistPass(bool HoistingGeps = false); // INTEL

//===----------------------------------------------------------------------===//
//
// GVNSink - This pass uses an "inverted" value numbering to decide the
// similarity of expressions and sinks similar expressions into successors.
//
FunctionPass *createGVNSinkPass();

//===----------------------------------------------------------------------===//
//
// MergedLoadStoreMotion - This pass merges loads and stores in diamonds. Loads
// are hoisted into the header, while stores sink into the footer.
//
FunctionPass *createMergedLoadStoreMotionPass(bool SplitFooterBB = false);

//===----------------------------------------------------------------------===//
//
// GVN - This pass performs global value numbering and redundant load
// elimination cotemporaneously.
//
FunctionPass *createNewGVNPass();

//===----------------------------------------------------------------------===//
//
// DivRemPairs - Hoist/decompose integer division and remainder instructions.
//
FunctionPass *createDivRemPairsPass();

//===----------------------------------------------------------------------===//
//
// MemCpyOpt - This pass performs optimizations related to eliminating memcpy
// calls and/or combining multiple stores into memset's.
//
FunctionPass *createMemCpyOptPass();

//===----------------------------------------------------------------------===//
//
// LoopDeletion - This pass performs DCE of non-infinite loops that it
// can prove are dead.
//
Pass *createLoopDeletionPass();

//===----------------------------------------------------------------------===//
//
// ConstantHoisting - This pass prepares a function for expensive constants.
//
FunctionPass *createConstantHoistingPass();

//===----------------------------------------------------------------------===//
//
// Sink - Code Sinking
//
FunctionPass *createSinkingPass();

//===----------------------------------------------------------------------===//
//
// LowerAtomic - Lower atomic intrinsics to non-atomic form
//
Pass *createLowerAtomicPass();

//===----------------------------------------------------------------------===//
//
// LowerGuardIntrinsic - Lower guard intrinsics to normal control flow.
//
Pass *createLowerGuardIntrinsicPass();

//===----------------------------------------------------------------------===//
//
// LowerWidenableCondition - Lower widenable condition to i1 true.
//
Pass *createLowerWidenableConditionPass();

//===----------------------------------------------------------------------===//
//
// MergeICmps - Merge integer comparison chains into a memcmp
//
Pass *createMergeICmpsLegacyPass();

//===----------------------------------------------------------------------===//
//
// ValuePropagation - Propagate CFG-derived value information
//
Pass *createCorrelatedValuePropagationPass();

//===----------------------------------------------------------------------===//
//
// InferAddressSpaces - Modify users of addrspacecast instructions with values
// in the source address space if using the destination address space is slower
// on the target. If AddressSpace is left to its default value, it will be
// obtained from the TargetTransformInfo.
//
FunctionPass *createInferAddressSpacesPass(unsigned AddressSpace = ~0u);
extern char &InferAddressSpacesID;

//===----------------------------------------------------------------------===//
//
// LowerExpectIntrinsics - Removes llvm.expect intrinsics and creates
// "block_weights" metadata.
FunctionPass *createLowerExpectIntrinsicPass();

#if INTEL_CUSTOMIZATION
//===----------------------------------------------------------------------===//
//
// LowerSubscriptIntrinsics - Removes llvm.intel.subscript intrinsics.
FunctionPass *createLowerSubscriptIntrinsicLegacyPass();

//===----------------------------------------------------------------------===//
//
// ConvertGEPtoSubscriptIntrinsic - Converts getelementptr to
// llvm.intel.subscript intrinsics.
FunctionPass *createConvertGEPToSubscriptIntrinsicLegacyPass();
#endif // INTEL_CUSTOMIZATION

//===----------------------------------------------------------------------===//
//
// LowerConstantIntrinsicss - Expand any remaining llvm.objectsize and
// llvm.is.constant intrinsic calls, even for the unknown cases.
//
FunctionPass *createLowerConstantIntrinsicsPass();

//===----------------------------------------------------------------------===//
//
// PartiallyInlineLibCalls - Tries to inline the fast path of library
// calls such as sqrt.
//
FunctionPass *createPartiallyInlineLibCallsPass();

//===----------------------------------------------------------------------===//
//
// SeparateConstOffsetFromGEP - Split GEPs for better CSE
//
FunctionPass *createSeparateConstOffsetFromGEPPass(bool LowerGEP = false);

//===----------------------------------------------------------------------===//
//
// SpeculativeExecution - Aggressively hoist instructions to enable
// speculative execution on targets where branches are expensive.
//
FunctionPass *createSpeculativeExecutionPass();

// Same as createSpeculativeExecutionPass, but does nothing unless
// TargetTransformInfo::hasBranchDivergence() is true.
FunctionPass *createSpeculativeExecutionIfHasBranchDivergencePass();

//===----------------------------------------------------------------------===//
//
// StraightLineStrengthReduce - This pass strength-reduces some certain
// instruction patterns in straight-line code.
//
FunctionPass *createStraightLineStrengthReducePass();

//===----------------------------------------------------------------------===//
//
// PlaceSafepoints - Rewrite any IR calls to gc.statepoints and insert any
// safepoint polls (method entry, backedge) that might be required.  This pass
// does not generate explicit relocation sequences - that's handled by
// RewriteStatepointsForGC which can be run at an arbitrary point in the pass
// order following this pass.
//
FunctionPass *createPlaceSafepointsPass();

//===----------------------------------------------------------------------===//
//
// RewriteStatepointsForGC - Rewrite any gc.statepoints which do not yet have
// explicit relocations to include explicit relocations.
//
ModulePass *createRewriteStatepointsForGCLegacyPass();

//===----------------------------------------------------------------------===//
//
// Float2Int - Demote floats to ints where possible.
//
FunctionPass *createFloat2IntPass();

//===----------------------------------------------------------------------===//
//
// NaryReassociate - Simplify n-ary operations by reassociation.
//
FunctionPass *createNaryReassociatePass();

//===----------------------------------------------------------------------===//
//
// LoopDistribute - Distribute loops.
//
FunctionPass *createLoopDistributePass();

//===----------------------------------------------------------------------===//
//
// LoopFuse - Fuse loops.
//
FunctionPass *createLoopFusePass();

//===----------------------------------------------------------------------===//
//
// LoopLoadElimination - Perform loop-aware load elimination.
//
FunctionPass *createLoopLoadEliminationPass();

//===----------------------------------------------------------------------===//
//
// LoopVersioning - Perform loop multi-versioning.
//
FunctionPass *createLoopVersioningPass();

//===----------------------------------------------------------------------===//
//
// LoopDataPrefetch - Perform data prefetching in loops.
//
FunctionPass *createLoopDataPrefetchPass();

///===---------------------------------------------------------------------===//
ModulePass *createNameAnonGlobalPass();
ModulePass *createCanonicalizeAliasesPass();

//===----------------------------------------------------------------------===//
//
// LibCallsShrinkWrap - Shrink-wraps a call to function if the result is not
// used.
//
FunctionPass *createLibCallsShrinkWrapPass();

//===----------------------------------------------------------------------===//
//
// LoopSimplifyCFG - This pass performs basic CFG simplification on loops,
// primarily to help other loop passes.
//
Pass *createLoopSimplifyCFGPass();

//===----------------------------------------------------------------------===//
//
// WarnMissedTransformations - This pass emits warnings for leftover forced
// transformations.
//
Pass *createWarnMissedTransformationsPass();

#if INTEL_CUSTOMIZATION
//===----------------------------------------------------------------------===//
//
// IVSplit - This pass performs IV live range split on nesting inner loops
//
FunctionPass *createIVSplitLegacyPass();
#endif // INTEL_CUSTOMIZATION
} // End llvm namespace

#endif

//===- llvm/Transforms/IPO.h - Interprocedural Transformations --*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021-2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This header file defines prototypes for accessor functions that expose passes
// in the IPO transformations library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_H
#define LLVM_TRANSFORMS_IPO_H

#include "llvm/ADT/SmallVector.h"
#include <functional>
#include <vector>

namespace llvm {

struct InlineParams;
class ModulePass;
class Pass;
class BasicBlock;
class GlobalValue;
class raw_ostream;
class InlineReportBuilder; // INTEL

//===----------------------------------------------------------------------===//
//
// This pass adds !annotation metadata to entries in the
// @llvm.global.annotations global constant.
//
ModulePass *createAnnotation2MetadataLegacyPass();

//===----------------------------------------------------------------------===//
//
// These functions removes symbols from functions and modules.  If OnlyDebugInfo
// is true, only debugging information is removed from the module.
//
ModulePass *createStripSymbolsPass(bool OnlyDebugInfo = false);

//===----------------------------------------------------------------------===//
//
// These functions strips symbols from functions and modules.
// Only debugging information is not stripped.
//
ModulePass *createStripNonDebugSymbolsPass();

//===----------------------------------------------------------------------===//
//
// This pass removes llvm.dbg.declare intrinsics.
ModulePass *createStripDebugDeclarePass();

//===----------------------------------------------------------------------===//
//
// This pass removes unused symbols' debug info.
ModulePass *createStripDeadDebugInfoPass();

//===----------------------------------------------------------------------===//
/// createConstantMergePass - This function returns a new pass that merges
/// duplicate global constants together into a single constant that is shared.
/// This is useful because some passes (ie TraceValues) insert a lot of string
/// constants into the program, regardless of whether or not they duplicate an
/// existing string.
///
ModulePass *createConstantMergePass();

//===----------------------------------------------------------------------===//
/// createGlobalOptimizerPass - This function returns a new pass that optimizes
/// non-address taken internal globals.
///
ModulePass *createGlobalOptimizerPass();

//===----------------------------------------------------------------------===//
/// createGlobalDCEPass - This transform is designed to eliminate unreachable
/// internal globals (functions or global variables)
///
ModulePass *createGlobalDCEPass();

//===----------------------------------------------------------------------===//
/// This transform is designed to eliminate available external globals
/// (functions or global variables)
///
ModulePass *createEliminateAvailableExternallyPass();

//===----------------------------------------------------------------------===//
/// createGVExtractionPass - If deleteFn is true, this pass deletes
/// the specified global values. Otherwise, it deletes as much of the module as
/// possible, except for the global values specified. If keepConstInit is true,
/// the initializers of global constants are not deleted even if they are
/// unused.
///
ModulePass *createGVExtractionPass(std::vector<GlobalValue*>& GVs, bool
                                  deleteFn = false, bool keepConstInit = false);

//===----------------------------------------------------------------------===//
/// createFunctionInliningPass - Return a new pass object that uses a heuristic
/// to inline direct function calls to small functions.
///
/// The Threshold can be passed directly, or asked to be computed from the
/// given optimization and size optimization arguments.
///
/// The -inline-threshold command line option takes precedence over the
/// threshold given here.
Pass *createFunctionInliningPass();
Pass *createFunctionInliningPass(int Threshold);
Pass *createFunctionInliningPass(unsigned OptLevel, unsigned SizeOptLevel,
#if INTEL_CUSTOMIZATION
                                 bool DisableInlineHotCallSite,
                                 bool PrepareForLTO = false,
                                 bool LinkForLTO = false,
                                 bool SYCLOptimizationMode = false);
#endif // INTEL_CUSTOMIZATION
Pass *createFunctionInliningPass(InlineParams &Params);

//===----------------------------------------------------------------------===//
/// createPruneEHPass - Return a new pass object which transforms invoke
/// instructions into calls, if the callee can _not_ unwind the stack.
///
Pass *createPruneEHPass();

//===----------------------------------------------------------------------===//
/// createInternalizePass - This pass loops over all of the functions in the
/// input module, internalizing all globals (functions and variables) it can.
////
/// Before internalizing a symbol, the callback \p MustPreserveGV is invoked and
/// gives to the client the ability to prevent internalizing specific symbols.
///
/// The symbol in DSOList are internalized if it is safe to drop them from
/// the symbol table.
///
/// Note that commandline options that are used with the above function are not
/// used now!
ModulePass *
createInternalizePass(std::function<bool(const GlobalValue &)> MustPreserveGV);

/// createInternalizePass - Same as above, but with an empty exportList.
ModulePass *createInternalizePass();

//===----------------------------------------------------------------------===//
/// createDeadArgEliminationPass - This pass removes arguments from functions
/// which are not used by the body of the function.
///
ModulePass *createDeadArgEliminationPass();

/// DeadArgHacking pass - Same as DAE, but delete arguments of external
/// functions as well.  This is definitely not safe, and should only be used by
/// bugpoint.
ModulePass *createDeadArgHackingPass();

/// DeadArgumentElimination pass for SYCL kernel functions
ModulePass *createDeadArgEliminationSYCLPass();

//===----------------------------------------------------------------------===//
/// createOpenMPOptLegacyPass - OpenMP specific optimizations.
Pass *createOpenMPOptCGSCCLegacyPass();

//===----------------------------------------------------------------------===//
/// createIPSCCPPass - This pass propagates constants from call sites into the
/// bodies of functions, and keeps track of whether basic blocks are executable
/// in the process.
///
ModulePass *createIPSCCPPass();

//===----------------------------------------------------------------------===//
/// createFunctionSpecializationPass - This pass propagates constants from call
/// sites to the specialized version of the callee function.
ModulePass *createFunctionSpecializationPass();

//===----------------------------------------------------------------------===//
//
/// createLoopExtractorPass - This pass extracts all natural loops from the
/// program into a function if it can.
///
Pass *createLoopExtractorPass();

/// createSingleLoopExtractorPass - This pass extracts one natural loop from the
/// program into a function if it can.  This is used by bugpoint.
///
Pass *createSingleLoopExtractorPass();

/// createBlockExtractorPass - This pass extracts all the specified blocks
/// from the functions in the module.
///
ModulePass *createBlockExtractorPass();
ModulePass *
createBlockExtractorPass(const SmallVectorImpl<BasicBlock *> &BlocksToExtract,
                         bool EraseFunctions);
ModulePass *
createBlockExtractorPass(const SmallVectorImpl<SmallVector<BasicBlock *, 16>>
                             &GroupsOfBlocksToExtract,
                         bool EraseFunctions);

/// createStripDeadPrototypesPass - This pass removes any function declarations
/// (prototypes) that are not used.
ModulePass *createStripDeadPrototypesPass();

//===----------------------------------------------------------------------===//
/// createReversePostOrderFunctionAttrsPass - This pass walks SCCs of the call
/// graph in RPO to deduce and propagate function attributes. Currently it
/// only handles synthesizing norecurse attributes.
///
Pass *createReversePostOrderFunctionAttrsPass();

//===----------------------------------------------------------------------===//
/// createMergeFunctionsPass - This pass discovers identical functions and
/// collapses them.
///
ModulePass *createMergeFunctionsPass();

//===----------------------------------------------------------------------===//
/// createHotColdSplittingPass - This pass outlines cold blocks into a separate
/// function(s).
ModulePass *createHotColdSplittingPass();

//===----------------------------------------------------------------------===//
/// createIROutlinerPass - This pass finds similar code regions and factors
/// those regions out into functions.
ModulePass *createIROutlinerPass();

//===----------------------------------------------------------------------===//
/// createPartialInliningPass - This pass inlines parts of functions.
///
#if INTEL_CUSTOMIZATION
ModulePass *createPartialInliningPass(bool RunLTOPartialInline = false,
                                      bool EnableSpecialCases = false);
#endif // INTEL_CUSTOMIZATION

//===----------------------------------------------------------------------===//
/// createBarrierNoopPass - This pass is purely a module pass barrier in a pass
/// manager.
ModulePass *createBarrierNoopPass();

/// createCalledValuePropagationPass - Attach metadata to indirct call sites
/// indicating the set of functions they may target at run-time.
ModulePass *createCalledValuePropagationPass();

/// What to do with the summary when running passes that operate on it.
enum class PassSummaryAction {
  None,   ///< Do nothing.
  Import, ///< Import information from summary.
  Export, ///< Export information to summary.
};

/// This pass export CFI checks for use by external modules.
ModulePass *createCrossDSOCFIPass();

/// This pass splits globals into pieces for the benefit of whole-program
/// devirtualization and control-flow integrity.
ModulePass *createGlobalSplitPass();

<<<<<<< HEAD
#if INTEL_CUSTOMIZATION
/// \brief This pass enables more functions to be converted to use the 'fastcc'
/// calling convention.
ModulePass *createIntelAdvancedFastCallWrapperPass();

#if INTEL_FEATURE_SW_ADVANCED
/// \brief This pass conducts IPO-based prefetching
ModulePass *createIntelIPOPrefetchWrapperPass();
#endif // INTEL_FEATURE_SW_ADVANCED

/// \brief This pass implements a constant propagation in those places where
/// the memory alignment is being computed.
ModulePass *createIntelArgumentAlignmentLegacyPass();

/// \brief This pass implements a simplified dead argument elimination with
/// IPO analysis. The goal is to eliminate an argument if it initializes data
/// that won't be use across multiple functions. The actual value will be
/// removed too.
ModulePass *createIntelIPODeadArgEliminationWrapperPass();

#if INTEL_FEATURE_SW_DTRANS
/// \brief This pass folds the intrinsic llvm.intel.wholeprogramsafe using the
/// results from the whole program analysis.
ModulePass *createIntelFoldWPIntrinsicLegacyPass();
#endif // INTEL_FEATURE_SW_DTRANS

/// \brief This pass will add the declarations for math functions that are
/// expressed as llvm intrinsics.
ModulePass *createIntelMathLibrariesDeclarationWrapperPass();

#if INTEL_FEATURE_SW_ADVANCED
/// \brief This pass implements a simple partial inlining for small functions.
/// This partial inliner will take care of small functions that the compiler
/// will like to fully inline. The difference between this partial inliner and
/// the traditional partial inliner is that this pass won't do inlining, just
/// create a new function that will call the original function and set the
/// the inlining attributes. The inliner will read the attributes and inline
/// the new function that calls the original, creating a partial inline
/// behavior. The traditional partial inliner will actually do inlining.
ModulePass *createIntelPartialInlineLegacyPass();
#endif // INTEL_FEATURE_SW_ADVANCED

/// \brief This pass conducts IPO-based Array Transpose.
ModulePass *createIPArrayTransposeLegacyPass();

#if INTEL_FEATURE_SW_ADVANCED
/// \brief This pass conducts IPO-based Predicate optimization.
ModulePass *createIPPredOptLegacyPass();
/// \brief This pass implements IP Cloning
ModulePass *createIPCloningLegacyPass(bool AfterInl = false,
                                      bool IfSwitchHeuristic = false);
#endif // INTEL_FEATURE_SW_ADVANCED

/// \brief This pass parses -[no]inline-list option and assigns corresponding
/// attributes to callsites (for experimental purposes).
ModulePass *createInlineListsPass();
ModulePass *createInlineReportSetupPass(InlineReportBuilder *IRB = nullptr);
ModulePass *createInlineReportEmitterPass(unsigned OptLevel = 0,
                                          unsigned SizeLevel = 0,
                                          bool PrepareForLTO = false);

/// \brief This pass implements optimization of dynamic_cast calls depending on
/// the classes hierarchy.
ModulePass *createOptimizeDynamicCastsPass();

/// \brief This pass implements IP Cloning of call trees based on constant
/// parameter set propagation and folding.
ModulePass* createCallTreeCloningPass();

/// \brief This pass implements DopeVectorConstProp (propagation of constant
/// lower bounds, strides, and extents of dope vectors whose pointers are
/// formal parameters).
ModulePass *createDopeVectorConstPropLegacyPass(void);

#if INTEL_FEATURE_SW_ADVANCED
/// \brief This pass will attempt to recognize each Function as a "qsort".
/// For those it recognizes as such, it will add the Function attribute
/// "is-qsort".
ModulePass *createQsortRecognizerLegacyPass(void);
#endif // INTEL_FEATURE_SW_ADVANCED

/// \brief This pass will mark callsites that should be aggressively
/// inlined with the "prefer-inline-aggressive" attribute.
ModulePass *createAggInlinerLegacyPass(void);

#if INTEL_FEATURE_SW_ADVANCED
/// \brief This pass eliminates dead array element operations.
ModulePass *createDeadArrayOpsEliminationLegacyPass(void);

/// \brief This pass multiversions for tiling and marks tiled functions for
/// inlining.
ModulePass *createTileMVInlMarkerLegacyPass(void);
#endif // INTEL_FEATURE_SW_ADVANCED

/// This pass adds noalias attribute to function arguments where it is safe
/// to do so.
ModulePass *createArgNoAliasPropPass(void);

ModulePass *createIntelVTableFixupPass(void);
#endif // INTEL_CUSTOMIZATION

/// Write ThinLTO-ready bitcode to Str.
ModulePass *createWriteThinLTOBitcodePass(raw_ostream &Str,
                                          raw_ostream *ThinLinkOS = nullptr);

=======
>>>>>>> 633f5663c37a670e28040cadd938200abd854483
} // End llvm namespace

#endif

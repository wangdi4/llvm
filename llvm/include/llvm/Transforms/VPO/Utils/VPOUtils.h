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
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include <unordered_map>

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
class Constant;
class LLVMContext;

namespace vpo {

/// \brief This class contains a set of utility functions used by VPO passes.
class VPOUtils {

private:

    /// \brief Private utility function used to encode a StringRef as a
    /// Metadata Value. This Value is then used by the createDirective*
    /// functions as the parameter representing the type of directive.
    static Value* createMetadataAsValueFromString(Module &M, StringRef Str);

public:
    /// Constructor and destructor
    VPOUtils() {}
    ~VPOUtils() {}

    enum OMP_DIRECTIVES {
      DIR_OMP_PARALLEL = 0,
      DIR_OMP_END_PARALLEL,
      DIR_OMP_PARALLEL_LOOP,
      DIR_OMP_END_PARALLEL_LOOP,
      DIR_OMP_LOOP_SIMD,
      DIR_OMP_END_LOOP_SIMD,
      DIR_OMP_PARALLEL_LOOP_SIMD,
      DIR_OMP_END_PARALLEL_LOOP_SIMD,
      DIR_OMP_SECTIONS,
      DIR_OMP_END_SECTIONS,
      DIR_OMP_PARALLEL_SECTIONS,
      DIR_OMP_END_PARALLEL_SECTIONS,
      DIR_OMP_WORKSHARE,
      DIR_OMP_END_WORKSHARE,
      DIR_OMP_PARALLEL_WORKSHARE,
      DIR_OMP_END_PARALLEL_WORKSHARE,
      DIR_OMP_SECTION,
      DIR_OMP_SINGLE,
      DIR_OMP_END_SINGLE,
      DIR_OMP_TASK,
      DIR_OMP_END_TASK,
      DIR_OMP_MASTER,
      DIR_OMP_END_MASTER,
      DIR_OMP_CRITICAL,
      DIR_OMP_END_CRITICAL,
      DIR_OMP_BARRIER,
      DIR_OMP_TASKWAIT,
      DIR_OMP_TASKYIELD,
      DIR_OMP_ATOMIC,
      DIR_OMP_END_ATOMIC,
      DIR_OMP_FLUSH,
      DIR_OMP_ORDERED,
      DIR_OMP_END_ORDERED,
      DIR_OMP_SIMD,
      DIR_OMP_END_SIMD,
      DIR_OMP_TASKLOOP,
      DIR_OMP_END_TASKLOOP,
      DIR_OMP_TASKLOOP_SIMD,
      DIR_OMP_END_TASKLOOP_SIMD,
      DIR_OMP_TARGET,
      DIR_OMP_END_TARGET,
      DIR_OMP_TARGET_DATA,
      DIR_OMP_END_TARGET_DATA,
      DIR_OMP_TARGET_UPDATE,
      DIR_OMP_TEAMS,
      DIR_OMP_END_TEAMS,
      DIR_OMP_TEAMS_DISTRIBUTE,
      DIR_OMP_END_TEAMS_DISTRIBUTE,
      DIR_OMP_TEAMS_SIMD,
      DIR_OMP_END_TEAMS_SIMD,
      DIR_OMP_TEAMS_DISTRIBUTE_SIMD,
      DIR_OMP_END_TEAMS_DISTRIBUTE_SIMD,
      DIR_OMP_DISTRIBUTE,
      DIR_OMP_END_DISTRIBUTE,
      DIR_OMP_DISTRIBUTE_PARLOOP,
      DIR_OMP_END_DISTRIBUTE_PARLOOP,
      DIR_OMP_DISTRIBUTE_SIMD,
      DIR_OMP_END_DISTRIBUTE_SIMD,
      DIR_OMP_DISTRIBUTE_PARLOOP_SIMD,
      DIR_OMP_END_DISTRIBUTE_PARLOOP_SIMD,
      DIR_OMP_TARGET_ENTER_DATA,
      DIR_OMP_TARGET_EXIT_DATA,
      DIR_OMP_TARGET_TEAMS,
      DIR_OMP_END_TARGET_TEAMS,
      DIR_OMP_TEAMS_DISTRIBUTE_PARLOOP,
      DIR_OMP_END_TEAMS_DISTRIBUTE_PARLOOP,
      DIR_OMP_TEAMS_DISTRIBUTE_PARLOOP_SIMD,
      DIR_OMP_END_TEAMS_DISTRIBUTE_PARLOOP_SIMD,
      DIR_OMP_TARGET_TEAMS_DISTRIBUTE,
      DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE,
      DIR_OMP_TARGET_TEAMS_DISTRIBUTE_PARLOOP,
      DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARLOOP,
      DIR_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD,
      DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_SIMD,
      DIR_OMP_TARGET_TEAMS_DISTRIBUTE_PARLOOP_SIMD,
      DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARLOOP_SIMD,
      DIR_OMP_CANCEL,
      DIR_OMP_CANCELLATION_POINT,
      DIR_QUAL_LIST_END,
    };

    enum OMP_CLAUSES {
      QUAL_OMP_DEFAULT_PRIVATE = 0,
      QUAL_OMP_DEFAULT_FIRSTPRIVATE,
      QUAL_OMP_NOWAIT,
      QUAL_OMP_UNTIED,
      QUAL_OMP_READ,
      QUAL_OMP_READ_SEQ_CST,
      QUAL_OMP_WRITE,
      QUAL_OMP_WRITE_SEQ_CST,
      QUAL_OMP_UPDATE,
      QUAL_OMP_UPDATE_SEQ_CST,
      QUAL_OMP_CAPTURE,
      QUAL_OMP_CAPTURE_SEQ_CST,
      QUAL_OMP_MERGEABLE,
      QUAL_OMP_NOGROUP,
      QUAL_OMP_PRIVATE,
      QUAL_OMP_FIRSTPRIVATE,
      QUAL_OMP_LASTPRIVATE,
      QUAL_OMP_SHARED,
      QUAL_OMP_COPYIN,
      QUAL_OMP_COPYPRIVATE,
      QUAL_OMP_TO,
      QUAL_OMP_FROM,
      QUAL_OMP_LINEAR,
      QUAL_OMP_ALIGNED,
      QUAL_OMP_FLUSH,
      QUAL_OMP_THREADPRIVATE,
      QUAL_OMP_IF,
      QUAL_OMP_NUM_THREADS,
      QUAL_OMP_FINAL,
      QUAL_OMP_COLLAPSE,
      QUAL_OMP_ORDERED,
      QUAL_OMP_SAFELEN,
      QUAL_OMP_SIMDLEN,
      QUAL_OMP_GRAINSIZE,
      QUAL_OMP_NUM_TASKS,
      QUAL_OMP_THREAD_LIMIT,
      QUAL_OMP_REDUCTION_ADD,
      QUAL_OMP_REDUCTION_SUB,
      QUAL_OMP_REDUCTION_MUL,
      QUAL_OMP_REDUCTION_AND,
      QUAL_OMP_REDUCTION_OR,
      QUAL_OMP_REDUCTION_XOR,
      QUAL_OMP_REDUCTION_BAND,
      QUAL_OMP_REDUCTION_BOR,
      QUAL_OMP_REDUCTION_UDR,
      QUAL_OMP_MAP_TO,
      QUAL_OMP_MAP_FROM,
      QUAL_OMP_MAP_TOFROM,
      QUAL_OMP_DEPEND_IN,
      QUAL_OMP_DEPEND_OUT,
      QUAL_OMP_DEPEND_INOUT,
      QUAL_OMP_DIST_SCHEDULE_STATIC,
      QUAL_OMP_SCHEDULE_STATIC,
      QUAL_OMP_SCHEDULE_DYNAMIC,
      QUAL_OMP_SCHEDULE_GUIDED,
      QUAL_OMP_SCHEDULE_AUTO,
      QUAL_OMP_SCHEDULE_RUNTIME,
      QUAL_OMP_NAME,
      QUAL_LIST_END,
    };

    // Map OMP_DIRECTIVES to StringRefs
    static std::unordered_map<int, StringRef> DirectiveStrings;

    // Map OMP_CLAUSES to StringRefs
    static std::unordered_map<int, StringRef> ClauseStrings;

    /// \brief This function restructures the CFG on demand, where each
    /// directive for Cilk, OpenMP, Offload, Vectorization is put into a
    /// standalone basic block. This is a pre-required process for WRegion
    /// construction for each function. 
    ///
    /// Note that, since WRegion construction requires
    /// DominatorTreeWrapperPass and LoopInfoWrapperPass to be executed prior
    /// to it, when calling CFGRestructuring, we need to update DominatorTree
    /// and LoopInfo whenever a basic block splitting happens.
    static void CFGRestructuring(Function &F, DominatorTree *DT = nullptr,
                     LoopInfo *LI = nullptr);

    /// \brief Return the string representation of the metadata argument used
    /// within a call to the llvm.intel.directive intrinsic. Currently, this
    /// function only supports calls to llvm.intel.directive. Future support
    /// will be added for llvm.intel.directive.qual,
    /// llvm.intel.directive.qual.opnd and llvm.intel.directive.qual.opndlist.
    static StringRef getDirectiveMetadataString(CallInst *Call);

    /// \brief Return a call to the llvm.intel.directive intrinsic.
    static CallInst* createDirectiveCall(Module &M, StringRef DirectiveStr);

    /// \brief Return a call to the llvm.intel.directive.qual intrinsic.
    static CallInst* createDirectiveQualCall(Module &M, StringRef DirectiveStr);

    /// \brief Return a call to the llvm.intel.directive.qual.opnd intrinsic.
    static CallInst* createDirectiveQualOpndCall(Module &M,
                                                 StringRef DirectiveStr,
                                                 Value *Val);

    /// \brief Return a call to the llvm.intel.directive.qual.opndlist
    /// intrinsic.
    static CallInst* createDirectiveQualOpndListCall(
        Module &M,
        StringRef DirectiveStr,
        SmallVector<Value*, 4>& VarCallArgs);

    /// \brief Returns strings corresponding to OpenMP directives.
    static StringRef getDirectiveString(int Id);

    /// \brief Returns strings corresponding to OpenMP clauses.
    static StringRef getClauseString(int Id);

    /// \brief Removes calls to directive intrinsics.
    static void stripDirectives(Function &F);

    /// \brief Returns a floating point or integer constant depending on Ty.
    template <typename T>
    static Constant* getConstantValue(Type *Ty, LLVMContext &Context, T Val);

    /// \brief Returns Loop in LoopInfo corresponding to the WRN's EntryBB
    static Loop* getLoopFromLoopInfo(LoopInfo* LI, BasicBlock *WRNEntryBB);
};

} // End vpo namespace

} // End llvm namespace
#endif

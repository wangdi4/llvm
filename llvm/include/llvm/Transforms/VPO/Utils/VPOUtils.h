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
class IntrinsicInst;
class Constant;
class LLVMContext;

namespace vpo {
typedef SmallVector<BasicBlock *, 32> VPOSmallVectorBB;

typedef enum OMP_DIRECTIVES {
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
      // DIR_OMP_THREADPRIVATE,  // we should need this too
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
      DIR_OMP_END_TARGET_UPDATE,
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
      DIR_QUAL_LIST_END              // must be last; marks end of DIR
} OMP_DIRECTIVES;

typedef enum OMP_CLAUSES {

  // Clauses with no argument (ie, "DirQual")
  
      QUAL_OMP_DEFAULT_NONE=0,
      QUAL_OMP_DEFAULT_SHARED,
      QUAL_OMP_DEFAULT_PRIVATE,
      QUAL_OMP_DEFAULT_FIRSTPRIVATE,
      QUAL_OMP_NOWAIT,
      QUAL_OMP_MERGEABLE,
      QUAL_OMP_NOGROUP,
      QUAL_OMP_UNTIED,
      QUAL_OMP_READ,
      QUAL_OMP_READ_SEQ_CST,
      QUAL_OMP_WRITE,
      QUAL_OMP_WRITE_SEQ_CST,
      QUAL_OMP_UPDATE,
      QUAL_OMP_UPDATE_SEQ_CST,
      QUAL_OMP_CAPTURE,
      QUAL_OMP_CAPTURE_SEQ_CST,
      QUAL_OMP_SCHEDULE_AUTO,
      QUAL_OMP_SCHEDULE_RUNTIME,
      QUAL_OMP_PROC_BIND_MASTER,
      QUAL_OMP_PROC_BIND_CLOSE,
      QUAL_OMP_PROC_BIND_SPREAD,

  // Clauses with one argument (ie, "DirQualOpnd")
 
      QUAL_OMP_IF,
      QUAL_OMP_COLLAPSE,
      QUAL_OMP_NUM_THREADS,
      QUAL_OMP_ORDERED,
      QUAL_OMP_SAFELEN,
      QUAL_OMP_SIMDLEN,
      QUAL_OMP_FINAL,
      QUAL_OMP_GRAINSIZE,
      QUAL_OMP_NUM_TASKS,
      QUAL_OMP_PRIORITY,
      QUAL_OMP_NUM_TEAMS,
      QUAL_OMP_THREAD_LIMIT,
      QUAL_OMP_DIST_SCHEDULE_STATIC,
      QUAL_OMP_SCHEDULE_STATIC,      // default chunk_size=0 (static even)
      QUAL_OMP_SCHEDULE_DYNAMIC,     // default chunk_size=1
      QUAL_OMP_SCHEDULE_GUIDED,      // default chunk_size=1
  
  // Clauses with list-type arguments (ie, "DirQualOpndList")

      QUAL_OMP_SHARED,
      QUAL_OMP_PRIVATE,
      QUAL_OMP_FIRSTPRIVATE,
      QUAL_OMP_LASTPRIVATE,
      QUAL_OMP_COPYIN,
      QUAL_OMP_COPYPRIVATE,
      QUAL_OMP_REDUCTION_ADD,   // must be first REDUCTION op in enum
      QUAL_OMP_REDUCTION_SUB,
      QUAL_OMP_REDUCTION_MUL,
      QUAL_OMP_REDUCTION_AND,
      QUAL_OMP_REDUCTION_OR,
      QUAL_OMP_REDUCTION_XOR,
      QUAL_OMP_REDUCTION_BAND,
      QUAL_OMP_REDUCTION_BOR,
      QUAL_OMP_REDUCTION_UDR,   // must be last REDUCTION op in enum
      QUAL_OMP_TO,
      QUAL_OMP_FROM,
      QUAL_OMP_LINEAR,
      QUAL_OMP_ALIGNED,
      QUAL_OMP_FLUSH,
      QUAL_OMP_THREADPRIVATE,
      QUAL_OMP_MAP_TO,
      QUAL_OMP_MAP_FROM,
      QUAL_OMP_MAP_TOFROM,
      QUAL_OMP_DEPEND_IN,
      QUAL_OMP_DEPEND_OUT,
      QUAL_OMP_DEPEND_INOUT,
      QUAL_OMP_NAME,            // for critical section name (a string)
      QUAL_LIST_END             // must be last (dummy placeholder only)
} OMP_CLAUSES;



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

    // Map OMP_DIRECTIVES to StringRefs
    static std::unordered_map<int, StringRef> DirectiveStrings;

    // Map OMP_CLAUSES to StringRefs
    static std::unordered_map<int, StringRef> ClauseStrings;

    // Map StringRefs to OMP_DIRECTIVES
    static StringMap<int> DirectiveIDs;

    // Map StringRefs to OMP_CLAUSES
    static StringMap<int> ClauseIDs;

    /// \brief Initialize maps of directive & clause string to ID.
    /// This routine must be invoked (once) before calling query
    /// routines such as getDirectiveID() and getClauseID().
    static void initDirectiveAndClauseStringMap();

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
  
    /// \brief Return true if the intrinsic Id is intel_directive.
    static bool isIntelDirective(Intrinsic::ID Id);

    /// \brief Return true if the intrinsic Id corresponds to a clause:
    ///    intel_directive_qual,
    ///    intel_directive_qual_opnd, or 
    ///    intel_directive_qual_opndlist.
    static bool isIntelClause(Intrinsic::ID Id);

    /// \brief Return true if the intrinsic Id corresponds to an
    /// Intel directive or clause.
    static bool isIntelDirectiveOrClause(Intrinsic::ID Id);

    /// \brief Return the string representation of the metadata argument used
    /// within a call to one of these intrinsics: 
    ///    llvm.intel.directive, 
    ///    llvm.intel.directive.qual, 
    ///    llvm.intel.directive.qual.opnd, and 
    ///    llvm.intel.directive.qual.opndlist.
    static StringRef getDirectiveMetadataString(IntrinsicInst *Call);

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

    /// \brief Similar to getDirectiveString(), 
    /// but strips out the leading "DIR_OMP_" prefix substring
    static StringRef getDirectiveName(int Id);

    /// \brief Similar to getClauseString(), 
    /// but strips out the leading "QUAL_OMP_" prefix substring
    static StringRef getClauseName(int Id);

    /// \brief Returns true if the string corresponds to an OpenMP directive.
    static bool isOpenMPDirective(StringRef DirFullName);

    /// \brief Returns true if the string corresponds to an OpenMP clause.
    static bool isOpenMPClause(StringRef ClauseFullName);

    /// \brief Returns the ID (enum) corresponding to OpenMP directives.
    static int getDirectiveID(StringRef DirFullName);

    /// \brief Returns the ID (enum) corresponding to OpenMP clauses.
    static int getClauseID(StringRef ClauseFullName);

    /// \brief Removes calls to directive intrinsics.
    static void stripDirectives(Function &F);

    /// \brief Returns a floating point or integer constant depending on Ty.
    template <typename T>
    static Constant* getConstantValue(Type *Ty, LLVMContext &Context, T Val);

    /// \brief Returns Loop in LoopInfo corresponding to the WRN's EntryBB
    static Loop* getLoopFromLoopInfo(LoopInfo* LI, BasicBlock *WRNEntryBB);

    /// Utilities to handle directives & clauses 

    /// \brief Return true iff DirString corresponds to a directive that
    /// begins a region (eg, DIR_OMP_PARALLEL, DIR_OMP_SIMD, etc.
    static bool isBeginDirective(StringRef DirString);
    static bool isBeginDirective(int DirID);

    /// \brief Return true iff DirString corresponds to a directive that
    /// ends a region (eg, DIR_OMP_END_PARALLEL, DIR_OMP_END_SIMD, etc.
    static bool isEndDirective(StringRef DirString);
    static bool isEndDirective(int DirID);

    /// \brief Return true iff DirString corresponds to a directive that
    /// begins or ends a region
    static bool isBeginOrEndDirective(StringRef DirString);
    static bool isBeginOrEndDirective(int DirID);

    /// \brief Return true iff DirString corresponds to a stand-alone 
    /// directive (doesn't begin or end a region). Eg: DIR_OMP_FLUSH
    static bool isSoloDirective(StringRef DirString);
    static bool isSoloDirective(int DirID);

    /// \brief Return true iff DirString corresponds to DIR_QUAL_LIST_END,
    /// the mandatory marker to end a directive
    static bool isListEndDirective(StringRef DirString);
    static bool isListEndDirective(int DirID);

    /// \brief Generates BB set in sub CFG for a given WRegionNode.
    /// The entry basic bblock 'EntryBB' and the exit basic
    /// block 'ExitBB' are the inputs, and 'BBSet' is the output containing all
    /// the basic blocks that belong to this region. It guarantees that the
    /// first item in BBSet is 'EntryBB' and the last item is 'ExitBB'. 
    static VPOSmallVectorBB* collectBBSet(
            BasicBlock *EntryBB, BasicBlock *ExitBB);

};

} // End vpo namespace

} // End llvm namespace
#endif


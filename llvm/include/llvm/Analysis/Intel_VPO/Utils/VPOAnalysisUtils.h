//===-- VPOAnalysisUtils.h - Class definitions for VPO utilites -*- C++ -*-===//
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
/// This file defines the VPOAnalysisUtils class and provides a set of common
/// utilities shared primarily by the various components of VPO Analysis 
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_ANALYSIS_VPO_UTILS_VPOANALYSISUTILS_H
#define LLVM_ANALYSIS_VPO_UTILS_VPOANALYSISUTILS_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Analysis/Intel_Directives.h"
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
typedef SmallVector<Instruction *, 32> VPOSmallVectorInst;

/// \brief This class contains a set of utility functions used by VPO passes.
class VPOAnalysisUtils {

public:
    /// Constructor and destructor
    VPOAnalysisUtils() {}
    ~VPOAnalysisUtils() {}

    /// \brief Initialize maps of directive & clause string to ID.
    /// This routine must be invoked (once) before calling query
    /// routines such as getDirectiveID() and getClauseID().
    static void initDirectiveAndClauseStringMap();

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

    /// \brief Return the string representation of the modifier metadata 
    /// argument used in an llvm.intel.directive.qual.opndlist intrinsic
    /// that represents a schedule clause
    static StringRef getScheduleModifierMDString(IntrinsicInst *Call);

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
    static bool isStandAloneDirective(StringRef DirString);
    static bool isStandAloneDirective(int DirID);

    /// \brief Return true iff DirString corresponds to DIR_QUAL_LIST_END,
    /// the mandatory marker to end a directive
    static bool isListEndDirective(StringRef DirString);
    static bool isListEndDirective(int DirID);

    /// \brief Return true iff the ClauseID represents a REDUCTION clause,
    /// such as QUAL_OMP_REDUCTION_ADD
    static bool isReductionClause(int ClauseID);

    /// \brief Return true iff the ClauseID represents a SCHEDULE or a
    /// DIST_SCHEDULE clause
    static bool isScheduleClause(int ClauseID);

    /// \brief True for MAP, TO, or FROM clauses
    static bool isMapClause(int ClauseID);
};

} // End vpo namespace

} // End llvm namespace
#endif

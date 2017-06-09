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

/// \brief ClauseSpecifier holds information for an OMP clause name. Fullname
/// is the string obtained directly from the intrinsic. Its format is:
///     FullName = BaseName[:Modifier]
/// Most clauses don't need a the modifier so usually FullName == BaseName
/// and Modifier is empty. 
//
/// The Modifier is used in the following cases:
///
/// 1. Schedule modifiers. Example:
///      FullName = "QUAL.OMP.SCHEDULE.STATIC:SIMD.MONOTONIC"
///      BaseName = "QUAL.OMP.SCHEDULE.STATIC"
///      Modifier = "SIMD.MONOTONIC"
///      Id = QUAL_OMP_SCHEDULE_STATIC
///
/// 2. Array sections. Example:
///      FullName = "QUAL.OMP.REDUCTION.ADD:ARRSECT"
///      BaseName = "QUAL.OMP.REDUCTION.ADD"
///      Modifier = "ARRSECT"
///      Id = QUAL_OMP_REDUCTION_ADD
///      
/// 3. NonPOD operands. Example:
///      FullName = "QUAL.OMP.PRIVATE:NONPOD"
///      BaseName = "QUAL.OMP.PRIVATE"
///      Modifier = "NONPOD"
///      Id = QUAL_OMP_PRIVATE
/// 
/// Id is the enum corresponding to BaseName.
class ClauseSpecifier {
private:
  StringRef FullName;
  StringRef BaseName;
  StringRef Modifier; 
  int  Id;

  // These properties are extracted from the Modifier substring

  // Data properties
  bool IsArraySection:1;
  bool IsNonPod:1;
  bool IsUnsigned:1;     // needed by min/max reduction

  // Conditional lastprivate
  bool IsConditional:1;

  // Schedule modifiers
  bool IsScheduleMonotonic:1;
  bool IsScheduleNonmonotonic:1;
  bool IsScheduleSimd:1;

public:

  // Constructor
  ClauseSpecifier(StringRef Name);

  // Setters
  void setFullName(StringRef S) { FullName = S; }
  void setBaseName(StringRef S) { BaseName = S; }
  void setModifier(StringRef S) { Modifier = S; }
  void setId(int N) { Id = N; }
  void setIsArraySection()         { IsArraySection = true; }
  void setIsNonPod()               { IsNonPod = true; }
  void setIsUnsigned()             { IsUnsigned = true; }
  void setIsConditional()          { IsConditional = true; }
  void setIsScheduleMonotonic()    { IsScheduleMonotonic = true; }
  void setIsScheduleNonmonotonic() { IsScheduleNonmonotonic = true; }
  void setIsScheduleSimd()         { IsScheduleSimd = true; }

  // Getters
  StringRef getFullName() const { return FullName; }
  StringRef getBaseName() const { return BaseName; }
  StringRef getModifier() const { return Modifier; }
  int getId() const { return Id; }
  bool getIsArraySection() const { return IsArraySection; }
  bool getIsNonPod() const { return IsNonPod; }
  bool getIsUnsigned() const { return IsUnsigned; }
  bool getIsConditional() const { return IsConditional; }
  bool getIsScheduleMonotonic() const { return IsScheduleMonotonic; }
  bool getIsScheduleNonmonotonic() const { return IsScheduleNonmonotonic; }
  bool getIsScheduleSimd() const { return IsScheduleSimd; }
};

/// \brief This class contains a set of utility functions used by VPO passes.
class VPOAnalysisUtils {

public:
    /// Constructor and destructor
    VPOAnalysisUtils() {}
    ~VPOAnalysisUtils() {}

    /// \brief Return true for a directive_region_entry/exit intrinsic.
    static bool isRegionDirective(Intrinsic::ID Id);
    static bool isRegionDirective(Instruction *I);

    /// \brief If the instruction is a directive_region_entry/exit intrinsic,
    /// return its first OperandBundle's tagname. Otherwise, return an empty
    /// StringRef.
    static StringRef getRegionDirectiveString(Instruction *I);

    /// \brief If getRegionDirectiveString(I) is an OpenMP directive name, then
    /// return its corresponding ID (enum). Otherwise, return -1.
    static int getRegionDirectiveID(Instruction *I);

    /// \brief Return true if the instruction is an intel_directive intrinsic
    /// or a directive_region_entry/exit intrinsic whose first OperandBundle's
    /// tagname matches the name of an OpenMP directive.
    static bool isIntelDirective(Instruction *I, bool doClauses=false);

    /// \brief Return true if the intrinsic Id is intel_directive.
    static bool isIntelDirective(Intrinsic::ID Id);

    /// \brief Return true if the intrinsic Id corresponds to a clause:
    ///    intel_directive_qual,
    ///    intel_directive_qual_opnd, or 
    ///    intel_directive_qual_opndlist.
    static bool isIntelClause(Intrinsic::ID Id);

    /// \brief Return true if the instruction is an intel_directive* intrinsic
    /// or a directive_region_entry/exit intrinsic whose first OperandBundle's
    /// tagname matches the name of an OpenMP directive.
    static bool isIntelDirectiveOrClause(Instruction *I);

    /// \brief Return true if the intrinsic Id corresponds to an
    /// Intel directive or clause.
    static bool isIntelDirectiveOrClause(Intrinsic::ID Id);

    /// \brief Return the string representation of the metadata argument used
    /// within a call to one of these intrinsics: 
    ///    llvm.intel.directive, 
    ///    llvm.intel.directive.qual, 
    ///    llvm.intel.directive.qual.opnd, and 
    ///    llvm.intel.directive.qual.opndlist.
    static StringRef getDirectiveMetadataString(const IntrinsicInst *Call);

    /// \brief Return the string representation of the modifier metadata 
    /// argument used in an llvm.intel.directive.qual.opndlist intrinsic
    /// that represents a schedule clause
    static StringRef getScheduleModifierMDString(const IntrinsicInst *Call);
    static StringRef getScheduleModifierMDString(Value *Modifier);

    /// \brief If the instruction is an OpenMP directive or clause, return the
    /// directive or clause name. Otherwise, return an empty StringRef.
    static StringRef getDirOrClauseString(Instruction *I);

    /// \brief If the instruction is an OpenMP directive, return the directive
    /// name. Otherwise, return an empty StringRef.
    static StringRef getDirectiveString(Instruction *I, bool doClauses=false);

    /// \brief Returns strings corresponding to OpenMP directives.
    static StringRef getDirectiveString(int Id);

    /// \brief Returns strings corresponding to OpenMP clauses.
    static StringRef getClauseString(int Id);

    /// \brief Similar to getDirectiveString(int), but strips out the leading 
    /// "DIR_OMP_" prefix substring.
    static StringRef getDirectiveName(int Id);

    /// \brief Similar to getClauseString(), but strips out the leading 
    /// "QUAL_OMP_" prefix substring.
    static StringRef getClauseName(int Id);

    /// \brief Given a ClauseId for a reduction, strip out the leading 
    /// "QUAL_OMP_REDUCTION_" prefix substring, and return the remaining
    /// substring which describes the reduction operation (eg "ADD", "MUL").
    static StringRef getReductionOpName(int Id);

    /// \brief Returns true if the string corresponds to an OpenMP directive.
    static bool isOpenMPDirective(StringRef DirFullName);

    /// \brief Returns true if the string corresponds to an OpenMP clause.
    static bool isOpenMPClause(StringRef ClauseFullName);

    /// \brief Returns the ID (enum) corresponding to OpenMP directives.
    static int getDirectiveID(StringRef DirFullName);
    static int getDirectiveID(Instruction *I);

    /// \brief Returns the ID (enum) corresponding to OpenMP clauses.
    static int getClauseID(StringRef ClauseFullName);

    /// Utilities to handle directives & clauses

    /// \brief Return true for a directive that begins a region, such as
    /// DIR_OMP_PARALLEL and DIR_OMP_SIMD. 
    static bool isBeginDirective(int DirID);
    static bool isBeginDirective(StringRef DirString);
    static bool isBeginDirective(Instruction *I);
    static bool isBeginDirective(BasicBlock *BB);

    /// \brief Return true for a directive that ends a region, such as
    /// DIR_OMP_END_PARALLEL and DIR_OMP_END_SIMD.
    static bool isEndDirective(int DirID);
    static bool isEndDirective(StringRef DirString);
    static bool isEndDirective(Instruction *I);
    static bool isEndDirective(BasicBlock *BB);

    /// \brief Return true for a directive that begins or ends a region.
    static bool isBeginOrEndDirective(int DirID);
    static bool isBeginOrEndDirective(StringRef DirString);
    static bool isBeginOrEndDirective(Instruction *I);
    static bool isBeginOrEndDirective(BasicBlock *BB);

    /// \brief Return true if it begins a stand-alone directive 
    static bool isStandAloneBeginDirective(int DirID);
    static bool isStandAloneBeginDirective(StringRef DirString);
    static bool isStandAloneBeginDirective(Instruction *I);
    static bool isStandAloneBeginDirective(BasicBlock *BB);

    /// \brief Return true if it ends a stand-alone directive 
    static bool isStandAloneEndDirective(int DirID);
    static bool isStandAloneEndDirective(StringRef DirString);
    static bool isStandAloneEndDirective(Instruction *I);
    static bool isStandAloneEndDirective(BasicBlock *BB);

    /// \brief Return true if it corresponds to DIR_QUAL_LIST_END, the 
    /// mandatory marker to end a directive
    static bool isListEndDirective(int DirID);
    static bool isListEndDirective(StringRef DirString);
    static bool isListEndDirective(Instruction *I);

    /// \brief Given a DirID for a BEGIN directive, return the DirID of its
    /// corresponding END directive. 
    static int getMatchingEndDirective(int BeginDirID);

    /// \brief Return true iff the ClauseID represents a DEPEND clause,
    /// such as QUAL_OMP_DEPEND_IN
    static bool isDependClause(int ClauseID);

    /// \brief Return true iff the ClauseID represents a REDUCTION clause,
    /// such as QUAL_OMP_REDUCTION_ADD
    static bool isReductionClause(int ClauseID);

    /// \brief Return true iff the ClauseID represents a SCHEDULE or a
    /// DIST_SCHEDULE clause
    static bool isScheduleClause(int ClauseID);

    /// \brief True for MAP, TO, or FROM clauses
    static bool isMapClause(int ClauseID);

    /// \brief Return 0, 1, or 2 based on the number of arguments that the
    /// clause can take:
    ///   0 for clauses that take no arguments
    ///   1 for clauses that take exactly 1 argument
    ///   2 for all else (such as clauses that take lists)
    static unsigned getClauseType(int ClauseID);


    /// \brief Verify if the BB is malformed w.r.t. OpenMP directive rules.
    /// Return false if malform, true otherwise.
    static bool verifyBB(BasicBlock &BB, bool DoAssert);


    ///////////////////////
    //   GENERAL UTILS   //
    ///////////////////////

    /// \brief Check whether the instruction is a call of the given \p Name
    static bool isCallOfName(Instruction *I, StringRef Name);

    /// \brief If V is an AllocaInst, return it. If V is a CastInst, trace
    /// back the cast chain and return an AllocaInst if it's found, or nullptr
    /// if no AllocaInst is found.
    static AllocaInst *findAllocaInst(Value *V); 
};

} // End vpo namespace

} // End llvm namespace
#endif

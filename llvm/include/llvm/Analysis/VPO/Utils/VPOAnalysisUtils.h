#if INTEL_COLLAB // -*- C++ -*-
//===-- VPOAnalysisUtils.h - Class definitions for VPO utilites -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
#include "llvm/Analysis/Directives.h"
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

/// ClauseSpecifier holds information for an OMP clause name. Fullname
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
/// 3. ByRef operands. Example:
///      FullName = "QUAL.OMP.PRIVATE:BYREF"
///      BaseName = "QUAL.OMP.PRIVATE"
///      Modifier = "BYREF"
///      Id = QUAL_OMP_PRIVATE
///
/// 4. NonPOD operands. Example:
///      FullName = "QUAL.OMP.PRIVATE:NONPOD"
///      BaseName = "QUAL.OMP.PRIVATE"
///      Modifier = "NONPOD"
///      Id = QUAL_OMP_PRIVATE
///
/// 5. MAP clause for aggregate objects. Example:
///      FullName = "QUAL.OMP.MAP.TOFROM:AGGRHEAD"
///      BaseName = "QUAL.OMP.MAP.TOFROM"
///      Modifier = "AGGRHEAD"
///      Id = QUAL_OMP_MAP_TOFROM
#if INTEL_CUSTOMIZATION
///
/// 6. F90 Dope Vector operands. Example:
///      FullName = "QUAL.OMP.PRIVATE:F90_DV"
///      BaseName = "QUAL.OMP.PRIVATE"
///      Modifier = "F90_DV"
///      Id = QUAL_OMP_PRIVATE
#endif // INTEL_CUSTOMIZATION
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
  bool IsByRef:1;
  bool IsNonPod:1;
#if INTEL_CUSTOMIZATION
  bool IsF90DopeVector:1;
#endif // INTEL_CUSTOMIZATION
  bool IsUnsigned:1;     // needed by min/max reduction

  // Conditional lastprivate
  bool IsConditional:1;

  // Schedule modifiers
  bool IsScheduleMonotonic:1;
  bool IsScheduleNonmonotonic:1;
  bool IsScheduleSimd:1;

  // Map clause for aggregate objects
  bool IsMapAggrHead:1;
  bool IsMapAggr:1;

public:

  // Constructor
  ClauseSpecifier(StringRef Name);

  // Setters
  void setFullName(StringRef S) { FullName = S; }
  void setBaseName(StringRef S) { BaseName = S; }
  void setModifier(StringRef S) { Modifier = S; }
  void setId(int N) { Id = N; }
  void setIsArraySection()         { IsArraySection = true; }
  void setIsByRef()                { IsByRef = true; }
  void setIsNonPod()               { IsNonPod = true; }
  void setIsUnsigned()             { IsUnsigned = true; }
  void setIsConditional()          { IsConditional = true; }
  void setIsScheduleMonotonic()    { IsScheduleMonotonic = true; }
  void setIsScheduleNonmonotonic() { IsScheduleNonmonotonic = true; }
  void setIsScheduleSimd()         { IsScheduleSimd = true; }
  void setIsMapAggrHead()          { IsMapAggrHead = true; }
  void setIsMapAggr()              { IsMapAggr = true; }

  // Getters
  StringRef getFullName() const { return FullName; }
  StringRef getBaseName() const { return BaseName; }
  StringRef getModifier() const { return Modifier; }
  int getId() const { return Id; }
  bool getIsArraySection() const { return IsArraySection; }
  bool getIsByRef() const { return IsByRef; }
  bool getIsNonPod() const { return IsNonPod; }
  bool getIsUnsigned() const { return IsUnsigned; }
  bool getIsConditional() const { return IsConditional; }
  bool getIsScheduleMonotonic() const { return IsScheduleMonotonic; }
  bool getIsScheduleNonmonotonic() const { return IsScheduleNonmonotonic; }
  bool getIsScheduleSimd() const { return IsScheduleSimd; }
  bool getIsMapAggrHead() const { return IsMapAggrHead; }
  bool getIsMapAggr() const { return IsMapAggr; }
#if INTEL_CUSTOMIZATION
  void setIsF90DopeVector() {IsF90DopeVector = true; }
  bool getIsF90DopeVector() const { return IsF90DopeVector; }
#endif // INTEL_CUSTOMIZATION
};

/// This class contains a set of utility functions used by VPO passes.
class VPOAnalysisUtils {

private:
    // Deleted constructor and destructor to prevent instantiation.
    VPOAnalysisUtils() = delete;
    ~VPOAnalysisUtils() = delete;

public:
    /// Return true for a directive_region_entry/exit intrinsic.
    static bool isRegionDirective(Intrinsic::ID Id);
    static bool isRegionDirective(const Instruction *I, bool *IsEntry=nullptr);

    /// If the instruction is a directive_region_entry/exit intrinsic,
    /// return its first OperandBundle's tagname. Otherwise, return an empty
    /// StringRef.
    /// \p IsEntry is filled according to whether the directive is an entry
    static StringRef getRegionDirectiveString(const Instruction *I,
                                              bool *IsEntry = nullptr);

    /// If getRegionDirectiveString(I) is a directive name, then
    /// return its corresponding ID (enum). Otherwise, return -1.
    /// \p IsEntry is filled according to whether the directive is an entry
    static int getRegionDirectiveID(const Instruction *I,
                                    bool *IsEntry = nullptr);

    /// If the instruction is a directive, return the directive name.
    /// Otherwise, return an empty StringRef.
    static StringRef getDirectiveString(Instruction *I);

    /// Returns the string corresponding to a directive.
    static StringRef getDirectiveString(int Id);

    /// Returns the string corresponding to a clause.
    static StringRef getClauseString(int Id);

    /// Given an enum \p Id for an OpenMP directive, return its
    /// corresponding directive name without the "DIR_OMP_" prefix.
    static StringRef getOmpDirectiveName(int Id);

    /// Given an enum \p Id for an OpenMP clause, return its
    /// corresponding clause name without the "QUAL_OMP_" prefix.
    static StringRef getOmpClauseName(int Id);

    /// Given a ClauseId for a reduction, strip out the leading
    /// "QUAL_OMP_REDUCTION_" prefix substring, and return the remaining
    /// substring which describes the reduction operation (eg "ADD", "MUL").
    static StringRef getReductionOpName(int Id);

    /// Returns true if the instruction represents an OpenMP directive.
    static bool isOpenMPDirective(Instruction *I);

    /// Returns true if the string corresponds to an OpenMP directive.
    static bool isOpenMPDirective(StringRef DirFullName);

    /// Returns true if the string corresponds to an OpenMP clause.
    static bool isOpenMPClause(StringRef ClauseFullName);

    /// Returns the ID (enum) corresponding to a directive,
    /// or -1 if \p DirFullName does not correspond to a directive name.
    static int getDirectiveID(StringRef DirFullName);
    static int getDirectiveID(Instruction *I);

    /// Returns the ID (enum) corresponding to a clause,
    /// or -1 if \p ClauseFullName does not correspond to a clause name.
    static int getClauseID(StringRef ClauseFullName);

    /// Utilities to handle directives & clauses

    /// Return true for a directive that begins a region, such as
    /// DIR_OMP_PARALLEL and DIR_OMP_SIMD.
    static bool isBeginDirective(int DirID);
    static bool isBeginDirective(StringRef DirString);
    static bool isBeginDirective(Instruction *I);
    static bool isBeginDirective(BasicBlock *BB);

    /// Return true for a directive that ends a region, such as
    /// DIR_OMP_END_PARALLEL and DIR_OMP_END_SIMD.
    static bool isEndDirective(int DirID);
    static bool isEndDirective(StringRef DirString);
    static bool isEndDirective(Instruction *I);
    static bool isEndDirective(BasicBlock *BB);

    /// Return true for a directive that begins or ends a region.
    static bool isBeginOrEndDirective(int DirID);
    static bool isBeginOrEndDirective(StringRef DirString);
    static bool isBeginOrEndDirective(Instruction *I);
    static bool isBeginOrEndDirective(BasicBlock *BB);

    /// Return true if it begins a stand-alone directive
    static bool isStandAloneBeginDirective(int DirID);
    static bool isStandAloneBeginDirective(StringRef DirString);
    static bool isStandAloneBeginDirective(Instruction *I);
    static bool isStandAloneBeginDirective(BasicBlock *BB);

    /// Return true if it ends a stand-alone directive
    static bool isStandAloneEndDirective(int DirID);
    static bool isStandAloneEndDirective(StringRef DirString);
    static bool isStandAloneEndDirective(Instruction *I);
    static bool isStandAloneEndDirective(BasicBlock *BB);

    /// Return true if this directive supports the private clause.
    static bool supportsPrivateClause(int DirID);
    static bool supportsPrivateClause(StringRef DirString);
    static bool supportsPrivateClause(Instruction *I);
    static bool supportsPrivateClause(BasicBlock *BB);

    /// Given an instruction for a region.begin directive, return its
    /// corresponding region.end directive instruction or BB
    static Instruction *getEndRegionDir(Instruction *BeginDir);
    static BasicBlock *getEndRegionDirBB(Instruction *BeginDir);

    /// Given a DirID for a BEGIN directive, return the DirID of its
    /// corresponding END directive.
    static int getMatchingEndDirective(int BeginDirID);

    /// If \p DirString is a begin directive of a construct which needs
    /// outlining, such as parallel, task etc., return \b true. Otherwise,
    /// return \b false.
    static bool isBeginDirectiveOfRegionsNeedingOutlining(StringRef DirString);
    /// Alternate version of the above function, which takes in a directive ID.
    static bool isBeginDirectiveOfRegionsNeedingOutlining(int DirID);

    /// Return true iff the ClauseID represents a DEPEND clause,
    /// such as QUAL_OMP_DEPEND_IN
    static bool isDependClause(int ClauseID);

    /// Return true iff the ClauseID represents a REDUCTION clause,
    /// such as QUAL_OMP_REDUCTION_ADD
    static bool isReductionClause(int ClauseID);

    /// Return true iff the ClauseID represents a SCHEDULE or a
    /// DIST_SCHEDULE clause
    static bool isScheduleClause(int ClauseID);

    /// True for MAP, TO, or FROM clauses
    static bool isMapClause(int ClauseID);

    /// Return 0, 1, or 2 based on the number of arguments that the
    /// clause can take:
    ///   0 for clauses that take no arguments
    ///   1 for clauses that take exactly 1 argument
    ///   2 for all else (such as clauses that take lists)
    static unsigned getClauseType(int ClauseID);

    /// Verify if the BB is malformed w.r.t. OpenMP directive rules.
    /// Return false if malform, true otherwise.
    static bool verifyBB(BasicBlock &BB, bool DoAssert);


    ///////////////////////
    //   GENERAL UTILS   //
    ///////////////////////

    /// Check whether the instruction is a call of the given \p Name
    static bool isCallOfName(Instruction *I, StringRef Name);

    /// If V is an AllocaInst, return it. If V is a CastInst, trace
    /// back the cast chain and return an AllocaInst if it's found, or nullptr
    /// if no AllocaInst is found.
    static AllocaInst *findAllocaInst(Value *V);

    /// \returns true if the function has the string attribute
    /// "may-have-openmp-directive" set to "true"
    static bool mayHaveOpenmpDirective(Function &F);

    /// \returns !mayHaveOpenmpDirective(F). This is mainly used in
    /// passes required by OpenMP that would otherwise be skipped at -O0.
    static bool skipFunctionForOpenmp(Function &F);

    /// \return true if the program is compiled for SPIRV.
    static bool isTargetSPIRV(Module *M);

    /// True for bind clauses
    static bool isBindClause(int ClauseID);
    static bool isBindClause(StringRef ClauseFullName);
};

} // End vpo namespace

} // End llvm namespace
#endif // LLVM_ANALYSIS_VPO_UTILS_VPOANALYSISUTILS_H
#endif // INTEL_COLLAB

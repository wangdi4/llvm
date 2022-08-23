#if INTEL_COLLAB // -*- C++ -*-
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
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

namespace VPOParoptAtomicFreeReduction {
constexpr StringRef GlobalBufferAttr = "paropt_red_globalbuf";
constexpr StringRef TeamsCounterAttr = "paropt_red_teamscounter";
constexpr StringRef GlobalStoreMD = "paropt_red_globalstore";

enum Kind { Kind_Local = 1, Kind_Global = 2 };
} // namespace VPOParoptAtomicFreeReduction

/// opencl address space.
enum AddressSpace {
  ADDRESS_SPACE_PRIVATE = 0,
  ADDRESS_SPACE_GLOBAL = 1,
  ADDRESS_SPACE_CONSTANT = 2,
  ADDRESS_SPACE_LOCAL = 3,
  ADDRESS_SPACE_GENERIC = 4
};

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
/// * Schedule modifiers. Example:
///      FullName = "QUAL.OMP.SCHEDULE.STATIC:SIMD.MONOTONIC"
///      BaseName = "QUAL.OMP.SCHEDULE.STATIC"
///      Modifier = "SIMD.MONOTONIC"
///      Id = QUAL_OMP_SCHEDULE_STATIC
///
/// * Array sections. Example:
///      FullName = "QUAL.OMP.REDUCTION.ADD:ARRSECT"
///      BaseName = "QUAL.OMP.REDUCTION.ADD"
///      Modifier = "ARRSECT"
///      Id = QUAL_OMP_REDUCTION_ADD
///
/// * ByRef operands. Example:
///      FullName = "QUAL.OMP.PRIVATE:BYREF"
///      BaseName = "QUAL.OMP.PRIVATE"
///      Modifier = "BYREF"
///      Id = QUAL_OMP_PRIVATE
///
/// * NonPOD operands. Example:
///      FullName = "QUAL.OMP.PRIVATE:NONPOD"
///      BaseName = "QUAL.OMP.PRIVATE"
///      Modifier = "NONPOD"
///      Id = QUAL_OMP_PRIVATE
///
#if INTEL_CUSTOMIZATION
/// * Fortran NonPOD operands. Example:
///      FullName = "QUAL.OMP.PRIVATE:F90_NONPOD"
///      BaseName = "QUAL.OMP.PRIVATE"
///      Modifier = "F90_NONPOD"
///      Id = QUAL_OMP_PRIVATE
///
#endif // INTEL_CUSTOMIZATION
/// * MAP clause for aggregate objects. Example:
///      FullName = "QUAL.OMP.MAP.TOFROM:AGGRHEAD"
///      BaseName = "QUAL.OMP.MAP.TOFROM"
///      Modifier = "AGGRHEAD"
///      Id = QUAL_OMP_MAP_TOFROM
///
/// * MAP chains' links after the first link. Example:
///      FullName = "QUAL.OMP.MAP.TOFROM:CHAIN"
///      BaseName = "QUAL.OMP.MAP.TOFROM"
///      Modifier = "CHAIN"
///      Id = QUAL_OMP_MAP_TOFROM
///
/// * LINEAR clause on original loop's IV. Example:
///      FullName = "QUAL.OMP.LINEAR:IV"
///      BaseName = "QUAL.OMP.LINEAR"
///      Modifier = "IV"
///      Id = QUAL_OMP_LINEAR
#if INTEL_CUSTOMIZATION
///
/// * F90 Dope Vector operands. Example:
///      FullName = "QUAL.OMP.PRIVATE:F90_DV"
///      BaseName = "QUAL.OMP.PRIVATE"
///      Modifier = "F90_DV"
///      Id = QUAL_OMP_PRIVATE
///
/// * WI local operands of clauses implying privatization. Example:
///      FullName = "QUAL.OMP.PRIVATE:WILOCAL
///      BaseName = "QUAL.OMP.PRIVATE"
///      Modifier = "WILOCAL"
///      Id = QUAL_OMP_PRIVATE
#endif // INTEL_CUSTOMIZATION
///
/// *  ALWAYS, CLOSE, PRESENT modifiers for map clause. Example:
///      FullName = "QUAL.OMP.MAP:ALWAYS.CLOSE"
///      BaseName = "QUAL.OMP.MAP"
///      Modifiers = "ALWAYS" and "CLOSE"
///      Id = QUAL_OMP_MAP
///
#if INTEL_CUSTOMIZATION
/// *  AGGREGATE, ALLOCATABLE, POINTER, SCALAR defaultmap categories. Example:
#else
/// *  AGGREGATE, POINTER, SCALAR defaultmap categories. Example:
#endif // INTEL_CUSTOMIZATION
///      FullName = "QUAL.OMP.DEFAULTMAP.TO:SCALAR"
///      BaseName = "QUAL.OMP.DEFAULTMAP.TO"
///      Modifier = "SCALAR"
///      Id = QUAL_OMP_DEFAULTMAP_TO
///
/// * Pointer to pointer operands to linear, is/use_device_ptr, etc. clauses.
///   Example:
///      FullName = "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"
///      BaseName = "QUAL.OMP.USE_DEVICE_PTR"
///      Modifier = "PTR_TO_PTR"
///      Id = QUAL_OMP_USE_DEVICE_PTR
///
/// * A function pointer is the base of the map-chain link. Example:
///      FullName = "QUAL.OMP.MAP:FPTR"
///      BaseName = "QUAL.OMP.MAP"
///      Modifier = "FPTR"
///      Id = QUAL_OMP_MAP
///
/// * TASK modifier on REDUCTION clause. Example:
///      FullName = "QUAL.OMP.REDUCTION.ADD:TASK"
///      BaseName = "QUAL.OMP.REDUCTION.ADD"
///      Modifier = "TASK"
///      Id = QUAL_OMP_REDUCTION_ADD
///
/// * INSCAN modifier on REDUCTION clause. Example:
///      FullName = "QUAL.OMP.REDUCTION.ADD:INSCAN"
///      BaseName = "QUAL.OMP.REDUCTION.ADD"
///      Modifier = "INSCAN"
///      Id = QUAL_OMP_REDUCTION_ADD
///
/// * Operand with a variable number-of-elements when the clause was emitted.
///   Example:
///      FullName = "QUAL.OMP.PRIVATE:VARLEN"
///      BaseName = "QUAL.OMP.PRIVATE"
///      Modifier = "VARLEN"
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
  bool IsByRef:1;
  bool IsNonPod:1;
#if INTEL_CUSTOMIZATION
  bool IsF90DopeVector:1;
  bool IsF90NonPod:1;
  bool IsCptr:1;
  bool IsWILocal:1;
  bool IsAllocatable:1;
#endif // INTEL_CUSTOMIZATION
  bool IsVarLen:1;
  bool IsAggregate:1;
  bool IsPointer:1;
  bool IsFunctionPointer:1;
  bool IsPointerToPointer:1;
  bool IsScalar:1;
  bool IsAlways:1;
  bool IsClose:1;
  bool IsPresent:1;
  bool IsUnsigned:1;     // needed by min/max reduction
  bool IsComplex:1;

  // Conditional lastprivate
  bool IsConditional:1;

  // Schedule modifiers
  bool IsScheduleMonotonic:1;
  bool IsScheduleNonmonotonic:1;
  bool IsScheduleSimd:1;

  // Map clause for aggregate objects
  bool IsMapAggrHead:1;
  bool IsMapAggr:1;
  // Map clause is for a link in an ongoing map chain.
  bool IsMapChainLink:1;

  bool IsIV:1;

  // Modidifers for init clause of the interop construct
  bool IsInitTarget:1;
  bool IsInitTargetSync:1;
  bool IsInitPrefer:1;

  // Modifier for reduction clause
  bool IsTask:1;
  bool IsInscan:1;

  bool IsTyped : 1; // needed in case of data type transfer

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
  void setIsAlways()               { IsAlways = true; }
  void setIsClose()                { IsClose = true; }
  void setIsPresent()              { IsPresent = true; }
  void setIsUnsigned()             { IsUnsigned = true; }
  void setIsConditional()          { IsConditional = true; }
  void setIsInitTarget()           { IsInitTarget = true; }
  void setIsInitTargetSync()       { IsInitTargetSync = true; }
  void setIsInitPrefer()           { IsInitPrefer = true; }
  void setIsTask()                 { IsTask = true; }
  void setIsInscan()               { IsInscan = true; }
  void setIsScheduleMonotonic()    { IsScheduleMonotonic = true; }
  void setIsScheduleNonmonotonic() { IsScheduleNonmonotonic = true; }
  void setIsScheduleSimd()         { IsScheduleSimd = true; }
  void setIsMapAggrHead()          { IsMapAggrHead = true; }
  void setIsMapAggr()              { IsMapAggr = true; }
  void setIsMapChainLink()         { IsMapChainLink = true; }
  void setIsVarLen()               { IsVarLen = true; }
  void setIsAggregate()            { IsAggregate = true; }
  void setIsPointer()              { IsPointer = true; }
  void setIsFunctionPointer()      { IsFunctionPointer = true; }
  void setIsPointerToPointer()     { IsPointerToPointer = true; }
  void setIsScalar()               { IsScalar = true; }
  void setIsIV()                   { IsIV = true; }
  void setIsComplex()              { IsComplex = true; }
  void setIsTyped() { IsTyped = true; }

  // Getters
  StringRef getFullName() const { return FullName; }
  StringRef getBaseName() const { return BaseName; }
  StringRef getModifier() const { return Modifier; }
  int getId() const { return Id; }
  bool getIsArraySection() const { return IsArraySection; }
  bool getIsByRef() const { return IsByRef; }
  bool getIsNonPod() const { return IsNonPod; }
  bool getIsAlways() const { return IsAlways; }
  bool getIsClose() const { return IsClose; }
  bool getIsPresent() const { return IsPresent; }
  bool getIsUnsigned() const { return IsUnsigned; }
  bool getIsConditional() const { return IsConditional; }
  bool getIsInitTarget() const { return IsInitTarget; }
  bool getIsInitTargetSync() const { return IsInitTargetSync; }
  bool getIsInitPrefer() const { return IsInitPrefer; }
  bool getIsTask() const { return IsTask; }
  bool getIsInscan() const { return IsInscan; }
  bool getIsScheduleMonotonic() const { return IsScheduleMonotonic; }
  bool getIsScheduleNonmonotonic() const { return IsScheduleNonmonotonic; }
  bool getIsScheduleSimd() const { return IsScheduleSimd; }
  bool getIsMapAggrHead() const { return IsMapAggrHead; }
  bool getIsMapAggr() const { return IsMapAggr; }
  bool getIsMapChainLink() const { return IsMapChainLink; }
#if INTEL_CUSTOMIZATION
  void setIsF90DopeVector(bool Flag = true) {IsF90DopeVector = Flag; }
  bool getIsF90DopeVector() const { return IsF90DopeVector; }
  void setIsF90NonPod(bool Flag = true) {IsF90NonPod = Flag; }
  bool getIsF90NonPod() const { return IsF90NonPod; }
  void setIsCptr(bool Flag = true) { IsCptr = Flag; }
  bool getIsCptr() const { return IsCptr; }
  void setIsWILocal() { IsWILocal = true; }
  bool getIsWILocal() const { return IsWILocal; }
  void setIsAllocatable() { IsAllocatable = true; }
  bool getIsAllocatable() const { return IsAllocatable; }
#endif // INTEL_CUSTOMIZATION
  bool getIsVarLen() const { return IsVarLen; }
  bool getIsAggregate() const { return IsAggregate; }
  bool getIsPointer() const { return IsPointer; }
  bool getIsFunctionPointer() const { return IsFunctionPointer; }
  bool getIsPointerToPointer() const { return IsPointerToPointer; }
  bool getIsScalar() const { return IsScalar; }
  bool getIsIV() const { return IsIV; }
  bool getIsComplex() const { return IsComplex; }
  bool getIsTyped() const { return IsTyped; }
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
    static StringRef getDirectiveString(const Instruction *I);

    /// Returns the string corresponding to a directive.
    static StringRef getDirectiveString(int Id);

    /// Returns the string corresponding to a clause.
    static StringRef getClauseString(int Id);

    /// Returns the string corresponding to a clause, with the "TYPED" modifier.
    static std::string getTypedClauseString(int Id);

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
    static int getDirectiveID(const Instruction *I);

    /// Returns the ID (enum) corresponding to a clause,
    /// or -1 if \p ClauseFullName does not correspond to a clause name.
    static int getClauseID(StringRef ClauseFullName);

    /// Utilities to handle directives & clauses

    /// Return true for a directive that begins a region, such as
    /// DIR_OMP_PARALLEL and DIR_OMP_SIMD.
    static bool isBeginDirective(int DirID);
    static bool isBeginDirective(StringRef DirString);
    static bool isBeginDirective(const Instruction *I);
    static bool isBeginDirective(BasicBlock *BB);

    /// Return true for a directive that begins a loop region, such as
    /// DIR_OMP_PARALLEL_LOOP and DIR_OMP_SIMD.
    static bool isBeginLoopDirective(int DirID);
    static bool isBeginLoopDirective(StringRef DirString);
    static bool isBeginLoopDirective(Instruction *I);

    /// Return true for a directive that ends a region, such as
    /// DIR_OMP_END_PARALLEL and DIR_OMP_END_SIMD.
    static bool isEndDirective(int DirID);
    static bool isEndDirective(StringRef DirString);
    static bool isEndDirective(Instruction *I);
    static bool isEndDirective(BasicBlock *BB);

    /// Return true for a directive that ends a loop region, such as
    /// DIR_OMP_END_PARALLEL_LOOP and DIR_OMP_END_SIMD.
    static bool isEndLoopDirective(int DirID);
    static bool isEndLoopDirective(StringRef DirString);
    static bool isEndLoopDirective(Instruction *I);

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

    /// If \p DirString is a end directive of a construct which needs
    /// outlining, such as parallel, task etc., return \b true. Otherwise,
    /// return \b false.
    static bool isEndDirectiveOfRegionsNeedingOutlining(StringRef DirString);
    /// Alternate version of the above function, which takes in a directive ID.
    static bool isEndDirectiveOfRegionsNeedingOutlining(int DirID);

    /// Return true iff the ClauseID represents a DEPEND clause,
    /// such as QUAL_OMP_DEPEND_IN
    static bool isDependClause(int ClauseID);

    /// Return true iff the ClauseID represents a INREDUCTION clause,
    /// such as QUAL_OMP_INREDUCTION_ADD
    static bool isInReductionClause(int ClauseID);

    /// Return true iff the ClauseID represents a REDUCTION clause,
    /// such as QUAL_OMP_REDUCTION_ADD
    static bool isReductionClause(int ClauseID);

    /// Return true iff the ClauseID represents a SCHEDULE or a
    /// DIST_SCHEDULE clause
    static bool isScheduleClause(int ClauseID);

    /// True for MAP, TO, or FROM clauses
    static bool isMapClause(int ClauseID);

    /// True for DEFAULTMAP clauses
    static bool isDefaultmapClause(int ClauseID);

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
    static bool isTargetSPIRV(const Module *M);

    /// True for bind clauses
    static bool isBindClause(int ClauseID);
    static bool isBindClause(StringRef ClauseFullName);

    /// Returns begin loop directive instruction if it exists.
    static Instruction *getBeginLoopDirective(const Loop &Lp);

    /// Returns end loop directive instruction if it exists.
    static Instruction *getEndLoopDirective(const Loop &Lp);

    // Returns true if V is an operand to a "QUAL.OMP.JUMP.TO.END.IF" clause.
    static bool seenOnJumpToEndIfClause(Value *V);
};

} // End vpo namespace

} // End llvm namespace
#endif // LLVM_ANALYSIS_VPO_UTILS_VPOANALYSISUTILS_H
#endif // INTEL_COLLAB

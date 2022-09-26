#if INTEL_COLLAB
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
//===----- WRegionClause.cpp - Implements the template Clause  class ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//   This file has constructor and print functions for some Clause classes.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#define DEBUG_TYPE "wregion-clause"

namespace llvm {

namespace vpo {
cl::opt<bool> UseDevicePtrIsDefaultByRef(
    "vpo-paropt-use-device-ptr-is-default-by-ref", cl::Hidden, cl::init(false),
    cl::desc("Temporary flag to be used until Clang starts adding PTR_TO_PTR "
             "modifier to use_device_ptr clause."));

DenseMap<int, WRNDefaultmapBehavior> WRNDefaultmapBehaviorFromClauseID = {
    {QUAL_OMP_DEFAULTMAP_ALLOC,   WRNDefaultmapBehavior::WRNDefaultmapAlloc},
    {QUAL_OMP_DEFAULTMAP_TO,      WRNDefaultmapBehavior::WRNDefaultmapTo},
    {QUAL_OMP_DEFAULTMAP_FROM,    WRNDefaultmapBehavior::WRNDefaultmapFrom},
    {QUAL_OMP_DEFAULTMAP_TOFROM,  WRNDefaultmapBehavior::WRNDefaultmapToFrom},
    {QUAL_OMP_DEFAULTMAP_FIRSTPRIVATE,
                              WRNDefaultmapBehavior::WRNDefaultmapFirstprivate},
    {QUAL_OMP_DEFAULTMAP_NONE,    WRNDefaultmapBehavior::WRNDefaultmapNone},
    {QUAL_OMP_DEFAULTMAP_DEFAULT, WRNDefaultmapBehavior::WRNDefaultmapDefault},
    {QUAL_OMP_DEFAULTMAP_PRESENT, WRNDefaultmapBehavior::WRNDefaultmapPresent}};

DenseMap<int, StringRef> WRNDefaultName = {
    {WRNDefaultKind::WRNDefaultAbsent,       "UNSPECIFIED"},
    {WRNDefaultKind::WRNDefaultNone,         "NONE"},
    {WRNDefaultKind::WRNDefaultShared,       "SHARED"},
    {WRNDefaultKind::WRNDefaultPrivate,      "PRIVATE"},
    {WRNDefaultKind::WRNDefaultFirstprivate, "FIRSTPRIVATE"},
    {WRNDefaultKind::WRNDefaultAbsent,       "LASTPRIVATE"}};

DenseMap<int, StringRef> WRNDefaultmapBehaviorName = {
    {WRNDefaultmapBehavior::WRNDefaultmapAbsent,       "UNSPECIFIED"},
    {WRNDefaultmapBehavior::WRNDefaultmapAlloc,        "ALLOC"},
    {WRNDefaultmapBehavior::WRNDefaultmapTo,           "TO"},
    {WRNDefaultmapBehavior::WRNDefaultmapFrom,         "FROM"},
    {WRNDefaultmapBehavior::WRNDefaultmapToFrom,       "TOFROM"},
    {WRNDefaultmapBehavior::WRNDefaultmapFirstprivate, "FIRSTPRIVATE"},
    {WRNDefaultmapBehavior::WRNDefaultmapNone,         "NONE"},
    {WRNDefaultmapBehavior::WRNDefaultmapDefault,      "DEFAULT"},
    {WRNDefaultmapBehavior::WRNDefaultmapPresent,      "PRESENT"}};

DenseMap<int, StringRef> WRNDefaultmapCategoryName = {
    {WRNDefaultmapCategory::WRNDefaultmapAllVars,      "UNSPECIFIED"},
    {WRNDefaultmapCategory::WRNDefaultmapAggregate,    "AGGREGATE"},
#if INTEL_CUSTOMIZATION
    {WRNDefaultmapCategory::WRNDefaultmapAllocatable,  "ALLOCATABLE"},
#endif // INTEL_CUSTOMIZATION
    {WRNDefaultmapCategory::WRNDefaultmapPointer,      "POINTER"},
    {WRNDefaultmapCategory::WRNDefaultmapScalar,       "SCALAR"}};

DenseMap<int, StringRef> WRNAtomicName = {
    {WRNAtomicKind::WRNAtomicUpdate,  "UPDATE"},
    {WRNAtomicKind::WRNAtomicRead,    "READ"},
    {WRNAtomicKind::WRNAtomicWrite,   "WRITE"},
    {WRNAtomicKind::WRNAtomicCapture, "CAPTURE"}};

DenseMap<int, StringRef> WRNCancelName = {
    {WRNCancelKind::WRNCancelError,    "ERROR"},  // not optional
    {WRNCancelKind::WRNCancelParallel, "PARALLEL"},
    {WRNCancelKind::WRNCancelLoop,     "LOOP"},
    {WRNCancelKind::WRNCancelSections, "SECTIONS"},
    {WRNCancelKind::WRNCancelTaskgroup,"TASKGROUP"}};

DenseMap<int, StringRef> WRNProcBindName = {
    {WRNProcBindKind::WRNProcBindAbsent, "UNSPECIFIED"},
    {WRNProcBindKind::WRNProcBindTrue,   "TRUE"},
    {WRNProcBindKind::WRNProcBindMaster, "MASTER"},
    {WRNProcBindKind::WRNProcBindClose,  "CLOSE"},
    {WRNProcBindKind::WRNProcBindSpread, "SPREAD"}};

DenseMap<int, StringRef> WRNLoopBindName = {
    {WRNLoopBindKind::WRNLoopBindAbsent, "UNSPECIFIED"},
    {WRNLoopBindKind::WRNLoopBindTeams, "TEAMS"},
    {WRNLoopBindKind::WRNLoopBindParallel, "PARALLEL"},
    {WRNLoopBindKind::WRNLoopBindThread, "THREAD"}};

DenseMap<int, StringRef> WRNLoopOrderName = {
    {WRNLoopOrderKind::WRNLoopOrderAbsent, "UNSPECIFIED"},
    {WRNLoopOrderKind::WRNLoopOrderConcurrent, "CONCURRENT"}};

DenseMap<int, StringRef> WRNScheduleName = {
    {WRNScheduleKind::WRNScheduleCrewloop, "Crew Loop"},
    {WRNScheduleKind::WRNScheduleStatic, "Static"},
    {WRNScheduleKind::WRNScheduleStaticEven, "Static Even"},
    {WRNScheduleKind::WRNScheduleDynamic, "Dynamic"},
    {WRNScheduleKind::WRNScheduleGuided, "Guided"},
    {WRNScheduleKind::WRNScheduleRuntime, "Runtime"},
    {WRNScheduleKind::WRNScheduleAuto, "Auto"},
    {WRNScheduleKind::WRNScheduleTrapezoidal, "Trapezoidal"},
    {WRNScheduleKind::WRNScheduleStaticGreedy, "Static Greedy"},
    {WRNScheduleKind::WRNScheduleStaticBalanced, "Static Balanced"},
    {WRNScheduleKind::WRNScheduleGuidedIterative, "Guided Iterative"},
    {WRNScheduleKind::WRNScheduleGuidedAnalytical, "Guided Analytical"},
    {WRNScheduleKind::WRNScheduleOrderedStatic, "Ordered Static"},
    {WRNScheduleKind::WRNScheduleOrderedStaticEven, "Ordered Static Even"},
    {WRNScheduleKind::WRNScheduleOrderedDynamic, "Ordered Dynamic"},
    {WRNScheduleKind::WRNScheduleOrderedGuided, "Ordered Guided"},
    {WRNScheduleKind::WRNScheduleOrderedRuntime, "Ordered Runtime"},
    {WRNScheduleKind::WRNScheduleOrderedAuto, "Ordered Auto"},
    {WRNScheduleKind::WRNScheduleOrderedTrapezoidal, "Ordered Trapezoidal"},
    {WRNScheduleKind::WRNScheduleOrderedStaticGreedy, "Ordered Static Greedy"},
    {WRNScheduleKind::WRNScheduleOrderedStaticBalanced,
                                                  "Ordered Static Balanced"},
    {WRNScheduleKind::WRNScheduleOrderedGuidedIterative,
                                                  "Ordered Guided Iterative"},
    {WRNScheduleKind::WRNScheduleOrderedGuidedAnalytical,
                                                  "Ordered Guided Analytical"},
    {WRNScheduleKind::WRNScheduleDistributeStatic, "Distribute Static"},
    {WRNScheduleKind::WRNScheduleDistributeStaticEven,
                                                  "Distribute Static Even"}};

// Constructors for template Clause classes
template<>Clause<SharedItem>      ::Clause():ClauseID(QUAL_OMP_SHARED){}
template<>Clause<PrivateItem>     ::Clause():ClauseID(QUAL_OMP_PRIVATE){}
template<>Clause<FirstprivateItem>::Clause():ClauseID(QUAL_OMP_FIRSTPRIVATE){}
template<>Clause<LastprivateItem> ::Clause():ClauseID(QUAL_OMP_LASTPRIVATE){}
template<>Clause<ReductionItem>   ::Clause():ClauseID(QUAL_OMP_REDUCTION_ADD){}
template<>Clause<CopyinItem>      ::Clause():ClauseID(QUAL_OMP_COPYIN){}
template<>Clause<CopyprivateItem> ::Clause():ClauseID(QUAL_OMP_COPYPRIVATE){}
template<>Clause<LinearItem>      ::Clause():ClauseID(QUAL_OMP_LINEAR){}
template<>Clause<UniformItem>     ::Clause():ClauseID(QUAL_OMP_UNIFORM){}
template<>Clause<MapItem>         ::Clause():ClauseID(QUAL_OMP_MAP_TO){}
template<>Clause<IsDevicePtrItem> ::Clause():ClauseID(QUAL_OMP_IS_DEVICE_PTR){}
template<>Clause<UseDevicePtrItem>::Clause():ClauseID(QUAL_OMP_USE_DEVICE_PTR){}
template<>Clause<InclusiveItem>   ::Clause():ClauseID(QUAL_OMP_INCLUSIVE){}
template<>Clause<ExclusiveItem>   ::Clause():ClauseID(QUAL_OMP_EXCLUSIVE){}
template<>Clause<SubdeviceItem>   ::Clause():ClauseID(QUAL_OMP_SUBDEVICE){}
template<>Clause<InteropActionItem>::Clause():ClauseID(QUAL_OMP_INIT){}
template<>Clause<DependItem>      ::Clause():ClauseID(QUAL_OMP_DEPEND_IN){}
template<>Clause<DepSinkItem>     ::Clause():ClauseID(QUAL_OMP_DEPEND_SINK){}
template<>Clause<DepSourceItem>   ::Clause():ClauseID(QUAL_OMP_DEPEND_SOURCE){}
template<>Clause<AlignedItem>     ::Clause():ClauseID(QUAL_OMP_ALIGNED){}
template<>Clause<NontemporalItem> ::Clause():ClauseID(QUAL_OMP_NONTEMPORAL){}
template<>Clause<FlushItem>       ::Clause():ClauseID(QUAL_OMP_FLUSH){}
template<>Clause<SizesItem>       ::Clause():ClauseID(QUAL_OMP_SIZES){}
template<>Clause<LiveinItem>      ::Clause():ClauseID(QUAL_OMP_LIVEIN){}
template<>Clause<AllocateItem>    ::Clause():ClauseID(QUAL_OMP_ALLOCATE){}
template<>Clause<DataItem>        ::Clause():ClauseID(QUAL_OMP_DATA){}
template<>Clause<InteropItem>     ::Clause():ClauseID(QUAL_OMP_INTEROP){}


// Print routine for ScheduleClause. Returns true iff something is printed.
// Eg,
//    SCHEDULE clause: Dynamic (MONOTONIC ), ChunkSize: i32 8
//
bool ScheduleClause::print(formatted_raw_ostream &OS, unsigned Depth,
                           unsigned Verbosity) const {
  if (Verbosity==0 && !ChunkExpr)
    return false;  // ChunkExpr==NULL means there was not SCHEDULE clause

  StringRef Title;

  if (isDistSchedule())
    Title = "DIST_SCHEDULE";
  else
    Title = "SCHEDULE";

  OS.indent(2*Depth) << Title << " clause: ";

  if (!ChunkExpr) {
    // ChunkExpr==NULL means there is no SCHEDULE clause
    OS << "UNSPECIFIED\n";
    return true;
  }

  OS << WRNScheduleName[Kind] << " (";

  if(getIsSchedMonotonic())
    OS << "MONOTONIC ";
  if(getIsSchedNonmonotonic())
    OS << "NONMONOTONIC ";
  if(getIsSchedSimd())
    OS << "SIMD ";
  OS << "), ChunkSize: ";

  if (Chunk==0)
    OS << "UNSPECIFIED\n";
  else
    OS << *ChunkExpr << "\n";

  return true;
}

void ArraySectionInfo::print(formatted_raw_ostream &OS, bool PrintType) const {
  if (ArraySectionDims.empty())
    return;

  OS << "ARRAY SECTION INFO: (";
  if (Offset) {
    OS << " Offset: ";
    Offset->printAsOperand(OS, PrintType);
  };
  if (Size) {
    OS << " Size: ";
    Size->printAsOperand(OS, PrintType);
  };
  if (ElementType) {
    OS << " ElementType: ";
    ElementType->print(OS, PrintType);
  };
  OS << " Dims:";
  for (const auto &Dim : ArraySectionDims) {
    OS << " ( ";
    std::get<0>(Dim)->printAsOperand(OS, PrintType);
    OS << ", ";
    std::get<1>(Dim)->printAsOperand(OS, PrintType);
    OS << ", ";
    std::get<2>(Dim)->printAsOperand(OS, PrintType);
    OS << " )";
  }
  OS << ")";
}

void ArraySectionInfo::print(raw_ostream &OS, bool PrintType) const {
  formatted_raw_ostream FOS(OS);
  print(FOS, PrintType);
}

// An N-dimensional array section is represented in the IR as follows:
//  Args[0] holds the base pointer
//  Args[1] holds N
//  Args[2,3,4] hold the <LB:Size:Stride> tuple for dimesion 0
//  Args[5,6,7] hold the <LB:Size:Stride> tuple for dimesion 1
//  ...etc.
//
//  For user-defined reduction, there are 4 additional arguments at the end (L
//  is the argument length for the array section):
//  Args[L-4] holds initializer
//  Args[L-3] holds combiner
//  Args[L-2] holds constructor
//  Args[L-1] holds destructor
//
// This routine populates ArraySectionDims with the tuples.
void ArraySectionInfo::populateArraySectionDims(const Use *Args,
                                                unsigned NumArgs) {
  assert(isa<ConstantInt>(Args[1]) &&
         "Non-constant Value for number of array section dimensions.");
  ConstantInt *CI = cast<ConstantInt>(Args[1]);
  uint64_t NumDims = CI->getZExtValue();

  for (unsigned I = 0; I < NumDims; ++I) {
    Value *LB = Args[3 * I + 2];
    Value *SZ = Args[3 * I + 3];
    Value *ST = Args[3 * I + 4];

    addDimension(std::make_tuple(LB, SZ, ST));
  }
}

bool ArraySectionInfo::isVariableLengthArraySection() const {
  const int NumDims = ArraySectionDims.size();
  for (int I = NumDims - 1; I >= 0; --I) {
    auto const &Dim = ArraySectionDims[I];
    Value *SectionDimSize = std::get<1>(Dim);
    if (SectionDimSize && !isa<ConstantInt>(SectionDimSize))
      return true;
  }
  return false;
}

bool ArraySectionInfo::hasVariableStartingOffset() const {
  const int NumDims = ArraySectionDims.size();
  for (int I = NumDims - 1; I >= 0; --I) {
    auto const &Dim = ArraySectionDims[I];
    Value *SectionLB = std::get<0>(Dim);
    if (SectionLB && !isa<ConstantInt>(SectionLB))
      return true;
  }
  return false;
}

bool ArraySectionInfo::isArraySectionWithVariableLengthOrOffset() const {
  return hasVariableStartingOffset() || isVariableLengthArraySection();
}

// This routine populates PreferList from the Args.
void InteropActionItem::populatePreferList(const Use *Args, unsigned NumArgs) {
  setIsPrefer();
  assert(NumArgs != 0 &&
         "prefer_type modifier is not allowed with empty preference-list");
  for (unsigned i = 0; i < NumArgs; i++) {
    ConstantInt *CI = cast<ConstantInt>(Args[i]);
    if (CI->getValue() == 3) {
      setIsPreferOpenCL();
      PreferList.push_back(3);
    } else if (CI->getValue() == 4) {
      setIsPreferSycl();
      PreferList.push_back(4);
    } else if (CI->getValue() == 6) {
      setIsPreferL0();
      PreferList.push_back(6);
    } else
      llvm_unreachable("Non acceptable option of preference-list");
  }
}

void printFnPtr(Function *Fn, formatted_raw_ostream &OS, bool PrintType) {
  if (Fn==nullptr)
    OS << "UNSPECIFIED";
  else
    Fn->printAsOperand(OS, PrintType);
}
#if INTEL_CUSTOMIZATION
template <> VarType<LLVMIR> Item::getOrig<LLVMIR>() const { return OrigItem; }
template <> VarType<HIR> Item::getOrig<HIR>() const { return HOrigItem; }

template <> ExprType<LLVMIR> LinearItem::getStep<LLVMIR>() const {return Step;}
template <> ExprType<HIR> LinearItem::getStep<HIR>() const { return HStep; }
#endif // INTEL_CUSTOMIZATION
} // End namespace vpo

} // End namespace llvm

#endif // INTEL_COLLAB

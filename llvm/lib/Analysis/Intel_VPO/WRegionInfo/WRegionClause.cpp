#if INTEL_COLLAB
//===----- WRegionClause.cpp - Implements the template Clause  class ------===// //
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
//   This file has constructor and print functions for some Clause classes.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"

namespace llvm {

namespace vpo {

std::unordered_map<int, StringRef> WRNDefaultName = {
    {WRNDefaultKind::WRNDefaultAbsent,       "UNSPECIFIED"},
    {WRNDefaultKind::WRNDefaultNone,         "NONE"},
    {WRNDefaultKind::WRNDefaultShared,       "SHARED"},
    {WRNDefaultKind::WRNDefaultPrivate,      "PRIVATE"},
    {WRNDefaultKind::WRNDefaultFirstprivate, "FIRSTPRIVATE"},
    {WRNDefaultKind::WRNDefaultAbsent,       "LASTPRIVATE"}};

std::unordered_map<int, StringRef> WRNAtomicName = {
    {WRNAtomicKind::WRNAtomicUpdate,  "UPDATE"},
    {WRNAtomicKind::WRNAtomicRead,    "READ"},
    {WRNAtomicKind::WRNAtomicWrite,   "WRITE"},
    {WRNAtomicKind::WRNAtomicCapture, "CAPTURE"}};

std::unordered_map<int, StringRef> WRNCancelName = {
    {WRNCancelKind::WRNCancelError,    "ERROR"},  // not optional
    {WRNCancelKind::WRNCancelParallel, "PARALLEL"},
    {WRNCancelKind::WRNCancelLoop,     "LOOP"},
    {WRNCancelKind::WRNCancelSections, "SECTIONS"},
    {WRNCancelKind::WRNCancelTaskgroup,"TASKGROUP"}};

std::unordered_map<int, StringRef> WRNProcBindName = {
    {WRNProcBindKind::WRNProcBindAbsent, "UNSPECIFIED"},
    {WRNProcBindKind::WRNProcBindTrue,   "TRUE"},
    {WRNProcBindKind::WRNProcBindMaster, "MASTER"},
    {WRNProcBindKind::WRNProcBindClose,  "CLOSE"},
    {WRNProcBindKind::WRNProcBindSpread, "SPREAD"}};

std::unordered_map<int, StringRef> WRNScheduleName = {
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
template<>Clause<DependItem>      ::Clause():ClauseID(QUAL_OMP_DEPEND_IN){}
template<>Clause<DepSinkItem>     ::Clause():ClauseID(QUAL_OMP_DEPEND_SINK){}
template<>Clause<AlignedItem>     ::Clause():ClauseID(QUAL_OMP_ALIGNED){}
template<>Clause<FlushItem>       ::Clause():ClauseID(QUAL_OMP_FLUSH){}


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

void printFnPtr(Function *Fn, formatted_raw_ostream &OS, bool PrintType) {
  if (Fn==nullptr)
    OS << "UNSPECIFIED";
  else
    Fn->printAsOperand(OS, PrintType);
}

} // End namespace vpo

} // End namespace llvm

#endif // INTEL_COLLAB

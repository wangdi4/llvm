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


// print() routine for ScheduleClause
void ScheduleClause::print(formatted_raw_ostream &OS, unsigned Depth,
                           bool Verbose) const {
  if (!Verbose && !ChunkExpr)
    return;  // ChunkExpr==NULL means there was not SCHEDULE clause

  OS.indent(2*Depth) << "SCHEDULE clause";
  if (!ChunkExpr) {
    // ChunkExpr==NULL means there is no SCHEDULE clause
    OS << " is ABSENT\n";
    return;
  }

  switch(Kind) {
  case WRNScheduleAuto:
    OS << ": AUTO (";
    break;
  case WRNScheduleDynamic:
    OS << ": DYNAMIC (";
    break;
  case WRNScheduleGuided:
    OS << ": GUIDED (";
    break;
  case WRNScheduleRuntime:
    OS << ": RUNTIME (";
    break;
  case WRNScheduleStatic:
    OS << ": STATIC (";
    break;
  default:
    OS << ": TYPE=" << Kind << " (";
  }

  if(getIsSchedMonotonic())
    OS << "MONOTONIC ";
  if(getIsSchedNonmonotonic())
    OS << "NONMONOTONIC ";
  if(getIsSchedSimd())
    OS << "SIMD ";
  OS << "), ChunkSize= " << *ChunkExpr << "\n";
}

} // End namespace vpo

} // End namespace llvm


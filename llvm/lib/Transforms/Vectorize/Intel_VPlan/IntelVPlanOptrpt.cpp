//===-- IntelVPlanOptrpt.cpp ------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of utilities for opt remark emission.
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanOptrpt.h"
#include "IntelVPlan.h"

using namespace llvm;
using namespace llvm::vpo;

template <class LoopTy>
void OptReportStatsTracker::emitRemarks(VPlanOptReportBuilder &Builder,
                                        LoopTy *Lp) const {
  int Gathers = MaskedGathers + UnmaskedGathers;
  int Scatters = MaskedScatters + UnmaskedScatters;
#define VPLAN_OPTRPT_GS(Gathers, Scatters)                                     \
  if (Gathers)                                                                 \
    Builder.addRemark(Lp, OptReportVerbosity::High,                            \
                      OptRemarkID::GatherReason);                              \
  if (Scatters)                                                                \
    Builder.addRemark(Lp, OptReportVerbosity::High, OptRemarkID::ScatterReason);
#define VPLAN_OPTRPT_HANDLE_GROUP_BEGIN(ID)                                    \
  Builder.addRemark(Lp, OptReportVerbosity::High, ID);
#define VPLAN_OPTRPT_HANDLE(ID, NAME)                                          \
  if (NAME != 0)                                                               \
    Builder.addRemark(Lp, OptReportVerbosity::High, ID, Twine(NAME).str());
#define VPLAN_OPTRPT_HANDLE_GROUP_END(ID)                                      \
  Builder.addRemark(Lp, OptReportVerbosity::High, ID);
#define VPLAN_OPTRPT_VEC_HANDLE(VEC)                                           \
  for (auto &Itr : VEC) {                                                      \
    Builder.addRemark(Lp, Itr.MessageVerbosity, Itr.Remark);                   \
  } // end of definition
#define VPLAN_OPTRPT_ORIGIN_VEC_HANDLE(VEC)                                    \
  for (auto &Itr : VEC) {                                                      \
    Builder.addOrigin(Lp, Itr.Remark.getRemarkID());                           \
  } // end of definition
#include "IntelVPlanOptrpt.inc"
}

template void OptReportStatsTracker::emitRemarks<Loop>(VPlanOptReportBuilder &,
                                                       Loop *) const;
template void
OptReportStatsTracker::emitRemarks<loopopt::HLLoop>(VPlanOptReportBuilder &,
                                                    loopopt::HLLoop *) const;

void OptReportStatsTracker::emitRemarks(OptReportBuilder &Builder, VPLoop *Lp,
                                        VPLoopInfo *VPLI) const {
  int Gathers = MaskedGathers + UnmaskedGathers;
  int Scatters = MaskedScatters + UnmaskedScatters;
#define VPLAN_OPTRPT_GS(Gathers, Scatters)                                     \
  if (Gathers)                                                                 \
    Builder(*Lp, *VPLI)                                                        \
        .addRemark(OptReportVerbosity::High, OptRemarkID::GatherReason);       \
  if (Scatters)                                                                \
    Builder(*Lp, *VPLI)                                                        \
        .addRemark(OptReportVerbosity::High, OptRemarkID::ScatterReason);
#define VPLAN_OPTRPT_HANDLE_GROUP_BEGIN(ID)                                    \
  Builder(*Lp, *VPLI).addRemark(OptReportVerbosity::High, ID);
#define VPLAN_OPTRPT_HANDLE(ID, NAME)                                          \
  if (NAME != 0)                                                               \
    Builder(*Lp, *VPLI)                                                        \
        .addRemark(OptReportVerbosity::High, ID, Twine(NAME).str());
#define VPLAN_OPTRPT_HANDLE_GROUP_END(ID)                                      \
  Builder(*Lp, *VPLI).addRemark(OptReportVerbosity::High, ID);
#define VPLAN_OPTRPT_VEC_HANDLE(VEC)                                           \
  for (auto &Itr : VEC) {                                                      \
    Builder(*Lp, *VPLI).addRemark(Itr.MessageVerbosity, Itr.Remark);           \
  } // end of definition
#define VPLAN_OPTRPT_ORIGIN_VEC_HANDLE(VEC)                                    \
  for (auto &Itr : VEC) {                                                      \
    Builder(*Lp, *VPLI).addOrigin(Itr.Remark.getRemarkID());                   \
  } // end of definition
#include "IntelVPlanOptrpt.inc"
}

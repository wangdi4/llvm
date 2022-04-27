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
  if (Gathers) Builder.addRemark(Lp, OptReportVerbosity::High, 15567);         \
  if (Scatters) Builder.addRemark(Lp, OptReportVerbosity::High, 15568);
#define VPLAN_OPTRPT_HANDLE_GROUP_BEGIN(ID)                                    \
  Builder.addRemark(Lp, OptReportVerbosity::High, ID);
#define VPLAN_OPTRPT_HANDLE(ID, NAME)                                          \
  Builder.addRemark(Lp, OptReportVerbosity::High, ID, Twine(NAME).str());
#define VPLAN_OPTRPT_HANDLE_GROUP_END(ID)                                      \
  Builder.addRemark(Lp, OptReportVerbosity::High, ID);
#define VPLAN_OPTRPT_VEC_HANDLE(VEC)                                           \
  for (auto &Itr : VEC) {                                                      \
    if (Itr.Msg.empty())                                                       \
      Builder.addRemark(Lp, Itr.MessageVerbosity, Itr.RemarkID);               \
    else                                                                       \
      Builder.addRemark(Lp, Itr.MessageVerbosity, Itr.RemarkID, Itr.Msg);      \
  } // end of definition
#define VPLAN_OPTRPT_ORIGIN_VEC_HANDLE(VEC)                                    \
  for (auto &Itr : VEC) {                                                      \
    Builder.addOrigin(Lp, Itr.RemarkID);                                       \
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
    Builder(*Lp, *VPLI).addRemark(OptReportVerbosity::High, 15567u);           \
  if (Scatters)                                                                \
    Builder(*Lp, *VPLI).addRemark(OptReportVerbosity::High, 15568u);
#define VPLAN_OPTRPT_HANDLE_GROUP_BEGIN(ID)                                    \
  Builder(*Lp, *VPLI).addRemark(OptReportVerbosity::High, ID);
#define VPLAN_OPTRPT_HANDLE(ID, NAME)                                          \
  Builder(*Lp, *VPLI)                                                          \
      .addRemark(OptReportVerbosity::High, ID, Twine(NAME).str());
#define VPLAN_OPTRPT_HANDLE_GROUP_END(ID)                                      \
  Builder(*Lp, *VPLI).addRemark(OptReportVerbosity::High, ID);
#define VPLAN_OPTRPT_VEC_HANDLE(VEC)                                           \
  for (auto &Itr : VEC) {                                                      \
    if (Itr.Msg.empty())                                                       \
      Builder(*Lp, *VPLI).addRemark(Itr.MessageVerbosity, Itr.RemarkID);       \
    else                                                                       \
      Builder(*Lp, *VPLI)                                                      \
          .addRemark(Itr.MessageVerbosity, Itr.RemarkID, Itr.Msg);             \
  } // end of definition
#define VPLAN_OPTRPT_ORIGIN_VEC_HANDLE(VEC)                                    \
  for (auto &Itr : VEC) {                                                      \
    Builder(*Lp, *VPLI).addOrigin(Itr.RemarkID);                               \
  } // end of definition
#include "IntelVPlanOptrpt.inc"
}

//===- DPCPPStatistic.cpp - Save Statistic as metadata ----------*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_PRODUCT_RELEASE

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/DPCPPStatistic.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataStatsAPI.h"
#include <ctime>

using namespace llvm;
using namespace DPCPPKernelMetadataAPI;

bool DPCPPStatistic::StatFlag = false;
std::string DPCPPStatistic::CurrentStatType;

// -dpcpp-stat - Command line option to enable DPCPP statistics collection in
// DPCPP passes.
static cl::opt<bool, true> DpcppStat("dpcpp-stat",
                                     cl::desc("Enable dpcpp stat collection"),
                                     cl::Hidden,
                                     cl::location(DPCPPStatistic::StatFlag));

namespace {

// -dpcpp-stat-only - Command line option to collect DPCPP statistics only from
// a specific pass.
struct StatOnlyOpt {
  StatOnlyOpt &operator=(const std::string &Val) {
    DPCPPStatistic::StatFlag |= !Val.empty();
    DPCPPStatistic::CurrentStatType = Val;
    return *this;
  }
};

} // namespace

static StatOnlyOpt StatOnlyOptLoc;

static cl::opt<StatOnlyOpt, true, cl::parser<std::string>>
    DpcppStatOnly("dpcpp-stat-only",
                  cl::desc("Collect stats for a specifc pass only"), cl::Hidden,
                  cl::value_desc("debug type string"),
                  cl::location(StatOnlyOptLoc), cl::ValueRequired);

void DPCPPStatistic::setModuleStatInfo(Module *M, StringRef RunTimeVersion,
                                       StringRef WorkloadName,
                                       StringRef ModuleName) {
  assert(M && "trying to record stat Info for non existing module");

  time_t RawTime = std::time(nullptr);
  std::tm *TimeInfo = std::localtime(&RawTime);
  assert(TimeInfo && "Getting local time failed");
  char Buffer[64];
  std::strftime(Buffer, sizeof(Buffer), "%Y-%m-%d %H:%M:%S", TimeInfo);

  auto MSMD = ModuleStatMetadataAPI(M);
  MSMD.StatType.set(CurrentStatType);
  MSMD.ExecTime.set(Buffer);
  MSMD.RunTimeVersion.set(RunTimeVersion.str());
  MSMD.WorkloadName.set(WorkloadName.str());
  MSMD.ModuleName.set(ModuleName.str());
}

void DPCPPStatistic::pushFunctionStats(ActiveStatsT &ActiveStats, Function &F,
                                       StringRef Ty) {

  // if stats are off or if stats are not collected for this module return
  if (!StatFlag || !isCurrentStatType(Ty))
    return;

  // if no stats were collected return
  if (ActiveStats.empty())
    return;

  // verify that the function is part of a module
  assert(F.getParent() &&
         "Trying to add stats to function that is not part of a module");
  if (F.getParent() == nullptr)
    return;

  FunctionStatMetadataAPI::StatListTy Stats;

  // copy stat values from the DPCPPStatistic object to the meta data
  for (unsigned I = 0; I < ActiveStats.size(); ++I) {
    DPCPPStatistic *LightStat = ActiveStats[I];

    assert(LightStat->Initialized &&
           "Non initialized stat is in the active list");

    Stats.push_back(
        {LightStat->Name.str(), (int)LightStat->Value, LightStat->Desc.str()});

    // clear stat and make it ready for use with another function
    LightStat->reset();
  }

  FunctionStatMetadataAPI::set(F, Stats);

  // remove all stats registered for this list
  ActiveStats.clear();
}

void DPCPPStatistic::moveFunctionStats(Function &FromFunction,
                                       Function &ToFunction) {
  // if stats are off or if stats are not collected for this module return
  if (!StatFlag)
    return;

  // verify that ToFunction is part of a module
  assert(ToFunction.getParent() &&
         "Trying to move stats to function that is not part of a module");
  if (ToFunction.getParent() == nullptr)
    return;

  // it is assumed that ToFunction and FromFunction are defined in the same
  // context
  FunctionStatMetadataAPI::move(FromFunction, ToFunction);
}

void DPCPPStatistic::copyFunctionStats(Function &FromFunction,
                                       Function &ToFunction) {
  // if stats are off or if stats are not collected for this module return
  if (!StatFlag)
    return;

  FunctionStatMetadataAPI::copy(FromFunction, ToFunction);
}

void DPCPPStatistic::removeFunctionStats(Function &FromFunction) {

  // if stats are off or if stats are not collected for this module return
  if (!StatFlag)
    return;

  // verify that the function is part of a module
  assert(FromFunction.getParent() &&
         "Trying to remove stats from function that is not part of a module");
  if (FromFunction.getParent() == nullptr)
    return;

  FunctionStatMetadataAPI::remove(FromFunction);
}

#endif // INTEL_PRODUCT_RELEASE

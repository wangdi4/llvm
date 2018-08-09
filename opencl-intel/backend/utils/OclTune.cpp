// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef INTEL_PRODUCT_RELEASE

#include "OclTune.h"
#include "buildversion.h"
#include "MetadataStatsAPI.h"

#include "llvm/Support/CommandLine.h"

#include <time.h>

using namespace llvm;
using namespace Intel;
using namespace MetadataAPI;

namespace intel {

bool Statistic::StatFlag = false;
std::string Statistic::CurrentStatType;

// -oclstat - Command line option to enable OCL statistics collection in OCL
// passes.
static cl::opt<bool, true>
OclStat("oclstat", cl::desc("Enable ocl stat collection"), cl::Hidden,
      cl::location(Statistic::StatFlag));

namespace {

// -oclstat-only - Command line option to collect OCL statistics only from a
// specific pass.
struct StatOnlyOpt {
  void operator=(const std::string &Val) const {
    Statistic::StatFlag |= !Val.empty();
    Statistic::CurrentStatType = Val;
  }
};

}

static StatOnlyOpt StatOnlyOptLoc;

static cl::opt<StatOnlyOpt, true, cl::parser<std::string> >
OclStatOnly("oclstat-only", cl::desc("Collect stats for a specifc pass only"),
          cl::Hidden, cl::value_desc("debug type string"),
          cl::location(StatOnlyOptLoc), cl::ValueRequired);


void Statistic::setModuleStatInfo (llvm::Module *M, const char * workloadName,
    const char * moduleName)
{
  assert (M && "trying to record stat Info for non existing module");
  if (!M)
    return;

  time_t rawtime;
  struct tm * timeinfo;
  char buffer [80];

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime (buffer,80,"%Y-%m-%d %H:%M:%S",timeinfo);

  auto msimd = ModuleStatMetadataAPI(M);
  msimd.StatType.set(CurrentStatType);
  msimd.ExecTime.set(buffer);
  msimd.RunTimeVersion.set(VERSIONSTRING); // runtime version
  msimd.WorkloadName.set(workloadName);
  msimd.ModuleName.set(moduleName);
}

void Statistic::pushFunctionStats(ActiveStatsT &activeStats, llvm::Function &F,
                                  const char *type) {

  // if stats are off or if stats are not collected for this module return
  if (!StatFlag || !isCurrentStatType(type))
    return;

  // if no stats were collected return
  if (activeStats.empty())
    return;

  // verify that the function is part of a module
  assert (F.getParent() &&
      "Trying to add stats to function that is not part of a module");
  if (F.getParent() == nullptr)
    return;

  FunctionStatMetadataAPI::StatListTy stats;

  // copy stat values from the Statistic object to the meta data
  for (unsigned i = 0; i < activeStats.size(); i++) {
    Statistic *lightStat = activeStats[i];

    assert (lightStat->Initialized &&
        "Non initialized stat is in the active list");

    stats.push_back({ lightStat->Name, (int)lightStat->Value, lightStat->Desc });

    // clear stat and make it ready for use with another function
    lightStat->reset();
  }

  FunctionStatMetadataAPI::set(F, stats);

  // remove all stats registered for this list
  activeStats.clear();
}

void Statistic::moveFunctionStats(
  llvm::Function &FromFunction,
  llvm::Function &ToFunction)
{
  // if stats are off or if stats are not collected for this module return
  if (!StatFlag)
    return;

  // verify that ToFunction is part of a module
  assert (ToFunction.getParent() &&
      "Trying to move stats to function that is not part of a module");
  if (ToFunction.getParent() == nullptr)
    return;

  // it is assumed that ToFunction and FromFunction are defined in the same context
  FunctionStatMetadataAPI::move(FromFunction, ToFunction);
}

void Statistic::copyFunctionStats (
  llvm::Function &FromFunction,
  llvm::Function &ToFunction)
{

  // if stats are off or if stats are not collected for this module return
  if (!StatFlag)
    return;

  FunctionStatMetadataAPI::copy(FromFunction, ToFunction);
}

void Statistic::removeFunctionStats (llvm::Function &FromFunction)
{

  // if stats are off or if stats are not collected for this module return
  if (!StatFlag)
    return;

  // verify that the function is part of a module
  assert (FromFunction.getParent() &&
      "Trying to remove stats from function that is not part of a module");
  if (FromFunction.getParent() == nullptr)
    return;

  FunctionStatMetadataAPI::remove(FromFunction);
}

}

#endif // INTEL_PRODUCT_RELEASE

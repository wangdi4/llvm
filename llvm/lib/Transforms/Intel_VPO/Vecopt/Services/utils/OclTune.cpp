/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2014).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  OclTune.cpp

\*****************************************************************************/

#ifdef OCLT
#include "OclTune.h"
#include "llvm/Support/CommandLine.h"
#ifdef USE_METADATA_API
#include "MetaDataApi.h"
#endif

#include <time.h>

using namespace llvm;
#ifdef USE_METADATA_API
using namespace Intel;
#endif

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

#ifdef USE_METADATA_API
  Intel::MetaDataUtils mdUtils(M);

  ModuleStatInfoMetaData *statInfo = ModuleStatInfoMetaData::get();
  statInfo->setExecTime(buffer);
#define VERSIONSTRING "1.2.3.4" // xmain
  statInfo->setRunTimeVersion(VERSIONSTRING); // runtime version
  statInfo->setWorkloadName(workloadName);
  statInfo->setModuleName(moduleName);

  mdUtils.addModuleStatInfoCItem(ModuleStatInfoMetaDataHandle(statInfo));

  //Save Metadata to the module
  mdUtils.save(M->getContext());
#endif
}

void Statistic::pushFunctionStats (ActiveStatsT &activeStats, llvm::Function &F,
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
  if (F.getParent() == NULL)
    return;

#ifdef USE_METADATA_API
  Intel::MetaDataUtils mdUtils(F.getParent());

  // get a pointer to the list of this function stats
  StatValueListContainerMetaData *statList =
      mdUtils.getOrInsertFunctionsStatsItem(&F).get();
  // copy stat values from the Statistic object to the meta data
  for (unsigned i = 0; i < activeStats.size(); i++) {
    Statistic *lightStat = activeStats[i];
    StatValueItemMetaData *MDStat = StatValueItemMetaData::get();

    assert (lightStat->Initialized &&
        "Non initialized stat is in the active list");

    MDStat->setName(lightStat->Name);
    MDStat->setValue(lightStat->Value);
    statList->addStatValueListItem(StatValueItemMetaDataHandle(MDStat));

    // clear stat and make it ready for use with another function
    lightStat->reset();

    // keep stat description once for this module
    if (mdUtils.findStatDescriptionsItem(lightStat->Name)
        == mdUtils.end_StatDescriptions()) {
      StrCMetaData *MDDesc = StrCMetaData::get();
      MDDesc->setstr(lightStat->Desc);
      mdUtils.setStatDescriptionsItem(lightStat->Name,
          StrCMetaDataHandle(MDDesc));
    }
  }

  //Save Metadata to the module
  mdUtils.save(F.getContext());
#endif

  // remove all stats registered for this list
  activeStats.clear();
}

void Statistic::moveFunctionStats (llvm::Function &FromFunction,
    llvm::Function &ToFunction)
{
  // if stats are off or if stats are not collected for this module return
  if (!StatFlag)
    return;

  // verify that ToFunction is part of a module
  assert (ToFunction.getParent() &&
      "Trying to move stats to function that is not part of a module");
  if (ToFunction.getParent() == NULL)
    return;

  // it is assumed that ToFunction and FromFunction are defined in the same context
#ifdef USE_METADATA_API
  Intel::MetaDataUtils mdUtils(ToFunction.getParent());

  // if FromFunction has no stats return
  if (mdUtils.findFunctionsStatsItem(&FromFunction) ==
      mdUtils.end_FunctionsStats())
    return;

  // if ToFunction already has stats need to use copy here and not move
  assert (mdUtils.findFunctionsStatsItem(&ToFunction) ==
      mdUtils.end_FunctionsStats() &&
          "ToFunction already has stats. Use copy instead of move stats");

  // switch the content of the FromFunction and ToFunction
  StatValueListContainerMetaDataHandle &statListFrom =
      mdUtils.findFunctionsStatsItem(&FromFunction)->second;

  mdUtils.getOrInsertFunctionsStatsItem(&ToFunction);
  StatValueListContainerMetaDataHandle &statListTo =
      mdUtils.findFunctionsStatsItem(&ToFunction)->second;

  mdUtils.setFunctionsStatsItem(&ToFunction, statListFrom);

  mdUtils.setFunctionsStatsItem(&FromFunction, statListTo);

  // remove stat entry for FromFunction
  mdUtils.eraseFunctionsStatsItem(mdUtils.findFunctionsStatsItem(&FromFunction));

  //Save Metadata to the module
  mdUtils.save(FromFunction.getContext());
#endif
}

void Statistic::copyFunctionStats (llvm::Function &FromFunction,
    llvm::Function &ToFunction)
{

  // if stats are off or if stats are not collected for this module return
  if (!StatFlag)
    return;

  // verify that ToFunction is part of a module
  assert (ToFunction.getParent() &&
      "Trying to copy stats to function that is not part of a module");
  if (ToFunction.getParent() == NULL)
    return;

  // it is assumed that ToFunction and FromFunction are defined in the same context
#ifdef USE_METADATA_API
  Intel::MetaDataUtils mdUtils(ToFunction.getParent());

  // if FromFunction has no stats return
  if (mdUtils.findFunctionsStatsItem(&FromFunction) ==
      mdUtils.end_FunctionsStats())
    return;

  // get a pointer to the list of FromFunction stats
  StatValueListContainerMetaData *statListFrom =
      mdUtils.getOrInsertFunctionsStatsItem(&FromFunction).get();
  // get a pointer to the list of ToFunction stats
  StatValueListContainerMetaData *statListTo =
      mdUtils.getOrInsertFunctionsStatsItem(&ToFunction).get();
  // copy stat values from FromFunction to ToFunction
  for (unsigned i = 0; i < statListFrom->size_StatValueList(); i++) {
    StatValueItemMetaData *MDStat = StatValueItemMetaData::get();

    MDStat->setName(statListFrom->getStatValueListItem(i)->getName());
    MDStat->setValue(statListFrom->getStatValueListItem(i)->getValue());
    statListTo->addStatValueListItem(StatValueItemMetaDataHandle(MDStat));
  }

  //Save Metadata to the module
  mdUtils.save(FromFunction.getContext());
#endif
}

void Statistic::removeFunctionStats (llvm::Function &FromFunction)
{

  // if stats are off or if stats are not collected for this module return
  if (!StatFlag)
    return;

  // verify that the function is part of a module
  assert (FromFunction.getParent() &&
      "Trying to remove stats from function that is not part of a module");
  if (FromFunction.getParent() == NULL)
    return;

#ifdef USE_METADATA_API
  Intel::MetaDataUtils mdUtils(FromFunction.getParent());

  // if FromFunction has no stats return
  if (mdUtils.findFunctionsStatsItem(&FromFunction) ==
      mdUtils.end_FunctionsStats())
    return;

  // remove stat entry for FromFunction
  mdUtils.eraseFunctionsStatsItem(mdUtils.findFunctionsStatsItem(&FromFunction));

  //Save Metadata to the module
  mdUtils.save(FromFunction.getContext());
#endif
}

}

#endif // OCLT

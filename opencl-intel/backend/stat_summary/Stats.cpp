// INTEL CONFIDENTIAL
//
// Copyright 2014-2018 Intel Corporation.
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

#include "Stats.h"

#include "llvm/Support/Path.h"
#include "llvm/ADT/StringRef.h"


const string &WorkloadInfo::getWorkloadID (const string &location,
    const string &name) {

  string *id = new string();
  assert (id && "No space for new workload ID");

  id->assign(llvm::sys::path::parent_path(llvm::StringRef(location.c_str())));

  if (id->size() == 0 || *id->rbegin() != '/')
    id->append("/");

  id->append(name);
  return *id;
}


void StatValueMapC::dumpStatValueMapHead(stringstream &str,
    const StatValueMap &statNames)
{
  for (StatValueMap::const_iterator it = statNames.begin();
       it != statNames.end(); it++) {
    str << it->first << ",";
  }

  str << "Total";
}

void StatValueMapC::dumpStatValueMapValue (stringstream &str,
    const StatValueMap &statNames, const StatValueMap &values)
{
  unsigned sum = 0;

  for (StatValueMap::const_iterator it = statNames.begin();
       it != statNames.end(); it++) {
    StatValueMap::const_iterator valueItr = values.find(it->first);
    if (valueItr == values.end())
      str << "0,";
    else {
      str << valueItr->second << ",";
      sum += valueItr->second;
    }
  }

  str << sum;
}

void ModuleStats::dumpStats(stringstream &str, const StatValueMap &names,
    unsigned levels) const
{
  // dump module summary stats and workload info
  str << "2," << moduleName << ",";
  StatValueMapC::dumpStatValueMapValue(str, names, moduleStats);
  str << "," << runtimeVersion << "," << collectedTime << "\n";

  if (levels >= 2) {
    // dum stats for each function
    for (NamedStatValueMap::const_iterator it = functionStats.begin();
         it != functionStats.end(); it++) {
      str << "3, " << it->first << ",";
      StatValueMapC::dumpStatValueMapValue(str, names, it->second);
      str << ",, \n";
    }
  }
}

void WorkloadInfo::dumpStats(stringstream &str, const StatValueMap &names,
    unsigned levels) const
{
  // dump worload sums
  string s = llvm::sys::path::filename(llvm::StringRef(id.c_str()));

  str << "1," << s << ",";
  StatValueMapC::dumpStatValueMapValue(str, names, workloadStats);
  str << ",, \n";

  if (levels > 1) {
    // dump stats for each module
    for (unsigned int i = 0; i < statList.size(); i++) {
      statList[i]->dumpStats(str, names, levels-1);
    }
  }
}

void ExperimentInfo::dumpStats(stringstream &str, unsigned levels)
{
  // dump header line
  str << ",,";
  StatValueMapC::dumpStatValueMapHead(str, allStats);
  str << ",Version,Time\n";

  if (levels > 1) {
    // dump stats of each workload
    for (WorkloadMap::const_iterator it = workloads.begin();
         it != workloads.end(); it++) {
      it->second.dumpStats(str, allStats, levels-1);
    }
  }

  // dump summary line
  str << "Total,,";
  StatValueMapC::dumpStatValueMapValue(str, allStats, allStats);
  str << "\n";
}


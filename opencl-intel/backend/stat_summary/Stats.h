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

#include <map>
#include <string>
#include <vector>
#include <sstream>

#include <assert.h>

using namespace std;

typedef map<const string, unsigned> StatValueMap;
typedef map<string, StatValueMap> NamedStatValueMap;
typedef map<const string, string> StatDescrMap;


class StatValueMapC {
public:
  static void SumStats(const StatValueMap &from, StatValueMap& to) {
    for (StatValueMap::const_iterator it = from.begin();
         it != from.end(); it++) {
      to[it->first] += it->second;
    }
  }

  static void dumpStatValueMapHead(stringstream &str,
      const StatValueMap &statNames);
  static void dumpStatValueMapValue (stringstream &str,
      const StatValueMap &statNames,
      const StatValueMap &values);
};

class ModuleStats {
public:
  ModuleStats (const string &ver,
      const string &time,
      const string &wname,
      const string &mname) :
        runtimeVersion(ver),
        collectedTime(time),
        workloadName(wname),
        moduleName(mname) {}

  const string& getCollectedTime() const {
    return collectedTime;
  }

  const string& getModuleName() const {
    return moduleName;
  }

  const string& getRuntimeVersion() const {
    return runtimeVersion;
  }

  const string& getWorkloadName() const {
    return workloadName;
  }

  StatValueMap &getFunctionStats(const string&functionName) {
    return functionStats[functionName];
  }

  const StatValueMap &getModuleStats() const {
    return moduleStats;
  }

  void addStatValue (const string statName, unsigned value) {
    moduleStats[statName] += value;
  }

  unsigned getStatValue (const string &name) {
    return moduleStats[name];
  }

  void dumpStats(stringstream &str, const StatValueMap &names, unsigned levels)
       const;

private:
  string runtimeVersion;
  string collectedTime;
  string workloadName;
  string moduleName;

  StatValueMap moduleStats;            // Module stat summary
  NamedStatValueMap functionStats;     // stats per function
};

class WorkloadInfo {
public:
  static const string &getWorkloadID (const string &location,
      const string &name);

  void setId (const string &name) {
    id = name;
  }

  const string &getId() const {
    return id;
  }

  ModuleStats &addModule(const string &ver,
      const string &time,
      const string &wname,
      const string &mname) {
    ModuleStats *MS = new ModuleStats(ver, time, wname, mname);
    statList.push_back(MS);
    return *MS;
  }

  void sumStats() {
    for (unsigned i = 0; i < statList.size(); i++) {
      StatValueMapC::SumStats(statList[i]->getModuleStats(), workloadStats);
    }
  }

  const StatValueMap &getWorkloadStats() const {
    return workloadStats;
  }

  void dumpStats(stringstream &str, const StatValueMap &names, unsigned levels)
       const;

private:
  string id;
  StatValueMap workloadStats;          // workload stats
  vector<ModuleStats *> statList;      // stats per module in this workload
};

// ExperimentInfo - information of all workloads in this experiment
class ExperimentInfo {
public:
  ExperimentInfo() {}

  WorkloadInfo& getWorkloadInfo(const string &workloadId) {
    if (workloads.find(workloadId) == workloads.end())
      workloads[workloadId].setId(workloadId);
    return workloads[workloadId];
  }

  void addStatDescription(const string &name, const string &descr) {
    if (allStats.find(name) != allStats.end())
      return;

    allStats[name] = 0;
    statDescriptions[name] = descr;
  }

  void sumStats() {
    for (WorkloadMap::iterator it = workloads.begin();
         it != workloads.end(); it++) {
      WorkloadInfo &WI = it->second;
      WI.sumStats();
      StatValueMapC::SumStats(WI.getWorkloadStats(), allStats);
    }
  }

  void dumpStats(stringstream &str, unsigned levels);

private:
  typedef map<string, WorkloadInfo> WorkloadMap;
  StatDescrMap statDescriptions;       // description of stats
  StatValueMap allStats;               // totals of stats from all modules
  WorkloadMap workloads;               // map from workload name to its info
};

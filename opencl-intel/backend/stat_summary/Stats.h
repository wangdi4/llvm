// INTEL CONFIDENTIAL
//
// Copyright 2014 Intel Corporation.
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

#include <assert.h>
#include <map>
#include <sstream>
#include <string>
#include <vector>

typedef std::map<const std::string, unsigned> StatValueMap;
typedef std::map<std::string, StatValueMap> NamedStatValueMap;
typedef std::map<const std::string, std::string> StatDescrMap;

class StatValueMapC {
public:
  static void SumStats(const StatValueMap &from, StatValueMap &to) {
    for (StatValueMap::const_iterator it = from.begin(); it != from.end();
         it++) {
      to[it->first] += it->second;
    }
  }

  static void dumpStatValueMapHead(std::stringstream &str,
                                   const StatValueMap &statNames);
  static void dumpStatValueMapValue(std::stringstream &str,
                                    const StatValueMap &statNames,
                                    const StatValueMap &values);
};

class ModuleStats {
public:
  ModuleStats(const std::string &ver, const std::string &time,
              const std::string &wname, const std::string &mname)
      : runtimeVersion(ver), collectedTime(time), workloadName(wname),
        moduleName(mname) {}

  const std::string &getCollectedTime() const { return collectedTime; }

  const std::string &getModuleName() const { return moduleName; }

  const std::string &getRuntimeVersion() const { return runtimeVersion; }

  const std::string &getWorkloadName() const { return workloadName; }

  StatValueMap &getFunctionStats(const std::string &functionName) {
    return functionStats[functionName];
  }

  const StatValueMap &getModuleStats() const { return moduleStats; }

  void addStatValue(const std::string statName, unsigned value) {
    moduleStats[statName] += value;
  }

  unsigned getStatValue(const std::string &name) { return moduleStats[name]; }

  void dumpStats(std::stringstream &str, const StatValueMap &names,
                 unsigned levels) const;

private:
  std::string runtimeVersion;
  std::string collectedTime;
  std::string workloadName;
  std::string moduleName;

  StatValueMap moduleStats;        // Module stat summary
  NamedStatValueMap functionStats; // stats per function
};

class WorkloadInfo {
public:
  static const std::string &getWorkloadID(const std::string &location,
                                          const std::string &name);

  void setId(const std::string &name) { id = name; }

  const std::string &getId() const { return id; }

  ModuleStats &addModule(const std::string &ver, const std::string &time,
                         const std::string &wname, const std::string &mname) {
    ModuleStats *MS = new ModuleStats(ver, time, wname, mname);
    statList.push_back(MS);
    return *MS;
  }

  void sumStats() {
    for (unsigned i = 0; i < statList.size(); i++) {
      StatValueMapC::SumStats(statList[i]->getModuleStats(), workloadStats);
    }
  }

  const StatValueMap &getWorkloadStats() const { return workloadStats; }

  void dumpStats(std::stringstream &str, const StatValueMap &names,
                 unsigned levels) const;

private:
  std::string id;
  StatValueMap workloadStats;     // workload stats
  std::vector<ModuleStats *> statList; // stats per module in this workload
};

// ExperimentInfo - information of all workloads in this experiment
class ExperimentInfo {
public:
  ExperimentInfo() {}

  WorkloadInfo &getWorkloadInfo(const std::string &workloadId) {
    if (workloads.find(workloadId) == workloads.end())
      workloads[workloadId].setId(workloadId);
    return workloads[workloadId];
  }

  void addStatDescription(const std::string &name, const std::string &descr) {
    if (allStats.find(name) != allStats.end())
      return;

    allStats[name] = 0;
    statDescriptions[name] = descr;
  }

  void sumStats() {
    for (WorkloadMap::iterator it = workloads.begin(); it != workloads.end();
         it++) {
      WorkloadInfo &WI = it->second;
      WI.sumStats();
      StatValueMapC::SumStats(WI.getWorkloadStats(), allStats);
    }
  }

  void dumpStats(std::stringstream &str, unsigned levels);

private:
  typedef std::map<std::string, WorkloadInfo> WorkloadMap;
  StatDescrMap statDescriptions; // description of stats
  StatValueMap allStats;         // totals of stats from all modules
  WorkloadMap workloads;         // map from workload name to its info
};

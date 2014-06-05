// Copyright (c) 2014 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "Stats.h"

#include "llvm/Support/Path.h"
#include "llvm/ADT/StringRef.h"


const string &WorkloadInfo::getWorkloadID (const string &location,
    const string &name) {

  string *id = new string();
  assert (id && "No space for new workload ID");

  id->assign(llvm::sys::path::parent_path(llvm::StringRef(location.c_str())));

  if (id->size() == 0)
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


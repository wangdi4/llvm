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

#include "MetaDataApi.h"

#include "llvm/IRReader/IRReader.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/SourceMgr.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "Stats.h"

using namespace llvm;
using namespace Intel;

enum DumpLevel {
  S=1, W=2, M=3, F=4
};

namespace Intel {
bool getIRFileNames(const char *dirname, vector<string> &fname);
}

static cl::opt<DumpLevel> dumpLevel("l", cl::desc("Set dump detail level:"),
    cl::values(
        clEnumVal(S, "Dump summary of all counters"),
        clEnumVal(W, "Dump counter summary per workload"),
        clEnumVal(M, "Dump counter summary per workload & module"),
        clEnumVal(F, "Dump counter summary per workload, module & function")),
        cl::init(W));

static cl::opt<string> outFileName("o", cl::desc("Output file name"),
    cl::value_desc("filename"), cl::init("Experiment.csv"));

static cl::list<std::string> inDirs(cl::Positional, cl::OneOrMore,
    cl::desc("<directory-names>"));


bool getFlist (vector<string> &flist)
{
  bool result = true;
  for (unsigned i = 0; i != inDirs.size(); i++) {
    result = result && getIRFileNames(inDirs[i].c_str(), flist);
  }
  cout << "Found " << flist.size() << " files in " << inDirs.size() <<
      " directories.\n";
  return result;
}


bool readStatFiles (vector<string> &flist, ExperimentInfo & expr)
{
  LLVMContext &C = getGlobalContext();
  // for each file
  for (unsigned i = 0; i < flist.size(); i++) {
    SMDiagnostic err;
    // parse the IR
    std::unique_ptr<Module> M = llvm::parseIRFile(flist[i], err, C);
    if (!M)
      continue;

    // get handle to module stat info metadata
    MetaDataUtils mdUtils(M.release());
    if (mdUtils.size_ModuleStatInfoC() == 0) {
      cout << "IR file " << flist[i] << " contains no stats\n";
      continue;
    }

    ModuleStatInfoMetaDataHandle *ModuleStatInfo =
        &*mdUtils.begin_ModuleStatInfoC();

    string workloadName((*ModuleStatInfo)->getWorkloadName());

    // compose unique workload id
    WorkloadInfo& WI = expr.getWorkloadInfo(
        WorkloadInfo::getWorkloadID(flist[i], workloadName));

    // copy module data to stat class
    ModuleStats &moduleStats = WI.addModule((*ModuleStatInfo)->getRunTimeVersion(),
        (*ModuleStatInfo)->getExecTime(),
        workloadName,
        (*ModuleStatInfo)->getModuleName());

    // add stat descriptions to map common to all modules
    for (MetaDataUtils::StatDescriptionsMap::iterator it =
        mdUtils.begin_StatDescriptions();
         it != mdUtils.end_StatDescriptions(); it++) {
      expr.addStatDescription(it->first, it->second->getstr());
    }

    // add stat values to function stats and module stats
    // iterate over all functions
    for (MetaDataUtils::FunctionsStatsMap::iterator it =
        mdUtils.begin_FunctionsStats();
         it != mdUtils.end_FunctionsStats(); it++) {

      if (it->first == NULL) {
        cout << "Warning: found null function in file " << flist[i] << ".\n";
        continue;
      }

      // get function name
      const string funcName(it->first->getName().data());
      StatValueMap &funcStatMap = moduleStats.getFunctionStats(funcName);
      // get list of stats for this function
      StatValueListContainerMetaData *funcStatList = &*it->second;
      // iterate over stat entries in the list
      // accumulate stat values per function and module
      for(StatValueListContainerMetaData::StatValueListList::iterator its =
          funcStatList->begin_StatValueList();
          its != funcStatList->end_StatValueList(); its++) {
        moduleStats.addStatValue(funcStatMap, (*its)->getName(),
            (*its)->getValue());
      }
    }
  }
  return true;
}

bool openStatFile(ofstream &ofs) {

  ofs.open(outFileName.c_str(), ofstream::out);
  if (!ofs.is_open()) {
    cout << "Can't open output file " << outFileName << ".\n";
    return false;
  }

  return true;
}

void transposeDump(stringstream &dump, string &lineStr) {
  vector<stringstream *> trans;

  getline(dump, lineStr);
  for (unsigned i = 0; i < lineStr.size(); i++) {
    if (lineStr[i] == ',') {
      trans.push_back(new stringstream());
    }
  }
  trans.push_back(new stringstream());

  while (lineStr.size() > 0) {
    unsigned i = 0;
    string val;
    unsigned row = 0;
    while (i < lineStr.size()) {
      while (i < lineStr.size() && lineStr[i] != ',') {
        *trans[row] << lineStr[i];
        i++;
      }
      if (i < lineStr.size())
        i++;
      *trans[row] << ',';
      row++;
    }
    getline(dump, lineStr);
  }
  lineStr = "";
  for (unsigned i = 0; i < trans.size(); i++) {
    string tmp = trans[i]->str();
    tmp[tmp.size()-1] = '\n';
    lineStr += tmp;
  }
}


bool dumpStats(ExperimentInfo &exp) {

  ofstream file;
  if (!openStatFile(file))
      return false;

  stringstream str;
  string result;

  exp.dumpStats(str, dumpLevel);

  transposeDump(str, result);

  file << result;

  return true;
}

void parseCL (int argc, char *argv[])
{
  // remove some non-relevant command line options that come from llvm
  StringMap<cl::Option*> optMap (std::move(cl::getRegisteredOptions()));

  if (optMap.count("print-after-all") > 0)
    optMap["print-after-all"]->setHiddenFlag(cl::ReallyHidden);
  if (optMap.count("print-before-all") > 0)
    optMap["print-before-all"]->setHiddenFlag(cl::ReallyHidden);
  if (optMap.count("time-passes") > 0)
    optMap["time-passes"]->setHiddenFlag(cl::ReallyHidden);
  if (optMap.count("version") > 0)
    optMap["version"]->setHiddenFlag(cl::ReallyHidden);
  if (optMap.count("help") > 0)
    optMap["help"]->setDescription("Display available options");


  cl::ParseCommandLineOptions(argc, argv,
      "This program dumps stats to a csv file. It searches for the stats in "
      "IR (.ll) files located in the specified directories.");
}

int main (int argc, char *argv[])
{
  parseCL(argc, argv);
  vector<string> flist;

  ExperimentInfo *experiment = new ExperimentInfo();

  if (!getFlist(flist))
    return -1;

  if (!readStatFiles(flist, *experiment))
    return -1;

  experiment->sumStats();

  if (dumpStats(*experiment))
    return -1;

  return 0;
}

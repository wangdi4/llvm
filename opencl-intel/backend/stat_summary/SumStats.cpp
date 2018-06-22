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

#include "MetadataStatsAPI.h"

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
using namespace MetadataAPI;

extern "C" LLVMContextRef LLVMGetGlobalContext(void);

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
  LLVMContext Ctx;
  // for each file
  for (unsigned i = 0; i < flist.size(); i++) {
    SMDiagnostic err;
    // parse the IR
    auto M = llvm::parseIRFile(flist[i], err, Ctx).release();
    if (!M)
      continue;

    // check module stat info metadata presence
    auto msimd = ModuleStatMetadataAPI(M);
    if (!msimd.StatType.hasValue()) {
      cout << "IR file " << flist[i] << " contains no stats\n";
      continue;
    }

    assert(msimd.WorkloadName.hasValue() && "No Workload Name Stat Metadata!");
    string workloadName(msimd.WorkloadName.get());

    // compose unique workload id
    WorkloadInfo& WI = expr.getWorkloadInfo(
        WorkloadInfo::getWorkloadID(flist[i], workloadName));

    // copy module data to stat class
    assert(msimd.RunTimeVersion.hasValue() && "No Runtime Version Metadata!");
    assert(msimd.ExecTime.hasValue() && "No Ecec Time Metadata!");
    assert(msimd.ModuleName.hasValue() && "No Module Name Metadata!");
    ModuleStats &moduleStats = WI.addModule(msimd.RunTimeVersion.get(),
        msimd.ExecTime.get(),
        workloadName,
        msimd.ModuleName.get());

    // add stat descriptions to map common to all modules
    FunctionStatMetadataAPI::DescriptionListTy descriptionList;
    FunctionStatMetadataAPI::readDescription(*M, descriptionList);
    for (auto &desc : descriptionList) {
      expr.addStatDescription(desc.Name, desc.Description);
    }

    // add stat values to function stats and module stats
    // iterate over all functions
    for (const auto &func : *M) {
      // get function name
      // iterate over stat entries in the list
      // accumulate stat values per function and module
      for (const auto &stat : FunctionStatMetadataAPI(func)) {
        moduleStats.addStatValue(stat.Name, stat.Value);
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

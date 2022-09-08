// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "OclBuiltinEmitter.h"
#include "OclBuiltinsHeaderGen.h"
#include "VectInfoGenerator.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Main.h"
#include "llvm/TableGen/Record.h"

using namespace llvm;

enum ActionType {
  GenOCLBuiltin,
  GenOCLBuiltisnHeader,
  GenVectorizationInfo,
  PrintRecords,
  PrintEnums
};

namespace {

cl::opt<ActionType>
Action(cl::desc("Action to perform:"),
       cl::values(clEnumValN(PrintRecords, "print-records",
                             "Print all records to stdout (default)"),
                  clEnumValN(PrintEnums, "print-enums",
                             "Print enum values for a class"),
                  clEnumValN(GenOCLBuiltin, "gen-ocl-bi",
                             "Generate builtins files for ocl"),
                  clEnumValN(GenOCLBuiltisnHeader, "gen-ocl-biheader",
                            "Generates a header with builtins mangled names"),
                  clEnumValN(GenVectorizationInfo, "gen-vectorization-info",
                            "Generates the vector-variant attibutes for builtins")),
       cl::init(PrintRecords));

cl::opt<std::string>
Class("class", cl::desc("Print Enum list for this class"),
      cl::value_desc("class name"));

} // anonymous namespace

bool OCLTableGenMain(raw_ostream &OS, RecordKeeper &Records) {
    switch (Action) {
      case PrintRecords:
        OS << Records;
        return false;
      case PrintEnums:
        {
          std::vector<Record*> Recs = Records.getAllDerivedDefinitions(Class);
          for (unsigned i = 0, e = Recs.size(); i != e; ++i)
            OS << Recs[i]->getName() << ", ";
          OS << "\n";
          return false;
        }
      case GenOCLBuiltin:
        OclBuiltinEmitter(Records).run(OS);
        return false;
      case GenOCLBuiltisnHeader:
        OclBuiltinsHeaderGen(Records).run(OS);
        return false;
      case GenVectorizationInfo:
        VectInfoGenerator(Records).run(OS);
        return false;
    }

    assert(false && "Invalid Action");
    return true;
  }

int
main(int argc, char** argv)
{
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);

  return TableGenMain(argv[0], &OCLTableGenMain);
}

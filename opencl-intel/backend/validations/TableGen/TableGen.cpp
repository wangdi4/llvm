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

//===----------------------------------------------------------------------===//
//
// This file contains the main function for OCL TableGen.
//
//===----------------------------------------------------------------------===//


#include "OclBuiltinEmitter.h"
#include "OclBuiltinsHeaderGen.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Main.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace llvm;

enum ActionType {
  GenOCLBuiltisnHeader,
  PrintRecords
};

namespace {

cl::opt<ActionType>
Action(cl::desc("Action to perform:"),
       cl::values(clEnumValN(PrintRecords, "print-records",
                             "Print all records to stdout (default)"),
                  clEnumValN(GenOCLBuiltisnHeader, "gen-ocl-src",
                            "Generates a source with builtins mangled names")),
       cl::init(PrintRecords));

cl::opt<std::string>
Class("class", cl::desc("Print Enum list for this class"),
      cl::value_desc("class name"));

} // anonymous namespace

bool OCLTableGenAction(raw_ostream& OS, RecordKeeper& Records) {
  switch (Action) {
    default:
      assert(1 && "Invalid Action");
      return true;
    case PrintRecords:
      OS << Records;
      break;
    case GenOCLBuiltisnHeader:
      OclBuiltinsHeaderGen(Records).run(OS);
      break;
  }

  return false;
}


int
main(int argc, char** argv)
{
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);

  return TableGenMain(argv[0], OCLTableGenAction);
}

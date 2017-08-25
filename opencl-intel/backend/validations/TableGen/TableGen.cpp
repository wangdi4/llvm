// vim:ts=2:sw=2:et:
//===- OclBuiltinEmitter.cpp - Generate OCL builtin impl --------*- C++ -*-===//
//
// Copyright (c) 2012 Intel Corporation
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
//
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

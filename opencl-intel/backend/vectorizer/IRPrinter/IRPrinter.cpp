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

#include "IRPrinter.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

namespace intel {

  char IRPrinter::ID = 0;

  IRPrinter::IRPrinter(std::string dumpDir, std::string dumpName):
    FunctionPass(ID), m_dumpName(dumpName), m_dumpDir(dumpDir) {
  }    

  bool IRPrinter::runOnFunction(llvm::Function &F) {
    // Create the output file.
    using namespace llvm;
    std::stringstream fileName;
    if (m_dumpDir.length() == 0) {
      return false;
    }

    fileName << m_dumpDir.c_str() << "/dump." 
      << F.getName().data()
      << "_"
      << m_dumpName.c_str()
      << ".ll" << std::ends;
    std::error_code ErrorInfo;
    llvm::raw_fd_ostream FDTemp(fileName.str(), ErrorInfo, sys::fs::FA_Write);
    if (ErrorInfo) {
      return false;
    }
    F.print(FDTemp);
    return false;
  }

} // namespace intel {

// createPrintIRPass - Create and return a pass that dumps the module
// to the specified file.
extern "C" {
  llvm::FunctionPass *createIRPrinterPass(std::string dumpDir, std::string dumpName) {
    return new intel::IRPrinter(dumpDir, dumpName);
  }
}

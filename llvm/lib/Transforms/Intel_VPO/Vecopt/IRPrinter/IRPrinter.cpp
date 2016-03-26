/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "IRPrinter.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

namespace intel {

  char IRPrinter::ID = 0;

  IRPrinter::IRPrinter(std::string dumpDir, std::string dumpName):
    FunctionPass(ID), m_dumpName(dumpName), m_dumpDir(dumpDir) {
  }    

  bool IRPrinter::runOnFunction(llvm::Function &F) {
    // Create the output file.
    std::stringstream fileName;
    if (m_dumpDir.length() == 0) {
      return false;
    }

    fileName << m_dumpDir.c_str() << "/dump." 
      << F.getName().data()
      << "_"
      << m_dumpName.c_str()
      << ".ll" << std::ends;
    std::error_code EC;
    llvm::raw_fd_ostream FDTemp(fileName.str().c_str(), EC, llvm::sys::fs::F_None);
    if (EC) {
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

/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  PrintIRPass.cpp

\*****************************************************************************/

#include "IRPrinter.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Bitcode/ReaderWriter.h"

namespace intel
{

char IRPrinter::ID = 0;

IRPrinter::IRPrinter(std::string dumpDir, std::string dumpName):
    FunctionPass(ID), m_dumpName(dumpName), m_dumpDir(dumpDir)
{

}    

bool IRPrinter::runOnFunction(llvm::Function &F)
{
    // Create the output file.
    std::stringstream fileName;
    if (m_dumpDir.length() == 0)
      return false;

    fileName << m_dumpDir.c_str() << "/dump." 
      << F.getName().data()
      << "_"
      << m_dumpName.c_str()
      << ".ll" << std::ends;
    std::string ErrorInfo;
    llvm::raw_fd_ostream FDTemp(fileName.str().c_str(), ErrorInfo,
                llvm::raw_fd_ostream::F_Binary);
    if (!ErrorInfo.empty()) {
        return false;
    }
    F.print(FDTemp, 0);
    return false;
}

llvm::FunctionPass *createIRPrinterPass(std::string dumpDir, 
  std::string dumpName) 
{
    return new IRPrinter(dumpDir, dumpName);
}
}

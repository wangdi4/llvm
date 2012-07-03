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

File Name:  PrintIRPass.h

\*****************************************************************************/
#ifndef __IR_PRINTER__H__
#define __IR_PRINTER__H__

#include <llvm/Pass.h>
#include <llvm/Module.h>

#include <vector>
#include <sstream>
#include <string>

//Copies from the backend PrintIRPass, that I can't use, since the 
//configuration is in the wrong place...
namespace intel {

class IRPrinter : public llvm::FunctionPass
{
    static char ID; // Pass identification, replacement for typeid
    std::string m_dumpName;
    std::string m_dumpDir;
public:
    IRPrinter(std::string dumpDir, std::string dumpName);
    // doPassInitialization - prints the IR
    bool runOnFunction(llvm::Function &F);
};

// createPrintIRPass - Create and return a pass that dumps the module
// to the specified file.
llvm::FunctionPass *createIRPrinterPass(std::string dumpDir, 
  std::string dumpName);
}

#endif

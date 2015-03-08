/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __IR_PRINTER__H__
#define __IR_PRINTER__H__

#include <llvm/Pass.h>
#include <llvm/IR/Function.h>

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

}

#endif
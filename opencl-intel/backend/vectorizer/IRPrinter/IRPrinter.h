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

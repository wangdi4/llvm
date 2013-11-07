/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#pragma once

#include "llvm/PassManager.h"
#include "llvm/ADT/SmallVector.h"

#include <map>
#include <string>

namespace intel {
    class OptimizerConfig;
}

namespace llvm {
    class Pass;
    class Module;
    class Function;
    class ModulePass;
    class LLVMContext;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/**
 *  Responsible for running the IR=>IR optimization passes on given program
 */
class Optimizer
{
public:
    Optimizer( llvm::Module* pModule,
               llvm::Module* pRtlModule,
               const intel::OptimizerConfig* pConfig);

    ~Optimizer();

    void Optimize();

    bool hasUndefinedExternals() const;

    const std::vector<std::string>& GetUndefinedExternals() const;

    /// @brief function pointer calls were detected after standard LLVM optimizations
    /// @return true if function pointer calls were detected
    bool hasFunctionPtrCalls();

    /// @brief obtain functions names with function pointer calls detected
    /// @return reference to std::vector with function names
    std::vector<std::string> GetFunctionPtrCallNames();

private:
    
    // hold the collection of passes
    llvm::PassManager m_PostFailCheckPM;
    llvm::PassManager m_PreFailCheckPM;
    llvm::Module* m_pModule;

    std::vector<std::string> m_undefinedExternalFunctions;
};



}}}



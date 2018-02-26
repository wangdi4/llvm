/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#pragma once

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/LegacyPassManager.h"

#include <string>
#include <vector>

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
               llvm::SmallVector<llvm::Module *, 2> pRtlModuleList,
               const intel::OptimizerConfig* pConfig);

    ~Optimizer();

    void Optimize();

    bool hasUndefinedExternals() const;

    const std::vector<std::string>& GetUndefinedExternals() const;

    /// @brief recursion was detected after standard LLVM optimizations
    /// @return true if recursion was detected
    bool hasRecursion();

    /// @brief checks if some pipes access were not resolved statically
    bool hasFpgaPipeDynamicAccess();

    /// @brief checks if there are some channels whose depths are differs from
    /// real depth on FPGA hardware due to channel depth mode, so we should emit
    /// diagnostic message
    bool hasFPGAChannelsWithDepthIgnored();

    enum InvalidFunctionType {
        RECURSION,
        FPGA_PIPE_DYNAMIC_ACCESS
    };

    enum InvalidGVType {
        FPGA_DEPTH_IS_IGNORED
    };

    /// @brief obtain functions names wich are not valid for OpenCL
    /// @param Ty is a type of invalid function
    /// @return std::vector with function names
    std::vector<std::string> GetInvalidFunctions(InvalidFunctionType Ty);

    /// @brief obtain global variable names wich are not valid due to some
    /// limitations
    /// @param Ty is a type of global variables to search
    /// @return std::vector with global variable names
    std::vector<std::string> GetInvalidGlobals(InvalidGVType Ty);

private:

    // hold the collection of passes
    llvm::legacy::PassManager m_PostFailCheckPM;
    llvm::legacy::PassManager m_PreFailCheckPM;
    llvm::Module* m_pModule;
    llvm::SmallVector<llvm::Module*, 2> m_pRtlModuleList;
    std::vector<std::string> m_undefinedExternalFunctions;
};



}}}



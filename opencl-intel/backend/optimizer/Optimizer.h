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



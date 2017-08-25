/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in accordance with the terms of that agreement
Copyright(c) 2011 - 2013 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#ifndef __RENDERSCRIPT_DRIVER_H__
#define __RENDERSCRIPT_DRIVER_H__

#include "BuiltinLibInfo.h"
#include "Logger.h"

#include "llvm/Pass.h"

using namespace llvm;

namespace Intel {
  class CPUId;
}

namespace intel {
class OptimizerConfig;

// Used for setting the size of a container, which holds pointers to all the functions
#define ESTIMATED_NUM_OF_FUNCTIONS 8

/// @brief Vectorizer pass is used to abstract all the Vectorizer's work
///  as a single module pass, which is to be scheduled by the compiler
class RenderscriptVectorizer : public ModulePass {
private:
    typedef SmallVector<Function*, ESTIMATED_NUM_OF_FUNCTIONS> funcsVector;

public:
    static char ID;
    /// @brief C'tor
    /// @param pConfig Optimizer configuration (contains also vectorizer configuration)
    /// @param optimizerFunctions used to return list of vectorized functions
    /// @param optimizerWidths used to return list of vectorized function width
    RenderscriptVectorizer(const OptimizerConfig* pConfig,
      SmallVectorImpl<Function*> &optimizerFunctions,
      SmallVectorImpl<int> &optimizerWidths);

    /// @brief Default C'tor
    ///        Used only for opt
    RenderscriptVectorizer();

    /// @brief D'tor
    ~RenderscriptVectorizer();

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
        return "Intel RenderScript Vectorizer";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M);

    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<BuiltinLibInfo>();
    }

private:

    /// @brief holds all the "original" (scalar) functions
    funcsVector m_scalarFuncsList;

    /// @brief Pointer to runtime module
    SmallVector<Module*, 2> m_runtimeModuleList;

    /// @brief Number of kernels in current module
    unsigned m_numOfKernels;

    /// @brief Was current module vectorized
    bool m_isModuleVectorized;

    /// @brief Configuration options
    const OptimizerConfig* m_pConfig;

    /// @brief CPU Id
    const Intel::CPUId *m_pCPUId;

    /// @brief pointer to optimizer vecorized functions buffer.
    SmallVectorImpl<Function*> *m_optimizerFunctions;

    /// @brief pointer to optimizer vector widths buffer.
    SmallVectorImpl<int> *m_optimizerWidths;
};

} // namespace intel

#endif // __RENDERSCRIPT_DRIVER_H__


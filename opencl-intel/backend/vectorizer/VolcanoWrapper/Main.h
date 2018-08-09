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

#ifndef __MAIN_H__
#define __MAIN_H__

#include "BuiltinLibInfo.h"
#include "Logger.h"

#include "llvm/Pass.h"

using namespace llvm;


namespace intel {
class OptimizerConfig;

// Used for setting the size of a container, which holds pointers to all the functions
#define ESTIMATED_NUM_OF_FUNCTIONS 8

/// @brief Vectorizer pass is used to abstract all the Vectorizer's work
///  as a single module pass, which is to be scheduled by the compiler
class Vectorizer : public ModulePass {
public:
    static char ID;
    /// @brief C'tor
    /// @param rt Runtime module (contains declarations of all builtin funcs)
    Vectorizer(llvm::SmallVector<llvm::Module*, 2> rtList, const OptimizerConfig* pConfig);
    /// @brief D'tor
    ~Vectorizer();
    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
        return "Intel OpenCL Vectorizer";
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
    Vectorizer(); // Do not implement

    /// @brief List of pointers to runtime modules
    llvm::SmallVector<llvm::Module*, 2> m_runtimeModuleList;

    /// Configuration options
    const OptimizerConfig* m_pConfig;

    /// @brief pointer to optimizer vecorized functions buffer.
    SmallVectorImpl<Function*> *m_optimizerFunctions;
    
    /// @brief pointer to optimizer vector widths buffer.
    SmallVectorImpl<int> *m_optimizerWidths;
};

} // namespace intel

#endif // __MAIN_H__


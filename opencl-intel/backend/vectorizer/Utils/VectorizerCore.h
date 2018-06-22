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

#ifndef __VECTORIZER_CORE_H__
#define __VECTORIZER_CORE_H__

#include "BuiltinLibInfo.h"
#include "Logger.h"
#include "VecConfig.h"

#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"


namespace intel {

/// @brief VectorizerCore pass is used to abstract all the VectorizerCore's work
///  as a single module pass, which is to be scheduled by the compiler
class VectorizerCore : public llvm::FunctionPass {

public:
    static char ID;
    /// @brief C'tor
    VectorizerCore(const OptimizerConfig* pConfig=0);
    /// @brief D'tor
    ~VectorizerCore();
    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
        return "Intel OpenCL VectorizerCore";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnFunction(llvm::Function &F);

    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const {
      AU.addRequired<LoopInfoWrapperPass>();
      AU.addRequired<BuiltinLibInfo>();
    }

    /// @brief Function for querying the vectorization result width
    /// @returns vectorization width (if vectorization succesfull)
    unsigned getPacketWidth();

    /// @brief Function for querying whether function is vectorized
    bool isFunctionVectorized();

    /// @brief Function for querying the vectorization dimension
    unsigned int getVectorizationDim();

    /// @brief Function for querying whether it is ok to unite workgroups.
    bool getCanUniteWorkgroups();

private:
    /// @brief packetization width
    unsigned m_packetWidth;

    /// @brief flag whether vectorization is succesful
    bool m_isFunctionVectorized;

    /// Configuration options
    const OptimizerConfig* m_pConfig;

    /// Weight if the pre vectorized kernel.
    float m_preWeight;

    /// Weight of the post vectoprized kernel.
    float m_postWeight;

    /// The vectorized dimension
    unsigned int m_vectorizationDim;

    /// whether it is ok to unite workgroups.
    bool m_canUniteWorkgroups;

};

} // namespace intel

#endif // __VECTORIZER_CORE_H__


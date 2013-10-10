/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __VECTORIZER_CORE_H__
#define __VECTORIZER_CORE_H__

#include "RuntimeServices.h"
#include "Logger.h"
#include "VecConfig.h"

#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"


using namespace llvm;

namespace intel {


/// @brief VectorizerCore pass is used to abstract all the VectorizerCore's work
///  as a single module pass, which is to be scheduled by the compiler
class VectorizerCore : public FunctionPass {

public:
    static char ID;
    /// @brief C'tor
    /// @param rt Runtime module (contains declarations of all builtin funcs)
    VectorizerCore(const OptimizerConfig* pConfig=0);
    /// @brief D'tor
    ~VectorizerCore();
    /// @brief Provides name of pass
    virtual const char *getPassName() const {
        return "Intel OpenCL VectorizerCore";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnFunction(Function &F);
    
    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LoopInfo>();
  }

    /// @brief Function for querying the vectorization result width
    /// @returns vectorization width (if vectorization succesfull)
    unsigned getPacketWidth();

    /// @brief Function for querying whether function is vectorized
    bool isFunctionVectorized();
  
private:
    /// @brief packetization width
    unsigned m_packetWidth;
    
    /// @brief Pointer to runtimeServieces
    RuntimeServices * m_runtimeServices;

    /// @brief flag whether vectorization is succesful
    bool m_isFunctionVectorized;

    /// Configuration options
    const OptimizerConfig* m_pConfig;

    /// Weight if the pre vectorized kernel.
    float m_preWeight;

    /// Weight of the post vectoprized kernel.
    float m_postWeight;
   
};

} // namespace intel

#endif // __VECTORIZER_CORE_H__


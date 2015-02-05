/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __MAIN_H__
#define __MAIN_H__

#include "VectorVariant.h"
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
private:
    typedef SmallVector<Function*, ESTIMATED_NUM_OF_FUNCTIONS> funcsVector;
    typedef SmallVector<Function*, 20> VectorizationStubsVector;
    
public:
    static char ID;
    /// @brief C'tor
    /// @param rt Runtime module (contains declarations of all builtin funcs)
    Vectorizer(const Module * rt = NULL, const OptimizerConfig* pConfig = NULL);
    /// @brief D'tor
    ~Vectorizer();
    /// @brief Provides name of pass
    virtual const char *getPassName() const {
        return "Intel Vectorizer";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M);
    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      //      AU.addRequired<BuiltinLibInfo>(); /* xmain */
    }

private:

    Function* createFunctionToVectorize(Function& originalFunction,
					VectorVariant& vectorVariant,
					Type* characteristicDataType);
    bool preVectorizeFunction(Function& F);
    void vectorizeFunction(Function& F, VectorVariant& vectorVariant);
    Function* createVectorVersion(Function& vectorizedFunction,
				  VectorVariant& vectorVariant,
				  std::string scalarFuncName);
    void postVectorizeFunction(Function& F);

    void createVectorizationStubs(Module& M);
    void deleteVectorizationStubs();
    /// @brief declarations for functions used for expressing vectorization
    VectorizationStubsVector m_vectorizationStubs; 

    /// @brief holds all the "original" (scalar) functions
    funcsVector m_scalarFuncsList; 
/* xmain */
#if 0
    /// @brief Pointer to runtime module
    const Module * m_runtimeModule;
#endif

    /// @brief Number of kernels in current module
    unsigned m_numOfKernels;

    /// @brief Was current module vectorized
    bool m_isModuleVectorized;

    /// Configuration options
    const OptimizerConfig* m_pConfig;

    /// @brief pointer to optimizer vecorized functions buffer.
    SmallVectorImpl<Function*> *m_optimizerFunctions;
    
    /// @brief pointer to optimizer vector widths buffer.
    SmallVectorImpl<int> *m_optimizerWidths;
};

} // namespace intel

#endif // __MAIN_H__


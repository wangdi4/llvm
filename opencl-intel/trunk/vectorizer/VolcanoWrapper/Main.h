/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __MAIN_H__
#define __MAIN_H__

#include "llvm/Pass.h"
#include "Logger.h"


// Used for setting the size of a container, which holds pointers to all the functions
#define ESTIMATED_NUM_OF_FUNCTIONS 8
// Maximum supported value for vector width
#define MAX_SUPPORTED_VECTOR_WIDTH 16

using namespace llvm;

namespace intel {

class OptimizerConfig;

/// @brief Vectorizer pass is used to abstract all the Vectorizer's work
///  as a single module pass, which is to be scheduled by the compiler
class Vectorizer : public ModulePass {
private:
    typedef SmallVector<Function*, ESTIMATED_NUM_OF_FUNCTIONS> funcsVector;
public:
    static char ID;
    /// @brief C'tor
    /// @param rt Runtime module (contains declarations of all builtin funcs)
    Vectorizer(const Module * rt, const OptimizerConfig* pConfig);
    /// @brief D'tor
    ~Vectorizer();
    /// @brief Provides name of pass
    virtual const char *getPassName() const {
        return "Intel OpenCL Vectorizer";
    }

    /// @brief In the odd case where the get_global_id is given a parameter
    ///  which is not constant, we are unable (actually, don't want to) vectorize.
    ///  in here we check if we need to vectorize.
    /// @param F function to check
    /// @return True if need to vectorize
    bool shouldVectorize(Function* f);

    /// @brief Checks if any the function uses barriers
    /// @param F function to check
    /// @return True if barriers are used
    bool hasBarriers(Function* f);

    /// @brief Checks if any masks are needed in the vectorization of this kernel.
    /// @param F function to check
    /// @return True if masks are needed
    bool masksNeeded(Function* f);

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M);
    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const { AU.addRequired<LoopInfo>(); }

    /// @brief Function for querying the vectorization results
    /// @param Functions vector to be filled with pointers of vectorized
    ///  functions. Order is same as kernels list in the module metadata.
    ///  For non-vectorized kernel, a NULL pointer is inserted.
    /// @returns 0 if successful, non-zero value otherwise
    int getVectorizerFunctions(SmallVectorImpl<Function*> &Functions);

    /// @brief Function for querying the vectorization result widths
    /// @param Widths vector to be filled with packetization width of vectorized
    ///  functions. Order is same as kernels list in the module metadata.
    ///  For non-vectorized kernel, value "1" is inserted.
    /// @returns 0 if successful, non-zero value otherwise
    int getVectorizerWidths(SmallVectorImpl<int> &Widths);

private:
    Vectorizer(); // Do not implement

    /// @brief holds all the "original" (scalar) functions
    funcsVector m_scalarFuncsList; 

    /// @brief List for holding the vectorized kernels pointers
    funcsVector m_targetFunctionsList;
    /// @brief List for holding the vectorized kernels widths
    SmallVector<int, ESTIMATED_NUM_OF_FUNCTIONS> m_targetFunctionsWidth;
    
    /// @brief Pointer to runtime module
    const Module * m_runtimeModule;

    /// @brief Number of kernels in current module
    unsigned m_numOfKernels;

    /// @brief Was current module vectorized
    bool m_isModuleVectorized;

    /// Configuration options
    const OptimizerConfig* m_pConfig;
};

} // namespace intel

#endif // __MAIN_H__


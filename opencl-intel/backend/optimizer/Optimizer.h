/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#pragma once

#include <assert.h>
#include <string>
#include "TLLVMKernelInfo.h"
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

typedef std::pair<llvm::Function *, int> FunctionWidthPair;
typedef std::vector<FunctionWidthPair> FunctionWidthVector;
typedef std::map<const llvm::Function*, TLLVMKernelInfo> KernelsLocalBufferInfoMap;
typedef std::map<std::string, TKernelInfo> KernelsInfoMap;


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

    bool hasBarriers(llvm::Module *pModule);

    void getPrivateMemorySize(std::map<std::string, unsigned int>& bufferStrideMap);

    bool hasUndefinedExternals() const;

    const std::vector<std::string>& GetUndefinedExternals() const;

    void GetVectorizedFunctions(FunctionWidthVector& vector);

    void GetKernelsLocalBufferInfo(KernelsLocalBufferInfoMap& map);
    void GetKernelsInfo(KernelsInfoMap& map);

private:
    
    // hold the collection of passes
    llvm::PassManager m_modulePasses;
    llvm::FunctionPassManager m_funcPasses;
    llvm::Pass* m_vectorizerPass;
    llvm::Pass* m_barrierPass;
    llvm::Module* m_pModule;
    llvm::ModulePass* m_localBuffersPass;
    llvm::ModulePass* m_kernelInfoPass;

    llvm::SmallVector<llvm::Function*, 16> m_vectFunctions;
    llvm::SmallVector<int, 16> m_vectWidths;
    std::map<const llvm::Function*, Intel::OpenCL::DeviceBackend::TLLVMKernelInfo> m_kernelsLocalBufferMap;
    std::vector<std::string> m_undefinedExternalFunctions;
};



}}}



/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  Optimizer.h

\*****************************************************************************/
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
    std::map<const llvm::Function*, TLLVMKernelInfo> m_kernelsLocalBufferMap;
    std::vector<std::string> m_undefinedExternalFunctions;
};



}}}



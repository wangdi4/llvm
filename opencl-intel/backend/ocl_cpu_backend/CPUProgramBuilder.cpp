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

File Name:  CPUProgramBuilder.cpp

\*****************************************************************************/
#include <set>
#include <vector>
#include <string>
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "ProgramBuilder.h"
#include "Optimizer.h"
#include "VecConfig.h"
#include "Program.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "CPUDetect.h"
#include "BuiltinModule.h"
#include "exceptions.h"
#include "BuiltinModuleManager.h"
#include "StaticObjectLoader.h"
#include "plugin_manager.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "CPUProgramBuilder.h"
#include "CPUJITContainer.h"
#include "CompilationUtils.h"
#include "MetaDataApi.h"
#include "CPUBlockToKernelMapper.h"

using std::string;

namespace Intel { namespace OpenCL { namespace DeviceBackend {


namespace Utils
{
/**
 * Returns the true if the given function name is a kernel function in the given module
 */
bool IsKernel(llvm::Module* pModule, const char* szFuncName)
{
    MetaDataUtils mdUtils(pModule);

    for( MetaDataUtils::KernelsList::const_iterator i = mdUtils.begin_Kernels(), e = mdUtils.end_Kernels(); i != e; ++i )
    {
        llvm::Function *pFuncVal = NULL;
        // Obtain kernel function from annotation
        if( (*i)->isFunctionHasValue() )
        {
            pFuncVal = (*i)->getFunction();
        }

        //TODO: check stripPointerCasts()
        if ( NULL == pFuncVal )
        {
            continue;   // Not a function pointer
        }

        if ( !pFuncVal->getName().compare(szFuncName) )
        {
            return true;
        }
    }

    // Function not found
    return false;
}

}
CPUProgramBuilder::CPUProgramBuilder(IAbstractBackendFactory* pBackendFactory, const ICompilerConfig& config):
    ProgramBuilder(pBackendFactory, config),
    m_compiler(config)
{
}

CPUProgramBuilder::~CPUProgramBuilder()
{
}

Kernel* CPUProgramBuilder::CreateKernel(llvm::Function* pFunc, const std::string& funcName, KernelProperties* pProps) const
{
    std::vector<cl_kernel_argument> arguments;
    std::vector<unsigned int>       memoryArguments;

    // TODO : consider separating into a different analisys pass
    CompilationUtils::parseKernelArguments(pFunc->getParent() /* = pModule */,  pFunc, arguments, memoryArguments);

    return m_pBackendFactory->CreateKernel( funcName, arguments, memoryArguments, pProps );
}

KernelSet* CPUProgramBuilder::CreateKernels(Program* pProgram,
                                    llvm::Module* pModule,
                                    ProgramBuildResult& buildResult) const
{
    std::auto_ptr<KernelSet> spKernels( new KernelSet );
    MetaDataUtils mdUtils(pModule);

    MetaDataUtils::KernelsList::const_iterator i = mdUtils.begin_Kernels();
    MetaDataUtils::KernelsList::const_iterator e = mdUtils.end_Kernels();

    for ( ; i != e; ++i)
    {
        // Obtain kernel function from annotation
        llvm::Function *pFunc = (*i)->getFunction(); // TODO: stripPointerCasts());
        KernelInfoMetaDataHandle kimd = mdUtils.getKernelsInfoItem(pFunc);
        // Obtain kernel wrapper function from metadata info
        llvm::Function *pWrapperFunc = kimd->getKernelWrapper(); //TODO: stripPointerCasts());

        // Create a kernel and kernel JIT properties
        std::auto_ptr<KernelProperties> spKernelProps( CreateKernelProperties( pProgram,
                                                                               pFunc,
                                                                               buildResult));

        // get the vector size used to generate the function
        unsigned int vecSize = kimd->isVectorizedWidthHasValue() ? kimd->getVectorizedWidth() : 1;
        spKernelProps->SetMinGroupSizeFactorial(vecSize);

        std::auto_ptr<KernelJITProperties> spKernelJITProps( CreateKernelJITProperties( vecSize ));

        std::auto_ptr<Kernel> spKernel( CreateKernel( pFunc,
                                                      pWrapperFunc->getName().str(),
                                                      spKernelProps.get()));

        // We want the JIT of the wrapper function to be called
        AddKernelJIT(static_cast<CPUProgram*>(pProgram),
                     spKernel.get(),
                     pModule,
                     pWrapperFunc,
                     spKernelJITProps.release());


        //TODO (AABOUD): is this redundant code?
        const llvm::Type *vTypeHint = NULL; //pFunc->getVectTypeHint(); //TODO: Read from metadata (Guy)
        bool dontVectorize = false;

        if( NULL != vTypeHint)
        {
            //currently if the vector_type_hint attribute is set
            //we types that vector length is below 4, vectorizer restriction
            const llvm::VectorType* pVect = llvm::dyn_cast<llvm::VectorType>(vTypeHint);
            if( ( NULL != pVect) && pVect->getNumElements() >= 4)
            {
                dontVectorize = true;
            }
        }

        //Need to check if Vectorized Kernel Value exists, it is not guaranteed that
        //Vectorized is running in all scenarios.
        if (kimd->isVectorizedKernelHasValue())
        {
            Function *pVecFunc = kimd->getVectorizedKernel();
            assert(!(spKernelProps->GetJitCreateWIids() && pVecFunc) &&
                "if the vector kernel is inlined the entry of the vector kernel should be NULL");
            if(NULL != pVecFunc && !dontVectorize)
            {
                KernelInfoMetaDataHandle vkimd = mdUtils.getKernelsInfoItem(pVecFunc);
                // Obtain kernel wrapper function from metadata info
                llvm::Function *pWrapperVecFunc = vkimd->getKernelWrapper(); //TODO: stripPointerCasts());
                //Update vecSize according to vectorWidth of vectorized function
                vecSize = vkimd->getVectorizedWidth();
                // Create the vectorized kernel - no need to pass argument list here
                std::auto_ptr<KernelJITProperties> spVKernelJITProps(CreateKernelJITProperties(vecSize));
                spKernelProps->SetMinGroupSizeFactorial(vecSize);
                AddKernelJIT(static_cast<CPUProgram*>(pProgram),
                              spKernel.get(),
                              pModule,
                              pWrapperVecFunc,
                              spVKernelJITProps.release());
            }
        }
        if ( dontVectorize )
        {
            buildResult.LogS() << "Vectorization of kernel <" << spKernel->GetKernelName() << "> was disabled by the developer\n";
        }
        else if (vecSize <= 1)
        {
            buildResult.LogS() << "Kernel <" << spKernel->GetKernelName() << "> was not vectorized\n";
        }
        else
        {
            buildResult.LogS() << "Kernel <" << spKernel->GetKernelName() << "> was successfully vectorized\n";
        }
#ifdef OCL_DEV_BACKEND_PLUGINS
        // Notify the plugin manager
        m_pluginManger.OnCreateKernel(pProgram, spKernel.get(), pFunc);
#endif
        spKernels->AddKernel(spKernel.release());
        spKernelProps.release();
    }
    //LLVMBackend::GetInstance()->m_logger->Log(Logger::DEBUG_LEVEL, L"Iterating completed");

    buildResult.LogS() << "Done.";
    //LLVMBackend::GetInstance()->m_logger->Log(Logger::INFO_LEVEL, L"Exit");
    return spKernels.release();
}


void CPUProgramBuilder::AddKernelJIT(CPUProgram* pProgram, Kernel* pKernel, llvm::Module* pModule,
                                     llvm::Function* pFunc, KernelJITProperties* pProps) const
{
    IKernelJITContainer* pJIT = new CPUJITContainer( pProgram->GetPointerToFunction(pFunc),
                                                     pFunc,
                                                     pModule,
                                                     pProps);
    pKernel->AddKernelJIT( pJIT );
}

void CPUProgramBuilder::PostOptimizationProcessing(Program* pProgram, llvm::Module* spModule, const ICLDevBackendOptions* pOptions) const
{
    char*  pInjectedObjStart = NULL;
    size_t injectedObjSize;

    // Check if we are going to do injection
    if (pOptions
        && pOptions->GetValue(CL_DEV_BACKEND_OPTION_INJECTED_OBJECT, &pInjectedObjStart, &injectedObjSize)
        && pInjectedObjStart != NULL)
    {
        std::auto_ptr<StaticObjectLoader> pObjectLoader(new StaticObjectLoader());
        // Build the MemoryBuffer object from the supplied options
        std::auto_ptr<llvm::MemoryBuffer> pInjectedObj(
            llvm::MemoryBuffer::getMemBuffer( llvm::StringRef(pInjectedObjStart, injectedObjSize)) );

        pObjectLoader->addPreCompiled(spModule, pInjectedObj.release());
        // Add the injected object to the execution engine cache
        CPUProgram* pCPUProgram = static_cast<CPUProgram*>(pProgram);
        pCPUProgram->GetExecutionEngine()->setObjectCache(pObjectLoader.release());
    }

}

IBlockToKernelMapper * CPUProgramBuilder::CreateBlockToKernelMapper(Program* pProgram, const llvm::Module* pModule) const
{
    return new CPUBlockToKernelMapper(pProgram, pModule);
}


void CPUProgramBuilder::PostBuildProgramStep(Program* pProgram, llvm::Module* pModule,
  const ICLDevBackendOptions* pOptions) const 
{
  assert(pProgram && pModule && "inputs are NULL");

  // create block to kernel mapper
  IBlockToKernelMapper * pMapper = CreateBlockToKernelMapper(pProgram, pModule);
  assert(pMapper && "IBlockToKernelMapper object is NULL");
  assert(!pProgram->GetRuntimeService().isNull() && "RuntimeService in Program is NULL");
  // set in RuntimeService new BlockToKernelMapper object
  pProgram->GetRuntimeService()->SetBlockToKernelMapper(pMapper);
}

}}} // namespace

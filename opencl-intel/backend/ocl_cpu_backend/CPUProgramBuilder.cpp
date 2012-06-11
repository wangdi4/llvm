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
#include "plugin_manager.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetData.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Argument.h"
#include "llvm/Type.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#include "llvm/Instruction.h"
#include "llvm/LLVMContext.h"
#include "CPUProgramBuilder.h"
#include "CPUJITContainer.h"
#include "CompilationUtils.h"

using std::string;

namespace Intel { namespace OpenCL { namespace DeviceBackend {


namespace Utils 
{
/**
 * Returns the true if the given function name is a kernel function in the given module
 */
bool IsKernel(llvm::Module* pModule, const char* szFuncName)
{
    llvm::NamedMDNode *pOpenCLMetadata = pModule->getNamedMetadata("opencl.kernels");
    if ( !pOpenCLMetadata ) {
      //Module contains no MetaData, thus it contains no kernels
      return false;
    }

    for (unsigned i = 0, e = pOpenCLMetadata->getNumOperands(); i != e; ++i) 
    {
        // Obtain kernel function from annotation
        llvm::MDNode *elt = pOpenCLMetadata->getOperand(i);
        llvm::Function *pFuncVal = llvm::dyn_cast<llvm::Function>(elt->getOperand(0)->stripPointerCasts());
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

    // TODO : consider separating into a different analisys pass
    CompilationUtils::parseKernelArguments(pFunc->getParent() /* = pModule */,  pFunc, arguments);

    return m_pBackendFactory->CreateKernel( funcName, arguments, pProps );
}

KernelSet* CPUProgramBuilder::CreateKernels(Program* pProgram,
                                    llvm::Module* pModule, 
                                    ProgramBuildResult& buildResult) const
{
    std::auto_ptr<KernelSet> spKernels( new KernelSet );

    llvm::NamedMDNode *pModuleMetadata = pModule->getNamedMetadata("opencl.kernels");
    llvm::NamedMDNode *WrapperMD = pModule->getOrInsertNamedMetadata("opencl.wrappers");
    if ( !pModuleMetadata ) 
    {
      //Module contains no MetaData, thus it contains no kernels
      return spKernels.release();
    }

    std::vector<FunctionWidthPair>::const_iterator vecIter = buildResult.GetFunctionsWidths().begin();

    for (unsigned i = 0, e = pModuleMetadata->getNumOperands(); i != e; ++i) 
    {
        // Obtain kernel function from annotation
        llvm::MDNode *elt = pModuleMetadata->getOperand(i);
        llvm::MDNode *welt = WrapperMD->getOperand(i);
        llvm::Function *pFunc = llvm::dyn_cast<llvm::Function>(elt->getOperand(0)->stripPointerCasts());
        // The wrapper function that receives a single buffer as argument is the last node in the metadata
        llvm::Function *pWrapperFunc = llvm::dyn_cast<llvm::Function>(
                                       welt->getOperand(0)->stripPointerCasts());
        if ( NULL == pFunc )
        {
            continue;   // Not a function pointer
        }

        // Create a kernel and kernel JIT properties 
        std::auto_ptr<KernelProperties> spKernelProps( CreateKernelProperties( pProgram, 
                                                                               pFunc, 
                                                                                        pWrapperFunc,
                                                                               buildResult));
        unsigned int vecSize = 1;
        
        if( spKernelProps->GetJitCreateWIids() && vecIter != buildResult.GetFunctionsWidths().end())
        {
            vecSize = vecIter->second;
            spKernelProps->SetMinGroupSizeFactorial(vecSize);
        }

        std::auto_ptr<KernelJITProperties> spKernelJITProps( CreateKernelJITProperties( vecSize ));

        std::auto_ptr<Kernel>           spKernel( CreateKernel( pFunc, 
                                                                pWrapperFunc->getName().str(), 
                                                                spKernelProps.get()));

        // We want the JIT of the wrapper function to be called
        AddKernelJIT(static_cast<CPUProgram*>(pProgram), 
                     spKernel.get(), 
                     pModule, 
                     pWrapperFunc, 
                     spKernelJITProps.release());


        // Check if vectorized kernel present
        if( !buildResult.GetFunctionsWidths().empty())
        {
            assert(vecIter != buildResult.GetFunctionsWidths().end());
            assert( !(spKernelProps->GetJitCreateWIids() && vecIter->first) &&
                "if the vector kernel is inlined the entry of the vector kernel should be NULL");
            
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
            
            if(NULL != vecIter->first && !dontVectorize)
            {
                // Create the vectorized kernel - no need to pass argument list here
                std::auto_ptr<KernelJITProperties> spVKernelJITProps(CreateKernelJITProperties(vecIter->second));
                spKernelProps->SetMinGroupSizeFactorial(vecIter->second);
                AddKernelJIT(static_cast<CPUProgram*>(pProgram), spKernel.get(), pModule, 
                             vecIter->first, spVKernelJITProps.release());
                
            }
            if ( dontVectorize )
            {
                buildResult.LogS() << "Vectorization of kernel <" << spKernel->GetKernelName() << "> was disabled by the developer\n";
            }
            else if ( vecIter->second <= 1)
            {
                buildResult.LogS() << "Kernel <" << spKernel->GetKernelName() << "> was not vectorized\n";
            }
            else 
            {
                buildResult.LogS() << "Kernel <" << spKernel->GetKernelName() << "> was successfully vectorized\n";
            }
            vecIter++;
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
                                           pProgram->GetExecutionEngine(),
                                           pProps);
    pKernel->AddKernelJIT( pJIT );
}


}}} // namespace

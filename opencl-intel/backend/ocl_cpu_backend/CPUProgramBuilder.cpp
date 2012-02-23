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
#include "llvm/ExecutionEngine/JIT.h"
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
extern "C" void fillNoBarrierPathSet(llvm::Module *M, std::set<std::string>& noBarrierPath);

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
CPUProgramBuilder::CPUProgramBuilder(IAbstractBackendFactory* pBackendFactory, const CompilerConfig& config):
    ProgramBuilder(pBackendFactory),
    m_compiler(config),
    m_config(config)
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

KernelSet* CPUProgramBuilder::CreateKernels( const Program* pProgram,
                                    llvm::Module* pModule, 
                                    ProgramBuildResult& buildResult) const
{
    buildResult.LogS() << "Build started\n";
    std::auto_ptr<KernelSet> spKernels( new KernelSet );

    llvm::NamedMDNode *pModuleMetadata = pModule->getNamedMetadata("opencl.kernels");
    llvm::NamedMDNode *WrapperMD = pModule->getOrInsertNamedMetadata("opencl.wrappers");
    if ( !pModuleMetadata ) {
      //Module contains no MetaData, thus it contains no kernels
      return spKernels.release();
    }

    std::vector<FunctionWidthPair>::const_iterator vecIter = buildResult.GetFunctionsWidths().begin();
    std::set<std::string> noBarrier;
    fillNoBarrierPathSet(pModule, noBarrier);

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

        // Obtain parameters definition
        //llvm::MDString *pFuncArgs = llvm::dyn_cast<llvm::MDString>(elt->getOperand(4));
        //if (NULL == pFuncArgs)
        //{
        //    throw Exceptions::CompilerException( "Invalid argument's map", CL_DEV_BUILD_ERROR);
        //}

        // Create a kernel and kernel JIT properties 
        std::auto_ptr<KernelProperties> spKernelProps( CreateKernelProperties( pProgram, pFunc, buildResult.GetKernelsInfo()[pFunc]));
        std::auto_ptr<KernelJITProperties> spKernelJITProps( CreateKernelJITProperties( pModule,
                                                                                        pWrapperFunc,
                                                                                        buildResult.GetKernelsInfo()[pFunc]));
        // Check whether the kernel creates WI ids by itself (work group loops were not created by barrier)
        // This also means that this a 1-sise Jit (no vector kernel)
        std::string wrapperName = pWrapperFunc->getNameStr();
        bool bJitCreateWIids = noBarrier.count(wrapperName);
        spKernelProps->SetJitCreateWIids(bJitCreateWIids);

        // Private memory size contains the max size between
        // the needed size for scalar and needed size for vectorized versions.
        unsigned int privateMemorySize = buildResult.GetPrivateMemorySize()[pWrapperFunc->getNameStr()];
        // TODO: This is workaround till the SDK hanlde case of zero private memory size!
        privateMemorySize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(std::max<unsigned int>(1, privateMemorySize));
        spKernelProps->SetPrivateMemorySize( privateMemorySize );

        // Create a kernel 
        std::auto_ptr<Kernel>           spKernel( CreateKernel( pFunc, 
                                                                pWrapperFunc->getName().str(), 
                                                                spKernelProps.release()));

        // We want the JIT of the wrapper function to be called
        AddKernelJIT( spKernel.get(), pModule, pWrapperFunc, spKernelJITProps.release());


        // Check if vectorized kernel present

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

        if( !buildResult.GetFunctionsWidths().empty())
        {
            assert(vecIter != buildResult.GetFunctionsWidths().end());
            assert( !(bJitCreateWIids && vecIter->first) &&
                "if the vector kernel is inlined the entry of the vector kernel should be NULL");
            if(NULL != vecIter->first && !dontVectorize)
            {
                // Create the vectorized kernel - no need to pass argument list here
                std::auto_ptr<KernelJITProperties>  spVKernelProps(CreateKernelJITProperties(pModule, 
                                                                                      vecIter->first,
                                                                                      buildResult.GetKernelsInfo()[vecIter->first]));
                spVKernelProps->SetVectorSize(vecIter->second);
                AddKernelJIT( spKernel.get(), pModule, vecIter->first, spVKernelProps.release());
                
            }
            if ( dontVectorize )
            {
                buildResult.LogS() << "Vectorization of kernel <" << spKernel->GetKernelName() << "> was disabled by the developer\n";
            }
            else if ( 0 == vecIter->second )
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
        PluginManager::Instance().OnCreateKernel(pProgram, spKernel.get(), pFunc);
#endif
        spKernels->AddKernel(spKernel.release());
    }
    //LLVMBackend::GetInstance()->m_logger->Log(Logger::DEBUG_LEVEL, L"Iterating completed");
    
    buildResult.LogS() << "Done.";
    //LLVMBackend::GetInstance()->m_logger->Log(Logger::INFO_LEVEL, L"Exit");
    return spKernels.release();
}

KernelJITProperties* CPUProgramBuilder::CreateKernelJITProperties(llvm::Module* pModule, 
                                                         llvm::Function* pFunc,
                                                         const TLLVMKernelInfo& info) const
{
    KernelJITProperties* pProps = m_pBackendFactory->CreateKernelJITProperties();
    pProps->SetUseVTune(m_config.GetUseVTune());
    return pProps;
}

void CPUProgramBuilder::AddKernelJIT( Kernel* pKernel, llvm::Module* pModule, llvm::Function* pFunc, KernelJITProperties* pProps) const
{
    IKernelJITContainer* pJIT = new CPUJITContainer( m_compiler.GetPointerToFunction(pFunc),
                                           pFunc,
                                           pModule,
                                           &m_compiler,
                                           pProps);
    pKernel->AddKernelJIT( pJIT );
}


}}} // namespace

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

File Name:  MICProgramBuilder.cpp

\*****************************************************************************/

#include <vector>
#include <string>
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "ProgramBuilder.h"
#include "Optimizer.h"
#include "VecConfig.h"
#include "MICProgram.h"
#include "MICKernel.h"
#include "MICKernelProperties.h"
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
#include "llvm/Target/TargetSelect.h"
#include "llvm/Target/TargetData.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/MICJITEngine/MICCodeGenerationEngine.h"
#include "llvm/MICJITEngine/ModuleJITHolder.h"
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
#include "VTune/JITProfiling.h"
#include "MICProgramBuilder.h"
#include "ModuleJITHolder.h"
#include "MICJITContainer.h"
#include "CompilationUtils.h"

using std::string;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/*
 * Utility methods
 */
namespace Utils 
{
 
}

MICProgramBuilder::MICProgramBuilder(IAbstractBackendFactory* pBackendFactory, const CompilerConfig& config):
    ProgramBuilder(pBackendFactory),
    m_compiler(config),
    m_config(config)
{
}

MICProgramBuilder::~MICProgramBuilder()
{
}



MICKernel* MICProgramBuilder::CreateKernel(llvm::Function* pFunc, const std::string& funcName, const std::string& args, KernelProperties* pProps)
{
    std::vector<cl_kernel_argument> arguments;

    // TODO : consider separating into a different analisys pass
    CompilationUtils::parseKernelArguments(pFunc->getParent() /* = pModule */,  pFunc, args, arguments);

    return static_cast<MICKernel*>(m_pBackendFactory->CreateKernel( funcName, arguments, pProps ));
}

MICKernelJITProperties* MICProgramBuilder::CreateKernelJITProperties(llvm::Module* pModule, 
                                                         llvm::Function* pFunc,
                                                         const TLLVMKernelInfo& info)
{
    unsigned int stackSize = 0;

  // The following code roughly assume stack size needed for calls to
  // external functions (like printf, asyncopy and barrier)
  // TODO: Get rid of this ugly workaround by solving CSSD100006552
    if ( /*info.bDbgPrint*/ false )
    {
#if (!defined(_M_X64) && !defined(__LP64__))
        stackSize = std::max<unsigned int>(1024*32, stackSize);    // We need large stack here
#else
        stackSize = std::max<unsigned int>(1024*64, stackSize);    // We need large stack here
#endif
    } else
    {
        if ( /*info.bAsynCopy*/ false )
        {
#if (!defined(_M_X64) && !defined(__LP64__))
            stackSize = std::max<unsigned int>(1024*16, stackSize);
#else
            stackSize = std::max<unsigned int>(1024*32, stackSize);
#endif
        }
    }
    // THIS PROPORTY (STACK SIZE) NEED TO BE REMOVED ! NOT NEEDED

    // compile the function if needed - just to be able to get its stack size later
    void * pFuncAddr = NULL; //m_pExecEngine->getPointerToFunction(pFunc);
    uint64_t jitStackSize = 0; //m_pExecEngine->getJitFunctionStackSize(pFunc);
    //assert(((uint64_t)-1) != jitStackSize && "Check that the pFunc was actually compiled succesfully");
    
    
    stackSize = std::max<unsigned int>(CPU_DEV_MIN_WI_PRIVATE_SIZE, stackSize);
    // JIT generated get info from function
    // Deciphering of the magic numbers in length expression below: 
    // 64 stands for guard, 32 stands for Win64-ABI required 32-byte buffer in the caller stack frame
    stackSize = (unsigned int)(stackSize + jitStackSize + 64 + 32);

    unsigned int uiVTuneId = -1;
    if( m_config.GetUseVTune() && iJIT_SAMPLING_ON == iJIT_IsProfilingActive() )
    {
        uiVTuneId = iJIT_GetNewMethodID();

        iJIT_Method_Load ML;

        memset(&ML, 0, sizeof(iJIT_Method_Load));

        //Parameters
        ML.method_id = uiVTuneId;                                     // uniq method ID - can be any uniq value, (such as the mb)
        ML.method_name = const_cast<char *>(pFunc->getName().data()); // method name (can be with or without the class and signature, in any case the class name will be added to it)
        ML.method_load_address = pFuncAddr;                           // virtual address of that method  - This determines the method range for the iJVM_EVENT_TYPE_ENTER/LEAVE_METHOD_ADDR events
        // TODO: DEAL WITH THIS LATER
        ML.method_size = 0 ; //m_pExecEngine->getJitFunctionSize(pFunc);    // Size in memory - Must be exact

        
#if 0
        // Used of DebugInfo
        // Constants in this example
        ML.line_number_size = 0;        // Line Table size in number of entries - Zero if none
        ML.line_number_table = NULL;    // Pointer to the begining of the line numbers info array
        ML.class_id = 0;                // uniq class ID
        ML.class_file_name = NULL;      // class file name 
        ML.source_file_name = NULL;     // source file name
        ML.user_data = NULL;            // bits supplied by the user for saving in the JIT file...
        ML.user_data_size = 0;          // the size of the user data buffer
        ML.env = iJDE_JittingAPI;
#endif /* 0 */
        iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, &ML);
    }


    MICKernelJITProperties* pProps = static_cast<MICKernelJITProperties*>(m_pBackendFactory->CreateKernelJITProperties());
    pProps->SetUseVTune(m_config.GetUseVTune());
    pProps->SetVTuneId(uiVTuneId);
    pProps->SetStackSize(stackSize);
    return pProps;
}

KernelSet* MICProgramBuilder::CreateKernels( const Program* pProgram,
                                    llvm::Module* pModule, 
                                    ProgramBuildResult& buildResult)
{
    buildResult.LogS() << "Build started\n";
    std::auto_ptr<KernelSet> spKernels( new KernelSet );

    llvm::NamedMDNode *pModuleMetadata = pModule->getNamedMetadata("opencl.kernels");
    if ( !pModuleMetadata ) {
      //Module contains no MetaData, thus it contains no kernels
      return spKernels.release();
    }

    std::vector<FunctionWidthPair>::const_iterator vecIter = buildResult.GetFunctionsWidths().begin();
    for (unsigned i = 0, e = pModuleMetadata->getNumOperands(); i != e; ++i) 
    {
        // Obtain kernel function from annotation
        llvm::MDNode *elt = pModuleMetadata->getOperand(i);
        llvm::Function *pFunc = llvm::dyn_cast<llvm::Function>(elt->getOperand(0)->stripPointerCasts());
        // The wrapper function that receives a single buffer as argument is the last node in the metadata 
        llvm::Function *pWrapperFunc = llvm::dyn_cast<llvm::Function>( 
                                       elt->getOperand(elt->getNumOperands() - 1)->stripPointerCasts()); 

        if ( NULL == pFunc )
        {
            continue;   // Not a function pointer
        }

        // Obtain parameters definition
        llvm::MDString *pFuncArgs = llvm::dyn_cast<llvm::MDString>(elt->getOperand(4));
        if (NULL == pFuncArgs)
        {
            throw Exceptions::CompilerException( "Invalid argument's map", CL_DEV_BUILD_ERROR);
        }

        // Create a kernel and kernel JIT properties 
        std::auto_ptr<KernelProperties> spMICKernelProps( CreateKernelProperties( pProgram, elt, buildResult.GetKernelsInfo()[pFunc]));
        std::auto_ptr<MICKernelJITProperties> spKernelJITProps( CreateKernelJITProperties( pModule,
                                                                                        pWrapperFunc,
                                                                                        buildResult.GetKernelsInfo()[pFunc]));

        // TODO: This is workaround till the SDK hanlde case of zero private memory size!
        spMICKernelProps->SetPrivateMemorySize(ADJUST_SIZE_TO_MAXIMUM_ALIGN(std::max<unsigned int>(1, buildResult.GetPrivateMemorySize())));

        // Create a kernel 
        std::auto_ptr<MICKernel>           spKernel( CreateKernel( pFunc, 
                                                                pWrapperFunc->getName().str(),
                                                                pFuncArgs->getString().str(),
                                                                spMICKernelProps.release()));
        spKernel->SetKernelID(i);

        AddKernelJIT( static_cast<const MICProgram*>(pProgram), spKernel.get(), pModule, pWrapperFunc, spKernelJITProps.release());


        // Check if vectorized kernel present

        const llvm::Type *vTypeHint = NULL; //pFunc->getVectTypeHint(); //TODO: R namespace micead from metadata (Guy)
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
            if(NULL != vecIter->first && !dontVectorize)
            {
                //
                // Create the vectorized kernel - no need to pass argument list here
                std::auto_ptr<MICKernelJITProperties>  spVKernelProps(CreateKernelJITProperties(pModule, 
                                                                                      vecIter->first,
                                                                                      buildResult.GetKernelsInfo()[vecIter->first]));
                spVKernelProps->SetVectorSize(vecIter->second);
                AddKernelJIT( static_cast<const MICProgram*>(pProgram), spKernel.get(), pModule, vecIter->first, spVKernelProps.release());

                buildResult.LogS() << "Kernel <" << spKernel->GetKernelName() << "> was successfully vectorized\n";
            }
            if ( NULL == vecIter->first )
            {
                buildResult.LogS() << "Kernel <" << spKernel->GetKernelName() << "> was not vectorized\n";
            }
            else if ( dontVectorize )
            {
                buildResult.LogS() << "Vectorization of kernel <" << spKernel->GetKernelName() << "> was disabled by the developer\n";
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

void MICProgramBuilder::AddKernelJIT( const MICProgram* pProgram, Kernel* pKernel, llvm::Module* pModule, llvm::Function* pFunc, MICKernelJITProperties* pProps)
{
    IKernelJITContainer* pJIT = new MICJITContainer( pProgram->GetModuleJITHolder(),
                                           (unsigned long long int)pFunc,
                                           pProps);
    pKernel->AddKernelJIT( pJIT );
}

void MICProgramBuilder::PostOptimizationProcessing(Program* pProgram, llvm::Module* spModule)
{
    assert(spModule && "Invalid module for post optimization processing.");
    ModuleJITHolder* pModuleJIT = new ModuleJITHolder(); 
    std::auto_ptr<const llvm::ModuleJITHolder> spMICModuleJIT(m_compiler.GetModuleHolder(*spModule));

    // populate the Module JIT
    pModuleJIT->SetJITCodeSize(spMICModuleJIT->getJITCodeSize());
    pModuleJIT->SetJITCodeStartPoint(spMICModuleJIT->getJITCodeStartPoint());
    pModuleJIT->SetJITBufferPointer(spMICModuleJIT->getJITBufferPointer());
    pModuleJIT->SetJITAlignment(spMICModuleJIT->getJITCodeAlignment());

    for(llvm::KernelMap::const_iterator 
        it = spMICModuleJIT->getKernelMap().begin();
        it != spMICModuleJIT->getKernelMap().end();
        it++)
    {
        KernelInfo kernelInfo;
        kernelInfo.kernelOffset = it->second.offset;
        kernelInfo.kernelSize   = it->second.size;
        KernelID kernelID = (const KernelID)it->first;
        pModuleJIT->RegisterKernel(kernelID, kernelInfo);
}
    static_cast<MICProgram*>(pProgram)->SetModuleJITHolder(pModuleJIT);
}

}}} // namespace


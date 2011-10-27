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

File Name:  CPUCompiler.cpp

\*****************************************************************************/

#include <vector>
#include <string>
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "Compiler.h"
#include "Optimizer.h"
#include "VecConfig.h"
#include "Program.h"
#include "Kernel.h"
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
#include "VTune/JITProfiling.h"
#include "CPUCompiler.h"
#include "CPUJITContainer.h"
#include "CompilationUtils.h"

using std::string;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/*
 *  Constants
 */
extern const char* CPU_ARCH_AUTO;

/*
 * Utility methods
 */
namespace Utils 
{
 
/**
 * Class used to block the final termination
 *
 * The problem is that the compiler destruction out of the application
 * main() boundaries could be problematic and we would like to better
 * leak the memory then try to destroy the class normally, risking the
 * access violation or yet worse problems. 
 * So we introduce some late initialization static object (TerminationBlocker)
 * the will be initialized last and de-initialized first in LIFO order 
 *
 * Upon termination such static object will set its static state to
 * 'triggered' state, so that regular classes termination sequence could
 * inspect this state and abort its termination sequences
 */
// TODO[MA]: not  clear what is this !?!
class CPUTerminationBlocker
{
public:
    ~CPUTerminationBlocker() { s_released = true; }
    static bool IsReleased() { return s_released; }
private:
    static bool s_released;
};
bool CPUTerminationBlocker::s_released = false;

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

/**
 * Return the CPU identifier (from CPUDetect enumeration) given the CPU name.
 * CPU Name may be equal to 'auto', in this case the CPU detection will be performed
 */
Intel::ECPU GetOrDetectCpuId(const std::string& cpuArch)
{
    Intel::ECPU cpuId = ( cpuArch == CPU_ARCH_AUTO ) ? Utils::CPUDetect::GetInstance()->GetCPUId()
                                                     : Utils::CPUDetect::GetInstance()->GetCPUByName(cpuArch.c_str());

    if( Intel::CPU_LAST == cpuId )
    {
        throw Exceptions::CompilerException("Unsupported CPU Architecture");
    }

    return cpuId;
}

void SplitString( const std::string& s, const char* d, std::vector<std::string>& v )
{
    llvm::StringRef sr(s);
    llvm::SmallVector<llvm::StringRef,2> sv;

    sr.split(sv, d, -1, false);
    std::copy( sv.begin(), sv.end(), std::back_inserter( v ));
}

unsigned int SelectCpuFeatures( unsigned int cpuId, const std::vector<std::string>& forcedFeatures)
{
    unsigned int  cpuFeatures = CFS_SSE2;

    // Add standard features 
    if( cpuId >= (unsigned int)Utils::CPUDetect::GetInstance()->GetCPUByName("corei7") )
    {
        cpuFeatures |= CFS_SSE41 | CFS_SSE42;
    }

    if( cpuId >= (unsigned int)Utils::CPUDetect::GetInstance()->GetCPUByName("sandybridge"))
    {
        cpuFeatures |= CFS_AVX1;
    }

    if( cpuId >= (unsigned int)Utils::CPUDetect::GetInstance()->GetCPUByName("haswell"))
    {
        cpuFeatures |= CFS_AVX1;
        cpuFeatures |= CFS_AVX2;
    }

    // Add forced features
    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "+sse41" ) != forcedFeatures.end())
    {
        cpuFeatures |= CFS_SSE41;
    }

    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "+avx2" ) != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX2;
        cpuFeatures |= CFS_AVX1;
    }

    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "+avx" ) != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX1;
    }

    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "-sse41" ) != forcedFeatures.end())
    {
        cpuFeatures &= ~(CFS_SSE41 | CFS_SSE42);
    }

    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "-avx2" ) != forcedFeatures.end())
    {
        cpuFeatures &= ~CFS_AVX2;
    }
    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "-avx" ) != forcedFeatures.end())
    {
        cpuFeatures &= ~CFS_AVX1;
        cpuFeatures &= ~CFS_AVX2;
    }

    return cpuFeatures;

}

}

CPUCompiler::CPUCompiler(const CompilerConfig& config):
    Compiler(config),
    m_pBuiltinModule(NULL),
    m_pExecEngine(NULL)
{
    // WORKAROUND!!! See the notes in TerminationBlocker description
    static Utils::CPUTerminationBlocker blocker;

    SelectCpu( config.GetCpuArch(), config.GetCpuFeatures());

    // Initialize the BuiltinModule
    BuiltinLibrary* pLibrary = BuiltinModuleManager::GetInstance()->GetOrLoadCPULibrary(m_selectedCpuId, m_selectedCpuFeatures);
    std::auto_ptr<llvm::Module> spModule( CreateRTLModule(pLibrary) );
    m_pBuiltinModule = new BuiltinModule( spModule.get());

    // Initialize the ExecutionEngine
    // ExecutionEngine will own the pointer to the RT module, so we are releasing it here
    m_pExecEngine = CreateExecutionEngine( spModule.release() );
}

CPUCompiler::~CPUCompiler()
{
    // WORKAROUND!!! See the notes in TerminationBlocker description
    if( Utils::CPUTerminationBlocker::IsReleased() )
        return;

    delete m_pBuiltinModule;
    delete m_pExecEngine;
}

void CPUCompiler::SelectCpu( const std::string& cpuName, const std::string& cpuFeatures )
{
    
    m_selectedCpuId = Utils::GetOrDetectCpuId( cpuName );
    Utils::SplitString( cpuFeatures, ",", m_forcedCpuFeatures);
   
    // if we autodetected the SandyBridge CPU and a user didn't forced us to use AVX256 - disable it if not supported
    if( CPU_ARCH_AUTO == cpuName)
    {
        if( Intel::CPU_SANDYBRIDGE == m_selectedCpuId)
        {
            if( std::find( m_forcedCpuFeatures.begin(), m_forcedCpuFeatures.end(), "+avx" ) == m_forcedCpuFeatures.end() )
            {
                // check if the OS is AVX ready - if not, need to disable AVX at all
                bool AVXReadyOS = ((Intel::CFS_AVX1) & Utils::CPUDetect::GetInstance()->GetCPUFeatureSupport()) != 0;

                // if the OS is not AVX ready so disable AVX code generation
                if (false == AVXReadyOS)
                {
                    m_forcedCpuFeatures.push_back("-avx");
                }
            }
        }
    }

    m_selectedCpuFeatures = Utils::SelectCpuFeatures( m_selectedCpuId, m_forcedCpuFeatures );
}

llvm::ExecutionEngine* CPUCompiler::CreateExecutionEngine(llvm::Module* pModule )
{
    // Leaving MArch blank implies using auto-detect
    llvm::StringRef MCPU  = Utils::CPUDetect::GetInstance()->GetCPUName((Intel::ECPU)m_selectedCpuId);
    llvm::StringRef MArch = "";

    string strErr;
    bool AllocateGVsWithCode = true;  

    llvm::ExecutionEngine* pExecEngine = llvm::EngineBuilder(pModule)
                  .setEngineKind(llvm::EngineKind::JIT)
                  .setErrorStr(&strErr)
                  .setOptLevel(llvm::CodeGenOpt::Default)
                  .setAllocateGVsWithCode(AllocateGVsWithCode)
                  .setCodeModel(llvm::CodeModel::Default)
                  .setMArch(MArch)
                  .setMCPU(MCPU)
                  .setMAttrs(m_forcedCpuFeatures)
                  .create();

    if ( NULL == pExecEngine )
    {
        throw Exceptions::CompilerException("Failed to create execution engine");
    }
    
    return pExecEngine;
}

//TODO: replace this macro with CompilationUtils::NUMBER_IMPLICIT_ARGS
const int KRNL_NUM_CONST_ARGS = 9;

Kernel* CPUCompiler::CreateKernel(llvm::Function* pFunc, const std::string& funcName, const std::string& args, KernelProperties* pProps)
{
    std::vector<cl_kernel_argument> arguments;

    // TODO : consider separating into a different analisys pass
    CompilationUtils::parseKernelArguments(pFunc->getParent() /* = pModule */,  pFunc, args, arguments);

    return new Kernel( funcName, arguments, pProps );
        }

size_t CPUCompiler::ResolveFunctionCalls(llvm::Module* pModule, llvm::Function* pFunc)
{
    // Required stack
    size_t stStack = 0;

    // Go through function blocks, and resolve calls
    llvm::Function::BasicBlockListType::iterator bb_it = pFunc->getBasicBlockList().begin();
    while ( bb_it != pFunc->getBasicBlockList().end() )
    {
        llvm::BasicBlock::InstListType::iterator inst_it = bb_it->getInstList().begin();
        llvm::Value* pArgVal LLVM_BACKEND_UNUSED = NULL;
        while ( inst_it != bb_it->getInstList().end() )
        {
            switch (inst_it->getOpcode())
            {
                // Call instruction
            case llvm::Instruction::Call:
                // Check call to not inlined functions/ kernels
                llvm::Function* pCallee = (llvm::dyn_cast<llvm::CallInst>(inst_it))->getCalledFunction();
                if ( NULL != pCallee && !pCallee->isDeclaration() )
                {
                    if ( !Utils::IsKernel(pModule, pCallee->getNameStr().c_str()) )
                    {
                        size_t stLclStack = ResolveFunctionCalls(pModule, pCallee);
                        // compile the function if needed - just to be able to get its stack size later
                        m_pExecEngine->getPointerToFunction(pCallee);
                        uint64_t jitStackSize = m_pExecEngine->getJitFunctionStackSize(pCallee);
                        assert(((uint64_t)-1) != jitStackSize && "Check that the pCallee was actually compiled succesfully");
                        stLclStack += jitStackSize + 128; //64;
                        stStack = std::max(stStack, stLclStack);
                    }
                }
                break;
            }
            ++inst_it;
        }
        ++bb_it;
    }

    return stStack;
}

KernelJITProperties* CPUCompiler::CreateKernelJITProperties(llvm::Module* pModule, 
                                                         llvm::Function* pFunc,
                                                         const TLLVMKernelInfo& info)
{
    // TODO : I don't think we use this information since adding the barriers
    // This calculation may be not correct When pFunc is the wrapper function
    unsigned int stackSize = ResolveFunctionCalls(pModule, pFunc);

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
    // compile the function if needed - just to be able to get its stack size later
    void * pFuncAddr = m_pExecEngine->getPointerToFunction(pFunc);
    uint64_t jitStackSize = m_pExecEngine->getJitFunctionStackSize(pFunc);
    assert(((uint64_t)-1) != jitStackSize && "Check that the pFunc was actually compiled succesfully");
    
    
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
        ML.method_size = m_pExecEngine->getJitFunctionSize(pFunc);    // Size in memory - Must be exact

        
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


    KernelJITProperties* pProps = new KernelJITProperties();
    pProps->SetUseVTune(m_config.GetUseVTune());
    pProps->SetVTuneId(uiVTuneId);
    pProps->SetStackSize(stackSize);
    return pProps;
}

KernelSet* CPUCompiler::CreateKernels( const Program* pProgram,
                                    llvm::Module* pModule, 
                                    ProgramBuildResult& buildResult, 
                                    FunctionWidthVector& vectorizedFunctions, 
                                    KernelsInfoMap& kernelsInfo,
                                    size_t privateMemorySize )
{
    buildResult.LogS() << "Build started\n";
    std::auto_ptr<KernelSet> spKernels( new KernelSet );

    llvm::NamedMDNode *pModuleMetadata = pModule->getNamedMetadata("opencl.kernels");
    if ( !pModuleMetadata ) {
      //Module contains no MetaData, thus it contains no kernels
      return spKernels.release();
    }

    std::vector<FunctionWidthPair>::iterator vecIter = vectorizedFunctions.begin();
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
        std::auto_ptr<KernelProperties> spKernelProps( CreateKernelProperties( pProgram, elt, kernelsInfo[pFunc]));
        std::auto_ptr<KernelJITProperties> spKernelJITProps( CreateKernelJITProperties( pModule,
                                                                                        pWrapperFunc,
                                                                                        kernelsInfo[pFunc]));

        // TODO: This is workaround till the SDK hanlde case of zero private memory size!
        spKernelProps->SetPrivateMemorySize(ADJUST_SIZE_TO_MAXIMUM_ALIGN(std::max<unsigned int>(1, privateMemorySize)));

        // Create a kernel 
        std::auto_ptr<Kernel>           spKernel( CreateKernel( pFunc, 
                                                                pWrapperFunc->getName().str(), 
                                                                pFuncArgs->getString().str(),
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

        if( !vectorizedFunctions.empty())
        {
            assert(vecIter != vectorizedFunctions.end());
            if(NULL != vecIter->first && !dontVectorize)
            {
                //
                // Create the vectorized kernel - no need to pass argument list here
                std::auto_ptr<KernelJITProperties>  spVKernelProps(CreateKernelJITProperties(pModule, 
                                                                                      vecIter->first,
                                                                                      kernelsInfo[vecIter->first]));
                spVKernelProps->SetVectorSize(vecIter->second);
                AddKernelJIT( spKernel.get(), pModule, vecIter->first, spVKernelProps.release());

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

void CPUCompiler::AddKernelJIT( Kernel* pKernel, llvm::Module* pModule, llvm::Function* pFunc, KernelJITProperties* pProps)
{
    IKernelJITContainer* pJIT = new CPUJITContainer( m_pExecEngine->getPointerToFunction(pFunc),
                                           pFunc,
                                           pModule,
                                           m_pExecEngine,
                                           pProps);
    pKernel->AddKernelJIT( pJIT );
}

llvm::Module* CPUCompiler::GetRtlModule() const
{
    assert(m_pBuiltinModule);
    return m_pBuiltinModule->GetRtlModule();
}

}}} // namespace

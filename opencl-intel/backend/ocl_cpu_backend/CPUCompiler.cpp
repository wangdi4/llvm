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
#define NOMINMAX

#include <vector>
#include <string>
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "CPUCompiler.h"
#include "CPUDetect.h"
#include "BuiltinModule.h"
#include "exceptions.h"
#include "CompilationUtils.h"
#include "BuiltinModuleManager.h"
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
 * Return the CPU identifier (from CPUDetect enumeration) given the CPU name.
 * CPU Name may be equal to 'auto', in this case the CPU detection will be performed
 */
Intel::ECPU GetOrDetectCpuId(const std::string& cpuArch)
{
    Intel::ECPU cpuId = Intel::CPU_LAST;
    Utils::CPUDetect* pCpuDetect = Utils::CPUDetect::GetInstance();
    
    if ( CPU_ARCH_AUTO == cpuArch ) 
    {
        cpuId = pCpuDetect->GetCPUId();
    }
    else
    {
        cpuId = pCpuDetect->IsValidCPUName(cpuArch.c_str()) ? pCpuDetect->GetCPUByName(cpuArch.c_str()) : Intel::CPU_LAST;
    }

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
    m_config(config),
    m_pBuiltinModule(NULL),
    m_pExecEngine(NULL)
{
    // WORKAROUND!!! See the notes in TerminationBlocker description
    static Utils::CPUTerminationBlocker blocker;

    SelectCpu( config.GetCpuArch(), config.GetCpuFeatures());

    // Initialize the BuiltinModule
    if(config.GetLoadBuiltins())
    {
        BuiltinLibrary* pLibrary = BuiltinModuleManager::GetInstance()->GetOrLoadCPULibrary(m_selectedCpuId, m_selectedCpuFeatures);
        std::auto_ptr<llvm::Module> spModule( CreateRTLModule(pLibrary) );
        m_pBuiltinModule = new BuiltinModule( spModule.get());

        // Initialize the ExecutionEngine
        // ExecutionEngine will own the pointer to the RT module, so we are releasing it here
        m_pExecEngine = CreateCPUExecutionEngine( spModule.release() );
    }
}

CPUCompiler::~CPUCompiler()
{
    // WORKAROUND!!! See the notes in TerminationBlocker description
    if( Utils::CPUTerminationBlocker::IsReleased() )
        return;
    if( m_pBuiltinModule)
        delete m_pBuiltinModule;
    delete m_pExecEngine;
}

unsigned int CPUCompiler::GetTypeAllocSize(const llvm::Type* pType)
{
    assert(m_pExecEngine);
    return m_pExecEngine->getTargetData()->getTypeAllocSize(pType);
}

void *CPUCompiler::GetPointerToFunction(llvm::Function *pf)
{
    assert(m_pExecEngine);
    return m_pExecEngine->getPointerToFunction(pf);
}

uint64_t CPUCompiler::GetJitFunctionStackSize(const llvm::Function* pf)
{
    assert(m_pExecEngine);
    return m_pExecEngine->getJitFunctionStackSize(pf);
}

void CPUCompiler::freeMachineCodeForFunction(llvm::Function* pf)
{
    assert(m_pExecEngine);
    m_pExecEngine->freeMachineCodeForFunction(pf);
}

unsigned int CPUCompiler::GetJitFunctionSize(const llvm::Function* pf)
{
    assert(m_pExecEngine);
    return m_pExecEngine->getJitFunctionSize(pf);
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

void CPUCompiler::CreateExecutionEngine(llvm::Module* pModule)
{
    // pModule is owned by created execution engine
    m_pExecEngine = CreateCPUExecutionEngine(pModule);
}

llvm::ExecutionEngine* CPUCompiler::CreateCPUExecutionEngine(llvm::Module* pModule )
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

llvm::Module* CPUCompiler::GetRtlModule() const
{
    if(m_pBuiltinModule == NULL)
    {
        return NULL;
    }
    else
        return m_pBuiltinModule->GetRtlModule();
}

}}}

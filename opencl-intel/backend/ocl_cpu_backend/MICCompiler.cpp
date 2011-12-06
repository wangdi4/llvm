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

File Name:  MICCompiler.cpp

\*****************************************************************************/
#define NOMINMAX

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
class MICTerminationBlocker
{
public:
    ~MICTerminationBlocker() { s_released = true; }
    static bool IsReleased() { return s_released; }
private:
    static bool s_released;
};
bool MICTerminationBlocker::s_released = false;

}

void MICCompiler::SelectMICConfiguration(const CompilerConfig& config)
{
    // TODO[MA]: change this later
    m_selectedCpuId = Intel::MIC_KNIGHTSFERRY;
    m_selectedCpuFeatures = 0;
}

MICCompiler::MICCompiler(const CompilerConfig& config):
    Compiler(config),
    m_pBuiltinModule(NULL),
    m_pCGEngine(NULL),
    m_config(config)
{
    // WORKAROUND!!! See the notes in TerminationBlocker description
    static Utils::MICTerminationBlocker blocker;

    SelectMICConfiguration(config);

    if(config.GetLoadBuiltins())
    {
        // Initialize the BuiltinModule
        BuiltinLibrary* pLibrary = BuiltinModuleManager::GetInstance()->GetOrLoadMICLibrary(m_selectedCpuId, m_selectedCpuFeatures);
        std::auto_ptr<llvm::Module> spModule( CreateRTLModule(pLibrary) );
        m_pBuiltinModule = new BuiltinModule( spModule.get());

        // Initialize the ExecutionEngine
        // ExecutionEngine will own the pointer to the RT module, so we are releasing it here
        m_pCGEngine = CreateMICCodeGenerationEngine( spModule.release() );
    }
}

void MICCompiler::CreateExecutionEngine(llvm::Module* m)
{
    m_pCGEngine = CreateMICCodeGenerationEngine( m );
}

MICCompiler::~MICCompiler()
{
    // WORKAROUND!!! See the notes in TerminationBlocker description
    if( Utils::MICTerminationBlocker::IsReleased() )
        return;

    if(m_pBuiltinModule)
    {
        delete m_pBuiltinModule->GetRtlModule();
        delete m_pBuiltinModule;
    }
    delete m_pCGEngine;
}

unsigned int MICCompiler::GetTypeAllocSize(const llvm::Type* pType)
{
    assert(m_pCGEngine);
    return m_pCGEngine->sizeOf(pType);
}

const llvm::ModuleJITHolder* MICCompiler::GetModuleHolder(llvm::Module& module)
{
    assert(m_pCGEngine);
    return m_pCGEngine->getModuleHolder(module);
}

llvm::MICCodeGenerationEngine* MICCompiler::CreateMICCodeGenerationEngine( llvm::Module* pRtlModule )
{
    std::string MTriple = pRtlModule->getTargetTriple();
    std::string MCPU    = Utils::CPUDetect::GetInstance()->GetCPUName((Intel::ECPU)m_selectedCpuId);
    std::string MArch   = "x86-64"; //TODO[MA]: check why we need to send this !
    llvm::SmallVector<std::string, 1> MAttrs;

    llvm::TargetMachine *TM = llvm::MICCodeGenerationEngine::selectTarget(pRtlModule,
        MTriple, MArch, MCPU, 
        MAttrs, &m_ErrorStr);

    llvm::CodeGenOpt::Level OLvl = llvm::CodeGenOpt::Aggressive;

    return new llvm::MICCodeGenerationEngine(*TM, OLvl);
}

llvm::Module* MICCompiler::GetRtlModule() const
{
    if(m_pBuiltinModule)
    {
        assert(m_pBuiltinModule && "MIC Builtin Module not initialized");
        return NULL;
    }
    else
        return m_pBuiltinModule->GetRtlModule();
}

}}}
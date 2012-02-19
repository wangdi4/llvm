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
#include "llvm/Target/TargetData.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Target/TargetMachine.h"
//#include "llvm/MICJITEngine/MICCodeGenerationEngine.h"
//#include "llvm/MICJITEngine/ModuleJITHolder.h"
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

void MICCompiler::SelectMICConfiguration(const CompilerConfig& config)
{
    // TODO[MA]: change this later
    m_selectedCpuId = Intel::MIC_KNIGHTSFERRY;
    m_selectedCpuFeatures = 0;
}

MICCompiler::MICCompiler(const MICCompilerConfig& config):
    Compiler(config),
    m_pBuiltinModule(NULL),
#if MICJIT_ENABLE 
    m_pCGEngine(NULL),
#endif
    m_config(config)
{
    SelectMICConfiguration(config);

    if(config.GetLoadBuiltins())
    {
        // Initialize the BuiltinModule
        // TODO: fix target id in case of multi-MIC cards
        BuiltinLibrary* pLibrary = BuiltinModuleManager::GetInstance()->GetOrLoadMICLibrary(
            0, m_selectedCpuId, m_selectedCpuFeatures, &config.GetTargetDescription());
#if MICJIT_ENABLE 
        m_ResolverWrapper.SetResolver(pLibrary);
#endif
        std::auto_ptr<llvm::Module> spModule( CreateRTLModule(pLibrary) );
        m_pBuiltinModule = new BuiltinModule( spModule.get());

        // Initialize the ExecutionEngine
        // ExecutionEngine will own the pointer to the RT module, so we are releasing it here
#if MICJIT_ENABLE 
        m_pCGEngine = CreateMICCodeGenerationEngine( spModule.release() );
#endif
    }
}

void MICCompiler::CreateExecutionEngine(llvm::Module* m) const
{
#if MICJIT_ENABLE 
    m_pCGEngine = CreateMICCodeGenerationEngine( m );
#endif
}

MICCompiler::~MICCompiler()
{
    // WORKAROUND!!! See the notes in TerminationBlocker description
    if( Utils::TerminationBlocker::IsReleased() )
        return;

    if(m_pBuiltinModule)
    {
        delete m_pBuiltinModule->GetRtlModule();
        delete m_pBuiltinModule;
    }
#if MICJIT_ENABLE 
    delete m_pCGEngine;
#endif
}

unsigned int MICCompiler::GetTypeAllocSize(llvm::Type* pType) const
{
#if MICJIT_ENABLE 
    assert(m_pCGEngine);
    return m_pCGEngine->sizeOf(pType);
#else
    return 0;
#endif
}

const llvm::ModuleJITHolder* MICCompiler::GetModuleHolder(llvm::Module& module) const
{
#if MICJIT_ENABLE 
    assert(m_pCGEngine);
    return m_pCGEngine->getModuleHolder(module);
#else
    return NULL;
#endif
}

llvm::MICCodeGenerationEngine* MICCompiler::CreateMICCodeGenerationEngine( llvm::Module* pRtlModule ) const
{
#if MICJIT_ENABLE 
    std::string MTriple = pRtlModule->getTargetTriple();
    std::string MCPU    = Utils::CPUDetect::GetInstance()->GetCPUName((Intel::ECPU)m_selectedCpuId);
    std::string MArch   = "x86-64"; //TODO[MA]: check why we need to send this !
    llvm::SmallVector<std::string, 1> MAttrs;

    llvm::TargetMachine *TM = llvm::MICCodeGenerationEngine::selectTarget(pRtlModule,
        MTriple, MArch, MCPU, 
        MAttrs, &m_ErrorStr);

    llvm::CodeGenOpt::Level OLvl = llvm::CodeGenOpt::Aggressive;
    return new llvm::MICCodeGenerationEngine(*TM, OLvl, &m_ResolverWrapper);
#else
    return NULL;
#endif
}

llvm::Module* MICCompiler::GetRtlModule() const
{
    if(m_pBuiltinModule)
    {
        return m_pBuiltinModule->GetRtlModule();
    }
    assert(m_pBuiltinModule && "MIC Builtin Module not initialized");
    return NULL;
}

}}}

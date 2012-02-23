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
#include "MICJITEngine/include/MICCodeGenerationEngine.h"
#include "MICJITEngine/include/ModuleJITHolder.h"
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
    m_config(config),
    m_pCGEngine(NULL)
{
    SelectMICConfiguration(config);

    if(config.GetLoadBuiltins())
    {
        // Initialize the BuiltinModule
        // TODO: fix target id in case of multi-MIC cards
        BuiltinLibrary* pLibrary = BuiltinModuleManager::GetInstance()->GetOrLoadMICLibrary(
            0, m_selectedCpuId, m_selectedCpuFeatures, &config.GetTargetDescription());
        m_ResolverWrapper.SetResolver(pLibrary);
        std::auto_ptr<llvm::Module> spModule( CreateRTLModule(pLibrary) );
        m_pBuiltinModule = new BuiltinModule( spModule.get());

        // Initialize the ExecutionEngine
        // ExecutionEngine will own the pointer to the RT module, so we are releasing it here
        m_pCGEngine = CreateMICCodeGenerationEngine( spModule.release() );
    }
}

void MICCompiler::CreateExecutionEngine(llvm::Module* m) const
{
    m_pCGEngine = CreateMICCodeGenerationEngine( m );
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
    delete m_pCGEngine;
}

unsigned int MICCompiler::GetTypeAllocSize(llvm::Type* pType) const
{
    assert(m_pCGEngine);
    return m_pCGEngine->sizeOf(pType);
}

const llvm::ModuleJITHolder* MICCompiler::GetModuleHolder(llvm::Module& module) const
{
    assert(m_pCGEngine);
    return m_pCGEngine->getModuleHolder(module);
}

llvm::MICCodeGenerationEngine* MICCompiler::CreateMICCodeGenerationEngine( llvm::Module* pRtlModule ) const
{
    llvm::StringRef MTriple = pRtlModule->getTargetTriple();
    llvm::StringRef MCPU    = Utils::CPUDetect::GetInstance()->GetCPUName((Intel::ECPU)m_selectedCpuId);
    llvm::StringRef MArch   = "y86-64"; //TODO[MA]: check why we need to send this !
    llvm::SmallVector<std::string, 1> MAttrs;

    llvm::TargetMachine *TM = llvm::MICCodeGenerationEngine::selectTarget(pRtlModule,
        MArch, MCPU,
        MAttrs, Reloc::PIC_, CodeModel::Small, &m_ErrorStr);

    llvm::CodeGenOpt::Level OLvl = llvm::CodeGenOpt::Aggressive;
    return new llvm::MICCodeGenerationEngine(*TM, OLvl, &m_ResolverWrapper);
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

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
#include "KernelProperties.h"
#include "CPUDetect.h"
#include "BuiltinModule.h"
#include "exceptions.h"
#include "BuiltinModuleManager.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Triple.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Target/TargetMachine.h"
#include "MICJITEngine/include/MICCodeGenerationEngine.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "MICProgramBuilder.h"
#include "ModuleJITHolder.h"
#include "MICJITContainer.h"
#include "CompilationUtils.h"
#include "StringUtils.h"
using std::string;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/*
 *  Constants
 */
extern const char* CPU_ARCH_AUTO;

void MICCompiler::SelectMICConfiguration(const ICompilerConfig& config)
{
    Intel::ECPU CPU = Intel::CPUId::GetCPUByName(config.GetCpuArch().c_str());
    m_CpuId = Intel::CPUId(CPU, 0, true);
    Utils::SplitString( config.GetCpuFeatures(), ",", m_forcedCpuFeatures);
    assert(m_CpuId.HasGatherScatter());
}

MICCompiler::MICCompiler(const IMICCompilerConfig& config):
    Compiler(config),
    m_pBuiltinModule(NULL),
    m_pCGEngine(NULL)
{
    SelectMICConfiguration(config);

    if(config.GetLoadBuiltins())
    {
        // Initialize the BuiltinModule
        // TODO: fix target id in case of multi-MIC cards
        BuiltinLibrary* pLibrary = BuiltinModuleManager::GetInstance()->GetOrLoadMICLibrary(
            0, m_CpuId, &config.GetTargetDescription());
        m_ResolverWrapper.SetResolver(pLibrary);
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
    if( Utils::TerminationBlocker::IsReleased() )
        return;

    if(m_pBuiltinModule)
    {
        delete m_pBuiltinModule->GetRtlModule();
        delete m_pBuiltinModule;
    }
    delete m_pCGEngine;
}

llvm::LLVMModuleJITHolder* MICCompiler::GetModuleHolder(llvm::Module& module, const std::string& dumpAsm) const
{
    assert(m_pCGEngine);
    return m_pCGEngine->getModuleHolder(module, dumpAsm);
}

llvm::MICCodeGenerationEngine* MICCompiler::CreateMICCodeGenerationEngine( llvm::Module* pRtlModule ) const
{
    llvm::StringRef MTriple = pRtlModule->getTargetTriple();
    llvm::StringRef MArch   = "y86-64"; //TODO[MA]: check why we need to send this !
    llvm::SmallVector<std::string, 1> MAttrs(m_forcedCpuFeatures.begin(), m_forcedCpuFeatures.end());

    Triple TTriple(MTriple);
    // MIC device OS is always linux, no matter what the host is
    if (TTriple.getOS() != Triple::Linux)
    {
        TTriple.setOS(Triple::Linux);
        pRtlModule->setTargetTriple(TTriple.getTriple());
    }

    const char* pMcpu    = m_CpuId.GetCPUName();
    if( NULL == pMcpu )
    {
        throw Exceptions::CompilerException("Failed to create m-cpu object");
    }
    llvm::StringRef MCPU    = pMcpu;

    llvm::TargetMachine *TM = llvm::MICCodeGenerationEngine::selectTarget(pRtlModule,
        MArch, MCPU,
        MAttrs, Reloc::Default, CodeModel::Small, &m_ErrorStr);

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

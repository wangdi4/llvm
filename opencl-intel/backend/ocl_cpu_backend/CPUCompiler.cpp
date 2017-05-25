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

#include "BuiltinModules.h"
#include "BuiltinModuleManager.h"
#include "CPUCompiler.h"
#include "CPUDetect.h"
#include "ObjectCodeCache.h"
#include "CompilationUtils.h"
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "exceptions.h"

// Reference a symbol in JIT.cpp and MCJIT.cpp so that static or global constructors are called
#include "llvm/ADT/Triple.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

#include <iosfwd>
#include <sstream>
#include <string>
#include <vector>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/*
 *  Constants
 */
extern const char* CPU_ARCH_AUTO;

TargetOptions ExternInitTargetOptionsFromCodeGenFlags();

/*
 * Utility methods
 */
namespace Utils
{
/**
 * Return the CPU identifier (from CPUDetect enumeration) given the CPU name.
 * CPU Name may be equal to 'auto', in this case the CPU detection will be performed
 */
Intel::ECPU GetOrDetectCpuId(const std::string& cpuArch)
{
    Intel::ECPU cpuId = Intel::DEVICE_INVALID;
    Utils::CPUDetect* pCpuDetect = Utils::CPUDetect::GetInstance();

    if ( CPU_ARCH_AUTO == cpuArch )
    {
        cpuId = pCpuDetect->GetCPUId().GetCPU();
    }
    else
    {
        cpuId = Intel::CPUId::IsValidCPUName(cpuArch.c_str()) ? Intel::CPUId::GetCPUByName(cpuArch.c_str()) : Intel::DEVICE_INVALID;
    }

    if( Intel::DEVICE_INVALID == cpuId )
    {
        throw Exceptions::CompilerException("Unsupported CPU Architecture");
    }

    return cpuId;
}

/**
 * Splits the given string using the supplied delimiter
 * populates the given vector of strings
 */
void SplitString( const std::string& s, const char* d, std::vector<std::string>& v )
{
    llvm::StringRef sr(s);
    llvm::SmallVector<llvm::StringRef,2> sv;

    sr.split(sv, d, -1, false);
    std::copy( sv.begin(), sv.end(), std::back_inserter( v ));
}

/**
 * Joins the given strings (as a vector of strings) using
 * the supplied delimiter.
 * Returns: joined string
 */
std::string JoinStrings( const std::vector<std::string>& vs, const char* d)
{
    std::vector<std::string>::const_iterator i = vs.begin();
    std::vector<std::string>::const_iterator e = vs.end();
    std::stringstream ss;

    if( i != e )
    {
        ss << *i++;
        for(; i!= e; ++i)
        {
            ss << d << *i;
        }
    }

    return ss.str();
}

unsigned int SelectCpuFeatures( unsigned int cpuId, const std::vector<std::string>& forcedFeatures)
{
    unsigned int  cpuFeatures = CFS_SSE2;

    // Add standard features
    if( cpuId >= (unsigned int)Intel::CPUId::GetCPUByName("core2") )
    {
        cpuFeatures |= CFS_SSE3 | CFS_SSSE3;
    }

    if( cpuId >= (unsigned int)Intel::CPUId::GetCPUByName("corei7") )
    {
        cpuFeatures |= CFS_SSE41 | CFS_SSE42;
    }

    if( cpuId >= (unsigned int)Intel::CPUId::GetCPUByName("corei7-avx"))
    {
        cpuFeatures |= CFS_AVX1;
    }

    if( cpuId >= (unsigned int)Intel::CPUId::GetCPUByName("core-avx2"))
    {
        cpuFeatures |= CFS_AVX1;
        cpuFeatures |= CFS_AVX2;
        cpuFeatures |= CFS_FMA;
        cpuFeatures |= CFS_BMI;
        cpuFeatures |= CFS_BMI2;
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
        cpuFeatures |= CFS_FMA;
    }

    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "+avx" ) != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX1;
    }
    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "+avx512f" ) != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX512F;
    }
    if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512bw") != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX512BW;
    }
    if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512cd") != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX512CD;
    }
    if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512dq") != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX512DQ;
    }
    if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512er") != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX512ER;
    }
    if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512pf") != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX512PF;
    }
    if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512vl") != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX512VL;
    }

    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "-sse41" ) != forcedFeatures.end())
    {
        cpuFeatures &= ~(CFS_SSE41 | CFS_SSE42);
    }

    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "-avx2" ) != forcedFeatures.end())
    {
        cpuFeatures &= ~CFS_AVX2;
        cpuFeatures &= ~CFS_FMA;
    }

    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "-avx" ) != forcedFeatures.end())
    {
        cpuFeatures &= ~CFS_AVX1;
        cpuFeatures &= ~CFS_AVX2;
        cpuFeatures &= ~CFS_FMA;
    }
    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "-fma" ) != forcedFeatures.end())
    {
        cpuFeatures &= ~CFS_FMA;
    }
    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "-bmi" ) != forcedFeatures.end())
    {
        cpuFeatures &= ~CFS_BMI;
    }
    if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "-bmi2" ) != forcedFeatures.end())
    {
        cpuFeatures &= ~CFS_BMI2;
    }

    return cpuFeatures;

}

}

CPUCompiler::CPUCompiler(const ICompilerConfig& config):
    Compiler(config),
    m_pBuiltinModule(NULL),
    m_pExecEngine(NULL),
    m_pVTuneListener(NULL)
{
    SelectCpu( config.GetCpuArch(), config.GetCpuFeatures());

    // Initialize the BuiltinModules
    if(config.GetLoadBuiltins())
    {
        BuiltinLibrary* pLibrary = BuiltinModuleManager::GetInstance()->GetOrLoadCPULibrary(m_CpuId);

        llvm::SmallVector<llvm::Module*, 2> bltnFuncList;
        LoadBuiltinModules(pLibrary, bltnFuncList);
        m_pBuiltinModule = new BuiltinModules(bltnFuncList);
    }

    // Create the listener that allows Amplifier to profile OpenCL kernels
    if(config.GetUseVTune())
    {
        m_pVTuneListener = llvm::JITEventListener::createIntelJITEventListener();
    }
}

CPUCompiler::~CPUCompiler()
{
    // WORKAROUND!!! See the notes in TerminationBlocker description
    if( Utils::TerminationBlocker::IsReleased() )
        return;

    delete m_pBuiltinModule;
    delete m_pVTuneListener;
}

void *CPUCompiler::GetPointerToFunction(llvm::Function *pf)
{
    llvm::Module *pM = pf->getParent();

    // Perform codegen if needed (by constructing an EE for the module)
    if(!m_pExecEngine)
      m_pExecEngine = CreateCPUExecutionEngine(pM);

    return reinterpret_cast<void*>(m_pExecEngine->getFunctionAddress(pf->getName().str()));
}

void CPUCompiler::SelectCpu( const std::string& cpuName, const std::string& cpuFeatures )
{
    Intel::ECPU selectedCpuId = Utils::GetOrDetectCpuId( cpuName );
    Utils::SplitString( cpuFeatures, ",", m_forcedCpuFeatures);

    bool DisableAVX = false;
    // if we autodetected the SandyBridge CPU and a user didn't forced us to use AVX256 - disable it if not supported
    if (cpuName == CPU_ARCH_AUTO)
    {
        CPUId cpuId = Utils::CPUDetect::GetInstance()->GetCPUId();
        if (selectedCpuId == Intel::CPU_SANDYBRIDGE)
        {
            if( std::find( m_forcedCpuFeatures.begin(), m_forcedCpuFeatures.end(), "+avx" ) == m_forcedCpuFeatures.end() )
            {
                // check if the OS is AVX ready - if not, need to disable AVX at all
                if (!cpuId.HasAVX1())
                {
                    m_forcedCpuFeatures.push_back("-avx");
                    DisableAVX = true;
                }
            }
        }
        else if (selectedCpuId == Intel::CPU_HASWELL) {
            if (!cpuId.IsFeatureOn(CFS_BMI))
                m_forcedCpuFeatures.push_back("-bmi");
            if (!cpuId.IsFeatureOn(CFS_BMI2))
                m_forcedCpuFeatures.push_back("-bmi2");
        }
    }

    if (!DisableAVX && (selectedCpuId == Intel::CPU_SANDYBRIDGE))
      m_forcedCpuFeatures.push_back("+avx");

    if (!DisableAVX && (selectedCpuId == Intel::CPU_HASWELL)) {
      m_forcedCpuFeatures.push_back("+avx2");
      m_forcedCpuFeatures.push_back("+f16c");
    }
    if (selectedCpuId == Intel::CPU_KNL) {
      m_forcedCpuFeatures.push_back("+avx512f");
      m_forcedCpuFeatures.push_back("+avx512cd");
      m_forcedCpuFeatures.push_back("+avx512er");
      m_forcedCpuFeatures.push_back("+avx512pf");
    }
    if (selectedCpuId == Intel::CPU_SKX) {
      m_forcedCpuFeatures.push_back("+avx512f");
      m_forcedCpuFeatures.push_back("+avx512cd");
      m_forcedCpuFeatures.push_back("+avx512bw");
      m_forcedCpuFeatures.push_back("+avx512dq");
      m_forcedCpuFeatures.push_back("+avx512vl");
    }

    unsigned int selectedCpuFeatures = Utils::SelectCpuFeatures( selectedCpuId, m_forcedCpuFeatures );
    m_CpuId = CPUId(selectedCpuId, selectedCpuFeatures, sizeof(void*)==8);
}

void CPUCompiler::CreateExecutionEngine(llvm::Module* pModule)
{
    // Compiler keeps a pointer to the execution engine object
    // and is not responsible for EE release
    m_pExecEngine = CreateCPUExecutionEngine(pModule);
}

llvm::ExecutionEngine* CPUCompiler::CreateCPUExecutionEngine(llvm::Module* pModule) const
{
    // Leaving MArch blank implies using auto-detect
    llvm::StringRef MCPU  = m_CpuId.GetCPUName();
    llvm::StringRef MArch = "";

    std::string strErr;
    // [LLVM 3.6 UPGRADE] See below near the respective 'set' on why this is
    // commented out.
    // bool AllocateGVsWithCode = true;
    CodeGenOpt::Level OLevel = llvm::CodeGenOpt::Default;

    if (m_debug)
      OLevel = llvm::CodeGenOpt::None;

    // FP_CONTRACT defined in module
    // Exclude FMA instructions when FP_CONTRACT is disabled
    std::vector<std::string> cpuFeatures(m_forcedCpuFeatures);

    std::unique_ptr<llvm::Module> pModuleUniquePtr(pModule);
    llvm::EngineBuilder builder(std::move(pModuleUniquePtr));
    builder.setEngineKind(llvm::EngineKind::JIT);
    builder.setErrorStr(&strErr);
    builder.setOptLevel(OLevel);
    builder.setCodeModel(llvm::CodeModel::JITDefault);
    builder.setRelocationModel(llvm::Reloc::Default);
    builder.setMArch(MArch);
    builder.setMCPU(MCPU);
    builder.setMAttrs(cpuFeatures);
    builder.setMCJITMemoryManager(std::unique_ptr<RTDyldMemoryManager>(
        new SectionMemoryManager()));
    llvm::TargetOptions targetOpt = ExternInitTargetOptionsFromCodeGenFlags();
    if (pModule->getNamedMetadata("opencl.enable.FP_CONTRACT"))
      targetOpt.AllowFPOpFusion = llvm::FPOpFusion::Fast;
    else
      targetOpt.AllowFPOpFusion = llvm::FPOpFusion::Standard;
    builder.setTargetOptions(targetOpt);

    llvm::ExecutionEngine* pExecEngine = builder.create();

    if ( NULL == pExecEngine )
    {
        throw Exceptions::CompilerException("Failed to create execution engine");
    }

    if (m_pVTuneListener)
        pExecEngine->RegisterJITEventListener(m_pVTuneListener);

    return pExecEngine;
}

llvm::SmallVector<llvm::Module*, 2> CPUCompiler::GetBuiltinModuleList() const
{
    return m_pBuiltinModule->GetBuiltinModuleList();
}

void CPUCompiler::DumpJIT( llvm::Module *pModule, const std::string& filename) const
{
    assert(pModule && "pModule parameter should be valid");

    std::string err;
    llvm::Triple triple(pModule->getTargetTriple());
    const llvm::Target *pTarget = llvm::TargetRegistry::lookupTarget(triple.getTriple(), err);
    if( !err.empty() || NULL == pTarget )
    {
        throw Exceptions::CompilerException(std::string("Failed to retrieve the target for given module during dump operation:") + err);
    }

    TargetOptions Options;
    if (pModule->getNamedMetadata("opencl.enable.FP_CONTRACT")) {
        Options.AllowFPOpFusion = llvm::FPOpFusion::Fast;
    } else {
        Options.AllowFPOpFusion = llvm::FPOpFusion::Standard;
    }

    std::string cpuName( m_CpuId.GetCPUName());
    std::vector<std::string> localCpuFeatures = m_forcedCpuFeatures;
    std::string cpuFeatures( Utils::JoinStrings(localCpuFeatures, ","));
    TargetMachine* pTargetMachine = pTarget->createTargetMachine(triple.getTriple(), cpuName, cpuFeatures, Options);
    if( NULL == pTargetMachine )
    {
        throw Exceptions::CompilerException("Failed to create TargetMachine object");
    }

    std::error_code ec;
    llvm::raw_fd_ostream out(filename.c_str(), ec, llvm::sys::fs::F_RW);
    if (ec)
    {
        throw Exceptions::CompilerException(
        std::string("Failed to open the target file for dump: error code:") + ec.message());
    }

    // Build up all of the passes that we want to do to the module.
    llvm::legacy::PassManager pm;
    pTargetMachine->addPassesToEmitFile(pm, out, TargetMachine::CGFT_AssemblyFile, /*DisableVerify*/ true);
    pm.run(*pModule);
}

void CPUCompiler::SetObjectCache(ObjectCodeCache* pCache)
{
    ((llvm::ExecutionEngine*)GetExecutionEngine())->setObjectCache(pCache);
}

}}}

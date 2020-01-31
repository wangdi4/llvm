// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#define NOMINMAX

#include "BuiltinModules.h"
#include "BuiltinModuleManager.h"
#include "CPUCompiler.h"
#include "ObjectCodeCache.h"
#include "CompilationUtils.h"
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "debuggingservicetype.h"
#include "exceptions.h"
#include "LLDJITBuilder.h"
// Reference a symbol in JIT.cpp and MCJIT.cpp so that static or global constructors are called
#include "llvm/ADT/Triple.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"

#include <string>

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
void SplitString( const std::string& s, const char* d, llvm::SmallVectorImpl<std::string>& v )
{
    llvm::StringRef sr(s);
    llvm::SmallVector<llvm::StringRef,2> sv;

    sr.split(sv, d, -1, false);
    std::transform( sv.begin(), sv.end(), std::back_inserter( v ), [](llvm::StringRef s) { return std::string(s); });
}

unsigned int SelectCpuFeatures(
  unsigned int cpuId,
  const llvm::SmallVectorImpl<std::string>& forcedFeatures)
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
    if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512vbmi") != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX512VBMI;
    }
    if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512ifma") != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX512IFMA;
    }
    if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512bitalg") != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX512BITALG;
    }
    if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512vbmi2") != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX512VBMI2;
    }
    if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512vpopcntdq") != forcedFeatures.end())
    {
        cpuFeatures |= CFS_AVX512POPCNTDQ;
    }
    if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+clwb") != forcedFeatures.end())
    {
        cpuFeatures |= CFS_CLWB;
    }
    if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+wbnoinvd") != forcedFeatures.end())
    {
        cpuFeatures |= CFS_WBNOINVD;
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

// If binary not matchs current cpu arch
// and cpu is backwards compatible,load builtin modules again
void
CPUCompiler::SetBuiltinModules(const std::string& cpuName, const std::string& cpuFeatures="")
{
    // config.GetLoadBuiltins should be true
    if(m_pBuiltinModule != nullptr)
    {
        SelectCpu(cpuName, cpuFeatures);
        BuiltinLibrary* pLibrary = m_bIsEyeQEmulator ?
                                   BuiltinModuleManager::GetInstance()->GetOrLoadEyeQLibrary(m_CpuId) :
                                   BuiltinModuleManager::GetInstance()->GetOrLoadCPULibrary(m_CpuId);
        llvm::SmallVector<llvm::Module*, 2> bltnFuncList;
        LoadBuiltinModules(pLibrary, bltnFuncList);
        delete m_pBuiltinModule;
        m_pBuiltinModule = new BuiltinModules(bltnFuncList);
    }
}

CPUCompiler::CPUCompiler(const ICompilerConfig& config):
    Compiler(config),
    m_pBuiltinModule(nullptr),
    m_pExecEngine(nullptr),
    m_pVTuneListener(nullptr)
{
    SelectCpu( config.GetCpuArch(), config.GetCpuFeatures());

    // Initialize the BuiltinModules
    if(config.GetLoadBuiltins())
    {
        BuiltinLibrary* pLibrary = EYEQ_EMU_DEVICE == config.TargetDevice() ?
                                                      BuiltinModuleManager::GetInstance()->GetOrLoadEyeQLibrary(m_CpuId) :
                                                      BuiltinModuleManager::GetInstance()->GetOrLoadCPULibrary(m_CpuId);

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
    CPUId cpuId = Utils::CPUDetect::GetInstance()->GetCPUId();
    // if we autodetected the SandyBridge CPU and a user didn't forced us to use AVX256 - disable it if not supported
    if (cpuName == CPU_ARCH_AUTO)
    {
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

    if (!DisableAVX && (selectedCpuId == Intel::CPU_HASWELL))
      m_forcedCpuFeatures.push_back("+avx2");

    // Comment by Craig Topper: The F16C instructions are all encoded using the
    // VEX prefix which became available with AVX. That's why CPU_SANDYBRIDGE
    // appeared in condition.
    if (!DisableAVX && (selectedCpuId >= Intel::CPU_SANDYBRIDGE)
         && cpuId.IsFeatureOn(CFS_F16C)) {
      m_forcedCpuFeatures.push_back("+f16c");
    }

    if (selectedCpuId == Intel::CPU_KNL) {
      m_forcedCpuFeatures.push_back("+avx512f");
      m_forcedCpuFeatures.push_back("+avx512cd");
      m_forcedCpuFeatures.push_back("+avx512er");
      m_forcedCpuFeatures.push_back("+avx512pf");
    }

    if (selectedCpuId >= Intel::CPU_SKX) {
      m_forcedCpuFeatures.push_back("+avx512f");
      m_forcedCpuFeatures.push_back("+avx512cd");
      m_forcedCpuFeatures.push_back("+avx512bw");
      m_forcedCpuFeatures.push_back("+avx512dq");
      m_forcedCpuFeatures.push_back("+avx512vl");
    }

    if (selectedCpuId >= Intel::CPU_ICL) {
      // CNL features
      m_forcedCpuFeatures.push_back("+avx512vbmi");
      m_forcedCpuFeatures.push_back("+avx512ifma");
      // ICL features
      m_forcedCpuFeatures.push_back("+avx512vbmi2");
      m_forcedCpuFeatures.push_back("+avx512bitalg");
      m_forcedCpuFeatures.push_back("+avx512vpopcntdq");
      m_forcedCpuFeatures.push_back("+clwb");
    }
    if (selectedCpuId == Intel::CPU_ICX) {
      // ICX features
      m_forcedCpuFeatures.push_back("+wbnoinvd");
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

std::unique_ptr<LLJIT2> CPUCompiler::CreateLLJIT() {
    // Create LLJIT instance
    auto LLJIT = LLJIT2Builder().create();
    if (!LLJIT)
        throw Exceptions::CompilerException("Failed to create LLJIT");

    // Enable searching for symbols in the current process.
    char GlobalPrefix = '\0';
    auto Generator =
        llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
            GlobalPrefix);
    if (!Generator)
        throw Exceptions::CompilerException(
            "Failed to create DynamicLibrarySearchGenerator");
    (void)(*LLJIT)->getMainJITDylib().addGenerator(std::move(*Generator));

    // Add builtin function symbols
    if (auto Err =
            BuiltinModuleManager::GetInstance()->RegisterCPUBIFunctionsToLLJIT(
                (*LLJIT).get())) {
        llvm::logAllUnhandledErrors(std::move(Err), llvm::errs());
        throw Exceptions::CompilerException("Failed to add builtin symbols");
    }

    return std::move(*LLJIT);
}

bool CPUCompiler::useLLDJITForExecution(llvm::Module* pModule) const {
    bool hasCUs =
        (pModule->debug_compile_units_begin() !=
         pModule->debug_compile_units_end());
  
    bool useLLDJIT = intel::getDebuggingServiceType(m_debug && hasCUs,
                                                    pModule,
                                                    m_useNativeDebugger) ==
        intel::Native;
  
    return useLLDJIT;
}

llvm::ExecutionEngine* CPUCompiler::CreateCPUExecutionEngine(llvm::Module* pModule) const
{
    llvm::ExecutionEngine* pExecEngine;
#ifdef _WIN32
    if (useLLDJITForExecution(pModule))
    {
        LLDJITBuilder::prepareModuleForLLD(pModule);
        auto TargetMachine = GetTargetMachine(pModule);
        pExecEngine = LLDJITBuilder::CreateExecutionEngine(pModule, TargetMachine);
    }
    else
#endif
    {
        std::string strErr;

        std::unique_ptr<llvm::Module> pModuleUniquePtr(pModule);

        llvm::EngineBuilder builder(std::move(pModuleUniquePtr));
        builder.setEngineKind(llvm::EngineKind::JIT);
        builder.setErrorStr(&strErr);
        builder.setMCJITMemoryManager(std::unique_ptr<RTDyldMemoryManager>(
            new SectionMemoryManager()));

        auto TargetMachine = GetTargetMachine(pModule);
        pExecEngine = builder.create(TargetMachine);
    }
    if ( nullptr == pExecEngine )
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

void CPUCompiler::DumpJIT( llvm::Module *pModule, const std::string& filename,
                           CodeGenFileType genType) const
{
    assert(pModule && "pModule parameter should be valid");

    TargetMachine* pTargetMachine = GetTargetMachine(pModule);
    if( nullptr == pTargetMachine )
    {
        throw Exceptions::CompilerException("Failed to create TargetMachine object");
    }

    std::error_code ec;
    llvm::raw_fd_ostream out(filename.c_str(), ec, llvm::sys::fs::FA_Write);
    if (ec)
    {
        throw Exceptions::CompilerException(
        std::string("Failed to open the target file for dump: error code:") + ec.message());
    }

    // Build up all of the passes that we want to do to the module.
    llvm::legacy::PassManager pm;
    pTargetMachine->addPassesToEmitFile(pm, out,
        /*raw_pwrite_stream*/ nullptr, genType,
        /*DisableVerify*/ true);
    pm.run(*pModule);
}

void CPUCompiler::SetObjectCache(ObjectCodeCache* pCache)
{
    ((llvm::ExecutionEngine*)GetExecutionEngine())->setObjectCache(pCache);
}

}}}

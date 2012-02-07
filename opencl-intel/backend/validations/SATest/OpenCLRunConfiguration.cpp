/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  OpenCLRunConfiguration.cpp

\*****************************************************************************/

#include "OpenCLRunConfiguration.h"
#include "OpenCLProgramConfiguration.h"
#include "llvm/Support/CommandLine.h"
#include "SATest.h"

extern llvm::cl::opt<Validation::TEST_MODE>
TestMode;

extern llvm::cl::opt<bool>
NoRef;

extern llvm::cl::opt<bool>
TraceMarks;

extern llvm::cl::opt<bool>
UseNEAT;

extern llvm::cl::opt<bool>
RunSingleWG;

extern llvm::cl::opt<uint32_t>
ExecuteIterations;

extern llvm::cl::opt<uint32_t>
BuildIterations;

extern llvm::cl::opt<Intel::OpenCL::DeviceBackend::ETransposeSize>
TransposeSize;

extern llvm::cl::opt<std::string>
CPUArch;

extern llvm::cl::opt<std::string>
CPUFeatures;

extern llvm::cl::opt<bool>
FlagForceRunReference;

extern llvm::cl::opt<double>
ULP_tolerance;

extern llvm::cl::opt<uint32_t>
DefaultLocalWGSize;

extern llvm::cl::opt<std::string>
OptimizedLLVMIRDumpFile;

extern llvm::cl::opt<bool>
DetailedStat;

extern llvm::cl::opt<bool>
UseVTune;

extern llvm::cl::list<Intel::OpenCL::DeviceBackend::IRDumpOptions>
PrintIRAfter;

extern llvm::cl::list<Intel::OpenCL::DeviceBackend::IRDumpOptions>
PrintIRBefore;

extern llvm::cl::opt<std::string>
DumpIRDir;

extern llvm::cl::opt<std::string>
DumpJIT;

extern llvm::cl::opt<std::string>
TimePasses;

namespace Validation
{
    BERunOptions::BERunOptions():
        m_measurePerformance(::TestMode==PERFORMANCE),
        m_useTraceMarks(::TraceMarks),
        m_useVTune(::UseVTune),
        m_runSingleWG(::RunSingleWG),
        m_buildOnly(::TestMode==BUILD),
        m_defaultLocalWGSize(::DefaultLocalWGSize),
        m_buildIterationsCount(::BuildIterations),
        m_executeIterationsCount(::ExecuteIterations),
        m_cpuArch(::CPUArch),
        m_cpuFeatures(::CPUFeatures),
        m_optimizedLLVMIRDumpFile(::OptimizedLLVMIRDumpFile),
        m_transposeSize(::TransposeSize),
        m_PrintIRAfter(::PrintIRAfter),
        m_PrintIRBefore(::PrintIRBefore),
        m_DumpIRDir(::DumpIRDir),
        m_DumpJIT(::DumpJIT),
        m_TimePasses(::TimePasses)
    {
    }

    template<>
    bool BERunOptions::GetValue<bool>(RunConfigurationOption rc, bool defaultValue) const
    {
        switch(rc)
        {
        case RC_COMMON_RUN_SINGLE_WG :
            return m_runSingleWG;
        case RC_BR_USE_PIN_TRACE_MARKS :
            return m_useTraceMarks;
        case RC_BR_USE_VTUNE :
            return m_useVTune;
        case RC_BR_MEASURE_PERFORMANCE :
            return m_measurePerformance;
        case RC_BR_BUILD_ONLY :
            return m_buildOnly;
        default:
            return defaultValue;
        }
    }

    template<>
    uint32_t BERunOptions::GetValue<uint32_t>(RunConfigurationOption rc, uint32_t defaultValue) const
    {
        switch(rc)
        {
        case RC_BR_EXECUTE_ITERATIONS_COUNT :
            return m_executeIterationsCount;
        case RC_BR_BUILD_ITERATIONS_COUNT :
            return m_buildIterationsCount;
        case RC_COMMON_DEFAULT_LOCAL_WG_SIZE :
            return m_defaultLocalWGSize;
        default:
            return defaultValue;
        }
    }

    template<>
    std::string BERunOptions::GetValue<std::string>(RunConfigurationOption rc, std::string defaultValue) const
    {
        switch(rc)
        {
        case RC_BR_CPU_ARCHITECTURE :
            return m_cpuArch;
        case RC_BR_CPU_FEATURES :
            return m_cpuFeatures;
        case RC_BR_DUMP_OPTIMIZED_LLVM_IR :
            return m_optimizedLLVMIRDumpFile;
        case RC_BR_DUMP_IR_DIR :
            return m_DumpIRDir;
        case RC_BR_DUMP_JIT :
            return m_DumpJIT;
        case RC_BR_TIME_PASSES :
            return m_TimePasses;
        default:
            return defaultValue;
        }
    }

    template<>
    ETransposeSize BERunOptions::GetValue<ETransposeSize>(RunConfigurationOption rc, ETransposeSize defaultValue) const
    {
        switch(rc)
        {
        case RC_BR_TRANSPOSE_SIZE :
            return m_transposeSize;
        default:
            return defaultValue;
        }
    }

    template<>
    const std::vector<IRDumpOptions>* BERunOptions::GetValue<const std::vector<IRDumpOptions> * >
                    (RunConfigurationOption rc, const std::vector<IRDumpOptions>* defaultValue) const
    {
        switch(rc)
        {
        case RC_BR_DUMP_IR_AFTER :
            return &m_PrintIRAfter;
        case RC_BR_DUMP_IR_BEFORE :
            return &m_PrintIRBefore;
        default:
            return defaultValue;
        }
        return defaultValue;
    }

    void BERunOptions::InitFromCommandLine()
    {
        m_measurePerformance = (::TestMode == PERFORMANCE);
        m_buildIterationsCount = ::BuildIterations;
        m_executeIterationsCount = ::ExecuteIterations;
        if((!m_measurePerformance) && (m_buildIterationsCount != 1 || m_executeIterationsCount != 1))
        {
            throw Exception::CmdLineException("Specified iterations count"
                " can be used only for performance mode");
        }
        m_transposeSize = ::TransposeSize;
        m_cpuArch = ::CPUArch;
        m_cpuFeatures = ::CPUFeatures;
        m_useTraceMarks = ::TraceMarks;
        m_buildOnly = (::TestMode == BUILD);
        m_runSingleWG = ::RunSingleWG;
        m_useVTune = ::UseVTune;
        m_defaultLocalWGSize = ::DefaultLocalWGSize;
        m_optimizedLLVMIRDumpFile = ::OptimizedLLVMIRDumpFile;
        m_PrintIRAfter = ::PrintIRAfter;
        m_PrintIRBefore = ::PrintIRBefore;
        m_DumpIRDir = ::DumpIRDir;
        m_DumpJIT = ::DumpJIT;
        m_TimePasses = ::TimePasses;
    }

    ComparatorRunOptions::ComparatorRunOptions():
        m_ULP_tolerance(::ULP_tolerance),
        m_detailedStat(::DetailedStat)
    {}

    template <>
    bool ComparatorRunOptions::GetValue<bool>(RunConfigurationOption rc, bool defaultValue) const
    {
        switch(rc)
        {
        case RC_COMP_DETAILED_STAT :
            return m_detailedStat;
        default:
            return defaultValue;
        }
    }

    template <>
    double ComparatorRunOptions::GetValue<double>(RunConfigurationOption rc, double defaultValue) const
    {
        switch(rc)
        {
        case RC_COMP_ULP_TOLERANCE :
            return m_ULP_tolerance;
        default:
            return defaultValue;
        }
    }

    void ComparatorRunOptions::InitFromCommandLine()
    {
        m_ULP_tolerance = ::ULP_tolerance;
        m_detailedStat = ::DetailedStat;
    }

    ReferenceRunOptions::ReferenceRunOptions():
        m_useNEAT(::UseNEAT),
        m_runSingleWG(::RunSingleWG),
        m_defaultLocalWGSize(::DefaultLocalWGSize)
    {}

    template <>
    bool ReferenceRunOptions::GetValue<bool>(RunConfigurationOption rc, bool defaultValue) const
    {
        switch(rc)
        {
        case RC_REF_USE_NEAT :
            return m_useNEAT;
        case RC_COMMON_RUN_SINGLE_WG :
            return m_runSingleWG;
        default:
            return defaultValue;
        }
    }

    template <>
    uint32_t ReferenceRunOptions::GetValue<uint32_t>(RunConfigurationOption rc, uint32_t defaultValue) const
    {
        switch(rc)
        {
        case RC_COMMON_DEFAULT_LOCAL_WG_SIZE:
            return m_defaultLocalWGSize;
        default:
            return defaultValue;
        }
    }

    void ReferenceRunOptions::InitFromCommandLine()
    {
        m_useNEAT = ::UseNEAT;
        m_runSingleWG = ::RunSingleWG;
        m_defaultLocalWGSize = ::DefaultLocalWGSize;
    }

    void OpenCLRunConfiguration::InitFromCommandLine()
    {
        m_useReference = !::NoRef;
        m_forceReference = ::FlagForceRunReference;
        m_testMode = ::TestMode;
        m_backendOptions.InitFromCommandLine();
        m_comparatorOptions.InitFromCommandLine();
        m_referenceOptions.InitFromCommandLine();
    }

    OpenCLRunConfiguration::OpenCLRunConfiguration():
        m_useReference(!::NoRef),
        m_forceReference(::FlagForceRunReference),
        m_testMode(::TestMode)
    {
    }

    bool OpenCLRunConfiguration::UseReference() const
    {
        return m_useReference;
    }

    bool OpenCLRunConfiguration::ForceReference() const
    {
        return m_forceReference;
    }

    TEST_MODE OpenCLRunConfiguration::TestMode() const
    {
        return m_testMode;
    }

    const IRunComponentConfiguration *OpenCLRunConfiguration::GetBackendRunnerConfiguration() const
    {
        return &m_backendOptions;
    }

    const IRunComponentConfiguration *OpenCLRunConfiguration::GetReferenceRunnerConfiguration() const
    {
        return &m_referenceOptions;
    }

    const IRunComponentConfiguration *OpenCLRunConfiguration::GetComparatorConfiguration() const
    {
        return &m_comparatorOptions;
    }
} // namespace Validation

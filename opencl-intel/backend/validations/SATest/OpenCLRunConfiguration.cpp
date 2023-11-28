// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#include "OpenCLRunConfiguration.h"
#include "OpenCLProgramConfiguration.h"
#include "SATest.h"
#include "llvm/Support/CommandLine.h"

using namespace Intel::OpenCL::DeviceBackend;

extern llvm::cl::opt<Validation::TEST_MODE> TestMode;

extern llvm::cl::opt<bool> NoRef;

extern llvm::cl::opt<bool> SDEEnabled;

extern llvm::cl::opt<bool> TraceMarks;

extern llvm::cl::opt<bool> UseNEAT;

extern llvm::cl::opt<bool> RunSingleWG;

extern llvm::cl::opt<uint32_t> ExecuteIterations;

extern llvm::cl::opt<uint32_t> BuildIterations;

extern llvm::cl::opt<bool> EnableSubgroupEmulation;

extern llvm::cl::opt<VectorizerType> OptVectorizerType;

extern llvm::cl::opt<Intel::OpenCL::DeviceBackend::ETransposeSize>
    TransposeSize;

extern llvm::cl::opt<std::string> CPUArch;

extern llvm::cl::opt<std::string> CPUFeatures;

extern llvm::cl::opt<bool> FlagForceRunReference;

extern llvm::cl::opt<double> ULP_tolerance;

extern llvm::cl::opt<uint32_t> DefaultLocalWGSize;

extern llvm::cl::opt<std::string> OptimizedLLVMIRDumpFile;

extern llvm::cl::opt<std::string> PerformanceLog;

extern llvm::cl::opt<bool> DetailedStat;

extern llvm::cl::opt<bool> UseVTune;

extern llvm::cl::opt<bool> StopBeforeJIT;

extern llvm::cl::opt<bool> PrintBuildLog;

extern llvm::cl::opt<std::string> LLVMOption;

extern llvm::cl::opt<bool> DumpHeuristicIR;

extern llvm::cl::opt<std::string> DumpJIT;

extern llvm::cl::opt<bool> DumpKernelProperty;

extern llvm::cl::opt<bool> Verbose;

extern llvm::cl::opt<std::string> TimePasses;

extern llvm::cl::opt<uint64_t> RandomDGSeed;

extern llvm::cl::opt<std::string> ObjectFile;

extern llvm::cl::opt<unsigned> ExpensiveMemOpts;

extern llvm::cl::opt<PassManagerType> OptPassManagerType;

extern llvm::cl::opt<bool> SerializeWorkGroups;

namespace Validation {
BERunOptions::BERunOptions()
    : m_measurePerformance(::TestMode == PERFORMANCE), m_useSDE(::SDEEnabled),
      m_useTraceMarks(::TraceMarks), m_useVTune(::UseVTune),
      m_printBuildLog(::PrintBuildLog), m_runSingleWG(::RunSingleWG),
      m_buildOnly(::TestMode == BUILD), m_expensiveMemOpts(::ExpensiveMemOpts),
      m_defaultLocalWGSize(::DefaultLocalWGSize),
      m_buildIterationsCount(::BuildIterations),
      m_executeIterationsCount(::ExecuteIterations), m_cpuArch(::CPUArch),
      m_cpuFeatures(::CPUFeatures),
      m_optimizedLLVMIRDumpFile(::OptimizedLLVMIRDumpFile),
      m_transposeSize(::TransposeSize), m_DumpJIT(::DumpJIT),
      m_TimePasses(::TimePasses), m_LLVMOption(::LLVMOption),
      m_dumpHeuristcIR(::DumpHeuristicIR),
      m_dumpKernelProperty(::DumpKernelProperty),
      m_vectorizerType(::OptVectorizerType),
      m_enableSubgroupEmulation(::EnableSubgroupEmulation),
      m_passManagerType(::OptPassManagerType),
      m_serializeWorkGroups(::SerializeWorkGroups) {}

template <>
bool BERunOptions::GetValue<bool>(RunConfigurationOption rc,
                                  bool defaultValue) const {
  switch (rc) {
  case RC_COMMON_RUN_SINGLE_WG:
    return m_runSingleWG;
  case RC_BR_USE_SDE:
    return m_useSDE;
  case RC_BR_USE_PIN_TRACE_MARKS:
    return m_useTraceMarks;
  case RC_BR_USE_VTUNE:
    return m_useVTune;
  case RC_BR_PRINT_BUILD_LOG:
    return m_printBuildLog;
  case RC_BR_VERBOSE:
    return m_verbose;
  case RC_BR_MEASURE_PERFORMANCE:
    return m_measurePerformance;
  case RC_BR_BUILD_ONLY:
    return m_buildOnly;
  case RC_BR_STOP_BEFORE_JIT:
    return m_stopBeforeJIT;
  case RC_BR_DUMP_HEURISTIC_IR:
    return m_dumpHeuristcIR;
  case RC_BR_DUMP_KERNEL_PROPERTY:
    return m_dumpKernelProperty;
  case RC_BR_ENABLE_SUBGROUP_EMULATION:
    return m_enableSubgroupEmulation;
  case RC_BR_SERIALIZE_WORK_GROUPS:
    return m_serializeWorkGroups;
  default:
    return defaultValue;
  }
}

template <>
int BERunOptions::GetValue<int>(RunConfigurationOption rc,
                                int defaultValue) const {
  switch (rc) {
  case RC_BR_DEVICE_MODE:
    return m_deviceMode;
  default:
    return defaultValue;
  }
}

template <>
uint32_t BERunOptions::GetValue<uint32_t>(RunConfigurationOption rc,
                                          uint32_t defaultValue) const {
  switch (rc) {
  case RC_BR_EXECUTE_ITERATIONS_COUNT:
    return m_executeIterationsCount;
  case RC_BR_BUILD_ITERATIONS_COUNT:
    return m_buildIterationsCount;
  case RC_COMMON_DEFAULT_LOCAL_WG_SIZE:
    return m_defaultLocalWGSize;
  case RC_BR_EXPENSIVE_MEM_OPT:
    return m_expensiveMemOpts;
  default:
    return defaultValue;
  }
}

template <>
uint64_t BERunOptions::GetValue<uint64_t>(RunConfigurationOption rc,
                                          uint64_t defaultValue) const {
  switch (rc) {
  case RC_COMMON_RANDOM_DG_SEED:
    return m_RandomDataGeneratorSeed;
  default:
    return defaultValue;
  }
}

template <>
std::string
BERunOptions::GetValue<std::string>(RunConfigurationOption rc,
                                    std::string defaultValue) const {
  switch (rc) {
  case RC_BR_CPU_ARCHITECTURE:
    return m_cpuArch;
  case RC_BR_CPU_FEATURES:
    return m_cpuFeatures;
  case RC_BR_DUMP_OPTIMIZED_LLVM_IR:
    return m_optimizedLLVMIRDumpFile;
  case RC_BR_DUMP_JIT:
    return m_DumpJIT;
  case RC_BR_TIME_PASSES:
    return m_TimePasses;
  case RC_BR_LLVM_OPTION:
    return m_LLVMOption;
  case RC_BR_PERF_LOG:
    return m_perfLogFile;
  case RC_BR_OBJECT_FILE:
    return m_InjectedObject;
  default:
    return defaultValue;
  }
}

template <>
VectorizerType
BERunOptions::GetValue<VectorizerType>(RunConfigurationOption rc,
                                       VectorizerType defaultValue) const {
  switch (rc) {
  case RC_BR_VECTORIZER_TYPE:
    return m_vectorizerType;
  default:
    return defaultValue;
  }
}

template <>
PassManagerType
BERunOptions::GetValue<PassManagerType>(RunConfigurationOption rc,
                                        PassManagerType defaultValue) const {
  switch (rc) {
  case RC_BR_PASS_MANAGER_TYPE:
    return m_passManagerType;
  default:
    return defaultValue;
  }
}

template <>
ETransposeSize
BERunOptions::GetValue<ETransposeSize>(RunConfigurationOption rc,
                                       ETransposeSize defaultValue) const {
  switch (rc) {
  case RC_BR_TRANSPOSE_SIZE:
    return m_transposeSize;
  default:
    return defaultValue;
  }
}

template <>
void BERunOptions::SetValue<int>(RunConfigurationOption rc, int setValue) {
  switch (rc) {
  case RC_BR_DEVICE_MODE:
    m_deviceMode = static_cast<DeviceMode>(setValue);
    break;
  default:
    break;
  }
}

void BERunOptions::InitFromCommandLine() {
  m_measurePerformance = (::TestMode == PERFORMANCE);
  m_buildIterationsCount = ::BuildIterations;
  m_executeIterationsCount = ::ExecuteIterations;
  if ((!m_measurePerformance) &&
      (m_buildIterationsCount != 1 || m_executeIterationsCount != 1)) {
    throw Exception::CmdLineException("Specified iterations count"
                                      " can be used only for performance mode");
  }
  m_transposeSize = ::TransposeSize;
  m_cpuArch = ::CPUArch;
  m_cpuFeatures = ::CPUFeatures;
  m_useSDE = ::SDEEnabled;
  m_useTraceMarks = ::TraceMarks;
  m_buildOnly = (::TestMode == BUILD);
  m_stopBeforeJIT = ::StopBeforeJIT;
  m_runSingleWG = ::RunSingleWG;
  m_useVTune = ::UseVTune;
  m_printBuildLog = ::PrintBuildLog;
  m_defaultLocalWGSize = ::DefaultLocalWGSize;
  m_RandomDataGeneratorSeed = ::RandomDGSeed;
  m_optimizedLLVMIRDumpFile = ::OptimizedLLVMIRDumpFile;
  m_perfLogFile = ::PerformanceLog;
  m_DumpJIT = ::DumpJIT;
  m_TimePasses = ::TimePasses;
  m_LLVMOption = ::LLVMOption;
  m_InjectedObject = ::ObjectFile;
  m_dumpHeuristcIR = ::DumpHeuristicIR;
  m_dumpKernelProperty = ::DumpKernelProperty;
  m_verbose = ::Verbose;
  m_vectorizerType = ::OptVectorizerType;
  m_enableSubgroupEmulation = ::EnableSubgroupEmulation;
}

ComparatorRunOptions::ComparatorRunOptions()
    : m_ULP_tolerance(::ULP_tolerance), m_detailedStat(::DetailedStat) {}

template <>
bool ComparatorRunOptions::GetValue<bool>(RunConfigurationOption rc,
                                          bool defaultValue) const {
  switch (rc) {
  case RC_COMP_DETAILED_STAT:
    return m_detailedStat;
  default:
    return defaultValue;
  }
}

template <>
double ComparatorRunOptions::GetValue<double>(RunConfigurationOption rc,
                                              double defaultValue) const {
  switch (rc) {
  case RC_COMP_ULP_TOLERANCE:
    return m_ULP_tolerance;
  default:
    return defaultValue;
  }
}

void ComparatorRunOptions::InitFromCommandLine() {
  m_ULP_tolerance = ::ULP_tolerance;
  m_detailedStat = ::DetailedStat;
}

ReferenceRunOptions::ReferenceRunOptions()
    : m_useNEAT(::UseNEAT), m_runSingleWG(::RunSingleWG),
      m_defaultLocalWGSize(::DefaultLocalWGSize) {}

template <>
bool ReferenceRunOptions::GetValue<bool>(RunConfigurationOption rc,
                                         bool defaultValue) const {
  switch (rc) {
  case RC_REF_USE_NEAT:
    return m_useNEAT;
  case RC_COMMON_RUN_SINGLE_WG:
    return m_runSingleWG;
  default:
    return defaultValue;
  }
}

template <>
uint32_t ReferenceRunOptions::GetValue<uint32_t>(RunConfigurationOption rc,
                                                 uint32_t defaultValue) const {
  switch (rc) {
  case RC_COMMON_DEFAULT_LOCAL_WG_SIZE:
    return m_defaultLocalWGSize;
  default:
    return defaultValue;
  }
}

template <>
uint64_t ReferenceRunOptions::GetValue<uint64_t>(RunConfigurationOption rc,
                                                 uint64_t defaultValue) const {
  switch (rc) {
  case RC_COMMON_RANDOM_DG_SEED:
    return m_RandomDataGeneratorSeed;
  default:
    return defaultValue;
  }
}

void ReferenceRunOptions::InitFromCommandLine() {
  m_useNEAT = ::UseNEAT;
  m_runSingleWG = ::RunSingleWG;
  m_defaultLocalWGSize = ::DefaultLocalWGSize;
  m_RandomDataGeneratorSeed = ::RandomDGSeed;
}

void OpenCLRunConfiguration::InitFromCommandLine() {
  m_useReference = !::NoRef;
  m_forceReference = ::FlagForceRunReference;
  m_testMode = ::TestMode;
  m_backendOptions.InitFromCommandLine();
  m_comparatorOptions.InitFromCommandLine();
  m_referenceOptions.InitFromCommandLine();
}

OpenCLRunConfiguration::OpenCLRunConfiguration()
    : m_useReference(!::NoRef), m_forceReference(::FlagForceRunReference),
      m_testMode(::TestMode) {}

bool OpenCLRunConfiguration::UseReference() const { return m_useReference; }

bool OpenCLRunConfiguration::GetForceReference() const {
  return m_forceReference;
}

void OpenCLRunConfiguration::SetForceReference(bool enable) {
  m_forceReference = enable;
}

TEST_MODE OpenCLRunConfiguration::TestMode() const { return m_testMode; }

IRunComponentConfiguration *
OpenCLRunConfiguration::GetBackendRunnerConfiguration() {
  return &m_backendOptions;
}

const IRunComponentConfiguration *
OpenCLRunConfiguration::GetReferenceRunnerConfiguration() const {
  return &m_referenceOptions;
}

const IRunComponentConfiguration *
OpenCLRunConfiguration::GetComparatorConfiguration() const {
  return &m_comparatorOptions;
}
} // namespace Validation

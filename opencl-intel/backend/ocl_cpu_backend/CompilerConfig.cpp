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

#include "CompilerConfig.h"
#include "PipeCommon.h"
#include "cl_config.h"
#include "cl_env.h"
#include "cl_sys_defines.h"
#include "cl_utils.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/DPCPPStatistic.h"

#include <sstream>
#include <string.h>

extern cl::opt<std::string> OptReqdSubGroupSizes;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

const char* CPU_ARCH_AUTO = "auto";
using Intel::OpenCL::Utils::ConfigFile;

static ETransposeSize parseTransposeSize(int TSize) {
  static constexpr int ValidTSizes[] = {
      TRANSPOSE_SIZE_NOT_SET, TRANSPOSE_SIZE_AUTO, TRANSPOSE_SIZE_1,
      TRANSPOSE_SIZE_4,       TRANSPOSE_SIZE_8,    TRANSPOSE_SIZE_16,
      TRANSPOSE_SIZE_32,      TRANSPOSE_SIZE_64};
  if (std::find(std::begin(ValidTSizes), std::end(ValidTSizes), TSize) ==
      std::end(ValidTSizes))
    return TRANSPOSE_SIZE_INVALID;
  return (ETransposeSize)TSize;
}

void GlobalCompilerConfig::LoadDefaults()
{
    m_enableTiming = false;
    m_disableStackDump = true;
    m_infoOutputFile = "";
    m_LLVMOptions.clear();
    m_targetDevice = CPU_DEVICE;
}

void GlobalCompilerConfig::LoadConfig()
{
  std::string Env;
#ifndef NDEBUG
  if (Intel::OpenCL::Utils::getEnvVar(Env, "VOLCANO_ENABLE_TIMING"))
    m_enableTiming = ConfigFile::ConvertStringToType<bool>(Env);
  if (Intel::OpenCL::Utils::getEnvVar(Env, "VOLCANO_INFO_OUTPUT_FILE"))
    m_infoOutputFile = Env;
#endif // NDEBUG
  if (Intel::OpenCL::Utils::getEnvVar(Env, "CL_DISABLE_STACK_TRACE"))
    m_disableStackDump = ConfigFile::ConvertStringToType<bool>(Env);
#ifndef INTEL_PRODUCT_RELEASE
    // Stat options are set as llvm options for 2 reasons
    // they are available also for opt
    // no need to fuse them all the way down to all passes

    // If environment variable VOLCANO_STATS is set to any non-empty string,
    // then IR containing statistic information will be dumped.
    // If the environment variable is set to 'all' (case-insensitive), all
    // statistic will be dumped, otherwise, only statistic with specified type
    // will be dumped.
    if (Intel::OpenCL::Utils::getEnvVar(Env, "VOLCANO_STATS") && !Env.empty()) {
      DPCPPStatistic::enableStats();
      if (STRCASECMP("all", Env.c_str()))
        DPCPPStatistic::setCurrentStatType(Env.c_str());
    }
    if (Intel::OpenCL::Utils::getEnvVar(Env, "VOLCANO_EQUALIZER_STATS") &&
        !Env.empty()) {
      DPCPPStatistic::enableStats();
      if (STRCASECMP("all", Env.c_str()))
        DPCPPStatistic::setCurrentStatType(Env.c_str());
    }
#endif // INTEL_PRODUCT_RELEASE

// INTEL VPO BEGIN
    m_LLVMOptions.emplace_back("-vector-library=SVML");
// INTEL VPO END

    if (Intel::OpenCL::Utils::getEnvVar(Env, "VOLCANO_LLVM_OPTIONS")) {
      std::vector<std::string> Options = SplitString(Env, ' ');
      m_LLVMOptions.append(Options.begin(), Options.end());
    }

    if (Intel::OpenCL::Utils::getEnvVar(Env,
                                        "CL_CONFIG_CPU_REQD_SUB_GROUP_SIZE"))
      m_LLVMOptions.emplace_back("-" + OptReqdSubGroupSizes.ArgStr.str() + "="
                                 + Env);
}

void GlobalCompilerConfig::ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions)
{
    if( nullptr == pBackendOptions)
        return;

    m_infoOutputFile = pBackendOptions->GetStringValue(
        (int)CL_DEV_BACKEND_OPTION_TIME_PASSES, m_infoOutputFile.c_str());
    m_enableTiming = !m_infoOutputFile.empty();

    std::string debugPassManager = pBackendOptions->GetStringValue(
        (int)CL_DEV_BACKEND_OPTION_DEBUG_PASS_MANAGER, "");
    PassManagerType passManagerType =
        static_cast<PassManagerType>(pBackendOptions->GetIntValue(
            CL_DEV_BACKEND_OPTION_PASS_MANAGER_TYPE, PM_NONE));
    if ((PM_LTO_LEGACY == passManagerType ||
         PM_OCL_LEGACY == passManagerType) &&
        !debugPassManager.empty())
      m_LLVMOptions.emplace_back("-debug-pass=" + debugPassManager);

    std::string LLVMOption = pBackendOptions->GetStringValue(
        (int)CL_DEV_BACKEND_OPTION_LLVM_OPTION, "");
    if (!LLVMOption.empty()) {
      std::vector<std::string> Options = SplitString(LLVMOption, ' ');
      m_LLVMOptions.append(Options.begin(), Options.end());
    }

    // C++ pipeline command line options.
    m_LLVMOptions.emplace_back("-enable-vec-clone=false");
    ETransposeSize TransposeSize =
        parseTransposeSize(pBackendOptions->GetIntValue(
            (int)CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE, TRANSPOSE_SIZE_NOT_SET));
    if (TRANSPOSE_SIZE_1 == TransposeSize)
      m_LLVMOptions.emplace_back("-vplan-driver=false");

    if (TRANSPOSE_SIZE_AUTO != TransposeSize &&
        TRANSPOSE_SIZE_NOT_SET != TransposeSize)
      m_LLVMOptions.emplace_back("-dpcpp-force-vf=" + std::to_string((int)TransposeSize));

    m_targetDevice = static_cast<DeviceMode>(pBackendOptions->GetIntValue(
        (int)CL_DEV_BACKEND_OPTION_DEVICE, CPU_DEVICE));

    if (FPGA_EMU_DEVICE == m_targetDevice)
    {
        int channelDepthEmulationMode = pBackendOptions->GetIntValue(
            (int)CL_DEV_BACKEND_OPTION_CHANNEL_DEPTH_EMULATION_MODE,
            (int)CHANNEL_DEPTH_MODE_STRICT);
        m_LLVMOptions.emplace_back("--dpcpp-channel-depth-emulation-mode="
            + std::to_string(channelDepthEmulationMode));
        m_LLVMOptions.emplace_back("--dpcpp-remove-fpga-reg");
        m_LLVMOptions.emplace_back("--dpcpp-demangle-fpga-pipes");
    }

    bool EnableSubgroupEmulation =
      pBackendOptions->GetBooleanValue(
        (int)CL_DEV_BACKEND_OPTION_SUBGROUP_EMULATION, true);
    if (!EnableSubgroupEmulation)
      m_LLVMOptions.emplace_back("-dpcpp-enable-subgroup-emulation=false");

    // Set machinesink's machine-sink-load-instrs-threshold and
    // machine-sink-load-blocks-threshold to 0 otherwise it may possibly
    // increase ocl's jit compile time dramatically, especially for large
    // kernels.
    m_LLVMOptions.emplace_back("-machine-sink-load-instrs-threshold=0");
    m_LLVMOptions.emplace_back("-machine-sink-load-blocks-threshold=0");
}

void CompilerConfig::LoadDefaults()
{
    m_cpuArch = CPU_ARCH_AUTO;
    m_cpuMaxWGSize = CPU_MAX_WORK_GROUP_SIZE;
    m_transposeSize = TRANSPOSE_SIZE_NOT_SET;
    m_rtLoopUnrollFactor = 1;
    m_cpuFeatures = "";
    m_useVTune = true;
    m_serializeWorkGroups = false;
    m_loadBuiltins = true;
    m_dumpHeuristicIR = false;
    m_streamingAlways = false;
    m_expensiveMemOpts = 0;
    m_targetDevice = CPU_DEVICE;
    m_forcedPrivateMemorySize = 0;
    m_useAutoMemory = false;
    m_passManagerType = PM_NONE;
}

void CompilerConfig::LoadConfig()
{
  std::string Env;
  // TODO: Add validation code
  if (Intel::OpenCL::Utils::getEnvVar(Env, "CL_CONFIG_CPU_TARGET_ARCH"))
    m_cpuArch = Env;
#ifndef NDEBUG
  if (Intel::OpenCL::Utils::getEnvVar(Env, "VOLCANO_CPU_FEATURES")) {
    // The validity of the cpud features are checked upon parsing of optimizer
    // options
    m_cpuFeatures = Env;
  }

  if (Intel::OpenCL::Utils::getEnvVar(Env, "VOLCANO_DEBUG"))
    llvm::DebugFlag = true;
  if (Intel::OpenCL::Utils::getEnvVar(Env, "VOLCANO_DEBUG_ONLY"))
    llvm::setCurrentDebugType(Env.c_str());
#endif // NDEBUG
#ifndef INTEL_PRODUCT_RELEASE
  if (Intel::OpenCL::Utils::getEnvVar(Env, "CL_CONFIG_DUMP_FILE_NAME_PREFIX")) {
    // base name for stat files
    m_dumpFilenamePrefix = Env;
  }
#endif // INTEL_PRODUCT_RELEASE
}

void CompilerConfig::ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions)
{
    if( nullptr == pBackendOptions)
    {
        return;
    }
    m_cpuArch       = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_SUBDEVICE, m_cpuArch.c_str());
    m_cpuFeatures   = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_SUBDEVICE_FEATURES, m_cpuFeatures.c_str());
    m_cpuMaxWGSize  = (size_t)pBackendOptions->GetIntValue(
        (int)CL_DEV_BACKEND_OPTION_CPU_MAX_WG_SIZE, (int)m_cpuMaxWGSize);

    m_transposeSize = parseTransposeSize(pBackendOptions->GetIntValue(
        (int)CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE, m_transposeSize));
    m_rtLoopUnrollFactor  = pBackendOptions->GetIntValue((int) CL_DEV_BACKEND_OPTION_RT_LOOP_UNROLL_FACTOR, m_rtLoopUnrollFactor);
    m_useVTune      = pBackendOptions->GetBooleanValue((int)CL_DEV_BACKEND_OPTION_USE_VTUNE, m_useVTune);
    m_serializeWorkGroups = pBackendOptions->GetBooleanValue(
        (int)CL_DEV_BACKEND_OPTION_SERIALIZE_WORK_GROUPS,
        m_serializeWorkGroups);
    m_forcedPrivateMemorySize = pBackendOptions->GetIntValue((int)CL_DEV_BACKEND_OPTION_FORCED_PRIVATE_MEMORY_SIZE, m_forcedPrivateMemorySize);
    m_useAutoMemory = pBackendOptions->GetBooleanValue((int)CL_DEV_BACKEND_OPTION_USE_AUTO_MEMORY, m_useAutoMemory);
    m_dumpHeuristicIR = pBackendOptions->GetBooleanValue((int)CL_DEV_BACKEND_OPTION_DUMP_HEURISTIC_IR, m_dumpHeuristicIR);
    m_streamingAlways = pBackendOptions->GetBooleanValue(
        (int)CL_DEV_BACKEND_OPTION_STREAMING_ALWAYS, m_streamingAlways);
    m_expensiveMemOpts = pBackendOptions->GetIntValue((int)CL_DEV_BACKEND_OPTION_EXPENSIVE_MEM_OPTS, m_expensiveMemOpts);
    m_targetDevice = static_cast<DeviceMode>(pBackendOptions->GetIntValue(
        (int)CL_DEV_BACKEND_OPTION_DEVICE, CPU_DEVICE));
    m_passManagerType =
        static_cast<PassManagerType>(pBackendOptions->GetIntValue(
            CL_DEV_BACKEND_OPTION_PASS_MANAGER_TYPE, PM_NONE));

    // Adjust m_forcedPrivateMemorySize if it is not set.
    if (m_forcedPrivateMemorySize == 0)
      m_forcedPrivateMemorySize =
          (m_targetDevice == FPGA_EMU_DEVICE && m_useAutoMemory)
              ? FPGA_DEV_MAX_WG_PRIVATE_SIZE
              : CPU_DEV_MAX_WG_PRIVATE_SIZE;
}

}}}

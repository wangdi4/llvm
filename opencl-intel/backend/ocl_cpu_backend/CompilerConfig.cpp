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
#include "OclTune.h"
#include "PipeCommon.h"
#include "cl_config.h"
#include "cl_sys_defines.h"
#include "cl_utils.h"
#include "llvm/Support/Debug.h"

#include <sstream>
#include <stdlib.h> // getenv
#include <string.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

const char* CPU_ARCH_AUTO = "auto";
using Intel::OpenCL::Utils::ConfigFile;

void GlobalCompilerConfig::LoadDefaults()
{
    m_enableTiming = false;
    m_disableStackDump = true;
    m_infoOutputFile = "";
    m_LLVMOptions = "";
    m_targetDevice = CPU_DEVICE;
}

void GlobalCompilerConfig::LoadConfig()
{
#ifndef NDEBUG
    if (const char *pEnv = getenv("VOLCANO_ENABLE_TIMING"))
    {
        m_enableTiming = !strcmp(pEnv, "TRUE");
    }
    if (const char *pEnv = getenv("VOLCANO_INFO_OUTPUT_FILE"))
    {
        m_infoOutputFile = pEnv;
    }
#endif // NDEBUG
    if (const char *pEnv = getenv("CL_DISABLE_STACK_TRACE"))
    {
      m_disableStackDump = ConfigFile::ConvertStringToType<bool>(pEnv);
    }
#ifndef INTEL_PRODUCT_RELEASE
    // Stat options are set as llvm options for 2 reasons
    // they are available also for opt
    // no need to fuse them all the way down to all passes

    // If environment variable VOLCANO_STATS is set to any non-empty string,
    // then IR containing statistic information will be dumped.
    // If the environment variable is set to 'all' (case-insensitive), all
    // statistic will be dumped, otherwise, only statistic with specified type
    // will be dumped.
    if (const char *pEnv = getenv("VOLCANO_STATS"))
    {
        if (pEnv[0] != 0) {
            intel::Statistic::enableStats();
            if (STRCASECMP("all", pEnv))
                intel::Statistic::setCurrentStatType(pEnv);
        }
    }
#endif // INTEL_PRODUCT_RELEASE

// INTEL VPO BEGIN
    m_LLVMOptions = "-vector-library=SVML ";
// INTEL VPO END

    if (const char *pEnv = getenv("VOLCANO_LLVM_OPTIONS"))
    {
        m_LLVMOptions += pEnv;
    }

    if (const char *pEnv = getenv("CL_CONFIG_CPU_REQD_SUB_GROUP_SIZE"))
    {
        m_LLVMOptions += "-reqd-sub-group-size=";
        m_LLVMOptions += pEnv;
    }
}

void GlobalCompilerConfig::ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions)
{
    if( nullptr == pBackendOptions)
    {
        return;
    }
    m_infoOutputFile = pBackendOptions->GetStringValue(
        (int)CL_DEV_BACKEND_OPTION_TIME_PASSES, m_infoOutputFile.c_str());
    m_enableTiming = !m_infoOutputFile.empty();

    m_targetDevice = static_cast<DeviceMode>(pBackendOptions->GetIntValue(
        (int)CL_DEV_BACKEND_OPTION_DEVICE, CPU_DEVICE));

    if (FPGA_EMU_DEVICE == m_targetDevice)
    {
        int channelDepthEmulationMode = pBackendOptions->GetIntValue(
            (int)CL_DEV_BACKEND_OPTION_CHANNEL_DEPTH_EMULATION_MODE,
            (int)CHANNEL_DEPTH_MODE_STRICT);
        m_LLVMOptions += " --channel-depth-emulation-mode="
            + std::to_string(channelDepthEmulationMode);
        m_LLVMOptions += " --remove-fpga-reg --demangle-fpga-pipes";
    }
    else if (EYEQ_EMU_DEVICE == m_targetDevice)
    {
        m_LLVMOptions += " -eyeq-div-crash-behavior";
    }

    if (m_LLVMOptions.find("-enable-vplan-kernel-vectorizer")
        == std::string::npos) {
      // If VOLCANO_LLVM_OPTIONS doesn't try to override vectorizer choice
      // look at CL_CONFIG_CPU_VECTORIZER_MODE. Check also whether
      // VOLCANO_VECTORIZER_OPTIONS wants to override
      // enable-default-kernel-vectorizer.
      // Can be verified with -emit-vectorizer-sign-on
      VectorizerType VType =
        static_cast<VectorizerType>(pBackendOptions->GetIntValue(
            (int)CL_DEV_BACKEND_OPTION_VECTORIZER_TYPE, DEFAULT_VECTORIZER));
      if (VType == VOLCANO_VECTORIZER)
        m_LLVMOptions += " -enable-vplan-kernel-vectorizer=0";
    }

    bool EnableNativeSubgroups =
      pBackendOptions->GetBooleanValue(
        (int)CL_DEV_BACKEND_OPTION_NATIVE_SUBGROUPS, false);
    if (EnableNativeSubgroups) {
        m_LLVMOptions += " -enable-native-opencl-subgroups";
    }

    bool EnableSubgroupEmulation =
      pBackendOptions->GetBooleanValue(
        (int)CL_DEV_BACKEND_OPTION_SUBGROUP_EMULATION, true);
    if (!EnableSubgroupEmulation) {
        m_LLVMOptions += " -enable-subgroup-emulation=false";
    }

    if (pBackendOptions->GetBooleanValue(CL_DEV_BACKEND_OPTION_LTO_LEGACY_PM,
                                         false))
      m_LLVMOptions += " -enable-dpcpp-kernel-transforms=true";
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
    m_DumpIROptionAfter = NULL;
    m_DumpIROptionBefore = NULL;
    m_dumpHeuristicIR = false;
    m_streamingAlways = false;
    m_expensiveMemOpts = 0;
    m_targetDevice = CPU_DEVICE;
    m_forcedPrivateMemorySize = 0;
    m_useAutoMemory = false;
    m_useLTOLegacyPM = false;
}

void CompilerConfig::LoadConfig()
{
    //TODO: Add validation code
    if (const char *pEnv = getenv("CL_CONFIG_CPU_TARGET_ARCH"))
    {
        m_cpuArch = pEnv;
    }

    if (const char *pEnv = getenv("CL_CONFIG_CPU_VECTORIZER_MODE"))
    {
        unsigned int size;
        if ((std::stringstream(pEnv) >> size).fail())
        {
            throw  Exceptions::BadConfigException("Failed to load the transpose size from environment");
        }
        m_transposeSize = ETransposeSize(size);
    }
#ifndef NDEBUG
    if (const char *pEnv = getenv("VOLCANO_CPU_FEATURES"))
    {
        // The validity of the cpud features are checked upon parsing of optimizer options
        m_cpuFeatures = pEnv;
    }

    if (getenv("VOLCANO_DEBUG"))
    {
      llvm::DebugFlag = true;
    }
    if (const char *pEnv = getenv("VOLCANO_DEBUG_ONLY"))
    {
      llvm::setCurrentDebugType(pEnv);
    }
#endif // NDEBUG
#ifndef INTEL_PRODUCT_RELEASE
    if (const char *pEnv = getenv("VOLCANO_IR_FILE_BASE_NAME"))
    {
        // base name for stat files
        m_statFileBaseName = pEnv;
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

    std::string ForcedWGSize = pBackendOptions->GetStringValue(
        CL_DEV_BACKEND_OPTION_FORCED_WG_SIZE, "");
    (void)SplitStringInteger(ForcedWGSize, ',', m_forcedWGSize);

    m_transposeSize = (ETransposeSize)pBackendOptions->GetIntValue((int)CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE, m_transposeSize);
    m_rtLoopUnrollFactor  = pBackendOptions->GetIntValue((int) CL_DEV_BACKEND_OPTION_RT_LOOP_UNROLL_FACTOR, m_rtLoopUnrollFactor);
    m_useVTune      = pBackendOptions->GetBooleanValue((int)CL_DEV_BACKEND_OPTION_USE_VTUNE, m_useVTune);
    m_serializeWorkGroups = pBackendOptions->GetBooleanValue(
        (int)CL_DEV_BACKEND_OPTION_SERIALIZE_WORK_GROUPS,
        m_serializeWorkGroups);
    m_forcedPrivateMemorySize = pBackendOptions->GetIntValue((int)CL_DEV_BACKEND_OPTION_FORCED_PRIVATE_MEMORY_SIZE, m_forcedPrivateMemorySize);
    m_useAutoMemory = pBackendOptions->GetBooleanValue((int)CL_DEV_BACKEND_OPTION_USE_AUTO_MEMORY, m_useAutoMemory);
    pBackendOptions->GetValue((int)OPTION_IR_DUMPTYPE_AFTER, &m_DumpIROptionAfter, 0);
    pBackendOptions->GetValue((int)OPTION_IR_DUMPTYPE_BEFORE, &m_DumpIROptionBefore, 0);
    m_dumpIRDir     = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_DUMP_IR_DIR, m_dumpIRDir.c_str());
    m_dumpHeuristicIR = pBackendOptions->GetBooleanValue((int)CL_DEV_BACKEND_OPTION_DUMP_HEURISTIC_IR, m_dumpHeuristicIR);
    m_streamingAlways = pBackendOptions->GetBooleanValue(
        (int)CL_DEV_BACKEND_OPTION_STREAMING_ALWAYS, m_streamingAlways);
    m_expensiveMemOpts = pBackendOptions->GetIntValue((int)CL_DEV_BACKEND_OPTION_EXPENSIVE_MEM_OPTS, m_expensiveMemOpts);
    m_targetDevice = static_cast<DeviceMode>(pBackendOptions->GetIntValue(
        (int)CL_DEV_BACKEND_OPTION_DEVICE, CPU_DEVICE));
    m_useLTOLegacyPM = pBackendOptions->GetBooleanValue(
        CL_DEV_BACKEND_OPTION_LTO_LEGACY_PM, m_useLTOLegacyPM);
}

}}}

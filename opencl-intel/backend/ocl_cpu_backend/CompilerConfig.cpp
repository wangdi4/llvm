/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2014).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  CompilerConfig.cpp

\*****************************************************************************/

#include "CompilerConfig.h"
#include "OclTune.h"

#include "llvm/Support/Debug.h"

#include <sstream>
#include <stdlib.h> // getenv
#include <string.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

const char* CPU_ARCH_AUTO = "auto";

void GlobalCompilerConfig::LoadDefaults()
{
    m_enableTiming = false;
    m_disableStackDump = false;
    m_infoOutputFile = "";
    m_LLVMOptions = "";
}

static bool parseBool(const char *val) {
    if (!strcmp(val, "0")     ||
        !strcmp(val, "FALSE") ||
        !strcmp(val, "False") ||
        !strcmp(val, "false") ||
        !strcmp(val, "NO")    ||
        !strcmp(val, "No")    ||
        !strcmp(val, "no")    ||
        !strcmp(val, "F")     ||
        !strcmp(val, "f")     ||
        !strcmp(val, "N")     ||
        !strcmp(val, "n")     ||
        !strcmp(val, "NONE")  ||
        !strcmp(val, "None")  ||
        !strcmp(val, "none"))
        return false;
    return true;
}

void GlobalCompilerConfig::LoadConfig()
{
#ifndef NDEBUG
    if (const char *pEnv = getenv("VOLCANO_ENABLE_TIMING"))
    {
        m_enableTiming = !strcmp(pEnv, "TRUE");
    }
    if (const char *pEnv = getenv("CL_DISABLE_STACK_TRACE"))
    {
        m_disableStackDump = parseBool(pEnv);
    }
    if (const char *pEnv = getenv("VOLCANO_INFO_OUTPUT_FILE"))
    {
        m_infoOutputFile = pEnv;
    }
#endif // NDEBUG
#ifndef INTEL_PRODUCT_RELEASE
    // Stat options are set as llvm options for 2 reasons
    // they are available also for opt
    // no need to fuse them all the way down to all passes
    if (const char *pEnv = getenv("VOLCANO_STATS"))
    {
        intel::Statistic::enableStats();
        if (pEnv[0] != 0 && strcmp("ALL", pEnv) && strcmp("all", pEnv))
        {
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
}

void GlobalCompilerConfig::ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions)
{
    if( nullptr == pBackendOptions)
    {
        return;
    }
    m_infoOutputFile = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_TIME_PASSES, "");
    m_enableTiming = !m_infoOutputFile.empty();
    m_disableStackDump = pBackendOptions->GetBooleanValue((int)CL_DEV_BACKEND_OPTION_DISABLE_STACKDUMP, false);
}

void CompilerConfig::LoadDefaults()
{
    m_cpuArch = CPU_ARCH_AUTO;
    m_transposeSize = TRANSPOSE_SIZE_NOT_SET;
    m_rtLoopUnrollFactor = 1;
    m_cpuFeatures = "";
    m_useVTune = true;
}

void CompilerConfig::LoadConfig()
{
    //TODO: Add validation code
    if (const char *pEnv = getenv("VOLCANO_CPU_ARCH"))
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
    m_transposeSize = (ETransposeSize)pBackendOptions->GetIntValue((int)CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE, m_transposeSize);
    m_rtLoopUnrollFactor  = pBackendOptions->GetIntValue((int) CL_DEV_BACKEND_OPTION_RT_LOOP_UNROLL_FACTOR, m_rtLoopUnrollFactor);
    m_useVTune      = pBackendOptions->GetBooleanValue((int)CL_DEV_BACKEND_OPTION_USE_VTUNE, m_useVTune);
    m_forcedPrivateMemorySize = pBackendOptions->GetIntValue((int)CL_DEV_BACKEND_OPTION_FORCED_PRIVATE_MEMORY_SIZE, m_forcedPrivateMemorySize);
    pBackendOptions->GetValue((int)OPTION_IR_DUMPTYPE_AFTER, &m_DumpIROptionAfter, 0);
    pBackendOptions->GetValue((int)OPTION_IR_DUMPTYPE_BEFORE, &m_DumpIROptionBefore, 0);
    m_dumpIRDir     = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_DUMP_IR_DIR, m_dumpIRDir.c_str());
    m_dumpHeuristicIR = pBackendOptions->GetBooleanValue((int)CL_DEV_BACKEND_OPTION_DUMP_HEURISTIC_IR, m_dumpHeuristicIR);
}

}}}

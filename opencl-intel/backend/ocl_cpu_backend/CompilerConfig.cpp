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

#include "CPUDetect.h"
#include "CompilerConfig.h"

#include "llvm/Support/Debug.h"

#include <stdlib.h> // getenv
#include <sstream>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

const char* CPU_ARCH_AUTO = "auto";

void CompilerConfig::LoadDefaults()
{
    m_cpuArch = CPU_ARCH_AUTO;
    m_transposeSize = TRANSPOSE_SIZE_AUTO;
    m_cpuFeatures = "";
    m_useVTune = true;
}

void CompilerConfig::SkipBuiltins()
{
    m_loadBuiltins = false;
}

void CompilerConfig::LoadConfig()
{
    //TODO: Add validation code
    if (const char *pEnv = getenv("VOLCANO_CPU_ARCH"))
    {
        m_cpuArch = pEnv;
    }

    if (const char *pEnv = getenv("VOLCANO_TRANSPOSE_SIZE"))
    {
        unsigned int size;
        if ((std::stringstream(pEnv) >> size).fail())
        {
            throw  Exceptions::BadConfigException("Failed to load the transpose size from environment");
        }
        m_transposeSize = ETransposeSize(size);
    }

    if (const char *pEnv = getenv("VOLCANO_CPU_FEATURES"))
    {
        // The validity of the cpud features are checked upon parsing of optimizer options
        m_cpuFeatures = pEnv;
    }
#ifndef NDEBUG
    if (getenv("VOLCANO_DEBUG"))
    {
      llvm::DebugFlag = true;
    }
    if (const char *pEnv = getenv("VOLCANO_DEBUG_ONLY"))
    {
      llvm::setCurrentDebugType(pEnv);
    }
#endif
}

}}}

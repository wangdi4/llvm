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

File Name:  BackendConfiguration.cpp

\*****************************************************************************/

#include "BackendConfiguration.h"
#include "exceptions.h"
#include "CPUDetect.h"
#include "cl_dev_backend_api.h"
#include "MICSerializationService.h"
#include "TargetDescription.h"

#include <assert.h>
#include <string>
#include <cstring>
#include <sstream>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

using Utils::CPUDetect;
const char* CPU_ARCH_AUTO = "auto";
const char* CPU_ARCH_AUTO_REMOTE = "auto-remote";

namespace Utils
{
    OPERATION_MODE SelectOperationMode(const char* cpuArch)
    {
        if(0 == strcmp(cpuArch, CPU_ARCH_AUTO)) return CPU_MODE;
        if(0 == strcmp(cpuArch, CPU_ARCH_AUTO_REMOTE)) return MIC_MODE;
        if (!Utils::CPUDetect::GetInstance()->IsValidCPUName(cpuArch))
        {
            throw Exceptions::DeviceBackendExceptionBase("Unsupported operation mode", CL_DEV_INVALID_OPERATION_MODE);
        }
        return (Utils::CPUDetect::GetInstance()->IsMICCPU(Utils::CPUDetect::GetInstance()->GetCPUByName(cpuArch))) 
            ? MIC_MODE : CPU_MODE;
    }
}

DEFINE_EXCEPTION(BadConfigException)

BackendConfiguration* BackendConfiguration::s_pInstance = NULL;

BackendConfiguration::BackendConfiguration()
{}

BackendConfiguration::~BackendConfiguration()
{}

void BackendConfiguration::Init(const ICLDevBackendOptions* pBackendOptions)
{
    assert(!s_pInstance);
    s_pInstance = new BackendConfiguration();
}

void BackendConfiguration::Terminate()
{
    if( NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

const BackendConfiguration* BackendConfiguration::GetInstance()
{
    assert(s_pInstance);
    return s_pInstance;
}

CompilerConfiguration BackendConfiguration::GetCPUCompilerConfig() const 
{
    CompilerConfiguration CPUCompilerConfig;

    CPUCompilerConfig.LoadDefaults();
    CPUCompilerConfig.LoadConfig(); 

    return CPUCompilerConfig; 
}

MICCompilerConfiguration BackendConfiguration::GetMICCompilerConfig() const
{
    MICCompilerConfiguration MICCompilerConfig;

    MICCompilerConfig.LoadDefaults();
    MICCompilerConfig.LoadConfig();  

    return MICCompilerConfig; 
}

void CompilerConfiguration::LoadDefaults()
{
    m_cpuArch = CPU_ARCH_AUTO;
    m_transposeSize = TRANSPOSE_SIZE_AUTO;
    m_cpuFeatures = "";
    m_useVTune = true;
}

void CompilerConfiguration::SkipBuiltins()
{
    m_loadBuiltins = false;
}

void CompilerConfiguration::LoadConfig()
{
    //TODO: Add validation code
    if (const char *pEnv = getenv("VOLCANO_ARCH"))
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
}

void CompilerConfiguration::ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions)
{
    if( NULL == pBackendOptions)
    {
        return;
    }
    m_cpuArch       = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_CPU_ARCH, m_cpuArch.c_str());
    m_cpuFeatures   = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_CPU_FEATURES, m_cpuFeatures.c_str());
    m_transposeSize = (ETransposeSize)pBackendOptions->GetIntValue((int)CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE, m_transposeSize);
    m_useVTune      = pBackendOptions->GetBooleanValue((int)CL_DEV_BACKEND_OPTION_USE_VTUNE, m_useVTune);
    pBackendOptions->GetValue((int)OPTION_IR_DUMPTYPE_AFTER, &m_DumpIROptionAfter, 0);
    pBackendOptions->GetValue((int)OPTION_IR_DUMPTYPE_BEFORE, &m_DumpIROptionBefore, 0);
    m_dumpIRDir     = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_DUMP_IR_DIR, m_dumpIRDir.c_str());
    m_TimePasses    = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_TIME_PASSES, m_TimePasses.c_str());
}


void MICCompilerConfiguration::LoadDefaults()
{
    m_cpuArch = CPU_ARCH_AUTO_REMOTE;
    m_transposeSize = TRANSPOSE_SIZE_AUTO;
    m_cpuFeatures = "";
    m_useVTune = true;
}

void MICCompilerConfiguration::SkipBuiltins()
{
    m_loadBuiltins = false;
}

void MICCompilerConfiguration::LoadConfig()
{
    //TODO: Add validation code
    if (const char *pEnv = getenv("VOLCANO_ARCH"))
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
}

void MICCompilerConfiguration::ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions)
{
    if( NULL == pBackendOptions)
    {
        return;
    }
    m_cpuArch       = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_CPU_ARCH, m_cpuArch.c_str());
    m_cpuFeatures   = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_CPU_FEATURES, m_cpuFeatures.c_str());
    m_transposeSize = (ETransposeSize)pBackendOptions->GetIntValue((int)CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE, m_transposeSize);
    m_useVTune      = pBackendOptions->GetBooleanValue((int)CL_DEV_BACKEND_OPTION_USE_VTUNE, m_useVTune);
    pBackendOptions->GetValue((int)OPTION_IR_DUMPTYPE_AFTER, &m_DumpIROptionAfter, 0);
    pBackendOptions->GetValue((int)OPTION_IR_DUMPTYPE_BEFORE, &m_DumpIROptionBefore, 0);
    m_dumpIRDir     = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_DUMP_IR_DIR, m_dumpIRDir.c_str());
    m_TimePasses    = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_TIME_PASSES, m_TimePasses.c_str());

    size_t targetDescriptionSize = pBackendOptions->GetIntValue(CL_DEV_BACKEND_OPTION_TARGET_DESC_SIZE, 0);
    if(0 != targetDescriptionSize)
    {
        char* pTargetDescriptionBlob = new char[targetDescriptionSize];
        
        bool ret = pBackendOptions->GetValue(CL_DEV_BACKEND_OPTION_TARGET_DESC_BLOB, pTargetDescriptionBlob, 
            &targetDescriptionSize);
        if(!ret) 
        {
            delete[] pTargetDescriptionBlob;
            throw Exceptions::BadConfigException("Failed to get target description");
        }

        MICSerializationService mss(NULL);
        TargetDescription* pTarget = NULL;
        
        // check if error
        cl_dev_err_code errCode = 
            mss.DeSerializeTargetDescription(&pTarget, pTargetDescriptionBlob, targetDescriptionSize);
        delete[] pTargetDescriptionBlob;

        if(CL_DEV_SUCCESS != errCode) 
        {
            throw Exceptions::BadConfigException("Failed to read target description");
        }

        m_TargetDescription = *pTarget;
        delete pTarget;
    }
}

}}}

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

File Name:  MICCompilerConfig.cpp

\*****************************************************************************/

#include "ICLDevBackendOptions.h"
#include "MICCompilerConfig.h"
#include "MICSerializationService.h"
#include "TargetDescription.h"

#include "llvm/Support/Debug.h"

#include <sstream>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

void MICCompilerConfig::LoadConfig()
{
    //TODO: Add validation code

    if (const char *pEnv = getenv("VOLCANO_TRANSPOSE_SIZE"))
    {
        unsigned int size;
        if ((std::stringstream(pEnv) >> size).fail())
        {
            throw  Exceptions::BadConfigException("Failed to load the transpose size from environment");
        }
        m_transposeSize = ETransposeSize(size);
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

#ifdef OCLT
    if (const char *pEnv = getenv("VOLCANO_IR_FILE_BASE_NAME"))
    {
        // base name for stat files
        m_statFileBaseName = pEnv;
    }
#endif // OCLT
}

void MICCompilerConfig::ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions)
{
    CompilerConfig::ApplyRuntimeOptions(pBackendOptions);

    if( nullptr == pBackendOptions)
    {
        return;
    }

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

        MICSerializationService mss(nullptr);
        TargetDescription* pTarget = nullptr;

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


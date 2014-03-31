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

File Name:  CPUSerializationService.h

\*****************************************************************************/
#ifndef __CPU_SERIALIZATION_SERVICE
#define __CPU_SERIALIZATION_SERVICE

#include "cl_dev_backend_api.h"
#include "IAbstractBackendFactory.h"
#include <map>
#include <string>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class CPUSerializationService : public ICLDevBackendSerializationService
{
public:
    
    CPUSerializationService(const ICLDevBackendOptions* pBackendOptions);

    // Program Functions
    virtual cl_dev_err_code GetSerializationBlobSize(
        cl_serialization_type serializationType,
        const ICLDevBackendProgram_* pProgram, size_t* pSize) const;

    virtual cl_dev_err_code SerializeProgram(
        cl_serialization_type serializationType, 
        const ICLDevBackendProgram_* pProgram, 
        void* pBlob, size_t blobSize) const;

    virtual cl_dev_err_code ReloadProgram(
        cl_serialization_type serializationType, 
        ICLDevBackendProgram_* pProgram, 
        const void* pBlob, size_t blobSize) const;

    virtual cl_dev_err_code DeSerializeProgram(
        cl_serialization_type serializationType, 
        ICLDevBackendProgram_** ppProgram, 
        const void* pBlob, size_t blobSize) const;
    
    virtual void ReleaseProgram(ICLDevBackendProgram_* pProgram) const;

    virtual void Release();
private:
    IAbstractBackendFactory* m_pBackendFactory;
};

}}} // namespace

#endif // __CPU_SERIALIZATION_SERVICE

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

File Name:  MICExecutionService.h

\*****************************************************************************/
#pragma once

#include "cl_dev_backend_api.h"
#include "ExecutionService.h"
#include "TargetDescription.h"
#include "DynamicLibraryLoader.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class MICExecutionService: public ExecutionService
{
public:
    MICExecutionService(const ICLDevBackendOptions* pOptions, const Intel::CPUId&
        cpuId);
	
    virtual size_t GetTargetMachineDescriptionSize() const;
    
    virtual cl_dev_err_code GetTargetMachineDescription(
        void* pTargetDescription, 
        size_t descriptionSize) const;
        
    virtual cl_dev_err_code CreateExecutable(
        ICLDevBackendBinary_* pBinary, 
        void** pExecutionMemoryResources, 
        unsigned int resourcesCount, 
        ICLDevBackendExecutable_** ppExecutable) const;
        
private:
    DynamicLibraryLoader m_Loader;
    TargetDescription m_TargetDescription;
};

}}}

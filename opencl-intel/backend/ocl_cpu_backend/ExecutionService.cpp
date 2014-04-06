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

File Name:  ExecutionService.cpp

\*****************************************************************************/

#include "ExecutionService.h"
#include <assert.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

ExecutionService::ExecutionService(const ICLDevBackendOptions* pOptions)
{
    // obtain Device Command Manager
    assert(pOptions && "pOptions are NULL");
}

size_t ExecutionService::GetTargetMachineDescriptionSize() const
{
    assert(false && "NotImplemented");
    return 0;
}

cl_dev_err_code ExecutionService::GetTargetMachineDescription(
        void* pTargetDescription,
        size_t descriptionSize) const

{
    assert(false && "NotImplemented");
    return CL_DEV_NOT_SUPPORTED;
}

void ExecutionService::Release()
{
    delete this;
}

}}}

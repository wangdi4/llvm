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

File Name:  MICExecutionService.cpp

\*****************************************************************************/

#include "MICExecutionService.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "Binary.h"
#include "MICDeviceBackendFactory.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

MICExecutionService::MICExecutionService()
{
    m_pBackendFactory = MICDeviceBackendFactory::GetInstance(); 
}

size_t MICExecutionService::GetTargetMachineDescriptionSize() const
{
    return sizeof(char) * 3; // dummy to return "KNF"
}

cl_dev_err_code MICExecutionService::GetTargetMachineDescription(
    void* pTargetDescription, 
    size_t descriptionSize) const
{
    assert(pTargetDescription && "Target Description buffer is null");
    assert((descriptionSize >= GetTargetMachineDescriptionSize()) && "Too small buffer size");

    ((char*)pTargetDescription)[0] = 'K';
    ((char*)pTargetDescription)[1] = 'N';
    ((char*)pTargetDescription)[2] = 'F';
    return CL_DEV_SUCCESS;
}

}}}

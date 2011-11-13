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

File Name:  CPUExecutionService.cpp

\*****************************************************************************/

#include "CPUExecutionService.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "Binary.h"
#include "CPUDeviceBackendFactory.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

CPUExecutionService::CPUExecutionService()
{
    m_pBackendFactory = CPUDeviceBackendFactory::GetInstance(); 
}

}}}

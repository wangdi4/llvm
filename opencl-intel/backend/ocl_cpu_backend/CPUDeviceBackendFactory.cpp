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

File Name:  CPUDeviceBackendFactory.cpp

\*****************************************************************************/

#include "CPUDeviceBackendFactory.h"
#include "CPUProgram.h"
#include "Kernel.h"
#include "CPUKernel.h"
#include "KernelProperties.h"
#include "CPUJITContainer.h"
#include <assert.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

CPUDeviceBackendFactory* CPUDeviceBackendFactory::s_pInstance = NULL;

void CPUDeviceBackendFactory::Init()
{
    assert(!s_pInstance);
    s_pInstance = new CPUDeviceBackendFactory();
}

void CPUDeviceBackendFactory::Terminate()
{
    if( NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

CPUDeviceBackendFactory* CPUDeviceBackendFactory::GetInstance()
{
    assert(s_pInstance);
    return s_pInstance;
}

Program* CPUDeviceBackendFactory::CreateProgram()
{
    return new CPUProgram();
}

Kernel* CPUDeviceBackendFactory::CreateKernel()
{
    return new CPUKernel();
}

Kernel* CPUDeviceBackendFactory::CreateKernel(
    const std::string& name,
    const std::vector<cl_kernel_argument>& args,
    const std::vector<unsigned int>& memArgs,
    KernelProperties* pProps)
{
    return new CPUKernel(name, args, memArgs, pProps);
}

KernelProperties* CPUDeviceBackendFactory::CreateKernelProperties()
{
    return new KernelProperties();
}

KernelJITProperties* CPUDeviceBackendFactory::CreateKernelJITProperties()
{
    return new KernelJITProperties();
}

IKernelJITContainer* CPUDeviceBackendFactory::CreateKernelJITContainer()
{
    return new CPUJITContainer();
}

}}} // namespace

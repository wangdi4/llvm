// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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

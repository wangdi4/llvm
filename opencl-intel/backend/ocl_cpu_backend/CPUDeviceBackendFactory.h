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

#pragma once
#include "cl_dev_backend_api.h"
#include "IAbstractBackendFactory.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/**
 * This class is used to generate the suitable objects for CPU device
 */
class CPUDeviceBackendFactory : public IAbstractBackendFactory
{
protected:
    CPUDeviceBackendFactory() { }; 
    virtual ~CPUDeviceBackendFactory() { };

public:
    static void Init();
    static void Terminate();

    static CPUDeviceBackendFactory* GetInstance();

    virtual Program* CreateProgram();
    virtual Kernel* CreateKernel();

    virtual Kernel* CreateKernel(
        const std::string& name,
        const std::vector<cl_kernel_argument>& args,
        const std::vector<unsigned int>& memArgs,
        KernelProperties* pProps);

    virtual KernelProperties* CreateKernelProperties();
    virtual KernelJITProperties* CreateKernelJITProperties();
    virtual IKernelJITContainer* CreateKernelJITContainer();
 
protected:
    static CPUDeviceBackendFactory* s_pInstance;
};

}}} // namespace

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

#include <vector>
#include <string>
#include "cl_dev_backend_api.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class Program;
class Kernel;
class KernelProperties;
class KernelJITProperties;
class IKernelJITContainer;
class IBlockToKernelMapper;

/**
 * This interface is used to generate the suitable objects for CPU\MIC devices
 */
class IAbstractBackendFactory
{
public:
    virtual Program* CreateProgram() = 0;
    virtual Kernel* CreateKernel()   = 0;

    virtual Kernel* CreateKernel(
        const std::string& name,
        const std::vector<cl_kernel_argument>& args,
        const std::vector<unsigned int>& memArgs,
        KernelProperties* pProps) = 0;

    virtual KernelProperties* CreateKernelProperties() = 0;
    virtual KernelJITProperties* CreateKernelJITProperties() = 0;
    virtual IKernelJITContainer* CreateKernelJITContainer() = 0;
};

}}} // namespace

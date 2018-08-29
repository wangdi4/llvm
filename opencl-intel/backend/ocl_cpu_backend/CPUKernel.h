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

// NOTICE: THIS CLASS WILL BE SERIALIZED TO THE DEVICE, IF YOU MAKE ANY CHANGE 
//  OF THE CLASS FIELDS YOU SHOULD UPDATE THE SERILIZE METHODS  
#pragma once

#include "Kernel.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class CPUKernel : public Kernel
{
public:
    CPUKernel();

    CPUKernel(const std::string& name,
        const std::vector<cl_kernel_argument>& args,
        const std::vector<unsigned int>& memArgs,
        KernelProperties* pProps);

    virtual ~CPUKernel() { };

    /**
     * @effects prepares the thread for kernel execution
     * NOTICE: only the thread which called this api is ready to execute the thread
     * @param state object to save the old state in
     * @returns CL_DEV_SUCCESS in success; CL_DEV_ERROR_FAIL otherwise
     */
    virtual cl_dev_err_code PrepareThreadState(ICLDevExecutionState& state) const;

    /**
     * @effects restore the thread state after kernel execution
     * @param state object to restore from
     * @returns CL_DEV_SUCCESS in success; CL_DEV_ERROR_FAIL otherwise
     */
    virtual cl_dev_err_code RestoreThreadState(ICLDevExecutionState& state) const;

protected:
    bool m_hasAVX1;        // Are we running on a CPU supporting AVX?
    bool m_hasAVX2;        // Running on Haswell
};

}}} // namespace

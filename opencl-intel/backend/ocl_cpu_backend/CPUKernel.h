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

File Name:  CPUKernel.h

\*****************************************************************************/
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

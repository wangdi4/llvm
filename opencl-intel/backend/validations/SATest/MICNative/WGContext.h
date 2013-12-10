/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  WGContext.h

\*****************************************************************************/

#pragma once

#include <cl_device_api.h>
#include <cl_dev_backend_api.h>

#include <vector>

#include "mem_utils.h"

class WGContext
{
public:
    WGContext(const ICLDevBackendKernel_* pKernel,
                              cl_work_description_type* workInfo, 
                              void* pArgsBuffer, size_t argsBufferSize);
    virtual ~WGContext();

    void GetMemoryBuffersDescriptions(size_t* IN pBufferSizes, 
                                      size_t* INOUT pBufferCount );

    cl_dev_err_code PrepareThread();
    cl_dev_err_code RestoreThreadState();
    cl_dev_err_code Execute(const size_t *pGroupID);

    ICLDevBackendKernelRunner::ICLDevExecutionState m_tExecState;

private:
    void InitParams(const ICLDevBackendKernel_* pKernel, char* pArgsBuffer, cl_work_description_type workInfo);
    const ICLDevBackendKernelRunner * m_pKernelRunner;

    Validation::auto_ptr_aligned    m_pPrivateMem;
    size_t                          m_stPrivMemAllocSize;

    Validation::auto_ptr_aligned    m_pArgumentBuffer;

    std::vector<size_t>     m_kernelLocalMemSizes;

    size_t                  m_stAlignedKernelParamSize;
    size_t                  m_stWIidsBufferSize;
    size_t                  m_stPrivateMemorySize;
    size_t                  m_stKernelParamSize;

    unsigned int            m_uiVectorWidth; // vector size that was actually used
    unsigned int            m_uiWGSize;
};


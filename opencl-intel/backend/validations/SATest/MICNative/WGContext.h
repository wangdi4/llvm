/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

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
using namespace Intel::OpenCL::DeviceBackend;

class WGContext
{
public:
    WGContext();
    virtual ~WGContext();

    cl_dev_err_code CreateContext(ICLDevBackendBinary_* pExec, size_t* pBuffSizes, size_t count);
    inline ICLDevBackendExecutable_*    GetExecutable() const {return m_pContext;}

protected:
    ICLDevBackendExecutable_*   m_pContext;
    char*                       m_pLocalMem;
    void*                       m_pPrivateMem;
    size_t                      m_stPrivMemAllocSize;
};


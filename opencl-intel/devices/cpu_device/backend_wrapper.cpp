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

File Name:  OpenCLBackendWrapper.cpp

\*****************************************************************************/
#include <assert.h>
#include <memory>
#include "backend_wrapper.h"
#include "cl_sys_info.h"

#if defined(_WIN32)
#if defined(_M_X64)
const char* szOclCpuBackendDllName = "OclCpuBackEnd64.dll";
#else
const char* szOclCpuBackendDllName = "OclCpuBackEnd32.dll";
#endif
#else
const char* szOclCpuBackendDllName = "libOclCpuBackEnd.so";
#endif

namespace Intel{ namespace OpenCL { namespace CPUDevice {

OpenCLBackendWrapper::OpenCLBackendWrapper(void):
    // ALERT!!! DK!!! Backend sometimes corrups heap on Linux if it unloads in parallel with shutdown
    m_dll(false),
    m_funcInit(NULL),
    m_funcTerminate(NULL),
    m_funcGetFactory(NULL)
{
}

cl_dev_err_code OpenCLBackendWrapper::LoadDll()
{
    if( !m_dll.Load(Intel::OpenCL::Utils::GetFullModuleNameForLoad(szOclCpuBackendDllName)) )
    {
        return CL_DEV_ERROR_FAIL;
    }

    m_funcInit = (BACKEND_INIT_FUNCPTR)(intptr_t)m_dll.GetFunctionPtrByName("InitDeviceBackend");
    if( NULL == m_funcInit)
    {
        return CL_DEV_ERROR_FAIL;
    }

    m_funcTerminate = (BACKEND_TERMINATE_FUNCPTR)(intptr_t)m_dll.GetFunctionPtrByName("TerminateDeviceBackend");
    if( NULL == m_funcTerminate)
    {
        return CL_DEV_ERROR_FAIL;
    }

    m_funcGetFactory = (BACKEND_GETFACTORY_FUNCPTR)(intptr_t)m_dll.GetFunctionPtrByName("GetDeviceBackendFactory");
    if( NULL == m_funcGetFactory )
    {
        return CL_DEV_ERROR_FAIL;
    }

    return CL_DEV_SUCCESS;
}

void OpenCLBackendWrapper::UnloadDll()
{
    m_funcInit = NULL;
    m_funcTerminate = NULL;
    m_funcGetFactory = NULL;
    // ALERT!!! DK!!! Backend sometimes corrups heap on Linux if it unloads in parallel with shutdown
    //m_dll.Close();
}

cl_dev_err_code OpenCLBackendWrapper::Init(const ICLDevBackendOptions* pBackendOptions)
{
    assert( !m_funcInit && "You must not call Init more then once, without calling Terminate first");

    cl_dev_err_code ret = LoadDll();
    if( CL_DEV_FAILED(ret) )
    {
        return ret;
    }

    return m_funcInit(pBackendOptions);
}

void OpenCLBackendWrapper::Terminate()
{
    assert(m_funcTerminate && "The Init method failed and you didn't noticed, did you?");
    m_funcTerminate();
    UnloadDll();
}

ICLDevBackendServiceFactory* OpenCLBackendWrapper::GetBackendFactory()
{
    assert(m_funcGetFactory && "The Init method failed and you didn't noticed, did you?");
    return m_funcGetFactory();
}
  
}}}
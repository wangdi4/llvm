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

File Name:  OpenCLBackendWrapper.cpp

\*****************************************************************************/
#include <assert.h>
#include <memory>
#include "backend_wrapper.h"

namespace Intel{ namespace OpenCL { namespace MICDevice
{
OpenCLBackendWrapper::OpenCLBackendWrapper(const char* backend_name):
    m_dll_name( backend_name ),
    // ALERT!!! DK!!! Backend sometimes corrups heap on Linux if it unloads in parallel with shutdown
    m_dll(false),
    m_funcInit(nullptr),
    m_funcTerminate(nullptr),
    m_funcGetFactory(nullptr)
{
}

cl_dev_err_code OpenCLBackendWrapper::LoadDll()
{
    if( !m_dll.Load(m_dll_name.c_str()) )
    {
        return CL_DEV_ERROR_FAIL;
    }

    m_funcInit = (BACKEND_INIT_FUNCPTR)(intptr_t)m_dll.GetFunctionPtrByName("InitDeviceBackend");
    if( nullptr == m_funcInit)
    {
        return CL_DEV_ERROR_FAIL;
    }

    m_funcTerminate = (BACKEND_TERMINATE_FUNCPTR)(intptr_t)m_dll.GetFunctionPtrByName("TerminateDeviceBackend");
    if( nullptr == m_funcTerminate)
    {
        return CL_DEV_ERROR_FAIL;
    }

    m_funcGetFactory = (BACKEND_GETFACTORY_FUNCPTR)(intptr_t)m_dll.GetFunctionPtrByName("GetDeviceBackendFactory");
    if( nullptr == m_funcGetFactory )
    {
        return CL_DEV_ERROR_FAIL;
    }

    return CL_DEV_SUCCESS;
}

void OpenCLBackendWrapper::UnloadDll()
{
    m_funcInit = nullptr;
    m_funcTerminate = nullptr;
    m_funcGetFactory = nullptr;
    // ALERT!!! DK!!! Backend sometimes corrups heap on Linux if it unloads in parallel with shutdown
    //m_dll.Close();
}

cl_dev_err_code OpenCLBackendWrapper::Init(void)
{
    assert( !m_funcInit && "You must not call Init more then once, without calling Terminate first");

    cl_dev_err_code ret = LoadDll();
    if( CL_DEV_FAILED(ret) )
    {
        return ret;
    }

    return m_funcInit(nullptr);
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

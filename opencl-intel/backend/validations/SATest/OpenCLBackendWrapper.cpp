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
#include "OpenCLBackendWrapper.h"
#include "SATestException.h"

#if defined(_WIN32)
const char* szOclCpuBackendDllName = "OclCpuBackEnd.dll";
#else
const char* szOclCpuBackendDllName = "libOclCpuBackEnd.so";
#endif

namespace Validation
{
OpenCLBackendWrapper* OpenCLBackendWrapper::s_instance = NULL;

void OpenCLBackendWrapper::Init()
{
    assert(!s_instance);
    s_instance = new OpenCLBackendWrapper();
    s_instance->LoadDll();
    s_instance->Init(NULL); // we could probably pass the logger routine here, but currently it's not supported
}

OpenCLBackendWrapper& OpenCLBackendWrapper::GetInstance()
{
    assert(s_instance);
    return *s_instance;
}

OpenCLBackendWrapper::OpenCLBackendWrapper(void):
    m_funcInit(NULL),
    m_funcTerminate(NULL),
    m_funcGetFactory(NULL)
{
}

OpenCLBackendWrapper::~OpenCLBackendWrapper(void)
{}

void OpenCLBackendWrapper::LoadDll()
{
    if( !m_dll.Load(szOclCpuBackendDllName) )
    {
        throw Exception::GeneralException("Can't load the backend library");
    }

    m_funcInit = (BACKEND_INIT_FUNCPTR)(intptr_t)m_dll.GetFuncPtr("InitDeviceBackend");

    if( NULL == m_funcInit)
    {
        throw Exception::GeneralException("'Init' export function is not found in OCL Backend library");
    }

    m_funcTerminate = (BACKEND_TERMINATE_FUNCPTR)(intptr_t)m_dll.GetFuncPtr("TerminateDeviceBackend");

    if( NULL == m_funcTerminate)
    {
        throw Exception::GeneralException("'Terminate' export function is not found in OCL Backend library");
    }

    m_funcGetFactory = (BACKEND_GETFACTORY_FUNCPTR)(intptr_t)m_dll.GetFuncPtr("GetDeviceBackendFactory");

    if( NULL == m_funcGetFactory )
    {
        throw Exception::GeneralException("'GetDeviceBackendFactory' export function is not found in OCL Backend library");
    }
}

cl_dev_err_code OpenCLBackendWrapper::Init(const ICLDevBackendOptions* pBackendOptions)
{
    assert(m_funcInit);
    return m_funcInit(pBackendOptions);
}

void OpenCLBackendWrapper::Terminate()
{
    assert(s_instance);
    s_instance->Release();
    delete s_instance;
    s_instance = NULL;
}

void OpenCLBackendWrapper::Release()
{
    assert(m_funcTerminate);
    m_funcTerminate();
}

ICLDevBackendServiceFactory* OpenCLBackendWrapper::GetBackendServiceFactory()
{
    assert(m_funcGetFactory);
    return m_funcGetFactory();
}
  
}
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

#include "debuggingservicewrapper.h"
#include "DebuggerPipeWrapper.h"
#include "cl_utils.h"
#include "llvm/Support/DynamicLibrary.h"
#include <string>
#include <cassert>
#include <cstdlib>

using namespace std;


#if defined(_WIN32)
// Compatibility of OclCpuDebugging library with older revisions of OCL CPU RT libraries
// could be broken at some point, e.g. due to LLVM upgrade. To overcome this issue it was
// decided to versionize the debugging library and once its version is changed notify
// SDK about the OCL RT version it is compatible with.
// Actually it is unknown why the debugging library isn't delivered with the GEN driver.
// This would solve the problem described above. For the reference see task CORC-1070.
const char* DEBUGGER_DLL_NAME = "OclCpuDebugging5.dll";
#else
const char* DEBUGGER_DLL_NAME = "libOclCpuDebugging.so";
#endif


namespace Intel { namespace OpenCL { namespace DeviceBackend {


// Define the singleton instance
//
DebuggingServiceWrapper DebuggingServiceWrapper::instance;


DebuggingServiceWrapper::DebuggingServiceWrapper()
    : m_dll_loaded(false), 
    m_init_func(nullptr), m_terminate_func(nullptr), m_instance_func(nullptr)
{
}


cl_dev_err_code DebuggingServiceWrapper::Init()
{
    unsigned int port_number = 0;
    bool debugging_enabled = false;
    assert(!m_dll_loaded && "DebuggingServiceWrapper::Init called more than once");

#ifdef _WIN32
    DebuggerPipeWrapper pipeWrapper;
    bool res = pipeWrapper.init("\\\\.\\pipe\\INTEL_OCL_DBG_PIPE" + stringify(GetCurrentProcessId()));
    if (res && pipeWrapper.isDebuggingEnabled()) {
        debugging_enabled = true;
        port_number = pipeWrapper.getDebuggingPort();
    }
#else
    const char* env_val = getenv("CL_CONFIG_DBG_ENABLE");
    debugging_enabled = (env_val && string(env_val) == "1");
#endif
    if (debugging_enabled) {
        cl_dev_err_code rc = LoadDll();
        if (CL_DEV_FAILED(rc))
            return rc;

        if (m_init_func(port_number) == false)
            return CL_DEV_ERROR_FAIL;
        else
            return CL_DEV_SUCCESS;
    }
    else
        return CL_DEV_SUCCESS;
}


void DebuggingServiceWrapper::Terminate()
{
    if (m_dll_loaded) {
        m_terminate_func();
        
        m_init_func = nullptr;
        m_terminate_func = nullptr;
        m_instance_func = nullptr;
        m_dll_loaded = false;
    }
}


ICLDebuggingService* DebuggingServiceWrapper::GetDebuggingService()
{
    if (m_dll_loaded)
        return m_instance_func();
    else
        return nullptr;
}


cl_dev_err_code DebuggingServiceWrapper::LoadDll()
{
    string err_str;
    bool error = llvm::sys::DynamicLibrary::LoadLibraryPermanently(
        DEBUGGER_DLL_NAME, &err_str);
    if (error) {
        return CL_DEV_ERROR_FAIL;
    }

    m_init_func = (DEBUGGING_SERVICE_INIT_FUNC) 
        (intptr_t) llvm::sys::DynamicLibrary::SearchForAddressOfSymbol("InitDebuggingService");
    m_terminate_func = (DEBUGGING_SERVICE_TERMINATE_FUNC)
        (intptr_t) llvm::sys::DynamicLibrary::SearchForAddressOfSymbol("TerminateDebuggingService");
    m_instance_func = (DEBUGGING_SERVICE_INSTANCE_FUNC)
        (intptr_t) llvm::sys::DynamicLibrary::SearchForAddressOfSymbol("DebuggingServiceInstance");

    if (!m_init_func || !m_terminate_func || !m_instance_func)
        return CL_DEV_ERROR_FAIL;

    m_dll_loaded = true;
    return CL_DEV_SUCCESS;
}


void DebuggingServiceWrapper::UnloadDll()
{

}


}}}


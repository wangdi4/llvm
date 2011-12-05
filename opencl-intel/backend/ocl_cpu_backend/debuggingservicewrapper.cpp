#include "debuggingservicewrapper.h"
#include "llvm/System/DynamicLibrary.h"
#include <string>
#include <cassert>
#include <cstdlib>

using namespace std;


#if defined(_WIN32)
const char* DEBUGGER_DLL_NAME = "OclCpuDebugging.dll";
#else
const char* DEBUGGER_DLL_NAME = "libOclCpuDebugging.so";
#endif


namespace Intel { namespace OpenCL { namespace DeviceBackend {


// Define the singleton instance
//
DebuggingServiceWrapper DebuggingServiceWrapper::instance;


DebuggingServiceWrapper::DebuggingServiceWrapper()
    : m_dll_loaded(false), 
    m_init_func(NULL), m_terminate_func(NULL), m_instance_func(NULL)    
{
}


cl_dev_err_code DebuggingServiceWrapper::Init()
{
    assert(!m_dll_loaded && "DebuggingServiceWrapper::Init called more than once");

    const char* env_val = getenv("CL_CONFIG_DBG_ENABLE");
    if (env_val && string(env_val) == "1") {
        cl_dev_err_code rc = LoadDll();
        if (CL_DEV_FAILED(rc))
            return rc;

        if (m_init_func() == false)
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
        
        m_init_func = NULL;
        m_terminate_func = NULL;
        m_instance_func = NULL;
        m_dll_loaded = false;
    }
}


ICLDebuggingService* DebuggingServiceWrapper::GetDebuggingService()
{
    if (m_dll_loaded)
        return m_instance_func();
    else
        return NULL;
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


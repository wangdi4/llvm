// INTEL CONFIDENTIAL
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.

#include "cl_dynamic_lib.h"
#include "cl_shutdown.h"
#include "cl_sys_info.h"

#include <sys/stat.h>
#include <dlfcn.h>
#include <assert.h>

using namespace Intel::OpenCL::Utils;

IAtExitCentralPoint* OclDynamicLib::m_atexit_fn = nullptr;

// Get function pointer from library handle
ptrdiff_t OclDynamicLib::GetFuntionPtrByNameFromHandle(void* hLibrary,
                                                       const char* szFuncName)
{
    //clear errors
    dlerror();
    void* func = dlsym(hLibrary, szFuncName);
    const char * error;
    if ((error = dlerror()) != nullptr)  {
        return 0;
    }

    return (ptrdiff_t)func;
}

OclDynamicLib::OclDynamicLib(bool bUnloadOnDestructor) :
    m_hLibrary(nullptr), m_bUnloadOnDestructor(bUnloadOnDestructor)
{
}

OclDynamicLib::~OclDynamicLib()
{
    if (m_bUnloadOnDestructor)
    {
        Close();
    }
}

// -----------------------------------------------------------------------------
// Checks for existance of a file with specified name
bool OclDynamicLib::IsExists(const char* pLibName)
{
    struct stat stFileInfo;
    int rc;

    rc = stat(pLibName, &stFileInfo);

    return (0 == rc);
}

// -----------------------------------------------------------------------------
// Loads a dynamically link library into process address space
bool OclDynamicLib::Load(const char* pLibName)
{
    if ( nullptr != m_hLibrary )
    {
        return false;
    }

    // Load library
    std::string strLibName(MAX_PATH, '\0');
    // Get a full path of a library from where the function was called. We
    // expect, that a callee library is having the same path as the caller.
    // Add this path to string to pass in dlopen function.
    Intel::OpenCL::Utils::GetModuleDirectory(&strLibName[0], MAX_PATH);
    strLibName.resize(strLibName.find_first_of('\0'));
    // To make library name canonical add a version string at its ending.
    strLibName += std::string(pLibName) + std::string(".") +
                  std::string(VERSIONSTRING);
    m_hLibrary = dlopen(strLibName.c_str(), RTLD_LAZY);

    if ( nullptr == m_hLibrary )
    {
        // We didn't find the called library by a full path. Step back and try
        // to find it disregarding the calculated path.
        strLibName = std::string(pLibName) + std::string(".") +
                                 std::string(VERSIONSTRING);
        m_hLibrary = dlopen(strLibName.c_str(), RTLD_LAZY);
        if ( nullptr == m_hLibrary )
        {

#ifdef _DEBUG
            const char* e = dlerror();
            printf("Error loading %s: %s\n", pLibName, e);
#endif
            return false;
        }
    }

    RegisterAtExitNotification_Func AtExitFunc = 
        (RegisterAtExitNotification_Func)GetFunctionPtrByName(
            OclDynamicLib_AT_EXIT_REGISTER_FUNC_NAME);

    if (nullptr != AtExitFunc)
    {
        AtExitFunc( m_atexit_fn );
    }

    return true;
}

// Loads a dynamically link library into process address space
void OclDynamicLib::Close()
{
    if ( nullptr == m_hLibrary )
    {
        return;
    }

    if (IsShuttingDown())
    {
        return;
    }

    m_uiFuncCount = 0;
    m_pOffsetNames = nullptr;
    m_pOffsetFunc = nullptr;

    UseShutdownHandler::UnloadingDll(true);
    dlclose(m_hLibrary);
    UseShutdownHandler::UnloadingDll(false);

    m_hLibrary = nullptr;
}

// Returns a number of named functions found in the library
unsigned int OclDynamicLib::GetNumberOfFunctions() const
{
    assert(0 && "Not implemented on Linux");
    return 0;
}

// Returns a pointer to function name
const char* OclDynamicLib::GetFunctionName(unsigned int uiFuncId) const
{
    assert(0 && "Not implemented on Linux");
    return nullptr;
}

// Returns a function pointer
const void* OclDynamicLib::GetFunctionPtr(unsigned int uiFuncId) const
{
    assert(0 && "Not implemented on Linux");
    return nullptr;
}

// Returns a function pointer
ptrdiff_t OclDynamicLib::GetFunctionPtrByName(const char* szFuncName) const
{
    if ( nullptr == m_hLibrary )
    {
        return 0;
    }

    void* func;
    const char* error;

    //clear errors
    dlerror();
    func = dlsym(m_hLibrary, szFuncName);
        if ((error = dlerror()) != nullptr)  {
            return 0;
      }

    return (ptrdiff_t)func;
}

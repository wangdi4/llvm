// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
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

#include "cl_dynamic_lib.h"
#include "cl_shutdown.h"

#include <sys/stat.h>
#include <dlfcn.h>
#include <assert.h>

using namespace Intel::OpenCL::Utils;

IAtExitCentralPoint* OclDynamicLib::m_atexit_fn = nullptr;

// Get function pointer from library handle
ptrdiff_t OclDynamicLib::GetFuntionPtrByNameFromHandle(void* hLibrary, const char* szFuncName)
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

// ------------------------------------------------------------------------------
// Checks for existance of a file with specified name
bool OclDynamicLib::IsExists(const char* pLibName)
{
    struct stat stFileInfo;
    int rc;

    rc = stat(pLibName, &stFileInfo);

    return (0 == rc);
}

// ------------------------------------------------------------------------------
// Loads a dynamically link library into process address space
bool OclDynamicLib::Load(const char* pLibName)
{
    if ( nullptr != m_hLibrary )
    {
        return false;
    }

    // Load library
    m_hLibrary = dlopen(pLibName, RTLD_LAZY);

    if ( nullptr == m_hLibrary )
    {
#ifdef _DEBUG
        const char* e = dlerror();
        printf("Error loading %s: %s\n", pLibName, e);
#endif
        return false;
    }

    RegisterAtExitNotification_Func AtExitFunc = 
        (RegisterAtExitNotification_Func)GetFunctionPtrByName(OclDynamicLib_AT_EXIT_REGISTER_FUNC_NAME);

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

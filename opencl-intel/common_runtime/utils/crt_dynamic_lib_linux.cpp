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

#include "crt_dynamic_lib.h"

#include <sys/stat.h>
#include <dlfcn.h>
#include <assert.h>
#include <cstdio>

using namespace OCLCRT::Utils;

OclDynamicLib::OclDynamicLib() :
    m_hLibrary(NULL)
{
}

OclDynamicLib::~OclDynamicLib()
{
    Close();
}

// ------------------------------------------------------------------------------
// Checks for existance of a file with specified name
crt_err_code OclDynamicLib::IsExists(const char* pLibName)
{
    struct stat stFileInfo;
    int rc;

    rc = stat(pLibName, &stFileInfo);

    return (0 == rc);
}

// ------------------------------------------------------------------------------
// Loads a dynamically link library into process address space
crt_err_code OclDynamicLib::Load(const char* pLibName)
{
    if ( NULL != m_hLibrary )
    {
        return CRT_FAIL;
    }

    // Load library
    m_hLibrary = dlopen(pLibName, RTLD_LAZY);

    if ( NULL == m_hLibrary )
    {
#ifdef _DEBUG
        const char* e = dlerror();
        printf("Warning. Unable to load %s: %s\n", pLibName, e);
#endif
        return CRT_FAIL;
    }

    return CRT_SUCCESS;
}

// Loads a dynamically link library into process address space
void OclDynamicLib::Close()
{
    if ( NULL == m_hLibrary )
    {
        return;
    }

    m_uiFuncCount = 0;
    m_pOffsetNames = NULL;
    m_pOffsetFunc = NULL;

    dlclose(m_hLibrary);
    m_hLibrary = NULL;
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
        return NULL;
}

// Returns a function pointer
const void* OclDynamicLib::GetFunctionPtr(unsigned int uiFuncId) const
{
    assert(0 && "Not implemented on Linux");
        return NULL;
}

// Returns a function pointer
OclDynamicLib::func_t    OclDynamicLib::GetFunctionPtrByName(const char* szFuncName) const
{
    if ( NULL == m_hLibrary )
    {
        return NULL;
    }

    void* func;
    const char* error;

    //clear errors
    dlerror();
    func = dlsym(m_hLibrary, szFuncName);
        if ((error = dlerror()) != NULL) {
         return NULL;
    }

    return (func_t)(ptrdiff_t)func;
}

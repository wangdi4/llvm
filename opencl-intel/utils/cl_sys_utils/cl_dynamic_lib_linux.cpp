/////////////////////////////////////////////////////////////////////////
// cl_dynamic_lib.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "cl_dynamic_lib.h"
#include "cl_shutdown.h"

#include <sys/stat.h>
#include <dlfcn.h>
#include <assert.h>

using namespace Intel::OpenCL::Utils;

IAtExitCentralPoint* OclDynamicLib::m_atexit_fn = NULL;

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
    m_hLibrary(NULL), m_bUnloadOnDestructor(bUnloadOnDestructor)
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
    if ( NULL != m_hLibrary )
    {
        return false;
    }

    // Load library
    m_hLibrary = dlopen(pLibName, RTLD_LAZY);

    if ( NULL == m_hLibrary )
    {
#ifdef _DEBUG
        const char* e = dlerror();
        printf("Error loading %s: %s\n", pLibName, e);
#endif
        return false;
    }

    RegisterAtExitNotification_Func AtExitFunc = 
        (RegisterAtExitNotification_Func)GetFunctionPtrByName(OclDynamicLib_AT_EXIT_REGISTER_FUNC_NAME);

    if (NULL != AtExitFunc)
    {
        AtExitFunc( m_atexit_fn );
    }

    return true;
}

// Loads a dynamically link library into process address space
void OclDynamicLib::Close()
{
    if ( NULL == m_hLibrary )
    {
        return;
    }

    if (IsShuttingDown())
    {
        return;
    }

    m_uiFuncCount = 0;
    m_pOffsetNames = NULL;
    m_pOffsetFunc = NULL;

    UseShutdownHandler::UnloadingDll(true);
    dlclose(m_hLibrary);
    UseShutdownHandler::UnloadingDll(false);

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

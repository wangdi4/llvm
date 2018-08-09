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
#include "cl_sys_info.h"

#include <windows.h>

using namespace Intel::OpenCL::Utils;

IAtExitCentralPoint* OclDynamicLib::m_atexit_fn = nullptr;

// Get function pointer from library handle
ptrdiff_t OclDynamicLib::GetFuntionPtrByNameFromHandle(void* hLibrary, const char* szFuncName)
{
    return (ptrdiff_t)GetProcAddress((HMODULE)hLibrary, szFuncName);
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
    DWORD rc;

    rc = GetFileAttributesA(pLibName);

    return (INVALID_FILE_ATTRIBUTES != rc);
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
	m_hLibrary = LoadLibraryEx( pLibName, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH );

    if ( nullptr == m_hLibrary )
    {
        return false;
    }

    // Library was succefully loaded
    BYTE *hMod = (BYTE*)m_hLibrary;
    IMAGE_NT_HEADERS *pnt = (IMAGE_NT_HEADERS*)&hMod[PIMAGE_DOS_HEADER(hMod)->e_lfanew];
    IMAGE_EXPORT_DIRECTORY *exp = (IMAGE_EXPORT_DIRECTORY*)&hMod[pnt->OptionalHeader.DataDirectory->VirtualAddress];

    m_uiFuncCount = exp->NumberOfNames;
    m_pOffsetNames = (unsigned int*)&hMod[exp->AddressOfNames];
    m_pOffsetFunc = (unsigned int*)&hMod[exp->AddressOfFunctions];

    
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

    m_uiFuncCount = 0;
    m_pOffsetNames = nullptr;
    m_pOffsetFunc = nullptr;

    UseShutdownHandler::UnloadingDll(true);
    FreeLibrary((HMODULE)m_hLibrary);
    UseShutdownHandler::UnloadingDll(false);
    m_hLibrary = nullptr;
}

// Returns a number of named functions found in the library
unsigned int OclDynamicLib::GetNumberOfFunctions() const
{
    if ( nullptr == m_hLibrary )
    {
        return 0;
    }
    return m_uiFuncCount;
}

// Returns a pointer to function name
const char* OclDynamicLib::GetFunctionName(unsigned int uiFuncId) const
{
    if ( (nullptr == m_hLibrary) || (uiFuncId >= m_uiFuncCount) )
    {
        return nullptr;
    }

    BYTE* hMod = (BYTE*)m_hLibrary;
    return (const char*)&hMod[m_pOffsetNames[uiFuncId]];
}

// Returns a function pointer
const void* OclDynamicLib::GetFunctionPtr(unsigned int uiFuncId) const
{
    if ( (nullptr == m_hLibrary) || (uiFuncId >= m_uiFuncCount) )
    {
        return nullptr;
    }

    BYTE* hMod = (BYTE*)m_hLibrary;
    return (const void*)&hMod[m_pOffsetFunc[uiFuncId]];
}

// Returns a function pointer
ptrdiff_t OclDynamicLib::GetFunctionPtrByName(const char* szFuncName) const
{
    if ( nullptr == m_hLibrary )
    {
        return NULL;
    }

    return (ptrdiff_t)GetProcAddress((HMODULE)m_hLibrary, szFuncName);
}

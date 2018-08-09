// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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

#include <windows.h>
#include "DriverStore.h"

using namespace OCLCRT::Utils;

OclDynamicLib::OclDynamicLib() :
    m_hLibrary( NULL )
{
}

OclDynamicLib::~OclDynamicLib()
{
    Close();
}

crt_err_code OclDynamicLib::IsExists( const char* pLibName )
{
    DWORD rc;

    rc = GetFileAttributesA(pLibName);

    return( ( INVALID_FILE_ATTRIBUTES != rc ) ? CRT_SUCCESS : CRT_FAIL );
}

crt_err_code OclDynamicLib::Load( const char* pLibName )
{
    if( NULL != m_hLibrary )
    {
        return CRT_FAIL;
    }

    // Load library
    if((strstr(pLibName, "\\") != nullptr) || (strstr(pLibName, "/") != nullptr))
    {
        // pLibName is a full path to the lib (assuming it's not available in standard search path)
        m_hLibrary = LoadLibraryEx( pLibName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
    }
    else
    {
        // pLibName is just the lib name
        m_hLibrary = LoadLibrary( pLibName );
    }

    if ( NULL == m_hLibrary )
    {
        return CRT_FAIL;
    }

    // Library was succefully loaded
    BYTE *hMod = (BYTE*)m_hLibrary;
    IMAGE_NT_HEADERS *pnt = ( IMAGE_NT_HEADERS* )&hMod[ PIMAGE_DOS_HEADER( hMod )->e_lfanew ];
    IMAGE_EXPORT_DIRECTORY *exp = ( IMAGE_EXPORT_DIRECTORY* )&hMod[ pnt->OptionalHeader.DataDirectory->VirtualAddress ];

    m_uiFuncCount = exp->NumberOfNames;
    m_pOffsetNames = (unsigned int*)&hMod[ exp->AddressOfNames ];
    m_pOffsetFunc = (unsigned int*)&hMod[ exp->AddressOfFunctions ];

    return CRT_SUCCESS;
}

crt_err_code OclDynamicLib::LoadDependency( const char* pLibName )
{
    if ( NULL != m_hLibrary )
    {
        return CRT_FAIL;
    }

    m_hLibrary = ::LoadDependency( pLibName );

    if( NULL == m_hLibrary )
    {
        return CRT_FAIL;
    }

    // Library was succefully loaded
    BYTE *hMod = (BYTE*)m_hLibrary;
    IMAGE_NT_HEADERS *pnt = (IMAGE_NT_HEADERS*)&hMod[PIMAGE_DOS_HEADER(hMod)->e_lfanew];
    IMAGE_EXPORT_DIRECTORY *exp = (IMAGE_EXPORT_DIRECTORY*)&hMod[pnt->OptionalHeader.DataDirectory->VirtualAddress];

    m_uiFuncCount = exp->NumberOfNames;
    m_pOffsetNames = (unsigned int*)&hMod[exp->AddressOfNames];
    m_pOffsetFunc = (unsigned int*)&hMod[exp->AddressOfFunctions];

    return CRT_SUCCESS;
}

// Loads a dynamically link library into process address space
void OclDynamicLib::Close()
{
    if( NULL == m_hLibrary )
    {
        return;
    }

    m_uiFuncCount = 0;
    m_pOffsetNames = NULL;
    m_pOffsetFunc = NULL;

    FreeLibrary( (HMODULE)m_hLibrary );
}

// Returns a number of named functions found in the library
unsigned int OclDynamicLib::GetNumberOfFunctions() const
{
    if( NULL == m_hLibrary )
    {
        return 0;
    }
    return m_uiFuncCount;
}

// Returns a pointer to function name
const char* OclDynamicLib::GetFunctionName( unsigned int uiFuncId ) const
{
    if( ( NULL == m_hLibrary ) || ( uiFuncId >= m_uiFuncCount ) )
    {
        return NULL;
    }

    BYTE* hMod = (BYTE*)m_hLibrary;
    return (const char*)&hMod[ m_pOffsetNames[ uiFuncId ] ];
}

// Returns a function pointer
const void* OclDynamicLib::GetFunctionPtr( unsigned int uiFuncId ) const
{
    if( ( NULL == m_hLibrary ) || ( uiFuncId >= m_uiFuncCount ) )
    {
        return NULL;
    }

    BYTE* hMod = (BYTE*)m_hLibrary;
    return (const void*)&hMod[ m_pOffsetFunc[ uiFuncId ] ];
}

// Returns a function pointer
OclDynamicLib::func_t    OclDynamicLib::GetFunctionPtrByName( const char* szFuncName ) const
{
    if( NULL == m_hLibrary )
    {
        return NULL;
    }

    return (func_t)GetProcAddress( (HMODULE)m_hLibrary, szFuncName );
}

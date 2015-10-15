// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////
#include "crt_dynamic_lib.h"

#include <windows.h>

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
    m_hLibrary = LoadLibraryEx( pLibName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );

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
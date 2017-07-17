// Copyright (c) 2006-2016 Intel Corporation
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
#include "DriverStore.h"

using namespace OCLCRT::Utils;

OclDynamicLib::OclDynamicLib() :
    m_hLibrary( nullptr )
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
    if( nullptr != m_hLibrary )
    {
        return CRT_FAIL;
    }

    // Load library
    if((strstr(pLibName, "\\") != nullptr) || (strstr(pLibName, "/") != nullptr))
    {
        // pLibName is a full path to the lib (assuming it's not available in standard search path)
        m_hLibrary = LoadLibraryEx( pLibName, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH );
    }
    else
    {
        // pLibName is just the lib name
        m_hLibrary = LoadLibrary( pLibName );
    }

    if ( nullptr == m_hLibrary )
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
    if ( nullptr != m_hLibrary )
    {
        return CRT_FAIL;
    }

    m_hLibrary = ::LoadDependency( pLibName );

    if( nullptr == m_hLibrary )
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
    if( nullptr == m_hLibrary )
    {
        return;
    }

    m_uiFuncCount = 0;
    m_pOffsetNames = nullptr;
    m_pOffsetFunc = nullptr;

    FreeLibrary( (HMODULE)m_hLibrary );
}

// Returns a number of named functions found in the library
unsigned int OclDynamicLib::GetNumberOfFunctions() const
{
    if( nullptr == m_hLibrary )
    {
        return 0;
    }
    return m_uiFuncCount;
}

// Returns a pointer to function name
const char* OclDynamicLib::GetFunctionName( unsigned int uiFuncId ) const
{
    if( ( nullptr == m_hLibrary ) || ( uiFuncId >= m_uiFuncCount ) )
    {
        return nullptr;
    }

    BYTE* hMod = (BYTE*)m_hLibrary;
    return (const char*)&hMod[ m_pOffsetNames[ uiFuncId ] ];
}

// Returns a function pointer
const void* OclDynamicLib::GetFunctionPtr( unsigned int uiFuncId ) const
{
    if( ( nullptr == m_hLibrary ) || ( uiFuncId >= m_uiFuncCount ) )
    {
        return nullptr;
    }

    BYTE* hMod = (BYTE*)m_hLibrary;
    return (const void*)&hMod[ m_pOffsetFunc[ uiFuncId ] ];
}

// Returns a function pointer
OclDynamicLib::func_t    OclDynamicLib::GetFunctionPtrByName( const char* szFuncName ) const
{
    if( nullptr == m_hLibrary )
    {
        return nullptr;
    }

    return (func_t)GetProcAddress( (HMODULE)m_hLibrary, szFuncName );
}

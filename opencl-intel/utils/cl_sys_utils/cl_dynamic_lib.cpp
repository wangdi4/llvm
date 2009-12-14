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

#include <windows.h>

using namespace Intel::OpenCL::Utils;

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
	if ( NULL != m_hLibrary )
	{
		return false;
	}

	// Load library
	m_hLibrary = LoadLibraryA(pLibName);

	if ( NULL == m_hLibrary )
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

	return true;
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

	FreeLibrary((HMODULE)m_hLibrary);
}

// Returns a number of named functions found in the library
unsigned int OclDynamicLib::GetNumberOfFunctions() const
{
	if ( NULL == m_hLibrary )
	{
		return 0;
	}
	return m_uiFuncCount;
}

// Returns a pointer to function name
const char* OclDynamicLib::GetFunctionName(unsigned int uiFuncId) const
{
	if ( (NULL == m_hLibrary) || (uiFuncId >= m_uiFuncCount) )
	{
		return NULL;
	}

	BYTE* hMod = (BYTE*)m_hLibrary;
	return (const char*)&hMod[m_pOffsetNames[uiFuncId]];
}

// Returns a function pointer
const void* OclDynamicLib::GetFunctionPtr(unsigned int uiFuncId) const
{
	if ( (NULL == m_hLibrary) || (uiFuncId >= m_uiFuncCount) )
	{
		return NULL;
	}

	BYTE* hMod = (BYTE*)m_hLibrary;
	return (const void*)&hMod[m_pOffsetFunc[uiFuncId]];
}

// Returns a function pointer
const void*	 OclDynamicLib::GetFunctionPtrByName(const char* szFuncName) const
{
	if ( NULL == m_hLibrary )
	{
		return NULL;
	}

	return GetProcAddress((HMODULE)m_hLibrary, szFuncName);
}
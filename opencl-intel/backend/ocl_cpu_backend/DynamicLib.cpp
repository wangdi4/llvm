/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  DynamicLib.cpp

\*****************************************************************************/

#include "DynamicLib.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <stdio.h>
#include <dlfcn.h>
#endif

using namespace Intel::OpenCL::DeviceBackend::Utils;

DynamicLib::DynamicLib(void) :
m_hLibrary(NULL)
{
}

DynamicLib::~DynamicLib(void) 
{
	Close();
}

bool DynamicLib::Load(const char* pLibName)
{
	if ( NULL != m_hLibrary )
	{
		return false;
	}

	// Load library
#if defined(_WIN32)
	m_hLibrary = LoadLibraryA(pLibName);
#else
    m_hLibrary = dlopen( pLibName, RTLD_NOW);
#endif

	if ( NULL == m_hLibrary )
	{
		return false;
	}

	// Library was succefully loaded
#if 0
	BYTE *hMod = (BYTE*)m_hLibrary;
	IMAGE_NT_HEADERS *pnt = (IMAGE_NT_HEADERS*)&hMod[PIMAGE_DOS_HEADER(hMod)->e_lfanew];
	IMAGE_EXPORT_DIRECTORY *exp = (IMAGE_EXPORT_DIRECTORY*)&hMod[pnt->OptionalHeader.DataDirectory->VirtualAddress];
#endif
	return true;
}

void* DynamicLib::GetFuncPtr(const char* funcName)
{
	if ( NULL == m_hLibrary )
	{
		return NULL;
	}
#if defined(_WIN32)
	return GetProcAddress((HMODULE)m_hLibrary, funcName);
#else
    return dlsym(m_hLibrary, funcName);
#endif

}

void DynamicLib::Close()
{
	if ( NULL == m_hLibrary )
	{
		return;
	}
#if defined(_WIN32)
	FreeLibrary((HMODULE)m_hLibrary);
#else
    dlclose( m_hLibrary);
#endif
}


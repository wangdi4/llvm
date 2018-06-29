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

#include "task_executor.h"

#if ! defined( __THREAD_EXECUTOR__) && ! defined( __TBB_EXECUTOR__ )
#define __THREAD_EXECUTOR__
#endif

#ifdef __TBB_EXECUTOR__
#include "tbb_executor.h"
#define PTR_CAST	TBBTaskExecutor
#endif
#ifdef __THREAD_EXECUTOR__
#include "thread_executor.h"
#define PTR_CAST	ThreadTaskExecutor
#endif

#pragma comment (lib, "cl_logger" OPENCL_BINARIES_POSTFIX ".lib")
#pragma comment (lib, "cl_sys_utils.lib")

#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace Intel::OpenCL::TaskExecutor;
using namespace Intel::OpenCL::Utils;

#ifndef _USRDLL
// Shared memory for singleton object storage
// We need this shared memory because we use static library and want to have singleton across DLL's
// We need assure that the name is unique for each process
const char g_szMemoryNameTemplate[]="TaskExecuterSharedMemory(%06d)";
const char g_szMutexNameTemplate[]="TaskExecuterMutex(%06d)";

struct ExecutorSingletonHandler
{
	ExecutorSingletonHandler()
	{
		char szName[sizeof(g_szMemoryNameTemplate)+6];

		// Create process unique name
		sprintf_s(szName, sizeof(szName), g_szMemoryNameTemplate, GetCurrentProcessId());

		// Open shared memory, we are looking for previously allocated executor
		hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			nullptr,                    // default security 
			PAGE_READWRITE,          // read/write access
			0,                       // max. object size 
			sizeof(void*),           // buffer size  
			szName);         // name of mapping object
		if (hMapFile == nullptr) 
		{ 
			return;
		}

		// Get pointer to shared memory
		pSharedBuf = MapViewOfFile(hMapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,                   
			0,                   
			sizeof(void*));           
		if (pSharedBuf == nullptr) 
		{ 
			CloseHandle(hMapFile);
			return;
		}

		// Test for singleton existence
		sprintf_s(szName, sizeof(szName), g_szMutexNameTemplate, GetCurrentProcessId());
		hMutex = CreateMutex(nullptr, TRUE, szName);
		if ( nullptr == hMutex)
		{
			UnmapViewOfFile(pSharedBuf);
			CloseHandle(hMapFile);
			return;
		}
		// test if we have allocated executor
		ITaskExecutor*	*ppTaskExecutor = (ITaskExecutor**)(pSharedBuf);
		// Check if executor already exists
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			// If so, wait for completion of executor initialization
			if ( WAIT_OBJECT_0 != WaitForSingleObject(hMutex, INFINITE) )
			{
				CloseHandle(hMutex);
				UnmapViewOfFile(pSharedBuf);
				CloseHandle(hMapFile);
				return;
			}
			// The mutex exists and released, we have a pointer to task executor instance in shared buffer
			pTaskExecutor = *ppTaskExecutor;
			return;
		}

		// The mutex was created, we need allocate task executor and share it.
#ifdef __TBB_EXECUTOR__
		pTaskExecutor = new TBBTaskExecutor;
		pTaskExecutor->Init(0);
#endif
#ifdef __THREAD_EXECUTOR__
		pTaskExecutor = new ThreadTaskExecutorImpl;
		pTaskExecutor->Init(0);
#endif
		*ppTaskExecutor = pTaskExecutor;
		// Release Mutex
		ReleaseMutex(hMutex);
	}

	~ExecutorSingletonHandler()
	{
		CloseHandle(hMutex);
		UnmapViewOfFile(pSharedBuf);
		CloseHandle(hMapFile);
	}

	// Pointer to a singleton object
	static ITaskExecutor*	pTaskExecutor;
	HANDLE					hMapFile;
	LPVOID					pSharedBuf;
	HANDLE					hMutex;
};
ITaskExecutor* ExecutorSingletonHandler::pTaskExecutor = nullptr;

ExecutorSingletonHandler	executor;

ITaskExecutor* Intel::OpenCL::TaskExecutor::GetTaskExecutor()
{
	return ExecutorSingletonHandler::pTaskExecutor;
}
#else

ITaskExecutor* g_pTaskExecutor = nullptr;

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
#ifdef _DEBUG  // this is needed to initialize allocated objects DB, which is maintained in only in debug
        InitSharedPtrs();
#endif
#ifdef __TBB_EXECUTOR__
		g_pTaskExecutor = new TBBTaskExecutor;
#endif
#ifdef __THREAD_EXECUTOR__
		g_pTaskExecutor = new ThreadTaskExecutor;
#endif
		 break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:		
		if (g_pTaskExecutor) 
		{
			delete ((PTR_CAST*)g_pTaskExecutor);
			g_pTaskExecutor = nullptr;						
		}
#ifdef _DEBUG
        FiniSharedPts();
#endif
		return TRUE;
	}
	return g_pTaskExecutor != nullptr;
}

TASK_EXECUTOR_API ITaskExecutor* Intel::OpenCL::TaskExecutor::GetTaskExecutor()
{
	return g_pTaskExecutor;
}

namespace Intel { namespace OpenCL{ namespace TaskExecutor {
void RegisterReleaseSchedulerForMasterThread()
{
	// DO nothing on Windows. Function is called during DllMain(DLL_THREAD_DETACH)
}

}}}
#endif

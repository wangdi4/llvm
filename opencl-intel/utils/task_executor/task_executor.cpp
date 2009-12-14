// Copyright (c) 2006-2009 Intel Corporation
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
#include <stdafx.h>
#include "task_executor.h"

#define __THREAD_EXECUTOR__
//#define __TBB_EXECUTOR__
//#define __XN_EXECUTOR__

#ifdef __TBB_EXECUTOR__
#include "tbb_executor.h"
#define PTR_CAST	TBBTaskExecutor
#endif
#ifdef __THREAD_EXECUTOR__
#include "thread_executor.h"
#define PTR_CAST	ThreadTaskExecutor
#endif
#ifdef __XN_EXECUTOR__
#include "xn_executor.h"
#define PTR_CAST	XNTaskExecutor
#endif

#include <Windows.h>
#include <stdio.h>

using namespace Intel::OpenCL::TaskExecutor;

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
			NULL,                    // default security 
			PAGE_READWRITE,          // read/write access
			0,                       // max. object size 
			sizeof(void*),           // buffer size  
			szName);         // name of mapping object
		if (hMapFile == NULL) 
		{ 
			return;
		}

		// Get pointer to shared memory
		pSharedBuf = MapViewOfFile(hMapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,                   
			0,                   
			sizeof(void*));           
		if (pSharedBuf == NULL) 
		{ 
			CloseHandle(hMapFile);
			return;
		}

		// Test for singleton existence
		sprintf_s(szName, sizeof(szName), g_szMutexNameTemplate, GetCurrentProcessId());
		hMutex = CreateMutex(NULL, TRUE, szName);
		if ( NULL == hMutex)
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
#ifdef __XN_EXECUTOR__
		pTaskExecutor = new XNTaskExecutor;
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
ITaskExecutor* ExecutorSingletonHandler::pTaskExecutor = NULL;

ExecutorSingletonHandler	executor;

ITaskExecutor* Intel::OpenCL::TaskExecutor::GetTaskExecutor()
{
	return ExecutorSingletonHandler::pTaskExecutor;
}
#else

ITaskExecutor* g_pTaskExecutor = NULL;

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
#ifdef __TBB_EXECUTOR__
		g_pTaskExecutor = new TBBTaskExecutor;
#endif
#ifdef __THREAD_EXECUTOR__
		g_pTaskExecutor = new ThreadTaskExecutor;
#endif
#ifdef __XN_EXECUTOR__
		g_pTaskExecutor = new XNTaskExecutor;
#endif
		 break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		delete ((PTR_CAST*)g_pTaskExecutor);
		break;
	}
	return g_pTaskExecutor != NULL;
}

TASK_EXECUTOR_API ITaskExecutor* Intel::OpenCL::TaskExecutor::GetTaskExecutor()
{
	return g_pTaskExecutor;
}

#endif

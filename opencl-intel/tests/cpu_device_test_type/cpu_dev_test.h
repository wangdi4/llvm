// Copyright (c) 2006-2012 Intel Corporation
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

///////////////////////////////////////////////////////////
// cpu_dev_test.cpp
///////////////////////////////////////////////////////////

#pragma once

#include "cl_device_api.h"


#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32
	#define SLEEP(mili) Sleep(mili)
	#define DECLSPEC __declspec
	#define FOPEN(file, name, mode) fopen_s(&(file), (name), (mode))
	#define GET_THREAD_ID (unsigned int)GetThreadId(GetCurrentThread())
	#define STDCALL_ENTRY_POINT _stdcall
	#define RETURN_TYPE_ENTRY_POINT unsigned int
#else
	#define SLEEP(mili) usleep(mili * 1000)
	#define DECLSPEC __attribute__
	#define FOPEN(file, name, mode) (file) = fopen((name), (mode))
	#define GET_THREAD_ID (unsigned int)syscall(SYS_gettid)
	#define STDCALL_ENTRY_POINT __attribute((stdcall))
	#define RETURN_TYPE_ENTRY_POINT void *
#endif

extern IOCLDeviceAgent*		dev_entry;
extern volatile bool	gExecDone;

extern "C" int clDevCreateDeviceInstance(
								   unsigned int		dev_id,
								   IOCLFrameworkCallbacks	*pDevCallBacks,
								   IOCLDevLogDescriptor		*pLogDesc,
								   IOCLDeviceAgent*				*pDevice
								   );

extern "C" int clDevGetDeviceInfo(
							cl_device_info  param, 
							size_t          valSize, 
							void*           paramVal,
							size_t*         paramValSizeRet						
							);

extern "C" char* clDevErr2Txt(cl_dev_err_code error_code);
bool test_task_executor();

// Simple RT memory object implementation
class RTMemObjService : public IOCLDevRTMemObjectService
{
public:

	RTMemObjService() : m_pBS(NULL) {}
	~RTMemObjService()
	{
		if ( NULL != m_pBS )
		{
			m_pBS->RemovePendency();
		}
	}

	cl_dev_err_code GetBackingStore(cl_dev_bs_flags flags, IOCLDevBackingStore* *ppBS)
	{
		*ppBS = NULL;
		return CL_DEV_SUCCESS;
	}
	cl_dev_err_code SetBackingStore(IOCLDevBackingStore* pBS)
	{
		return CL_DEV_SUCCESS;
	}
	size_t GetDeviceAgentListSize() const
	{
		return 1;
	}

	const IOCLDeviceAgent* const *GetDeviceAgentList() const
	{
		return (const IOCLDeviceAgent* const *)&dev_entry;
	}

protected:

	IOCLDevBackingStore* m_pBS;
};

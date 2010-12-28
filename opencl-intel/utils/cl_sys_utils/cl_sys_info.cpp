/////////////////////////////////////////////////////////////////////////
// cl_utils.cpp:
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
// or disclosed in any way without Intel�s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel�s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#include <cassert>
#include "cl_sys_info.h"

using namespace Intel::OpenCL::Utils;

#ifdef WIN32
#include<windows.h>
#include <powrprof.h>
#endif

unsigned long long Intel::OpenCL::Utils::TotalVirtualSize()
{
#ifdef WIN32
	MEMORYSTATUSEX	memStatus;

	memStatus.dwLength = sizeof(MEMORYSTATUSEX);
	if ( !GlobalMemoryStatusEx(&memStatus) )
	{
		return 0;
	}
	return min(memStatus.ullTotalPhys, memStatus.ullTotalVirtual);
#else
	return 0;
#endif
}

unsigned long long Intel::OpenCL::Utils::MaxClockFrequency()
{
	ULONG maxClockFrequency = 0;

#ifdef WIN32
	// missing Windows processor power information struct
	typedef struct _PROCESSOR_POWER_INFORMATION {
		ULONG  Number;
		ULONG  MaxMhz;
		ULONG  CurrentMhz;
		ULONG  MhzLimit;
		ULONG  MaxIdleState;
		ULONG  CurrentIdleState;
	} PROCESSOR_POWER_INFORMATION , *PPROCESSOR_POWER_INFORMATION;


	PROCESSOR_POWER_INFORMATION	*powerInfo;
	SYSTEM_INFO sysInfo;
	ULONG nOutputBufferSize; 

	// find out how many processors we have in the system
	GetSystemInfo(&sysInfo);
	nOutputBufferSize = sysInfo.dwNumberOfProcessors * sizeof(PROCESSOR_POWER_INFORMATION);
	powerInfo = (PROCESSOR_POWER_INFORMATION*)malloc(nOutputBufferSize);

	if(NULL == powerInfo)
	{
		return 0;
	}

	if ( ERROR_SUCCESS != CallNtPowerInformation(ProcessorInformation,NULL, 0, powerInfo, nOutputBufferSize) )
	{
		free(powerInfo);
		return 0;
	}

	maxClockFrequency = powerInfo[0].MaxMhz;
	free(powerInfo);

#endif

	return maxClockFrequency;
}

unsigned long long Intel::OpenCL::Utils::ProfilingTimerFrequency()
{
#ifdef WIN32
	LARGE_INTEGER freq;

	QueryPerformanceFrequency(&freq);
	return freq.QuadPart;

#endif

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// HostTime - Return host time in nano second
/////////////////////////////////////////////////////////////////////////////////////////
#pragma data_seg(".MYSEC_FREQ")
static double freq = 1e9/ProfilingTimerFrequency();
#pragma data_seg()
#pragma comment(linker, "/SECTION:.MYSEC_FREQ,RWS")

unsigned long long Intel::OpenCL::Utils::HostTime()
{
#ifdef WIN32
	//Generates the rdtsc instruction, which returns the processor time stamp. 
	//The processor time stamp records the number of clock cycles since the last reset.
	LARGE_INTEGER tiks;

	QueryPerformanceCounter(&tiks);

	//Convert from ticks to nano second
	return (unsigned long long)(tiks.QuadPart * freq);
#else
	return 0;
#endif
	
}

/////////////////////////////////////////////////////////////////////////////////////////
// CurrentProcessName
/////////////////////////////////////////////////////////////////////////////////////////
void Intel::OpenCL::Utils::GetProcessName(wchar_t* pProcName, size_t strLen)
{
	assert(strLen <= MAXUINT32);
	GetModuleFileNameW((HMODULE)NULL, pProcName, (DWORD)strLen);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CurrentProcessId
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int Intel::OpenCL::Utils::GetProcessId()
{
	return GetCurrentProcessId();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Current Module Directory Name
/////////////////////////////////////////////////////////////////////////////////////////
void Intel::OpenCL::Utils::GetModuleDirectory(char* szModuleDir, size_t strLen)
{
	HMODULE hModule = NULL;
	GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCSTR)GetModuleDirectory,
		&hModule);

	GetModuleFileNameA(hModule, szModuleDir, MAX_PATH);
	char* pLastDelimiter = strrchr(szModuleDir, '\\');
	if ( NULL != pLastDelimiter )
	{
		*(pLastDelimiter+1) = 0;
	} else
	{
		szModuleDir[0] = 0;
	}
}

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

#include "cl_sys_info.h"

using namespace Intel::OpenCL::Utils;

#include<windows.h>
#include <powrprof.h>
#include <assert.h>

unsigned long long Intel::OpenCL::Utils::TotalVirtualSize()
{
	static unsigned long long vsize = 0;
	if ( 0 == vsize )
	{
		MEMORYSTATUSEX	memStatus;

		memStatus.dwLength = sizeof(MEMORYSTATUSEX);
		if ( !GlobalMemoryStatusEx(&memStatus) )
		{
			return 0;
		}
		vsize = min(memStatus.ullTotalPhys, memStatus.ullTotalVirtual);
	}
	return vsize;
}

unsigned long long Intel::OpenCL::Utils::TotalPhysicalSize()
{
	static unsigned long long psize = 0;

	if ( 0 == psize )
	{
		MEMORYSTATUSEX	memStatus;

		memStatus.dwLength = sizeof(MEMORYSTATUSEX);
		if ( !GlobalMemoryStatusEx(&memStatus) )
		{
			return 0;
		}
		psize = memStatus.ullTotalPhys;
	}
	return psize;
}

unsigned long long Intel::OpenCL::Utils::MaxClockFrequency()
{
	static unsigned long long freq = 0;
	int cpuInfo[4] = {-1};
	char buffer[sizeof(cpuInfo)*3 + 1];
	char* pBuffer = buffer;

	if ( freq )
	{
		return freq;
	}

	memset(buffer, 0, sizeof(cpuInfo)*3 + 1);
	for (unsigned int i = 0x80000002; i <= 0x80000004; i++)
	{
		__cpuid(cpuInfo, i);
		memcpy(pBuffer, cpuInfo, sizeof(cpuInfo));
		pBuffer = pBuffer + sizeof(cpuInfo);
	}

	size_t buffLen = strlen(buffer);
	long long mul = 0;
	double freqDouble = 0;
	if ((buffer[buffLen-1] == 'z') && (buffer[buffLen-2] == 'H') && ((buffer[buffLen-3] == 'M') || (buffer[buffLen-3] == 'G') || (buffer[buffLen-3] == 'T')))
	{
		switch (buffer[buffLen-3])
		{
		case 'M':
			mul = 1;
			break;
		case 'G':
			mul = 1000;
			break;
		case 'T':
			mul = 1000000;
			break;
		}

		int i = (int)buffLen - 1;
		while (i >= 0)
		{
			if (buffer[i] == ' ')
			{
				freqDouble = strtod(&(buffer[i]), NULL);
				break;
			}
			i--;
		}

	}

	// We return ClockFreq in MHz
	freq = (unsigned long long)(freqDouble * mul);
	return freq;
}

unsigned long long Intel::OpenCL::Utils::ProfilingTimerResolution()
{
	LARGE_INTEGER freq;

	QueryPerformanceFrequency(&freq);

	return (unsigned long long)(1e9/freq.QuadPart);
}

/////////////////////////////////////////////////////////////////////////////////////////
// HostTime - Return host time in nano second
/////////////////////////////////////////////////////////////////////////////////////////
#pragma data_seg(".MYSEC_FREQ")
static double timerRes = (double)ProfilingTimerResolution();
#pragma data_seg()
#pragma comment(linker, "/SECTION:.MYSEC_FREQ,RWS")

unsigned long long Intel::OpenCL::Utils::HostTime()
{
	//Generates the rdtsc instruction, which returns the processor time stamp. 
	//The processor time stamp records the number of clock cycles since the last reset.
	LARGE_INTEGER tiks;

	QueryPerformanceCounter(&tiks);

	//Convert from ticks to nano second
	return (unsigned long long)(tiks.QuadPart * timerRes);
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
void Intel::OpenCL::Utils::GetModuleDirectoryImp(const void* addr, char* szModuleDir, size_t strLen)
{
	HMODULE hModule = NULL;
	GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCSTR)addr,
		&hModule);

	GetModuleFileNameA(hModule, szModuleDir, (DWORD)strLen);
	char* pLastDelimiter = strrchr(szModuleDir, '\\');
	if ( NULL != pLastDelimiter )
	{
		*(pLastDelimiter+1) = 0;
	} else
	{
		szModuleDir[0] = 0;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////
// Specific module full path name intended for loaded library full path.
// On Win32 - it delegates to GetModuleFileNameA method. (modulePtr must be pointer to HMODULE)
// On Linux - it investigates the loaded library path from /proc/self/maps. (modulePtr must be address of method belongs to the loaded library)
////////////////////////////////////////////////////////////////////
int Intel::OpenCL::Utils::GetModulePathName(const void* modulePtr, char* fileName, size_t strLen)
{
	HMODULE hModule = *((HMODULE*)modulePtr);
	return GetModuleFileNameA(hModule, fileName, (DWORD)(strLen-1));
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the number of logical processors in the current group.
////////////////////////////////////////////////////////////////////
unsigned long Intel::OpenCL::Utils::GetNumberOfProcessors()
{
        SYSTEM_INFO sInfo;
        GetSystemInfo(&sInfo);
        return sInfo.dwNumberOfProcessors;
}


///////////////////////////////////////////////////////////////////////////////////////////
// return the number of NUMA nodes on the system
////////////////////////////////////////////////////////////////////
unsigned long Intel::OpenCL::Utils::GetMaxNumaNode()
{
    unsigned long ret = 0;
    GetNumaHighestNodeNumber(&ret);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return a bitmask representing the processors in a given NUMA node
////////////////////////////////////////////////////////////////////
bool Intel::OpenCL::Utils::GetProcessorMaskFromNumaNode(unsigned long node, affinityMask_t* pMask)
{
    return 0 != GetNumaNodeProcessorMask((unsigned char)node, pMask); 
}

/////////////////////////////////////////////////////////////////////////
// cl_utils.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2013 Intel Corporation All Rights Reserved.
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
#include "cl_sys_defines.h"
#include "cl_utils.h"

#if _MSC_VER == 1600
#include <intrin.h>
#endif

using namespace Intel::OpenCL::Utils;

#include <windows.h>
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
		MEMCPY_S(pBuffer, sizeof(cpuInfo)*3 + 1, cpuInfo, sizeof(cpuInfo));
		pBuffer = pBuffer + sizeof(cpuInfo);
	}

	size_t buffLen = strlen(buffer);
	long long mul = 0;
	double freqDouble = 0;
	assert(buffLen >= 3 && "Insufficient length of a buffer");
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
static double timerRes = (double)ProfilingTimerResolution();

unsigned long long Intel::OpenCL::Utils::HostTime()
{
	//Generates the rdtsc instruction, which returns the processor time stamp. 
	//The processor time stamp records the number of clock cycles since the last reset.
	LARGE_INTEGER tiks;

	QueryPerformanceCounter(&tiks);

	//Convert from ticks to nano second
	return (unsigned long long)(tiks.QuadPart * timerRes);
}

unsigned long long Intel::OpenCL::Utils::AccurateHostTime()
{
    static int iIsRdtscpSupported = -1;
    if (-1 == iIsRdtscpSupported)
    {
        int cpuInfo[4];
        __cpuid(cpuInfo, 0x80000001);
        iIsRdtscpSupported = (1 == ((cpuInfo[3] >> 27) & 0x1)) ? 1 : 0;
    }
    if (iIsRdtscpSupported)
    {
        unsigned int uiAux;
        return __rdtscp(&uiAux);
    }
    else
    {
        return HostTime();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// CurrentProcessName
/////////////////////////////////////////////////////////////////////////////////////////
void Intel::OpenCL::Utils::GetProcessName(char* pProcName, size_t strLen)
{
	assert(strLen <= MAXUINT32);
	GetModuleFileName((HMODULE)NULL, pProcName, (DWORD)strLen);
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
// On Win32 - it asks for the module handle then calls GetModuleFileNameA method.  (modulePtr must be address of method belongs to the loaded library)
// On Linux - it investigates the loaded library path from /proc/self/maps. (modulePtr must be address of method belongs to the loaded library)
////////////////////////////////////////////////////////////////////
int Intel::OpenCL::Utils::GetModulePathName(const void* modulePtr, char* fileName, size_t strLen)
{
	HMODULE hModule = NULL;
	GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCSTR)modulePtr,
		&hModule);
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
bool Intel::OpenCL::Utils::GetProcessorMaskFromNumaNode(unsigned long node, affinityMask_t* pMask, unsigned int* nodeSize)
{
    if (0 == GetNumaNodeProcessorMask((unsigned char)node, pMask))
    {
        return false;
    }

    unsigned int node_size = 0;
    unsigned long long mask = *pMask;
    while (0 != mask)
    {
        if (mask & 0x1)
        {
            ++node_size;
        }
        mask >>= 1;
    }
    if (NULL != nodeSize)
    {
        *nodeSize = node_size;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the ID of the CPU the current thread is running on
////////////////////////////////////////////////////////////////////
unsigned int Intel::OpenCL::Utils::GetCpuId()
{
    return GetCurrentProcessorNumber();
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the full path to the module that is about to be loaded
////////////////////////////////////////////////////////////////////
const char* Intel::OpenCL::Utils::GetFullModuleNameForLoad(const char* moduleName)
{
	static _declspec(thread) char sModulePath[MAX_PATH];
	
	GetModuleDirectory(sModulePath, MAX_PATH);
	sprintf_s(sModulePath, MAX_PATH, "%s%s", sModulePath, moduleName);

	return sModulePath;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the product version:
// On Windows: it returns the product version number that is stored in the version info of the shared object
// On Linux:   not implemented TODO
// Arguments - someLocalFunc - some function in the requested module
//             major, minor, revision, build - output version numbers
////////////////////////////////////////////////////////////////////
bool Intel::OpenCL::Utils::GetModuleProductVersion(const void* someLocalFunc, int* major, int* minor, int* revision, int* build)
{
    char filePath[MAX_PATH];
    DWORD  verInfoSize          = 0;
    BYTE*  verInfo              = NULL;
    UINT   fileInfoSize         = 0;
    VS_FIXEDFILEINFO *fileInfo  = NULL;

    GetModulePathName(someLocalFunc, filePath, MAX_PATH - 1);

    verInfoSize = GetFileVersionInfoSize( filePath, NULL );
    if (0 == verInfoSize)
    {
        return false;
    }

    verInfo = (BYTE*) STACK_ALLOC(sizeof(BYTE)*verInfoSize);

    if (!GetFileVersionInfo(filePath, 0, verInfoSize, verInfo))
    {
        STACK_FREE(verInfo);
        return false;
    }

    if (!VerQueryValue(verInfo, TEXT("\\"), (LPVOID*) &fileInfo, &fileInfoSize))
    {
        STACK_FREE(verInfo);
        return false;
    }

    *major    = ( fileInfo->dwProductVersionMS >> 16 ) & 0xffff;
    *minor    = ( fileInfo->dwProductVersionMS) & 0xffff;
    *revision = ( fileInfo->dwProductVersionLS >> 16 ) & 0xffff;
    *build    = ( fileInfo->dwProductVersionLS) & 0xffff;

    STACK_FREE(verInfo);
    return true;
}

unsigned int Intel::OpenCL::Utils::GetThreadId()
{
    return GetCurrentThreadId();
}

// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
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

#include "cl_sys_info.h"
#include "cl_sys_defines.h"
#include "cl_utils.h"

#if _MSC_VER == 1600
#include <intrin.h>
#endif

using namespace Intel::OpenCL::Utils;

#include <algorithm>
#include <bitset>
#include <vector>

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
				freqDouble = strtod(&(buffer[i]), nullptr);
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
	GetModuleFileName((HMODULE)nullptr, pProcName, (DWORD)strLen);
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
	HMODULE hModule = nullptr;
	GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCSTR)addr,
		&hModule);

	GetModuleFileNameA(hModule, szModuleDir, (DWORD)strLen);
	char* pLastDelimiter = strrchr(szModuleDir, '\\');
	if ( nullptr != pLastDelimiter )
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
	HMODULE hModule = nullptr;
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
    DWORD Size = 0;

    // Ask for buffer size first
    auto Status = GetLogicalProcessorInformationEx(RelationProcessorCore, NULL, &Size);

    assert((!Status && Size) && "Failed to get required buffer size!");

    // Alloc the memory
    std::vector<unsigned char> Bytes(Size);

    // Fill allocated buffer with information
    Status = GetLogicalProcessorInformationEx(RelationProcessorCore,
        (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)Bytes.data(), &Size);
    assert(Status && "Failed to get Logical Processor Info!");

    // Parse the info
    unsigned long CpuCount = 0;
    unsigned char *BytePtr = Bytes.data(), *BytePtrEnd = Bytes.data() + Size;
    while (BytePtr < BytePtrEnd) {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX LPI = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)BytePtr;
        for (int i = 0; i < LPI->Processor.GroupCount; ++i) {
            std::bitset<64> bits(LPI->Processor.GroupMask[i].Mask);
            CpuCount += bits.count();
        }
        BytePtr += LPI->Size;
    }

    return CpuCount;
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
    if (nullptr != nodeSize)
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

unsigned int Intel::OpenCL::Utils::GetThreadId()
{
    return GetCurrentThreadId();
}

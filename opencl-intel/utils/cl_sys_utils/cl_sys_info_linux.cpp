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

#include <sstream>
#include "cl_sys_info.h"
#include "cl_utils.h"

using namespace Intel::OpenCL::Utils;
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <assert.h>
#include <unistd.h>

#include <sys/resource.h> 
#include <sys/sysinfo.h>

#ifndef __ANDROID__
#include <sys/syscall.h>
#endif

#ifndef DISABLE_NUMA_SUPPORT
#define DISABLE_NUMA_SUPPORT
#endif

#ifndef DISABLE_NUMA_SUPPORT
//cl_numa.h is actually the standard numa.h from numactl. I don't know why our Linux distro doesn't have it and I don't care enough
#include <numa.h>
#endif //DISABLE_NUMA_SUPPORT

#include "hw_utils.h"
#include "cl_secure_string_linux.h"

#include "valgrind/valgrind.h"

unsigned long long Intel::OpenCL::Utils::TotalVirtualSize()
{

	static unsigned long long vsize = 0;
	if ( 0 == vsize )
	{
		rlimit tLimitStruct;
		if (getrlimit(RLIMIT_AS, &tLimitStruct) != 0)
		{
			return 0;
		}
		unsigned long long totalVirtual = tLimitStruct.rlim_cur;

		struct sysinfo tSysInfoStruct;
		if (sysinfo(&tSysInfoStruct) != 0)
		{
			return 0;
		}
		unsigned long long totalPhys = tSysInfoStruct.totalram * tSysInfoStruct.mem_unit;

		vsize = min(totalPhys, totalVirtual);
	}
	return vsize;
}

unsigned long long Intel::OpenCL::Utils::TotalPhysicalSize()
{
	static unsigned long long totalPhys = 0;
	if ( 0 == totalPhys )
	{
		struct sysinfo tSysInfoStruct;
		if (sysinfo(&tSysInfoStruct) != 0)
		{
			return 0;
		}
		totalPhys = tSysInfoStruct.totalram * tSysInfoStruct.mem_unit;
	}

	return totalPhys;
}

unsigned long long Intel::OpenCL::Utils::MaxClockFrequency()
{
	static unsigned long long freq = 0;
	unsigned int cpuInfo[4] = {-1};
	char buffer[sizeof(cpuInfo)*3 + 1];
	char* pBuffer = buffer;

	if ( freq )
	{
		return freq;
	}

	memset(buffer, 0, sizeof(cpuInfo)*3 + 1);
	for (unsigned int i = 0x80000002; i <= 0x80000004; i++)
	{
		cpuid(cpuInfo, i);
		memcpy(pBuffer, cpuInfo, sizeof(cpuInfo));
		pBuffer = pBuffer + sizeof(cpuInfo);
	}

	int buffLen = strlen(buffer);
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

		int i = buffLen - 1;
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
	/* sys_clock_getres returns resolution (ns interval between ticks) and not the frequency. */
	struct timespec tp;

	clock_getres(CLOCK_MONOTONIC, &tp);
	return tp.tv_nsec;
}

/////////////////////////////////////////////////////////////////////////////////////////
// HostTime - Return host time in nano second
/////////////////////////////////////////////////////////////////////////////////////////
unsigned long long Intel::OpenCL::Utils::HostTime()
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return (unsigned long long)(tp.tv_sec) * 1000000000 + tp.tv_nsec;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CurrentProcessName
/////////////////////////////////////////////////////////////////////////////////////////
void Intel::OpenCL::Utils::GetProcessName(char* pProcName, size_t strLen)
{
	const int readChars = readlink("/proc/self/exe", pProcName, strLen);
	if (-1 == readChars)
	{
        pProcName[0] = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// CurrentProcessId
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int Intel::OpenCL::Utils::GetProcessId()
{
	return getpid();
}


/////////////////////////////////////////////////////////////////////////////////////////
// Current Module Directory Name
/////////////////////////////////////////////////////////////////////////////////////////
void Intel::OpenCL::Utils::GetModuleDirectoryImp(const void* addr, char* szModuleDir, size_t strLen)
{
	const int readChars = GetModulePathName(addr, szModuleDir, strLen - 1);
	if (readChars > 0)
	{
		char* pLastDelimiter = strrchr(szModuleDir, '/');
		if ( NULL != pLastDelimiter )
		{
			*(pLastDelimiter+1) = 0;
		}
		else
		{
			szModuleDir[0] = 0;
		}
	}
	else
	{
		szModuleDir[0] = 0;
	}
}

int CharToHexDigit(char c)
{

	if ((c >= '0') && (c <= '9'))
	{
		return c - '0';
	}
	if ((c >= 'a') && (c <= 'f'))
	{
		return c - 'a' + 10;
	}
	if ((c >= 'A') && (c <= 'F'))
	{
		return c - 'A' + 10;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Specific module full path name intended for loaded library full path.
// On Win32 - it asks for the module handle then calls GetModuleFileNameA method.  (modulePtr must be address of method belongs to the loaded library)
// On Linux - it investigates the loaded library path from /proc/self/maps. (modulePtr must be address of method belongs to the loaded library)
////////////////////////////////////////////////////////////////////
int Intel::OpenCL::Utils::GetModulePathName(const void* modulePtr, char* fileName, size_t strLen)
{
	if ((fileName == NULL) || (strLen <= 0))
	{
		return 0;
	}
	ifstream ifs("/proc/self/maps", ifstream::in);
	if (ifs == NULL)
	{
		fileName[0] = 0;
		return 0;
	}
	string address, perms, offset, dev, inode, pathName;

	char buff[MAX_PATH + 1024];
	while (ifs.getline(buff, MAX_PATH + 1024))
	{
		istringstream strStream(buff);
		address = "\0";
		pathName = "\0";
		strStream >> address >> perms >> offset >> dev >> inode >> pathName;
		if ((address != "\0") && (pathName != "\0"))
		{
			string::size_type pos = address.find("-");
			if (pos != string::npos)
			{
				size_t from = 0;
				size_t to = 0;
				bool legalAddress = true;
				for (unsigned int i = 0; ((i < pos) && (legalAddress)); i++)
				{
					int digit = CharToHexDigit(address.at(i));
					if (digit >= 0) {
						from = (from << 4) + digit;
					}
					else
					{
						legalAddress = false;
					}
				}
				if (!legalAddress)
				{
					continue;
				}
				int len = address.length();
				for (int i = pos + 1; ((i < len) && (legalAddress)); i++)
				{
					int digit = CharToHexDigit(address.at(i));
					if (digit >= 0) {
						to = (to << 4) + digit;
					}
					else
					{
						legalAddress = false;
					}
				}
				if (!legalAddress)
				{
					continue;
				}
				if (((size_t)modulePtr >= from) && ((size_t)modulePtr <= to))
				{
					if (0 != safeStrCpy(fileName, strLen, pathName.c_str()))
					{
						int counter = 0;
						for (unsigned int i = 0; ((i < strLen - 1) && (i < pathName.length())); i++)
						{
							fileName[i] = pathName.at(i);
							counter ++;
						}
						fileName[counter] = 0;
						return counter;
					}
					return pathName.length();
				}
			}

		}
	}
	fileName[0] = 0;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the number of processors configured.
// It is also possible to ge the number of processors currently online (Available),
// by changing the function to sysconf(_SC_NPROCESSORS_ONLN)
////////////////////////////////////////////////////////////////////
unsigned long Intel::OpenCL::Utils::GetNumberOfProcessors()
{
    static unsigned long numProcessors = 0;
    if (0 == numProcessors)
    {
        affinityMask_t mask;
        threadid_t mainThreadTID = getpid();
        clGetThreadAffinityMask(&mask, mainThreadTID);
        numProcessors = CPU_COUNT(&mask);
    }
    return numProcessors;        
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the number of NUMA nodes on the system
////////////////////////////////////////////////////////////////////
unsigned long Intel::OpenCL::Utils::GetMaxNumaNode()
{
#ifndef DISABLE_NUMA_SUPPORT
    int ret = numa_max_node();
    return (unsigned long)ret;
#else
    return 0;
#endif //DISABLE_NUMA_SUPPORT
}

///////////////////////////////////////////////////////////////////////////////////////////
// return a bitmask representing the processors in a given NUMA node
////////////////////////////////////////////////////////////////////
bool Intel::OpenCL::Utils::GetProcessorMaskFromNumaNode(unsigned long node, affinityMask_t* pMask, unsigned int* nodeSize)
{
#ifndef DISABLE_NUMA_SUPPORT
    struct bitmask b;
    unsigned long long CPUs;
    b.size  = 8 * sizeof(unsigned long long);
    b.maskp = (unsigned long *)(&CPUs);
    int ret = numa_node_to_cpus((int)node, &b);
    if (0 != ret)
    {
        return false;
    }
    CPU_ZERO(pMask);
    int cpu = 0;
    unsigned int node_size = 0;
    while (0 != CPUs)
    {
        if (CPUs & 0x1)
        {
            CPU_SET(cpu, pMask);
            ++node_size;
        }
        CPUs >>= 1;
        ++cpu;
    }
    if (NULL != nodeSize)
    {
        *nodeSize = node_size;
    }
    return true;
#else 
    return false;
#endif //DISABLE_NUMA_SUPPORT
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the ID of the CPU the current thread is running on
////////////////////////////////////////////////////////////////////
unsigned int Intel::OpenCL::Utils::GetCpuId()
{
	int id;

	if (RUNNING_ON_VALGRIND)
	{
		id = 0;
	} else {
#if defined(__ANDROID__)
		if( syscall(__NR_getcpu, &id, NULL, NULL) < 0 )
		{
			return 0;
		}
#else
		id = sched_getcpu();
#endif
	}
	assert(id >= 0);
	return (unsigned int)id;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the full path to the module that is about to be loaded
// In linux returns only the modules name as modules are loaded according to LD_LIBRARY_PATH
////////////////////////////////////////////////////////////////////
const char* Intel::OpenCL::Utils::GetFullModuleNameForLoad(const char* moduleName)
{
	return moduleName;
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
    return false;
}

unsigned int Intel::OpenCL::Utils::GetThreadId()
{
#if defined(__ANDROID__) //we would like to use CONF but it's buggy on Android
	return (unsigned int) gettid();
#else
    return (unsigned int)syscall(SYS_gettid);
#endif	
}

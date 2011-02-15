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

#include <sstream>
#include "cl_sys_info.h"

using namespace Intel::OpenCL::Utils;
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <assert.h>
#include <unistd.h>

#include <sys/resource.h> 
#include <sys/sysinfo.h>

#include "hw_utils.h"
#include "cl_secure_string_linux.h"
unsigned long long Intel::OpenCL::Utils::TotalVirtualSize()
{

	unsigned long long vsize = 0;
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
	unsigned long long totalPhys = tSysInfoStruct.totalram;

	vsize = min(totalPhys, totalVirtual);
	return vsize;
}

unsigned long long Intel::OpenCL::Utils::MaxClockFrequency()
{
	unsigned long long freq = 0;
	int cpuInfo[4] = {-1};
	char buffer[sizeof(cpuInfo)*3 + 1];
	char* pBuffer = buffer;
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
				mul = 1000000;
				break;
			case 'G':
				mul = 1000000000;
				break;
			case 'T':
				mul = 1000000000000;
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

	freq = (long long unsigned int)(freqDouble * mul);
	return freq;
}

unsigned long long Intel::OpenCL::Utils::ProfilingTimerFrequency()
{
	unsigned long freq = 0;
	FILE* f = fopen("/proc/cpuinfo", "r");
	const int bufSize = 1024;
	char buf[bufSize];
	int buflen = fread(buf,1,bufSize,f);
	fclose(f);
	if (buflen > 0)
	{
		const char marker[] = "cpu MHz\t\t:";
		const char* mhz = ::strstr(buf, marker);
		if (mhz != NULL)
		{
			freq = (unsigned long)strtod(mhz + sizeof(marker), NULL);
		}
	}

	return freq;
}

/////////////////////////////////////////////////////////////////////////////////////////
// HostTime - Return host time in nano second
/////////////////////////////////////////////////////////////////////////////////////////
unsigned long long Intel::OpenCL::Utils::HostTime()
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return (unsigned long long)(tp.tv_sec * 1000000000 + tp.tv_nsec);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CurrentProcessName
/////////////////////////////////////////////////////////////////////////////////////////
void Intel::OpenCL::Utils::GetProcessName(wchar_t* pProcName, size_t strLen)
{
	char buf[MAX_PATH];
	const int readChars = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
	if (readChars != -1)
	{
		buf[readChars] = 0;
		mbstowcs(pProcName, buf, readChars + 1);
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
// On Win32 - it delegates to GetModuleFileNameA method. (modulePtr must be pointer to HMODULE)
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
        return sysconf(_SC_NPROCESSORS_CONF);
}


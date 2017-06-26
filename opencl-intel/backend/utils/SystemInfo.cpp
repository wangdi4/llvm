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

File Name:  SystemInfo.cpp

\*****************************************************************************/

#include "SystemInfo.h"

#if defined(_WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#include <linux/limits.h>
#define MAX_PATH PATH_MAX
#endif

#include <time.h>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <time.h>
#include <assert.h>
#include <string.h>


using namespace Intel::OpenCL::DeviceBackend::Utils;
using namespace std;

SystemInfo::SystemInfo(void)
{
}

SystemInfo::~SystemInfo(void) 
{
}

static int CharToHexDigit( char c)
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

unsigned long long SystemInfo::HostTime()
{
#if defined (_WIN32)
  
  LARGE_INTEGER freqInfo;

  QueryPerformanceFrequency( &freqInfo);

  static double freq = 1e9/((unsigned long long) freqInfo.QuadPart);
  
  //Generates the rdtsc instruction, which returns the processor time stamp. 
  //The processor time stamp records the number of clock cycles since the last reset.
  LARGE_INTEGER ticks;

  QueryPerformanceCounter( &ticks);

  //Convert from ticks to nano second
  return (unsigned long long)(ticks.QuadPart * freq);
#else
  struct timespec tp;
  clock_gettime( CLOCK_MONOTONIC, &tp);
  return (unsigned long long)(tp.tv_sec * 1000000000 + tp.tv_nsec);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
// Current Module Directory Name
/////////////////////////////////////////////////////////////////////////////////////////
void SystemInfo::GetModuleDirectory( char* szModuleDir, size_t strLen)
{
   assert( szModuleDir && strLen > 0);

#if defined(_WIN32)
  HMODULE hModule = NULL;
  // GetModuleHandleExA receives an address in the module (the GetModuleDirectory 
  // method) and return the handler of the module
  GetModuleHandleExA(
    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
    (LPCSTR)GetModuleDirectory,
    &hModule);

  GetModuleFileNameA( hModule, szModuleDir, MAX_PATH);
  char* pLastDelimiter = strrchr( szModuleDir, '\\');
  if ( NULL != pLastDelimiter )
  {
    *(pLastDelimiter+1) = 0;
  } else
  {
    szModuleDir[0] = 0;
  }
#else
// On Linux - it investigates the loaded library path from /proc/self/maps. 
// (modulePtr must be address of method belongs to the loaded library)
////////////////////////////////////////////////////////////////////
  ifstream ifs( "/proc/self/maps", ifstream::in);
  if (!ifs.good()) {
    szModuleDir[0] = 0;
    return;
  }
  string address, perms, offset, dev, inode, pathName;

  char buff[MAX_PATH + 1024];
  while (ifs.getline( buff, MAX_PATH + 1024))
  {
    istringstream strStream( buff );
    address = "\0";
    pathName = "\0";
    strStream >> address >> perms >> offset >> dev >> inode >> pathName;
    if ((address != "\0") && (pathName != "\0")) {
      string::size_type pos = address.find( "-");
      if (pos != string::npos) {
        size_t from = 0;
        size_t to = 0;
        bool legalAddress = true;
        for (unsigned int i = 0; ((i < pos) && (legalAddress)); i++)
        {
          int digit = CharToHexDigit( address.at(i));
          if (digit >= 0) {
            from = (from << 4) + digit;
          } else {
            legalAddress = false;
          }
        }
        if (!legalAddress) {
          continue;
        }
        int len = address.length();
        for (int i = pos + 1; ((i < len) && (legalAddress)); i++)
        {
          int digit = CharToHexDigit( address.at(i));
          if (digit >= 0) {
            to = (to << 4) + digit;
          }
          else {
            legalAddress = false;
          }
        }
        if (!legalAddress) {
          continue;
        }
        if (((size_t)GetModuleDirectory >= from) && ((size_t)GetModuleDirectory <= to))
        {
          if (0 != strncpy( szModuleDir, pathName.c_str(), strLen))
          {
            int counter = 0;
            for (unsigned int i = 0; ((i < strLen - 1) && (i < pathName.length())); i++)
            {
              szModuleDir[i] = pathName.at(i);
              counter ++;
            }
            szModuleDir[counter] = 0;
          }
          char* pLastDelimiter = strrchr( szModuleDir, '/');
          if (NULL != pLastDelimiter) {
            *(pLastDelimiter+1) = 0;
          } else {
            szModuleDir[0] = 0;
          }
          return;
        }
      }
    }
  }
  szModuleDir[0] = 0;
#endif
}


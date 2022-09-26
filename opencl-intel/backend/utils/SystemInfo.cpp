// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#include "SystemInfo.h"
#include "llvm/Support/Path.h"

#if defined(_WIN32)
#include <Windows.h>
#include <codecvt>
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

std::string SystemInfo::GetExecutableFilename() {
  std::string name;
#if defined(WIN32)
  WCHAR path[MAX_PATH];
  if (GetModuleFileNameW(GetModuleHandleW(nullptr), path, MAX_PATH)) {
    std::wstring wstr(path);
    name = std::string(wstr.begin(), wstr.end());
  }
  // Remove .exe from the name.
  if (!name.empty())
    name = llvm::sys::path::stem(llvm::StringRef(name)).str();
#else // WIN32
  char buf[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
  if (-1 != len) {
    buf[len] = '\0';
    name = buf;
  }
  // Find the base name by searching for the last '/' in the name
  if (!name.empty())
    name = llvm::sys::path::filename(llvm::StringRef(name)).str();
#endif // WIN32

  return name;
}

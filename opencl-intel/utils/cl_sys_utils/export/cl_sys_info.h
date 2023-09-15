// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
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

#pragma once

#include "cl_sys_defines.h"
#include "cl_utils.h"
#include <stdint.h>
#include <unordered_map>
#include <vector>

namespace Intel {
namespace OpenCL {
namespace Utils {
#define GetModuleDirectory(str, strLen)                                        \
  GetModuleDirectoryImp(__FUNCTION__, str, strLen)
extern unsigned long long TotalVirtualSize();
extern unsigned long long TotalPhysicalSize();
extern unsigned long long MaxClockFrequency();
extern unsigned long long ProfilingTimerResolution();
extern unsigned long long HostTime();
extern unsigned long long AccurateHostTime();
extern void GetProcessName(char *pProcName, size_t strLen);
extern unsigned int GetProcessId();
extern void GetModuleDirectoryImp(const void *addr, char *szModuleDir,
                                  size_t strLen);
extern int GetModulePathName(const void *modulePtr, char *fileName,
                             size_t strLen);
extern std::string GetClangRuntimePath();
extern unsigned long GetNumberOfProcessors();
extern unsigned int GetNumberOfCpuSockets();
extern bool IsHyperThreadingEnabled();
extern unsigned long GetMaxNumaNode();
extern bool GetProcessorIndexFromNumaNode(unsigned long node,
                                          std::vector<cl_uint> &index);
extern bool GetProcessorMaskFromNumaNode(unsigned long node,
                                         affinityMask_t *pMask,
                                         unsigned int *nodeSize = nullptr);
extern std::unordered_map<int, int> GetProcessorToSocketMap();
extern unsigned int GetCpuId();
extern const char *GetFullModuleNameForLoad(const char *moduleName);
extern const char *GetModuleProductVersion();
extern unsigned int GetThreadId();
extern void InitHwloc();
extern void DestroyHwloc();
} // namespace Utils
} // namespace OpenCL
} // namespace Intel

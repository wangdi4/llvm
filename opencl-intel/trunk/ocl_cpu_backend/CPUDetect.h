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

File Name:  CPUDetect.h

\*****************************************************************************/

#pragma once
#include "TargetArch.h"

#include "TargetArch.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

class CPUDetect
{
public:

  static CPUDetect *  GetInstance() { return &m_Instance; }
  unsigned            GetCPUFeatureSupport() const { return m_uiCPUFeatures; }
  ECPU                GetCPUByName(const char *CPUName) const;
  const char*         GetCPUName() const { return m_CPUNames[m_CPU]; }
  const char*         GetCPUName(ECPU CPU) const { return m_CPUNames[CPU]; }
  const char*         GetCPUPrefix() const { return m_CPUPrefixes[m_CPU]; }
  const char*         GetCPUPrefix(ECPU CPU) const { return m_CPUPrefixes[CPU]; }
  unsigned            GetLatestSupportedFeature() const { return 1 << m_CPU; }
  ECPU                GetCPUId() const { return m_CPU; }  
  bool                IsMICCPU(ECPU cpuId);

  static bool         HasAVX1(ECPU CPU) { return CPU >= CPU_SANDYBRIDGE; }
  static bool         HasAVX2(ECPU CPU) { return CPU >= CPU_HASWELL; }
  static unsigned     GetLatestSupportedFeature(ECPU CPU) { return (1 << CPU); }
  
private:
    CPUDetect(void);
    ~CPUDetect(void);

    const char* m_CPUNames[CPU_LAST];
    const char* m_CPUEnvNames[CPU_LAST];
    const char* m_CPUPrefixes[CPU_LAST];

    unsigned int m_uiCPUFeatures;
    ECPU m_CPU;

    static CPUDetect m_Instance;
};

inline CPUDetect * CPUInfoDetect() { return CPUDetect::GetInstance(); }

}}}}


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
#include "ProcessorDetect.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

class CPUDetect : public ProcessorDetect
{
public:
  static CPUDetect *  GetInstance() {
      if (!m_Instance)
          m_Instance = new CPUDetect();
      return m_Instance;
  }
  static void Release() { delete m_Instance; m_Instance = 0; }
  const CPUId & GetCPUId() { return m_CPUId; }
private:
    CPUDetect(void);
    ~CPUDetect(void);

    CPUId m_CPUId;
    static CPUDetect *m_Instance;
};

inline CPUDetect * CPUInfoDetect() { return CPUDetect::GetInstance(); }

}}}}


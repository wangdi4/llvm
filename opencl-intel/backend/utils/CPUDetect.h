/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __CPU_DETECT_H__
#define __CPU_DETECT_H__

#include "TargetArch.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

class CPUDetect
{
public:
  static CPUDetect * GetInstance() {
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

}}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

#endif // __CPU_DETECT_H__
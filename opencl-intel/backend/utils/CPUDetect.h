// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

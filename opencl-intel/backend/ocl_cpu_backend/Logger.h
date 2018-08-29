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

#pragma once

#include "cl_device_api.h"
#include "CL/cl.h"

#include <fstream>

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

class Logger
{
public:

  enum LogLevel
  {
    DEBUG_LEVEL = 100,
    INFO_LEVEL = 200,
    ERROR_LEVEL = 300
  };

  Logger(const wchar_t * name, LogLevel level);
  ~Logger(void);

  void Log(LogLevel level, const wchar_t * message, ...);

private:
  IOCLDevLogDescriptor* m_pLogDescriptor;
  cl_int                m_iLogHandle;

  std::wfstream filestr;

};

}}}}

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

#include "Logger.h"

#include <stdarg.h>
#include <string>


using namespace Intel::OpenCL::DeviceBackend::Utils;

Logger::Logger(const wchar_t * name, Logger::LogLevel level) : m_pLogDescriptor(0),m_iLogHandle(0)
{
#ifdef _DEBUG
  //// TODO : fix m_uiCpuId
  //cl_int m_uiCpuId = 0;
  //m_pLogDescriptor->clLogCreateClient(m_uiCpuId, name, &m_iLogHandle);

  filestr.open ("OclCpuBackEnd.txt", std::fstream::out | std::fstream::app);
#endif
}

Logger::~Logger(void) 
{
#ifdef _DEBUG
  //if ( NULL != m_pLogDescriptor )
  //{
  //  m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
  //}

  filestr.close();
#endif

}

void Logger::Log(Logger::LogLevel level, const wchar_t * message, ...)
{

#ifdef _DEBUG
  //if (m_pLogDescriptor && m_iLogHandle) 
  //{
  //  // TODO : fix WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__
  //  // m_pLogDescriptor->clLogAddLine(m_iLogHandle, cl_int(level),WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, message,  __VA_ARGS__);
  //}
  std::wstring levelStr;
  switch (level) 
  {
  case DEBUG_LEVEL:
    levelStr = L"Debug";
    break;
  case INFO_LEVEL:
    levelStr = L"Info";
    break;
  case ERROR_LEVEL:
    levelStr = L"Error";
    break;
  }
  std::wstring messageStr(message);
  filestr << levelStr << L": " << messageStr << std::endl;

#endif

}

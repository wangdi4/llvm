// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#include "log_message.h"
#include "cl_sys_defines.h"
#include <assert.h>
#include <sstream> // required by: owstringstream
#include <stdarg.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include "cl_secure_string_linux.h"
#endif

using namespace Intel::OpenCL::Utils;

/////////////////////////////////////////////////////////////////////////////////////////
// LogMessage Ctor Implementation
/////////////////////////////////////////////////////////////////////////////////////////
LogMessage::LogMessage(ELogLevel eLevel, ELogConfigField eConfig,
                       const char *psClientName, const char *psSourceFile,
                       const char *psFunctionName, __int32 i32SourceLine,
                       const char *psMessage, va_list va) {
  m_bUnicodeMessage = false;

  m_eLogLevel = eLevel;
  m_eLogConfig = eConfig;
  m_psMessage = psMessage;
  m_psSourceFile = psSourceFile;
  m_i32SourceLine = i32SourceLine;
  m_psFunctionName = psFunctionName;
  VA_COPY(m_va, va);
  m_psClientName = psClientName;
  m_psFormattedMsg = nullptr;

  CreateFormattedMessage();
  VA_END(m_va);
}

/////////////////////////////////////////////////////////////////////////////////////////
// LogMessage Dtor
/////////////////////////////////////////////////////////////////////////////////////////

LogMessage::~LogMessage() {
  if (m_bUnicodeMessage && m_psFormattedMsg) {
    delete[] m_psFormattedMsg;
  } else if (m_psFormattedMsg) {
    delete[] m_psFormattedMsg;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
// CreateFormattedMessage
/////////////////////////////////////////////////////////////////////////////////////////
void LogMessage::CreateFormattedMessage() {

  char szLine[MAX_LOG_STRING_LENGTH] = {0};
  STRCAT_S(szLine, MAX_LOG_STRING_LENGTH, "\n");

  // std::wostringstream tmpFormatMessage;
  // tmpFormatMessage <<
  // "\n////////////////////////////////////////////////////////////////\n";

  // Message format:
  // <LEVEL>|<TAB>|<DATE>|<TAB>|<TIME>|<TAB>|<PID>|<TAB>|<TID>|<TAB>|<FILE>(<LINE#>)|<TAB>|<FUNC>|<TAB>|<MSG>

  // write log level
  switch (m_eLogLevel) {
  case LL_DEBUG:
    STRCAT_S(szLine, MAX_LOG_STRING_LENGTH, "DEBUG\t");
    break;
  case LL_INFO:
    STRCAT_S(szLine, MAX_LOG_STRING_LENGTH, "INFO\t");
    break;
  case LL_ERROR:
    STRCAT_S(szLine, MAX_LOG_STRING_LENGTH, "ERROR\t");
    break;
  case LL_CRITICAL:
    STRCAT_S(szLine, MAX_LOG_STRING_LENGTH, "CRITICAL\t");
    break;
  case LL_STATISTIC:
    STRCAT_S(szLine, MAX_LOG_STRING_LENGTH, "STATISTIC\t");
    break;
  default:;
  }

  // write client name
  if ((m_eLogConfig & LCF_LINE_CLIENT_NAME) && m_psClientName != nullptr &&
      strlen(m_psClientName) > 0) {
    SPRINTF_S(&(szLine[strlen(szLine)]), MAX_LOG_STRING_LENGTH - strlen(szLine),
              "%s\t", m_psClientName);
  }

  // get time and date
  time_t tNow = 0;
  tm tmNow;
  tNow = time(nullptr);
  GMTIME(tmNow, tNow);

  // date
  if (m_eLogConfig & LCF_LINE_DATE) {

    strftime(&szLine[strlen(szLine)], MAX_LOG_STRING_LENGTH - strlen(szLine),
             "%x\t", &tmNow);
  }

  // time
  if (m_eLogConfig & LCF_LINE_TIME) {
    strftime(&szLine[strlen(szLine)], MAX_LOG_STRING_LENGTH - strlen(szLine),
             "%X\t", &tmNow);
  }

  // write process ID
  if (m_eLogConfig & LCF_LINE_PID) {
    SPRINTF_S(&(szLine[strlen(szLine)]), MAX_LOG_STRING_LENGTH - strlen(szLine),
              "%d\t", GET_CURRENT_PROCESS_ID());
  }

  // write thread ID
  if (m_eLogConfig & LCF_LINE_TID) {
    SPRINTF_S(&(szLine[strlen(szLine)]), MAX_LOG_STRING_LENGTH - strlen(szLine),
              "%d\t", GET_CURRENT_THREAD_ID());
  }

  // write source file name
  if (nullptr != m_psSourceFile && strlen(m_psSourceFile) > 0) {
    SPRINTF_S(&(szLine[strlen(szLine)]), MAX_LOG_STRING_LENGTH - strlen(szLine),
              "%s\t", m_psSourceFile);
  }

  // write line number
  if (m_i32SourceLine >= 0) {
    SPRINTF_S(&(szLine[strlen(szLine)]), MAX_LOG_STRING_LENGTH - strlen(szLine),
              "(%d)\t", m_i32SourceLine);
  }

  // write function name
  if (nullptr != m_psFunctionName && strlen(m_psFunctionName) > 0) {
    SPRINTF_S(&(szLine[strlen(szLine)]), MAX_LOG_STRING_LENGTH - strlen(szLine),
              "%s\t", m_psFunctionName);
  }

  // write message
  VSPRINTF_S(&(szLine[strlen(szLine)]), MAX_LOG_STRING_LENGTH - strlen(szLine),
             m_psMessage, m_va);

  m_psFormattedMsg = new char[MAX_LOG_STRING_LENGTH];
  STRCPY_S(m_psFormattedMsg, MAX_LOG_STRING_LENGTH, szLine);
}

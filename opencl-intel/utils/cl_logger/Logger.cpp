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

#include "Logger.h"
#include "cl_user_logger.h"
#include "llvm/Support/ManagedStatic.h"

#include <assert.h>
#include <malloc.h>
#include <sstream> // required by: owstringstream
#include <stdarg.h>
#include <stdio.h>

using namespace Intel::OpenCL::Utils;

/////////////////////////////////////////////////////////////////////////////////////////
// Logger Ctor
/////////////////////////////////////////////////////////////////////////////////////////
Logger::Logger() {
  m_bIsActive = false;
  memset(m_logHandlers, 0, sizeof(m_logHandlers));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger Dtor
/////////////////////////////////////////////////////////////////////////////////////////
Logger::~Logger() {}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::GetInstance
/////////////////////////////////////////////////////////////////////////////////////////
static llvm::ManagedStatic<Logger> LoggerInstance;
Logger *Logger::GetInstance() { return &*LoggerInstance; }

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::AddLogHandler
/////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Logger::AddLogHandler(LogHandler *logHandler) {

  std::lock_guard<std::mutex> CS(m_CS); // Lock the function
  int i;
  for (i = 0; i < MAX_LOG_HANDLERS; i++) {
    if (m_logHandlers[i] == logHandler) {
      return CL_ERR_LOGGER_FAILED;
    }
    if (m_logHandlers[i] == nullptr) {
      m_logHandlers[i] = logHandler;
      return CL_SUCCESS;
    }
  }
  return CL_ERR_LOGGER_FAILED;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::GetLogHandlerParams
/////////////////////////////////////////////////////////////////////////////////////////
const char *Logger::GetLogHandlerParams(const char * /*logHandler*/) {
  // not implemented yet
  assert(false);
  return "";
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::Log
/////////////////////////////////////////////////////////////////////////////////////////
void Logger::Log(ELogLevel level, ELogConfigField config,
                 const char *psClientName, const char *sourceFile,
                 const char *functionName, __int32 sourceLine,
                 const char *message, va_list va) {
  LogMessage logMessage(level, config, psClientName, sourceFile, functionName,
                        sourceLine, message, va);
  if (FrameworkUserLogger::GetInstance()->IsErrorLoggingEnabled() &&
      (LL_ERROR == level || LL_CRITICAL == level)) {
    FrameworkUserLogger::GetInstance()->PrintError(
        logMessage.GetFormattedMessage());
  }

  for (int i = 0; i < MAX_LOG_HANDLERS && m_logHandlers[i]; i++) {
    if (m_logHandlers[i] != nullptr) {
      m_logHandlers[i]->Log(logMessage);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient Ctor
/////////////////////////////////////////////////////////////////////////////////////////
LoggerClient::LoggerClient(const char * /*clientHandle*/, ELogLevel loglevel) {
  m_logLevel = loglevel;
  m_eLogConfig = (ELogConfigField)(LCF_LINE_TID | LCF_LINE_TIME |
                                   LCF_LINE_CLIENT_NAME | LCF_LINE_LOG_LEVEL);
  m_handle = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient Dtor
/////////////////////////////////////////////////////////////////////////////////////////
inline LoggerClient::~LoggerClient() {}

/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient::Log
/////////////////////////////////////////////////////////////////////////////////////////
void LoggerClient::Log(ELogLevel level, const char *sourceFile,
                       const char *functionName, __int32 sourceLine, ...) {
  if (!Logger::GetInstance() || m_logLevel > level)
    return;
  va_list va;
  va_start(va, sourceLine);
  const char *message = va_arg(va, char *);
  assert(message && "Printf-style format string in LOG_XXX is NULL");

  Logger::GetInstance()->Log(level, m_eLogConfig, "", sourceFile, functionName,
                             sourceLine, message, va);

  va_end(va);
}

/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient::LogArgList
/////////////////////////////////////////////////////////////////////////////////////////
void LoggerClient::LogArgList(ELogLevel level, const char *sourceFile,
                              const char *functionName, __int32 sourceLine,
                              const char *message, va_list va) {
  if (!Logger::GetInstance() || m_logLevel > level)
    return;

  Logger::GetInstance()->Log(level, m_eLogConfig, "", sourceFile, functionName,
                             sourceLine, message, va);
}

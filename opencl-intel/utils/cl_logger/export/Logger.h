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

#pragma once

#include "cl_logger.h"
#include "cl_synch_objects.h"
#include "cl_user_logger.h"
#include "log_handler.h"
#include "log_message.h"

#include <mutex>
#include <stdio.h>

namespace Intel {
namespace OpenCL {
namespace Utils {
class IAtExitCentralPoint;

/*******************************************************************************
 * Class name: LoggerClient
 *
 * Description: Module programmer interface for logging
 *              Programmers willing to log messages in their module, need to
 *              create a logger client for their module (or more than one), and
 *              log messages using it's Log interface
 ******************************************************************************/
class LoggerClient {
public:
  /*****************************************************************************
   * Function: LoggerClient
   * Description: The LoggerClient class constructor - creater logger client
   * Arguments: handle [in] - client unique string handle
   *            loglevel [in] - log level
   ****************************************************************************/
  LoggerClient(const char *handle, ELogLevel loglevel);

  /*****************************************************************************
   * Function: ~LoggerClient
   * Description: The LoggerClient class destructor
   * Arguments:
   ****************************************************************************/
  virtual ~LoggerClient();

  /*****************************************************************************
   * Function: Log(W)
   * Description: log message to the logger client
   * Arguments: level [in] - message log level
   *            sourceFile [in] - the filename where the message was generated
   *            functionName [in] - the functionName where the message was
   *                                generated
   *            sourceLine [in] - the sourceLine where the message was generated
   *            message [in] - the message body with a variable arguments list
   * Return value:
   ****************************************************************************/
  void Log(ELogLevel level, const char *sourceFile, const char *functionName,
           __int32 sourceLine, ...);

  /*****************************************************************************
   * Function: Log
   * Description: log message to the logger client
   * Arguments: level [in] - message log level
   *            sourceFile [in] - the filename where the message was generated
   *            functionName [in] - the functionName where the message was
   *                                generated
   *            sourceLine [in] - the sourceLine where the message was generated
   *            message [in] - the message body with a variable arguments list
   *            va [in] - the message variable arguments list
   * Return value:
   ****************************************************************************/
  void LogArgList(ELogLevel level, const char *sourceFile,
                  const char *functionName, __int32 sourceLine,
                  const char *message, va_list va);

private:
  char *m_handle;               // unique string handle representation
  ELogLevel m_logLevel;         // client log level (ignore levels < m_logLevel)
  ELogConfigField m_eLogConfig; // configuration flags
};

/*******************************************************************************
 * Class name: Logger
 *
 * Description: represents the logging interface for clients.
 ******************************************************************************/
class Logger {
public:
  /*****************************************************************************
   * Function: Logger
   * Description: The Logger class constructor
   * Arguments:
   ****************************************************************************/
  Logger();

  /*****************************************************************************
   * Function: ~Logger
   * Description: The Logger class destructor
   * Arguments:
   ****************************************************************************/
  virtual ~Logger();

  /*****************************************************************************
   * Function: GetInstance
   * Description: returns the Logger instance
   *              The logger is implemented as a singleton. in order to get an
   *              instance of the logger module users should call this method
   * Arguments:
   * Return value: an instance of the logger
   ****************************************************************************/
  static Logger *GetInstance();

  /*****************************************************************************
   * Function: Log
   * Description: instruct the logger to log message. the logger will propagate
   *              the message to all log handlers.
   * Arguments: level [in] - message log level
   *            sourceFile [in] - the filename where the message was generated
   *            functionName [in]  - the functionName where the message was
   *                                 generated
   *            sourceLine [in] - the sourceLine where the message was generated
   *            message [in]    - the message body
   *            va [in] - the message variable arguments list
   * Return value:
   ****************************************************************************/
  void Log(ELogLevel level, ELogConfigField config, const char *psClientName,
           const char *sourceFile, const char *functionName, __int32 sourceLine,
           const char *message, va_list va);

  /*****************************************************************************
   * Function: GetLogHandlerParams
   * Description: add new logger handler
   *              Logger dispatches log messages to all Added loghandlers. in
   *              case log handler's level <= logMessage's level, the message
   *              will be sent to output
   * Arguments: handle [in] - log handler unique string handle
   * Return value:
   ****************************************************************************/
  const char *GetLogHandlerParams(const char *handle);

  /*****************************************************************************
   * Function: AddLogHandler
   * Description: add new logger handler
   *              Logger dispatches log messages to all Added loghandlers. in
   *              case log handler's level <= logMessage's level, the message
   *              will be sent to output
   * Arguments: logHandler [in] - pointer to LogHandler instance
   * Return value:
   ****************************************************************************/
  cl_err_code AddLogHandler(LogHandler *logHandler);

  void SetActive(const bool bActive) { m_bIsActive = bActive; }

  bool IsActive() const { return m_bIsActive; }

private:
  // each logging output generator is being represented by a LogHandler. Logger
  // will propagate the LogMessages to all registered LogHandlers. Each will
  // decide whether to emit the message according to its own LogLevel and the
  // message's LogLevel.
  LogHandler *m_logHandlers[MAX_LOG_HANDLERS];

  bool m_bIsActive;

  // The class critical section object.
  std::mutex m_CS;
};

} // namespace Utils
} // namespace OpenCL
} // namespace Intel

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

#define _CRT_SECURE_NO_WARNINGS 1

#include "log_handler.h"

#include "cl_synch_objects.h"
#include "cl_sys_defines.h"
#include "cl_user_logger.h"

#include <assert.h>
#include <malloc.h>
#include <string.h>

using namespace Intel::OpenCL::Utils;

#if defined(_WIN32)
#define WCSDUP _wcsdup
#else
#define WCSDUP wcsdup
#endif

#define MAX_STRDUP_SIZE 1024

namespace Intel {
namespace OpenCL {
namespace Utils {

static llvm::ManagedStatic<FrameworkUserLogger> UserLogger;

FrameworkUserLogger *FrameworkUserLogger::GetInstance() {
  return &*UserLogger;
}
}
} // namespace OpenCL
} // namespace Intel

/**
 * Safe version of strdup.
 */
char *strdup_safe(const char *src) {
  size_t actual = strlen(src);
  actual = (actual > MAX_STRDUP_SIZE) ? MAX_STRDUP_SIZE : actual;

  char *retStr = (char *)malloc((actual + 1) * sizeof(char));
  if (nullptr == retStr)
    return nullptr;

  STRCPY_S(retStr, actual + 1, src);
  return retStr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileDescriptorLogHandler Ctor Implementation
/////////////////////////////////////////////////////////////////////////////////////////
FileDescriptorLogHandler::FileDescriptorLogHandler(const char *handle)
    : LogHandler(), m_fileHandler(nullptr), m_dupStderr(-1) {
  if (nullptr != handle) {
    m_handle = strdup_safe(handle);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileDescriptorLogHandler Dtor
/////////////////////////////////////////////////////////////////////////////////////////
FileDescriptorLogHandler::~FileDescriptorLogHandler() {
  if (nullptr != m_handle) {
    free(m_handle);
    m_handle = nullptr;
  }

  if (-1 != m_dupStderr) {
    // redirect back stderr
    DUP2(m_dupStderr, fileno(stderr));
    m_dupStderr = -1;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileDescriptorLogHandler::Init
/////////////////////////////////////////////////////////////////////////////////////////
cl_err_code FileDescriptorLogHandler::Init(ELogLevel level,
                                           const char * /*fileName*/,
                                           const char *title, FILE *fileDesc) {
  if (nullptr == m_handle) {
    return CL_ERR_INITILIZATION_FAILED;
  }

  if (nullptr == fileDesc) {
    return CL_ERR_LOGGER_FAILED;
  }

  m_fileHandler = fileDesc;

  m_logLevel = level; // retrieve this info from Logger (not implemented yet)

  // redirect stderr to fileDesc (in order to get log messages from MIC device)
  fflush(stderr);
  // Let m_dupStderr refers to stderr
  m_dupStderr = DUP(fileno(stderr));
  if (-1 != m_dupStderr)
    // If succeed, stderr will refer to m_fileHandler.
    // If an error occurs, we don't need exit. Diagnostic or error messages
    // are typically attached to the user's terminal instead of m_fileHandler
    DUP2(fileno(m_fileHandler), fileno(stderr));

  const char *pTitle =
      (nullptr == title)
          ? "\n################################################################"
            "##########################################\n"
          : title;

  // fputs is thread safe.
  if (EOF == fputs(pTitle, m_fileHandler)) {
    printf("fwrite failed\n");
    assert(false);
    return CL_ERR_LOGGER_FAILED;
  }
  Flush();

  return CL_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileDescriptorLogHandler::Log
/////////////////////////////////////////////////////////////////////////////////////////
void FileDescriptorLogHandler::Log(LogMessage &logMessage) {
  if (m_logLevel > logMessage.GetLogLevel()) {
    // ignore messages with lower log level
    return;
  }

  char *formattedMsg = logMessage.GetFormattedMessage();
  // error logging still causes some link errors in Linux
  // TODO: here we should print error in user logger
  // fputs is thread safe.
  if (EOF == fputs(formattedMsg, m_fileHandler)) {
    printf("fwrite failed\n");
    assert(false);
    return;
  }
  Flush();

  return;
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileDescriptorLogHandler::Flush
/////////////////////////////////////////////////////////////////////////////////////////
void FileDescriptorLogHandler::Flush() {
  if (m_fileHandler) {
    fflush(m_fileHandler); // thread safe
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler Ctor Implementation
/////////////////////////////////////////////////////////////////////////////////////////
FileLogHandler::FileLogHandler(const char *handle)
    : FileDescriptorLogHandler(handle), m_fileName(nullptr) {}

/////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler Dtor Implementation
/////////////////////////////////////////////////////////////////////////////////////////
FileLogHandler::~FileLogHandler() {
  if (nullptr != m_fileHandler) {
    fclose(m_fileHandler);
    m_fileHandler = nullptr;
  }

  if (nullptr != m_fileName) {
    free(m_fileName);
    m_fileName = nullptr;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler::Init
/////////////////////////////////////////////////////////////////////////////////////////
cl_err_code FileLogHandler::Init(ELogLevel level, const char *fileName,
                                 const char *title, FILE * /*fileDesc*/) {
  if (m_handle == nullptr) {
    return CL_ERR_INITILIZATION_FAILED;
  }
  if (nullptr == fileName) {
    printf("logger initialization failed, fileName must be valid pointer\n");
    return CL_ERR_LOGGER_FAILED;
  }
  m_fileName = strdup_safe(fileName);
  FILE *tFileHandler = nullptr;
  if (m_fileName) {
    tFileHandler = fopen(m_fileName, "w");
    if (nullptr == tFileHandler) {
      printf("can't open log file for writing\n");
      return CL_ERR_LOGGER_FAILED;
    }
  }
  return FileDescriptorLogHandler::Init(level, fileName, title, tFileHandler);
}

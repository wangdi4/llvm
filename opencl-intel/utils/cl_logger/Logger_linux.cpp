// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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

#include <stdio.h>
#include <stdarg.h>
#include <sstream>              // required by: owstringstream
#include <assert.h>
#include <malloc.h>

#include "cl_secure_string_linux.h"
#include "cl_shutdown.h"

using namespace Intel::OpenCL::Utils;

#ifndef DEVICE_NATIVE
	// use only global handler - do not use local one
	USE_SHUTDOWN_HANDLER(nullptr); 
#endif

void Logger::RegisterGlobalAtExitNotification( IAtExitCentralPoint* fn )
{
    Intel::OpenCL::Utils::RegisterGlobalAtExitNotification(fn);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger Ctor
/////////////////////////////////////////////////////////////////////////////////////////
Logger::Logger()
{
    m_bIsActive = false;
    memset(m_logHandlers, 0, sizeof(m_logHandlers));
}


/////////////////////////////////////////////////////////////////////////////////////////
// Logger Dtor
/////////////////////////////////////////////////////////////////////////////////////////
Logger::~Logger()
{

}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::GetInstance
/////////////////////////////////////////////////////////////////////////////////////////
struct LoggerSingletonHandler
{
    LoggerSingletonHandler()
    {
        pLogger = new Logger;
    }

    ~LoggerSingletonHandler()
    {
        delete(pLogger);
    }

    // Pointer to a singleton object
    static Logger*    pLogger;
};

Logger* LoggerSingletonHandler::pLogger = nullptr;

LoggerSingletonHandler    logger;

Logger& Logger::GetInstance()
{
    return *LoggerSingletonHandler::pLogger;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::AddLogHandler
/////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Logger::AddLogHandler(LogHandler* logHandler)
{

    OclAutoMutex CS(&m_CS); // Lock the function
    int    i;
    for (i = 0; i < MAX_LOG_HANDLERS; i++)
    {
        if (m_logHandlers[i] == logHandler)
        {
            return CL_ERR_LOGGER_FAILED;
        }
        if (m_logHandlers[i] == nullptr)
        {
            m_logHandlers[i] = logHandler;
            return CL_SUCCESS;
        }
    }
    return CL_ERR_LOGGER_FAILED;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::GetLogHandlerParams
/////////////////////////////////////////////////////////////////////////////////////////
const char* Logger::GetLogHandlerParams(const char* logHandler)
{
    // not implemented yet
    assert(false);
    return "";
}

/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient Ctor
/////////////////////////////////////////////////////////////////////////////////////////
LoggerClient::LoggerClient(const char* clientHandle, ELogLevel loglevel)
{
    m_logLevel = loglevel;
    m_eLogConfig =    (ELogConfigField)(LCF_LINE_TID | LCF_LINE_TIME |
                                        LCF_LINE_CLIENT_NAME | LCF_LINE_LOG_LEVEL);
    m_handle = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient Dtor
/////////////////////////////////////////////////////////////////////////////////////////
inline LoggerClient::~LoggerClient()
{

}

/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient::Log
/////////////////////////////////////////////////////////////////////////////////////////
void LoggerClient::Log(ELogLevel level, const char* sourceFile, const char* functionName, __int32 sourceLine, const char* message, ...)
{
    if (m_logLevel > level)
    {
        return;
    }
    va_list va;
    va_start(va, message);

    Logger::GetInstance().Log(level, m_eLogConfig, "", sourceFile, functionName,  sourceLine, message, va);

    va_end( va );
}

/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient::LogArgList
/////////////////////////////////////////////////////////////////////////////////////////
void LoggerClient::LogArgList(ELogLevel level, const char* sourceFile, const char* functionName, __int32 sourceLine, const char* message, va_list va)
{
    if (m_logLevel > level)
    {
        return;
    }
    Logger::GetInstance().Log(level, m_eLogConfig, "", sourceFile, functionName,  sourceLine, message, va);
}
